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

#include "TTUnitTest.h"

#include "avstream/AudioResampler.h"
#include "avstream/SoundSystem.h"
#include "codec/WaveFile.h"
#include "teamtalk/CodecCommon.h"
#include "teamtalk/PacketLayout.h"
#include "teamtalk/client/AudioThread.h"
#include "teamtalk/client/StreamPlayers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <format>
#include <future>
#include <set>

TEST_CASE("OpusPlayer packetloss")
{
    auto drop_packets = GENERATE(
        std::set<uint16_t>{},
        std::set<uint16_t>{10 /* 400msec -> 440msec */},
        std::set<uint16_t>{10,20, /* 400msec -> 440msec, 800msec -> 840msec, */},
        std::set<uint16_t>{10,11,12 /* 400msec -> 520msec */}
        );
    constexpr auto SAMPLERATE = 48000, CHANNELS = 1;

    WavePCMFile recordedFile, playedFile;

    auto sndsys = soundsystem::GetInstance();
    REQUIRE(sndsys);
    auto sndgrp = sndsys->OpenSoundGroup();
    REQUIRE(sndgrp>0);

    const media::AudioFormat infmt(SAMPLERATE, CHANNELS), outfmt(SAMPLERATE, CHANNELS);

    auto recordedFilename = std::format("PacketLossRecorded_loss_{}.wav", drop_packets.size());
    REQUIRE(recordedFile.NewFile(Utf8ToUnicode(recordedFilename.c_str()), infmt));
    auto playedFilename = std::format("PacketLossPlayed_loss_{}.wav", drop_packets.size());
    REQUIRE(playedFile.NewFile(Utf8ToUnicode(playedFilename.c_str()), infmt));

    auto resampler = MakeAudioResampler(infmt, outfmt);
    teamtalk::AudioCodec codec;
    codec.codec = teamtalk::CODEC_OPUS;
    codec.opus.application = OPUS_APPLICATION_VOIP;
    codec.opus.bitrate = OPUS_MIN_BITRATE;
    codec.opus.channels = CHANNELS;
    codec.opus.complexity = 10;
    codec.opus.dtx = true;
    codec.opus.fec = true;
    codec.opus.frame_size = PCM16_DURATION_SAMPLES(40, SAMPLERATE);
    codec.opus.frames_per_packet = 1;
    codec.opus.samplerate = SAMPLERATE;
    codec.opus.vbr = false;
    codec.opus.vbr_constraint = false;

    std::vector<short> played;
    constexpr auto RECORD_DURATION_MSEC = 1000;

    constexpr auto USERID = 1;
    constexpr auto STREAMID = 2;
    std::promise<bool> done;
    auto sig_done = done.get_future();
    teamtalk::OpusPlayer player(USERID, teamtalk::STREAMTYPE_VOICE, sndsys,
                                [&](int userid, teamtalk::StreamType stream_type, const media::AudioFrame& frm) {
        bool exceeded = PCM16_SAMPLES_DURATION(played.size() / CHANNELS, SAMPLERATE) >= RECORD_DURATION_MSEC;
        played.insert(played.end(), frm.InputSamples().begin(), frm.InputSamples().end());
        if (exceeded != PCM16_SAMPLES_DURATION(played.size() / CHANNELS, SAMPLERATE) >= RECORD_DURATION_MSEC)
            done.set_value(true);

    }, codec, resampler);
    player.SetAudioBufferSize(RECORD_DURATION_MSEC);

    AudioThread encoder;
    encoder.EnableTone(800);
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
    while (PCM16_SAMPLES_DURATION(samples_submitted, SAMPLERATE) < RECORD_DURATION_MSEC)
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
    playedFile.AppendSamples(played.data(), PCM16_DURATION_SAMPLES(RECORD_DURATION_MSEC, SAMPLERATE));

    REQUIRE(sndsys->CloseOutputStream(&player));
    sndsys->RemoveSoundGroup(sndgrp);
}
