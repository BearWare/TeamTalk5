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

#if !defined(PORTAUDIOWRAPPER_H)
#define PORTAUDIOWRAPPER_H

#include "SoundSystemBase.h"

#include "SoundSystem.h"
#include "codec/MediaUtil.h"
#include "myace/MyACE.h"
#include "mystd/MyStd.h"

#include <portaudio.h>

#include <cassert>
#include <cstdint>
#include <memory>
#if defined(WIN32)
#include <future>
#endif /* WIN32 */

namespace soundsystem
{

    struct PaStreamer
    {
        PaStream* stream;
        PaStreamer() : stream(nullptr)
        {
            
        }
        virtual ~PaStreamer()
        {
            MYTRACE(ACE_TEXT("~PaStreamer()\n"));
        }

        uint32_t DurationMSec() const
        {
            assert(!m_initialcallback);
            return GETTIMESTAMP() - m_starttime;
        }
        uint32_t DurationSamplesMSec(int samplerate) const
        {
            return uint32_t(PCM16_SAMPLES_DURATION(m_processedsamples, samplerate));
        }
        bool Tick(int samples)
        {
            bool const b = m_initialcallback;
            if (m_initialcallback)
                m_starttime = GETTIMESTAMP();
            m_initialcallback = false;
            m_processedsamples += samples;
            return b;
        }
    private:
        uint64_t m_processedsamples = 0;
        uint32_t m_starttime = 0;
        bool m_initialcallback = true;
    };

    struct PaInputStreamer : InputStreamer, PaStreamer
    {
        PaInputStreamer(StreamCapture* r, int sg, int fs, int sr, int chs, SoundAPI sndsys, int devid)
            : InputStreamer(r, sg, fs, sr, chs, sndsys, devid)
        { }
    };

    struct PaOutputStreamer : OutputStreamer, PaStreamer
    {
        PaOutputStreamer(StreamPlayer* p, int sg, int fs, int sr, int chs, SoundAPI sndsys, int devid)
            : OutputStreamer(p, sg, fs, sr, chs, sndsys, devid)
        { }
    };

    struct PaDuplexStreamer : DuplexStreamer, PaStreamer
    {
        // Pa_OpenStream doesn't return until DuplexCallback is called at least once
        PaDuplexStreamer(StreamDuplex* d, int sg, int fs, int sr, int inchs, int outchs,
                         SoundAPI out_sndsys, int inputdeviceid, int outputdeviceid)
            : DuplexStreamer(d, sg, fs, sr, inchs, outchs, out_sndsys, inputdeviceid, outputdeviceid)
        { }
        ~PaDuplexStreamer() override
        {
        }
#if defined(WIN32)
        std::shared_ptr<class CWMAudioAECCapture> winaec;
        void TickEcho(int echosamples)
        {
            echosamples22050 += echosamples;
        }
        uint32_t DurationEchoSamplesMSec(int samplerate)
        {
            assert(samplerate == 22050);
            return uint32_t(PCM16_SAMPLES_DURATION(echosamples22050, samplerate));
        }
    private:
        uint64_t echosamples22050 = 0;
#endif /* WIN32 */
    };

    struct PaSoundGroup : SoundGroup
    {
        bool autoposition = false;;
        PaSoundGroup() = default;
    };

    using SSB = SoundSystemBase < PaSoundGroup, PaInputStreamer, PaOutputStreamer, PaDuplexStreamer >;
    class PortAudio : public SSB
    {
    private:
        PortAudio();
        PortAudio(const PortAudio& aud);
        const PortAudio& operator = (const PortAudio& aud);

    protected:
        bool Init() override;
        void Close() override;

        //sound group members
        soundgroup_t NewSoundGroup() override;
        void RemoveSoundGroup(soundgroup_t sndgrp) override;

        //input members
        inputstreamer_t NewStream(StreamCapture* capture, int inputdeviceid, 
                                  int sndgrpid, int samplerate, int channels,
                                  int framesize) override;
        bool StartStream(inputstreamer_t streamer) override;
        void CloseStream(inputstreamer_t streamer) override;
        bool IsStreamStopped(inputstreamer_t streamer) override;

