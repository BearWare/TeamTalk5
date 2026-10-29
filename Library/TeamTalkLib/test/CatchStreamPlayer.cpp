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

#include "AudioAnalyze.h"

#include "avstream/AudioResampler.h"
#include "avstream/SoundSystem.h"
#include "codec/WaveFile.h"
#include "teamtalk/CodecCommon.h"
#include "teamtalk/PacketLayout.h"
#include "teamtalk/client/AudioThread.h"
#include "teamtalk/client/StreamPlayers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <future>
#include <set>

static void PlayAudioPackets(const std::set<uint16_t>& drop_packets,
                             const teamtalk::AudioCodec& codec,
                             const uint32_t recordDurationMSec,
                             const ACE_TString& recordedFilename,
                             const ACE_TString& playedFilename,
                             int toneFrequency)
{

    WavePCMFile recordedFile, playedFile;

    auto sndsys = soundsystem::GetInstance();
    REQUIRE(sndsys);
    auto sndgrp = sndsys->OpenSoundGroup();
    REQUIRE(sndgrp>0);

    const media::AudioFormat infmt(teamtalk::GetAudioCodecSampleRate(codec), teamtalk::GetAudioCodecChannels(codec)),
        outfmt(teamtalk::GetAudioCodecSampleRate(codec), teamtalk::GetAudioCodecChannels(codec));

    REQUIRE(recordedFile.NewFile(recordedFilename, infmt));
    REQUIRE(playedFile.NewFile(playedFilename, infmt));

    std::vector<short> played;

    constexpr auto USERID = 1;
    constexpr auto STREAMID = 2;
    std::promise<bool> done;
    auto sig_done = done.get_future();
    REQUIRE(codec.codec == teamtalk::CODEC_OPUS);
    teamtalk::OpusPlayer player(USERID, teamtalk::STREAMTYPE_VOICE, sndsys,
                                [&](int userid, teamtalk::StreamType stream_type, const media::AudioFrame& frm) {
                                    bool exceeded = PCM16_SAMPLES_DURATION(played.size() / teamtalk::GetAudioCodecChannels(codec),
                                               teamtalk::GetAudioCodecSampleRate(codec)) >= recordDurationMSec;
                                    played.insert(played.end(), frm.InputSamples().begin(), frm.InputSamples().end());
                                    if (exceeded != PCM16_SAMPLES_DURATION(played.size() / teamtalk::GetAudioCodecChannels(codec),
                                                                           teamtalk::GetAudioCodecSampleRate(codec)) >= recordDurationMSec)
                                    {
                                        done.set_value(true);
                                    }
                                }, codec, MakeAudioResampler(infmt, outfmt));
    player.SetAudioBufferSize(recordDurationMSec);

    AudioThread encoder;
    encoder.EnableTone(toneFrequency);
    std::vector<short> buf(teamtalk::GetAudioCodecCbTotalSamples(codec));
    uint16_t packetno = -1;
    REQUIRE(encoder.StartEncoder([&](const teamtalk::AudioCodec& codec,
                                     const char* enc_data, int enc_len,
                                     const std::vector<int>& enc_frame_sizes,
                                     const media::AudioFrame& org_frame) {
        teamtalk::VoicePacket vop(teamtalk::PACKET_KIND_VOICE, USERID, GETTIMESTAMP(),
                                  STREAMID, ++packetno, enc_data, enc_len, teamtalk::ConvertFrameSizes(enc_frame_sizes));
        recordedFile.AppendSamples(org_frame.InputSamples().data(), org_frame.input_samples);
        if (!drop_packets.contains(packetno))
            player.QueuePacket(vop);
    }, codec, false));

    media::AudioFrame audfrm(infmt, buf.data(), teamtalk::GetAudioCodecCbSamples(codec));
    audfrm.force_enc = true;
    std::size_t samples_submitted = 0;
    while (PCM16_SAMPLES_DURATION(samples_submitted, teamtalk::GetAudioCodecSampleRate(codec)) < recordDurationMSec)
    {
        encoder.QueueAudio(audfrm);
        ACE_Time_Value zero;
        encoder.ProcessQueue(&zero);
        samples_submitted += audfrm.input_samples;
    }

    REQUIRE(sndsys->OpenOutputStream(&player, soundsystem::SOUND_DEVICEID_VIRTUAL,
                                     sndgrp, teamtalk::GetAudioCodecSampleRate(codec),
                                     teamtalk::GetAudioCodecChannels(codec),
                                     teamtalk::GetAudioCodecCbSamples(codec)));

    REQUIRE(sndsys->StartStream(&player));

    REQUIRE(sig_done.get());

    REQUIRE(player.GetNumAudioPacketsLost(true) == drop_packets.size());
    playedFile.AppendSamples(played.data(), PCM16_DURATION_SAMPLES(recordDurationMSec, teamtalk::GetAudioCodecSampleRate(codec)));

    REQUIRE(sndsys->CloseOutputStream(&player));
    sndsys->RemoveSoundGroup(sndgrp);
}

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
TEST_CASE("OpusPlayer packetloss")
{
    constexpr std::size_t SAMPLERATE = 48000, CHANNELS = 1, FRAME_DURATION_MSEC = 40;
    constexpr std::size_t BITRATE = 384000, COMPLEXITY = 10, RECORD_DURATION_MSEC = 1000;
    constexpr auto TONE_FREQUENCY = 800;

    std::vector<std::set<uint16_t>> test_input = {
        std::set<uint16_t>{},
        std::set<uint16_t>{ 10 },
        std::set<uint16_t>{ 10, 20},
        std::set<uint16_t>{ 10, 11, 12 }
    };

    teamtalk::AudioCodec codec;
    codec.codec = teamtalk::CODEC_OPUS;
    codec.opus.application = OPUS_APPLICATION_VOIP;
    codec.opus.bitrate = BITRATE;
    codec.opus.channels = CHANNELS;
    codec.opus.complexity = COMPLEXITY;
    codec.opus.dtx = true;
    codec.opus.fec = true;
    codec.opus.frame_size = PCM16_DURATION_SAMPLES(FRAME_DURATION_MSEC, SAMPLERATE);
    codec.opus.frames_per_packet = 1;
    codec.opus.samplerate = SAMPLERATE;
    codec.opus.vbr = false;
    codec.opus.vbr_constraint = false;

    for (const auto& drop_packets : test_input) {
        tostringstream os_recorded, os_played;
        os_recorded << ACE_TEXT("PacketLossRecorded_loss_") << drop_packets.size() << ACE_TEXT(".wav");
        os_played << ACE_TEXT("PacketLossPlayed_loss_") << drop_packets.size() << ACE_TEXT(".wav");
        PlayAudioPackets(drop_packets, codec, RECORD_DURATION_MSEC, os_recorded.str().c_str(), os_played.str().c_str(), TONE_FREQUENCY);
    }

    WavePCMFile originalFile;
    std::vector<std::vector<float>> packetlossSamples;
    REQUIRE(originalFile.OpenFile(ACE_TEXT("PacketLossRecorded_loss_0.wav"), true));
    for (int i=0;i<test_input.size();++i)
    {
        tostringstream os_played;
        os_played << ACE_TEXT("PacketLossPlayed_loss_") << i << ACE_TEXT(".wav");
        WavePCMFile packetlossFile;
        REQUIRE(packetlossFile.OpenFile(os_played.str().c_str(), true));
        packetlossSamples.push_back(LoadAudioAsFloat(packetlossFile));
        NormalizeAudio(packetlossSamples.back(), CHANNELS);
    }
    // with RMSE less is better, i.e. a lower value indicates the audio files are the same
    REQUIRE(CompareAudioRMSE(packetlossSamples[0], packetlossSamples[0], 0, CHANNELS) ==  Catch::Approx(0.0).margin(0.00001));
    REQUIRE(CompareAudioRMSE(packetlossSamples[0], packetlossSamples[1], 0, CHANNELS) ==  Catch::Approx(0.02).margin(0.005));
    REQUIRE(CompareAudioRMSE(packetlossSamples[0], packetlossSamples[2], 0, CHANNELS) ==  Catch::Approx(0.03).margin(0.005));
    REQUIRE(CompareAudioRMSE(packetlossSamples[0], packetlossSamples[3], 0, CHANNELS) ==  Catch::Approx(0.1).margin(0.05));
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
