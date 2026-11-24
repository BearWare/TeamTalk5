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

#if !defined(SOUNDSYSTEMSHARED_H)
#define SOUNDSYSTEMSHARED_H

#include "SoundSystem.h"

#include "AudioResampler.h"
#include "codec/MediaUtil.h"
#include "mystd/MyQueue.h"

#include <ace/Synch_Traits.h>
#include <ace/Time_Value.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

namespace soundsystem {

/* SharedStreamCapture::MakeKey() has limited space */
constexpr auto SHAREDSTREAM_MAX_SAMPLERATES = 16;
constexpr auto SHAREDSTREAM_MAX_CHANNELS    = 2;
constexpr auto SHAREDSTREAM_MAX_FRAMESIZE   = (1 << 27) - 1;

constexpr auto DEBUG_RESAMPLER = 0;

    template < typename INPUTSTREAMER >
    class SharedStreamCapture : public StreamCapture
    {
        std::vector<int> m_keysamplerates;

        uint32_t MakeKey(const InputStreamer& streamer)
        {
            assert(!m_keysamplerates.empty());
            assert(streamer.channels <= 2);
            auto i = std::ranges::find(m_keysamplerates, streamer.samplerate);
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
            return key & SHAREDSTREAM_MAX_FRAMESIZE;
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

        SoundDeviceFeatures m_features;

    public:
        using inputstreamer_t = std::shared_ptr < INPUTSTREAMER >;

        SharedStreamCapture(SoundDeviceFeatures features) : m_features(features) { }
        ~SharedStreamCapture() override
        {
            assert(m_sndgrpid == 0);
            assert(m_inputstreams.empty());
            assert(m_activestreams.empty());
            assert(m_resample_buffers.empty());
            assert(m_callback_buffers.empty());

            m_samples_queue.Close();
            if (m_resample_thread)
                m_resample_thread->join();

            MYTRACE(TTEXT("~SharedCaptureStream()\n"));
        }

        SoundDeviceFeatures GetCaptureFeatures() override
        {
            return m_features;
        }

        // Set the source input stream which feeds the shared input streams
        void SetOrigin(inputstreamer_t streamer, int sndgrpid)
        {
            m_originalstream = streamer;
            m_sndgrpid = sndgrpid;

            size_t buffersize = PCM16_BYTES(streamer->framesize, streamer->channels);
            buffersize *= 3; //TODO: what is a good number??
            m_samples_queue.SetWaterMark(buffersize);
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
                assert(m_keysamplerates.size() <= SHAREDSTREAM_MAX_SAMPLERATES);
            }

            MYTRACE(TTEXT("Number of active inputs on sound device #%d: %u\n"),
                    streamer->inputdeviceid & SOUND_DEVICEID_MASK, uint32_t(m_resamplers.size()));

            // create audio resampler if samplerate/framesize/channels
            // are different from original
            if (SameStreamProperties(*GetOrigin(), *streamer))
                return true;

            auto key = MakeKey(*streamer);
            if (m_resamplers.find(key) == m_resamplers.end())
            {
                media::AudioFormat infmt(m_originalstream->samplerate, m_originalstream->channels);
                media::AudioFormat outfmt(streamer->samplerate, streamer->channels);
                auto resampler = MakeAudioResampler(infmt, outfmt);
                if (!resampler)
                    return false;

                m_resamplers[key] = resampler;
                int resampleoutput = CalcSamples(m_originalstream->samplerate,
                                                 m_originalstream->framesize, streamer->samplerate);

                m_resample_buffers[key].resize(static_cast<size_t>(resampleoutput) * streamer->channels);
                m_callback_buffers[key].resize(size_t(streamer->framesize) * streamer->channels);

                if (!m_resample_thread)
                {
                    m_resample_thread = std::make_shared<std::thread>(&SharedStreamCapture<INPUTSTREAMER>::ResampleFunc, this);
                }
            }

            MYTRACE(TTEXT("Number of active input resamplers on sound device #%d: %u\n"),
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
            std::scoped_lock g(m_mutex);
            assert(m_inputstreams.find(streamer) != m_inputstreams.end());

            if (active)
                m_activestreams.insert(streamer);
            else
                m_activestreams.erase(streamer);
        }

        bool InputStreamsExists()
        {
            std::scoped_lock g(m_mutex);
            return !m_inputstreams.empty();
        }

        bool ActiveStreamsExists()
        {
            std::scoped_lock g(m_mutex);
            return !m_activestreams.empty();
        }

        void StreamCaptureCb(const InputStreamer& streamer,
                             const short* buffer, int samples) override
        {
            assert((streamer.inputdeviceid & SOUND_DEVICE_SHARED_FLAG) == 0);

            std::scoped_lock g(m_mutex);

            MYTRACE_COND(DEBUG_RESAMPLER, TTEXT("Original for %p samplerate %d, framesize %d, channels %d\n"),
                    streamer.recorder, streamer.samplerate,
                    streamer.framesize, streamer.channels);

            bool resample = false;
            for (auto stream : m_activestreams)
            {
                if (SameStreamProperties(*stream, streamer))
                {
                    stream->recorder->StreamCaptureCb(*stream, buffer, samples);
                    MYTRACE_COND(DEBUG_RESAMPLER, TTEXT("Shared for %p samplerate %d, framesize %d, channels %d\n"),
                                 stream->recorder, stream->samplerate,
                                 stream->framesize, stream->channels);
                }
                else
                {
                    resample = true;
                }
            }

            if (resample)
            {
                size_t const size = PCM16_BYTES(samples, streamer.channels);
                auto mb = std::make_unique<MsgBlock>(size);
                mb->Copy(reinterpret_cast<const char*>(buffer), size);
                m_samples_queue.Put(std::move(mb)); // queue owns the block now
            }
        }

        void ResampleFunc()
        {
            std::unique_ptr<MsgBlock> mb;
            while (m_samples_queue.Get(mb))
            {
                std::scoped_lock g(m_mutex);

                assert(mb->Length() == PCM16_BYTES(m_originalstream->framesize, m_originalstream->channels));
                for (const auto& i : m_resamplers)
                {
                    auto key = i.first;
                    int cbsr = GetSampleRateFromKey(key);
                    int cbch = GetChannelsFromKey(key);
                    int cbframesize = GetFrameSizeFromKey(key);
                    short* cbbufptr = m_callback_buffers[key].data();

                    auto rsbuf = m_resample_buffers.find(key);
                    assert(rsbuf != m_resample_buffers.end());
                    short* rsbufptr = rsbuf->second.data();
                    assert(cbch);
                    int rsframesize = static_cast<int>(rsbuf->second.size()) / cbch;
                    assert(i.second);
                    int samples = i.second->Resample(reinterpret_cast<const short*>(mb->RdPtr()),
                                                     m_originalstream->framesize,
                                                     rsbufptr, rsframesize);
                    MYTRACE_COND(DEBUG_RESAMPLER, TTEXT("Resampled for samplerate %d, framesize %d, channels %d\n"),
                            cbsr, cbframesize, cbch);
                    MYTRACE_COND(samples != rsframesize,
                                 TTEXT("Resampled output frame for samplerate %d, channels %d doesn't match framesize %d. Was %d\n"),
                                 cbsr, cbch, rsframesize, samples);

                    // Now copy samples from m_resample_buffer[key] to
                    // m_callback_buffer[key], i.e. from original
                    // capture stream to shared capture stream.
                    //
                    // Here we want to use "total" samples where
                    // channel information (mono/stereo) is omitted.
                    int const totalsamples = rsframesize * cbch;
                    int rspos = 0;
                    while (rspos < totalsamples)
                    {
                        // space in callback
                        std::size_t cbpos = m_callback_index[key];
                        std::size_t cbbufspace = m_callback_buffers[key].size() - cbpos;
                        assert(int(cbpos) >= 0 && cbpos < m_callback_buffers[key].size());

                        // calc samples to copy
                        std::size_t const rsremain = totalsamples - rspos;
                        std::size_t const n_samples = std::min(cbbufspace, rsremain);

                        //where to copy from
                        MYTRACE_COND(DEBUG_RESAMPLER, TTEXT("Copying at cbpos %d, rspos %u for samplerate %d, framesize %d, channels %d\n"),
                                int(cbpos), rspos, cbsr, cbframesize, cbch);
                        assert(rspos + n_samples <= m_resample_buffers[key].size());
                        assert(cbpos + n_samples <= m_callback_buffers[key].size());

                        std::size_t const bytes = n_samples * sizeof(cbbufptr[0]);
                        std::memcpy(&cbbufptr[cbpos], &rsbufptr[rspos], bytes);

                        cbpos += n_samples;
                        rspos += static_cast<int>(n_samples);

                        if (cbpos == m_callback_buffers[key].size())
                        {
                            for (auto streamer : m_activestreams)
                            {
                                if (MakeKey(*streamer) == key)
                                {
                                    streamer->recorder->StreamCaptureCb(*streamer, cbbufptr, cbframesize);
                                    MYTRACE_COND(DEBUG_RESAMPLER, TTEXT("Callback for %p samplerate %d, framesize %d, channels %d\n"),
                                            streamer->recorder, cbsr, cbframesize, cbch);
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
        MsgQueue m_samples_queue;
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

    constexpr auto DEBUG_SHAREDPLAYER = 0;
    constexpr auto DEBUG_SHAREDPLAYER_RESAMPLER = 0;

    template < typename OUTPUTSTREAMER >
    class SharedStreamPlayer : public StreamPlayer
    {
        bool SameStreamProperties(const OutputStreamer& os1, const OutputStreamer& os2) const
        {
            return os1.samplerate == os2.samplerate && os1.channels == os2.channels &&
                os1.framesize == os2.framesize;
        }

    public:
        using outputstreamer_t = std::shared_ptr < OUTPUTSTREAMER >;

        SharedStreamPlayer(SoundSystem* sndsys)
        : m_sndsys(sndsys)
        {
            MYTRACE_COND(DEBUG_SHAREDPLAYER, TTEXT("SharedStreamPlayer() - %p\n"), this);
        }

        ~SharedStreamPlayer() override
        {
            MYTRACE_COND(DEBUG_SHAREDPLAYER, TTEXT("~SharedStreamPlayer() - %p\n"), this);
            assert(m_outputs.empty());
        }

        bool StreamPlayerCb(const OutputStreamer& streamer,
                            short* buffer, int samples) override
        {
            assert(SameStreamProperties(*m_orgstream, streamer));

            const size_t reqbytes = PCM16_BYTES(streamer.framesize, streamer.channels);
            memset(buffer, 0, reqbytes);

            std::scoped_lock g(m_mutex);

            MYTRACE_COND(DEBUG_SHAREDPLAYER_RESAMPLER,
                         TTEXT("--------------------- Mixer inputs %d -----------------------------\n"),
                         int(m_active_outputs.size()));

            for (auto i : m_outputs)
            {
                if (m_active_outputs.find(i.first) == m_active_outputs.end())
                    continue;

                MYTRACE_COND(DEBUG_SHAREDPLAYER_RESAMPLER,
                             TTEXT("Resample source: %d samples %d channels @ %d Hz duration %d msec. Destination %p: %d samples %d channels @ %d Hz duration %d msec\n"),
                             m_orgstream->framesize, m_orgstream->channels, m_orgstream->samplerate,
                             int(PCM16_SAMPLES_DURATION(m_orgstream->framesize, m_orgstream->samplerate)),
                             i.first, i.second->framesize, i.second->channels, i.second->samplerate,
                             int(PCM16_SAMPLES_DURATION(i.second->framesize, i.second->samplerate)));

                if (SameStreamProperties(*i.second, *m_orgstream))
                {
                    assert(i.second->framesize == samples);
                    i.first->StreamPlayerCb(*i.second, m_tmpbuffer.data(), samples);
                    int mastervol = m_sndsys->GetMasterVolume(i.second->sndgrpid);
                    bool mastermute = m_sndsys->IsAllMute(i.second->sndgrpid);
                    SoftVolume(*i.second, m_tmpbuffer.data(), samples, mastervol, mastermute);
                    MYTRACE_COND(DEBUG_SHAREDPLAYER_RESAMPLER, TTEXT("Same stream properties. Destination: %p\n"), i.first);
                }
                else
                {
                    auto streamer_resam = i.second;
                    assert(m_resamplers.find(i.first) != m_resamplers.end());
                    auto msgq = m_resambuffers[i.first];
                    auto resampler = m_resamplers[i.first];
                    short* input = &m_callbackbuffers[i.first][0];
                    int outputsamples = CalcSamples(streamer_resam->samplerate, streamer_resam->framesize, m_orgstream->samplerate);
                    const size_t outputbytes = PCM16_BYTES(outputsamples, m_orgstream->channels);

                    MYTRACE_COND(DEBUG_SHAREDPLAYER_RESAMPLER, TTEXT("Duration of queue before: %d msec\n"),
                                 int(PCM16_BYTES_DURATION(msgq->BytesQueued(), streamer.channels,
                                                          streamer.samplerate)));

                    // fill up buffer with enough bytes to do a callback
                    while (msgq->BytesQueued() < reqbytes)
                    {
                        i.first->StreamPlayerCb(*streamer_resam, input, streamer_resam->framesize);
                        int mastervol = m_sndsys->GetMasterVolume(streamer_resam->sndgrpid);
                        bool mastermute = m_sndsys->IsAllMute(streamer_resam->sndgrpid);
                        SoftVolume(*streamer_resam, input, streamer_resam->framesize, mastervol, mastermute);
                        short* output = resampler->Resample(input);

                        auto mb = std::make_unique<MsgBlock>(outputbytes);
                        mb->Copy(reinterpret_cast<const char*>(output), outputbytes);
                        if (!msgq->Put(std::move(mb)))
                            msgq->Close();
                    }

                    MYTRACE_COND(DEBUG_SHAREDPLAYER_RESAMPLER, TTEXT("Duration of queue after refill: %d msec. Bytes: %u\n"),
                                 int(PCM16_BYTES_DURATION(msgq->BytesQueued(), streamer.channels,
                                                          streamer.samplerate)), unsigned(msgq->BytesQueued()));


                    // copy buffer to callback
                    char* bytebuffer = reinterpret_cast<char*>(m_tmpbuffer.data());
                    size_t copied = 0;
                    while (copied < reqbytes)
                    {
                        std::unique_ptr<MsgBlock> mb;
                        if (!msgq->Get(mb))
                        {
                            msgq->Close();
                            break;
                        }

                        size_t const copylimit = std::min(mb->Length(), reqbytes - copied);
                        std::memcpy(bytebuffer + copied, mb->RdPtr(), copylimit);
                        auto *was = mb->RdPtr();
                        mb->RdPtr(copylimit);
                        assert(mb->RdPtr() == was + copylimit);
                        copied += copylimit;

                        if (mb->Length() != 0u)
                        {
                            assert(copied == reqbytes);
                            if (!msgq->PutFront(std::move(mb)))
                            {
                                msgq->Close();
                                break;
                            }
                        }
                    }
                    assert(copied == reqbytes);

                    MYTRACE_COND(DEBUG_SHAREDPLAYER_RESAMPLER, TTEXT("Duration of queue after dequeue: %d msec\n"),
                                 int(PCM16_BYTES_DURATION(msgq->BytesQueued(), streamer.channels,
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
                        buffer[i] = static_cast<short>(val);
                }

            } // for-loop - streams

            return true;
        }

        void SetOrigin(outputstreamer_t streamer)
        {
            assert(!m_orgstream);
            m_orgstream = streamer;
            m_tmpbuffer.resize(size_t(streamer->channels) * streamer->framesize);
        }

        outputstreamer_t GetOrigin()
        {
            return m_orgstream;
        }

        bool AddOutputStreamer(outputstreamer_t streamer, StreamPlayer* player)
        {
            std::scoped_lock g(m_mutex);

            assert(m_orgstream);

            assert(m_outputs.find(player) == m_outputs.end());

            MYTRACE_COND(DEBUG_SHAREDPLAYER, TTEXT("Added player %p to SharedStreamPlayer %p\n"), player, this);

            m_outputs[player] = streamer;

            MYTRACE(TTEXT("Number of output streams from sound device #%d: %d\n"),
                    streamer->outputdeviceid & SOUND_DEVICEID_MASK, int(m_outputs.size()));

            if (SameStreamProperties(*m_orgstream, *streamer))
                return true;

            media::AudioFormat infmt(streamer->samplerate, streamer->channels);
            media::AudioFormat outfmt(m_orgstream->samplerate, m_orgstream->channels);
            auto resampler = MakeAudioResampler(infmt, outfmt, streamer->framesize);
            if (!resampler)
            {
                m_outputs.erase(player);
                return false;
            }

            m_resamplers[player] = resampler;
            m_callbackbuffers[player].resize(size_t(streamer->channels) * streamer->framesize);
            m_resambuffers[player] = std::make_shared<MsgQueue>();
            constexpr auto ONEMBYTE = 1024 * 1024;
            m_resambuffers[player]->SetWaterMark(ONEMBYTE);
            return true;
        }

        void RemoveOutputStreamer(StreamPlayer* player)
        {
            std::scoped_lock g(m_mutex);

            MYTRACE_COND(DEBUG_SHAREDPLAYER, TTEXT("Removed player %p from SharedStreamPlayer %p\n"), player, this);

            m_active_outputs.erase(player);
            m_outputs.erase(player);
            m_resamplers.erase(player);
            m_callbackbuffers.erase(player);
            m_resambuffers.erase(player);
        }

        void ActivatePlayer(StreamPlayer* player, bool active)
        {
            std::scoped_lock g(m_mutex);
            assert(m_outputs.find(player) != m_outputs.end());
            if (active)
            {
                assert(!m_active_outputs.contains(player));
                m_active_outputs.insert(player);
            }
            else
            {
                assert(m_active_outputs.contains(player));
                m_active_outputs.erase(player);
            }
        }

        bool Empty()
        {
            std::scoped_lock g(m_mutex);
            return m_outputs.empty();
        }

        bool IsActive(StreamPlayer* player)
        {
            std::scoped_lock g(m_mutex);
            return m_active_outputs.contains(player);
        }

    private:
        using msg_queue_t = std::shared_ptr< MsgQueue >;

        SoundSystem* m_sndsys;
        outputstreamer_t m_orgstream;
        std::vector<short> m_tmpbuffer;
        std::map<StreamPlayer*, outputstreamer_t> m_outputs;
        std::set<StreamPlayer*> m_active_outputs;
        std::map<StreamPlayer*, audio_resampler_t> m_resamplers;
        std::map<StreamPlayer*, std::vector<short>> m_callbackbuffers;
        std::map<StreamPlayer*, msg_queue_t> m_resambuffers;
        std::recursive_mutex m_mutex;
    };

} // namespace soundsystem
#endif
