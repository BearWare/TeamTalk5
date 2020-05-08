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

#include "SoundSystem.h"

#if defined(ENABLE_PORTAUDIO)
#include "PortAudioWrapper.h"
#elif defined(ENABLE_OPENSLES)
#include "OpenSLESWrapper.h"
#elif defined(ENABLE_AUDIOTOOLKIT)
#include "AudioTbox.h"
#elif defined(ENABLE_AUDIOUNIT)
#include "AudioUnit.h"
#else

#include "SoundSystemBase.h"

namespace soundsystem {

    class MySoundSystem : public SoundSystemBase<SoundGroup, InputStreamer, OutputStreamer, DuplexStreamer>
    {
    public:
        MySoundSystem() { Init(); }
        ~MySoundSystem() { Close(); }

        bool Init() { RefreshDevices(); return true; }
        void Close() {}

        void FillDevices(sounddevices_t& sounddevs) {}

        bool GetDefaultDevices(int& inputdeviceid, int& outputdeviceid)
        {
            inputdeviceid = outputdeviceid = SOUND_DEVICEID_VIRTUAL;
            return true;
        }

        bool GetDefaultDevices(SoundAPI sndsys, int& inputdeviceid, int& outputdeviceid)
        {
            switch(sndsys)
            {
            case SOUND_API_NOSOUND :
                inputdeviceid = outputdeviceid = SOUND_DEVICEID_VIRTUAL;
                return true;
            default:
                return false;
            }
        }

        soundgroup_t NewSoundGroup()
        {
            return soundgroup_t(new SoundGroup());
        }
        void RemoveSoundGroup(soundgroup_t grp) {}

        inputstreamer_t NewStream(StreamCapture* capture,
                                  int inputdeviceid, int sndgrpid,
                                  int samplerate, int channels,
                                  int framesize) { return inputstreamer_t(); }
        bool StartStream(inputstreamer_t streamer) { return false; }
        void CloseStream(inputstreamer_t streamer) {}
        bool IsStreamStopped(inputstreamer_t streamer) { return true; }

        outputstreamer_t NewStream(StreamPlayer* player, int outputdeviceid,
                                   int sndgrpid, int samplerate, int channels,
                                   int framesize) { return outputstreamer_t(); }
        void CloseStream(outputstreamer_t streamer) {}

        bool StartStream(outputstreamer_t streamer) { return false; }
        bool StopStream(outputstreamer_t streamer) { return false; }
        bool IsStreamStopped(outputstreamer_t streamer) { return true; }
        duplexstreamer_t NewStream(StreamDuplex* duplex, int inputdeviceid,
                                   int outputdeviceid, int sndgrpid,
                                   int samplerate, int input_channels,
                                   int output_channels, int framesize) { return duplexstreamer_t(); }
        void CloseStream(duplexstreamer_t streamer) {}
        bool StartStream(duplexstreamer_t streamer) { return false; }
        bool IsStreamStopped(duplexstreamer_t streamer) { return true; }
    };
}

#endif

namespace soundsystem {

    soundsystem_t GetInstance()
    {
#if defined(ENABLE_PORTAUDIO)
        return PortAudio::getInstance();
#elif defined(ENABLE_OPENSLES)
        return OpenSLESWrapper::getInstance();
#elif defined(ENABLE_AUDIOTOOLKIT)
        return AudioTbox::getInstance();
#elif defined(ENABLE_AUDIOUNIT)
        return soundsystem::getAudUnit();
#else
        static soundsystem_t p(new MySoundSystem());
        return p;
#endif
    }

}
