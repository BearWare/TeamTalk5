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

namespace soundsystem {

    void SoftVolume(const OutputStreamer& streamer, short* buffer, int samples);
    void DuplexCallback(DuplexStreamer& dpxStream, const short* recorded, short* playback);
    void MuxPlayers(const std::vector<OutputStreamer*>& players, short* tmp_buffer, short* playback);
    void DuplexEnded(DuplexStreamer& dpxStream);

    class StreamCaller : public ACE_Task_Base
    {
        std::vector<short> m_buffer;
        ACE_Time_Value m_start, m_interval;
        ACE_Reactor m_reactor;

    protected:
        // Derived classes with member variables must call this.
        // Otherwise StreamCallback() maybe be called from timer
        // on a destroyed object
        void Stop()
        {
            m_interval = ACE_Time_Value::zero;
            int ret = m_reactor.end_reactor_event_loop();
            assert(ret >= 0);
            ret = wait();
            assert(ret >= 0);
        }
    public:
        StreamCaller(const SoundStreamer& streamer, int channels)
        {
            m_buffer.resize(channels * streamer.framesize, 0);

            int msec = streamer.framesize * 1000 / streamer.samplerate;
            m_interval = ACE_Time_Value(msec / 1000, (msec % 1000) * 1000);
            m_start = ACE_OS::gettimeofday();
        }

        virtual ~StreamCaller()
        {
        }

        int handle_timeout(const ACE_Time_Value& tv, const void* arg)
        {
            while(m_start <= ACE_OS::gettimeofday() && m_interval != ACE_Time_Value::zero)
            {
                StreamCallback(&m_buffer[0]);
                m_start += m_interval;
            }

            return 0;
        }
        int svc()
        {
            m_reactor.owner (ACE_OS::thr_self ());
            int ret = m_reactor.schedule_timer(this, 0, m_interval, m_interval);
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
            m_streamer->player->StreamPlayerCbEnded();
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
        DuplexStreamer* m_streamer;
        std::vector<short> m_inputbuffer;

    public:
        StreamDuplexCallback(DuplexStreamer* streamer)
            : StreamCaller(*streamer, streamer->output_channels)
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
            DuplexCallback(*m_streamer, &m_inputbuffer[0], buffer);
            return true;
        }
    };

    template < typename SOUNDGROUP, typename INPUTSTREAMER, typename OUTPUTSTREAMER, typename DUPLEXSTREAMER >
    class SoundSystemBase : public SoundSystem
    {
    public:
        typedef ACE_Strong_Bound_Ptr < SOUNDGROUP, ACE_MT_SYNCH::RECURSIVE_MUTEX > soundgroup_t;
        typedef ACE_Strong_Bound_Ptr < INPUTSTREAMER, ACE_MT_SYNCH::RECURSIVE_MUTEX > inputstreamer_t;
        typedef ACE_Strong_Bound_Ptr < OUTPUTSTREAMER, ACE_MT_SYNCH::RECURSIVE_MUTEX > outputstreamer_t;
        typedef ACE_Strong_Bound_Ptr < DUPLEXSTREAMER, ACE_MT_SYNCH::RECURSIVE_MUTEX > duplexstreamer_t;

        //sndgrpid -> SoundGroup
        typedef std::map<int, soundgroup_t> soundgroups_t;
        typedef std::map<StreamCapture*, inputstreamer_t> inputstreamers_t;
        typedef std::map<StreamPlayer*, outputstreamer_t> outputstreamers_t;
        typedef std::map<StreamDuplex*, duplexstreamer_t> duplexstreamers_t;

    protected:
        SoundSystemBase()
        {
            standardSampleRates.push_back(8000);
            standardSampleRates.push_back(12000);
            standardSampleRates.push_back(16000);
            standardSampleRates.push_back(24000);
            standardSampleRates.push_back(32000);
            standardSampleRates.push_back(44100);
            standardSampleRates.push_back(48000);
        }
        virtual ~SoundSystemBase()
        {
            assert(m_nodev_streams.empty());
        }

        virtual bool Init() = 0;
        virtual void Close() = 0;

        typedef std::map<int, DeviceInfo> sounddevices_t;

        virtual void FillDevices(sounddevices_t& sounddevs) = 0;
        std::vector<int> standardSampleRates;

