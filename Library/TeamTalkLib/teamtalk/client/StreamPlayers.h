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

#include "avstream/AudioResampler.h"
#include "avstream/SoundSystem.h"
#include "codec/MediaUtil.h"
#include "myace/MyACE.h"
#include "teamtalk/Common.h"
#include "teamtalk/PacketHelper.h"
#include "teamtalk/PacketLayout.h"

#if defined(ENABLE_SPEEX)
#include "codec/SpeexDecoder.h"
#endif
#if defined(ENABLE_OPUS)
#include "codec/OpusDecoder.h"
#endif
#if defined(ENABLE_VPX)
#include "codec/VpxDecoder.h"
#endif

#include <ace/Message_Block.h>
#include <ace/Recursive_Thread_Mutex.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <vector>

constexpr auto STOPPED_TALKING_DELAY = 500; //msec

namespace teamtalk {

    struct encframe
    {
        std::vector<char> enc_frames;
        std::vector<uint16_t> enc_frame_sizes;
        uint32_t timestamp = 0;
        int stream_id = 0;

        encframe() = default;
        void Reset()
        {
            timestamp = 0;
            enc_frames.clear();
            enc_frame_sizes.clear();
            stream_id = 0;
        }
    };

    using useraudio_callback_t = std::function< void (int userid, StreamType stream_type, const media::AudioFrame& frm) >;

    class AudioPlayer
        : public soundsystem::StreamPlayer
    {
    public:
        AudioPlayer(int userid, StreamType stream_type, soundsystem::soundsystem_t sndsys,
                    useraudio_callback_t audio_cb, const AudioCodec& codec,
                    audio_resampler_t& resampler);
        ~AudioPlayer() override;

        //returns reassembled AudioPacket if 'new_audpkt' has fragments
        audiopacket_t QueuePacket(const AudioPacket& new_audpkt);

        bool StreamPlayerCb(const soundsystem::OutputStreamer& streamer,
                                    short* output_buffer, int n_samples) override;

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

        int m_userid = 0;
        StreamType m_streamtype = STREAMTYPE_NONE;
        soundsystem::soundsystem_t m_sndsys;
        useraudio_callback_t m_audio_callback;
        bool m_talking = false;
        AudioCodec m_codec;
        //start/stop playback attributes
        uint32_t m_last_playback = 0;  //local timestamp for last playback
        uint32_t m_play_stopped_delay = STOPPED_TALKING_DELAY; //wait for how long to stop playing
        uint32_t m_played_packet_time = 0; //time of last packet to be send to audio buffer
        //AudioMuxer attributes
        ACE_UINT32 m_samples_played = 0;
        //Resample buffer
        audio_resampler_t m_resampler;
        std::vector<short> m_resample_buffer;
        StereoMask m_stereo = STEREO_BOTH;
        bool m_no_recording = false;
        //id of the stream currently being played
        int m_stream_id = 0;

        //stats
        int m_audiopackets_recv = 0;
        int m_audiopacket_lost = 0;

        //received frames
        using enc_frames_t = std::map<uint16_t, encframe, W16LessComp>;
        enc_frames_t m_buffer;
        int m_buffer_msec = 0;
        //current packet number being played
        uint16_t m_play_pkt_no = 0;

        //container for fragmented packets
        //packet no -> fragments
        using fragments_queue_t = std::map<uint16_t, audiofragments_t>;
        fragments_queue_t m_audfragments;
        ACE_Recursive_Thread_Mutex m_mutex;
    };

    using audio_player_t = std::shared_ptr< AudioPlayer >;

#if defined(ENABLE_SPEEX)
    class SpeexPlayer : public AudioPlayer
    {
    public:
        SpeexPlayer(int userid, StreamType stream_type, soundsystem::soundsystem_t sndsys,
                    useraudio_callback_t audio_cb, const AudioCodec& codec,
                    audio_resampler_t resampler);
        ~SpeexPlayer() override;

        bool DecodeFrame(const encframe& enc_frame,
                         short* output_buffer, int n_samples) override;

    protected:
        void Reset() override;
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
        ~OpusPlayer() override;

        bool DecodeFrame(const encframe& enc_frame,
                         short* output_buffer, int n_samples) override;

    protected:
        void Reset() override;
        OpusDecode m_decoder;
    };
#endif

    using fragmentnums_t = std::vector<uint16_t>;

#if defined(ENABLE_VPX)

    class WebMPlayer
    {
    public:
        WebMPlayer(int userid, int stream_id);
        ~WebMPlayer();

        bool AddPacket(const VideoPacket& packet, size_t* n_packets = nullptr);
        ACE_Message_Block* GetNextFrame(const uint32_t* timestamp = nullptr);
        bool GetNextFrameTime(uint32_t* tm);

        static VideoCodec GetVideoCodec() ;
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

        void DumpFragments();

        int m_userid = 0;
        int m_video_pkts_recv = 0;
        int m_videoframes_recv = 0;
        int m_videoframes_lost = 0;
        int m_videoframes_dropped = 0;

        uint8_t m_videostream_id = 0;
        uint32_t m_packet_no = 0;
        //local time stamp of latest packet to arrive
        uint32_t m_local_timestamp = 0;

        struct enc_frame
        {
            std::vector<char> enc_data;
            uint32_t packet_no = 0;
            enc_frame() = default;
        };

        //packetno -> video fragments (sorted by UINT32 wrap)
        using reassm_queue_t = std::map<uint32_t, video_fragments_t, W32LessComp>;
        reassm_queue_t m_video_fragments;

        //timestamp -> enc video frame (sorted by UINT32 wrap)
        using video_frames_t = std::map<uint32_t, enc_frame, W32LessComp >;
        video_frames_t m_video_frames;

        VpxDecoder m_decoder;
        bool m_decoder_ready = false;

        ACE_Recursive_Thread_Mutex m_mutex;
    };

    using webm_player_t = std::shared_ptr< WebMPlayer >;

#endif /* ENABLE_VPX */
} // namespace teamtalk

#endif
