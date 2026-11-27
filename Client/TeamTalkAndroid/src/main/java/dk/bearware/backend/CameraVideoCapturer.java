/*
 * Copyright (c) 2005-2026, BearWare.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 */

package dk.bearware.backend;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.util.Range;
import android.util.Size;
import android.view.Surface;
import android.view.WindowManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.content.ContextCompat;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * Captures frames from Android Camera2 API, converts YUV_420_888 to upright I420,
 * and delivers them to TeamTalk's native video capture pipeline.
 */
public class CameraVideoCapturer {

    public static final String TAG = "bearware";
    private static final int TARGET_WIDTH = 640;
    private static final int TARGET_HEIGHT = 480;
    private static final int TARGET_FPS = 15;
    private static final long MIN_FRAME_INTERVAL_NS = 1_000_000_000L / TARGET_FPS;

    public interface FrameConsumer {
        void onI420Frame(byte[] i420Data, int width, int height);
        void onCameraError(String reason);
    }

    private final Context appContext;
    private final FrameConsumer frameConsumer;
    private final Object stateLock = new Object();

    private HandlerThread cameraThread;
    private Handler cameraHandler;
    private CameraDevice cameraDevice;
    private CameraCaptureSession captureSession;
    private ImageReader imageReader;
    private Surface previewSurface;

    private String activeCameraId;
    private boolean usingFrontCamera = true;
    private int captureWidth = TARGET_WIDTH;
    private int captureHeight = TARGET_HEIGHT;
    private int outputWidth = TARGET_HEIGHT;
    private int outputHeight = TARGET_WIDTH;
    private int rotationDegrees = 0;
    private Range<Integer> targetFpsRange;
    private byte[] i420Buffer;
    private byte[] planeScratch;
    private long lastFrameTimeNs = 0L;
    private volatile boolean started = false;

    public CameraVideoCapturer(@NonNull Context context, @NonNull FrameConsumer consumer) {
        this.appContext = context.getApplicationContext();
        this.frameConsumer = consumer;
    }

    public int getOutputWidth() {
        return outputWidth;
    }

    public int getOutputHeight() {
        return outputHeight;
    }

    public int getTargetFps() {
        return TARGET_FPS;
    }

    public boolean isUsingFrontCamera() {
        return usingFrontCamera;
    }

    public boolean isStarted() {
        return started;
    }

    public boolean hasMultipleCameras() {
        CameraManager cm = (CameraManager) appContext.getSystemService(Context.CAMERA_SERVICE);
        if (cm == null) return false;
        try {
            return cm.getCameraIdList().length > 1;
        } catch (CameraAccessException e) {
            return false;
        }
    }

    /**
     * Resolves camera ID, capture dimensions, and upright output dimensions before opening.
     */
    public boolean prepare(boolean preferFront) {
        CameraManager cm = (CameraManager) appContext.getSystemService(Context.CAMERA_SERVICE);
        if (cm == null) {
            return false;
        }
        try {
            String[] ids = cm.getCameraIdList();
            if (ids.length == 0) {
                return false;
            }
            int desiredFacing = preferFront
                    ? CameraCharacteristics.LENS_FACING_FRONT
                    : CameraCharacteristics.LENS_FACING_BACK;
            String chosenId = null;
            int chosenFacing = desiredFacing;

            for (String id : ids) {
                CameraCharacteristics cc = cm.getCameraCharacteristics(id);
                Integer facing = cc.get(CameraCharacteristics.LENS_FACING);
                if (facing != null && facing == desiredFacing) {
                    chosenId = id;
                    chosenFacing = facing;
                    break;
                }
            }
            if (chosenId == null) {
                chosenId = ids[0];
                CameraCharacteristics cc = cm.getCameraCharacteristics(chosenId);
                Integer facing = cc.get(CameraCharacteristics.LENS_FACING);
                chosenFacing = (facing != null) ? facing : CameraCharacteristics.LENS_FACING_BACK;
            }

            CameraCharacteristics chars = cm.getCameraCharacteristics(chosenId);
            StreamConfigurationMap map = chars.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            Size best = pickBestSize(map != null ? map.getOutputSizes(ImageFormat.YUV_420_888) : null);
            Range<Integer> bestFpsRange = pickBestFpsRange(
                    chars.get(CameraCharacteristics.CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES));

            Integer sensorOrientationObj = chars.get(CameraCharacteristics.SENSOR_ORIENTATION);
            int sensorOrientation = (sensorOrientationObj != null) ? sensorOrientationObj : 0;
            int displayDegrees = getDisplayRotationDegrees();

            int rot;
            if (chosenFacing == CameraCharacteristics.LENS_FACING_FRONT) {
                rot = (sensorOrientation + displayDegrees) % 360;
            } else {
                rot = (sensorOrientation - displayDegrees + 360) % 360;
            }

            synchronized (stateLock) {
                activeCameraId = chosenId;
                usingFrontCamera = (chosenFacing == CameraCharacteristics.LENS_FACING_FRONT);
                captureWidth = best.getWidth();
                captureHeight = best.getHeight();
                rotationDegrees = rot;
                targetFpsRange = bestFpsRange;
                if (rot == 90 || rot == 270) {
                    outputWidth = captureHeight;
                    outputHeight = captureWidth;
                } else {
                    outputWidth = captureWidth;
                    outputHeight = captureHeight;
                }
                i420Buffer = new byte[(outputWidth * outputHeight * 3) / 2];
            }
            return true;
        } catch (CameraAccessException e) {
            Log.e(TAG, "Camera prepare failed", e);
            return false;
        }
    }

