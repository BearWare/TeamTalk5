package dk.bearware.backend;

import android.content.Context;
import android.media.AudioDeviceInfo;
import android.media.AudioManager;
import android.os.Build;

import androidx.annotation.ChecksSdkIntAtLeast;
import androidx.annotation.RequiresApi;

import java.util.ArrayList;
import java.util.List;

import dk.bearware.gui.R;

/**
 * Selects which microphone is used while voice preprocessing is enabled.
 *
 * OpenSL ES cannot record from a specific device, so the selection is made
 * with AudioManager.setCommunicationDevice(), which routes voice
 * communication audio (Android 12 and later). Bluetooth headsets are left to
 * BluetoothHeadsetHelper.
 */
public class CommunicationDeviceHelper {

    /** Selection key for the phone's own microphone. */
    public static final String BUILTIN = "builtin";

    @ChecksSdkIntAtLeast(api = Build.VERSION_CODES.S)
    public static boolean isSupported() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.S;
    }

    /** Selection keys of the microphones that can be selected right now. */
    @RequiresApi(Build.VERSION_CODES.S)
    public static List<String> getKeys(AudioManager audioManager) {
        List<String> keys = new ArrayList<>();
        keys.add(BUILTIN);
        for (AudioDeviceInfo dev : audioManager.getAvailableCommunicationDevices()) {
            if (isExternal(dev) && !keys.contains(getKey(dev)))
                keys.add(getKey(dev));
        }
        return keys;
    }

    /** Human readable name of a microphone returned by getKeys(). */
    @RequiresApi(Build.VERSION_CODES.S)
    public static String getName(Context context, AudioManager audioManager, String key) {
        if (BUILTIN.equals(key))
            return context.getString(R.string.pref_microphone_builtin);
        AudioDeviceInfo dev = findDevice(audioManager, key);
        if (dev == null)
            return context.getString(R.string.pref_microphone_unavailable);
        if (dev.getType() == AudioDeviceInfo.TYPE_WIRED_HEADSET)
            return context.getString(R.string.pref_microphone_wired);
        return dev.getProductName().toString();
    }

    /**
     * Route voice communication to the selected microphone.
     *
     * @return true if the microphone was selected, false if the selection is
     * empty or the device is not connected.
     */
    @RequiresApi(Build.VERSION_CODES.S)
    public static boolean apply(AudioManager audioManager, String key, boolean speakerphone) {
        AudioDeviceInfo dev = findDevice(audioManager, key);
        if (dev == null)
            return false;
        if (BUILTIN.equals(key)) {
            // the phone's microphone is paired with either the earpiece or the speaker
            int type = speakerphone ? AudioDeviceInfo.TYPE_BUILTIN_SPEAKER : AudioDeviceInfo.TYPE_BUILTIN_EARPIECE;
            for (AudioDeviceInfo d : audioManager.getAvailableCommunicationDevices()) {
                if (d.getType() == type)
                    dev = d;
            }
        }
        return audioManager.setCommunicationDevice(dev);
    }

    @RequiresApi(Build.VERSION_CODES.S)
    public static void clear(AudioManager audioManager) {
        audioManager.clearCommunicationDevice();
    }

    @RequiresApi(Build.VERSION_CODES.S)
    private static AudioDeviceInfo findDevice(AudioManager audioManager, String key) {
        if (key == null || key.isEmpty())
            return null;
        for (AudioDeviceInfo dev : audioManager.getAvailableCommunicationDevices()) {
            if (key.equals(getKey(dev)))
                return dev;
        }
        return null;
    }

    @RequiresApi(Build.VERSION_CODES.S)
    private static String getKey(AudioDeviceInfo dev) {
        switch (dev.getType()) {
            case AudioDeviceInfo.TYPE_BUILTIN_EARPIECE:
            case AudioDeviceInfo.TYPE_BUILTIN_SPEAKER:
                return BUILTIN;
            default:
                // device IDs change when a device is reconnected, so use type and address
                return dev.getType() + ":" + dev.getAddress();
        }
    }

    private static boolean isExternal(AudioDeviceInfo dev) {
        switch (dev.getType()) {
            case AudioDeviceInfo.TYPE_WIRED_HEADSET:
            case AudioDeviceInfo.TYPE_USB_HEADSET:
            case AudioDeviceInfo.TYPE_USB_DEVICE:
                return true;
            default:
                return false;
        }
    }
}
