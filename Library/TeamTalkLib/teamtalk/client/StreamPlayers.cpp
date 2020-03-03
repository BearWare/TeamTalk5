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

#include "StreamPlayers.h"
#include <teamtalk/CodecCommon.h>
#include <teamtalk/PacketHelper.h>
#include <teamtalk/ttassert.h>
#include <codec/MediaUtil.h>

#define DEBUG_PLAYBACK 0

using namespace media;

namespace teamtalk {

AudioPlayer::AudioPlayer(int userid, StreamType stream_type,
                         useraudio_callback_t audio_cb, const AudioCodec& codec,
                         audio_resampler_t& resampler)
: m_userid(userid)
, m_streamtype(stream_type)
, m_talking(false)
, m_codec(codec)
, m_last_playback(0)
, m_play_stopped_delay(STOPPED_TALKING_DELAY)
, m_played_packet_time(0)
, m_audio_callback(audio_cb)
, m_samples_played(0)
, m_resampler(resampler)
, m_stereo(STEREO_BOTH)
, m_no_recording(false)
, m_stream_id(0)
, m_audiopackets_recv(0)
, m_audiopacket_lost(0)
{
    MYTRACE(ACE_TEXT("New AudioPlayer() - #%d\n"), m_userid);

    Reset();

    int input_channels = GetAudioCodecChannels(m_codec);
    if(GetAudioCodecSimulateStereo(m_codec))
        input_channels = 2;
    int input_samples = GetAudioCodecCbSamples(m_codec);
    if (m_resampler)
        m_resample_buffer.resize(input_samples*input_channels);

    SetAudioBufferSize(GetAudioCodecCbMillis(m_codec) * 4);
}

AudioPlayer::~AudioPlayer()
{
    //store in muxer (if enabled)
    media::AudioFrame frm(GetAudioCodecAudioFormat(m_codec), nullptr, 0);
    frm.sample_no = m_samples_played;
    frm.streamid = m_stream_id;
    m_audio_callback(m_userid, m_streamtype, frm);
    MYTRACE(ACE_TEXT("~AudioPlayer() - %p - #%d\n"), this, m_userid);
}

audiopacket_t AudioPlayer::QueuePacket(const AudioPacket& new_audpkt)
{
    const AudioPacket* audpkt;
    audiopacket_t ptr_audpkt; //ensures a reassembled packet gets deleted

    if(!new_audpkt.HasFragments())
        audpkt = &new_audpkt;
    else
    {
        wguard_t g(m_mutex);

        uint8_t fragno = 0, frag_cnt = 0;
        uint16_t packetno = new_audpkt.GetPacketNumberAndFragNo(fragno, &frag_cnt);
        MYTRACE_COND(fragno == AudioPacket::INVALID_FRAGMENT_NO,
            ACE_TEXT("User #%d, received fragmented packet #%d with no fragno\n"), 
                     m_userid, packetno);
        if(fragno == AudioPacket::INVALID_FRAGMENT_NO)
            return ptr_audpkt;

        //MYTRACE(ACE_TEXT("User #%d, received pkt no %d, frag no %d/%d\n"), m_userid, int(packetno), int(fragno), int(frag_cnt));

        //clean out fragments which were never played
        uint16_t playing_pkt_no = GetPlayedPacketNo();
        if (playing_pkt_no)
            CleanUpAudioFragments(playing_pkt_no-1);

        //copy packet and queue it
        audiopacket_t q_audpkt(new AudioPacket(new_audpkt));
        fragments_queue_t::iterator ii=m_audfragments.find(packetno);
        if(ii != m_audfragments.end())
            ii->second[fragno] = q_audpkt;
        else
            m_audfragments[packetno][fragno] = q_audpkt;

        ii = m_audfragments.find(packetno);

        //try to reassemble
        ptr_audpkt = ReassembleAudioPacket(ii->second, m_codec);

        if (!ptr_audpkt)
            return ptr_audpkt;//couldn't reassemble

        //erase what we reassembled
        m_audfragments.erase(ii);

        //make 'audpkt' point to our reassembled packet
        audpkt = ptr_audpkt.get();
    }

    TTASSERT(!audpkt->HasFragments());

    AddPacket(*audpkt);

    return ptr_audpkt;
}

void AudioPlayer::CleanUpAudioFragments(uint16_t too_old_packet_no)
{
    //throw away what couldn't be reassembled (what has already been
    //played by "newer" sound
    fragments_queue_t::iterator ii=m_audfragments.begin();
    while(ii != m_audfragments.end())
    {
        if (PACKETNO_GEQ(too_old_packet_no, ii->first))
        {
            MYTRACE(ACE_TEXT("Packet #%d wasn't reassembled, ejected!\n"), int(ii->first));
            m_audfragments.erase(ii++);
        }
        else ii++;
    }
}

void AudioPlayer::Reset()
{
    wguard_t g(m_mutex);
    m_audfragments.clear();

    m_buffer.clear();
    m_play_pkt_no = 0;
    m_stream_id = 0;

    //do not reset play time since they're used by ClientUser to check
    //how long the player has been inactive.
}

bool AudioPlayer::StreamPlayerCb(const soundsystem::OutputStreamer& streamer,
                                 short* output_buffer, int output_samples)
{
    int input_channels = GetAudioCodecChannels(m_codec);
    if(GetAudioCodecSimulateStereo(m_codec))
        input_channels = 2;
    int input_samplerate = GetAudioCodecSampleRate(m_codec);
    int input_samples = GetAudioCodecCbSamples(m_codec);

    short* tmp_output_buffer;
    bool played;
    if (m_resampler)
        tmp_output_buffer = &m_resample_buffer[0];
    else
        tmp_output_buffer = output_buffer;

    played = PlayBuffer(tmp_output_buffer, input_samples);

    if(played)
    {
        m_last_playback = GETTIMESTAMP();

        if(input_channels == 2)
        {
            //If in stereo then choose which channels to output audio to
            SelectStereo(m_stereo, tmp_output_buffer, input_samples);
        }

        m_talking = true;
    }
    else if(m_talking && W32_GEQ(GETTIMESTAMP(), (m_last_playback+m_play_stopped_delay)))
    {
        //stop playing if no new sound packet has been received for some time
        MYTRACE(ACE_TEXT("Stopped playing stream after %u for #%d\n"), 
                GETTIMESTAMP() - m_last_playback, m_userid);
        m_talking = false;

        //reset packet numbers
        Reset();
    }

    // store in muxer before resampling
    if (!m_no_recording || !played)
    {
        //store in muxer (if enabled)
        media::AudioFormat fmt = GetAudioCodecAudioFormat(m_codec);
        media::AudioFrame frm(fmt, tmp_output_buffer, GetAudioCodecCbSamples(m_codec));
        frm.sample_no = m_samples_played;
        frm.streamid = m_stream_id;
        m_audio_callback(m_userid, m_streamtype, frm);
    }

    //increment samples played (used by AudioMuxer)
    m_samples_played += input_samples;

    if (m_resampler)
    {
        int ret = m_resampler->Resample(tmp_output_buffer, input_samples,
                                        output_buffer, output_samples);
        assert(ret > 0);
        assert(ret <= output_samples);
        MYTRACE_COND(ret != output_samples,
                     ACE_TEXT("Resampler output incorrect no. samples, expect %d, got %d\n"),
                     output_samples, ret);
    }
    return true;
}

int AudioPlayer::GetNumAudioPacketsRecv(bool reset)
{
    int n = m_audiopackets_recv;
    if(reset)
        m_audiopackets_recv = 0;
    return n;
}

int AudioPlayer::GetNumAudioPacketsLost(bool reset)
{
    int n = m_audiopacket_lost;
    if(reset)
        m_audiopacket_lost = 0;
    return n;
}

void AudioPlayer::SetAudioBufferSize(int msec)
{
    m_buffer_msec = std::max(msec, GetAudioCodecCbMillis(m_codec));
}

int AudioPlayer::GetBufferedAudioMSec()
{
    wguard_t g(m_mutex);

    int codec_msec = GetAudioCodecCbMillis(m_codec);
    if (m_stream_id && m_buffer.size() && codec_msec)
    {
        int16_t n_packets = (int16_t)(m_buffer.rbegin()->first - m_play_pkt_no);
        return codec_msec * (++n_packets);
    }
    return int(m_buffer.size()) * codec_msec;
}

void AudioPlayer::AddPacket(const teamtalk::AudioPacket& packet)
{
    TTASSERT(!packet.HasFragments());
    uint16_t enc_len;
    const char* enc_data = packet.GetEncodedAudio(enc_len);
    if(!enc_data || !enc_len)
        return;

    wguard_t g(m_mutex);
    m_audiopackets_recv++;

    uint16_t pkt_no = packet.GetPacketNumber();
    
    MYTRACE_COND(packet.GetStreamID() == 0,
                 ACE_TEXT("Received stream ID 0 from #%d. Packet no %d\n"),
                 m_userid, (int)pkt_no);

    if(packet.GetStreamID() == 0)
        return;

    if (m_stream_id && W16_LT(pkt_no, m_play_pkt_no))
    {
        MYTRACE(ACE_TEXT("User #%d, packet %d arrived too late\n"), m_userid, pkt_no);
        return;
    }

    MYTRACE_COND(m_buffer.find(pkt_no) != m_buffer.end(),
                 ACE_TEXT("User #%d, duplicate packet %d, start %d, end %d\n"), (int)pkt_no,
                 m_userid, m_buffer.begin()->first, m_buffer.rbegin()->first);
       
    if(packet.HasFrameSizes())
    {
        m_buffer[pkt_no].enc_frame_sizes = packet.GetEncodedFrameSizes();
        MYTRACE_COND(SumFrameSizes(m_buffer[pkt_no].enc_frame_sizes) != enc_len,
                     ACE_TEXT("User #%d, sum of frame sizes didn't match - %d != %d\n"),
                     m_userid, SumFrameSizes(m_buffer[pkt_no].enc_frame_sizes), enc_len);

        if(SumFrameSizes(m_buffer[pkt_no].enc_frame_sizes) == enc_len)
            m_buffer[pkt_no].enc_frames.assign(enc_data, enc_data+enc_len);
        else
        {
            m_buffer.erase(pkt_no);
            return;
        }
    }
    else
    {
        int frames_per_packet = GetAudioCodecFramesPerPacket(m_codec);
        m_buffer[pkt_no].enc_frames.assign(enc_data, enc_data+enc_len);
        if (frames_per_packet > 1)
        {
            int encfrmsize = enc_len / frames_per_packet;
            m_buffer[pkt_no].enc_frame_sizes.assign(frames_per_packet, encfrmsize);
        }
        else
            m_buffer[pkt_no].enc_frame_sizes.push_back(enc_len);
    }
    m_buffer[pkt_no].timestamp = packet.GetTime();
    TTASSERT(packet.GetStreamID());
    m_buffer[pkt_no].stream_id = packet.GetStreamID();

    //ensure buffer doesn't overflow
    while(GetBufferedAudioMSec() > m_buffer_msec && m_buffer.size())
    {
        MYTRACE(ACE_TEXT("User #%d, removing pkt_no %d to limit buffer to %d msec, cur buffer is %d msec. Play pkt %d\n"),
                m_userid, m_buffer.begin()->first, m_buffer_msec, 
                GetBufferedAudioMSec(), (int)m_play_pkt_no);
        m_buffer.erase(m_buffer.begin());
        //update next packet to be played
        if(m_buffer.size())
            m_play_pkt_no = m_buffer.begin()->first;
    }

    MYTRACE_COND(GetBufferedAudioMSec() > m_buffer_msec,
                 ACE_TEXT("User #%d buffer size is foobar, msec: %d\n"),
                 m_userid, GetBufferedAudioMSec());

    if (m_stream_id == 0)
    {
        m_play_pkt_no = pkt_no;
        m_stream_id = packet.GetStreamID();
    }
    //MYTRACE(ACE_TEXT("#%d - Packet store no: %d, play: %d\n"), m_user.GetUserID(),
    //    packet_number, m_play_pkt_no);
}

bool AudioPlayer::PlayBuffer(short* output_buffer, int n_samples)
{
    wguard_t g(m_mutex);
    bool played = false;

    //play until last packet arrived
    if(m_buffer.size())
    {
        TTASSERT(W16_GEQ(m_buffer.begin()->first, m_play_pkt_no));

        while(m_stream_id && GetBufferedAudioMSec() > m_buffer_msec)
        {
            MYTRACE(ACE_TEXT("User #%d, dropped packet %d, max %d\n"), 
                    m_userid, m_buffer.begin()->first, m_buffer.rbegin()->first);
            m_buffer.erase(m_buffer.begin());

            if (m_buffer.size())
            {
                MYTRACE(ACE_TEXT("User #%d, skipped %d-%d packets\n"),
                        m_userid, m_play_pkt_no, m_buffer.begin()->first-1);
                m_play_pkt_no = m_buffer.begin()->first;
            }
        }

        MYTRACE_COND(DEBUG_PLAYBACK,
                     ACE_TEXT("User #%d, streamtype %u, stream id %d, cur_pkt %d, max pkt %d, tm: %u\n"),
                     m_userid, m_streamtype, m_stream_id, m_play_pkt_no, m_buffer.rbegin()->first,
                     GETTIMESTAMP());

        if(DecodeFrame(m_buffer[m_play_pkt_no], output_buffer, n_samples))
        {
            m_played_packet_time = m_buffer[m_play_pkt_no].timestamp;
            MYTRACE_COND(m_stream_id != m_buffer[m_play_pkt_no].stream_id,
                         ACE_TEXT("User #%d started new audio stream %d\n"), m_userid, 
                         m_buffer[m_play_pkt_no].stream_id);
            m_stream_id = m_buffer[m_play_pkt_no].stream_id;
        }
        else
        {
            m_audiopacket_lost++;
        }

        //clear slot
        m_buffer.erase(m_play_pkt_no);

        //increment packet number to be played next time
        m_play_pkt_no++;
        played = true;
    }
    else
    {
        memset(output_buffer, 0, GetAudioCodecCbBytes(m_codec));
        played = false;
        
        MYTRACE_COND(DEBUG_PLAYBACK,
                     ACE_TEXT("No packets available for playback for user #%d. Current packet: %d\n"),
                     m_userid, m_play_pkt_no);
    }

    //stereo simulation
    if (GetAudioCodecSimulateStereo(m_codec))
    {
        //Speex doesn't support stereo so simulate
        //If in stereo then choose which channels to output audio to
        for(int i=2*n_samples - 2; i >= 0; i -= 2)
        {
            output_buffer[i] = output_buffer[i/2];
            output_buffer[i+1] = output_buffer[i/2];
        }
    }

    return played;
}

#if defined(ENABLE_SPEEX)
SpeexPlayer::SpeexPlayer(int userid, StreamType stream_type,
                         useraudio_callback_t audio_cb, const AudioCodec& codec,
                         audio_resampler_t resampler)
: AudioPlayer(userid, stream_type, audio_cb, codec, resampler)
{
    TTASSERT(codec.codec == CODEC_SPEEX || codec.codec == CODEC_SPEEX_VBR);
    bool b = false;
    switch(codec.codec)
    {
    case CODEC_SPEEX :
        b = m_decoder.Initialize(codec.speex.bandmode);
        break;
    case CODEC_SPEEX_VBR :
        b = m_decoder.Initialize(codec.speex_vbr.bandmode);
        break;
    case CODEC_NO_CODEC :
    case CODEC_OPUS :
    case CODEC_WEBM_VP8 :
        TTASSERT(0);
        break;
    }
    MYTRACE_COND(!b, ACE_TEXT("Failed to initialize Speex decoder\n"));
}

SpeexPlayer::~SpeexPlayer()
{
    m_decoder.Close();
}

void SpeexPlayer::Reset()
{
    AudioPlayer::Reset();
    m_decoder.Reset();
}

bool SpeexPlayer::DecodeFrame(const encframe& enc_frame,
                              short* output_buffer, int n_samples)
{
    if(enc_frame.enc_frames.size()) //packet available
    {
        // reset decoder before starting new stream
        if (enc_frame.stream_id != m_stream_id)
            m_decoder.Reset();

        // first do bounds check
        std::vector<int> frmsizes = ConvertFrameSizes(enc_frame.enc_frame_sizes);
        int totalsize = SumFrameSizes(frmsizes);
        assert(totalsize == enc_frame.enc_frames.size());
        if (totalsize > enc_frame.enc_frames.size())
            return false;

        m_decoder.DecodeMultiple(&enc_frame.enc_frames[0], 
                                 frmsizes,
                                 output_buffer);
        return true;
    }
    else  //packet lost
    {
        MYTRACE(ACE_TEXT("User #%d is missing packet %d\n"), 
                m_userid, m_play_pkt_no);
        std::vector<int> frm_sizes(GetAudioCodecFramesPerPacket(m_codec), 0);
        m_decoder.DecodeMultiple(NULL, frm_sizes, output_buffer);
        //increment 'm_played_packet_time' with GetAudioCodecCbMillis()?
        return false;
    }
}
#endif /* ENABLE_SPEEX */

#if defined(ENABLE_OPUS)

OpusPlayer::OpusPlayer(int userid, StreamType stream_type,
                       useraudio_callback_t audio_cb, const AudioCodec& codec,
                       audio_resampler_t resampler)
: AudioPlayer(userid, stream_type, audio_cb, codec, resampler)
{
    TTASSERT(codec.codec == CODEC_OPUS);
    bool b = false;
    switch(codec.codec)
    {
    case CODEC_OPUS :
        b = m_decoder.Open(codec.opus.samplerate, codec.opus.channels);
        break;
    case CODEC_SPEEX :
    case CODEC_SPEEX_VBR :
    case CODEC_WEBM_VP8 :
    case CODEC_NO_CODEC :
        TTASSERT(0);
        break;
    }
    MYTRACE_COND(!b, ACE_TEXT("Failed to initialize OPUS decoder\n"));
}

OpusPlayer::~OpusPlayer()
{
    m_decoder.Close();
}

void OpusPlayer::Reset()
{
    AudioPlayer::Reset();
    m_decoder.Reset();
}

bool OpusPlayer::DecodeFrame(const encframe& enc_frame,
                             short* output_buffer, int n_samples)
{
    MYTRACE_COND(enc_frame.stream_id && enc_frame.stream_id != m_stream_id,
                 ACE_TEXT("New stream id %d\n"), enc_frame.stream_id);
    
    int framesize = GetAudioCodecFrameSize(m_codec);
    int samples = GetAudioCodecCbSamples(m_codec);
    int channels = GetAudioCodecChannels(m_codec);
    int ret;

    assert(samples == n_samples);
    
    if (enc_frame.enc_frames.size()) //packet available
    {
        // reset decoder before starting new stream
        if (enc_frame.stream_id != m_stream_id)
            m_decoder.Reset();

        int fpp = GetAudioCodecFramesPerPacket(m_codec);
        assert(fpp == enc_frame.enc_frame_sizes.size());
        if (fpp != enc_frame.enc_frame_sizes.size())
            return false;

        // first do bounds check
        int frmsizes = SumFrameSizes(enc_frame.enc_frame_sizes);
        assert(frmsizes == enc_frame.enc_frames.size());
        if (frmsizes > enc_frame.enc_frames.size())
            return false;

        int encoffset = 0, decoffset = 0;
        for (size_t i=0;i<enc_frame.enc_frame_sizes.size();i++)
        {
            //MYTRACE(ACE_TEXT("Decoding frame %d/%d, %d bytes\n"),
            //        int(i), int(enc_frame.enc_frame_sizes.size()),
            //        int(enc_frame.enc_frame_sizes[i]));
            
            ret = m_decoder.Decode(&enc_frame.enc_frames[encoffset], 
                                   enc_frame.enc_frame_sizes[i],
                                   &output_buffer[decoffset*channels], framesize);
            MYTRACE_COND(ret != framesize, ACE_TEXT("OPUS decode failed for #%d. Ret = %d\n"), m_userid, ret);
            encoffset += enc_frame.enc_frame_sizes[i];
            decoffset += framesize;
        }
        return true;
    }
    else  //packet lost
    {
        MYTRACE(ACE_TEXT("User #%d is missing packet %d\n"), m_userid, m_play_pkt_no);
        int fpp = GetAudioCodecFramesPerPacket(m_codec);
        int decoffset = 0;
        for (int i=0;i<fpp;i++)
        {
            m_decoder.Decode(NULL, 0, &output_buffer[decoffset*channels], framesize);
            decoffset += framesize;
        }
        //increment 'm_played_packet_time' with GetAudioCodecCbMillis()?
        return false;
    }
}
#endif

#if defined(ENABLE_VPX)

#define VPX_MAX_FRAG_PACKETS 3000
#define VPX_MAX_PACKETS 3000

WebMPlayer::WebMPlayer(int userid, int stream_id)
: m_userid(userid)
, m_video_pkts_recv(0)
, m_videoframes_recv(0)
, m_videoframes_lost(0)
, m_videoframes_dropped(0)
, m_videostream_id(stream_id)
, m_packet_no(0)
, m_local_timestamp(GETTIMESTAMP())
, m_decoder_ready(false)
{
    MYTRACE(ACE_TEXT("New WebMPlayer() - #%d stream id %d\n"), m_userid, stream_id);
}

WebMPlayer::~WebMPlayer()
{
    MYTRACE(ACE_TEXT("~WebMPlayer() - #%d stream id %d. Fragments: %u, frames: %u\n"),
            m_userid, m_videostream_id, (unsigned)m_video_fragments.size(), 
            (unsigned)m_video_frames.size());
}

bool WebMPlayer::AddPacket(const VideoPacket& packet,
                           size_t* n_packets/* = NULL*/)
{
    m_local_timestamp = GETTIMESTAMP();

    m_video_pkts_recv++;

    // MYTRACE(ACE_TEXT("Received packet no %d\n"), packet.GetPacketNo());

    assert(packet.GetStreamID() == m_videostream_id);
    if(!m_decoder_ready)
    {
        uint16_t w, h;
        if(!packet.GetStreamID(&m_packet_no, 0, 0, &w, &h))
            return false;
        if(!m_decoder.Open(w, h))
            return false;

        MYTRACE(ACE_TEXT("Starting new video stream %d for user #%d. %dx%d\n"), 
                packet.GetStreamID(), m_userid, w, h);
        m_decoder_ready = true;
    }

    size_t n_frames = m_video_frames.size();
    ProcessVideoPacket(packet);

    // dumpFragments();

    //return true if packet has ended up in the packet queue
    return m_video_frames.find(packet.GetTime()) != m_video_frames.end();
}

void WebMPlayer::ProcessVideoPacket(const VideoPacket& packet)
{
    wguard_t g(m_mutex);

    uint32_t packet_no = packet.GetPacketNo();
    MYTRACE_COND(W32_LT(packet_no, m_packet_no),
                 ACE_TEXT("Packet %u from #%d arrived too late. Current is %u\n"),
                 packet.GetPacketNo(), m_userid, m_packet_no);
                 
    if(W32_LT(packet_no, m_packet_no))
        return;

    uint16_t fragno = packet.GetFragmentNo();
    if(fragno == VideoPacket::INVALID_FRAGMENT_NO)
    {
        uint16_t frame_size = 0;
        const char* data = packet.GetEncodedData(frame_size);
        assert(data);
        if(!data)
            return;

        enc_frame new_frame;
        new_frame.enc_data.assign(data, data+frame_size);
        new_frame.packet_no = packet_no;
        m_video_frames[packet.GetTime()] = new_frame;
        m_videoframes_recv++;
    }
    else //fragmented packet
    {
        bool store_fragment = true;
        reassm_queue_t::iterator ii = m_video_fragments.find(packet_no);
        if(ii != m_video_fragments.end())
        {
            enc_frame new_frame;
            new_frame.packet_no = packet_no;
            if(ReassembleVideoPackets(ii->second, packet, new_frame.enc_data))
            {
                m_video_frames[packet.GetTime()] = new_frame;
                m_videoframes_recv++;
                m_video_fragments.erase(ii);
                store_fragment = false;
            }
        }
        
        if(store_fragment)
        {
            VideoPacket* new_packet;
            ACE_NEW(new_packet, VideoPacket(packet));
            m_video_fragments[packet_no][fragno] = videopacket_t(new_packet);
        }

        if(m_video_fragments.size()>VPX_MAX_FRAG_PACKETS)
        {
            MYTRACE(ACE_TEXT("Dropped fragmented video packet %d\n"), m_video_fragments.begin()->first);
            m_video_fragments.erase(m_video_fragments.begin());
        }
    }

    if(m_video_frames.size()>VPX_MAX_PACKETS)
    {
        MYTRACE(ACE_TEXT("Dropped video packet %d\n"), m_video_frames.begin()->first);
        m_video_frames.erase(m_video_frames.begin());
    }

    // if a packet is more than 5 seconds old it will be evicted
    if(m_video_frames.size()>2)
    {
        video_frames_t::iterator next_pkt, last_pkt;
        next_pkt = m_video_frames.begin();
        last_pkt = m_video_frames.end();
        last_pkt--;
        while(next_pkt != last_pkt)
        {
            if(W32_LT(next_pkt->first + 5000, last_pkt->first))
            {
                m_packet_no = next_pkt->second.packet_no;
                MYTRACE(ACE_TEXT("Packet no. now moved to %u, %u is too old. Video frame queue holds %d packets\n"), 
                        m_packet_no, next_pkt->first, (unsigned)m_video_frames.size()); 
            }
            else break;

            next_pkt++;
        }
    }

    RemoveObsoletePackets();
}

ACE_Message_Block* WebMPlayer::GetNextFrame(uint32_t* timestamp)
{
    wguard_t g(m_mutex);

    dumpFragments();

    //m_video_frames are sorted with UINT32 wrap
    video_frames_t::iterator ii = m_video_frames.begin();

    MYTRACE_COND(!m_decoder_ready, ACE_TEXT("Decoder not ready from user #%d\n"), m_userid);
    MYTRACE_COND(ii == m_video_frames.end(), ACE_TEXT("No video frames ready from user #%d\n"), m_userid);
    if(!m_decoder_ready || ii == m_video_frames.end() ||
       (timestamp && W32_GT(ii->first, *timestamp)))
    {
        MYTRACE_COND(ii != m_video_frames.end(), ACE_TEXT("Video frame ignored from user #%d. Time diff: %d\n"),
                     m_userid, int(*timestamp - ii->first));
        return NULL;
    }

    // MYTRACE(ACE_TEXT("GetNextFrame(), process video packet %d, size %d, csum 0x%x\n"),
    //         ii->second.packet_no, ii->second.enc_data.size(), 
    //         ACE::crc32(&ii->second.enc_data[0], ii->second.enc_data.size()));

    int ret = m_decoder.PushDecoder(&ii->second.enc_data[0], 
                                    int(ii->second.enc_data.size()));

    switch(ret)
    {
    case VPX_CODEC_UNSUP_BITSTREAM :
    {
        //restart decoder
        int w, h;
        w = m_decoder.GetConfig().w;
        h = m_decoder.GetConfig().h;

        m_decoder.Close();
        m_decoder.Open(w, h);
    }
    default :
        MYTRACE(ACE_TEXT("VPX decoder reported error %d in packet %d for user #%d\n"),
                ret, ii->second.packet_no, m_userid);
        m_packet_no = ii->second.packet_no;
        m_video_frames.erase(ii);
        return NULL;
    case VPX_CODEC_OK :
        break;
    }

    m_packet_no = ii->second.packet_no;
    m_video_frames.erase(ii);

    RemoveObsoletePackets();

    int w = m_decoder.GetConfig().w, h = m_decoder.GetConfig().h;
    int bytes = RGB32_BYTES(w, h);
    VideoFrame vid_frame(NULL, bytes, w, h, FOURCC_RGB32, true);
    vid_frame.key_frame = false; //TODO: detect key frame
    vid_frame.stream_id = m_videostream_id;
    ACE_Message_Block* mb = VideoFrameInMsgBlock(vid_frame);

    //sweep the decoder clean
    while(m_decoder.GetRGB32Image(vid_frame.frame, vid_frame.frame_length));

    return mb;
}

bool WebMPlayer::GetNextFrameTime(uint32_t* tm)
{
    wguard_t g(m_mutex);

    if(tm && m_video_frames.size())
    {
        *tm = m_video_frames.begin()->first;
        return true;
    }
    return false;
}

void WebMPlayer::dumpFragments()
{
    if(m_video_fragments.empty())
        return;
    reassm_queue_t::const_iterator ii;
    uint32_t packet_no = m_video_fragments.begin()->first;
    ii = m_video_fragments.end(); ii--;
    uint32_t packet_no_last = ii->first;
    MYTRACE_COND(m_video_fragments.size(),
                 ACE_TEXT("Missing video packet(s) from #%d ")
                 ACE_UINT32_FORMAT_SPECIFIER ACE_TEXT("-")
                 ACE_UINT32_FORMAT_SPECIFIER ACE_TEXT("\n"), 
                 m_userid, packet_no, packet_no_last);

    for(uint32_t i=packet_no;i<=packet_no_last && m_video_fragments.size();i++)
    {
        ii = m_video_fragments.find(i);
        if(ii != m_video_fragments.end())
            MYTRACE(ACE_TEXT("\tPacket #") ACE_UINT32_FORMAT_SPECIFIER 
                    ACE_TEXT(" holds ") ACE_UINT32_FORMAT_SPECIFIER
                    ACE_TEXT(" fragments of ") ACE_UINT32_FORMAT_SPECIFIER
                    ACE_TEXT("\n"), i, (uint32_t)ii->second.size(),
                    ii->second.begin()->second->GetFragmentCount());
        else
            MYTRACE(ACE_TEXT("\tPacket #") ACE_UINT32_FORMAT_SPECIFIER 
                    ACE_TEXT(" holds ") ACE_UINT32_FORMAT_SPECIFIER 
                    ACE_TEXT(" fragments\n"), i, 0);
    }
}

void WebMPlayer::RemoveObsoletePackets()
{
    //first remove obsolete fragments ( < m_packet_no)
    while(m_video_fragments.size() &&
          W32_LEQ(m_video_fragments.begin()->first, m_packet_no))
    {
        uint32_t packet_no = m_video_fragments.begin()->first;
#if defined(_DEBUG)
        dumpFragments();
#endif
        m_videoframes_lost += m_packet_no - packet_no;
        m_video_fragments.erase(packet_no);
        packet_no++;
    }

    video_frames_t::iterator i;
    while((i = m_video_frames.begin()) != m_video_frames.end() &&
          W32_LT(i->second.packet_no, m_packet_no))
    {
        m_video_frames.erase(i);
    }
}


VideoCodec WebMPlayer::GetVideoCodec() const
{
    VideoCodec codec;
    codec.codec = CODEC_WEBM_VP8;
    return codec;
}

VideoFormat WebMPlayer::GetVideoFormat() const
{
    if(!m_decoder_ready)
        return VideoFormat();

    VideoFormat fmt;
    fmt.fourcc = FOURCC_RGB32;
    fmt.width = m_decoder.GetConfig().w;
    fmt.height = m_decoder.GetConfig().h;
    return fmt;
}

int WebMPlayer::GetVideoPacketRecv(bool reset)
{
    int n = m_video_pkts_recv;
    if(reset)
        m_video_pkts_recv = 0;
    return n;
}

int WebMPlayer::GetVideoFramesRecv(bool reset)
{
    int n = m_videoframes_recv;
    if(reset)
        m_videoframes_recv = 0;
    return n;
}

int WebMPlayer::GetVideoFramesLost(bool reset)
{
    int n = m_videoframes_lost;
    if(reset)
        m_videoframes_lost = 0;
    return n;
}

int WebMPlayer::GetVideoFramesDropped(bool reset)
{
    int n = m_videoframes_dropped;
    if(reset)
        m_videoframes_dropped = 0;
    return n;
}
#endif

}//namespace