    @SuppressLint("MissingPermission")
    public boolean start(@Nullable Surface preview) {
        if (ContextCompat.checkSelfPermission(appContext, Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED) {
            return false;
        }

        CameraManager cm = (CameraManager) appContext.getSystemService(Context.CAMERA_SERVICE);
        if (cm == null) {
            return false;
        }

        synchronized (stateLock) {
            if (activeCameraId == null && !prepare(usingFrontCamera)) {
                return false;
            }
            previewSurface = preview;
            lastFrameTimeNs = 0L;
            started = true;

            cameraThread = new HandlerThread("TTCamera2Thread");
            cameraThread.start();
            cameraHandler = new Handler(cameraThread.getLooper());

            imageReader = ImageReader.newInstance(captureWidth, captureHeight, ImageFormat.YUV_420_888, 3);
            imageReader.setOnImageAvailableListener(this::onImageAvailable, cameraHandler);

            try {
                cm.openCamera(activeCameraId, deviceStateCallback, cameraHandler);
                return true;
            } catch (CameraAccessException | SecurityException | IllegalArgumentException e) {
                Log.e(TAG, "Failed to open camera " + activeCameraId, e);
                stopLocked();
                return false;
            }
        }
    }

    public void setPreviewSurface(@Nullable Surface surface) {
        synchronized (stateLock) {
            previewSurface = surface;
            if (started && cameraDevice != null && cameraHandler != null) {
                cameraHandler.post(this::recreateSessionSafe);
            }
        }
    }

    public void stop() {
        synchronized (stateLock) {
            stopLocked();
        }
    }

    private void stopLocked() {
        started = false;
        if (captureSession != null) {
            try {
                captureSession.stopRepeating();
            } catch (Exception ignored) {
            }
            try {
                captureSession.close();
            } catch (Exception ignored) {
            }
            captureSession = null;
        }
        if (cameraDevice != null) {
            try {
                cameraDevice.close();
            } catch (Exception ignored) {
            }
            cameraDevice = null;
        }
        if (imageReader != null) {
            try {
                imageReader.close();
            } catch (Exception ignored) {
            }
            imageReader = null;
        }
        if (cameraThread != null) {
            cameraThread.quitSafely();
            cameraThread = null;
            cameraHandler = null;
        }
        previewSurface = null;
    }

    private final CameraDevice.StateCallback deviceStateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(@NonNull CameraDevice camera) {
            synchronized (stateLock) {
                if (!started) {
                    camera.close();
                    return;
                }
                cameraDevice = camera;
                createCaptureSessionLocked();
            }
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice camera) {
            synchronized (stateLock) {
                if (cameraDevice == camera) {
                    stopLocked();
                } else {
                    camera.close();
                }
            }
            frameConsumer.onCameraError("Camera disconnected");
        }

        @Override
        public void onError(@NonNull CameraDevice camera, int error) {
            synchronized (stateLock) {
                if (cameraDevice == camera) {
                    stopLocked();
                } else {
                    camera.close();
                }
            }
            frameConsumer.onCameraError("Camera error: " + error);
        }
    };

    private void recreateSessionSafe() {
        synchronized (stateLock) {
            if (!started || cameraDevice == null || imageReader == null) {
                return;
            }
            if (captureSession != null) {
                try {
                    captureSession.close();
                } catch (Exception ignored) {
                }
                captureSession = null;
            }
            createCaptureSessionLocked();
        }
    }