        virtual soundgroup_t NewSoundGroup() = 0;
        virtual void RemoveSoundGroup(soundgroup_t grp) = 0;

        //sndgrp_lock() must be acquired before calling
        soundgroup_t GetSoundGroup(int sndgrpid)
        {
            wguard_t g(sndgrp_lock());
            typename soundgroups_t::iterator i = m_sndgrps.find(sndgrpid);
            if(i != m_sndgrps.end())
                return i->second;
            return soundgroup_t();
        }

        virtual inputstreamer_t NewStream(StreamCapture* capture, 
                                          int inputdeviceid, int sndgrpid, 
                                          int samplerate, int channels,
                                          int framesize) = 0;
        virtual bool StartStream(inputstreamer_t streamer) = 0;
        virtual void CloseStream(inputstreamer_t streamer) = 0;

        //capture_lock() must be acquired before calling
        inputstreamer_t GetStream(StreamCapture* capture)
        {
            wguard_t g(capture_lock());
            typename inputstreamers_t::iterator ii = m_input_streamers.find(capture);
            if(ii != m_input_streamers.end())
                return ii->second;
            return inputstreamer_t();
        }

        virtual outputstreamer_t NewStream(StreamPlayer* player, int outputdeviceid,
                                           int sndgrpid, int samplerate, int channels, 
                                           int framesize) = 0;
        virtual void CloseStream(outputstreamer_t streamer) = 0;

        virtual bool StartStream(outputstreamer_t streamer) = 0;
        virtual bool StopStream(outputstreamer_t streamer) = 0;
        virtual bool IsStreamStopped(outputstreamer_t streamer) = 0;

        //players_lock() must be acquired before calling
        outputstreamer_t GetStream(StreamPlayer* player)
        {
            wguard_t g(players_lock());
            typename outputstreamers_t::iterator ii = m_output_streamers.find(player);
            if(ii != m_output_streamers.end())
                return ii->second;
            return outputstreamer_t();
        }
        //players_lock() must be acquired before calling
        void GetPlayers(int sndgrpid, std::vector<StreamPlayer*>& players)
        {
            wguard_t g(players_lock());
            typename outputstreamers_t::const_iterator ite;
            for(ite=m_output_streamers.begin();ite!=m_output_streamers.end();ite++)
            {
                if(ite->second->sndgrpid == sndgrpid)
                    players.push_back(ite->first);
            }
        }
        
        virtual duplexstreamer_t NewStream(StreamDuplex* duplex, int inputdeviceid,
                                           int outputdeviceid, int sndgrpid,
                                           int samplerate, int input_channels, 
                                           int output_channels, int framesize) = 0;
        virtual void CloseStream(duplexstreamer_t streamer) = 0;
        virtual bool StartStream(duplexstreamer_t streamer) = 0;
        duplexstreamer_t GetStream(StreamDuplex* duplex)
        {
            wguard_t g(duplex_lock());
            typename duplexstreamers_t::iterator ii = m_duplex_streamers.find(duplex);
            if(ii != m_duplex_streamers.end())
                return ii->second;
            return duplexstreamer_t();
        }

    private:

        soundgroups_t m_sndgrps;
        inputstreamers_t m_input_streamers;
        outputstreamers_t m_output_streamers;
        duplexstreamers_t m_duplex_streamers;

        ACE_Recursive_Thread_Mutex& sndgrp_lock() { return m_sndgrp_lock; }
        ACE_Recursive_Thread_Mutex& capture_lock() { return m_cap_lock; }
        ACE_Recursive_Thread_Mutex& players_lock() { return m_play_lock; }
        ACE_Recursive_Thread_Mutex& duplex_lock() { return m_dpx_lock; }

        ACE_Recursive_Thread_Mutex m_sndgrp_lock, m_cap_lock, m_play_lock, m_dpx_lock;

        inputstreamer_t NewVirtualStream(StreamCapture* capture, int sndgrpid, 
                                         int samplerate, int channels,
                                         int framesize)
        {
            DeviceInfo dev;
            bool b = GetDevice(SOUND_DEVICEID_VIRT, dev);
            assert(b);
            inputstreamer_t streamer(new INPUTSTREAMER(capture, sndgrpid, 
                                                       framesize, samplerate,
                                                       channels, dev.soundsystem));
            return streamer;
        }

