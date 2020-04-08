/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.Drawing.Imaging;
using System.Threading;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using c_tt;

namespace BearWare
{

    /** @addtogroup sounddevices
     * @{ */

    /**
     * @brief The supported sound systems.
     *
     * @see SoundDevice
     * @see TeamTalkBase.InitSoundInputDevice()
     * @see TeamTalkBase.InitSoundOutputDevice()
     * @see TeamTalkBase.InitSoundDuplexDevices() */
    public enum SoundSystem : uint
    {
        /** @brief Sound system denoting invalid or not found. */
        SOUNDSYSTEM_NONE = 0,
        /** @brief Windows legacy audio system. Should be used on Windows Mobile. */
        SOUNDSYSTEM_WINMM = 1,
        /** @brief DirectSound audio system. Should be used on Windows. */
        SOUNDSYSTEM_DSOUND = 2,
        /**
         * @brief Advanced Linux Sound Architecture (ALSA). Should be used on Linux.
         *
         * Often ALSA sound devices only support a limited number of
         * sample rates so TeamTalk internally use software filters to
         * resample the audio to the sample rate used by the selected
         * audio codecs. */
        SOUNDSYSTEM_ALSA = 3,
        /** @brief Core Audio. Should be used on MacOS. */
        SOUNDSYSTEM_COREAUDIO = 4,
        /** @brief Windows Audio Session API (WASAPI). Should be used
         * on Windows Vista/7/8/10.
         *
         * WASAPI audio devices typically only support a single sample
         * rate so internally TeamTalk uses software filters to
         * resample audio to the sample rate used by the selected
         * audio codecs.
         * 
         * Check @c supportedSampleRates and @c nDefaultSampleRate of
         * #BearWare.SoundDevice to see which sample rates are supported. */
        SOUNDSYSTEM_WASAPI = 5,
        /** @brief Android sound API. */
        SOUNDSYSTEM_OPENSLES_ANDROID = 7,
        /** @brief iOS sound API.
         *
         * Two sound devices will appear when calling
         * TeamTalkBase.GetSoundDevices(). Sound device ID 0 will be AudioUnit
         * subtype Remote I/O Unit and sound device ID 1 will be
         * AudioUnit subtype Voice-Processing I/O Unit.
         *
         * Note that iOS only supports one active Voice-Processing I/O
         * Unit, i.e. only one #BearWare.TeamTalkBase instance can use the
         * Voice-Processing I/O Unit.
         *
         * Add libraries @c AVFoundation.framework and
         * @c AudioToolbox.framework.
         *
         * Duplex mode is not supported by AudioUnit iOS sound API. */
        SOUNDSYSTEM_AUDIOUNIT = 8
    }

