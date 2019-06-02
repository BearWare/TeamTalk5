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

#include "ClientUser.h"
#include "ClientNode.h"

#include <teamtalk/ttassert.h>
#include <teamtalk/Commands.h>
#include <teamtalk/CodecCommon.h>

using namespace std;
using namespace teamtalk;
using namespace soundsystem;

#define TIMEOUT_STOP_AUDIO_PLAYBACK          30000    //msec for timeout of when to stop stream
#define TIMEOUT_STOP_VIDEOFILE_PLAYBACK      5000

#define VOICE_BUFFER_MSEC              1000
#define MEDIAFILE_BUFFER_MSEC          20000

ClientUser::ClientUser(int userid, ClientNode* clientnode,
                       ClientListener* listener)
                       : User(userid)
                       , m_clientnode(clientnode)
                       , m_listener(listener)
                       , m_usertype(USERTYPE_NONE)
                       , m_userdata(0)
                       , m_voice_active(false)
                       , m_voice_buf_msec(VOICE_BUFFER_MSEC)
                       , m_audiofile_active(false)
                       , m_media_buf_msec(MEDIAFILE_BUFFER_MSEC)
                       , m_desktop_packets_expected(0)
                       , m_desktop_input_rx_pktno(0)
                       , m_desktop_input_tx_pktno(0)
                       , m_snddev_error(false)
                       , m_snd_duplexmode(false)
                       , m_voice_volume(VOLUME_DEFAULT)
                       , m_audiofile_volume(VOLUME_DEFAULT)
                       , m_voice_mute(false)
                       , m_audiofile_mute(false)
                       , m_voice_stopped_delay(STOPPED_TALKING_DELAY)
                       , m_audiofile_stopped_delay(STOPPED_TALKING_DELAY)
                       , m_voice_stereo(STEREO_BOTH)
                       , m_audiofile_stereo(STEREO_BOTH)
                       , m_localsubscriptions(SUBSCRIBE_NONE)
                       , m_peersubscriptions(SUBSCRIBE_NONE)
                       , m_aff(AFF_NONE)
                       , m_voicelog_counter(0)
{
    TTASSERT(clientnode);
    m_voice_position[0] = m_voice_position[1] = m_voice_position[2] = 0.0f;
    m_audiofile_position[0] = m_audiofile_position[1] = m_audiofile_position[2] = 0.0f;
}

ClientUser::~ClientUser()
{
    TTASSERT(!m_voice_player);
    TTASSERT(!m_audiofile_player);
#if defined(ENABLE_VPX)
    TTASSERT(!m_videofile_player);
    TTASSERT(!m_vidcap_player);
#endif
    ResetAllStreams();
}

void ClientUser::ResetAllStreams()
{
    ResetAudioPlayers(false);
    CloseVideoCapturePlayer();
    CloseVideoFilePlayer();
    CloseDesktopSession();
}

void ClientUser::ResetInactiveStreams()
{
    //reset players if user is no longer active, i.e. no subscription
    if(!LocalSubscribes(SUBSCRIBE_VOICE | SUBSCRIBE_INTERCEPT_VOICE) ||
       m_channel.null())
        ResetVoicePlayer();
    if(!LocalSubscribes(SUBSCRIBE_MEDIAFILE | SUBSCRIBE_INTERCEPT_MEDIAFILE) ||
       m_channel.null())
    {
        ResetAudioFilePlayer();
        CloseVideoFilePlayer();
    }
    if(!LocalSubscribes(SUBSCRIBE_VIDEOCAPTURE | SUBSCRIBE_INTERCEPT_VIDEOCAPTURE) ||
       m_channel.null())
        CloseVideoCapturePlayer();
    if(!LocalSubscribes(SUBSCRIBE_DESKTOP | SUBSCRIBE_INTERCEPT_DESKTOP) ||
       m_channel.null())
        CloseDesktopSession();
}

void ClientUser::SetChannel(clientchannel_t& chan)
{
    m_channel = chan;
}

int ClientUser::TimerMonitorVoicePlayback()
{
    if (!m_voice_player)
        return -1;

    bool talking = m_voice_player->IsTalking();
    bool changed = talking != IsAudioActive(STREAMTYPE_VOICE);
    m_voice_active = talking;
    if(changed)
        m_listener->OnUserStateChange(*this);

    int n_blocks = m_voice_player->GetNumAudioBlocks(true);
    m_stats.voicepackets_recv += m_voice_player->GetNumAudioPacketsRecv(true);
    m_stats.voicepackets_lost += m_voice_player->GetNumAudioPacketsLost(true);

    //MYTRACE_COND(n_blocks, ACE_TEXT("User #%d has %d new voice block at %u\n"), GetUserID(), n_blocks, GETTIMESTAMP());
    while(n_blocks--)
    {
        m_listener->OnUserAudioBlock(GetUserID(), STREAMTYPE_VOICE);
    }

    //check if player should be reset
    if(m_voice_player->GetLastPlaytime() &&
       W32_GEQ(GETTIMESTAMP(), m_voice_player->GetLastPlaytime() + TIMEOUT_STOP_AUDIO_PLAYBACK))
    {
        MYTRACE(ACE_TEXT("Resetting voice stream for %s\n"), GetNickname().c_str());
        ResetVoicePlayer();
        return -1;
    }

    return 0;
}

int ClientUser::TimerMonitorAudioFilePlayback()
{
    if (!m_audiofile_player)
        return -1;

    bool active = m_audiofile_player->IsTalking();
    bool changed = m_audiofile_active != active;
    m_audiofile_active = active;
    if(changed)
        m_listener->OnUserStateChange(*this);

    int n_blocks = m_audiofile_player->GetNumAudioBlocks(true);
    m_stats.mediafile_audiopackets_recv += m_audiofile_player->GetNumAudioPacketsRecv(true);
    m_stats.mediafile_audiopackets_lost += m_audiofile_player->GetNumAudioPacketsLost(true);

    while(n_blocks--)
        m_listener->OnUserAudioBlock(GetUserID(), STREAMTYPE_MEDIAFILE_AUDIO);

    //check if player should be reset
    if(m_audiofile_player->GetLastPlaytime() &&
       W32_GEQ(GETTIMESTAMP(), m_audiofile_player->GetLastPlaytime() + TIMEOUT_STOP_AUDIO_PLAYBACK))
    {
        MYTRACE(ACE_TEXT("Resetting media file stream for %s\n"), GetNickname().c_str());
        ResetAudioFilePlayer();
        return -1;
    }

    return 0;
}

int ClientUser::TimerMonitorVideoFilePlayback()
{
#if defined(ENABLE_VPX)
    if (!m_videofile_player)
        return -1;

    if(W32_GEQ(GETTIMESTAMP(), m_videofile_player->GetLastTimeStamp() +
               TIMEOUT_STOP_VIDEOFILE_PLAYBACK))
    {
        CloseVideoFilePlayer();
        return -1;
    }

    return 0;
#else
    return -1;
#endif
}

