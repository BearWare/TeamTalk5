#if !defined(TEAMTALKDLL_H)
#define TEAMTALKDLL_H

/*
 * BearWare.dk TeamTalk 5 SDK.
 *
 * Copyright 2005-2018, BearWare.dk.
 *
 * Read the License.txt file included with the TeamTalk 5 SDK for
 * terms of use.
 */

/**
 * @brief Ensure the header and DLL are exactly the same version. To
 * get the version of the loaded DLL call TT_GetVersion(). A remote
 * client's version can be seen in the @a szVersion member of the
 * #User-struct. */

#define TEAMTALK_VERSION "5.5.0.4993"


#if defined(WIN32)

#ifdef TEAMTALKDLL_EXPORTS
#define TEAMTALKDLL_API __declspec(dllexport)
#else
#define TEAMTALKDLL_API __declspec(dllimport)
#endif

#else 

#define TEAMTALKDLL_API

#endif


#ifdef __cplusplus
extern "C" {
#endif

    /* OS specific types. */
#if !defined(TEAMTALK_TYPES)

#define TEAMTALK_TYPES 1

#if defined(WIN32)
    /* If you get a compiler error here you probably forgot to include
     * <windows.h> before this file.  */

    /** @brief TeamTalk uses Unicode on Windows. @see TT_STRLEN */
    typedef WCHAR TTCHAR;
    typedef int TTBOOL;
#else
    typedef char TTCHAR;
    typedef int TTBOOL;
    typedef unsigned short UINT16;
    typedef int INT32;
    typedef long long INT64;
    typedef unsigned int UINT32;
    typedef void VOID;

/** Windows has macros which tell whether a parameter is used as input
 * or output so these are just defined as nothing. */

#define IN
#define OUT

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif

    /** @def TT_STRLEN
     *
     * If a string is passed to the client instance is longer than
     * TT_STRLEN it will be truncated.
     *
     * On Windows the client instance converts unicode characters to
     * UTF-8 before transmission, so be aware of non-ASCII characters
     * if communicating with the TeamTalk server from another
     * applications than the TeamTalk client. */
#define TT_STRLEN 512

    /** @def TT_USERID_MAX
     * The highest user ID */
#define TT_USERID_MAX 0xFFF

    /** @def TT_CHANNELID_MAX
     * The highest channel ID. Also used for #TT_CLASSROOM_FREEFORALL */
#define TT_CHANNELID_MAX 0xFFF

    /** @ingroup videocapture
     * @def TT_VIDEOFORMATS_MAX
     * The maximum number of video formats which will be queried for a 
     * #VideoCaptureDevice. */
#define TT_VIDEOFORMATS_MAX 1024

    /** @ingroup channels
     * @def TT_TRANSMITUSERS_MAX
     * The maximum number of users allowed to transmit when a
     * #Channel is configured with #CHANNEL_CLASSROOM. */
#define TT_TRANSMITUSERS_MAX 128

    /** @ingroup channels
     * @def TT_CLASSROOM_FREEFORALL
     * If a #Channel is configured with #CHANNEL_CLASSROOM then only
     * users certain user IDs are allowed to transmit. If, however, @c
     * TT_CLASSROOM_FREEFORALL is put in @c transmitUsers then
     * everyone in the channel are allowed to transmit. */
#define TT_CLASSROOM_FREEFORALL 0xFFF

     /** @ingroup channels
      * User ID index in @c transmitUsers of #Channel */
#define TT_CLASSROOM_USERID_INDEX 0

      /** @ingroup channels
      * #StreamTypes index in @c transmitUsers of #Channel */
#define TT_CLASSROOM_STREAMTYPE_INDEX 1

    /** @ingroup channels 
     * @def TT_TRANSMITUSERS_FREEFORALL
     * Same as #TT_CLASSROOM_FREEFORALL */
#define TT_TRANSMITUSERS_FREEFORALL 0xFFF

    /** @ingroup channels 
     * @def TT_TRANSMITUSERS_USERID_INDEX
     * Same as #TT_CLASSROOM_USERID_INDEX */
#define TT_TRANSMITUSERS_USERID_INDEX 0

     /** @ingroup channels
     * @def TT_TRANSMITUSERS_STREAMTYPE_INDEX
     * Same as #TT_CLASSROOM_STREAMTYPE_INDEX */
#define TT_TRANSMITUSERS_STREAMTYPE_INDEX 1

    /** @ingroup channels
     * @def TT_CHANNELS_OPERATOR_MAX
     * The maximum number of channels where a user can automatically become
     * channel operator.
     * @see #UserAccount */
#define TT_CHANNELS_OPERATOR_MAX 16

    /** @ingroup channels
     * @def TT_TRANSMITQUEUE_MAX
     * The maximum number of users in a #Channel's transmit queue when channel
     * is configured with #CHANNEL_SOLO_TRANSMIT */
#define TT_TRANSMITQUEUE_MAX 16

    /** @ingroup sounddevices
     * The maximum number of sample rates supported by a #SoundDevice. */
#define TT_SAMPLERATES_MAX 16

/** @ingroup desktopshare
 * @def TT_DESKTOPINPUT_MAX
 *
 * The maximum number #DesktopInput instances which can be sent by
 * TT_SendDesktopInput(). */
#define TT_DESKTOPINPUT_MAX 16

/** @ingroup desktopshare
 * @def TT_DESKTOPINPUT_KEYCODE_IGNORE
 *
 * If @c uKeyCode in #DesktopInput is set to
 * #TT_DESKTOPINPUT_KEYCODE_IGNORE it means no key (or mouse button)
 * was pressed in the desktop input event and
 * TT_DesktopInput_Execute() will ignore the value. */
#define TT_DESKTOPINPUT_KEYCODE_IGNORE  0xFFFFFFFF

/** @ingroup desktopshare
 * @def TT_DESKTOPINPUT_MOUSEPOS_IGNORE
 *
 * If @c uMousePosX or @c uMousePosY in #DesktopInput are set to
 * #TT_DESKTOPINPUT_MOUSEPOS_IGNORE it means the mouse position is
 * ignored when calling TT_DesktopInput_Execute(). */
#define TT_DESKTOPINPUT_MOUSEPOS_IGNORE 0xFFFF

/** @ingroup desktopshare
 * @def TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN
 *
 * If @c uKeyCode of #DesktopInput is set to
 * #TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN then TT_DesktopInput_Execute()
 * will see the key-code as a left mouse button click. */
#define TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN 0x1000

/** @ingroup desktopshare
 * @def TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN
 *
 * If @c uKeyCode of #DesktopInput is set to
 * #TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN then TT_DesktopInput_Execute()
 * will see the key-code as a right mouse button click. */
#define TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN 0x1001

/** @ingroup desktopshare
 * @def TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN
 *
 * If @c uKeyCode of #DesktopInput is set to
 * #TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN then TT_DesktopInput_Execute()
 * will see the key-code as a middle mouse button click. */
#define TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN 0x1002

/** @ingroup mediastream
 * @def TT_MEDIAPLAYBACK_OFFSET_IGNORE
 *
 * Specify this value as uOffsetMSec in #MediaFilePlayback when
 * calling TT_InitLocalPlayback() and TT_UpdateLocalPlayback() to
 * ignore rewind or forward.
 */
#define TT_MEDIAPLAYBACK_OFFSET_IGNORE 0xFFFFFFFF

#endif /* TEAMTALK_TYPES */

    /** @addtogroup sounddevices
     * @{ */

    /**
     * @brief The supported sound systems.
     *
     * @see SoundDevice
     * @see TT_InitSoundInputDevice()
     * @see TT_InitSoundOutputDevice()
     * @see TT_InitSoundDuplexDevices() */
    typedef enum SoundSystem
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
         * #SoundDevice to see which sample rates are supported. */
        SOUNDSYSTEM_WASAPI = 5,
        /** @brief Android sound API.
         *
         * The OpenSL ES sound API requires Android 4.1 or later.
         *
         * Duplex mode is not supported by OpenSL ES @see
         * TT_InitSoundDuplexDevices() */
        SOUNDSYSTEM_OPENSLES_ANDROID = 7,
        /** @brief iOS sound API.
         *
         * The following sound devices will appear when calling
         * TT_GetSoundDevices(). Sound device ID
         * #TT_SOUNDDEVICE_ID_REMOTEIO will be AudioUnit subtype
         * Remote I/O Unit and sound device ID
         * #TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO will be AudioUnit
         * subtype Voice-Processing I/O Unit.
         *
         * Note that iOS only supports one active Voice-Processing I/O
         * Unit, i.e. only one #TTInstance can use the
         * Voice-Processing I/O Unit.
         *
         * Add libraries @c AVFoundation.framework and
         * @c AudioToolbox.framework.
         *
         * Duplex mode is not supported by AudioUnit iOS sound API.
         * @see TT_SOUNDDEVICE_ID_REMOTEIO
         * @see TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO */
        SOUNDSYSTEM_AUDIOUNIT = 8
    } SoundSystem;

    /** 
     * @brief A struct containing the properties of a sound device
     * for either playback or recording.
     *
     * Use @a nDeviceID to pass to #TT_InitSoundInputDevice or
     * #TT_InitSoundOutputDevice.
     *
     * Note that the @a nDeviceID may change if the user application
     * is restarted and a new sound device is added or removed from
     * the computer.
     * 
     * @see TT_GetSoundDevices */
    typedef struct SoundDevice
    {
        /** @brief The ID of the sound device. Used for passing to
         * #TT_InitSoundInputDevice and
         * #TT_InitSoundOutputDevice. Note that @a nDeviceID might change
         * if USB sound devices are plugged in or unplugged, therefore
         * use @a szDeviceID to ensure proper device is used.  */
        INT32 nDeviceID;
        /** @brief The sound system used by the sound device */
        SoundSystem nSoundSystem;
        /** @brief The name of the sound device */
        TTCHAR szDeviceName[TT_STRLEN];
        /** @brief An identifier uniquely identifying the sound device
         * even when new sound devices are being added and removed. In
         * DirectSound, WASAPI and WinMM it would be the GUID of the sound
         * device. Note that it may not always be available. */
        TTCHAR szDeviceID[TT_STRLEN];
#if defined(WIN32)
        /** 
         * @brief The ID of the device used in Win32's
         * waveInGetDevCaps and waveOutGetDevCaps.
         *
         * Value will be -1 if no ID could be found This ID can also
         * be used to find the corresponding mixer on Windows passing
         * it as @a nWaveDeviceID.  Note that this ID applies both to
         * DirectSound and WinMM.
         *
         * @see TT_Mixer_GetWaveInName
         * @see TT_Mixer_GetWaveOutName
         * @see TT_Mixer_GetMixerCount */
        INT32 nWaveDeviceID;
#endif
        /** @brief Whether the sound device supports 3D-sound
         * effects. */
        TTBOOL bSupports3D;
        /** @brief The maximum number of input channels. */
        INT32 nMaxInputChannels;
        /** @brief The maximum number of output channels. */
        INT32 nMaxOutputChannels;
        /** @brief Supported sample rates by device for recording. A
         * zero value terminates the list of supported sample rates or
         * its maximum size of #TT_SAMPLERATES_MAX. */
        INT32 inputSampleRates[TT_SAMPLERATES_MAX];
        /** @brief Supported sample rates by device for playback. A
         * zero value terminates the list of supported sample rates or
         * its maximum size of #TT_SAMPLERATES_MAX. */
        INT32 outputSampleRates[TT_SAMPLERATES_MAX];
        /** @brief The default sample rate for the sound device. */
        INT32 nDefaultSampleRate;
    } SoundDevice;

/**
 * @brief Flag/bit in @c nDeviceID telling if the #SoundDevice is a
 * shared version of an existing sound device.
 *
 * On Android the recording device can only be used by one TeamTalk
 * instance. As a workaround for this issue a shared recording device
 * has been introduced. Internally TeamTalk initializes
 * #TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT which then resample and
 * distribute the audio data to multiple TeamTalk instances.
 *
 * The shared audio device on Android will show up as
 * (TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT | TT_SOUNDDEVICE_ID_SHARED_FLAG),
 * i.e. 2048.
 */
#define TT_SOUNDDEVICE_ID_SHARED_FLAG           0x00000800

/** @brief Extract sound device ID of @c nDeviceID in #SoundDevice by
 * and'ing this value.
 *
 * let PhysicalDeviceID = (SoundDevice.nDeviceID & TT_SOUNDDEVICE_ID_MASK). */
#define TT_SOUNDDEVICE_ID_MASK                  0x000007FF
    
/** @brief Sound device ID for iOS AudioUnit subtype Remote I/O
 * Unit. @see SOUNDSYSTEM_AUDIOUNIT */
#define TT_SOUNDDEVICE_ID_REMOTEIO              0
    
/** @brief Sound device ID for iOS AudioUnit subtype Voice-Processing
 * I/O Unit. @see SOUNDSYSTEM_AUDIOUNIT */
#define TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO  1
    
/** @brief Sound device ID for Android OpenSL ES default audio
 * device. Note that this sound device may also exist in the form
 * where the @c nDeviceID as been or'ed with
 * #TT_SOUNDDEVICE_ID_SHARED_FLAG. @see SOUNDSYSTEM_OPENSLES_ANDROID */
#define TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT      0
    
/** @brief Sound device ID for virtual TeamTalk sound device.
 *
 * This is a sound device which decodes received audio packets but
 * does not send the decoded audio to a real sound device. When used
 * for recording the virtual sound device injects silence.
 *
 * In duplex mode the virtual TeamTalk sound device can only be used
 * as input/output device. @see SOUNDSYSTEM_NONE */
#define TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL      1978

    /**
     * @brief An enum encapsulation the minimum, maximum and default sound
     * levels for input and output sound devices. */
    typedef enum SoundLevel
    {
        /**
         * @brief The maximum value of recorded audio.
         * @see TT_GetSoundInputLevel
         * @see TT_SetVoiceActivationLevel
         * @see TT_GetVoiceActivationLevel */
        SOUND_VU_MAX = 100,
        /**
         * @brief The minimum value of recorded audio.
         * @see TT_GetSoundInputLevel
         * @see TT_SetVoiceActivationLevel
         * @see TT_GetVoiceActivationLevel */
        SOUND_VU_MIN = 0,
        /**
         * @brief The maximum volume.
         *
         * @see TT_SetSoundOutputVolume
         * @see TT_GetSoundOutputVolume
         * @see TT_SetUserVolume
         * @see SOUND_VOLUME_DEFAULT */
        SOUND_VOLUME_MAX = 32000,
        /**
         * @brief The default volume. Use this whenever possible since
         * it requires the least amount of CPU usage.
         *
         * @see TT_SetSoundOutputVolume
         * @see TT_GetSoundOutputVolume
         * @see TT_SetUserVolume */
        SOUND_VOLUME_DEFAULT = 1000,
        /**
         * @brief The minimum volume.
         * @see TT_SetSoundOutputVolume
         * @see TT_GetSoundOutputVolume
         * @see TT_SetUserVolume */
        SOUND_VOLUME_MIN = 0,
        /**
         * @brief The maximum gain level. 
         *
         * A gain level of 32000 gains the volume by a factor 32.  A gain
         * level of #SOUND_GAIN_DEFAULT means no gain.
         *
         * @see TT_SetSoundInputGainLevel
         * @see TT_GetSoundInputGainLevel */
        SOUND_GAIN_MAX = 32000,
        /**
         * @brief The default gain level.
         *
         * A gain level of 1000 means no gain. Check #SOUND_GAIN_MAX
         * and #SOUND_GAIN_MIN to see how to increase and lower gain
         * level.
         *
         * @see TT_SetSoundInputGainLevel
         * @see TT_GetSoundInputGainLevel */
        SOUND_GAIN_DEFAULT = 1000,
        /**
         * @brief The minimum gain level (since it's zero it means
         * silence).
         *
         * A gain level of 100 is 1/10 of the default volume.
         *
         * @see TT_SetSoundInputGainLevel
         * @see TT_GetSoundInputGainLevel */
        SOUND_GAIN_MIN = 0
    } SoundLevel;

    /**
     * @brief An audio block containing the raw audio from a user who
     * was talking.
     *
     * To enable audio blocks first call TT_EnableAudioBlockEvent()
     * then whenever new audio is played the event
     * #CLIENTEVENT_USER_AUDIOBLOCK is generated. Use
     * TT_AcquireUserAudioBlock() to retrieve the audio block.
     *
     * Note that each user is limited to 128 kbytes of audio data.
     *
     * @see TT_EnableAudioBlockEvent()
     * @see TT_AcquireUserAudioBlock()
     * @see TT_ReleaseUserAudioBlock() */
    typedef struct AudioBlock
    {
        /** @brief The ID of the stream. The stream id changes every time
         * the user enables a new transmission using TT_EnableTransmission()
         * or through voice activation. */
        INT32 nStreamID;
        /** @brief The sample rate of the raw audio. */
        INT32 nSampleRate;
        /** @brief The number of channels used (1 for mono, 2 for stereo). */
        INT32 nChannels;
        /** @brief The raw audio in 16-bit integer format array. The
         * size of the array in bytes is @c sizeof(short) * @c
         * nSamples * @c nChannels. */
        VOID* lpRawAudio;
        /** @brief The number of samples in the raw audio array. */
        INT32 nSamples;
        /** @brief The index of the first sample in @c lpRawAudio. Its
         * value will be a multiple of @c nSamples. The sample index
         * can be used to detect overflows of the internal
         * buffer. When a user initially starts talking the @c
         * nSampleIndex will be 0 and while the user is talking @c
         * nSampleIndex will be greater than 0. When the user stops
         * talking @c nSampleIndex will be reset to 0 again. */
        UINT32 uSampleIndex;
    } AudioBlock;

/**
 * @brief User ID passed to TT_EnableAudioBlockEvent() in order to
 * receive #AudioBlock directly from sound input device after joining
 * a channel.
 *
 * When this user ID is passed then the #AudioBlock received will be
 * prior to audio preprocessing (#AudioPreprocessor).
 *
 * Note, however, that #CLIENTEVENT_USER_AUDIOBLOCK will not be
 * triggered until the #TTInstance is in a channel. This is because
 * the sound input device is not started until it knows the
 * #AudioCodec's sample rate, number of channels and transmit
 * interval. */
#define TT_LOCAL_USERID 0

/**
 * @brief User ID used to identify muxed audio that has been mixed
 * into a single stream.
 *
 * This user ID is passed to TT_EnableAudioBlockEvent() in order to
 * receive #AudioBlock of audio that is played in the #TTInstance's
 * channel. */
