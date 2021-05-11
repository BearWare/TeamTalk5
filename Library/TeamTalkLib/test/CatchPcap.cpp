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

#include "catch.hpp"

#include "TTUnitTest.h"

#include <iostream>
#include <map>
#include <vector>

#include <pcap/pcap.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <ace/OS.h>
#include <ace/Time_Value.h>

#include <TeamTalk.h>

#include <teamtalk/PacketLayout.h>
#include <teamtalk/client/AudioMuxer.h>
#include <teamtalk/client/StreamPlayers.h>
#include <teamtalk/client/ClientNodeBase.h>
#include <codec/WaveFile.h>
#include <avstream/SoundSystem.h>
#include <bin/dll/TTClientMsg.h>

#if defined(ENABLE_OPUS)
#include <codec/OpusDecoder.h>
#endif

typedef std::map< ACE_Time_Value, std::vector<char> > ttpackets_t;

ttpackets_t GetTTPackets(const ACE_CString& filename,
                         const ACE_CString& srcip,
                         int srcport,
                         const ACE_CString& destip,
                         int destport)
{
    std::map< ACE_Time_Value, std::vector<char> > result;
    ACE_Time_Value first;

    char errbuff[PCAP_ERRBUF_SIZE];
    pcap_t * pcap = pcap_open_offline(filename.c_str(), errbuff);
    REQUIRE(pcap);

    struct pcap_pkthdr *header;
    const u_char *data;
    while (int returnValue = pcap_next_ex(pcap, &header, &data) >= 0)
    {
        struct iphdr *iph = (struct iphdr*)(data + sizeof(struct ethhdr));
        struct sockaddr_in src = {}, dest = {};
        src.sin_addr.s_addr = iph->saddr;
        dest.sin_addr.s_addr = iph->daddr;

        if (srcip.length() && srcip != inet_ntoa(src.sin_addr))
            continue;
        if (destip.length() && destip != inet_ntoa(dest.sin_addr))
            continue;

        unsigned short iphdrlen = iph->ihl * 4;
        switch (iph->protocol)
        {
        case 1:  //ICMP Protocol
            break;

        case 2:  //IGMP Protocol
            break;

        case 6:  //TCP Protocol
            break;

        case 17: //UDP Protocol
        {
            struct udphdr *udph = (struct udphdr*)(data + iphdrlen + sizeof(struct ethhdr));
            if (srcport && srcport != ntohs(udph->source))
                continue;
            if (destport && destport != ntohs(udph->dest))
                continue;

            int header_size =  sizeof(struct ethhdr) + iphdrlen + sizeof(struct udphdr);
            auto dataoffset = reinterpret_cast<const char*>(&data[header_size]);
            auto datalen = ntohs(udph->len) - sizeof(struct udphdr);
            std::vector<char> ttraw(dataoffset, dataoffset + datalen);
            ACE_Time_Value tm(ACE_Time_Value(header->ts.tv_sec, header->ts.tv_usec));
            if (first == ACE_Time_Value())
                first = tm;

            tm -= first;
            result[tm] = ttraw;
            break;
        }
        default:
            break;
        }
    }
    pcap_close(pcap);
    return result;
}

#if defined(ENABLE_OPUS)