int ClientUser::TimerDesktopDelayedAck()
{
    TTASSERT(m_clientnode->TimerExists(USER_TIMER_DESKTOPACKPACKET_ID, GetUserID()));

    clientchannel_t chan = GetChannel();
    if(chan.null())
        return -1;

    uint8_t session_id = 0;
    uint32_t upd_time = 0;
    set<uint16_t> acked_packets;
    GetAckedDesktopPackets(session_id, upd_time, acked_packets);
    if(acked_packets.empty())
        return -1;

    packet_range_t ack_range;
    set<uint16_t> ack_single;
    GetPacketRanges(acked_packets, ack_range, ack_single);

    DesktopAckPacket* ack_packet;
    ACE_NEW_RETURN(ack_packet, DesktopAckPacket(m_clientnode->GetUserID(),
                                                GETTIMESTAMP(),
                                                GetUserID(), session_id,
                                                upd_time, ack_single,
                                                ack_range), -1);

    ack_packet->SetChannel(chan->GetChannelID());

    if(!m_clientnode->QueuePacket(ack_packet))
        delete ack_packet;

    return -1;
}

void ClientUser::AddVoicePacket(const VoicePacket& audpkt,
                                const struct SoundProperties& sndprop,
                                VoiceLogger& voice_logger, bool allowrecord)
{
    ASSERT_REACTOR_THREAD(*m_clientnode->reactor());

    clientchannel_t chan = GetChannel();
    if(chan.null() || chan->GetChannelID() != audpkt.GetChannel())
    {
        MYTRACE(ACE_TEXT("Received voice packet #%d outside channel, %d != %d\n"),
                GetUserID(), audpkt.GetChannel(),
                chan.null()? 0 : chan->GetChannelID());
        return;
    }

    //ignore packet if we're unsubscribed
    if(!LocalSubscribes(audpkt))
        return;

    //store time of packet for later use
    UpdateLastTimeStamp(audpkt);

    if (!m_voice_player)
        LaunchVoicePlayer(chan->GetAudioCodec(), sndprop);
    if (!m_voice_player)
        return;

    assert(m_voice_player->GetAudioCodec() == chan->GetAudioCodec());
    audiopacket_t reassem_pkt = m_voice_player->QueuePacket(audpkt);

    m_voice_player->SetNoRecording(!allowrecord);

    //store in voicelog
    if(GetAudioFolder().length() && allowrecord)
    {
        if(audpkt.HasFragments())
        {
            if(!reassem_pkt.null())
                voice_logger.AddVoicePacket(*this, *chan, *reassem_pkt);
        }
        else
            voice_logger.AddVoicePacket(*this, *chan, audpkt);
    }

    // MYTRACE(ACE_TEXT("Added audio packet #%d, TS: %u, Local TS: %u\n"),
    //         (int)audpkt.GetPacketNumber(), audpkt.GetTime(), GETTIMESTAMP());
    if(!m_snd_duplexmode)
    {
        if( SOUNDSYSTEM->IsStreamStopped(m_voice_player.get()) )
        {
            SOUNDSYSTEM->StartStream(m_voice_player.get());
            MYTRACE(ACE_TEXT("Starting voice stream for #%d\n"), GetUserID());
        }
    }
}

void ClientUser::AddAudioFilePacket(const AudioFilePacket& audpkt,
                                    const struct SoundProperties& sndprop)
{
    ASSERT_REACTOR_THREAD(*m_clientnode->reactor());

    clientchannel_t chan = GetChannel();
    if(chan.null() || chan->GetChannelID() != audpkt.GetChannel())
    {
        MYTRACE(ACE_TEXT("Received media file packet #%d outside channel, %d != %d\n"),
                GetUserID(), audpkt.GetChannel(),
                chan.null()? 0 : chan->GetChannelID());
        return;
    }

    //ignore packet if we're unsubscribed
    if(!LocalSubscribes(audpkt))
        return;

    //store time of packet for later use
    UpdateLastTimeStamp(audpkt);

    if (!m_audiofile_player)
        LaunchAudioFilePlayer(chan->GetAudioCodec(), sndprop);
    if (!m_audiofile_player)
        return;

    bool no_record = (chan->GetChannelType() & CHANNEL_NO_RECORDING);
    m_audiofile_player->SetNoRecording(no_record);

    assert(m_audiofile_player->GetAudioCodec() == chan->GetAudioCodec());
    audiopacket_t reassem_pkt = m_audiofile_player->QueuePacket(audpkt);
    if(!m_snd_duplexmode)
    {
        if( SOUNDSYSTEM->IsStreamStopped(m_audiofile_player.get()) )
        {
            SOUNDSYSTEM->StartStream(m_audiofile_player.get());
            MYTRACE(ACE_TEXT("Starting media audio stream for #%d\n"), GetUserID());
        }
    }
}

void ClientUser::AddVideoCapturePacket(const VideoCapturePacket& p,
                                       const ClientChannel& chan)
{
    ASSERT_REACTOR_THREAD(*m_clientnode->reactor());

    //ignore packet if we're unsubscribed
    if(!LocalSubscribes(p))
        return;

    UpdateLastTimeStamp(p);

#if defined(ENABLE_VPX)
    bool new_vidframe = false;
    if (m_vidcap_player &&
       p.GetStreamID() == m_vidcap_player->GetStreamID())
    {
        new_vidframe = m_vidcap_player->AddPacket(p);
    }
    else if ((m_vidcap_player &&
             p.GetStreamID() != m_vidcap_player->GetStreamID() &&
             W32_GEQ(p.GetTime(), GetLastTimeStamp(p))) ||
            (!m_vidcap_player && W32_GEQ(p.GetTime(), GetLastTimeStamp(p))))
    {
        WebMPlayer* webm_player;
        ACE_NEW(webm_player, WebMPlayer(GetUserID(), p.GetStreamID()));
        m_vidcap_player = webm_player_t(webm_player);
        new_vidframe = m_vidcap_player->AddPacket(p);
        m_listener->OnUserStateChange(*this);
    }
    else
    {
        return;
    }

    if(new_vidframe)
        m_listener->OnUserVideoCaptureFrame(GetUserID(), p.GetStreamID());

    m_stats.vidcappackets_recv += m_vidcap_player->GetVideoPacketRecv(true);
    m_stats.vidcapframes_recv += m_vidcap_player->GetVideoFramesRecv(true);
    m_stats.vidcapframes_dropped += m_vidcap_player->GetVideoFramesDropped(true);
    m_stats.vidcapframes_lost += m_vidcap_player->GetVideoFramesLost(true);
#endif
}