#define TT_MUXED_USERID 0x1001 /* TT_USERID_MAX + 1 */

    /** @} */

    /** @addtogroup mediastream
     * @{ */

    /**
     * @brief Status of media file being written to disk.
     * @see CLIENTEVENT_USER_RECORD_MEDIAFILE */
    typedef enum MediaFileStatus
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
         * #MediaFileInfo. */
        MFS_PLAYING         = 6
    } MediaFileStatus;

    /**
     * @brief Media file formats supported for muxed audio recordings.
     * @see TT_StartRecordingMuxedAudioFile() */
    typedef enum AudioFileFormat
    {
        /** @brief Used to denote nothing selected. */
        AFF_NONE                 = 0,
        /** @brief Store audio in the same format as the #Channel's
         * configured audio codec.
         *
         * Audio is stored in OGG format.  OGG format is supported by
         * https://www.xiph.org/ogg and can be played using VLC media player
         * http://www.videolan.org
         * 
         * Requires TeamTalk version 5.2.0.4730.
         * @see TT_SetUserMediaStorageDir()
         * @see TT_StartRecordingMuxedAudioFile() */
        AFF_CHANNELCODEC_FORMAT  = 1,
        /** @brief Store in 16-bit wave format. */
        AFF_WAVE_FORMAT          = 2,
        /** @brief Store in MP3-format. */
        AFF_MP3_16KBIT_FORMAT    = 3,
        /** @see #AFF_MP3_16KBIT_FORMAT */
        AFF_MP3_32KBIT_FORMAT    = 4,
        /** @see #AFF_MP3_16KBIT_FORMAT */
        AFF_MP3_64KBIT_FORMAT    = 5,
        /** @see #AFF_MP3_16KBIT_FORMAT */
        AFF_MP3_128KBIT_FORMAT   = 6,
        /** @see #AFF_MP3_16KBIT_FORMAT */
        AFF_MP3_256KBIT_FORMAT   = 7,
    } AudioFileFormat;

    /**
     * @brief Struct describing the audio format used by a
     * media file.
     *
     * @see TT_GetMediaFileInfo()
     * @see MediaFileInfo
     */
    typedef struct AudioFormat
    {
        /** @brief The audio file format, e.g. wave or MP3. */
        AudioFileFormat nAudioFmt;
        /** @brief Sample rate of media file. */
        INT32 nSampleRate;
        /** @brief Channels used by media file, mono = 1, stereo = 2. */
        INT32 nChannels;
    } AudioFormat;

    /** @} */

    /** @addtogroup videocapture
     * @{ */

    /** 
     * @brief The picture format used by a capture device. 
     *
     * @see VideoFormat
     * @see VideoCaptureDevice */
    typedef enum FourCC
    {
        /** @brief Internal use to denote no supported formats. */
        FOURCC_NONE   =   0,
        /** @brief Prefered image format with the lowest bandwidth
         * usage. A 640x480 pixel image takes up 460.800 bytes. */
        FOURCC_I420   = 100,
        /** @brief Image format where a 640x480 pixel images takes up
         * 614.400 bytes. */
        FOURCC_YUY2   = 101,
        /** @brief The image format with the highest bandwidth
         * usage. A 640x480 pixel images takes up 1.228.880 bytes. */
        FOURCC_RGB32  = 102
    } FourCC;

    /** 
     * @brief A struct containing the properties of a video capture
     * format.
     *
     * A struct for holding a supported video capture format by a 
     * #VideoCaptureDevice. */
    typedef struct VideoFormat
    {
        /** @brief The width in pixels of the video device supported
         * video format. */
        INT32 nWidth;
        /** @brief The height in pixels of the video device supported
         * video format. */
        INT32 nHeight;
        /** @brief The numerator of the video capture device's video
         * format. Divinding @a nFPS_Numerator with @a
         * nFPS_Denominator gives the frame-rate. */
        INT32 nFPS_Numerator; 
        /** @brief The denominator of the video capture device's video
         * format. Divinding @a nFPS_Numerator with @a
         * nFPS_Denominator gives the frame-rate.*/
        INT32 nFPS_Denominator; 
        /** @brief Picture format for capturing. */
        FourCC picFourCC; 
    } VideoFormat;

    /**
     * @brief A RGB32 image where the pixels can be accessed directly
     * in an allocated @a frameBuffer.
     *
     * Use TT_AcquireUserVideoCaptureFrame() to acquire a user's image and
     * remember to call TT_ReleaseUserVideoCaptureFrame() when the image has
     * been processed so TeamTalk can release its resources. */
    typedef struct VideoFrame
    {
        /** @brief The width in pixels of the image contained in @a
         * frameBuffer. */
        INT32 nWidth;
        /** @brief The height in pixels of the image contained in @a
         * imageBuffer. */
        INT32 nHeight;
        /** @brief A unique identifier for the frames which are part of the
         * same video sequence. If the stream ID changes it means the
         * frames which are being received are part of a new video sequence
         * and @a nWidth and @a nHeight may have changed. The @a nStreamID
         * will always be a positive integer value.*/
        INT32 nStreamID;
        /** @brief Whether the image acquired is a key-frame. If it is
         * not a key-frame and there has been packet loss or a
         * key-frame has not been acquired prior then the image may
         * look blurred. */
        TTBOOL bKeyFrame;
        /** @brief A buffer allocated internally by client instance. */
        VOID* frameBuffer;
        /** @brief The size in bytes of the buffer allocate in @a
         * frameBuffer. */
        INT32 nFrameBufferSize;
    } VideoFrame;

    /** 
     * @brief A struct containing the properties of a video capture
     * device.
     *
     * The information retrieved from the video capture device is used
     * to initialize the video capture device using the
     * #TT_InitVideoCaptureDevice function.
     * 
     * @see TT_GetVideoCaptureDevices */
    typedef struct VideoCaptureDevice
    {
        /** @brief A string identifying the device. */
        TTCHAR szDeviceID[TT_STRLEN];
        /** @brief The name of the capture device. */
        TTCHAR szDeviceName[TT_STRLEN];
        /** @brief The name of the API used to capture video.
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
        TTCHAR szCaptureAPI[TT_STRLEN];
        /** @brief The supported capture formats. */
        VideoFormat videoFormats[TT_VIDEOFORMATS_MAX];
        /** @brief The number of capture formats available in @a
         * videoFormats array. */
        INT32 nVideoFormatsCount; 
    } VideoCaptureDevice;

    /** @} */

    /** @addtogroup desktopshare
     * @{ */

    /**
     * @brief The bitmap format used for a #DesktopWindow. */
    typedef enum BitmapFormat
    {
        /** @brief Used to denote nothing selected. */
        BMP_NONE            = 0,
        /** @brief The bitmap is a 256-colored bitmap requiring a
         * palette. The default 256 colored palette is the Netscape
         * browser-safe palette. Use TT_Palette_GetColorTable() to
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
    } BitmapFormat;

    /** @brief The protocols supported for transferring a
     * #DesktopWindow.
     *
     * So far only one, UDP-based, protocol is supported. */
    typedef enum DesktopProtocol
    {
        /** @brief Desktop protocol based on ZLIB for image
         * compression and UDP for data transmission. */
        DESKTOPPROTOCOL_ZLIB_1  = 1
    } DesktopProtocol;

    /**
     * @brief A struct containing the properties of a shared desktop window.
     *
     * The desktop window is a description of the bitmap which can be retrieved using 
     * TT_AcquireUserDesktopWindow() or the bitmap which should be transmitted using
     * TT_SendDesktopWindow(). */
    typedef struct DesktopWindow
    {
        /** @brief The width in pixels of the bitmap. */
        INT32 nWidth;
        /** @brief The height in pixels of the bitmap. */
        INT32 nHeight;
        /** @brief The format of the bitmap. */
        BitmapFormat bmpFormat;
        /** @brief The number of bytes for each scan-line in the
         * bitmap. Zero means 4-byte aligned. */
        INT32 nBytesPerLine;
        /** @brief The ID of the session which the bitmap belongs
         * to. If the session ID changes it means the user has started
         * a new session. This e.g. happens if the desktop session has
         * been closed and restart or if the bitmap has been
         * resized. Set @c nSessionID to 0 if the desktop window is
         * used with TT_SendDesktopWindow(). */
        INT32 nSessionID;
        /** @brief The desktop protocol used for transmitting the desktop window. */
        DesktopProtocol nProtocol;
        /** @brief A buffer pointing to the bitmap data (often refered to as Scan0). */
        VOID* frameBuffer;
        /** @brief The size in bytes of the buffer allocate in @a
         * frameBuffer. Typically @c nBytesPerLine * @c nHeight. */
        INT32 nFrameBufferSize;
    } DesktopWindow;

    /**
     * @brief The state of a key (or mouse button), i.e. if it's
     * pressed or released. @see DesktopInput */
    typedef enum DesktopKeyState
    {
        /** @brief The key is ignored. */
        DESKTOPKEYSTATE_NONE       = 0x00000000,
        /** @brief The key is pressed. */
        DESKTOPKEYSTATE_DOWN       = 0x00000001,
        /** @brief The key is released. */
        DESKTOPKEYSTATE_UP         = 0x00000002,
    } DesktopKeyState;

    /** @brief Mask of key states.  */
    typedef UINT32 DesktopKeyStates;

    /**
     * @brief A struct containing a mouse or keyboard event.
     *
     * The DesktopInput struct is used for desktop access where a
     * remote user can control mouse or keybaord on a shared
     * desktop. Check out section @ref desktopinput on how to use
     * remote desktop access. */
    typedef struct DesktopInput
    {
        /** @brief The X coordinate of the mouse. If used with
         * TT_DesktopInput_Execute() and the mouse position should be
         * ignored then set to #TT_DESKTOPINPUT_MOUSEPOS_IGNORE. */
        UINT16 uMousePosX;
        /** @brief The Y coordinate of the mouse. If used with
         * TT_DesktopInput_Execute() and the mouse position should be
         * ignored then set to #TT_DESKTOPINPUT_MOUSEPOS_IGNORE. */
        UINT16 uMousePosY;
        /** @brief The key-code (or mouse button) pressed. If used
         * with TT_DesktopInput_Execute() and no key (or mouse button)
         * is pressed then set to #TT_DESKTOPINPUT_KEYCODE_IGNORE.
         * Read section @ref transdesktopinput on issues with
         * key-codes and keyboard settings. */
        UINT32 uKeyCode;
        /** @brief The state of the key (or mouse button) pressed,
         * i.e. if it's up or down. */
        DesktopKeyStates uKeyState;
    } DesktopInput;

    /** @} */

    /** @addtogroup codecs
     * @{ */

    /** @brief Speex audio codec settings for Constant Bitrate mode
     * (CBR). @see SpeexVBRCodec */
    typedef struct SpeexCodec
    {
        /** @brief Set to 0 for 8 KHz (narrow band), set to 1 for 16 KHz 
         * (wide band), set to 2 for 32 KHz (ultra-wide band). */
        INT32 nBandmode;
        /** @brief A value from 1-10. As of DLL version 4.2 also 0 is
         * supported.*/
        INT32 nQuality;
        /** @brief Milliseconds of audio data before each
         * transmission.
         *
         * Speex uses 20 msec frame sizes. Recommended is 40 msec. Min
         * is 20, max is 500 msec.
         *
         * The #SoundSystem must be able to process audio packets at
         * this interval. In most cases this makes less than 40 msec
         * transmission interval unfeasible. */
        INT32 nTxIntervalMSec;
        /** @brief Playback should be done in stereo. Doing so will
         * disable 3d-positioning.
         *
         * @see TT_SetUserPosition
         * @see TT_SetUserStereo */
        TTBOOL bStereoPlayback;
    } SpeexCodec;

    /** @brief Speex audio codec settings for Variable Bitrate mode
     * (VBR). */
    typedef struct SpeexVBRCodec
    {
        /** @brief Set to 0 for 8 KHz (narrow band), set to 1 for 16 KHz 
         * (wide band), set to 2 for 32 KHz (ultra-wide band). */
        INT32 nBandmode;
        /** @brief A value from 0-10. If @c nBitRate is non-zero it
         * will override this value. */
        INT32 nQuality;
        /** @brief The bitrate at which the audio codec should output
         * encoded audio data. Dividing it by 8 gives roughly the
         * number of bytes per second used for transmitting the
         * encoded data. For limits check out #SPEEX_NB_MIN_BITRATE,
         * #SPEEX_NB_MAX_BITRATE, #SPEEX_WB_MIN_BITRATE,
         * #SPEEX_WB_MAX_BITRATE, #SPEEX_UWB_MIN_BITRATE and
         * #SPEEX_UWB_MAX_BITRATE. Note that specifying @c nBitRate
         * will override nQualityVBR. */
        INT32 nBitRate;
        /** @brief The maximum bitrate at which the audio codec is
         * allowed to output audio. Set to zero if it should be
         * ignored. */
        INT32 nMaxBitRate;
        /** @brief Enable/disable discontinuous transmission. When
         * enabled Speex will ignore silence, so the bitrate will
         * become very low. */
        TTBOOL bDTX;
        /** @brief Milliseconds of audio data before each transmission.
         *
         * Speex uses 20 msec frame sizes. Recommended is 40 msec. Min
         * is 20, max is 500 msec.
         *
         * The #SoundSystem must be able to process audio packets at
         * this interval. In most cases this makes less than 40 msec
         * transmission interval unfeasible. */
        INT32 nTxIntervalMSec;
        /** @brief Playback should be done in stereo. Doing so will
         * disable 3d-positioning.
         *
         * @see TT_SetUserPosition
         * @see TT_SetUserStereo */
        TTBOOL bStereoPlayback; 
    } SpeexVBRCodec;

/** @brief The minimum bitrate for Speex codec in 8 KHz mode. Bandmode
 * = 0. */
#define SPEEX_NB_MIN_BITRATE 2150
/** @brief The maximum bitrate for Speex codec in 8 KHz mode. Bandmode
 * = 0. */
#define SPEEX_NB_MAX_BITRATE 24600
/** @brief The minimum bitrate for Speex codec in 16 KHz
 * mode. Bandmode = 1. */
#define SPEEX_WB_MIN_BITRATE 3950
/** @brief The maximum bitrate for Speex codec in 16 KHz
 * mode. Bandmode = 1. */
#define SPEEX_WB_MAX_BITRATE 42200
/** @brief The minimum bitrate for Speex codec in 32 KHz
 * mode. Bandmode = 2. */
#define SPEEX_UWB_MIN_BITRATE 4150
/** @brief The maximum bitrate for Speex codec in 32 KHz
 * mode. Bandmode = 2. */
#define SPEEX_UWB_MAX_BITRATE 44000

    /** @brief OPUS audio codec settings. For detailed information
     * about the OPUS codec check out http://www.opus-codec.org */
    typedef struct OpusCodec
    {
        /** @brief The sample rate to use. Sample rate must be
         * 8000, 12000, 16000, 24000 or 48000 Hz. */
        INT32 nSampleRate;
        /** @brief Mono = 1 or stereo = 2. */
        INT32 nChannels;
        /** @brief Application of encoded audio, i.e. VoIP or music.
         * @see OPUS_APPLICATION_VOIP
         * @see OPUS_APPLICATION_AUDIO */
        INT32 nApplication;
        /** @brief Complexity of encoding (affects CPU usage).
         * Value from 0-10. */
        INT32 nComplexity;
        /** @brief Forward error correction. 
         * Corrects errors if there's packetloss. */
        TTBOOL bFEC;
        /** @brief Discontinuous transmission.
         * Enables "null" packets during silence. */
        TTBOOL bDTX;
        /** @brief Bitrate for encoded audio. Should be between
         * #OPUS_MIN_BITRATE and #OPUS_MAX_BITRATE. */
        INT32 nBitRate;
        /** @brief Enable variable bitrate. */
        TTBOOL bVBR;
        /** @brief Enable constrained VBR.
         * @c bVBR must be enabled to enable this. */
        TTBOOL bVBRConstraint;
        /** @brief Duration of audio before each transmission. Minimum is 2 msec.
         * Recommended is 40 msec. Maximum is 500 msec.
         * 
         * The #SoundSystem must be able to process audio packets at
         * this interval. In most cases this makes less than 40 msec
         * transmission interval unfeasible. */
        INT32 nTxIntervalMSec;
        /** @brief OPUS supports 2.5, 5, 10, 20, 40, 60, 80, 100 and 120 msec.
         * If @c nFrameSizeMSec is 0 then @c nFrameSizeMSec will be same as 
         * @c nTxIntervalMSec. */
        INT32 nFrameSizeMSec;
    } OpusCodec;

/** @brief Audio encoding is for VoIP. This value should be set as
 * nApplicaton in #OpusCodec. */
#define OPUS_APPLICATION_VOIP 2048
/** @brief Audio encoding is for music.  This value should be set as
 * nApplicaton in #OpusCodec. */
#define OPUS_APPLICATION_AUDIO 2049
/** @brief The minimum bitrate for OPUS codec. Checkout @c nBitRate of
 * #OpusCodec. */
#define OPUS_MIN_BITRATE 6000
/** @brief The maximum bitrate for OPUS codec. Checkout @c nBitRate of
 * #OpusCodec. */
#define OPUS_MAX_BITRATE 510000
/** @brief The minimum frame size for OPUS codec. Checkout @c nFrameSizeMSec
 * of #OpusCodec. */
#define OPUS_MIN_FRAMESIZE 2 /* Actually it's 2.5 */
/** @brief The maximum frame size for OPUS codec. Checkout @c nFrameSizeMSec
 * of #OpusCodec. */
#define OPUS_MAX_FRAMESIZE 60
/** @brief The real maximum frame size for OPUS codec. Checkout @c nFrameSizeMSec
 * of #OpusCodec. Although OPUS states it only supports 2.5 - 60 msec, it actually
 * support up to 120 msec. */
#define OPUS_REALMAX_FRAMESIZE 120

    /** @brief Speex DSP is used for specifying how recorded audio
     * from a sound input device should be preprocessed before
     * transmission.
     *
     * Users' audio levels may be diffent due to how their microphone
     * is configured in their OS. Automatic Gain Control (AGC) can be
     * used to ensure all users in the same channel have the same
     * audio level.
     *
     * Enable the preprocessing configuration by calling
     * TT_SetSoundInputPreprocess().
     *
     * When joining a #Channel and @c bEnableGainControl of
     * #AudioConfig is enabled in the channel then enable sound input
     * preprocessing by setting @c bEnableAGC to TRUE and @c
     * nGainLevel of #SpeexDSP to the @c nGainLevel of
     * #AudioConfig. */
    typedef struct SpeexDSP
    {
        /** @brief Whether to enable AGC with the settings specified
         * @a nGainLevel, @a nMaxIncDBSec, @a nMaxDecDBSec and @a
         * nMaxGainDB.
         * 
         * Note that AGC is not supported on ARM (iOS and Android),
         * since there's no fixed point implementation of AGC in
         * SpeexDSP. */
        TTBOOL bEnableAGC;
        /** @brief A value from 0 to 32768. Default is 8000.
         * Value is ignored if @a bEnableAGC is FALSE. */
        INT32 nGainLevel;
        /** @brief Used so volume should not be amplified too quickly 
         * (maximal gain increase in dB/second). Default is 12. 
         * * Value is ignored if @a bEnableAGC is FALSE. */
        INT32 nMaxIncDBSec;
        /** @brief Used so volume should not be attenuated
         * too quickly (maximal gain decrease in dB/second).
         * Negative value! Default is -40.
         * Value is ignored if @a bEnableAGC is FALSE. */
        INT32 nMaxDecDBSec;
        /** @brief Ensure volume doesn't become too loud (maximal gain
         * in dB). Default is 30.
         * Value is ignored if @a bEnableAGC is FALSE. */
        INT32 nMaxGainDB;
        /** @brief Whether clients who join the channel should automatically
         * enable denoising. */
        TTBOOL bEnableDenoise;
        /** @brief Maximum attenuation of the noise in dB.
         * Negative value! Default value is -30. 
         * Value is ignored if @a bEnableDenoise is FALSE. */
        INT32 nMaxNoiseSuppressDB;
        /** @brief Enable/disable acoustic echo cancellation (AEC).
         *
         * In order to enable echo cancellation mode the local client
         * instance must first be set in sound duplex mode by calling
         * TT_InitSoundDuplexDevices(). This is because the echo canceller
         * must first mixed all audio streams into a single stream and
         * have then run in synch with the input stream. After calling
         * TT_InitSoundDuplexDevices() the flag #CLIENT_SNDINOUTPUT_DUPLEX
         * will be set.
         *
         * For echo cancellation to work the sound input and output device
         * must be the same sound card since the input and output stream
         * must be completely synchronized. Also it is recommended to also
         * enable denoising and AGC for better echo cancellation. */
        TTBOOL bEnableEchoCancellation;
        /** @brief Set maximum attenuation of the residual echo in dB 
         * (negative number). Default is -40.
         * Value is ignored if @a bEnableEchoCancellation is FALSE. */
        INT32 nEchoSuppress;
        /** @brief Set maximum attenuation of the residual echo in dB 
         * when near end is active (negative number). Default is -15.
         * Value is ignored if @a bEnableEchoCancellation is FALSE. */
        INT32 nEchoSuppressActive;
    } SpeexDSP;

    /** @brief Use TeamTalk's internal audio preprocessor for gain
     * audio. Same as used for TT_SetSoundInputGainLevel(). */
    typedef struct TTAudioPreprocessor
    {
        /** @brief Gain level between #SOUND_GAIN_MIN and
         * #SOUND_GAIN_MAX. Default is #SOUND_GAIN_DEFAULT (no
         * gain). */
        INT32 nGainLevel;
        /** @brief Whether to mute left speaker in stereo playback. */
        TTBOOL bMuteLeftSpeaker;
        /** @brief Whether to mute right speaker in stereo playback. */
        TTBOOL bMuteRightSpeaker;
    } TTAudioPreprocessor;


    /** @brief The types of supported audio preprocessors.
     *
     * @see TT_InitLocalPlayback() */
    typedef enum AudioPreprocessorType
    {
        /** @brief Value for specifying that no audio preprocessing
         * should occur. */
        NO_AUDIOPREPROCESSOR        = 0,
        /** @brief Use the #SpeexDSP audio preprocessor. */
        SPEEXDSP_AUDIOPREPROCESSOR  = 1,
        /** @brief Use TeamTalk's internal audio preprocessor #TTAudioPreprocessor. */
        TEAMTALK_AUDIOPREPROCESSOR  = 2,
    } AudioPreprocessorType;

    /** @brief Configure the audio preprocessor specified by @c nPreprocessor. */
    typedef struct AudioPreprocessor
    {
        /** @brief The audio preprocessor to use in the union of audio preprocessors. */
        AudioPreprocessorType nPreprocessor;
        union
        {
            /** @brief Used when @c nPreprocessor is #SPEEXDSP_AUDIOPREPROCESSOR. */
            SpeexDSP speexdsp;
            /** @brief Used when @c nPreprocessor is #TEAMTALK_AUDIOPREPROCESSOR. */
            TTAudioPreprocessor ttpreprocessor;
        };
    } AudioPreprocessor;
    
    /** @brief WebM video codec settings. 
     * @see VideoCodec
     * @see TT_InitVideoCaptureDevice
     * @see TT_StartStreamingMediaFileToChannel() */
    typedef struct WebMVP8Codec
    {
        union
        {
            /** @brief Same as @c rc_target_bitrate */
            INT32 nRcTargetBitrate;
            /** @brief Target bitrate in kbits/sec. This value must be
             * greater than 0. 
             *
             * Same as 'rc_target_bitrate' in 'vpx_codec_enc_cfg_t'.
             * http://www.webmproject.org/docs/webm-sdk/ */
            UINT32 rc_target_bitrate;
        };
        /** @brief Time that should be spent on encoding a frame.
         *
         * Same as 'deadline' parameter in 'vpx_codec_encode()'.
         * http://www.webmproject.org/docs/webm-sdk/
         * 
         * Supported values are VPX_DL_REALTIME = 1, VPX_DL_GOOD_QUALITY = 1000000,
         * and VPX_DL_BEST_QUALITY = 0. */
        UINT32 nEncodeDeadline;
    } WebMVP8Codec;

/** @brief @c nEncodeDeadline value for fastest encoding.
 * @see WebMVP8Codec */
#define WEBM_VPX_DL_REALTIME 1
/** @brief @c nEncodeDeadline value for good encoding.
 * @see WebMVP8Codec */
#define WEBM_VPX_DL_GOOD_QUALITY 1000000
/** @brief @c nEncodeDeadline value for best encoding.
 * @see WebMVP8Codec */
#define WEBM_VPX_DL_BEST_QUALITY 0

    /** @brief The codecs supported.
     * @see AudioCodec
     * @see VideoCodec */
    typedef enum Codec
    {
        /** @brief No codec specified. */
        NO_CODEC                    = 0,
        /** @brief Speex audio codec, http://www.speex.org @see
         * SpeexCodec */
        SPEEX_CODEC                 = 1,
        /** @brief Speex audio codec in VBR mode, http://www.speex.org
         * @see SpeexVBRCodec */
        SPEEX_VBR_CODEC             = 2,
        /** @brief OPUS audio codec. @see OpusCodec */
        OPUS_CODEC                  = 3,
        /** @brief WebM video codec. @see WebMVP8Codec */
        WEBM_VP8_CODEC              = 128,
    } Codec;

    /** @brief Struct used for specifying which audio codec a channel
     * uses. */
    typedef struct AudioCodec
    {
        /** @brief Specifies whether the member @a speex, @a speex_vbr or
         * @a opus holds the codec settings. */
        Codec nCodec;  
        union
        {
            /** @brief Speex codec settings if @a nCodec is
             * #SPEEX_CODEC */
            SpeexCodec speex;
            /** @brief Speex codec settings if @a nCodec is
             * #SPEEX_VBR_CODEC */
            SpeexVBRCodec speex_vbr;
            /** @brief Opus codec settings if @a nCodec is
             * OPUS_CODEC */
            OpusCodec opus;
        };
    } AudioCodec;

    /** @brief Audio configuration for clients in a channel.
     *
     * An audio configuration can be used to set common audio
     * properties for all users in a channel. Checkout @c audiocfg of
     * #Channel.
     *
     * The audio configuration only supports same audio level
     * for all users by manually converting the values to the
     * #SpeexDSP preprocessor.
     *
     * @see TT_SetSoundInputPreprocess()
     * @see TT_DoMakeChannel()
     * @see TT_DoJoinChannel() */
    typedef struct AudioConfig
    {
        /** @brief Users should enable automatic gain control. */
        TTBOOL bEnableAGC;
        /** @brief Reference gain level to be used by all users. */
        INT32 nGainLevel;
    } AudioConfig;

    /** @brief Struct used for specifying the video codec to use. */
    typedef struct VideoCodec
    {
        /** @brief Specifies member holds the codec settings. So far
         * there is only one video codec to choose from, namely @c
         * webm_vp8. */
        Codec nCodec;  
        union
        {
            WebMVP8Codec webm_vp8;
        };
    } VideoCodec;
    /** @} */

    /** @addtogroup mediastream
     * @{ */

    /**
     * @brief Struct describing the audio and video format used by a
     * media file.
     *
     * @see TT_GetMediaFile() */
    typedef struct MediaFileInfo
    {
        /** @brief Status of media file if it's being saved to
         * disk. */
        MediaFileStatus nStatus;
        /** @brief Name of file. */
        TTCHAR szFileName[TT_STRLEN];
        /** @brief The audio properties of the media file. */
        AudioFormat audioFmt;
        /** @brief The video properties of the media file. */
        VideoFormat videoFmt;
        /** @brief The duration of the media file in miliseconds. */
        UINT32 uDurationMSec;
        /** @brief The elapsed time of the media file in miliseconds. */
        UINT32 uElapsedMSec;
    } MediaFileInfo;

    /**
     * @brief Properties for initializing or updating a file for media
     * streaming.
     *
     * @see TT_InitLocalPlayback()
     * @see TT_UpdateLocalPlayback()
     * @see TT_StartStreamingMediaFileToChannelEx()
     * @see TT_UpdateStreamingMediaFileToChannel() */
    typedef struct MediaFilePlayback
    {
        /** @brief Offset in milliseconds in the media file where to
         * start playback. Pass -1 (0xffffffff) to ignore this value when 
         * using TT_UpdateLocalPlayback() or TT_UpdateStreamingMediaFileToChannel().
         * @c uOffsetMSec must be less than @c uDurationMSec in #MediaFileInfo. */
        UINT32 uOffsetMSec;
        /** @brief Start or pause media file playback. */
        TTBOOL bPaused;
        /** @brief Option to activate audio preprocessor on local media file playback. */
        AudioPreprocessor audioPreprocessor;
    } MediaFilePlayback;

    /** @brief The progress of the audio currently being processed as
     * audio input.  @see TT_InsertAudioBlock() */
    typedef struct AudioInputProgress
    {
        /** @brief The stream ID provided in the #AudioBlock. */
        INT32 nStreamID;
        /** @brief The duration of the audio currently queued for
         * transmission. */
        UINT32 uQueueMSec;
        /** @brief The duration of the audio that has been
         * transmitted. */
        UINT32 uElapsedMSec;
    } AudioInputProgress;

    /** @} */
    
    /** @addtogroup transmission
     * @{ */

    /** @brief The types of streams which are available for
     * transmission. */
    typedef enum StreamType
    {
        /** @brief No stream. */
        STREAMTYPE_NONE                     = 0x00000000,
        /** @brief Voice stream type which is audio recorded from a
         * sound input device. @see TT_InitSoundInputDevice() */
        STREAMTYPE_VOICE                    = 0x00000001,
        /** @brief Video capture stream type which is video recorded
         * from a webcam. @see TT_InitVideoCaptureDevice() */
        STREAMTYPE_VIDEOCAPTURE             = 0x00000002,
        /** @brief Audio stream type from a media file which is being
         * streamed. @see TT_StartStreamingMediaFileToChannel() */
        STREAMTYPE_MEDIAFILE_AUDIO          = 0x00000004,
        /** @brief Video stream type from a media file which is being
         * streamed. @see TT_StartStreamingMediaFileToChannel() */
        STREAMTYPE_MEDIAFILE_VIDEO          = 0x00000008,
        /** @brief Desktop window stream type which is a window (or
         * bitmap) being transmitted. @see TT_SendDesktopWindow() */
        STREAMTYPE_DESKTOP                  = 0x00000010,
        /** @brief Desktop input stream type which is keyboard or
         * mouse input being transmitted. @see
         * TT_SendDesktopInput() */
        STREAMTYPE_DESKTOPINPUT             = 0x00000020,
        /** @brief Shortcut to allow both audio and video media files. */
        STREAMTYPE_MEDIAFILE                = STREAMTYPE_MEDIAFILE_AUDIO |
                                              STREAMTYPE_MEDIAFILE_VIDEO,

        /** @brief Shortcut to allow voice, media files, desktop and webcamera. */
        STREAMTYPE_CLASSROOM_ALL            = STREAMTYPE_VOICE |
                                              STREAMTYPE_VIDEOCAPTURE |
                                              STREAMTYPE_DESKTOP |
                                              STREAMTYPE_MEDIAFILE,
    } StreamType;

    /** @brief Mask of #StreamType. */
    typedef UINT32 StreamTypes;

    /** @} */

    /** @addtogroup server
     * @{ */

    /** 
     * @brief The rights users have once they have logged on to the
     * server.
     *
     * #ServerProperties holds the user rights in its \a uUserRights
     * member variable and is retrieved by calling
     * #TT_GetServerProperties once connected to the server.
     *
     * @see ServerProperties
     * @see TT_GetServerProperties */
    typedef enum UserRight
    {
        /** @brief Users who log onto the server has none of the
          * rights below. */
        USERRIGHT_NONE                      = 0x00000000, 
        /** @brief Allow multiple users to log on to the server with
         * the same #UserAccount. @see TT_DoLogin() */
        USERRIGHT_MULTI_LOGIN               = 0x00000001,
        /** @brief User can see users in all other channels. */
        USERRIGHT_VIEW_ALL_USERS            = 0x00000002,
        /** @brief User is allowed to create temporary channels which
         * disappear when last user leaves the channel.
         * @see TT_DoJoinChannel() */ 
        USERRIGHT_CREATE_TEMPORARY_CHANNEL  = 0x00000004,
        /** @brief User is allowed to create permanent channels which
         * are stored in the server's configuration file.
         * @see TT_DoMakeChannel() */ 
        USERRIGHT_MODIFY_CHANNELS           = 0x00000008,
        /** @brief User can broadcast text message of type 
         * #MSGTYPE_BROADCAST to all users. */
        USERRIGHT_TEXTMESSAGE_BROADCAST     = 0x00000010,
        /** @brief User can kick users off the server. @see TT_DoKickUser() */
        USERRIGHT_KICK_USERS                = 0x00000020,
        /** @brief User can add and remove banned users.
         * @see TT_DoBanUser() @see TT_DoListBans() */
        USERRIGHT_BAN_USERS                 = 0x00000040,
        /** @brief User can move users from one channel to another.
         * @see TT_DoMoveUser() */
        USERRIGHT_MOVE_USERS                = 0x00000080,
        /** @brief User can make other users channel operator.
         * @see TT_DoChannelOp() */
        USERRIGHT_OPERATOR_ENABLE           = 0x00000100,
        /** @brief User can upload files to channels. @see TT_DoSendFile() */
        USERRIGHT_UPLOAD_FILES              = 0x00000200,
        /** @brief User can download files from channels. 
         * @see TT_DoRecvFile() */
        USERRIGHT_DOWNLOAD_FILES            = 0x00000400,
        /** @brief User can update server properties.
         * @see TT_DoUpdateServer() */
        USERRIGHT_UPDATE_SERVERPROPERTIES   = 0x00000800,
        /** @brief Users are allowed to forward audio packets through
         * server. TT_EnableVoiceTransmission() */
        USERRIGHT_TRANSMIT_VOICE            = 0x00001000, 
        /** @brief User is allowed to forward video packets through
         * server. TT_StartVideoCaptureTransmission() */
        USERRIGHT_TRANSMIT_VIDEOCAPTURE     = 0x00002000,
        /** @brief User is allowed to forward desktop packets through
         * server. @see TT_SendDesktopWindow() */
        USERRIGHT_TRANSMIT_DESKTOP          = 0x00004000,
        /** @brief User is allowed to forward desktop input packets through
         * server. @see TT_SendDesktopInput() */
        USERRIGHT_TRANSMIT_DESKTOPINPUT     = 0x00008000,
        /** @brief User is allowed to stream audio files to channel.
         * @see TT_StartStreamingMediaFileToChannel() */
        USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO  = 0x00010000,
        /** @brief User is allowed to stream video files to channel.
         * @see TT_StartStreamingMediaFileToChannel() */
        USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO  = 0x00020000,
        /** @brief User is allowed to stream media files to channel.
         * @see TT_StartStreamingMediaFileToChannel() */
        USERRIGHT_TRANSMIT_MEDIAFILE = USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO | USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO,
        /** @brief User's nick name is locked.
         * TT_DoChangeNickname() cannot be used and TT_DoLogin() 
         * will ignore szNickname parameter. 
         * @see TT_DoLogin()
         * @see TT_DoLoginEx()
         * @see TT_DoChangeNickname() */
        USERRIGHT_LOCKED_NICKNAME           = 0x00040000,
        /** @brief User's status is locked. TT_DoChangeStatus()
        * cannot be used. */
        USERRIGHT_LOCKED_STATUS             = 0x00080000,
        /** @brief User can record voice in all channels. Even channels
         * with #CHANNEL_NO_RECORDING. */
        USERRIGHT_RECORD_VOICE              = 0x00100000,
    } UserRight;

    /** 
     * @brief A bitmask based on #UserRight for holding the rights users 
     * have who log on the server.
     * @see ServerProperties */
    typedef UINT32 UserRights;

    /** 
     * @brief A struct containing the properties of the server's
     * settings.
     *
     * The server properties is available after a successful call to
     * #TT_DoLogin
     *
     * @see TT_DoUpdateServer
     * @see TT_GetServerProperties 
     * @see TT_Login
     * @see UserRight */
    typedef struct ServerProperties
    {
        /** @brief The server's name. */
        TTCHAR szServerName[TT_STRLEN];
        /** @brief The message of the day. Read-only property. Use @c szMOTDRaw
         *  to update this property.*/
        TTCHAR szMOTD[TT_STRLEN];
        /** @brief The message of the day including variables. The result of the
         * szMOTDRaw string will be displayed in @c szMOTD.
         * When updating the MOTD the variables %users% (number of users), 
         * %admins% (number
         * of admins), %uptime% (hours, minutes and seconds the server has
         * been online), %voicetx% (KBytes transmitted), %voicerx% (KBytes
         * received) and %lastuser% (nickname of last user to log on to the
         * server) as part of the MOTD. */
        TTCHAR szMOTDRaw[TT_STRLEN];
        /** @brief The maximum number of users allowed on the server. A user
         * with admin account can ignore this. */
        INT32 nMaxUsers;
        /** @brief The maximum number of logins with wrong password before
         * banning user's IP-address. */
        INT32 nMaxLoginAttempts;
        /** @brief The maximum number of users allowed to log in with the same
         * IP-address. 0 means disabled. */
        INT32 nMaxLoginsPerIPAddress;
        /** @brief The maximum number of bytes per second which the server 
         * will allow for voice packets. If this value is exceeded the server
         * will start dropping audio packets. 0 = disabled. */
        INT32 nMaxVoiceTxPerSecond;
        /** @brief The maximum number of bytes per second which the
         * server will allow for video input packets. If this value is
         * exceeded the server will start dropping video packets. 0 =
         * disabled. */
        INT32 nMaxVideoCaptureTxPerSecond;
        /** @brief The maximum number of bytes per second which the server 
         * will allow for media file packets. If this value is exceeded the server
         * will start dropping media file packets. 0 = disabled. */
        INT32 nMaxMediaFileTxPerSecond;
        /** @brief The maximum number of bytes per second which the server 
         * will allow for desktop packets. If this value is exceeded the server
         * will start dropping desktop packets. 0 = disabled. */
        INT32 nMaxDesktopTxPerSecond;
        /** @brief The amount of bytes per second which the server 
         * will allow for packet forwarding.  If this value is exceeded the server
         * will start dropping packets. 0 = disabled. */
        INT32 nMaxTotalTxPerSecond;
        /** @brief Whether the server automatically saves changes */
        TTBOOL bAutoSave;
        /** @brief The server's TCP port. */
        INT32 nTcpPort;
        /** @brief The server's UDP port. */
        INT32 nUdpPort;
        /** @brief The number of seconds before a user who hasn't
         * responded to keepalives will be kicked off the server. */
        INT32 nUserTimeout;
        /** @brief The server version. Read-only property. */
        TTCHAR szServerVersion[TT_STRLEN];
        /** @brief The version of the server's protocol. Read-only 
         * property. */
        TTCHAR szServerProtocolVersion[TT_STRLEN];
        /** @brief Number of msec before an IP-address can make
         * another login attempt. If less than this amount then
         * TT_DoLogin() will result in
         * #CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED. Zero means
         * disabled.
         * 
         * Also checkout @c nMaxLoginAttempts and @c
         * nMaxLoginsPerIPAddress. */
        INT32 nLoginDelayMSec;
        /** @brief A randomly generated 256 bit access token created
         * by the server to identify the login session.
         *
         * The szAccessToken is available after receiving
         * #CLIENTEVENT_CON_SUCCESS.
         *
         * Read-only property. */
        TTCHAR szAccessToken[TT_STRLEN];
    } ServerProperties;

    /**
     * @brief A struct containing the server's statistics,
     * i.e. bandwidth usage and user activity.
     *
     * Use TT_DoQueryServerStats() to query the server's statistics. */
    typedef struct ServerStatistics
    {
        /** @brief The number of bytes sent from the server to
         * clients. */
        INT64 nTotalBytesTX;
        /** @brief The number of bytes received by the server from
         * clients. */
        INT64 nTotalBytesRX;
        /** @brief The number of bytes in audio packets sent from the
         *  server to clients. */
        INT64 nVoiceBytesTX;
        /** @brief The number of bytes in audio packets received by
         *  the server from clients. */
        INT64 nVoiceBytesRX;
        /** @brief The number of bytes in video packets sent from the
         *  server to clients. */
        INT64 nVideoCaptureBytesTX;
        /** @brief The number of bytes in video packets received by
         *  the server from clients. */
        INT64 nVideoCaptureBytesRX;
        /** @brief The number of bytes in media file packets sent from the
         *  server to clients. */
        INT64 nMediaFileBytesTX;
        /** @brief The number of bytes in media file packets received by
         *  the server from clients. */
        INT64 nMediaFileBytesRX;
        /** @brief The number of bytes in desktop packets sent from the
         *  server to clients. */
        INT64 nDesktopBytesTX;
        /** @brief The number of bytes in desktop packets received by
         *  the server from clients. */
        INT64 nDesktopBytesRX;
        /** @brief The number of users who have logged on to the server. */
        INT32 nUsersServed;
        /** @brief The highest numbers of users online. */
        INT32 nUsersPeak;
        /** @brief The number of bytes for file transmission transmitted
         * from the server. */
        INT64 nFilesTx;
        /** @brief The number of bytes for file transmission received
         * by the server. */
        INT64 nFilesRx;
        /** @brief The server's uptime in msec. */
        INT64 nUptimeMSec;
    } ServerStatistics;

    /**
     * @brief Way to ban a user from either login or joining a
     * channel.
     *
     * @see BannedUser */
    typedef enum BanType
    {
        /** @brief Ban type not set. */
        BANTYPE_NONE                = 0x00,
        /** @brief The ban applies to the channel specified in the @c
         * szChannel of #BannedUser. Otherwise the ban applies to the
         * entire server. */
        BANTYPE_CHANNEL             = 0x01,
        /** @brief Ban @c szIPAddress specified in #BannedUser. */
        BANTYPE_IPADDR              = 0x02,
        /** @brief Ban @c szUsername specified in #BannedUser. */
        BANTYPE_USERNAME            = 0x04
    } BanType;

    /** @brief A mask of types of bans that apply. @see #BanType */
    typedef UINT32 BanTypes;

    /**
     * @brief A struct containing the properties of a banned user.
     * @see TT_DoListBans()
     * @see TT_DoBanUserEx() */
    typedef struct BannedUser
    {
        /** @brief IP-address of banned user. */
        TTCHAR szIPAddress[TT_STRLEN]; 
        /** @brief Channel where user was located when banned. */
        TTCHAR szChannelPath[TT_STRLEN]; 
        /** @brief Date and time when user was banned. Read-only property. */
        TTCHAR szBanTime[TT_STRLEN];
        /** @brief Nickname of banned user. Read-only property.  */
        TTCHAR szNickname[TT_STRLEN];
        /** @brief Username of banned user. */
        TTCHAR szUsername[TT_STRLEN];
        /** @brief The type of ban that applies to this banned user. */
        BanTypes uBanTypes;
    } BannedUser;

    /** @ingroup users
     * @brief The types of users supported. 
     * @see User @see UserAccount */
    typedef enum UserType
    {
        /** @brief Used internally to denote an unauthenticated
         * user. */
        USERTYPE_NONE    = 0x0, 
        /** @brief A default user who can join channels. */
        USERTYPE_DEFAULT = 0x01, 
        /** @brief A user with administrator privileges. */
        USERTYPE_ADMIN   = 0x02 
    } UserType;

    /** @ingroup users
     * @brief A bitmask based on #UserType describing the user type.
     * @see UserType */
    typedef UINT32 UserTypes;

    /**
     * @brief Properties to prevent server abuse.
     * 
     * The AbusePrevention-struct is part of #UserAccount and can be
     * used to limit the number of commands a user can issue to the 
     * server.
     * 
     * Requires TeamTalk version 5.2.3.4896. */
    typedef struct AbusePrevention
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
        INT32 nCommandsLimit;
        /** @brief Commands within given interval. */
        INT32 nCommandsIntervalMSec;
    } AbusePrevention;

    /** 
     * @brief A struct containing the properties of a user account.
     *
     * A registered user is one that has a user account on the server.
     *
     * @see TT_DoListUserAccounts
     * @see TT_DoNewUserAccount
     * @see TT_DoDeleteUserAccount */
    typedef struct UserAccount
    {
        /** @brief The account's username. */
        TTCHAR szUsername[TT_STRLEN];
        /** @brief The account's password. */
        TTCHAR szPassword[TT_STRLEN];
        /** @brief A bitmask of the type of user based on #UserType. */
        UserTypes uUserType;
        /** @brief A bitmask based on #UserRight which specifies the rights 
         * the user have who logs onto the server. A user of type 
         * #USERTYPE_ADMIN by default has all rights. */
        UserRights uUserRights;
        /** @brief A user data field which can be used for additional
         * information. The @a nUserData field of the #User struct will
         * contain this value when a user who logs in with this account. */
        INT32 nUserData;
        /** @brief Additional notes about this user. */
        TTCHAR szNote[TT_STRLEN];
        /** @brief User should (manually) join this channel after login.
         * If an initial channel is specified in the user's account then
         * no password is required for the user to join the channel.
         * @see TT_DoJoinChannel() */
        TTCHAR szInitChannel[TT_STRLEN];
        /** @brief Channels where this user will automatically become channel
         * operator when joining. The channels must be of type #CHANNEL_PERMANENT.
         * @see TT_DoChannelOp() */
        INT32 autoOperatorChannels[TT_CHANNELS_OPERATOR_MAX];
        /** @brief Bandwidth restriction for audio codecs created by 
         * this user. This value will hold the highest bitrate which 
         * is allowed for audio codecs. 0 = no limit. @see AudioCodec */
        INT32 nAudioCodecBpsLimit;
        /** @brief Properties which can be set to prevent abuse of a
         * server, e.g. limit number of commands issued.
         * 
         * Requires TeamTalk version 5.2.3.4896. */
        AbusePrevention abusePrevent;
    } UserAccount;
    /** @} */

    /** @addtogroup users
     * @{ */

    /** 
     * @brief A user by default accepts audio, video and text messages
     * from all users. Using subscribtions can, however, change what
     * the local client instance is willing to accept from other
     * users.
     *
     * By calling #TT_DoSubscribe and #TT_DoUnsubscribe the local
     * client instance can tell the server (and thereby remote users)
     * what he is willing to accept from other users.
     *
     * To check what a user subscribes to check out the #User struct's
     * @a uLocalSubscriptions. The subscriptions with the prefix
     * @c SUBSCRIBE_INTERCEPT_* options can be used to spy on users and
     * receive data from them even if one is not participating in the
     * same channel as they are.
     *
     * @see TT_DoSubscribe
     * @see TT_DoUnsubscribe */
    typedef enum Subscription
    {
        /** @brief No subscriptions. */
        SUBSCRIBE_NONE                    = 0x00000000,
        /** @brief Subscribing to user text messages.
         * @see #MSGTYPE_USER. */
        SUBSCRIBE_USER_MSG                = 0x00000001,
        /** @brief Subscribing to channel texxt messages.
         * @see #MSGTYPE_CHANNEL. */
        SUBSCRIBE_CHANNEL_MSG             = 0x00000002,
        /** @brief Subscribing to broadcast text messsages. 
         * @see #MSGTYPE_BROADCAST.*/
        SUBSCRIBE_BROADCAST_MSG           = 0x00000004,
        /** @brief Subscribing to custom user messages. 
         * @see #MSGTYPE_CUSTOM. */
        SUBSCRIBE_CUSTOM_MSG              = 0x00000008,
        /** @brief Subscribing to #STREAMTYPE_VOICE. */
        SUBSCRIBE_VOICE                   = 0x00000010,
        /** @brief Subscribing to #STREAMTYPE_VIDEOCAPTURE. */
        SUBSCRIBE_VIDEOCAPTURE            = 0x00000020,
        /** @brief Subscribing to #STREAMTYPE_DESKTOP. */
        SUBSCRIBE_DESKTOP                 = 0x00000040,
        /** @brief Subscribing to #STREAMTYPE_DESKTOPINPUT.
         * @see TT_GetUserDesktopInput()
         * @see TT_SendDesktopInput() */
        SUBSCRIBE_DESKTOPINPUT            = 0x00000080,
        /** @brief Subscribing to #STREAMTYPE_MEDIAFILE_VIDEO and
         * #STREAMTYPE_MEDIAFILE_AUDIO. */
        SUBSCRIBE_MEDIAFILE               = 0x00000100,
        /** @brief Intercept all user text messages sent by a
        * user. Only user-type #USERTYPE_ADMIN can do this. */
        SUBSCRIBE_INTERCEPT_USER_MSG      = 0x00010000,
        /** @brief Intercept all channel messages sent by a user. Only
        * user-type #USERTYPE_ADMIN can do this. */
        SUBSCRIBE_INTERCEPT_CHANNEL_MSG   = 0x00020000,
        /* unused SUBSCRIBE_INTERCEPT_BROADCAST_MSG = 0x00040000 */
        /** @brief Intercept all custom text messages sent by user. 
         * Only user-type #USERTYPE_ADMIN can do this.  */
        SUBSCRIBE_INTERCEPT_CUSTOM_MSG    = 0x00080000,
        /** @brief Intercept all voice sent by a user. Only user-type
         * #USERTYPE_ADMIN can do this. By enabling this subscription an
         * administrator can listen to audio sent by users outside his
         * own channel. */
        SUBSCRIBE_INTERCEPT_VOICE         = 0x00100000,
        /** @brief Intercept all video sent by a user. Only user-type
         * #USERTYPE_ADMIN can do this. By enabling this subscription an
         * administrator can receive video frames sent by users
         * outside his own channel. */
        SUBSCRIBE_INTERCEPT_VIDEOCAPTURE  = 0x00200000,
        /** @brief Intercept all desktop data sent by a user. Only
         * user-type #USERTYPE_ADMIN can do this. By enabling this
         * subscription an administrator can views desktops
         * sent by users outside his own channel. */
        SUBSCRIBE_INTERCEPT_DESKTOP       = 0x00400000,
        /* unused SUBSCRIBE_INTERCEPT_DESKTOPINPUT = 0x00800000, */
        /** @brief Intercept all media file data sent by a user. Only user-type
         * #USERTYPE_ADMIN can do this. By enabling this subscription an
         * administrator can listen to media files sent by users outside his
         * own channel. */
        SUBSCRIBE_INTERCEPT_MEDIAFILE     = 0x01000000,
    } Subscription;

    /** 
     * @brief A bitmask based on #Subscription describing which 
     * subscriptions are enabled.
     * @see Subscription */
    typedef UINT32 Subscriptions;

    /** @brief The possible states for a user. Used for #User's @a
     * uUserState variable. */
    typedef enum UserState
    {
        /** @brief The user is in initial state. */
        USERSTATE_NONE                  = 0x0000000,
        /** @brief If set the user is currently talking. If this flag
         * changes the event #CLIENTEVENT_USER_STATECHANGE is
         * posted. */
        USERSTATE_VOICE                 = 0x00000001,
        /** @brief If set the user's voice is muted. @see TT_SetUserMute */
        USERSTATE_MUTE_VOICE            = 0x00000002,
        /** @brief If set the user's media file playback is muted.
         * @see TT_SetUserMute */
        USERSTATE_MUTE_MEDIAFILE        = 0x00000004,
        /** @brief If set the user currently has an active desktop
         * session. If this flag changes the event
         * #CLIENTEVENT_USER_STATECHANGE is posted.  @see
         * TT_SendDesktopWindow(). */
        USERSTATE_DESKTOP               = 0x00000008,
        /** @brief If set the user currently has an active video
         * stream.  If this flag changes the event
         * #CLIENTEVENT_USER_STATECHANGE is posted.  @see
         * CLIENTEVENT_USER_VIDEOCAPTURE. */
        USERSTATE_VIDEOCAPTURE          = 0x00000010,
        /** @brief If set the user currently streams an audio file. If
         * user is streaming a video file with audio then this value
         * is also set.  If this flag changes the event
         * #CLIENTEVENT_USER_STATECHANGE is posted.  @see
         * TT_StartStreamingMediaFile() */
        USERSTATE_MEDIAFILE_AUDIO       = 0x00000020,
        /** @brief If set the user currently streams a video file.  If
         * this flag changes the event #CLIENTEVENT_USER_STATECHANGE
         * is posted.  @see TT_StartStreamingMediaFile() */
        USERSTATE_MEDIAFILE_VIDEO       = 0x00000040,
        /** @brief If set user is streaming a media file.  If this
         * flag changes the event #CLIENTEVENT_USER_STATECHANGE is
         * posted.  @see TT_StartStreamingMediaFile() */
        USERSTATE_MEDIAFILE             = USERSTATE_MEDIAFILE_AUDIO |
                                          USERSTATE_MEDIAFILE_VIDEO
    } UserState;

    /** @brief A bitmask based on #UserState indicating a #User's current
     * state. */
    typedef UINT32 UserStates;

    /** 
     * @brief A struct containing the properties of a user.
     * @see UserType
     * @see TT_GetUser */
    typedef struct User
    {
        /** @brief The user's ID. A value from 1 -
         * #TT_USERID_MAX. This property is set by the server and will
         * not change after login. */
        INT32 nUserID;
        /** @brief The @a szUsername of the user's #UserAccount. A
         * user account is created by calling
         * TT_DoNewUserAccount(). This property is set by the server
         * and will not change after login.  */
        TTCHAR szUsername[TT_STRLEN];
        /** @brief The @a nUserData of the user's #UserAccount. This
         * field can be use to denote e.g. a database ID. This
         * property is set by the server and will not change after
         * login.*/
        INT32 nUserData;
        /** @brief The @a uUserType of the user's #UserAccount. This
         * property is set by the server and will not change after
         * login. */
        UserTypes uUserType;
        /** @brief The user's IP-address. This value is set by the server. */
        TTCHAR szIPAddress[TT_STRLEN];
        /** @brief The user's client version. This property is set by
         * the server and will not change after login. Three octets
         * are used for the version number. Third octet is major,
         * second octet is minor and third octet is
         * build. I.e. 0x00050201 is version 5.2.1. */ 
        UINT32 uVersion;
        /** @brief The channel which the user is currently
         * participating in. 0 if none. This value can change as a
         * result of TT_DoJoinChannel() or TT_DoLeaveChannel(). Events
         * #CLIENTEVENT_CMD_USER_JOINED and #CLIENTEVENT_CMD_USER_LEFT
         * are posted when this value changes. */
        INT32 nChannelID; 
        /** @brief A bitmask of what the local user subscribes to from
         * this user. Invoking TT_DoSubscribe() and TT_DoUnsubscribe()
         * on the local client instance can change this value. Event
         * #CLIENTEVENT_CMD_USER_UPDATE is posted if this value
         * changes. */
        Subscriptions uLocalSubscriptions;
        /** @brief A bitmask of what this user subscribes to from
         * local client instance. Invoking TT_DoSubscribe() and
         * TT_DoUnsubscribe() on the remote client instance can change
         * this value. Event #CLIENTEVENT_CMD_USER_UPDATE is posted if
         * this value changes. */
        Subscriptions uPeerSubscriptions;
         /** @brief The user's nickname. Invoking
          * TT_DoChangeNickname() changes this value. Event
          * #CLIENTEVENT_CMD_USER_UPDATE is posted if this value
          * changes. */
        TTCHAR szNickname[TT_STRLEN];
        /** @brief The user's current status mode. Invoke
         * TT_DoChangeStatus() to change this value. Event
         * #CLIENTEVENT_CMD_USER_UPDATE is posted if this value
         * changes. */
        INT32 nStatusMode;
        /** @brief The user's current status message. Invoke
         * TT_DoChangeStatus() to change this value. Event
         * #CLIENTEVENT_CMD_USER_UPDATE is posted if this value
         * changes. */
        TTCHAR szStatusMsg[TT_STRLEN];
        /** @brief A bitmask of the user's current state,
         * e.g. talking, muted, etc.   */
        UserStates uUserState;
        /** @brief Store audio received from this user to this
         * folder. @see TT_SetUserMediaStorageDir */
        TTCHAR szMediaStorageDir[TT_STRLEN];
        /** @brief The user's voice volume level. Note that it's a virtual 
         * volume which is being set since the master volume affects 
         * the user volume. The value will be between
         * #SOUND_VOLUME_MIN and #SOUND_VOLUME_MAX
         * @see TT_SetUserVolume */
        INT32 nVolumeVoice;
        /** @brief The user's voice volume level. Note that it's a virtual 
         * volume which is being set since the master volume affects 
         * the user volume. The value will be between
         * #SOUND_VOLUME_MIN and #SOUND_VOLUME_MAX
         * @see TT_SetUserVolume */
        INT32 nVolumeMediaFile;
        /** @brief The delay of when a user should no longer be 
         * considered as talking.
         * @see TT_SetUserStoppedTalkingDelay */
        INT32 nStoppedDelayVoice;
        /** @brief The delay of when a user should no longer be 
         * considered playing audio of a media file.
         * @see TT_SetUserStoppedTalkingDelay */
        INT32 nStoppedDelayMediaFile;
        /** @brief User's position when using 3D-sound (DirectSound option).
         * Index 0 is x-axis, index 1 is y-axis and index 2 is Z-axis.
         * @see TT_SetUserPosition
         * @see SoundDevice */
        float soundPositionVoice[3];
        /** @brief User's position when using 3D-sound (DirectSound option).
         * Index 0 is x-axis, index 1 is y-axis and index 2 is Z-axis.
         * @see TT_SetUserPosition
         * @see SoundDevice */
        float soundPositionMediaFile[3];
        /** @brief Check what speaker a user is outputting to. 
         * If index 0 is TRUE then left speaker is playing. If index 1 is
         * TRUE then right speaker is playing.
         * @see TT_SetUserStereo */
        TTBOOL stereoPlaybackVoice[2];
        /** @brief Check what speaker a user is outputting to. 
         * If index 0 is TRUE then left speaker is playing. If index 1 is
         * TRUE then right speaker is playing.
         * @see TT_SetUserStereo */
        TTBOOL stereoPlaybackMediaFile[2];
        /** @brief The size of the buffer (in msec) to hold voice
         * content.
         * @see TT_SetUserAudioStreamBufferSize() */
        INT32 nBufferMSecVoice;
        /** @brief The size of the buffer (in msec) to hold media file 
         * content.
         * @see TT_SetUserAudioStreamBufferSize() */
        INT32 nBufferMSecMediaFile;
        /** @brief The name of the client application which the user
         * is using. This is the value passed as @c szClientName in
         * TT_DoLoginEx() */
        TTCHAR szClientName[TT_STRLEN];
    } User;

    /**
     * @brief Packet reception and data statistics for a user.
     * 
     * @see TT_GetUserStatistics */
    typedef struct UserStatistics
    { 
        /** @brief Number of voice packets received from user. */
        INT64 nVoicePacketsRecv;
        /** @brief Number of voice packets lost from user. */
        INT64 nVoicePacketsLost;
        /** @brief Number of video packets received from user. A video 
         * frame can consist of several video packets. */
        INT64 nVideoCapturePacketsRecv;
        /** @brief Number of video frames received from user. */
        INT64 nVideoCaptureFramesRecv;
        /** @brief Video frames which couldn't be shown because packets were
         * lost. */
        INT64 nVideoCaptureFramesLost;
        /** @brief Number of video frames dropped because user application  
         * didn't retrieve video frames in time. */
        INT64 nVideoCaptureFramesDropped;
        /** @brief Number of media file audio packets received from user. */
        INT64 nMediaFileAudioPacketsRecv;
        /** @brief Number of media file audio packets lost from user. */
        INT64 nMediaFileAudioPacketsLost;
        /** @brief Number of media file video packets received from user. 
         * A video frame can consist of several video packets. */
        INT64 nMediaFileVideoPacketsRecv;
        /** @brief Number of media file video frames received from user. */
        INT64 nMediaFileVideoFramesRecv;
        /** @brief Media file  video frames which couldn't be shown because 
         * packets were lost. */
        INT64 nMediaFileVideoFramesLost;
        /** @brief Number of media file video frames dropped because user 
         * application didn't retrieve video frames in time. */
        INT64 nMediaFileVideoFramesDropped;
    } UserStatistics;

    /** 
     * @brief Text message types.
     * 
     * The types of messages which can be passed to #TT_DoTextMessage().
     *
     * @see TextMessage
     * @see TT_DoTextMessage
     * @see CLIENTEVENT_CMD_USER_TEXTMSG */ 
    typedef enum TextMsgType
    {
         /** @brief A User to user text message. A message of this
          * type can be sent across channels. */
        MSGTYPE_USER      = 1,
        /** @brief A User to channel text message. Users of type
         * #USERTYPE_DEFAULT can only send this text message to the
         * channel they're participating in, whereas users of type
         * #USERTYPE_ADMIN can send to any channel. */
        MSGTYPE_CHANNEL   = 2,
         /** @brief A broadcast message. Requires
          * #USERRIGHT_TEXTMESSAGE_BROADCAST. */
        MSGTYPE_BROADCAST = 3,
        /** @brief A custom user to user text message. Works the same
         * way as #MSGTYPE_USER. */
        MSGTYPE_CUSTOM    = 4
    } TextMsgType;

    /** 
     * @brief A struct containing the properties of a text message
     * sent by a user.
     *
     * @see CLIENTEVENT_CMD_USER_TEXTMSG
     * @see TT_DoTextMessage */
    typedef struct TextMessage
    {
        /** @brief The type of text message. */
        TextMsgType nMsgType;
        /** @brief Will be set automatically on outgoing message. */
        INT32 nFromUserID;
        /** @brief The originators username. */
        TTCHAR szFromUsername[TT_STRLEN];
        /** @brief Set to zero if channel message. */
        INT32 nToUserID;
        /** @brief Set to zero if @a nMsgType is #MSGTYPE_USER or
         * #MSGTYPE_BROADCAST. */
        INT32 nChannelID;
        /** @brief The actual text message. The message can be
         * multi-line (include EOL).  */
        TTCHAR szMessage[TT_STRLEN];
    } TextMessage;
    /** @} */

    /** @addtogroup channels
     * @{ */

    /**
     * @brief The types of channels supported. @see Channel */
    typedef enum ChannelType
    {
        /** @brief A default channel is a channel which disappears
         * after the last user leaves the channel. */
        CHANNEL_DEFAULT             = 0x0000,
        /** @brief A channel which persists even when the last user
         * leaves the channel. */
        CHANNEL_PERMANENT           = 0x0001,
        /** @brief Only one user can transmit at a time. */
        CHANNEL_SOLO_TRANSMIT       = 0x0002,
        /** @brief Voice and video transmission in the channel is
         * controlled by a channel operator.
         *
         * For a user to transmit audio or video to this type of
         * channel the channel operator must add the user's ID to @c
         * transmitUsers in the #Channel struct and call
         * TT_DoUpdateChannel().
         *
         * @see TT_IsChannelOperator
         * @see #USERTYPE_ADMIN */
        CHANNEL_CLASSROOM           = 0x0004,
        /** @brief Only channel operators (and administrators) will receive 
         * audio/video/desktop transmissions. Default channel users 
         * will only see transmissions from operators and/or 
         * administrators. */
        CHANNEL_OPERATOR_RECVONLY   = 0x0008,
        /** @brief Don't allow voice transmission if it's trigged by
         * voice activation. @see TT_EnableVoiceActivation() */
        CHANNEL_NO_VOICEACTIVATION  = 0x0010,
        /** @brief Don't allow recording to files in the channel. */
        CHANNEL_NO_RECORDING        = 0x0020
    } ChannelType;

    /** 
     * @brief Bitmask of #ChannelType. */
    typedef UINT32 ChannelTypes;

    /** 
     * @brief A struct containing the properties of a channel.
     *
     * To change the properties of a channel call
     * TT_DoUpdateChannel(). Note that @a audiocodec cannot be
     * changed if the channel has users.
     *
     * @see TT_GetChannel
     * @see ChannelType
     * @see AudioCodec */
    typedef struct Channel
    {
        /** @brief Parent channel ID. 0 means no parent channel,
         * i.e. it's the root channel. */
        INT32 nParentID;
        /** @brief The channel's ID. A value from 1 - #TT_CHANNELID_MAX. */
        INT32 nChannelID;
        /** @brief Name of the channel. */
        TTCHAR szName[TT_STRLEN];
        /** @brief Topic of the channel. */
        TTCHAR szTopic[TT_STRLEN];
        /** @brief Password to join the channel. When extracted through
         * #TT_GetChannel the password will only be set for users of 
         * user-type #USERTYPE_ADMIN. */
        TTCHAR szPassword[TT_STRLEN];
        /** @brief Whether password is required to join channel. Read-only 
         * property. */
        TTBOOL bPassword;
        /** @brief A bitmask of the type of channel based on #ChannelType. */
        ChannelTypes uChannelType;
        /** @brief User specific data which will be stored on
         * persistent storage on the server if the channel type is
         * #CHANNEL_PERMANENT. */
        INT32 nUserData;
        /** @brief Number of bytes available for file storage. */
        INT64 nDiskQuota;
        /** @brief Password to become channel operator. @see
         * TT_DoChannelOpEx() */
        TTCHAR szOpPassword[TT_STRLEN];
        /** @brief Max number of users in channel. */
        INT32 nMaxUsers;
        /** @brief The audio codec used by users in the channel. */
        AudioCodec audiocodec;
        /** @brief The audio configuration which users who join the channel
         * should use. @see TT_SetSoundInputPreprocess() */
        AudioConfig audiocfg;
        /** @brief List of users who can transmit in a channel.
         *
         * @c transmitUsers is a 2-dimensional array which specifies
         * who can transmit to the channel.
         *
         * If @c uChannelType is set to #CHANNEL_CLASSROOM then only
         * the users in @c transmitUsers are allowed to transmit. 
         *
         * In TeamTalk v5.4 and onwards adding a user ID to @c
         * transmitUsers will block the user from transmitting if the
         * #ChannelType is not #CHANNEL_CLASSROOM. Basically the
         * opposite effect of #CHANNEL_CLASSROOM.
         * 
         * To specify user ID 46 can transmit voice to a
         * #CHANNEL_CLASSROOM channel is done by assigning the
         * following:
         *
         * @verbatim
         * transmitUsers[0][0] = 46;
         * transmitUsers[0][1] = STREAMTYPE_VOICE;
         * @endverbatim
         *
         * To specify user ID 46 can transmit both voice and video
         * capture to a #CHANNEL_CLASSROOM channel is done by
         * assigning the following:
         *
         * @verbatim
         * transmitUsers[0][0] = 46;
         * transmitUsers[0][1] = STREAMTYPE_VOICE | STREAMTYPE_VIDEOCAPTURE;
         * @endverbatim
         *
         * The transmission list is terminated by assigning user ID 0
         * to the end of the list, i.e.:
         *
         * @verbatim
         * transmitUsers[0][0] = 0;
         * transmitUsers[0][1] = STREAMTYPE_NONE;
         * @endverbatim
         *
         * To allow all users of a #CHANNEL_CLASSROOM channel to
         * transmit a specific #StreamType is done like this:
         *
         * @verbatim
         * transmitUsers[0][0] = TT_CLASSROOM_FREEFORALL;
         * transmitUsers[0][1] = STREAMTYPE_VOICE;
         * @endverbatim
         *
         * Only channel operators are allowed to change the users who
         * are allowed to transmit data to a channel. Call
         * TT_DoUpdateChannel() to update the list of users who are
         * allowed to transmit data to the channel.
         *
         * @see TT_IsChannelOperator
         * @see TT_DoChannelOp
         * @see TT_CLASSROOM_FREEFORALL */
        INT32 transmitUsers[TT_TRANSMITUSERS_MAX][2];
        /** @brief The users currently queued for voice or media file transmission.
         *
         * This property only applied with channel is configured with
         * #CHANNEL_SOLO_TRANSMIT. Read-only property. */
        INT32 transmitUsersQueue[TT_TRANSMITQUEUE_MAX];
    } Channel;


    /** @brief Status of a file transfer. @see FileTransfer */
    typedef enum FileTransferStatus
    {
        /** brief Invalid transfer. */
        FILETRANSFER_CLOSED     = 0,
        /** @brief Error during file transfer. */
        FILETRANSFER_ERROR      = 1,
        /** @brief File transfer active. */
        FILETRANSFER_ACTIVE     = 2,
        /** @brief File transfer finished. */
        FILETRANSFER_FINISHED   = 3
    } FileTransferStatus;

    /** 
     * @brief A struct containing the properties of a file transfer.
     * @see TT_GetFileTransferInfo */
    typedef struct FileTransfer
    {
        /** @brief Status of file transfer. */
        FileTransferStatus nStatus;
        /** @brief The ID identifying the file transfer. */
        INT32 nTransferID;
        /** @brief The channel where the file is/will be located. */
        INT32 nChannelID;
        /** @brief The file path on local disk. */
        TTCHAR szLocalFilePath[TT_STRLEN];
        /** @brief The filename in the channel. */
        TTCHAR szRemoteFileName[TT_STRLEN];
        /** @brief The size of the file being transferred. */
        INT64 nFileSize;
        /** @brief The number of bytes transferred so far. */
        INT64 nTransferred;
        /** @brief TRUE if download and FALSE if upload. */
        TTBOOL bInbound;
    } FileTransfer;


    /**
     * @brief A struct containing the properties of a file in a #Channel.
     * @see TT_GetChannelFile */
    typedef struct RemoteFile
    {
        /** @brief The ID of the channel where the file is located. */
        INT32 nChannelID;
        /** @brief The ID identifying the file. */
        INT32 nFileID;
        /** @brief The name of the file. */
        TTCHAR szFileName[TT_STRLEN];
        /** @brief The size of the file. */
        INT64 nFileSize;
        /** @brief Username of the person who uploaded the file. */
        TTCHAR szUsername[TT_STRLEN];
    } RemoteFile;
    /** @} */

    /** @ingroup connectivity
     * @brief Control timers for sending keep alive information to the
     * server.
     *
     * @see TT_DoPing()
     */
    typedef struct ClientKeepAlive
    {
        /** @brief The duration before the #TTInstance should consider
         * the client/server connection lost.
         *
         * This value must be greater than @c
         * nTcpKeepAliveIntervalMSec and @c nUdpKeepAliveIntervalMSec.
         *
         * This timeout applies to both the TCP and UDP
         * connection. I.e. @c nTcpServerSilenceSec or @c
         * nUdpServerSilenceSec in #ClientStatistics should not exceed
         * @c nConnectionLostMSec. */
        INT32 nConnectionLostMSec;
        /** @brief Client instance's interval between automatically
         * doing TT_DoPing() command. Read-only value. Will be half of
         * #ServerProperties' @c nUserTimeout. */
        INT32 nTcpKeepAliveIntervalMSec;
        /** @brief Client instance's interval between sending UDP keep
         * alive packets. The UDP keep alive packets are used to
         * ensure audio, video and desktop streams can be sent from
         * the server to the client immediately. This value must be
         * less than @c nConnectionLostMSec. */
        INT32 nUdpKeepAliveIntervalMSec;
        /** @brief Client instance's interval for retransmitting UDP
         * keep alive packets. If server hasn't responded to UDP keep
         * alive sent at interval @c nUdpKeepAliveIntervalMSec then a
         * new UDP keep alive will be sent at the rate specified by
         * @c nUdpKeepAliveRTXMSec. */
        INT32 nUdpKeepAliveRTXMSec;
        /** @brief Client instance's interval for retransmitting UDP
         * connect packets. UDP connect packets are only sent when
         * TT_Connect() is initially called. If the server doesn't
         * respond to the client instance's initial UDP connect then a
         * retransmission will be started at the rate of @c
         * nUdpConnectRTXMSec. */
        INT32 nUdpConnectRTXMSec;
        /** @brief The duration before the client instance should give
         * up trying to connect to the server on UDP. When
         * TT_Connect() manages to connect to the server's TCP port
         * then the client will afterwards try to connect on server's
         * UDP port. If the client cannot connect on UDP before the
         * time specified by @c nUdpConnectTimeoutMSec then the client
         * instance will report #CLIENTEVENT_CON_FAILED. */
        INT32 nUdpConnectTimeoutMSec;
    } ClientKeepAlive;
    
    /** @ingroup connectivity
     * @brief Statistics of bandwidth usage and ping times in the local 
     * client instance.
     * @see TT_GetClientStatistics */
    typedef struct ClientStatistics
    {
        /** @brief Bytes sent on UDP. */
        INT64 nUdpBytesSent;
        /** @brief Bytes received on UDP. */
        INT64 nUdpBytesRecv;
        /** @brief Voice data sent (on UDP). */
        INT64 nVoiceBytesSent;
        /** @brief Voice data received (on UDP). */
        INT64 nVoiceBytesRecv;
        /** @brief Video data sent (on UDP). */
        INT64 nVideoCaptureBytesSent;
        /** @brief Video data received (on UDP). */
        INT64 nVideoCaptureBytesRecv;
        /** @brief Audio from media file data sent (on UDP). */
        INT64 nMediaFileAudioBytesSent;
        /** @brief Audio from media file data received (on UDP). */
        INT64 nMediaFileAudioBytesRecv;
        /** @brief Video from media file data sent (on UDP). */
        INT64 nMediaFileVideoBytesSent;
        /** @brief Video from media file data received (on UDP). */
        INT64 nMediaFileVideoBytesRecv;
        /** @brief Desktop data sent (on UDP). */
        INT64 nDesktopBytesSent;
        /** @brief Desktop data received (on UDP). */
        INT64 nDesktopBytesRecv;
        /** @brief Response time to server on UDP (based on ping/pong
         * sent at a specified interval. Set to -1 if not currently
         * available. */
        INT32 nUdpPingTimeMs;
        /** @brief Response time to server on TCP (based on ping/pong
         * sent at a specified interval. Set to -1 if not currently
         * available.  @see TT_DoPing() @see ClientKeepAlive */
        INT32 nTcpPingTimeMs;
        /** @brief The number of seconds nothing has been received by
         * the client on TCP. @see TT_DoPing() @see ClientKeepAlive */
        INT32 nTcpServerSilenceSec;
        /** @brief The number of seconds nothing has been received by
         * the client on UDP. @see ClientKeepAlive */
        INT32 nUdpServerSilenceSec;
    } ClientStatistics;

    /** @addtogroup errorhandling
     * @{ */

    /** 
     * @brief Errors which can occur either as a result of client
     * commands or as a result of internal errors.
     *
     * Use #TT_GetErrorMessage to get a text-description of the
     * error. */
    typedef enum ClientError
    {
        /** @brief Command indicating success. Only used internally. */
        CMDERR_SUCCESS = 0,

        /* COMMAND ERRORS 1000-1999 ARE DUE TO INVALID OR UNSUPPORTED
         * COMMANDS */

        /** @brief Command has syntax error. Only used internally. */
        CMDERR_SYNTAX_ERROR = 1000,
        /** @brief The server doesn't support the issued command.
         *
         * This error may occur if the server is an older version than
         * the client instance. */
        CMDERR_UNKNOWN_COMMAND = 1001,
        /** @brief Command cannot be performed due to missing
         * parameter. Only used internally. */
        CMDERR_MISSING_PARAMETER = 1002,
        /** @brief The server uses a protocol which is incompatible
         * with the client instance. */
        CMDERR_INCOMPATIBLE_PROTOCOLS = 1003,
        /** @brief The server does not support the audio codec specified
         * by the client. Introduced in version 4.1.0.1264. 
         * @see TT_DoMakeChannel
         * @see TT_DoJoinChannel */
        CMDERR_UNKNOWN_AUDIOCODEC = 1004,
        /** @brief Invalid username for #UserAccount.
         * @see TT_DoNewUserAccount() */
        CMDERR_INVALID_USERNAME = 1005,

        /* COMMAND ERRORS 2000-2999 ARE DUE TO INSUFFICIENT RIGHTS */

        /** @brief Invalid channel password. 
         *
         * The #TT_DoJoinChannel or #TT_DoJoinChannelByID passed an
         * invalid channel password. #TT_DoMakeChannel can also cause
         * a this error if the password is longer than #TT_STRLEN. */
        CMDERR_INCORRECT_CHANNEL_PASSWORD = 2001,
        /** @brief Invalid username or password for account.
         *
         * The #TT_DoLogin command was issued with invalid account
         * properties. This error can also occur by
         * #TT_DoNewUserAccount if username is empty. */
        CMDERR_INVALID_ACCOUNT = 2002,
        /** @brief Login failed due to maximum number of users on
         * server.
         *
         * #TT_DoLogin failed because the server does not allow any
         * more users. */
        CMDERR_MAX_SERVER_USERS_EXCEEDED = 2003,
        /** @brief Cannot join channel because it has maximum number
         * of users.
         *
         * #TT_DoJoinChannel or #TT_DoJoinChannelByID failed because
         * no more users are allowed in the channel. */
        CMDERR_MAX_CHANNEL_USERS_EXCEEDED = 2004,
        /** @brief IP-address has been banned from server.
         *
         * #TT_DoLogin failed because the local client's IP-address
         * has been banned on the server. */
        CMDERR_SERVER_BANNED = 2005,
        /** @brief Command not authorized.
         *
         * The command cannot be performed because the client instance
         * has insufficient rights.
         *
         * @see TT_DoDeleteFile
         * @see TT_DoJoinChannel
         * @see TT_DoJoinChannelByID
         * @see TT_DoLeaveChannel
         * @see TT_DoChannelOp
         * @see TT_DoChannelOpEx
         * @see TT_DoKickUser
         * @see TT_DoUpdateChannel
         * @see TT_DoChangeNickname
         * @see TT_DoChangeStatus
         * @see TT_DoTextMessage
         * @see TT_DoSubscribe
         * @see TT_DoUnsubscribe
         * @see TT_DoMakeChannel
         * @see TT_DoRemoveChannel
         * @see TT_DoMoveUser
         * @see TT_DoUpdateServer
         * @see TT_DoSaveConfig
         * @see TT_DoSendFile 
         * @see TT_DoRecvFile 
         * @see TT_DoBanUser
         * @see TT_DoUnBanUser
         * @see TT_DoListBans
         * @see TT_DoListUserAccounts
         * @see TT_DoNewUserAccount
         * @see TT_DoDeleteUserAccount */
        CMDERR_NOT_AUTHORIZED = 2006,
        /** @brief Cannot upload file because disk quota will be exceeded.
         *
         * #TT_DoSendFile was not allowed because there's not enough
         * disk space available for upload.
         *
         * @see Channel */
        CMDERR_MAX_DISKUSAGE_EXCEEDED = 2008,
        /** @brief Invalid password for becoming channel operator.
         * 
         * The password specified in #TT_DoChannelOpEx is not correct.
         * The operator password is the @a szOpPassword of the 
         * #Channel-struct. */
        CMDERR_INCORRECT_OP_PASSWORD = 2010,

        /** @brief The selected #AudioCodec exceeds what the server allows.
         *
         * A server can limit the vitrate of audio codecs if @c 
         * nAudioCodecBpsLimit of #ServerProperties is specified. */
        CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED = 2011,

        /** @brief The maximum number of logins allowed per IP-address has
         * been exceeded.
         * 
         * @see ServerProperties
         * @see TT_DoLogin() */
        CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED = 2012,
        
        /** @brief The maximum number of channels has been exceeded.
         * @see TT_CHANNELID_MAX */
        CMDERR_MAX_CHANNELS_EXCEEDED = 2013,

        /** @brief Command flooding prevented by server.
         *
         * Commands are issued faster than allowed by the server. See
         * #UserAccount @c commandsPerMSec.  @see TT_CHANNELID_MAX */
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
         * #TT_DoLogin has not been issued successfully or
         * #TT_DoLogout could not be performed because client
         * instance is already logged in.*/
        CMDERR_NOT_LOGGEDIN = 3000,

        /** @brief Already logged in.
         *
         * #TT_DoLogin cannot be performed twice. */
        CMDERR_ALREADY_LOGGEDIN = 3001,
        /** @brief Cannot leave channel because not in channel.
         *
         * #TT_DoLeaveChannel failed because user is not in a channel. */
        CMDERR_NOT_IN_CHANNEL = 3002,
        /** @brief Cannot join same channel twice.
         * 
         * #TT_DoJoinChannel or #TT_DoJoinChannelByID failed because
         * client instance is already in the specified channel. */
        CMDERR_ALREADY_IN_CHANNEL = 3003,
        /** @brief Channel already exists.
         *
         * #TT_DoMakeChannel failed because channel already exists. */
        CMDERR_CHANNEL_ALREADY_EXISTS = 3004,
        /** @brief Channel does not exist.
         *
         * Command failed because channel does not exists.
         * @see TT_DoRemoveChannel
         * @see TT_DoUpdateChannel
         * @see TT_DoMakeChannel Due to invalid channel name
         * @see TT_DoSendFile
         * @see TT_DoRecvFile
         * @see TT_DoDeleteFile
         * @see TT_DoJoinChannel
         * @see TT_DoJoinChannelByID
         * @see TT_DoLeaveChannel
         * @see TT_DoChannelOp
         * @see TT_DoKickUser
         * @see TT_DoBanUser
         * @see TT_DoMoveUser
         * @see TT_DoTextMessage */
        CMDERR_CHANNEL_NOT_FOUND = 3005,
        /** @brief User not found.
         * 
         * Command failed because user does not exists.
         * @see TT_DoChannelOp
         * @see TT_DoKickUser
         * @see TT_DoBanUser
         * @see TT_DoMoveUser
         * @see TT_DoTextMessage
         * @see TT_DoSubscribe
         * @see TT_DoUnsubscribe */
        CMDERR_USER_NOT_FOUND = 3006,
        /** @brief Banned IP-address does not exist.
         * 
         * #TT_DoUnBanUser failed because there is no banned
         * IP-address which matches what was specified. */
        CMDERR_BAN_NOT_FOUND = 3007,
        /** @brief File transfer doesn't exists.
         *
         * TT_DoSendFile() or TT_DoRecvFile() failed because the server
         * cannot process the file transfer. */
        CMDERR_FILETRANSFER_NOT_FOUND = 3008,
        /** @brief Server failed to open file.
         *
         * TT_DoSendFile() or TT_DoRecvFile() failed because the server
         * cannot open the specified file (possible file lock). */
        CMDERR_OPENFILE_FAILED = 3009,
        /** @brief Cannot find user account.
         * 
         * #TT_DoDeleteUserAccount failed because the specified user
         * account does not exists. */
        CMDERR_ACCOUNT_NOT_FOUND = 3010,
        /** @brief File does not exist.
         *
         * #TT_DoSendFile, #TT_DoRecvFile or #TT_DoDeleteFile failed
         * because the server cannot find the specified file. */
        CMDERR_FILE_NOT_FOUND = 3011,
        /** @brief File already exist.
         *
         * #TT_DoSendFile failed because the file already exists in
         * the channel. */
        CMDERR_FILE_ALREADY_EXISTS = 3012,
        /** @brief Server does not allow file transfers.
         *
         * #TT_DoSendFile or #TT_DoRecvFile failed because the server
         * does not allow file transfers. */
        CMDERR_FILESHARING_DISABLED = 3013,
        /** @brief Cannot process command since channel is not empty.
         *
         * @see TT_DoUpdateChannel #AudioCodec cannot be changed while
         * there are users in a channel. */
        CMDERR_CHANNEL_HAS_USERS = 3015,

        /** @brief The login service is currently unavailable.
         *
         * Added in TeamTalk v5.3 to support web-logins. */
        CMDERR_LOGINSERVICE_UNAVAILABLE = 3016,

        /* ERRORS 10000-10999 ARE NOT COMMAND ERRORS BUT INSTEAD
         * ERRORS IN THE CLIENT INSTANCE. */

        /** @brief A sound input device failed. 
         *
         * This can e.g. happen when joining a channel and the sound
         * input device has been unplugged. 
         * 
         * Call #TT_CloseSoundInputDevice and TT_InitSoundInputDevice
         * with a valid #SoundDevice to releave the problem. */
        INTERR_SNDINPUT_FAILURE = 10000,
        /** @brief A sound output device failed.
         *
         * This can e.g. happen when joining a channel and the sound
         * output device has been unplugged. Note that it can be posted
         * multiple times if there's several users in the channel.
         *
         * Call #TT_CloseSoundOutputDevice and TT_InitSoundOutputDevice
         * with a valid #SoundDevice to releave the problem. */
        INTERR_SNDOUTPUT_FAILURE = 10001,
        /** @brief Audio codec used by channel failed to initialize.
         * Ensure the settings specified in #AudioCodec are valid.
         * @see TT_DoJoinChannel() */
        INTERR_AUDIOCODEC_INIT_FAILED = 10002,
        /** @brief #SpeexDSP failed to initialize.
         *
         * This error occurs when joining a channel.
         *
         * The settings specified by TT_SetSoundInputPreprocess() are
         * invalid for the specified audio codec. @see
         * TT_DoJoinChannel() */
        INTERR_SPEEXDSP_INIT_FAILED = 10003,
        /** @brief #TTMessage event queue overflowed.
         *
         * The message queue for events has overflowed because
         * TT_GetMessage() has not drained the queue in time. The
         * #TTMessage message queue will suspend event handling once
         * the queue overflows and resumes event handling again when
         * the message queue has been drained. */
        INTERR_TTMESSAGE_QUEUE_OVERFLOW = 10004,
    } ClientError;

    /** @brief Struct containing an error message. */
    typedef struct ClientErrorMsg
    {
        /** @brief Error number based on #ClientError. */
        INT32 nErrorNo;
        /** @brief Text message describing the error. */
        TTCHAR szErrorMsg[TT_STRLEN];
    } ClientErrorMsg;
    /** @} */

    /** @addtogroup events
     * @{ */

    /** 
     * @brief TeamTalk client event messages.
     *
     * Events are retrieved using TT_GetMessage(). */
    typedef enum ClientEvent
    {
        CLIENTEVENT_NONE = 0,

        /**
         * @brief Connected successfully to the server.
         *
         * This event is posted if TT_Connect() was successful.
         *
         * #TT_DoLogin can now be called in order to logon to the
         * server.
         *
         * @param nSource 0
         * @param ttType #__NONE
         * @see TT_Connect */
        CLIENTEVENT_CON_SUCCESS = CLIENTEVENT_NONE + 10,
        /** 
         * @brief Failed to connect to server.
         *
         * This event is posted if #TT_Connect fails. Ensure to call
         * #TT_Disconnect before calling #TT_Connect again.
         *
         * @param nSource 0
         * @param ttType #__NONE
         * @see TT_Connect */
        CLIENTEVENT_CON_FAILED = CLIENTEVENT_NONE + 20,
        /** 
         * @brief Connection to server has been lost.
         *
         * The server is not responding to requests from the local
         * client instance and the connection has been dropped.
         *
         * #TT_GetClientStatistics can be used to check when data was
         * last received from the server.
         *
         * Ensure to call #TT_Disconnect before calling #TT_Connect
         * again.
         *
         * @param nSource 0
         * @param ttType #__NONE
         * @see TT_Connect */
        CLIENTEVENT_CON_LOST = CLIENTEVENT_NONE + 30,
        /**
         * @brief The maximum size of the payload put into UDP packets
         * has been updated.
         *
         * @param nSource Ignored
         * @param ttType #__INT32
         * @param nPayloadSize Placed in union of #TTMessage. The
         * maximum size in bytes of the payload data which is put in
         * UDP packets. 0 means the max payload query failed.  @see
         * TT_QueryMaxPayload() */
        CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED = CLIENTEVENT_NONE + 40,
        /** 
         * @brief A command issued by @c TT_Do* methods is being
         * processed.
         *
         * Read section @ref cmdprocessing on how to use command
         * processing in the user application.
         *
         * @param nSource Command ID being processed (returned by
         * TT_Do* commands)
         * @param ttType #__TTBOOL
         * @param bActive Placed in union of #TTMessage. Is TRUE if
         * command ID started processing and FALSE if the command has
         * finished processing. */
        CLIENTEVENT_CMD_PROCESSING = CLIENTEVENT_NONE + 200,
        /** 
         * @brief The server rejected a command issued by the local
         * client instance.
         *
         * To figure out which command failed use the command ID
         * returned by the TT_Do* command. Section @ref cmdprocessing
         * explains how to use command ID.
         *
         * @param nSource The command ID returned from the TT_Do*
         * commands.
         * @param ttType #__CLIENTERRORMSG
         * @param clienterrormsg Placed in union of #TTMessage. Contains
         * error description. */
        CLIENTEVENT_CMD_ERROR = CLIENTEVENT_NONE + 210,
        /**
         * @brief The server successfully processed a command issued
         * by the local client instance.
         *
         * To figure out which command succeeded use the command ID
         * returned by the TT_Do* command. Section @ref cmdprocessing
         * explains how to use command ID.
         *
         * @param nSource The command ID returned from the TT_Do*
         * commands.
         * @param ttType #__NONE */
        CLIENTEVENT_CMD_SUCCESS = CLIENTEVENT_NONE + 220,
        /** 
         * @brief The client instance successfully logged on to
         * server.
         *
         * The call to #TT_DoLogin was successful and all channels on
         * the server will be posted in the event
         * #CLIENTEVENT_CMD_CHANNEL_NEW immediately following this
         * event. If #USERRIGHT_VIEW_ALL_USERS is enabled the client
         * instance will also receive the events
         * #CLIENTEVENT_CMD_USER_LOGGEDIN and
         * #CLIENTEVENT_CMD_USER_JOINED for every user on the server.
         *
         * @param nSource The client instance's user ID, i.e. what can now 
         * be retrieved through TT_GetMyUserID().
         * @param ttType #__USERACCOUNT
         * @param useraccount Placed in union of #TTMessage.
         * @see TT_DoLogin */
        CLIENTEVENT_CMD_MYSELF_LOGGEDIN = CLIENTEVENT_NONE + 230,
        /** 
         * @brief The client instance logged out of the server.
         *
         * A response to #TT_DoLogout.
         *
         * @param nSource 0
         * @param ttType #__NONE
         * @see TT_DoLogout */
        CLIENTEVENT_CMD_MYSELF_LOGGEDOUT = CLIENTEVENT_NONE + 240,
        /** 
         * @brief The client instance was kicked from a channel.
         *
         * @param nSource If greater than zero indicates local client
         * instance was kicked from a channel. Otherwise kicked from
         * server.
         * @param ttType #__USER if kicked by a user otherwise #__NONE.
         * @param user Placed in union of #TTMessage if @a ttType
         * equals #__USER otherwise #__NONE. */
        CLIENTEVENT_CMD_MYSELF_KICKED = CLIENTEVENT_NONE + 250,
        /**
         * @brief A new user logged on to the server.
         *
         * @param nSource 0
         * @param ttType #__USER
         * @param user Placed in union of #TTMessage.
         *
         * @see TT_DoLogin
         * @see TT_GetUser To retrieve user.
         * @see CLIENTEVENT_CMD_USER_LOGGEDOUT */
        CLIENTEVENT_CMD_USER_LOGGEDIN = CLIENTEVENT_NONE + 260,
        /**
         * @brief A client logged out of the server. 
         *
         * This event is called when a user logs out with
         * #TT_DoLogout or disconnects with #TT_Disconnect.
         *
         * @param nSource 0
         * @param ttType #__USER
         * @param user Placed in union of #TTMessage.
         *
         * @see TT_DoLogout
         * @see TT_Disconnect
         * @see CLIENTEVENT_CMD_USER_LOGGEDIN */
        CLIENTEVENT_CMD_USER_LOGGEDOUT = CLIENTEVENT_NONE + 270,
        /**
         * @brief User changed properties.
         *
         * @param nSource 0
         * @param ttType #__USER
         * @param user Placed in union of #TTMessage.
         *
         * @see TT_GetUser To retrieve user. */
        CLIENTEVENT_CMD_USER_UPDATE = CLIENTEVENT_NONE + 280,
        /** 
         * @brief A user has joined a channel.
         *
         * @param nSource 0
         * @param ttType #__USER
         * @param user Placed in union of #TTMessage.
         *
         * @see TT_GetUser To retrieve user. */
        CLIENTEVENT_CMD_USER_JOINED = CLIENTEVENT_NONE + 290,
        /** 
         * @brief User has left a channel.
         *
         * @param nSource Channel ID of previous channel.
         * @param ttType #__USER
         * @param user Placed in union of #TTMessage. */
        CLIENTEVENT_CMD_USER_LEFT = CLIENTEVENT_NONE + 300,
        /** 
         * @brief A user has sent a text-message.
         *
         * @param nSource 0
         * @param ttType #__TEXTMESSAGE
         * @param textmessage Placed in union of #TTMessage.
         *
         * @see TT_GetUser To retrieve user.
         * @see TT_DoTextMessage() to send text message. */
        CLIENTEVENT_CMD_USER_TEXTMSG = CLIENTEVENT_NONE + 310,
        /** 
         * @brief A new channel has been created.
         *
         * @param nSource 0
         * @param ttType #__CHANNEL
         * @param channel Placed in union of #TTMessage.
         *
         * @see TT_GetChannel To retrieve channel. */
        CLIENTEVENT_CMD_CHANNEL_NEW = CLIENTEVENT_NONE + 320,
        /** 
         * @brief A channel's properties has been updated.
         *
         * @param nSource 0
         * @param ttType #__CHANNEL
         * @param channel Placed in union of #TTMessage.
         *
         * @see TT_GetChannel To retrieve channel. */
        CLIENTEVENT_CMD_CHANNEL_UPDATE = CLIENTEVENT_NONE + 330,
        /** 
         * @brief A channel has been removed.
         *
         * Note that calling the #TT_GetChannel with the channel ID
         * will fail because the channel is no longer there.
         *
         * @param nSource 0
         * @param ttType #__CHANNEL
         * @param channel Placed in union of #TTMessage. */
        CLIENTEVENT_CMD_CHANNEL_REMOVE = CLIENTEVENT_NONE + 340,
        /** 
         * @brief Server has updated its settings (server name, MOTD,
         * etc.)
         * 
         * Get new settings in @c serverproperties of #TTMessage.
         *
         * @param nSource 0
         * @param ttType #__SERVERPROPERTIES
         * @param serverproperties Placed in union of #TTMessage. */
        CLIENTEVENT_CMD_SERVER_UPDATE = CLIENTEVENT_NONE + 350,
        /** 
         * @brief Server statistics available.
         *
         * This is a response to TT_DoServerStatistics()
         *
         * @param nSource 0
         * @param ttType #__SERVERSTATISTICS
         * @param serverstatistics Placed in union of #TTMessage. */
        CLIENTEVENT_CMD_SERVERSTATISTICS = CLIENTEVENT_NONE + 360,
        /** 
         * @brief A new file is added to a channel. 
         *
         * Use TT_GetChannelFile() to get information about the
         * file.
         *
         * @param nSource 0
         * @param ttType #__REMOTEFILE
         * @param remotefile Placed in union of #TTMessage.
         *
         * @see TT_GetChannelFile To retrieve file. */
        CLIENTEVENT_CMD_FILE_NEW = CLIENTEVENT_NONE + 370,
        /** 
         * @brief A file has been removed from a channel.
         *
         * @param nSource 0
         * @param ttType #__REMOTEFILE
         * @param remotefile Placed in union of #TTMessage. */
        CLIENTEVENT_CMD_FILE_REMOVE = CLIENTEVENT_NONE + 380,
        /** 
         * @brief A user account has been received from the server.
         *
         * This message is posted as a result of TT_DoListUserAccounts()
         *
         * @param nSource 0
         * @param ttType #__USERACCOUNT
         * @param useraccount Placed in union of #TTMessage. */
        CLIENTEVENT_CMD_USERACCOUNT = CLIENTEVENT_NONE + 390,
        /** 
         * @brief A banned user has been received from the server.
         *
         * This message is posted as a result of TT_DoListBans()
         *
         * @param nSource 0
         * @param ttType #__BANNEDUSER
         * @param useraccount Placed in union of #TTMessage. */
        CLIENTEVENT_CMD_BANNEDUSER  = CLIENTEVENT_NONE + 400,
        /**
         * @brief A user state has changed.
         *
         * The @a uUserState has changed for a user due to playback of
         * an incoming stream.
         *
         * The user state changes if:
         * - A user has started/stopped voice stream,
         *   i.e. #USERSTATE_VOICE
         * - A user has started/stopped a video capture stream,
         *   i.e. #USERSTATE_VIDEOCAPTURE
         * - A user has started/stopped a media file stream, i.e.
         *   i.e. #USERSTATE_MEDIAFILE_AUDIO or #USERSTATE_MEDIAFILE_VIDEO
         *
         * @param nSource 0
         * @param ttType #__USER.
         * @param user Placed in union of #TTMessage.
         *
         * @see TT_SetUserStoppedTalkingDelay */
        CLIENTEVENT_USER_STATECHANGE = CLIENTEVENT_NONE + 500,
        /** 
         * @brief A new video frame from a video capture device 
         * was received from a user.
         *
         * Use #TT_AcquireUserVideoCaptureFrame to display the video frame.
         *
         * @param nSource User's ID.
         * @param ttType #__INT32
         * @param nStreamID Placed in union of #TTMessage. The ID of
         * the video stream currently active for this user. If stream
         * ID becomes 0 it means the current stream has been
         * closed.  */
        CLIENTEVENT_USER_VIDEOCAPTURE = CLIENTEVENT_NONE + 510,
        /** 
         * @brief A new video frame from a video media file 
         * was received from a user.
         *
         * Use TT_AcquireUserMediaVideoFrame() to display the video frame.
         *
         * @param nSource User's ID.
         * @param ttType #__INT32
         * @param nStreamID Placed in union of #TTMessage. The ID of
         * the video stream currently active for this user. If stream
         * ID becomes 0 it means the current stream has been
         * closed.  */
        CLIENTEVENT_USER_MEDIAFILE_VIDEO = CLIENTEVENT_NONE + 520,
        /**
         * @brief A new or updated desktop window has been received
         * from a user.
         *
         * Use TT_AcquireUserDesktopWindow() to retrieve the bitmap of the
         * desktop window.
         *
         * @param nSource The user's ID.
         * @param ttType #__INT32
         * @param nStreamID Placed in union of #TTMessage. The ID of the
         * desktop window's session. If this ID changes it means the
         * user has started a new session. If the session ID becomes 0
         * it means the desktop session has been closed by the user.
         * @see TT_SendDesktopWindow() */
        CLIENTEVENT_USER_DESKTOPWINDOW = CLIENTEVENT_NONE + 530,
        /**
         * @brief A user has sent the position of the mouse cursor.
         *
         * Use TT_SendDesktopCursorPosition() to send the position of
         * the mouse cursor.
         *
         * @param nSource The user ID of the owner of the mouse cursor.
         * @param ttType #__DESKTOPINPUT
         * @param desktopinput Placed in union of #TTMessage. Contains 
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
         * TT_SendDesktopInput(). In order for the local client
         * instance to receive desktop input it must have enabled the
         * subscription #SUBSCRIBE_DESKTOPINPUT.
         *
         * Due to different keyboard layout it might be a good idea 
         * to look into the key-translation function 
         * TT_DesktopInput_KeyTranslate().
         *
         * See @ref rxdesktopinput for more information on receiving
         * desktop input.
         *
         * @param nSource User ID
         * @param ttType #__DESKTOPINPUT
         * @param desktopinput Placed in union of #TTMessage. */
        CLIENTEVENT_USER_DESKTOPINPUT = CLIENTEVENT_NONE + 550,
        /** 
         * @brief A media file recording has changed status.
         *
         * #TT_SetUserMediaStorageDir makes the client instance store all
         * audio from a user to a specified folder. Every time an
         * audio file is being processed this event is posted.
         *
         * @param nSource The user's ID.
         * @param ttType #__MEDIAFILEINFO
         * @param mediafileinfo Placed in union of
         * #TTMessage. Properties for the media file currently being
         * recorded */
        CLIENTEVENT_USER_RECORD_MEDIAFILE = CLIENTEVENT_NONE + 560,
        /**
         * @brief A new audio block can be extracted.
         *
         * The #AudioBlock can either be of #STREAMTYPE_VOICE or
         * #STREAMTYPE_MEDIAFILE_AUDIO.
         *
         * This event is only generated if TT_EnableAudioBlockEvent()
         * is first called.
         *
         * Call TT_AcquireUserAudioBlock() to extract the #AudioBlock.
         *
         * @param nSource The user ID. @see TT_LOCAL_USERID
         * @see TT_MUTEX_USERID
         * @param ttType #__STREAMTYPE */
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
         * @param ttType #__CLIENTERRORMSG
         * @param clienterrormsg Placed in union of #TTMessage. Contains
         * information on what caused an error. */
        CLIENTEVENT_INTERNAL_ERROR = CLIENTEVENT_NONE + 1000,
        /** 
         * @brief Voice activation has triggered transmission.
         *
         * @param nSource 0
         * @param ttType #__TTBOOL
         * @param bActive Placed in union of #TTMessage. TRUE if voice
         * is being transmitted due to voice level high than
         * activation level.
         *
         * @see TT_GetSoundInputLevel()
         * @see TT_SetVoiceActivationLevel()
         * @see CLIENT_SNDINPUT_VOICEACTIVATION
         * @see TT_EnableTransmission */
        CLIENTEVENT_VOICE_ACTIVATION = CLIENTEVENT_NONE + 1010,
        /** 
         * @brief A hotkey has been acticated or deactivated.
         *
         * @param nSource The hotkey ID passed to TT_HotKey_Register().
         * @param ttType #__TTBOOL
         * @param bActive Placed in union of #TTMessage. TRUE when
         * hotkey is active and FALSE when it becomes inactive.
         *
         * @see TT_HotKey_Register
         * @see TT_HotKey_Unregister */
        CLIENTEVENT_HOTKEY = CLIENTEVENT_NONE + 1020,
        /**
         * @brief A button was pressed or released on the user's
         * keyboard or mouse.
         * 
         * When #TT_HotKey_InstallTestHook is called a hook is
         * installed in Windows which intercepts all keyboard and
         * mouse presses. Every time a key or mouse is pressed or
         * released this event is posted.
         *
         * Use #TT_HotKey_GetKeyString to get a key description of the 
         * pressed key.
         *
         * @param nSource The virtual key code. Look here for a list of virtual
         * key codes: http://msdn.microsoft.com/en-us/library/ms645540(VS.85).aspx
         * @param ttType #__TTBOOL
         * @param bActive Placed in union of #TTMessage. TRUE when key
         * is down and FALSE when released.
         * @see TT_HotKey_InstallTestHook */
        CLIENTEVENT_HOTKEY_TEST = CLIENTEVENT_NONE + 1030,
        /**
         * @brief A file transfer is processing. 
         *
         * Use #TT_GetFileTransferInfo to get information about the
         * file transfer. Ensure to check if the file transfer is
         * completed, because the file transfer instance will be
         * removed from the client instance when the user application
         * reads the #FileTransfer object and it has completed the
         * transfer.
         *
         * @param nSource 0
         * @param ttType #__FILETRANSFER
         * @param filetransfer Placed in union of #TTMessage. Properties 
         * and status information about the file transfer.
         *
         * @see TT_GetFileTransferInfo To retrieve #FileTransfer. */
        CLIENTEVENT_FILETRANSFER = CLIENTEVENT_NONE + 1040,
        /**
         * @brief Used for tracking when a desktop window has been
         * transmitted to the server.
         *
         * When the transmission has completed the flag #CLIENT_TX_DESKTOP
         * will be cleared from the local client instance.
         *
         * @param nSource The desktop session's ID. If the desktop session ID
         * becomes 0 it means the desktop session has been closed and/or
         * cancelled.
         * @param ttType #__INT32
         * @param nBytesRemain Placed in union of #TTMessage. The number of
         * bytes remaining before transmission of last desktop window
         * completes. When remaining bytes is 0 TT_SendDesktopWindow()
         * can be called again. */
        CLIENTEVENT_DESKTOPWINDOW_TRANSFER = CLIENTEVENT_NONE + 1050,
        /** 
         * @brief Media file being streamed to a channel is processing.
         *
         * This event is called as a result of
         * TT_StartStreamingMediaFileToChannel() to monitor progress
         * of streaming.
         *
         * @param nSource 0
         * @param ttType #__MEDIAFILEINFO
         * @param mediafileinfo Placed in union of #TTMessage. Contains
         * properties and status information about the media file 
         * being streamed. */
        CLIENTEVENT_STREAM_MEDIAFILE = CLIENTEVENT_NONE + 1060,
        /**
         * @brief Media file played locally is processing.
         *
         * This event is called as a result of TT_InitLocalPlayback()
         * to monitor progress of playback.
         * @param nSource Session ID returned by TT_InitLocalPlayback()
         * @param ttType #__MEDIAFILEINFO
         * @param mediafileinfo Placed in union of #TTMessage. Contains
         * properties and status information about the media file
         * being played.
         */
         CLIENTEVENT_LOCAL_MEDIAFILE = CLIENTEVENT_NONE + 1070,

        /**
         * @brief Progress is audio being injected as
         * #STREAMTYPE_VOICE.
         *
         * @c nStreamID of #AudioInputProgress is the stream ID
         * provided in the #AudioBlock when calling
         * TT_InsertAudioBlock().
         *
         * When @c uElapsedMSec and @c uQueueMSec of
         * #AudioInputProgress are zero then the stream ID (session)
         * has ended. An audio input session has ended when an empty
         * #AudioBlock has been inserted using TT_InsertAudioBlock().
         *
         * @param nSource Stream ID used for sending audio input.
         * The stream ID will appear in #AudioBlock's @c nStreamID
         * on the receiving side.
         * @param ttType #__AUDIOINPUTPROGRESS
         * @param audioinputprogress Placed in union of #TTMessage.
         * Tells how much audio remains in queue. The queue should 
         * be refilled as long as the audio input should remain active.
         */
        CLIENTEVENT_AUDIOINPUT = CLIENTEVENT_NONE + 1080,
    } ClientEvent;

    /* List of structures used internally by TeamTalk. */
    typedef enum TTType
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
        __UINT32                  = 39,
        __AUDIOINPUTPROGRESS      = 40
    } TTType;

    /**
     * @brief A struct containing the properties of an event.
     *
     * The event can be retrieved by called #TT_GetMessage. This
     * struct is only required on non-Windows systems.
     *
     * Section @ref events explains event handling in the local client
     * instance.
     *
     * @see TT_GetMessage */
    typedef struct TTMessage
    {
        /** @brief The event's message number @see ClientEvent */
        ClientEvent nClientEvent;
        /** @brief The source of the event depends on @c wmMsg */
        INT32 nSource;
        /** @brief Specifies which member to access in the union */
        TTType ttType;
        /** @brief Reserved. To preserve alignment. */
        UINT32 uReserved;
        union
        {
            /** @brief Valid if @c ttType is #__CHANNEL. */
            Channel channel;
            /** @brief Valid if @c ttType is #__CLIENTERRORMSG. */
            ClientErrorMsg clienterrormsg;
            /** @brief Valid if @c ttType is #__DESKTOPINPUT. */
            DesktopInput desktopinput;
            /** @brief Valid if @c ttType is #__FILETRANSFER. */
            FileTransfer filetransfer;
            /** @brief Valid if @c ttType is #__MEDIAFILEINFO. */
            MediaFileInfo mediafileinfo;
            /** @brief Valid if @c ttType is #__REMOTEFILE. */
            RemoteFile remotefile;
            /** @brief Valid if @c ttType is #__SERVERPROPERTIES. */
            ServerProperties serverproperties;
            /** @brief Valid if @c ttType is #__SERVERSTATISTICS. */
            ServerStatistics serverstatistics;
            /** @brief Valid if @c ttType is #__TEXTMESSAGE. */
            TextMessage textmessage;
            /** @brief Valid if @c ttType is #__USER. */
            User user;
            /** @brief Valid if @c ttType is #__USERACCOUNT. */
            UserAccount useraccount;
            /** @brief Valid if @c ttType is #__BANNEDUSER. */
            BannedUser banneduser;
            /** @brief Valid if @c ttType is #__TTBOOL. */
            TTBOOL bActive;
            /** @brief Valid if @c ttType is #__INT32. */
            INT32 nBytesRemain;
            /** @brief Valid if @c ttType is #__INT32. */
            INT32 nStreamID;
            /** @brief Valid if @c ttType is #__INT32. */
            INT32 nPayloadSize;
            /** @brief Valid if @c ttType is #__STREAMTYPE. */
            StreamType nStreamType;
            /** @brief Valid if @c ttType is #__AUDIOINPUTPROGRESS. */
            AudioInputProgress audioinputprogress;
            /* brief First byte in union. */
            char data[1];
        };
    } TTMessage;

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
     * #TT_GetFlags. This enables the user application to display the
     * possible options to the end user. If e.g. the flag
     * #CLIENT_AUTHORIZED is not set it will not be possible to
     * perform any other commands except #TT_DoLogin. Doing so will
     * make the server return an error message to the client. */
    typedef enum ClientFlag
    {
        /** @brief The client instance (#TTInstance) is in closed
         * state, i.e. #TT_InitTeamTalk has return a valid instance
         * ready for use but no operations has been performed on
         * it. */
        CLIENT_CLOSED                   = 0x00000000,
        /** @brief If set the client instance's sound input device has
         * been initialized, i.e. #TT_InitSoundInputDevice has been
         * called successfully. */
        CLIENT_SNDINPUT_READY           = 0x00000001,
        /** @brief If set the client instance's sound output device
         * has been initialized, i.e. #TT_InitSoundOutputDevice has
         * been called successfully. */
        CLIENT_SNDOUTPUT_READY          = 0x00000002,
        /** @brief If set the client instance is running in sound
         * duplex mode where multiple audio output streams are mixed
         * into a single stream. This option must be enabled to
         * support echo cancellation (see TT_SetSoundInputPreprocess()). Call
         * TT_InitSoundDuplexDevices() to enable duplex mode.*/
        CLIENT_SNDINOUTPUT_DUPLEX       = 0x00000004,
        /** @brief If set the client instance will start transmitting
         * audio if the sound level is above the voice activation
         * level. The event #CLIENTEVENT_VOICE_ACTIVATION is posted
         * when voice activation initiates transmission.
         * @see TT_SetVoiceActivationLevel
         * @see TT_EnableVoiceActivation */
        CLIENT_SNDINPUT_VOICEACTIVATED  = 0x00000008,
        /** @brief If set GetSoundInputLevel() is higher than the
         * voice activation level.  To enable voice transmission if
         * voice level is higher than actication level also enable
         * #CLIENT_SNDINPUT_VOICEACTIVATED.  @see
         * TT_SetVoiceActivationLevel() @see
         * TT_EnableVoiceActivation() */
        CLIENT_SNDINPUT_VOICEACTIVE     = 0x00000010,
        /** @brief If set the client instance has muted all users.
        * @see TT_SetSoundOutputMute */
        CLIENT_SNDOUTPUT_MUTE           = 0x00000020,
        /** @brief If set the client instance will auto position users
        * in a 180 degree circle using 3D-sound. This option is only
        * available with #SOUNDSYSTEM_DSOUND.
        * @see TT_SetUserPosition 
        * @see TT_Enable3DSoundPositioning */
        CLIENT_SNDOUTPUT_AUTO3DPOSITION = 0x00000040,
        /** @brief If set the client instance's video device has been
         * initialized, i.e. #TT_InitVideoCaptureDevice has been
         * called successfuly. */
        CLIENT_VIDEOCAPTURE_READY       = 0x00000080,
        /** @brief If set the client instance is currently transmitting
         * audio.  @see TT_EnableVoiceTransmission() */
        CLIENT_TX_VOICE                 = 0x00000100,
        /** @brief If set the client instance is currently
         * transmitting video.  @see
         * TT_StartVideoCaptureTransmission() */
        CLIENT_TX_VIDEOCAPTURE          = 0x00000200,
        /** @brief If set the client instance is currently transmitting
         * a desktop window. A desktop window update is issued by calling
         * TT_SendDesktopWindow(). The event 
         * #CLIENTEVENT_DESKTOPWINDOW_TRANSFER is triggered when a desktop
         * window transmission completes. */
        CLIENT_TX_DESKTOP               = 0x00000400,
        /** @brief If set the client instance current have an active
         * desktop session, i.e. TT_SendDesktopWindow() has been
         * called. Call TT_CloseDesktopWindow() to close the desktop
         * session. */
        CLIENT_DESKTOP_ACTIVE           = 0x00000800,
        /** @brief If set the client instance is currently muxing
         * audio streams into a single file. This is enabled by calling
         * TT_StartRecordingMuxedAudioFile(). */
        CLIENT_MUX_AUDIOFILE            = 0x00001000,
        /** @brief If set the client instance is currently try to
         * connect to a server, i.e. #TT_Connect has been called. */
        CLIENT_CONNECTING               = 0x00002000,
        /** @brief If set the client instance is connected to a server,
         * i.e. #CLIENTEVENT_CON_SUCCESS event has been issued after
         * doing a #TT_Connect. Valid commands in this state:
         * #TT_DoLogin */
        CLIENT_CONNECTED                = 0x00004000,
        /** @brief Helper for #CLIENT_CONNECTING and #CLIENT_CONNECTED
         * to see if #TT_Disconnect should be called. */
        CLIENT_CONNECTION               = CLIENT_CONNECTING | CLIENT_CONNECTED,
        /** @brief If set the client instance is logged on to a
         * server, i.e. got #CLIENTEVENT_CMD_MYSELF_LOGGEDIN event
         * after issueing #TT_DoLogin. */
        CLIENT_AUTHORIZED               = 0x00008000,
        /** @brief If set the client is currently streaming the audio
         * of a media file. When streaming a video file the
         * #CLIENT_STREAM_VIDEO flag is also typically set.
         * @see TT_StartStreamingMediaFileToChannel() */
        CLIENT_STREAM_AUDIO             = 0x00010000,
        /** @brief If set the client is currently streaming the video
        of a media file. When streaming a video file the
        #CLIENT_STREAM_AUDIO flag is also typically set.
        * @see TT_StartStreamingMediaFileToChannel() */
        CLIENT_STREAM_VIDEO             = 0x00020000
    } ClientFlag;

    /** @brief A bitmask based on #ClientFlag describing the local client 
     * instance's current state.  */
    typedef UINT32 ClientFlags;
    
    /** @ingroup initclient
     *
     * @brief Pointer to a TeamTalk client instance created by
     * #TT_InitTeamTalk. @see TT_CloseTeamTalk */
    typedef VOID TTInstance;

    /** @ingroup sounddevices
     *
     * @brief Pointer to a sound loop for testing sound devices
     * created by TT_StartSoundLoopbackTest() */
    typedef VOID TTSoundLoop;

    /** @brief Get the DLL's version number. */
    TEAMTALKDLL_API const TTCHAR* TT_GetVersion(void);

