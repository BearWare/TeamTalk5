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

#if !defined(SOUNDSYSTEMBASE_H)
#define SOUNDSYSTEMBASE_H

#include "SoundSystem.h"

#include "AudioResampler.h"

#include <codec/MediaUtil.h>

#include <memory>
#include <thread>
#include <mutex>
#include <cstring>

namespace soundsystem {

    void SoftVolume(SoundSystem* sndsys, const OutputStreamer& streamer, short* buffer, int samples);
    void DuplexCallback(SoundSystem* sndsys, DuplexStreamer& dpxStream, const short* recorded, short* playback);
    void MuxPlayers(SoundSystem* sndsys, const std::vector<OutputStreamer*>& players, short* tmp_buffer, short* playback);
    void DuplexEnded(SoundSystem* sndsys, DuplexStreamer& dpxStream);

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
        StreamCaller(const SoundStreamer& streamer, int channels)
        {
            m_buffer.resize(channels * streamer.framesize, 0);
            m_interval = streamer.framesize * 1000 / streamer.samplerate;
            m_start = GETTIMESTAMP();
        }

        virtual ~StreamCaller()
        {
        }

        int handle_timeout(const ACE_Time_Value& tv, const void* arg)
        {
            while (W32_LEQ(m_start, GETTIMESTAMP()) && m_interval != 0)
            {
                StreamCallback(&m_buffer[0]);
                m_start += m_interval;
            }

            return 0;
        }
        int svc()
        {
            m_reactor.owner (ACE_OS::thr_self ());
            auto tv = ToTimeValue(m_interval);
            int ret = m_reactor.schedule_timer(this, 0, tv, tv);
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

        virtual ~StreamCaptureCallback()
        {
            Stop();
        }

        bool StreamCallback(short* buffer)
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

        virtual ~StreamPlayerCallback()
        {
            Stop();
        }

        bool StreamCallback(short* buffer)
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

        virtual ~StreamDuplexCallback()
        {
            Stop();
        }

        bool StreamCallback(short* buffer)
        {
            DuplexCallback(m_sndsys, *m_streamer, &m_inputbuffer[0], buffer);
            return true;
        }
    };

/* SharedStreamCapture::MakeKey() has limited space */
#define MAX_SAMPLERATES       16
#define MAX_CHANNELS          2
#define MAX_FRAMESIZE         ((1 << 27) - 1)

#define DEBUG_RESAMPLER 0

    template < typename INPUTSTREAMER >
    class SharedStreamCapture : public StreamCapture
    {
        std::vector<int> m_keysamplerates;

        uint32_t MakeKey(const InputStreamer& streamer)
        {
            assert(m_keysamplerates.size());
            assert(streamer.channels <= 2);
            auto i = std::find(m_keysamplerates.begin(), m_keysamplerates.end(), streamer.samplerate);
            assert(i != m_keysamplerates.end());
            auto srindex = i - m_keysamplerates.begin();

            uint32_t key = (streamer.channels - 1) << 31;
            key |= (srindex << 27);
            assert((1 << 27) > streamer.framesize);
            key |= streamer.framesize;
            return key;
        }

        int GetFrameSizeFromKey(uint32_t key) const
        {
            return key & MAX_FRAMESIZE;
        }

        int GetSampleRateFromKey(uint32_t key) const
        {
            return m_keysamplerates[(key & 0x7FFFFFFF) >> 27];
        }

        int GetChannelsFromKey(uint32_t key) const
        {
            return ((1<<31) & key) == 0? 1 : 2;
        }

        bool SameStreamProperties(const InputStreamer& s1, const InputStreamer& s2)
        {
            return s1.channels == s2.channels &&
                s1.samplerate == s2.samplerate &&
                s1.framesize == s2.framesize;
        }

    public:
        typedef std::shared_ptr < INPUTSTREAMER > inputstreamer_t;

        SharedStreamCapture() { }
        ~SharedStreamCapture()
        {
            assert(m_sndgrpid == 0);
            assert(m_inputstreams.empty());
            assert(m_activestreams.empty());
            assert(m_resample_buffers.empty());
            assert(m_callback_buffers.empty());

            m_samples_queue.close();
            if (m_resample_thread)
                m_resample_thread->join();

            MYTRACE(ACE_TEXT("~SharedCaptureStream()\n"));
        }

        // Set the source input stream which feeds the shared input streams
        void SetOrigin(inputstreamer_t streamer, int sndgrpid)
        {
            m_originalstream = streamer;
            m_sndgrpid = sndgrpid;

            int buffersize = PCM16_BYTES(streamer->framesize, streamer->channels);
            buffersize *= 3; //TODO: what is a good number??
            m_samples_queue.low_water_mark(buffersize);
            m_samples_queue.high_water_mark(buffersize);
        }

        inputstreamer_t GetOrigin() const { return m_originalstream; }
        int ClearSoundGroup()
        {
            int sndgrp = m_sndgrpid;
            m_sndgrpid = 0;
            return sndgrp;
        }

        // Add/remove/update shared input streams
        bool AddInputStreamer(inputstreamer_t streamer)
        {
            std::unique_lock<std::recursive_mutex> g(m_mutex);

            assert(streamer->inputdeviceid & SOUND_DEVICE_SHARED_FLAG);
            m_inputstreams.insert(streamer);

            if (std::find(m_keysamplerates.begin(), m_keysamplerates.end(), streamer->samplerate) == m_keysamplerates.end())
            {
                m_keysamplerates.push_back(streamer->samplerate);
                assert(m_keysamplerates.size() <= MAX_SAMPLERATES);
            }

            MYTRACE(ACE_TEXT("Number of active inputs on sound device #%d: %u\n"),
                    streamer->inputdeviceid & SOUND_DEVICEID_MASK, uint32_t(m_resamplers.size()));

            // create audio resampler if samplerate/framesize/channels
            // are different from original
            if (SameStreamProperties(*GetOrigin(), *streamer))
                return true;

            auto key = MakeKey(*streamer);
            if (m_resamplers.find(key) == m_resamplers.end())
            {
                media::AudioFormat infmt(m_originalstream->samplerate, m_originalstream->channels),
                    outfmt(streamer->samplerate, streamer->channels);
                m_resamplers[key] = MakeAudioResampler(infmt, outfmt);

                int resampleoutput = CalcSamples(m_originalstream->samplerate,
                                                 m_originalstream->framesize, streamer->samplerate);

                m_resample_buffers[key].resize(resampleoutput * streamer->channels);
                m_callback_buffers[key].resize(streamer->framesize * streamer->channels);

                if (!m_resample_thread)
                {
                    m_resample_thread.reset(new std::thread(&SharedStreamCapture<INPUTSTREAMER>::ResampleFunc, this));
                }
            }

            MYTRACE(ACE_TEXT("Number of active input resamplers on sound device #%d: %u\n"),
                    streamer->inputdeviceid & SOUND_DEVICEID_MASK, uint32_t(m_resamplers.size()));

            return true;
        }

        void RemoveInputStreamer(inputstreamer_t streamer)
        {
            std::unique_lock<std::recursive_mutex> g(m_mutex);
            m_activestreams.erase(streamer);
            m_inputstreams.erase(streamer);

            // if resampler is no longer needed then erase it
            auto key = MakeKey(*streamer);
            for (auto i : m_inputstreams)
            {
                if (MakeKey(*i) == key)
                    return; // it's still needed
            }

            m_resamplers.erase(key);
            m_resample_buffers.erase(key);
            m_callback_buffers.erase(key);
        }

