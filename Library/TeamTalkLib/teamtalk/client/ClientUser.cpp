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
#include "ClientNodeEvent.h"
#include "ClientNodeBase.h"

#include <teamtalk/ttassert.h>
#include <teamtalk/Commands.h>
#include <teamtalk/CodecCommon.h>

using namespace std;
using namespace teamtalk;
using namespace std::placeholders;

#define TIMEOUT_STOP_AUDIO_PLAYBACK          30000    //msec for timeout of when to stop stream
#define TIMEOUT_STOP_VIDEOFILE_PLAYBACK      5000

int JitterCalculator::PacketReceived(const int streamid, const int nominal_delay, bool& isfirststreampacket)
{
    //Note: reentrancy is assumed to be guarded outside this function

    isfirststreampacket = false;
    if (streamid != m_current_stream)
    {
        m_current_stream = streamid;
        isfirststreampacket = true;
    }

    // Check if jitter control is configured at all
    if ((m_fixed_jitter_delay_ms <= 0) && !m_use_adaptive_jitter_control)
    {
        return 0;
    }

    uint32_t packet_reception_time = GETTIMESTAMP();

    // Adaptive jitter control.
    // The basic idea is still to the delay playout at the start of a stream just like with only a fixed delay.
    // The adaptive control only tries to make the delay match the expected jitter of an entire voice stream by measuring the actual jitters.
    // So the whole solution is still geared towards relatively short voice sessions (e.g. a PTT session or VAD session)
    //
    // The algorithm is aggressive and aims to provide smooth playout even in the worst circumstances at the expense of long delays
    // The idea is that end-users prefer delayed non-jittered voice over jittered voice with shorter delays
    //
    // The adaptive jitter is calculated in two ways:
    // 1. The maximum measured jitter in the last X received packets.
    // 2. The total accumulated jitter of a stream in case the buffers during the stream where insufficient.
    //
    // Both measurements are treated the same way: they are enqueued in a queue of max X deep.
    // X means "number of packets" here and is arbitrarily set to 100 currently.
    // X is important bcause it determines the amount of time (number of packets) it takes to scale down the delay.
    //
    // If the queue reaches X packets, an item is dequeued for every received packet.
    // if the the dequeued jitter is the adaptive delay then a new adaptive delay is determined by finding the highest value in the queue.
    // If a jitter is enqueued that is higher than the adaptive delay than the adaptive delay is immediately set
    // to that jitter. This means that the adaptive delay adapts upwards very quickly and adapts slowly downwards.

    // Note that jitters are tracked on a per-packet basis whereas the goal of the adaptive delay is to set a sufficient buffer
    // for the entire stream. It would be better and easier to queue the accumulated jitter per stream.
    // The reason that's not done is because there is no good heuristic for when to dequeue items as the number
    // of voice streams is much more unpredictable and unreliable as a measurement than the number of packets.
    // Other, time-based, approaches add too much complexity and ultimately have the same problem.
    // At the same time, using the packet rate seems a sufficient approximation and this has at least been proven to work really well on 
    // extremely high-jitter networks. It for example creates a jitter window of 1s if 10ms voice packets are used and 24s with a 240ms packet-rate. 
    // It is likely that networks on which TeamTalk is configured with a slow packets rate are also networks on which prolonged jitter
    // is more likely and vice versa on networks where a very fast packet rate is used. 

    int jitter_delay = 0;
    if (isfirststreampacket)
    {
        //Start of new stream. Reset stream stats
        m_current_playout_buffer = 0;
        m_stream_silence = 0;
        m_stream_delay = 0;

        // This is the main point of jitter control where we delay the initial stream playout.
        // A large delay here is not noticable at jitter by the end-users as jitter because the playout hasn't started yet.
        // They just experience delays in their conversation
        // If this initial delay/buffer is insufficient then that is detected and those packets are then delayed
        // mid-stream (duration: fixed jitter buffer delay)
        jitter_delay = m_fixed_jitter_delay_ms;
        if (m_use_adaptive_jitter_control)
            jitter_delay += m_adaptive_delay;

        MYTRACE_COND((jitter_delay > 0), ACE_TEXT("Jitter delay for new stream of user #%d: %d ms.\n"), m_userid, jitter_delay);
    }
    else if (m_use_adaptive_jitter_control)
    {
        // Measure the inter-packet delay and jitter
        // Note that the inter-packet-time is likely to be zero. This typically happens on satelite links.
        // These have high jitter that includes many seconds of non-reception, followed by a burst of delayed packets
        // that are received all at once. The first packet in this burst has a high inter-packet-time and the subsequent ones
        // have an inter-packet-time of (near-)zero.
        int msec_since_last_packet = (packet_reception_time - m_lastpacket_time);

        // Calculate the jitter for this packet
        // NB: the timer resolution of some platforms is crappy and results in small positive of negative jitter
        // even though the inter-packet delay was completely nominal.
        // This makes the adaptive jitter delay downward adjustments slightly slower.
        int jitter_last_packet = (msec_since_last_packet - nominal_delay);
        //MYTRACE(ACE_TEXT("Jitter of last packet %d ms. Nominal delay %d msec_since_last: %d\n"),
        //                    jitter_last_packet, nominal_delay, msec_since_last_packet);

        // Keep track of the actual jitter buffer in the playout buffer by adding/removing the last jitter (might be negative jitter)
        m_current_playout_buffer -= jitter_last_packet;


        if (m_current_playout_buffer < 0)
        {
            // At this point the accumulated jitter during the stream exceeds the buffer set at the start.
            // This will result in noticeable jitter for the end-user.

            m_stream_silence += std::abs(m_current_playout_buffer);

            MYTRACE(ACE_TEXT("Jitter exceeds buffered playout time of user #%d. Silence: %d ms. Accumulated silence: %d ms. Accumulated delay %d ms\n"),
                m_userid, std::abs(m_current_playout_buffer), m_stream_silence, m_stream_delay);

            //Set the playout buffer tracker to zero (ignore the underrun)
            m_current_playout_buffer = 0;

            // There's aleady silence now. Add the fixed delay as a new buffer into the playout to ensure some buffering for the remainder of this session.
            // This makes the immediate jitter silence worse, but improves subsequent jitters
            jitter_delay = m_fixed_jitter_delay_ms;

            //The buffering so far was inadequate for this stream. Adapt the new adaptive delay for the next stream.
            //The next stream should be initially delayed by the accumulated silence in this stream plus already inserted delay in this stream so-far.
            //Do this by changing the packet jitter so it is processed below as the new max.
            jitter_last_packet = m_stream_delay + m_stream_silence;

            // Add the in-stream inserted delay to the total delay in the stream
            m_stream_delay += m_fixed_jitter_delay_ms;
        }

        // At this point, the jitter_last_packet contains the jitter that will be stored and processed.
        // Note that the jitter_last_packet does not *have* to be modified in the IF statement above to lead to a new adaptive delay.
        // it is perfectly possible for an unmodified jitter_last_packet to become the new adaptive delay. This typically happens if the current adaptive delay
        // is dequeued during the stream and a new (lower) adaptive delay is chosen. This doesn't affect the playout buffer, so a high jitter that
        // is within the current playout buffer but above the (new) current adaptive delay will still become the next adaptive delay.

        // Cap the jitter delay to the configured max.
        if ((m_max_adaptive_delay_msec > 0) && (jitter_last_packet > m_max_adaptive_delay_msec))
        {
            MYTRACE(ACE_TEXT("Adaptive jitter delay capped to configured maximum of %d for user #%d. Received jitter %d, .\n"),
                m_max_adaptive_delay_msec, m_userid, jitter_last_packet);
            jitter_last_packet = m_max_adaptive_delay_msec;
        }

        // Keep track of the jitters in the last X packets
        m_last_jitters.push_back(jitter_last_packet);

        if (m_last_jitters.size() > 100) // This max size is arbitrary but determines the scale down time
        {
            // If the queue exceeds size, remove the first one.
            // If that was the current adaptive delay, calculate the new adaptive delay by finding the highest in the queue
            int dequeuedjitter = m_last_jitters.front();
            m_last_jitters.pop_front();

            if (dequeuedjitter == m_adaptive_delay)
            {
                m_adaptive_delay = 0;
                deque<int>::iterator ii;
                for (ii = m_last_jitters.begin(); ii != m_last_jitters.end(); ++ii)
                {
                    if (*ii > m_adaptive_delay)
                        m_adaptive_delay = *ii;
                }
                MYTRACE(ACE_TEXT("Adaptive delay was dequeued. New adaptive jitter delay determined: %d ms for user #%d.\n"),
                                    m_adaptive_delay, m_userid);
            }
        }

        if (jitter_last_packet > m_adaptive_delay)
        {
            //Last jitter is the highest measured. This will be the new highest
            m_adaptive_delay = jitter_last_packet;
            MYTRACE(ACE_TEXT("New adaptive jitter delay for user #%d: %d ms.\n"), m_userid, m_adaptive_delay);
        }
    }

    m_lastpacket_time = packet_reception_time;
    m_current_playout_buffer += jitter_delay;
    m_stream_delay += jitter_delay;
    m_current_stream = streamid;
    return jitter_delay;
}