#if defined(WIN32)
    /** 
     * @brief Create a new TeamTalk client instance where events are
     * posted to a HWND.
     *
     * This function must be invoked before any other of the TT_*
     * functions can be called. Call #TT_CloseTeamTalk to shutdown the
     * TeamTalk client and release its resources.
     *
     * @param hWnd The window handle which will receive the events defined
     * in #ClientEvent.
     * @param uMsg The message ID which will be passed to @c hWnd when a 
     * new event can be retrieved by TT_GetMessage().
     * @return A pointer to a new client instance. NULL if a failure occured.
     * @see TT_CloseTeamTalk */
    TEAMTALKDLL_API TTInstance* TT_InitTeamTalk(IN HWND hWnd, IN UINT32 uMsg);

    /**
     * @brief Replace the HWND passed as parameter to #TT_InitTeamTalk
     * with this HWND.
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @param hWnd The new HWND which should receive event messages. */
    TEAMTALKDLL_API TTBOOL TT_SwapTeamTalkHWND(IN TTInstance* lpTTInstance,
                                               IN HWND hWnd);
#endif

    /** 
     * @brief Create a new TeamTalk client instance where events are 
     * 'polled' using #TT_GetMessage.
     *
     * This 'polled' method can be used by application which doesn't
     * have a HWND, e.g. console applications.
     *
     * This function must be invoked before any other of the TT_*
     * functions can be called. Call #TT_CloseTeamTalk to shutdown the
     * TeamTalk client and release its resources.
     *
     * @return A pointer to a new client instance. NULL if a failure occured.
     * @see TT_CloseTeamTalk */
    TEAMTALKDLL_API TTInstance* TT_InitTeamTalkPoll(void);

    /** 
     * @brief Close the TeamTalk client instance and release its
     * resources.
     *
     * It is adviced to call this before closing the main application
     * to ensure a proper shutdown.
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @see TT_InitTeamTalk */
    TEAMTALKDLL_API TTBOOL TT_CloseTeamTalk(IN TTInstance* lpTTInstance);

    /**
     * @brief Poll for events in the client instance.
     * 
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param pMsg Pointer to a TTMessage instance which will hold the 
     * event that has occured.
     * @param pnWaitMs The amount of time to wait for the event. If NULL or -1
     * the function will block forever or until the next event occurs.
     * @return Returns TRUE if an event has occured otherwise FALSE.
     * @see TT_InitTeamTalkPolled
     * @see ClientEvent */
    TEAMTALKDLL_API TTBOOL TT_GetMessage(IN TTInstance* lpTTInstance, 
                                         OUT TTMessage* pMsg,
                                         IN const INT32* pnWaitMs);

    /**
     * @brief Cause client instance event thread to schedule an update
     * event.
     *
     * Normally all events of type #TTMessage received from
     * TT_GetMessage() are due to a state change in the client
     * instance. The state change (#ClientEvent) is submitted by the
     * client instance's internal thread. In some cases it's, however,
     * convenient to make the internal thread submit the latest
     * properties of an object. One example is after having changed
     * the volume of a #User. Then your local copy of #User will no
     * longer contain the latest @c nVolumeVoice.
     *
     * Calling TT_PumpMessage() will make the client instance's
     * internal thread queue an update of #User so the latest
     * properties of the user can be retrieved from TT_GetMessage().
     *
     * It's also possible to simply use TT_GetUser() but the problem
     * with this approach is that this call is from a separate thread
     * and therefore doesn't take the event queue into account.
     * 
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nClientEvent The event which should be queued. Currently 
     * only #CLIENTEVENT_USER_STATECHANGE is supported.
     * @param nIdentifier The ID of the object to retrieve. Currently
     * only nUserID is supported.
     *
     * @return Returns TRUE if the event has been scheduled. */
    TEAMTALKDLL_API TTBOOL TT_PumpMessage(IN TTInstance* lpTTInstance,
                                          ClientEvent nClientEvent,
                                          INT32 nIdentifier);

    /**
     * @brief Get a bitmask describing the client's current state.
     *
     * Checks whether the client is connecting, connected, authorized,
     * etc. The current state can be checked by and'ing the returned
     * bitmask which is based on #ClientFlag.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @return A bitmask describing the current state (based on 
     * #ClientFlag).
     * @see ClientFlag */
     TEAMTALKDLL_API ClientFlags TT_GetFlags(IN TTInstance* lpTTInstance);

     /**
      * @brief Set license information to disable trial mode.
      *
      * This function must be called before #TT_InitTeamTalk.
      *
      * @param szRegName The registration name provided by BearWare.dk.
      * @param szRegKey The registration key provided by BearWare.dk.
      * @return True if the provided registration is acceptable. */
     TEAMTALKDLL_API TTBOOL TT_SetLicenseInformation(IN const TTCHAR szRegName[TT_STRLEN],
                                                     IN const TTCHAR szRegKey[TT_STRLEN]);
    /** @} */

    /** @addtogroup sounddevices
     * @{ */

    /**
     * @brief Get the default sound devices. 
     *
     * @param lpnInputDeviceID The ID of the default input device.
     * @param lpnOutputDeviceID The ID of the default output device.
     * @see TT_InitSoundInputDevice
     * @see TT_InitSoundOutputDevice */
    TEAMTALKDLL_API TTBOOL TT_GetDefaultSoundDevices(OUT INT32* lpnInputDeviceID, 
                                                     OUT INT32* lpnOutputDeviceID);
    /**
     * @brief Get the default sound devices for the specified sound system.
     *
     * @see TT_GetDefaultSoundDevices() */
    TEAMTALKDLL_API TTBOOL TT_GetDefaultSoundDevicesEx(IN SoundSystem nSndSystem, 
                                                       OUT INT32* lpnInputDeviceID, 
                                                       OUT INT32* lpnOutputDeviceID);

    /**
     * @brief Retrieve list of sound devices for recording and playback.
     *
     * @param lpSoundDevices Array of SoundDevice-structs where lpnHowMany holds
     * the size of the array. Pass NULL as @a lpSoundDevices to query the 
     * number of devices.
     * @param lpnHowMany This is both an input and an output parameter. If 
     * @a lpSoundDevices is NULL lpnHowMany will after the call hold the
     * number of devices, otherwise it should hold the size of the
     * @a lpSoundDevices array.
     * @see TT_GetDefaultSoundDevices
     * @see TT_InitSoundInputDevice
     * @see TT_InitSoundOutputDevice */
    TEAMTALKDLL_API TTBOOL TT_GetSoundDevices(IN OUT SoundDevice* lpSoundDevices,
                                              IN OUT INT32* lpnHowMany);


    /**
     * @brief Reinitialize sound system (in order to detect
     * new/removed devices).
     *
     * When the TeamTalk client is first initialized all the sound
     * devices are detected and stored in a list inside the client
     * instance. If a user adds or removes e.g. a USB sound device
     * then it's not picked up automatically by the client
     * instance. TT_RestartSoundSystem() can be used to reinitialize
     * the sound system and thereby detect if sound devices have been
     * removed or added.
     *
     * In order to restart the sound system all sound devices in all
     * client instances must be closed using TT_CloseSoundInputDevice(),
     * TT_CloseSoundoutputDevice() and TT_CloseSoundDuplexDevices(). */
    TEAMTALKDLL_API TTBOOL TT_RestartSoundSystem(void);

    /**
     * @brief Perform a record and playback test of specified sound
     * devices along with an audio configuration.
     *
     * Call TT_CloseSoundLoopbackTest() to stop the loopback
     * test.
     *
     * This function allows the use of #SpeexDSP to enable AGC and echo
     * cancellation.
     *
     * @param nInputDeviceID Should be the @a nDeviceID extracted through 
     * TT_GetSoundDevices().
     * @param nOutputDeviceID Should be the @a nDeviceID extracted through 
     * TT_GetSoundDevices().
     * @param nSampleRate The sample rate the client's recorder should 
     * use.
     * @param nChannels Number of channels to use, i.e. 1 = mono, 2 = stereo.
     * @param bDuplexMode Both input and output devices MUST support
     * the specified sample rate since this loop back test uses duplex
     * mode ( @see TT_InitSoundDuplexDevices() ). Check out @c
     * supportedSampleRates of #SoundDevice to see which sample rates
     * are supported.
     * @param lpSpeexDSP The preprocessing settings to use, i.e. AGC 
     * and denoising properties. Pass NULL to ignore AGC, denoise and AEC.
     * @return Returns NULL in case of error, otherwise sound loop instance
     * which can be closed by TT_CloseSoundLoopbackTest();
     * @see TT_InitSoundInputDevice()
     * @see TT_InitSoundOutputDevice()
     * @see TT_InitSoundDuplexDevices()
     * @see TT_CloseSoundLoopbackTest() */
    TEAMTALKDLL_API TTSoundLoop* TT_StartSoundLoopbackTest(IN INT32 nInputDeviceID, 
                                                           IN INT32 nOutputDeviceID,
                                                           IN INT32 nSampleRate,
                                                           IN INT32 nChannels,
                                                           IN TTBOOL bDuplexMode,
                                                           IN const SpeexDSP* lpSpeexDSP);
    
    /**
     * @brief Stop recorder and playback test.
     *
     * @c lpTTSoundLoop is destroyed after this call.
     *
     * @param lpTTSoundLoop Pointer to sound loop instance created by 
     * TT_StartSoundLoopbackTest().
     * @see TT_InitSoundInputDevice
     * @see TT_InitSoundOutputDevice
     * @see TT_StartSoundLoopbackTest */
    TEAMTALKDLL_API TTBOOL TT_CloseSoundLoopbackTest(IN TTSoundLoop* lpTTSoundLoop);

    /**
     * @brief Initialize the sound input device (for recording audio).
     *
     * The @a nDeviceID of the #SoundDevice should be used as @a 
     * nInputDeviceID.
     *
     * The @c nInputDeviceID can be or'ed with
     * #TT_SOUNDDEVICE_ID_SHARED_FLAG if the #TTInstance should share
     * recording device with other instances.
     *
     * Notice fixed sound device ID for some platforms:
     * - iOS
     *   - #TT_SOUNDDEVICE_ID_REMOTEIO
     *   - #TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO
     * - Android
     *   - #TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT
     * - All platforms
     *   - #TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL
     *
     * Calling this function will set the flag #CLIENT_SNDINPUT_READY.
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @param nInputDeviceID The @a nDeviceID of #SoundDevice extracted 
     * through TT_GetSoundDevices().
     * @see SoundDevice
     * @see TT_GetDefaultSoundDevices
     * @see TT_GetSoundDevices
     * @see TT_CloseSoundInputDevice
     * @see TT_GetSoundInputLevel */
    TEAMTALKDLL_API TTBOOL TT_InitSoundInputDevice(IN TTInstance* lpTTInstance, 
                                                   IN INT32 nInputDeviceID);

    /** 
     * @brief Initialize the sound output device (for audio playback).
     *
     * The @a nDeviceID of the #SoundDevice should be used as @a 
     * nOutputDeviceID.
     *
     * The @c nOutputDeviceID can be or'ed with
     * #TT_SOUNDDEVICE_ID_SHARED_FLAG if the #TTInstance should share
     * output device with other instances.
     *
     * Notice fixed sound device ID for some platforms:
     * - iOS
     *   - #TT_SOUNDDEVICE_ID_REMOTEIO
     *   - #TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO
     * - Android
     *   - #TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT
     * - All platforms
     *   - #TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL
     *
     * Callling this function will set the flag
     * #CLIENT_SNDOUTPUT_READY.
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @param nOutputDeviceID The @a nDeviceID of #SoundDevice extracted
     * through TT_GetSoundDevices().
     * @see SoundDevice
     * @see TT_GetDefaultSoundDevices
     * @see TT_GetSoundDevices
     * @see TT_CloseSoundOutputDevice */
    TEAMTALKDLL_API TTBOOL TT_InitSoundOutputDevice(IN TTInstance* lpTTInstance, 
                                                    IN INT32 nOutputDeviceID);

    /**
     * @brief Enable duplex mode where multiple audio streams are
     * mixed into a single stream using software.
     *
     * Duplex mode can @b ONLY be enabled on sound devices which
     * support the same sample rate. Sound systems #SOUNDSYSTEM_WASAPI
     * and #SOUNDSYSTEM_ALSA typically only support a single sample
     * rate.  Check @c supportedSampleRates in #SoundDevice to see
     * which sample rates are supported.
     *
     * Sound duplex mode is required for echo cancellation since sound
     * input and output device must be synchronized. Also sound cards
     * which does not support multiple output streams should use
     * duplex mode.
     *
     * If TT_InitSoundDuplexDevices() is successful the following
     * flags will be set:
     *
     * - #CLIENT_SNDINOUTPUT_DUPLEX
     * - #CLIENT_SNDOUTPUT_READY
     * - #CLIENT_SNDINPUT_READY
     *
     * Call TT_CloseSoundDuplexDevices() to shut down duplex mode.
     *
     * Note that it is only the audio streams from users in the local
     * client instance's current channel which will be mixed. If the
     * local client instance calls TT_DoSubscribe() with
     * #SUBSCRIBE_INTERCEPT_VOICE on a user in another channel then
     * the audio from this user will be started in a separate
     * stream. The reason for this is that the other user may use a
     * different audio codec.
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @param nInputDeviceID The @a nDeviceID of #SoundDevice extracted 
     * through TT_GetSoundDevices().
     * @param nOutputDeviceID The @a nDeviceID of #SoundDevice extracted
     * through TT_GetSoundDevices().
     * @see TT_InitSoundInputDevice()
     * @see TT_InitSoundOutputDevice()
     * @see TT_CloseSoundDuplexDevices() */
    TEAMTALKDLL_API TTBOOL TT_InitSoundDuplexDevices(IN TTInstance* lpTTInstance, 
                                                     IN INT32 nInputDeviceID,
                                                     IN INT32 nOutputDeviceID);

    /**
     * @brief Shutdown the input sound device.
     *
     * Callling this function will clear the flag
     * #CLIENT_SNDINPUT_READY.
     *
     * If the local client instance is running in duplex mode (flag
     * #CLIENT_SNDINOUTPUT_DUPLEX is set) then trying to close the
     * sound device will fail since duplex mode require that both
     * input and output sound devices are active at the same
     * time. Therefore in order to close sound devices running in
     * duplex mode call TT_CloseSoundDuplexDevices().
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @return If running in sound duplex mode (#CLIENT_SNDINOUTPUT_DUPLEX)
     * then ensure to disable duplex mode prior to closing the sound device.
     * @see TT_InitSoundInputDevice */
    TEAMTALKDLL_API TTBOOL TT_CloseSoundInputDevice(IN TTInstance* lpTTInstance);

    /**
     * @brief Shutdown the output sound device.
     *
     * Callling this function will clear set the flag
     * #CLIENT_SNDOUTPUT_READY.
     *
     * If the local client instance is running in duplex mode (flag
     * #CLIENT_SNDINOUTPUT_DUPLEX is set) then trying to close the
     * sound device will fail since duplex mode require that both
     * input and output sound devices are active at the same
     * time. Therefore in order to close sound devices running in
     * duplex mode call TT_CloseSoundDuplexDevices().
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @return If running in sound duplex mode (#CLIENT_SNDINOUTPUT_DUPLEX)
     * then ensure to disable duplex mode prior to closing the sound device.
     * @see TT_InitSoundOutputDevice */
    TEAMTALKDLL_API TTBOOL TT_CloseSoundOutputDevice(IN TTInstance* lpTTInstance);

    /**
     * @brief Shut down sound devices running in duplex mode.
     *
     * Calling this function only applies if sound devices has been
     * initialized with TT_InitSoundDuplexDevices().
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk. */
    TEAMTALKDLL_API TTBOOL TT_CloseSoundDuplexDevices(IN TTInstance* lpTTInstance);

    /**
     * @brief Get the volume level of the current recorded audio.
     *
     * The current level is updated at an interval specified in a channel's
     * #AudioCodec.
     *
     * Note that the volume level will not be available until the
     * client instance joins a channel, i.e. it knows what sample rate
     * to use.
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @return Returns a value between #SOUND_VU_MIN and #SOUND_VU_MAX. */
    TEAMTALKDLL_API INT32 TT_GetSoundInputLevel(IN TTInstance* lpTTInstance);

    /** 
     * @brief Set voice gaining of recorded audio. 
     *
     * The gain level ranges from #SOUND_GAIN_MIN to #SOUND_GAIN_MAX
     * where #SOUND_GAIN_DEFAULT is no gain. So 100 is 1/10 of the
     * original volume and 8000 is 8 times the original volume.
     *
     * Note that using TT_SetSoundInputPreprocess() will override
     * settings an input gain level. This is because automatic gain
     * control will adjust the volume level.
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @param nLevel A value from #SOUND_GAIN_MIN to #SOUND_GAIN_MAX.
     * @see TT_GetSoundInputGainLevel */
    TEAMTALKDLL_API TTBOOL TT_SetSoundInputGainLevel(IN TTInstance* lpTTInstance, 
                                                     IN INT32 nLevel);

    /**
     * @brief Get voice gain level of outgoing audio
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @return A value from #SOUND_GAIN_MIN to #SOUND_GAIN_MAX.
     * @see TT_SetSoundInputGainLevel */
    TEAMTALKDLL_API INT32 TT_GetSoundInputGainLevel(IN TTInstance* lpTTInstance);

    /**
     * @brief Enable sound preprocessor which should be used for
     * processing audio recorded by the sound input device (voice input).
     *
     * To ensure common settings for all users in a channel it's
     * possible to use the @c audiocfg member of #Channel as shared
     * source for audio settings.
     *
     * In order for echo cancellation to work best it's important to
     * also enable AGC in the #SpeexDSP.
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @param lpSpeexDSP The sound preprocessor settings to use. 
     * Preferably from the #Channel's @c audiocfg member to ensure common
     * settings for all users.
     * @return TRUE on success, FALSE on failure. */
    TEAMTALKDLL_API TTBOOL TT_SetSoundInputPreprocess(IN TTInstance* lpTTInstance,
                                                      const IN SpeexDSP* lpSpeexDSP);

    /** 
     * @brief Get the sound preprocessor settings which are currently in use
     * for recorded sound input device (voice input).
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @param lpSpeexDSP A preallocated SpeexDSP which will 
     * receive the settings that is currently in effect.
     *
     * @return TRUE on success, FALSE on failure. */
    TEAMTALKDLL_API TTBOOL TT_GetSoundInputPreprocess(IN TTInstance* lpTTInstance,
                                                      OUT SpeexDSP* lpSpeexDSP);

    /**
     * @brief Set master volume. 
     *
     * If still not loud enough use #TT_SetUserVolume.
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @param nVolume A value from #SOUND_VOLUME_MIN to  #SOUND_VOLUME_MAX.
     * @see TT_SetUserVolume */
    TEAMTALKDLL_API TTBOOL TT_SetSoundOutputVolume(IN TTInstance* lpTTInstance, 
                                                   IN INT32 nVolume);

    /**
     * @brief Get master volume.
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @return Returns the master volume.
     * @see SOUND_VOLUME_MAX
     * @see SOUND_VOLUME_MIN */
    TEAMTALKDLL_API INT32 TT_GetSoundOutputVolume(IN TTInstance* lpTTInstance);

    /**
     * @brief Set all users mute.
     *
     * To stop receiving audio from a user call #TT_DoUnsubscribe.
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @param bMuteAll Whether to mute or unmute all users.
     * @see CLIENT_SNDOUTPUT_MUTE */
    TEAMTALKDLL_API TTBOOL TT_SetSoundOutputMute(IN TTInstance* lpTTInstance, 
                                                 IN TTBOOL bMuteAll);

    /** 
     * @brief Enable automatically position users using 3D-sound.
     *
     * Note that 3d-sound does not work if sound is running in duplex
     * mode (#CLIENT_SNDINOUTPUT_DUPLEX).
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param bEnable TRUE to enable, otherwise FALSE.
     * @see TT_SetUserPosition */
    TEAMTALKDLL_API TTBOOL TT_Enable3DSoundPositioning(IN TTInstance* lpTTInstance, 
                                                       IN TTBOOL bEnable);

    /** 
     * @brief Automatically position users using 3D-sound.
     *
     * Note that 3d-sound does not work if sound is running in duplex
     * mode (#CLIENT_SNDINOUTPUT_DUPLEX).
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @see TT_SetUserPosition */
    TEAMTALKDLL_API TTBOOL TT_AutoPositionUsers(IN TTInstance* lpTTInstance);

    /**
     * @brief Enable/disable access to raw audio from individual
     * users, local microphone input or muxed stream of all users.
     *
     * With audio block event enabled all audio which has been played
     * will be accessible by calling TT_AcquireUserAudioBlock(). Every
     * time a new #AudioBlock is available the event
     * #CLIENTEVENT_USER_AUDIOBLOCK is generated.
     * 
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID The user ID to monitor for audio callback. Pass
     * special user ID #TT_LOCAL_USERID to monitor local recorded
     * audio prior to encoding/processing. Pass special user ID
     * #TT_MUXED_USERID to get a single audio stream of all audio that
     * is being played from users.
     * @param nStreamType Either #STREAMTYPE_VOICE or 
     * #STREAMTYPE_MEDIAFILE_AUDIO.
     * @param bEnable Whether to enable the #CLIENTEVENT_USER_AUDIOBLOCK event.
     * @see TT_AcquireUserAudioBlock()
     * @see TT_ReleaseUserAudioBlock()
     * @see CLIENTEVENT_USER_AUDIOBLOCK */
    TEAMTALKDLL_API TTBOOL TT_EnableAudioBlockEvent(IN TTInstance* lpTTInstance,
                                                    IN INT32 nUserID,
                                                    IN StreamType nStreamType,
                                                    IN TTBOOL bEnable);

    /** @} */

    /** @addtogroup transmission
     * @{ */

    /**
     * @brief Transmit application provided raw audio in
     * #AudioBlock-structs as #STREAMTYPE_VOICE, i.e. microphone
     * input.
     *
     * Since #STREAMTYPE_VOICE is being replaced by audio input this
     * means that while audio input is active then subsequent calls to
     * TT_EnableVoiceTransmission() or TT_EnableVoiceActivation() will
     * fail until the audio input has ended.
     *
     * If the flags #CLIENT_TX_VOICE or
     * #CLIENT_SNDINPUT_VOICEACTIVATED are active then calling
     * TT_InputAudioBlock() will fail because #STREAMTYPE_VOICE is
     * already in use.
     *
     * TT_InsertAudioBlock() can be called multiple times until the
     * client instance's internal queue is full. When the queue has
     * been filled then monitor #CLIENTEVENT_AUDIOINPUT to see when
     * more data can be queued.
     *
     * To end raw audio input set parameter @c lpAudioBlock to NULL
     * and then TT_EnableVoiceTransmission() or
     * TT_StartStreamingMediaFileToChannel() will be available again.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param lpAudioBlock The audio to submit as audio input.
     * The member @c nStreamID of #AudioBlock is used to identify the
     * audio input session which is currently in progress and is
     * posted as the @c nSource of #CLIENTEVENT_AUDIOINPUT.
     * The member @c uSampleIndex of #AudioBlock is ignored. */
    TEAMTALKDLL_API TTBOOL TT_InsertAudioBlock(IN TTInstance* lpTTInstance,
                                               IN const AudioBlock* lpAudioBlock);
    
    /**
     * @brief Start/stop transmitting of voice data from sound input.
     *
     * Sound input is initialized using TT_InitSoundInputDevice() or
     * TT_InitSoundDuplexDevices().
     *
     * Voice transmission is stream type #STREAMTYPE_VOICE.
     *
     * User rights required:
     * - #USERRIGHT_TRANSMIT_VOICE
     *
     * Note that voice activation cannot be enabled when
     * TT_InsertAudioBlock() is active.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param bEnable Enable/disable transmission.
     * @return TRUE on success. FALSE if voice transmission could
     * not be activated on the client instance. */
    TEAMTALKDLL_API TTBOOL TT_EnableVoiceTransmission(IN TTInstance* lpTTInstance,
                                                      IN TTBOOL bEnable);

    /**
     * @brief Enable voice activation.
     *
     * The client instance will start transmitting audio if the
     * recorded audio level is above or equal to the voice activation
     * level set by TT_SetVoiceActivationLevel(). Once the voice
     * activation level is reached the event
     * #CLIENTEVENT_VOICE_ACTIVATION is posted.
     *
     * The current volume level can be queried calling
     * #TT_GetSoundInputLevel.
     *
     * Voice transmission is stream type #STREAMTYPE_VOICE.
     *
     * User rights required:
     * - #USERRIGHT_TRANSMIT_VOICE
     *
     * Note that voice activation cannot be enabled when
     * TT_InsertAudioBlock() is active.
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @param bEnable TRUE to enable, otherwise FALSE.
     * @return TRUE on success. FALSE if voice activation cannot 
     * be enabled on the client instance.
     *
     * @see CLIENT_SNDINPUT_VOICEACTIVATION
     * @see TT_SetVoiceActivationStopDelay */
    TEAMTALKDLL_API TTBOOL TT_EnableVoiceActivation(IN TTInstance* lpTTInstance, 
                                                    IN TTBOOL bEnable);

    /** 
     * @brief Set voice activation level.
     *
     * The current volume level can be queried calling
     * #TT_GetSoundInputLevel.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nLevel Must be between #SOUND_VU_MIN and #SOUND_VU_MAX
     * @see TT_EnableVoiceActivation
     * @see TT_GetVoiceActivationLevel
     * @see TT_SetVoiceActivationStopDelay */
    TEAMTALKDLL_API TTBOOL TT_SetVoiceActivationLevel(IN TTInstance* lpTTInstance, 
                                                      IN INT32 nLevel);

    /** 
     * @brief Get voice activation level.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @return Returns A value between #SOUND_VU_MIN and #SOUND_VU_MAX
     * @see TT_EnableVoiceActivation
     * @see TT_SetVoiceActivationLevel */
    TEAMTALKDLL_API INT32 TT_GetVoiceActivationLevel(IN TTInstance* lpTTInstance);

    /**
     * @brief Set the delay of when voice activation should be stopped.
     *
     * When TT_GetSoundInputLevel() becomes higher than the specified
     * voice activation level the client instance will start
     * transmitting until TT_GetSoundInputLevel() becomes lower than
     * the voice activation level, plus a delay. This delay is by
     * default set to 1500 msec but this value can be changed by
     * calling TT_SetVoiceActivationStopDelay().
     *
     * @see TT_EnableVoiceActivation
     * @see TT_SetVoiceActivationLevel */
    TEAMTALKDLL_API TTBOOL TT_SetVoiceActivationStopDelay(IN TTInstance* lpTTInstance,
                                                          IN INT32 nDelayMSec);

    /**
     * @brief Get the delay of when voice active state should be disabled.
     *
     * @return The number of miliseconds before voice activated state
     * should be turned back to inactive.
     *
     * @see TT_SetVoiceActivationStopDelay
     * @see TT_EnableVoiceActivation
     * @see TT_SetVoiceActivationLevel */
    TEAMTALKDLL_API INT32 TT_GetVoiceActivationStopDelay(IN TTInstance* lpTTInstance);

    /**
     * @brief Store all audio conversations with specific #AudioCodec
     * settings to a single file.
     *
     * To record conversations from a specific channel to a single
     * file call TT_StartRecordingMuxedAudioFileEx().
     *
     * TT_StartRecordingMuxedAudioFile() can be used to record
     * conversations "across" channels given that the channels use the
     * same #AudioCodec properties (i.e. @c audiocodec member of
     * #Channel). To receive audio outside the TeamTalk instance's
     * channel use TT_DoSubscribe() and #SUBSCRIBE_INTERCEPT_VOICE.
     *
     * If the user changes to a channel which uses a different audio
     * codec then the recording will continue but simply be silent
     * until the user again joins a channel with the same audio codec
     * as was used for initializing muxed audio recording.
     *
     * Calling TT_StartRecordingMuxedAudioFile() will enable the
     * #CLIENT_MUX_AUDIOFILE flag from TT_GetFlags().
     *
     * Call TT_StopRecordingMuxedAudioFile() to stop recording. Note
     * that only one muxed audio recording can be active at the same
     * time.
     *
     * Only #STREAMTYPE_VOICE is stored into the audio file, not
     * #STREAMTYPE_MEDIAFILE_AUDIO.
     *
     * Use TT_SetUserMediaStorageDir() to store users' audio streams
     * in separate files.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param lpAudioCodec The audio codec which should be used as
     * reference for muxing users' audio streams. In most situations
     * this is the #AudioCodec of the current channel, i.e.
     * TT_GetMyChannelID().
     * @param szAudioFileName The file to store audio to, e.g. 
     * C:\\MyFiles\\Conf.mp3.
     * @param uAFF The audio format which should be used in the recorded
     * file. The muxer will convert to this format.
     *
     * @see TT_SetUserMediaStorageDir()
     * @see TT_StartRecordingMuxedAudioFileEx()
     * @see TT_StopRecordingMuxedAudioFile() */
    TEAMTALKDLL_API TTBOOL TT_StartRecordingMuxedAudioFile(IN TTInstance* lpTTInstance,
                                                           IN const AudioCodec* lpAudioCodec,
                                                           IN const TTCHAR* szAudioFileName,
                                                           IN AudioFileFormat uAFF);

    /**
     * @brief Store audio conversations from a specific channel into a
     * single file.
     *
     * To record audio outside the #TTInstance's current channel use
     * the TT_DoSubscribe() with the #SUBSCRIBE_INTERCEPT_VOICE on all
     * the user's in the channel.
     *
     * Unlike TT_StartRecordingMuxedAudioFile() this function does not
     * toggle the flag #CLIENT_MUX_AUDIOFILE.
     *
     * Use TT_StartRecordingMuxedAudioFile() to record conversations
     * from many different channels with the same #AudioCodec
     * settings.
     *
     * Only #STREAMTYPE_VOICE is stored into the audio file, not
     * #STREAMTYPE_MEDIAFILE_AUDIO.
     *
     * @see TT_StopRecordingMuxedAudioFileEx() */
    TEAMTALKDLL_API TTBOOL TT_StartRecordingMuxedAudioFileEx(IN TTInstance* lpTTInstance,
                                                             IN INT32 nChannelID,
                                                             IN const TTCHAR* szAudioFileName,
                                                             IN AudioFileFormat uAFF);
    
    /**
     * @brief Stop an active muxed audio recording.
     *
     * A muxed audio recording started with
     * TT_StartRecordingMuxedAudioFile() can be stopped using this
     * function.
     *
     * Calling TT_StopRecordingMuxedAudioFile() will clear the
     * #CLIENT_MUX_AUDIOFILE flag from TT_GetFlags().
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     *
     * @see TT_StartRecordingMuxedAudioFile() */
    TEAMTALKDLL_API TTBOOL TT_StopRecordingMuxedAudioFile(IN TTInstance* lpTTInstance);

    /**
     * @brief Stop recording conversations from a channel to a single file.
     *
     * Stop a recording initiated by TT_StartRecordingMuxedAudioFileEx().
     *
     * @see TT_StopRecordingMuxedAudioFile() */
    TEAMTALKDLL_API TTBOOL TT_StopRecordingMuxedAudioFileEx(IN TTInstance* lpTTInstance,
                                                            IN INT32 nChannelID);
    
    /** 
     * @brief Start transmitting from video capture device.
     *
     * The video capture device is initiated by calling
     * TT_InitVideoCaptureDevice(). After joining a channel and
     * calling this function the other users will see the video from
     * the capture device.
     *
     * Video capture data is transmitted with stream type
     * #STREAMTYPE_VIDEOCAPTURE and is subscribed/unsubscribed using
     * #SUBSCRIBE_VIDEOCAPTURE.
     *
     * To stop transmitting call TT_StopVideoCaptureTransmission()
     *
     * User rights required:
     * - #USERRIGHT_TRANSMIT_VIDEOCAPTURE.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param lpVideoCodec The video codec settings to use for 
     * transmission.
     *
     * @see TT_StartStreamingMediaFileToChannel()
     * @see TT_EnableVoiceTransmission() */
    TEAMTALKDLL_API TTBOOL TT_StartVideoCaptureTransmission(IN TTInstance* lpTTInstance,
                                                            IN const VideoCodec* lpVideoCodec);

    /**
     * @brief Stop transmitting from video capture device.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @see TT_StartVideoCaptureTransmission() */
    TEAMTALKDLL_API TTBOOL TT_StopVideoCaptureTransmission(IN TTInstance* lpTTInstance);

    /** @} */

    /** @addtogroup videocapture
     * @{ */

    /**
     * @brief Get the list of devices available for video capture.
     *
     * @param lpVideoDevices Array of VideoCaptureDevice-stucts where
     * @a lpnHowMany hold the size of the array. Pass NULL to query
     * the number of devices.
     * @param lpnHowMany This is both an input and output
     * parameter. If @a lpVideoDevices is NULL @a lpnHowMany will after
     * the call hold the number of devices, otherwise it should hold
     * the size of the @a lpVideoDevices array.
     * @see TT_InitVideoCaptureDevice */
    TEAMTALKDLL_API TTBOOL TT_GetVideoCaptureDevices(IN OUT VideoCaptureDevice* lpVideoDevices,
                                                     IN OUT INT32* lpnHowMany);

    /**
     * @brief Initialize a video capture device.
     *
     * To transmit video capture data to a channel call
     * TT_StartVideoCaptureTransmission()
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param szDeviceID The device idenfier @a szDeviceID of #VideoCaptureDevice.
     * @param lpVideoFormat The capture format to use,
     * i.e. frame-rate, resolution and picture format.
     * @see TT_GetVideoCaptureDevices
     * @see TT_CloseVideoCaptureDevice */
    TEAMTALKDLL_API TTBOOL TT_InitVideoCaptureDevice(IN TTInstance* lpTTInstance,
                                                     IN const TTCHAR* szDeviceID,
                                                     IN const VideoFormat* lpVideoFormat);
    /**
     * @brief Close a video capture device.
     *
     * @see TT_InitVideoCaptureDevice */
    TEAMTALKDLL_API TTBOOL TT_CloseVideoCaptureDevice(IN TTInstance* lpTTInstance);