        void ActivateInputStreamer(inputstreamer_t streamer, bool active)
        {
            std::lock_guard<std::recursive_mutex> g(m_mutex);
            assert(m_inputstreams.find(streamer) != m_inputstreams.end());

            if (active)
                m_activestreams.insert(streamer);
            else
                m_activestreams.erase(streamer);
        }

        bool InputStreamsExists()
        {
            std::lock_guard<std::recursive_mutex> g(m_mutex);
            return !m_inputstreams.empty();
        }

        bool ActiveStreamsExists()
        {
            std::lock_guard<std::recursive_mutex> g(m_mutex);
            return !m_activestreams.empty();
        }

        void StreamCaptureCb(const InputStreamer& streamer,
                             const short* buffer, int samples)
        {
            assert((streamer.inputdeviceid & SOUND_DEVICE_SHARED_FLAG) == 0);

            std::lock_guard<std::recursive_mutex> g(m_mutex);

#if DEBUG_RESAMPLER
            MYTRACE("Original for %p samplerate %d, framesize %d, channels %d\n",
                    streamer.recorder, streamer.samplerate,
                    streamer.framesize, streamer.channels);
#endif

            bool resample = false;
            for (auto stream : m_activestreams)
            {
                if (SameStreamProperties(*stream, streamer))
                {
                    stream->recorder->StreamCaptureCb(*stream, buffer, samples);
#if DEBUG_RESAMPLER
                    MYTRACE("Shared for %p samplerate %d, framesize %d, channels %d\n",
                            stream->recorder, stream->samplerate,
                            stream->framesize, stream->channels);
#endif
                }
                else
                {
                    resample = true;
                }
            }

            if (resample)
            {
                ACE_Message_Block* mb;
                int size = PCM16_BYTES(samples, streamer.channels);
                ACE_NEW(mb, ACE_Message_Block(size));
                if (mb->copy(reinterpret_cast<const char*>(buffer), size) < 0)
                    mb->release();
                else
                {
                    ACE_Time_Value tm = ACE_Time_Value::zero;
                    if (m_samples_queue.enqueue(mb, &tm) < 0)
                    {
                        mb->release();
                    }
                }
            }
        }

        void ResampleFunc()
        {
            ACE_Message_Block* mb;
            while (m_samples_queue.dequeue(mb) >= 0)
            {
                MBGuard gmb(mb);
                std::lock_guard<std::recursive_mutex> g(m_mutex);

                assert(mb->length() == PCM16_BYTES(m_originalstream->framesize, m_originalstream->channels));
                for (auto i : m_resamplers)
                {
                    auto key = i.first;
                    int cbsr = GetSampleRateFromKey(key);
                    int cbch = GetChannelsFromKey(key);
                    int cbframesize = GetFrameSizeFromKey(key);
                    short* cbbufptr = &m_callback_buffers[key][0];

                    auto rsbuf = m_resample_buffers.find(key);
                    assert(rsbuf != m_resample_buffers.end());
                    short* rsbufptr = &rsbuf->second[0];
                    assert(cbch);
                    int rsframesize = int(rsbuf->second.size()) / cbch;
                    int samples = i.second->Resample(reinterpret_cast<const short*>(mb->rd_ptr()),
                                                     m_originalstream->framesize,
                                                     rsbufptr, rsframesize);
#if DEBUG_RESAMPLER
                    MYTRACE("Resampled for samplerate %d, framesize %d, channels %d\n",
                            cbsr, cbframesize, cbch);
#endif

                    MYTRACE_COND(samples != rsframesize,
                                 ACE_TEXT("Resampled output frame for samplerate %d, channels %d doesn't match framesize %d. Was %d\n"),
                                 cbsr, cbch, rsframesize, samples);

                    // Now copy samples from m_resample_buffer[key] to
                    // m_callback_buffer[key], i.e. from original
                    // capture stream to shared capture stream.
                    //
                    // Here we want to use "total" samples where
                    // channel information (mono/stereo) is omitted.
                    int totalsamples = rsframesize * cbch;
                    int rspos = 0;
                    while (rspos < totalsamples)
                    {
                        // space in callback
                        std::size_t cbpos = m_callback_index[key];
                        std::size_t cbbufspace = m_callback_buffers[key].size() - cbpos;
                        assert(int(cbpos) >= 0 && cbpos < m_callback_buffers[key].size());

                        // calc samples to copy
                        std::size_t rsremain = totalsamples - rspos;
                        std::size_t n_samples = std::min(cbbufspace, rsremain);

                        //where to copy from
#if DEBUG_RESAMPLER
                        MYTRACE("Copying at cbpos %d, rspos %u for samplerate %d, framesize %d, channels %d\n",
                                int(cbpos), rspos, cbsr, cbframesize, cbch);
#endif
                        assert(rspos + n_samples <= m_resample_buffers[key].size());
                        assert(cbpos + n_samples <= m_callback_buffers[key].size());

                        std::size_t bytes = n_samples * sizeof(cbbufptr[0]);
                        std::memcpy(&cbbufptr[cbpos], &rsbufptr[rspos], bytes);

                        cbpos += n_samples;
                        rspos += int(n_samples);

                        if (cbpos == m_callback_buffers[key].size())
                        {
                            for (auto streamer : m_activestreams)
                            {
                                if (MakeKey(*streamer) == key)
                                {
                                    streamer->recorder->StreamCaptureCb(*streamer, cbbufptr, cbframesize);
#if DEBUG_RESAMPLER
                                    MYTRACE("Callback for %p samplerate %d, framesize %d, channels %d\n",
                                            streamer->recorder, cbsr, cbframesize, cbch);
#endif
                                }
                            }
                            cbpos = 0;
                        }

                        m_callback_index[key] = cbpos;
                    }
                } // for each resampler
            } // while dequeue
        }

    private:
        int m_sndgrpid = 0;
        msg_queue_t m_samples_queue;
        inputstreamer_t m_originalstream;

        std::set<inputstreamer_t> m_inputstreams, m_activestreams;
        // MakeKey() -> resampler
        std::map<uint32_t, audio_resampler_t> m_resamplers;
        // MakeKey() -> resample and callback buffer, i.e. temp
        // storage of resampled audio and the callback buffer for the
        // StreamCapture-class. It cannot be the same because
        // framesize might be different
        std::map<uint32_t, std::vector<short>> m_resample_buffers, m_callback_buffers;
        std::map<uint32_t, std::size_t> m_callback_index;
        // One thread to perform resampling of all active input streams
        std::shared_ptr<std::thread> m_resample_thread;
        // m_mutex for 'm_inputstreams, m_activestreams',
        // 'm_resamplers, m_resample_buffers, m_callback_buffers,
        // m_callback_index'
        std::recursive_mutex m_mutex;
    };

#define DEBUG_SHAREDPLAYER 0
    
    template < typename OUTPUTSTREAMER >
    class SharedStreamPlayer : public StreamPlayer
    {
        bool SameStreamProperties(const OutputStreamer& os1, const OutputStreamer& os2) const
        {
            return os1.samplerate == os2.samplerate && os1.channels == os2.channels &&
                os1.framesize == os2.framesize;
        }

    public:
        typedef std::shared_ptr < OUTPUTSTREAMER > outputstreamer_t;

        SharedStreamPlayer(SoundSystem* sndsys)
        : m_sndsys(sndsys)
        {
        }

        ~SharedStreamPlayer()
        {
            MYTRACE(ACE_TEXT("~SharedStreamPlayer() - %p\n"), this);
        }
        