    /**
     * @brief A struct containing the properties of a sound device
     * for either playback or recording.
     *
     * Use @a nDeviceID to pass to TeamTalkBase.InitSoundInputDevice() or
     * TeamTalkBase.InitSoundOutputDevice().
     *
     * Note that the @a nDeviceID may change if the user application
     * is restarted and a new sound device is added or removed from
     * the computer.
     * 
     * @see TeamTalkBase.GetSoundDevices */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct SoundDevice
    {
        /** @brief The ID of the sound device. Used for passing to
         * TeamTalkBase.InitSoundInputDevice() and
         * TeamTalkBase.InitSoundOutputDevice(). Note that @a nDeviceID might change
         * if USB sound devices are plugged in or unplugged, therefore
         * use @a szDeviceID to ensure proper device is used.  */
        public int nDeviceID;
        /** @brief The sound system used by the sound device. */
        public SoundSystem nSoundSystem;
        /** @brief The name of the sound device. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szDeviceName;
        /** @brief An identifier uniquely identifying the sound device
         * even when new sound devices are being added and removed.  In
         * DirectSound, WASAPI and WinMM it would be the GUID of the sound
         * device. Note that it may not always be available. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szDeviceID;
        /** 
         * @brief The ID of the device used in Win32's
         * waveInGetDevCaps and waveOutGetDevCaps.
         *
         * Value will be -1 if no ID could be found This ID can also
         * be used to find the corresponding mixer on Windows passing
         * it as @a nWaveDeviceID.  Note that this ID applies both to
         * DirectSound and WinMM.
         *
         * @see WindowsMixer.GetWaveInName
         * @see WindowsMixer.GetWaveOutName
         * @see WindowsMixer.GetMixerCount */
        public int nWaveDeviceID;
        /** @brief Whether the sound device supports 3D-sound
         * effects. */
        public bool bSupports3D;
        /** @brief The maximum number of input channels. */
        public int nMaxInputChannels;
        /** @brief The maximum number of output channels. */
        public int nMaxOutputChannels;
        /** @brief Supported sample rates by device for recording. A
         * zero value terminates the list of supported sample rates or
         * its maximum size of #BearWare.TeamTalkBase.TT_SAMPLERATES_MAX. */
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = TeamTalkBase.TT_SAMPLERATES_MAX)]
        public int[] inputSampleRates;
        /** @brief Supported sample rates by device for playback. A
         * zero value terminates the list of supported sample rates or
         * its maximum size of #BearWare.TeamTalkBase.TT_SAMPLERATES_MAX. */
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = TeamTalkBase.TT_SAMPLERATES_MAX)]
        public int[] outputSampleRates;
        /** @brief The default sample rate for the sound device. */
        public int nDefaultSampleRate;
    }

    /**
     * @brief IDs for sound devices. */
    public struct SoundDeviceConstants
    {
        /** @brief Sound device ID for iOS AudioUnit subtype Remote I/O
         * Unit. @see SOUNDSYSTEM_AUDIOUNIT */
        public const int TT_SOUNDDEVICE_ID_REMOTEIO = 0;
        /** @brief Sound device ID for iOS AudioUnit subtype Voice-Processing
         * I/O Unit. @see SOUNDSYSTEM_AUDIOUNIT */
        public const int TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO = 1;
        /** @brief Sound device ID for Android OpenSL ES default audio
         * device. @see SOUNDSYSTEM_OPENSLES_ANDROID */
        public const int TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT = 0;
        /** @brief Sound device ID for virtual TeamTalk sound device.
         *
         * This is a sound device which decodes received audio packets but
         * does not send the decoded audio to a real sound device. When used
         * for recording the virtual sound device injects silence.
         *
         * In duplex mode the virtual TeamTalk sound device can only
         * be used as input/output device. @see SOUNDSYSTEM_NONE */
        public const int TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL = 1978;


        /** @brief Flag/bit in @c nDeviceID telling if the #BearWare.SoundDevice is a
         * shared version of an existing sound device.
         *
         * On Android the recording device can only be used by one TeamTalk
         * instance. As a workaround for this issue a shared recording device
         * has been introduced. Internally TeamTalk initializes
         * #TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT which then resample and
         * distribution the audio data to multiple TeamTalk instances.
         *
         * The shared audio device on Android will show up as
         * (TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT | TT_SOUNDDEVICE_ID_SHARED_FLAG),
         * i.e. 2048.
         */
        public const uint TT_SOUNDDEVICE_ID_SHARED_FLAG = 0x00000800;

        /** @brief Extract sound device ID of @c nDeviceID in #BearWare.SoundDevice by
         * and'ing this value.
         *
         * let PhysicalDeviceID = (SoundDevice.nDeviceID & TT_SOUNDDEVICE_ID_MASK). */
        public const uint TT_SOUNDDEVICE_ID_MASK = 0x000007FF;
    }

    /**
     * @brief An enum encapsulation the minimum, maximum and default sound
     * levels for input and output sound devices. */
    public struct SoundLevel
    {
        /**
         * @brief The maximum value of recorded audio.
         * @see TeamTalkBase.GetSoundInputLevel
         * @see TeamTalkBase.SetVoiceActivationLevel
         * @see TeamTalkBase.GetVoiceActivationLevel */
        public const int SOUND_VU_MAX = 100;
        /**
         * @brief The minimum value of recorded audio.
         * @see TeamTalkBase.GetSoundInputLevel
         * @see TeamTalkBase.SetVoiceActivationLevel
         * @see TeamTalkBase.GetVoiceActivationLevel */
        public const int SOUND_VU_MIN = 0;
        /**
         * @brief The maximum volume.
         *
         * @see BearWare.TeamTalkBase.SetSoundOutputVolume
         * @see BearWare.TeamTalkBase.GetSoundOutputVolume
         * @see BearWare.TeamTalkBase.SetUserVolume
         * @see SOUND_VOLUME_DEFAULT */
        public const int SOUND_VOLUME_MAX = 32000;
        /**
         * @brief The default volume. Use this whenever possible since
         * it requires the least amount of CPU usage.
         *
         * @see BearWare.TeamTalkBase.SetSoundOutputVolume
         * @see BearWare.TeamTalkBase.GetSoundOutputVolume
         * @see BearWare.TeamTalkBase.SetUserVolume */
        public const int SOUND_VOLUME_DEFAULT = 1000;
        /**
         * @brief The minimum volume.
         * @see BearWare.TeamTalkBase.SetSoundOutputVolume
         * @see BearWare.TeamTalkBase.GetSoundOutputVolume
         * @see BearWare.TeamTalkBase.SetUserVolume */
        public const int SOUND_VOLUME_MIN = 0;
        /**
         * @brief The maximum gain level. 
         *
         * A gain level of 32000 gains the volume by a factor 32.  A gain
         * level of #SOUND_GAIN_DEFAULT means no gain.
         *
         * @see BearWare.TeamTalkBase.SetSoundInputGainLevel
         * @see BearWare.TeamTalkBase.GetSoundInputGainLevel */
        public const int SOUND_GAIN_MAX = 32000;
        /**
         * @brief The default gain level.
         *
         * A gain level of 1000 means no gain. Check #SOUND_GAIN_MAX
         * and #SOUND_GAIN_MIN to see how to increase and lower gain
         * level.
         *
         * @see BearWare.TeamTalkBase.SetSoundInputGainLevel
         * @see BearWare.TeamTalkBase.GetSoundInputGainLevel */
        public const int SOUND_GAIN_DEFAULT = 1000;
        /**
         * @brief The minimum gain level (since it's zero it means
         * silence).
         *
         * A gain level of 100 is 1/10 of the default volume.
         *
         * @see BearWare.TeamTalkBase.SetSoundInputGainLevel
         * @see BearWare.TeamTalkBase.GetSoundInputGainLevel */
        public const int SOUND_GAIN_MIN = 0;
    }

    /**
     * @brief An audio block containing the raw audio from a user who
     * was talking.
     *
     * To enable audio blocks first call TeamTalkBase.EnableAudioBlockEvent()
     * then whenever new audio is played the event
     * TeamTalkBase.OnUserAudioBlock() is generated. Use
     * TeamTalkBase.AcquireUserAudioBlock() to retrieve the audio block.
     *
     * Note that each user is limited to 128 kbytes of audio data.
     *
     * @see TeamTalkBase.EnableAudioBlockEvent()
     * @see TeamTalkBase.AcquireUserAudioBlock()
     * @see TeamTalkBase.ReleaseUserAudioBlock() */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct AudioBlock
    {
        /** @brief The ID of the stream. The stream id changes every time
         * the user enables a new transmission using TeamTalkBase.EnableTransmission()
         * or through voice activation. */
        public int nStreamID;
        /** @brief The sample rate of the raw audio. */
        public int nSampleRate;
        /** @brief The number of channels used (1 for mono, 2 for stereo). */
        public int nChannels;
        /** @brief The raw audio in 16-bit integer format array. The
         * size of the array in bytes is @c sizeof(short) * @c
         * nSamples * @c nChannels. */
        public System.IntPtr lpRawAudio;
        /** @brief The number of samples in the raw audio array. */
        public int nSamples;
        /** @brief The index of the first sample in @c lpRawAudio. Its
         * value will be a multiple of @c nSamples. The sample index
         * can be used to detect overflows of the internal
         * buffer. When a user initially starts talking the @c
         * nSampleIndex will be 0 and while the user is talking @c
         * nSampleIndex will be greater than 0. When the user stops
         * talking @c nSampleIndex will be reset to 0 again. */
        public uint uSampleIndex;
    }

    /** @} */

    /** @addtogroup mediastream
     * @{ */

    /**
     * @brief Status of media file being written to disk.
     * @see CLIENTEVENT_USER_RECORD_MEDIAFILE */
    public enum MediaFileStatus : uint
    {
        MFS_CLOSED          = 0,
        /** @brief Error while processing media file. */
        MFS_ERROR           = 1,
        /** @brief Started processing media file. */
        MFS_STARTED         = 2,
        /** @brief Finished processing media file. */
        MFS_FINISHED        = 3,
        /** @brief Aborted processing of media file. */
        MFS_ABORTED         = 4,
        /** @brief Paused processing of media file. */
        MFS_PAUSED          = 5,
        /** @brief Playing media file with updated @c uElapsedMSec of
         * #BearWare.MediaFileInfo. */
        MFS_PLAYING         = 6
    }

    /**
     * @brief Media file formats supported for muxed audio recordings.
     * @see TeamTalkBase.StartRecordingMuxedAudioFile() */
    public enum AudioFileFormat : uint
    {
        /** @brief Used to denote nothing selected. */
        AFF_NONE = 0,
        /** @brief Store audio in the same format as the #BearWare.Channel's
         * configured audio codec.
         *
         * Audio is stored in OGG format.  OGG format is supported by
         * https://www.xiph.org/ogg and can be played using VLC media player
         * http://www.videolan.org
         * 
         * Requires TeamTalk version 5.2.0.4730.
         * @see TeamTalkBase.SetUserMediaStorageDir()
         * @see TeamTalkBase.StartRecordingMuxedAudioFile() */
        AFF_CHANNELCODEC_FORMAT  = 1,
        /** @brief Store in 16-bit wave format. */
        AFF_WAVE_FORMAT = 2,
        /** @brief Store in MP3-format. */
        AFF_MP3_16KBIT_FORMAT = 3,
        /** @see #AudioFileFormat.AFF_MP3_16KBIT_FORMAT */
        AFF_MP3_32KBIT_FORMAT = 4,
        /** @see #AudioFileFormat.AFF_MP3_16KBIT_FORMAT */
        AFF_MP3_64KBIT_FORMAT = 5,
        /** @see #AudioFileFormat.AFF_MP3_16KBIT_FORMAT */
        AFF_MP3_128KBIT_FORMAT = 6,
        /** @see #AudioFileFormat.AFF_MP3_16KBIT_FORMAT */
        AFF_MP3_256KBIT_FORMAT = 7,
    }

    /**
     * @brief Struct describing the audio format used by a
     * media file.
     *
     * @see TeamTalkBase.GetMediaFileInfo()
     * @see MediaFileInfo
     */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct AudioFormat
    {
        /** @brief The audio file format, e.g. wave or MP3. */
        public AudioFileFormat nAudioFmt;
        /** @brief Sample rate of media file. */
        public int nSampleRate;
        /** @brief Channels used by media file, mono = 1, stereo = 2. */
        public int nChannels;
    }

    /** @} */

    /** @addtogroup videocapture
     * @{ */

    /** 
     * @brief The picture format used by a capture device. 
     *
     * @see VideoFormat
     * @see VideoCaptureDevice */
    public enum FourCC : uint
    {
        /** @brief Internal use to denote no supported formats. */
        FOURCC_NONE     = 0,
        /** @brief Prefered image format with the lowest bandwidth
         * usage. A 640x480 pixel image takes up 460.800 bytes. */
        FOURCC_I420     = 100,
        /** @brief Image format where a 640x480 pixel images takes up
         * 614.400 bytes. */
        FOURCC_YUY2     = 101,
        /** @brief The image format with the highest bandwidth
         * usage. A 640x480 pixel images takes up 1.228.880 bytes. */
        FOURCC_RGB32    = 102
    }

    /**
     * @brief A struct containing the properties of a video capture
     * format.
     *
     * A struct for holding a supported video capture format by a 
     * #BearWare.VideoCaptureDevice. */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct VideoFormat
    {
        /** @brief The width in pixels of the video device supported
         * video format. */
        public int nWidth;
        /** @brief The height in pixels of the video device supported
         * video format. */
        public int nHeight;
        /** @brief The numerator of the video capture device's video
         * format. Divinding @a nFPS_Numerator with @a
         * nFPS_Denominator gives the frame-rate. */
        public int nFPS_Numerator;
        /** @brief The denominator of the video capture device's video
         * format. Divinding @a nFPS_Numerator with @a
         * nFPS_Denominator gives the frame-rate.*/
        public int nFPS_Denominator;
        /** @brief Picture format for capturing. */
        public FourCC picFourCC;
    }

    /**
     * @brief A RGB32 image where the pixels can be accessed directly
     * in an allocated @a imageBuffer.
     *
     * Use TeamTalkBase.AcquireUserCaptureFrame() to acquire a user's image and
     * remember to call TeamTalkBase.ReleaseUserCaptureFrame() when the image has
     * been processed so TeamTalk can release its resources. */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct VideoFrame
    {
        /** @brief The width in pixels of the image contained in @a
         * frameBuffer. */
        public int nWidth;
        /** @brief The height in pixels of the image contained in @a
         * imageBuffer. */
        public int nHeight;
        /** @brief A unique identifier for the frames which are part of the
         * same video sequence. If the stream ID changes it means the
         * frames which are being received are part of a new video sequence
         * and @a nWidth and @a nHeight may have changed. The @a nStreamID
         * will always be a positive integer value.*/
        public int nStreamID;
        /** @brief Whether the image acquired is a key-frame. If it is
         * not a key-frame and there has been packet loss or a
         * key-frame has not been acquired prior then the image may
         * look blurred. */
        public bool bKeyFrame;
        /** @brief A buffer allocated internally by TeamTalkBase. */
        public System.IntPtr frameBuffer;
        /** @brief The size in bytes of the buffer allocate in @a
         * frameBuffer. */
        public int nFrameBufferSize;
    }

    /** 
     * @brief A struct containing the properties of a video capture
     * device.
     *
     * The information retrieved from the video capture device is used
     * to initialize the video capture device using the
     * TeamTalkBase.InitVideoCaptureDevice() function.
     * 
     * @see TeamTalkBase.GetVideoCaptureDevices */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct VideoCaptureDevice
    {
        /** @brief A string identifying the device. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szDeviceID;
        /** @brief The name of the capture device. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szDeviceName;
        /** @brief @brief The name of the API used to capture video.
         *
         * The following video capture APIs are supported:
         * - AVFoundation (Mac OS)
         * - DirectShow (Windows)
         * - V4L2 (Linux)
         *
         * Mac OS's QTkit video capture API was removed in TeamTalk
         * 5.2 because Apple's AppStore will reject apps which have
         * dependencies to it.
         *
         * V4L support was removed in TeamTalk 5.2. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szCaptureAPI;
        /** @brief The supported capture formats. */
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = TeamTalkBase.TT_VIDEOFORMATS_MAX)]
        public VideoFormat[] videoFormats;
        /** @brief The number of capture formats available in @a
         * captureFormats array. */
        public int nVideoFormatsCount;
    }

    /** @} */

    /** @addtogroup desktopshare
     * @{ */

    /**
     * @brief The bitmap format used for a #BearWare.DesktopWindow. */
    public enum BitmapFormat : uint
    {
        /** @brief Used to denote nothing selected. */
        BMP_NONE            = 0,
        /** @brief The bitmap is a 256-colored bitmap requiring a
         * palette. The default 256 colored palette is the Netscape
         * browser-safe palette. Use TeamTalkBase.Palette_GetColorTable() to
         * access or change the palette. The maximum size of a 
         * 8-bit bitmap is 4095 blocks of 120 by 34 pixels. */
        BMP_RGB8_PALETTE    = 1,
        /** @brief The bitmap is a 16-bit colored bitmap. The maximum
         * pixels. */
        BMP_RGB16_555       = 2,
        /** @brief The bitmap is a 24-bit colored bitmap. The maximum
         * size of a 24-bit bitmap is 4095 blocks of 85 by 16
         * pixels. */
        BMP_RGB24           = 3,
        /** @brief The bitmap is a 32-bit colored bitmap. The maximum
         * size of a 32-bit bitmap is 4095 blocks of 51 by 20
         * pixels. */
        BMP_RGB32           = 4
    }

    /** @brief The protocols supported for transferring a
     * #BearWare.DesktopWindow.
     *
     * So far only one, UDP-based, protocol is supported. */
    public enum DesktopProtocol : uint
    {
        /** @brief Desktop protocol based on ZLIB for image
         * compression and UDP for data transmission. */
        DESKTOPPROTOCOL_ZLIB_1  = 1
    }

    /**
     * @brief A struct containing the properties of a shared desktop window.
     *
     * The desktop window is a description of the bitmap which can be retrieved using 
     * TeamTalkBase.AcquireUserDesktopWindow() or the bitmap which should be transmitted using
     * TeamTalkBase.SendDesktopWindow(). */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct DesktopWindow
    {
        /** @brief The width in pixels of the bitmap. */
        public int nWidth;
        /** @brief The height in pixels of the bitmap. */
        public int nHeight;
        /** @brief The format of the bitmap. */
        public BitmapFormat bmpFormat;
        /** @brief The number of bytes for each scan-line in the
         * bitmap. Zero means 4-byte aligned. */
        public int nBytesPerLine;
        /** @brief The ID of the session which the bitmap belongs
         * to. If the session ID changes it means the user has started
         * a new session. This e.g. happens if the desktop session has
         * been closed and restart or if the bitmap has been
         * resized. Set @c nSessionID to 0 if the desktop window is
         * used with TeamTalkBase.SendDesktopWindow(). */
        public int nSessionID;
        /** @brief The desktop protocol used for transmitting the desktop window. */
        public DesktopProtocol nProtocol;
        /** @brief A buffer pointing to the bitmap data (often refered to as Scan0). */
        public IntPtr frameBuffer;
        /** @brief The size in bytes of the buffer allocate in @a
         * frameBuffer. Typically @c nBytesPerLine * @c nHeight. */
        public int nFrameBufferSize;
    }

    /**
     * @brief The state of a key (or mouse button), i.e. if it's
     * pressed or released. @see DesktopInput */
    public enum DesktopKeyState : uint
    {
        /** @brief The key is ignored. */
        DESKTOPKEYSTATE_NONE       = 0x00000000,
        /** @brief The key is pressed. */
        DESKTOPKEYSTATE_DOWN       = 0x00000001,
        /** @brief The key is released. */
        DESKTOPKEYSTATE_UP         = 0x00000002,
    }

    /**
     * @brief A struct containing a mouse or keyboard event.
     *
     * The DesktopInput struct is used for desktop access where a
     * remote user can control mouse or keybaord on a shared
     * desktop. Check out section @ref desktopinput on how to use
     * remote desktop access. */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct DesktopInput
    {
        /** @brief The X coordinate of the mouse. If used with
         * WindowsHelper.Execute() and the mouse position should be
         * ignored then set to #BearWare.DesktopInputConstants.DESKTOPINPUT_MOUSEPOS_IGNORE. */
        public ushort uMousePosX;
        /** @brief The Y coordinate of the mouse. If used with
         * TeamTalkBase.DesktopInput_Execute() and the mouse position should be
         * ignored then set to #BearWare.DesktopInputConstants.DESKTOPINPUT_MOUSEPOS_IGNORE. */
        public ushort uMousePosY;
        /** @brief The key-code (or mouse button) pressed. If used
         * with TeamTalkBase.DesktopInput_Execute() and no key (or mouse button)
         * is pressed then set to #BearWare.DesktopInputConstants.DESKTOPINPUT_KEYCODE_IGNORE.
         * Read section @ref transdesktopinput on issues with
         * key-codes and keyboard settings. */
        public uint uKeyCode;
        /** @brief The state of the key (or mouse button) pressed,
         * i.e. if it's up or down. */
        public DesktopKeyState uKeyState;
        /** @brief When true initializes the desktop input to ignore mouse and key values. */
        public DesktopInput(bool set_defaults)
        {
            if (set_defaults)
            {
                uMousePosX = uMousePosY = (ushort)DesktopInputConstants.DESKTOPINPUT_MOUSEPOS_IGNORE;
                uKeyCode = DesktopInputConstants.DESKTOPINPUT_KEYCODE_IGNORE;
                uKeyState = DesktopKeyState.DESKTOPKEYSTATE_NONE;
            }
            else
            {
                uMousePosX = uMousePosY = 0;
                uKeyCode = 0;
                uKeyState = DesktopKeyState.DESKTOPKEYSTATE_NONE;
            }
        }
    }

    /** @brief Constants for #BearWare.DesktopInput. */
    public struct DesktopInputConstants
    {
        /** @ingroup desktopshare
         *
         * If @c uKeyCode in #BearWare.DesktopInput is set to
         * #BearWare.DesktopInputConstants.DESKTOPINPUT_KEYCODE_IGNORE
         * it means no key (or mouse button) was pressed in the
         * desktop input event and WindowsHelper.DesktopInputExecute() will
         * ignore the value. */
        public const uint DESKTOPINPUT_KEYCODE_IGNORE = 0xFFFFFFFF;

        /** @ingroup desktopshare
         *
         * If @c uMousePosX or @c uMousePosY in #BearWare.DesktopInput
         * are set to
         * #BearWare.DesktopInputConstants.DESKTOPINPUT_MOUSEPOS_IGNORE
         * it means the mouse position is ignored when calling
         * WindowsHelper.Execute(). */
        public const ushort DESKTOPINPUT_MOUSEPOS_IGNORE = 0xFFFF;

        /** @ingroup desktopshare
         *
         * If @c uKeyCode of #BearWare.DesktopInput is set to
         * #BearWare.DesktopInputConstants.DESKTOPINPUT_KEYCODE_LMOUSEBTN
         * then WindowsDesktopInput.Execute() will see the key-code as
         * a left mouse button click. */
        public const uint DESKTOPINPUT_KEYCODE_LMOUSEBTN = 0x1000;

        /** @ingroup desktopshare
         *
         * If @c uKeyCode of #BearWare.DesktopInput is set to
         * #BearWare.DesktopInputConstants.DESKTOPINPUT_KEYCODE_RMOUSEBTN
         * then WindowsDesktopInput.Execute() will see the key-code as
         * a right mouse button click. */
        public const uint DESKTOPINPUT_KEYCODE_RMOUSEBTN = 0x1001;

        /** @ingroup desktopshare
         *
         * If @c uKeyCode of #BearWare.DesktopInput is set to
         * #BearWare.DesktopInputConstants.DESKTOPINPUT_KEYCODE_MMOUSEBTN
         * then WindowsDesktopInput.Execute() will see the key-code as
         * a middle mouse button click. */
        public const uint DESKTOPINPUT_KEYCODE_MMOUSEBTN = 0x1002;
    }

    /** @} */


    /** @addtogroup codecs
     * @{ */

    /** @brief Speex audio codec settings for Constant Bitrate mode
     * (CBR). @see SpeexVBRCodec */
    [StructLayout(LayoutKind.Explicit, CharSet = CharSet.Unicode)]
    public struct SpeexCodec
    {
        /** @brief Set to 0 for 8 KHz (narrow band), set to 1 for 16 KHz 
         * (wide band), set to 2 for 32 KHz (ultra-wide band). */
        [FieldOffset(0)]
        public int nBandmode;
        /** @brief A value from 1-10. As of DLL version 4.2 also 0 is
         * supported.*/
        [FieldOffset(4)]
        public int nQuality;
        /** @brief Milliseconds of audio data before each
         * transmission.
         *
         * Speex uses 20 msec frame sizes. Recommended is 40 msec. Min
         * is 20, max is 500 msec.
         *
         * The #SoundSystem must be able to process audio packets at
         * this interval. In most cases this makes less than 40 msec
         * transmission interval unfeasible. */
        [FieldOffset(8)]
        public int nTxIntervalMSec;
        /** @brief Playback should be done in stereo. Doing so will
         * disable 3d-positioning.
         *
         * @see TeamTalkBase.SetUserPosition
         * @see TeamTalkBase.SetUserStereo */
        [FieldOffset(12)]
        public bool bStereoPlayback;
    }

    /** @brief Speex audio codec settings for Variable Bitrate mode
     * (VBR). */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct SpeexVBRCodec
    {
        /** @brief Set to 0 for 8 KHz (narrow band), set to 1 for 16 KHz 
         * (wide band), set to 2 for 32 KHz (ultra-wide band). */
        public int nBandmode;
        /** @brief A value from 0-10. If @c nBitRate is non-zero it
         * will override this value. */
        public int nQuality;
        /** @brief The bitrate at which the audio codec should output
         * encoded audio data. Dividing it by 8 gives roughly the
         * number of bytes per second used for transmitting the
         * encoded data. For limits check out
         * #BearWare.SpeexConstants. Note that specifying @c nBitRate
         * will override nQuality. */
        public int nBitRate;
        /** @brief The maximum bitrate at which the audio codec is
         * allowed to output audio. Set to zero if it should be
         * ignored. */
        public int nMaxBitRate;
        /** @brief Enable/disable discontinuous transmission. When
         * enabled Speex will ignore silence, so the bitrate will
         * become very low. */
        public bool bDTX;
        /** @brief Milliseconds of audio data before each transmission. Speex
         * uses 20 msec frame sizes. Recommended is 40 ms. Min is 20,
         * max is 1000. */
        public int nTxIntervalMSec;
        /** @brief Playback should be done in stereo. Doing so will
         * disable 3d-positioning.
         *
         * @see TeamTalkBase.SetUserPosition
         * @see TeamTalkBase.SetUserStereo */
        public bool bStereoPlayback; 
    }

    /** @brief Speex constants for #BearWare.SpeexCodec and #BearWare.SpeexVBRCodec. */
    public struct SpeexConstants
    {
        /** @brief Use #BearWare.SpeexCodec or #BearWare.SpeexVBRCodec as 8 KHz */
        public const int SPEEX_BANDMODE_NARROW = 0;
        /** @brief Use #BearWare.SpeexCodec or #BearWare.SpeexVBRCodec as 16 KHz */
        public const int SPEEX_BANDMODE_WIDE = 1;
        /** @brief Use #BearWare.SpeexCodec or #BearWare.SpeexVBRCodec as 32 KHz */
        public const int SPEEX_BANDMODE_UWIDE = 2;
        /** @brief The minimum quality for Speex codec. */ 
        public const int SPEEX_QUALITY_MIN = 0;
        /** @brief The maximum quality for Speex codec. */ 
        public const int SPEEX_QUALITY_MAX = 10;
        /** @brief The minimum bitrate for Speex codec in 8 KHz
         * mode, i.e. quality set to 0. */
        public const int SPEEX_NB_MIN_BITRATE = 2150;
        /** @brief The maximum bitrate for Speex codec in 8 KHz
         * mode, i.e. quality set to 10. */
        public const int SPEEX_NB_MAX_BITRATE = 24600;
        /** @brief The minimum bitrate for Speex codec in 16 KHz
         * mode, i.e. quality set to 0. */
        public const int SPEEX_WB_MIN_BITRATE = 3950;
        /** @brief The maximum bitrate for Speex codec in 16 KHz
         * mode, i.e. quality set to 10. */
        public const int SPEEX_WB_MAX_BITRATE = 42200;
        /** @brief The minimum bitrate for Speex codec in 32 KHz
         * mode, i.e. quality set to 0. */
        public const int SPEEX_UWB_MIN_BITRATE = 4150;
        /** @brief The maximum bitrate for Speex codec in 32 KHz
         * mode, i.e. quality set to 10. */
        public const int SPEEX_UWB_MAX_BITRATE = 44000;
        /** @brief Default Speex bandmode for #BearWare.SpeexCodec or
         * #BearWare.SpeexVBRCodec. */
        public const int DEFAULT_SPEEX_BANDMODE = 1;
        /** @brief Default Speex quality for #BearWare.SpeexCodec or
         * #BearWare.SpeexVBRCodec. */
        public const int DEFAULT_SPEEX_QUALITY = 4;
        /** @brief Default Speex delay for #BearWare.SpeexCodec or
         * #BearWare.SpeexVBRCodec. */
        public const int DEFAULT_SPEEX_DELAY = 40;
        /** @brief Default Speex stereo playback for #BearWare.SpeexCodec or
         * #BearWare.SpeexVBRCodec. */
        public const bool DEFAULT_SPEEX_SIMSTEREO = false;
        /** @brief Default Speex bitrate for #BearWare.SpeexCodec or
         * #BearWare.SpeexVBRCodec. */
        public const int DEFAULT_SPEEX_BITRATE = 0;
        /** @brief Default Speex max bitrate for #BearWare.SpeexCodec or
         * #BearWare.SpeexVBRCodec. */
        public const int DEFAULT_SPEEX_MAXBITRATE = 0;
        /** @brief Default Speex DTX for #BearWare.SpeexCodec or
         * #BearWare.SpeexVBRCodec. */
        public const bool DEFAULT_SPEEX_DTX = true;
    }

    /** @brief OPUS audio codec settings. For detailed information
     * about the OPUS codec check out http://www.opus-codec.org */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct OpusCodec
    {
        /** @brief The sample rate to use. Sample rate must be
         * 8000, 12000, 16000, 24000 or 48000 Hz. */
        public int nSampleRate;
        /** @brief Mono = 1 or stereo = 2. */
        public int nChannels;
        /** @brief Application of encoded audio, VoIP or music.
         * @see OPUS_APPLICATION_VOIP
         * @see OPUS_APPLICATION_AUDIO */
        public int nApplication;
        /** @brief Complexity of encoding (affects CPU usage).
         * Value from 0-10. */
        public int nComplexity;
        /** @brief Forward error correction. 
         * Corrects errors if there's packetloss. */
        public bool bFEC;
        /** @brief Discontinuous transmission.
         * Enables "null" packets during silence. */
        public bool bDTX;
        /** @brief Bitrate for encoded audio. Should be between
         * #BearWare.OpusConstants.OPUS_MIN_BITRATE and
         * #BearWare.OpusConstants.OPUS_MAX_BITRATE. */
        public int nBitRate;
        /** @brief Enable variable bitrate. */
        public bool bVBR;
        /** @brief Enable constrained VBR.
         * @c bVBR must be enabled to enable this. */
        public bool bVBRConstraint;
        /** @brief Duration of audio before each transmission. Minimum is 2 msec.
         * Recommended is 40 msec. Maximum is 500 msec.
         * 
         * The #BearWare.SoundSystem must be able to process audio packets at
         * this interval. In most cases this makes less than 40 msec
         * transmission interval unfeasible. */
        public int nTxIntervalMSec;
        /** @brief OPUS supports 2.5, 5, 10, 20, 40, 60, 80, 100 and 120 msec.
         * If @c nFrameSizeMSec is 0 then @c nFrameSizeMSec will be same as 
         * @c nTxIntervalMSec. */
        public int nFrameSizeMSec;
    }

    /** @brief OPUS constants for #BearWare.OpusCodec. */
    public struct OpusConstants
    {
        /** @brief Audio encoding is for VoIP. This value should be set as
         * nApplicaton in #BearWare.OpusCodec. */
        public const int OPUS_APPLICATION_VOIP = 2048;
        /** @brief Audio encoding is for music.  This value should be set as
         * nApplicaton in #BearWare.OpusCodec. */
        public const int OPUS_APPLICATION_AUDIO = 2049;
        /** @brief The minimum bitrate for OPUS codec. Checkout @c nBitRate of
         * #BearWare.OpusCodec. */
        public const int OPUS_MIN_BITRATE = 6000;
        /** @brief The maximum bitrate for OPUS codec. Checkout @c nBitRate of
         * #BearWare.OpusCodec. */
        public const int OPUS_MAX_BITRATE = 510000;

        /** @brief The minimum frame size for OPUS codec. Checkout @c nFrameSizeMSec
         * of #BearWare.OpusCodec. */
        public const int OPUS_MIN_FRAMESIZE = 2; /* Actually it's 2.5 */
        /** @brief The maximum frame size for OPUS codec. Checkout @c nFrameSizeMSec
         * of #BearWare.OpusCodec. */
        public const int OPUS_MAX_FRAMESIZE = 60;
        /** @brief The real maximum frame size for OPUS codec. Checkout @c nFrameSizeMSec
         * of #BearWare.OpusCodec. Although OPUS states it only supports 2.5 - 60 msec, it actually
         * support up to 120 msec. */
        public const int OPUS_REALMAX_FRAMESIZE = 120;

        public const int DEFAULT_OPUS_APPLICATION = OPUS_APPLICATION_VOIP;
        public const int DEFAULT_OPUS_SAMPLERATE = 48000;
        public const int DEFAULT_OPUS_CHANNELS = 1;
        public const int DEFAULT_OPUS_COMPLEXITY = 10;
        public const bool DEFAULT_OPUS_FEC = true;
        public const bool DEFAULT_OPUS_DTX = false;
        public const bool DEFAULT_OPUS_VBR = true;
        public const bool DEFAULT_OPUS_VBRCONSTRAINT = false;
        public const int DEFAULT_OPUS_BITRATE = 32000;
        public const int DEFAULT_OPUS_DELAY = 20;
    }

    /** @brief Audio configuration specifying how recorded audio from
    * sound input device should be preprocessed before transmission.
    *
    * Users' audio levels may be diffent due to how their microphone
    * is configured in their OS. Automatic Gain Control (AGC) can be used
    * to ensure all users in the same channel have the same audio level.
    *
    * Enable the preprocessing configuration by calling
    * TeamTalkBase.SetSoundInputPreprocess().
    *
    * When joining a #BearWare.Channel and @c bEnableGainControl of
    * #BearWare.AudioConfig is enabled in the channel then enable sound input
    * preprocessing by setting @c bEnableAGC to TRUE and @c
    * nGainLevel of #BearWare.SpeexDSP to the @c nGainLevel of
    * #BearWare.AudioConfig. */
    [StructLayout(LayoutKind.Explicit)]
    public struct SpeexDSP
    {
        /** @brief Whether clients who join a #BearWare.Channel should
         * enable AGC with the settings specified @a nGainLevel, @a
         * nMaxIncDBSec, @a nMaxDecDBSec and @a nMaxGainDB.
         *
         * Note that AGC is not supported on ARM (iOS and Android),
         * since there's no fixed point implementation of AGC in
         * SpeexDSP. */
        [FieldOffset(0)]
        public bool bEnableAGC;
        /** @brief A value from 0 to 32768. Default is 8000.
         * Value is ignored if @a bEnableAGC is FALSE. */
        [FieldOffset(4)]
        public int nGainLevel;
        /** @brief Used so volume should not be amplified too quickly 
         * (maximal gain increase in dB/second). Default is 12. 
         * * Value is ignored if @a bEnableAGC is FALSE. */
        [FieldOffset(8)]
        public int nMaxIncDBSec;
        /** @brief Used so volume should not be attenuated
         * too quickly (maximal gain decrease in dB/second).
         * Negative value! Default is -40.
         * Value is ignored if @a bEnableAGC is FALSE. */
        [FieldOffset(12)]
        public int nMaxDecDBSec;
        /** @brief Ensure volume doesn't become too loud (maximal gain
         * in dB). Default is 30.
         * Value is ignored if @a bEnableAGC is FALSE. */
        [FieldOffset(16)]
        public int nMaxGainDB;
        /** @brief Whether clients who join the channel should automatically
         * enable denoising.  */
        [FieldOffset(20)]
        public bool bEnableDenoise;
        /** @brief Maximum attenuation of the noise in dB.
         * Negative value! Default value is -30. 
         * Value is ignored if @a bEnableDenoise is FALSE. */
        [FieldOffset(24)]
        public int nMaxNoiseSuppressDB;
        /** @brief Speex DSP is used for specifying how recorded audio
         * from a sound input device should be preprocessed before
         * transmission.
         *
         * In order to enable echo cancellation mode the local client
         * instance must first be set in sound duplex mode by calling
         * TeamTalkBase.InitSoundDuplexDevices(). This is because the echo canceller
         * must first mixed all audio streams into a single stream and
         * have then run in synch with the input stream. After calling
         * TeamTalkBase.InitSoundDuplexDevices() the flag #ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX
         * will be set.
         *
         * For echo cancellation to work the sound input and output device
         * must be the same sound card since the input and output stream
         * must be completely synchronized. Also it is recommended to also
         * enable denoising and AGC for better echo cancellation.
         *
         * @see TeamTalkBase.SetSoundInputPreprocess() */
        [FieldOffset(28)]
        public bool bEnableEchoCancellation;
        /** @brief Set maximum attenuation of the residual echo in dB 
         * (negative number). Default is -40.
         * Value is ignored if @a bEnableEchoCancellation is FALSE. */
        [FieldOffset(32)]
        public int nEchoSuppress;
        /** @brief Set maximum attenuation of the residual echo in dB 
         * when near end is active (negative number). Default is -15.
         * Value is ignored if @a bEnableEchoCancellation is FALSE. */
        [FieldOffset(36)]
        public int nEchoSuppressActive;

        public SpeexDSP(bool set_defaults)
        {
            if (set_defaults)
            {
                bEnableAGC = SpeexDSPConstants.DEFAULT_AGC_ENABLE;
                nGainLevel = SpeexDSPConstants.DEFAULT_AGC_GAINLEVEL;
                nMaxIncDBSec = SpeexDSPConstants.DEFAULT_AGC_INC_MAXDB;
                nMaxDecDBSec = SpeexDSPConstants.DEFAULT_AGC_DEC_MAXDB;
                nMaxGainDB = SpeexDSPConstants.DEFAULT_AGC_GAINMAXDB;

                bEnableDenoise = SpeexDSPConstants.DEFAULT_DENOISE_ENABLE;
                nMaxNoiseSuppressDB = SpeexDSPConstants.DEFAULT_DENOISE_SUPPRESS;

                bEnableEchoCancellation = SpeexDSPConstants.DEFAULT_ECHO_ENABLE;
                nEchoSuppress = SpeexDSPConstants.DEFAULT_ECHO_SUPPRESS;
                nEchoSuppressActive = SpeexDSPConstants.DEFAULT_ECHO_SUPPRESS_ACTIVE;
            }
            else
            {
                bEnableAGC = false;
                nGainLevel = 0;
                nMaxIncDBSec = 0;
                nMaxDecDBSec = 0;
                nMaxGainDB = 0;

                bEnableDenoise = false;
                nMaxNoiseSuppressDB = 0;

                bEnableEchoCancellation = false;
                nEchoSuppress = 0;
                nEchoSuppressActive = 0;
            }
        }
    }

    /** @brief Use TeamTalk's internal audio preprocessor for gain
     * audio. Same as used for TT_SetSoundInputGainLevel(). */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct TTAudioPreprocessor
    {
        /** @brief Gain level between #BearWare.SoundLevel.SOUND_GAIN_MIN and
         * #BearWare.SoundLevel.SOUND_GAIN_MAX. Default is #BearWare.SoundLevel.SOUND_GAIN_DEFAULT (no
         * gain). */
        public int nGainLevel;
        /** @brief Whether to mute left speaker in stereo playback. */
        public bool bMuteLeftSpeaker;
        /** @brief Whether to mute right speaker in stereo playback. */
        public bool bMuteRightSpeaker;
    }

    /** @brief The types of supported audio preprocessors.
     *
     * @see TT_InitLocalPlayback() */
    public enum AudioPreprocessorType : uint
    {
        /** @brief Value for specifying that no audio preprocessing
         * should occur. */
        NO_AUDIOPREPROCESSOR = 0,
        /** @brief Use the #BearWare.SpeexDSP audio preprocessor. */
        SPEEXDSP_AUDIOPREPROCESSOR = 1,
        /** @brief Use TeamTalk's internal audio preprocessor #BearWare.TTAudioPreprocessor. */
        TEAMTALK_AUDIOPREPROCESSOR = 2,
    };

    /** @brief Configure the audio preprocessor specified by @c nPreprocessor. */
    [StructLayout(LayoutKind.Explicit)]
    public struct AudioPreprocessor
    {
        /** @brief The audio preprocessor to use in the union of audio preprocessors. */
        [FieldOffset(0)]
        public AudioPreprocessorType nPreprocessor;
        /** @brief Used when @c nPreprocessor is #SPEEXDSP_AUDIOPREPROCESSOR. */
        [FieldOffset(4)]
        public SpeexDSP speexdsp;
        /** @brief Used when @c nPreprocessor is #TEAMTALK_AUDIOPREPROCESSOR. */
        [FieldOffset(4)]
        public TTAudioPreprocessor ttpreprocessor;
    }

    /** @brief Default values for #BearWare.SpeexDSP. */
    public struct SpeexDSPConstants
    {
        public const bool DEFAULT_AGC_ENABLE = true;
        public const int DEFAULT_AGC_GAINLEVEL = 8000;
        public const int DEFAULT_AGC_INC_MAXDB = 12;
        public const int DEFAULT_AGC_DEC_MAXDB = -40;
        public const int DEFAULT_AGC_GAINMAXDB = 30;
        public const bool DEFAULT_DENOISE_ENABLE = true;
        public const int DEFAULT_DENOISE_SUPPRESS = -30;
        public const bool DEFAULT_ECHO_ENABLE = true;
        public const int DEFAULT_ECHO_SUPPRESS = -40;
        public const int DEFAULT_ECHO_SUPPRESS_ACTIVE = -15;
    }

    /** @brief WebM video codec settings.
     * @see VideoCodec
     * @see TeamTalkBase.InitVideoCaptureDevice
     * @see TeamTalkBase.StartStreamingMediaFileToChannel() */
    [StructLayout(LayoutKind.Explicit, CharSet = CharSet.Unicode)]
    public struct WebMVP8Codec
    {
        /** @brief Same as @c rc_target_bitrate */
        [FieldOffset(0)]
        public int nRcTargetBitrate;
        /** @brief Target bitrate in kbits/sec. This value must be
         * greater than 0. 
         *
         * Same as 'rc_target_bitrate' in 'vpx_codec_enc_cfg_t'.
         * http://www.webmproject.org/docs/webm-sdk/ */
        [FieldOffset(0)]
        public int rc_target_bitrate;
        /** @brief Time that should be spent on encoding a frame.
         *
         * Same as 'deadline' parameter in 'vpx_codec_encode()'.
         * http://www.webmproject.org/docs/webm-sdk/
         * 
         * Supported values are VPX_DL_REALTIME = 1, VPX_DL_GOOD_QUALITY = 1000000,
         * and VPX_DL_BEST_QUALITY = 0. */
        [FieldOffset(4)]
        public uint nEncodeDeadline;
    }

    public struct WebMVP8CodecConstants
    {
        /** @brief @c nEncodeDeadline value for fastest encoding.
         * @see WebMVP8Codec */
        public const int WEBM_VPX_DL_REALTIME = 1;
        /** @brief @c nEncodeDeadline value for good encoding.
         * @see WebMVP8Codec */
        public const int WEBM_VPX_DL_GOOD_QUALITY = 1000000;
        /** @brief @c nEncodeDeadline value for best encoding.
         * @see WebMVP8Codec */
        public const int WEBM_VPX_DL_BEST_QUALITY = 0;
    }

    /**
     * @brief The codecs supported.
     * @see AudioCodec
     * @see VideoCodec */
    public enum Codec : uint
    {
        /** @brief No codec specified. */
        NO_CODEC = 0,
        /** @brief Speex audio codec, http://www.speex.org @see
         * SpeexCodec */
        SPEEX_CODEC = 1,
        /** @brief Speex audio codec in VBR mode, http://www.speex.org
         * @see SpeexVBRCodec */
        SPEEX_VBR_CODEC = 2,
        /** @brief OPUS audio codec. @see OpusCodec */
        OPUS_CODEC = 3,
        /** @brief WebM video codec. @see WebMVP8Codec */
        WEBM_VP8_CODEC = 128
    }

    /**
     * @brief Struct used for specifying which audio codec a channel
     * uses. */
    [StructLayout(LayoutKind.Explicit)]
    public struct AudioCodec
    {
        /** @brief Specifies whether the member @a speex, @a speex_vbr or
         * @a opus holds the codec settings. */
        [FieldOffset(0)]
        public Codec nCodec;
        /** @brief Speex codec settings if @a nCodec is
         * #Codec.SPEEX_CODEC. */
        [FieldOffset(4)]
        public SpeexCodec speex;
        /** @brief Speex codec settings if @a nCodec is
         * #Codec.SPEEX_VBR_CODEC */
        [FieldOffset(4)]
        public SpeexVBRCodec speex_vbr;
        /** @brief Opus codec settings if @a nCodec is
         * OPUS_CODEC */
        [FieldOffset(4)]
        public OpusCodec opus;
    }

    /** @brief Audio configuration for clients in a channel.
     *
     * An audio configuration can be used to set common audio
     * properties for all users in a channel. Checkout @c audiocfg of
     * #BearWare.Channel.
     *
     * The audio configuration only supports same audio level
     * for all users by manually converting the values to the
     * #BearWare.SpeexDSP preprocessor.
     *
     * @see TeamTalkBase.SetSoundInputPreprocess()
     * @see TeamTalkBase.DoMakeChannel()
     * @see TeamTalkBase.DoJoinChannel() */
    [StructLayout(LayoutKind.Explicit)]
    public struct AudioConfig
    {
        /** @brief Users should enable automatic gain control. */
        [FieldOffset(0)]
        public bool bEnableAGC;
        /** @brief Reference gain level to be used by all users. */
        [FieldOffset(4)]
        public int nGainLevel;

        public AudioConfig(bool set_defaults)
        {
            if (set_defaults)
            {
                bEnableAGC = true;
                nGainLevel = AudioConfigConstants.DEFAULT_AGC_GAINLEVEL;
            }
            else
            {
                bEnableAGC = false;
                nGainLevel = 0;
            }
        }
    }

    public struct AudioConfigConstants
    {
        public const int DEFAULT_AGC_GAINLEVEL = 8000;
        public const int DEFAULT_AGC_INC_MAXDB = 12;
    }

    /** @brief Struct used for specifying the video codec to use. */
    [StructLayout(LayoutKind.Explicit)]
    public struct VideoCodec
    {
        /** @brief Specifies member holds the codec settings. So far
         * there is only one video codec to choose from, namely @c
         * webm_vp8. */
        [FieldOffset(0)]
        public Codec nCodec;
        [FieldOffset(4)]
        public WebMVP8Codec webm_vp8;
    }
    /** @} */

    /** @addtogroup mediastream
     * @{ */

    /**
     * @brief Struct describing the audio and video format used by a
     * media file.
     *
     * @see TeamTalkBase.GetMediaFile() */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct MediaFileInfo
    {
        /** @brief Status of media file if it's being saved to
         * disk. */
        public MediaFileStatus nStatus;
        /** @brief Name of file. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szFileName;
        /** @brief The audio properties of the media file. */
        public AudioFormat audioFmt;
        /** @brief The video properties of the media file. */
        public VideoFormat videoFmt;
        /** @brief The duration of the media file in miliseconds. */
        public uint uDurationMSec;
        /** @brief The elapsed time of the media file in miliseconds. */
        public uint uElapsedMSec;
    }
    /**
     * @brief Properties for initializing or updating a file for media
     * streaming.
     *
     * @see TeamTalkBase.InitLocalPlayback()
     * @see TeamTalkBase.UpdateLocalPlayback()
     * @see TeamTalkBase.StartStreamingMediaFileToChannel()
     * @see TeamTalkBase.UpdateStreamingMediaFileToChannel() */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct MediaFilePlayback
    {
        /** @brief Offset in milliseconds in the media file where to
         * start playback. Pass -1 (0xffffffff) to ignore this value when 
         * using TeamTalkBase.UpdateLocalPlayback() or TeamTalkBase.UpdateStreamingMediaFileToChannel().
         * @c uOffsetMSec must be less than @c uDurationMSec in #BearWare.MediaFileInfo. */
        public uint uOffsetMSec;
        /** @brief Start or pause media file playback. */
        public bool bPaused;
        /** @brief Option to activate audio preprocessor on local media file playback. */
        public AudioPreprocessor audioPreprocessor;
    }

    /** @} */


    /** @addtogroup transmission
     * @{ */

    /** @brief The types of streams which are available for
     * transmission. */
    [Flags]
    public enum StreamType : uint
    {
        /** @brief No stream. */
        STREAMTYPE_NONE                     = 0x00000000,
        /** @brief Voice stream type which is audio recorded from a
         * sound input device. @see TeamTalkBase.InitSoundInputDevice() */
        STREAMTYPE_VOICE                    = 0x00000001,
        /** @brief Video capture stream type which is video recorded
         * from a webcam. @see TeamTalkBase.InitVideoCaptureDevice() */
        STREAMTYPE_VIDEOCAPTURE             = 0x00000002,
        /** @brief Audio stream type from a media file which is being
         * streamed. @see TeamTalkBase.StartStreamingMediaFileToChannel() */
        STREAMTYPE_MEDIAFILE_AUDIO          = 0x00000004,
        /** @brief Video stream type from a media file which is being
         * streamed. @see TeamTalkBase.StartStreamingMediaFileToChannel() */
        STREAMTYPE_MEDIAFILE_VIDEO          = 0x00000008,
        /** @brief Desktop window stream type which is a window (or
         * bitmap) being transmitted. @see TeamTalkBase.SendDesktopWindow() */
        STREAMTYPE_DESKTOP                  = 0x00000010,
        /** @brief Desktop input stream type which is keyboard or
         * mouse input being transmitted. @see
         * TeamTalkBase.SendDesktopInput() */
        STREAMTYPE_DESKTOPINPUT             = 0x00000020,
        /** @brief Shortcut to allow both audio and video media files. */
        STREAMTYPE_MEDIAFILE                = STREAMTYPE_MEDIAFILE_AUDIO |
                                              STREAMTYPE_MEDIAFILE_VIDEO,
        /** @brief Shortcut to allow voice, media files, desktop and webcamera. */
        STREAMTYPE_CLASSROOM_ALL            = STREAMTYPE_VOICE |
                                              STREAMTYPE_VIDEOCAPTURE |
                                              STREAMTYPE_DESKTOP |
                                              STREAMTYPE_MEDIAFILE,
    }
    /** @} */


    /** @addtogroup server
     * @{ */

    /** 
     * @brief The rights users have once they have logged on to the
     * server.
     *
     * #BearWare.ServerProperties holds the user rights in its \a uUserRights
     * member variable and is retrieved by calling
     * TeamTalkBase.GetServerProperties() once connected to the server.
     *
     * @see BearWare.ServerProperties
     * @see BearWare.TeamTalkBase.GetServerProperties */
    [Flags]
    public enum UserRight : uint
    {
        /** @brief Users who log onto the server has none of the
          * rights below. */
        USERRIGHT_NONE                              = 0x00000000,
        /** @brief Allow multiple users to log on to the server with
         * the same #BearWare.UserAccount. @see TeamTalkBase.DoLogin() */
        USERRIGHT_MULTI_LOGIN                       = 0x00000001,
        /** @brief User can see users in all other channels. */
        USERRIGHT_VIEW_ALL_USERS                    = 0x00000002,
        /** @brief User is allowed to create temporary channels which
         * disappear when last user leaves the channel.
         * @see TeamTalkBase.DoJoinChannel() */
        USERRIGHT_CREATE_TEMPORARY_CHANNEL          = 0x00000004,
        /** @brief User is allowed to create permanent channels which
         * are stored in the server's configuration file.
         * @see TeamTalkBase.DoMakeChannel() */
        USERRIGHT_MODIFY_CHANNELS                   = 0x00000008,
        /** @brief User can broadcast text message of type 
         * #TextMsgType.MSGTYPE_BROADCAST to all users. */
        USERRIGHT_TEXTMESSAGE_BROADCAST             = 0x00000010,
        /** @brief User can kick users off the server. @see TeamTalkBase.DoKickUser() */
        USERRIGHT_KICK_USERS                        = 0x00000020,
        /** @brief User can add and remove banned users.
         * @see TeamTalkBase.DoBanUser() @see TeamTalkBase.DoListBans() */
        USERRIGHT_BAN_USERS                         = 0x00000040,
        /** @brief User can move users from one channel to another.
         * @see TeamTalkBase.DoMoveUser() */
        USERRIGHT_MOVE_USERS                        = 0x00000080,
        /** @brief User can make other users channel operator.
         * @see TeamTalkBase.DoChannelOp() */
        USERRIGHT_OPERATOR_ENABLE                   = 0x00000100,
        /** @brief User can upload files to channels. @see TeamTalkBase.DoSendFile() */
        USERRIGHT_UPLOAD_FILES                      = 0x00000200,
        /** @brief User can download files from channels. 
         * @see TeamTalkBase.DoRecvFile() */
        USERRIGHT_DOWNLOAD_FILES                    = 0x00000400,
        /** @brief User can update server properties.
         * @see TeamTalkBase.DoUpdateServer() */
        USERRIGHT_UPDATE_SERVERPROPERTIES           = 0x00000800,
        /** @brief Users are allowed to forward audio packets through
         * server. TeamTalkBase.EnableVoiceTransmission() */
        USERRIGHT_TRANSMIT_VOICE                    = 0x00001000,
        /** @brief User is allowed to forward video packets through
         * server. TeamTalkBase.StartVideoCaptureTransmission() */
        USERRIGHT_TRANSMIT_VIDEOCAPTURE             = 0x00002000,
        /** @brief User is allowed to forward desktop packets through
         * server. @see TeamTalkBase.SendDesktopWindow() */
        USERRIGHT_TRANSMIT_DESKTOP                  = 0x00004000,
        /** @brief User is allowed to forward desktop input packets through
         * server. @see TeamTalkBase.SendDesktopInput() */
        USERRIGHT_TRANSMIT_DESKTOPINPUT             = 0x00008000,
        /** @brief User is allowed to stream audio files to channel.
         * @see TeamTalkBase.StartStreamingMediaFileToChannel() */
        USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO          = 0x00010000,
        /** @brief User is allowed to stream video files to channel.
         * @see TeamTalkBase.StartStreamingMediaFileToChannel() */
        USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO          = 0x00020000,
        /** @brief User is allowed to media files to channel.
         * @see TeamTalkBase.StartStreamingMediaFileToChannel() */
        USERRIGHT_TRANSMIT_MEDIAFILE = USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO | USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO,
        /** @brief User's nick name is locked.
          * TeamTalkBase.DoChangeNickname() cannot be used and TeamTalkBase.DoLogin() 
          * will ignore szNickname parameter. 
          * @see TeamTalkBase.DoLogin()
          * @see TeamTalkBase.DoLoginEx()
          * @see TeamTalkBase.DoChangeNickname() */
        USERRIGHT_LOCKED_NICKNAME                   = 0x00040000,
        /** @brief User's status is locked. TeamTalkBase.DoChangeStatus()
          * cannot be used. */
        USERRIGHT_LOCKED_STATUS                     = 0x00080000,
        /** @brief User can record voice in all channels. Even channels
         * with #ChannelType.CHANNEL_NO_RECORDING. */
        USERRIGHT_RECORD_VOICE                      = 0x00100000,
        /** @brief User with all rights.*/
        USERRIGHT_ALL                               = 0xFFFFFFFF & ~USERRIGHT_LOCKED_NICKNAME & ~USERRIGHT_LOCKED_STATUS
    }

    /** 
     * @brief A struct containing the properties of the server's
     * settings.
     *
     * The server properties is available after a successful call to
     * TeamTalkBase.DoLogin()
     *
     * @see TeamTalkBase.DoUpdateServer
     * @see TeamTalkBase.GetServerProperties 
     * @see TeamTalkBase.DoLogin
     * @see UserRight */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct ServerProperties
    {
        /** @brief The server's name. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szServerName;
        /** @brief The message of the day. Read-only property. Use @c szMOTDRaw
         *  to update this property.*/
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szMOTD;
        /** @brief The message of the day including variables. The result of the
         * szMOTDRaw string will be displayed in @c szMOTD.
         * When updating the MOTD the variables %users% (number of users), 
         * %admins% (number
         * of admins), %uptime% (hours, minutes and seconds the server has
         * been online), %voicetx% (KBytes transmitted), %voicerx% (KBytes
         * received) and %lastuser% (nickname of last user to log on to the
         * server) as part of the MOTD. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szMOTDRaw;
        /** @brief The maximum number of users allowed on the server.  A user
         * with admin account can ignore this */
        public int nMaxUsers;
        /** @brief The maximum number of logins with wrong password before
         * banning user's IP-address. */
        public int nMaxLoginAttempts;
        /** @brief The maximum number of users allowed to log in with the same
         * IP-address. 0 means disabled. */
        public int nMaxLoginsPerIPAddress;
        /** @brief The maximum number of bytes per second which the server 
         * will allow for voice packets. If this value is exceeded the server
         * will start dropping audio packets. 0 = disabled. */
        public int nMaxVoiceTxPerSecond;
        /** @brief The maximum number of bytes per second which the
         * server will allow for video input packets. If this value is
         * exceeded the server will start dropping video packets. 0 =
         * disabled. */
        public int nMaxVideoCaptureTxPerSecond;
        /** @brief The maximum number of bytes per second which the server 
         * will allow for media file packets. If this value is exceeded the server
         * will start dropping media file packets. 0 = disabled. */
        public int nMaxMediaFileTxPerSecond;
        /** @brief The maximum number of bytes per second which the server 
         * will allow for desktop packets. If this value is exceeded the server
         * will start dropping desktop packets. 0 = disabled. */
        public int nMaxDesktopTxPerSecond;
        /** @brief The amount of bytes per second which the server 
         * will allow for packet forwarding.  If this value is exceeded the server
         * will start dropping packets. 0 = disabled. */
        public int nMaxTotalTxPerSecond;
        /** @brief Whether the server automatically saves changes */
        public bool bAutoSave;
        /** @brief The server's TCP port. */
        public int nTcpPort;
        /** @brief The server's UDP port. */
        public int nUdpPort;
        /** @brief The number of seconds before a user who hasn't
         * responded to keepalives will be kicked off the server. */
        public int nUserTimeout;
        /** @brief The server version. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szServerVersion;
        /** @brief The version of the server's protocol. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szServerProtocolVersion;
        /** @brief Number of msec before an IP-address can make
         * another login attempt. If less than this amount then
         * TeamTalkBase.DoLogin() will result in
         * #CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED. Zero means
         * disabled.
         * 
         * Also checkout @c nMaxLoginAttempts and @c
         * nMaxLoginsPerIPAddress. */
        public int nLoginDelayMSec;
        /** @brief A randomly generated 256 bit access token created
         * by the server to identify the login session.
         * Read-only property. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szAccessToken;
    }

    /**
     * @brief A struct containing the server's statistics,
     * i.e. bandwidth usage and user activity.
     *
     * Use BearWare.TeamTalkBase.DoQueryServerStats() to query the server's statistics
     * and when the command completes use BearWare.TeamTalkBase.GetServerStatistics() to
     * extract the statistics. */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct ServerStatistics
    {
        /** @brief The number of bytes sent from the server to
         * clients. */
        public long nTotalBytesTX;
        /** @brief The number of bytes received by the server from
         * clients. */
        public long nTotalBytesRX;
        /** @brief The number of bytes in audio packets sent from the
         *  server to clients. */
        public long nVoiceBytesTX;
        /** @brief The number of bytes in audio packets received by
         *  the server from clients. */
        public long nVoiceBytesRX;
        /** @brief The number of bytes in video packets sent from the
         *  server to clients. */
        public long nVideoCaptureBytesTX;
        /** @brief The number of bytes in video packets received by
         *  the server from clients. */
        public long nVideoCaptureBytesRX;
        /** @brief The number of bytes in media file packets sent from the
         *  server to clients. */
        public long nMediaFileBytesTX;
        /** @brief The number of bytes in media file packets received by
         *  the server from clients. */
        public long nMediaFileBytesRX;
        /** @brief The number of bytes in desktop packets sent from the
         *  server to clients. */
        public long nDesktopBytesTX;
        /** @brief The number of bytes in desktop packets received by
         *  the server from clients. */
        public long nDesktopBytesRX;
        /** @brief The number of users who have logged on to the server. */
        public int nUsersServed;
        /** @brief The highest numbers of users online. */
        public int nUsersPeak;
        /** @brief The number of bytes for file transmission transmitted
         * from the server. */
        public long nFilesTx;
        /** @brief The number of bytes for file transmission received
         * by the server. */
        public long nFilesRx;
        /** @brief The server's uptime in msec. */
        public long nUptimeMSec;
    }

    /**
      * @brief Way to ban a user from either login or joining a
      * channel.
      *
      * @see BannedUser */
    [Flags]
    public enum BanType : uint
    {
        /** @brief Ban type not set. */
        BANTYPE_NONE = 0x00,
        /** @brief The ban applies to the channel specified in the @c
         * szChannel of #BearWare.BannedUser. Otherwise the ban applies to the
         * entire server. */
        BANTYPE_CHANNEL = 0x01,
        /** @brief Ban @c szIPAddress specified in #BearWare.BannedUser. */
        BANTYPE_IPADDR = 0x02,
        /** @brief Ban @c szUsername specified in #BearWare.BannedUser. */
        BANTYPE_USERNAME = 0x04
    };

    /**
     * @brief A struct containing the properties of a banned user.
     * @see TeamTalkBase.DoListBans() 
     * @see TT_DoBanUserEx() */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct BannedUser
    {
        /** @brief IP-address of banned user. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szIPAddress;
        /** @brief Channel where user was located when banned. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szChannelPath;
        /** @brief Date and time when user was banned. Read-only property. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szBanTime;
        /** @brief Nickname of banned user. Read-only property.  */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szNickname;
        /** @brief Username of banned user. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szUsername;
        /** @brief The type of ban that applies to this banned user. */
        public BanType uBanTypes;
    }

    /** @ingroup users
     * @brief The types of users supported. 
     * @see User */
    [Flags]
    public enum UserType : uint
    {
        /** @brief Used internally to denote an unauthenticated
         * user. */
        USERTYPE_NONE       = 0x0,
        /** @brief A default user who can join channels. */
        USERTYPE_DEFAULT    = 0x01,
        /** @brief A user with administrator privileges. */
        USERTYPE_ADMIN      = 0x02
    }

    /**
     * @brief Properties to prevent server abuse.
     * 
     * The AbusePrevention-struct is part of #BearWare.UserAccount and can be
     * used to limit the number of commands a user can issue to the 
     * server.
     * 
     * Requires TeamTalk version 5.2.3.4896. */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct AbusePrevention
    {
        /** @brief Limit number of commands a user can send to the
         * server. 
         *
         * This can be used to prevent flooding where a user is
         * sending several hundred text messages to another user.
         *
         * Values set like this: User can issue @c nCommandsLimit
         * commands within duration @c nCommandsIntervalMSec. Put zeros to
         * disable.  @see CMDERR_COMMAND_FLOOD */
        public int nCommandsLimit;
        /** @brief Commands within given interval. */
        public int nCommandsIntervalMSec;
    }

    /** 
     * @brief A struct containing the properties of a user account.
     *
     * A registered user is one that has a user account on the server.
     *
     * @see TeamTalkBase.DoListUserAccounts
     * @see TeamTalkBase.DoNewUserAccount
     * @see TeamTalkBase.DoDeleteUserAccount */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct UserAccount
    {
        /** @brief The account's username */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szUsername;
        /** @brief The account's password. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szPassword;
        /** @brief A bitmask of the type of user based on #UserType. */
        public UserType uUserType;
        /** @brief A bitmask based on #UserRight which specifies the rights 
         * the user have who logs onto the server. A user of type 
         * #UserType.USERTYPE_ADMIN by default has all rights. */
        public UserRight uUserRights;
        /** @brief A user data field which can be used for additional
         * information. The @a nUserData field of the #BearWare.User struct will
         * contain this value when a user who logs in with this account. */
        public int nUserData;
        /** @brief Additional notes about this user. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szNote;
        /** @brief User should (manually) join this channel after login.
         * If an initial channel is specified in the user's account then
         * no password is required for the user to join the channel.
         * @see TeamTalkBase.DoJoinChannel() */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szInitChannel;
        /** @brief Channels where this user will automatically become channel
         * operator when joining. The channels must be of type #ChannelType.CHANNEL_PERMANENT.
         * @see TeamTalkBase.DoChannelOp() */
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = TeamTalkBase.TT_CHANNELS_OPERATOR_MAX)]
        public int[] autoOperatorChannels;
        /** @brief Bandwidth restriction for audio codecs created by 
         * this user. This value will hold the highest bitrate which 
         * is allowed for audio codecs. 0 = no limit. @see AudioCodec */
        public int nAudioCodecBpsLimit;
        /** @brief Properties which can be set to prevent abuse of a
         * server, e.g. limit number of commands issued.
         * 
         * Requires TeamTalk version 5.2.3.4896. */
        public AbusePrevention abusePrevent;
    }
    /** @} */

    /** @addtogroup users
     * @{ */

    /** 
     * @brief A user by default accepts audio, video and text messages
     * from all users. Using subscribtions can, however, change what
     * the local client instance is willing to accept from other
     * users.
     *
     * By calling TeamTalkBase.DoSubscribe() and TeamTalkBase.DoUnsubscribe() the local
     * client instance can tell the server (and thereby remote users)
     * what he is willing to accept from other users.
     *
     * To check what a user subscribes to check out the #BearWare.User struct's
     * @a uLocalSubscriptions. The subscriptions with the prefix
     * SUBSCRIBE_INTERCEPT_* options can be used to spy on users and
     * receive data from them even if one is not participating in the
     * same channel as they are.
     *
     * @see TeamTalkBase.DoSubscribe
     * @see TeamTalkBase.DoUnsubscribe */
    [Flags]
    public enum Subscription : uint
    {
        /** @brief No subscriptions. */
        SUBSCRIBE_NONE = 0x00000000,
        /** @brief Subscribing to user text messages.
         * @see #TextMsgType.MSGTYPE_USER. */
        SUBSCRIBE_USER_MSG = 0x00000001,
        /** @brief Subscribing to channel texxt messages.
         * @see #TextMsgType.MSGTYPE_CHANNEL. */
        SUBSCRIBE_CHANNEL_MSG = 0x00000002,
        /** @brief Subscribing to broadcast text messsages. 
         * @see #TextMsgType.MSGTYPE_BROADCAST.*/
        SUBSCRIBE_BROADCAST_MSG = 0x00000004,
        /** @brief Subscribing to custom user messages. 
         * @see #TextMsgType.MSGTYPE_CUSTOM. */
        SUBSCRIBE_CUSTOM_MSG = 0x00000008,
        /** @brief Subscribing to #StreamType.STREAMTYPE_VOICE. */
        SUBSCRIBE_VOICE = 0x00000010,
        /** @brief Subscribing to #StreamType.STREAMTYPE_VIDEOCAPTURE. */
        SUBSCRIBE_VIDEOCAPTURE = 0x00000020,
        /** @brief Subscribing to #StreamType.STREAMTYPE_DESKTOP. */
        SUBSCRIBE_DESKTOP = 0x00000040,
        /** @brief Subscribing to #StreamType.STREAMTYPE_DESKTOPINPUT.
         * @see TeamTalkBase.GetUserDesktopInput()
         * @see TeamTalkBase.SendDesktopInput() */
        SUBSCRIBE_DESKTOPINPUT = 0x00000080,
        /** @brief Subscribing to #StreamType.STREAMTYPE_MEDIAFILE_VIDEO and
         * #StreamType.STREAMTYPE_MEDIAFILE_AUDIO. */
        SUBSCRIBE_MEDIAFILE = 0x00000100,
        /** @brief Intercept all user text messages sent by a
        * user. Only user-type #UserType.USERTYPE_ADMIN can do this. */
        SUBSCRIBE_INTERCEPT_USER_MSG = 0x00010000,
        /** @brief Intercept all channel messages sent by a user. Only
        * user-type #UserType.USERTYPE_ADMIN can do this. */
        SUBSCRIBE_INTERCEPT_CHANNEL_MSG = 0x00020000,
        /* unused SUBSCRIBE_INTERCEPT_BROADCAST_MSG = 0x00040000 */
        /** @brief Intercept all custom text messages sent by user. 
         * Only user-type #UserType.USERTYPE_ADMIN can do this.  */
        SUBSCRIBE_INTERCEPT_CUSTOM_MSG = 0x00080000,
        /** @brief Intercept all voice sent by a user. Only user-type
         * #UserType.USERTYPE_ADMIN can do this. By enabling this subscription an
         * administrator can listen to audio sent by users outside his
         * own channel. */
        SUBSCRIBE_INTERCEPT_VOICE = 0x00100000,
        /** @brief Intercept all video sent by a user. Only user-type
         * #UserType.USERTYPE_ADMIN can do this. By enabling this subscription an
         * administrator can receive video frames sent by users
         * outside his own channel. */
        SUBSCRIBE_INTERCEPT_VIDEOCAPTURE = 0x00200000,
        /** @brief Intercept all desktop data sent by a user. Only
         * user-type #UserType.USERTYPE_ADMIN can do this. By enabling this
         * subscription an administrator can views desktops
         * sent by users outside his own channel. */
        SUBSCRIBE_INTERCEPT_DESKTOP = 0x00400000,
        /* unused SUBSCRIBE_INTERCEPT_DESKTOPINPUT = 0x00800000, */
        /** @brief Intercept all media file data sent by a user. Only user-type
         * #UserType.USERTYPE_ADMIN can do this. By enabling this subscription an
         * administrator can listen to media files sent by users outside his
         * own channel. */
        SUBSCRIBE_INTERCEPT_MEDIAFILE = 0x01000000,
    }

    /**
     * @brief The possible states for a user. Used for #BearWare.User's @a
     * uUserState variable. */
    [Flags]
    public enum UserState : uint
    {
        /** @brief The user is in initial state. */
        USERSTATE_NONE                                  = 0x0000000,
        /** @brief If set the user is currently talking. If this flag
         * changes the event #ClientEvent.CLIENTEVENT_USER_STATECHANGE is
         * posted. */
        USERSTATE_VOICE                                 = 0x00000001,
        /** @brief If set the user's voice is muted. @see TeamTalkBase.SetUserMute */
        USERSTATE_MUTE_VOICE                            = 0x00000002,
        /** @brief If set the user's media file playback is muted.
         * @see TeamTalkBase.SetUserMute */
        USERSTATE_MUTE_MEDIAFILE                        = 0x00000004,
        /** @brief If set the user currently has an active desktop
         * session. If this flag changes the event
         * #ClientEvent.CLIENTEVENT_USER_STATECHANGE is posted.  @see
         * TeamTalkBase.SendDesktopWindow(). */
        USERSTATE_DESKTOP                               = 0x00000008,
        /** @brief If set the user currently has an active video
         * stream.  If this flag changes the event
         * #ClientEvent.CLIENTEVENT_USER_STATECHANGE is posted.  @see
         * CLIENTEVENT_USER_VIDEOCAPTURE. */
        USERSTATE_VIDEOCAPTURE                          = 0x00000010,
        /** @brief If set the user currently streams an audio file. If
         * user is streaming a video file with audio then this value
         * is also set.  If this flag changes the event
         * #ClientEvent.CLIENTEVENT_USER_STATECHANGE is posted.  @see
         * TeamTalkBase.StartStreamingMediaFile() */
        USERSTATE_MEDIAFILE_AUDIO                       = 0x00000020,
        /** @brief If set the user currently streams a video file.  If
         * this flag changes the event #ClientEvent.CLIENTEVENT_USER_STATECHANGE
         * is posted.  @see TeamTalkBase.StartStreamingMediaFile() */
        USERSTATE_MEDIAFILE_VIDEO                       = 0x00000040,
        /** @brief If set user is streaming a media file.  If this
         * flag changes the event #ClientEvent.CLIENTEVENT_USER_STATECHANGE is
         * posted.  @see TeamTalkBase.StartStreamingMediaFile() */
        USERSTATE_MEDIAFILE                             = USERSTATE_MEDIAFILE_AUDIO |
                                                          USERSTATE_MEDIAFILE_VIDEO
    }

    /**
     * @brief A struct containing the properties of a user.
     * @see BearWare.UserType
     * @see BearWare.TeamTalkBase.GetUser */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct User
    {
        /** @brief The user's ID. A value from 1 -
         * #BearWare.TeamTalkBase.TT_USERID_MAX. This property is set by
         * the server and will not change after login. */
        public int nUserID;
        /** @brief The @a szUsername of the user's
         * #BearWare.UserAccount. A user account is created by calling
         * TeamTalkBase.DoNewUserAccount(). This property is set by the
         * server and will not change after login.  */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szUsername;
        /** @brief The @a nUserData of the user's #BearWare.UserAccount. This
         * field can be use to denote e.g. a database ID. This
         * property is set by the server and will not change after
         * login.*/
        public int nUserData;
        /** @brief The @a uUserType of the user's #BearWare.UserAccount. This
         * property is set by the server and will not change after
         * login. */
        public UserType uUserType;
        /** @brief The user's IP-address. This value is set by the server. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szIPAddress;
        /** @brief The user's client version. This property is set by
         * the server and will not change after login. Three octets
         * are used for the version number. Third octet is major,
         * second octet is minor and third octet is
         * build. I.e. 0x00050201 is version 5.2.1. */ 
        public uint uVersion;
        /** @brief The channel which the user is currently
         * participating in. 0 if none. This value can change as a
         * result of TeamTalkBase.DoJoinChannel() or TeamTalkBase.DoLeaveChannel(). Events
         * #ClientEvent.CLIENTEVENT_CMD_USER_JOINED and #ClientEvent.CLIENTEVENT_CMD_USER_LEFT
         * are posted when this value changes. */
        public int nChannelID; 
        /** @brief A bitmask of what the local user subscribes to from
         * this user. Invoking TeamTalkBase.DoSubscribe() and TeamTalkBase.DoUnsubscribe()
         * on the local client instance can change this value. Event
         * #ClientEvent.CLIENTEVENT_CMD_USER_UPDATE is posted if this value
         * changes. */
        public Subscription uLocalSubscriptions;
        /** @brief A bitmask of what this user subscribes to from
         * local client instance. Invoking TeamTalkBase.DoSubscribe() and
         * TeamTalkBase.DoUnsubscribe() on the remoe client instance can change
         * this value. Event #ClientEvent.CLIENTEVENT_CMD_USER_UPDATE is posted if
         * this value changes. */
        public Subscription uPeerSubscriptions;
         /** @brief The user's nickname. Invoking
          * TeamTalkBase.DoChangeNickname() changes this value. Event
          * #ClientEvent.CLIENTEVENT_CMD_USER_UPDATE is posted if this value
          * changes. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szNickname;
        /** @brief The user's current status mode. Invoke
         * TeamTalkBase.DoChangeStatus() to change this value. Event
         * #ClientEvent.CLIENTEVENT_CMD_USER_UPDATE is posted if this value
         * changes. */
        public int nStatusMode;
        /** @brief The user's current status message. Invoke
         * TeamTalkBase.DoChangeStatus() to change this value. Event
         * #ClientEvent.CLIENTEVENT_CMD_USER_UPDATE is posted if this value
         * changes. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szStatusMsg;
        /** @brief A bitmask of the user's current state,
         * e.g. talking, muted, etc.   */
        public UserState uUserState;
        /** @brief Store audio received from this user to this
         * folder. @see TeamTalkBase.SetUserMediaStorageDir */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szMediaStorageDir;
        /** @brief The user's voice volume level. Note that it's a virtual 
         * volume which is being set since the master volume affects 
         * the user volume. The value will be between
         * #BearWare.SoundLevel.SOUND_VOLUME_MIN and #BearWare.SoundLevel.SOUND_VOLUME_MAX
         * @see TeamTalkBase.SetUserVolume */
        public int nVolumeVoice;
        /** @brief The user's voice volume level. Note that it's a virtual 
         * volume which is being set since the master volume affects 
         * the user volume. The value will be between
         * #BearWare.SoundLevel.SOUND_VOLUME_MIN and #BearWare.SoundLevel.SOUND_VOLUME_MAX
         * @see TeamTalkBase.SetUserVolume */
        public int nVolumeMediaFile;
        /** @brief The delay of when a user should no longer be 
         * considered as talking.
         * @see TeamTalkBase.SetUserStoppedTalkingDelay */
        public int nStoppedDelayVoice;
        /** @brief The delay of when a user should no longer be 
         * considered playing audio of a media file.
         * @see TeamTalkBase.SetUserStoppedTalkingDelay */
        public int nStoppedDelayMediaFile;
        /** @brief User's position when using 3D-sound (DirectSound option).
         * Index 0 is x-axis, index 1 is y-axis and index 2 is Z-axis.
         * @see TeamTalkBase.SetUserPosition
         * @see SoundDevice */
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
        public float[] soundPositionVoice;
        /** @brief User's position when using 3D-sound (DirectSound option).
         * Index 0 is x-axis, index 1 is y-axis and index 2 is Z-axis.
         * @see TeamTalkBase.SetUserPosition
         * @see SoundDevice */
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
        public float[] soundPositionMediaFile;
        /** @brief Check what speaker a user is outputting to. 
         * If index 0 is TRUE then left speaker is playing. If index 1 is
         * TRUE then right speaker is playing.
         * @see TeamTalkBase.SetUserStereo */
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public bool[] stereoPlaybackVoice;
        /** @brief Check what speaker a user is outputting to. 
         * If index 0 is TRUE then left speaker is playing. If index 1 is
         * TRUE then right speaker is playing.
         * @see TeamTalkBase.SetUserStereo */
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public bool[] stereoPlaybackMediaFile;
        /** @brief The size of the buffer (in msec) to hold voice
         * content.
         * @see TeamTalkBase.SetUserAudioStreamBufferSize() */
        public int nBufferMSecVoice;
        /** @brief The size of the buffer (in msec) to hold media file 
         * content.
         * @see TeamTalkBase.SetUserAudioStreamBufferSize() */
        public int nBufferMSecMediaFile;
        /** @brief The name of the client application which the user
         * is using. This is the value passed as @c szClientName in
         * TT_DoLoginEx() */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szClientName;
    }

    /**
     * @brief Packet reception and data statistics for a user.
     * 
     * @see BearWare.TeamTalkBase.GetUserStatistics */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct UserStatistics
    {
        /** @brief Number of voice packets received from user. */
        public long nVoicePacketsRecv;
        /** @brief Number of voice packets lost from user. */
        public long nVoicePacketsLost;
        /** @brief Number of video packets received from user. A video 
         * frame can consist of several video packets. */
        public long nVideoCapturePacketsRecv;
        /** @brief Number of video frames received from user. */
        public long nVideoCaptureFramesRecv;
        /** @brief Video frames which couldn't be shown because packets were
         * lost. */
        public long nVideoCaptureFramesLost;
        /** @brief Number of video frames dropped because user application  
         * didn't retrieve video frames in time. */
        public long nVideoCaptureFramesDropped;
        /** @brief Number of media file audio packets received from user. */
        public long nMediaFileAudioPacketsRecv;
        /** @brief Number of media file audio packets lost from user. */
        public long nMediaFileAudioPacketsLost;
        /** @brief Number of media file video packets received from user. 
         * A video frame can consist of several video packets. */
        public long nMediaFileVideoPacketsRecv;
        /** @brief Number of media file video frames received from user. */
        public long nMediaFileVideoFramesRecv;
        /** @brief Media file  video frames which couldn't be shown because 
         * packets were lost. */
        public long nMediaFileVideoFramesLost;
        /** @brief Number of media file video frames dropped because user 
         * application didn't retrieve video frames in time. */
        public long nMediaFileVideoFramesDropped;
    }

    /** 
     * @brief Text message types.
     * 
     * The types of messages which can be passed to TeamTalkBase.DoTextMessage()().
     *
     * @see BearWare.TextMessage
     * @see BearWare.TeamTalkBase.DoTextMessage
     * @see CLIENTEVENT_CMD_USER_TEXTMSG */ 
    public enum TextMsgType : uint
    {
        /** @brief A User to user text message. A message of this
         * type can be sent across channels. */
        MSGTYPE_USER        = 1,
        /** @brief A User to channel text message. Users of type
         * #UserType.USERTYPE_DEFAULT can only send this text message to the
         * channel they're participating in, whereas users of type
         * #UserType.USERTYPE_ADMIN can send to any channel. */
        MSGTYPE_CHANNEL     = 2,
        /** @brief A broadcast message. Requires
         * #UserRight.USERRIGHT_TEXTMESSAGE_BROADCAST. */
        MSGTYPE_BROADCAST   = 3,
        /** @brief A custom user to user text message. Works the same
         * way as #TextMsgType #TextMsgType.MSGTYPE_USER. */
        MSGTYPE_CUSTOM = 4
    }

    /** 
     * @brief A struct containing the properties of a text message
     * sent by a user.
     *
     * @see OnCmdUserTextMessage
     * @see TeamTalkBase.DoTextMessage
     * @see TeamTalkBase.GetTextMessage */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct TextMessage
    {
        /** @brief The type of text message */
        public TextMsgType nMsgType;
        /** @brief Will be set automatically on outgoing message. */
        public int nFromUserID;
        /** @brief The originators username. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szFromUsername;
        /** @brief Set to zero if channel message. */
        public int nToUserID;
        /** @brief Set to zero if @a nMsgType is #TextMsgType #TextMsgType.MSGTYPE_USER or
         * #TextMsgType #TextMsgType.MSGTYPE_BROADCAST. */
        public int nChannelID;
        /** @brief The actual text message. The message can be
         * multi-line (include EOL)  */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szMessage;
    }
    /** @} */

    /** @addtogroup channels
     * @{ */

    /**
     * @brief The types of channels supported. @see Channel */
    [Flags]
    public enum ChannelType : uint
    {
        /** @brief A default channel is a channel which disappears
         * after the last user leaves the channel. */
        CHANNEL_DEFAULT                                         = 0x0000,
        /** @brief A channel which persists even when the last user
         * leaves the channel. */
        CHANNEL_PERMANENT                                       = 0x0001,
        /** @brief Only one user can transmit at a time. */
        CHANNEL_SOLO_TRANSMIT                                   = 0x0002,
        /** @brief Voice and video transmission in the channel is
         * controlled by a channel operator.
         *
         * For a user to transmit audio or video to this type of
         * channel the channel operator must add the user's ID to @c
         * transmitUsers in the #BearWare.Channel struct and call
         * TeamTalkBase.DoUpdateChannel().
         *
         * @see TeamTalkBase.IsChannelOperator
         * @see #UserType.USERTYPE_ADMIN */
        CHANNEL_CLASSROOM = 0x0004,
        /** @brief Only channel operators (and administrators) will receive 
         * audio/video/desktop transmissions. Default channel users 
         * will only see transmissions from operators and/or 
         * administrators. */
        CHANNEL_OPERATOR_RECVONLY                               = 0x0008,
        /** @brief Don't allow voice transmission if it's trigged by
         * voice activation. @see TeamTalkBase.EnableVoiceActivation() */
        CHANNEL_NO_VOICEACTIVATION                              = 0x0010,
        /** @brief Don't allow recording to files in the channel. */
        CHANNEL_NO_RECORDING                                    = 0x0020
    }

    /**
     * @brief A struct containing the properties of a channel.
     * 
     *
     * To change the properties of a channel call
     * TeamTalkBase.DoUpdateChannel(). Note that @a audiocodec cannot be
     * changed if the channel has users.
     *
     * @see TeamTalkBase.GetChannel
     * @see ChannelType
     * @see AudioCodec */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct Channel
    {
        /** @brief Parent channel ID. 0 means no parent channel,
         * i.e. it's the root channel. */
        public int nParentID;
        /** @brief The channel's ID. A value from 1 -
         * #BearWare.TeamTalkBase.TT_CHANNELID_MAX. */
        public int nChannelID;
        /** @brief Name of the channel. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szName;
        /** @brief Topic of the channel. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szTopic;
        /** @brief Password to join the channel.  When extracted
         * through TeamTalkBase.GetChannel() the password will only be set for
         * users of user-type #UserType #UserType.USERTYPE_ADMIN. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szPassword;
        /** @brief Whether password is required to join channel. Read-only 
         * property. */
        public bool bPassword;
        /** @brief A bitmask of the type of channel based on #BearWare.ChannelType. */
        public ChannelType uChannelType;
        /** @brief User specific data which will be stored on
         * persistent storage on the server if the channel type is
         * #ChannelType.CHANNEL_PERMANENT. */
        public int nUserData;
        /** @brief Number of bytes available for file storage. */
        public long nDiskQuota;
        /** @brief Password to become channel operator. @see
         * TeamTalkBase.DoChannelOpEx() */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szOpPassword;
        /** @brief Max number of users in channel. */
        public int nMaxUsers;
        /** @brief The audio codec used by users in the channel. */
        public AudioCodec audiocodec;
        /** @brief The audio configuration which users who join the
         * channel should use. @see TeamTalkBase.SetSoundInputPreprocess() */
        public AudioConfig audiocfg;
        /** @brief List of users who can transmit in a channel.
         * 
         * @c transmitUsers is a 2-dimensional array which specifies
         * who can transmit to the channel.
         *
         * If @c uChannelType is set to #ChannelType.CHANNEL_CLASSROOM then only
         * the users in @c transmitUsers are allowed to transmit. 
         *
         * In TeamTalk v5.4 and onwards adding a user ID to @c
         * transmitUsers will block the user from transmitting if the
         * #BearWare.ChannelType is not #ChannelType.CHANNEL_CLASSROOM. Basically the
         * opposite effect of #ChannelType.CHANNEL_CLASSROOM.
         * 
         * To specify user ID 46 can transmit voice to a
         * #ChannelType.CHANNEL_CLASSROOM channel is done by assigning the
         * following:
         *
         * @verbatim
         * transmitUsers[0][0] = 46;
         * transmitUsers[0][1] = StreamType.STREAMTYPE_VOICE;
         * @endverbatim
         *
         * To specify user ID 46 can transmit both voice and video
         * capture to a #ChannelType.CHANNEL_CLASSROOM channel is done by
         * assigning the following:
         *
         * @verbatim
         * transmitUsers[0][0] = 46;
         * transmitUsers[0][1] = StreamType.STREAMTYPE_VOICE | StreamType.STREAMTYPE_VIDEOCAPTURE;
         * @endverbatim
         *
         * The transmission list is terminated by assigning user ID 0
         * to the end of the list, i.e.:
         *
         * @verbatim
         * transmitUsers[0][0] = 0;
         * transmitUsers[0][1] = StreamType.STREAMTYPE_NONE;
         * @endverbatim
         *
         * To allow all users of a #ChannelType.CHANNEL_CLASSROOM channel to
         * transmit a specific #StreamType is done like this:
         *
         * @verbatim
         * transmitUsers[0][0] = TeamTalkBase.TT_CLASSROOM_FREEFORALL;
         * transmitUsers[0][1] = StreamType.STREAMTYPE_VOICE;
         * @endverbatim
         *
         * Only channel operators are allowed to change the users who
         * are allowed to transmit data to a channel. Call
         * TeamTalkBase.DoUpdateChannel() to update the list of users who
         * are allowed to transmit data to the channel.
         *
         * @see TeamTalkBase.IsChannelOperator
         * @see TeamTalkBase.DoChannelOp
         * @see TeamTalkBase.TT_CLASSROOM_FREEFORALL */
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = TeamTalkBase.TT_TRANSMITUSERS_MAX * 2)]
        public int[,] transmitUsers;
        /** @brief The users currently queued for voice or media file transmission.
         *
         * This property only applied with channel is configured with
         * #BearWare.ChannelType.CHANNEL_SOLO_TRANSMIT. Read-only property. */
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = TeamTalkBase.TT_TRANSMITQUEUE_MAX)]
        public int[] transmitUsersQueue;

        public Channel(bool set_defaults) : this()
        {
            if (set_defaults)
            {
                transmitUsers = new int[TeamTalkBase.TT_TRANSMITUSERS_MAX, 2];
            }
        }

        /** @brief Helper function for adding a user and
         * #BearWare.StreamType to @c transmitUsers */
        public void AddTransmitUser(int nUserID, StreamType uStreamType)
        {
            if (transmitUsers.Rank == 1)
                transmitUsers = new int[TeamTalkBase.TT_TRANSMITUSERS_MAX, 2];

            int i;
            for (i = 0; i < TeamTalkBase.TT_TRANSMITUSERS_MAX; i++)
            {
                if (transmitUsers[i, 0] == 0 || transmitUsers[i, 0] == nUserID)
                    break;
            }
            if (i < TeamTalkBase.TT_TRANSMITUSERS_MAX)
            {
                transmitUsers[i, 0] = nUserID;
                transmitUsers[i, 1] |= (int)uStreamType;
            }
        }
        /** @brief Helper function for getting the #StreamType a user
         * can transmit by querying @c transmitUsers. */
        public StreamType GetTransmitStreamTypes(int nUserID)
        {
            if (transmitUsers.Rank == 1)
                transmitUsers = new int[TeamTalkBase.TT_TRANSMITUSERS_MAX, 2];

            int i;
            for (i = 0; i < TeamTalkBase.TT_TRANSMITUSERS_MAX; i++)
            {
                if (transmitUsers[i, 0] == nUserID)
                    return (StreamType)transmitUsers[i, 1];
            }
            return StreamType.STREAMTYPE_NONE;
        }
        /** @brief Get the number of users who can currently transmit to the #ChannelType.CHANNEL_CLASSROOM. */
        public int GetTransmitUserCount()
        {
            int i;
            for (i = 0; i < TeamTalkBase.TT_TRANSMITUSERS_MAX; i++)
            {
                if (transmitUsers[i, 0] == 0)
                    break;
            }
            return i;
        }
        /** @brief Helper function for removing a #StreamType for a user in @c transmitUsers. */
        public void RemoveTransmitUser(int nUserID, StreamType uStreamType)
        {
            if (transmitUsers.Rank == 1)
                transmitUsers = new int[TeamTalkBase.TT_TRANSMITUSERS_MAX, 2];

            int i;
            for (i = 0; i < TeamTalkBase.TT_TRANSMITUSERS_MAX; i++)
            {
                if (transmitUsers[i, 0] == nUserID)
                    break;
            }
            if (i < TeamTalkBase.TT_TRANSMITUSERS_MAX)
            {
                transmitUsers[i, 0] = nUserID;
                transmitUsers[i, 1] &= (int)~uStreamType;

                if (transmitUsers[i, 1] == (int)StreamType.STREAMTYPE_NONE)
                {
                    for (; i < TeamTalkBase.TT_TRANSMITUSERS_MAX - 1; i++)
                    {
                        transmitUsers[i, 0] = transmitUsers[i + 1, 0];
                        transmitUsers[i, 1] = transmitUsers[i + 1, 1];
                    }
                }
            }
        }
    }

    /** @brief Status of a file transfer.
     * @see OnFileTransfer */
    public enum FileTransferStatus : uint
    {
        /** brief Invalid transfer. */
        FILETRANSFER_CLOSED = 0,
        /** @brief Error during file transfer. */
        FILETRANSFER_ERROR = 1,
        /** @brief File transfer active. */
        FILETRANSFER_ACTIVE = 2,
        /** @brief File transfer finished. */
        FILETRANSFER_FINISHED = 3
    }

    /** 
     * @brief A struct containing the properties of a file transfer.
     * @see TeamTalkBase.GetFileTransferInfo */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct FileTransfer
    {
        /** @brief Status of file transfer. */
        public FileTransferStatus nStatus;
        /** @brief The ID identifying the file transfer. */
        public int nTransferID;
        /** @brief The channel where the file is/will be located. */
        public int nChannelID;
        /** @brief The file path on local disk. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szLocalFilePath;
        /** @brief The filename in the channel. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szRemoteFileName;
        /** @brief The size of the file being transferred. */
        public long nFileSize;
        /** @brief The number of bytes transferred so far. */
        public long nTransferred;
        /** @brief TRUE if download and FALSE if upload. */
        public bool bInbound;
    }

    /**
     * @brief A struct containing the properties of a file in a #BearWare.Channel.
     * @see TeamTalkBase.GetChannelFileInfo */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct RemoteFile
    {
        /** @brief The ID of the channel where the file is located. */
        public int nChannelID;
        /** @brief The ID identifying the file. */
        public int nFileID;
        /** @brief The name of the file. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szFileName;
        /** @brief The size of the file. */
        public long nFileSize;
        /** @brief Username of the person who uploaded the files. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szUsername;
    }
    /** @} */


    /** @ingroup connectivity
     * @brief Control timers for sending keep alive information to the
     * server.
     *
     * @see TT_DoPing()
     */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct ClientKeepAlive
    {
        /** @brief The duration before the TeamTalk instance should consider
         * the client/server connection lost.
         *
         * This value must be greater than @c
         * nTcpKeepAliveIntervalMSec and @c nUdpKeepAliveIntervalMSec.
         *
         * This timeout applies to both the TCP and UDP
         * connection. I.e. @c nTcpServerSilenceSec or @c
         * nUdpServerSilenceSec in #BearWare.ClientStatistics should not exceed
         * this value.  */
        public int nConnectionLostMSec;
        /** @brief Client instance's interval between TeamTalkBase.DoPing()
         * command. Read-only value. Will be half of
         * #BearWare.ServerProperties' @c nUserTimeout.
         */
        public int nTcpKeepAliveIntervalMSec;
        /** @brief Client instance's interval between sending UDP keep
         * alive packets. This value must be less than @c
         * nConnectionLostMSec. */
        public int nUdpKeepAliveIntervalMSec;
        /** @brief Client instance's interval for retransmitting UDP
         * keep alive packets. */
        public int nUdpKeepAliveRTXMSec;
        /** @brief Client instance's interval for retransmitting UDP
         * connect packets. UDP connect packets are only sent when
         * TeamTalkBase.Connect() is initially called. */
        public int nUdpConnectRTXMSec;
        /** @brief The duration before the TeamTalk instance should give up
         * trying to connect to the server. */
        public int nUdpConnectTimeoutMSec;
    }

    /** @ingroup connectivity
     * @brief Statistics of bandwidth usage and ping times in the local 
     * client instance.
     * @see TeamTalkBase.GetClientStatistics */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct ClientStatistics
    {
        /** @brief Bytes sent on UDP. */
        public long nUdpBytesSent;
        /** @brief Bytes received on UDP. */
        public long nUdpBytesRecv;
        /** @brief Voice data sent (on UDP). */
        public long nVoiceBytesSent;
        /** @brief Voice data received (on UDP). */
        public long nVoiceBytesRecv;
        /** @brief Video data sent (on UDP). */
        public long nVideoCaptureBytesSent;
        /** @brief Video data received (on UDP). */
        public long nVideoCaptureBytesRecv;
        /** @brief Audio from media file data sent (on UDP). */
        public long nMediaFileAudioBytesSent;
        /** @brief Audio from media file data received (on UDP). */
        public long nMediaFileAudioBytesRecv;
        /** @brief Video from media file data sent (on UDP). */
        public long nMediaFileVideoBytesSent;
        /** @brief Video from media file data received (on UDP). */
        public long nMediaFileVideoBytesRecv;
        /** @brief Desktop data sent (on UDP). */
        public long nDesktopBytesSent;
        /** @brief Desktop data received (on UDP). */
        public long nDesktopBytesRecv;
        /** @brief Response time to server on UDP (based on ping/pong
         * sent at a specified interval. Set to -1 if not currently
         * available. */
        public int nUdpPingTimeMs;
        /** @brief Response time to server on TCP (based on ping/pong
         * sent at a specified interval. Set to -1 if not currently
         * available.   @see TeamTalkBase.DoPing()  @see ClientKeepAlive */
        public int nTcpPingTimeMs;
        /** @brief The number of seconds nothing has been received by
         * the client on TCP. @see TeamTalkBase.DoPing() @see ClientKeepAlive */
        public int nTcpServerSilenceSec;
        /** @brief The number of seconds nothing has been received by
         * the client on UDP.  @see ClientKeepAlive */
        public int nUdpServerSilenceSec;
    }

    /** @addtogroup errorhandling
     * @{ */

    /**
     * @brief Errors which can occur either as a result of client
     * commands or as a result of internal errors.
     *
     * Use TeamTalkBase.GetErrorMessage() to get a text-description of the
     * error. */
    public enum ClientError : uint
    {
        /** @brief Command indicating success. */
        CMDERR_SUCCESS = 0,
        SUCCESS = 0,

        /* COMMAND ERRORS 1000-1999 ARE DUE TO INVALID OR UNSUPPORTED
         * COMMANDS */

        /** @brief Command has syntax error. Only used internally. */
        CMDERR_SYNTAX_ERROR                     = 1000,
        /** @brief The server doesn't support the issued command.
         *
         * This error may occur if the server is an older version than
         * the client instance. */
        CMDERR_UNKNOWN_COMMAND                  = 1001,
        /** @brief Command cannot be performed due to missing
         * parameter. Only used internally. */
        CMDERR_MISSING_PARAMETER                = 1002,
        /** @brief The server uses a protocol which is incompatible
         * with the client instance. */
        CMDERR_INCOMPATIBLE_PROTOCOLS           = 1003,
        /** @brief The server does not support the audio codec specified
         * by the client. Introduced in version 4.1.0.1264. 
         * @see TeamTalkBase.DoMakeChannel()
         * @see TeamTalkBase.DoJoinChannel() */
        CMDERR_UNKNOWN_AUDIOCODEC               = 1004,
        /** @brief Invalid username for #BearWare.UserAccount.
         * @see TeamTalkBase.DoNewUserAccount() */
        CMDERR_INVALID_USERNAME = 1005,

        /* COMMAND ERRORS 2000-2999 ARE DUE TO INSUFFICIENT RIGHTS */

        /** @brief Invalid server password. 
         *
         * The TeamTalkBase.DoLogin() command passed a server password which was
         * invalid.  @see TeamTalkBase.DoLogin */
        CMDERR_INCORRECT_SERVER_PASSWORD        = 2000,
        /** @brief Invalid channel password. 
         *
         * The TeamTalkBase.DoJoinChannel() or TeamTalkBase.DoJoinChannel()ByID passed an
         * invalid channel password. TeamTalkBase.DoMakeChannel() can also cause
         * a this error if the password is longer than #BearWare.TeamTalkBase.TT_STRLEN. */
        CMDERR_INCORRECT_CHANNEL_PASSWORD       = 2001,
        /** @brief Invalid username or password for account.
         *
         * The TeamTalkBase.DoLogin() command was issued with invalid account
         * properties. This error can also occur by
         * TeamTalkBase.DoNewUserAccount() if username is empty. */
        CMDERR_INVALID_ACCOUNT                  = 2002,
        /** @brief Login failed due to maximum number of users on
         * server.
         *
         * TeamTalkBase.DoLogin() failed because the server does not allow any
         * more users. */
        CMDERR_MAX_SERVER_USERS_EXCEEDED        = 2003,
        /** @brief Cannot join channel because it has maximum number
         * of users.
         *
         * TeamTalkBase.DoJoinChannel() or TeamTalkBase.DoJoinChannel()ByID failed because
         * no more users are allowed in the channel. */
        CMDERR_MAX_CHANNEL_USERS_EXCEEDED       = 2004,
        /** @brief IP-address has been banned from server.
         *
         * TeamTalkBase.DoLogin() failed because the local client's IP-address
         * has been banned on the server. */
        CMDERR_SERVER_BANNED                    = 2005,
        /** @brief Command not authorized.
         *
         * The command cannot be performed because the client instance
         * has insufficient rights.
         *
         * @see TeamTalkBase.DoDeleteFile
         * @see TeamTalkBase.DoJoinChannel
         * @see TeamTalkBase.DoJoinChannelByID
         * @see TeamTalkBase.DoLeaveChannel
         * @see TeamTalkBase.DoChannelOp
         * @see TeamTalkBase.DoChannelOpEx
         * @see TeamTalkBase.DoKickUser
         * @see TeamTalkBase.DoUpdateChannel
         * @see TeamTalkBase.DoChangeNickname
         * @see TeamTalkBase.DoChangeStatus
         * @see TeamTalkBase.DoTextMessage
         * @see TeamTalkBase.DoSubscribe
         * @see TeamTalkBase.DoUnsubscribe
         * @see TeamTalkBase.DoMakeChannel
         * @see TeamTalkBase.DoRemoveChannel
         * @see TeamTalkBase.DoMoveUser
         * @see TeamTalkBase.DoUpdateServer
         * @see TeamTalkBase.DoSaveConfig
         * @see TeamTalkBase.DoSendFile 
         * @see TeamTalkBase.DoRecvFile 
         * @see TeamTalkBase.DoBanUser
         * @see TeamTalkBase.DoUnBanUser
         * @see TeamTalkBase.DoListBans
         * @see TeamTalkBase.DoListUserAccounts
         * @see TeamTalkBase.DoNewUserAccount
         * @see TeamTalkBase.DoDeleteUserAccount */
        CMDERR_NOT_AUTHORIZED                   = 2006,
        /** @brief Cannot upload file because disk quota will be exceeded.
         *
         * TeamTalkBase.DoSendFile() was not allowed because there's not enough
         * disk space available for upload.
         *
         * @see Channel */
        CMDERR_MAX_DISKUSAGE_EXCEEDED           = 2008,

        /** @brief Invalid password for becoming channel operator.
         * 
         * The password specified in TeamTalkBase.DoChannelOpEx() is not correct.
         * The operator password is the @a szOpPassword of the 
         * #BearWare.Channel-struct. */
        CMDERR_INCORRECT_OP_PASSWORD            = 2010,

        /** @brief The selected #BearWare.AudioCodec exceeds what the
         * server allows.
         *
         * A server can limit the vitrate of audio codecs if @c 
         * nAudioCodecBpsLimit of #BearWare.ServerProperties is specified. */
        CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED = 2011,

        /** @brief The maximum number of logins allowed per IP-address has
         * been exceeded.
         * 
         * @see ServerProperties
         * @see TeamTalkBase.DoLogin() */
        CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED = 2012,

        /** @brief The maximum number of channels has been exceeded.
         * @see TT_CHANNELID_MAX */
        CMDERR_MAX_CHANNELS_EXCEEDED            = 2013,

        /** @brief Command flooding prevented by server.
         *
         * Commands are issued faster than allowed by the server. See
         * #BearWare.UserAccount @c commandsPerMSec.  @see TT_CHANNELID_MAX */
        CMDERR_COMMAND_FLOOD = 2014,

        /** @brief Banned from joining a channel.
         * 
         * @see TT_DoJoinChannel()
         * @see TT_DoJoinChannelByID()
         * @see TT_DoBanUser() */
        CMDERR_CHANNEL_BANNED = 2015,

        /* COMMAND ERRORS 3000-3999 ARE DUE TO INVALID STATE OF CLIENT INSTANCE */

        /** @brief Client instance has not been authenticated.
         * 
         * TeamTalkBase.DoLogin() has not been issued successfully or
         * TeamTalkBase.DoLogout() could not be performed because client
         * instance is already logged in.*/
        CMDERR_NOT_LOGGEDIN = 3000,

        /** @brief Already logged in.
         *
         * TeamTalkBase.DoLogin() cannot be performed twice. */
        CMDERR_ALREADY_LOGGEDIN                 = 3001,
        /** @brief Cannot leave channel because not in channel.
         *
         * TeamTalkBase.DoLeaveChannel() failed because user is not in a channel. */
        CMDERR_NOT_IN_CHANNEL                   = 3002,
        /** @brief Cannot join same channel twice.
         * 
         * TeamTalkBase.DoJoinChannel() or TeamTalkBase.DoJoinChannel()ByID failed because
         * client instance is already in the specified channel. */
        CMDERR_ALREADY_IN_CHANNEL               = 3003,
        /** @brief Channel already exists.
         *
         * TeamTalkBase.DoMakeChannel() failed because channel already exists. */
        CMDERR_CHANNEL_ALREADY_EXISTS           = 3004,
        /** @brief Channel does not exist.
         *
         * Command failed because channel does not exists.
         * @see TeamTalkBase.DoRemoveChannel
         * @see TeamTalkBase.DoUpdateChannel
         * @see TeamTalkBase.DoMakeChannel Due to invalid channel name
         * @see TeamTalkBase.DoSendFile
         * @see TeamTalkBase.DoRecvFile
         * @see TeamTalkBase.DoDeleteFile
         * @see TeamTalkBase.DoJoinChannel
         * @see TeamTalkBase.DoJoinChannelByID
         * @see TeamTalkBase.DoLeaveChannel
         * @see TeamTalkBase.DoChannelOp
         * @see TeamTalkBase.DoKickUser
         * @see TeamTalkBase.DoBanUser
         * @see TeamTalkBase.DoMoveUser
         * @see TeamTalkBase.DoTextMessage */
        CMDERR_CHANNEL_NOT_FOUND                = 3005,
        /** @brief User not found.
         * 
         * Command failed because user does not exists.
         * @see TeamTalkBase.DoChannelOp
         * @see TeamTalkBase.DoKickUser
         * @see TeamTalkBase.DoBanUser
         * @see TeamTalkBase.DoMoveUser
         * @see TeamTalkBase.DoTextMessage
         * @see TeamTalkBase.DoSubscribe
         * @see TeamTalkBase.DoUnsubscribe */
        CMDERR_USER_NOT_FOUND                   = 3006,
        /** @brief Banned IP-address does not exist.
         * 
         * TeamTalkBase.DoUnBanUser() failed because there is no banned
         * IP-address which matches what was specified. */
        CMDERR_BAN_NOT_FOUND                    = 3007,
        /** @brief File transfer doesn't exists.
         *
         * TeamTalkBase.DoSendFile() or TeamTalkBase.DoRecvFile() failed because the server
         * cannot process the file transfer. */
        CMDERR_FILETRANSFER_NOT_FOUND           = 3008,
        /** @brief Server failed to open file.
         *
         * TeamTalkBase.DoSendFile() or TeamTalkBase.DoRecvFile() failed because the server
         * cannot open the specified file (possible file lock). */
        CMDERR_OPENFILE_FAILED                  = 3009,
        /** @brief Cannot find user account.
         * 
         * TeamTalkBase.DoDeleteUserAccount() failed because the specified user
         * account does not exists. */
        CMDERR_ACCOUNT_NOT_FOUND                = 3010,
        /** @brief File does not exist.
         *
         * TeamTalkBase.DoSendFile(), TeamTalkBase.DoRecvFile() or TeamTalkBase.DoDeleteFile() failed
         * because the server cannot find the specified file. */
        CMDERR_FILE_NOT_FOUND                   = 3011,
        /** @brief File already exist.
         *
         * TeamTalkBase.DoSendFile() failed because the file already exists in
         * the channel. */
        CMDERR_FILE_ALREADY_EXISTS              = 3012,
        /** @brief Server does not allow file transfers.
         *
         * TeamTalkBase.DoSendFile() or TeamTalkBase.DoRecvFile() failed because the server
         * does not allow file transfers. */
        CMDERR_FILESHARING_DISABLED             = 3013,

        /** @brief Cannot process command since channel is not empty.
         * @see TeamTalkBase.DoUpdateChannel() #BearWare.AudioCodec cannot be changed while
         * there are users in a channel. */
        CMDERR_CHANNEL_HAS_USERS                = 3015,

        /** @brief The login service is currently unavailable.
         *
         * Added in TeamTalk v5.3 to support web-logins. */
        CMDERR_LOGINSERVICE_UNAVAILABLE = 3016,

        /* ERRORS 10000-10999 ARE NOT COMMAND ERRORS BUT INSTEAD
         * ERRORS IN THE CLIENT INSTANCE. */

        /** @brief A sound input device failed. 
         *
         * This can e.g. happen when joining a channel and the sound
         * device has been unplugged. */
        INTERR_SNDINPUT_FAILURE = 10000,
        /** @brief A sound output device failed.
         *
         * This can e.g. happen when joining a channel and the sound
         * device has been unplugged. */
        INTERR_SNDOUTPUT_FAILURE                = 10001,
        /** @brief Audio codec used by channel failed to initialize.
         * Ensure the settings specified in #BearWare.AudioCodec are valid.
         * @see TeamTalkBase.DoJoinChannel() */
        INTERR_AUDIOCODEC_INIT_FAILED           = 10002,
        /** @brief #BearWare.SpeexDSP failed to initialize.
         *
         * This error occurs when joining a channel.
         *
         * The settings specified by TeamTalkBase.SetSoundInputPreprocess() are
         * invalid for the specified audio codec. @see
         * TeamTalkBase.DoJoinChannel() */
        INTERR_SPEEXDSP_INIT_FAILED             = 10003,
        /** @brief #BearWare.TTMessage event queue overflowed.
         *
         * The message queue for events has overflowed because
         * TeamTalkBase.GetMessage() has not drained the queue in
         * time. The #BearWare.TTMessage message queue will suspend
         * event handling once the queue overflows and resumes event
         * handling again when the message queue has been drained. */
        INTERR_TTMESSAGE_QUEUE_OVERFLOW         = 10004
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct ClientErrorMsg
    {
        /** @brief Error number based on #ClientError. */
        public int nErrorNo;
        /** @brief Text message describing the error. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szErrorMsg;
    }

    /** @} */

    /** @addtogroup events
     * @{ */

    /** 
     * @brief TeamTalk client event messages.
     *
     * Events are retrieved using TeamTalkBase.GetMessage(). */
    public enum ClientEvent : uint
    {
        CLIENTEVENT_NONE = 0,
        /**
         * @brief Connected successfully to the server.
         *
         * This event is posted if TeamTalkBase.Connect() was successful.
         *
         * TeamTalkBase.DoLogin() can now be called in order to logon to the
         * server.
         *
         * @param nSource 0
         * @param ttType #TTType.__NONE
         * @see TeamTalkBase.Connect */
        CLIENTEVENT_CON_SUCCESS = CLIENTEVENT_NONE + 10,
        /** 
         * @brief Failed to connect to server.
         *
         * This event is posted if TeamTalkBase.Connect() fails. Ensure to call
         * TeamTalkBase.Disconnect() before calling TeamTalkBase.Connect() again.
         *
         * @param nSource 0
         * @param ttType #TTType.__NONE
         * @see TeamTalkBase.Connect */
        CLIENTEVENT_CON_FAILED = CLIENTEVENT_NONE + 20,
        /** 
         * @brief Connection to server has been lost.
         *
         * The server is not responding to requests from the local
         * client instance and the connection has been dropped.
         *
         * TeamTalkBase.GetClientStatistics() can be used to check when data was
         * last received from the server.
         *
         * Ensure to call TeamTalkBase.Disconnect() before calling TeamTalkBase.Connect()
         * again.
         *
         * @param nSource 0
         * @param ttType #TTType.__NONE
         * @see TeamTalkBase.Connect */
        CLIENTEVENT_CON_LOST = CLIENTEVENT_NONE + 30,
        /**
         * @brief The maximum size of the payload put into UDP packets
         * has been updated.
         *
         * @param nSource Ignored
         * @param ttType #TTType.__INT32
         * @param nPayloadSize Placed in union of #BearWare.TTMessage. The
         * maximum size in bytes of the payload data which is put in
         * UDP packets. 0 means the max payload query failed.  @see
         * TeamTalkBase.QueryMaxPayload() */
        CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED = CLIENTEVENT_NONE + 40,
        /** 
         * @brief A command issued by @c TeamTalkBase.Do* methods is being
         * processed.
         *
         * Read section @ref cmdprocessing on how to use command
         * processing in the user application.
         *
         * @param nSource Command ID being processed (returned by
         * TeamTalkBase.Do* commands)
         * @param ttType #TTType.__TTBOOL
         * @param bActive Placed in union of #BearWare.TTMessage. Is TRUE if
         * command ID started processing and FALSE if the command has
         * finished processing. */
        CLIENTEVENT_CMD_PROCESSING = CLIENTEVENT_NONE + 200,
        /** 
         * @brief The server rejected a command issued by the local
         * client instance.
         *
         * To figure out which command failed use the command ID
         * returned by the TeamTalkBase.Do* command. Section @ref cmdprocessing
         * explains how to use command ID.
         *
         * @param nSource The command ID returned from the TeamTalkBase.Do*
         * commands.
         * @param ttType #TTType.__CLIENTERRORMSG
         * @param clienterrormsg Placed in union of #BearWare.TTMessage. Contains
         * error description. */
        CLIENTEVENT_CMD_ERROR = CLIENTEVENT_NONE + 210,
        /**
         * @brief The server successfully processed a command issued
         * by the local client instance.
         *
         * To figure out which command succeeded use the command ID
         * returned by the TeamTalkBase.Do* command. Section @ref cmdprocessing
         * explains how to use command ID.
         *
         * @param nSource The command ID returned from the TeamTalkBase.Do*
         * commands.
         * @param ttType #TTType.__NONE */
        CLIENTEVENT_CMD_SUCCESS = CLIENTEVENT_NONE + 220,
        /** 
         * @brief The client instance successfully logged on to
         * server.
         *
         * The call to TeamTalkBase.DoLogin() was successful and all channels on
         * the server will be posted in the event
         * #ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW immediately following this
         * event. If #UserRight.USERRIGHT_VIEW_ALL_USERS is enabled the client
         * instance will also receive the events
         * #ClientEvent.CLIENTEVENT_CMD_USER_LOGGEDIN and
         * #ClientEvent.CLIENTEVENT_CMD_USER_JOINED for every user on the server.
         *
         * @param nSource The client instance's user ID, i.e. what can now 
         * be retrieved through TeamTalkBase.GetMyUserID().
         * @param ttType #TTType.__USERACCOUNT
         * @param useraccount Placed in union of #BearWare.TTMessage.
         * @see TeamTalkBase.DoLogin */
        CLIENTEVENT_CMD_MYSELF_LOGGEDIN = CLIENTEVENT_NONE + 230,
        /** 
         * @brief The client instance logged out of the server.
         *
         * A response to TeamTalkBase.DoLogout().
         *
         * @param nSource User ID of local client instance.
         * @param ttType #TTType.__NONE
         * @see TeamTalkBase.DoLogout */
        CLIENTEVENT_CMD_MYSELF_LOGGEDOUT = CLIENTEVENT_NONE + 240,
        /** 
         * @brief The client instance was kicked from a channel.
         *
         * @param nSource If greater than zero indicates local client
         * instance was kicked from a channel. Otherwise kicked from
         * server.
         * @param ttType #TTType.__USER if kicked by a user otherwise #TTType.__NONE.
         * @param user Placed in union of #BearWare.TTMessage if @a ttType
         * equals #TTType.__USER otherwise #TTType.__NONE. */
        CLIENTEVENT_CMD_MYSELF_KICKED = CLIENTEVENT_NONE + 250,
        /**
         * @brief A new user logged on to the server.
         *
         * @param nSource 0
         * @param ttType #TTType.__USER
         * @param user Placed in union of #BearWare.TTMessage.
         *
         * @see TeamTalkBase.DoLogin
         * @see TeamTalkBase.GetUser To retrieve user.
         * @see CLIENTEVENT_CMD_USER_LOGGEDOUT */
        CLIENTEVENT_CMD_USER_LOGGEDIN = CLIENTEVENT_NONE + 260,
        /**
         * @brief A client logged out of the server. 
         *
         * This event is called when a user logs out with
         * TeamTalkBase.DoLogout() or disconnects with TeamTalkBase.Disconnect().
         *
         * @param nSource 0
         * @param ttType #TTType.__USER
         * @param user Placed in union of #BearWare.TTMessage.
         *
         * @see TeamTalkBase.DoLogout
         * @see TeamTalkBase.Disconnect
         * @see CLIENTEVENT_CMD_USER_LOGGEDIN */
        CLIENTEVENT_CMD_USER_LOGGEDOUT = CLIENTEVENT_NONE + 270,
        /**
         * @brief User changed properties.
         *
         * @param nSource 0
         * @param ttType #TTType.__USER
         * @param user Placed in union of #BearWare.TTMessage.
         *
         * @see TeamTalkBase.GetUser To retrieve user. */
        CLIENTEVENT_CMD_USER_UPDATE = CLIENTEVENT_NONE + 280,
        /** 
         * @brief A user has joined a channel.
         *
         * @param nSource 0
         * @param ttType #TTType.__USER
         * @param user Placed in union of #BearWare.TTMessage.
         *
         * @see TeamTalkBase.GetUser To retrieve user. */
        CLIENTEVENT_CMD_USER_JOINED = CLIENTEVENT_NONE + 290,
        /** 
         * @brief User has left a channel.
         *
         * @param nSource Channel ID of previous channel.
         * @param ttType #TTType.__USER
         * @param user Placed in union of #BearWare.TTMessage. */
        CLIENTEVENT_CMD_USER_LEFT = CLIENTEVENT_NONE + 300,
        /** 
         * @brief A user has sent a text-message.
         *
         * @param nSource 0
         * @param ttType #TTType.__TEXTMESSAGE
         * @param textmessage Placed in union of #BearWare.TTMessage.
         *
         * @see TeamTalkBase.GetUser() To retrieve user.
         * @see TeamTalkBase.DoTextMessage() to send text message. */
        CLIENTEVENT_CMD_USER_TEXTMSG = CLIENTEVENT_NONE + 310,
        /** 
         * @brief A new channel has been created.
         *
         * @param nSource 0
         * @param ttType #TTType.__CHANNEL
         * @param channel Placed in union of #BearWare.TTMessage.
         *
         * @see TeamTalkBase.GetChannel To retrieve channel. */
        CLIENTEVENT_CMD_CHANNEL_NEW = CLIENTEVENT_NONE + 320,
        /** 
         * @brief A channel's properties has been updated.
         *
         * @param nSource 0
         * @param ttType #TTType.__CHANNEL
         * @param channel Placed in union of #BearWare.TTMessage.
         *
         * @see TeamTalkBase.GetChannel To retrieve channel. */
        CLIENTEVENT_CMD_CHANNEL_UPDATE = CLIENTEVENT_NONE + 330,
        /** 
         * @brief A channel has been removed.
         *
         * Note that calling the TeamTalkBase.GetChannel() with the channel ID
         * will fail because the channel is no longer there.
         *
         * @param nSource 0
         * @param ttType #TTType.__CHANNEL
         * @param channel Placed in union of #BearWare.TTMessage. */
        CLIENTEVENT_CMD_CHANNEL_REMOVE = CLIENTEVENT_NONE + 340,
        /** 
         * @brief Server has updated its settings (server name, MOTD,
         * etc.)
         * 
         * Use TeamTalkBase.GetServerProperties() to get the new server
         * properties.
         *
         * @param nSource 0
         * @param ttType #TTType.__SERVERPROPERTIES
         * @param serverproperties Placed in union of #BearWare.TTMessage. */
        CLIENTEVENT_CMD_SERVER_UPDATE = CLIENTEVENT_NONE + 350,
        /** 
         * @brief Server statistics available.
         *
         * This is a response to TeamTalkBase.DoServerStatistics()
         *
         * @param nSource 0
         * @param ttType #TTType.__SERVERSTATISTICS
         * @param serverstatistics Placed in union of #BearWare.TTMessage. */
        CLIENTEVENT_CMD_SERVERSTATISTICS = CLIENTEVENT_NONE + 360,
        /** 
         * @brief A new file is added to a channel. 
         *
         * Use TeamTalkBase.GetChannelFile() to get information about the
         * file.
         *
         * @param nSource Channel ID where file is located.
         * @param ttType #TTType.__REMOTEFILE
         * @param remotefile Placed in union of #BearWare.TTMessage.
         *
         * @see TeamTalkBase.GetChannelFile To retrieve file. */
        CLIENTEVENT_CMD_FILE_NEW = CLIENTEVENT_NONE + 370,
        /** 
         * @brief A file has been removed from a channel.
         *
         * @param nSource Channel ID where file was located. 
         * @param ttType #TTType.__REMOTEFILE
         * @param remotefile Placed in union of #BearWare.TTMessage. */
        CLIENTEVENT_CMD_FILE_REMOVE = CLIENTEVENT_NONE + 380,
        /** 
         * @brief A user account has been received from the server.
         *
         * This message is posted as a result of TeamTalkBase.DoListUserAccounts()
         *
         * @param nSource 0
         * @param ttType #TTType.__USERACCOUNT
         * @param useraccount Placed in union of #BearWare.TTMessage. */
        CLIENTEVENT_CMD_USERACCOUNT = CLIENTEVENT_NONE + 390,
        /** 
         * @brief A banned user has been received from the server.
         *
         * This message is posted as a result of TeamTalkBase.DoListBans()
         *
         * @param nSource 0
         * @param ttType #TTType.__BANNEDUSER
         * @param useraccount Placed in union of #BearWare.TTMessage. */
        CLIENTEVENT_CMD_BANNEDUSER = CLIENTEVENT_NONE + 400,
        /**
         * @brief A user state has changed.
         *
         * The @a uUserState has changed for a user due to playback of
         * an incoming stream.
         *
         * The user state changes if:
         * - A user has started/stopped voice stream,
         *   i.e. #UserState.USERSTATE_VOICE
         * - A user has started/stopped a video capture stream,
         *   i.e. #UserState.USERSTATE_VIDEOCAPTURE
         * - A user has started/stopped a media file stream, i.e.
         *   i.e. #UserState.USERSTATE_MEDIAFILE_AUDIO or #UserState.USERSTATE_MEDIAFILE_VIDEO
         *
         * @param nSource 0
         * @param ttType #TTType.__USER.
         * @param user Placed in union of #BearWare.TTMessage.
         *
         * @see TeamTalkBase.SetUserStoppedTalkingDelay() */
        CLIENTEVENT_USER_STATECHANGE = CLIENTEVENT_NONE + 500,
        /** 
         * @brief A new video frame from a video capture device 
         * was received from a user.
         *
         * Use TeamTalkBase.AcquireUserVideoCaptureFrame() to display the video frame.
         *
         * @param nSource User's ID.
         * @param ttType #TTType.__INT32
         * @param nStreamID Placed in union of #BearWare.TTMessage. The ID of
         * the video stream currently active for this user. If stream
         * ID becomes 0 it means the current stream has been
         * closed.  */
        CLIENTEVENT_USER_VIDEOCAPTURE = CLIENTEVENT_NONE + 510,
        /** 
         * @brief A new video frame from a video media file 
         * was received from a user.
         *
         * Use TeamTalkBase.AcquireUserMediaVideoFrame() to display the video frame.
         *
         * @param nSource User's ID.
         * @param ttType #TTType.__INT32
         * @param nStreamID Placed in union of #BearWare.TTMessage. The ID of
         * the video stream currently active for this user. If stream
         * ID becomes 0 it means the current stream has been
         * closed.  */
        CLIENTEVENT_USER_MEDIAFILE_VIDEO = CLIENTEVENT_NONE + 520,
        /**
         * @brief A new or updated desktop window has been received
         * from a user.
         *
         * Use TeamTalkBase.AcquireUserDesktopWindow() to retrieve the bitmap of the
         * desktop window.
         *
         * @param nSource The user's ID.
         * @param ttType #TTType.__INT32
         * @param nStreamID Placed in union of #BearWare.TTMessage. The ID of the
         * desktop window's session. If this ID changes it means the
         * user has started a new session. If the session ID becomes 0
         * it means the desktop session has been closed by the user.
         * @see TeamTalkBase.SendDesktopWindow() */
        CLIENTEVENT_USER_DESKTOPWINDOW = CLIENTEVENT_NONE + 530,
        /**
         * @brief A user has sent the position of the mouse cursor.
         *
         * Use TeamTalkBase.SendDesktopCursorPosition() to send the position of
         * the mouse cursor.
         *
         * @param nSource The user ID of the owner of the mouse cursor.
         * @param ttType #TTType.__DESKTOPINPUT
         * @param desktopinput Placed in union of #BearWare.TTMessage. Contains 
         * the coordinates of the mouse cursor. */
        CLIENTEVENT_USER_DESKTOPCURSOR = CLIENTEVENT_NONE + 540,
        /**
         * @brief Desktop input (mouse or keyboard input) has been
         * received from a user.
         *
         * Desktop input is used in combination with a shared desktop
         * window, see @ref desktopshare.
         *
         * This event is generated if a remote user has called
         * TeamTalkBase.SendDesktopInput(). In order for the local client
         * instance to receive desktop input it must have enabled the
         * subscription #Subscription.SUBSCRIBE_DESKTOPINPUT.
         *
         * Due to different keyboard layout it might be a good idea 
         * to look into the key-translation function 
         * WindowsHelper.DesktopInputKeyTranslate().
         *
         * See @ref rxdesktopinput for more information on receiving
         * desktop input.
         *
         * @param nSource User ID
         * @param ttType #TTType.__DESKTOPINPUT
         * @param desktopinput Placed in union of #BearWare.TTMessage. */
        CLIENTEVENT_USER_DESKTOPINPUT = CLIENTEVENT_NONE + 550,
        /** 
         * @brief A media file recording has changed status.
         *
         * TeamTalkBase.SetUserMediaStorageDir() makes the client instance store all
         * audio from a user to a specified folder. Every time an
         * audio file is being processed this event is posted.
         *
         * @param nSource The user's ID.
         * @param ttType #TTType.__MEDIAFILEINFO
         * @param mediafileinfo Placed in union of
         * #BearWare.TTMessage. Properties for the media file currently being
         * recorded */
        CLIENTEVENT_USER_RECORD_MEDIAFILE = CLIENTEVENT_NONE + 560,
        /**
         * @brief A new audio block can be extracted.
         *
         * The #BearWare.AudioBlock can either be of #StreamType.STREAMTYPE_VOICE or
         * #StreamType.STREAMTYPE_MEDIAFILE_AUDIO.
         * 
         * This event is only generated if TeamTalkBase.EnableAudioBlockEvent()
         * is first called.
         *
         * Call TeamTalkBase.AcquireUserAudioBlock() to extract the #BearWare.AudioBlock.
         *
         * @param nSource The user ID.
         * @param ttType #TTType.__STREAMTYPE */
        CLIENTEVENT_USER_AUDIOBLOCK = CLIENTEVENT_NONE + 570,
        /** 
         * @brief An internal error occurred in the client instance.
         *
         * This can e.g. happen if a new user joins a channel and
         * a sound output device fails to start a new audio stream.
         *
         * For at list of internal error messages check out #ClientError
         * with errors prefixed @c INTERR_*
         *
         * @param nSource 0
         * @param ttType #TTType.__CLIENTERRORMSG
         * @param clienterrormsg Placed in union of #BearWare.TTMessage. Contains
         * information on what caused an error. */
        CLIENTEVENT_INTERNAL_ERROR = CLIENTEVENT_NONE + 1000,
        /** 
         * @brief Voice activation has triggered transmission.
         *
         * @param nSource 0
         * @param ttType #TTType.__TTBOOL
         * @param bActive Placed in union of #BearWare.TTMessage. TRUE if voice
         * is being transmitted due to voice level high than
         * activation level.
         *
         * @see TeamTalkBase.GetSoundInputLevel()
         * @see TeamTalkBase.SetVoiceActivationLevel()
         * @see CLIENT_SNDINPUT_VOICEACTIVATION
         * @see TeamTalkBase.EnableTransmission */
        CLIENTEVENT_VOICE_ACTIVATION = CLIENTEVENT_NONE + 1010,
        /** 
         * @brief A hotkey has been acticated or deactivated.
         *
         * @param nSource The hotkey ID passed to TeamTalkBase.HotKey_Register().
         * @param ttType #TTType.__TTBOOL
         * @param bActive Placed in union of #BearWare.TTMessage. TRUE when
         * hotkey is active and FALSE when it becomes inactive.
         *
         * @see TeamTalkBase.HotKey_Register()
         * @see TeamTalkBase.HotKey_Unregister() */
        CLIENTEVENT_HOTKEY = CLIENTEVENT_NONE + 1020,
        /**
         * @brief A button was pressed or released on the user's
         * keyboard or mouse.
         * 
         * When TeamTalkBase.HotKey_InstallTestHook() is called a hook is
         * installed in Windows which intercepts all keyboard and
         * mouse presses. Every time a key or mouse is pressed or
         * released this event is posted.
         *
         * Use TeamTalkBase.HotKey_GetKeyString() to get a key description of the 
         * pressed key.
         *
         * @param nSource The virtual key code. Look here for a list of virtual
         * key codes: http://msdn.microsoft.com/en-us/library/ms645540(VS.85).aspx
         * @param ttType #TTType.__TTBOOL
         * @param bActive Placed in union of #BearWare.TTMessage. TRUE when key
         * is down and FALSE when released.
         * @see TeamTalkBase.HotKey_InstallTestHook() */
        CLIENTEVENT_HOTKEY_TEST = CLIENTEVENT_NONE + 1030,
        /**
         * @brief A file transfer is processing. 
         *
         * Use TeamTalkBase.GetFileTransferInfo() to get information about the
         * file transfer. Ensure to check if the file transfer is
         * completed, because the file transfer instance will be
         * removed from the client instance when the user application
         * reads the #BearWare.FileTransfer object and it has completed the
         * transfer.
         *
         * @param nSource 0
         * @param ttType #TTType.__FILETRANSFER
         * @param filetransfer Placed in union of #BearWare.TTMessage. Properties 
         * and status information about the file transfer.
         *
         * @see TeamTalkBase.GetFileTransferInfo To retrieve #BearWare.FileTransfer. */
        CLIENTEVENT_FILETRANSFER = CLIENTEVENT_NONE + 1040,
        /**
         * @brief Used for tracking when a desktop window has been
         * transmitted to the server.
         *
         * When the transmission has completed the flag #ClientFlag.CLIENT_TX_DESKTOP
         * will be cleared from the local client instance.
         *
         * @param nSource The desktop session's ID. If the desktop session ID
         * becomes 0 it means the desktop session has been closed and/or
         * cancelled.
         * @param ttType #TTType.__INT32
         * @param nBytesRemain Placed in union of #BearWare.TTMessage. The number of
         * bytes remaining before transmission of last desktop window
         * completes. When remaining bytes is 0 TeamTalkBase.SendDesktopWindow()
         * can be called again. */
        CLIENTEVENT_DESKTOPWINDOW_TRANSFER = CLIENTEVENT_NONE + 1050,
        /** 
         * @brief Media file being streamed to a channel is processing.
         *
         * This event is called as a result of
         * TeamTalkBase.StartStreamingMediaFileToChannel() to monitor progress
         * of streaming.
         *
         * @param nSource 0
         * @param ttType #TTType.__MEDIAFILEINFO
         * @param mediafileinfo Placed in union of #BearWare.TTMessage. Contains
         * properties and status information about the media file 
         * being streamed. */
        CLIENTEVENT_STREAM_MEDIAFILE = CLIENTEVENT_NONE + 1060,
        /**
         * @brief Media file played locally is procesing.
         *
         * This event is called as a result of TeamTalkBase.InitLocalPlayback()
         * to monitor progress of playback.
         * @param nSource Session ID returned by TeamTalkBase.InitLocalPlayback()
         * @param ttType #TTType.__MEDIAFILEINFO
         * @param mediafileinfo Placed in union of #BearWare.TTMessage. Contains
         * properties and status information about the media file
         * being played.
         */
        CLIENTEVENT_LOCAL_MEDIAFILE = CLIENTEVENT_NONE + 1070,
    }


    /* List of structures used internally by TeamTalkBase. */
    public enum TTType : uint
    {
        __NONE                    =  0,
        __AUDIOCODEC              =  1,
        __BANNEDUSER              =  2,
        __VIDEOFORMAT             =  3,
        __OPUSCODEC               =  4,
        __CHANNEL                 =  5,
        __CLIENTSTATISTICS        =  6,
        __REMOTEFILE              =  7,
        __FILETRANSFER            =  8,
        __MEDIAFILESTATUS         =  9,
        __SERVERPROPERTIES        = 10,
        __SERVERSTATISTICS        = 11,
        __SOUNDDEVICE             = 12,
        __SPEEXCODEC              = 13,
        __TEXTMESSAGE             = 14,
        __WEBMVP8CODEC            = 15,
        __TTMESSAGE               = 16,
        __USER                    = 17,
        __USERACCOUNT             = 18,
        __USERSTATISTICS          = 19,
        __VIDEOCAPTUREDEVICE      = 20,
        __VIDEOCODEC              = 21,
        __AUDIOCONFIG             = 22,
        __SPEEXVBRCODEC           = 23,
        __VIDEOFRAME              = 24,
        __AUDIOBLOCK              = 25,
        __AUDIOFORMAT             = 26,
        __MEDIAFILEINFO           = 27,
        __CLIENTERRORMSG          = 28,
        __TTBOOL                  = 29,
        __INT32                   = 30,
        __DESKTOPINPUT            = 31,
        __SPEEXDSP                = 32,
        __STREAMTYPE              = 33,
        __AUDIOPREPROCESSORTYPE   = 34,
        __AUDIOPREPROCESSOR       = 35,
        __TTAUDIOPREPROCESSOR     = 36,
        __MEDIAFILEPLAYBACK       = 37,
        __CLIENTKEEPALIVE         = 38,
    }

    /**
     * @brief A struct containing the properties of an event.
     *
     * The event can be retrieved by called TeamTalkBase.GetMessage(). This
     * struct is only required on non-Windows systems.
     *
     * Section @ref events explains event handling in the local client
     * instance.
     *
     * @see TeamTalkBase.GetMessage */
    [StructLayout(LayoutKind.Sequential)]
    public struct TTMessage
    {
        /** @brief The event's message number @see ClientEvent */
        public ClientEvent nClientEvent;
        /** @brief The source of the event depends on @c wmMsg */
        public int nSource;
        /** @brief Specifies which member to access in the union */
        public TTType ttType;
        /** @brief Reserved. To preserve alignment. */
        public uint uReserved;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 6248)]
        public byte[] data;
        //UnionData data;
        
        public object DataToObject()
        {
            switch (ttType)
            {
                case TTType.__CLIENTERRORMSG:
                    return Marshal.PtrToStructure(TTDLL.TT_DBG_GETDATAPTR(ref this), typeof(ClientErrorMsg));
                case TTType.__CHANNEL:
                    return Marshal.PtrToStructure(TTDLL.TT_DBG_GETDATAPTR(ref this), typeof(Channel));
                case TTType.__DESKTOPINPUT:
                    return Marshal.PtrToStructure(TTDLL.TT_DBG_GETDATAPTR(ref this), typeof(DesktopInput));
                case TTType.__FILETRANSFER:
                    return Marshal.PtrToStructure(TTDLL.TT_DBG_GETDATAPTR(ref this), typeof(FileTransfer));
                case TTType.__MEDIAFILEINFO:
                    return Marshal.PtrToStructure(TTDLL.TT_DBG_GETDATAPTR(ref this), typeof(MediaFileInfo));
                case TTType.__REMOTEFILE:
                    return Marshal.PtrToStructure(TTDLL.TT_DBG_GETDATAPTR(ref this), typeof(RemoteFile));
                case TTType.__SERVERPROPERTIES:
                    return Marshal.PtrToStructure(TTDLL.TT_DBG_GETDATAPTR(ref this), typeof(ServerProperties));
                case TTType.__SERVERSTATISTICS:
                    return Marshal.PtrToStructure(TTDLL.TT_DBG_GETDATAPTR(ref this), typeof(ServerStatistics));
                case TTType.__TEXTMESSAGE:
                    return Marshal.PtrToStructure(TTDLL.TT_DBG_GETDATAPTR(ref this), typeof(TextMessage));
                case TTType.__USER:
                    return Marshal.PtrToStructure(TTDLL.TT_DBG_GETDATAPTR(ref this), typeof(User));
                case TTType.__USERACCOUNT:
                    return Marshal.PtrToStructure(TTDLL.TT_DBG_GETDATAPTR(ref this), typeof(UserAccount));
                case TTType.__BANNEDUSER :
                    return Marshal.PtrToStructure(TTDLL.TT_DBG_GETDATAPTR(ref this), typeof(BannedUser));
                case TTType.__TTBOOL:
                    return Marshal.ReadInt32(TTDLL.TT_DBG_GETDATAPTR(ref this)) != 0;
                case TTType.__INT32:
                    return Marshal.ReadInt32(TTDLL.TT_DBG_GETDATAPTR(ref this));
                case TTType.__STREAMTYPE :
                    return (StreamType)Marshal.ReadInt32(TTDLL.TT_DBG_GETDATAPTR(ref this));
                default:
                    return null;
            }
        }
    }

    [StructLayout(LayoutKind.Explicit, Size = 5224)]
    struct UnionData
    {
        [FieldOffset(0)]
        public ClientErrorMsg clienterrormsg;
        [FieldOffset(0)]
        public Channel channel;
        [FieldOffset(0)]
        public DesktopInput desktopinput;
        [FieldOffset(0)]
        public FileTransfer filetransfer;
        [FieldOffset(0)]
        public MediaFileInfo mediafileinfo;
        [FieldOffset(0)]
        public RemoteFile remotefile;
        [FieldOffset(0)]
        public ServerProperties serverproperties;
        [FieldOffset(0)]
        public TextMessage textmessage;
        [FieldOffset(0)]
        public User user;
        [FieldOffset(0)]
        public UserAccount useraccount;
        [FieldOffset(0)]
        public bool bActive;
        [FieldOffset(0)]
        public int nBytesRemain;
        [FieldOffset(0)]
        public int nStreamID;
        [FieldOffset(0)]
        public int nPayloadSize;
    }

    /** @}*/

    /** @addtogroup initclient
     * @{ */

    /**
     * @brief Flags used to describe the the client instance current
     * state.
     *
     * The client's state is a bitmask of the flags in #ClientFlag.
     *
     * The state of the client instance can be retrieved by calling
     * TeamTalkBase.GetFlags() This enables the user application to display the
     * possible options to the end user. If e.g. the flag
     * #ClientFlag.CLIENT_AUTHORIZED is not set it will not be possible to
     * perform any other commands except TeamTalkBase.DoLogin(). Doing so will
     * make the server return an error message to the client. */
    [Flags]
    public enum ClientFlag : uint
    {
        /** @brief The client instance is in closed state, i.e. no
         * operations has been performed on it. */
        CLIENT_CLOSED = 0x00000000,
        /** @brief If set the client instance's sound input device has
         * been initialized, i.e. TeamTalkBase.InitSoundInputDevice() has
         * been called successfully. */
        CLIENT_SNDINPUT_READY = 0x00000001,
        /** @brief If set the client instance's sound output device
         * has been initialized, i.e. TeamTalkBase.InitSoundOutputDevice()
         * has been called successfully. */
        CLIENT_SNDOUTPUT_READY = 0x00000002,
        /** @brief If set the client instance is running in sound
         * duplex mode where multiple audio output streams are mixed
         * into a single stream. This option must be enabled to
         * support echo cancellation (see #BearWare.SpeexDSP). Call
         * TeamTalkBase.InitSoundDuplexDevices() to enable duplex mode.*/
        CLIENT_SNDINOUTPUT_DUPLEX = 0x00000004,
        /** @brief If set the client instance will start transmitting
         * audio if the sound level is above the voice activation
         * level. The event #ClientEvent.CLIENTEVENT_VOICE_ACTIVATION is posted
         * when voice activation initiates transmission.
         * @see TeamTalkBase.SetVoiceActivationLevel()
         * @see TeamTalkBase.EnableVoiceActivation() */
        CLIENT_SNDINPUT_VOICEACTIVATED = 0x00000008,
        /** @brief If set GetSoundInputLevel() is higher than the
         * voice activation level.  To enable voice transmission if
         * voice level is higher than actication level also enable
         * #ClientFlag.CLIENT_SNDINPUT_VOICEACTIVATED.  @see
         * TeamTalkBase.SetVoiceActivationLevel() @see
         * TeamTalkBase.EnableVoiceActivation() */
        CLIENT_SNDINPUT_VOICEACTIVE = 0x00000010,
        /** @brief If set the client instance has muted all users.
        * @see TeamTalkBase.SetSoundOutputMute() */
        CLIENT_SNDOUTPUT_MUTE = 0x00000020,
        /** @brief If set the client instance will auto position users
        * in a 180 degree circle using 3D-sound. This option is only
        * available with #SoundSystem.SOUNDSYSTEM_DSOUND.
        * @see TeamTalkBase.SetUserPosition()
        * @see TeamTalkBase.Enable3DSoundPositioning() */
        CLIENT_SNDOUTPUT_AUTO3DPOSITION = 0x00000040,
        /** @brief If set the client instance's video device has been
         * initialized, i.e. TeamTalkBase.InitVideoCaptureDevice() has been
         * called successfuly. */
        CLIENT_VIDEOCAPTURE_READY = 0x00000080,
        /** @brief If set the client instance is currently transmitting
         * audio.  @see TeamTalkBase.EnableVoiceTransmission() */
        CLIENT_TX_VOICE = 0x00000100,
        /** @brief If set the client instance is currently transmitting video.
         * @see TeamTalkBase.StartVideoCaptureTransmission() */
        CLIENT_TX_VIDEOCAPTURE = 0x00000200,
        /** @brief If set the client instance is currently transmitting
         * a desktop window. A desktop window update is issued by calling
         * TeamTalkBase.SendDesktopWindow(). The event 
         * #ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER is triggered when a desktop
         * window transmission completes. */
        CLIENT_TX_DESKTOP = 0x00000400,
        /** @brief If set the client instance current have an active
         * desktop session, i.e. TeamTalkBase.SendDesktopWindow() has been
         * called. Call TeamTalkBase.CloseDesktopWindow() to close the desktop
         * session. */
        CLIENT_DESKTOP_ACTIVE = 0x00000800,
        /** @brief If set the client instance is currently muxing
         * audio streams into a single file. This is enabled by calling
         * TeamTalkBase.StartRecordingMuxedAudioFile(). */
        CLIENT_MUX_AUDIOFILE = 0x00001000,
        /** @brief If set the client instance is currently try to
         * connect to a server, i.e. TeamTalkBase.Connect() has been called. */
        CLIENT_CONNECTING = 0x00002000,
        /** @brief If set the client instance is connected to a server,
         * i.e. #ClientEvent.CLIENTEVENT_CON_SUCCESS event has been issued after
         * doing a TeamTalkBase.Connect(). Valid commands in this state:
         * TeamTalkBase.DoLogin() */
        CLIENT_CONNECTED = 0x00004000,
        /** @brief Helper for #ClientFlag.CLIENT_CONNECTING and #ClientFlag.CLIENT_CONNECTED
         * to see if TeamTalkBase.Disconnect() should be called. */
        CLIENT_CONNECTION = CLIENT_CONNECTING | CLIENT_CONNECTED,
        /** @brief If set the client instance is logged on to a
         * server, i.e. got #ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDIN event
         * after issueing TeamTalkBase.DoLogin(). */
        CLIENT_AUTHORIZED = 0x00008000,
        /** @brief If set the client is currently streaming the audio
         * of a media file. When streaming a video file the
         * #ClientFlag.CLIENT_STREAM_VIDEO flag is also typically set.
         * @see TeamTalkBase.StartStreamingMediaFileToChannel() */
        CLIENT_STREAM_AUDIO = 0x00010000,
        /** @brief If set the client is currently streaming the video
         * of a media file. When streaming a video file the
         * #ClientFlag.CLIENT_STREAM_AUDIO flag is also typically set.
         * @see TeamTalkBase.StartStreamingMediaFileToChannel() */
        CLIENT_STREAM_VIDEO = 0x00020000
    }

    /** @} */

    /** @ingroup initclient
     * 
     * @brief Abstract class which encapsulates the TeamTalk 5 client. Instantiate 
     * either #BearWare.TeamTalk5 (TeamTalk 5 SDK Standard Edition) or #BearWare.TeamTalk5Pro 
     * (TeamTalk 5 SDK Professional Edition) to create the TeamTalk instance. */
    public abstract class TeamTalkBase : IDisposable
    {
        /** @addtogroup initclient
         * @{ */

        /** @brief The maximum length of all strings used in
            TeamTalkBase. This value includes the zero terminator, so @b
            511 characters. Note that Unicode characters are converted
            to UTF-8 internally and limited once again to 512 if the
            string gets longer. */
        public const int TT_STRLEN = 512;

        /** @brief The highest user ID */
        public const int TT_USERID_MAX = 0xFFF;

        /** @brief The highest channel ID. Also used for
         * #TT_CLASSROOM_FREEFORALL */
        public const int TT_CHANNELID_MAX = 0xFFF;

        /** @ingroup videocapture
         * The maximum number of video formats which will be queried for a 
         * #BearWare.VideoCaptureDevice. */
        public const int TT_VIDEOFORMATS_MAX = 1024;

        /** @ingroup channels
         *
         * @brief The maximum number of users allowed to transmit when
         * a #BearWare.Channel is configured with
         * #ChannelType.CHANNEL_CLASSROOM. */
        public const int TT_TRANSMITUSERS_MAX = 128;

        /** @ingroup channels
         *
         * If a #BearWare.Channel is configured with #ChannelType
         * #ChannelType.CHANNEL_CLASSROOM then only users certain user IDs are
         * allowed to transmit. If, however, @c
         * TT_CLASSROOM_FREEFORALL is put in @c transmitUsers then
         * everyone in the channel are allowed to transmit. */
        public const int TT_CLASSROOM_FREEFORALL = 0xFFF;

        /** @ingroup channels
         * User ID index in @c transmitUsers of #BearWare.Channel */
        public const int TT_CLASSROOM_USERID_INDEX = 0;

        /** @ingroup channels
        * #StreamType index in @c transmitUsers of #BearWare.Channel */
        public const int TT_CLASSROOM_STREAMTYPE_INDEX = 1;

        /** @ingroup channels
         * Same as #TT_CLASSROOM_FREEFORALL */
        public const int TT_TRANSMITUSERS_FREEFORALL = 0xFFF;

        /** @ingroup channels
         * Same as #TT_CLASSROOM_USERID_INDEX */
        public const int TT_TRANSMITUSERS_USERID_INDEX = 0;

        /** @ingroup channels
        * Same as #TT_CLASSROOM_STREAMTYPE_INDEX */
        public const int TT_TRANSMITUSERS_STREAMTYPE_INDEX = 1;

        /** @ingroup users
         * The maximum number of channels where a user can automatically become
         * channel operator.
         * @see #BearWare.UserAccount */
        public const int TT_CHANNELS_OPERATOR_MAX = 16;

        /** @ingroup channels
         * The maximum number of users in the #BearWare.Channel transmit queue when channel
         * is configured with #BearWare.ChannelType.CHANNEL_SOLO_TRANSMIT */
        public const int TT_TRANSMITQUEUE_MAX = 16;

        /** @ingroup sounddevices
         * The maximum number of sample rates supported by a #BearWare.SoundDevice. */
        public const int TT_SAMPLERATES_MAX = 16;

        /** @ingroup desktopshare
         *
         * The maximum number #BearWare.DesktopInput instances which can be sent by
         * BearWare.TeamTalkBase.SendDesktopInput */
        public const int TT_DESKTOPINPUT_MAX = 16;

        /** @ingroup mediastream
         *
         * Specify this value as uOffsetMSec in #BearWare.MediaFilePlayback when
         * calling TT_InitLocalPlayback() and TeamTalkBase.UpdateLocalPlayback() to
         * ignore rewind or forward.
         */
        public const uint TT_MEDIAPLAYBACK_OFFSET_IGNORE = 0xFFFFFFFF;


        /** @brief Get the DLL's version number. */
        public static string GetVersion() { return Marshal.PtrToStringAuto(TTDLL.TT_GetVersion()); }

        /** 
         * @brief Create a new TeamTalk client instance.
         *
         * @param poll_based If the application using this class is a
         * Windows Forms application @a poll_based should be 'false'
         * since events will be posted using the Windows message
         * loop. In Console applications on the other hand, the user
         * application will have to 'poll' for events using
         * GetMessage(). Remember to put the @c [STAThread] macro on
         * the @c Main method if building a console application. */
        protected TeamTalkBase(bool poll_based)
        {
            Assembly assembly = Assembly.GetExecutingAssembly();
            AssemblyName name = assembly.GetName();
            Version dllversion = new Version(Marshal.PtrToStringUni((c_tt.TTDLL.TT_GetVersion())));
            if (!name.Version.Equals(dllversion))
            {
                string errmsg = String.Format("Invalid {2} version loaded. {2} is version {0} and {3} is version {1}",
                    dllversion.ToString(), name.Version.ToString(), c_tt.TTDLL.dllname, c_tt.TTDLL.mgtdllname);

                // throw new Exception(errmsg);

                System.Diagnostics.Debug.WriteLine(errmsg);
            }

            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__AUDIOCODEC) == Marshal.SizeOf(new AudioCodec()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__BANNEDUSER) == Marshal.SizeOf(new BannedUser()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__VIDEOFORMAT) == Marshal.SizeOf(new VideoFormat()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__OPUSCODEC) == Marshal.SizeOf(new OpusCodec()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__CHANNEL) == Marshal.SizeOf(new Channel()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__CLIENTSTATISTICS) == Marshal.SizeOf(new ClientStatistics()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__REMOTEFILE) == Marshal.SizeOf(new RemoteFile()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__FILETRANSFER) == Marshal.SizeOf(new FileTransfer()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__MEDIAFILESTATUS) == Marshal.SizeOf(Enum.GetUnderlyingType(typeof(MediaFileStatus))));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__SERVERPROPERTIES) == Marshal.SizeOf(new ServerProperties()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__SERVERSTATISTICS) == Marshal.SizeOf(new ServerStatistics()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__SOUNDDEVICE) == Marshal.SizeOf(new SoundDevice()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__SPEEXCODEC) == Marshal.SizeOf(new SpeexCodec()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__TEXTMESSAGE) == Marshal.SizeOf(new TextMessage()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__WEBMVP8CODEC) == Marshal.SizeOf(new WebMVP8Codec()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__TTMESSAGE) == Marshal.SizeOf(new TTMessage()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__USER) == Marshal.SizeOf(new User()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__USERACCOUNT) == Marshal.SizeOf(new UserAccount()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__USERSTATISTICS) == Marshal.SizeOf(new UserStatistics()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__VIDEOCAPTUREDEVICE) == Marshal.SizeOf(new VideoCaptureDevice()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__VIDEOCODEC) == Marshal.SizeOf(new VideoCodec()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__AUDIOCONFIG) == Marshal.SizeOf(new AudioConfig()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__SPEEXVBRCODEC) == Marshal.SizeOf(new SpeexVBRCodec()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__VIDEOFRAME) == Marshal.SizeOf(new VideoFrame()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__AUDIOBLOCK) == Marshal.SizeOf(new AudioBlock()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__AUDIOFORMAT) == Marshal.SizeOf(new AudioFormat()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__MEDIAFILEINFO) == Marshal.SizeOf(new MediaFileInfo()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__CLIENTERRORMSG) == Marshal.SizeOf(new ClientErrorMsg()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__DESKTOPINPUT) == Marshal.SizeOf(new DesktopInput()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__SPEEXDSP) == Marshal.SizeOf(new SpeexDSP()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__STREAMTYPE) == Marshal.SizeOf(Enum.GetUnderlyingType(typeof(StreamType))));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__AUDIOPREPROCESSORTYPE) == Marshal.SizeOf(Enum.GetUnderlyingType(typeof(AudioPreprocessorType))));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__AUDIOPREPROCESSOR) == Marshal.SizeOf(new AudioPreprocessor()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__TTAUDIOPREPROCESSOR) == Marshal.SizeOf(new TTAudioPreprocessor()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__MEDIAFILEPLAYBACK) == Marshal.SizeOf(new MediaFilePlayback()));
            Debug.Assert(TTDLL.TT_DBG_SIZEOF(TTType.__CLIENTKEEPALIVE) == Marshal.SizeOf(new ClientKeepAlive()));

            if (poll_based)
                m_ttInst = TTDLL.TT_InitTeamTalkPoll();
            else
            {
                m_eventhandler = new MyEventHandler(this);
#if PocketPC
                IntPtr hWnd = m_eventhandler.Hwnd;
#else
                m_eventhandler.CreateControl();
                IntPtr hWnd = m_eventhandler.Handle;
#endif
                m_ttInst = TTDLL.TT_InitTeamTalk(hWnd, MyEventHandler.WM_TEAMTALK_CLIENTEVENT);
            }
        }

        /** 
         * @brief Close the TeamTalk client instance and release its
         * resources.
         */
        ~TeamTalkBase()
        {
            DeleteMe();
        }


        #region IDisposable Members

        public void Dispose()
        {
            DeleteMe();
        }

        private void DeleteMe()
        {
            if (m_ttInst != IntPtr.Zero)
            {
                TTDLL.TT_CloseTeamTalk(m_ttInst);
                m_ttInst = IntPtr.Zero;
            }
        }

        #endregion

        /**
         * @brief Poll for events in the client instance.
         * 
         * In a Windows Forms application events can be processed using
         * the application message-loop, but in Console application the
         * application must process the events itself. The most common
         * way to do this is to start a timer which calls TeamTalkBase.GetMessage()
         * every now and then to ensure the UI is updated with the latest
         * events.
         *
         * @param pMsg Reference to a TTMessage instance which will hold the 
         * event that has occured.
         * @param nWaitMs The amount of time to wait for the event. If -1 the 
         * function will block forever or until the next event occurs.
         * @return Returns TRUE if an event has occured otherwise FALSE.
         * @see TeamTalk()
         * @see ClientEvent */
        public bool GetMessage(ref TTMessage pMsg, int nWaitMs)
        {
            return TTDLL.TT_GetMessage(m_ttInst, ref pMsg, ref nWaitMs);
        }

        /**
         * @brief Cause client instance event thread to schedule an update
         * event.
         *
         * Normally all events are due to a state change in the client
         * instance. The state change (#ClientEvent) is submitted by the
         * client instance's internal thread. In some cases it's, however,
         * convenient to make the internal thread submit the latest
         * properties of an object. One example is after having changed
         * the volume of a #BearWare.User. Then your local copy of #BearWare.User will no
         * longer contain the latest @c nVolumeVoice.
         *
         * Calling PumpMessage() will make the client instance's
         * internal thread queue an update of #BearWare.User so the latest
         * properties of the user can be retrieved from GetMessage().
         *
         * It's also possible to simply use GetUser() but the problem
         * with this approach is that this call is from a separate thread
         * and therefore doesn't take the event queue into account.
         * 
         * @param nClientEvent The event which should be queued. Currently 
         * only #ClientEvent.CLIENTEVENT_USER_STATECHANGE is supported.
         * @param nIdentifier The ID of the object to retrieve. Currently
         * only nUserID is supported.
         *
         * @return Returns true if the event has been scheduled. */
        public bool PumpMessage(ClientEvent nClientEvent,
                                int nIdentifier)
        {
            return TTDLL.TT_PumpMessage(m_ttInst, nClientEvent, nIdentifier);
        }

        /**
         * @brief Get a bitmask describing the client's current state.
         *
         * Checks whether the client is connecting, connected, authorized,
         * etc. The current state can be checked by and'ing the returned
         * bitmask which is based on #ClientFlag.
         *
         * @return A bitmask describing the current state. */
        public BearWare.ClientFlag GetFlags()
        {
            return TTDLL.TT_GetFlags(m_ttInst);
        }

        /**
         * @brief Same as GetFlags().
         */
        public ClientFlag Flags
        {
            get { return GetFlags(); }
        }

        /**
         * @brief Set license information to disable trial mode.
         *
         * This function must be called before instantiating either
         * the #BearWare.TeamTalk5 or #BearWare.TeamTalk5Pro-class.
         *
         * @param szRegName The registration name provided by BearWare.dk.
         * @param szRegKey The registration key provided by BearWare.dk.
         * @return True if the provided registration is acceptable. */
        public static bool SetLicenseInformation(string szRegName, string szRegKey)
        {
            return TTDLL.TT_SetLicenseInformation(szRegName, szRegKey);
        }

        /**
         * @brief Event handler for #BearWare.TTMessage.
         * 
         * @param msg The #BearWare.TTMessage retrieved by TeamTalkBase.GetMessage() */
        public void ProcessMsg(TTMessage msg)
        {
            switch (msg.nClientEvent)
            {
                case ClientEvent.CLIENTEVENT_CON_SUCCESS:
                    if(OnConnectionSuccess != null)
                        OnConnectionSuccess();
                    break;
                case ClientEvent.CLIENTEVENT_CON_FAILED:
                    if (OnConnectionFailed != null)
                    OnConnectionFailed();
                    break;
                case ClientEvent.CLIENTEVENT_CON_LOST:
                    if (OnConnectionLost != null)
                        OnConnectionLost();
                    break;
                case ClientEvent.CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED :
                    if (OnConnectionMaxPayloadUpdated != null)
                        OnConnectionMaxPayloadUpdated((int)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_PROCESSING:
                    if (OnCmdProcessing != null)
                        OnCmdProcessing(msg.nSource, (bool)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_ERROR:
                    if (OnCmdError != null)
                        OnCmdError((int)msg.nSource, (ClientErrorMsg)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_SUCCESS :
                    if (OnCmdSuccess != null)
                        OnCmdSuccess((int)msg.nSource);
                    break;
                case ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDIN:
                    if (OnCmdMyselfLoggedIn != null)
                        OnCmdMyselfLoggedIn((int)msg.nSource, (UserAccount)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDOUT:
                    if (OnCmdMyselfLoggedOut != null)
                        OnCmdMyselfLoggedOut();
                    break;
                case ClientEvent.CLIENTEVENT_CMD_MYSELF_KICKED:
                    if (msg.ttType == TTType.__USER)
                    {
                        if (OnCmdMyselfKicked != null)
                            OnCmdMyselfKicked((User)msg.DataToObject());
                    }
                    else if (OnCmdMyselfKicked != null)
                        OnCmdMyselfKicked(new User());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_USER_LOGGEDIN:
                    if (OnCmdUserLoggedIn != null)
                        OnCmdUserLoggedIn((User)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_USER_LOGGEDOUT:
                    if (OnCmdUserLoggedOut != null)
                        OnCmdUserLoggedOut((User)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_USER_UPDATE:
                    if (OnCmdUserUpdate != null)
                        OnCmdUserUpdate((User)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_USER_JOINED:
                    if (OnCmdUserJoinedChannel != null)
                        OnCmdUserJoinedChannel((User)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_USER_LEFT:
                    if (OnCmdUserLeftChannel != null)
                        OnCmdUserLeftChannel((User)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_USER_TEXTMSG:
                    if (OnCmdUserTextMessage != null)
                        OnCmdUserTextMessage((TextMessage)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW:
                    if (OnCmdChannelNew != null)
                        OnCmdChannelNew((Channel)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE:
                    if (OnCmdChannelUpdate != null)
                        OnCmdChannelUpdate((Channel)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_CHANNEL_REMOVE:
                    if (OnCmdChannelRemove != null)
                        OnCmdChannelRemove((Channel)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_SERVER_UPDATE:
                    if (OnCmdServerUpdate != null)
                        OnCmdServerUpdate((ServerProperties)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_SERVERSTATISTICS :
                    if (OnCmdServerStatistics != null)
                        OnCmdServerStatistics((ServerStatistics)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_FILE_NEW:
                    if (OnCmdFileNew != null)
                        OnCmdFileNew((RemoteFile)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_FILE_REMOVE:
                    if (OnCmdFileRemove != null)
                        OnCmdFileRemove((RemoteFile)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_USERACCOUNT :
                    if (OnCmdUserAccount != null)
                        OnCmdUserAccount((UserAccount)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_CMD_BANNEDUSER :
                    if (OnCmdBannedUser != null)
                        OnCmdBannedUser((BannedUser)msg.DataToObject());
                    break;
                
                case ClientEvent.CLIENTEVENT_USER_STATECHANGE :
                    if (OnUserStateChange != null)
                        OnUserStateChange((User)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_USER_VIDEOCAPTURE:
                    if (OnUserVideoCapture != null)
                        OnUserVideoCapture(msg.nSource, (int)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_USER_MEDIAFILE_VIDEO:
                    if (OnUserMediaFileVideo != null)
                        OnUserMediaFileVideo((int)msg.nSource, (int)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW:
                    if (OnUserDesktopWindow != null)
                        OnUserDesktopWindow((int)msg.nSource, (int)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_USER_DESKTOPCURSOR:
                    if (OnUserDesktopCursor != null)
                        OnUserDesktopCursor((int)msg.nSource, (DesktopInput)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_USER_DESKTOPINPUT :
                    if (OnUserDesktopInput != null)
                        OnUserDesktopInput((int)msg.nSource, (DesktopInput)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE :
                    if(OnUserRecordMediaFile != null)
                        OnUserRecordMediaFile((int)msg.nSource, (MediaFileInfo)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK :
                    if(OnUserAudioBlock != null)
                        OnUserAudioBlock((int)msg.nSource, (StreamType)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_INTERNAL_ERROR :
                    if(OnInternalError!= null)
                        OnInternalError((ClientErrorMsg)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_VOICE_ACTIVATION :
                    if(OnVoiceActivation != null)
                        OnVoiceActivation((bool)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_HOTKEY :
                    if(OnHotKeyToggle != null)
                        OnHotKeyToggle(msg.nSource, (bool)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_HOTKEY_TEST :
                    if(OnHotKeyTest != null)
                        OnHotKeyTest(msg.nSource, (bool)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_FILETRANSFER :
                    if(OnFileTransfer != null)
                        OnFileTransfer((FileTransfer)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER :
                    if(OnDesktopWindowTransfer != null)
                        OnDesktopWindowTransfer(msg.nSource, (int)msg.DataToObject());
                    break;
                case ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE :
                    if(OnStreamMediaFile != null)
                        OnStreamMediaFile((MediaFileInfo)msg.DataToObject());
                    break;
            }
        }

        /** @} */

        /** @addtogroup sounddevices
         * @{ */

        /**
         * @brief Get the default sound devices. 
         *
         * @param lpnInputDeviceID The ID of the default input device.
         * @param lpnOutputDeviceID The ID of the default output device.
         * @see TeamTalkBase.InitSoundInputDevice
         * @see TeamTalkBase.InitSoundOutputDevice */
        public static bool GetDefaultSoundDevices(ref int lpnInputDeviceID,
                                                  ref int lpnOutputDeviceID)
        {
            return TTDLL.TT_GetDefaultSoundDevices(ref lpnInputDeviceID, ref lpnOutputDeviceID);
        }
        /**
         * @brief Get the default sound devices for the specified sound system.
         *
         * @see TeamTalkBase.GetDefaultSoundDevices() */
        public static bool GetDefaultSoundDevicesEx(SoundSystem nSndSystem,
                                                    ref int lpnInputDeviceID,
                                                    ref int lpnOutputDeviceID)
        {
            return TTDLL.TT_GetDefaultSoundDevicesEx(nSndSystem, ref lpnInputDeviceID, ref lpnOutputDeviceID);
        }
        /**
         * @brief Retrieve list of sound devices for recording and playback.
         *
         * @param lpSoundDevices An output array of #BearWare.SoundDevice-structs which
         * will receive the available sound devices.
         * @see TeamTalkBase.GetDefaultSoundDevices
         * @see TeamTalkBase.InitSoundInputDevice
         * @see TeamTalkBase.InitSoundOutputDevice */
        public static bool GetSoundDevices(out SoundDevice[] lpSoundDevices)
        {
            int count = 0;
            bool b = TTDLL.TT_GetSoundDevices_NULL(IntPtr.Zero, ref count);
            SoundDevice[] devs = new SoundDevice[count];
            b = TTDLL.TT_GetSoundDevices(devs, ref count);
            lpSoundDevices = b ? devs : null;
            return b;
        }
        /**
         * @brief Reinitialize sound system (in order to detect
         * new/removed devices).
         *
         * When the TeamTalk client is first initialized all the sound
         * devices are detected and stored in a list inside the client
         * instance. If a user adds or removes e.g. a USB sound device
         * then it's not picked up automatically by the client
         * instance. TeamTalkBase.RestartSoundSystem() can be used to reinitialize
         * the sound system and thereby detect if sound devices have been
         * removed or added.
         *
         * In order to restart the sound system all sound devices in all
         * client instances must be closed using TeamTalkBase.CloseSoundInputDevice(),
         * TeamTalkBase.CloseSoundoutputDevice() and TeamTalkBase.CloseSoundDuplexDevices(). */
        public static bool RestartSoundSystem()
        {
            return TTDLL.TT_RestartSoundSystem();
        }
        /**
         * @brief Perform a record and playback test of specified sound
         * devices along with an audio configuration and ability to try
         * echo cancellation.
         *
         * This function allows the use of #BearWare.SpeexDSP to enable AGC and echo
         * cancellation.
         * 
         * @param nInputDeviceID Should be the @a nDeviceID extracted through 
         * TeamTalkBase.GetSoundDevices().
         * @param nOutputDeviceID Should be the @a nDeviceID extracted through 
         * TeamTalkBase.GetSoundDevices().
         * @param nSampleRate The sample rate the client's recorder should 
         * use.
         * @param nChannels Number of channels to use, i.e. 1 = mono, 2 = stereo.
         * @param bDuplexMode Both input and output devices MUST support
         * the specified sample rate since this loop back test uses duplex
         * mode ( @see TeamTalkBase.InitSoundDuplexDevices() ). Check out @c
         * supportedSampleRates of #BearWare.SoundDevice to see which sample rates
         * are supported.
         * @param lpSpeexDSP The preprocessing settings to use, i.e. AGC 
         * and denoising properties.
         * @return Returns IntPtr.Zero in case of error, otherwise sound loop instance
         * which can be closed by TeamTalkBase.CloseSoundLoopbackTest();
         * @see TeamTalkBase.InitSoundInputDevice()
         * @see TeamTalkBase.InitSoundOutputDevice()
         * @see TeamTalkBase.InitSoundDuplexDevices()
         * @see TeamTalkBase.StopSoundLoopbackTest() */
        public static IntPtr StartSoundLoopbackTest(int nInputDeviceID, int nOutputDeviceID,
                                                    int nSampleRate, int nChannels,
                                                    bool bDuplexMode, SpeexDSP lpSpeexDSP)
        {
            return TTDLL.TT_StartSoundLoopbackTest(nInputDeviceID, nOutputDeviceID,
                                                 nSampleRate, nChannels, bDuplexMode,
                                                 ref lpSpeexDSP);
        }
        /**
         * @brief Stop recorder and playback test.
         *
         * @see TeamTalkBase.InitSoundInputDevice
         * @see TeamTalkBase.InitSoundOutputDevice
         * @see TeamTalkBase.StartSoundLoopbackTest */
        public static bool CloseSoundLoopbackTest(IntPtr lpTTSoundLoop)
        {
            return TTDLL.TT_CloseSoundLoopbackTest(lpTTSoundLoop);
        }
        /**
         * @brief Initialize the sound input devices (for recording audio).
         *
         * The @a nDeviceID of the #BearWare.SoundDevice should be used as @a 
         * nInputDeviceID.
         * 
         * Calling this function will set the flag #ClientFlag.CLIENT_SNDINPUT_READY.
         *
         * @param nInputDeviceID The @a nDeviceID of #BearWare.SoundDevice extracted 
         * through GetSoundDevices().
         * @see BearWare.SoundDevice
         * @see GetDefaultSoundDevices
         * @see GetSoundDevices
         * @see CloseSoundInputDevice
         * @see GetSoundInputLevel */
        public bool InitSoundInputDevice(int nInputDeviceID)
        {
            return TTDLL.TT_InitSoundInputDevice(m_ttInst, nInputDeviceID);
        }

        /** 
         * @brief Initialize the sound output devices (for sound playback).
         *
         * The @a nDeviceID of the #BearWare.SoundDevice should be used as @a 
         * nOutputDeviceID.
         *
         * Callling this function will set the flag
         * #ClientFlag.CLIENT_SNDOUTPUT_READY.
         *
         * @param nOutputDeviceID Should be the @a nDeviceID of 
         * #BearWare.SoundDevice extracted through GetSoundDevices().
         * @see BearWare.SoundDevice
         * @see TeamTalkBase.GetDefaultSoundDevices
         * @see TeamTalkBase.GetSoundOutputDevices
         * @see TeamTalkBase.CloseSoundOutputDevice */
        public bool InitSoundOutputDevice(int nOutputDeviceID)
        {
            return TTDLL.TT_InitSoundOutputDevice(m_ttInst, nOutputDeviceID);
        }
        /**
         * @brief Enable duplex mode where multiple audio streams are
         * mixed into a single stream using software.
         *
         * Duplex mode can @b ONLY be enabled on sound devices which
         * support the same sample rate. Sound systems #SoundSystem.SOUNDSYSTEM_WASAPI
         * and #SoundSystem.SOUNDSYSTEM_ALSA typically only support a single sample
         * rate.  Check @c supportedSampleRates in #BearWare.SoundDevice to see
         * which sample rates are supported.
         *
         * Sound duplex mode is required for echo cancellation since sound
         * input and output device must be synchronized. Also sound cards
         * which does not support multiple output streams should use
         * duplex mode.
         *
         * If TeamTalkBase.InitSoundDuplexDevices() is successful the following
         * flags will be set:
         *
         * - #ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX
         * - #ClientFlag.CLIENT_SNDOUTPUT_READY
         * - #ClientFlag.CLIENT_SNDINPUT_READY
         *
         * Sound duplex mode is required for echo cancellation since sound
         * input and output device must be synchronized. Also sound cards
         * which does not support multiple output stream should use
         * duplex mode.
         *
         * Call TeamTalkBase.CloseSoundDuplexDevices() to shut down duplex mode.
         *
         * Note that it is only the audio streams from users in the local
         * client instance's current channel which will be mixed. If the
         * local client instance calls TeamTalkBase.DoSubscribe() with
         * #Subscription #Subscription.SUBSCRIBE_INTERCEPT_VOICE on a user in another channel then
         * the audio from this user will be started in a separate
         * stream. The reason for this is that the other user may use a
         * different audio codec.
         *
         * @param nInputDeviceID The @a nDeviceID of #BearWare.SoundDevice extracted 
         * through GetSoundDevices().
         * @param nOutputDeviceID The @a nDeviceID of #BearWare.SoundDevice extracted
         * through GetSoundDevices().
         * @see TeamTalkBase.InitSoundInputDevice()
         * @see TeamTalkBase.InitSoundOutputDevice()
         * @see TeamTalkBase.EnableEchoCancellation()
         * @see TeamTalkBase.CloseSoundDuplexDevices() */
        public bool InitSoundDuplexDevices(int nInputDeviceID, int nOutputDeviceID)
        {
            return TTDLL.TT_InitSoundDuplexDevices(m_ttInst, nInputDeviceID, nOutputDeviceID);
        }
        /**
         * @brief Shutdown the input sound device.
         *
         * Callling this function will clear the flag
         * #ClientFlag.CLIENT_SNDINPUT_READY.
         * 
         * If the local client instance is running in duplex mode
         * (flag #ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX is set) then
         * trying to close the sound device will fail since duplex
         * mode require that both input and output sound devices are
         * active at the same time. Therefore in order to close sound
         * devices running in duplex mode call
         * TeamTalkBase.CloseSoundDuplexDevices().
         * 
         * @return If running in sound duplex mode (#ClientFlag
         * #ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX) then ensure to disable duplex
         * mode prior to closing the sound device.
         * 
         * @see TeamTalkBase.InitSoundInputDevice */
        public bool CloseSoundInputDevice()
        {
            return TTDLL.TT_CloseSoundInputDevice(m_ttInst);
        }
        /**
         * @brief Shutdown the output sound device.
         *
         * Callling this function will clear set the flag
         * #ClientFlag.CLIENT_SNDOUTPUT_READY.
         *
         * If the local client instance is running in duplex mode (flag
         * #ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX is set) then trying to close the
         * sound device will fail since duplex mode require that both
         * input and output sound devices are active at the same
         * time. Therefore in order to close sound devices running in
         * duplex mode call TeamTalkBase.CloseSoundDuplexDevices().
         *
         * @return If running in sound duplex mode (#ClientFlag
         * #ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX) then ensure to disable duplex
         * mode prior to closing the sound device.
         * 
         * @see TeamTalkBase.InitSoundOutputDevice */
        public bool CloseSoundOutputDevice()
        {
            return TTDLL.TT_CloseSoundOutputDevice(m_ttInst);
        }
        /**
         * @brief Shut down sound devices running in duplex mode.
         *
         * Calling this function only applies if sound devices has been
         * initialized with TeamTalkBase.InitSoundDuplexDevices().
         */
        public bool CloseSoundDuplexDevices()
        {
            return TTDLL.TT_CloseSoundDuplexDevices(m_ttInst);
        }
        /**
         * @brief Get the volume level of the current recorded audio.
         *
         * The current level is updated at an interval specified in a channel's
         * #BearWare.AudioCodec.
         *
         * Note that the volume level will not be available until the
         * client instance joins a channel, i.e. it knows what sample rate
         * to use.
         *
         * @return Returns a value between
         * #BearWare.SoundLevel.SOUND_VU_MIN and
         * #BearWare.SoundLevel.SOUND_VU_MAX */
        public int GetSoundInputLevel()
        {
            return TTDLL.TT_GetSoundInputLevel(m_ttInst);
        }
        /** 
         * @brief Set voice gaining of recorded audio. 
         *
         * The gain level ranges from #BearWare.SoundLevel.SOUND_GAIN_MIN to #BearWare.SoundLevel.SOUND_GAIN_MAX
         * where #BearWare.SoundLevel.SOUND_GAIN_DEFAULT is no gain. So 100 is 1/10 of the
         * original volume and 8000 is 8 times the original volume.
         *
         * Note that using TeamTalkBase.SetSoundInputPreprocess() will override
         * settings an input gain level. This is because automatic gain
         * control will adjust the volume level.
         *
         * @param nLevel A value from
         * #BearWare.SoundLevel.SOUND_GAIN_MIN to
         * #BearWare.SoundLevel.SOUND_GAIN_MAX.
         * @see TeamTalkBase.GetSoundInputGainLevel */
        public bool SetSoundInputGainLevel(int nLevel)
        {
            return TTDLL.TT_SetSoundInputGainLevel(m_ttInst, nLevel);
        }
        /**
         * @brief Get voice gain level of outgoing audio
         *
         * @return A value from #BearWare.SoundLevel.SOUND_GAIN_MIN to
         * #BearWare.SoundLevel.SOUND_GAIN_MAX.
         * @see SetSoundInputGainLevel */
        public int GetSoundInputGainLevel()
        {
            return TTDLL.TT_GetSoundInputGainLevel(m_ttInst);
        }

        /**
         * @brief Enable sound preprocessor which should be used for
         * processing audio recorded by the sound input device (voice input).
         *
         * To ensure common settings for all users in a channel it's
         * possible to use the @c audiocfg member of #BearWare.Channel as shared
         * source for audio settings.
         *
         * In order for echo cancellation to work best it's important to
         * also enable AGC in the #BearWare.SpeexDSP.
         *
         * @param lpSpeexDSP The sound preprocessor settings to use. 
         * Preferably from the #BearWare.Channel's @c audiocfg member to ensure common
         * settings for all users.
         * @return TRUE on success, FALSE on failure. */
        public bool SetSoundInputPreprocess(SpeexDSP lpSpeexDSP)
        {
            return TTDLL.TT_SetSoundInputPreprocess(m_ttInst, ref lpSpeexDSP);
        }

        /** 
         * @brief Get the sound preprocessor settings which are currently in use
         * for recorded sound input device (voice input).
         *
         * @param lpSpeexDSP A preallocated SpeexDSP which will 
         * receive the settings that is currently in effect.
         *
         * @return TRUE on success, FALSE on failure. */
        public bool GetSoundInputPreprocess(ref SpeexDSP lpSpeexDSP)
        {
            return TTDLL.TT_GetSoundInputPreprocess(m_ttInst, ref lpSpeexDSP);
        }

        /**
         * @brief Set master volume. 
         *
         * If still not loud enough use SetUserVolume().
         *
         * @param nVolume A value from #BearWare.SoundLevel.SOUND_VOLUME_MIN to  #BearWare.SoundLevel.SOUND_VOLUME_MAX.
         * @see TeamTalkBase.SetUserVolume */
        public bool SetSoundOutputVolume(int nVolume)
        {
            return TTDLL.TT_SetSoundOutputVolume(m_ttInst, nVolume);
        }
        /**
         * @brief Get master volume.
         *
         * @return Returns the master volume.
         * @see BearWare.SoundLevel.SOUND_VOLUME_MAX
         * @see BearWare.SoundLevel.SOUND_VOLUME_MIN */
        public int GetSoundOutputVolume()
        {
            return TTDLL.TT_GetSoundOutputVolume(m_ttInst);
        }
        /**
         * @brief Set all users mute.
         *
         * To stop receiving audio from a user call DoUnsubscribe().
         *
         * @param bMuteAll Whether to mute or unmute all users.
         * @see ClientFlag #ClientFlag.CLIENT_SNDOUTPUT_MUTE */
        public bool SetSoundOutputMute(bool bMuteAll)
        {
            return TTDLL.TT_SetSoundOutputMute(m_ttInst, bMuteAll);
        }
        /** 
         * @brief Enable automatically position users using 3D-sound.
         *
         * Note that 3d-sound does not work if sound is running in duplex
         * mode (#ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX).
         *
         * @param bEnable TRUE to enable, otherwise FALSE.
         * @see TeamTalkBase.SetUserPosition */
        public bool Enable3DSoundPositioning(bool bEnable)
        {
            return TTDLL.TT_Enable3DSoundPositioning(m_ttInst, bEnable);
        }
        /** 
         * @brief Automatically position users using 3D-sound.
         *
         * Note that 3d-sound does not work if sound is running in duplex
         * mode (#ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX).
         *
         * @see TeamTalkBase.SetUserPosition */
        public bool AutoPositionUsers()
        {
            return TTDLL.TT_AutoPositionUsers(m_ttInst);
        }

        /**
         * @brief Enable/disable access to user's raw audio.
         *
         * With audio callbacks enabled all audio which has been played
         * will be accessible by calling TeamTalkBase.AcquireUserAudioBlock(). Every
         * time a new #BearWare.AudioBlock is available the event
         * OnUserAudioBlock() is generated.
         * 
         * @param nUserID The user ID to monitor for audio callback. Pass 0
         * to monitor local audio.
         * @param nStreamType Either #StreamType.STREAMTYPE_VOICE or 
         * #StreamType.STREAMTYPE_MEDIAFILE_AUDIO.
         * @param bEnable Whether to enable the #ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK event.
         * 
         * @see TeamTalkBase.AcquireUserAudioBlock()
         * @see TeamTalkBase.ReleaseUserAudioBlock()
         * @see OnUserAudioBlock() */
        public bool EnableAudioBlockEvent(int nUserID, StreamType nStreamType,
                                          bool bEnable)
        {
            return TTDLL.TT_EnableAudioBlockEvent(m_ttInst, nUserID, nStreamType, bEnable);
        }
        /** @} */

        /** @addtogroup transmission
         * @{ */

        /**
         * @brief Start/stop transmitting of voice data from sound input.
         *
         * Sound input is initialized using TeamTalkBase.InitSoundInputDevice() or
         * TeamTalkBase.InitSoundDuplexDevices().
         *
         * Voice transmission is stream type #StreamType.STREAMTYPE_VOICE.
         *
         * User rights required:
         * - #UserRight.USERRIGHT_TRANSMIT_VOICE
         *
         *
         * @param bEnable Enable/disable transmission. */
        public bool EnableVoiceTransmission(bool bEnable)
        {
            return TTDLL.TT_EnableVoiceTransmission(m_ttInst, bEnable);
        }

        /**
         * @brief Enable voice activation.
         *
         * The client instance will start transmitting audio if the recorded audio
         * level is above or equal to the voice activation level set by
         * #SetVoiceActivationLevel. Once the voice activation level is reached
         * the event #OnVoiceActivation is posted.
         *
         * The current volume level can be queried calling
         * #GetSoundInputLevel.
         *
         * Voice transmission is stream type #StreamType.STREAMTYPE_VOICE.
         *
         * User rights required:
         * - #UserRight.USERRIGHT_TRANSMIT_VOICE
         *
         * @param bEnable TRUE to enable, otherwise FALSE.
         * @see ClientFlag #ClientFlag.CLIENT_SNDINPUT_VOICEACTIVATED */
        public bool EnableVoiceActivation(bool bEnable)
        {
            return TTDLL.TT_EnableVoiceActivation(m_ttInst, bEnable);
        }
        /** 
         * @brief Set voice activation level.
         *
         * The current volume level can be queried calling
         * GetSoundInputLevel().
         *
         * @param nLevel Must be between #BearWare.SoundLevel.SOUND_VU_MIN and #BearWare.SoundLevel.SOUND_VU_MAX
         * @see TeamTalkBase.EnableVoiceActivation
         * @see TeamTalkBase.GetVoiceActivationLevel */
        public bool SetVoiceActivationLevel(int nLevel)
        {
            return TTDLL.TT_SetVoiceActivationLevel(m_ttInst, nLevel);
        }
        /** 
         * @brief Get voice activation level.
         *
         * @return Returns A value between #BearWare.SoundLevel.SOUND_VU_MIN and #BearWare.SoundLevel.SOUND_VU_MAX
         * @see TeamTalkBase.EnableVoiceActivation
         * @see TeamTalkBase.SetVoiceActivationLevel */
        public int GetVoiceActivationLevel()
        {
            return TTDLL.TT_GetVoiceActivationLevel(m_ttInst);
        }
        /**
         * @brief Set the delay of when voice activation should be stopped.
         *
         * When TeamTalkBase.GetSoundInputLevel() becomes higher than the specified
         * voice activation level the client instance will start
         * transmitting until TeamTalkBase.GetSoundInputLevel() becomes lower than
         * the voice activation level, plus a delay. This delay is by
         * default set to 1500 msec but this value can be changed by
         * calling TeamTalkBase.SetVoiceActivationStopDelay().
         *
         * @see EnableVoiceActivation
         * @see SetVoiceActivationLevel */
        public bool SetVoiceActivationStopDelay(int nDelayMSec)
        {
            return TTDLL.TT_SetVoiceActivationStopDelay(m_ttInst, nDelayMSec);
        }

        /**
         * @brief Get the delay of when voice active state should be disabled.
         *
         * @return The number of miliseconds before voice activated state
         * should be turned back to inactive.
         *
         * @see SetVoiceActivationStopDelay
         * @see EnableVoiceActivation
         * @see SetVoiceActivationLevel */
        public int GetVoiceActivationStopDelay()
        {
            return TTDLL.TT_GetVoiceActivationStopDelay(m_ttInst);
        }

        /**
         * @brief Store audio conversations to a single file.
         *
         * Unlike TeamTalkBase.SetUserMediaStorageDir(), which stores users' audio
         * streams in separate files, TeamTalkBase.StartRecordingMuxedAudioFile()
         * muxes the audio streams into a single file.
         *
         * The audio streams, which should be muxed together, are
         * required to use the same audio codec. In most cases this is
         * the audio codec of the channel where the user is currently
         * participating (i.e. @c audiocodec member of #BearWare.Channel).
         *
         * If the user changes to a channel which uses a different audio
         * codec then the recording will continue but simply be silent
         * until the user again joins a channel with the same audio codec
         * as was used for initializing muxed audio recording.
         *
         * Calling TeamTalkBase.StartRecordingMuxedAudioFile() will enable the
         * #ClientFlag.CLIENT_MUX_AUDIOFILE flag from TeamTalkBase.GetFlags().
         *
         * Call TeamTalkBase.StopRecordingMuxedAudioFile() to stop recording. Note
         * that only one muxed audio recording can be active at the same
         * time.
         *
         * @param lpAudioCodec The audio codec which should be used as
         * reference for muxing users' audio streams. In most situations
         * this is the #BearWare.AudioCodec of the current channel, i.e.
         * TeamTalkBase.GetMyChannelID().
         * @param szAudioFileName The file to store audio to, e.g. 
         * C:\\MyFiles\\Conf.mp3.
         * @param uAFF The audio format which should be used in the recorded
         * file. The muxer will convert to this format.
         *
         * @see SetUserMediaStorageDir()
         * @see StopRecordingMuxedAudioFile() */
        public bool StartRecordingMuxedAudioFile(AudioCodec lpAudioCodec,
                                                 string szAudioFileName,
                                                 AudioFileFormat uAFF)
        {
            return TTDLL.TT_StartRecordingMuxedAudioFile(m_ttInst,
                                                       ref lpAudioCodec,
                                                       szAudioFileName,
                                                       uAFF);
        }

        /**
         * @brief Stop an active muxed audio recording.
         *
         * A muxed audio recording started with
         * TeamTalkBase.StartRecordingMuxedAudioFile() can be stopped using this
         * function.
         *
         * Calling TeamTalkBase.StopRecordingMuxedAudioFile() will clear the
         * #ClientFlag.CLIENT_MUX_AUDIOFILE flag from TeamTalkBase.GetFlags().
         *
         * @see StartRecordingMuxedAudioFile() */
        public bool StopRecordingMuxedAudioFile()
        {
            return TTDLL.TT_StopRecordingMuxedAudioFile(m_ttInst);
        }

        /**
         * @brief Start transmitting from video capture device.
         *
         * The video capture device is initiated by calling
         * TeamTalkBase.InitVideoCaptureDevice(). After joining a channel and
         * calling this function the other users will see the video from
         * the capture device.
         *
         * Video capture data is transmitted with stream type
         * #StreamType.STREAMTYPE_VIDEOCAPTURE and is subscribed/unsubscribed using
         * #Subscription.SUBSCRIBE_VIDEOCAPTURE.
         *
         * To stop transmitting call TeamTalkBase.StopVideoCaptureTransmission()
         *
         * User rights required:
         * - #UserRight.USERRIGHT_TRANSMIT_VIDEOCAPTURE.
         *
         * @param lpVideoCodec The video codec settings to use for
         * transmission.
         *
         * @see TeamTalkBase.StartStreamingMediaFileToChannel()
         * @see TeamTalkBase.EnableVoiceTransmission() */
        public bool StartVideoCaptureTransmission(VideoCodec lpVideoCodec)
        {
            return TTDLL.TT_StartVideoCaptureTransmission(m_ttInst, ref lpVideoCodec);
        }

        /**
         * @brief Stop transmitting from video capture device.
         *
         * @see TeamTalkBase.StartVideoCaptureTransmission() */
        public bool StopVideoCaptureTransmission()
        {
            return TTDLL.TT_StopVideoCaptureTransmission(m_ttInst);
        }

        /** @} */

        /** @addtogroup videocapture
         * @{ */

        /**
         * @brief Get the list of devices available for video capture.
         *
         * @param lpVideoDevices An output array of #BearWare.VideoCaptureDevice-stucts 
         * which will receive the available video capture devices
         * @see TeamTalkBase.InitVideoCaptureDevice */
        public static bool GetVideoCaptureDevices(out VideoCaptureDevice[] lpVideoDevices)
        {
            //To speed up query we only query for a max of 25.
            //Hopefully no one has more than 25 capture devices. 
            VideoCaptureDevice[] devs = new VideoCaptureDevice[25];
            int count = devs.Length;
            bool b = TTDLL.TT_GetVideoCaptureDevices(devs, ref count);
            if (b)
            {
                lpVideoDevices = new VideoCaptureDevice[count];
                for (int i = 0; i < count; i++)
                    lpVideoDevices[i] = devs[i];
            }
            else lpVideoDevices = null;

            return b;
        }
        /**
         * @brief Initialize a video capture device.
         *
         * To transmit video capture data to a channel call
         * TeamTalkBase.StartVideoCaptureTransmission()
         *
         * @param szDeviceID The device idenfier @a szDeviceID of #BearWare.VideoCaptureDevice.
         * @param lpVideoFormat The capture format to use,
         * i.e. frame-rate, resolution and picture format.
         * @see TeamTalkBase.GetVideoCaptureDevices
         * @see TeamTalkBase.CloseVideoCaptureDevice */
        public bool InitVideoCaptureDevice(string szDeviceID,
                                           VideoFormat lpVideoFormat)
        {
            return TTDLL.TT_InitVideoCaptureDevice(m_ttInst, szDeviceID, ref lpVideoFormat);
        }
        /**
         * @brief Close a video capture device.
         *
         * @see TeamTalkBase.InitVideoCaptureDevice */
        public bool CloseVideoCaptureDevice()
        {
            return TTDLL.TT_CloseVideoCaptureDevice(m_ttInst);
        }
        /**
         * @brief Paint user's video frame using a Windows' DC (device
         * context).
         *
         * Same as calling TeamTalkBase.PaintVideoFrameEx() like this:
         *
           @verbatim
           ttclient.PaintVideoFrameEx(nUserID, hDC, XDest, YDest, 
                                      nDestWidth, nDestHeight, 0, 0, 
                                      src_bmp_width, src_bmp_height);
           @endverbatim
         *
         * @c src_bmp_width and @c src_bmp_height are extracted internally
         * from the source image. */
        public bool PaintVideoFrame(int nUserID,
                                    System.IntPtr hDC,
                                    int XDest,
                                    int YDest,
                                    int nDestWidth,
                                    int nDestHeight,
                                    ref VideoFrame lpVideoFrame)
        {
            return TTDLL.TT_PaintVideoFrame(nUserID, hDC, XDest, YDest, nDestWidth, 
                                          nDestHeight, ref lpVideoFrame);
        }

        /** 
         * @brief Paint user's video frame using a Windows' DC (device
         * context).
         *
         * An application can either paint using TeamTalkBase.AcquireUserVideoCaptureFrame()
         * which provides a raw RGB32 array of the image or the
         * application can ask the client instance to paint the image
         * using this function.
         *
         * Typically this paint operation will be called in the WM_PAINT
         * message. Here is how the client instance paints internally:
         *
           @verbatim
           StretchDIBits(hDC, nPosX, nPosY, nWidth, nHeight, XSrc, YSrc, 
                         nSrcWidth, nSrcHeight, frame_buf, &bmi,
                         DIB_RGB_COLORS, SRCCOPY);
           @endverbatim 
         * 
         * @param nUserID The user's ID. 0 for local user.
         * @param hDC The handle to the Windows device context.
         * @param XDest Coordinate of left corner where to start painting.
         * @param YDest Coordinate or top corner where to start painting.
         * @param nDestWidth The width of the image.
         * @param nDestHeight The height of the image.
         * @param XSrc The left coordinate in the source bitmap of where
         * to start reading.
         * @param YSrc The top left coordinate in the source bitmap of where
         * to start reading.
         * @param nSrcWidth The number of width pixels to read from source bitmap.
         * @param nSrcHeight The number of height pixels to read from source bitmap.
         * @param lpVideoFrame Video frame retrieved by TeamTalkBase.AcquireUserVideoCaptureFrame() */
        public bool PaintVideoFrameEx(int nUserID,
                                      System.IntPtr hDC,
                                      int XDest,
                                      int YDest,
                                      int nDestWidth,
                                      int nDestHeight,
                                      int XSrc,
                                      int YSrc,
                                      int nSrcWidth,
                                      int nSrcHeight,
                                      ref VideoFrame lpVideoFrame)
        {
            return TTDLL.TT_PaintVideoFrameEx(nUserID, hDC, XDest, YDest, nDestWidth, nDestHeight, 
                                            XSrc, YSrc, nSrcWidth, nSrcHeight, 
                                            ref lpVideoFrame);
        }

        /** @brief Extract a user's video frame for display.
         *
         * The #BearWare.VideoFrame extracted from the client instance will contain a
         * pointer to the image's frame buffer, so a RGB32 bitmap can be
         * displayed in a window control. 
         *
         * To release the acquired #BearWare.VideoFrame call TeamTalkBase.ReleaseUserVideoCaptureFrame().
         *
         * A video capture frame comes from a user's #StreamType.STREAMTYPE_VIDEOCAPTURE.
         *
         * @param nUserID The user's ID. 0 for local user.
         * @param bmp A bitmap created from the #BearWare.VideoFrame's data.
         *
         * @return Returns video frame which will contain the image data. Note 
         * that it's the @a frameBuffer member of #BearWare.VideoFrame which will contain 
         * the image data allocated internally by TeamTalkBase. A #BearWare.VideoFrame with
         * all members assigned to 0 will be returned if no video frame is available.
         * @see TeamTalkBase.ReleaseUserVideoCaptureFrame() */
        public VideoFrame AcquireUserVideoCaptureFrame(int nUserID, out Bitmap bmp)
        {
            bmp = null;
            IntPtr ptr = TTDLL.TT_AcquireUserVideoCaptureFrame(m_ttInst, nUserID);
            if(ptr == IntPtr.Zero)
                return new VideoFrame();

            VideoFrame frm = (VideoFrame)Marshal.PtrToStructure(ptr, typeof(VideoFrame));
            vidcapframes.Add(frm.frameBuffer, ptr);

            PixelFormat pixelformat = PixelFormat.Format32bppRgb;
            bmp = new Bitmap(frm.nWidth, frm.nHeight, frm.nWidth*4, pixelformat, frm.frameBuffer);
            return frm;
        }

        Dictionary<IntPtr, IntPtr> vidcapframes = new Dictionary<IntPtr, IntPtr>();

        /** @brief Delete a user's video frame, acquired through
         * TeamTalkBase.AcquireUserVideoCaptureFrame(), so its allocated resources can be
         * released.
         *
         * @param lpVideoFrame Pointer to #BearWare.VideoFrame which should be deallocated. 
         * @return Returns TRUE If a video frame was successfully deallocated.
         * @see TeamTalkBase.AcquireUserVideoCaptureFrame() */
        public bool ReleaseUserVideoCaptureFrame(VideoFrame lpVideoFrame)
        {
            IntPtr ptr;
            if (vidcapframes.TryGetValue(lpVideoFrame.frameBuffer, out ptr))
            {
                vidcapframes.Remove(lpVideoFrame.frameBuffer);
                return TTDLL.TT_ReleaseUserVideoCaptureFrame(m_ttInst, ptr);
            }
            return false;
        }

        /**
         * @brief Get or query the raw RGB32 bitmap data of a user's video frame.
         *
         * @param nUserID The user's ID. 0 for local client instance.
         * @param lpPicture Reference to a bitmap which will receive the video frame.
         * If the @a lpPicture reference is null or not the proper size a new Bitmap object
         * will be allocated and returned instead. Ensure to ALWAYS call with the 
         * same @a lpPicture reference for each @a nUserID, otherwise a new Bitmap 
         * object will be allocated each time and cause the application to use a 
         * significant amount of memory.
         * @see PaintVideoFrame */

        /*
        public bool GetUserVideoFrame(int nUserID,
                               ref System.Drawing.Bitmap lpPicture)
        {
            CaptureFormat cap;
            if (!TTDLL.TT_GetUserVideoFrame(m_ttInst, nUserID, IntPtr.Zero, 0, out cap))
                return false;

            PixelFormat pixelformat = PixelFormat.Format32bppRgb;

            if (lpPicture == null ||
               lpPicture.Width != cap.nWidth ||
               lpPicture.Height != cap.nHeight
#if PocketPC
                )
#else
               || lpPicture.PixelFormat != pixelformat)
#endif
            {
                lpPicture = new Bitmap(cap.nWidth, cap.nHeight,
                                       pixelformat);
            }

            // Lock the bitmap's bits.  
            System.Drawing.Rectangle rect = new System.Drawing.Rectangle(0, 0, lpPicture.Width, lpPicture.Height);
            BitmapData bmpData = lpPicture.LockBits(rect, ImageLockMode.ReadWrite, pixelformat);

            // Get the address of the first line.
            IntPtr ptr = bmpData.Scan0;

            bool b = TTDLL.TT_GetUserVideoFrame(m_ttInst, nUserID, ptr,
                                              cap.nWidth * cap.nHeight * 4, out cap);
            // Unlock the bits.
            lpPicture.UnlockBits(bmpData);
            return b;
        }
         * */

        /** @} */

        /** @addtogroup mediastream
         * @{ */

        /** @brief Stream media file to channel, e.g. avi-, wav- or MP3-file.
         * @see TeamTalkBase.StartStreamingMediaFileToChannel() */
        public bool StartStreamingMediaFileToChannel(string szMediaFilePath,
                                                     VideoCodec lpVideoCodec)
        {
            return TTDLL.TT_StartStreamingMediaFileToChannel(m_ttInst, szMediaFilePath,
                                                           ref lpVideoCodec);
        }

        /**
         * @brief Stream media file to channel, e.g. avi-, wav- or MP3-file.
         *
         * Call TeamTalkBase.GetMediaFileInfo() to get the properties of a media
         * file, i.e. audio and video format.
         *
         * The event #ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE  is posted when
         * the media file starts streaming. The flags #ClientFlag.CLIENT_STREAM_AUDIO
         * and/or #ClientFlag.CLIENT_STREAM_VIDEO will be set if the call is successful.
         *
         * A media file is streamed using #StreamType.STREAMTYPE_MEDIAFILE_AUDIO
         * and/or #StreamType.STREAMTYPE_MEDIAFILE_VIDEO. To subscribe/unsubscribe a
         * media file being streamed use #Subscription.SUBSCRIBE_MEDIAFILE.
         *
         * Streaming a media file requires
         * #UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO and/or
         * #UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO.
         *
         * @param szMediaFilePath File path to media file.
         * @param lpMediaFilePlayback Playback settings to pause, seek and
         * preprocess audio. If #SPEEXDSP_AUDIOPREPROCESSOR then the echo
         * cancellation part of #BearWare.SpeexDSP is unused. Only denoise and AGC
         * settings are applied.
         * @param lpVideoCodec If video file then specify output codec properties 
         * here. Specify #Codec .NO_CODEC if video should be ignored.
         *
         * @see TeamTalkBase.UpdateStreamingMediaFileToChannel()
         * @see TeamTalkBase.StopStreamingMediaFileToChannel()
         * @see TeamTalkBase.InitLocalPlayback() */
        public bool StartStreamingMediaFileToChannel(string szMediaFilePath,
                                                     MediaFilePlayback lpMediaFilePlayback,
                                                     VideoCodec lpVideoCodec)
        {
            return TTDLL.TT_StartStreamingMediaFileToChannelEx(m_ttInst, szMediaFilePath,
                                                               ref lpMediaFilePlayback,
                                                               ref lpVideoCodec);
        }

        /**
         * @brief Update active media file being streamed to channel.
         *
         * While streaming a media file to a channel it's possible to
         * pause, seek and manipulate audio preprocessing by passing new
         * #BearWare.MediaFilePlayback properties.
         *
         * @param lpMediaFilePlayback Playback settings to pause, seek and
         * preprocess audio. If #SPEEXDSP_AUDIOPREPROCESSOR then the echo
         * cancellation part of #BearWare.SpeexDSP is unused. Only denoise and AGC
         * settings are applied.
         * @param lpVideoCodec If video file then specify output codec properties 
         * here, otherwise NULL.
         *
         * @see TeamTalkBase.StartStreamingMediaFileToChannel()
         * @see TeamTalkBase.StopStreamingMediaFileToChannel() */
        public bool UpdateStreamingMediaFileToChannel(MediaFilePlayback lpMediaFilePlayback,
                                                      VideoCodec lpVideoCodec)
        {
            return TTDLL.TT_UpdateStreamingMediaFileToChannel(m_ttInst, ref lpMediaFilePlayback, ref lpVideoCodec);
        }

        /**
         * @brief Stop streaming media file to channel.
         *
         * This will clear the flags #ClientFlag.CLIENT_STREAM_AUDIO
         * and/or #ClientFlag.CLIENT_STREAM_VIDEO.
         *
         * @see TeamTalkBase.StartStreamingMediaFileToChannel() */
        public bool StopStreamingMediaFileToChannel()
        {
            return TTDLL.TT_StopStreamingMediaFileToChannel(m_ttInst);
        }

        /**
         * Play media file using settings from TeamTalk instance.
         * I.e. TeamTalkBase.SetSoundOutputMute(), TeamTalkBase.SetSoundOutputVolume() and
         * TeamTalkBase.InitSoundOutputDevice().
         *
         * @param szMediaFilePath Path to media file.
         * @param lpMediaFilePlayback Playback settings to pause, seek and
         * preprocess audio. If #SPEEXDSP_AUDIOPREPROCESSOR then the echo
         * cancellation part of #BearWare.SpeexDSP is unused. Only denoise and AGC
         * settings are applied.
         *
         * @return A Session ID for identifing the media playback session.
         * If Session ID is <= 0 indicates an error.
         *
         * @return A session ID identifier referred to as @c nPlaybackSessionID.
         * 
         * @see TeamTalkBase.UpdateLocalPlayback()
         * @see TeamTalkBase.StopLocalPlayback()
         * @see TeamTalkBase.StartStreamingMediaFileToChannel() */
        public int InitLocalPlayback(string szMediaFilePath, MediaFilePlayback lpMediaFilePlayback)
        {
            return TTDLL.TT_InitLocalPlayback(m_ttInst, szMediaFilePath, ref lpMediaFilePlayback);
        }

        /**
         * Update media file currently being played by TeamTalk instance.
         * 
         * @param nPlaybackSessionID Session ID created by TeamTalkBase.InitLocalPlayback().
         * @param lpMediaFilePlayback #BearWare.AudioPreprocessorType of
         * #BearWare.AudioPreprocessor cannot be changed. It must be the same as
         * used in TeamTalkBase.InitLocalPlayback().
         *
         * @see TeamTalkBase.InitLocalPlayback()
         * @see TeamTalkBase.StopLocalPlayback()
         * @see TeamTalkBase.UpdateStreamingMediaFileToChannel() */
        public bool UpdateLocalPlayback(int nPlaybackSessionID,
                                        MediaFilePlayback lpMediaFilePlayback)
        {
            return TTDLL.TT_UpdateLocalPlayback(m_ttInst, nPlaybackSessionID, ref lpMediaFilePlayback);
        }

        /**
         * Stop media file currently being played by TeamTalk instance.
         *
         * @param nPlaybackSessionID Session ID created by TeamTalkBase.InitLocalPlayback().
         *
         * @see TeamTalkBase.InitLocalPlayback()
         * @see TeamTalkBase.UpdateLocalPlayback()
         * @see TeamTalkBase.StopStreamingMediaFileToChannel() */
        public bool StopLocalPlayback(int nPlaybackSessionID)
        {
            return TTDLL.TT_StopLocalPlayback(m_ttInst, nPlaybackSessionID);
        }

        /**
         * @brief Get the properties of a media file.
         *
         * Use this function to determine the audio and video properties of
         * a media file, so the user knows what can be streamed.
         *
         * @see TeamTalkBase.StartStreamingMediaFileToChannel() */
        public static bool GetMediaFileInfo(string szMediaFilePath,
                                            ref MediaFileInfo lpMediaFileInfo)
        {
            return TTDLL.TT_GetMediaFileInfo(szMediaFilePath, ref lpMediaFileInfo);
        }

        /** @brief Extract a user's media video frame for display.
         *
         * The #BearWare.VideoFrame extracted from the client instance will contain a
         * pointer to the image's frame buffer, so a RGB32 bitmap can be
         * displayed in a window control.
         *
         * To release the acquired #BearWare.VideoFrame call
         * TeamTalkBase.ReleaseUserMediaVideoFrame().
         *
         * A media video frame comes from a user's
         * #StreamType.STREAMTYPE_MEDIAFILE_VIDEO.
         *
         * @param nUserID The user's ID. 0 for local user.
         * @param bmp A bitmap created from the #BearWare.VideoFrame's data.
         * @return Returns video frame which will contain the image data. Note
         * that it's the @a frameBuffer member of #BearWare.VideoFrame which will contain
         * the image data allocated internally by TeamTalkBase. Returns NULL if no
         * video frame could be acquired.
         * @see TeamTalkBase.ReleaseUserMediaVideoFrame() */
        public VideoFrame AcquireUserMediaVideoFrame(int nUserID, out Bitmap bmp)
        {
            bmp = null;
            IntPtr ptr = TTDLL.TT_AcquireUserMediaVideoFrame(m_ttInst, nUserID);
            if (ptr == IntPtr.Zero)
                return new VideoFrame();

            VideoFrame frm = (VideoFrame)Marshal.PtrToStructure(ptr, typeof(VideoFrame));
            mediaframes.Add(frm.frameBuffer, ptr);

            PixelFormat pixelformat = PixelFormat.Format32bppRgb;
            bmp = new Bitmap(frm.nWidth, frm.nHeight, frm.nWidth * 4, pixelformat, frm.frameBuffer);
            return frm;
        }

        /** @brief Delete a user's video frame, acquired through
         * TeamTalkBase.AcquireUserMediaVideoFrame(), so its allocated resources can
         * be released.
         *
         * @param lpVideoFrame Pointer to #BearWare.VideoFrame which should be deallocated. 
         * @return Returns TRUE if a video frame was successfully deallocated.
         * @see AcquireUserMediaVideoFrame() */
        public bool ReleaseUserMediaVideoFrame(VideoFrame lpVideoFrame)
        {
            IntPtr ptr;
            if (mediaframes.TryGetValue(lpVideoFrame.frameBuffer, out ptr))
            {
                mediaframes.Remove(lpVideoFrame.frameBuffer);
                return TTDLL.TT_ReleaseUserMediaVideoFrame(m_ttInst, ptr);
            }
            return false;
        }

        Dictionary<IntPtr, IntPtr> mediaframes = new Dictionary<IntPtr, IntPtr>();

        /** @} */

        /** @addtogroup desktopshare
         * @{ */

        /**
         * @brief Transmit a desktop window (bitmap) to users in the same
         * channel.
         *
         * When TeamTalkBase.SendDesktopWindow() is called the first time a new
         * desktop session will be started. To update the current desktop
         * session call TeamTalkBase.SendDesktopWindow() again once the previous
         * desktop transmission has finished. Tracking progress of the
         * current desktop transmission is done by checking for the
         * TeamTalkBase.OnDesktopTransferUpdate() event. While the desktop
         * transmission is active the flag #ClientFlag.CLIENT_TX_DESKTOP will be set
         * on the local client instance.
         *
         * If the desktop window (bitmap) changes size (width/height) or
         * format a new desktop session will be started. Also if the user
         * changes channel a new desktop session will be started. Check @c
         * nSessionID of #BearWare.DesktopWindow to see if a new desktop session is
         * started or the TeamTalkBase.OnUserDesktopWindow() event.
         *
         * Remote users will get the TeamTalkBase.OnUserDesktopWindow() event
         * and can call TeamTalkBase.AcquireUserDesktopWindow() to retrieve the desktop
         * window.
         * 
         * User rights required:
         * - #UserRight.USERRIGHT_TRANSMIT_DESKTOP
         * 
         * @param lpDesktopWindow Properties of the bitmap. Set the @c nSessionID 
         * property to 0.
         * @param nConvertBmpFormat Before transmission convert the bitmap to this 
         * format.
         * @return TRUE if desktop window is queued for transmission. FALSE if 
         * @c nBitmapSize is invalid or if a desktop transmission is already 
         * active.
         * @return -1 on error. 0 if bitmap has no changes. Greater than 0 on 
         * success.
         * @see TeamTalkBase.CloseDesktopWindow()
         * @see TeamTalkBase.SendDesktopCursorPosition() */
        public int SendDesktopWindow(DesktopWindow lpDesktopWindow,
                                     BitmapFormat nConvertBmpFormat)
        {
            return TTDLL.TT_SendDesktopWindow(m_ttInst, ref lpDesktopWindow, nConvertBmpFormat);
        }

        /**
         * @brief Close the current desktop session.
         *
         * Closing the desktop session will cause the users receiving the
         * current desktop session to see the desktop session ID change to
         * 0 in the TeamTalkBase.OnUserDesktopWindow() event.
         */
        public bool CloseDesktopWindow()
        {
            return TTDLL.TT_CloseDesktopWindow(m_ttInst);
        }

        /**
         * @brief Get RGB values of the palette for the bitmap format.
         *
         * This currently only applies to bitmaps of format #BitmapFormat.BMP_RGB8_PALETTE.
         *
         * Note that the pointer returned is non-const which means the
         * palette can be overwritten with a custom palette. The custom
         * palette will then be used internally during bitmap
         * conversion.
         *
         * @param nBmpPalette The bitmap format. Currently only #BitmapFormat.BMP_RGB8_PALETTE
         * is supported.
         * @param nIndex The index in the color table of the RGB values to 
         * extract.
         * @return Pointer to RGB colors. First byte is Red, second Blue and 
         * third Green. Returns NULL if the color-index is invalid. */
        public static System.Drawing.Color Palette_GetColorTable(BitmapFormat nBmpPalette,
                                                          int nIndex)
        {
            IntPtr ptr = TTDLL.TT_Palette_GetColorTable(nBmpPalette, nIndex);
            switch(nBmpPalette)
            {
                case BitmapFormat.BMP_RGB8_PALETTE:
                    return Color.FromArgb(Marshal.ReadInt32(ptr));
            }
            return new Color();
        }

        /**
         * @brief Transmit the specified window in a desktop session.
         *
         * Same as TeamTalkBase.SendDesktopWindow() except the properties for the
         * #BearWare.DesktopWindow are extracted automatically.
         * 
         * @param hWnd Windows handle for the window to transmit.
         * @param nBitmapFormat Bitmap format to use for the transmitted image.
         * @param nDesktopProtocol The protocol to use for transmitting the image.
         * @return See TeamTalkBase.SendDesktopWindow(). */
        public int SendDesktopWindowFromHWND(System.IntPtr hWnd,
                                               BitmapFormat nBitmapFormat,
                                               DesktopProtocol nDesktopProtocol)
        {
            return TTDLL.TT_SendDesktopWindowFromHWND(m_ttInst, hWnd, nBitmapFormat, nDesktopProtocol);
        }

        /**
         * @brief Paint user's desktop window using a Windows' DC (device
         * context).
         *
         * Same as calling TeamTalkBase.PaintDesktopWindowEx() like this:
         *
           @verbatim
           TeamTalkBase.PaintDesktopWindowEx(nUserID, hDC, XDest, YDest, nDestWidth,
                                   nDestHeight, 0, 0, 
                                   'src_bmp_width', 'src_bmp_height');
           @endverbatim
         *
         * @c src_bmp_width and @c src_bmp_height are extracted internally
         * from the source image. */
        public bool PaintDesktopWindow(int nUserID,
                                       System.IntPtr hDC,
                                       int XDest,
                                       int YDest,
                                       int nDestWidth,
                                       int nDestHeight)
        {
            return TTDLL.TT_PaintDesktopWindow(m_ttInst, nUserID, hDC, XDest, YDest, nDestWidth, nDestHeight);
        }

        /**
         * @brief Paint user's desktop window using a Windows' DC (device
         * context).
         *
         * An application can either paint a bitmap by using
         * TeamTalkBase.AcquireUserDesktopWindow() which provides a pointer to a bitmap
         * or the application can ask the client instance to paint the
         * image using this function.
         *
         * Typically this paint operation will be called in the WM_PAINT
         * message. Here is how the client instance paints internally:
         *
           @verbatim
           StretchDIBits(hDC, nPosX, nPosY, nWidth, nHeight, XSrc, YSrc, 
                         nSrcWidth, nSrcHeight, frame_buf, &bmi,
                         DIB_RGB_COLORS, SRCCOPY);
           @endverbatim 
         * 
         * @param nUserID The user's ID.
         * @param hDC The handle to the Windows device context.
         * @param XDest Coordinate of left corner where to start painting.
         * @param YDest Coordinate or top corner where to start painting.
         * @param nDestWidth The width of the image.
         * @param nDestHeight The height of the image.
         * @param XSrc The left coordinate in the source bitmap of where
         * to start reading.
         * @param YSrc The top left coordinate in the source bitmap of where
         * to start reading.
         * @param nSrcWidth The number of width pixels to read from source bitmap.
         * @param nSrcHeight The number of height pixels to read from source bitmap.
         * @return TRUE on success. FALSE on error, e.g. if user doesn't exist.
         * @see TeamTalkBase.AcquireUserDesktopWindow() */
        public bool PaintDesktopWindowEx(int nUserID,
                                         System.IntPtr hDC,
                                         int XDest,
                                         int YDest,
                                         int nDestWidth,
                                         int nDestHeight,
                                         int XSrc,
                                         int YSrc,
                                         int nSrcWidth,
                                         int nSrcHeight)
        {
            return TTDLL.TT_PaintDesktopWindowEx(m_ttInst, nUserID, hDC, XDest, 
                                               YDest, nDestWidth, nDestHeight, 
                                               XSrc, YSrc, nSrcWidth, nSrcHeight);
        }

        /**
         * @brief Send the position of mouse cursor to users in the same channel.
         *
         * It's only possible to send the mouse cursor position if there's
         * a desktop session which is currently active.
         * 
         * User rights required:
         * - #UserRight.USERRIGHT_TRANSMIT_DESKTOP
         *
         * @param nPosX X coordinate of mouse cursor.
         * @param nPosY Y coordinate of mouse cursor.
         * @see TeamTalkBase.SendDesktopWindow() */
        public bool SendDesktopCursorPosition(ushort nPosX,
                                              ushort nPosY)
        {
            return TTDLL.TT_SendDesktopCursorPosition(m_ttInst, nPosX, nPosY);
        }

        /** 
         * @brief Send a mouse or keyboard event to a shared desktop window.
         *
         * If a user is sharing a desktop window it's possible for a
         * remote user to take control of mouse and keyboard input on the
         * remote computer. Read section @ref txdesktopinput on how to
         * transmit desktop input to a shared window.
         *
         * When the remote user receives the issued #BearWare.DesktopInput the
         * event #BearWare.TeamTalkBase.OnUserDesktopInput is posted to the client
         * instance sharing the desktop window.
         *
         * User rights required:
         * - #UserRight.USERRIGHT_TRANSMIT_DESKTOPINPUT
         *
         * @param nUserID The user's ID who owns the shared desktop window
         * and should receive desktop input.
         * @param lpDesktopInputs An array of #BearWare.DesktopInput structs which
         * should be transmitted to the user. Internally in the client
         * instance each user ID has an internal queue which can contain a
         * maximum of 100 #BearWare.DesktopInput structs.
         * @return FALSE If user doesn't exist or if desktop input queue is full or
         * if @c nUserID doesn't subscribe to desktop input. */
        public bool SendDesktopInput(int nUserID,
                                     DesktopInput[] lpDesktopInputs)
        {
            return TTDLL.TT_SendDesktopInput(m_ttInst, nUserID, lpDesktopInputs, lpDesktopInputs.Length);
        }

        /**
         * @brief Get a user's desktop window (bitmap image).
         *
         * A user's desktop window can be extracted when the 
         * TeamTalkBase.OnUserDesktopWindow() is received.
         *
         * When the #BearWare.DesktopWindow is no longer needed call
         * TeamTalkBase.ReleaseUserDesktopWindow() to release the memory allocated by
         * the client instance.
         *
         * A desktop window is simply a bitmap image. This method is used for
         * retrieving the user's bitmap image.
         *
         * To know the properties of the bitmap call this method with @c
         * lpBitmap set to NULL and extract the properties in @c
         * lpDesktopWindow.  The size of the buffer to allocate will be @c
         * nBytesPerLine multiplied by @c nHeight in the #BearWare.DesktopWindow.
         *
         * For #BitmapFormat.BMP_RGB8_PALETTE bitmaps check out TeamTalkBase.Palette_GetColorTable().
         *
         * @param nUserID The user's ID.
         * @return A zero'ed #BearWare.DesktopWindow if there's no active desktop window for this user.
         * @see TeamTalkBase.SendDesktopWindow() */
        public DesktopWindow AcquireUserDesktopWindow(int nUserID)
        {
            IntPtr ptr = TTDLL.TT_AcquireUserDesktopWindow(m_ttInst, nUserID);
            if (ptr == IntPtr.Zero)
                return new DesktopWindow();
            DesktopWindow lpDesktopWindow = (DesktopWindow)Marshal.PtrToStructure(ptr, typeof(DesktopWindow));
            desktopwindows.Add(lpDesktopWindow.frameBuffer, ptr);
            return lpDesktopWindow;
        }

        /**
         * @brief Same as TeamTalkBase.AcquireUserDesktopWindow() except an extra
         * option for converting bitmap to a different format.
         *
         * It is highly adviced to use TeamTalkBase.AcquireUserDesktopWindow() since
         * converting to a different bitmap format is very inefficient. */
        public DesktopWindow AcquireUserDesktopWindowEx(int nUserID, BitmapFormat nBitmapFormat)
        {
            IntPtr ptr = TTDLL.TT_AcquireUserDesktopWindowEx(m_ttInst, nUserID, nBitmapFormat);
            if (ptr == IntPtr.Zero)
                return new DesktopWindow();
            DesktopWindow lpDesktopWindow = (DesktopWindow)Marshal.PtrToStructure(ptr, typeof(DesktopWindow));
            desktopwindows.Add(lpDesktopWindow.frameBuffer, ptr);
            return lpDesktopWindow;
        }

        Dictionary<IntPtr, IntPtr> desktopwindows = new Dictionary<IntPtr, IntPtr>();
        /** @brief Release memory allocated by the #BearWare.DesktopWindow.
         * @see TeamTalkBase.AcquireUserDesktopWindow() */
        public bool ReleaseUserDesktopWindow(DesktopWindow lpDesktopWindow)
        {
            IntPtr ptr;
            if (desktopwindows.TryGetValue(lpDesktopWindow.frameBuffer, out ptr))
            {
                desktopwindows.Remove(lpDesktopWindow.frameBuffer);
                return TTDLL.TT_ReleaseUserDesktopWindow(m_ttInst, ptr);
            }
            return false;
        }

        /** @} */

        /** @addtogroup connectivity
         * @{ */

        /**
         * @brief Connect to a server. 
         * 
         * This is a non-blocking call (but may block due to DNS lookup)
         * so the user application must wait for the event
         * #OnConnectionSuccess to be posted once the connection has
         * been established or #OnConnectionFailed if connection could
         * not be established. If the connection could not be establish
         * ensure to call #Disconnect to close open connections in the
         * client instance before trying again.
         *
         * Once connected call #DoLogin to log on to the server.
         * 
         * @param szHostAddress The IP-address or hostname of the server.
         * @param nTcpPort The host port of the server (TCP).
         * @param nUdpPort The audio/video port of the server (UDP).
         * @param nLocalTcpPort The local TCP port which should be used. 
         * Setting it to 0 makes OS select a port number (recommended).
         * @param nLocalUdpPort The local UDP port which should be used. 
         * Setting it to 0 makes OS select a port number (recommended).
         * @param bEncrypted Whether the server requires an encrypted 
         * connection. Encryption is only available in the TeamTalk
         * Professional SDK.
         * @return Returns TRUE if connection process was initiated.
         * @see OnConnectionSuccess
         * @see OnConnectionFailed
         * @see TeamTalkBase.DoLogin */
        public bool Connect(string szHostAddress,
                            int nTcpPort,
                            int nUdpPort,
                            int nLocalTcpPort,
                            int nLocalUdpPort,
                            bool bEncrypted)
        {
            return TTDLL.TT_Connect(m_ttInst, szHostAddress, nTcpPort, nUdpPort, 
                                    nLocalTcpPort, nLocalUdpPort, bEncrypted);
        }

        /**
         * @brief Same as Connect() but the option of providing a
         * unique system-ID.
         *
         * The system-ID is set in the TeamTalk server API using
         * TTS_StartServerSysID(). If a client tries to connect with a
         * different system-ID that client will receive the error
         * #CMDERR_INCOMPATIBLE_PROTOCOLS when trying to log in.
         *
         * @param szHostAddress The IP-address or hostname of the server.
         * @param nTcpPort The host port of the server (TCP).
         * @param nUdpPort The audio/video port of the server (UDP).
         * @param nLocalTcpPort The local TCP port which should be used. 
         * Setting it to 0 makes OS select a port number (recommended).
         * @param nLocalUdpPort The local UDP port which should be used. 
         * Setting it to 0 makes OS select a port number (recommended).
         * @param bEncrypted Whether the server requires an encrypted 
         * connection. Encryption is only available in the TeamTalk
         * Professional SDK.
         * @param szSystemID The identification of the conferencing system.
         * The default value is "teamtalk". See TT_StartServerSysID()
         * @return Returns TRUE if connection process was initiated. */
        public bool ConnectSysID(string szHostAddress,
                                 int nTcpPort,
                                 int nUdpPort,
                                 int nLocalTcpPort,
                                 int nLocalUdpPort,
                                 bool bEncrypted,
                                 string szSystemID)
        {
            return TTDLL.TT_ConnectSysID(m_ttInst, szHostAddress, 
                                         nTcpPort, nUdpPort, nLocalTcpPort, 
                                         nLocalUdpPort, bEncrypted, szSystemID);
        }

        /**
         * @brief Bind to specific IP-address priot to connecting to server.
         *
         * Same as Connect() except that this also allows which IP-address
         * to bind to on the local interface.
         *
         * @param szHostAddress The IP-address or hostname of the server.
         * @param nTcpPort The host port of the server (TCP).
         * @param nUdpPort The audio/video port of the server (UDP).
         * @param szBindIPAddr The IP-address to bind to on the local interface
         * in dotted decimal format, e.g. 192.168.1.10.
         * @param nLocalTcpPort The local TCP port which should be used. 
         * Setting it to 0 makes OS select a port number (recommended).
         * @param nLocalUdpPort The local UDP port which should be used. 
         * Setting it to 0 makes OS select a port number (recommended).
         * @param bEncrypted Whether the server requires an encrypted 
         * connection. Encryption is only available in the TeamTalk
         * Professional SDK.
         * @see Connect */
        public bool ConnectEx(string szHostAddress,
                              int nTcpPort,
                              int nUdpPort,
                              string szBindIPAddr,
                              int nLocalTcpPort,
                              int nLocalUdpPort,
                              bool bEncrypted)
        {
            return TTDLL.TT_ConnectEx(m_ttInst, szHostAddress, nTcpPort, nUdpPort,
                                      szBindIPAddr, nLocalTcpPort, nLocalUdpPort,
                                      bEncrypted);
        }

        /**
         * @brief Disconnect from the server.
         * 
         * This will clear the flag #ClientFlag.CLIENT_CONNECTED and #ClientFlag.CLIENT_CONNECTING. */
        public bool Disconnect()
        {
            return TTDLL.TT_Disconnect(m_ttInst);
        }
        /**
         * @brief Query the maximum size of UDP data packets to the user
         * or server.
         *
         * The TeamTalkBase.OnConnectionMaxPayloadUpdated() event is posted when
         * the query has finished.
         *
         * @param nUserID The ID of the user to query or 0 for querying 
         * server. Currently only @c nUserID = 0 is supported. */
        public bool QueryMaxPayload(int nUserID)
        {
            return TTDLL.TT_QueryMaxPayload(m_ttInst, nUserID);
        }
        /**
         * @brief Retrieve client statistics of bandwidth usage and
         * response times.
         *
         * @see BearWare.ClientStatistics */
        public bool GetClientStatistics(ref ClientStatistics lpClientStatistics)
        {
            return TTDLL.TT_GetClientStatistics(m_ttInst, ref lpClientStatistics);
        }

        /**
         * @brief Update the client instance's default keep alive settings.
         *
         * It is generally discouraged to change the client instance's
         * keep alive settings unless the network has special
         * requirements.
         *
         * After calling SetClientKeepAlive() it is recommended doing a
         * DoPing() since all TCP and UDP keep alive timers will be
         * restarted.
         *
         * @see DoPing()
         * @see GetClientKeepAlive()
         * @see GetClientStatistics() */
        public bool SetClientKeepAlive(ClientKeepAlive lpClientKeepAlive)
        {
            return TTDLL.TT_SetClientKeepAlive(m_ttInst, ref lpClientKeepAlive);
        }

        /**
         * @brief Get the client instance's current keep alive settings.
         *
         * @see DoPing()
         * @see SetClientKeepAlive()
         * @see GetClientStatistics() */
        public bool GetClientKeepAlive(ref ClientKeepAlive lpClientKeepAlive)
        {
            return TTDLL.TT_GetClientKeepAlive(m_ttInst, ref lpClientKeepAlive);
        }
        /** @} */

        /** @addtogroup commands
         * @{ */

        /** @brief Ping server and wait for server to reply.
         *
         * Use this command to check if the server is responding.
         *
         * If the client instance doesn't ping the server within the timeout
         * specified by @c nUserTimeout in #BearWare.ServerProperties, then the server
         * will disconnect the client.
         *
         * The client instance automatically pings the server at half the time
         * specified by @c nUserTimeout in #BearWare.ServerProperties.
         *
         * #ClientEvent.CLIENTEVENT_CMD_PROCESSING is posted if server replies.
         *
         * @return Returns command ID which will be passed in 
         * #ClientEvent.CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
         * command. -1 is returned in case of error.
         */
        public int DoPing()
        {
            return TTDLL.TT_DoPing(m_ttInst);
        }

        /** @brief Same as DologinEx() but without the option to
         * specify @c szClientName. Kept for backwards compatibility.
         *
         * @param szNickname The nickname to use.
         * @param szUsername The username of the #BearWare.UserAccount set up on the 
         * server.
         * @param szPassword The password of the user account on the server. Leave 
         * blank if no account is needed on the server.
         * @return Returns command ID which will be passed in 
         * #ClientEvent.CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
         * command. -1 is returned in case of error. */
        public int DoLogin(string szNickname, string szUsername, string szPassword)
        {
            return TTDLL.TT_DoLogin(m_ttInst, szNickname, szUsername, szPassword);
        }

        /**
         * @brief Logon to a server.
         * 
         * Once connected to a server call this function to logon. If
         * the login is successful #OnCmdMyselfLoggedIn is
         * posted, otherwise #OnCmdError. Once logged on it's
         * not possible to talk to other users until the client instance
         * joins a channel. Call #DoJoinChannel to join a channel.
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_INCORRECT_CHANNEL_PASSWORD
         * - #ClientError #ClientError.CMDERR_INVALID_ACCOUNT
         * - #ClientError #ClientError.CMDERR_MAX_SERVER_USERS_EXCEEDED
         * - #ClientError #ClientError.CMDERR_SERVER_BANNED
         * - #ClientError #ClientError.CMDERR_ALREADY_LOGGEDIN
         *
         * @param szNickname The nickname to use.
         * @param szUsername The username of the #BearWare.UserAccount set up on the 
         * server.
         * @param szPassword The password of the user account on the server. Leave 
         * blank if no account is needed on the server.
         * @param szClientName The name of the client application used. This is an 
         * optional value and can be kept blank.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see TeamTalkBase.DoJoinChannel
         * @see OnCmdMyselfLoggedIn
         * @see OnCmdError */
        public int DoLoginEx(string szNickname, string szUsername, string szPassword,
                             string szClientName)
        {
            return TTDLL.TT_DoLoginEx(m_ttInst, szNickname, szUsername, szPassword, szClientName);
        }
        /**
         * @brief Logout of the server.
         *
         * If successful the event #OnCmdMyselfLoggedOut
         * will be posted.
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         *
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see OnCmdMyselfLoggedOut */
        public int DoLogout()
        {
            return TTDLL.TT_DoLogout(m_ttInst);
        }
        /**
         * @brief Create a new channel and join it.
         *
         * This function can also be used to join an existing channel and
         * in this case the parameters @a szTopic and @a szOpPassword are
         * ignored.
         *
         * When TeamTalkBase.DoJoinChannel() is used to create channels it works
         * similar to IRC. If the client instance tries to join a channel
         * which does not exist it will be created as a new channel. If
         * the client instance is the last user to leave a channel the
         * channel will be removed on the server.
         *
         * If the channel is created successfully the event
         * #ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW will be sent, followed by
         * #ClientEvent.CLIENTEVENT_CMD_USER_JOINED.
         *
         * User rights required:
         * - #UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_INCORRECT_CHANNEL_PASSWORD
         * - #ClientError #ClientError.CMDERR_MAX_CHANNEL_USERS_EXCEEDED
         * - #ClientError #ClientError.CMDERR_ALREADY_IN_CHANNEL
         * - #ClientError #ClientError.CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED
         * - #ClientError #ClientError.CMDERR_UNKNOWN_AUDIOCODEC If the server doesn't support the audio
         *   codec. Introduced in version 4.1.0.1264.
         *
         * @param lpChannel The channel to join or create if it doesn't already
         * exist.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoLeaveChannel
         * @see DoMakeChannel
         * @see OnCmdChannelNew
         * @see OnCmdMyselfJoinedChannel */
        public int DoJoinChannel(Channel lpChannel)
        {
            return TTDLL.TT_DoJoinChannel(m_ttInst, ref lpChannel);
        }
        /**
         * @brief Join an existing channel.
         * 
         * This command basically calls #DoJoinChannel but omits the
         * unnecessary parameters for only joining a channel and not
         * creating a new one.
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_INCORRECT_CHANNEL_PASSWORD
         * - #ClientError #ClientError.CMDERR_MAX_CHANNEL_USERS_EXCEEDED
         * - #ClientError #ClientError.CMDERR_ALREADY_IN_CHANNEL
         *
         * @param nChannelID The ID of the channel to join.
         * @param szPassword The password for the channel to join.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoLeaveChannel
         * @see DoMakeChannel
         * @see OnCmdChannelNew
         * @see OnCmdMyselfJoinedChannel */
        public int DoJoinChannelByID(int nChannelID, string szPassword)
        {
            return TTDLL.TT_DoJoinChannelByID(m_ttInst, nChannelID, szPassword);
        }
        /**
         * @brief Leave the current channel.
         *
         * Note that #DoLeaveChannel() doesn't take any parameters
         * since a user can only participate in one channel at the time.
         * If command is successful the event #OnCmdUserLeftChannel
         * will be posted.
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_NOT_IN_CHANNEL
         * - #ClientError #ClientError.CMDERR_CHANNEL_NOT_FOUND
         *
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoJoinChannel
         * @see OnCmdMyselfLeftChannel */
        public int DoLeaveChannel()
        {
            return TTDLL.TT_DoLeaveChannel(m_ttInst);
        }
        /**
         * @brief Change the client instance's nick name.
         *
         * The event #OnCmdUserUpdate will be posted if the
         * update was successful.
         *
         * Command will be rejected if #UserRight.USERRIGHT_LOCKED_NICKNAME is set.
         *
         * Possible errors:
         * - #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError.CMDERR_NOT_AUTHORIZED
         *
         * @param szNewNick is the new nick name to use.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see OnCmdUserUpdate */
        public int DoChangeNickname(string szNewNick)
        {
            return TTDLL.TT_DoChangeNickname(m_ttInst, szNewNick);
        }
        /**
         * @brief Change the client instance's currect status
         *
         * The event #OnCmdUserUpdate will be posted if the update
         * was successful.
         *
         * Command will be rejected if #UserRight.USERRIGHT_LOCKED_STATUS is set.
         *
         * Possible errors:
         * - #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError.CMDERR_NOT_AUTHORIZED
         *
         * @param nStatusMode The value for the status mode.
         * @param szStatusMessage The user's message associated with the status 
         * mode.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see OnCmdUserUpdate */
        public int DoChangeStatus(int nStatusMode, string szStatusMessage)
        {
            return TTDLL.TT_DoChangeStatus(m_ttInst, nStatusMode, szStatusMessage);
        }
        /**
         * @brief Send a text message to either a user or a channel. 
         *
         * Can also be a broadcast message which is received by all users
         * on the server. This, however, requires
         * #UserRight.USERRIGHT_TEXTMESSAGE_BROADCAST.
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED 
         * - #ClientError #ClientError.CMDERR_CHANNEL_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_USER_NOT_FOUND
         *
         * @param lpTextMessage A preallocated text-message struct.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see UserRight #UserRight.USERRIGHT_TEXTMESSAGE_BROADCAST */
        public int DoTextMessage(TextMessage lpTextMessage)
        {
            return TTDLL.TT_DoTextMessage(m_ttInst, ref lpTextMessage);
        }
        /**
         * @brief Make another user operator of a channel. 
         * 
         * User rights required:
         * - #UserRight.USERRIGHT_OPERATOR_ENABLE
         *
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_CHANNEL_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_USER_NOT_FOUND
         *
         * @param nUserID The user who should become channel operator. 
         * @param nChannelID The channel where the user should become operator
         * @param bMakeOperator Whether user should be op'ed or deop'ed
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoChannelOpEx */
        public int DoChannelOp(int nUserID, int nChannelID, bool bMakeOperator)
        {
            return TTDLL.TT_DoChannelOp(m_ttInst, nUserID, nChannelID, bMakeOperator);
        }
        /**
         * @brief Make another user operator of a channel using the 
         * @a szOpPassword of #BearWare.Channel.
         * 
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_CHANNEL_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_USER_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_INCORRECT_OP_PASSWORD
         *
         * @param nUserID The user who should become channel operator. 
         * @param nChannelID The channel where the user should become operator.
         * @param szOpPassword The @a szOpPassword of #BearWare.Channel.
         * @param bMakeOperator Whether user should be op'ed or deop'ed.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoChannelOp */
        public int DoChannelOpEx(int nUserID,
                          int nChannelID,
                          string szOpPassword,
                          bool bMakeOperator)
        {
            return TTDLL.TT_DoChannelOpEx(m_ttInst, nUserID, nChannelID, szOpPassword, bMakeOperator);
        }
        /**
         * @brief Kick user from either channel or server. 
         *
         * To ban a user call TeamTalkBase.DoBanUser() before TeamTalkBase.DoKickUser().
         *
         * User rights required:
         * - #UserRight.USERRIGHT_KICK_USERS
         * - Alternative channel-operator (see TeamTalkBase.DoChannelOp()).
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_CHANNEL_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_USER_NOT_FOUND
         *
         * @param nUserID The ID of the user to kick. 
         * @param nChannelID The channel where the user shoul be kicked from
         * or specify 0 to kick the user off the server.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoBanUser */
        public int DoKickUser(int nUserID, int nChannelID)
        {
            return TTDLL.TT_DoKickUser(m_ttInst, nUserID, nChannelID);
        }
        /**
         * @brief Send a file to the specified channel. 
         *
         * If user is logged on as an admin the file can be located in any
         * channel. If the user is not an admin the file must be located
         * in the same channel as the user is currently participating in.
         * The file being uploaded must have a file size which is less
         * than the disk quota of the channel, minus the sum of all the
         * files in the channel. The disk quota of a channel can be
         * obtained in the @c nDiskQuota of the #BearWare.Channel struct passed to
         * TeamTalkBase.GetChannel().
         *
         * User rights required:
         * - #UserRight.USERRIGHT_UPLOAD_FILES
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_MAX_DISKUSAGE_EXCEEDED
         * - #ClientError #ClientError.CMDERR_CHANNEL_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_FILETRANSFER_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_OPENFILE_FAILED
         * - #ClientError #ClientError.CMDERR_FILE_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_FILE_ALREADY_EXISTS
         * - #ClientError #ClientError.CMDERR_FILESHARING_DISABLED
         *
         * @param nChannelID The ID of the channel of where to put the file. Only 
         * admins can upload in channel other then their own.
         * @param szLocalFilePath The path of the file to upload, e.g. C:\\myfile.txt.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see Channel
         * @see GetChannel */
        public int DoSendFile(int nChannelID, string szLocalFilePath)
        {
            return TTDLL.TT_DoSendFile(m_ttInst, nChannelID, szLocalFilePath);
        }
        /**
         * @brief Download a file from the specified channel. 
         *
         * If user is logged on as an admin the file can be located in any
         * channel. If the user is not an admin the file must be located
         * in the same channel as the user is currently participating in.
         *
         * User rights required:
         * - #UserRight.USERRIGHT_DOWNLOAD_FILES
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_CHANNEL_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_FILETRANSFER_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_OPENFILE_FAILED
         * - #ClientError #ClientError.CMDERR_FILE_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_FILESHARING_DISABLED
         *
         * @param nChannelID The ID of the channel of where to get the file. Only 
         * admins can download in channel other then their own.
         * @param nFileID The ID of the file which is passed by #OnCmdFileNew.
         * @param szLocalFilePath The path of where to store the file, e.g. 
         * C:\\myfile.txt.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see OnCmdFileNew
         * @see GetChannelFiles */
        public int DoRecvFile(int nChannelID, int nFileID, string szLocalFilePath)
        {
            return TTDLL.TT_DoRecvFile(m_ttInst, nChannelID, nFileID, szLocalFilePath);
        }
        /**
         * @brief Delete a file from a channel. 
         *
         * A user is allowed to delete a file from a channel if either the
         * user is an admin, operator of the channel or owner of the
         * file. To be owner of the file a user must have a #BearWare.UserAccount
         * on the server.
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED 
         * - #ClientError #ClientError.CMDERR_CHANNEL_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_FILE_NOT_FOUND
         *
         * @param nChannelID The ID of the channel where the file is located.
         * @param nFileID The ID of the file to delete. The ID of the file which 
         * is passed by #OnCmdFileNew.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see OnCmdFileNew
         * @see GetChannelFiles */
        public int DoDeleteFile(int nChannelID, int nFileID)
        {
            return TTDLL.TT_DoDeleteFile(m_ttInst, nChannelID, nFileID);
        }
        /**
         * @brief Subscribe to user events and/or data.
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED 
         * - #ClientError #ClientError.CMDERR_USER_NOT_FOUND
         *
         * @param nUserID The ID of the user this should affect.
         * @param uSubscriptions Union of #Subscription to subscribe to.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see Subscription */
        public int DoSubscribe(int nUserID, Subscription uSubscriptions)
        {
            return TTDLL.TT_DoSubscribe(m_ttInst, nUserID, uSubscriptions);
        }
        /**
         * @brief Unsubscribe to user events/data. This can be used to ignore messages
         * or voice data from a specific user.
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED 
         * - #ClientError #ClientError.CMDERR_USER_NOT_FOUND
         *
         * @param nUserID The ID of the user this should affect.
         * @param uSubscriptions Union of #Subscription to unsubscribe.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see Subscription */
        public int DoUnsubscribe(int nUserID, Subscription uSubscriptions)
        {
            return TTDLL.TT_DoUnsubscribe(m_ttInst, nUserID, uSubscriptions);
        }
        /**
         * @brief Make a new channel on the server.
         * 
         * This command only applies to users with #UserRight.USERRIGHT_MODIFY_CHANNELS.
         *
         * User rights required:
         * - #UserRight.USERRIGHT_MODIFY_CHANNELS
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_CHANNEL_ALREADY_EXISTS
         * - #ClientError #ClientError.CMDERR_CHANNEL_NOT_FOUND If channel's combined path is longer than
         *   #BearWare.TeamTalkBase.TT_STRLEN.
         * - #ClientError #ClientError.CMDERR_INCORRECT_CHANNEL_PASSWORD If the password is longer than
         *   #BearWare.TeamTalkBase.TT_STRLEN.
         * - #ClientError #ClientError.CMDERR_UNKNOWN_AUDIOCODEC If the server doesn't support the audio
         *   codec. Introduced in version 4.1.0.1264.
         *
         * @param lpChannel A Channel-structure containing information about
         * the channel being created. The Channel's member @a nChannelID is ignored.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoJoinChannel */
        public int DoMakeChannel(Channel lpChannel)
        {
            return TTDLL.TT_DoMakeChannel(m_ttInst, ref lpChannel);
        }
        /**
         * @brief Update a channel's properties.
         *
         * Users with #UserRight.USERRIGHT_MODIFY_CHANNELS can update all properties of
         * a channel.
         *
         * A user with channel-operator status (see TeamTalkBase.DoChannelOp()) can 
         * also update a channel's properties, but is not able to change the 
         * following properties:
         * - @c audiocodec
         * - @c nDiskQuota
         * - @c nMaxUsers
         * - @c uChannelType
         *   - Cannot modify #ChannelType.CHANNEL_PERMANENT
         *
         * Note that a channel's #BearWare.AudioCodec cannot be changed
         * if there's currently users in the channel.
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED 
         * - #ClientError #ClientError.CMDERR_CHANNEL_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_CHANNEL_HAS_USERS
         * - #ClientError #ClientError.CMDERR_CHANNEL_ALREADY_EXISTS
         *
         * @param lpChannel A Channel-structure containing information about
         * the channel being modified. The channel member's \a nParentID
         * and \a szName are ignored.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoMakeChannel */
        public int DoUpdateChannel(Channel lpChannel)
        {
            return TTDLL.TT_DoUpdateChannel(m_ttInst, ref lpChannel);
        }
        /**
         * @brief Remove a channel from a server. 
         *
         * This command only applies to users with #UserRight.USERRIGHT_MODIFY_CHANNELS.
         *
         * If there's any users in the channel they will be kicked and
         * subchannels will be deleted as well.
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_CHANNEL_NOT_FOUND
         *
         * @param nChannelID The ID of the channel to remove.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoMakeChannel */
        public int DoRemoveChannel(int nChannelID)
        {
            return TTDLL.TT_DoRemoveChannel(m_ttInst, nChannelID);
        }
        /**
         * @brief Issue command to move a user from one channel to
         * another.
         *
         * User rights required:
         * - #UserRight.USERRIGHT_MOVE_USERS
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_CHANNEL_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_USER_NOT_FOUND
         *
         * @param nUserID User to be moved.
         * @param nChannelID Channel where user should be put into.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoMoveUserByID */
        public int DoMoveUser(int nUserID, int nChannelID)
        {
            return TTDLL.TT_DoMoveUser(m_ttInst, nUserID, nChannelID);
        }
        /**
         * @brief Update server properties.
         *
         * User rights required:
         * - #UserRight.USERRIGHT_UPDATE_SERVERPROPERTIES
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         *
         * @param lpServerProperties A structure holding the information to be set 
         * on the server.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see GetServerProperties */
        public int DoUpdateServer(ServerProperties lpServerProperties)
        {
            return TTDLL.TT_DoUpdateServer(m_ttInst, ref lpServerProperties);
        }
        /**
         * @brief Issue command to list user accounts on the server.
         *
         * The event OnCmdUserAccount() will be posted for every
         * #BearWare.UserAccount on the server. Ensure not to list too many many user
         * accounts since this may suspend event handling.
         *
         * User accounts can be used to create users with different user
         * rights.
         *
         * Only #UserType.USERTYPE_ADMIN can issue this command.
         *
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         *
         * @param nIndex Index of first user to display.
         * @param nCount The number of users to retrieve.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see UserAccount
         * @see UserType */
        public int DoListUserAccounts(int nIndex, int nCount)
        {
            return TTDLL.TT_DoListUserAccounts(m_ttInst, nIndex, nCount);
        }
        /**
         * @brief Issue command to create a new user account on the
         * server.
         *
         * Check out section @ref useradmin to see how the server handles
         * users.
         * 
         * Only #UserType.USERTYPE_ADMIN can issue this command.
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_INVALID_USERNAME
         *
         * @param lpUserAccount The properties of the user account to create.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoListUserAccounts
         * @see DoDeleteUserAccount
         * @see UserAccount
         * @see UserType */
        public int DoNewUserAccount(UserAccount lpUserAccount)
        {
            return TTDLL.TT_DoNewUserAccount(m_ttInst, ref lpUserAccount);
        }
        /**
         * @brief Issue command to delete a user account on the server.
         *
         * Only #UserType.USERTYPE_ADMIN can issue this command.
         * 
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_ACCOUNT_NOT_FOUND
         *
         * @param szUsername The username of the user account to delete.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoListUserAccounts
         * @see DoNewUserAccount
         * @see BearWare.UserAccount
         * @see BearWare.UserType */
        public int DoDeleteUserAccount(string szUsername)
        {
            return TTDLL.TT_DoDeleteUserAccount(m_ttInst, szUsername);
        }
        /**
         * @brief Issue a ban command on a user in a specific channel. 
         *
         * The ban applies to the user's IP-address. Call TeamTalkBase.DoKickUser()
         * to kick the user off the server.
         *
         * User rights required:
         * - #UserRight.USERRIGHT_BAN_USERS
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_CHANNEL_NOT_FOUND
         * - #ClientError #ClientError.CMDERR_USER_NOT_FOUND
         *
         * @param nUserID The ID of the user to ban.
         * @param nChannelID Set to 0 to ban from logging in. Otherwise specify
         * user's current channel.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoKickUser()
         * @see DoListBans()
         * @see DoBanIPAddress()
         * @see DoBan()
         * @see DoBanUser()
         * @see DoBanUserEx() */
        public int DoBanUser(int nUserID, int nChannelID)
        {
            return TTDLL.TT_DoBanUser(m_ttInst, nUserID, nChannelID);
        }

        /** 
         * @brief Ban the user with @c nUserID using the ban types specified.
         *
         * If @c uBanTypes contains #BANTYPE_USERNAME then the username cannot join
         * the channel where @n nUserID is currently present.
         *
         * If @c uBanTypes contains #BANTYPE_IPADDR then the IP-address cannot join
         * the channel where @n nUserID is currently present.
         *
         * @see TT_DoListBans()
         * @see TT_DoBan() */
        public int DoBanUserEx(int nUserID, BanType uBanTypes)
        {
            return TTDLL.TT_DoBanUserEx(m_ttInst, nUserID, uBanTypes);
        }

        /**
         * @brief Ban the properties specified in @c lpBannedUser.
         *
         * The @c uBanTypes specifies what the ban applies to.  If
         * #BanType.BANTYPE_CHANNEL is specified in the @c uBanTypes of @c
         * lpBannedUser then the ban applies to joining a channel,
         * DoJoinChannel(). Otherwise the ban applies to login,
         * DoLogin().
         *
         * If #BANTYPE_IPADDR is specified then the IP-address must be set
         * in @c szIPAddress and any IP-address matching will receive
         * #CMDERR_SERVER_BANNED or #CMDERR_CHANNEL_BANNED for
         * TT_DoLogin() or TT_DoJoinChannel(). If instead
         * #BANTYPE_USERNAME is specified then @c szUsername must be set
         * and the same rule applies as for IP-addresses.
         *
         * @see TT_DoListBans()
         * @see TT_DoBanUserEx() */
        public int DoBan(BannedUser lpBannedUser)
        {
            return TTDLL.TT_DoBan(m_ttInst, ref lpBannedUser);
        }

        /**
         * @brief Issue a ban command on an IP-address user. 
         *
         * Same as TeamTalkBase.DoBanUser() except this command applies to IP-addresses
         * and therefore doesn't require a user to be logged in.
         *
         * User rights required:
         * - #UserRight.USERRIGHT_BAN_USERS
         *
         * Possible errors:
         * - #CMDERR_NOT_LOGGEDIN
         * - #CMDERR_NOT_AUTHORIZED
         *
         * @param szIPAddress The IP-address to ban.
         * @param nChannelID Set to zero.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see TeamTalkBase.DoKickUser
         * @see TeamTalkBase.DoListBans */
        public int DoBanIPAddress(string szIPAddress, int nChannelID)
        {
            return TTDLL.TT_DoBanIPAddress(m_ttInst, szIPAddress, nChannelID);
        }

        /**
         * @brief Unban the user with the specified IP-address.
         *
         * User rights required:
         * - #UserRight.USERRIGHT_BAN_USERS
         *
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_BAN_NOT_FOUND
         *
         * @param szIPAddress The IP-address to unban.
         * @param nChannelID Set to zero.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoBanUser
         * @see DoListBans
         * @see DoBanIPAddress */
        public int DoUnBanUser(string szIPAddress, int nChannelID)
        {
            return TTDLL.TT_DoUnBanUser(m_ttInst, szIPAddress, nChannelID);
        }

        /**
         * @brief Unban the properties specified in #BearWare.BannedUser.
         *
         * The uBanTypes in #BearWare.BannedUser determines which properties should have
         * their ban remove. E.g. uBanTypes = #BanType.BANTYPE_USERNAME and 
         * @c szUsername = "guest" will remove all bans where the username
         * is "guest".
         *
         * @see DoBan() */
        public int DoUnBanUserEx(BannedUser lpBannedUser)
        {
            return TTDLL.TT_DoUnBanUserEx(m_ttInst, ref lpBannedUser);
        }

        /**
         * @brief Issue a command to list the banned users.
         *
         * The event OnCmdBannedUser() will be posted for every
         * #BearWare.BannedUser on the server.
         *
         * User rights required:
         * - #UserRight.USERRIGHT_BAN_USERS
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         *
         * @param nChannelID Set to zero.
         * @param nIndex Index of first ban to display.
         * @param nCount The number of bans to display.
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see TeamTalkBase.DoBanUser() */
        public int DoListBans(int nChannelID, int nIndex, int nCount)
        {
            return TTDLL.TT_DoListBans(m_ttInst, nChannelID, nIndex, nCount);
        }
        /**
         * @brief Save the server's current state to its settings file
         * (typically the server's .xml file).
         *
         * Only #UserType.USERTYPE_ADMIN can issue this command.
         *
         * Note that the server only saves channels with the flag
         * #ChannelType #ChannelType.CHANNEL_PERMANENT.
         *
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         *
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error.
         * @see DoUpdateServer */
        public int DoSaveConfig()
        {
            return TTDLL.TT_DoSaveConfig(m_ttInst);
        }
        /**
         * @brief Get the server's current statistics.
         *
         * Event #ClientEvent.CLIENTEVENT_CMD_SERVERSTATISTICS will be posted when
         * server has sent response.
         *
         * Only #UserType.USERTYPE_ADMIN can issue this command.
         * 
         * Possible errors:
         * - #ClientError #ClientError.CMDERR_NOT_LOGGEDIN
         * - #ClientError #ClientError.CMDERR_NOT_AUTHORIZED
         * - #ClientError #ClientError.CMDERR_UNKNOWN_COMMAND
         *
         * @return Returns command ID which will be passed in 
         * OnCmdProcessing() event when the server is processing the 
         * command. -1 is returned in case of error. */
        public int DoQueryServerStats()
        {
            return TTDLL.TT_DoQueryServerStats(m_ttInst);
        }
        /**
         * @brief Quit from server. 
         *
         * Possible errors:
         * - none
         *
         * This will generate a #OnConnectionLost since the server
         * will drop the client.
         *
         * @return Returns command ID which will be passed in 
         * #OnCmdProcessing event when the server is processing the 
         * command. -1 is returned in case of error. */
        public int DoQuit()
        {
            return TTDLL.TT_DoQuit(m_ttInst);
        }
        /** @} */

        /**
         * @addtogroup server
         * @{ */

        /** 
         *
         * @brief Get the server's properties.
         *
         * @param lpServerProperties A struct to hold the server's properties. */
        public bool GetServerProperties(ref ServerProperties lpServerProperties)
        {
            return TTDLL.TT_GetServerProperties(m_ttInst, ref lpServerProperties);
        }
       /**
         * @brief Get all the users on the server.
         *
         * If only users in a specific channel is needed call
         * TeamTalkBase.GetChannelUsers()
         *
         * @param lpUsers An output array of #BearWare.User which will receive the
         * users on the server.
         * @see GetChannelUsers
         * @see GetUser */
        public bool GetServerUsers(out User[] lpUsers)
        {
            int count = 0;
            bool b = TTDLL.TT_GetServerUsers_NULL(m_ttInst, IntPtr.Zero, ref count);
            User[] users = new User[count];
            b = TTDLL.TT_GetServerUsers(m_ttInst, users, ref count);
            lpUsers = b ? users : null;
            return b;
        }
        /** @} */

        /** @addtogroup channels
         * @{ */

        /**
         * @brief Get the root channel's ID
         *
         * @return Returns the ID of the root channel. If 0 is returned no root 
         * channel exists.
         * @see GetMyChannelID
         * @see GetChannelPath */
        public int GetRootChannelID()
        {
            return TTDLL.TT_GetRootChannelID(m_ttInst);
        }
        /**
         * @brief Get the channel which the local client instance is
         * currently participating in.
         *
         * @return Returns the ID of the current channel. If 0 is returned the 
         * user is not participating in a channel. */
        public int GetMyChannelID()
        {
            return TTDLL.TT_GetMyChannelID(m_ttInst);
        }
        /**
         * @brief Same as GetMyChannelID().
         */
        public int ChannelID { get { return GetMyChannelID(); } }
        /**
         * @brief Get the channel with a specific ID.
         *
         * @param nChannelID The ID of the channel to get information about.
         * @param lpChannel A preallocated struct which will receive the 
         * channel's properties.
         * @return FALSE if unable to retrieve channel otherwise TRUE. */
        public bool GetChannel(int nChannelID, ref Channel lpChannel)
        {
            return TTDLL.TT_GetChannel(m_ttInst, nChannelID, ref lpChannel);
        }
        /**
         * @brief Get the channel's path. Channels are separated by '/'.
         *
         * @param nChannelID The channel's ID.
         * @param szChannelPath Will receive the channel's path.
         * @return Returns TRUE if channel exists. */
        public bool GetChannelPath(int nChannelID, ref string szChannelPath)
        {
            IntPtr ptr = Marshal.AllocHGlobal(TeamTalkBase.TT_STRLEN * 2);
            bool b = TTDLL.TT_GetChannelPath(m_ttInst, nChannelID, ptr);
            if(b)
                szChannelPath = Marshal.PtrToStringUni(ptr);
            Marshal.FreeHGlobal(ptr);
            return b;
        }
        /**
         * @brief Get the channel ID of the supplied path. Channels are
         * separated by '/'
         *
         * @param szChannelPath Will receive the channel's path.
         * @return The channel's ID or 0 on error. */
        public int GetChannelIDFromPath(string szChannelPath)
        {
            return TTDLL.TT_GetChannelIDFromPath(m_ttInst, szChannelPath);
        }
        /**
         * @brief Get the IDs of all users in a channel.
         *
         * @param nChannelID The channel's ID.
         * @param lpUsers An output array of User which will receive the
         * users in the channel.
         * @see BearWare.User 
         * @see GetChannel */
        public bool GetChannelUsers(int nChannelID, out User[] lpUsers)
        {
            int count = 0;
            bool b = TTDLL.TT_GetChannelUsers_NULL(m_ttInst, nChannelID, IntPtr.Zero, ref count);
            User[] users = new User[count];
            b = TTDLL.TT_GetChannelUsers(m_ttInst, nChannelID, users, ref count);
            lpUsers = b ? users : null;
            return b;
        }
        /**
         * @brief Get the list of the files in a channel which can be
         * downloaded.
         *
         * @param nChannelID The ID of the channel to extract the files from.
         * @param lpRemoteFiles A array of #BearWare.RemoteFile which will receive 
         * file information.
         * @see TeamTalkBase.GetChannelFile */
        public bool GetChannelFiles(int nChannelID, out RemoteFile[] lpRemoteFiles)
        {
            int count = 0;
            bool b = TTDLL.TT_GetChannelFiles_NULL(m_ttInst, nChannelID, IntPtr.Zero, ref count);
            RemoteFile[] files = new RemoteFile[count];
            b = TTDLL.TT_GetChannelFiles(m_ttInst, nChannelID, files, ref count);
            lpRemoteFiles = b ? files : null;
            return b;
        }
        /**
         * @brief Get information about a file which can be downloaded.
         *
         * @param nChannelID The ID of the channel to extract the file from.
         * @param nFileID The ID of the file.
         * @param lpRemoteFile A preallocated struct which will receive 
         * file information. */
        public bool GetChannelFile(int nChannelID, int nFileID, ref RemoteFile lpRemoteFile)
        {
            return TTDLL.TT_GetChannelFile(m_ttInst, nChannelID, nFileID, ref lpRemoteFile);
        }
        /**
         * @brief Check whether user is operator of a channel
         *
         * @param nUserID the ID of the user to check.
         * @param nChannelID the ID of the channel to check whether user
         * is operator of. */
        public bool IsChannelOperator(int nUserID, int nChannelID)
        {
            return TTDLL.TT_IsChannelOperator(m_ttInst, nUserID, nChannelID);
        }
        /** 
         * @brief Get all the channels on the server.
         *
         * Use TeamTalkBase.GetChannel() to get more information about each of the
         * channels. 
         * @see TeamTalkBase.GetServerUsers() */
        public bool GetServerChannels(out Channel[] lpChannels)
        {
            int count = 0;
            bool b = TTDLL.TT_GetServerChannels_NULL(m_ttInst, IntPtr.Zero, ref count);
            Channel[] channels = new Channel[count];
            b = TTDLL.TT_GetServerChannels(m_ttInst, channels, ref count);
            lpChannels = b ? channels : null;
            return b;
        }
        /** @} */

        /** @addtogroup users
         * @{ */

        /**
         * @brief Get the local client instance's user ID. 
         *
         * This information can be retrieved after the
         * #OnCmdMyselfLoggedIn event.
         *
         * @return Returns the user ID assigned to the current user on the server.
         * -1 is returned if no ID has been assigned to the user. */
        public int GetMyUserID()
        {
            return TTDLL.TT_GetMyUserID(m_ttInst);
        }

        /**
         * @brief Get the local client instance's #BearWare.UserAccount.
         *
         * This information can be retrieved after
         * TeamTalkBase.OnCmdMyselfLoggedIn() event.
         *
         * @param lpUserAccount The local client's user account registered on
         * the server. Note that the @a szPassword field of #BearWare.UserAccount
         * will not be set.
         * @see TeamTalkBase.DoLogin */
        public bool GetMyUserAccount(ref UserAccount lpUserAccount)
        {
            return TTDLL.TT_GetMyUserAccount(m_ttInst, ref lpUserAccount);
        }

        /**
         * @brief Same as GetMyUserID().
         */
        public int UserID { get { return GetMyUserID(); } }

        /**
         * @brief Get the client instance's user type. 
         *
         * This information can be retrieved after
         * #OnCmdMyselfLoggedIn event.
         * 
         * @return A bitmask of the type of user based on #UserType.
         * @see DoLogin
         * @see BearWare.UserType
         * @see GetMyUserAccount */
        public UserType GetMyUserType()
        {
            return TTDLL.TT_GetMyUserType(m_ttInst);
        }

        /**
         * @brief Same as GetMyUserType().
         */
        public BearWare.UserType UserType { get { return GetMyUserType(); } }

        /**
         * @brief Same as GetMyUserRights().
         */
        public BearWare.UserRight UserRights { get { return GetMyUserRights(); } }

        /** @brief Convenience method for TeamTalkBase.GetMyUserAccount() */
        public BearWare.UserRight GetMyUserRights()
        {
            return TTDLL.TT_GetMyUserRights(m_ttInst);
        }

        /**
         * @brief If an account was used in TeamTalkBase.DoLogin() then this value will 
         * return the @a nUserData from the #BearWare.UserAccount.
         *
         * This information can be retrieved after
         * #OnCmdMyselfLoggedIn event.
         *
         * @return If set, @a nUserData from #BearWare.UserAccount,
         * otherwise 0. */
        public int GetMyUserData()
        {
            return TTDLL.TT_GetMyUserData(m_ttInst);
        }

        /**
         * @brief Same as GetMyUserData().
         */
        public int UserData { get { return GetMyUserData(); } }

        /**
         * @brief Get the user with the specified ID.
         *
         * @param nUserID The ID of the user to extract.
         * @param lpUser A preallocated #BearWare.User struct.
         * @see GetUserByUsername */
        public bool GetUser(int nUserID, ref User lpUser)
        {
            return TTDLL.TT_GetUser(m_ttInst, nUserID, ref lpUser);
        }
        /**
         * @brief Get statistics for data and packet reception from a user.
         *
         * @param nUserID The ID of the user to extract.
         * @param lpUserStatistics A preallocated #BearWare.UserStatistics struct. */
        public bool GetUserStatistics(int nUserID, ref UserStatistics lpUserStatistics)
        {
            return TTDLL.TT_GetUserStatistics(m_ttInst, nUserID, ref lpUserStatistics);
        }
        /**
         * @brief Get the user with the specified username.
         *
         * Remember to take into account that multiple users can log in
         * with the same account if #UserRight.USERRIGHT_MULTI_LOGIN is specified.
         * 
         * @param szUsername The user's username (from #BearWare.UserAccount).
         * @param lpUser A preallocated #BearWare.User struct. */
        public bool GetUserByUsername(string szUsername, ref User lpUser)
        {
            return TTDLL.TT_GetUserByUsername(m_ttInst, szUsername, ref lpUser);
        }
        /** @} */

        /** @addtogroup sounddevices
         * @{ */

        /**
         * @brief Set the volume of a user.
         *
         * Note that it's a virtual volume which is being set since the
         * master volume affects the user volume.
         * 
         * @param nUserID The ID of the user whose volume will be changed.
         * @param nStreamType The type of stream to change, either 
         * #StreamType.STREAMTYPE_VOICE or #StreamType.STREAMTYPE_MEDIAFILE_AUDIO.
         * @param nVolume Must be between #BearWare.SoundLevel.SOUND_VOLUME_MIN and #BearWare.SoundLevel.SOUND_VOLUME_MAX.
         * @see SetSoundOutputVolume */
        public bool SetUserVolume(int nUserID, StreamType nStreamType, int nVolume)
        {
            return TTDLL.TT_SetUserVolume(m_ttInst, nUserID, nStreamType, nVolume);
        }
        /**
         * @brief Mute a user.
         *
         * To stop receiving audio from a user call TeamTalkBase.DoUnsubscribe() with
         * #Subscription.SUBSCRIBE_VOICE.
         *
         * @param nUserID The user ID of the user to mute (or unmute).
         * @param nStreamType The type of stream to change, either 
         * #StreamType.STREAMTYPE_VOICE or #StreamType.STREAMTYPE_MEDIAFILE_AUDIO.
         * @param bMute TRUE will mute, FALSE will unmute.
         * @see SetSoundOutputMute */
        public bool SetUserMute(int nUserID, StreamType nStreamType, bool bMute)
        {
            return TTDLL.TT_SetUserMute(m_ttInst, nUserID, nStreamType, bMute);
        }
        /**
         * @brief Set the delay of when a user should no longer be considered
         * as playing audio (either voice or audio from media file).
         *
         * When a user starts talking the TeamTalkBase.OnUserStateChange() is
         * triggered with @c uUserState changing. A user will remain
         * in this active state until no packets are received from this
         * user, plus a delay (due to network interruptions). This delay
         * is by default set to 500 msec but can be changed by calling
         * TeamTalkBase.SetUserStoppedTalkingDelay().
         *
         * @see GetUserStoppedTalkingDelay */
        public bool SetUserStoppedPlaybackDelay(int nUserID, 
                                                StreamType nStreamType,
                                                int nDelayMSec)
        {
            return TTDLL.TT_SetUserStoppedPlaybackDelay(m_ttInst, nUserID, nStreamType, nDelayMSec);
        }

        /**
         * @brief Set the position of a user.
         *
         * This can only be done using DirectSound (#SoundSystem.SOUNDSYSTEM_DSOUND)
         * and with sound duplex mode (#ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX)
         * disabled.
         *
         * @param nUserID ID of user.
         * @param nStreamType The type of stream to change, either 
         * #StreamType.STREAMTYPE_VOICE or #StreamType.STREAMTYPE_MEDIAFILE_AUDIO.
         * @param x Distance in meters to user (left/right).
         * @param y Distance in meters to user (back/forward).
         * @param z Distance in meters to user (up/down). */
        public bool SetUserPosition(int nUserID, StreamType nStreamType, 
                                    float x, float y, float z)
        {
            return TTDLL.TT_SetUserPosition(m_ttInst, nUserID, nStreamType, x, y, z);
        }
        /**
         * @brief Set whether a user should speak in the left, right or
         * both speakers. This function only works if #BearWare.AudioCodec has been
         * set to use stereo.
         *
         * @param nUserID ID of user.
         * @param nStreamType The type of stream to change, either 
         * #StreamType.STREAMTYPE_VOICE or #StreamType.STREAMTYPE_MEDIAFILE_AUDIO.
         * @param bLeftSpeaker TRUE if user should be played in left speaker.
         * @param bRightSpeaker TRUE if user should be played in right speaker.
         * @see GetUserStereo */
        public bool SetUserStereo(int nUserID, StreamType nStreamType, bool bLeftSpeaker, bool bRightSpeaker)
        {
            return TTDLL.TT_SetUserStereo(m_ttInst, nUserID, nStreamType, bLeftSpeaker, bRightSpeaker);
        }
        /**
         * @brief Store user's audio to disk.
         * 
         * Set the path of where to store audio from a user to disk.
         * Event #OnUserRecordMediaFile is triggered when
         * recording starts/stops.
         *
         * To store in MP3 format instead of .wav format ensure that the
         * LAME MP3 encoder file lame_enc.dll is placed in the same
         * directory as the SDKs DLL files. To stop recording set @c
         * szFolderPath to an empty string and @a uAFF to #AudioFileFormat.AFF_NONE.
         *
         * To store audio of other channels than the client instance check
         * out the section @ref spying.
         *
         * @param nUserID The ID of the #BearWare.User which should
         * store audio to disk. Pass 0 to store local instance's media stream.
         * @param szFolderPath The path on disk to where files should be stored.
         * This value will be stored in @a szMediaStorageDir of #BearWare.User.
         * @param szFileNameVars The file name used for audio files
         * can consist of the following variables: \%nickname\%,
         * \%username\%, \%userid\%, \%counter\% and a specified time
         * based on @c strftime (google @c 'strftime' for a
         * description of the format. The default format used by
         * TeamTalk is: '\%Y\%m\%d-\%H\%M\%S #\%userid\%
         * \%username\%'.  The \%counter\% variable is a 9 digit
         * integer which is incremented for each audio file. The file
         * extension is automatically appended based on the file type
         * (.wav for #AudioFileFormat #AudioFileFormat.AFF_WAVE_FORMAT and .mp3 for
         * AFF_MP3_*_FORMAT). Pass NULL or empty string to revert to
         * default format.
         * @param uAFF The #AudioFileFormat to use for storing audio files. Passing
         * #AudioFileFormat #AudioFileFormat.AFF_NONE will cancel/reset the current recording.
         * @return FALSE if path is invalid, otherwise TRUE.
         * 
         * @see BearWare.User
         * @see OnUserRecordMediaFile */
        public bool SetUserMediaStorageDir(int nUserID, string szFolderPath, string szFileNameVars,
                                       AudioFileFormat uAFF)
        {
            return TTDLL.TT_SetUserMediaStorageDir(m_ttInst, nUserID, szFolderPath, szFileNameVars, uAFF);
        }
        /**
         * @brief Change the amount of media data which can be buffered
         * in the user's playback queue.
         * 
         * Increasing the media buffer size is especially important when
         * the user is currently streaming a media file using
         * TeamTalkBase.StartStreamingMediaFileToChannel(). Once streaming has finished
         * it is recommended to reset the media buffer, i.e. setting it
         * to zero.
         *
         * A simple way to notify users that the client instance is streaming
         * a media file is to change the status of the local client instance
         * using TeamTalkBase.DoChangeStatus() or to send a #BearWare.TextMessage using
         * TeamTalkBase.DoTextMessage().
         *
         * @param nUserID The ID of the user who should have changed
         * the size of the media buffer.
         * @param uStreamType The stream type to apply the buffer for, i.e.
         * either #StreamType.STREAMTYPE_VOICE or #StreamType.STREAMTYPE_MEDIAFILE_AUDIO.
         * @param nMSec The number of miliseconds of audio and video which
         * should be allowed to be stored in the media buffer. 2000 - 3000 
         * msec is a good size for a media buffer. Set the media
         * buffer size to 0 msec to reset the media buffer to its default value.
         */
        public bool SetUserAudioStreamBufferSize(int nUserID, StreamType uStreamType,
                                                 int nMSec)
        {
            return TTDLL.TT_SetUserAudioStreamBufferSize(m_ttInst, nUserID, uStreamType, nMSec);
        }
        /** @brief Extract the raw audio from a user who has been talking.
         *
         * To enable access to user's raw audio first call
         * TeamTalkBase.EnableAudioBlockEvent(). Whenever new audio becomes
         * available the event TeamTalkBase.OnUserAudioBlock() is generated and 
         * TeamTalkBase.AcquireUserAudioBlock() can be called to extract the audio.
         *
         * The #BearWare.AudioBlock contains shared memory with the local client
         * instance therefore always remember to call
         * TeamTalkBase.ReleaseUserAudioBlock() to release the shared memory.
         *
         * @param nUserID The user ID to monitor for audio callback. Pass 0
         * to monitor local audio.
         * @param nStreamType Either #StreamType.STREAMTYPE_VOICE or 
         * #StreamType.STREAMTYPE_MEDIAFILE_AUDIO.
         * 
         * @see TeamTalkBase.ReleaseUserAudioBlock()
         * @see TeamTalkBase.EnableAudioBlockEvent()
         * @see TeamTalkBase.OnUserAudioBlock() */
        public AudioBlock AcquireUserAudioBlock(StreamType nStreamType, int nUserID)
        {
            IntPtr ptr = TTDLL.TT_AcquireUserAudioBlock(m_ttInst, nStreamType, nUserID);
            if (ptr == IntPtr.Zero)
                return new AudioBlock();
            AudioBlock lpAudioBlock = (AudioBlock)Marshal.PtrToStructure(ptr, typeof(AudioBlock));
            audioblocks.Add(lpAudioBlock.lpRawAudio, ptr);
            return lpAudioBlock; 
        }

        Dictionary<IntPtr, IntPtr> audioblocks = new Dictionary<IntPtr, IntPtr>();

        /** 
         * @brief Release the shared memory of an #BearWare.AudioBlock.
         *
         * All #BearWare.AudioBlock-structures extracted through
         * TeamTalkBase.AcquireUserAudioBlock() must be released again since they
         * share memory with the local client instance.
         *
         * Never access @c lpAudioBlock after releasing its
         * #BearWare.AudioBlock. This will cause the application to crash with a
         * memory exception.
         *
         * @see TeamTalkBase.AcquireUserAudioBlock()
         * @see TeamTalkBase.OnUserAudioBlock() */
        public bool ReleaseUserAudioBlock(AudioBlock lpAudioBlock)
        {
            IntPtr ptr;
            if (audioblocks.TryGetValue(lpAudioBlock.lpRawAudio, out ptr))
            {
                audioblocks.Remove(lpAudioBlock.lpRawAudio);
                return TTDLL.TT_ReleaseUserAudioBlock(m_ttInst, ptr);
            }
            return false;
        }
        /** @} */

        /** @ingroup channels
         * @brief Get information about an active file transfer.  
         *
         * An active file transfer is one which has been posted through the
         * event #OnFileTransfer.
         *
         * @param nTransferID The ID of the file transfer to investigate. Transfer
         * ID is passed by #OnFileTransfer.
         * @param lpFileTransfer A preallocated struct which will receive the file 
         * transfer information.
         * @see CancelFileTransfer */
        public bool GetFileTransferInfo(int nTransferID, ref FileTransfer lpFileTransfer)
        {
            return TTDLL.TT_GetFileTransferInfo(m_ttInst, nTransferID, ref lpFileTransfer);
        }

        /** @ingroup channels
         * @brief Cancel an active file transfer. 
         *
         * An active file transfer is one which has been post through the
         * event #OnFileTransfer.
         *
         * @param nTransferID The ID of the file transfer to investigate. Transfer 
         * ID is passed by #OnFileTransfer. */
        public bool CancelFileTranfer(int nTransferID)
        {
            return TTDLL.TT_CancelFileTransfer(m_ttInst, nTransferID);
        }

        /** @ingroup errorhandling
         * @brief Get textual discription of an error message.
         * 
         * Get a description of an error code posted by either
         * #OnCmdError or #OnInternalError.
         *
         * @param nError The number of the error.
         * @return A text description of the error.
         * @see OnCmdError
         * @see OnInternalError */
        public static string GetErrorMessage(ClientError nError)
        {
            IntPtr ptr = Marshal.AllocHGlobal(TeamTalkBase.TT_STRLEN * 2);
            TTDLL.TT_GetErrorMessage((int)nError, ptr);
            string s = Marshal.PtrToStringUni(ptr);
            Marshal.FreeHGlobal(ptr);
            return s;
        }

        /** @addtogroup hotkey
         * @{ */

        /**
         * @brief Register a global hotkey. 
         *
         * When the hotkey becomes active or inactive it will post the event
         * #OnHotKeyToggle.
         *
         * A hotkey can e.g. be used as a push-to-talk key
         * combination. When the hotkey becomes active call
         * EnableVoiceTransmission().
         *
         * Note that having a hotkey enabled makes the Visual Studio
         * debugger really slow to respond, so when debugging it's best
         * not to have hotkeys enabled.
         *
         * @param nHotKeyID The ID of the hotkey to register. It will be
         * passed as the event #OnHotKeyToggle when the hotkey becomes either active or inactive.
         * @param lpnVKCodes An array of virtual key codes which constitute the
         * hotkey. This document outlines the virtual key codes:
         * http://msdn.microsoft.com/en-us/library/ms645540(VS.85).aspx
         * A hotkey consisting of Left Control+A would have the array consist of 
         * [162, 65].
         * @see HotKey_Unregister
         * @see HotKey_InstallTestHook */
        public bool HotKey_Register(int nHotKeyID, int[] lpnVKCodes)
        {
            return TTDLL.TT_HotKey_Register(m_ttInst, nHotKeyID, lpnVKCodes, lpnVKCodes.Length);
        }
        /**
         * @brief Unregister a registered hotkey.
         *
         * @param nHotKeyID is the ID of the hotkey to unregister.
         * @see HotKey_Register */
        public bool HotKey_Unregister(int nHotKeyID)
        {
            return TTDLL.TT_HotKey_Unregister(m_ttInst, nHotKeyID);
        }
        /**
         * @brief Check whether hotkey is active.
         *
         * @param nHotKeyID is the ID of the registered hotkey. 
         * @return 1 if active, 0 if inactive, -1 if hotkey-ID is invalid */
        public int HotKey_IsActive(int nHotKeyID)
        {
            return TTDLL.TT_HotKey_IsActive(m_ttInst, nHotKeyID);
        }
        /**
         * @brief Install a test hook which calls the event #OnHotKeyTest
         * whenever a key or mouse button is pressed.
         * 
         * This can be used to determine a key-combination, like e.g.
         * a push-to-talk key.
         * 
         * @see HotKey_RemoveTestHook
         * @see OnHotKeyTest */
        public bool HotKey_InstallTestHook()
        {
            if (m_eventhandler != null)
            {
#if PocketPC
                IntPtr hWnd = m_eventhandler.Hwnd;
#else
                IntPtr hWnd = m_eventhandler.Handle;
#endif
                return TTDLL.TT_HotKey_InstallTestHook(m_ttInst, hWnd,
                                                     MyEventHandler.WM_TEAMTALK_CLIENTEVENT);
            }
            return false;
        }
        /**
         * @brief Remove the test hook again so the #OnHotKeyTest event
         * will no longer be notified.
         *
         * @see HotKey_InstallTestHook */
        public bool HotKey_RemoveTestHook()
        {
            return TTDLL.TT_HotKey_RemoveTestHook(m_ttInst);
        }
        /**
         * @brief Get a string description of the virtual-key code.
         *
         * @param nVKCode The virtual key code passed in #OnHotKeyTest.
         * @param szKeyName Will receive key description in local language.
         * @see HotKey_InstallTestHook */
        public bool HotKey_GetKeyString(int nVKCode, ref string szKeyName)
        {
            IntPtr ptr = Marshal.AllocHGlobal(TeamTalkBase.TT_STRLEN * 2);
            bool b = TTDLL.TT_HotKey_GetKeyString(m_ttInst, nVKCode, ptr);
            if(b)
                szKeyName = Marshal.PtrToStringUni(ptr);
            Marshal.FreeHGlobal(ptr);
            return b;
        }
        /** @} */

        public bool DBG_SetSoundInputTone(StreamType uStreamTypes, int nFrequency)
        {
            return TTDLL.TT_DBG_SetSoundInputTone(m_ttInst, uStreamTypes, nFrequency);
        }

        /** @addtogroup events
         * @{ */

        /**
         * @brief Delegate for events #OnConnectionSuccess, 
         * #OnConnectionFailed and #OnConnectionLost. */
        public delegate void Connection();
        /**
         * @brief Connected successfully to the server.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_CON_SUCCESS
         * 
         * @see Connection Delegate for this event.
         * @see Connect
         * @see DoLogin */
        public event Connection OnConnectionSuccess;
        /** 
         * @brief Failed to connect to server.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_CON_FAILED
         *
         * @see Connection Delegate for this event.
         * @see Connect */
        public event Connection OnConnectionFailed;
        /** 
         * @brief Connection to server has been lost.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_CON_LOST
         * 
         * @see Connection Delegate for this event.
         * @see Connect */
        public event Connection OnConnectionLost;

        /** @brief Delegate for event OnConnectionMaxPayloadUpdated().
         * 
         * @param nPayloadSize The maximum size in bytes of the payload data which
         * is put in UDP packets. 0 means the max payload query failed.
         * @see TeamTalkBase.QueryMaxPayload() */
        public delegate void MaxPayloadUpdate(int nPayloadSize);

        /**
         * @brief The maximum size of the payload put into UDP packets
         * has been updated.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED
         *
         * @see TeamTalkBase.QueryMaxPayload() */
        public event MaxPayloadUpdate OnConnectionMaxPayloadUpdated;

        /**
         * @brief Delegate for event #OnCmdProcessing.
         * 
         * @param nCmdID Command ID being processed (returned by @c Do* 
         * commands)
         * @param bActive Is true if command ID started processing and false
         * if the command has finished processing. */
        public delegate void CommandProcessing(int nCmdID, bool bActive);

        /** 
         * @brief A command issued by @c Do* methods is being
         * processed.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_PROCESSING
         * 
         * Read section @ref cmdprocessing on how to use command
         * processing in the user application.
         * 
         * @see CommandProcessing Delegate for this event. */
        public event CommandProcessing OnCmdProcessing;

        /** @brief Delegate for event #OnCmdError.
         * 
         * @param nCmdID The command ID returned from the @c Do* commands.
         * @param clienterrormsg Contains error description. */
        public delegate void CommandError(int nCmdID, ClientErrorMsg clienterrormsg);

        /** 
         * @brief The server rejected a command issued by the local
         * client instance.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_ERROR
         *
         * @see CommandError Delegate for this event.
         * @see GetErrorMessage */
        public event CommandError OnCmdError;

        /** @brief Delegate for event #CommandSuccess.
         * 
         * @param nCmdID The command ID returned from the @c Do* commands. */
        public delegate void CommandSuccess(int nCmdID);

        /**
         * @brief The server successfully processed  a command issued by the local
         * client instance.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_SUCCESS
         *
         * @see CommandError Delegate for this event. */
        public event CommandSuccess OnCmdSuccess;

        /** @brief Delegate for event #OnCmdMyselfLoggedIn
         * 
         * @param nMyUserID The client instance's user ID, i.e. what can now 
         * be retrieved through #GetMyUserID.
         * @param useraccount The #BearWare.UserAccount of the local client instance. */
        public delegate void MyselfLoggedIn(int nMyUserID, UserAccount useraccount);

        /** 
         * @brief The client instance successfully logged on to
         * server.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDIN
         * 
         * @see MyselfLoggedIn Delegate for this event.
         * @see DoLogin */
        public event MyselfLoggedIn OnCmdMyselfLoggedIn;

        /** @brief Delegate for event #OnCmdMyselfLoggedOut */
        public delegate void MyselfLoggedOut();
        /** 
         * @brief The client instance logged out of a server.  
         *
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDOUT
         * 
         * @see MyselfLoggedOut Delegate for this event.
         * @see DoLogout */
        public event MyselfLoggedOut OnCmdMyselfLoggedOut;

        /** @brief Delegate for event #OnCmdMyselfKicked.
         * 
         * @param user The user who kicked the local client instance.
         * If user who kicked is unspecified a null user (user-id is 0) will 
         * be passed. */
        public delegate void MyselfKicked(User user);
        /** 
         * @brief The client instance was kicked from a channel.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_MYSELF_KICKED
         * */
        public event MyselfKicked OnCmdMyselfKicked;

        /** @brief Delegate for events #OnCmdUserLoggedIn,
         * #OnCmdUserLoggedOut, #OnCmdUserUpdate, #OnCmdUserJoinedChannel
         * and #OnCmdUserLeftChannel.
         * @param user The user. */
        public delegate void UserUpdate(User user);

        /**
         * @brief A new user logged on to the server.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_USER_LOGGEDIN
         *
         * @see DoLogin
         * @see OnCmdUserLoggedOut */
        public event UserUpdate OnCmdUserLoggedIn;

        /**
         * @brief A user logged out of the server. 
         *
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_USER_LOGGEDOUT
         *
         * @see DoLogout
         * @see Disconnect
         * @see OnCmdUserLoggedIn */
        public event UserUpdate OnCmdUserLoggedOut;

        /**
         * @brief User changed properties.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_USER_UPDATE */
        public event UserUpdate OnCmdUserUpdate;

        /** 
         * @brief A user has joined a channel.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_USER_JOINED */
        public event UserUpdate OnCmdUserJoinedChannel;

        /** 
         * @brief User has left a channel.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_USER_LEFT */
        public event UserUpdate OnCmdUserLeftChannel;

        /** @brief Delegate for events #OnCmdUserTextMessage.
         * 
         * @see GetTextMessage To retrieve message.
         * @see GetUser To retrieve user.
         * @see DoTextMessage() To send a text message. */
        public delegate void UserTextMessage(TextMessage textmessage);

        /** 
         * @brief A user has sent a text-message.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_USER_TEXTMSG */
        public event UserTextMessage OnCmdUserTextMessage;


        /** @brief Delegate for #OnCmdChannelNew, #OnCmdChannelUpdate and
         * #OnCmdChannelRemove. */
        public delegate void ChannelUpdate(Channel channel);

        /** 
         * @brief A new channel has been created.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW
         * 
         * @see ChannelUpdate Delegate for this event. */
        public event ChannelUpdate OnCmdChannelNew;

        /** 
         * @brief A channel's properties has been updated.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE */
        public event ChannelUpdate OnCmdChannelUpdate;

        /** 
         * @brief A channel has been removed.
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_CHANNEL_REMOVE */
        public event ChannelUpdate OnCmdChannelRemove;

        /** @brief Delegate for event #OnCmdServerUpdate. */
        public delegate void ServerUpdate(ServerProperties serverproperties);

        /** 
         * @brief Server has updated its settings (server name, MOTD,
         * etc.)
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_SERVER_UPDATE */
        public event ServerUpdate OnCmdServerUpdate;

        /** @brief Delegate for event #OnCmdServerStatistics. */
        public delegate void ServerStats(ServerStatistics serverstatistics);
            
        /** 
         * @brief Server statistics available.
         *
         * This is a response to TeamTalkBase.DoServerStatistics() */
        public event ServerStats OnCmdServerStatistics;

        /** @brief A delegate for events #OnCmdFileNew and #OnCmdFileRemove. */
        public delegate void FileUpdate(RemoteFile remotefile);

        /** 
         * @brief A new file is added to a channel. 
         *
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_FILE_NEW */
        public event FileUpdate OnCmdFileNew;

        /** 
         * @brief A file has been removed from a channel.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_FILE_REMOVE */
        public event FileUpdate OnCmdFileRemove;

        /** @brief A delegate for event #OnCmdUserAccount. */
        public delegate void ListUserAccount(UserAccount useraccount);

        /** @brief A new user account has been listed by the server.
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_USERACCOUNT */
        public event ListUserAccount OnCmdUserAccount;

        /** @brief A delegate for event #OnCmdUserAccount. */
        public delegate void ListBannedUser(BannedUser banneduser);

        /** @brief A new banned user has been listed by the server.
         * Event handler for #ClientEvent.CLIENTEVENT_CMD_BANNEDUSER */
        public event ListBannedUser OnCmdBannedUser;

        /** @brief A user's state has been updated.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_USER_STATECHANGE */
        public event UserUpdate OnUserStateChange;

        /** @brief A delegate for the event #OnUserVideoCapture. */
        public delegate void UserVideoFrame(int nUserID, int nStreamID);

        /** 
         * @brief A new video frame was received from a user.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_USER_VIDEOCAPTURE */
        public event UserVideoFrame OnUserVideoCapture;

        /** 
         * @brief A new video frame was received from a user.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_USER_MEDIAFILE_VIDEO */
        public event UserVideoFrame OnUserMediaFileVideo;

        /** @brief Delegate for event #OnUserDesktopWindow.
         *
         * @param nUserID The user's ID.
         * @param nStreamID The ID of the desktop window's session. If
         * this ID changes it means the user has started a new
         * session. If the session ID becomes 0 it means the desktop
         * session has been closed by the user.
         * @see TeamTalkBase.SendDesktopWindow() */
        public delegate void NewDesktopWindow(int nUserID, int nStreamID);

        /**
         * @brief A new or updated desktop window has been received
         * from a user.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW
         *
         * Use TeamTalkBase.AcquireUserDesktopWindow() to retrieve the bitmap of the
         * desktop window. */
        public event NewDesktopWindow OnUserDesktopWindow;

        /** @brief Delegate for event OnUserDesktopCursor(). */
        public delegate void UserDesktopInput(int nSrcUserID, DesktopInput desktopinput);

        /**
         * @brief A user has sent the position of the mouse cursor.
         *
         * Use TeamTalkBase.SendDesktopCursorPosition() to send the position of
         * the mouse cursor. */
        public event UserDesktopInput OnUserDesktopCursor;

        /**
         * @brief Desktop input (mouse or keyboard input) has been
         * received from a user.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_USER_DESKTOPINPUT */
        public event UserDesktopInput OnUserDesktopInput;

        /** @brief Delegate for event #OnUserRecordMediaFile. */
        public delegate void UserRecordMediaFile(int nUserID, MediaFileInfo mediafileinfo);

        /** 
         * @brief An media file recording has changed status.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE */
        public event UserRecordMediaFile OnUserRecordMediaFile;

        /** @brief Delegate for event #OnUserAudioBlock. */
        public delegate void NewAudioBlock(int nUserID, StreamType nStreamType);

        /**
         * @brief A new audio block can be extracted.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK
         * 
         * This event is only generated if TeamTalkBase.EnableAudioBlockEvent()
         * is first called.
         *
         * Call TeamTalkBase.AcquireUserAudioBlock() to extract the #BearWare.AudioBlock. */
        public event NewAudioBlock OnUserAudioBlock;

        /** @brief Delegate for event #OnInternalError. */
        public delegate void ErrorOccured(ClientErrorMsg clienterrormsg);

        /** 
         * @brief A internal error occured.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_INTERNAL_ERROR */
        public event ErrorOccured OnInternalError;

        /** @brief Delegate for event #OnVoiceActivation.
         * @param bVoiceActive TRUE if enabled, FALSE if disabled. */
        public delegate void VoiceActivation(bool bVoiceActive);

        /** 
         * @brief Voice activation has triggered transmission.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_VOICE_ACTIVATION */
        public event VoiceActivation OnVoiceActivation;

        /** @brief Delegate for event #OnHotKeyToggle. 
         *
         * @param nHotKeyID The hotkey ID passed to #HotKey_Register
         * @param bActive TRUE when hotkey is active and FALSE when 
         * it becomes inactive. */
        public delegate void HotKeyToggle(int nHotKeyID, bool bActive);

        /** 
         * @brief A hotkey has been acticated or deactivated.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_HOTKEY
         * 
         * @see HotKeyToggle Delegate for this event.
         * @see HotKey_Register
         * @see HotKey_Unregister */
        public event HotKeyToggle OnHotKeyToggle;

        /** @brief Delegate for event #OnHotKeyTest.
         * @param nVkCode The virtual key code. Look here for a list of virtual
         * key codes: http://msdn.microsoft.com/en-us/library/ms645540(VS.85).aspx
         * @param bActive TRUE when key is down and FALSE when released. */
        public delegate void HotKeyTest(int nVkCode, bool bActive);

        /**
         * @brief A button was pressed or released on the user's
         * keyboard or mouse.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_HOTKEY_TEST
         * 
         * When #HotKey_InstallTestHook is called a hook is
         * installed in Windows which intercepts all keyboard and
         * mouse presses. Every time a key or mouse is pressed or
         * released this event is posted.
         *
         * Use #HotKey_GetKeyString to get a key description of the 
         * pressed key.
         * 
         * @see HotKeyTest Delegate for this event.
         * @see HotKey_InstallTestHook */
        public event HotKeyTest OnHotKeyTest;

        /** @brief Delegate for event #OnFileTransfer.
         * @param filetransfer Properties 
         * and status information about the file transfer. */
        public delegate void FileTransferUpdate(FileTransfer filetransfer);

        /**
         * @brief A file transfer is processing.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_FILETRANSFER
         *
         * Use TeamTalkBase.GetFileTransferInfo() to get information about the
         * file transfer. Ensure to check if the file transfer is
         * completed, because the file transfer instance will be
         * removed from the client instance when the user application
         * reads the #BearWare.FileTransfer object and it has completed the
         * transfer. */
        public event FileTransferUpdate OnFileTransfer;

        /** 
         * @brief Delegate for event OnDesktopWindowTransfer().
         *
         * @param nSessionID The desktop session's ID. If the desktop session ID
         * becomes 0 it means the desktop session has been closed and/or
         * cancelled.
         * @param nBytesRemain The number of bytes remaining before transmission
         * of last desktop window completes. When remaining bytes is 0 
         * TeamTalkBase.SendDesktopWindow() can be called again. */
        public delegate void DesktopTransferUpdate(int nSessionID, int nBytesRemain);

        /**
         * @brief Used for tracking when a desktop window has been
         * transmitted to the server.
         * 
         * Event handler for #ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER
         *
         * When the transmission has completed the flag
         * #ClientFlag.CLIENT_TX_DESKTOP will be cleared from the
         * local client instance. */
        public event DesktopTransferUpdate OnDesktopWindowTransfer;

        /** @brief Delegate for event #OnStreamMediaFile. */
        public delegate void StreamMediaFile(MediaFileInfo mediafileinfo);

        /**
         * @brief Media file being streamed to a channel is processing.
         *
         * Event handler for #ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE */
        public event StreamMediaFile OnStreamMediaFile;

        /** @} */

        //TTDLL instance
        private IntPtr m_ttInst;
        //TTDLL event handler
        c_tt.MyEventHandler m_eventhandler;
    }

    /** @addtogroup initclient
     * @{ */

    /** @brief Class used to load @c TeamTalk5.dll and instantiate a
     * TeamTalk client in TeamTalk 5 SDK Standard Edition.
     *
     * Section @ref clientsetup outlines the location of the required
     * DLL files. */
    public class TeamTalk5 : TeamTalkBase
    {
        public const int DEFAULT_TCPPORT = 10333;
        public const int DEFAULT_UDPPORT = 10333;

        /** @brief Simply calls TeamTalkBase.TeamTalkBase() */
        public TeamTalk5(bool poll_based)
            : base(poll_based)
        {
        }
    }

    /** @brief Class used to load @c TeamTalk5Pro.dll and instantiate
     * a TeamTalk client in TeamTalk 5 SDK Professional Edition.
     *
     * Section @ref clientsetup outlines the location of the required
     * DLL files. */
    public class TeamTalk5Pro : TeamTalkBase
    {
        public const int DEFAULT_TCPPORT = 10443;
        public const int DEFAULT_UDPPORT = 10443;

        /** @brief Simply calls TeamTalkBase.TeamTalkBase() */
        public TeamTalk5Pro(bool poll_based)
            : base(poll_based)
        {
        }
    }
    /** @} */

    /** @addtogroup desktopshare
     * @{ */

    /**
     * @brief Translate to and from TeamTalk's intermediate key-codes (TTKEYCODE).
     * @see TeamTalkBase.OnUserDesktopInput() */
    public enum TTKeyTranslate : uint
    {
        /** @brief Perform no translation. */
        TTKEY_NO_TRANSLATE                  = 0,
        /** @brief Translate from Windows scan-code to TTKEYCODE. The
         * Windows scan-code can be retrieved in Windows' @c
         * WM_KEYDOWN and @c WM_KEYUP event. */
        TTKEY_WINKEYCODE_TO_TTKEYCODE       = 1,
        /** @brief Translate from TTKEYCODE to Windows scan-code. */
        TTKEY_TTKEYCODE_TO_WINKEYCODE       = 2,
        /** @brief Translate from Mac OS X Carbon @c kVK_* key-code to
         * TTKEYCODE. The Mac OS X key-codes are defined in Carbon's
         * API. */
        TTKEY_MACKEYCODE_TO_TTKEYCODE       = 3,
        /** @brief Translate from TTKEYCODE to Mac OS X Carbon @c
         * kVK_* key-code. */
        TTKEY_TTKEYCODE_TO_MACKEYCODE       = 4
    }

    /** @} */

    /** @addtogroup mixer
     * @{ */

    /**
     * @brief The Windows mixer controls which can be queried by the
     * #BearWare.WindowsMixer class.
     *
     * Wave-In devices which are not in the enum-structure can be
     * accessed by WindowsMixer.GetWaveInControlCount() which allows the user to
     * query selection based on an index.
     *
     * Note that Windows Vista has deprecated mixer controls.
     *
     * @see WindowsMixer.SetWaveOutMute
     * @see WindowsMixer.SetWaveOutVolume
     * @see WindowsMixer.SetWaveInSelected
     * @see WindowsMixer.SetWaveInVolume
     * @see WindowsMixer.GetWaveInControlName
     * @see WindowsMixer.SetWaveInControlSelected */
    public enum MixerControl : uint
    {
        WAVEOUT_MASTER,
        WAVEOUT_WAVE,
        WAVEOUT_MICROPHONE,

        WAVEIN_MICROPHONE,
        WAVEIN_LINEIN,
        WAVEIN_WAVEOUT,
    }

    /**
     * @brief A wrapper for the Windows Mixer API.
     */
    public class WindowsMixer
    {
        /**
         * @brief Get the number of Windows Mixers available.
         *
         * The index from 0 to GetMixerCount() - 1 should be passed to the
         * WindowsMixer.* functions.
         * @see WindowsMixer.GetMixerName */
        public static int GetMixerCount()
        {
            return TTDLL.TT_Mixer_GetMixerCount();
        }
        /**
         * @brief Get the name of a Windows Mixer based on its name.
         *
         * @param nMixerIndex The index of the mixer. Ranging from 0 to 
         * GetMixerCount() - 1.
         * @param szMixerName The output string receiving the name of the device. */
        public static bool GetMixerName(int nMixerIndex,
                                        ref string szMixerName)
        {
            IntPtr ptr = Marshal.AllocHGlobal(TeamTalkBase.TT_STRLEN * 2);
            bool b = TTDLL.TT_Mixer_GetMixerName(nMixerIndex, ptr);
            if(b)
                szMixerName = Marshal.PtrToStringUni(ptr);

            Marshal.FreeHGlobal(ptr);
            return b;
        }
        /**
         * @brief Get the name of the mixer associated with a wave-in device.
         * 
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param szMixerName The output string receiving the name of the device. 
         * @see TeamTalkBase.GetSoundDevices() */
        public static bool GetWaveInName(int nWaveDeviceID,
                                         ref string szMixerName)
        {
            IntPtr ptr = Marshal.AllocHGlobal(TeamTalkBase.TT_STRLEN * 2);
            bool b = TTDLL.TT_Mixer_GetWaveInName(nWaveDeviceID, ptr);
            if(b)
                szMixerName = Marshal.PtrToStringUni(ptr);
            Marshal.FreeHGlobal(ptr);
            return b;
        }
        /**
         * @brief Get the name of the mixer associated with a wave-out device.
         * 
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param szMixerName The output string receiving the name of the device. 
         * @see TeamTalkBase.GetSoundOutputDevices */
        public static bool GetWaveOutName(int nWaveDeviceID,
                                          ref string szMixerName)
        {
            IntPtr ptr = Marshal.AllocHGlobal(TeamTalkBase.TT_STRLEN * 2);
            bool b = TTDLL.TT_Mixer_GetWaveOutName(nWaveDeviceID, ptr);
            if(b)
                szMixerName = Marshal.PtrToStringUni(ptr);

            Marshal.FreeHGlobal(ptr);
            return b;
        }

        /**
         * @brief Mute or unmute a Windows Mixer Wave-Out device from the
         * 'enum' of devices.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param nControl A mixer control.
         * @param bMute True if device should be muted, False if it should be
         * unmuted.
         * @see WindowsMixer.GetWaveOutMute */
        public static bool SetWaveOutMute(int nWaveDeviceID, MixerControl nControl, bool bMute)
        {
            return TTDLL.TT_Mixer_SetWaveOutMute(nWaveDeviceID, nControl, bMute);
        }
        /**
         * @brief Get the mute state of a Windows Mixer Wave-Out device
         * from the 'enum' of devices.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param nControl A mixer control.
         * @return TRUE if mute, FALSE if unmuted, -1 on error.
         * @see WindowsMixer.SetWaveOutMute */
        public static int GetWaveOutMute(int nWaveDeviceID, MixerControl nControl)
        {
            return TTDLL.TT_Mixer_GetWaveOutMute(nWaveDeviceID, nControl);
        }
        /**
         * @brief Set the volume of a Windows Mixer Wave-Out device from
         * the 'enum' of devices.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param nControl A mixer control.
         * @param nVolume A value ranging from 0 to 65535. */
        public static bool SetWaveOutVolume(int nWaveDeviceID, MixerControl nControl, int nVolume)
        {
            return TTDLL.TT_Mixer_SetWaveOutVolume(nWaveDeviceID, nControl, nVolume);
        }
        /**
         * @brief Get the volume of a Windows Mixer Wave-Out device from
         * the 'enum' of devices.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param nControl A mixer control.
         * @return A value ranging from 0 to 65535, or -1 on error. */
        public static int GetWaveOutVolume(int nWaveDeviceID, MixerControl nControl)
        {
            return TTDLL.TT_Mixer_GetWaveOutVolume(nWaveDeviceID, nControl);
        }
        /**
         * @brief Set the selected state of a Windows Mixer Wave-In
         * device from the 'enum' of devices.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param nControl A mixer control. */
        public static bool SetWaveInSelected(int nWaveDeviceID, MixerControl nControl)
        {
            return TTDLL.TT_Mixer_SetWaveInSelected(nWaveDeviceID, nControl);
        }
        /**
         * @brief Get the selected state of a Windows Mixer Wave-In device
         * from the 'enum' of devices.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param nControl A mixer control.
         * @return TRUE if mute, FALSE if unmuted, -1 on error. */
        public static int GetWaveInSelected(int nWaveDeviceID, MixerControl nControl)
        {
            return TTDLL.TT_Mixer_GetWaveInSelected(nWaveDeviceID, nControl);
        }
        /**
         * @brief Set the volume of a Windows Mixer Wave-In device from
         * the 'enum' of devices.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param nControl A mixer control.
         * @param nVolume A value ranging from 0 to 65535. */
        public static bool SetWaveInVolume(int nWaveDeviceID, MixerControl nControl, int nVolume)
        {
            return TTDLL.TT_Mixer_SetWaveInVolume(nWaveDeviceID, nControl, nVolume);
        }
        /**
         * @brief Get the volume of a Windows Mixer Wave-In device from
         * the 'enum' of devices.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param nControl A mixer control.
         * @return A value ranging from 0 to 65535, or -1 on error. */
        public static int GetWaveInVolume(int nWaveDeviceID, MixerControl nControl)
        {
            return TTDLL.TT_Mixer_GetWaveInVolume(nWaveDeviceID, nControl);
        }
        /**
         * @brief Enable and disable microphone boost.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param bEnable TRUE to enable, FALSE to disable. */
        public static bool SetWaveInBoost(int nWaveDeviceID, bool bEnable)
        {
            return TTDLL.TT_Mixer_SetWaveInBoost(nWaveDeviceID, bEnable);
        }
        /**
         * @brief See if microphone boost is enabled.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @return TRUE if boost is enabled, FALSE if disabled, -1 on error. */
        public static int GetWaveInBoost(int nWaveDeviceID)
        {
            return TTDLL.TT_Mixer_GetWaveInBoost(nWaveDeviceID);
        }
        /**
         * @brief Mute/unmute microphone input.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param bEnable TRUE to enable, FALSE to disable. */
        public static bool SetWaveInMute(int nWaveDeviceID, bool bEnable)
        {
            return TTDLL.TT_Mixer_SetWaveInMute(nWaveDeviceID, bEnable);
        }
        /**
         * @brief See if microphone is muted.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @return TRUE if mute is enabled, FALSE if disabled, -1 on error. */
        public static int GetWaveInMute(int nWaveDeviceID)
        {
            return TTDLL.TT_Mixer_GetWaveInMute(nWaveDeviceID);
        }
        /**
         * @brief Get the number of Windows Mixer Wave-In devices.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @see WindowsMixer.GetWaveInControlName
         * @see WindowsMixer.SetWaveInControlSelected
         * @see WindowsMixer.GetWaveInControlSelected */
        public static int GetWaveInControlCount(int nWaveDeviceID)
        {
            return TTDLL.TT_Mixer_GetWaveInControlCount(nWaveDeviceID);
        }
        /**
         * @brief Get the name of the Wave-In device with the specified
         * index.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param nControlIndex The index of the control. Randing from 0 to 
         * GetWaveInControlCount() - 1.
         * @param szDeviceName The output string of the name of the device.
         * @see WindowsMixer.GetWaveInControlCount */
        public static bool GetWaveInControlName(int nWaveDeviceID, int nControlIndex,
                                                ref string szDeviceName)
        {
            IntPtr ptr = Marshal.AllocHGlobal(TeamTalkBase.TT_STRLEN * 2);
            bool b = TTDLL.TT_Mixer_GetWaveInControlName(nWaveDeviceID, nControlIndex, ptr);
            if(b)
                szDeviceName = Marshal.PtrToStringUni(ptr);
            Marshal.FreeHGlobal(ptr);
            return b;
        }
        /**
         * @brief Set the selected state of a Wave-In device in the
         * Windows Mixer.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param nControlIndex The index of the device. Randing from 0 to 
         * GetWaveInControlCount() - 1.
         * @see WindowsMixer.GetWaveInControlCount */
        public static bool SetWaveInControlSelected(int nWaveDeviceID, int nControlIndex)
        {
            return TTDLL.TT_Mixer_SetWaveInControlSelected(nWaveDeviceID, nControlIndex);
        }
        /**
         * @brief Get the selected state of a Wave-In device in the
         * Windows Mixer.
         *
         * @param nWaveDeviceID The @a nWaveDeviceID from the #BearWare.SoundDevice
         * struct.
         * @param nControlIndex The index of the device. Randing from 0 to  
         * GetWaveInControlCount() - 1.
         * @see WindowsMixer.GetWaveInControlCount */
        public static bool GetWaveInControlSelected(int nWaveDeviceID, int nControlIndex)
        {
            return TTDLL.TT_Mixer_GetWaveInControlSelected(nWaveDeviceID, nControlIndex);
        }
    }
    /** @} */

    /** @addtogroup firewall
     * @{ */

    /**
     * @brief A wrapper for the Windows Firewall API.
     */
    public class WindowsFirewall
    {
        /**
         * @brief Check if the Windows Firewall is currently enabled.
         *
         * This function does not invoke UAC on Windows Vista/7.
         * @see WindowsFirewall.Enable() */
        public static bool IsEnabled()
        {
            return TTDLL.TT_Firewall_IsEnabled();
        }
        /**
         * @brief Enable/disable the Windows Firewall.
         *
         * The Windows Firewall was introduced in Windows XP SP2.
         *
         * On Windows XP (SP2+) the user calling this function is assumed
         * to have administrator rights. On Windows Vista/7 UAC is invoked
         * to ask the user for administrator rights.
         * @see WindowsFirewall.IsEnabled() */
        public static bool Enable(bool bEnable)
        {
            return TTDLL.TT_Firewall_Enable(bEnable);
        }
        /**
         * @brief Check if an executable is already in the Windows
         * Firewall exception list.
         *
         * This function does not invoke UAC on Windows Vista/7.
         * @see WindowsFirewall.AddAppException() */
        public static bool AppExceptionExists(string szExecutable)
        {
            return TTDLL.TT_Firewall_AppExceptionExists(szExecutable);
        }
        /**
         * @brief Add an application to the Windows Firewall exception
         * list.
         *
         * On Windows XP (SP2+) the user calling this function is assumed
         * to have administrator rights. On Windows Vista/7 UAC is invoked
         * to ask the user for administrator rights.
         * @see WindowsFirewall.AppExceptionExists()
         * @see WindowsFirewall.RemoveAppException() */
        public static bool AddAppException(string szName, string szExecutable)
        {
            return TTDLL.TT_Firewall_AddAppException(szName, szExecutable);
        }
        /**
         * @brief Remove an application from the Windows Firewall exception
         * list.
         *
         * On Windows XP (SP2+) the user calling this function is assumed
         * to have administrator rights. On Windows Vista/7 UAC is invoked
         * to ask the user for administrator rights.
         * @see WindowsFirewall.AppExceptionExists()
         * @see WindowsFirewall.AddAppException() */
        public static bool RemoveAppException(string szExecutable)
        {
            return TTDLL.TT_Firewall_RemoveAppException(szExecutable);
        }
    }
    /** @} */

    /** ingroup desktopshare
     * @brief A struct which describes the properties of a window
     * which can be shared. */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct ShareWindow
    {
        /** @brief The Windows handle of the window. */
        public IntPtr hWnd;
        /** @brief X coordinate of the window relative to the Windows desktop. */
        public int nWndX;
        /** @brief Y coordinate of the window relative to the Windows desktop. */
        public int nWndY;
        /** @brief The width in pixels of the window. */
        public int nWidth;
        /** @brief The height in pixels of the window. */
        public int nHeight;
        /** @brief The title of the window. */
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = TeamTalkBase.TT_STRLEN)]
        public string szWindowTitle;
    }

    /** ingroup desktopshare
     * @brief Helper class for extracting Windows HWND handles and
     * controlling mouse and keyboard. */
    public class WindowsHelper
    {
        /** @brief Get the handle (HWND) of the window which is currently
         * active (focused) on the Windows desktop. */
        public static System.IntPtr GetDesktopActiveHWND()
        {
            return TTDLL.TT_Windows_GetDesktopActiveHWND();
        }

        /** @brief Get the handle (HWND) of the Windows desktop (full desktop). */
        public static System.IntPtr GetDesktopHWND()
        {
            return TTDLL.TT_Windows_GetDesktopHWND();
        }

        /** @brief Get the all the handles (HWND) of visible windows. */
        public static bool GetDesktopWindowHWND(int nIndex,
                                                ref System.IntPtr lpHWnd)
        {
            return TTDLL.TT_Windows_GetDesktopWindowHWND(nIndex, ref lpHWnd);
        }

        /**
         * @brief Get the properties of a window from its window handle (HWND). */
        public static bool GetWindow(System.IntPtr hWnd,
                                     ref ShareWindow lpShareWindow)
        {
            return TTDLL.TT_Windows_GetWindow(hWnd, ref lpShareWindow);
        }

        /**
         * @brief Translate platform key-code to and from TeamTalk's
         * intermediate format.
         *
         * Section @ref keytranslate has a table which shows how the keys on a US
         * 104-keyboard are translated to TeamTalk's intermediate format.
         *
         * Section @ref transdesktopinput explains how to transmit key-codes.
         *
         * @param nTranslate The key-code format to translate to and from.
         * @param lpDesktopInputs An array of #BearWare.DesktopInput structs to translate.
         * @param lpTranslatedDesktopInputs A pre-allocated array of #BearWare.DesktopInput
         * struct to hold the translated desktop input.
         * @return The number of translated #BearWare.DesktopInput stucts. If value
         * is different from @c nDesktopInputCount then some @c uKeyCode
         * values could not be translated and have been assigned the value
         * #BearWare.DesktopInputConstants.DESKTOPINPUT_KEYCODE_IGNORE.
         * @see TeamTalkBase.SendDesktopInput()
         * @see WindowsHelper.Execute() */
        public static int DesktopInputKeyTranslate(TTKeyTranslate nTranslate,
                                                   DesktopInput[] lpDesktopInputs,
                                                   out DesktopInput[] lpTranslatedDesktopInputs)
        {
            lpTranslatedDesktopInputs = new DesktopInput[lpDesktopInputs.Length];
            return TTDLL.TT_DesktopInput_KeyTranslate(nTranslate, lpDesktopInputs,
                                                    lpTranslatedDesktopInputs,
                                                    lpDesktopInputs.Length);
        }

        /**
         * @brief Execute desktop (mouse or keyboard) input.
         *
         * When executed either a key-press, key-release or mouse move
         * will take place on the computer running the client
         * instance. Remember to calculate the offsets for the mouse
         * cursor prior to this call. The mouse position will be relative
         * to the screen resolution.
         *
         * The content of the #BearWare.DesktopInput struct must been translated to
         * the platform's key-code format prior to this
         * call. I.e. uKeyCode must be a either a Windows scan-code, Mac
         * OS X Carbon key-code or one of the mouse buttons:
         * #BearWare.DesktopInputConstants.DESKTOPINPUT_KEYCODE_LMOUSEBTN,
         * #BearWare.DesktopInputConstants.DESKTOPINPUT_KEYCODE_RMOUSEBTN,
         * #BearWare.DesktopInputConstants.DESKTOPINPUT_KEYCODE_MMOUSEBTN.
         *
         * @param lpDesktopInputs The mouse or keyboard inputs.
         * @return The number of mouse and keyboard events inserted.
         * @see WindowsHelper.KeyTranslate() */
        public static int DesktopInputExecute(DesktopInput[] lpDesktopInputs)
        {
            return TTDLL.TT_DesktopInput_Execute(lpDesktopInputs, lpDesktopInputs.Length);
        }

    }
}