#if defined(WIN32)
    /**
     * @brief Paint user's video frame using a Windows' DC (device
     * context).
     *
     * Same as calling TT_PaintVideoFrameEx() like this:
     *
       @verbatim
       TT_PaintVideoFrameEx(lpTTInstance, nUserID, hDC, 
                            XDest, YDest, nDestWidth,
                            nDestHeight, 0, 0, 
                            src_bmp_width, src_bmp_height);
       @endverbatim
     *
     * @c src_bmp_width and @c src_bmp_height are extracted internally
     * from the source image. */
    TEAMTALKDLL_API TTBOOL TT_PaintVideoFrame(IN HDC hDC,
                                              IN INT32 XDest,
                                              IN INT32 YDest,
                                              IN INT32 nDestWidth,
                                              IN INT32 nDestHeight,
                                              IN VideoFrame* lpVideoFrame);

    /** 
     * @brief Paint user's video frame using a Windows' DC (device
     * context).
     *
     * An application can either paint using #TT_AcquireUserVideoCaptureFrame
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
     * @param lpVideoFrame Video frame retrieved by TT_AcquireUserVideoCaptureFrame()
     * @see TT_AcquireUserVideoCaptureFrame */
    TEAMTALKDLL_API TTBOOL TT_PaintVideoFrameEx(IN HDC hDC,
                                                IN INT32 XDest,
                                                IN INT32 YDest,
                                                IN INT32 nDestWidth,
                                                IN INT32 nDestHeight,
                                                IN INT32 XSrc,
                                                IN INT32 YSrc,
                                                IN INT32 nSrcWidth,
                                                IN INT32 nSrcHeight,
                                                IN VideoFrame* lpVideoFrame);