        bool StreamPlayerCb(const OutputStreamer& streamer,
                            short* buffer, int samples)
        {
            assert(SameStreamProperties(*m_orgstream, streamer));

            const size_t reqbytes = PCM16_BYTES(streamer.framesize, streamer.channels);
            memset(buffer, 0, reqbytes);

            std::lock_guard<std::recursive_mutex> g(m_mutex);

            MYTRACE_COND(DEBUG_SHAREDPLAYER,
                         ACE_TEXT("--------------------- Mixer inputs %d -----------------------------\n"),
                         int(m_outputs.size()));
            
            for (auto i : m_outputs)
            {
                MYTRACE_COND(DEBUG_SHAREDPLAYER,
                             ACE_TEXT("Resample source: %d samples %d channels @ %d Hz duration %d msec. Destination %p: %d samples %d channels @ %d Hz duration %d msec\n"),
                             m_orgstream->framesize, m_orgstream->channels, m_orgstream->samplerate,
                             int(PCM16_SAMPLES_DURATION(m_orgstream->framesize, m_orgstream->samplerate)),
                             i.first, i.second->framesize, i.second->channels, i.second->samplerate,
                             int(PCM16_SAMPLES_DURATION(i.second->framesize, i.second->samplerate)));
                
                if (SameStreamProperties(*i.second, *m_orgstream))
                {
                    assert(i.second->framesize == samples);
                    i.first->StreamPlayerCb(*i.second, &m_tmpbuffer[0], samples);
                    SoftVolume(m_sndsys, *i.second, &m_tmpbuffer[0], samples);
                    MYTRACE_COND(DEBUG_SHAREDPLAYER, ACE_TEXT("Same stream properties. Destination: %p\n"), i.first);
                }
                else
                {
                    auto streamer_resam = i.second;
                    auto msgq = m_resambuffers[i.first];
                    auto resampler = m_resamplers[i.first];
                    short* input = &m_callbackbuffers[i.first][0];
                    int outputsamples = CalcSamples(streamer_resam->samplerate, streamer_resam->framesize, m_orgstream->samplerate);
                    const size_t outputbytes = PCM16_BYTES(outputsamples, m_orgstream->channels);

                    MYTRACE_COND(DEBUG_SHAREDPLAYER, ACE_TEXT("Duration of queue before: %d msec\n"),
                                 int(PCM16_BYTES_DURATION(msgq->message_length(), streamer.channels,
                                                          streamer.samplerate)));
                    
                    // fill up buffer with enough bytes to do a callback
                    while (msgq->message_length() < reqbytes)
                    {
                        i.first->StreamPlayerCb(*streamer_resam, input, streamer_resam->framesize);
                        SoftVolume(m_sndsys, *streamer_resam, input, streamer_resam->framesize);
                        short* output = resampler->Resample(input);

                        ACE_Message_Block* mb;
                        ACE_NEW_NORETURN(mb, ACE_Message_Block(outputbytes));
                        if (mb->copy(reinterpret_cast<const char*>(output), outputbytes) < 0)
                            mb->release();
                        else
                        {
                            ACE_Time_Value tv;
                            int ret = msgq->enqueue(mb, &tv);
                            assert(ret >= 0);
                            if (ret < 0)
                            {
                                msgq->close();
                                mb->release();
                            }
                        }
                    }

                    MYTRACE_COND(DEBUG_SHAREDPLAYER, ACE_TEXT("Duration of queue after refill: %d msec. Bytes: %u\n"),
                                 int(PCM16_BYTES_DURATION(msgq->message_length(), streamer.channels,
                                                          streamer.samplerate)), unsigned(msgq->message_length()));
                    

                    // copy buffer to callback
                    char* bytebuffer = reinterpret_cast<char*>(&m_tmpbuffer[0]);
                    size_t copied = 0;
                    while (copied < reqbytes)
                    {
                        ACE_Time_Value tv;
                        ACE_Message_Block* mb = nullptr;
                        int ret = msgq->dequeue(mb, &tv);
                        assert(ret >= 0);
                        if (ret < 0)
                        {
                            msgq->close();
                            break;
                        }

                        size_t copylimit = std::min(mb->length(), reqbytes - copied);
                        memcpy(bytebuffer + copied, mb->rd_ptr(), copylimit);
                        auto was = mb->rd_ptr();
                        mb->rd_ptr(copylimit);
                        assert(mb->rd_ptr() == was + copylimit);
                        copied += copylimit;

                        if (mb->length())
                        {
                            assert(copied == reqbytes);
                            int ret = msgq->enqueue_head(mb, &tv);
                            assert(ret >= 0);
                            if (ret < 0)
                            {
                                mb->release();
                                msgq->close();
                                break;
                            }
                        }
                        else
                        {
                            mb->release();
                        }
                    }
                    assert(copied == reqbytes);

                    MYTRACE_COND(DEBUG_SHAREDPLAYER, ACE_TEXT("Duration of queue after dequeue: %d msec\n"),
                                 int(PCM16_BYTES_DURATION(msgq->message_length(), streamer.channels,
                                                          streamer.samplerate)));
                    
                }

                // mix all active streams
                for (size_t i=0;i<m_tmpbuffer.size();++i)
                {
                    int val = m_tmpbuffer[i] + buffer[i];
                    if (val > 32767)
                        buffer[i] = 32767;
                    else if (val < -32768)
                        buffer[i] = -32768;
                    else
                        buffer[i] = short(val);
                }

            } // for-loop - streams

            return true;
        }

        void SetOrigin(outputstreamer_t streamer)
        {
            assert(!m_orgstream);
            m_orgstream = streamer;
            m_tmpbuffer.resize(streamer->channels * streamer->framesize);
        }

        outputstreamer_t GetOrigin()
        {
            return m_orgstream;
        }

        bool AddOutputStreamer(outputstreamer_t streamer, StreamPlayer* player)
        {
            std::lock_guard<std::recursive_mutex> g(m_mutex);

            assert(m_orgstream);

            assert(m_outputs.find(player) == m_outputs.end());

            m_outputs[player] = streamer;

            MYTRACE(ACE_TEXT("Number of active output streams from sound device #%d: %d\n"),
                    streamer->outputdeviceid & SOUND_DEVICEID_MASK, int(m_outputs.size()));

            if (SameStreamProperties(*m_orgstream, *streamer))
                return true;

            media::AudioFormat infmt(streamer->samplerate, streamer->channels),
                outfmt(m_orgstream->samplerate, m_orgstream->channels);
            auto resampler = MakeAudioResampler(infmt, outfmt, streamer->framesize);
            if (!resampler)
            {
                m_outputs.erase(player);
                return false;
            }

            m_resamplers[player] = resampler;
            m_callbackbuffers[player].resize(streamer->channels * streamer->framesize);
            m_resambuffers[player].reset(new ACE_Message_Queue< ACE_NULL_SYNCH >());
            m_resambuffers[player]->high_water_mark(1024*1024);
            m_resambuffers[player]->low_water_mark(1024*1024);
            return true;
        }

        void RemoveOutputStreamer(StreamPlayer* player)
        {
            std::lock_guard<std::recursive_mutex> g(m_mutex);

            m_outputs.erase(player);
            m_resamplers.erase(player);
        }

        bool Empty()
        {
            std::lock_guard<std::recursive_mutex> g(m_mutex);
            return m_outputs.empty();
        }

        bool Exists(StreamPlayer* player)
        {
            std::lock_guard<std::recursive_mutex> g(m_mutex);
            return m_outputs.find(player) != m_outputs.end();
        }