void ClientUser::AddVideoFilePacket(const VideoFilePacket& p,
                                    const ClientChannel& chan)
{
    ASSERT_REACTOR_THREAD(*m_clientnode->reactor());

    // MYTRACE("Received video packet %d fragment %d/%d from #%d\n",
    //         p.GetPacketNo(), p.GetFragmentNo(), p.GetFragmentCount(), GetUserID());

    //ignore packet if we're unsubscribed
    if(!LocalSubscribes(p))
        return;

#if defined(ENABLE_VPX)
    UpdateLastTimeStamp(p);

    bool new_vidframe = false;
    uint8_t stream_id = 0;
    //check if new
    if (m_videofile_player &&
       p.GetStreamID() == m_videofile_player->GetStreamID())
    {
        new_vidframe = m_videofile_player->AddPacket(p);
        stream_id = m_videofile_player->GetStreamID();
    }
    else if ((m_videofile_player &&
             p.GetStreamID() != m_videofile_player->GetStreamID() &&
             W32_GEQ(p.GetTime(), GetLastTimeStamp(p))) ||
            (!m_videofile_player && W32_GEQ(p.GetTime(), GetLastTimeStamp(p))))
    {
        WebMPlayer* webm_player;
        ACE_NEW(webm_player, WebMPlayer(GetUserID(), p.GetStreamID()));
        m_videofile_player = webm_player_t(webm_player);
        new_vidframe = m_videofile_player->AddPacket(p);
        stream_id = m_videofile_player->GetStreamID();

        m_listener->OnUserStateChange(*this);

        if(!m_clientnode->TimerExists(USER_TIMER_MEDIAFILE_VIDEO_PLAYBACK_ID, GetUserID()))
        {
            ACE_Time_Value tm(1, 0);
            long timerid = m_clientnode->StartUserTimer(USER_TIMER_MEDIAFILE_VIDEO_PLAYBACK_ID,
                                                        GetUserID(), 0, tm, tm);
            TTASSERT(timerid >= 0);
        }
    }
    else
    {
        return;
    }

    if(new_vidframe)
    {
        if (m_audiofile_player &&
           GetAudioStreamBufferSize(STREAMTYPE_MEDIAFILE_AUDIO))
        {
            if(m_audiofile_player->GetPlayedPacketNo())
            {
                uint32_t audio_tm = m_audiofile_player->GetPlayedPacketTime();
                uint32_t video_tm = 0;
                if(m_videofile_player->GetNextFrameTime(&video_tm) &&
                   W32_LEQ(video_tm, audio_tm))
                {
                    m_listener->OnUserMediaFileVideoFrame(GetUserID(), stream_id);
                }
            }
        }
        else
            m_listener->OnUserMediaFileVideoFrame(GetUserID(), stream_id);
    }

    m_stats.mediafile_video_packets_recv += m_videofile_player->GetVideoPacketRecv(true);
    m_stats.mediafile_video_frames_recv += m_videofile_player->GetVideoFramesRecv(true);
    m_stats.mediafile_video_frames_dropped += m_videofile_player->GetVideoFramesDropped(true);
    m_stats.mediafile_video_frames_lost += m_videofile_player->GetVideoFramesLost(true);
#endif
}

void ClientUser::AddPacket(const DesktopPacket& p, const ClientChannel& chan)
{
    ASSERT_REACTOR_THREAD(*m_clientnode->reactor());

    //ignore packet if we're unsubscribed
    if(!LocalSubscribes(p))
        return;

    uint8_t session_id;
    uint16_t pkt_index, pkt_count;
    if(!p.GetUpdateProperties(&session_id, &pkt_index, &pkt_count) &&
       !p.GetSessionProperties(&session_id, NULL, NULL, NULL, &pkt_index,
                               &pkt_count))
       return;

    //ensure it's not an old packet
    bool is_set = false;
    if(W32_LT(p.GetTime(), GetLastTimeStamp(p, &is_set)) && is_set)
    {
        MYTRACE(ACE_TEXT("Dropped old desktop packet: %d:%u\n"),
                (int)session_id, p.GetTime());
        return;
    }

    //check if it's a new stream, timestamp will be valid since we previously
    //had a session
    if(!m_desktop.null() && m_desktop->GetSessionID() != session_id &&
       W32_GEQ(p.GetTime(), this->GetLastTimeStamp(p)))
        CloseDesktopSession();

    //packet is ok for further processing, schedule ack timer
    if(!m_clientnode->TimerExists(USER_TIMER_DESKTOPACKPACKET_ID, GetUserID()))
    {
        long timerid = m_clientnode->StartUserTimer(USER_TIMER_DESKTOPACKPACKET_ID,
                                                    GetUserID(), 0,
                                                    ACE_Time_Value::zero);
        TTASSERT(timerid >= 0);
    }

    if(m_desktop.null())
    {
        uint16_t width, height;
        uint8_t bmp_mode;
        if(!p.GetSessionProperties(NULL, &width, &height, &bmp_mode,
                                   NULL, NULL))
        {
            RemoveObsoleteDesktopPackets(p, m_desktop_queue);

            DesktopPacket* ptmp;
            ACE_NEW(ptmp, DesktopPacket(p));
            m_desktop_queue.push_back(desktoppacket_t(ptmp));
            return;
        }

        DesktopWindow wnd(session_id, width, height,
                          (RGBMode)bmp_mode, DESKTOPPROTOCOL_ZLIB_1);

        DesktopViewer* viewer;
        ACE_NEW(viewer, DesktopViewer(wnd));
        m_desktop = desktop_viewer_t(viewer);

        if(m_desktop->GetBlocksCount() == 0)
        {
            CloseDesktopSession();
            return;
        }

        UpdateLastTimeStamp(p);
        m_desktop_packets_expected = pkt_count;
        TTASSERT(m_block_fragments.empty());
        TTASSERT(m_acked_desktoppackets.empty());

        //recursive call to self
        desktop_queue_t::const_iterator ii = m_desktop_queue.begin();
        while(ii != m_desktop_queue.end())
        {
            AddPacket(*ii->get(), chan);
            ii++;
        }
        m_desktop_queue.clear();

        //new desktop session
        m_listener->OnUserStateChange(*this);
    }

    if(m_desktop->GetSessionID() != p.GetSessionID())
        return;

    if(p.GetTime() != GetLastTimeStamp(p))
    {
        //don't process packet if we haven't received all desktop packets
        if(W32_GEQ(p.GetTime(), GetLastTimeStamp(p)) &&
           m_acked_desktoppackets.size() == m_desktop_packets_expected)
        {
            UpdateLastTimeStamp(p);
            m_block_fragments.clear();
            m_acked_desktoppackets.clear();
            m_desktop_packets_expected = pkt_count;
        }
        else
            return;
    }

    TTASSERT(GetLastTimeStamp(p) == p.GetTime());

    MYTRACE_COND(m_acked_desktoppackets.find(pkt_index) != m_acked_desktoppackets.end(),
        ACE_TEXT("Desktop Dup ACK for #%d on %d:%u - pkt %d\n"), GetUserID(),
        m_desktop->GetSessionID(), GetLastTimeStamp(p), pkt_index);

    //don't process packet if it's a DUP ack
    if(m_acked_desktoppackets.find(pkt_index) != m_acked_desktoppackets.end())
        return;

    m_acked_desktoppackets.insert(pkt_index);

    TTASSERT(m_desktop_packets_expected == pkt_count);

    map_block_t block_nums;
    p.GetBlocks(block_nums);

    bool updated_window = block_nums.size();

    //process complete blocks
    map_block_t::const_iterator ii = block_nums.begin();
    while(ii != block_nums.end())
    {
        m_desktop->AddCompressedBlock(ii->first, ii->second.block_data,
                                      ii->second.block_size);
        ii++;
    }

    //process fragmented blocks
    block_frags_t fragments;
    p.GetBlockFragments(fragments);
    if(fragments.size())
    {
        DesktopPacket* p_cache;
        ACE_NEW(p_cache, DesktopPacket(p));
        desktoppacket_t packet_ptr(p_cache);

        block_frags_t::const_iterator fi = fragments.begin();
        while(fi != fragments.end())
        {
            map_desktoppacket_t::iterator bi = m_block_fragments.find(fi->block_no);
            if(bi == m_block_fragments.end())
            {
                //new fragmented desktop packet
                map_frag_desktoppacket_t new_frag_set;
                new_frag_set[fi->frag_no] = packet_ptr;
                m_block_fragments[fi->block_no] = new_frag_set;
            }
            else
            {
                //we already have some fragments
                bi->second[fi->frag_no] = packet_ptr;
#ifdef _DEBUG
                map_frag_desktoppacket_t::iterator rr = bi->second.begin();
                while(rr != bi->second.end())
                {
                    assert(rr->second->GetTime() == packet_ptr->GetTime());
                    rr++;
                }
#endif
            }
            fi++;
        }

        map_blocks_t blocks;
        ReassembleDesktopBlocks(m_block_fragments, blocks);
        updated_window |= (bool)blocks.size();
        map_blocks_t::iterator bi = blocks.begin();
        while(bi != blocks.end())
        {
            m_desktop->AddCompressedBlock(bi->first, &bi->second[0],
                int(bi->second.size()));
            bi++;
        }
    }

    //process duplicate blocks
    p.GetDuplicateBlocks(m_dup_blocks);
    //only update duplicate blocks if everything has been received
    if(m_acked_desktoppackets.size() == m_desktop_packets_expected &&
       m_dup_blocks.size())
    {
        map_dup_blocks_t::iterator dbi = m_dup_blocks.begin();
        for(;dbi!=m_dup_blocks.end();dbi++)
        {
            set<uint16_t>::iterator si = dbi->second.begin();
            for(;si!=dbi->second.end();si++)
                m_desktop->AddDuplicateBlock(dbi->first, *si);
        }
        m_dup_blocks.clear();
    }


    //MYTRACE(ACE_TEXT("Fragments left: %d\n"), m_block_fragments.size());

    //TTASSERT(m_acked_desktoppackets.size() < pkt_count ||
    //    (m_acked_desktoppackets.size() == pkt_count && m_block_fragments.empty()));

    MYTRACE_COND(m_acked_desktoppackets.size() == m_desktop_packets_expected,
        ACE_TEXT("Desktop update for #%d update id %d:%u\n"), GetUserID(),
        m_desktop->GetSessionID(), GetLastTimeStamp(p));

    if(updated_window)
        m_listener->OnUserDesktopWindow(GetUserID(), m_desktop->GetSessionID());
}