void JitterCalculator::SetConfig(const JitterControlConfig& config)
{
    //Note: reentrancy is assumed to be guarded outside this function
    m_fixed_jitter_delay_ms = config.fixedDelayMSec;
    m_use_adaptive_jitter_control = config.useAdativeDejitter;
    m_max_adaptive_delay_msec = config.maxAdaptiveDelayMSec;
    // Reset stats
    m_lastpacket_time = 0;
    m_current_stream = 0;
    m_current_playout_buffer = 0;
    m_stream_silence = 0;
    m_stream_delay = 0;

    m_last_jitters.clear();

    m_adaptive_delay = 0;

    //Set and queue the current active adaptive delay
    if (config.activeAdaptiveDelayMSec > 0)
    {
        if (config.activeAdaptiveDelayMSec > config.maxAdaptiveDelayMSec)
        {
            m_adaptive_delay = config.maxAdaptiveDelayMSec;
        }
        else
        {
            m_adaptive_delay = config.activeAdaptiveDelayMSec;
        }
        m_last_jitters.push_back(m_adaptive_delay);
    }
};

bool JitterCalculator::GetConfig(JitterControlConfig& config)
{
    //Note: reentrancy is assumed to be guarded outside this function
    config.fixedDelayMSec = m_fixed_jitter_delay_ms;
    config.useAdativeDejitter = m_use_adaptive_jitter_control;
    config.maxAdaptiveDelayMSec = m_max_adaptive_delay_msec;
    config.activeAdaptiveDelayMSec = m_adaptive_delay;
    return true;
};