    private:
        typedef std::shared_ptr< ACE_Message_Queue< ACE_NULL_SYNCH > > msg_queue_t;

        SoundSystem* m_sndsys;
        outputstreamer_t m_orgstream;
        std::vector<short> m_tmpbuffer;
        std::map<StreamPlayer*, outputstreamer_t> m_outputs;
        std::map<StreamPlayer*, audio_resampler_t> m_resamplers;
        std::map<StreamPlayer*, std::vector<short>> m_callbackbuffers;
        std::map<StreamPlayer*, msg_queue_t> m_resambuffers;
        std::recursive_mutex m_mutex;
    };

    template < typename SOUNDGROUP, typename INPUTSTREAMER, typename OUTPUTSTREAMER, typename DUPLEXSTREAMER >
    class SoundSystemBase : public SoundSystem
    {
    public:
        typedef std::shared_ptr < SOUNDGROUP > soundgroup_t;
        typedef std::shared_ptr < INPUTSTREAMER > inputstreamer_t;
        typedef std::shared_ptr < OUTPUTSTREAMER > outputstreamer_t;
        typedef std::shared_ptr < DUPLEXSTREAMER > duplexstreamer_t;
        typedef std::shared_ptr < SharedStreamCapture < INPUTSTREAMER > > sharedstreamcapture_t;
        typedef std::shared_ptr < SharedStreamPlayer < OUTPUTSTREAMER > > sharedstreamplayer_t;

        //sndgrpid -> SoundGroup
        typedef std::map<int, soundgroup_t> soundgroups_t;
        typedef std::map<StreamCapture*, inputstreamer_t> inputstreamers_t;
        typedef std::map<StreamPlayer*, outputstreamer_t> outputstreamers_t;
        typedef std::map<StreamDuplex*, duplexstreamer_t> duplexstreamers_t;
        // SoundDeviceID -> SharedStreamCapture
        typedef std::map<int, sharedstreamcapture_t> sharedstreamcaptures_t;
        typedef std::map<int, sharedstreamplayer_t> sharedstreamplayers_t;

    protected:
        SoundSystemBase()
        {
            assert(standardSampleRates.size() <= MAX_SAMPLERATES);
        }
        virtual ~SoundSystemBase()
        {
            assert(m_virtual_streams.empty());
            assert(m_sndgrps.empty());
            assert(m_input_streamers.empty());
            assert(m_output_streamers.empty());
            assert(m_duplex_streamers.empty());
            assert(m_shared_streamcaptures.empty());
        }

        virtual bool Init() = 0;
        virtual void Close() = 0;

        typedef std::map<int, DeviceInfo> sounddevices_t;

        virtual void FillDevices(sounddevices_t& sounddevs) = 0;
        const std::vector<int> standardSampleRates = {8000, 12000, 16000, 24000, 32000, 44100, 48000};

        virtual soundgroup_t NewSoundGroup() = 0;
        virtual void RemoveSoundGroup(soundgroup_t grp) = 0;

        //sndgrp_lock() must be acquired before calling
        soundgroup_t GetSoundGroup(int sndgrpid)
        {
            std::lock_guard<std::recursive_mutex> g(sndgrp_lock());
            typename soundgroups_t::iterator i = m_sndgrps.find(sndgrpid);
            if(i != m_sndgrps.end())
                return i->second;
            return soundgroup_t();
        }

        std::vector<soundgroup_t> GetSoundGroups()
        {
            std::vector<soundgroup_t> result;

            std::lock_guard<std::recursive_mutex> g(sndgrp_lock());
            for (auto grp : m_sndgrps)
                result.push_back(grp.second);
            return result;
        }

        virtual inputstreamer_t NewStream(StreamCapture* capture,
                                          int inputdeviceid, int sndgrpid,
                                          int samplerate, int channels,
                                          int framesize) = 0;
        virtual bool StartStream(inputstreamer_t streamer) = 0;
        virtual void CloseStream(inputstreamer_t streamer) = 0;

        inputstreamer_t GetStream(StreamCapture* capture)
        {
            std::lock_guard<std::recursive_mutex> g(capture_lock());
            typename inputstreamers_t::iterator ii = m_input_streamers.find(capture);
            if(ii != m_input_streamers.end())
                return ii->second;
            return inputstreamer_t();
        }

        std::vector<StreamCapture*> GetRecorders(int sndgrpid)
        {
            std::lock_guard<std::recursive_mutex> g(capture_lock());
            std::vector<StreamCapture*> recorders;
            typename inputstreamers_t::const_iterator ite;
            for (ite=m_input_streamers.begin();ite!=m_input_streamers.end();ite++)
            {
                if(ite->second->sndgrpid == sndgrpid)
                    recorders.push_back(ite->first);
            }
            return recorders;
        }

        virtual outputstreamer_t NewStream(StreamPlayer* player, int outputdeviceid,
                                           int sndgrpid, int samplerate, int channels,
                                           int framesize) = 0;
        virtual void CloseStream(outputstreamer_t streamer) = 0;

        virtual bool StartStream(outputstreamer_t streamer) = 0;
        virtual bool StopStream(outputstreamer_t streamer) = 0;
        virtual bool IsStreamStopped(outputstreamer_t streamer) = 0;

        outputstreamer_t GetStream(StreamPlayer* player)
        {
            std::lock_guard<std::recursive_mutex> g(players_lock());
            typename outputstreamers_t::iterator ii = m_output_streamers.find(player);
            if(ii != m_output_streamers.end())
                return ii->second;
            return outputstreamer_t();
        }

        std::vector<StreamPlayer*> GetPlayers(int sndgrpid)
        {
            std::lock_guard<std::recursive_mutex> g(players_lock());
            std::vector<StreamPlayer*> players;
            typename outputstreamers_t::const_iterator ite;
            for(ite=m_output_streamers.begin();ite!=m_output_streamers.end();ite++)
            {
                if(ite->second->sndgrpid == sndgrpid)
                    players.push_back(ite->first);
            }
            return players;
        }

        virtual duplexstreamer_t NewStream(StreamDuplex* duplex, int inputdeviceid,
                                           int outputdeviceid, int sndgrpid,
                                           int samplerate, int input_channels,
                                           int output_channels, int framesize) = 0;
        virtual void CloseStream(duplexstreamer_t streamer) = 0;
        virtual bool StartStream(duplexstreamer_t streamer) = 0;
        duplexstreamer_t GetStream(StreamDuplex* duplex)
        {
            std::lock_guard<std::recursive_mutex> g(duplex_lock());
            typename duplexstreamers_t::iterator ii = m_duplex_streamers.find(duplex);
            if(ii != m_duplex_streamers.end())
                return ii->second;
            return duplexstreamer_t();
        }

        std::vector<StreamDuplex*> GetDuplexers(int sndgrpid)
        {
            std::lock_guard<std::recursive_mutex> g(duplex_lock());
            std::vector<StreamDuplex*> duplexers;
            typename duplexstreamers_t::const_iterator ite;
            for (ite=m_duplex_streamers.begin();ite!=m_duplex_streamers.end();ite++)
            {
                if(ite->second->sndgrpid == sndgrpid)
                    duplexers.push_back(ite->first);
            }
            return duplexers;
        }

    private:

        soundgroups_t m_sndgrps; // sndgrp_lock()
        inputstreamers_t m_input_streamers; // capture_lock()
        outputstreamers_t m_output_streamers; //players_lock()
        duplexstreamers_t m_duplex_streamers; // duplex_lock()
        sharedstreamcaptures_t m_shared_streamcaptures; // capture_lock()
        sharedstreamplayers_t m_shared_streamplayers; //players_lock()