void ClientUser::GetAckedDesktopPackets(uint8_t& session_id,
                                        uint32_t& upd_time,
                                        std::set<uint16_t>& acked) const
{
    if(m_desktop.null())
    {
        desktoppackets_t::const_iterator ii = m_desktop_queue.begin();
        if(ii != m_desktop_queue.end())
        {
            session_id = (*ii)->GetSessionID();
            upd_time = (*ii)->GetTime();
            ::GetAckedDesktopPackets(session_id, upd_time, m_desktop_queue, acked);
        }
    }
    else
    {
        session_id = m_desktop->GetSessionID();
        upd_time = GetLastTimeStamp(PACKET_KIND_DESKTOP);
        acked.insert(m_acked_desktoppackets.begin(),
                     m_acked_desktoppackets.end());
    }
}

void ClientUser::AddPacket(const DesktopCursorPacket& p,
                           const ClientChannel& chan)
{
    //ignore packet if we're unsubscribed
    if(!LocalSubscribes(p))
        return;

    bool is_set = false;
    uint32_t tm = GetLastTimeStamp(p, &is_set);

    int16_t x, y;

    if(p.GetSessionCursor(0, 0, &x, &y) &&
       (W32_GT(p.GetTime(), tm) || !is_set))
    {
        DesktopInput input;
        input.x = x;
        input.y = y;

        m_listener->OnUserDesktopCursor(p.GetSrcUserID(), input);
    }
}

void ClientUser::AddPacket(const DesktopInputPacket& p,
                           const ClientChannel& chan)
{
    //ignore packet if we're unsubscribed
    if(!LocalSubscribes(p))
        return;

    //session ID match is checked in ClientNode
    bool is_set = false, found = false;
    uint8_t packetno = 0;
    packetno = p.GetPacketNo(&found);
    assert(found);
    if(!found)
        return;

    //ensure it's not an retransmission (and already processed)
    if (W32_LEQ(p.GetTime(), GetLastTimeStamp(p, &is_set)) && is_set &&
        W8_LT(packetno, m_desktop_input_rx_pktno))
    {
        MYTRACE(ACE_TEXT("Dropped desktop input rtx: %d:%u, pkt: %d\n"),
                (int)p.GetSessionID(), p.GetTime(), packetno);
        return;
    }
    
    //insert packet into queue
    list<desktopinput_pkt_t>::iterator ii = m_desktop_input_rx.begin();
    while(ii != m_desktop_input_rx.end())
    {
        if(W8_LT(packetno, (*ii)->GetPacketNo()))
            break;
        if((*ii)->GetPacketNo() == packetno) //already have packet
        {
            MYTRACE(ACE_TEXT("Duplicate desktop input. Pkt: %d\n"),
                    int(packetno));
            return;
        }
        ii++;
    }

    DesktopInputPacket* new_pkt;
    ACE_NEW(new_pkt, DesktopInputPacket(p));
    desktopinput_pkt_t pkt(new_pkt);
    m_desktop_input_rx.insert(ii, pkt);

    MYTRACE(ACE_TEXT("DesktopInput in queue: "));
    ii = m_desktop_input_rx.begin();
    while(ii != m_desktop_input_rx.end())
    {
       MYTRACE(ACE_TEXT("%u, "), (ACE_UINT32)(*ii)->GetPacketNo());
       ii++;
    }
    MYTRACE(ACE_TEXT(". Next: %d\n"), int(m_desktop_input_rx_pktno));

    if(m_desktop_input_rx.size() > DESKTOPINPUT_QUEUE_MAX_SIZE)
    {
        m_desktop_input_rx.pop_front();
        m_desktop_input_rx_pktno = (*m_desktop_input_rx.begin())->GetPacketNo();
    }

    ii = m_desktop_input_rx.begin();
    while(ii != m_desktop_input_rx.end())
    {
        if((*ii)->GetPacketNo() == m_desktop_input_rx_pktno)
        {
            //update timestamp of last processed packet
            UpdateLastTimeStamp(*(*ii));

            m_desktop_input_rx_pktno++;
        }
        ii++;
    }

    //notify for all received desktop inputs
    while(m_desktop_input_rx.size() &&
          W8_LT((*m_desktop_input_rx.begin())->GetPacketNo(), m_desktop_input_rx_pktno))
    {
        std::vector<DesktopInput> inputs;
        (*m_desktop_input_rx.begin())->GetDesktopInput(inputs);
        for(size_t i=0;i<inputs.size();i++)
            m_listener->OnUserDesktopInput(GetUserID(), inputs[i]);

        MYTRACE(ACE_TEXT("Ejected desktop input pkt: %d\n"),
                (*m_desktop_input_rx.begin())->GetPacketNo());
        m_desktop_input_rx.erase(m_desktop_input_rx.begin());
    }
}

