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

#include <portaudio.h>

#include <myace/MyACE.h>

#include <vector>
#include <map>
#include <future>
#include <assert.h>

namespace soundsystem
{

    struct PaStreamer
    {
        PaStream* stream;
        PaStreamer()
        {
            stream = nullptr;
        }
        virtual ~PaStreamer()
        {
            MYTRACE(ACE_TEXT("~PaStreamer()\n"));
        }
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
        bool initialcallback = true;
        uint32_t playedsamples_msec = 0;
        uint32_t starttime = 0;
#if defined(WIN32)
        std::shared_ptr<class CWMAudioAECCapture> winaec;
        uint32_t echosamples_msec = 0;
#endif
        PaDuplexStreamer(StreamDuplex* d, int sg, int fs, int sr, int inchs, int outchs,
                         SoundAPI out_sndsys, int inputdeviceid, int outputdeviceid)
            : DuplexStreamer(d, sg, fs, sr, inchs, outchs, out_sndsys, inputdeviceid, outputdeviceid)
        { }
        ~PaDuplexStreamer()
        {
        }
    };

    struct PaSoundGroup : SoundGroup
    {
        bool autoposition = false;;
        PaSoundGroup() { }
    };

    typedef SoundSystemBase < PaSoundGroup, PaInputStreamer, PaOutputStreamer, PaDuplexStreamer > SSB;
    class PortAudio : public SSB
    {
    private:
        PortAudio();
        PortAudio(const PortAudio& aud);
        const PortAudio& operator = (const PortAudio& aud);

    protected:
        bool Init();
        void Close();

        //sound group members
        soundgroup_t NewSoundGroup();
        void RemoveSoundGroup(soundgroup_t sndgrp);

        //input members
        inputstreamer_t NewStream(StreamCapture* capture, int inputdeviceid, 
                                  int sndgrpid, int samplerate, int channels,
                                  int framesize);
        bool StartStream(inputstreamer_t streamer);
        void CloseStream(inputstreamer_t streamer);
        bool IsStreamStopped(inputstreamer_t streamer);

        //output members
        outputstreamer_t NewStream(StreamPlayer* player, int outputdeviceid,
                                   int sndgrpid, int samplerate, int channels, 
                                   int framesize);
        void CloseStream(outputstreamer_t streamer);

        bool StartStream(outputstreamer_t streamer);
        bool StopStream(outputstreamer_t streamer);
        bool IsStreamStopped(outputstreamer_t streamer);

        //duplex members
        duplexstreamer_t NewStream(StreamDuplex* duplex, int inputdeviceid,
                                   int outputdeviceid, int sndgrpid,
                                   int samplerate, int input_channels, 
                                   int output_channels, int framesize);
        bool StartStream(duplexstreamer_t streamer);
        void CloseStream(duplexstreamer_t streamer);
        bool IsStreamStopped(duplexstreamer_t streamer);
        bool UpdateStreamDuplexFeatures(duplexstreamer_t streamer);

    public:
        virtual ~PortAudio();
        static std::shared_ptr<PortAudio> getInstance();

        bool SetAutoPositioning(int sndgrpid, bool enable);
        bool IsAutoPositioning(int sndgrpid);
        bool AutoPositionPlayers(int sndgrpid, bool all_players);//bAll = true => Also those with autoposition disabled

        //input and output devices
        bool GetDefaultDevices(int& inputdeviceid,
                               int& outputdeviceid);
        bool GetDefaultDevices(SoundAPI sndsys,
                               int& inputdeviceid,
                               int& outputdeviceid);

        void SetSampleRate(StreamPlayer* player, int samplerate);
        int GetSampleRate(StreamPlayer* player);

        void SetAutoPositioning(StreamPlayer* player, bool enable);
        bool IsAutoPositioning(StreamPlayer* player);

        bool SetPosition(StreamPlayer* player, float x, float y, float z);
        bool GetPosition(StreamPlayer* player, float& x, float& y, float& z);

    private:

        void FillDevices(sounddevices_t& sounddevs);
        SoundAPI GetSoundSystem(const PaDeviceInfo* devinfo);    //see if device is DSound, Alsa, etc.
    };

    typedef SSB::soundgroup_t soundgroup_t;
    typedef SSB::inputstreamer_t inputstreamer_t;
    typedef SSB::outputstreamer_t outputstreamer_t;
    typedef SSB::duplexstreamer_t duplexstreamer_t;


#if defined(WIN32)
    class CWMAudioAECCapture
    {
        PaDuplexStreamer* m_streamer;
        std::shared_ptr<std::thread> m_callback_thread;

        typedef std::map<ACE_TString, UINT> mapsndid_t;

        bool FindDevs(LONG& indevindex, LONG& outdevindex);
        void Run();
        void ProcessAudioQueue();
        // true means audio frame was consumed, false means audio frame
        // was queued
        bool QueueAudioInput(const media::AudioFrame& frm);

        // signaling semaphores
        std::promise<bool> m_started, m_stop;

        size_t m_input_index = 0;
        std::vector<short> m_input_buffer; // audio from input device
        msg_queue_t m_input_queue; // audio from input device that didn't fit in 'm_input_buffer'
        audio_resampler_t m_resampler;
        short* m_resampled_input = nullptr;
        std::mutex m_mutex;
        SoundDeviceFeatures m_features;
    public:
        CWMAudioAECCapture(PaDuplexStreamer* duplex, SoundDeviceFeatures features);
        ~CWMAudioAECCapture();
        bool Open();
        short* AcquireBuffer();
        void ReleaseBuffer();
        SoundDeviceFeatures GetFeatures() const { return m_features; }
    };
#endif
}

#endif