        std::recursive_mutex& sndgrp_lock() { return m_sndgrp_lock; }
        std::recursive_mutex& capture_lock() { return m_cap_lock; }
        std::recursive_mutex& players_lock() { return m_play_lock; }
        std::recursive_mutex& duplex_lock() { return m_dpx_lock; }

        std::recursive_mutex m_sndgrp_lock, m_cap_lock, m_play_lock, m_dpx_lock;

        inputstreamer_t NewVirtualStream(StreamCapture* capture, int sndgrpid,
                                         int samplerate, int channels,
                                         int framesize)
        {
            DeviceInfo dev;
            bool b = GetDevice(SOUND_DEVICEID_VIRTUAL, dev);
            assert(b);
            inputstreamer_t streamer(new INPUTSTREAMER(capture, sndgrpid,
                                                       framesize, samplerate,
                                                       channels, dev.soundsystem,
                                                       SOUND_DEVICEID_VIRTUAL));
            return streamer;
        }

        inputstreamer_t NewSharedStream(StreamCapture* capture,
                                        int inputdeviceid, int sndgrpid,
                                        int samplerate, int channels,
                                        int framesize)
        {
            DeviceInfo snddev;
            if (!GetDevice(inputdeviceid, snddev))
                return inputstreamer_t();

            std::unique_lock<std::recursive_mutex> g(capture_lock());

            inputstreamer_t streamer = inputstreamer_t(new INPUTSTREAMER(capture,
                                                                         sndgrpid,
                                                                         framesize,
                                                                         samplerate,
                                                                         channels,
                                                                         snddev.soundsystem,
                                                                         inputdeviceid));

            // check if shared recording device already exists
            if (m_shared_streamcaptures.find(inputdeviceid) != m_shared_streamcaptures.end())
            {
                // shared device already exists, just add new input stream to
                // existing listeners

                m_shared_streamcaptures[inputdeviceid]->AddInputStreamer(streamer);

                MYTRACE(ACE_TEXT("Added shared capture stream %p, samplerate %d, channels %d\n"),
                        capture, samplerate, channels);

                return streamer;
            }

            // shared device does not exist, create as new original stream
            sharedstreamcapture_t sharedstream;
            sharedstream.reset(new SharedStreamCapture<INPUTSTREAMER>());

            // create new sound group. We cannot use 'sndgrpid' since
            // that instance might be deleted and the shared stream
            // would stop.
            int newsndgrpid = OpenSoundGroup();
            if (!newsndgrpid)
                return inputstreamer_t();

            inputstreamer_t orgstream;
            if (snddev.id == SOUND_DEVICEID_VIRTUAL)
                orgstream = NewVirtualStream(sharedstream.get(),
                                             newsndgrpid,
                                             snddev.default_samplerate,
                                             snddev.max_input_channels,
                                             int(snddev.default_samplerate * 0.04));
            else
                orgstream = NewStream(sharedstream.get(),
                                      snddev.id & SOUND_DEVICEID_MASK,
                                      newsndgrpid,
                                      snddev.default_samplerate,
                                      snddev.max_input_channels,
                                      int(snddev.default_samplerate * 0.04));

            if (!orgstream)
            {
                return inputstreamer_t();
            }

            // store shared stream 'original' input
            sharedstream->SetOrigin(orgstream, newsndgrpid);
            // add input stream to set of listeners
            sharedstream->AddInputStreamer(streamer);

            // insert into container of shared input devices
            m_shared_streamcaptures[inputdeviceid] = sharedstream;

            // don't hold lock during callback
            g.unlock();

            if (snddev.id == SOUND_DEVICEID_VIRTUAL)
            {
                StartVirtualStream(orgstream);
            }
            else if (!StartStream(orgstream))
            {
                // failure, erase again
                g.lock();
                sharedstream->RemoveInputStreamer(streamer);
                m_shared_streamcaptures.erase(inputdeviceid);
                return inputstreamer_t();
            }

            MYTRACE(ACE_TEXT("Opened shared capture stream %p, samplerate %d, channels %d\n"),
                    capture, samplerate, channels);

            return streamer;
        }

        outputstreamer_t NewVirtualStream(StreamPlayer* player, int sndgrpid,
                                          int samplerate, int channels,
                                          int framesize)
        {
            DeviceInfo dev;
            bool b = GetDevice(SOUND_DEVICEID_VIRTUAL, dev);
            assert(b);
            outputstreamer_t streamer(new OUTPUTSTREAMER(player, sndgrpid,
                                                         framesize, samplerate,
                                                         channels, dev.soundsystem,
                                                         SOUND_DEVICEID_VIRTUAL));
            return streamer;
        }

        outputstreamer_t NewSharedStream(StreamPlayer* player, int outputdeviceid,
                                         int sndgrpid, int samplerate,
                                         int channels, int framesize)
        {
            DeviceInfo snddev;
            if (!GetDevice(outputdeviceid, snddev))
                return outputstreamer_t();

            std::unique_lock<std::recursive_mutex> g(players_lock());

            outputstreamer_t streamer = outputstreamer_t(new OUTPUTSTREAMER(player,
                                                                            sndgrpid,
                                                                            framesize,
                                                                            samplerate,
                                                                            channels,
                                                                            snddev.soundsystem,
                                                                            outputdeviceid));

            if (m_shared_streamplayers.find(outputdeviceid) != m_shared_streamplayers.end())
            {
                return streamer;
            }

            sharedstreamplayer_t sharedstream;
            sharedstream.reset(new SharedStreamPlayer<OUTPUTSTREAMER>(this));

            int newsndgrpid = OpenSoundGroup();
            if (!newsndgrpid)
                return outputstreamer_t();

            // store in container so others will not try to create
            // another shared stream as well on 'outputdeviceid'
            m_shared_streamplayers[outputdeviceid] = sharedstream;
            
            outputstreamer_t orgstream;
            if (snddev.id == SOUND_DEVICEID_VIRTUAL)
                orgstream = NewVirtualStream(sharedstream.get(), newsndgrpid,
                                             snddev.default_samplerate,
                                             snddev.max_output_channels,
                                             int(snddev.default_samplerate * 0.04));
            else
                orgstream = NewStream(sharedstream.get(), snddev.id & SOUND_DEVICEID_MASK,
                                      newsndgrpid, snddev.default_samplerate,
                                      snddev.max_output_channels, int(snddev.default_samplerate * 0.04));

            if (!orgstream)
            {
                RemoveSoundGroup(newsndgrpid);

                m_shared_streamplayers.erase(outputdeviceid);
                return outputstreamer_t();
            }

            sharedstream->SetOrigin(orgstream);

            // a hack to get new player into container, otherwise we
            // cannot start it
            m_output_streamers[sharedstream.get()] = orgstream;

            g.unlock();

            if (!StartStream(sharedstream.get()))
            {
                RemoveSoundGroup(newsndgrpid);

                g.lock();
                m_shared_streamplayers.erase(outputdeviceid);
                return outputstreamer_t();
            }

            return streamer;
        }

        duplexstreamer_t NewVirtualStream(StreamDuplex* duplex, int sndgrpid,
                                          int samplerate, int input_channels,
                                          int output_channels, int framesize)
       {
            DeviceInfo dev;
            bool b = GetDevice(SOUND_DEVICEID_VIRTUAL, dev);
            assert(b);
            duplexstreamer_t streamer(new DUPLEXSTREAMER(duplex, sndgrpid,
                                                         framesize, samplerate,
                                                         input_channels,
                                                         output_channels,
                                                         dev.soundsystem,
                                                         SOUND_DEVICEID_VIRTUAL,
                                                         SOUND_DEVICEID_VIRTUAL));
            return streamer;
       }

