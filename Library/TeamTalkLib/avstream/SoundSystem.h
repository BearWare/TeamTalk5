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

#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

#include <myace/MyACE.h>

#include <map>
#include <memory>
#include <mutex>
#include <assert.h>

namespace soundsystem {

#define VOLUME_MAX 32000
#define VOLUME_DEFAULT 1000
#define VOLUME_MIN 0

    enum SoundAPI
    {
        SOUND_API_NOSOUND = 0,
        SOUND_API_WINMM = 1,
        SOUND_API_DSOUND = 2,
        SOUND_API_ALSA = 3,
        SOUND_API_COREAUDIO = 4,
        SOUND_API_WASAPI = 5,
        SOUND_API_WDMKS = 6,
        SOUND_API_OPENSLES_ANDROID = 7,
        SOUND_API_AUDIOUNIT = 8,
        SOUND_API_AUDIOTOOLKIT = 9,
    };

    enum SoundDeviceID
    {
        SOUND_DEVICEID_INVALID           = /* 0xFFFFFFFF */ -1,
        SOUND_DEVICEID_VIRTUAL           = /* 0x000007BA */ 1978,
        
        /* Sound devices are from 0 => 0x7FF */
        SOUND_DEVICEID_MASK              = 0x000007FF,
        /* Flag for a shared sound device. The original sound device
         * is in the mask SOUND_DEVICEID_MASK */
        SOUND_DEVICE_SHARED_FLAG         = 0x00000800
    };

    class StreamCapture;
    class StreamPlayer;
    class StreamDuplex;

    struct DeviceInfo
    {
        ACE_TString devicename;
        SoundAPI soundsystem;
        int id;
        ACE_TString deviceid;
#if defined(WIN32)
        int wavedeviceid;
#endif
        bool supports3d;
        int max_input_channels;
        int max_output_channels;
        int default_samplerate;
        std::set<int> input_samplerates;
        std::set<int> output_samplerates;
        std::set<int> input_channels;
        std::set<int> output_channels;

        bool SupportsInputFormat(int channels, int samplerate) const
        {
            return input_channels.find(channels) != input_channels.end() &&
               input_samplerates.find(samplerate) != input_samplerates.end();
        }

        bool SupportsOutputFormat(int channels, int samplerate) const
        {
            return output_channels.find(channels) != output_channels.end() &&
               output_samplerates.find(samplerate) != output_samplerates.end();
        }

        int MaxInputSampleRate() const
        {
            if(input_samplerates.empty())
                return 0;

            std::set<int>::const_iterator israte = input_samplerates.end();
            return *(--israte);
        }

        int MaxOutputSampleRate() const
        {
            if(output_samplerates.empty())
                return 0;

            std::set<int>::const_iterator osrate = output_samplerates.end();
            return *(--osrate);
        }

        int GetSupportedInputChannels(int preffered_channels) const
        {
            if(input_channels.empty())
                return 0;
            if(input_channels.find(preffered_channels) != input_channels.end())
                return preffered_channels;
            std::set<int>::const_iterator ii = input_channels.end();
            return *(--ii);
        }

        int GetSupportedOutputChannels(int preffered_channels) const
        {
            if(output_channels.empty())
                return 0;
            if(output_channels.find(preffered_channels) != output_channels.end())
                return preffered_channels;
            std::set<int>::const_iterator ii = output_channels.end();
            return *(--ii);
        }

        DeviceInfo()
        {
            id = SOUND_DEVICEID_INVALID;
            max_input_channels = max_output_channels = 0;
            default_samplerate = 0;
            supports3d = false;
            soundsystem = SOUND_API_NOSOUND;
#if defined(WIN32)
            wavedeviceid = -1;
#endif
        }
    };

    typedef std::vector< DeviceInfo > devices_t;

    struct SoundGroup
    {
        int mastervolume;
        bool muteall;
        SoundGroup()
        {
            mastervolume = VOLUME_DEFAULT;
            muteall = false;
        }
    };

    struct SoundStreamer
    {
        int framesize;
        int samplerate;
        int sndgrpid;