#endif

    /** @brief Extract a user's video capture frame for display.
     *
     * The #VideoFrame extracted from the client instance will contain a
     * pointer to the image's frame buffer, so a RGB32 bitmap can be
     * displayed in a window control.
     *
     * To release the acquired #VideoFrame call
     * TT_ReleaseUserVideoCaptureFrame().
     *
     * A video capture frame comes from a user's
     * #STREAMTYPE_VIDEOCAPTURE.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param nUserID The user's ID. 0 for local user.
     * @return Returns video frame which will contain the image data. Note 
     * that it's the @a frameBuffer member of #VideoFrame which will contain 
     * the image data allocated internally by TeamTalk. Returns NULL if no 
     * video frame could be acquired.
     * @see TT_ReleaseUserVideoCaptureFrame */
    TEAMTALKDLL_API VideoFrame* TT_AcquireUserVideoCaptureFrame(IN TTInstance* lpTTInstance,
                                                                IN INT32 nUserID);

    /** @brief Delete a user's video frame, acquired through
     * TT_AcquireUserVideoCaptureFrame(), so its allocated resources can be
     * released.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param lpVideoFrame Pointer to #VideoFrame which should be deallocated. 
     * @return Returns TRUE If a video frame was successfully deallocated.
     * @see TT_AcquireUserVideoCaptureFrame */
    TEAMTALKDLL_API TTBOOL TT_ReleaseUserVideoCaptureFrame(IN TTInstance* lpTTInstance,
                                                           IN VideoFrame* lpVideoFrame);
    /** @} */

    /** @addtogroup mediastream
     * @{ */

    /** @brief Stream media file to channel, e.g. avi-, wav- or MP3-file.
     * @see TT_StartStreamingMediaFileToChannelEx() */
    TEAMTALKDLL_API TTBOOL TT_StartStreamingMediaFileToChannel(IN TTInstance* lpTTInstance,
                                                               IN const TTCHAR* szMediaFilePath,
                                                               IN const VideoCodec* lpVideoCodec);

    /**
     * @brief Stream media file to channel, e.g. avi, wav or MP3-file.
     *
     * Call TT_GetMediaFileInfo() to get the properties of a media
     * file, i.e. audio and video format.
     *
     * The event #CLIENTEVENT_STREAM_MEDIAFILE  is posted when
     * the media file starts streaming. The flags #CLIENT_STREAM_AUDIO
     * and/or #CLIENT_STREAM_VIDEO will be set if the call is successful.
     *
     * A media file is streamed using #STREAMTYPE_MEDIAFILE_AUDIO
     * and/or #STREAMTYPE_MEDIAFILE_VIDEO. To subscribe/unsubscribe a
     * media file being streamed use #SUBSCRIBE_MEDIAFILE.
     *
     * Streaming a media file requires
     * #USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO and/or
     * #USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param szMediaFilePath File path to media file.
     * @param lpMediaFilePlayback Playback settings to pause, seek and
     * preprocess audio. If #SPEEXDSP_AUDIOPREPROCESSOR then the echo
     * cancellation part of #SpeexDSP is unused. Only denoise and AGC
     * settings are applied.
     * @param lpVideoCodec If video file then specify output codec properties 
     * here, otherwise NULL.
     *
     * @see TT_UpdateStreamingMediaFileToChannel()
     * @see TT_StopStreamingMediaFileToChannel()
     * @see TT_InitLocalPlayback() */
    TEAMTALKDLL_API TTBOOL TT_StartStreamingMediaFileToChannelEx(IN TTInstance* lpTTInstance,
                                                                 IN const TTCHAR* szMediaFilePath,
                                                                 IN const MediaFilePlayback* lpMediaFilePlayback,
                                                                 IN const VideoCodec* lpVideoCodec);

    /**
     * @brief Update active media file being streamed to channel.
     *
     * While streaming a media file to a channel it's possible to
     * pause, seek and manipulate audio preprocessing by passing new
     * #MediaFilePlayback properties.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param lpMediaFilePlayback Playback settings to pause, seek and
     * preprocess audio. If #SPEEXDSP_AUDIOPREPROCESSOR then the echo
     * cancellation part of #SpeexDSP is unused. Only denoise and AGC
     * settings are applied.
     * @param lpVideoCodec If video file then specify output codec properties 
     * here, otherwise NULL.
     *
     * @see TT_StartStreamingMediaFileToChannel()
     * @see TT_StopStreamingMediaFileToChannel() */
    TEAMTALKDLL_API TTBOOL TT_UpdateStreamingMediaFileToChannel(IN TTInstance* lpTTInstance,
                                                                IN const MediaFilePlayback* lpMediaFilePlayback,
                                                                IN const VideoCodec* lpVideoCodec);

    /**
     * @brief Stop streaming media file to channel.
     *
     * This will clear the flags #CLIENT_STREAM_AUDIO
     * and/or #CLIENT_STREAM_VIDEO.
     *
     * @see TT_StartStreamingMediaFileToChannel() */
    TEAMTALKDLL_API TTBOOL TT_StopStreamingMediaFileToChannel(IN TTInstance* lpTTInstance);

    /**
     * Play media file using settings from @c lpTTInstance,
     * i.e. TT_SetSoundOutputMute(), TT_SetSoundOutputVolume() and
     * TT_InitSoundOutputDevice().
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param szMediaFilePath Path to media file.
     * @param lpMediaFilePlayback Playback settings to pause, seek and
     * preprocess audio. If #SPEEXDSP_AUDIOPREPROCESSOR then the echo
     * cancellation part of #SpeexDSP is unused. Only denoise and AGC
     * settings are applied.
     *
     * @return A Session ID for identifing the media playback session.
     * If Session ID is <= 0 indicates an error.
     *
     * @return A session ID identifier referred to as @c nPlaybackSessionID.
     * 
     * @see TT_UpdateLocalPlayback()
     * @see TT_StopLocalPlayback()
     * @see TT_StartStreamingMediaFileToChannel() */
    TEAMTALKDLL_API INT32 TT_InitLocalPlayback(IN TTInstance* lpTTInstance,
                                               IN const TTCHAR* szMediaFilePath,
                                               IN const MediaFilePlayback* lpMediaFilePlayback);

    /**
     * Update media file currently being played by TeamTalk instance.
     * 
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param nPlaybackSessionID Session ID created by TT_InitLocalPlayback().
     * @param lpMediaFilePlayback #AudioPreprocessorType of
     * #AudioPreprocessor cannot be changed. It must be the same as
     * used in TT_InitLocalPlayback().
     *
     * @see TT_InitLocalPlayback()
     * @see TT_StopLocalPlayback()
     * @see TT_UpdateStreamingMediaFileToChannel() */
    TEAMTALKDLL_API TTBOOL TT_UpdateLocalPlayback(IN TTInstance* lpTTInstance,
                                                  IN INT32 nPlaybackSessionID,
                                                  IN const MediaFilePlayback* lpMediaFilePlayback);

    /**
     * Stop media file currently being played by TeamTalk instance.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nPlaybackSessionID Session ID created by TT_InitLocalPlayback().
     *
     * @see TT_InitLocalPlayback()
     * @see TT_UpdateLocalPlayback() */
    TEAMTALKDLL_API TTBOOL TT_StopLocalPlayback(IN TTInstance* lpTTInstance,
                                                IN INT32 nPlaybackSessionID);
    
    /**
     * @brief Get the properties of a media file.
     *
     * Use this function to determine the audio and video properties of
     * a media file, so the user knows what can be streamed.
     *
     * @see TT_StartStreamingMediaFileToChannel() */
    TEAMTALKDLL_API TTBOOL TT_GetMediaFileInfo(IN const TTCHAR* szMediaFilePath,
                                               OUT MediaFileInfo* lpMediaFileInfo);

    /** @brief Extract a user's media video frame for display.
     *
     * The #VideoFrame extracted from the client instance will contain a
     * pointer to the image's frame buffer, so a RGB32 bitmap can be
     * displayed in a window control.
     *
     * To release the acquired #VideoFrame call
     * TT_ReleaseUserMediaVideoFrame().
     *
     * A media video frame comes from a user's
     * #STREAMTYPE_MEDIAFILE_VIDEO.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param nUserID The user's ID. 0 for local user.
     * @return Returns video frame which will contain the image data. Note 
     * that it's the @a frameBuffer member of #VideoFrame which will contain 
     * the image data allocated internally by TeamTalk. Returns NULL if no 
     * video frame could be acquired.
     * @see TT_ReleaseUserMediaVideoFrame() */
    TEAMTALKDLL_API VideoFrame* TT_AcquireUserMediaVideoFrame(IN TTInstance* lpTTInstance,
                                                              IN INT32 nUserID);

    /** @brief Delete a user's video frame, acquired through
     * TT_AcquireUserMediaVideoFrame(), so its allocated resources can
     * be released.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param lpVideoFrame Pointer to #VideoFrame which should be deallocated. 
     * @return Returns TRUE if a video frame was successfully deallocated.
     * @see TT_AcquireUserMediaVideoFrame() */
    TEAMTALKDLL_API TTBOOL TT_ReleaseUserMediaVideoFrame(IN TTInstance* lpTTInstance,
                                                         IN VideoFrame* lpVideoFrame);
    /** @} */

    /** @addtogroup desktopshare
     * @{ */

    /**
     * @brief Transmit a desktop window (bitmap) to users in the same
     * channel.
     *
     * When TT_SendDesktopWindow() is called the first time a new
     * desktop session will be started. To update the current desktop
     * session call TT_SendDesktopWindow() again once the previous
     * desktop transmission has finished. Tracking progress of the
     * current desktop transmission is done by checking for the
     * #CLIENTEVENT_DESKTOPWINDOW_TRANSFER event. While the desktop
     * transmission is active the flag #CLIENT_TX_DESKTOP will be set
     * on the local client instance.
     *
     * If the desktop window (bitmap) changes size (width/height) or
     * format a new desktop session will be started. Also if the user
     * changes channel a new desktop session will be started. Check @c
     * nSessionID of #DesktopWindow to see if a new desktop session is
     * started or the #CLIENTEVENT_USER_DESKTOPWINDOW event.
     *
     * Remote users will get the #CLIENTEVENT_USER_DESKTOPWINDOW event
     * and can call TT_AcquireUserDesktopWindow() to retrieve the desktop
     * window.
     * 
     * User rights required:
     * - #USERRIGHT_TRANSMIT_DESKTOP
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param lpDesktopWindow Properties of the bitmap. Set the @c nSessionID 
     * property to 0.
     * @param nConvertBmpFormat Before transmission convert the bitmap to this 
     * format.
     * @return TRUE if desktop window is queued for transmission. FALSE if 
     * @c nBitmapSize is invalid or if a desktop transmission is already 
     * active.
     * @return -1 on error. 0 if bitmap has no changes. Greater than 0 on 
     * success.
     * @see TT_CloseDesktopWindow()
     * @see TT_SendDesktopCursorPosition() */
    TEAMTALKDLL_API INT32 TT_SendDesktopWindow(IN TTInstance* lpTTInstance,
                                               IN const DesktopWindow* lpDesktopWindow,
                                               IN BitmapFormat nConvertBmpFormat);

    /**
     * @brief Close the current desktop session.
     *
     * Closing the desktop session will cause the users receiving the
     * current desktop session to see the desktop session ID change to
     * 0 in the #CLIENTEVENT_USER_DESKTOPWINDOW event.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. */
    TEAMTALKDLL_API TTBOOL TT_CloseDesktopWindow(IN TTInstance* lpTTInstance);

    /**
     * @brief Get RGB values of the palette for the bitmap format.
     *
     * This currently only applies to bitmaps of format #BMP_RGB8_PALETTE.
     *
     * Note that the pointer returned is non-const which means the
     * palette can be overwritten with a custom palette. The custom
     * palette will then be used internally during bitmap
     * conversion.
     *
     * @param nBmpPalette The bitmap format. Currently only #BMP_RGB8_PALETTE
     * is supported.
     * @param nIndex The index in the color table of the RGB values to 
     * extract.
     * @return Pointer to RGB colors. First byte is Red, second Blue and 
     * third Green. Returns NULL if the color-index is invalid. */
    TEAMTALKDLL_API unsigned char* TT_Palette_GetColorTable(IN BitmapFormat nBmpPalette,
                                                            IN INT32 nIndex);
