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

#ifndef STREAMPLAYERS_H
#define STREAMPLAYERS_H

#include <myace/MyACE.h>

#include <avstream/SoundSystem.h>
#include <avstream/AudioResampler.h>

#include <teamtalk/Common.h>
#include <teamtalk/PacketLayout.h>
#include <teamtalk/PacketHelper.h>

#if defined(ENABLE_SPEEX)
#include <codec/SpeexDecoder.h>
#include <codec/SpeexEncoder.h>
#endif
#if defined(ENABLE_OPUS)
#include <codec/OpusDecoder.h>
#endif
#if defined(ENABLE_VPX)
#include <codec/VpxDecoder.h>
#endif
#include "VideoThread.h"

#include <memory>

#define STOPPED_TALKING_DELAY 500 //msec

namespace teamtalk {

    struct encframe
    {
        std::vector<char> enc_frames;
        std::vector<uint16_t> enc_frame_sizes;
        uint32_t timestamp;
        int stream_id;

        encframe() : timestamp(0), stream_id(0) {}
        void reset()
        {
            timestamp = 0;
            enc_frames.clear();
            enc_frame_sizes.clear();
            stream_id = 0;
        }
    };

    typedef std::function< void (int userid, StreamType stream_type, const media::AudioFrame& frm) > useraudio_callback_t;

    class AudioPlayer
        : public soundsystem::StreamPlayer
    {
    public:
        AudioPlayer(int userid, StreamType stream_type, soundsystem::soundsystem_t sndsys,
                    useraudio_callback_t audio_cb, const AudioCodec& codec,
                    audio_resampler_t& resampler);
        virtual ~AudioPlayer();

        //returns reassembled AudioPacket if 'new_audpkt' has fragments
        audiopacket_t QueuePacket(const AudioPacket& new_audpkt);

        virtual bool StreamPlayerCb(const soundsystem::OutputStreamer& streamer,
                                    short* output_buffer, int n_samples);

        bool PlayBuffer(short* output_buffer, int n_samples);
        virtual bool DecodeFrame(const encframe& enc_frame,
                                 short* output_buffer, int n_samples) = 0;

        uint32_t GetLastPlaytime() const { return m_last_playback; }
        uint16_t GetPlayedPacketNo() const { return m_play_pkt_no; }
        uint32_t GetPlayedPacketTime() const { return m_played_packet_time; }
        int GetBufferedAudioMSec();

        bool IsTalking() const { return m_talking; }
        void SetStoppedTalkingDelay(uint32_t msec) { m_play_stopped_delay = msec; }

        void SetStereoMask(StereoMask stereo) { m_stereo = stereo; }
        void SetNoRecording(bool no_recording) { m_no_recording = no_recording; }
        bool IsRecordingAllowed() const { return !m_no_recording; }

        void SetAudioBufferSize(int msec);

        int GetNumAudioPacketsRecv(bool reset);
        int GetNumAudioPacketsLost(bool reset);

        const AudioCodec& GetAudioCodec() const { return m_codec; }

    protected:
        void CleanUpAudioFragments(uint16_t too_old_packet_no);

        void AddPacket(const AudioPacket& packet);
        virtual void Reset();

        int m_userid;
        StreamType m_streamtype;
        soundsystem::soundsystem_t m_sndsys;
        useraudio_callback_t m_audio_callback;
        bool m_talking;
        AudioCodec m_codec;
        //start/stop playback attributes
        uint32_t m_last_playback;  //local timestamp for last playback
        uint32_t m_play_stopped_delay; //wait for how long to stop playing
        uint32_t m_played_packet_time; //time of last packet to be send to audio buffer
        //AudioMuxer attributes
        ACE_UINT32 m_samples_played;
        //Resample buffer
        audio_resampler_t m_resampler;
        std::vector<short> m_resample_buffer;
        StereoMask m_stereo;
        bool m_no_recording;
        //id of the stream currently being played
        int m_stream_id;

        //stats
        int m_audiopackets_recv;
        int m_audiopacket_lost;

        //received frames
        typedef std::map<uint16_t, encframe, w16_less_comp> enc_frames_t;
        enc_frames_t m_buffer;
        int m_buffer_msec;
        //current packet number being played
        uint16_t m_play_pkt_no;

        //container for fragmented packets
        //packet no -> fragments
        typedef std::map<uint16_t, audiofragments_t> fragments_queue_t;
        fragments_queue_t m_audfragments;
        ACE_Recursive_Thread_Mutex m_mutex;
    };

