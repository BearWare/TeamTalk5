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

#if !defined(SOUNDSYSTEMEX_H)
#define SOUNDSYSTEMEX_H

#include "SoundSystem.h"

#include "codec/MediaUtil.h"
#include "mystd/MyStd.h"
#include "myace/MyACE.h"

#include <ace/Reactor.h>
#include <ace/Task.h>
#include <ace/Time_Value.h>

#include <cassert>
#include <cstdint>
#include <vector>

namespace soundsystem {

    void SoftVolume(const OutputStreamer& streamer, short* buffer, int samples, int mastervol, bool mastermute);
    void DuplexCallback(DuplexStreamer& dpxStream, const short* recorded, short* playback, int mastervol, bool mastermute);
    void MuxPlayers(const std::vector<OutputStreamer*>& players, short* tmp_buffer, short* playback, int mastervol, bool mastermute);
    void DuplexEnded(SoundSystem* sndsys, DuplexStreamer& dpxStream);

    // audio input and output streamers for SOUND_DEVICEID_VIRTUAL

    class StreamCaller : public ACE_Task_Base
    {
        std::vector<short> m_buffer;
        uint32_t m_start, m_interval;
        ACE_Reactor m_reactor;

    protected:
        // Derived classes with member variables must call this.
        // Otherwise StreamCallback() maybe be called from timer
        // on a destroyed object
        void Stop()
        {
            m_interval = 0;
            int ret = m_reactor.end_reactor_event_loop();
            assert(ret >= 0);
            ret = wait();
            assert(ret >= 0);
        }
    public:
        StreamCaller(const SoundStreamer& streamer, int channels) : m_interval(PCM16_SAMPLES_DURATION(streamer.framesize, streamer.samplerate)), m_start(GETTIMESTAMP())
        {
            m_buffer.resize(channels * streamer.framesize, 0);            
        }

        ~StreamCaller() override = default;

        int handle_timeout(const ACE_Time_Value&  /*tv*/, const void*  /*arg*/) override
        {
            while (m_interval != 0 && W32_LEQ(m_start + m_interval, GETTIMESTAMP()))
            {
                StreamCallback(m_buffer.data());
                m_start += m_interval;
            }

            return 0;
        }
        int svc() override
        {
            m_reactor.owner (ACE_OS::thr_self ());
            ACE_Time_Value const zero;
            auto tv = ToTimeValue(m_interval);
            int const ret = m_reactor.schedule_timer(this, nullptr, zero, tv);
            MYTRACE_COND(ret < 0, ACE_TEXT("Failed to schedule timer in StreamCaller\n"));
            m_reactor.run_reactor_event_loop();
            return 0;
        }
        virtual bool StreamCallback(short* buffer) = 0;
    };

    class StreamCaptureCallback : public StreamCaller
    {
        InputStreamer* m_streamer;

    public:
        StreamCaptureCallback(InputStreamer* streamer)
            : StreamCaller(*streamer, streamer->channels)
            , m_streamer(streamer)
        {
        }

        ~StreamCaptureCallback() override
        {
            Stop();
        }

        bool StreamCallback(short* buffer) override
        {
            m_streamer->recorder->StreamCaptureCb(*m_streamer,
                                                  buffer,
                                                  m_streamer->framesize);
            return true;
        }
    };

    class StreamPlayerCallback : public StreamCaller
    {
        OutputStreamer* m_streamer;

    public:
        StreamPlayerCallback(OutputStreamer* streamer)
            : StreamCaller(*streamer, streamer->channels)
            , m_streamer(streamer)
        {
        }

        ~StreamPlayerCallback() override
        {
            Stop();
        }

        bool StreamCallback(short* buffer) override
        {
            m_streamer->player->StreamPlayerCb(*m_streamer, buffer, m_streamer->framesize);
            return true;
        }

    };

    class StreamDuplexCallback : public StreamCaller
    {
        SoundSystem* m_sndsys;
        DuplexStreamer* m_streamer;
        std::vector<short> m_inputbuffer;

    public:
        StreamDuplexCallback(SoundSystem* sndsys, DuplexStreamer* streamer)
            : StreamCaller(*streamer, streamer->output_channels)
            , m_sndsys(sndsys)
            , m_streamer(streamer)
        {
            m_inputbuffer.resize(streamer->input_channels * streamer->framesize, 0);
        }

        ~StreamDuplexCallback() override
        {
            Stop();
        }

        bool StreamCallback(short* buffer) override
        {
            int const mastervol = m_sndsys->GetMasterVolume(m_streamer->sndgrpid);
            bool const mastermute = m_sndsys->IsAllMute(m_streamer->sndgrpid);
            DuplexCallback(*m_streamer, m_inputbuffer.data(), buffer, mastervol, mastermute);
            return true;
        }
    };
} // namespace soundsystem

#endif