ClientUser::ClientUser(int userid, ClientNodeBase* clientnode,
                       ClientListener* listener,
                       soundsystem::soundsystem_t sndsys)
                       : User(userid)
                       , m_clientnode(clientnode)
                       , m_listener(listener)
                       , m_soundsystem(sndsys)
                       , m_jitter_calculator(userid)
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
    if (!LocalSubscribes(SUBSCRIBE_VOICE | SUBSCRIBE_INTERCEPT_VOICE) || m_channel.expired())
        ResetVoicePlayer();
    
    if (!LocalSubscribes(SUBSCRIBE_MEDIAFILE | SUBSCRIBE_INTERCEPT_MEDIAFILE) || m_channel.expired())
    {
        ResetAudioFilePlayer();
        CloseVideoFilePlayer();
    }
    
    if(!LocalSubscribes(SUBSCRIBE_VIDEOCAPTURE | SUBSCRIBE_INTERCEPT_VIDEOCAPTURE) || m_channel.expired())
        CloseVideoCapturePlayer();
    
    if(!LocalSubscribes(SUBSCRIBE_DESKTOP | SUBSCRIBE_INTERCEPT_DESKTOP) || m_channel.expired())
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

    m_stats.voicepackets_recv += m_voice_player->GetNumAudioPacketsRecv(true);
    m_stats.voicepackets_lost += m_voice_player->GetNumAudioPacketsLost(true);

    //MYTRACE_COND(n_blocks, ACE_TEXT("User #%d has %d new voice block at %u\n"), GetUserID(), n_blocks, GETTIMESTAMP());

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
    bool changed = active != IsAudioActive(STREAMTYPE_MEDIAFILE_AUDIO);
    m_audiofile_active = active;
    if(changed)
        m_listener->OnUserStateChange(*this);

    m_stats.mediafile_audiopackets_recv += m_audiofile_player->GetNumAudioPacketsRecv(true);
    m_stats.mediafile_audiopackets_lost += m_audiofile_player->GetNumAudioPacketsLost(true);

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
    if (!chan)
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


int ClientUser::TimerVoiceJitterBuffer()
{
    while (!m_jitterbuffer.empty())
    {
        auto queuedVoicePacket = m_jitterbuffer.front();
        m_jitterbuffer.pop();
        //MYTRACE(ACE_TEXT("Feed packet from jitter buffer to playout\n"));
        FeedVoicePacketToPlayer(*queuedVoicePacket);
    }
    //MYTRACE(ACE_TEXT("Jitter buffer emptied\n"));
    return -1; //Single shot time
}