        sounddevices_t m_sounddevs;
        std::recursive_mutex m_devs_lock;

        typedef std::shared_ptr <StreamCaller> streamcallback_t;
        typedef std::map<SoundStreamer*, streamcallback_t> streamcallbacks_t;
        streamcallbacks_t m_virtual_streams;
        std::recursive_mutex m_virtdev_lock; // lock for 'm_virtual_streams'

    public:
        int OpenSoundGroup()
        {
            soundgroup_t sg = NewSoundGroup();
            if(!sg)
                return 0;

            std::lock_guard<std::recursive_mutex> g(sndgrp_lock());
            int sndgrpid = int(m_sndgrps.size()+1);
            while(sndgrpid && m_sndgrps.find(sndgrpid) != m_sndgrps.end())
                sndgrpid++;

            m_sndgrps[sndgrpid] = sg;
            MYTRACE(ACE_TEXT("Active sound groups %d\n"), int(m_sndgrps.size()));
            return sndgrpid;
        }

        void RemoveSoundGroup(int sndgrpid)
        {
            assert(GetPlayers(sndgrpid).empty());
            assert(GetRecorders(sndgrpid).empty());
            assert(GetDuplexers(sndgrpid).empty());

            soundgroup_t sg = GetSoundGroup(sndgrpid);
            {
                std::lock_guard<std::recursive_mutex> g(sndgrp_lock());
                m_sndgrps.erase(sndgrpid);
            }

            if (sg)
                RemoveSoundGroup(sg);
        }

        bool OpenInputStream(StreamCapture* capture, int inputdeviceid,
                             int sndgrpid, int samplerate, int channels,
                             int framesize)
        {
            MYTRACE(ACE_TEXT("Opening StreamCapture %p, sample rate %d, channels %d, framesize %d, device #%d\n"),
                    capture, samplerate, channels, framesize, inputdeviceid);

            assert(channels <= MAX_CHANNELS);
            assert(framesize <= MAX_FRAMESIZE);

            inputstreamer_t streamer;
            if(inputdeviceid == SOUND_DEVICEID_VIRTUAL)
                streamer = NewVirtualStream(capture, sndgrpid,
                                            samplerate, channels, framesize);
            else if (inputdeviceid & SOUND_DEVICE_SHARED_FLAG)
            {
                streamer = NewSharedStream(capture, inputdeviceid, sndgrpid,
                                           samplerate, channels, framesize);
            }
            else
                streamer = NewStream(capture, inputdeviceid, sndgrpid,
                                     samplerate, channels, framesize);

            MYTRACE_COND(!streamer, ACE_TEXT("Failed to open input stream on device #%d\n"), inputdeviceid);

            if (!streamer)
                return false;

            {
                std::lock_guard<std::recursive_mutex> g(capture_lock());
                m_input_streamers[capture] = streamer;
            }

            if(streamer->IsVirtual())
            {
                StartVirtualStream(streamer);
                return true;
            }
            else if (streamer->IsShared())
            {
                std::lock_guard<std::recursive_mutex> g(capture_lock());

                sharedstreamcapture_t sharedstream = m_shared_streamcaptures[inputdeviceid];
                assert(sharedstream);
                sharedstream->ActivateInputStreamer(streamer, true);

                return true;
            }
            else
            {
                if(StartStream(streamer))
                    return true;
            }

            CloseInputStream(capture);
            return false;
        }

        bool CloseInputStream(StreamCapture* capture)
        {
            inputstreamer_t streamer = GetStream(capture);
            if (!streamer)
                return false;

            if(streamer->IsVirtual())
            {
                StopVirtualStream(streamer.get());
            }
            else if (streamer->IsShared())
            {
                std::unique_lock<std::recursive_mutex> g(capture_lock());
                assert(m_shared_streamcaptures.find(streamer->inputdeviceid) != m_shared_streamcaptures.end());
                auto sharedstream = m_shared_streamcaptures[streamer->inputdeviceid];
                sharedstream->RemoveInputStreamer(streamer);

                MYTRACE(ACE_TEXT("Closed shared capture stream %p\n"), streamer->recorder);

                if (!sharedstream->InputStreamsExists())
                {
                    int sndgrp = sharedstream->ClearSoundGroup();
                    m_shared_streamcaptures.erase(streamer->inputdeviceid);
                    g.unlock();

                    if (sharedstream->GetOrigin()->IsVirtual())
                        StopVirtualStream(sharedstream->GetOrigin().get());
                    else
                        CloseStream(sharedstream->GetOrigin());

                    RemoveSoundGroup(sndgrp);
                }
            }
            else
            {
                CloseStream(streamer);
            }

            MYTRACE(ACE_TEXT("Closed StreamCapture %p\n"), capture);

            std::lock_guard<std::recursive_mutex> g(capture_lock());
            m_input_streamers.erase(capture);
            return true;
        }

        bool OpenOutputStream(StreamPlayer* player, int outputdeviceid,
                              int sndgrpid, int samplerate, int channels,
                              int framesize)
        {
            MYTRACE(ACE_TEXT("Opening StreamPlayer %p, sample rate %d, channels %d, framesize %d, device #%d\n"),
                    player, samplerate, channels, framesize, outputdeviceid);

            outputstreamer_t streamer;
            if (outputdeviceid == SOUND_DEVICEID_VIRTUAL)
                streamer = NewVirtualStream(player, sndgrpid, samplerate,
                                            channels, framesize);
            else if (outputdeviceid & SOUND_DEVICE_SHARED_FLAG)
                streamer = NewSharedStream(player, outputdeviceid,
                                           sndgrpid, samplerate,
                                           channels, framesize);
            else
                streamer = NewStream(player, outputdeviceid, sndgrpid,
                                     samplerate, channels, framesize);
            if (!streamer)
                return false;

            std::lock_guard<std::recursive_mutex> g(players_lock());
            m_output_streamers[player] = streamer;

            return true;
        }
        bool CloseOutputStream(StreamPlayer* player)
        {
            outputstreamer_t streamer = GetStream(player);
            if (!streamer)
                return false;

            if (streamer->IsVirtual())
            {
                StopVirtualStream(streamer.get());
            }
            else if (streamer->IsShared())
            {
                std::unique_lock<std::recursive_mutex> g(players_lock());

                assert(m_shared_streamplayers.find(streamer->outputdeviceid) != m_shared_streamplayers.end());
                auto sharedstream = m_shared_streamplayers[streamer->outputdeviceid];
                sharedstream->RemoveOutputStreamer(player);
                if (sharedstream->Empty())
                {
                    m_shared_streamplayers.erase(streamer->outputdeviceid);
                    g.unlock();

                    if (sharedstream->GetOrigin()->IsVirtual())
                        StopVirtualStream(sharedstream->GetOrigin().get());
                    else
                        CloseStream(sharedstream->GetOrigin());

                    // hack to remove player, so sound group can be removed
                    g.lock();
                    m_output_streamers.erase(sharedstream.get());
                    g.unlock();

                    RemoveSoundGroup(sharedstream->GetOrigin()->sndgrpid);
                }
            }
            else
            {
                CloseStream(streamer);
            }

            std::lock_guard<std::recursive_mutex> g(players_lock());
            m_output_streamers.erase(player);

            MYTRACE(ACE_TEXT("Closed StreamPlayer %p\n"), player);

            return true;
        }
        bool StartStream(StreamPlayer* player)
        {
            outputstreamer_t streamer = GetStream(player);
            if (!streamer)
            {
                return false;
            }

            if (streamer->IsVirtual())
            {
                StartVirtualStream(streamer);
                return true;
            }
            else if (streamer->IsShared())
            {
                std::unique_lock<std::recursive_mutex> g(players_lock());

                assert(m_shared_streamplayers.find(streamer->outputdeviceid) != m_shared_streamplayers.end());
                auto sharedstream = m_shared_streamplayers[streamer->outputdeviceid];
                g.unlock();
                return sharedstream->AddOutputStreamer(streamer, player);
            }

            return StartStream(streamer);
        }
        bool StopStream(StreamPlayer* player)
        {
            outputstreamer_t streamer = GetStream(player);
            if (!streamer)
                return false;

            if (streamer->IsVirtual())
            {
                StopVirtualStream(streamer.get());
                return true;
            }
            else if (streamer->IsShared())
            {
                std::unique_lock<std::recursive_mutex> g(players_lock());

                assert(m_shared_streamplayers.find(streamer->outputdeviceid) != m_shared_streamplayers.end());
                auto sharedstream = m_shared_streamplayers[streamer->outputdeviceid];
                g.unlock();
                sharedstream->RemoveOutputStreamer(player);
                return true;
            }

            return StopStream(streamer);
        }
        bool IsStreamStopped(StreamPlayer* player)
        {
            outputstreamer_t streamer = GetStream(player);
            if (!streamer)
                return true; //non-existing stream is same as stopped

            if(streamer->IsVirtual())
            {
                return IsVirtualStreamStopped(streamer.get());
            }
            else if (streamer->IsShared())
            {
                std::unique_lock<std::recursive_mutex> g(players_lock());

                assert(m_shared_streamplayers.find(streamer->outputdeviceid) != m_shared_streamplayers.end());
                auto sharedstream = m_shared_streamplayers[streamer->outputdeviceid];
                g.unlock();
                return !sharedstream->Exists(player);
            }

            return IsStreamStopped(streamer);
        }