        SoundStreamer(int sg, int fs, int sr)
            : framesize(fs)
            , samplerate(sr)
            , sndgrpid(sg) { }
    };

    struct InputStreamer : public SoundStreamer
    {
        StreamCapture* recorder;
        SoundAPI soundsystem;
        int inputdeviceid;
        int channels;
#if defined(DEBUG)
        bool duplex;
#endif
        InputStreamer(StreamCapture* r, int sg, int fs, int sr, int chs, SoundAPI sndsys, int devid)
            : SoundStreamer(sg, fs, sr)
            , recorder(r)
            , soundsystem(sndsys)
            , inputdeviceid(devid)
            , channels(chs)
#if defined(DEBUG)
            , duplex(false)
#endif
            { }

        virtual ~InputStreamer()
        {
            MYTRACE(ACE_TEXT("~InputStreamer() - %p for StreamCapture %p\n"), this, recorder);
        }
        bool IsVirtual() const { return inputdeviceid == SOUND_DEVICEID_VIRTUAL; }
        bool IsShared() const { return (inputdeviceid & SOUND_DEVICE_SHARED_FLAG) != 0; }
    };

    struct OutputStreamer : public SoundStreamer
    {
        StreamPlayer* player;
        SoundAPI soundsystem;
        int outputdeviceid;
        int channels;
        int volume;
        bool mute;
        bool autoposition;
#if defined(DEBUG)
        bool duplex;
#endif
        OutputStreamer(StreamPlayer* p, int sg, int fs, int sr, int chs, SoundAPI sndsys, int devid)
            : SoundStreamer(sg, fs, sr)
            , player(p)
            , soundsystem(sndsys)
            , outputdeviceid(devid)
            , channels(chs)
            , volume(VOLUME_DEFAULT)
            , mute(false)
            , autoposition(true)
#if defined(DEBUG)
            , duplex(false)
#endif
            { }
        virtual ~OutputStreamer()
        {
            MYTRACE(ACE_TEXT("~OutputStreamer() - %p for StreamPlayer %p\n"), this, player);
        }
        bool IsVirtual() const { return outputdeviceid == SOUND_DEVICEID_VIRTUAL; }
        bool IsShared() const { return (outputdeviceid & SOUND_DEVICE_SHARED_FLAG) != 0; }
    };

    struct DuplexStreamer : public SoundStreamer
    {
        StreamDuplex* duplex;
        std::vector<OutputStreamer*> players; // only modify if holding 'players_mtx'
        std::recursive_mutex players_mtx;
        int input_channels;
        int output_channels;
        SoundAPI output_soundsystem;
        int inputdeviceid, outputdeviceid;
        std::vector<short> tmpOutputBuffer;

        DuplexStreamer(StreamDuplex* d, int sg, int fs, int sr, int inchs, int outchs,
                       SoundAPI out_sndsys, int indevid, int outdevid)
            : SoundStreamer(sg, fs, sr)
            , duplex(d)
            , input_channels(inchs)
            , output_channels(outchs)
            , output_soundsystem(out_sndsys)
            , inputdeviceid(indevid)
            , outputdeviceid(outdevid)
        {
            tmpOutputBuffer.resize(outchs * fs);
        }

        virtual ~DuplexStreamer()
        {
            assert(players.empty());
            MYTRACE(ACE_TEXT("~DuplexStreamer() - %p for StreamDuplex %p\n"), this, duplex);
        }

        bool IsVirtual() const { return inputdeviceid == SOUND_DEVICEID_VIRTUAL &&
                                        outputdeviceid == SOUND_DEVICEID_VIRTUAL; }
    };


    //interface for objects which will receive recorded data
    class StreamCapture
    {
    public:
        virtual ~StreamCapture() {}
        virtual void StreamCaptureCb(const InputStreamer& streamer,
                                     const short* buffer, int samples) = 0;
    };

    class StreamPlayer
    {
    public:
        virtual ~StreamPlayer() {}
        virtual bool StreamPlayerCb(const OutputStreamer& streamer, 
                                    short* buffer, int samples) = 0;
    };

    class StreamDuplex
    {
    public:
        virtual ~StreamDuplex() {}