        outputstreamer_t NewVirtualStream(StreamPlayer* player, int sndgrpid, 
                                          int samplerate, int channels, 
                                          int framesize)
        {
            DeviceInfo dev;
            bool b = GetDevice(SOUND_DEVICEID_VIRT, dev);
            assert(b);
            outputstreamer_t streamer(new OUTPUTSTREAMER(player, sndgrpid, 
                                                         framesize, samplerate,
                                                         channels, dev.soundsystem));
            return streamer;
        }
        
        duplexstreamer_t NewVirtualStream(StreamDuplex* duplex, int sndgrpid,
                                           int samplerate, int input_channels, 
                                           int output_channels, int framesize)
       {
            DeviceInfo dev;
            bool b = GetDevice(SOUND_DEVICEID_VIRT, dev);
            assert(b);
            duplexstreamer_t streamer(new DUPLEXSTREAMER(duplex, sndgrpid, 
                                                         framesize, samplerate, 
                                                         input_channels, 
                                                         output_channels,
                                                         dev.soundsystem));
            return streamer;
       }

        sounddevices_t m_sounddevs;
        ACE_Recursive_Thread_Mutex m_devs_lock;

        typedef ACE_Strong_Bound_Ptr<StreamCaller, ACE_MT_SYNCH::RECURSIVE_MUTEX> streamcallback_t;
        typedef std::map<SoundStreamer*, streamcallback_t> streamcallbacks_t;
        streamcallbacks_t m_nodev_streams;
        ACE_Recursive_Thread_Mutex m_nodev_lock;

    public:
        int OpenSoundGroup()
        {
            wguard_t g(sndgrp_lock());
            int sndgrpid = int(m_sndgrps.size()+1);
            while(sndgrpid && m_sndgrps.find(sndgrpid) != m_sndgrps.end())
                sndgrpid++;

            soundgroup_t sg = NewSoundGroup();
            if(sg.null())
                return 0;

            m_sndgrps[sndgrpid] = sg;
            return sndgrpid;
        }

        void RemoveSoundGroup(int sndgrpid)
        {
            wguard_t g(sndgrp_lock());
            soundgroup_t sg = GetSoundGroup(sndgrpid);
            if(!sg.null())
                RemoveSoundGroup(sg);
            m_sndgrps.erase(sndgrpid);
        }