    @SuppressWarnings("deprecation")
    private void createCaptureSessionLocked() {
        if (!started || cameraDevice == null || imageReader == null) {
            return;
        }
        List<Surface> targets = new ArrayList<>(2);
        targets.add(imageReader.getSurface());
        if (previewSurface != null && previewSurface.isValid()) {
            targets.add(previewSurface);
        }

        try {
            cameraDevice.createCaptureSession(targets, new CameraCaptureSession.StateCallback() {
                @Override
                public void onConfigured(@NonNull CameraCaptureSession session) {
                    synchronized (stateLock) {
                        if (!started || cameraDevice == null || imageReader == null) {
                            session.close();
                            return;
                        }
                        captureSession = session;
                        try {
                            CaptureRequest.Builder builder;
                            try {
                                builder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_RECORD);
                            } catch (CameraAccessException e) {
                                builder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
                            }
                            builder.addTarget(imageReader.getSurface());
                            if (previewSurface != null && previewSurface.isValid()) {
                                builder.addTarget(previewSurface);
                            }
                            builder.set(CaptureRequest.CONTROL_MODE,
                                    CaptureRequest.CONTROL_MODE_AUTO);
                            builder.set(CaptureRequest.CONTROL_AE_MODE,
                                    CaptureRequest.CONTROL_AE_MODE_ON);
                            builder.set(CaptureRequest.CONTROL_AWB_MODE,
                                    CaptureRequest.CONTROL_AWB_MODE_AUTO);
                            builder.set(CaptureRequest.CONTROL_AE_ANTIBANDING_MODE,
                                    CaptureRequest.CONTROL_AE_ANTIBANDING_MODE_AUTO);
                            builder.set(CaptureRequest.CONTROL_AF_MODE,
                                    CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_VIDEO);
                            if (targetFpsRange != null) {
                                builder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE, targetFpsRange);
                            }
                            session.setRepeatingRequest(builder.build(), null, cameraHandler);
                        } catch (Exception e) {
                            Log.e(TAG, "Failed to start repeating camera request", e);
                            frameConsumer.onCameraError("Capture request failed");
                        }
                    }
                }

                @Override
                public void onConfigureFailed(@NonNull CameraCaptureSession session) {
                    Log.e(TAG, "Camera capture session configuration failed");
                    frameConsumer.onCameraError("Session configuration failed");
                }
            }, cameraHandler);
        } catch (CameraAccessException | IllegalStateException e) {
            Log.e(TAG, "createCaptureSession failed", e);
            frameConsumer.onCameraError("Camera session failed");
        }
    }

    private void onImageAvailable(ImageReader reader) {
        Image image = null;
        try {
            image = reader.acquireLatestImage();
            if (image == null || !started) {
                return;
            }
            long nowNs = System.nanoTime();
            if (nowNs - lastFrameTimeNs < MIN_FRAME_INTERVAL_NS) {
                return;
            }
            lastFrameTimeNs = nowNs;

            byte[] out;
            int outW;
            int outH;
            int rot;
            synchronized (stateLock) {
                if (!started || i420Buffer == null) {
                    return;
                }
                out = i420Buffer;
                outW = outputWidth;
                outH = outputHeight;
                rot = rotationDegrees;
            }

            if (yuv420ToRotatedI420(image, rot, out, outW, outH)) {
                frameConsumer.onI420Frame(out, outW, outH);
            }
        } catch (Exception e) {
            Log.e(TAG, "Error processing camera frame", e);
        } finally {
            if (image != null) {
                image.close();
            }
        }
    }

    private boolean yuv420ToRotatedI420(
            @NonNull Image image,
            int rotationDegrees,
            @NonNull byte[] out,
            int outW,
            int outH) {
        int srcW = image.getWidth();
        int srcH = image.getHeight();
        Image.Plane[] planes = image.getPlanes();
        if (planes.length < 3) {
            return false;
        }

        int ySize = outW * outH;
        int uvSize = (outW / 2) * (outH / 2);
        if (out.length < ySize + uvSize * 2) {
            return false;
        }

        rotatePlane(planes[0].getBuffer(), srcW, srcH,
                planes[0].getRowStride(), planes[0].getPixelStride(),
                out, 0, outW, outH, rotationDegrees);

        int srcChromaW = srcW / 2;
        int srcChromaH = srcH / 2;
        int outChromaW = outW / 2;
        int outChromaH = outH / 2;

        rotatePlane(planes[1].getBuffer(), srcChromaW, srcChromaH,
                planes[1].getRowStride(), planes[1].getPixelStride(),
                out, ySize, outChromaW, outChromaH, rotationDegrees);

        rotatePlane(planes[2].getBuffer(), srcChromaW, srcChromaH,
                planes[2].getRowStride(), planes[2].getPixelStride(),
                out, ySize + uvSize, outChromaW, outChromaH, rotationDegrees);

        return true;
    }

    private void rotatePlane(
            @NonNull ByteBuffer srcBuf,
            int srcW,
            int srcH,
            int rowStride,
            int pixelStride,
            @NonNull byte[] dst,
            int dstOffset,
            int dstW,
            int dstH,
            int rotationDegrees) {
        ByteBuffer dup = srcBuf.duplicate();
        dup.position(0);
        int len = dup.remaining();
        if (planeScratch == null || planeScratch.length < len) {
            planeScratch = new byte[len];
        }
        byte[] src = planeScratch;
        dup.get(src, 0, len);

        if (rotationDegrees == 0) {
            for (int y = 0; y < srcH; y++) {
                int srcRow = y * rowStride;
                int dstRow = dstOffset + y * dstW;
                if (pixelStride == 1) {
                    System.arraycopy(src, srcRow, dst, dstRow, srcW);
                } else {
                    for (int x = 0; x < srcW; x++) {
                        dst[dstRow + x] = src[srcRow + x * pixelStride];
                    }
                }
            }
        } else if (rotationDegrees == 90) {
            for (int y = 0; y < srcH; y++) {
                int srcRow = y * rowStride;
                int dx = srcH - 1 - y;
                for (int x = 0; x < srcW; x++) {
                    dst[dstOffset + x * dstW + dx] = src[srcRow + x * pixelStride];
                }
            }
        } else if (rotationDegrees == 180) {
            for (int y = 0; y < srcH; y++) {
                int srcRow = y * rowStride;
                int dy = srcH - 1 - y;
                int dstRow = dstOffset + dy * dstW;
                for (int x = 0; x < srcW; x++) {
                    dst[dstRow + (srcW - 1 - x)] = src[srcRow + x * pixelStride];
                }
            }
        } else { // 270 degrees clockwise
            for (int y = 0; y < srcH; y++) {
                int srcRow = y * rowStride;
                for (int x = 0; x < srcW; x++) {
                    int dy = srcW - 1 - x;
                    dst[dstOffset + dy * dstW + y] = src[srcRow + x * pixelStride];
                }
            }
        }
    }

    private Size pickBestSize(@Nullable Size[] sizes) {
        if (sizes == null || sizes.length == 0) {
            return new Size(TARGET_WIDTH, TARGET_HEIGHT);
        }
        Size best = null;
        int bestScore = Integer.MAX_VALUE;
        for (Size s : sizes) {
            int w = s.getWidth();
            int h = s.getHeight();
            if ((w % 2) != 0 || (h % 2) != 0 || w < 160 || h < 120) {
                continue;
            }
            // Prefer 16-aligned resolutions <= 640x480 and closest to 640x480 for VP8
            int penalty = (w > TARGET_WIDTH || h > TARGET_HEIGHT) ? 10000 : 0;
            if ((w % 16) != 0 || (h % 16) != 0) {
                penalty += 200;
            }
            int score = penalty + Math.abs(w - TARGET_WIDTH) + Math.abs(h - TARGET_HEIGHT);
            if (score < bestScore) {
                bestScore = score;
                best = s;
            }
        }
        return (best != null) ? best : new Size(TARGET_WIDTH, TARGET_HEIGHT);
    }

    @Nullable
    private Range<Integer> pickBestFpsRange(@Nullable Range<Integer>[] ranges) {
        if (ranges == null || ranges.length == 0) {
            return null;
        }
        Range<Integer> bestDynamic = null;
        Range<Integer> bestAny = null;
        int bestUpper = Integer.MAX_VALUE;
        for (Range<Integer> r : ranges) {
            int low = r.getLower();
            int high = r.getUpper();
            if (low <= TARGET_FPS && high >= TARGET_FPS && high <= 30) {
                if (low < high) {
                    if (bestDynamic == null || high > bestDynamic.getUpper()) {
                        bestDynamic = r;
                    }
                } else if (high < bestUpper) {
                    bestAny = r;
                    bestUpper = high;
                }
            }
        }
        return (bestDynamic != null) ? bestDynamic : bestAny;
    }

    @SuppressWarnings("deprecation")
    private int getDisplayRotationDegrees() {
        WindowManager wm = (WindowManager) appContext.getSystemService(Context.WINDOW_SERVICE);
        if (wm == null || wm.getDefaultDisplay() == null) {
            return 0;
        }
        int rot = wm.getDefaultDisplay().getRotation();
        switch (rot) {
            case Surface.ROTATION_90:
                return 90;
            case Surface.ROTATION_180:
                return 180;
            case Surface.ROTATION_270:
                return 270;
            default:
                return 0;
        }
    }
}