    typedef std::shared_ptr< AudioPlayer > audio_player_t;

#if defined(ENABLE_SPEEX)
    class SpeexPlayer : public AudioPlayer
    {
    public:
        SpeexPlayer(int userid, StreamType stream_type, soundsystem::soundsystem_t sndsys,
                    useraudio_callback_t audio_cb, const AudioCodec& codec,
                    audio_resampler_t resampler);
        virtual ~SpeexPlayer();

        bool DecodeFrame(const encframe& enc_frame,
                         short* output_buffer, int n_samples);

    protected:
        void Reset();
        SpeexDecoder m_decoder;
    };
#endif

#if defined(ENABLE_OPUS)
    class OpusPlayer : public AudioPlayer
    {
    public:
        OpusPlayer(int userid, StreamType stream_type, soundsystem::soundsystem_t sndsys,
                   useraudio_callback_t audio_cb, const AudioCodec& codec,
                   audio_resampler_t resampler);
        virtual ~OpusPlayer();

        bool DecodeFrame(const encframe& enc_frame,
                         short* output_buffer, int n_samples);

    protected:
        void Reset();
        OpusDecode m_decoder;
    };
#endif

    typedef std::vector<uint16_t> fragmentnums_t;

#if defined(ENABLE_VPX)

    class WebMPlayer
    {
    public:
        WebMPlayer(int userid, int stream_id);
        ~WebMPlayer();

        bool AddPacket(const VideoPacket& packet, size_t* n_packets = NULL);
        ACE_Message_Block* GetNextFrame(uint32_t* timestamp = NULL);
        bool GetNextFrameTime(uint32_t* tm);

        VideoCodec GetVideoCodec() const;
        media::VideoFormat GetVideoFormat() const;

        int GetVideoPacketRecv(bool reset);
        int GetVideoFramesRecv(bool reset);
        int GetVideoFramesLost(bool reset);
        int GetVideoFramesDropped(bool reset);

        uint8_t GetStreamID() const { return m_videostream_id; }

        uint32_t GetLastTimeStamp() const { return m_local_timestamp; }

    private:
        void ProcessVideoPacket(const VideoPacket& packet);
        void RemoveObsoletePackets();

        void dumpFragments();

        int m_userid;
        int m_video_pkts_recv;
        int m_videoframes_recv;
        int m_videoframes_lost;
        int m_videoframes_dropped;

        uint8_t m_videostream_id;
        uint32_t m_packet_no;
        //local time stamp of latest packet to arrive
        uint32_t m_local_timestamp;

        struct enc_frame
        {
            std::vector<char> enc_data;
            uint32_t packet_no;
            enc_frame() : packet_no(0) {}
        };

        //packetno -> video fragments (sorted by UINT32 wrap)
        typedef std::map<uint32_t, video_fragments_t, w32_less_comp> reassm_queue_t;
        reassm_queue_t m_video_fragments;

        //timestamp -> enc video frame (sorted by UINT32 wrap)
        typedef std::map<uint32_t, enc_frame, w32_less_comp > video_frames_t;
        video_frames_t m_video_frames;

        VpxDecoder m_decoder;
        bool m_decoder_ready;

        ACE_Recursive_Thread_Mutex m_mutex;
    };

    typedef std::shared_ptr< WebMPlayer > webm_player_t;

#endif /* ENABLE_VPX */
}

#endif