#if defined(WIN32)

    /** @brief Get the handle (HWND) of the window which is currently
     * active (focused) on the Windows desktop. */
    TEAMTALKDLL_API HWND TT_Windows_GetDesktopActiveHWND(void);

    /** @brief Get the handle (HWND) of the Windows desktop (full desktop). */
    TEAMTALKDLL_API HWND TT_Windows_GetDesktopHWND(void);

    /** @brief Enumerate all the handles (@c HWND) of visible
     * windows. Increment @c nIndex until the function returns
     * FALSE. Use TT_Windows_GetWindow() to get information about each
     * window. */
    TEAMTALKDLL_API TTBOOL TT_Windows_GetDesktopWindowHWND(IN INT32 nIndex,
                                                           OUT HWND* lpHWnd);

    /**
     * @brief A struct which describes the properties of a window
     * which can be shared.
     * @see TT_Windows_GetDesktopWindowHWND()
     * @see TT_Windows_GetWindow() */
    typedef struct ShareWindow
    {
        /** @brief The Windows handle of the window. */
        HWND hWnd;
        /** @brief X coordinate of the window relative to the Windows desktop. */
        INT32 nWndX;
        /** @brief Y coordinate of the window relative to the Windows desktop. */
        INT32 nWndY;
        /** @brief The width in pixels of the window. */
        INT32 nWidth;
        /** @brief The height in pixels of the window. */
        INT32 nHeight;
        /** @brief The title of the window. */
        TTCHAR szWindowTitle[TT_STRLEN];
    } ShareWindow;

    /**
     * @brief Get the properties of a window from its window handle (HWND). */
    TEAMTALKDLL_API TTBOOL TT_Windows_GetWindow(IN HWND hWnd,
                                                OUT ShareWindow* lpShareWindow);

    /**
     * @brief Transmit the specified window in a desktop session.
     *
     * Same as TT_SendDesktopWindow() except the properties for the
     * #DesktopWindow are extracted automatically.
     * 
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param hWnd Windows handle for the window to transmit.
     * @param nBitmapFormat Bitmap format to use for the transmitted image.
     * @param nDesktopProtocol The protocol to use for transmitting the image.
     * @return See TT_SendDesktopWindow(). */
    TEAMTALKDLL_API INT32 TT_SendDesktopWindowFromHWND(IN TTInstance* lpTTInstance,
                                                       IN HWND hWnd, 
                                                       IN BitmapFormat nBitmapFormat,
                                                       IN DesktopProtocol nDesktopProtocol);
    
    /**
     * @brief Paint user's desktop window using a Windows' DC (device
     * context).
     *
     * Same as calling TT_PaintDesktopWindowEx() like this:
     *
       @verbatim
       TT_PaintDesktopWindowEx(lpTTInstance, nUserID, hDC, 
                               XDest, YDest, nDestWidth,
                               nDestHeight, 0, 0, 
                               'src_bmp_width', 'src_bmp_height');
       @endverbatim
     *
     * @c src_bmp_width and @c src_bmp_height are extracted internally
     * from the source image. */
    TEAMTALKDLL_API TTBOOL TT_PaintDesktopWindow(IN TTInstance* lpTTInstance,
                                                 IN INT32 nUserID,
                                                 IN HDC hDC,
                                                 IN INT32 XDest,
                                                 IN INT32 YDest,
                                                 IN INT32 nDestWidth,
                                                 IN INT32 nDestHeight);

    /**
     * @brief Paint user's desktop window using a Windows' DC (device
     * context).
     *
     * An application can either paint a bitmap by using
     * TT_AcquireUserDesktopWindow() which provides a pointer to a bitmap
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
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
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
     * @see TT_AcquireUserDesktopWindow() */
    TEAMTALKDLL_API TTBOOL TT_PaintDesktopWindowEx(IN TTInstance* lpTTInstance,
                                                   IN INT32 nUserID,
                                                   IN HDC hDC,
                                                   IN INT32 XDest,
                                                   IN INT32 YDest,
                                                   IN INT32 nDestWidth,
                                                   IN INT32 nDestHeight,
                                                   IN INT32 XSrc,
                                                   IN INT32 YSrc,
                                                   IN INT32 nSrcWidth,
                                                   IN INT32 nSrcHeight);
#endif

#if defined(__APPLE__)

    /**
     * @brief A struct which describes the properties of a window
     * which can be shared.
     * @see TT_MacOS_GetWindow()
     * @see TT_MacOS_GetWindowFromWindowID() */
    typedef struct ShareWindow
    {
        /** @brief The CGWindowID */
        INT64 nWindowID;
        /** @brief X coordinate of window. */
        INT32 nWindowX;
        /** @brief Y coordinate of window. */
        INT32 nWindowY;
        /** @brief The width of the window. */
        INT32 nWidth;
        /** @brief The height of the window. */
        INT32 nHeight;
        /** @brief The title of the window. */
        TTCHAR szWindowTitle[TT_STRLEN];
        /** @brief The PID of the owning process. */
        INT64 nPID;
    } ShareWindow;

    /** @brief Enumerate all windows on the desktop. Increment @c
     * nIndex until the function returns FALSE. Use
     * TT_MacOS_GetWindowFromWindowID() to get information about the
     * window, e.g. title, dimensions, etc. */
    TEAMTALKDLL_API TTBOOL TT_MacOS_GetWindow(IN INT32 nIndex,
                                              OUT ShareWindow* lpShareWindow);

    /** @brief Get information about a window by passing its handle
     * (@c CGWindowID). @see TT_MacOS_GetWindow() */
    TEAMTALKDLL_API TTBOOL TT_MacOS_GetWindowFromWindowID(IN INT64 nWindowID,
                                                          OUT ShareWindow* lpShareWindow);

    /**
     * @brief Transmit the specified window in a desktop session.
     *
     * Same as TT_SendDesktopWindow() except the properties for the
     * #DesktopWindow are extracted automatically.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nWindowID The handle of the window which should be converted to
     * bitmap and sent to the server.
     * @param nBitmapFormat Bitmap format to use for the transmitted image.
     * @param nDesktopProtocol The protocol to use for transmitting the image.
     * @return See TT_SendDesktopWindow(). */
    TEAMTALKDLL_API INT32 TT_SendDesktopFromWindowID(IN TTInstance* lpTTInstance,
                                                     IN INT64 nWindowID, 
                                                     IN BitmapFormat nBitmapFormat,
                                                     IN DesktopProtocol nDesktopProtocol);