        virtual void StreamDuplexEchoCb(const DuplexStreamer& streamer,
                                        const short* input_buffer, 
                                        const short* prev_output_buffer, int samples){}
        virtual void StreamDuplexCb(const DuplexStreamer& streamer,
                                    const short* input_buffer, 
                                    short* output_buffer, int samples){}
    };

    class SoundSystem
    {
    public:
        virtual ~SoundSystem() {}

        virtual bool GetDefaultDevices(int& inputdeviceid,
                                       int& outputdeviceid) = 0;
        virtual bool GetDefaultDevices(SoundAPI sndsys,
                                       int& inputdeviceid,
                                       int& outputdeviceid) = 0;
        
        //sound group members
        virtual int OpenSoundGroup() = 0;
        virtual void RemoveSoundGroup(int sndgrpid) = 0;

        //input members
        virtual bool OpenInputStream(StreamCapture* capture, int inputdeviceid, 
                                     int sndgrpid, int samplerate, int channels,
                                     int framesize) = 0;
        virtual bool CloseInputStream(StreamCapture* capture) = 0;

        //output members
        virtual bool OpenOutputStream(StreamPlayer* player, int outputdeviceid,
                                      int sndgrpid, int samplerate, int channels, 
                                      int framesize) = 0;
        virtual bool CloseOutputStream(StreamPlayer* player) = 0;
        virtual bool StartStream(StreamPlayer* player) = 0;
        virtual bool StopStream(StreamPlayer* player) = 0;
        virtual bool IsStreamStopped(StreamPlayer* player) = 0;

        //duplex members
        virtual bool OpenDuplexStream(StreamDuplex* duplex, int inputdeviceid,
                                      int outputdeviceid, int sndgrpid,
                                      int samplerate, int input_channels, 
                                      int output_channels, int framesize) = 0;
        virtual bool CloseDuplexStream(StreamDuplex* duplex) = 0;
        virtual bool AddDuplexOutputStream(StreamDuplex* duplex,
                                           StreamPlayer* player) = 0;
        virtual bool RemoveDuplexOutputStream(StreamDuplex* duplex,
                                              StreamPlayer* player) = 0;

        virtual bool SetMasterVolume(int sndgrpid, int volume) = 0;
        virtual int GetMasterVolume(int sndgrpid) = 0;
        virtual bool SetAutoPositioning(int sndgrpid, bool enable) = 0;
        virtual bool IsAutoPositioning(int sndgrpid) = 0;
        virtual bool AutoPositionPlayers(int sndgrpid, bool all_players) = 0;
        virtual bool IsAllMute(int sndgrpid) = 0;
        virtual bool MuteAll(int sndgrpid, bool mute) = 0;
        virtual bool RestartSoundSystem() = 0;
        virtual bool GetSoundDevices(devices_t& snddevices) = 0;
        virtual bool CheckInputDevice(int inputdeviceid) = 0;
        virtual bool CheckOutputDevice(int outputdeviceid) = 0;
        virtual bool SupportsInputFormat(int inputdeviceid,
                                         int input_channels,
                                         int samplerate) = 0;
        virtual bool SupportsOutputFormat(int outputdeviceid,
                                          int output_channels,
                                          int samplerate) = 0;
        virtual bool GetDevice(int id, DeviceInfo& dev) = 0;
        virtual void SetVolume(StreamPlayer* player, int volume) = 0;
        virtual int GetVolume(StreamPlayer* player) = 0;
        virtual void SetAutoPositioning(StreamPlayer* player, bool enable) = 0;
        virtual bool IsAutoPositioning(StreamPlayer* player) = 0;
        virtual bool SetPosition(StreamPlayer* player, float x, float y, float z) = 0;
        virtual bool GetPosition(StreamPlayer* player, float& x, float& y, float& z) = 0;
        virtual void SetMute(StreamPlayer* player, bool mute) = 0;
        virtual bool IsMute(StreamPlayer* player) = 0;
    };


    typedef std::shared_ptr< SoundSystem > soundsystem_t;
    soundsystem_t GetInstance();

}

#endif