TEST_CASE("AudioMuxerJitter")
{
    auto ttpackets = GetTTPackets("testdata/Jitter/netem.pcapng", "10.157.1.34", 10333, "10.157.1.40", 63915);

    const int FPP = 2;
    const int SAMPLERATE = 48000, CHANNELS = 2;
    const double FRMDURATION = .120;
    const int PLAYBACKFRAMESIZE = FPP * FRMDURATION * SAMPLERATE;

    teamtalk::AudioCodec codec;
    codec.codec = teamtalk::CODEC_OPUS;
    codec.opus.samplerate = SAMPLERATE;
    codec.opus.channels = CHANNELS;
    codec.opus.application = OPUS_APPLICATION_VOIP;
    codec.opus.complexity = 10;
    codec.opus.fec = true;
    codec.opus.dtx = true;
    codec.opus.bitrate = 6000;
    codec.opus.vbr = false;
    codec.opus.vbr_constraint = false;
    codec.opus.frame_size = SAMPLERATE * FRMDURATION;
    codec.opus.frames_per_packet = FPP;

    OpusDecode decoder;
    REQUIRE(decoder.Open(SAMPLERATE, CHANNELS));
    std::vector<short> frame(CHANNELS * SAMPLERATE * FRMDURATION * FPP);

    WavePCMFile wavefile;
    REQUIRE(wavefile.NewFile(ACE_TEXT("netem.wav"), SAMPLERATE, CHANNELS));
    uint32_t sampleno = 0;

#define DIRECTRECORDER 0
#if DIRECTRECORDER
    AudioMuxer directrecorder;
    REQUIRE(directrecorder.SaveFile(codec, ACE_TEXT("netem_muxer_direct.wav"), teamtalk::AFF_WAVE_FORMAT));
#endif

    AudioMuxer playbackrecorder(teamtalk::STREAMTYPE_VOICE);
    // playbackrecorder.SetMuxInterval(2000);
    REQUIRE(playbackrecorder.SaveFile(codec, ACE_TEXT("netem_muxer_playback.wav"), teamtalk::AFF_WAVE_FORMAT));

    teamtalk::useraudio_callback_t audiocb = [&](int userid, teamtalk::StreamType stream_type, const media::AudioFrame& frm)
    {
        REQUIRE(playbackrecorder.QueueUserAudio(userid, stream_type, frm));
    };

    std::shared_ptr<teamtalk::OpusPlayer> player;
    auto snd = soundsystem::GetInstance();
    int sndgrp = snd->OpenSoundGroup();

    ACE_Time_Value last;
    for (auto i : ttpackets)
    {
        auto data = &i.second[0];
        auto len = i.second.size();
        switch (teamtalk::FieldPacket(data, len).GetKind())
        {
        case teamtalk::PACKET_KIND_VOICE :
        {
            teamtalk::AudioPacket p(data, len);
            REQUIRE(p.ValidatePacket());
            REQUIRE(!p.HasFragments());
            REQUIRE(!p.HasFrameSizes());
            // std::cout << "User ID " << p.GetSrcUserID() << " Packet No " << p.GetPacketNumber() << " Stream ID " << int(p.GetStreamID()) << std::endl;

            if (!player)
            {
                player.reset(new teamtalk::OpusPlayer(p.GetSrcUserID(), teamtalk::STREAMTYPE_VOICE,
                                                      snd, audiocb, codec, audio_resampler_t()));
                player->SetAudioBufferSize(5000);
                player->SetStoppedTalkingDelay(5000);
                REQUIRE(snd->OpenOutputStream(player.get(), TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL,
                                              sndgrp, SAMPLERATE, CHANNELS, PLAYBACKFRAMESIZE));
                REQUIRE(snd->StartStream(player.get()));
            }

            // no reassembly packets
            REQUIRE(player->QueuePacket(p).get() == nullptr);

            uint16_t opuslen = 0;
            auto opusenc = p.GetEncodedAudio(opuslen);
            REQUIRE(opusenc);
            int encoffset = 0;
            int enclen = opuslen / FPP;
            int frameoffset = 0;

            for (int i=0; i < FPP; ++i)
            {
                int ret = decoder.Decode(&opusenc[encoffset], enclen, &frame[frameoffset * CHANNELS], codec.opus.frame_size);
                REQUIRE(ret == codec.opus.frame_size);
                encoffset += enclen;
                frameoffset += codec.opus.frame_size;
            }
            REQUIRE(frameoffset == PLAYBACKFRAMESIZE);
            wavefile.AppendSamples(&frame[0], PLAYBACKFRAMESIZE);

#if DIRECTRECORDER
            media::AudioFrame frm(media::AudioFormat(SAMPLERATE, CHANNELS),
                                  &frame[0], PLAYBACKFRAMESIZE, sampleno);
            REQUIRE(directrecorder.QueueUserAudio(p.GetSrcUserID(), teamtalk::STREAMTYPE_VOICE, frm));
#endif
            sampleno += PLAYBACKFRAMESIZE;
        }
        }
        ACE_Time_Value wait = i.first - last;
        // std::cout << "Sleeping " << wait << std::endl;
        ACE_OS::sleep(wait);
        last = i.first;
    }

    REQUIRE(snd->CloseOutputStream(player.get()));
    snd->RemoveSoundGroup(sndgrp);
}

#endif /* ENABLE_OPUS */