bool ClientUser::IsAudioActive(StreamType stream_type) const
{
    switch(stream_type)
    {
    case STREAMTYPE_VOICE :
        return m_voice_active;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        return m_audiofile_active;
    default :
        return false;
    }
}

void ClientUser::SetPlaybackStoppedDelay(StreamType stream_type, int msec)
{
    switch(stream_type)
    {
    case STREAMTYPE_VOICE :
        if (m_voice_player)
            m_voice_player->SetStoppedTalkingDelay((uint32_t)msec);
        m_voice_stopped_delay = msec;
        break;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        if (m_audiofile_player)
            m_audiofile_player->SetStoppedTalkingDelay((uint32_t)msec);
        m_audiofile_stopped_delay = msec;
        break;
    default :
        TTASSERT(0);
        break;
    }
}

int ClientUser::GetPlaybackStoppedDelay(StreamType stream_type) const
{
    switch(stream_type)
    {
    case STREAMTYPE_VOICE :
        return m_voice_stopped_delay;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        return m_audiofile_stopped_delay;
    default:
        return 0;
    }
}

void ClientUser::SetVolume(StreamType stream_type, int volume)
{
    switch(stream_type)
    {
    case STREAMTYPE_VOICE :
        if (m_voice_player)
            SOUNDSYSTEM->SetVolume(m_voice_player.get(), volume);
        m_voice_volume = volume;
        break;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        if (m_audiofile_player)
            SOUNDSYSTEM->SetVolume(m_audiofile_player.get(), volume);
        m_audiofile_volume = volume;
        break;
    default :
        TTASSERT(0);
        break;
    }
}

int ClientUser::GetVolume(StreamType stream_type) const
{
    switch(stream_type)
    {
    case STREAMTYPE_VOICE :
        return m_voice_volume;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        return m_audiofile_volume;
    default :
        return 0;
    }
}

void ClientUser::SetMute(StreamType stream_type, bool mute)
{
    switch(stream_type)
    {
    case STREAMTYPE_VOICE :
        if (m_voice_player)
            SOUNDSYSTEM->SetMute(m_voice_player.get(), mute);
        m_voice_mute = mute;
        break;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        if (m_audiofile_player)
            SOUNDSYSTEM->SetMute(m_audiofile_player.get(), mute);
        m_audiofile_mute = mute;
        break;
    default :
        TTASSERT(0);
        break;
    }
}

bool ClientUser::IsMute(StreamType stream_type) const
{
    switch(stream_type)
    {
    case STREAMTYPE_VOICE :
        return m_voice_mute;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        return m_audiofile_mute;
    default :
        return false;
    }
}

void ClientUser::SetPosition(StreamType stream_type, float x, float y, float z)
{
    switch(stream_type)
    {
    case STREAMTYPE_VOICE :
        if (m_voice_player)
        {
            SOUNDSYSTEM->SetPosition(m_voice_player.get(), x, y, z);
            SOUNDSYSTEM->SetAutoPositioning(m_voice_player.get(), false);
        }
        m_voice_position[0] = x;
        m_voice_position[1] = y;
        m_voice_position[2] = z;
        break;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        if (m_audiofile_player)
        {
            SOUNDSYSTEM->SetPosition(m_audiofile_player.get(), x, y, z);
            SOUNDSYSTEM->SetAutoPositioning(m_audiofile_player.get(), false);
        }
        m_audiofile_position[0] = x;
        m_audiofile_position[1] = y;
        m_audiofile_position[2] = z;
        break;
    default :
        TTASSERT(0);
        break;
    }
}

void ClientUser::GetPosition(StreamType stream_type, float& x, float& y, float& z) const
{
    switch(stream_type)
    {
    case STREAMTYPE_VOICE :
        x = m_voice_position[0];
        y = m_voice_position[1];
        z = m_voice_position[2];
        break;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        x = m_audiofile_position[0];
        y = m_audiofile_position[1];
        z = m_audiofile_position[2];
        break;
    default :
        TTASSERT(0);
        break;
    }
}

void ClientUser::SetStereo(StreamType stream_type, bool left, bool right)
{
    switch(stream_type)
    {
    case STREAMTYPE_VOICE :
        m_voice_stereo = STEREO_NONE;
        if(left)
            m_voice_stereo |= STEREO_LEFT;
        if(right)
            m_voice_stereo |= STEREO_RIGHT;
        if (m_voice_player)
            m_voice_player->SetStereoMask(m_voice_stereo);
        break;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        m_audiofile_stereo = STEREO_NONE;
        if(left)
            m_audiofile_stereo |= STEREO_LEFT;
        if(right)
            m_audiofile_stereo |= STEREO_RIGHT;
        if (m_audiofile_player)
            m_audiofile_player->SetStereoMask(m_audiofile_stereo);
        break;
    default :
        TTASSERT(0);
        break;
    }
}

void ClientUser::GetStereo(StreamType stream_type, bool& left, bool& right) const
{
    switch(stream_type)
    {
    case STREAMTYPE_VOICE :
        left = (bool)(m_voice_stereo & STEREO_LEFT);
        right = (bool)(m_voice_stereo & STEREO_RIGHT);
        break;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        left = (bool)(m_audiofile_stereo & STEREO_LEFT);
        right = (bool)(m_audiofile_stereo & STEREO_RIGHT);
        break;
    default :
        TTASSERT(0);
        break;
    }
}

void ClientUser::ResetAudioPlayers(bool reset_snderr)
{
    ResetVoicePlayer();
    ResetAudioFilePlayer();

    m_snd_duplexmode = false;
    if(reset_snderr)
        m_snddev_error = false;
}