        //output members
        outputstreamer_t NewStream(StreamPlayer* player, int outputdeviceid,
                                   int sndgrpid, int samplerate, int channels, 
                                   int framesize) override;
        void CloseStream(outputstreamer_t streamer) override;

        bool StartStream(outputstreamer_t streamer) override;
        bool StopStream(outputstreamer_t streamer) override;
        bool IsStreamStopped(outputstreamer_t streamer) override;

        //duplex members
        duplexstreamer_t NewStream(StreamDuplex* duplex, int inputdeviceid,
                                   int outputdeviceid, int sndgrpid,
                                   int samplerate, int input_channels, 
                                   int output_channels, int framesize) override;
        bool StartStream(duplexstreamer_t streamer) override;
        void CloseStream(duplexstreamer_t streamer) override;
        bool IsStreamStopped(duplexstreamer_t streamer) override;
        bool UpdateStreamDuplexFeatures(duplexstreamer_t streamer) override;

    public:
        ~PortAudio() override;
        static std::shared_ptr<PortAudio> GetInstance();

        bool SetAutoPositioning(int sndgrpid, bool enable) override;
        bool IsAutoPositioning(int sndgrpid) override;
        bool AutoPositionPlayers(int sndgrpid, bool all_players) override;//bAll = true => Also those with autoposition disabled

        //input and output devices
        bool GetDefaultDevices(int& inputdeviceid,
                               int& outputdeviceid) override;
        bool GetDefaultDevices(SoundAPI sndsys,
                               int& inputdeviceid,
                               int& outputdeviceid) override;

        void SetAutoPositioning(StreamPlayer* player, bool enable) override;
        bool IsAutoPositioning(StreamPlayer* player) override;

        bool SetPosition(StreamPlayer* player, float x, float y, float z) override;
        bool GetPosition(StreamPlayer* player, float& x, float& y, float& z) override;

    private:

        void FillDevices(sounddevices_t& sounddevs) override;
        static void SetupDeviceFeatures(const PaDeviceInfo* devinfo, soundsystem::DeviceInfo& device);
        static void FillSampleFormats(const PaDeviceInfo* devinfo, soundsystem::DeviceInfo& device);
        void SetupDefaultCommunicationDevice(sounddevices_t& sounddevs);
        static SoundAPI GetSoundSystem(const PaDeviceInfo* devinfo);    //see if device is DSound, Alsa, etc.
    };

    using soundgroup_t = SSB::soundgroup_t;
    using inputstreamer_t = SSB::inputstreamer_t;
    using outputstreamer_t = SSB::outputstreamer_t;
    using duplexstreamer_t = SSB::duplexstreamer_t;


#if defined(WIN32)
    class CWMAudioAECCapture
    {
        PaDuplexStreamer* m_streamer;
        std::shared_ptr<std::thread> m_callback_thread;

        typedef std::map<ACE_TString, UINT> mapsndid_t;

        bool FindDevs(LONG& indevindex, LONG& outdevindex);
        void Run();
        void ProcessAudioQueue();
        void QueueAudioFrame(const media::AudioFrame& frm);
        // true means audio frame was consumed, false means audio frame
        // was queued
        bool ProcessAudioFrame(const media::AudioFrame& frm);

        // signaling semaphores
        std::promise<bool> m_started, m_stop;

        size_t m_input_index = 0;
        std::vector<short> m_input_buffer; // audio from input device
        msg_queue_t m_input_queue; // audio from input device that didn't fit in 'm_input_buffer'
        audio_resampler_t m_resampler;
        short* m_resampled_input = nullptr;
        std::recursive_mutex m_mutex;
        SoundDeviceFeatures m_features;
    public:
        CWMAudioAECCapture(PaDuplexStreamer* duplex, SoundDeviceFeatures features);
        ~CWMAudioAECCapture();
        bool Open();
        short* AcquireBuffer();
        void ReleaseBuffer();
        SoundDeviceFeatures GetFeatures() const { return m_features; }
    };
#endif /* WIN32 */

} // namespace soundsystem

#endif