        bool OpenDuplexStream(StreamDuplex* duplex, int inputdeviceid,
                              int outputdeviceid, int sndgrpid,
                              int samplerate, int input_channels,
                              int output_channels, int framesize)
        {
            MYTRACE(ACE_TEXT("Opening StreamDuplex %p, sample rate %d, channels %d-%d, framesize %d, devices #%d-#%d\n"),
                duplex, samplerate, input_channels, output_channels, framesize, inputdeviceid, outputdeviceid);

            duplexstreamer_t streamer;

            if(inputdeviceid == SOUND_DEVICEID_VIRTUAL && outputdeviceid == SOUND_DEVICEID_VIRTUAL)
            {
                streamer = NewVirtualStream(duplex, sndgrpid, samplerate,
                                            input_channels, output_channels,
                                            framesize);
            }
            else if(inputdeviceid == SOUND_DEVICEID_VIRTUAL || outputdeviceid == SOUND_DEVICEID_VIRTUAL)
            {
                return false;
            }
            else
            {
                streamer= NewStream(duplex, inputdeviceid,
                                    outputdeviceid, sndgrpid,
                                    samplerate, input_channels,
                                    output_channels, framesize);
            }

            if (!streamer)
                return false;

            {
                std::lock_guard<std::recursive_mutex> g(duplex_lock());
                m_duplex_streamers[duplex] = streamer;
            }

            if(streamer->IsVirtual())
            {
                StartVirtualStream(streamer);
                return true;
            }

            if(StartStream(streamer))
                return true;

            CloseDuplexStream(duplex);
            return false;
        }

        bool CloseDuplexStream(StreamDuplex* duplex)
        {
            duplexstreamer_t streamer = GetStream(duplex);
            if (!streamer)
                return false;

            if(streamer->IsVirtual())
            {
                StopVirtualStream(streamer.get());
            }
            else
            {
                CloseStream(streamer);
            }

            std::lock_guard<std::recursive_mutex> g(duplex_lock());
            m_duplex_streamers.erase(duplex);

            MYTRACE(ACE_TEXT("Closed StreamDuplex %p\n"), duplex);

            return true;
        }

        bool AddDuplexOutputStream(StreamDuplex* duplex,
                                   StreamPlayer* player)
        {
            duplexstreamer_t streamer = GetStream(duplex);
            if (!streamer)
                return false;

            outputstreamer_t newstreamer(new OUTPUTSTREAMER(player,
                                                            streamer->sndgrpid,
                                                            streamer->framesize,
                                                            streamer->samplerate,
                                                            streamer->output_channels,
                                                            streamer->output_soundsystem,
                                                            streamer->outputdeviceid));
#if defined(_DEBUG)
            newstreamer->duplex = true;
#endif
            {
                {
                    std::lock_guard<std::recursive_mutex> g1(players_lock());
                    m_output_streamers[player] = newstreamer;
                }

                std::lock_guard<std::recursive_mutex> g2(streamer->players_mtx);
                //store in list of duplex players which will receive output-callback
                streamer->players.push_back(newstreamer.get());
            }

            //set master volume so it's relative to master volume
            SetVolume(player, VOLUME_DEFAULT);

            return true;
        }

        bool RemoveDuplexOutputStream(StreamDuplex* duplex,
                                      StreamPlayer* player)
        {
            duplexstreamer_t streamer = GetStream(duplex);
            if (!streamer)
                return false;

            {
                std::lock_guard<std::recursive_mutex> g2(streamer->players_mtx);
                for(size_t i = 0; i<streamer->players.size();)
                {
                    if(streamer->players[i]->player == player)
                    {
                        streamer->players.erase(streamer->players.begin() + i);
                    }
                    else i++;
                }
            }

            // player must be erased after it is removed from streamer->players
            std::lock_guard<std::recursive_mutex> g1(players_lock());
            m_output_streamers.erase(player);

            return true;
        }

        virtual bool SetMasterVolume(int sndgrpid, int volume)
        {
            if(volume > VOLUME_MAX) volume = VOLUME_MAX;
            else if(volume < VOLUME_MIN) volume = VOLUME_MIN;

            {
                soundgroup_t sndgrp = GetSoundGroup(sndgrpid);
                if (!sndgrp)
                    return false;
                sndgrp->mastervolume = volume;
            }

            std::vector<StreamPlayer*> players = GetPlayers(sndgrpid);
            for(size_t i=0;i<players.size();i++)
            {
                outputstreamer_t streamer = GetStream(players[i]);
                if (streamer)
                    SetVolume(players[i], streamer->volume);
            }
            return true;
        }
        virtual int GetMasterVolume(int sndgrpid)
        {
            soundgroup_t sndgrp = GetSoundGroup(sndgrpid);
            if (!sndgrp)
                return VOLUME_MIN;
            return sndgrp->mastervolume;
        }

        virtual bool SetAutoPositioning(int sndgrpid, bool enable) { return false; }
        virtual bool IsAutoPositioning(int sndgrpid) { return false; }
        //bAll = true => Also those with autoposition disabled
        virtual bool AutoPositionPlayers(int sndgrpid, bool all_players) { return false; }

        virtual bool IsAllMute(int sndgrpid)
        {
            soundgroup_t sndgrp = GetSoundGroup(sndgrpid);
            if (!sndgrp)
                return false;

            return sndgrp->muteall;
        }