void ClientUser::AddVoicePacket(const VoicePacket& audpkt,
                                const struct SoundProperties& sndprop,
                                bool allowrecord)
{
    ASSERT_REACTOR_THREAD(*m_clientnode->GetEventLoop());

    clientchannel_t chan = GetChannel();
    if (!chan || chan->GetChannelID() != audpkt.GetChannel())
    {
        MYTRACE(ACE_TEXT("Received voice packet #%d outside channel, %d != %d\n"),
                GetUserID(), audpkt.GetChannel(),
                !chan? 0 : chan->GetChannelID());
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

    m_voice_player->SetNoRecording(!allowrecord);
    
    // Jitter buffer implementation:
    // Delay the playout of received packets at the start of a stream.
    // This delay acts as a time buffer for network jitter
    // The delayed packets are enqueued at the start of a stream and then emptied into the playout buffer via a single-shot timer.
    // This means that the actual buffering is done in the playout queue. The jitter buffer only delays the inital playout.
    // After that initial delay, all received packets are directly fed into the playout buffer

    // Guard both the jitter-buffer and the jitter-calculator
    bool isfirststeampacket = false;
    int jitter_delay = m_jitter_calculator.PacketReceived(audpkt.GetStreamID(), GetAudioCodecCbMillis(chan->GetAudioCodec()), isfirststeampacket);

    if (isfirststeampacket)
        m_listener->OnUserFirstStreamVoicePacket(*this, audpkt.GetStreamID());

    // Put packets in the jitter buffer if we're already queueing or if the jitter calculation resulted in a delay
    if ((jitter_delay > 0) || (!m_jitterbuffer.empty()))
    {
        //MYTRACE(ACE_TEXT("Enqueue Received voice packet. Start of stream %d, Already queueing %d \n"),
        //                      (audpkt.GetStreamID() != m_current_stream), (!m_jitterbuffer.empty()));

        audiopacket_t queuedVoicePacket(new VoicePacket(audpkt));

        m_jitterbuffer.push(queuedVoicePacket);

        // start timer to dequeue the buffered packets
        if ((jitter_delay > 0) && (!m_clientnode->TimerExists(USER_TIMER_JITTER_BUFFER_ID, GetUserID())))
        {
            ACE_Time_Value tm(jitter_delay / 1000, (jitter_delay % 1000) * 1000);
            // Set up a single shot timer (interval = 0)
            long timerid = m_clientnode->StartUserTimer(USER_TIMER_JITTER_BUFFER_ID, GetUserID(), 0, tm);
            TTASSERT(timerid >= 0);
        }
    }
    else
    {
        // Immediate enqueueing into the playout buffer
        //MYTRACE(ACE_TEXT("Direct playout of voice packet.\n"));
        FeedVoicePacketToPlayer(audpkt);
    }
}

void ClientUser::FeedVoicePacketToPlayer(const VoicePacket& audpkt)
{
    if (!m_voice_player)
        return;

    clientchannel_t chan = GetChannel();
    VoiceLogger& voice_logger = m_clientnode->voicelogger();
    
    audiopacket_t reassem_pkt = m_voice_player->QueuePacket(audpkt);

    //store in voicelog
    if (GetAudioFolder().length() && m_voice_player->IsRecordingAllowed())
    {
        if(audpkt.HasFragments())
        {
            if(reassem_pkt)
                voice_logger.AddVoicePacket(*this, *chan, *reassem_pkt);
        }
        else
            voice_logger.AddVoicePacket(*this, *chan, audpkt);
    }

    // MYTRACE(ACE_TEXT("Added audio packet #%d, TS: %u, Local TS: %u\n"),
    //         (int)audpkt.GetPacketNumber(), audpkt.GetTime(), GETTIMESTAMP());
    if(!m_snd_duplexmode)
    {
        if (m_soundsystem->IsStreamStopped(m_voice_player.get()) )
        {
            m_soundsystem->StartStream(m_voice_player.get());
            MYTRACE(ACE_TEXT("Starting voice stream for #%d\n"), GetUserID());
        }
    }
}

void ClientUser::AddAudioFilePacket(const AudioFilePacket& audpkt,
                                    const struct SoundProperties& sndprop)
{
    ASSERT_REACTOR_THREAD(*m_clientnode->GetEventLoop());

    clientchannel_t chan = GetChannel();
    if (!chan || chan->GetChannelID() != audpkt.GetChannel())
    {
        MYTRACE(ACE_TEXT("Received media file packet #%d outside channel, %d != %d\n"),
                GetUserID(), audpkt.GetChannel(),
                !chan? 0 : chan->GetChannelID());
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
        if (m_soundsystem->IsStreamStopped(m_audiofile_player.get()) )
        {
            m_soundsystem->StartStream(m_audiofile_player.get());
            MYTRACE(ACE_TEXT("Starting media audio stream for #%d\n"), GetUserID());
        }
    }
}

void ClientUser::AddVideoCapturePacket(const VideoCapturePacket& p,
                                       const ClientChannel& chan)
{
    ASSERT_REACTOR_THREAD(*m_clientnode->GetEventLoop());

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
    ASSERT_REACTOR_THREAD(*m_clientnode->GetEventLoop());

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
    ASSERT_REACTOR_THREAD(*m_clientnode->GetEventLoop());

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
    if (m_desktop && m_desktop->GetSessionID() != session_id &&
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

    if (!m_desktop)
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
    if (!m_desktop)
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
        MYTRACE(ACE_TEXT("Querying incorrect stream type 0x%x\n"), stream_type);
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

void ClientUser::SetJitterControl(const StreamType stream_type, const JitterControlConfig& config)
{
    if (stream_type != STREAMTYPE_VOICE)
        return;

    m_jitter_calculator.SetConfig(config);
}

bool ClientUser::GetJitterControl(const StreamType stream_type, JitterControlConfig& config)
{
    if (stream_type != STREAMTYPE_VOICE)
        return false;

    return m_jitter_calculator.GetConfig(config);
}

void ClientUser::SetVolume(StreamType stream_type, int volume)
{
    switch(stream_type)
    {
    case STREAMTYPE_VOICE :
        if (m_voice_player)
            m_soundsystem->SetVolume(m_voice_player.get(), volume);
        m_voice_volume = volume;
        break;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        if (m_audiofile_player)
            m_soundsystem->SetVolume(m_audiofile_player.get(), volume);
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
            m_soundsystem->SetMute(m_voice_player.get(), mute);
        m_voice_mute = mute;
        break;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        if (m_audiofile_player)
            m_soundsystem->SetMute(m_audiofile_player.get(), mute);
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
            m_soundsystem->SetPosition(m_voice_player.get(), x, y, z);
            m_soundsystem->SetAutoPositioning(m_voice_player.get(), false);
        }
        m_voice_position[0] = x;
        m_voice_position[1] = y;
        m_voice_position[2] = z;
        break;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        if (m_audiofile_player)
        {
            m_soundsystem->SetPosition(m_audiofile_player.get(), x, y, z);
            m_soundsystem->SetAutoPositioning(m_audiofile_player.get(), false);
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
        bool b = m_soundsystem->RemoveDuplexOutputStream(m_clientnode,
                                                         m_voice_player.get());
        assert(b);
    }
    else
    {
        bool b = m_soundsystem->CloseOutputStream(m_voice_player.get());
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
        bool b = m_soundsystem->RemoveDuplexOutputStream(m_clientnode,
                                                         m_audiofile_player.get());
        assert(b);
    }
    else
    {
        bool b = m_soundsystem->CloseOutputStream(m_audiofile_player.get());
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

    int codec_samplerate = GetAudioCodecSampleRate(codec);
    int codec_samples = GetAudioCodecCbSamples(codec);
    int codec_channels = GetAudioCodecChannels(codec);
    if(GetAudioCodecSimulateStereo(codec))
        codec_channels = 2;

    //check whether resampler is required for selected samplerate and channels
    audio_resampler_t resampler;

    int output_samplerate = 0, output_channels = 0, output_samples = 0;
    if (!m_soundsystem->SupportsOutputFormat(sndprop.outputdeviceid,
                                             codec_channels,
                                             codec_samplerate))
    {
        soundsystem::DeviceInfo dev;
        if (!m_soundsystem->GetDevice(sndprop.outputdeviceid, dev) ||
            dev.default_samplerate == 0)
            return audio_player_t();

        //choose default sample rate supported by device
        output_samplerate = dev.default_samplerate;
        //choose channels supported by device
        output_channels = dev.GetSupportedOutputChannels(codec_channels);
        //get callback size for new samplerate
        output_samples = CalcSamples(codec_samplerate, codec_samples,
                                     output_samplerate);
        media::AudioFormat infmt(codec_samplerate, codec_channels),
            outfmt(output_samplerate, output_channels);
        resampler = MakeAudioResampler(infmt, outfmt);
        assert(resampler);
        if (!resampler)
            return audio_player_t();
    }
    else
    {
        output_samplerate = codec_samplerate;
        output_channels = codec_channels;
        output_samples = codec_samples;
    }

    auto audiofunc = std::bind(&ClientNodeBase::AudioUserCallback, m_clientnode, _1, _2, _3);

    AudioPlayer* audio_player = NULL;
    switch(codec.codec)
    {
#if defined(ENABLE_SPEEX)
    case teamtalk::CODEC_SPEEX :
        ACE_NEW_RETURN(audio_player,
                       SpeexPlayer(GetUserID(), stream_type, m_soundsystem,
                                   audiofunc, codec, resampler),
                        audio_player_t());
        break;
#endif
#if defined(ENABLE_SPEEX)
    case teamtalk::CODEC_SPEEX_VBR :
        ACE_NEW_RETURN(audio_player,
                       SpeexPlayer(GetUserID(), stream_type, m_soundsystem,
                                   audiofunc, codec, resampler), audio_player_t());
        break;
#endif
#if defined(ENABLE_OPUS)
    case teamtalk::CODEC_OPUS :
        ACE_NEW_RETURN(audio_player,
                       OpusPlayer(GetUserID(), stream_type, m_soundsystem,
                                  audiofunc, codec, resampler),
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

    //only launch in duplex mode if it's "my" channel
    m_snd_duplexmode = (GetChannel() && GetChannel()->GetChannelID() == m_clientnode->GetChannelID());
    m_snd_duplexmode &= m_clientnode->SoundDuplexMode();

    bool success;
    if (m_snd_duplexmode)
    {
        MYTRACE(ACE_TEXT("Launching duplex player for #%d \"%s\", SampleRate %d, Channels %d, Callback %d\n"),
                GetUserID(), GetNickname().c_str(), output_samplerate, output_channels, output_samples);
        success = m_soundsystem->AddDuplexOutputStream(m_clientnode, audio_player);
    }
    else
    {
        MYTRACE(ACE_TEXT("Launching player for #%d \"%s\", SampleRate %d, Channels %d, Callback %d\n"),
                GetUserID(), GetNickname().c_str(), output_samplerate, output_channels, output_samples);
        success = m_soundsystem->OpenOutputStream(audio_player, sndprop.outputdeviceid,
                                                  sndprop.soundgroupid, output_samplerate,
                                                  output_channels, output_samples);
    }

    MYTRACE_COND(!success, ACE_TEXT("Failed to launch player for #%d\n"), GetUserID());
    
    if (success)
    {
        // don't make sense to use auto position on duplex but just ignore return value
        m_soundsystem->SetAutoPositioning(audio_player, true);
        if (m_soundsystem->IsAutoPositioning(sndprop.soundgroupid))
            m_soundsystem->AutoPositionPlayers(sndprop.soundgroupid, false);
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

    SetRecordingCloseExtraDelay(GetRecordingCloseExtraDelay());

    SetStereo(STREAMTYPE_VOICE, m_voice_stereo & STEREO_LEFT, m_voice_stereo & STEREO_RIGHT);
    SetStereo(STREAMTYPE_MEDIAFILE_AUDIO, m_audiofile_stereo & STEREO_LEFT, m_audiofile_stereo & STEREO_RIGHT);
}


ACE_Message_Block* ClientUser::GetVideoCaptureFrame()
{
#if defined(ENABLE_VPX)
    if (m_vidcap_player)
    {
        //if(m_voice_player && GetMediaFileBufferSize() &&
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
    if (!m_desktop)
        return false;

    wnd.width = m_desktop->GetWidth();
    wnd.height = m_desktop->GetHeight();
    wnd.session_id = m_desktop->GetSessionID();
    wnd.rgb_mode = m_desktop->GetRGBMode();

    return true;
}

bool ClientUser::GetDesktopWindow(char* buffer, int length)
{
    if (!m_desktop)
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
    bool notify = m_desktop.get() != nullptr;
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

    int mychanid = m_clientnode->GetChannelID();
    int userchanid = (GetChannel() ? GetChannel()->GetChannelID() : 0);

    if(local_subs && local_intercept_subs)
    {
        if ((m_localsubscriptions & local_subs) && mychanid > 0)
        {
            if (mychanid == packet.GetChannel())
                return true;
        }
        if ((m_localsubscriptions & local_intercept_subs) && userchanid > 0)
        {
            if (userchanid == packet.GetChannel())
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