TEST_CASE("PlaybackJitter")
{
    auto ttpackets = GetTTPackets("testdata/Jitter/netem.pcapng", "10.157.1.34", 10333, "10.157.1.40", 63915);
    REQUIRE(ttpackets.size());

    // Setup ClientNode's own 'ClientChannel'
    const int FPP = 2;
    const int SAMPLERATE = 48000, CHANNELS = 2;
    const double FRMDURATION = .120;
    const int PLAYBACKFRAMESIZE = FPP * FRMDURATION * SAMPLERATE;

    teamtalk::AudioCodec codec;
    codec.codec = teamtalk::CODEC_OPUS;
    codec.opus.samplerate = SAMPLERATE;
    codec.opus.channels = CHANNELS;
    codec.opus.application = OPUS_APPLICATION_VOIP;
    codec.opus.complexity = 10;
    codec.opus.fec = true;
    codec.opus.dtx = true;
    codec.opus.bitrate = 6000;
    codec.opus.vbr = false;
    codec.opus.vbr_constraint = false;
    codec.opus.frame_size = SAMPLERATE * FRMDURATION;
    codec.opus.frames_per_packet = FPP;

    teamtalk::clientchannel_t mychan;
    mychan.reset(new teamtalk::ClientChannel(25));
    mychan->SetAudioCodec(codec);

    // event listener for ClientUser and VoiceLogger
    TTMsgQueue events;

    // Create ClientNode
    class MyClientNode : public teamtalk::ClientNodeBase
    {
        soundsystem::soundsystem_t m_snd;
        int m_userid;
        teamtalk::clientchannel_t m_mychan;
        teamtalk::SoundProperties m_sndprop;

        teamtalk::VoiceLogger m_vl;
        std::map<int, teamtalk::clientuser_t> m_users;
        const ttpackets_t& m_packets;
        ttpackets_t::const_iterator m_nextpacket;
        teamtalk::ClientListener* m_listener;

    public:
        MyClientNode(int myuserid, teamtalk::clientchannel_t mychan, teamtalk::ClientListener* listener, const ttpackets_t& packets)
            : m_userid(myuserid), m_mychan(mychan), m_vl(listener), m_listener(listener), m_packets(packets)
        {
            ResumeEventHandling();

            // Setup Sound System
            m_snd = soundsystem::GetInstance();
            m_sndprop.soundgroupid = m_snd->OpenSoundGroup();

            SoundDevice indev, outdev;
            REQUIRE(GetSoundDevices(indev, outdev, SOUNDDEVICEID_DEFAULT, SOUNDDEVICEID_DEFAULT));
            m_sndprop.inputdeviceid = indev.nDeviceID;
            m_sndprop.outputdeviceid = outdev.nDeviceID;

            m_nextpacket = m_packets.begin();

            REQUIRE(StartUserTimer(teamtalk::USER_TIMER_MASK, 0, 0, ACE_Time_Value()) >= 0);
        }

        ~MyClientNode()
        {
            while (m_users.size())
            {
                m_users.begin()->second->ResetVoicePlayer();
                m_users.erase(m_users.begin());
            }
            m_snd->RemoveSoundGroup(m_sndprop.soundgroupid);

            SuspendEventHandling(true);
        }

        int TimerEvent(ACE_UINT32 timer_event_id, long userdata)
        {
            REQUIRE(m_nextpacket != m_packets.end());
            auto data = &m_nextpacket->second[0];
            auto len = m_nextpacket->second.size();
            switch (teamtalk::FieldPacket(data, len).GetKind())
            {
            case teamtalk::PACKET_KIND_VOICE :
            {
                teamtalk::AudioPacket p(data, len);
                REQUIRE(p.ValidatePacket());
                REQUIRE(!p.HasFragments());
                REQUIRE(!p.HasFrameSizes());
                REQUIRE(p.GetChannel() == m_mychan->GetChannelID());

                if (m_users.find(p.GetSrcUserID()) == m_users.end())
                {
                    m_users[p.GetSrcUserID()].reset(new teamtalk::ClientUser(p.GetSrcUserID(), this, m_listener, m_snd));
                    if (m_mychan->GetChannelID() == p.GetChannel())
                        m_users[p.GetSrcUserID()]->SetChannel(m_mychan);
                    m_users[p.GetSrcUserID()]->SetLocalSubscriptions(teamtalk::SUBSCRIBE_ALL);
                }

                REQUIRE(p.GetChannel() == m_mychan->GetChannelID());
                // std::cout << "User ID " << p.GetSrcUserID() << " Packet No " << p.GetPacketNumber() << " Stream ID " << int(p.GetStreamID()) << " Channel ID: " << p.GetChannel() << std::endl;
                m_users[p.GetSrcUserID()]->AddVoicePacket(p, m_sndprop, true);
            }
            }

            auto last = m_nextpacket->first;

            m_nextpacket++;

            if (m_nextpacket != m_packets.end())
            {
                // remove timer since we cannot have two with same ID
                ClearTimer(timer_event_id);
                // start timer for next packet arrival
                REQUIRE(StartUserTimer(teamtalk::USER_TIMER_MASK, 0, 0, m_nextpacket->first - last) >= 0);
            }
            else
            {
                m_listener->OnConnectionLost(); // stop unit-test. No more packets
            }
            
            return -1;
        }

        void StreamDuplexCb(const soundsystem::DuplexStreamer& streamer, const short* input_buffer, short* output_buffer, int samples) override {}
        soundsystem::SoundDeviceFeatures GetDuplexFeatures() override { return SOUNDDEVICEFEATURE_NONE; }
        // Sound system is running is duplex mode, soundsystem::OpenDuplexStream()
        bool SoundDuplexMode() override { return false; }
        // Get my user-ID (0 = not set)
        int GetUserID() const override { return m_userid; }
        // Get ID of current channel (0 = not set)
        int GetChannelID() override { return m_mychan->GetChannelID(); }
        // Queue packet for transmission
        bool QueuePacket(teamtalk::FieldPacket* packet) override { return true; }
        // Get logger for writing audio streams to disk (wav, ogg, etc)
        teamtalk::VoiceLogger& voicelogger() override { return m_vl; }
        // Callback function for teamtalk::AudioPlayer-class
        void AudioUserCallback(int userid, teamtalk::StreamType st, const media::AudioFrame& audio_frame) override { }
    } myclientnode(57, mychan, &events, ttpackets);

    TTMessage msg;
    while (events.GetMessage(msg, nullptr) && msg.nClientEvent != CLIENTEVENT_CON_LOST);
}