        bool OpenInputStream(StreamCapture* capture, int inputdeviceid, 
                             int sndgrpid, int samplerate, int channels,
                             int framesize)
        {
            MYTRACE(ACE_TEXT("Opening StreamCapture %p, sample rate %d, channels %d, framesize %d, device #%d\n"), 
                    capture, samplerate, channels, framesize, inputdeviceid);

            inputstreamer_t streamer;
            if(inputdeviceid == SOUND_DEVICEID_VIRT)
                streamer = NewVirtualStream(capture, sndgrpid, 
                                            samplerate, channels, framesize);
            else
                streamer = NewStream(capture, inputdeviceid, sndgrpid, 
                                     samplerate, channels, framesize);

            if(streamer.null())
                return false;
            
            wguard_t g(capture_lock());
            m_input_streamers[capture] = streamer;

            if(streamer->IsVirtual())
            {
                StartVirtualStream(streamer);
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
            if(streamer.null())
                return false;

            if(streamer->IsVirtual())
            {
                StopVirtualStream(streamer.get());
            }
            else
            {
                CloseStream(streamer);
            }

            MYTRACE(ACE_TEXT("Closed StreamCapture %p\n"), capture);
            
            wguard_t g(capture_lock());
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
            if(outputdeviceid == SOUND_DEVICEID_VIRT)
                streamer = NewVirtualStream(player, sndgrpid, samplerate, 
                                   channels, framesize);
            else
                streamer = NewStream(player, outputdeviceid, sndgrpid, 
                                     samplerate, channels, framesize);
            if(streamer.null())
                return false;

            wguard_t g(players_lock());
            m_output_streamers[player] = streamer;

            return true;
        }
        bool CloseOutputStream(StreamPlayer* player)
        {
            outputstreamer_t streamer = GetStream(player);
            if(streamer.null())
                return false;

            if(streamer->IsVirtual())
            {
                StopVirtualStream(streamer.get());
            }
            else
            {
                CloseStream(streamer);
            }

            wguard_t g(players_lock());
            m_output_streamers.erase(player);

            MYTRACE(ACE_TEXT("Closed StreamPlayer %p\n"), player);

            return true;
        }
        bool StartStream(StreamPlayer* player)
        {
            outputstreamer_t streamer = GetStream(player);
            if(streamer.null())
                return false;
            
            if(streamer->IsVirtual())
            {
                StartVirtualStream(streamer);
                return true;
            }
            return StartStream(streamer);
        }
        bool StopStream(StreamPlayer* player)
        {
            outputstreamer_t streamer = GetStream(player);
            if(streamer.null())
                return false;
            
            if(streamer->IsVirtual())
            {
                StopVirtualStream(streamer.get());
                return true;
            }
            return StopStream(streamer);
        }
        bool IsStreamStopped(StreamPlayer* player)
        {
            outputstreamer_t streamer = GetStream(player);
            if(streamer.null())
                return true; //non-existing stream is same as stopped

            if(streamer->IsVirtual())
            {
                return IsVirtualStreamStopped(streamer.get());
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
            
            if(inputdeviceid == SOUND_DEVICEID_VIRT && outputdeviceid == SOUND_DEVICEID_VIRT)
            {
                streamer = NewVirtualStream(duplex, sndgrpid, samplerate, 
                                            input_channels, output_channels,
                                            framesize);
            }
            else if(inputdeviceid == SOUND_DEVICEID_VIRT || outputdeviceid == SOUND_DEVICEID_VIRT)
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

            if(streamer.null())
                return false;

            wguard_t g(duplex_lock());
            m_duplex_streamers[duplex] = streamer;

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
            if(streamer.null())
                return false;

            if(streamer->IsVirtual())
            {
                StopVirtualStream(streamer.get());
            }
            else
            {
                CloseStream(streamer);
            }

            wguard_t g(duplex_lock());
            m_duplex_streamers.erase(duplex);

            MYTRACE(ACE_TEXT("Closed StreamDuplex %p\n"), duplex);

            return true;
        }

        bool AddDuplexOutputStream(StreamDuplex* duplex,
                                   StreamPlayer* player)
        {
            duplexstreamer_t streamer = GetStream(duplex);
            if(streamer.null())
                return false;

            outputstreamer_t newstreamer(new OUTPUTSTREAMER(player, 
                                                            streamer->sndgrpid, 
                                                            streamer->framesize, 
                                                            streamer->samplerate,
                                                            streamer->output_channels,
                                                            streamer->output_soundsystem));
#if defined(_DEBUG)
            newstreamer->duplex = true;
#endif
            wguard_t g(players_lock());
            m_output_streamers[player] = newstreamer;

            wguard_t g3(streamer->players_mtx);
            //store in list of duplex players which will receive output-callback
            streamer->players.push_back(newstreamer.get());

            //set master volume so it's relative to master volume
            SetVolume(player, VOLUME_DEFAULT);

            return true;
        }

        bool RemoveDuplexOutputStream(StreamDuplex* duplex,
                                      StreamPlayer* player)
        {
            duplexstreamer_t streamer = GetStream(duplex);
            if(streamer.null())
                return false;

            wguard_t g2(streamer->players_mtx);
            for(size_t i=0;i<streamer->players.size();i++)
            {
                if(streamer->players[i]->player == player)
                {
                    streamer->players.erase(streamer->players.begin()+i);
                }
            }
            g2.release();

            //simulate to player that this was the last callback
            player->StreamPlayerCbEnded();

            wguard_t g(players_lock());
            m_output_streamers.erase(player);
            return true;
        }

        virtual bool SetMasterVolume(int sndgrpid, int volume)
        {
            if(volume > VOLUME_MAX) volume = VOLUME_MAX;
            else if(volume < VOLUME_MIN) volume = VOLUME_MIN;

            {
                soundgroup_t sndgrp = GetSoundGroup(sndgrpid);
                if(sndgrp.null())
                    return false;
                sndgrp->mastervolume = volume;
            }

            std::vector<StreamPlayer*> players;
            GetPlayers(sndgrpid, players);
            for(size_t i=0;i<players.size();i++)
            {
                outputstreamer_t streamer = GetStream(players[i]);
                if(!streamer.null())
                    SetVolume(players[i], streamer->volume);
            }
            return true;
        }
        virtual int GetMasterVolume(int sndgrpid)
        {
            soundgroup_t sndgrp = GetSoundGroup(sndgrpid);
            if(sndgrp.null())
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
            if(sndgrp.null())
                return false;
            
            return sndgrp->muteall;
        }

        virtual bool MuteAll(int sndgrpid, bool mute)
        {
            soundgroup_t sndgrp = GetSoundGroup(sndgrpid);
            if(sndgrp.null())
                return false;
            sndgrp->muteall = mute;

            std::vector<StreamPlayer*> players;
            GetPlayers(sndgrpid, players);
            for(size_t i=0;i<players.size();i++)
            {
                outputstreamer_t streamer = GetStream(players[i]);
                if(!streamer.null() && !mute)
                    SetVolume(players[i], streamer->volume);
            }

            return true;
        }

        bool RestartSoundSystem()
        {
            wguard_t g1(capture_lock());
            wguard_t g2(players_lock());
            wguard_t g3(duplex_lock());

            if(m_input_streamers.size() || m_output_streamers.size() || m_duplex_streamers.size())
                return false;

            Close();

            {
                wguard_t g(m_devs_lock);
                m_sounddevs.clear();
            }

            return Init();
        }

        //input and output devices
        
        void RefreshDevices()
        {
            wguard_t g(m_devs_lock);
            m_sounddevs.clear();

            FillDevices(m_sounddevs);
            AddVirtualDevice();
        }
        
        virtual bool GetSoundDevices(devices_t& snddevices)
        {
            wguard_t g(m_devs_lock);

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
            wguard_t g(m_devs_lock);

            sounddevices_t::const_iterator ii = m_sounddevs.find(id);
            if(ii != m_sounddevs.end())
            {
                dev = ii->second;
                return true;
            }
            return false;
        }

        virtual void SetVolume(StreamPlayer* player, int volume)
        {
            if(volume > VOLUME_MAX)
                volume = VOLUME_MAX;
            else if(volume < VOLUME_MIN)
                volume = VOLUME_MIN;

            outputstreamer_t streamer = GetStream(player);
            if(streamer.null())
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
            if(streamer.null())
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
            if(streamer.null())
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
            if(streamer.null())
                return false;

            return streamer->mute;
        }


    protected:
        void StartVirtualStream(inputstreamer_t streamer)
        {
            assert(streamer->IsVirtual());
            streamcallback_t scc(new StreamCaptureCallback(streamer.get()));
            wguard_t g(m_nodev_lock);
            m_nodev_streams[streamer.get()] = scc;

            int ret = scc->activate();
            MYTRACE_COND(ret < 0, ACE_TEXT("Failed to activate StreamCaller\n"));
        }

        void StartVirtualStream(outputstreamer_t streamer)
        {
            assert(streamer->IsVirtual());
            streamcallback_t scc(new StreamPlayerCallback(streamer.get()));
            wguard_t g(m_nodev_lock);
            m_nodev_streams[streamer.get()] = scc;

            int ret = scc->activate();
            MYTRACE_COND(ret < 0, ACE_TEXT("Failed to activate StreamCaller\n"));
        }

        void StartVirtualStream(duplexstreamer_t streamer)
        {
            assert(streamer->IsVirtual());
            streamcallback_t scc(new StreamDuplexCallback(streamer.get()));
            wguard_t g(m_nodev_lock);
            m_nodev_streams[streamer.get()] = scc;

            int ret = scc->activate();
            MYTRACE_COND(ret < 0, ACE_TEXT("Failed to activate StreamCaller\n"));
        }

        void StopVirtualStream(SoundStreamer* streamer)
        {
            wguard_t g(m_nodev_lock);
            m_nodev_streams.erase(streamer);
        }

        bool IsVirtualStreamStopped(SoundStreamer* streamer)
        {
            wguard_t g(m_nodev_lock);
            return m_nodev_streams.find(streamer) == m_nodev_streams.end();
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
            device.id = SOUND_DEVICEID_VIRT;
            m_sounddevs[device.id] = device;
        }

    };

}

#endif