#endif

    /**
     * @brief Send the position of mouse cursor to users in the same
     * channel.
     *
     * It's only possible to send the mouse cursor position if there's
     * a desktop session which is currently active.
     * 
     * User rights required:
     * - #USERRIGHT_TRANSMIT_DESKTOP
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param nPosX X coordinate of mouse cursor.
     * @param nPosY Y coordinate of mouse cursor.
     * @see TT_SendDesktopWindow() */
    TEAMTALKDLL_API TTBOOL TT_SendDesktopCursorPosition(IN TTInstance* lpTTInstance,
                                                        IN UINT16 nPosX,
                                                        IN UINT16 nPosY);
    /** 
     * @brief Send a mouse or keyboard event to a shared desktop
     * window.
     *
     * If a user is sharing a desktop window it's possible for a
     * remote user to take control of mouse and keyboard input on the
     * remote computer. Read section @ref txdesktopinput on how to
     * transmit desktop input to a shared window.
     *
     * When the remote user receives the issued #DesktopInput the
     * event #CLIENTEVENT_USER_DESKTOPINPUT is posted to the client
     * instance sharing the desktop window.
     *
     * User rights required:
     * - #USERRIGHT_TRANSMIT_DESKTOPINPUT
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param nUserID The user's ID who owns the shared desktop window
     * and should receive desktop input.
     * @param lpDesktopInputs An array of #DesktopInput structs which
     * should be transmitted to the user. Internally in the client
     * instance each user ID has an internal queue which can contain a
     * maximum of 100 #DesktopInput structs.
     * @param nDesktopInputCount Must be less or equal to #TT_DESKTOPINPUT_MAX.
     * @return FALSE If user doesn't exist or if desktop input queue is full or
     * if @c nUserID doesn't subscribe to desktop input. */
    TEAMTALKDLL_API TTBOOL TT_SendDesktopInput(IN TTInstance* lpTTInstance,
                                               IN INT32 nUserID,
                                               IN const DesktopInput lpDesktopInputs[TT_DESKTOPINPUT_MAX],
                                               IN INT32 nDesktopInputCount);

    /**
     * @brief Acquire a user's desktop window (bitmap image).
     *
     * A user's desktop window can be extracted when the 
     * #CLIENTEVENT_USER_DESKTOPWINDOW is received.
     *
     * When the #DesktopWindow is no longer needed call
     * TT_ReleaseUserDesktopWindow() to release the memory allocated by
     * the client instance.
     *
     * A desktop window is simply a bitmap image. This method is used for
     * retrieving the user's bitmap image.
     *
     * For #BMP_RGB8_PALETTE bitmaps check out TT_Palette_GetColorTable().
     *
     * To convert bitmap to a different format use
     * TT_AcquireUserDesktopWindowEx().
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param nUserID The user's ID.
     * @return NULL If there's no active desktop window for this user.
     * @see TT_SendDesktopWindow() */
    TEAMTALKDLL_API DesktopWindow* TT_AcquireUserDesktopWindow(IN TTInstance* lpTTInstance, 
                                                               IN INT32 nUserID);

    /**
     * @brief Same as TT_AcquireUserDesktopWindow() except an extra
     * option for converting bitmap to a different format.
     *
     * It is highly adviced to use TT_AcquireUserDesktopWindow() since
     * converting to a different bitmap format is very inefficient.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param nUserID The user's ID.
     * @param nBitmapFormat The bitmap format to convert to.
     * @return NULL If there's no active desktop window for this user. */
    TEAMTALKDLL_API DesktopWindow* TT_AcquireUserDesktopWindowEx(IN TTInstance* lpTTInstance, 
                                                                 IN INT32 nUserID,
                                                                 IN BitmapFormat nBitmapFormat);

    /** @brief Release memory allocated by the #DesktopWindow.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param lpDesktopWindow Pointer to #DesktopWindow to release.
     * @see TT_AcquireUserDesktopWindow() */
    TEAMTALKDLL_API TTBOOL TT_ReleaseUserDesktopWindow(IN TTInstance* lpTTInstance, 
                                                       IN DesktopWindow* lpDesktopWindow);
    /** @} */

    /** @addtogroup connectivity
     * @{ */

    /**
     * @brief Connect to a server. 
     * 
     * This is a non-blocking call (but may block due to DNS lookup)
     * so the user application must wait for the event
     * #CLIENTEVENT_CON_SUCCESS to be posted once the connection has
     * been established or #CLIENTEVENT_CON_FAILED if connection could
     * not be established. If the connection could not be establish
     * ensure to call #TT_Disconnect to close open connections in the
     * client instance before trying again.
     *
     * Once connected call #TT_DoLogin to log on to the server.
     * 
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
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
     * @see CLIENTEVENT_CON_SUCCESS
     * @see CLIENTEVENT_CON_FAILED
     * @see TT_DoLogin */
    TEAMTALKDLL_API TTBOOL TT_Connect(IN TTInstance* lpTTInstance,
                                      IN const TTCHAR* szHostAddress, 
                                      IN INT32 nTcpPort, 
                                      IN INT32 nUdpPort, 
                                      IN INT32 nLocalTcpPort, 
                                      IN INT32 nLocalUdpPort,
                                      IN TTBOOL bEncrypted);

    /**
     * @brief Same as TT_Connect() but the option of providing a
     * unique system-ID.
     *
     * The system-ID is set in the TeamTalk server API using
     * TTS_StartServerSysID(). If a client tries to connect with a
     * different system-ID that client will receive the error
     * #CMDERR_INCOMPATIBLE_PROTOCOLS when trying to log in.
     *
     * Requires TeamTalk version 5.1.3.4506.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
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
     * The default value is "teamtalk". See TTS_StartServerSysID()
     * @return Returns TRUE if connection process was initiated. */
    TEAMTALKDLL_API TTBOOL TT_ConnectSysID(IN TTInstance* lpTTInstance,
                                           IN const TTCHAR* szHostAddress, 
                                           IN INT32 nTcpPort, 
                                           IN INT32 nUdpPort, 
                                           IN INT32 nLocalTcpPort, 
                                           IN INT32 nLocalUdpPort,
                                           IN TTBOOL bEncrypted,
                                           IN const TTCHAR* szSystemID);

    /**
     * @brief Bind to specific IP-address priot to connecting to server.
     *
     * Same as TT_Connect() except that this also allows which IP-address
     * to bind to on the local interface.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
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
     * @see TT_Connect */
    TEAMTALKDLL_API TTBOOL TT_ConnectEx(IN TTInstance* lpTTInstance,
                                        IN const TTCHAR* szHostAddress,
                                        IN INT32 nTcpPort,
                                        IN INT32 nUdpPort,
                                        IN const TTCHAR* szBindIPAddr,
                                        IN INT32 nLocalTcpPort,
                                        IN INT32 nLocalUdpPort,
                                        IN TTBOOL bEncrypted);

    /**
     * @brief Disconnect from the server.
     * 
     * This will clear the flag #CLIENT_CONNECTED and #CLIENT_CONNECTING.
     *
     * Use #TT_CloseTeamTalk to release all resources allocated by the
     * client instance.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. */
    TEAMTALKDLL_API TTBOOL TT_Disconnect(IN TTInstance* lpTTInstance);

    /**
     * @brief Query the maximum size of UDP data packets to the user
     * or server.
     *
     * The #CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED event is posted when
     * the query has finished.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk. 
     * @param nUserID The ID of the user to query or 0 for querying 
     * server. Currently only @c nUserID = 0 is supported. */
    TEAMTALKDLL_API TTBOOL TT_QueryMaxPayload(IN TTInstance* lpTTInstance,
                                              IN INT32 nUserID);
    
    /**
     * @brief Retrieve client statistics of bandwidth usage and
     * response times.
     *
     * @see ClientStatistics */
     TEAMTALKDLL_API TTBOOL TT_GetClientStatistics(IN TTInstance* lpTTInstance,
                                                   OUT ClientStatistics* lpClientStatistics);

    /**
     * @brief Update the client instance's default keep alive settings.
     *
     * It is generally discouraged to change the client instance's
     * keep alive settings unless the network has special
     * requirements.
     *
     * After calling TT_SetClientKeepAlive() it is recommended doing a
     * TT_DoPing() since all TCP and UDP keep alive timers will be
     * restarted.
     *
     * @see TT_DoPing()
     * @see TT_GetClientKeepAlive()
     * @see TT_GetClientStatistics() */
    TEAMTALKDLL_API TTBOOL TT_SetClientKeepAlive(IN TTInstance* lpTTInstance,
                                                 IN const ClientKeepAlive* lpClientKeepAlive);

    /**
     * @brief Get the client instance's current keep alive settings.
     *
     * @see TT_DoPing()
     * @see TT_SetClientKeepAlive()
     * @see TT_GetClientStatistics() */
    TEAMTALKDLL_API TTBOOL TT_GetClientKeepAlive(IN TTInstance* lpTTInstance,
                                                 OUT ClientKeepAlive* lpClientKeepAlive);
    
    /** @} */

    /** @addtogroup commands
     * @{ */

    /** @brief Ping server and wait for server to reply.
     *
     * Use this command to check if the server is responding.
     *
     * If the client instance doesn't ping the server within the timeout
     * specified by @c nUserTimeout in #ServerProperties, then the server
     * will disconnect the client.
     *
     * The client instance automatically pings the server at half the time
     * specified by @c nUserTimeout in #ServerProperties.
     *
     * #CLIENTEVENT_CMD_PROCESSING is posted if server replies.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error. */
    TEAMTALKDLL_API INT32 TT_DoPing(IN TTInstance* lpTTInstance);

    /** @brief Same as TT_DologinEx() but without the option to
     * specify @c szClientName. Kept for backwards compatibility.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param szNickname The nickname to use.
     * @param szUsername The username of the #UserAccount set up on the 
     * server.
     * @param szPassword The password of the user account on the server. Leave 
     * blank if no account is needed on the server.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error. */
    TEAMTALKDLL_API INT32 TT_DoLogin(IN TTInstance* lpTTInstance,
                                     IN const TTCHAR* szNickname, 
                                     IN const TTCHAR* szUsername,
                                     IN const TTCHAR* szPassword);


    /**
     * @brief Logon to a server.
     * 
     * Once connected to a server call this function to logon. If
     * the login is successful #CLIENTEVENT_CMD_MYSELF_LOGGEDIN is
     * posted, otherwise #CLIENTEVENT_CMD_ERROR. Once logged on it's
     * not possible to talk to other users until the client instance
     * joins a channel. Call #TT_DoJoinChannel to join a channel.
     *
     * Possible errors:
     * - #CMDERR_INCORRECT_CHANNEL_PASSWORD
     * - #CMDERR_INVALID_ACCOUNT
     * - #CMDERR_MAX_SERVER_USERS_EXCEEDED
     * - #CMDERR_SERVER_BANNED
     * - #CMDERR_ALREADY_LOGGEDIN
     *
     * Requires TeamTalk version 5.1.3.4506.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param szNickname The nickname to use.
     * @param szUsername The username of the #UserAccount set up on the 
     * server.
     * @param szPassword The password of the user account on the server. Leave 
     * blank if no account is needed on the server.
     * @param szClientName The name of the client application used. This is an 
     * optional value and can be kept blank.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see ServerProperties
     * @see TT_DoJoinChannel
     * @see CLIENTEVENT_CMD_MYSELF_LOGGEDIN
     * @see CLIENTEVENT_CMD_ERROR */
    TEAMTALKDLL_API INT32 TT_DoLoginEx(IN TTInstance* lpTTInstance,
                                       IN const TTCHAR* szNickname, 
                                       IN const TTCHAR* szUsername,
                                       IN const TTCHAR* szPassword,
                                       IN const TTCHAR* szClientName);

    /**
     * @brief Logout of the server.
     *
     * If successful the event #CLIENTEVENT_CMD_MYSELF_LOGGEDOUT
     * will be posted.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see CLIENTEVENT_CMD_MYSELF_LOGGEDOUT */
    TEAMTALKDLL_API INT32 TT_DoLogout(IN TTInstance* lpTTInstance);

    /**
     * @brief Create a new channel and join it.
     *
     * This function can also be used to join an existing channel and
     * in this case the parameters @a szTopic and @a szOpPassword are
     * ignored.
     *
     * When #TT_DoJoinChannel is used to create channels it works
     * similar to IRC. If the client instance tries to join a channel
     * which does not exist it will be created as a new channel. If
     * the client instance is the last user to leave a channel the
     * channel will be removed on the server.
     *
     * If the channel is created successfully the event
     * #CLIENTEVENT_CMD_CHANNEL_NEW will be sent, followed by
     * #CLIENTEVENT_CMD_USER_JOINED.
     *
     * User rights required:
     * - #USERRIGHT_CREATE_TEMPORARY_CHANNEL
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_INCORRECT_CHANNEL_PASSWORD
     * - #CMDERR_MAX_CHANNEL_USERS_EXCEEDED
     * - #CMDERR_ALREADY_IN_CHANNEL
     * - #CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param lpChannel The channel to join or create if it doesn't already
     * exist.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoLeaveChannel
     * @see TT_DoMakeChannel
     * @see CLIENTEVENT_CMD_CHANNEL_NEW
     * @see CLIENTEVENT_CMD_MYSELF_JOINED */
    TEAMTALKDLL_API INT32 TT_DoJoinChannel(IN TTInstance* lpTTInstance,
                                           IN const Channel* lpChannel);

    /**
     * @brief Join an existing channel.
     * 
     * This command basically calls #TT_DoJoinChannel but omits the
     * unnecessary parameters for only joining a channel and not
     * creating a new one.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_INCORRECT_CHANNEL_PASSWORD
     * - #CMDERR_MAX_CHANNEL_USERS_EXCEEDED
     * - #CMDERR_ALREADY_IN_CHANNEL
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nChannelID The ID of the channel to join.
     * @param szPassword The password for the channel to join.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoLeaveChannel
     * @see TT_DoMakeChannel
     * @see CLIENTEVENT_CMD_CHANNEL_NEW
     * @see CLIENTEVENT_CMD_MYSELF_JOINED */
    TEAMTALKDLL_API INT32 TT_DoJoinChannelByID(IN TTInstance* lpTTInstance,
                                               IN INT32 nChannelID, 
                                               IN const TTCHAR* szPassword);

    /**
     * @brief Leave the current channel.
     *
     * Note that #TT_DoLeaveChannel() doesn't take any parameters
     * since a user can only participate in one channel at the time.
     * If command is successful the event #CLIENTEVENT_CMD_USER_LEFT
     * will be posted.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_NOT_IN_CHANNEL
     * - #CMDERR_CHANNEL_NOT_FOUND
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoJoinChannel
     * @see CLIENTEVENT_CMD_MYSELF_LEFT */
    TEAMTALKDLL_API INT32 TT_DoLeaveChannel(IN TTInstance* lpTTInstance);

    /**
     * @brief Change the client instance's nick name.
     *
     * The event #CLIENTEVENT_CMD_USER_UPDATE will be posted if the
     * update was successful.
     *
     * Command will be rejected if #USERRIGHT_LOCKED_NICKNAME is set.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param szNewNick is the new nick name to use.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see CLIENTEVENT_CMD_USER_UPDATE */
    TEAMTALKDLL_API INT32 TT_DoChangeNickname(IN TTInstance* lpTTInstance, 
                                              IN const TTCHAR* szNewNick);

    /**
     * @brief Change the client instance's currect status
     *
     * The event #CLIENTEVENT_CMD_USER_UPDATE will be posted if the update
     * was successful.
     *
     * Command will be rejected if #USERRIGHT_LOCKED_STATUS is set.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nStatusMode The value for the status mode.
     * @param szStatusMessage The user's message associated with the status 
     * mode.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see CLIENTEVENT_CMD_USER_UPDATE */
    TEAMTALKDLL_API INT32 TT_DoChangeStatus(IN TTInstance* lpTTInstance,
                                            IN INT32 nStatusMode, 
                                            IN const TTCHAR* szStatusMessage);

    /**
     * @brief Send a text message to either a user or a channel. 
     *
     * Can also be a broadcast message which is received by all users
     * on the server. This, however, requires
     * #USERRIGHT_TEXTMESSAGE_BROADCAST.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED 
     * - #CMDERR_CHANNEL_NOT_FOUND
     * - #CMDERR_USER_NOT_FOUND
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param lpTextMessage A preallocated text-message struct.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see USERRIGHT_TEXTMESSAGE_BROADCAST */
    TEAMTALKDLL_API INT32 TT_DoTextMessage(IN TTInstance* lpTTInstance,
                                           IN const TextMessage* lpTextMessage);

    /**
     * @brief Make another user operator of a channel. 
     * 
     * User rights required:
     * - #USERRIGHT_OPERATOR_ENABLE
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_CHANNEL_NOT_FOUND
     * - #CMDERR_USER_NOT_FOUND
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID The user who should become channel operator. 
     * @param nChannelID The channel where the user should become operator.
     * @param bMakeOperator Whether user should be op'ed or deop'ed.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoChannelOpEx */
    TEAMTALKDLL_API INT32 TT_DoChannelOp(IN TTInstance* lpTTInstance,
                                         IN INT32 nUserID,
                                         IN INT32 nChannelID,
                                         IN TTBOOL bMakeOperator);

    /**
     * @brief Make another user operator of a channel using the 
     * @a szOpPassword of #Channel.
     * 
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_CHANNEL_NOT_FOUND
     * - #CMDERR_USER_NOT_FOUND
     * - #CMDERR_INCORRECT_OP_PASSWORD
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID The user who should become channel operator. 
     * @param nChannelID The channel where the user should become operator.
     * @param szOpPassword The @a szOpPassword of #Channel.
     * @param bMakeOperator Whether user should be op'ed or deop'ed.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoChannelOp */
    TEAMTALKDLL_API INT32 TT_DoChannelOpEx(IN TTInstance* lpTTInstance,
                                           IN INT32 nUserID,
                                           IN INT32 nChannelID,
                                           IN const TTCHAR* szOpPassword,
                                           IN TTBOOL bMakeOperator);

    /**
     * @brief Kick user from either channel or server. 
     *
     * To ban a user call #TT_DoBanUser before #TT_DoKickUser.
     *
     * User rights required:
     * - #USERRIGHT_KICK_USERS
     * - Alternative channel-operator (see TT_DoChannelOp()).
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_CHANNEL_NOT_FOUND
     * - #CMDERR_USER_NOT_FOUND
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID The ID of the user to kick. 
     * @param nChannelID The channel where the user shoul be kicked from
     * or specify 0 to kick the user off the server.
     *
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoBanUser */
    TEAMTALKDLL_API INT32 TT_DoKickUser(IN TTInstance* lpTTInstance,
                                        IN INT32 nUserID,
                                        IN INT32 nChannelID);

    /**
     * @brief Send a file to the specified channel. 
     *
     * If user is logged on as an admin the file can be located in any
     * channel. If the user is not an admin the file must be located
     * in the same channel as the user is currently participating in.
     * The file being uploaded must have a file size which is less
     * than the disk quota of the channel, minus the sum of all the
     * files in the channel. The disk quota of a channel can be
     * obtained in the @c nDiskQuota of the #Channel struct passed to
     * #TT_GetChannel.
     *
     * User rights required:
     * - #USERRIGHT_UPLOAD_FILES
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_MAX_DISKUSAGE_EXCEEDED
     * - #CMDERR_CHANNEL_NOT_FOUND
     * - #CMDERR_FILETRANSFER_NOT_FOUND
     * - #CMDERR_OPENFILE_FAILED
     * - #CMDERR_FILE_NOT_FOUND
     * - #CMDERR_FILE_ALREADY_EXISTS
     * - #CMDERR_FILESHARING_DISABLED
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nChannelID The ID of the channel of where to put the file. Only 
     * admins can upload in channel other then their own.
     * @param szLocalFilePath The path of the file to upload, e.g. C:\\myfile.txt.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see Channel
     * @see TT_GetChannel */
    TEAMTALKDLL_API INT32 TT_DoSendFile(IN TTInstance* lpTTInstance,
                                        IN INT32 nChannelID,
                                        IN const TTCHAR* szLocalFilePath);

    /**
     * @brief Download a file from the specified channel. 
     *
     * If user is logged on as an admin the file can be located in any
     * channel. If the user is not an admin the file must be located
     * in the same channel as the user is currently participating in.
     *
     * User rights required:
     * - #USERRIGHT_DOWNLOAD_FILES
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_CHANNEL_NOT_FOUND
     * - #CMDERR_FILETRANSFER_NOT_FOUND
     * - #CMDERR_OPENFILE_FAILED
     * - #CMDERR_FILE_NOT_FOUND
     * - #CMDERR_FILESHARING_DISABLED
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nChannelID The ID of the channel of where to get the file. Only 
     * admins can download in channel other then their own.
     * @param nFileID The ID of the file which is passed by #CLIENTEVENT_CMD_FILE_NEW.
     * @param szLocalFilePath The path of where to store the file, e.g. 
     * C:\\myfile.txt.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see CLIENTEVENT_CMD_FILE_NEW
     * @see TT_GetChannelFiles */
    TEAMTALKDLL_API INT32 TT_DoRecvFile(IN TTInstance* lpTTInstance,
                                        IN INT32 nChannelID,
                                        IN INT32 nFileID, 
                                        IN const TTCHAR* szLocalFilePath);

    /**
     * @brief Delete a file from a channel. 
     *
     * A user is allowed to delete a file from a channel if either the
     * user is an admin, operator of the channel or owner of the
     * file. To be owner of the file a user must have a #UserAccount
     * on the server.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED 
     * - #CMDERR_CHANNEL_NOT_FOUND
     * - #CMDERR_FILE_NOT_FOUND
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nChannelID The ID of the channel where the file is located.
     * @param nFileID The ID of the file to delete. The ID of the file which 
     * is passed by #CLIENTEVENT_CMD_FILE_NEW.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see CLIENTEVENT_CMD_FILE_NEW
     * @see TT_GetChannelFiles */
    TEAMTALKDLL_API INT32 TT_DoDeleteFile(IN TTInstance* lpTTInstance,
                                          IN INT32 nChannelID,
                                          IN INT32 nFileID);

    /**
     * @brief Subscribe to user events and/or data.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED 
     * - #CMDERR_USER_NOT_FOUND
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID The ID of the user this should affect.
     * @param uSubscriptions Union of #Subscription to subscribe to.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see Subscription */
    TEAMTALKDLL_API INT32 TT_DoSubscribe(IN TTInstance* lpTTInstance,
                                         IN INT32 nUserID, 
                                         IN Subscriptions uSubscriptions);

    /**
     * @brief Unsubscribe to user events/data. This can be used to ignore messages
     * or voice data from a specific user.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED 
     * - #CMDERR_USER_NOT_FOUND
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID The ID of the user this should affect.
     * @param uSubscriptions Union of #Subscription to unsubscribe.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see Subscription */
    TEAMTALKDLL_API INT32 TT_DoUnsubscribe(IN TTInstance* lpTTInstance,
                                           IN INT32 nUserID, 
                                           IN Subscriptions uSubscriptions);

    /**
     * @brief Make a new channel on the server.
     * 
     * This command only applies to users with #USERRIGHT_MODIFY_CHANNELS.
     *
     * User rights required:
     * - #USERRIGHT_MODIFY_CHANNELS
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_CHANNEL_ALREADY_EXISTS
     * - #CMDERR_CHANNEL_NOT_FOUND If channel's combined path is longer than
     *   #TT_STRLEN.
     * - #CMDERR_INCORRECT_CHANNEL_PASSWORD If the password is longer than
     *   #TT_STRLEN.
     * - #CMDERR_UNKNOWN_AUDIOCODEC If the server doesn't support the audio
     *   codec. Introduced in version 4.1.0.1264.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param lpChannel A Channel-structure containing information about
     * the channel being created. The Channel's member @a nChannelID is ignored.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoJoinChannel */
    TEAMTALKDLL_API INT32 TT_DoMakeChannel(IN TTInstance* lpTTInstance,
                                           IN const Channel* lpChannel);

    /**
     * @brief Update a channel's properties.
     *
     * Users with #USERRIGHT_MODIFY_CHANNELS can update all properties of
     * a channel.
     *
     * A user with channel-operator status (see TT_DoChannelOp()) can 
     * also update a channel's properties, but is not able to change the 
     * following properties:
     * - @c audiocodec
     * - @c nDiskQuota
     * - @c nMaxUsers
     * - @c uChannelType
     *   - Cannot modify #CHANNEL_PERMANENT
     *
     * Note that a channel's #AudioCodec cannot be changed
     * if there's currently users in the channel.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED 
     * - #CMDERR_CHANNEL_NOT_FOUND
     * - #CMDERR_CHANNEL_HAS_USERS
     * - #CMDERR_CHANNEL_ALREADY_EXISTS
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param lpChannel A Channel-structure containing information about
     * the channel being modified. The channel member's @c nParentID
     * and @c szName are ignored.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoMakeChannel */
    TEAMTALKDLL_API INT32 TT_DoUpdateChannel(IN TTInstance* lpTTInstance,
                                             IN const Channel* lpChannel);

    /**
     * @brief Remove a channel from a server. 
     *
     * This command only applies to users with
     * #USERRIGHT_MODIFY_CHANNELS.
     *
     * If there's any users in the channel they will be kicked and
     * subchannels will be deleted as well.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_CHANNEL_NOT_FOUND
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.  
     * @param nChannelID The ID of the channel to remove.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoMakeChannel */
    TEAMTALKDLL_API INT32 TT_DoRemoveChannel(IN TTInstance* lpTTInstance,
                                             IN INT32 nChannelID);

    /**
     * @brief Issue command to move a user from one channel to
     * another.
     *
     * User rights required:
     * - #USERRIGHT_MOVE_USERS
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_CHANNEL_NOT_FOUND
     * - #CMDERR_USER_NOT_FOUND
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID User to be moved.
     * @param nChannelID Channel where user should be put into.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoMoveUserByID */
    TEAMTALKDLL_API INT32 TT_DoMoveUser(IN TTInstance* lpTTInstance,
                                        IN INT32 nUserID, 
                                        IN INT32 nChannelID);

    /**
     * @brief Update server properties.
     *
     * User rights required:
     * - #USERRIGHT_UPDATE_SERVERPROPERTIES
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param lpServerProperties A structure holding the information to be set 
     * on the server.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_GetServerProperties */
    TEAMTALKDLL_API INT32 TT_DoUpdateServer(IN TTInstance* lpTTInstance,
                                            IN const ServerProperties* lpServerProperties);

    /**
     * @brief Issue command to list user accounts on the server.
     *
     * The event #CLIENTEVENT_CMD_USERACCOUNT will be posted for every
     * #UserAccount on the server. Ensure not to list too many many user
     * accounts since this may suspend event handling.
     *
     * User accounts can be used to create users with different user
     * rights.
     *
     * Only #USERTYPE_ADMIN can issue this command.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nIndex Index of first user to display.
     * @param nCount The number of users to retrieve.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see UserAccount
     * @see UserType  */
    TEAMTALKDLL_API INT32 TT_DoListUserAccounts(IN TTInstance* lpTTInstance,
                                                IN INT32 nIndex,
                                                IN INT32 nCount);

    /**
     * @brief Issue command to create a new user account on the
     * server.
     *
     * Check out section @ref useradmin to see how the server handles
     * users.
     *
     * Only #USERTYPE_ADMIN can issue this command.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_INVALID_USERNAME
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param lpUserAccount The properties of the user account to create.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoListUserAccounts
     * @see TT_DoDeleteUserAccount
     * @see UserAccount
     * @see UserType */
    TEAMTALKDLL_API INT32 TT_DoNewUserAccount(IN TTInstance* lpTTInstance,
                                              IN const UserAccount* lpUserAccount);

    /**
     * @brief Issue command to delete a user account on the server.
     *
     * Only #USERTYPE_ADMIN can issue this command.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_ACCOUNT_NOT_FOUND
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param szUsername The username of the user account to delete.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoListUserAccounts
     * @see TT_DoNewUserAccount
     * @see UserAccount
     * @see UserType */
    TEAMTALKDLL_API INT32 TT_DoDeleteUserAccount(IN TTInstance* lpTTInstance,
                                                 IN const TTCHAR* szUsername);

    /**
     * @brief Issue a ban command on a user. 
     *
     * The ban applies to the user's IP-address. Call #TT_DoKickUser
     * to kick the user off the server.
     *
     * User rights required:
     * - #USERRIGHT_BAN_USERS
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_USER_NOT_FOUND
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID The ID of the user to ban.
     * @param nChannelID Set to 0 to ban from logging in. Otherwise specify
     * user's current channel.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoKickUser()
     * @see TT_DoListBans()
     * @see TT_DoBanIPAddress()
     * @see TT_DoBan()
     * @see TT_DoBanUser()
     * @see TT_DoBanUserEx() */
    TEAMTALKDLL_API INT32 TT_DoBanUser(IN TTInstance* lpTTInstance,
                                       IN INT32 nUserID,
                                       IN INT32 nChannelID);

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
    TEAMTALKDLL_API INT32 TT_DoBanUserEx(IN TTInstance* lpTTInstance,
                                         IN INT32 nUserID,
                                         IN BanTypes uBanTypes);
    
    /**
     * @brief Ban the properties specified in @c lpBannedUser.
     *
     * The @c uBanTypes specifies what the ban applies to.  If
     * #BANTYPE_CHANNEL is specified in the @c uBanTypes of @c
     * lpBannedUser then the ban applies to joining a channel,
     * TT_DoJoinChannel(). Otherwise the ban applies to login,
     * TT_DoLogin().
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
    TEAMTALKDLL_API INT32 TT_DoBan(IN TTInstance* lpTTInstance,
                                   IN const BannedUser* lpBannedUser);

    /**
     * @brief Issue a ban command on an IP-address user. 
     *
     * Same as TT_DoBanUser() except this command applies to IP-addresses
     * and therefore doesn't require a user to be logged in.
     *
     * User rights required:
     * - #USERRIGHT_BAN_USERS
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param szIPAddress The IP-address to ban.
     * @param nChannelID Set to zero.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoKickUser
     * @see TT_DoListBans */
    TEAMTALKDLL_API INT32 TT_DoBanIPAddress(IN TTInstance* lpTTInstance,
                                            IN const TTCHAR* szIPAddress,
                                            IN INT32 nChannelID);

    /**
     * @brief Unban the user with the specified IP-address.
     *
     * User rights required:
     * - #USERRIGHT_BAN_USERS
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_BAN_NOT_FOUND
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param szIPAddress The IP-address to unban.
     * @param nChannelID Set to zero.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     *
     * @see TT_DoUnBanUserEx()
     * @see TT_DoBanUser
     * @see TT_DoListBans
     * @see TT_DoBanIPAddress */
    TEAMTALKDLL_API INT32 TT_DoUnBanUser(IN TTInstance* lpTTInstance,
                                         IN const TTCHAR* szIPAddress,
                                         IN INT32 nChannelID);
    /**
     * @brief Unban the properties specified in #BannedUser.
     *
     * The uBanTypes in #BannedUser determines which properties should have
     * their ban remove. E.g. uBanTypes = #BANTYPE_USERNAME and 
     * @c szUsername = "guest" will remove all bans where the username
     * is "guest".
     *
     * @see TT_DoBan() */
    TEAMTALKDLL_API INT32 TT_DoUnBanUserEx(IN TTInstance* lpTTInstance,
                                           IN const BannedUser* lpBannedUser);

    /**
     * @brief Issue a command to list the banned users.
     *
     * The event #CLIENTEVENT_CMD_BANNEDUSER will be posted for every
     * #BannedUser on the server.
     *
     * User rights required:
     * - #USERRIGHT_BAN_USERS
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nChannelID Set to zero.
     * @param nIndex Index of first ban to display.
     * @param nCount The number of bans to display.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoBanUser() */
    TEAMTALKDLL_API INT32 TT_DoListBans(IN TTInstance* lpTTInstance,
                                        IN INT32 nChannelID,
                                        IN INT32 nIndex,
                                        IN INT32 nCount);

    /**
     * @brief Save the server's current state to its settings file
     * (typically the server's .xml file).
     *
     * Only #USERTYPE_ADMIN can issue this command.
     *
     * Note that the server only saves channels with the flag
     * #CHANNEL_PERMANENT.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error.
     * @see TT_DoUpdateServer */
    TEAMTALKDLL_API INT32 TT_DoSaveConfig(IN TTInstance* lpTTInstance);

    /**
     * @brief Get the server's current statistics.
     *
     * Event #CLIENTEVENT_CMD_SERVERSTATISTICS will be posted when
     * server has sent response.
     *
     * Only #USERTYPE_ADMIN can issue this command.
     *
     * Possible errors:
     * - #CMDERR_NOT_LOGGEDIN
     * - #CMDERR_NOT_AUTHORIZED
     * - #CMDERR_UNKNOWN_COMMAND
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error. */
    TEAMTALKDLL_API INT32 TT_DoQueryServerStats(IN TTInstance* lpTTInstance);

    /**
     * @brief Quit from server. 
     *
     * Possible errors:
     * - none
     *
     * This will generate a #CLIENTEVENT_CON_LOST since the server
     * will drop the client.
     *
     * @return Returns command ID which will be passed in 
     * #CLIENTEVENT_CMD_PROCESSING event when the server is processing the 
     * command. -1 is returned in case of error. */
    TEAMTALKDLL_API INT32 TT_DoQuit(IN TTInstance* lpTTInstance);
    /** @} */

    /** @addtogroup server
     * @{
     */
    
    /**
     * @brief Get the server's properties.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param lpServerProperties A struct to hold the server's properties. */
    TEAMTALKDLL_API TTBOOL TT_GetServerProperties(IN TTInstance* lpTTInstance,
                                                  OUT ServerProperties* lpServerProperties);

    /**
     * @brief Get all the users on the server.
     *
     * If only users in a specific channel is needed call
     * TT_GetChannelUsers()
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param lpUsers A preallocated array which has room for @a lpnHowMany 
     * #User elements. Pass NULL to query the number of users in channel.
     * @param lpnHowMany The number of elements in the array @a lpUsers. If
     * @a lpUsers is NULL @a lpnHowMany will receive the number of users.
     * @see TT_GetChannelUsers
     * @see TT_GetUser 
     * @see TT_GetServerChannels*/
    TEAMTALKDLL_API TTBOOL TT_GetServerUsers(IN TTInstance* lpTTInstance,
                                             IN OUT User* lpUsers,
                                             IN OUT INT32* lpnHowMany);
    /** @} */

    /** @addtogroup channels
     * @{ */

    /**
     * @brief Get the root channel's ID
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @return Returns the ID of the root channel. If 0 is returned no root 
     * channel exists.
     * @see TT_GetMyChannelID
     * @see TT_GetChannelPath */
    TEAMTALKDLL_API INT32 TT_GetRootChannelID(IN TTInstance* lpTTInstance);

    /**
     * @brief Get the channel which the local client instance is
     * currently participating in.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @return Returns the ID of the current channel. If 0 is returned the 
     * user is not participating in a channel. */
    TEAMTALKDLL_API INT32 TT_GetMyChannelID(IN TTInstance* lpTTInstance);

    /**
     * @brief Get the channel with a specific ID.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nChannelID The ID of the channel to get information about.
     * @param lpChannel A preallocated struct which will receive the 
     * channel's properties.
     * @return FALSE if unable to retrieve channel otherwise TRUE. */
    TEAMTALKDLL_API TTBOOL TT_GetChannel(IN TTInstance* lpTTInstance,
                                         IN INT32 nChannelID, 
                                         OUT Channel* lpChannel );
    
    /**
     * @brief Get the channel's path. Channels are separated by '/'.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nChannelID The channel's ID.
     * @param szChannelPath Will receive the channel's path.
     * @return Returns TRUE if channel exists. */
    TEAMTALKDLL_API TTBOOL TT_GetChannelPath(IN TTInstance* lpTTInstance,
                                             IN INT32 nChannelID, 
                                             OUT TTCHAR szChannelPath[TT_STRLEN]);

    /**
     * @brief Get the channel ID of the supplied path. Channels are
     * separated by '/'
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param szChannelPath Will receive the channel's path.
     * @return The channel's ID or 0 on error. */
    TEAMTALKDLL_API INT32 TT_GetChannelIDFromPath(IN TTInstance* lpTTInstance,
                                                  IN const TTCHAR* szChannelPath);

    /**
     * @brief Get the IDs of all users in a channel.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nChannelID The channel's ID.
     * @param lpUsers A preallocated array which has room for @a lpnHowMany 
     * user elements. Pass NULL to query the number of users in channel.
     * @param lpnHowMany The number of elements in the array @a lpUsers. If
     * @a lpUserIDs is NULL @a lpnHowMany will receive the number of users in
     * the channel.
     * @see User 
     * @see TT_GetChannel */
    TEAMTALKDLL_API TTBOOL TT_GetChannelUsers(IN TTInstance* lpTTInstance,
                                            IN INT32 nChannelID,
                                            IN OUT User* lpUsers,
                                            IN OUT INT32* lpnHowMany);

    /**
     * @brief Get the list of the files in a channel which can be
     * downloaded.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nChannelID The ID of the channel to extract the files from.
     * @param lpRemoteFiles A preallocated struct which will receive 
     * file information. If @a lpRemoteFiles is NULL then @a lpnHowMany will
     * receive the number of files in the channel.
     * @param lpnHowMany Use for both querying and specifying the number of
     * files. If @a lpRemoteFiles is NULL then lpnHowMany will receive the number
     * of files in the channel. If @a lpRemoteFiles is not NULL then
     * @a lpnHowMany should specify the size of the @a lpRemoteFiles array.
     * @see TT_GetChannelFile */
    TEAMTALKDLL_API TTBOOL TT_GetChannelFiles(IN TTInstance* lpTTInstance,
                                              IN INT32 nChannelID, 
                                              IN OUT RemoteFile* lpRemoteFiles,
                                              IN OUT INT32* lpnHowMany);

    /**
     * @brief Get information about a file which can be downloaded.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nChannelID The ID of the channel to extract the file from.
     * @param nFileID The ID of the file.
     * @param lpRemoteFile A preallocated struct which will receive 
     * file information. */
    TEAMTALKDLL_API TTBOOL TT_GetChannelFile(IN TTInstance* lpTTInstance,
                                             IN INT32 nChannelID, 
                                             IN INT32 nFileID, 
                                             OUT RemoteFile* lpRemoteFile); 
    
    /**
     * @brief Check whether user is operator of a channel
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID the ID of the user to check.
     * @param nChannelID the ID of the channel to check whether user
     * is operator of. */
    TEAMTALKDLL_API TTBOOL TT_IsChannelOperator(IN TTInstance* lpTTInstance,
                                                IN INT32 nUserID, 
                                                IN INT32 nChannelID);

    /** 
     * @brief Get all the channels on the server.
     *
     * Use TT_GetChannel() to get more information about each of the
     * channels. 
     * @see TT_GetServerUsers() */
    TEAMTALKDLL_API TTBOOL TT_GetServerChannels(IN TTInstance* lpTTInstance,
                                                IN OUT Channel* lpChannels,
                                                IN OUT INT32* lpnHowMany);
    /** @} */

    /** @addtogroup users
     * @{ */

    /**
     * @brief Get the local client instance's user ID. 
     *
     * This information can be retrieved after the
     * #CLIENTEVENT_CMD_MYSELF_LOGGEDIN event.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @return Returns the user ID assigned to the current user on the server.
     * -1 is returned if no ID has been assigned to the user. */
    TEAMTALKDLL_API INT32 TT_GetMyUserID(IN TTInstance* lpTTInstance);

    /**
     * @brief Get the local client instance's #UserAccount.
     *
     * This information can be retrieved after
     * #CLIENTEVENT_CMD_MYSELF_LOGGEDIN event.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param lpUserAccount The local client's user account registered on
     * the server. Note that the @a szPassword field of #UserAccount
     * will not be set.
     * @see TT_DoLogin */
    TEAMTALKDLL_API TTBOOL TT_GetMyUserAccount(IN TTInstance* lpTTInstance,
                                               OUT UserAccount* lpUserAccount);
    
    /**
     * @brief Get the client instance's user type. 
     *
     * This information can be retrieved after
     * #CLIENTEVENT_CMD_MYSELF_LOGGEDIN event.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @return A bitmask of the type of user based on #UserType.
     * @see TT_GetMyUserAccount
     * @see TT_DoLogin
     * @see UserType */
    TEAMTALKDLL_API UserTypes TT_GetMyUserType(IN TTInstance* lpTTInstance);

    /**
     * @brief Convenience method for TT_GetMyUserAccount()
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     */
    TEAMTALKDLL_API UserRights TT_GetMyUserRights(IN TTInstance* lpTTInstance);

    /**
     * @brief If an account was used in #TT_DoLogin then this value will 
     * return the @a nUserData from the #UserAccount.
     *
     * This information can be retrieved after
     * #CLIENTEVENT_CMD_MYSELF_LOGGEDIN event.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @return If set, @a nUserData from #UserAccount, otherwise 0.
     * @see TT_GetMyUserAccount */
    TEAMTALKDLL_API INT32 TT_GetMyUserData(IN TTInstance* lpTTInstance);

    /**
     * @brief Get the user with the specified ID.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID The ID of the user to extract.
     * @param lpUser A preallocated #User struct.
     * @see TT_GetUserByUsername */
    TEAMTALKDLL_API TTBOOL TT_GetUser(IN TTInstance* lpTTInstance,
                                      IN INT32 nUserID, OUT User* lpUser);
    
    /**
     * @brief Get statistics for data and packet reception from a user.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID The ID of the user to extract.
     * @param lpUserStatistics A preallocated #UserStatistics struct. */
    TEAMTALKDLL_API TTBOOL TT_GetUserStatistics(IN TTInstance* lpTTInstance,
                                                IN INT32 nUserID, 
                                                OUT UserStatistics* lpUserStatistics);
    /**
     * @brief Get the user with the specified username.
     *
     * Remember to take into account that multiple users can log in
     * with the same account if #USERRIGHT_MULTI_LOGIN is specified.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param szUsername The user's username (from #UserAccount).
     * @param lpUser A preallocated #User struct. */
    TEAMTALKDLL_API TTBOOL TT_GetUserByUsername(IN TTInstance* lpTTInstance,
                                                IN const TTCHAR* szUsername, 
                                                OUT User* lpUser);
    /** @} */

    /** @addtogroup sounddevices
     * @{ */

    /**
     * @brief Set the volume of a user.
     *
     * Note that it's a virtual volume which is being set since the
     * master volume affects the user volume.
     * 
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID The ID of the user whose volume will be changed.
     * @param nStreamType The type of stream to change, either 
     * #STREAMTYPE_VOICE or #STREAMTYPE_MEDIAFILE_AUDIO.
     * @param nVolume Must be between #SOUND_VOLUME_MIN and #SOUND_VOLUME_MAX.
     * @see TT_SetSoundOutputVolume */
    TEAMTALKDLL_API TTBOOL TT_SetUserVolume(IN TTInstance* lpTTInstance,
                                            IN INT32 nUserID, 
                                            IN StreamType nStreamType,
                                            IN INT32 nVolume);

    /**
     * @brief Mute a user.
     *
     * To stop receiving audio from a user call #TT_DoUnsubscribe with
     * #SUBSCRIBE_VOICE.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID The user ID of the user to mute (or unmute).
     * @param nStreamType The type of stream to change, either 
     * #STREAMTYPE_VOICE or #STREAMTYPE_MEDIAFILE_AUDIO.
     * @param bMute TRUE will mute, FALSE will unmute.
     * @see TT_SetSoundOutputMute */
    TEAMTALKDLL_API TTBOOL TT_SetUserMute(IN TTInstance* lpTTInstance,
                                          IN INT32 nUserID,
                                          IN StreamType nStreamType,
                                          IN TTBOOL bMute);

    /**
     * @brief Set the delay of when a user should no longer be considered
     * as playing audio (either voice or audio from media file).
     *
     * When a user starts talking the #CLIENTEVENT_USER_STATECHANGE is
     * triggered with @c uUserState changing. A user will remain in this active
     * state until no packets are received from this user, plus a
     * delay (due to network interruptions). This delay is by default
     * set to 500 msec but can be changed by calling
     * TT_SetUserStoppedTalkingDelay(). */
    TEAMTALKDLL_API TTBOOL TT_SetUserStoppedPlaybackDelay(IN TTInstance* lpTTInstance,
                                                          IN INT32 nUserID, 
                                                          IN StreamType nStreamType,
                                                          IN INT32 nDelayMSec);

    /**
     * @brief Set the position of a user.
     *
     * This can only be done using DirectSound (#SOUNDSYSTEM_DSOUND)
     * and with sound duplex mode (#CLIENT_SNDINOUTPUT_DUPLEX)
     * disabled.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID ID of user.
     * @param nStreamType The type of stream to change, either 
     * #STREAMTYPE_VOICE or #STREAMTYPE_MEDIAFILE_AUDIO.
     * @param x Distance in meters to user (left/right).
     * @param y Distance in meters to user (back/forward).
     * @param z Distance in meters to user (up/down). */
    TEAMTALKDLL_API TTBOOL TT_SetUserPosition(IN TTInstance* lpTTInstance,
                                              IN INT32 nUserID, 
                                              IN StreamType nStreamType,
                                              IN float x,
                                              IN float y, 
                                              IN float z);

    /**
     * @brief Set whether a user should speak in the left, right or
     * both speakers. This function only works if #AudioCodec has been
     * set to use stereo.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID ID of user.
     * @param nStreamType The type of stream to change, either 
     * #STREAMTYPE_VOICE or #STREAMTYPE_MEDIAFILE_AUDIO.
     * @param bLeftSpeaker TRUE if user should be played in left speaker.
     * @param bRightSpeaker TRUE if user should be played in right speaker. */
    TEAMTALKDLL_API TTBOOL TT_SetUserStereo(IN TTInstance* lpTTInstance,
                                            IN INT32 nUserID, 
                                            IN StreamType nStreamType,
                                            IN TTBOOL bLeftSpeaker, 
                                            IN TTBOOL bRightSpeaker);

    /**
     * @brief Store user's audio to disk.
     * 
     * Set the path of where to store audio from a user to disk.
     * Event #CLIENTEVENT_USER_RECORD_MEDIAFILE is triggered when
     * recording starts/stops.
     *
     * To stop recording set @a szFolderPath to an empty string and @a
     * uAFF to #AFF_NONE.
     *
     * To store audio of users not in current channel of the client
     * instance check out the section @ref spying.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID The ID of the #User which should store audio to
     * disk. Pass 0 to store local instance's media stream.
     * @param szFolderPath The path on disk to where files should be
     * stored.  This value will be stored in @a szMediaStorageDir of
     * #User.  
     * @param szFileNameVars The file name used for audio files can
     * consist of the following variables: \%nickname\%, \%username\%,
     * \%userid\%, \%counter\% and a specified time based on @c
     * strftime (google @c 'strftime' for a description of the
     * format. The default format used by TeamTalk is:
     * '\%Y\%m\%d-\%H\%M\%S #\%userid\% \%username\%'. The \%counter\%
     * variable is a 9 digit integer which is incremented for each
     * audio file. The file extension is automatically appended based
     * on the file type (.wav for #AFF_WAVE_FORMAT and .mp3 for
     * AFF_MP3_*_FORMAT). Pass NULL or empty string to revert to
     * default format.
     * @param uAFF The #AudioFileFormat to use for storing audio files. Passing
     * #AFF_NONE will cancel/reset the current recording.
     * @return FALSE if path is invalid, otherwise TRUE.
     * @see User
     * @see CLIENTEVENT_USER_RECORD_MEDIAFILE */
    TEAMTALKDLL_API TTBOOL TT_SetUserMediaStorageDir(IN TTInstance* lpTTInstance,
                                                     IN INT32 nUserID,
                                                     IN const TTCHAR* szFolderPath,
                                                     IN const TTCHAR* szFileNameVars,
                                                     IN AudioFileFormat uAFF);
    /**
     * @brief Change the amount of media data which can be buffered
     * in the user's playback queue.
     * 
     * Increasing the media buffer size is especially important when
     * the user is currently streaming a media file using
     * TT_StartStreamingMediaFileToChannel(). Once streaming has finished
     * it is recommended to reset the media buffer, i.e. setting it
     * to zero.
     *
     * A simple way to notify users that the client instance is streaming
     * a media file is to change the status of the local client instance
     * using TT_DoChangeStatus() or to send a #TextMessage using
     * TT_DoTextMessage().
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nUserID The ID of the user who should have changed
     * the size of the media buffer.
     * @param uStreamType The stream type to apply the buffer for, i.e.
     * either #STREAMTYPE_VOICE or #STREAMTYPE_MEDIAFILE_AUDIO.
     * @param nMSec The number of miliseconds of audio and video which
     * should be allowed to be stored in the media buffer. 2000 - 3000 
     * msec is a good size for a media buffer. Set the media
     * buffer size to 0 msec to reset the media buffer to its default value.
     */
    TEAMTALKDLL_API TTBOOL TT_SetUserAudioStreamBufferSize(IN TTInstance* lpTTInstance,
                                                           IN INT32 nUserID,
                                                           IN StreamTypes uStreamType,
                                                           IN INT32 nMSec);

    /**
     * @brief Extract the raw audio associated with the event
     * #CLIENTEVENT_USER_AUDIOBLOCK.
     *
     * To enable access to raw audio first call
     * TT_EnableAudioBlockEvent(). Whenever new audio becomes
     * available the event #CLIENTEVENT_USER_AUDIOBLOCK is generated
     * and TT_AcquireUserAudioBlock() can be called to extract the
     * audio.
     *
     * The #AudioBlock contains shared memory with the local client
     * instance therefore always remember to call
     * TT_ReleaseUserAudioBlock() to release the shared memory.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nStreamType The stream type to extract, either ::STREAMTYPE_VOICE
     * ::STREAMTYPE_MEDIAFILE_AUDIO.
     * @param nUserID The ID of the user to retrieve the #AudioBlock from.
     * Basically #TTMessage's @c nSource from #CLIENTEVENT_USER_AUDIOBLOCK.
     * @see TT_ReleaseUserAudioBlock()
     * @see TT_EnableAudioBlockEvent()
     * @see CLIENTEVENT_USER_AUDIOBLOCK */
    TEAMTALKDLL_API AudioBlock* TT_AcquireUserAudioBlock(IN TTInstance* lpTTInstance,
                                                         IN StreamType nStreamType,
                                                         IN INT32 nUserID);

    /** 
     * @brief Release the shared memory of an #AudioBlock.
     *
     * All #AudioBlock-structures extracted through
     * TT_AcquireUserAudioBlock() must be released again since they
     * share memory with the local client instance.
     *
     * Never access @c lpAudioBlock after releasing its
     * #AudioBlock. This will cause the application to crash with a
     * memory exception.
     *
     * @see TT_AcquireUserAudioBlock()
     * @see CLIENTEVENT_USER_AUDIOBLOCK */
    TEAMTALKDLL_API TTBOOL TT_ReleaseUserAudioBlock(IN TTInstance* lpTTInstance,
                                                    IN AudioBlock* lpAudioBlock);

    /** @} */

    /** @ingroup channels
     * @brief Get information about an active file transfer.  
     *
     * An active file transfer is one which has been posted through the
     * event #CLIENTEVENT_FILETRANSFER.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nTransferID The ID of the file transfer to investigate. Transfer
     * ID is passed by #CLIENTEVENT_FILETRANSFER.
     * @param lpFileTransfer A preallocated struct which will receive the file 
     * transfer information.
     * @see TT_CancelFileTransfer */
    TEAMTALKDLL_API TTBOOL TT_GetFileTransferInfo(IN TTInstance* lpTTInstance,
                                                  IN INT32 nTransferID, 
                                                  OUT FileTransfer* lpFileTransfer);

    /** @ingroup channels
     * @brief Cancel an active file transfer. 
     *
     * An active file transfer is one which has been post through the
     * event #CLIENTEVENT_FILETRANSFER.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nTransferID The ID of the file transfer to investigate. Transfer 
     * ID is passed by #CLIENTEVENT_FILETRANSFER. */
    TEAMTALKDLL_API TTBOOL TT_CancelFileTransfer(IN TTInstance* lpTTInstance,
                                                 IN INT32 nTransferID);

    /** @ingroup errorhandling
     * @brief Get textual discription of an error message.
     * 
     * Get a description of an error code posted by either
     * #CLIENTEVENT_CMD_ERROR or #CLIENTEVENT_INTERNAL_ERROR.
     *
     * @param nError The number of the error.
     * @param szErrorMsg A text description of the error.
     * @see CLIENTEVENT_CMD_ERROR
     * @see CLIENTEVENT_INTERNAL_ERROR */
    TEAMTALKDLL_API void TT_GetErrorMessage(IN INT32 nError, 
                                            OUT TTCHAR szErrorMsg[TT_STRLEN]);


    /** @addtogroup desktopshare
     * @{ */

    /**
     * @brief Translate to and from TeamTalk's intermediate key-codes (TTKEYCODE).
     * @see #CLIENTEVENT_USER_DESKTOPINPUT */
    typedef enum TTKeyTranslate
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
    } TTKeyTranslate;

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
     * @param lpDesktopInputs An array of #DesktopInput structs to translate.
     * @param lpTranslatedDesktopInputs A pre-allocated array of #DesktopInput
     * struct to hold the translated desktop input.
     * @param nDesktopInputCount The number of elements to translate in @c
     * lpDesktopInputs.
     * @return The number of translated #DesktopInput stucts. If value
     * is different from @c nDesktopInputCount then some @c uKeyCode
     * values could not be translated and have been assigned the value
     * TT_DESKTOPINPUT_KEYCODE_IGNORE.
     * @see TT_SendDesktopInput()
     * @see TT_DesktopInput_Execute() */
    TEAMTALKDLL_API INT32 TT_DesktopInput_KeyTranslate(TTKeyTranslate nTranslate,
                                                       IN const DesktopInput* lpDesktopInputs,
                                                       OUT DesktopInput* lpTranslatedDesktopInputs,
                                                       IN INT32 nDesktopInputCount);

    /**
     * @brief Execute desktop (mouse or keyboard) input.
     *
     * When executed either a key-press, key-release or mouse move
     * will take place on the computer running the client
     * instance. Remember to calculate the offsets for the mouse
     * cursor prior to this call. The mouse position will be relative
     * to the screen resolution.
     *
     * The content of the #DesktopInput struct must been translated to
     * the platform's key-code format prior to this
     * call. I.e. uKeyCode must be a either a Windows scan-code, Mac
     * OS X Carbon key-code or one of the mouse buttons:
     * #TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN,
     * #TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN,
     * #TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN.
     *
     * @param lpDesktopInputs The mouse or keyboard inputs.
     * @param nDesktopInputCount The number of elements in @c lpDesktopInputs.
     * @return The number of mouse and keyboard events inserted.
     * @see TT_DesktopInput_KeyTranslate() */
    TEAMTALKDLL_API INT32 TT_DesktopInput_Execute(IN const DesktopInput* lpDesktopInputs,
                                                  IN INT32 nDesktopInputCount);

    /** @} */