void ClientUser::ResetVoicePlayer()
{
    if (!m_voice_player)
        return;

    bool talking = IsAudioActive(STREAMTYPE_VOICE);

    if(m_snd_duplexmode)
    {
        bool b = SOUNDSYSTEM->RemoveDuplexOutputStream(m_clientnode,
                                                       m_voice_player.get());
        assert(b);
    }
    else
    {
        bool b = SOUNDSYSTEM->CloseOutputStream(m_voice_player.get());
        assert(b);
    }

    m_voice_player.reset();
    m_voice_active = false;

    if(talking)
        m_listener->OnUserStateChange(*this);

    //reset existing voice log
    if(GetAudioFolder().length())
        m_clientnode->voicelogger().CancelLog(GetUserID());

    MYTRACE(ACE_TEXT("Shutdown voice player for %s\n"), GetNickname().c_str());
}

void ClientUser::ResetAudioFilePlayer()
{
    if (!m_audiofile_player)
        return;

    bool active = IsAudioActive(STREAMTYPE_MEDIAFILE_AUDIO);

    if(m_snd_duplexmode)
    {
        bool b = SOUNDSYSTEM->RemoveDuplexOutputStream(m_clientnode,
                                                       m_audiofile_player.get());
        assert(b);
    }
    else
    {
        bool b = SOUNDSYSTEM->CloseOutputStream(m_audiofile_player.get());
        assert(b);
    }
    m_audiofile_player.reset();
    m_audiofile_active = false;

    if(active)
        m_listener->OnUserStateChange(*this);

    MYTRACE(ACE_TEXT("Shutdown media file player for %s\n"), GetNickname().c_str());
}

audio_player_t ClientUser::LaunchAudioPlayer(const teamtalk::AudioCodec& codec,
                                             const SoundProperties& sndprop,
                                             StreamType stream_type)
{
    if(m_snddev_error || sndprop.soundgroupid == 0 ||
        sndprop.outputdeviceid == SOUNDDEVICE_IGNORE_ID)
        return audio_player_t();

    if( !ValidAudioCodec(codec) )
        return audio_player_t();

    bool duplex_mode = (m_clientnode->GetFlags() & CLIENT_SNDINOUTPUT_DUPLEX) &&
        m_clientnode->GetMyChannel() == GetChannel();

    int codec_samplerate = GetAudioCodecSampleRate(codec);
    int codec_samples = GetAudioCodecCbSamples(codec);
    int codec_channels = GetAudioCodecChannels(codec);
    if(GetAudioCodecSimulateStereo(codec))
        codec_channels = 2;

    //check whether resampler is required for selected samplerate and channels
    audio_resampler_t resampler;

    int output_samplerate = 0, output_channels = 0, output_samples = 0;
    if(!SOUNDSYSTEM->SupportsOutputFormat(sndprop.outputdeviceid,
                                          codec_channels,
                                          codec_samplerate))
    {
        DeviceInfo dev;
        if(!SOUNDSYSTEM->GetDevice(sndprop.outputdeviceid, dev) ||
           dev.default_samplerate == 0)
            return audio_player_t();

        //choose default sample rate supported by device
        output_samplerate = dev.default_samplerate;
        //choose channels supported by device
        output_channels = dev.GetSupportedOutputChannels(codec_channels);
        //get callback size for new samplerate
        output_samples = CalcSamples(codec_samplerate, codec_samples,
                                     output_samplerate);

        resampler = MakeAudioResampler(codec_channels, codec_samplerate,
                                       output_channels, output_samplerate);

        assert(!resampler.null());
        if(resampler.null())
            return audio_player_t();
    }
    else
    {
        output_samplerate = codec_samplerate;
        output_channels = codec_channels;
        output_samples = codec_samples;
    }

    AudioPlayer* audio_player = NULL;
    switch(codec.codec)
    {
#if defined(ENABLE_SPEEX)
    case teamtalk::CODEC_SPEEX :
        ACE_NEW_RETURN(audio_player,
                       SpeexPlayer(sndprop.soundgroupid, GetUserID(),
                                   stream_type, m_clientnode->audiomuxer(),
                                    codec, resampler),
                        audio_player_t());
        break;
#endif
#if defined(ENABLE_SPEEX)
    case teamtalk::CODEC_SPEEX_VBR :
        ACE_NEW_RETURN(audio_player,
                       SpeexPlayer(sndprop.soundgroupid, GetUserID(),
                                   stream_type, m_clientnode->audiomuxer(),
                                   codec, resampler), audio_player_t());
        break;
#endif
#if defined(ENABLE_OPUS)
    case teamtalk::CODEC_OPUS :
        ACE_NEW_RETURN(audio_player,
                       OpusPlayer(sndprop.soundgroupid, GetUserID(),
                                  stream_type, m_clientnode->audiomuxer(),
                                  codec, resampler),
                       audio_player_t());
        break;
#endif
    default:
        MYTRACE(ACE_TEXT("Cannot launch player for user #%d. Codec %d is unknown\n"),
                GetUserID(), codec.codec);
        return audio_player_t();
    }

    if(!audio_player)
        return audio_player_t();

    audio_player_t ret(audio_player);

    m_snd_duplexmode = duplex_mode;

    //only launch in duplex mode if it's "my" channel
    bool success;
    if(m_snd_duplexmode)
    {
        MYTRACE(ACE_TEXT("Launching duplex player for #%d \"%s\", SampleRate %d, Channels %d, Callback %d\n"),
                GetUserID(), GetNickname().c_str(), output_samplerate, output_channels, output_samples);
        success = SOUNDSYSTEM->AddDuplexOutputStream(m_clientnode,
                                                     audio_player);
    }
    else
    {
        MYTRACE(ACE_TEXT("Launching player for #%d \"%s\", SampleRate %d, Channels %d, Callback %d\n"),
                GetUserID(), GetNickname().c_str(), output_samplerate, output_channels, output_samples);
        success = SOUNDSYSTEM->OpenOutputStream(audio_player, sndprop.outputdeviceid,
                                                sndprop.soundgroupid, output_samplerate,
                                                output_channels, output_samples);
    }

    MYTRACE_COND(!success, ACE_TEXT("Failed to launch player for #%d\n"), GetUserID());
    
    if(success)
    {
        // don't make sense to use auto position on duplex but just ignore return value
        SOUNDSYSTEM->SetAutoPositioning(audio_player, true);
        if(SOUNDSYSTEM->IsAutoPositioning(sndprop.soundgroupid))
            SOUNDSYSTEM->AutoPositionPlayers(sndprop.soundgroupid, false);
    }
    else
    {
        m_snddev_error = true;
        m_listener->OnInternalError(TT_INTERR_SNDOUTPUT_FAILURE,
                                    ACE_TEXT("Failed to open sound output device"));
        return audio_player_t();
    }

    return ret;
}

bool ClientUser::LaunchVoicePlayer(const teamtalk::AudioCodec& codec,
                                   const struct SoundProperties& sndprop)
{
    TTASSERT(!m_voice_player);
    if (m_voice_player)
        return false;
    m_voice_player = LaunchAudioPlayer(codec, sndprop, STREAMTYPE_VOICE);
    if (!m_voice_player)
        return false;

    SetDirtyProps();
    m_voice_player->SetAudioBufferSize(GetAudioStreamBufferSize(STREAMTYPE_VOICE));

    //start timer to monitor when to stop stream
    if(!m_clientnode->TimerExists(USER_TIMER_VOICE_PLAYBACK_ID, GetUserID()))
    {
        int msec = GetAudioCodecCbMillis(codec);
        if(msec>0)
        {
            ACE_Time_Value tm(msec/1000, (msec % 1000) * 1000);
            long timerid = m_clientnode->StartUserTimer(USER_TIMER_VOICE_PLAYBACK_ID,
                                                        GetUserID(), 0, tm, tm);
            TTASSERT(timerid >= 0);
        }
    }
    return true;
}

