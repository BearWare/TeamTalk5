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

#include "SoundSystemEx.h"
#include "SoundSystemShared.h"

#include <mutex>

namespace soundsystem {

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
            assert(standardSampleRates.size() <= SHAREDSTREAM_MAX_SAMPLERATES);
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

        // recording members

        virtual inputstreamer_t NewStream(StreamCapture* capture,
                                          int inputdeviceid, int sndgrpid,
                                          int samplerate, int channels,
                                          int framesize) = 0;
        virtual bool StartStream(inputstreamer_t streamer) = 0;
        virtual void CloseStream(inputstreamer_t streamer) = 0;
        virtual bool IsStreamStopped(inputstreamer_t streamer) = 0;
        virtual bool UpdateStreamCaptureFeatures(inputstreamer_t streamer) { return false; }

        inputstreamer_t GetStream(StreamCapture* capture, bool allowVirtual = true, bool getSharedOrigin = false)
        {
            std::lock_guard<std::recursive_mutex> g(capture_lock());
            typename inputstreamers_t::iterator ii = m_input_streamers.find(capture);
            if(ii != m_input_streamers.end())
            {
                auto inputstream = ii->second;

                if (inputstream->IsVirtual() && !allowVirtual)
                    return inputstreamer_t();

                if (inputstream->IsShared() && getSharedOrigin)
                {
                    sharedstreamcapture_t sharedstream = m_shared_streamcaptures[inputstream->inputdeviceid];
                    assert(sharedstream);
                    inputstream = sharedstream->GetOrigin();
                    assert(inputstream);
                    return inputstream;
                }

                return inputstream;
            }
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

        // playback members
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

        // duplex members
        virtual duplexstreamer_t NewStream(StreamDuplex* duplex, int inputdeviceid,
                                           int outputdeviceid, int sndgrpid,
                                           int samplerate, int input_channels,
                                           int output_channels, int framesize) = 0;
        virtual void CloseStream(duplexstreamer_t streamer) = 0;
        virtual bool StartStream(duplexstreamer_t streamer) = 0;
        virtual bool IsStreamStopped(duplexstreamer_t streamer) = 0;

        virtual bool UpdateStreamDuplexFeatures(duplexstreamer_t streamer) { return false; }

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

        // default shared input/output settings are sound device's
        // default samplerate, max channels and samplerate * 0.04
        // frame size.
        int m_shared_inputsamplerate = 0, m_shared_outputsamplerate = 0,
            m_shared_inputchannels = 0, m_shared_outputchannels = 0,
            m_shared_inputframesize = 0, m_shared_outputframesize = 0;

        std::recursive_mutex m_sndgrp_lock, m_cap_lock, m_play_lock, m_dpx_lock;

        std::recursive_mutex& sndgrp_lock() { return m_sndgrp_lock; }
        std::recursive_mutex& capture_lock() { return m_cap_lock; }
        std::recursive_mutex& players_lock() { return m_play_lock; }
        std::recursive_mutex& duplex_lock() { return m_dpx_lock; }

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
            sharedstream.reset(new SharedStreamCapture<INPUTSTREAMER>(capture->GetCaptureFeatures()));

            // create new sound group. We cannot use 'sndgrpid' since
            // that instance might be deleted and the shared stream
            // would stop.
            int newsndgrpid = OpenSoundGroup();
            if (!newsndgrpid)
                return inputstreamer_t();

            int sharedsamplerate = m_shared_inputsamplerate != 0? m_shared_inputsamplerate : snddev.default_samplerate;
            int sharedchannels = m_shared_inputchannels != 0? m_shared_inputchannels : snddev.max_input_channels;
            int sharedframesize = m_shared_inputframesize != 0? m_shared_inputframesize : int(snddev.default_samplerate * 0.04);

            inputstreamer_t orgstream;
            if (snddev.id == SOUND_DEVICEID_VIRTUAL)
            {
                orgstream = NewVirtualStream(sharedstream.get(),
                                             newsndgrpid, sharedsamplerate,
                                             sharedchannels, sharedframesize);
            }
            else
            {
                MYTRACE(ACE_TEXT("Opening shared input device on #%d\n"), snddev.id & SOUND_DEVICEID_MASK);
                orgstream = NewStream(sharedstream.get(),
                                      snddev.id & SOUND_DEVICEID_MASK,
                                      newsndgrpid, sharedsamplerate,
                                      sharedchannels, sharedframesize);
            }

            if (!orgstream)
            {
                MYTRACE(ACE_TEXT("Failed to open shared input device on #%d\n"), snddev.id & SOUND_DEVICEID_MASK);
                RemoveSoundGroup(newsndgrpid);
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
                RemoveSoundGroup(newsndgrpid);

                return inputstreamer_t();
            }

            MYTRACE(ACE_TEXT("Opened shared capture stream %p on device #%d, samplerate %d, channels %d\n"),
                    capture, (inputdeviceid & SOUND_DEVICEID_MASK), samplerate, channels);

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

            int sharedsamplerate = m_shared_outputsamplerate != 0? m_shared_outputsamplerate : snddev.default_samplerate;
            int sharedchannels = m_shared_outputchannels != 0? m_shared_outputchannels : snddev.max_output_channels;
            int sharedframesize = m_shared_outputframesize != 0? m_shared_outputframesize : int(snddev.default_samplerate * 0.04);

            outputstreamer_t orgstream;
            if (snddev.id == SOUND_DEVICEID_VIRTUAL)
            {
                orgstream = NewVirtualStream(sharedstream.get(), newsndgrpid,
                                             sharedsamplerate, sharedchannels, sharedframesize);
            }
            else
            {
                MYTRACE(ACE_TEXT("Opening shared output device on #%d\n"), snddev.id & SOUND_DEVICEID_MASK);
                orgstream = NewStream(sharedstream.get(), snddev.id & SOUND_DEVICEID_MASK,
                                      newsndgrpid, sharedsamplerate, sharedchannels, sharedframesize);
            }

            if (!orgstream)
            {
                RemoveSoundGroup(newsndgrpid);

                m_shared_streamplayers.erase(outputdeviceid);
                MYTRACE(ACE_TEXT("Failed to open shared output device on #%d\n"), snddev.id & SOUND_DEVICEID_MASK);
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

            MYTRACE(ACE_TEXT("Opened shared output stream %p on device #%d, samplerate %d, channels %d\n"),
                    player, (outputdeviceid & SOUND_DEVICEID_MASK), samplerate, channels);

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

            assert(channels <= SHAREDSTREAM_MAX_CHANNELS);
            assert(framesize <= SHAREDSTREAM_MAX_FRAMESIZE);

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

            MYTRACE_COND(!streamer, ACE_TEXT("Failed to open StreamCapture %p on device #%d\n"),
                         capture, inputdeviceid);

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

        bool IsStreamStopped(StreamCapture* capture)
        {
            // capture streams are started automatically so if they
            // exists then they're running
            inputstreamer_t streamer = GetStream(capture);
            if (!streamer)
                return true;

            return false;
        }

        bool UpdateStreamCaptureFeatures(StreamCapture* capture)
        {
            auto cap = GetStream(capture, true, true);
            if (cap)
                return UpdateStreamCaptureFeatures(cap);
            return false;
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

            MYTRACE_COND(!streamer, ACE_TEXT("Failed to open StreamPlayer %p on device #%d\n"),
                         player, outputdeviceid);

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

            MYTRACE_COND(!streamer, ACE_TEXT("Failed to open StreamDuplex %p on devices input #%d output %d\n"),
                         duplex, inputdeviceid, outputdeviceid);

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

        bool IsStreamStopped(StreamDuplex* duplex)
        {
            duplexstreamer_t streamer = GetStream(duplex);
            if (!streamer)
                return true;
            return IsStreamStopped(streamer);
        }

        bool UpdateStreamDuplexFeatures(StreamDuplex* duplex)
        {
            auto dpx = GetStream(duplex);
            if (dpx)
                return UpdateStreamDuplexFeatures(dpx);
            return false;
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

        bool InitSharedInputDevice(int samplerate, int channels, int framesize)
        {
            std::lock_guard<std::recursive_mutex> g(capture_lock());
            if (m_shared_streamcaptures.size())
                return false;

            m_shared_inputsamplerate = samplerate;
            m_shared_inputchannels = channels;
            m_shared_inputframesize = framesize;
            return true;
        }

        bool InitSharedOutputDevice(int samplerate, int channels, int framesize)
        {
            std::lock_guard<std::recursive_mutex> g(players_lock());
            if (m_shared_streamplayers.size())
                return false;

            m_shared_outputsamplerate = samplerate;
            m_shared_outputchannels = channels;
            m_shared_outputframesize = framesize;
            return true;
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

        // TODO: This doesn't support shared output device
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

    private:
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