        virtual bool MuteAll(int sndgrpid, bool mute)
        {
            soundgroup_t sndgrp = GetSoundGroup(sndgrpid);
            if (!sndgrp)
                return false;
            sndgrp->muteall = mute;

            std::vector<StreamPlayer*> players = GetPlayers(sndgrpid);
            for(size_t i=0;i<players.size();i++)
            {
                outputstreamer_t streamer = GetStream(players[i]);
                if (streamer && !mute)
                    SetVolume(players[i], streamer->volume);
            }

            return true;
        }

        bool RestartSoundSystem()
        {
            {
                std::lock_guard<std::recursive_mutex> g1(sndgrp_lock());
                std::lock_guard<std::recursive_mutex> g2(capture_lock());
                std::lock_guard<std::recursive_mutex> g3(players_lock());
                std::lock_guard<std::recursive_mutex> g4(duplex_lock());

                if(m_input_streamers.size() || m_output_streamers.size() || m_duplex_streamers.size())
                    return false;
            }

            Close();

            {
                std::lock_guard<std::recursive_mutex> g(m_devs_lock);
                m_sounddevs.clear();
            }

            return Init();
        }

        //input and output devices

        void RefreshDevices()
        {
            {
                std::lock_guard<std::recursive_mutex> g(m_devs_lock);
                m_sounddevs.clear();
            }

            FillDevices(m_sounddevs);
            AddVirtualDevice();
        }

        virtual bool GetSoundDevices(devices_t& snddevices)
        {
            std::lock_guard<std::recursive_mutex> g(m_devs_lock);

            sounddevices_t::const_iterator ii = m_sounddevs.begin();
            while(ii != m_sounddevs.end())
            {
                snddevices.push_back(ii->second);
                ii++;
            }

            return true;
        }

        virtual bool CheckInputDevice(int inputdeviceid)
        {
            DeviceInfo dev;
            if(GetDevice(inputdeviceid, dev) && dev.max_input_channels>0)
                return true;
            return false;
        }

        virtual bool CheckOutputDevice(int outputdeviceid)
        {
            DeviceInfo dev;
            if(GetDevice(outputdeviceid, dev) && dev.max_output_channels>0)
                return true;
            return false;
        }

        virtual bool SupportsInputFormat(int inputdeviceid,
                                         int input_channels,
                                         int samplerate)
        {
            DeviceInfo dev;
            return GetDevice(inputdeviceid, dev) &&
                dev.SupportsInputFormat(input_channels, samplerate);
        }

        virtual bool SupportsOutputFormat(int outputdeviceid,
                                          int output_channels,
                                          int samplerate)
        {
            DeviceInfo dev;
            return GetDevice(outputdeviceid, dev) &&
                dev.SupportsOutputFormat(output_channels, samplerate);
        }

        virtual bool GetDevice(int id, DeviceInfo& dev)
        {
            std::lock_guard<std::recursive_mutex> g(m_devs_lock);

            sounddevices_t::const_iterator ii = m_sounddevs.find(id & SOUND_DEVICEID_MASK);
            if(ii != m_sounddevs.end())
            {
                dev = ii->second;
                return true;
            }
            MYTRACE(ACE_TEXT("Cannot find sound device #%d\n"), id);
            return false;
        }

        virtual void SetVolume(StreamPlayer* player, int volume)
        {
            if(volume > VOLUME_MAX)
                volume = VOLUME_MAX;
            else if(volume < VOLUME_MIN)
                volume = VOLUME_MIN;

            outputstreamer_t streamer = GetStream(player);
            if (!streamer)
                return;

            streamer->volume = volume;

            if(!streamer->mute && !IsAllMute(streamer->sndgrpid))
            {
                //compensate for Master vol
                int master = GetMasterVolume(streamer->sndgrpid);
                int newvol = (int)((((float)master /(float)VOLUME_DEFAULT) * (float)volume)+0.5f);
                float fVol = (float)newvol / (float)VOLUME_DEFAULT;
            }
        }

        virtual int GetVolume(StreamPlayer* player)
        {
            outputstreamer_t streamer = GetStream(player);
            if (!streamer)
                return VOLUME_MIN;

            return streamer->volume;
        }

        virtual void SetAutoPositioning(StreamPlayer* player, bool enable) {}
        virtual bool IsAutoPositioning(StreamPlayer* player) { return false; }

        virtual bool SetPosition(StreamPlayer* player, float x, float y, float z) { return false; }
        virtual bool GetPosition(StreamPlayer* player, float& x, float& y, float& z) { return false; }

        virtual void SetMute(StreamPlayer* player, bool mute)
        {
            outputstreamer_t streamer = GetStream(player);
            if (!streamer)
                return;

            if(mute)
                streamer->mute = mute;
            else
            {
                streamer->mute = mute;
                SetVolume(player, streamer->volume);
            }

        }

        virtual bool IsMute(StreamPlayer* player)
        {
            outputstreamer_t streamer = GetStream(player);
            if (!streamer)
                return false;

            return streamer->mute;
        }


    protected:
        void StartVirtualStream(inputstreamer_t streamer)
        {
            assert(streamer->IsVirtual());
            streamcallback_t scc(new StreamCaptureCallback(streamer.get()));
            std::lock_guard<std::recursive_mutex> g(m_virtdev_lock);
            m_virtual_streams[streamer.get()] = scc;

            int ret = scc->activate();
            MYTRACE_COND(ret < 0, ACE_TEXT("Failed to activate StreamCaller\n"));
        }

        void StartVirtualStream(outputstreamer_t streamer)
        {
            assert(streamer->IsVirtual());
            streamcallback_t scc(new StreamPlayerCallback(streamer.get()));
            std::lock_guard<std::recursive_mutex> g(m_virtdev_lock);
            m_virtual_streams[streamer.get()] = scc;

            int ret = scc->activate();
            MYTRACE_COND(ret < 0, ACE_TEXT("Failed to activate StreamCaller\n"));
        }

        void StartVirtualStream(duplexstreamer_t streamer)
        {
            assert(streamer->IsVirtual());
            streamcallback_t scc(new StreamDuplexCallback(this, streamer.get()));
            std::lock_guard<std::recursive_mutex> g(m_virtdev_lock);
            m_virtual_streams[streamer.get()] = scc;

            int ret = scc->activate();
            MYTRACE_COND(ret < 0, ACE_TEXT("Failed to activate StreamCaller\n"));
        }

        void StopVirtualStream(SoundStreamer* streamer)
        {
            std::lock_guard<std::recursive_mutex> g(m_virtdev_lock);
            m_virtual_streams.erase(streamer);
        }

        bool IsVirtualStreamStopped(SoundStreamer* streamer)
        {
            std::lock_guard<std::recursive_mutex> g(m_virtdev_lock);
            return m_virtual_streams.find(streamer) == m_virtual_streams.end();
        }

    private:

        void AddVirtualDevice()
        {
            DeviceInfo device;
            device.devicename = ACE_TEXT("TeamTalk Virtual Sound Device");
            device.deviceid = ACE_TEXT("TeamTalk Sound Device #1");
            device.soundsystem = SOUND_API_NOSOUND;
            device.max_input_channels = 2;
            device.max_output_channels = 2;
            device.input_channels.insert(1);
            device.input_channels.insert(2);
            device.output_channels.insert(1);
            device.output_channels.insert(2);

            device.default_samplerate = standardSampleRates.back();
            device.input_samplerates.insert(standardSampleRates.begin(),
                                            standardSampleRates.end());
            device.output_samplerates.insert(standardSampleRates.begin(),
                                             standardSampleRates.end());
            device.id = SOUND_DEVICEID_VIRTUAL;
            m_sounddevs[device.id] = device;
        }

    };

}

#endif