bool ClientUser::LaunchAudioFilePlayer(const teamtalk::AudioCodec& codec,
                                       const SoundProperties& sndprop)
{
    TTASSERT(!m_audiofile_player);
    if (m_audiofile_player)
        return false;

    m_audiofile_player = LaunchAudioPlayer(codec, sndprop, STREAMTYPE_MEDIAFILE_AUDIO);
    if (!m_audiofile_player)
        return false;

    SetDirtyProps();
    m_audiofile_player->SetAudioBufferSize(GetAudioStreamBufferSize(STREAMTYPE_MEDIAFILE_AUDIO));

    //start timer to monitor when to stop stream
    if(!m_clientnode->TimerExists(USER_TIMER_MEDIAFILE_AUDIO_PLAYBACK_ID, GetUserID()))
    {
        int msec = GetAudioCodecCbMillis(codec);
        if(msec>0)
        {
            ACE_Time_Value tm(msec/1000, (msec % 1000) * 1000);
            long timerid = m_clientnode->StartUserTimer(USER_TIMER_MEDIAFILE_AUDIO_PLAYBACK_ID,
                                                        GetUserID(), 0, tm, tm);
            TTASSERT(timerid >= 0);
        }
    }

    m_listener->OnUserStateChange(*this);

    return true;
}

void ClientUser::SetDirtyProps()
{
    SetPosition(STREAMTYPE_VOICE, m_voice_position[0], m_voice_position[1], m_voice_position[2]);
    SetPosition(STREAMTYPE_MEDIAFILE_AUDIO, m_audiofile_position[0], m_audiofile_position[1], m_audiofile_position[2]);

    SetVolume(STREAMTYPE_VOICE, GetVolume(STREAMTYPE_VOICE));
    SetVolume(STREAMTYPE_MEDIAFILE_AUDIO, GetVolume(STREAMTYPE_MEDIAFILE_AUDIO));

    SetMute(STREAMTYPE_VOICE, IsMute(STREAMTYPE_VOICE));
    SetMute(STREAMTYPE_MEDIAFILE_AUDIO, IsMute(STREAMTYPE_MEDIAFILE_AUDIO));

    SetPlaybackStoppedDelay(STREAMTYPE_VOICE, GetPlaybackStoppedDelay(STREAMTYPE_VOICE));
    SetPlaybackStoppedDelay(STREAMTYPE_MEDIAFILE_AUDIO, GetPlaybackStoppedDelay(STREAMTYPE_MEDIAFILE_AUDIO));

    SetStereo(STREAMTYPE_VOICE, m_voice_stereo & STEREO_LEFT, m_voice_stereo & STEREO_RIGHT);
    SetStereo(STREAMTYPE_MEDIAFILE_AUDIO, m_audiofile_stereo & STEREO_LEFT, m_audiofile_stereo & STEREO_RIGHT);
}


ACE_Message_Block* ClientUser::GetVideoCaptureFrame()
{
#if defined(ENABLE_VPX)
    if (m_vidcap_player)
    {
        //if(!m_voice_player.null() && GetMediaFileBufferSize() &&
        //   m_voice_player->GetPlayedPacketNo())
        //{
        //    uint32_t audio_tm = m_voice_player->GetPlayedPacketTime();
        //    uint32_t video_tm;
        //    if(m_vidcap_player->GetNextFrameTime(&video_tm) &&
        //       W32_GT(video_tm, audio_tm))
        //       return NULL;

        //    return m_vidcap_player->GetNextFrame();
        //}
        return m_vidcap_player->GetNextFrame();
    }
#endif /* ENABLE_VPX */
    return NULL;
}

bool ClientUser::GetVideoCaptureCodec(VideoCodec& codec) const
{
#if defined(ENABLE_VPX)
    if (m_vidcap_player)
    {
        codec = m_vidcap_player->GetVideoCodec();
        return true;
    }
#endif
    return false;
}

void ClientUser::CloseVideoCapturePlayer()
{
#if defined(ENABLE_VPX)
    //notify that we're closing video player
    bool notify = m_vidcap_player.get() != nullptr;
    m_vidcap_player.reset();

    if(notify)
    {
        m_listener->OnUserVideoCaptureFrame(GetUserID(), 0);
        m_listener->OnUserStateChange(*this);
    }
#endif
}

ACE_Message_Block* ClientUser::GetVideoFileFrame()
{
#if defined(ENABLE_VPX)
    if (m_videofile_player)
    {
        if (m_audiofile_player && GetAudioStreamBufferSize(STREAMTYPE_MEDIAFILE_AUDIO) &&
           m_audiofile_player->GetPlayedPacketNo())
        {
            uint32_t audio_tm = m_audiofile_player->GetPlayedPacketTime();
            return m_videofile_player->GetNextFrame(&audio_tm);
        }
        return m_videofile_player->GetNextFrame();
    }
#endif /* ENABLE_VPX */

    return NULL;
}

bool ClientUser::GetVideoFileCodec(VideoCodec& codec) const
{
#if defined(ENABLE_VPX)
    if (m_videofile_player)
    {
        codec = m_videofile_player->GetVideoCodec();
        return true;
    }
#endif /* ENABLE_VPX */
    return false;
}

void ClientUser::CloseVideoFilePlayer()
{
#if defined(ENABLE_VPX)
    //notify that we're closing video player
    bool notify = m_videofile_player.get() != nullptr;
    m_videofile_player.reset();

    if(notify)
    {
        m_listener->OnUserMediaFileVideoFrame(GetUserID(), 0);
        m_listener->OnUserStateChange(*this);
    }
#endif /* ENABLE_VPX */
}


bool ClientUser::GetDesktopWindow(DesktopWindow& wnd)
{
    if(m_desktop.null())
        return false;

    wnd.width = m_desktop->GetWidth();
    wnd.height = m_desktop->GetHeight();
    wnd.session_id = m_desktop->GetSessionID();
    wnd.rgb_mode = m_desktop->GetRGBMode();

    return true;
}

bool ClientUser::GetDesktopWindow(char* buffer, int length)
{
    if(m_desktop.null())
        return false;

    int bmp_size = 0;
    const char* bmp = m_desktop->GetBitmap(&bmp_size);
    TTASSERT(length == bmp_size);
    if(bmp && length == bmp_size)
    {
        ACE_OS::memcpy(buffer, bmp, bmp_size);
        return true;
    }
    return false;
}