#if defined(WIN32)
    
    /** @addtogroup hotkey
     * @{ */

    /**
     * @brief Register a global hotkey. 
     *
     * When the hotkey becomes active or inactive it will send
     * #CLIENTEVENT_HOTKEY to the HWND passed to #TT_InitTeamTalk.
     *
     * A hotkey can e.g. be used as a push-to-talk key
     * combination. When the hotkey becomes active call
     * TT_EnableVoiceTransmission().
     *
     * Note that having a hotkey enabled makes the Visual Studio
     * debugger really slow to respond, so when debugging it's best
     * not to have hotkeys enabled.
     *
     * @param lpTTInstance Pointer to client instance created by 
     * #TT_InitTeamTalk.
     * @param nHotKeyID The ID of the hotkey to register. It will be
     * passed as the WPARAM when the hotkey becomes either active or inactive.
     * @param lpnVKCodes An array of virtual key codes which constitute the
     * hotkey. This document outlines the virtual key codes:
     * http://msdn.microsoft.com/en-us/library/ms645540(VS.85).aspx
     * A hotkey consisting of Left Control+A would have the array consist of 
     * [162, 65].
     * @param nVKCodeCount The number of virtual key codes in the array
     * (in other words the size of the @a lpnVKCodes array).
     * @see TT_InitTeamTalk
     * @see TT_HotKey_Unregister
     * @see TT_HotKey_InstallTestHook */
    TEAMTALKDLL_API TTBOOL TT_HotKey_Register(IN TTInstance* lpTTInstance,
                                              IN INT32 nHotKeyID, 
                                              IN const INT32* lpnVKCodes,
                                              IN INT32 nVKCodeCount);

    /**
     * @brief Unregister a registered hotkey.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nHotKeyID is the ID of the hotkey to unregister.
     * @see TT_HotKey_Register */
    TEAMTALKDLL_API TTBOOL TT_HotKey_Unregister(IN TTInstance* lpTTInstance,
                                                IN INT32 nHotKeyID);

    /**
     * @brief Check whether hotkey is active.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nHotKeyID is the ID of the registered hotkey. 
     * @return 1 if active, 0 if inactive, -1 if hotkey-ID is invalid */
    TEAMTALKDLL_API INT32 TT_HotKey_IsActive(IN TTInstance* lpTTInstance,
                                             IN INT32 nHotKeyID);

    /**
     * @brief Install a test hook so the HWND will be messaged
     * whenever a key or mouse button is pressed.
     *
     * Capture the event #CLIENTEVENT_HOTKEY_TEST.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param hWnd is the handle of the window which will be
     * notified.
     * @param uMsg The window message to post to @c hWnd when a key event
     * occurs. Basically it calls PostMessage(hWnd, Msg, VirtualKey-id, Active);
     * @see TT_HotKey_RemoveTestHook
     * @see CLIENTEVENT_HOTKEY_TEST */
    TEAMTALKDLL_API TTBOOL TT_HotKey_InstallTestHook(IN TTInstance* lpTTInstance,
                                                     IN HWND hWnd, UINT32 uMsg);

    /**
     * @brief Remove the test hook again so the @a hWnd in
     * #TT_HotKey_InstallTestHook will no longer be notified.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @see TT_HotKey_InstallTestHook */
    TEAMTALKDLL_API TTBOOL TT_HotKey_RemoveTestHook(IN TTInstance* lpTTInstance);

    /**
     * @brief Get a string description of the virtual-key code.
     *
     * @param lpTTInstance Pointer to client instance created by
     * #TT_InitTeamTalk.
     * @param nVKCode The virtual key code passed in #CLIENTEVENT_HOTKEY_TEST.
     * @param szKeyName Will receive key description in local language.
     * @see TT_HotKey_Register */
    TEAMTALKDLL_API TTBOOL TT_HotKey_GetKeyString(IN TTInstance* lpTTInstance,
                                                  IN INT32 nVKCode,
                                                  OUT TTCHAR szKeyName[TT_STRLEN]);
    /** @} */
#endif

    /* Get the 'sizeof' of a structure used by TeamTalk. Useful to ensuring 
     * binary compatibility when integrating with other programming 
     * languages. */
    TEAMTALKDLL_API INT32 TT_DBG_SIZEOF(IN TTType nType);

    TEAMTALKDLL_API VOID* TT_DBG_GETDATAPTR(IN TTMessage* pMsg);

    TEAMTALKDLL_API TTBOOL TT_DBG_SetSoundInputTone(IN TTInstance* lpTTInstance,
                                                    IN StreamTypes uStreamTypes,
                                                    IN INT32 nFrequency);

    TEAMTALKDLL_API TTBOOL TT_DBG_WriteAudioFileTone(IN const MediaFileInfo* lpMediaFileInfo,
                                                     IN INT32 nFrequency);

#if defined(WIN32) /* Exclude mixer and firewall functions from
                    * non-Windows platforms */

    /** @addtogroup mixer
     * @{ */

    /**
     * @brief The Windows mixer controls which can be queried by the
     * TT_Mixer_* functions.
     *
     * Wave-In devices which are not in the enum-structure can be
     * accessed by #TT_Mixer_GetWaveInControlCount which allows the user to
     * query selection based on an index.
     *
     * Note that Windows Vista has deprecated mixer controls.
     *
     * @see TT_Mixer_SetWaveOutMute
     * @see TT_Mixer_SetWaveOutVolume
     * @see TT_Mixer_SetWaveInSelected
     * @see TT_Mixer_SetWaveInVolume
     * @see TT_Mixer_GetWaveInControlName
     * @see TT_Mixer_SetWaveInControlSelected */
    typedef enum MixerControl
    {
        WAVEOUT_MASTER,
        WAVEOUT_WAVE,
        WAVEOUT_MICROPHONE,

        WAVEIN_MICROPHONE,
        WAVEIN_LINEIN,
        WAVEIN_WAVEOUT,
    } MixerControl;

    /**
     * @brief Get the number of Windows Mixers available.
     *
     * The index from 0 to #TT_Mixer_GetMixerCount()-1 should be passed to the
     * TT_Mixer_* functions.
     * @see TT_Mixer_GetMixerName */
    TEAMTALKDLL_API INT32 TT_Mixer_GetMixerCount(void);

    /**
     * @brief Get the name of a Windows Mixer based on its name.
     *
     * @param nMixerIndex The index of the mixer. Ranging from 0 to 
     * #TT_Mixer_GetMixerCount()-1.
     * @param szMixerName The output string receiving the name of the device. */
    TEAMTALKDLL_API TTBOOL TT_Mixer_GetMixerName(IN INT32 nMixerIndex,
                                                 OUT TTCHAR szMixerName[TT_STRLEN]);

    /**
     * @brief Get the name of the mixer associated with a wave-in device.
     * 
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param szMixerName The output string receiving the name of the device. 
     * @see TT_GetSoundDevices() */
    TEAMTALKDLL_API TTBOOL TT_Mixer_GetWaveInName(IN INT32 nWaveDeviceID,
                                                  OUT TTCHAR szMixerName[TT_STRLEN]);

    /**
     * @brief Get the name of the mixer associated with a wave-out device.
     * 
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param szMixerName The output string receiving the name of the device. 
     * @see TT_GetSoundDevices */
    TEAMTALKDLL_API TTBOOL TT_Mixer_GetWaveOutName(IN INT32 nWaveDeviceID,
                                                   OUT TTCHAR szMixerName[TT_STRLEN]);

    /**
     * @brief Mute or unmute a Windows Mixer Wave-Out device from the
     * 'enum' of devices.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param nControl A mixer control.
     * @param bMute True if device should be muted, False if it should be
     * unmuted.
     * @see TT_Mixer_GetWaveOutMute */
    TEAMTALKDLL_API TTBOOL TT_Mixer_SetWaveOutMute(IN INT32 nWaveDeviceID, 
                                                   IN MixerControl nControl, 
                                                   IN TTBOOL bMute);

    /**
     * @brief Get the mute state of a Windows Mixer Wave-Out device
     * from the 'enum' of devices.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param nControl A mixer control.
     * @return TRUE if mute, FALSE if unmuted, -1 on error.
     * @see TT_Mixer_SetWaveOutMute */
    TEAMTALKDLL_API INT32 TT_Mixer_GetWaveOutMute(IN INT32 nWaveDeviceID, 
                                                  IN MixerControl nControl);

    /**
     * @brief Set the volume of a Windows Mixer Wave-Out device from
     * the 'enum' of devices.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param nControl A mixer control.
     * @param nVolume A value ranging from 0 to 65535. */
    TEAMTALKDLL_API TTBOOL TT_Mixer_SetWaveOutVolume(IN INT32 nWaveDeviceID, 
                                                     IN MixerControl nControl, 
                                                     IN INT32 nVolume);

    /**
     * @brief Get the volume of a Windows Mixer Wave-Out device from
     * the 'enum' of devices.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param nControl A mixer control.
     * @return A value ranging from 0 to 65535, or -1 on error. */
    TEAMTALKDLL_API INT32 TT_Mixer_GetWaveOutVolume(IN INT32 nWaveDeviceID, 
                                                    IN MixerControl nControl);

    /**
     * @brief Set the selected state of a Windows Mixer Wave-In
     * device from the 'enum' of devices.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param nControl A mixer control. */
    TEAMTALKDLL_API TTBOOL TT_Mixer_SetWaveInSelected(IN INT32 nWaveDeviceID, 
                                                      IN MixerControl nControl);

    /**
     * @brief Get the selected state of a Windows Mixer Wave-In device
     * from the 'enum' of devices.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param nControl A mixer control.
     * @return TRUE if mute, FALSE if unmuted, -1 on error. */
    TEAMTALKDLL_API INT32 TT_Mixer_GetWaveInSelected(IN INT32 nWaveDeviceID, 
                                                     IN MixerControl nControl);

    /**
     * @brief Set the volume of a Windows Mixer Wave-In device from
     * the 'enum' of devices.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param nControl A mixer control.
     * @param nVolume A value ranging from 0 to 65535. */
    TEAMTALKDLL_API TTBOOL TT_Mixer_SetWaveInVolume(IN INT32 nWaveDeviceID, 
                                                    IN MixerControl nControl, 
                                                    IN INT32 nVolume);

    /**
     * @brief Get the volume of a Windows Mixer Wave-In device from
     * the 'enum' of devices.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param nControl A mixer control.
     * @return A value ranging from 0 to 65535, or -1 on error. */
    TEAMTALKDLL_API INT32 TT_Mixer_GetWaveInVolume(IN INT32 nWaveDeviceID, 
                                                   IN MixerControl nControl);

    /**
     * @brief Enable and disable microphone boost.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param bEnable TRUE to enable, FALSE to disable. */
    TEAMTALKDLL_API TTBOOL TT_Mixer_SetWaveInBoost(IN INT32 nWaveDeviceID, 
                                                   IN TTBOOL bEnable);
    /**
     * @brief See if microphone boost is enabled.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @return TRUE if boost is enabled, FALSE if disabled, -1 on error. */
    TEAMTALKDLL_API INT32 TT_Mixer_GetWaveInBoost(IN INT32 nWaveDeviceID);

    /**
     * @brief Mute/unmute microphone input.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param bEnable TRUE to enable, FALSE to disable. */
    TEAMTALKDLL_API TTBOOL TT_Mixer_SetWaveInMute(IN INT32 nWaveDeviceID, 
                                                  IN TTBOOL bEnable);

    /**
     * @brief See if microphone is muted.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @return TRUE if mute is enabled, FALSE if disabled, -1 on error. */
    TEAMTALKDLL_API INT32 TT_Mixer_GetWaveInMute(IN INT32 nWaveDeviceID);

    /**
     * @brief Get the number of Windows Mixer Wave-In devices.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @return Number of controls, or -1 on error.
     * @see TT_Mixer_GetWaveInControlName
     * @see TT_Mixer_SetWaveInControlSelected
     * @see TT_Mixer_GetWaveInControlSelected */
    TEAMTALKDLL_API INT32 TT_Mixer_GetWaveInControlCount(IN INT32 nWaveDeviceID);

    /**
     * @brief Get the name of the Wave-In device with the specified
     * index.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param nControlIndex The index of the control. Randing from 0 to 
     * #TT_Mixer_GetWaveInControlCount()-1.
     * @param szDeviceName The output string of the name of the device.
     * @see TT_Mixer_GetWaveInControlCount */
    TEAMTALKDLL_API TTBOOL TT_Mixer_GetWaveInControlName(IN INT32 nWaveDeviceID, 
                                                         IN INT32 nControlIndex, 
                                                         OUT TTCHAR szDeviceName[TT_STRLEN]);

    /**
     * @brief Set the selected state of a Wave-In device in the
     * Windows Mixer.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param nControlIndex The index of the device. Randing from 0 to 
     * #TT_Mixer_GetWaveInControlCount()-1.
     * @see TT_Mixer_GetWaveInControlCount */
    TEAMTALKDLL_API TTBOOL TT_Mixer_SetWaveInControlSelected(IN INT32 nWaveDeviceID, 
                                                             IN INT32 nControlIndex);

    /**
     * @brief Get the selected state of a Wave-In device in the
     * Windows Mixer.
     *
     * @param nWaveDeviceID The @a nWaveDeviceID from the #SoundDevice
     * struct.
     * @param nControlIndex The index of the device. Randing from 0 to  
     * #TT_Mixer_GetWaveInControlCount()-1.
     * @return TRUE if selected, FALSE if unselected, -1 on error.
     * @see TT_Mixer_GetWaveInControlCount */
    TEAMTALKDLL_API TTBOOL TT_Mixer_GetWaveInControlSelected(IN INT32 nWaveDeviceID, 
                                                             IN INT32 nControlIndex);
    /** @} */

    /** @addtogroup firewall
     * @{ */

    /**
     * @brief Check if the Windows Firewall is currently enabled.
     *
     * This function does not invoke UAC on Windows Vista/7.
     * @see TT_Firewall_Enable */
    TEAMTALKDLL_API TTBOOL TT_Firewall_IsEnabled(void);
    
    /**
     * @brief Enable/disable the Windows Firewall.
     *
     * The Windows Firewall was introduced in Windows XP SP2.
     *
     * On Windows XP (SP2+) the user calling this function is assumed
     * to have administrator rights. On Windows Vista/7 UAC is invoked
     * to ask the user for administrator rights.
     * @see TT_Firewall_IsEnabled */
    TEAMTALKDLL_API TTBOOL TT_Firewall_Enable(IN TTBOOL bEnable);

    /**
     * @brief Check if an executable is already in the Windows
     * Firewall exception list.
     *
     * This function does not invoke UAC on Windows Vista/7.
     * @see TT_Firewall_AddAppException */
    TEAMTALKDLL_API TTBOOL TT_Firewall_AppExceptionExists(IN const TTCHAR* szExecutable);

    /**
     * @brief Add an application to the Windows Firewall exception
     * list.
     *
     * On Windows XP (SP2+) the user calling this function is assumed
     * to have administrator rights. On Windows Vista/7 UAC is invoked
     * to ask the user for administrator rights.
     * @see TT_Firewall_AppExceptionExists
     * @see TT_Firewall_RemoveAppException */
    TEAMTALKDLL_API TTBOOL TT_Firewall_AddAppException(IN const TTCHAR* szName, 
                                                       IN const TTCHAR* szExecutable);
    
    /**
     * @brief Remove an application from the Windows Firewall exception
     * list.
     *
     * On Windows XP (SP2+) the user calling this function is assumed
     * to have administrator rights. On Windows Vista/7 UAC is invoked
     * to ask the user for administrator rights.
     * @see TT_Firewall_AppExceptionExists
     * @see TT_Firewall_AddAppException */
    TEAMTALKDLL_API TTBOOL TT_Firewall_RemoveAppException(IN const TTCHAR* szExecutable);
    /** @} */

#endif /** WIN32 */

#ifdef __cplusplus
}
#endif

#endif /** TEAMTALKDLL_H */