void ClientUser::CloseDesktopSession()
{
    //notify that we're closing session
    bool notify = !m_desktop.null();
    m_desktop.reset();
    m_block_fragments.clear();
    m_dup_blocks.clear();
    m_desktop_queue.clear();
    m_acked_desktoppackets.clear();

    if(m_clientnode->TimerExists(USER_TIMER_DESKTOPACKPACKET_ID, GetUserID()))
        m_clientnode->StopUserTimer(USER_TIMER_DESKTOPACKPACKET_ID, GetUserID());
    ResetDesktopInputTx();

    if(notify)
    {
        m_listener->OnUserDesktopWindow(GetUserID(), 0);
        m_listener->OnUserStateChange(*this);
    }
}

void ClientUser::ResetDesktopInputRx()
{
    m_desktop_input_rx.clear();
    m_desktop_input_rx_pktno = 0;
}

void ClientUser::ResetDesktopInputTx()
{
    m_desktop_input_tx.clear();
    m_desktop_input_rtx.clear();
    m_desktop_input_tx_pktno = 0;
}

void ClientUser::SetLocalSubscriptions(Subscriptions mask)
{
    m_localsubscriptions = mask;
}

bool ClientUser::LocalSubscribes(const FieldPacket& packet) const
{
    clientchannel_t mychan = m_clientnode->GetMyChannel();
    clientchannel_t userchan = GetChannel();

    Subscriptions local_subs = SUBSCRIBE_NONE;
    Subscriptions local_intercept_subs = SUBSCRIBE_NONE;

    switch(packet.GetKind())
    {
    case PACKET_KIND_VOICE :
    case PACKET_KIND_VOICE_CRYPT :
        local_subs = SUBSCRIBE_VOICE;
        local_intercept_subs = SUBSCRIBE_INTERCEPT_VOICE;
        break;
    case PACKET_KIND_VIDEO :
    case PACKET_KIND_VIDEO_CRYPT :
        local_subs = SUBSCRIBE_VIDEOCAPTURE;
        local_intercept_subs = SUBSCRIBE_INTERCEPT_VIDEOCAPTURE;
        break;
    case PACKET_KIND_MEDIAFILE_AUDIO :
    case PACKET_KIND_MEDIAFILE_VIDEO :
        local_subs = SUBSCRIBE_MEDIAFILE;
        local_intercept_subs = SUBSCRIBE_INTERCEPT_MEDIAFILE;
        break;
    case PACKET_KIND_DESKTOP :
    case PACKET_KIND_DESKTOP_CRYPT :
    case PACKET_KIND_DESKTOP_ACK :
    case PACKET_KIND_DESKTOP_ACK_CRYPT :
    case PACKET_KIND_DESKTOPCURSOR :
    case PACKET_KIND_DESKTOPCURSOR_CRYPT :
        local_subs = SUBSCRIBE_DESKTOP;
        local_intercept_subs = SUBSCRIBE_INTERCEPT_DESKTOP;
        break;
    case PACKET_KIND_DESKTOPINPUT :
    case PACKET_KIND_DESKTOPINPUT_CRYPT :
    case PACKET_KIND_DESKTOPINPUT_ACK :
    case PACKET_KIND_DESKTOPINPUT_ACK_CRYPT :
        local_subs = SUBSCRIBE_DESKTOPINPUT;
        local_intercept_subs = SUBSCRIBE_NONE;
        break;
    default :
        assert(0);
    }

    if(local_subs && local_intercept_subs)
    {
        if((m_localsubscriptions & local_subs) && !mychan.null())
        {
            if(mychan->GetChannelID() == packet.GetChannel())
                return true;
        }
        if((m_localsubscriptions & local_intercept_subs) && !userchan.null())
        {
            if(userchan->GetChannelID() == packet.GetChannel())
                return true;
        }
        return false;
    }
    return true;
}

bool ClientUser::PeerSubscribes(const FieldPacket& packet) const
{
    switch(packet.GetKind())
    {
    case PACKET_KIND_VOICE :
    case PACKET_KIND_VOICE_CRYPT :
        return (m_peersubscriptions &
            (SUBSCRIBE_VOICE | SUBSCRIBE_INTERCEPT_VOICE));
    case PACKET_KIND_VIDEO :
    case PACKET_KIND_VIDEO_CRYPT :
        return (m_peersubscriptions &
            (SUBSCRIBE_VIDEOCAPTURE | SUBSCRIBE_INTERCEPT_VIDEOCAPTURE));
    case PACKET_KIND_MEDIAFILE_AUDIO :
    case PACKET_KIND_MEDIAFILE_AUDIO_CRYPT :
    case PACKET_KIND_MEDIAFILE_VIDEO :
    case PACKET_KIND_MEDIAFILE_VIDEO_CRYPT :
        return (m_peersubscriptions &
            (SUBSCRIBE_MEDIAFILE | SUBSCRIBE_INTERCEPT_MEDIAFILE));
    case PACKET_KIND_DESKTOP :
    case PACKET_KIND_DESKTOP_CRYPT :
    case PACKET_KIND_DESKTOP_ACK :
    case PACKET_KIND_DESKTOP_ACK_CRYPT :
    case PACKET_KIND_DESKTOPCURSOR :
    case PACKET_KIND_DESKTOPCURSOR_CRYPT :
        return (m_peersubscriptions &
            (SUBSCRIBE_DESKTOP | SUBSCRIBE_INTERCEPT_DESKTOP));
    case PACKET_KIND_DESKTOPINPUT :
    case PACKET_KIND_DESKTOPINPUT_CRYPT :
    case PACKET_KIND_DESKTOPINPUT_ACK :
    case PACKET_KIND_DESKTOPINPUT_ACK_CRYPT :
        return (m_peersubscriptions & SUBSCRIBE_DESKTOPINPUT);
    default :
        assert(0);
    }
    return true;
}

ACE_TString ClientUser::GetAudioFileName() const
{
    return m_clientnode->voicelogger().GetVoiceLogFileName(GetUserID());
}

void ClientUser::SetAudioFileFormat(AudioFileFormat aff)
{
    m_aff = aff;
    if(aff == AFF_NONE)
        m_clientnode->voicelogger().CancelLog(GetUserID());
}

AudioFileFormat ClientUser::GetAudioFileFormat() const
{
    return m_aff;
}

void ClientUser::SetAudioStreamBufferSize(StreamType stream_type, int msec)
{
    switch(stream_type)
    {
    case STREAMTYPE_VOICE :
        m_voice_buf_msec = msec;
        if (m_voice_player)
            m_voice_player->SetAudioBufferSize(msec);
        break;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        m_media_buf_msec = msec;
        if (m_audiofile_player)
            m_audiofile_player->SetAudioBufferSize(msec);
        break;
    default :
        TTASSERT(0);
        break;
    }
}

int ClientUser::GetAudioStreamBufferSize(StreamType stream_type) const
{
    switch(stream_type)
    {
    case STREAMTYPE_VOICE :
        return m_voice_buf_msec;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        return m_media_buf_msec;
    default : return 0;
    }
}
