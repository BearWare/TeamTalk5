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

#include <catch2/catch_test_macros.hpp>

#include "TTUnitTest.h"
#include "avstream/MediaStreamer.h"
#include "avstream/VideoCapture.h"
#include "bin/ttsrv/ServerUtil.h"
#include "codec/MediaUtil.h"
#include "codec/SpeexEncoder.h"
#include "codec/WaveFile.h"
#include "myace/MyACE.h"
#include "myace/MyINet.h"
#include "mystd/MyStd.h"
#include "settings/Settings.h"
#include "teamtalk/Commands.h"
#include "teamtalk/Common.h"
#include "teamtalk/PacketLayout.h"
#include "teamtalk/StreamHandler.h"
#include "teamtalk/client/AudioMuxer.h"
#include "teamtalk/client/Client.h"

#if defined(ENABLE_OGG)
#include "codec/OggFileIO.h"
#endif

#if defined(ENABLE_OPUS)
#include "avstream/OpusFileStreamer.h"
#include "codec/OpusDecoder.h"
#endif

#if defined(ENABLE_FFMPEG)
#include "avstream/FFmpegStreamer.h"
#endif

#if defined(ENABLE_PORTAUDIO)
#include "avstream/PortAudioWrapper.h"
#endif

#include <ace/ACE.h>
#include <ace/Addr.h>
#include <ace/Connector.h>
#include <ace/Date_Time.h>
#include <ace/FILE_Addr.h>
#include <ace/FILE_Connector.h>
#include <ace/FILE_IO.h>
#include <ace/Reactor.h>
#include <ace/SSL/SSL_Context.h>
#include <ace/Select_Reactor.h>
#include <ace/Synch_Options.h>
#include <ace/Timer_Heap.h>

#include <array>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#if defined(WIN32)

#include <MMDeviceApi.h>
#include <Mmsystem.h>
#include <ace/Init_ACE.h>
#include <atlbase.h>
#include <cassert>
#include <mfapi.h>
#include <propsys.h>

static class WinInit
{
public:
    WinInit()
    {
        int ret = ACE::init();
        assert(ret >= 0);
#if defined(ENABLE_MEDIAFOUNDATION)
        HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
        assert(SUCCEEDED(hr));
#endif
    }
    ~WinInit()
    {
        int ret = ACE::fini();
        assert(ret >= 0);
    }
} wininit;
#endif

/* Known bugs */
#define TEAMTALK_KNOWN_BUGS 0

constexpr auto CURRENT_DIRECTORY_BUFFER_SIZE = 1024;
constexpr auto OPUS_SAMPLE_RATE_HZ = 48000;
constexpr auto OPUS_CHANNELS = 2;
constexpr auto OPUS_COMPLEXITY = 10;
constexpr auto OPUS_TX_INTERVAL_MSEC = 240;
constexpr auto OPUS_FRAME_SIZE_120MS_MSEC = 120;
constexpr auto OPUS_FRAME_SIZE_60MS_MSEC = 60;
constexpr auto OPUS_FRAME_SIZE_40MS_MSEC = 40;

static std::vector<char> FlattenPacket(const teamtalk::FieldPacket& packet,
                                       std::vector<size_t>& offsets)
{
    int buffers = 0;
    const iovec* packet_buffers = packet.GetPacket(buffers);
    std::vector<char> result;
    offsets.clear();
    for(int i = 0; i < buffers; ++i)
    {
        offsets.push_back(result.size());
        const auto* begin = static_cast<const char*>(packet_buffers[i].iov_base);
        result.insert(result.end(), begin, begin + packet_buffers[i].iov_len);
    }
    return result;
}

TEST_CASE("Reject malformed 12-bit packet arrays", "[packet]")
{
    using namespace teamtalk;

    SECTION("one-byte remainder in encoded audio frame sizes")
    {
        const std::array<char, 1> audio = {0};
        AudioPacket packet(PACKET_KIND_VOICE, 1, 2, 3, 4, audio.data(),
                           audio.size(), std::vector<uint16_t>{1});
        std::vector<size_t> offsets;
        auto bytes = FlattenPacket(packet, offsets);
        auto* fields = reinterpret_cast<uint8_t*>(bytes.data()) +
                       TT_CHANNEL_HEADER_SIZE;
        auto* frame_sizes = const_cast<uint8_t*>(FINDFIELD_TYPE(
            fields, AudioPacket::FIELDTYPE_ENCFRAMESIZES,
            bytes.size() - TT_CHANNEL_HEADER_SIZE));
        REQUIRE(frame_sizes != nullptr);

        const auto field_offset = static_cast<size_t>(
            frame_sizes - reinterpret_cast<uint8_t*>(bytes.data()));
        WRITEFIELD_TYPE(frame_sizes, AudioPacket::FIELDTYPE_ENCFRAMESIZES, 1);
        bytes.resize(field_offset + FIELDVALUE_PREFIX + 1);

        AudioPacket malformed(bytes.data(), uint16_t(bytes.size()));
        CHECK(malformed.GetEncodedFrameSizes().empty());
    }

    SECTION("declared block size exceeds block data")
    {
        const std::array<char, 2> block_data = {1, 2};
        map_block_t input_blocks = {
            {7, DesktopBlock{block_data.data(), uint16_t(block_data.size())}}
        };
        DesktopPacket packet(1, 2, 3, 51, 20, 0, 0, 1, input_blocks,
                             block_frags_t{}, mmap_dup_blocks_t{});
        std::vector<size_t> offsets;
        auto bytes = FlattenPacket(packet, offsets);
        REQUIRE(offsets.size() >= 3);

        auto* block_info = reinterpret_cast<uint8_t*>(bytes.data()) + offsets[2];
        REQUIRE(READFIELD_SIZE(block_info) == 3);
        SET2_UINT12_PTR(READFIELD_DATAPTR(block_info), 7, 3);

        DesktopPacket malformed(bytes.data(), uint16_t(bytes.size()));
        map_block_t output_blocks;
        CHECK_FALSE(malformed.GetBlocks(output_blocks));
        CHECK(output_blocks.empty());
    }
}
      
TEST_CASE("Reject incomplete desktop packet ranges", "[packet]")
{
    using namespace teamtalk;

    SECTION("duplicate block range missing its high value")
    {
        mmap_dup_blocks_t input_ranges;
        input_ranges.emplace(4, std::set<uint16_t>{5, 6});
        DesktopPacket packet(1, 2, 3, 51, 20, 0, 0, 1, map_block_t{},
                             block_frags_t{}, input_ranges);
        std::vector<size_t> offsets;
        auto bytes = FlattenPacket(packet, offsets);
        REQUIRE(offsets.size() >= 3);

        auto* range_field = reinterpret_cast<uint8_t*>(bytes.data()) +
                            offsets.back();
        const auto range_type = READFIELD_TYPE(range_field);
        REQUIRE(READFIELD_SIZE(range_field) == 5);
        WRITEFIELD_TYPE(range_field, range_type, 3);
        bytes.resize(offsets.back() + FIELDVALUE_PREFIX + 3);

        DesktopPacket malformed(bytes.data(), uint16_t(bytes.size()));
        map_dup_blocks_t output_ranges;
        CHECK_FALSE(malformed.GetDuplicateBlocks(output_ranges));
        CHECK(output_ranges.empty());
    }

    SECTION("acknowledgement range missing its high value")
    {
        DesktopAckPacket packet(1, 2, 3, 4, 5, std::set<uint16_t>{},
                                packet_range_t{{6, 7}});
        std::vector<size_t> offsets;
        auto bytes = FlattenPacket(packet, offsets);
        REQUIRE(offsets.size() >= 3);

        auto* range_field = reinterpret_cast<uint8_t*>(bytes.data()) +
                            offsets.back();
        const auto range_type = READFIELD_TYPE(range_field);
        REQUIRE(READFIELD_SIZE(range_field) == 4);
        WRITEFIELD_TYPE(range_field, range_type, 2);
        bytes.resize(offsets.back() + FIELDVALUE_PREFIX + 2);

        DesktopAckPacket malformed(bytes.data(), uint16_t(bytes.size()));
        std::set<uint16_t> packets;
        CHECK_FALSE(malformed.GetPacketsAcked(packets));
        CHECK(packets.empty());
    }
}

TEST_CASE( "Init TT", "" )
{
    TTInstance* ttinst = nullptr;
    REQUIRE( (ttinst = TT_InitTeamTalkPoll()) );
    REQUIRE( TT_CloseTeamTalk(ttinst) );
}

#if defined(ENABLE_OGG) && defined(ENABLE_SPEEX)
TEST_CASE( "Ogg Write", "" )
{
    if (GITHUBSKIP)
    {
        std::cout << "Skipping \"Ogg Write\"... Weird file permission on Windows GitHub" << std::endl;
        return;
    }
    constexpr auto SPEEX_BANDMODE = 1;
    constexpr auto SPEEX_VBR_QUALITY = 7;
    constexpr auto SPEEX_BITRATE = 32000;
    constexpr auto SPEEX_MAX_BITRATE = 48000;

    SpeexEncFile spxfile;
    REQUIRE( spxfile.Open(ACE_TEXT("/foo.spx"), SPEEX_BANDMODE, DEFAULT_SPEEX_COMPLEXITY, SPEEX_VBR_QUALITY, SPEEX_BITRATE, SPEEX_MAX_BITRATE, false) == false);
}
#endif

#if defined(ENABLE_OPUS)
TEST_CASE( "Record mux")
{
    constexpr auto CLIENT_COUNT = 2;

    std::vector<TTInstPtr> clients(CLIENT_COUNT);
    for (size_t i=0;i<clients.size();++i)
    {
        REQUIRE((clients[i] = InitTeamTalk()));
        REQUIRE(InitSound(clients[i], SHARED_INPUT));
        REQUIRE(Connect(clients[i]));
        REQUIRE(Login(clients[i], ACE_TEXT("MyNickname")));

        if (i == 0)
        {
            AudioCodec audiocodec = {};
            audiocodec.nCodec = OPUS_CODEC;
            audiocodec.opus.nApplication = OPUS_APPLICATION_VOIP;
            audiocodec.opus.nTxIntervalMSec = OPUS_TX_INTERVAL_MSEC;
#if defined(OPUS_FRAMESIZE_120_MS)
            audiocodec.opus.nFrameSizeMSec = OPUS_FRAME_SIZE_120MS_MSEC;
#else
            audiocodec.opus.nFrameSizeMSec = OPUS_FRAME_SIZE_40MS_MSEC;
#endif
            audiocodec.opus.nBitRate = OPUS_MIN_BITRATE;
            audiocodec.opus.nChannels = OPUS_CHANNELS;
            audiocodec.opus.nComplexity = OPUS_COMPLEXITY;
            audiocodec.opus.nSampleRate= OPUS_SAMPLE_RATE_HZ;
            audiocodec.opus.bDTX = 1;
            audiocodec.opus.bFEC = 1;
            audiocodec.opus.bVBR = 0;
            audiocodec.opus.bVBRConstraint = 0;

            Channel const chan = MakeChannel(clients[i], ACE_TEXT("foo"), TT_GetRootChannelID(clients[i]), audiocodec);
            REQUIRE(WaitForCmdSuccess(clients[i], TT_DoJoinChannel(clients[i], &chan)));
        }
        else
        {
            REQUIRE(WaitForCmdSuccess(clients[i], TT_DoJoinChannelByID(clients[i], TT_GetMyChannelID(clients[0]), ACE_TEXT(""))));
        }
    }

    Channel chan;
    REQUIRE(TT_GetChannel(clients[1], TT_GetMyChannelID(clients[1]), &chan));

    REQUIRE(TT_EnableVoiceTransmission(clients[0], true));
    constexpr auto TX_TONE_HZ = 500;
    constexpr auto RX_TONE_HZ = 600;
    constexpr auto STARTUP_WAIT_MSEC = 100;
    constexpr auto TRANSMIT_DURATION_MSEC = 2500;
    constexpr auto RECORDING_DURATION_MSEC = 10000;
    WaitForEvent(clients[0], CLIENTEVENT_NONE, STARTUP_WAIT_MSEC);
    REQUIRE(TT_EnableVoiceTransmission(clients[0], false));

    REQUIRE(TT_StartRecordingMuxedAudioFile(clients[1], &chan.audiocodec, ACE_TEXT("MyMuxFile.wav"), AFF_WAVE_FORMAT));

    REQUIRE(TT_DBG_SetSoundInputTone(clients[0], STREAMTYPE_VOICE, TX_TONE_HZ));
    REQUIRE(TT_EnableVoiceTransmission(clients[0], true));
    WaitForEvent(clients[0], CLIENTEVENT_NONE, TRANSMIT_DURATION_MSEC);
    REQUIRE(TT_EnableVoiceTransmission(clients[0], false));

    REQUIRE(TT_DBG_SetSoundInputTone(clients[1], STREAMTYPE_VOICE, RX_TONE_HZ));
    REQUIRE(TT_EnableVoiceTransmission(clients[1], true));
    WaitForEvent(clients[1], CLIENTEVENT_NONE, TRANSMIT_DURATION_MSEC);
    REQUIRE(TT_EnableVoiceTransmission(clients[1], false));

    WaitForEvent(clients[1], CLIENTEVENT_NONE, RECORDING_DURATION_MSEC);

    REQUIRE(TT_StopRecordingMuxedAudioFile(clients[1]));
}
#endif

#if defined(ENABLE_OPUS)
TEST_CASE( "Last voice packet" )
{

    auto txclient = InitTeamTalk();
    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(txclient, SHARED_INPUT));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));

    REQUIRE(InitSound(rxclient, SHARED_INPUT));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));

    AudioCodec audiocodec = {};
    audiocodec.nCodec = OPUS_CODEC;
    audiocodec.opus.nApplication = OPUS_APPLICATION_VOIP;
    audiocodec.opus.nTxIntervalMSec = OPUS_TX_INTERVAL_MSEC;
#if defined(OPUS_FRAMESIZE_120_MS)
    audiocodec.opus.nFrameSizeMSec = OPUS_FRAME_SIZE_120MS_MSEC;
#else
    audiocodec.opus.nFrameSizeMSec = OPUS_FRAME_SIZE_60MS_MSEC;
#endif
    audiocodec.opus.nBitRate = OPUS_MIN_BITRATE;
    audiocodec.opus.nChannels = OPUS_CHANNELS;
    audiocodec.opus.nComplexity = OPUS_COMPLEXITY;
    audiocodec.opus.nSampleRate= OPUS_SAMPLE_RATE_HZ;
    audiocodec.opus.bDTX = 1;
    audiocodec.opus.bFEC = 1;
    audiocodec.opus.bVBR = 0;
    audiocodec.opus.bVBRConstraint = 0;

    Channel const chan = MakeChannel(txclient, ACE_TEXT("foo"), TT_GetRootChannelID(txclient), audiocodec);
    REQUIRE(WaitForCmdSuccess(txclient, TT_DoJoinChannel(txclient, &chan)));

    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoJoinChannelByID(rxclient, TT_GetMyChannelID(txclient), ACE_TEXT(""))));

    constexpr auto RX_TONE_HZ = 600;
    constexpr auto TX_INTERVAL_COUNT = 5;
    constexpr auto TX_INTERVAL_EXTRA_FRACTION = .5;
    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, RX_TONE_HZ));

    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, int((audiocodec.opus.nTxIntervalMSec * TX_INTERVAL_COUNT) + (audiocodec.opus.nTxIntervalMSec * TX_INTERVAL_EXTRA_FRACTION)));
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));

    auto voicestop = [&](TTMessage msg)
    {
        return msg.nClientEvent == CLIENTEVENT_USER_STATECHANGE &&
            msg.user.nUserID == TT_GetMyUserID(txclient) &&
            (msg.user.uUserState & USERSTATE_VOICE) == 0;
    };

    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, voicestop));
    //WaitForEvent(txclient, CLIENTEVENT_NONE, nullptr, audiocodec.opus.nTxIntervalMSec * 2);

    std::array<TTCHAR, CURRENT_DIRECTORY_BUFFER_SIZE> curdir{};
    ACE_OS::getcwd(curdir.data(), curdir.size());
    REQUIRE(TT_SetUserMediaStorageDir(rxclient, TT_GetMyUserID(txclient), curdir.data(), ACE_TEXT(""), AFF_WAVE_FORMAT));

    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, 0));
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    constexpr auto SILENT_TRANSMIT_DURATION_MSEC = 1000;
    WaitForEvent(txclient, CLIENTEVENT_NONE, SILENT_TRANSMIT_DURATION_MSEC);
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));
}
#endif

TEST_CASE( "AudioMuxerToFile" )
{

    auto txclient = InitTeamTalk();
    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(txclient));

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    Channel chan;
    REQUIRE(TT_GetChannel(rxclient, TT_GetMyChannelID(rxclient), &chan));
    REQUIRE(TT_StartRecordingMuxedAudioFile(rxclient, &chan.audiocodec, ACE_TEXT("MyMuxFile.wav"), AFF_WAVE_FORMAT));

    constexpr auto TX_TONE_HZ = 500;
    constexpr auto RX_TONE_HZ = 600;
    constexpr auto TRANSMIT_DURATION_MSEC = 2000;
    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, TX_TONE_HZ));
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, TRANSMIT_DURATION_MSEC);
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));

    // This tone is not being stored in 'MyMuxFile.wav' because the
    // audio block will bypass the audio encoder.
    REQUIRE(TT_DBG_SetSoundInputTone(rxclient, STREAMTYPE_VOICE, RX_TONE_HZ));
    REQUIRE(TT_EnableVoiceTransmission(rxclient, true));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, TRANSMIT_DURATION_MSEC);
    REQUIRE(TT_EnableVoiceTransmission(rxclient, false));

    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, TRANSMIT_DURATION_MSEC);
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));

    REQUIRE(TT_EnableVoiceTransmission(rxclient, true));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, TRANSMIT_DURATION_MSEC);
    REQUIRE(TT_EnableVoiceTransmission(rxclient, false));

    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, TRANSMIT_DURATION_MSEC);
    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoUnsubscribe(rxclient, TT_GetMyUserID(txclient), SUBSCRIBE_VOICE)));

    WaitForEvent(txclient, CLIENTEVENT_NONE, TRANSMIT_DURATION_MSEC);

    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoSubscribe(rxclient, TT_GetMyUserID(txclient), SUBSCRIBE_VOICE)));

    REQUIRE(TT_EnableVoiceTransmission(rxclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, TRANSMIT_DURATION_MSEC);

    REQUIRE(TT_CloseSoundInputDevice(rxclient));
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, TRANSMIT_DURATION_MSEC);
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));

    REQUIRE(TT_StopRecordingMuxedAudioFile(rxclient));
}

static int GetAudioBlockSamplesSum(TTInstance* ttinst, int userid, StreamTypes sts)
{
    TTMessage msg;
    REQUIRE(WaitForEvent(ttinst, CLIENTEVENT_USER_AUDIOBLOCK, msg));
    auto *ab = TT_AcquireUserAudioBlock(ttinst, sts, userid);
    REQUIRE(ab);
    auto* audiobuf = reinterpret_cast<short*>(ab->lpRawAudio);
    uint32_t sum_samples = 0;
    for (int i=0;i<ab->nSamples * ab->nChannels;i++)
        sum_samples += std::abs(audiobuf[i]);
    sum_samples = sum_samples / ab->nChannels;
    REQUIRE(TT_ReleaseUserAudioBlock(ttinst, ab));

    // std::cout << "Sum samples: " << sum_samples << std::endl;

    return sum_samples;
};

TEST_CASE( "AudioMuxerSimple" )
{
    auto txclient = InitTeamTalk();
    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(txclient));

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));

    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK));

    REQUIRE(WaitForCmdComplete(rxclient, TT_DoLeaveChannel(rxclient)));

    REQUIRE(JoinRoot(rxclient));

    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK));
}

TEST_CASE( "AudioMuxerNoInputDevice" )
{
    auto txclient = InitTeamTalk();
    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(txclient));

    REQUIRE(InitSound(rxclient, DEFAULT, SOUNDDEVICEID_IGNORE));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));

    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK));
    auto *ab = TT_AcquireUserAudioBlock(rxclient, STREAMTYPE_VOICE, TT_MUXED_USERID);
    REQUIRE(ab != nullptr);
    REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));

    int const sum = GetAudioBlockSamplesSum(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE);
    REQUIRE(sum == 0);

    constexpr auto TX_TONE_HZ = 500;
    constexpr auto AUDIOBLOCK_RETRIES = 100;

    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, TX_TONE_HZ));
    REQUIRE(TT_EnableVoiceTransmission(txclient, TRUE));

    int retries = AUDIOBLOCK_RETRIES;
    while (((retries--) != 0) && GetAudioBlockSamplesSum(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE) == 0);
    REQUIRE(retries > 0);
}

TEST_CASE("AudioMuxerSoundInputDisabled")
{
    auto ttclient = InitTeamTalk();

    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(InitSound(ttclient));
    REQUIRE(JoinRoot(ttclient));

    REQUIRE(TT_CloseSoundInputDevice(ttclient));
    REQUIRE(TT_EnableAudioBlockEvent(ttclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));

    constexpr auto AUDIOBLOCK_COUNT = 10;
    int n_blocks = AUDIOBLOCK_COUNT;
    do
    {
        TTMessage msg;
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
        REQUIRE(msg.nSource == TT_MUXED_USERID);
        AudioBlock* ab = TT_AcquireUserAudioBlock(ttclient, STREAMTYPE_VOICE, TT_MUXED_USERID);
        REQUIRE(ab);
        REQUIRE(ab->nSamples>0);
        REQUIRE(TT_ReleaseUserAudioBlock(ttclient, ab));
    } while ((n_blocks--) != 0);
}

TEST_CASE( "AudioMuxerUserEvent" )
{
    auto txclient = InitTeamTalk();
    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(txclient));

    REQUIRE(InitSound(rxclient, DEFAULT, SOUNDDEVICEID_IGNORE, SOUNDDEVICEID_DEFAULT));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK));
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    auto voicestart = [&](TTMessage msg)
    {
        return msg.nClientEvent == CLIENTEVENT_USER_STATECHANGE &&
            msg.user.nUserID == TT_GetMyUserID(txclient) &&
            (msg.user.uUserState & USERSTATE_VOICE) == USERSTATE_VOICE;
    };
    auto voicestop = [&](TTMessage msg)
    {
        return msg.nClientEvent == CLIENTEVENT_USER_STATECHANGE &&
            msg.user.nUserID == TT_GetMyUserID(txclient) &&
            (msg.user.uUserState & USERSTATE_VOICE) == USERSTATE_NONE;
    };

    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, voicestart));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK));
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, voicestop));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK));
}

TEST_CASE( "AudioMuxerVolumeControl" )
{
    auto txclient = InitTeamTalk();
    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(txclient));

    constexpr auto TX_TONE_HZ = 500;

    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, TX_TONE_HZ));
    int const txuserid = TT_GetMyUserID(txclient);

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    REQUIRE(TT_EnableVoiceTransmission(txclient, true));

    TTMessage msg;

    // calc default volume level of muxed audio block
    REQUIRE((WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, msg) && (msg.user.uUserState & USERSTATE_VOICE) == USERSTATE_VOICE));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));

    uint32_t sum_nogain = 0;
    sum_nogain = GetAudioBlockSamplesSum(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE);

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);
    REQUIRE(TT_AcquireUserAudioBlock(rxclient,STREAMTYPE_VOICE, TT_MUXED_USERID) == nullptr);

    // double volume level of user
    User user;
    REQUIRE(TT_GetUser(rxclient, txuserid, &user));
    constexpr auto VOLUME_RETRIES = 5;
    constexpr auto VOLUME_MULTIPLIER = 2;
    REQUIRE(TT_SetUserVolume(rxclient, txuserid, STREAMTYPE_VOICE, user.nVolumeVoice * VOLUME_MULTIPLIER));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));

    int retries = VOLUME_RETRIES;
    uint32_t sum_gain = 0;
    constexpr auto GAIN_TOLERANCE_FACTOR = 1.9;
    do
    {
        sum_gain = GetAudioBlockSamplesSum(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE);
    }
    while (sum_gain <= sum_nogain * GAIN_TOLERANCE_FACTOR && (retries != 0));

    // volume level of muxed audio should now have doubled (roughly due to tone offset)
    REQUIRE(sum_gain > sum_nogain * GAIN_TOLERANCE_FACTOR);

    // reset
    REQUIRE(TT_SetUserVolume(rxclient, txuserid, STREAMTYPE_VOICE, SOUND_VOLUME_DEFAULT));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // volume level of muxed audio when changing master volume
    REQUIRE(TT_SetSoundOutputVolume(rxclient, SOUND_VOLUME_DEFAULT * VOLUME_MULTIPLIER));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));

    retries = VOLUME_RETRIES;
    do
    {
        sum_gain = GetAudioBlockSamplesSum(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE);
    }
    while (sum_gain <= sum_nogain * GAIN_TOLERANCE_FACTOR && ((retries--) != 0));

    // volume level of muxed audio should now have doubled (roughly due to tone offset)
    REQUIRE(sum_gain > sum_nogain * GAIN_TOLERANCE_FACTOR);

    // reset
    REQUIRE(TT_SetSoundOutputVolume(rxclient, SOUND_VOLUME_DEFAULT));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // calc default volume leve of single stream audioblock
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, TRUE));

    constexpr auto SUM_SAMPLE_COUNT = 2;
    for (int i=0;i<SUM_SAMPLE_COUNT;++i)
        sum_nogain = GetAudioBlockSamplesSum(rxclient, txuserid, STREAMTYPE_VOICE);

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // double volume level
    REQUIRE(TT_SetUserVolume(rxclient, txuserid, STREAMTYPE_VOICE, user.nVolumeVoice * VOLUME_MULTIPLIER));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, TRUE));

    retries = VOLUME_RETRIES;
    do
    {
        sum_gain = GetAudioBlockSamplesSum(rxclient, txuserid, STREAMTYPE_VOICE);
    }
    while (sum_gain <= sum_nogain * GAIN_TOLERANCE_FACTOR && ((retries--) != 0));

    // volume level of muxed audio should now have doubled (roughly due to tone offset)
    REQUIRE(sum_gain > sum_nogain * GAIN_TOLERANCE_FACTOR);

    // reset
    REQUIRE(TT_SetUserVolume(rxclient, txuserid, STREAMTYPE_VOICE, SOUND_VOLUME_DEFAULT));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // test master mute
    REQUIRE(TT_SetSoundOutputMute(rxclient, true));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, TRUE));

    retries = VOLUME_RETRIES;
    do
    {
        sum_gain = GetAudioBlockSamplesSum(rxclient, txuserid, STREAMTYPE_VOICE);
    }
    while (sum_gain != 0 && ((retries--) != 0));

    // mute master gives 0 sum
    REQUIRE(sum_gain == 0);

    REQUIRE(TT_SetSoundOutputMute(rxclient, true));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // test user mute
    REQUIRE(TT_SetUserMute(rxclient, txuserid, STREAMTYPE_VOICE, TRUE));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, TRUE));

    retries = VOLUME_RETRIES;
    do
    {
        sum_gain = GetAudioBlockSamplesSum(rxclient, txuserid, STREAMTYPE_VOICE);
    }
    while (sum_gain != 0 && ((retries--) != 0));

    // mute user gives 0 sum
    REQUIRE(sum_gain == 0);
}

TEST_CASE( "AudioMuxerRawSameStreamTypeSameAudioFormat" )
{
    constexpr auto SAMPLE_RATE_HZ = 12000;
    constexpr auto CHANNELS = 1;
    constexpr auto FRAME_SIZE_SEC = .01;
    constexpr auto QUEUE_WATER_MARK_BYTES = 1024 * 1024;

    media::AudioInputFormat const inputfmt(media::AudioFormat(SAMPLE_RATE_HZ, CHANNELS), int(SAMPLE_RATE_HZ * FRAME_SIZE_SEC));
    const int TOTALSAMPLES = inputfmt.GetTotalSamples();
    const int FRAMESIZE = inputfmt.samples;
    const auto FMT = inputfmt.fmt;
    const int FRAMEBYTES = inputfmt.GetBytes();

    msg_queue_t mixed_frames;
    mixed_frames.high_water_mark(QUEUE_WATER_MARK_BYTES);
    mixed_frames.low_water_mark(QUEUE_WATER_MARK_BYTES);

    AudioMuxer muxer(teamtalk::STREAMTYPE_VOICE);
    constexpr auto MUX_INTERVAL_FRAMES = 5;
    int const muxinterval_msec = inputfmt.GetDurationMSec() * MUX_INTERVAL_FRAMES;
    muxer.SetMuxInterval(muxinterval_msec);
    auto mixedfunc = [&] (teamtalk::StreamTypes  /*sts*/, const media::AudioFrame& frm)
    {
        auto *mb = AudioFrameToMsgBlock(frm);
        REQUIRE(mixed_frames.enqueue(mb) >= 0);
    };

    REQUIRE(muxer.RegisterMuxCallback(inputfmt, mixedfunc));

    std::vector<int> userids;
    userids.push_back(10);
    userids.push_back(20);
    userids.push_back(30);
    std::vector< std::vector<short> > user_bufs;
    std::vector<media::AudioFrame> user_frames;
    user_bufs.reserve(userids.size());
for (auto u : userids)
        user_bufs.emplace_back(inputfmt.GetTotalSamples(), u);
    for (size_t i=0;i<userids.size();++i)
    {
        constexpr auto SAMPLE_NO_STRIDE = 1000;
        media::AudioFrame frm(FMT, user_bufs[i].data(), inputfmt.samples, userids[i] * SAMPLE_NO_STRIDE);
        frm.streamid = userids[i];
        user_frames.push_back(frm);
    }

    REQUIRE(muxer.QueueUserAudio(user_frames[0].streamid, teamtalk::STREAMTYPE_VOICE, user_frames[0]));

    // test mixing of single stream
    ACE_Message_Block* mb = nullptr;
    REQUIRE(mixed_frames.dequeue(mb) >= 0);
    {
        MBGuard const g(mb);
        media::AudioFrame const frm_mixed(mb);
        REQUIRE(std::memcmp(frm_mixed.input_buffer, user_frames[0].input_buffer, FRAMEBYTES) == 0);
    }

    constexpr auto DEQUEUE_TIMEOUT_MSEC = 50;
    ACE_Time_Value tm = ACE_OS::gettimeofday() + ToTimeValue(DEQUEUE_TIMEOUT_MSEC);
    REQUIRE(mixed_frames.dequeue(mb, &tm) < 0);

    user_frames[0].sample_no += FRAMESIZE;
    REQUIRE(muxer.QueueUserAudio(user_frames[0].streamid, teamtalk::STREAMTYPE_VOICE, user_frames[0]));

    REQUIRE(mixed_frames.dequeue(mb) >= 0);
    {
        MBGuard const g(mb);
        media::AudioFrame const frm_mixed(mb);
        REQUIRE(std::memcmp(frm_mixed.input_buffer, user_frames[0].input_buffer, FRAMEBYTES) == 0);
    }

    // test mixing of two stream
    REQUIRE(muxer.QueueUserAudio(user_frames[1].streamid, teamtalk::STREAMTYPE_VOICE, user_frames[1]));
    user_frames[0].sample_no += FRAMESIZE;
    REQUIRE(muxer.QueueUserAudio(user_frames[0].streamid, teamtalk::STREAMTYPE_VOICE, user_frames[0]));
    REQUIRE(mixed_frames.dequeue(mb) >= 0);
    {
        MBGuard const g(mb);
        media::AudioFrame const frm_mixed(mb);
        for (int i=0;i<TOTALSAMPLES;++i)
            REQUIRE(frm_mixed.input_buffer[i] == userids[0] + userids[1]);
    }

    user_frames[1].sample_no += FRAMESIZE;
    REQUIRE(muxer.QueueUserAudio(user_frames[1].streamid, teamtalk::STREAMTYPE_VOICE, user_frames[1]));

    tm = ACE_OS::gettimeofday() + ToTimeValue(DEQUEUE_TIMEOUT_MSEC);
    REQUIRE(mixed_frames.dequeue(mb, &tm) < 0);

    // terminate stream for #0
    REQUIRE(muxer.QueueUserAudio(user_frames[0].streamid, teamtalk::STREAMTYPE_VOICE, media::AudioFrame()));

    REQUIRE(mixed_frames.dequeue(mb) >= 0);
    {
        MBGuard const g(mb);
        media::AudioFrame const frm_mixed(mb);
        REQUIRE(std::memcmp(frm_mixed.input_buffer, user_frames[1].input_buffer, FRAMEBYTES) == 0);
    }

    // terminate stream for #1
    REQUIRE(muxer.QueueUserAudio(user_frames[1].streamid, teamtalk::STREAMTYPE_VOICE, media::AudioFrame()));

    // expect silent mix
    REQUIRE(mixed_frames.dequeue(mb) >= 0);
    {
        MBGuard const g(mb);
        media::AudioFrame const frm_mixed(mb);
        for (int i=0;i<TOTALSAMPLES;++i)
            REQUIRE(frm_mixed.input_buffer[i] == 0);
    }

    // start new stream for #0
    user_frames[0].sample_no += FRAMESIZE;
    REQUIRE(muxer.QueueUserAudio(user_frames[0].streamid, teamtalk::STREAMTYPE_VOICE, user_frames[0]));

    // wait for #0 audio to appear so it blocks further muxing
    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard const g(mb);
        media::AudioFrame const frm_mixed(mb);
        if (frm_mixed.input_buffer[0] == userids[0])
            break;
    }

    constexpr auto FLOOD_INTERVAL_COUNT = 10;
    // overflow the buffer
    int const n_flood = FLOOD_INTERVAL_COUNT * muxinterval_msec / inputfmt.GetDurationMSec();
    for (int sr=0;sr<n_flood;++sr)
    {
        for (size_t i=0;i<userids.size();++i)
        {
            user_frames[i].sample_no += FRAMESIZE;
            REQUIRE(muxer.QueueUserAudio(user_frames[i].streamid, teamtalk::STREAMTYPE_VOICE, user_frames[i]));
        }
    }

    // ensure we end up with all streams being mixed
    int muxsum = 0;
    for (auto u : userids)
        muxsum += u;

    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard const g(mb);
        media::AudioFrame const frm_mixed(mb);
        if (frm_mixed.input_buffer[0] == muxsum)
            break;
    }

    constexpr auto DRAIN_INTERVAL_COUNT = 3;
    // wait for audio muxer to report delayer, i.e. drain muxer
    tm = ACE_OS::gettimeofday() + ToTimeValue(muxinterval_msec * DRAIN_INTERVAL_COUNT);
    bool gotmux = false;
    while (mixed_frames.dequeue(mb, &tm) >= 0)
    {
        MBGuard const g(mb);
        media::AudioFrame const frm_mixed(mb);
        REQUIRE(frm_mixed.input_buffer[0] == muxsum);
        gotmux = true;
    }
    REQUIRE(gotmux);

    // ensure stream can be restarted
    for (int m=0;m<muxinterval_msec / inputfmt.GetDurationMSec();++m)
    {
        for (size_t i=0;i<userids.size();++i)
        {
            user_frames[i].sample_no += FRAMESIZE;
            REQUIRE(muxer.QueueUserAudio(user_frames[i].streamid, teamtalk::STREAMTYPE_VOICE, user_frames[i]));
        }
    }

    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard const g(mb);
        media::AudioFrame const frm_mixed(mb);
        if (frm_mixed.input_buffer[0] == muxsum)
            break;
    }

    // terminate streams so we end up with silence
    for (size_t i=0;i<userids.size();++i)
    {
        REQUIRE(muxer.QueueUserAudio(user_frames[i].streamid, teamtalk::STREAMTYPE_VOICE, media::AudioFrame()));
    }

    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard const g(mb);
        media::AudioFrame const frm_mixed(mb);
        if (frm_mixed.input_buffer[0] == 0)
            break;
    }
}

TEST_CASE("MergeAudioBlocks")
{
    constexpr auto SAMPLE_RATE_HZ = 48000;
    constexpr auto CHANNELS = 2;
    constexpr auto TONE_AMPLITUDE = 1000;
    constexpr auto TOTAL_SAMPLES = 77000;
    constexpr auto FRAME1_SAMPLES = 11000;
    constexpr auto FRAME2_SAMPLES = 22000;
    constexpr auto FRAME3_SAMPLES = 44000;
    constexpr auto START_SAMPLE_NO = 7000;

    media::AudioFormat const fmt(SAMPLE_RATE_HZ, CHANNELS);
    std::vector<short> rawaudio;
    short v = TONE_AMPLITUDE;
    rawaudio.reserve(TOTAL_SAMPLES*fmt.channels);
for (size_t i=0;std::cmp_less(i,TOTAL_SAMPLES*fmt.channels);++i)
        rawaudio.push_back(v++);

    std::vector<ACE_Message_Block*> mbs;
    media::AudioFrame frm(fmt, rawaudio.data(), FRAME1_SAMPLES, START_SAMPLE_NO);
    mbs.push_back(AudioFrameToMsgBlock(frm));
    frm = media::AudioFrame(fmt, &rawaudio[FRAME1_SAMPLES * fmt.channels], FRAME2_SAMPLES, START_SAMPLE_NO + FRAME1_SAMPLES);
    mbs.push_back(AudioFrameToMsgBlock(frm));
    frm = media::AudioFrame(fmt, &rawaudio[(FRAME1_SAMPLES + FRAME2_SAMPLES) * fmt.channels], FRAME3_SAMPLES, START_SAMPLE_NO + FRAME1_SAMPLES + FRAME2_SAMPLES);
    mbs.push_back(AudioFrameToMsgBlock(frm));
    auto *mb = AudioFramesMerge(mbs);
    REQUIRE(mb->size() == mbs[0]->size() + mbs[1]->size() + mbs[2]->size() - sizeof(media::AudioFrame) * 2);
    MBGuard const g(mb);
    frm = media::AudioFrame(mb);
    for (size_t i=0;i<rawaudio.size();++i)
        REQUIRE(frm.input_buffer[i] == rawaudio[i]);
    REQUIRE(frm.sample_no == START_SAMPLE_NO);
    REQUIRE(fmt == frm.inputfmt);

    for (auto *m : mbs)
        m->release();
    mbs.clear();

    mbs.push_back(mb);
    mb = AudioFramesMerge(mbs);
    MBGuard const gg(mb);
    frm = media::AudioFrame(mb);
    for (size_t i=0;i<rawaudio.size();++i)
        REQUIRE(frm.input_buffer[i] == rawaudio[i]);
    REQUIRE(frm.sample_no == START_SAMPLE_NO);
    REQUIRE(fmt == frm.inputfmt);
}

TEST_CASE("BuildAudioFrame")
{
    constexpr auto SAMPLE_RATE_HZ = 48000;
    constexpr auto CHANNELS = 2;
    constexpr auto TONE_AMPLITUDE = 1000;
    constexpr auto TOTAL_SAMPLES = 77000;
    constexpr auto START_SAMPLE_NO = 7000;

    // Build from one big audio frame with remainder
    media::AudioFormat const fmt(SAMPLE_RATE_HZ, CHANNELS);
    std::vector<short> rawaudio;
    short v = TONE_AMPLITUDE;
    rawaudio.reserve(TOTAL_SAMPLES*fmt.channels);
for (size_t i=0;std::cmp_less(i,TOTAL_SAMPLES*fmt.channels);++i)
        rawaudio.push_back(v++);

    std::vector<ACE_Message_Block*> mbs;
    media::AudioFrame const frm(fmt, rawaudio.data(), TOTAL_SAMPLES, START_SAMPLE_NO);
    mbs.push_back(AudioFrameToMsgBlock(frm));

    uint32_t sampleindex = START_SAMPLE_NO;
    v = TONE_AMPLITUDE;
    constexpr auto CHUNK_120_SAMPLES = 120;
    while (auto *mb = AudioFrameFromList(CHUNK_120_SAMPLES, mbs))
    {
        MBGuard const g(mb);
        REQUIRE(mbs.size());
        media::AudioFrame const newfrm(mb);
        REQUIRE(newfrm.sample_no == sampleindex);
        sampleindex += CHUNK_120_SAMPLES;
        for (int j=0;j<newfrm.input_samples*fmt.channels;++j)
            REQUIRE(newfrm.input_buffer[j] == v++);
    }
    REQUIRE(mbs.size());
    media::AudioFrame remain(mbs[0]);
    MBGuard const g(mbs[0]);
    for (int j=0;j<remain.input_samples*fmt.channels;++j)
        REQUIRE(remain.input_buffer[j] == v++);
    REQUIRE(remain.input_samples == TOTAL_SAMPLES % CHUNK_120_SAMPLES);
    mbs.clear();

    // Build from one big audio frame without remainder
    mbs.push_back(AudioFrameToMsgBlock(frm));
    sampleindex = START_SAMPLE_NO;
    v = TONE_AMPLITUDE;
    std::vector<ACE_Message_Block*> mbs_next;
    constexpr auto CHUNK_100_SAMPLES = 100;
    while (auto *mb = AudioFrameFromList(CHUNK_100_SAMPLES, mbs))
    {
        MBGuard const g(mb);
        media::AudioFrame const newfrm(mb);
        REQUIRE(newfrm.sample_no == sampleindex);
        sampleindex += CHUNK_100_SAMPLES;
        for (int j=0;j<newfrm.input_samples*fmt.channels;++j)
            REQUIRE(newfrm.input_buffer[j] == v++);

        // store for next test-case
        mbs_next.push_back(AudioFrameToMsgBlock(newfrm));
    }
    REQUIRE(mbs.empty());

    // Build from many small audio frames
    mbs = mbs_next;
    sampleindex = START_SAMPLE_NO;
    v = TONE_AMPLITUDE;
    constexpr auto CHUNK_1554_SAMPLES = 1554;
    while (auto *mb = AudioFrameFromList(CHUNK_1554_SAMPLES, mbs))
    {
        MBGuard const g(mb);
        REQUIRE(mbs.size());
        media::AudioFrame const newfrm(mb);
        REQUIRE(newfrm.sample_no == sampleindex);
        sampleindex += CHUNK_1554_SAMPLES;
        for (int j=0;j<newfrm.input_samples*fmt.channels;++j)
            REQUIRE(newfrm.input_buffer[j] == v++);
    }

    remain = media::AudioFrame(mbs[0]);

    for (int j=0;j<remain.input_samples*fmt.channels;++j)
        REQUIRE(remain.input_buffer[j] == v++);
    int remainsamples = 0;
    for (auto *m : mbs)
    {
        remainsamples += media::AudioFrame(m).input_samples;
        m->release();
    }
    REQUIRE(remainsamples == TOTAL_SAMPLES % CHUNK_1554_SAMPLES);
}

TEST_CASE("GenerateToneStereoWaveFile")
{
    constexpr auto SAMPLE_RATE_HZ = 32000;
    constexpr auto CHANNELS = 2;
    constexpr auto TONE_HZ = 500;
    constexpr auto TONE_VOLUME = 8000;
    constexpr auto ITERATIONS = 5;

    media::AudioFormat const fmt(SAMPLE_RATE_HZ, CHANNELS);
    std::vector<short> buf(size_t(fmt.samplerate) * fmt.channels);
    media::AudioFrame frm(fmt, buf.data(), SAMPLE_RATE_HZ);
    WavePCMFile wavfile;
    REQUIRE(wavfile.NewFile(ACE_TEXT("stereo.wav"), fmt));
    for (int i=0;i<ITERATIONS;++i)
    {
        frm.sample_no = GenerateTone(frm, frm.sample_no, TONE_HZ, TONE_VOLUME, false, true);
        REQUIRE(wavfile.AppendSamples(frm.input_buffer, frm.input_samples));
        frm.sample_no = GenerateTone(frm, frm.sample_no, TONE_HZ, TONE_VOLUME, true, false);
        REQUIRE(wavfile.AppendSamples(frm.input_buffer, frm.input_samples));
    }
    wavfile.Close();
    REQUIRE(wavfile.OpenFile(ACE_TEXT("stereo.wav"), true));
    for (int i=0;i<ITERATIONS;++i)
    {
        REQUIRE(wavfile.ReadSamples(frm.input_buffer, frm.input_samples) == frm.input_samples);
        for (int j=0;j<frm.input_samples;j+=2)
        {
            if (i % 2 == 0)
                REQUIRE(frm.input_buffer[j+1] == 0);
            else
                REQUIRE(frm.input_buffer[j] == 0);
        }
    }
}

TEST_CASE( "AudioMuxerRawDifferentStreamTypeDifferentAudioFormat" )
{
    constexpr auto SAMPLE_RATE_HZ = 12000;
    constexpr auto CHANNELS = 1;
    constexpr auto FRAME_SIZE_SEC = .01;

    media::AudioInputFormat const inputfmt(media::AudioFormat(SAMPLE_RATE_HZ, CHANNELS), int(SAMPLE_RATE_HZ * FRAME_SIZE_SEC));
    const int TOTALSAMPLES = inputfmt.GetTotalSamples();
    const int FRAMESIZE = inputfmt.samples;
    const auto FMT = inputfmt.fmt;
    const int FRAMEBYTES = inputfmt.GetBytes();

    msg_queue_t mixed_frames;
    msg_queue_t mixed_ticker;
    AudioMuxer muxer(teamtalk::STREAMTYPE_VOICE | teamtalk::STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO | teamtalk::STREAMTYPE_MEDIAFILE_AUDIO);
    constexpr auto MUX_INTERVAL_FRAMES = 5;
    int const muxinterval_msec = inputfmt.GetDurationMSec() * MUX_INTERVAL_FRAMES;
    muxer.SetMuxInterval(muxinterval_msec);
    auto mixedfunc = [&] (teamtalk::StreamTypes  /*sts*/, const media::AudioFrame& frm)
    {
        auto *mb = AudioFrameToMsgBlock(frm);
        REQUIRE(mixed_frames.enqueue(mb) >= 0);
    };

    auto tickfunc = [&] (teamtalk::StreamTypes  /*sts*/, uint32_t sample_no)
    {
        media::AudioFrame tickfrm;
        tickfrm.sample_no = sample_no;
        REQUIRE(mixed_ticker.enqueue(AudioFrameToMsgBlock(tickfrm)) >= 0);
    };
    muxer.RegisterMuxTick(tickfunc);
    REQUIRE(muxer.RegisterMuxCallback(inputfmt, mixedfunc));

    std::vector<int> userids;
    userids.push_back(1);
    userids.push_back(10);
    userids.push_back(100);
    userids.push_back(1000);
    std::vector<media::AudioFormat> user_fmts;
    user_fmts.push_back(FMT);
    constexpr auto USER2_SAMPLE_RATE_HZ = 48000;
    constexpr auto USER2_CHANNELS = 2;
    constexpr auto USER3_SAMPLE_RATE_HZ = 32000;
    constexpr auto USER3_CHANNELS = 2;
    constexpr auto USER2_FRAME_SIZE_SEC = .01;
    constexpr auto USER3_FRAME_SIZE_SEC = .015;
    constexpr auto USER4_FRAME_SIZE_SEC = .005;
    user_fmts.emplace_back(USER2_SAMPLE_RATE_HZ, USER2_CHANNELS);
    user_fmts.emplace_back(USER3_SAMPLE_RATE_HZ, USER3_CHANNELS);
    user_fmts.push_back(FMT);
    std::vector< std::vector<short> > user_bufs;
    std::vector<media::AudioFrame> user_frames;
    user_bufs.emplace_back(inputfmt.GetTotalSamples(), userids[0]);
    user_bufs.emplace_back(int64_t((user_fmts[1].channels * user_fmts[1].samplerate) * USER2_FRAME_SIZE_SEC), userids[1]);
    user_bufs.emplace_back(int64_t((user_fmts[2].channels * user_fmts[2].samplerate) * USER3_FRAME_SIZE_SEC), userids[2]);
    user_bufs.emplace_back(int64_t((user_fmts[3].channels * user_fmts[3].samplerate) * USER4_FRAME_SIZE_SEC), userids[3]);
    std::vector<teamtalk::StreamType> user_sts;
    user_sts.push_back(teamtalk::STREAMTYPE_VOICE);
    user_sts.push_back(teamtalk::STREAMTYPE_MEDIAFILE_AUDIO);
    user_sts.push_back(teamtalk::STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO);
    user_sts.push_back(teamtalk::STREAMTYPE_VOICE);

    for (size_t i=0;i<userids.size();++i)
    {
        constexpr auto SAMPLE_NO_STRIDE = 1000;
        media::AudioFrame frm(user_fmts[i], user_bufs[i].data(), int(user_bufs[i].size() / user_fmts[i].channels), uint32_t(user_bufs[i].size() * SAMPLE_NO_STRIDE));
        frm.streamid = userids[i];
        user_frames.push_back(frm);
    }

    REQUIRE(user_frames[0].input_samples != user_frames[3].input_samples);

    // submit audio but not enough to generate a frame
    REQUIRE(muxer.QueueUserAudio(userids[0], user_sts[0], user_frames[0]));
    REQUIRE(muxer.QueueUserAudio(userids[1], user_sts[1], user_frames[1]));
    REQUIRE(muxer.QueueUserAudio(userids[2], user_sts[2], user_frames[2]));
    REQUIRE(muxer.QueueUserAudio(userids[3], user_sts[3], user_frames[3]));
    REQUIRE(!muxer.QueueUserAudio(userids[0], teamtalk::STREAMTYPE_DESKTOP, user_frames[0]));

    ACE_Message_Block* mb = nullptr;
    {
        mixed_ticker.dequeue(mb);
        MBGuard const g(mb);
    }

    REQUIRE(mixed_frames.message_count() == 0);

    // submit to generate full mux frame
    user_frames[3].sample_no += user_frames[3].input_samples;
    REQUIRE(muxer.QueueUserAudio(userids[3], user_sts[3], user_frames[3]));

    {
        REQUIRE(mixed_frames.dequeue(mb) >= 0);
        MBGuard const g(mb);
    }

    constexpr auto OVERFLOW_INTERVAL_COUNT = 3;
    // test overflow of resampler buffer
    int n_overflow = int(OVERFLOW_INTERVAL_COUNT * ((muxinterval_msec / 1000.) / USER3_FRAME_SIZE_SEC));
    while ((n_overflow--) != 0)
    {
        user_frames[2].sample_no += user_frames[2].input_samples;
        REQUIRE(muxer.QueueUserAudio(userids[2], user_sts[2], user_frames[2]));
    }

    // not enough audio to generate mux frame
    {
        mixed_ticker.dequeue(mb);
        MBGuard const g(mb);
    }
    REQUIRE(mixed_frames.message_count() == 0);
}

TEST_CASE( "AudioMuxerRawOverflow" )
{
    constexpr auto SAMPLE_RATE_HZ = 48000;
    constexpr auto CHANNELS = 2;
    constexpr auto FRAME_SIZE_SEC = .01;
    constexpr auto QUEUE_SIZE_BYTES = 1024*1024*10;

    media::AudioInputFormat const inputfmt(media::AudioFormat(SAMPLE_RATE_HZ, CHANNELS), int(SAMPLE_RATE_HZ * FRAME_SIZE_SEC));
    const int TOTALSAMPLES = inputfmt.GetTotalSamples();
    const int FRAMESIZE = inputfmt.samples;
    const auto FMT = inputfmt.fmt;

    msg_queue_t mixed_frames;
    msg_queue_t mixed_ticker;
    auto QSIZE = QUEUE_SIZE_BYTES;
    mixed_frames.high_water_mark(QSIZE);
    mixed_frames.low_water_mark(QSIZE);
    mixed_ticker.high_water_mark(QSIZE);
    mixed_ticker.low_water_mark(QSIZE);
    AudioMuxer muxer(teamtalk::STREAMTYPE_VOICE);
    auto mixedfunc = [&] (teamtalk::StreamTypes  /*sts*/, const media::AudioFrame& frm)
    {
        auto *mb = AudioFrameToMsgBlock(frm);
        REQUIRE(mixed_frames.enqueue(mb) >= 0);
    };
    auto tickfunc = [&] (teamtalk::StreamTypes  /*sts*/, uint32_t sample_no)
    {
        media::AudioFrame tickfrm;
        tickfrm.sample_no = sample_no;
        REQUIRE(mixed_ticker.enqueue(AudioFrameToMsgBlock(tickfrm)) >= 0);
    };

    muxer.RegisterMuxTick(tickfunc);
    REQUIRE(muxer.RegisterMuxCallback(inputfmt, mixedfunc));

    std::vector<short> buffer(TOTALSAMPLES, short(1));
    media::AudioFrame frm(FMT, buffer.data(), FRAMESIZE);

    ACE_Message_Block* mb = nullptr;
    {
        REQUIRE(mixed_frames.dequeue(mb) >= 0);
        MBGuard const g(mb);
    }

    // overflow AudioMuxer's queue
    short v = 2;
    constexpr auto OVERFLOW_DURATION_SEC = 2;
    while (frm.sample_no < uint32_t(frm.inputfmt.samplerate * OVERFLOW_DURATION_SEC))
    {
        REQUIRE(muxer.QueueUserAudio(16, teamtalk::STREAMTYPE_VOICE, frm));
        frm.sample_no += FRAMESIZE;
        for (short & i : buffer)
            i = v;
        ++v;
    }

    REQUIRE(muxer.QueueUserAudio(16, teamtalk::STREAMTYPE_VOICE, media::AudioFrame()));

    // verify AudioMuxer survives by ensuring AudioMuxer starts generating silence again.
    auto c = mixed_ticker.message_count();
    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard const g(mb);
        if (media::AudioFrame(mb).input_buffer[0] == 0 && c != mixed_ticker.message_count())
            break;
    }

    // AudioContainer has a 3 second buffer @ 48KHz, so lets overflow that
    auto offset = frm.sample_no;
    constexpr auto AUDIO_CONTAINER_SAMPLE_RATE_HZ = 48000;
    constexpr auto AUDIO_CONTAINER_OVERFLOW_SEC = 10;
    while (frm.sample_no - offset < AUDIO_CONTAINER_SAMPLE_RATE_HZ * AUDIO_CONTAINER_OVERFLOW_SEC)
    {
        REQUIRE(muxer.QueueUserAudio(16, teamtalk::STREAMTYPE_VOICE, frm));
        frm.sample_no += FRAMESIZE;
        for (short & i : buffer)
            i = v;
        ++v;
    }

    REQUIRE(muxer.QueueUserAudio(16, teamtalk::STREAMTYPE_VOICE, media::AudioFrame()));

    // verify AudioMuxer survives by ensuring AudioMuxer starts generating silence again.
    c = mixed_ticker.message_count();
    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard const g(mb);
        if (media::AudioFrame(mb).input_buffer[0] == 0 && c != mixed_ticker.message_count())
            break;
    }
}

TEST_CASE( "AudioMuxerRawSampleIndex" )
{
    constexpr auto SAMPLE_RATE_HZ = 48000;
    constexpr auto CHANNELS = 2;
    constexpr auto FRAME_SIZE_SEC = .01;
    constexpr auto QUEUE_SIZE_BYTES = 1024*1024*10;

    media::AudioInputFormat const inputfmt(media::AudioFormat(SAMPLE_RATE_HZ, CHANNELS), int(SAMPLE_RATE_HZ * FRAME_SIZE_SEC));
    const int TOTALSAMPLES = inputfmt.GetTotalSamples();
    const int FRAMESIZE = inputfmt.samples;
    const auto FMT = inputfmt.fmt;

    ACE_Time_Value zero;
    msg_queue_t mixed_frames;
    msg_queue_t mixed_ticker;
    auto QSIZE = QUEUE_SIZE_BYTES;
    mixed_frames.high_water_mark(QSIZE);
    mixed_frames.low_water_mark(QSIZE);
    mixed_ticker.high_water_mark(QSIZE);
    mixed_ticker.low_water_mark(QSIZE);
    AudioMuxer muxer(teamtalk::STREAMTYPE_VOICE);
    auto mixedfunc = [&] (teamtalk::StreamTypes  /*sts*/, const media::AudioFrame& frm)
    {
        auto *mb = AudioFrameToMsgBlock(frm);
        REQUIRE(mixed_frames.enqueue(mb, &zero) >= 0);
    };
    auto tickfunc = [&] (teamtalk::StreamTypes  /*sts*/, uint32_t sample_no)
    {
        media::AudioFrame tickfrm;
        tickfrm.sample_no = sample_no;
        REQUIRE(mixed_ticker.enqueue(AudioFrameToMsgBlock(tickfrm), &zero) >= 0);
    };

    muxer.RegisterMuxTick(tickfunc);
    REQUIRE(muxer.RegisterMuxCallback(inputfmt, mixedfunc));

    std::vector<short> buffer(TOTALSAMPLES, short(1));
    media::AudioFrame frm(FMT, buffer.data(), FRAMESIZE);

    REQUIRE(muxer.QueueUserAudio(16, teamtalk::STREAMTYPE_VOICE, frm));

    ACE_Message_Block* mb = nullptr;
    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard const g(mb);
        if (media::AudioFrame(mb).input_buffer[0] == 1)
            break;
    }

    buffer.assign(buffer.size(), short(2));
    constexpr auto MISMATCH_SKIP_FRAMES_1 = 2;
    frm.sample_no += FRAMESIZE * MISMATCH_SKIP_FRAMES_1; // sample index mismatch

    REQUIRE(muxer.QueueUserAudio(16, teamtalk::STREAMTYPE_VOICE, frm));
    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard const g(mb);
        // audio frame (2) is lost
        if (media::AudioFrame(mb).input_buffer[0] == 0)
            break;
    }

    buffer.assign(buffer.size(), short(3));
    constexpr auto MISMATCH_SKIP_FRAMES_2 = 3;
    frm.sample_no += FRAMESIZE * MISMATCH_SKIP_FRAMES_2;

    REQUIRE(muxer.QueueUserAudio(16, teamtalk::STREAMTYPE_VOICE, frm));
    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard const g(mb);
        if (media::AudioFrame(mb).input_buffer[0] == 3)
            break;
    }
}

TEST_CASE( "AudioMuxerStreamTypesIntoAudioBlock" )
{

    auto txclient = InitTeamTalk();
    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(txclient));

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    constexpr auto SAMPLE_RATE_HZ = 48000;
    constexpr auto CHANNELS = 2;
    constexpr auto MEDIA_FILE_DURATION_MSEC = 10 * 1000;
    constexpr auto MEDIA_FILE_TONE_HZ = 500;
    mfi.audioFmt.nChannels = CHANNELS;
    mfi.audioFmt.nSampleRate = SAMPLE_RATE_HZ;
    mfi.uDurationMSec = MEDIA_FILE_DURATION_MSEC;
    ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("muxtone.wav"));

    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, MEDIA_FILE_TONE_HZ));

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    mfp.bPaused = FALSE;

    auto session = TT_InitLocalPlayback(rxclient, mfi.szFileName, &mfp);
    REQUIRE(session > 0);

    constexpr auto TX_TONE_HZ = 800;
    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, TX_TONE_HZ));

    TTMessage msg;

    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
    REQUIRE(msg.mediafileinfo.nStatus == MFS_STARTED);

    StreamTypes const sts = STREAMTYPE_VOICE | STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO;
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, sts, TRUE));

    std::vector<int> premux;
    std::vector<int> aftermux;
    uint32_t sum_mux_mf = 0;
    uint32_t sum_mux_mf_voice = 0;
    sum_mux_mf = GetAudioBlockSamplesSum(rxclient, TT_MUXED_USERID, sts);

    REQUIRE(TT_EnableVoiceTransmission(txclient, true));

    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, msg));
    REQUIRE((msg.user.uUserState & USERSTATE_VOICE) == USERSTATE_VOICE);

    // drain
    while (WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK, msg, 0))
    {
        auto *ab = TT_AcquireUserAudioBlock(rxclient, sts, TT_MUXED_USERID);
        REQUIRE(ab);
        REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
    }

    constexpr auto GAIN_RETRIES = 10;
    constexpr auto GAIN_TOLERANCE_FACTOR = 1.2;
    int n_frames = GAIN_RETRIES;
    do
    {
        sum_mux_mf_voice = GetAudioBlockSamplesSum(rxclient, TT_MUXED_USERID, sts);
    }
    while (sum_mux_mf_voice <= sum_mux_mf * GAIN_TOLERANCE_FACTOR && (--n_frames != 0));

    REQUIRE(sum_mux_mf_voice > sum_mux_mf * GAIN_TOLERANCE_FACTOR);
}

TEST_CASE( "AudioMuxerStreamTypeRecording" )
{
    constexpr auto MEDIA_FILE_DURATION_MSEC = 1000;

    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    constexpr auto FILE1_SAMPLE_RATE_HZ = 48000;
    constexpr auto FILE1_CHANNELS = 2;
    constexpr auto FILE1_TONE_HZ = 100;
    constexpr auto FILE2_SAMPLE_RATE_HZ = 44100;
    constexpr auto FILE2_CHANNELS = 1;
    constexpr auto FILE2_TONE_HZ = 200;
    constexpr auto FILE3_SAMPLE_RATE_HZ = 12000;
    constexpr auto FILE3_CHANNELS = 2;
    constexpr auto FILE3_TONE_HZ = 300;
    mfi.audioFmt.nChannels = FILE1_CHANNELS;
    mfi.audioFmt.nSampleRate = FILE1_SAMPLE_RATE_HZ;
    mfi.uDurationMSec = MEDIA_FILE_DURATION_MSEC;
    ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("tone_100.wav"));
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, FILE1_TONE_HZ));
    mfi.audioFmt.nChannels = FILE2_CHANNELS;
    mfi.audioFmt.nSampleRate = FILE2_SAMPLE_RATE_HZ;
    ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("tone_200.wav"));
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, FILE2_TONE_HZ));
    mfi.audioFmt.nChannels = FILE3_CHANNELS;
    mfi.audioFmt.nSampleRate = FILE3_SAMPLE_RATE_HZ;
    ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("tone_300.wav"));
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, FILE3_TONE_HZ));

    Channel chan;
    REQUIRE(TT_GetChannel(rxclient, TT_GetRootChannelID(rxclient), &chan));
    REQUIRE(TT_StartRecordingMuxedStreams(rxclient, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, &chan.audiocodec, ACE_TEXT("muxlocalplayback.wav"), AFF_WAVE_FORMAT));

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    mfp.bPaused = FALSE;

    TTMessage msg;

    auto session = TT_InitLocalPlayback(rxclient, ACE_TEXT("tone_100.wav"), &mfp);
    REQUIRE(session > 0);
    do
    {
        REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
        REQUIRE(msg.nSource == session);
    }
    while (msg.mediafileinfo.nStatus != MFS_FINISHED);

    session = TT_InitLocalPlayback(rxclient, ACE_TEXT("tone_200.wav"), &mfp);
    REQUIRE(session > 0);
    do
    {
        REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
        REQUIRE(msg.nSource == session);
    }
    while (msg.mediafileinfo.nStatus != MFS_FINISHED);

    session = TT_InitLocalPlayback(rxclient, ACE_TEXT("tone_300.wav"), &mfp);
    REQUIRE(session > 0);
    do
    {
        REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
        REQUIRE(msg.nSource == session);
    }
    while (msg.mediafileinfo.nStatus != MFS_FINISHED);

    std::set<int> sessions;
    session = TT_InitLocalPlayback(rxclient, ACE_TEXT("tone_100.wav"), &mfp);
    REQUIRE(session > 0);
    sessions.insert(session);
    session = TT_InitLocalPlayback(rxclient, ACE_TEXT("tone_200.wav"), &mfp);
    REQUIRE(session > 0);
    sessions.insert(session);
    session = TT_InitLocalPlayback(rxclient, ACE_TEXT("tone_300.wav"), &mfp);
    REQUIRE(session > 0);
    sessions.insert(session);

    // mix streams at same time
    while (!sessions.empty())
    {
        REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
        if (msg.mediafileinfo.nStatus == MFS_FINISHED)
            sessions.erase(msg.nSource);
    }

    // quit during playback
    session = TT_InitLocalPlayback(rxclient, ACE_TEXT("tone_300.wav"), &mfp);
    do
    {
        REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
    }
    while (msg.mediafileinfo.uElapsedMSec < msg.mediafileinfo.uDurationMSec / 2);
    TT_StopLocalPlayback(rxclient, session);

    // pause during playback
    session = TT_InitLocalPlayback(rxclient, ACE_TEXT("tone_300.wav"), &mfp);
    do
    {
        REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
    }
    while (msg.mediafileinfo.uElapsedMSec < msg.mediafileinfo.uDurationMSec / 2);
    mfp.bPaused = TRUE;
    REQUIRE(TT_UpdateLocalPlayback(rxclient, session, &mfp));

    constexpr auto RESUME_WAIT_MSEC = 1000;
    // resume playback
    WaitForEvent(rxclient, CLIENTEVENT_NONE, RESUME_WAIT_MSEC);
    mfp.bPaused = FALSE;
    REQUIRE(TT_UpdateLocalPlayback(rxclient, session, &mfp));
    do
    {
        REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
        REQUIRE(msg.nSource == session);
    }
    while (msg.mediafileinfo.nStatus != MFS_FINISHED);

    // quit during playback
    session = TT_InitLocalPlayback(rxclient, ACE_TEXT("tone_300.wav"), &mfp);
    do
    {
        REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
    }
    while (msg.mediafileinfo.uElapsedMSec < msg.mediafileinfo.uDurationMSec / 2);

    REQUIRE(TT_StopRecordingMuxedAudioFile(rxclient));
}

TEST_CASE("AudioMuxerMixedAudioblockStream")
{
    // This test case reproduces a problem in which the Audioblock output stops
    // after playing a file.

    AudioCodec ac = MakeDefaultAudioCodec(OPUS_CODEC);
    ac.opus.nSampleRate = OPUS_SAMPLE_RATE_HZ;
    ac.opus.nChannels = OPUS_CHANNELS;
    ac.opus.nTxIntervalMSec = OPUS_TX_INTERVAL_MSEC;
#if defined(OPUS_FRAMESIZE_120_MS)
    ac.opus.nFrameSizeMSec = OPUS_FRAME_SIZE_120MS_MSEC;
#else
    ac.opus.nFrameSizeMSec = OPUS_FRAME_SIZE_40MS_MSEC;
#endif

    std::array<TTCHAR, TT_STRLEN> filename{};
    ACE_OS::strsncpy(filename.data(), ACE_TEXT("testdata/Opus/on.ogg"), filename.size());

    auto ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient, DEFAULT, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient"), ACE_TEXT("admin"), ACE_TEXT("admin")));

    auto chan = MakeChannel(ttclient, ACE_TEXT("Channel5"), TT_GetRootChannelID(ttclient), ac);
    chan.uChannelType = CHANNEL_PERMANENT | CHANNEL_HIDDEN;
    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoJoinChannel(ttclient, &chan)));

    AudioFormat af;
    af.nAudioFmt = AFF_WAVE_FORMAT;
    constexpr auto MIX_SAMPLE_RATE_HZ = 8000;
    constexpr auto MIX_CHANNELS = 1;
    af.nChannels = MIX_CHANNELS;
    af.nSampleRate = MIX_SAMPLE_RATE_HZ;
    REQUIRE(TT_EnableAudioBlockEventEx(ttclient, TT_MUXED_USERID, STREAMTYPE_VOICE | STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, &af, TRUE));

    WavePCMFile wavfile;
    REQUIRE(wavfile.NewFile(ACE_TEXT("channel5mix.wav"), af.nSampleRate, af.nChannels));

    uint32_t const starttime = GETTIMESTAMP();
    uint32_t durationMSec = 0;

    int silenceBlockCount = 0;
    bool playoutstarted = false;
    TTMessage msg;

    constexpr auto MIX_DURATION_MSEC = 3000;
    while (durationMSec < MIX_DURATION_MSEC)
    {
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
        auto const streamtype = msg.nStreamType;
        AudioBlock* ttAudioblock = TT_AcquireUserAudioBlock(ttclient, streamtype, msg.nSource);
        REQUIRE(ttAudioblock);
        constexpr auto AUDIOBLOCK_DURATION_SEC = .02;
        REQUIRE(ttAudioblock->nSamples == int(ttAudioblock->nSampleRate * AUDIOBLOCK_DURATION_SEC));
        REQUIRE(ttAudioblock->nSampleRate == af.nSampleRate);
        REQUIRE(ttAudioblock->nChannels == af.nChannels);
        wavfile.AppendSamples(reinterpret_cast<const short*>(ttAudioblock->lpRawAudio), ttAudioblock->nSamples);

        if (ttAudioblock->uStreamTypes == STREAMTYPE_NONE)
        {
            silenceBlockCount++;
        }

        constexpr auto SILENCE_BLOCKS_BEFORE_PLAYOUT = 1;
        // Start playout after receving a few block without audio. 
        // Not required to reproduce the problem, but the mechanism might be useful
        // for debugging purpose
        if (!playoutstarted && (silenceBlockCount > SILENCE_BLOCKS_BEFORE_PLAYOUT))
        {
            MediaFilePlayback mfp = {};
            mfp.bPaused = FALSE;
            mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

            INT32 const session = TT_InitLocalPlayback(ttclient, filename.data(), &mfp);
            REQUIRE(session > 0);
            std::cout << "Playout started" << std::endl;
            playoutstarted = true;
        }
        REQUIRE(TT_ReleaseUserAudioBlock(ttclient, ttAudioblock));

        durationMSec = GETTIMESTAMP() - starttime;
    }

    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoRemoveChannel(ttclient, TT_GetMyChannelID(ttclient))));
}

#if defined(ENABLE_OGG)
TEST_CASE( "Opus Read File" )
{

    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));
    AudioCodec codec;
#if defined(ENABLE_OPUSTOOLS) && 0
    codec.nCodec = OPUS_CODEC;
    codec.opus = {};
    codec.opus.nApplication = OPUS_APPLICATION_AUDIO;
    constexpr auto OPUS_BITRATE = 64000;
    constexpr auto OPUS_FRAMESIZE_MSEC = 5;
    constexpr auto OPUS_SAMPLE_RATE = 24000;
    constexpr auto OPUSTOOLS_TX_INTERVAL_MSEC = 400;
    codec.opus.nBitRate = OPUS_BITRATE;
    codec.opus.nChannels = 2;
    codec.opus.nComplexity = 9;
    codec.opus.nFrameSizeMSec = OPUS_FRAMESIZE_MSEC;
    codec.opus.nSampleRate = OPUS_SAMPLE_RATE;
    codec.opus.nTxIntervalMSec = OPUSTOOLS_TX_INTERVAL_MSEC;
    Channel chan = MakeChannel(rxclient, ACE_TEXT("opustools"), TT_GetMyChannelID(rxclient), codec);
    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoJoinChannel(rxclient, &chan)));
#else
    codec.nCodec = SPEEX_VBR_CODEC;
    codec.speex_vbr = {};
    constexpr auto SPEEX_BANDMODE = 1;
    constexpr auto SPEEX_BITRATE = 16000;
    constexpr auto SPEEX_MAX_BITRATE = 32000;
    constexpr auto SPEEX_QUALITY = 5;
    constexpr auto SPEEX_TX_INTERVAL_MSEC = 400;
    codec.speex_vbr.nBandmode = SPEEX_BANDMODE;
    codec.speex_vbr.nBitRate = SPEEX_BITRATE;
    codec.speex_vbr.nMaxBitRate = SPEEX_MAX_BITRATE;
    codec.speex_vbr.nQuality = SPEEX_QUALITY;
    codec.speex_vbr.nTxIntervalMSec = SPEEX_TX_INTERVAL_MSEC;
    Channel chan = MakeChannel(rxclient, ACE_TEXT("speex"), TT_GetMyChannelID(rxclient), codec);
    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoJoinChannel(rxclient, &chan)));
#endif
    std::array<TTCHAR, TT_STRLEN> FILENAME{};
    ACE_OS::strsncpy(FILENAME.data(), ACE_TEXT("MyMuxFile.ogg"), FILENAME.size());
    REQUIRE(TT_GetChannel(rxclient, TT_GetMyChannelID(rxclient), &chan));
    REQUIRE(TT_StartRecordingMuxedAudioFile(rxclient, &chan.audiocodec, FILENAME.data(), AFF_CHANNELCODEC_FORMAT));

    constexpr auto RECORDING_STARTUP_MSEC = 2000;
    WaitForEvent(rxclient, CLIENTEVENT_NONE, RECORDING_STARTUP_MSEC);

    OggFile of;
    REQUIRE(of.Open(FILENAME.data()));
    ogg_page op;
    int pages = 0;
    REQUIRE(of.ReadOggPage(op));
    REQUIRE(op.header_len>0);
    REQUIRE(op.body_len>0);
    pages++;
    while (of.ReadOggPage(op) != 0)pages++;
}
#endif

#if defined(ENABLE_ENCRYPTION)
TEST_CASE("TestHTTPS")
{
    if (GITHUBSKIP)
    {
        std::cout << "Skipping \"TestHTTPS\"... No Internet" << std::endl;
        return;
    }

    std::string response1;
    std::string response2;
    std::string response3;
    REQUIRE(1 == HttpGetRequest("http://www.bearware.dk/teamtalk/weblogin.php?ping=1", response1));
    REQUIRE(1 == HttpGetRequest("https://www.bearware.dk/teamtalk/weblogin.php?ping=1", response2));
    REQUIRE(response1 == response2);
    REQUIRE(1 == HttpGetRequest("https://www.google.com", response3));
}

TEST_CASE("TestWebLogin")
{
    if (GITHUBSKIP)
    {
        std::cout << "Skipping \"TestHTTPS\"... No Internet" << std::endl;
        return;
    }

    ACE_TString token;
    ACE_TString loginid;
    REQUIRE(LoginBearWareAccount(ACE_TEXT("foo"), ACE_TEXT("bar"), token, loginid) == WEBLOGIN_FAILED);
}

TEST_CASE("TestHTTPPost")
{
    if (GITHUBSKIP)
    {
        std::cout << "Skipping \"TestHTTPPost\"... No Internet" << std::endl;
        return;
    }
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = "application/x-www-form-urlencoded";
    std::string const content = "client=TeamTalk5&version=5.0.0.0";
    std::string result;
    REQUIRE(HttpPostRequest("http://www.bearware.dk/teamtalk/tt5update.php", content.c_str(), int(content.length()), headers, result) == 1);
}

TEST_CASE("TestHTTPPostData")
{
    if (GITHUBSKIP)
    {
        std::cout << "Skipping \"TestHTTPPost\"... No Internet" << std::endl;
        return;
    }
    std::map<std::string, std::string> formdata;
    formdata["client"] = "TeamTalk5";
    formdata["version"] = "5.0.0.0";
    std::string result;
    REQUIRE(HttpPostRequest("http://www.bearware.dk/teamtalk/tt5update.php", formdata, result) == 1);
    teamtalk::XMLDocument xmldoc("teamtalk", "1.0");
    REQUIRE(xmldoc.Parse(result));
    std::string const version = xmldoc.GetValue(false, "teamtalk/name", "");
    REQUIRE(version.size() > 0);
}
#endif

#if defined(WIN32)

TEST_CASE("TT_AEC")
{
    if (GITHUBSKIP)
    {
        std::cout << "Skipping \"TT_AEC\"... No WASAPI" << std::endl;
        return;
    }

    auto ttclient = InitTeamTalk();

    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));

    // Only WASAPI supported by CWMAudioAEC
    INT32 indev, outdev;
    REQUIRE(TT_GetDefaultSoundDevicesEx(SOUNDSYSTEM_WASAPI, &indev, &outdev));

    // Set sound effects prior to joining a channel results in CWMAudioAEC being enabled
    SoundDeviceEffects effects = {};
    effects.bEnableEchoCancellation = TRUE;
    REQUIRE(TT_SetSoundDeviceEffects(ttclient, &effects));
    // When using CWMAudioAEC the requirement for shared sample rate between input and output device doesn't apply
    REQUIRE(InitSound(ttclient, DUPLEX, indev, outdev));

    int chanid = TT_GetRootChannelID(ttclient);
    int waitms = DEFWAIT;
    TTMessage msg;
    int cmdid = TT_DoJoinChannelByID(ttclient, chanid, _T(""));
    REQUIRE(cmdid>0);
    while(TT_GetMessage(ttclient, &msg, &waitms))
    {
        REQUIRE(msg.nClientEvent != CLIENTEVENT_INTERNAL_ERROR);
        if (msg.nClientEvent == CLIENTEVENT_CMD_PROCESSING && msg.bActive == FALSE)
            break;
    }

    REQUIRE(TT_EnableAudioBlockEvent(ttclient, TT_LOCAL_USERID, STREAMTYPE_VOICE, TRUE));
    constexpr auto AUDIOBLOCK_COUNT = 20;
    int abCount = AUDIOBLOCK_COUNT;
    AudioBlock* ab;
    while (abCount--)
    {
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
        ab = TT_AcquireUserAudioBlock(ttclient, STREAMTYPE_VOICE, TT_LOCAL_USERID);
        REQUIRE(ab != nullptr);
        REQUIRE(TT_ReleaseUserAudioBlock(ttclient, ab));
    }

    // Reset state
    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoLeaveChannel(ttclient)));
    REQUIRE(TT_CloseSoundDuplexDevices(ttclient));
    effects.bEnableEchoCancellation = FALSE;
    REQUIRE(TT_SetSoundDeviceEffects(ttclient, &effects));
    REQUIRE(TT_EnableAudioBlockEvent(ttclient, TT_LOCAL_USERID, STREAMTYPE_VOICE, FALSE));
    while (ab = TT_AcquireUserAudioBlock(ttclient, STREAMTYPE_VOICE, TT_LOCAL_USERID))
        TT_ReleaseUserAudioBlock(ttclient, ab);

    // Test that we can also only run with AGC and NS
    effects.bEnableAGC = TRUE;
    effects.bEnableDenoise = TRUE;
    REQUIRE(TT_SetSoundDeviceEffects(ttclient, &effects));
    REQUIRE(InitSound(ttclient, DUPLEX, indev, outdev));
    cmdid = TT_DoJoinChannelByID(ttclient, chanid, _T(""));
    REQUIRE(cmdid>0);
    while(TT_GetMessage(ttclient, &msg, &waitms))
    {
        REQUIRE(msg.nClientEvent != CLIENTEVENT_INTERNAL_ERROR);
        if (msg.nClientEvent == CLIENTEVENT_CMD_PROCESSING && msg.bActive == FALSE)
            break;
    }

    REQUIRE(TT_EnableAudioBlockEvent(ttclient, TT_LOCAL_USERID, STREAMTYPE_VOICE, TRUE));

    abCount = AUDIOBLOCK_COUNT;
    while (abCount--)
    {
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
        ab = TT_AcquireUserAudioBlock(ttclient, STREAMTYPE_VOICE, TT_LOCAL_USERID);
        REQUIRE(ab != nullptr);
        REQUIRE(TT_ReleaseUserAudioBlock(ttclient, ab));
    }

    // It's not possible to change sound effects when sound device is active (in channel)
    effects.bEnableEchoCancellation = TRUE;
    REQUIRE(TT_SetSoundDeviceEffects(ttclient, &effects) == FALSE);

    REQUIRE(TT_CloseSoundDuplexDevices(ttclient));
    REQUIRE(TT_SetSoundDeviceEffects(ttclient, &effects));
    REQUIRE(InitSound(ttclient, DUPLEX, indev, outdev));
    constexpr auto NO_ERROR_WAIT_MSEC = 500;
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_INTERNAL_ERROR, NO_ERROR_WAIT_MSEC) == false);

    // cannot disable sound effects either
    effects.bEnableAGC = effects.bEnableDenoise = effects.bEnableEchoCancellation = FALSE;
    REQUIRE(TT_SetSoundDeviceEffects(ttclient, &effects) == FALSE);
}
#endif

#if defined(ENABLE_FFMPEG)
TEST_CASE("testThumbnail")
{
    constexpr auto STREAM_SAMPLE_RATE_HZ = 16000;
    constexpr auto STREAM_CHANNELS = 2;
    constexpr auto STREAM_FRAMESIZE = 1600;

    // ffmpeg -i in.mp3 -i teamtalk.png -map 0:0 -map 1:0 -c copy -id3v2_version 3 -metadata:s:v title="Album cover" -metadata:s:v comment="Cover (front)" out.mp3

    std::array<TTCHAR, TT_STRLEN> filename{};
    ACE_OS::strsncpy(filename.data(), ACE_TEXT("testdata/mp3/thumbnail.mp3"), filename.size());

    MediaFileProp mfp;
    REQUIRE(GetMediaFileProp(filename.data(), mfp));
    REQUIRE(mfp.video.IsValid() == false);

    MediaStreamOutput const prop(media::AudioFormat(STREAM_SAMPLE_RATE_HZ, STREAM_CHANNELS), STREAM_FRAMESIZE, media::FOURCC_NONE);
    FFmpegStreamer ffmpeg(filename.data(), prop);

    REQUIRE(ffmpeg.Open());

    std::promise<bool> done;
    auto sig_done = done.get_future();

    auto status = [&] (const MediaFileProp&  /*mfp*/, MediaStreamStatus status) {
                      if (status == MEDIASTREAM_FINISHED)
                          done.set_value(true);
                  };

    auto audio = [] (media::AudioFrame& /*audio_frame*/, ACE_Message_Block* /*mb_audio*/) {
                     return false;
                 };

    auto video = [] (media::VideoFrame& /*video_frame*/, ACE_Message_Block* /*mb_video*/) {
                    return false;
                };

    ffmpeg.RegisterStatusCallback(status, true);
    ffmpeg.RegisterAudioCallback(audio, true);
    ffmpeg.RegisterVideoCallback(video, true);

    REQUIRE(ffmpeg.StartStream());

    REQUIRE(sig_done.get());
}
#endif

#if defined(ENABLE_ENCRYPTION)

TEST_CASE("testSSLNonBlockConnector")
{

    ACE_Timer_Heap timer_queue;
    ACE_Reactor r(new ACE_Select_Reactor(nullptr, &timer_queue), true);
    auto grp = ACE_Thread_Manager::instance()->spawn(EventLoop, &r);
    SyncReactor(r);

    class MyClass : public StreamListener<CryptStreamHandler::StreamHandler_t>
    {
        ACE_Reactor* m_rr;
    public:
        MyClass(ACE_Reactor* r) : m_rr(r) {}
        void OnOpened(CryptStreamHandler::StreamHandler_t& /*streamer*/) override { }
        void OnClosed(CryptStreamHandler::StreamHandler_t& /*streamer*/) override { }
        bool OnReceive(CryptStreamHandler::StreamHandler_t& /*streamer*/, const char* buff, int /*len*/) override
        {
            constexpr auto TEAMTALK_PREFIX_LENGTH = 8;
            REQUIRE(std::string(buff).substr(0, TEAMTALK_PREFIX_LENGTH) == "teamtalk");
            m_rr->end_reactor_event_loop();
            return true;
        }
        bool OnSend(CryptStreamHandler::StreamHandler_t& /*streamer*/) override { return true; }
    } lsn(&r);

    teamtalk::crypt_connector_t connector(&r, ACE_NONBLOCK);
    ACE_SSL_Context* context = CryptStreamHandler::AddSSLContext(&r);
    REQUIRE(context);
    CryptStreamHandler::StreamHandler_t* csh = new CryptStreamHandler(&r);
    csh->SetListener(&lsn);
    ACE_Synch_Options const options(ACE_Synch_Options::USE_REACTOR, ACE_Time_Value(0,0));
    constexpr auto TEST_SERVER_PORT = 10443;
    auto addrs = DetermineHostAddress(ACE_TEXT("127.0.0.1"), TEST_SERVER_PORT);
    REQUIRE(addrs.size());
    int const ret = connector.connect(csh, addrs[0], options);
    REQUIRE((ret == -1 && ACE_OS::last_error() == EWOULDBLOCK || ret == 0));

    ACE_Thread_Manager::instance ()->wait_grp(grp);
    csh->close();
    CryptStreamHandler::RemoveSSLContext(&r);
}

TEST_CASE("testSSLBlockingConnector")
{

    ACE_Timer_Heap timer_queue;
    ACE_Reactor r(new ACE_Select_Reactor(nullptr, &timer_queue), true);
    auto grp = ACE_Thread_Manager::instance()->spawn(EventLoop, &r);
    SyncReactor(r);

    class MyClass: public StreamListener<CryptStreamHandler::StreamHandler_t>
    {
        ACE_Reactor* m_rr;
    public:
        MyClass(ACE_Reactor* r) : m_rr(r) {}
        void OnOpened(CryptStreamHandler::StreamHandler_t& /*streamer*/) override { }
        void OnClosed(CryptStreamHandler::StreamHandler_t&  /*streamer*/) override { }
        bool OnReceive(CryptStreamHandler::StreamHandler_t&  /*streamer*/, const char* buff, int /*len*/) override
        {
            constexpr auto TEAMTALK_PREFIX_LENGTH = 8;
            REQUIRE(std::string(buff).substr(0, TEAMTALK_PREFIX_LENGTH) == "teamtalk");
            m_rr->end_reactor_event_loop();
            return true;
        }
        bool OnSend(CryptStreamHandler::StreamHandler_t&  /*streamer*/) override { return true; }
    } lsn(&r);

    teamtalk::crypt_connector_t connector(&r);
    ACE_SSL_Context* context = CryptStreamHandler::AddSSLContext(&r);
    REQUIRE(context);
    CryptStreamHandler::StreamHandler_t* csh = new CryptStreamHandler(&r);
    csh->SetListener(&lsn);
    constexpr auto TEST_SERVER_PORT = 10443;
    auto addrs = DetermineHostAddress(ACE_TEXT("127.0.0.1"), TEST_SERVER_PORT);
    REQUIRE(addrs.size());
    REQUIRE(connector.connect(csh, addrs[0]) == 0);

    ACE_Thread_Manager::instance ()->wait_grp(grp);
    csh->close();
    CryptStreamHandler::RemoveSSLContext(&r);
}

TEST_CASE("testConnector")
{

    ACE_Timer_Heap timer_queue;
    ACE_Reactor r(new ACE_Select_Reactor(nullptr, &timer_queue), true);
    auto grp = ACE_Thread_Manager::instance()->spawn(EventLoop, &r);
    SyncReactor(r);

    class MyClass : public StreamListener<DefaultStreamHandler::StreamHandler_t>
    {
        ACE_Reactor* m_rr;
    public:
        MyClass(ACE_Reactor* r) : m_rr(r) {}
        void OnOpened(DefaultStreamHandler::StreamHandler_t&  /*streamer*/) override {m_rr->end_reactor_event_loop();}
        void OnClosed(DefaultStreamHandler::StreamHandler_t&  /*streamer*/) override {}
        bool OnReceive(DefaultStreamHandler::StreamHandler_t&  /*streamer*/, const char* /*buff*/, int /*len*/) override { return true; }
        bool OnSend(DefaultStreamHandler::StreamHandler_t&  /*streamer*/) override { return true; }
    } lsn(&r);

    //teamtalk::connector_t connector(&r);
    ACE_Connector< DefaultStreamHandler, ACE_SOCK_CONNECTOR > connector(&r);
    auto* csh = new DefaultStreamHandler(&r);
    csh->SetListener(&lsn);
    ACE_Synch_Options const options(ACE_Synch_Options::USE_REACTOR, ACE_Time_Value(0,0));
    constexpr auto TEST_SERVER_PORT = 10443;
    auto addrs = DetermineHostAddress(ACE_TEXT("127.0.0.1"), TEST_SERVER_PORT);
    REQUIRE(addrs.size());
    int const ret = connector.connect(csh, addrs[0], options);
    REQUIRE(((ret == -1 && ACE_OS::last_error() == EWOULDBLOCK) || ret == 0));

    ACE_Thread_Manager::instance ()->wait_grp(grp);
    csh->close();
}

#endif


TEST_CASE("NewVoiceStreamMatch")
{
    auto txclient = InitTeamTalk();
    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(txclient));

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));

    REQUIRE(TT_EnableAudioBlockEvent(txclient, TT_LOCAL_TX_USERID, STREAMTYPE_VOICE, TRUE));

    REQUIRE(TT_EnableVoiceTransmission(txclient, TRUE));
    TTMessage msg;
    REQUIRE(WaitForEvent(txclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
    auto *ab = TT_AcquireUserAudioBlock(txclient, STREAMTYPE_VOICE, TT_LOCAL_TX_USERID);
    REQUIRE(ab);
    int const streamid = ab->nStreamID;
    REQUIRE(TT_ReleaseUserAudioBlock(txclient, ab));
    REQUIRE(TT_EnableVoiceTransmission(txclient, FALSE));

    REQUIRE(JoinRoot(rxclient));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_GetMyUserID(txclient), STREAMTYPE_VOICE, TRUE));
    REQUIRE(TT_EnableVoiceTransmission(txclient, TRUE));
    auto firststream = [streamid] (TTMessage msg)
    {
        return msg.nSource == streamid + 1;
    };
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_FIRSTVOICESTREAMPACKET, firststream, &msg));
    REQUIRE(msg.nSource == streamid + 1);
}

#if defined(ENABLE_WEBRTC)

#if 0 /* gain_controller1 doesn't work */
TEST_CASE("SoundLoopbackDuplexDBFS1")
{
    SoundDevice indev, outdev;
    REQUIRE(GetSoundDevices(indev, outdev));

    std::cout << "input: " << indev.nDeviceID << " name: " << indev.szDeviceName
              << " channels: " << indev.nMaxInputChannels << " samplerate: " << indev.nDefaultSampleRate
              << " output: " << outdev.nDeviceID << " name: " << outdev.szDeviceName << std::endl;
    TTInstPtr ttclient = InitTeamTalk();

    AudioPreprocessor preprocess = {};

    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;
    preprocess.webrtc.gaincontroller1.bEnable = TRUE;
    preprocess.webrtc.gaincontroller1.nTargetLevelDBFS = 1;

    auto sndloop = TT_StartSoundLoopbackTestEx(indev.nDeviceID, outdev.nDeviceID, indev.nDefaultSampleRate,
                                          1, TRUE, &preprocess, nullptr);
    REQUIRE(sndloop);

    std::cout << "Recording...." << std::endl;

    WaitForEvent(ttclient, CLIENTEVENT_NONE, 10000);

    REQUIRE(TT_CloseSoundLoopbackTest(sndloop));
}

TEST_CASE("SoundLoopbackDuplexDBFS30")
{
    SoundDevice indev, outdev;
    REQUIRE(GetSoundDevices(indev, outdev));

    std::cout << "input: " << indev.nDeviceID << " name: " << indev.szDeviceName
              << " channels: " << indev.nMaxInputChannels << " samplerate: " << indev.nDefaultSampleRate
              << " output: " << outdev.nDeviceID << " name: " << outdev.szDeviceName << std::endl;
    TTInstPtr ttclient = InitTeamTalk();

    AudioPreprocessor preprocess = {};

    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;
    preprocess.webrtc.gaincontroller1.bEnable = TRUE;
    preprocess.webrtc.gaincontroller1.nTargetLevelDBFS = 30;

    auto sndloop = TT_StartSoundLoopbackTestEx(indev.nDeviceID, outdev.nDeviceID, indev.nDefaultSampleRate,
                                          1, TRUE, &preprocess, nullptr);
    REQUIRE(sndloop);

    std::cout << "Recording...." << std::endl;

    WaitForEvent(ttclient, CLIENTEVENT_NONE, 10000);

    REQUIRE(TT_CloseSoundLoopbackTest(sndloop));
}
#endif /* gain_controller1 */

TEST_CASE("SoundLoopbackDefault")
{

    SoundDevice indev;
    SoundDevice outdev;
    REQUIRE(GetSoundDevices(indev, outdev));

/*
#if defined(UNICODE)
    std::wcout <<
#else
    std::cout <<
#endif
        "input: " << indev.nDeviceID << " name: " << indev.szDeviceName
               << " channels: " << indev.nMaxInputChannels << " samplerate: " << indev.nDefaultSampleRate
               << " output: " << outdev.nDeviceID << " name: " << outdev.szDeviceName << std::endl;
*/
    TTInstPtr ttclient = InitTeamTalk();

    AudioPreprocessor preprocess = {};

    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;
    preprocess.webrtc.gaincontroller2.bEnable = TRUE;
    constexpr auto GAIN_DB = 25;
    preprocess.webrtc.gaincontroller2.fixeddigital.fGainDB = GAIN_DB;

    preprocess.webrtc.noisesuppression.bEnable = FALSE;
    preprocess.webrtc.noisesuppression.nLevel = 0;

    constexpr auto CHANNELS = 1;
    constexpr auto RECORDING_DURATION_MSEC = 30000;
    auto *sndloop = TT_StartSoundLoopbackTestEx(indev.nDeviceID, outdev.nDeviceID, indev.nDefaultSampleRate,
                                          CHANNELS, FALSE, &preprocess, nullptr);
    REQUIRE(sndloop);

    std::cout << "Recording...." << std::endl;

    WaitForEvent(ttclient, CLIENTEVENT_NONE, RECORDING_DURATION_MSEC);

    REQUIRE(TT_CloseSoundLoopbackTest(sndloop));
}

TEST_CASE("WebRTC_SampleRates")
{
    constexpr auto GAIN_DB = 25;

    TTInstPtr const ttclient = InitTeamTalk();

    AudioPreprocessor preprocess = {};

    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;
    preprocess.webrtc.gaincontroller2.bEnable = TRUE;
    preprocess.webrtc.gaincontroller2.fixeddigital.fGainDB = GAIN_DB;

    const std::vector<int> standardSampleRates = {8000, 12000, 16000, 24000, 32000, 44100, 48000};

    for (auto samplerate : standardSampleRates)
    {
        constexpr auto CHANNELS = 2;
        auto *sndloop = TT_StartSoundLoopbackTestEx(TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL,
                                                   TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL,
                                                   samplerate, CHANNELS, TRUE, &preprocess, nullptr);
        REQUIRE(sndloop);

        REQUIRE(TT_CloseSoundLoopbackTest(sndloop));
    }
}

TEST_CASE("WebRTCPreprocessor")
{

    TTInstPtr ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(ttclient));
    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoSubscribe(ttclient, TT_GetMyUserID(ttclient), SUBSCRIBE_VOICE)));

    std::array<TTCHAR, CURRENT_DIRECTORY_BUFFER_SIZE> curdir{};
    ACE_OS::getcwd(curdir.data(), curdir.size());
    REQUIRE(TT_SetUserMediaStorageDir(ttclient, TT_GetMyUserID(ttclient), curdir.data(), ACE_TEXT(""), AFF_WAVE_FORMAT));

    REQUIRE(TT_EnableVoiceTransmission(ttclient, true));
    constexpr auto RECORDING_DURATION_MSEC = 5000;
    WaitForEvent(ttclient, CLIENTEVENT_NONE, RECORDING_DURATION_MSEC);

    AudioPreprocessor preprocess = {};

    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;

    preprocess.webrtc.noisesuppression.bEnable = TRUE;
    constexpr auto DENOISE_LEVEL = 3;
    preprocess.webrtc.noisesuppression.nLevel = DENOISE_LEVEL;

    REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess));
    WaitForEvent(ttclient, CLIENTEVENT_NONE, RECORDING_DURATION_MSEC);

    preprocess.webrtc.gaincontroller2.bEnable = TRUE;
    constexpr auto GAIN_DB = 10;
    preprocess.webrtc.gaincontroller2.fixeddigital.fGainDB = GAIN_DB;

    REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess));
    WaitForEvent(ttclient, CLIENTEVENT_NONE, RECORDING_DURATION_MSEC);

    REQUIRE(TT_EnableVoiceTransmission(ttclient, false));

}

#if 0 /* gain_controller1 doesn't work */
TEST_CASE("WebRTC_gaincontroller1")
{
    TTInstPtr ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;
    mfp.audioPreprocessor.webrtc.gaincontroller1.bEnable = TRUE;
    mfp.audioPreprocessor.webrtc.gaincontroller1.nTargetLevelDBFS = 25;

    auto session = TT_InitLocalPlayback(ttclient, ACE_TEXT("input_low.wav"), &mfp);
    REQUIRE(session > 0);

    bool success = false, toggled = false, stop = false;
    TTMessage msg;
    while (WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, 5000) && !stop)
    {
        switch(msg.mediafileinfo.nStatus)
        {
        case MFS_PLAYING :
            if (msg.mediafileinfo.uElapsedMSec >= 3000 && !toggled)
            {
                mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
                mfp.audioPreprocessor.webrtc.gaincontroller1.bEnable = TRUE;
                mfp.audioPreprocessor.webrtc.gaincontroller1.nTargetLevelDBFS = 0;
                REQUIRE(TT_UpdateLocalPlayback(ttclient, session, &mfp));
                toggled = true;
                std::cout << "Toggled: " << msg.mediafileinfo.uElapsedMSec << std::endl;
            }
            if (msg.mediafileinfo.uElapsedMSec >= 10000)
            {
                std::cout << "Elapsed: " << msg.mediafileinfo.uElapsedMSec << std::endl;
                stop = true;
            }
            break;
        case MFS_FINISHED :
            success = true;
            break;
        }
    }
    REQUIRE(toggled);
    REQUIRE(success);
}
#endif /* gain_controller1 */

TEST_CASE("WebRTC_gaincontroller2")
{
    constexpr auto DENOISE_LEVEL = 3;

    TTInstPtr ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;

    mfp.audioPreprocessor.webrtc.gaincontroller2.bEnable = FALSE;
    mfp.audioPreprocessor.webrtc.gaincontroller2.fixeddigital.fGainDB = 0;

    mfp.audioPreprocessor.webrtc.noisesuppression.bEnable = FALSE;
    mfp.audioPreprocessor.webrtc.noisesuppression.nLevel = DENOISE_LEVEL;

    std::array<TTCHAR, TT_STRLEN> filename{};
    ACE_OS::strsncpy(filename.data(), ACE_TEXT("testdata/AGC/input_16k_mono_low.wav"), filename.size());

    auto session = TT_InitLocalPlayback(ttclient, filename.data(), &mfp);
    REQUIRE(session > 0);

    bool success = false;
    bool toggled = false;
    bool stop = false;
    TTMessage msg;
    constexpr auto EVENT_WAIT_MSEC = 5000;
    constexpr auto TOGGLE_ELAPSED_MSEC = 3000;
    constexpr auto STOP_ELAPSED_MSEC = 10000;
    constexpr auto GAIN_DB = 25;
    while (WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, EVENT_WAIT_MSEC) && !stop)
    {
        switch(msg.mediafileinfo.nStatus)
        {
        case MFS_PLAYING :
            if (msg.mediafileinfo.uElapsedMSec >= TOGGLE_ELAPSED_MSEC && !toggled)
            {
                mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
                mfp.audioPreprocessor.webrtc.gaincontroller2.bEnable = TRUE;
                mfp.audioPreprocessor.webrtc.gaincontroller2.fixeddigital.fGainDB = GAIN_DB;
                REQUIRE(TT_UpdateLocalPlayback(ttclient, session, &mfp));
                toggled = true;
                // std::cout << "Toggled: " << msg.mediafileinfo.uElapsedMSec << std::endl;
            }
            if (msg.mediafileinfo.uElapsedMSec >= STOP_ELAPSED_MSEC)
            {
                // std::cout << "Elapsed: " << msg.mediafileinfo.uElapsedMSec << std::endl;
                stop = true;
            }
            break;
        case MFS_FINISHED :
            success = true;
            break;
        default :
            break;
        }
    }
    REQUIRE(toggled);
    REQUIRE(success);
}

TEST_CASE("WebRTC_echocancel")
{

    TTInstPtr ttclient = InitTeamTalk();
    SoundDeviceEffects effects = {};
    effects.bEnableEchoCancellation = FALSE;
    REQUIRE(TT_SetSoundDeviceEffects(ttclient, &effects));
    REQUIRE(InitSound(ttclient, DUPLEX));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    // REQUIRE(JoinRoot(ttclient));

    AudioCodec codec = {};
    codec.nCodec = SPEEX_VBR_CODEC;
    codec.speex_vbr.nBandmode = 2;
    constexpr auto SPEEX_BITRATE = 16000;
    constexpr auto SPEEX_QUALITY = 10;
    constexpr auto SPEEX_TX_INTERVAL_MSEC = 40;
    codec.speex_vbr.nBitRate = SPEEX_BITRATE;
    codec.speex_vbr.nMaxBitRate = SPEEX_UWB_MAX_BITRATE;
    codec.speex_vbr.nQuality = SPEEX_QUALITY;
    codec.speex_vbr.nTxIntervalMSec = SPEEX_TX_INTERVAL_MSEC;
    Channel const chan = MakeChannel(ttclient, ACE_TEXT("speex"), TT_GetRootChannelID(ttclient), codec);
    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoJoinChannel(ttclient, &chan)));

    AudioPreprocessor preprocess = {};

    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;

    // WEBRTC_AUDIOPREPROCESSOR
    constexpr auto DENOISE_LEVEL = 2;
    constexpr auto GAIN_DB = 25;
    // SPEEXDSP_AUDIOPREPROCESSOR
    constexpr auto DEFAULT_AGC_ENABLE          = TRUE;
    constexpr auto DEFAULT_AGC_GAINLEVEL       = 8000;
    constexpr auto DEFAULT_AGC_INC_MAXDB       = 12;
    constexpr auto DEFAULT_AGC_DEC_MAXDB       = (-40);
    constexpr auto DEFAULT_AGC_GAINMAXDB       = 30;
    constexpr auto DEFAULT_DENOISE_ENABLE      = TRUE;
    constexpr auto DEFAULT_DENOISE_SUPPRESS    = (-30);
    constexpr auto DEFAULT_ECHO_ENABLE         = TRUE;
    constexpr auto DEFAULT_ECHO_SUPPRESS       = (-40);
    constexpr auto DEFAULT_ECHO_SUPPRESSACTIVE = (-15);

    switch (preprocess.nPreprocessor)
    {
    case WEBRTC_AUDIOPREPROCESSOR :
        preprocess.webrtc.echocanceller.bEnable = TRUE;

        preprocess.webrtc.noisesuppression.bEnable = TRUE;
        preprocess.webrtc.noisesuppression.nLevel = DENOISE_LEVEL;

        preprocess.webrtc.gaincontroller2.bEnable = TRUE;
        preprocess.webrtc.gaincontroller2.fixeddigital.fGainDB = GAIN_DB;
        break;
    case SPEEXDSP_AUDIOPREPROCESSOR : {
        preprocess.speexdsp.bEnableAGC = DEFAULT_AGC_ENABLE;
        preprocess.speexdsp.nGainLevel = DEFAULT_AGC_GAINLEVEL;
        preprocess.speexdsp.nMaxIncDBSec = DEFAULT_AGC_INC_MAXDB;
        preprocess.speexdsp.nMaxDecDBSec = DEFAULT_AGC_DEC_MAXDB;
        preprocess.speexdsp.nMaxGainDB = DEFAULT_AGC_GAINMAXDB;
        preprocess.speexdsp.bEnableDenoise = DEFAULT_DENOISE_ENABLE;
        preprocess.speexdsp.nMaxNoiseSuppressDB = DEFAULT_DENOISE_SUPPRESS;
        preprocess.speexdsp.bEnableEchoCancellation = DEFAULT_ECHO_ENABLE;
        preprocess.speexdsp.nEchoSuppress = DEFAULT_ECHO_SUPPRESS;
        preprocess.speexdsp.nEchoSuppressActive = DEFAULT_ECHO_SUPPRESSACTIVE;
        break;
    }
    case NO_AUDIOPREPROCESSOR :
    case TEAMTALK_AUDIOPREPROCESSOR :
    case WEBRTC_AUDIOPREPROCESSOR_OBSOLETE_R4332 :
        REQUIRE(false);
        break;
    }

    REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess));

    REQUIRE(TT_EnableVoiceTransmission(ttclient, true));

    constexpr auto RECORDING_DURATION_MSEC = 5000;
    WaitForEvent(ttclient, CLIENTEVENT_NONE, RECORDING_DURATION_MSEC);
}

TEST_CASE("WebRTC_Preamplifier")
{
    constexpr auto TONE_HZ = 300;

    TTInstPtr ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(ttclient));

    int level = 0;
    
    REQUIRE(TT_EnableVoiceTransmission(ttclient, TRUE));
    REQUIRE(TT_DBG_SetSoundInputTone(ttclient, STREAMTYPE_VOICE, TONE_HZ));
    REQUIRE(TT_EnableAudioBlockEvent(ttclient, TT_LOCAL_TX_USERID, STREAMTYPE_VOICE, TRUE));

    // no gain
    AudioPreprocessor preprocess = {};
    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;
    REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess));

    TTMessage msg = {};
    int streamid = 0;
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
    auto *ab = TT_AcquireUserAudioBlock(ttclient, STREAMTYPE_VOICE, TT_LOCAL_TX_USERID);
    REQUIRE(ab);
    streamid = ab->nStreamID;
    TT_ReleaseUserAudioBlock(ttclient, ab);
    level = TT_GetSoundInputLevel(ttclient);
    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoLeaveChannel(ttclient)));
    REQUIRE(TT_EnableAudioBlockEvent(ttclient, TT_LOCAL_TX_USERID, STREAMTYPE_VOICE, FALSE));
    REQUIRE(TT_AcquireUserAudioBlock(ttclient, STREAMTYPE_VOICE, TT_LOCAL_TX_USERID) == nullptr);
    REQUIRE(TT_EnableVoiceTransmission(ttclient, FALSE));

    // half gain
    preprocess.webrtc.preamplifier.bEnable = TRUE;
    constexpr auto HALF_GAIN_FACTOR = .5f;
    preprocess.webrtc.preamplifier.fFixedGainFactor = HALF_GAIN_FACTOR;
    REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess));

    REQUIRE(JoinRoot(ttclient));
    REQUIRE(TT_EnableVoiceTransmission(ttclient, TRUE));
    REQUIRE(TT_EnableAudioBlockEvent(ttclient, TT_LOCAL_TX_USERID, STREAMTYPE_VOICE, TRUE));
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK));
    ab = TT_AcquireUserAudioBlock(ttclient, STREAMTYPE_VOICE, TT_LOCAL_TX_USERID);
    REQUIRE(ab);
    REQUIRE(streamid + 1 == ab->nStreamID);
    TT_ReleaseUserAudioBlock(ttclient, ab);
    REQUIRE(level / 2 == TT_GetSoundInputLevel(ttclient));
    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoLeaveChannel(ttclient)));
    REQUIRE(TT_EnableAudioBlockEvent(ttclient, TT_LOCAL_TX_USERID, STREAMTYPE_VOICE, FALSE));
    REQUIRE(TT_EnableVoiceTransmission(ttclient, FALSE));
}

TEST_CASE("WebRTC-reinit")
{

    TTInstPtr ttclient = InitTeamTalk();
    SoundDeviceEffects effects = {};
    effects.bEnableEchoCancellation = FALSE;
    REQUIRE(TT_SetSoundDeviceEffects(ttclient, &effects));
    REQUIRE(InitSound(ttclient, DUPLEX));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    // REQUIRE(JoinRoot(ttclient));

    AudioCodec audiocodec = {};
    audiocodec.nCodec = OPUS_CODEC;
    audiocodec.opus.nApplication = OPUS_APPLICATION_VOIP;
    audiocodec.opus.nTxIntervalMSec = OPUS_TX_INTERVAL_MSEC;
#if defined(OPUS_FRAMESIZE_120_MS)
    audiocodec.opus.nFrameSizeMSec = OPUS_FRAME_SIZE_120MS_MSEC;
#else
    audiocodec.opus.nFrameSizeMSec = OPUS_FRAME_SIZE_40MS_MSEC;
#endif
    audiocodec.opus.nBitRate = OPUS_MIN_BITRATE;
    audiocodec.opus.nChannels = OPUS_CHANNELS;
    audiocodec.opus.nComplexity = OPUS_COMPLEXITY;
    audiocodec.opus.nSampleRate = OPUS_SAMPLE_RATE_HZ;
    audiocodec.opus.bDTX = 1;
    audiocodec.opus.bFEC = 1;
    audiocodec.opus.bVBR = 0;
    audiocodec.opus.bVBRConstraint = 0;

    Channel const chan = MakeChannel(ttclient, ACE_TEXT("speex"), TT_GetRootChannelID(ttclient), audiocodec);
    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoJoinChannel(ttclient, &chan)));

    AudioPreprocessor preprocess = {};

    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;
    preprocess.webrtc.echocanceller.bEnable = TRUE;

    preprocess.webrtc.noisesuppression.bEnable = TRUE;
    constexpr auto DENOISE_LEVEL = 2;
    constexpr auto GAIN_DB = 25;
    preprocess.webrtc.noisesuppression.nLevel = DENOISE_LEVEL;

    preprocess.webrtc.gaincontroller2.bEnable = TRUE;
    preprocess.webrtc.gaincontroller2.fixeddigital.fGainDB = GAIN_DB;


    AudioPreprocessor preprocess_reinit = {};
    preprocess_reinit.nPreprocessor = NO_AUDIOPREPROCESSOR;

    constexpr auto REINIT_ITERATIONS = 100;
    constexpr auto REINIT_WAIT_MSEC = 20;
    for (int i = 0; i < REINIT_ITERATIONS; i++)
    {
        REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess));
        WaitForEvent(ttclient, CLIENTEVENT_NONE, REINIT_WAIT_MSEC);

        REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess_reinit));
        WaitForEvent(ttclient, CLIENTEVENT_NONE, REINIT_WAIT_MSEC);

    }
}
#endif /* ENABLE_WEBRTC */

TEST_CASE("TeamTalk_VAD")
{
    constexpr auto VA_LEVEL = 63;
    constexpr auto VA_STOP_DELAY_MSEC = 100;
    constexpr auto TONE_HZ = 500;

    TTInstPtr ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(ttclient));

    REQUIRE(TT_EnableVoiceActivation(ttclient, TRUE));
    REQUIRE(TT_SetVoiceActivationLevel(ttclient, VA_LEVEL));
    REQUIRE(TT_SetVoiceActivationStopDelay(ttclient, VA_STOP_DELAY_MSEC));

    TTMessage msg = {};
    REQUIRE(TT_DBG_SetSoundInputTone(ttclient, STREAMTYPE_VOICE, TONE_HZ));
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_VOICE_ACTIVATION, msg));
    REQUIRE(msg.bActive);

    REQUIRE(TT_DBG_SetSoundInputTone(ttclient, STREAMTYPE_VOICE, 0));
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_VOICE_ACTIVATION, msg));
    REQUIRE(!msg.bActive);
}

#if defined(ENABLE_PORTAUDIO) && defined(WIN32)

int paSamples = 0;
uint32_t paTimeStamp = 0;

int Foo_StreamCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* pUserData)
{
    paSamples += framesPerBuffer;
    if (!paTimeStamp)
        paTimeStamp = GETTIMESTAMP();

    return paContinue;
}

TEST_CASE("PortAudioRaw_SamplesPerSec")
{

    if (GITHUBSKIP)
    {
        std::cout << "Skipping \"PortAudioRaw_SamplesPerSec\"... No WASAPI" << std::endl;
        return;
    }

    PaError err = Pa_Initialize();

    PaDeviceIndex inputdeviceid = -1, outputdeviceid = -1;
    PaHostApiIndex hostApi = Pa_HostApiTypeIdToHostApiIndex(paWASAPI);
    if (hostApi != paHostApiNotFound)
    {
        const PaHostApiInfo* hostapi = Pa_GetHostApiInfo(hostApi);
        if (hostapi)
        {
            inputdeviceid = hostapi->defaultInputDevice;
            outputdeviceid = hostapi->defaultOutputDevice;
        }
    }
    REQUIRE(outputdeviceid >= 0);

    const PaDeviceInfo* ininfo = Pa_GetDeviceInfo(outputdeviceid);
    REQUIRE(ininfo);
    PaStreamParameters outputParameters = {};
    outputParameters.device = outputdeviceid;
    outputParameters.channelCount = 1;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = ininfo->defaultLowOutputLatency;

    PaStream* outstream;
    constexpr auto FRAME_SIZE_SEC = .04;
    constexpr auto MEASURE_DURATION_SEC = 2;
    constexpr auto SLEEP_MSEC = 1000;
    err = Pa_OpenStream(&outstream, nullptr, &outputParameters,
                        ininfo->defaultSampleRate, uint32_t(ininfo->defaultSampleRate * FRAME_SIZE_SEC),
                        paClipOff, Foo_StreamCallback, static_cast<void*> (0));

    REQUIRE(Pa_StartStream(outstream) == paNoError);
    while (paSamples < ininfo->defaultSampleRate * MEASURE_DURATION_SEC)
    {
        Pa_Sleep(SLEEP_MSEC);

        auto samplesDurationMSec = PCM16_SAMPLES_DURATION(paSamples, int(ininfo->defaultSampleRate));
        auto durationMSec = GETTIMESTAMP() - paTimeStamp;
        auto skew = int(samplesDurationMSec - durationMSec);
        // std::cout << "Samples duration: " << samplesDurationMSec << " / " << durationMSec << "  " << skew << std::endl;

        constexpr auto SKEW_TOLERANCE_MSEC = 0.08 * 1000;
        REQUIRE(skew < SKEW_TOLERANCE_MSEC);
    }

    Pa_Terminate();
}

TEST_CASE("PortAudio_SamplesPerSec")
{
    if (GITHUBSKIP)
    {
        std::cout << "Skipping \"PortAudio_SamplesPerSec\"... No WASAPI" << std::endl;
        return;
    }

    auto snd = soundsystem::GetInstance();
    auto grp = snd->OpenSoundGroup();

    int inputdeviceid, outputdeviceid;
    REQUIRE(snd->GetDefaultDevices(soundsystem::SOUND_API_WASAPI, inputdeviceid, outputdeviceid));
    soundsystem::devices_t devs;
    REQUIRE(snd->GetSoundDevices(devs));

    auto ioutdev = std::find_if(devs.begin(), devs.end(), [outputdeviceid](soundsystem::DeviceInfo& d)
        {
            return d.id == outputdeviceid;
        });

    REQUIRE(ioutdev != devs.end());

    soundsystem::DeviceInfo& outdev = *ioutdev;

    uint32_t samples = 0, starttime = 0;

    class MyStream : public soundsystem::StreamPlayer
    {
        uint32_t& samples, &starttime;
    public:
        MyStream(uint32_t& s, uint32_t& st) : samples(s), starttime(st) {}
        bool StreamPlayerCb(const soundsystem::OutputStreamer& streamer, short* buffer, int framesPerBuffer)
        {
            if (!starttime)
                starttime = GETTIMESTAMP();
            samples += framesPerBuffer;
            return true;
        }
    } player(samples, starttime);

    constexpr int SAMPLERATE = 48000, CHANNELS = 1;
    constexpr auto FRAME_SIZE_SEC = .04;
    constexpr auto MEASURE_DURATION_SEC = 2;
    constexpr auto SLEEP_MSEC = 1000;
    REQUIRE(snd->OpenOutputStream(&player, outputdeviceid, grp, SAMPLERATE, CHANNELS, uint32_t(SAMPLERATE * FRAME_SIZE_SEC)));
    REQUIRE(snd->StartStream(&player));

    while (samples < uint32_t(outdev.default_samplerate * MEASURE_DURATION_SEC))
    {
        Pa_Sleep(SLEEP_MSEC);

        auto samplesDurationMSec = PCM16_SAMPLES_DURATION(samples, int(outdev.default_samplerate));
        auto durationMSec = GETTIMESTAMP() - starttime;
        auto skew = int(samplesDurationMSec - durationMSec);

        // std::cout << "Samples duration: " << samplesDurationMSec << " / " << durationMSec << "  " << skew << std::endl;

        constexpr auto SKEW_TOLERANCE_MSEC = 0.08 * 1000;
        REQUIRE(skew < SKEW_TOLERANCE_MSEC);
    }
    REQUIRE(snd->CloseOutputStream(&player));
    snd->RemoveSoundGroup(grp);
}

#endif

TEST_CASE("InjectAudio")
{

    TTInstPtr ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient, DEFAULT, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(ttclient));

    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoSubscribe(ttclient, TT_GetMyUserID(ttclient), SUBSCRIBE_VOICE)));

    constexpr auto SAMPLERATE = OPUS_SAMPLE_RATE_HZ;
    constexpr auto CHANNELS = OPUS_CHANNELS;
    std::vector<short> buf(SAMPLERATE * CHANNELS);
    AudioBlock ab = {};
    ab.nStreamID = 1;
    ab.nSampleRate = SAMPLERATE;
    ab.nChannels = CHANNELS;
    ab.lpRawAudio = buf.data();
    ab.nSamples = SAMPLERATE;

    constexpr auto INJECT_DURATION_SEC = 3;
    // 3 secs
    int samples = SAMPLERATE * INJECT_DURATION_SEC;

    do
    {
        REQUIRE(TT_InsertAudioBlock(ttclient, &ab));

        TTMessage msg;
        do
        {
            REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_AUDIOINPUT, msg));
        } while (msg.audioinputprogress.uQueueMSec > 0);

    } while ((samples -= SAMPLERATE) > 0);
}

TEST_CASE("InjectAudioInputGain")
{
    TTInstPtr ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient, DEFAULT, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(ttclient));

    Channel chan = {};
    REQUIRE(TT_GetChannel(ttclient, TT_GetMyChannelID(ttclient), &chan));
    REQUIRE(chan.audiocodec.nCodec == OPUS_CODEC);

    constexpr auto TONE_HZ = 500;
    constexpr auto N_FRAMES = 5;

    int const samples = PCM16_DURATION_SAMPLES(chan.audiocodec.opus.nTxIntervalMSec, chan.audiocodec.opus.nSampleRate) * N_FRAMES;
    std::vector<short> buffer(size_t(samples) * chan.audiocodec.opus.nChannels);
    media::AudioFrame frm(media::AudioFormat(chan.audiocodec.opus.nSampleRate, chan.audiocodec.opus.nChannels), buffer.data(), samples);
    int const sampleindex = GenerateTone(frm, 0, TONE_HZ);

    AudioBlock ab = {};
    ab.lpRawAudio = frm.input_buffer;
    ab.nChannels = frm.inputfmt.channels;
    ab.nSampleRate = frm.inputfmt.samplerate;
    ab.uSampleIndex = frm.sample_no;
    ab.nSamples = frm.input_samples;

    REQUIRE(TT_EnableAudioBlockEvent(ttclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));
    REQUIRE(TT_InsertAudioBlock(ttclient, &ab));

    // wait for 'ab' to appear
    int sumsamples_pregain = 0;
    while ((sumsamples_pregain = GetAudioBlockSamplesSum(ttclient, TT_MUXED_USERID, STREAMTYPE_VOICE)) == 0);
    int recv_frames = 1;
    for (;recv_frames < N_FRAMES;++recv_frames)
        sumsamples_pregain += GetAudioBlockSamplesSum(ttclient, TT_MUXED_USERID, STREAMTYPE_VOICE);

    constexpr auto GAIN_MULTIPLIER = 2;
    constexpr auto UNDERFLOW_WAIT_INTERVALS = 2;
    // AudioMuxer underflow
    REQUIRE(!WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, chan.audiocodec.opus.nTxIntervalMSec * UNDERFLOW_WAIT_INTERVALS));

    // double gain
    REQUIRE(TT_SetSoundInputGainLevel(ttclient, TT_GetSoundInputGainLevel(ttclient) * GAIN_MULTIPLIER));
    ab.uSampleIndex = ab.nSamples;
    REQUIRE(TT_InsertAudioBlock(ttclient, &ab));

    int sumsamples_postgain = 0;
    for (recv_frames=0;recv_frames < N_FRAMES;++recv_frames)
        sumsamples_postgain += GetAudioBlockSamplesSum(ttclient, TT_MUXED_USERID, STREAMTYPE_VOICE);

    constexpr auto GAIN_TOLERANCE_FACTOR = 1.9;
    REQUIRE(sumsamples_pregain * GAIN_TOLERANCE_FACTOR < sumsamples_postgain);
}

TEST_CASE("FixedJitterBuffer")
{

    auto txclient = InitTeamTalk();
    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(txclient));

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    constexpr auto FIXED_DELAY_MSEC = 240;
    constexpr auto MAX_ADAPTIVE_DELAY_MSEC = 10000;
    constexpr auto TONE_HZ = 500;
    uint32_t const fixeddelay = FIXED_DELAY_MSEC;

    JitterConfig jitterconf{};
    jitterconf.nFixedDelayMSec = fixeddelay;
    jitterconf.bUseAdativeDejitter = 0;
    jitterconf.nMaxAdaptiveDelayMSec = MAX_ADAPTIVE_DELAY_MSEC;

    TT_SetUserJitterControl(rxclient, TT_GetMyUserID(txclient), STREAMTYPE_VOICE, &jitterconf);

    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, TONE_HZ));
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));

    auto voicestart = [&](TTMessage msg)
    {
        return msg.nClientEvent == CLIENTEVENT_USER_STATECHANGE &&
            msg.user.nUserID == TT_GetMyUserID(txclient) &&
            (msg.user.uUserState & USERSTATE_VOICE) == USERSTATE_VOICE;
    };

    uint32_t const starttime = GETTIMESTAMP();
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, voicestart));

    uint32_t const endtime = GETTIMESTAMP();
    uint32_t const delay = (endtime - starttime);
    INFO("Measured voice delay is " << delay);

    REQUIRE((delay >= fixeddelay));
    //Measuring the maximum deviation of the delay is not reliably possible because the CLIENTEVENT_USER_STATECHANGE is detected/notified on it's own timer.
}

TEST_CASE("SetGetJitterBufferControl")
{

    auto txclient = InitTeamTalk();
    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(txclient));

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    constexpr auto FIXED_DELAY_MSEC = 240;
    constexpr auto MAX_ADAPTIVE_DELAY_MSEC = 10000;
    constexpr auto ACTIVE_ADAPTIVE_DELAY_MSEC = 800;
    uint32_t const fixeddelay = FIXED_DELAY_MSEC;

    JitterConfig jitterconf_in{};
    jitterconf_in.nFixedDelayMSec = fixeddelay;
    jitterconf_in.bUseAdativeDejitter = 0;
    jitterconf_in.nMaxAdaptiveDelayMSec = MAX_ADAPTIVE_DELAY_MSEC;
    jitterconf_in.nActiveAdaptiveDelayMSec = ACTIVE_ADAPTIVE_DELAY_MSEC;

    TT_SetUserJitterControl(rxclient, TT_GetMyUserID(txclient), STREAMTYPE_VOICE, &jitterconf_in);

    JitterConfig jitterconf_out{};
    TT_GetUserJitterControl(rxclient, TT_GetMyUserID(txclient), STREAMTYPE_VOICE, &jitterconf_out);

    REQUIRE(jitterconf_out.nFixedDelayMSec == jitterconf_out.nFixedDelayMSec);
    REQUIRE(jitterconf_out.bUseAdativeDejitter == jitterconf_out.bUseAdativeDejitter);
    REQUIRE(jitterconf_out.nMaxAdaptiveDelayMSec == jitterconf_out.nMaxAdaptiveDelayMSec);
    REQUIRE(jitterconf_out.nActiveAdaptiveDelayMSec == jitterconf_out.nActiveAdaptiveDelayMSec);
}

TEST_CASE("VideoCapture")
{
    using namespace vidcap;

    videocapture_t capture = VideoCapture::Create();

    auto devs = capture->GetDevices();

    vidcap::VidCapDevice dev;

    for (size_t i=0;i<devs.size() && dev.vidcapformats.empty();++i)
    {
        if (!devs[i].vidcapformats.empty())
            dev = devs[i];
    }

    if (dev.deviceid.empty())
        return;

    auto capformat = dev.vidcapformats.at(0);

    std::condition_variable cv;

    constexpr auto FRAME_COUNT = 10;
    int frames = FRAME_COUNT;
    auto callback = [&] (media::VideoFrame& /*video_frame*/, ACE_Message_Block* /*mb_video*/)
    {
        frames--;
        cv.notify_all();
        return false;
    };

    REQUIRE(capture->InitVideoCapture(dev.deviceid, capformat));

    REQUIRE(capture->RegisterVideoFormat(callback, capformat.fourcc));

    REQUIRE(capture->StartVideoCapture());

    do {
        std::mutex mtx;
        std::unique_lock<std::mutex> lck(mtx);
        REQUIRE(cv.wait_for(lck, std::chrono::milliseconds(DEFWAIT)) == std::cv_status::no_timeout);
    } while (frames >= 0);
}

#if defined(ENABLE_VPX)
TEST_CASE("StreamVideoFile")
{

    auto txclient = InitTeamTalk();
    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(txclient));

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    VideoCodec vid = {};
    vid.nCodec = WEBM_VP8_CODEC;
    constexpr auto VP8_TARGET_BITRATE_KBPS = 128;
    vid.webm_vp8.nRcTargetBitrate = VP8_TARGET_BITRATE_KBPS;
    vid.webm_vp8.nEncodeDeadline = WEBM_VPX_DL_REALTIME;

#if defined(WIN32)
    std::array<TTCHAR, TT_STRLEN> filename{};
    ACE_OS::strsncpy(filename.data(), ACE_TEXT("testdata/Video/EasterFlowers.wmv"), filename.size());
#else
    std::array<TTCHAR, TT_STRLEN> filename{};
    ACE_OS::strsncpy(filename.data(), ACE_TEXT("testdata/Video/MOV03830.MPG"), filename.size());
#endif

    REQUIRE(TT_StartStreamingMediaFileToChannel(txclient, filename.data(), &vid));

    TTMessage msg;
    bool stop = false;
    while (!stop)
    {
        REQUIRE(WaitForEvent(txclient, CLIENTEVENT_STREAM_MEDIAFILE, msg));
        switch(msg.mediafileinfo.nStatus)
        {
        case MFS_PLAYING :
            break;
        case MFS_FINISHED :
            stop = true;
            break;
        default :
            break;
        }

        if (WaitForEvent(rxclient, CLIENTEVENT_USER_MEDIAFILE_VIDEO, msg, 0))
        {
            auto *vb = TT_AcquireUserMediaVideoFrame(rxclient, msg.nSource);
            if (vb != nullptr)
            {
                REQUIRE(TT_ReleaseUserMediaVideoFrame(rxclient, vb));
            }
        }
    }
}
#endif /* ENABLE_VPX */

TEST_CASE("ReactorDeadlock_BUG")
{
    constexpr auto SAMPLE_RATE_HZ = 48000;
    constexpr auto CHANNELS = 2;
    constexpr auto MEDIA_FILE_DURATION_MSEC = 1000;
    constexpr auto TONE_HZ = 500;

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = CHANNELS;
    mfi.audioFmt.nSampleRate = SAMPLE_RATE_HZ;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("temp.wav"), TT_STRLEN);
    mfi.uDurationMSec = MEDIA_FILE_DURATION_MSEC;
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, TONE_HZ));
    
    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
    mfp.bPaused = FALSE;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    TTMessage msg;
    INT32 playid = 0;

    TTInstPtr ttclient(TT_InitTeamTalkPoll());
    REQUIRE(InitSound(ttclient));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(ttclient));
    playid = TT_InitLocalPlayback(ttclient, ACE_TEXT("temp.wav"), &mfp);
    REQUIRE(playid > 0);
    constexpr auto PLAYBACK_INSTANCES = 10;
    constexpr auto PLAYBACK_START_WAIT_MSEC = 10;
    for (int i=0;i<PLAYBACK_INSTANCES;++i)
    {
        REQUIRE(TT_InitLocalPlayback(ttclient, ACE_TEXT("temp.wav"), &mfp) > 0);
        WaitForEvent(ttclient, CLIENTEVENT_NONE, PLAYBACK_START_WAIT_MSEC);
    }

    bool done = false;
    while(WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg) && !done)
    {
        switch (msg.mediafileinfo.nStatus)
        {
        default :
        case MFS_FINISHED :
            if (playid == msg.nSource)
                done = true;
            break;
        case MFS_PLAYING :
            break;
        }
    }
    REQUIRE(TT_Disconnect(ttclient));
}

TEST_CASE("LocalPlaybackEventOrder")
{
    constexpr auto SAMPLE_RATE_HZ = 48000;
    constexpr auto CHANNELS = 2;
    constexpr auto TONE_HZ = 500;

    std::vector<MediaFileInfo> mfis;
    for (int const duration : {10, 20, 50, 80, 120})
    {
        MediaFileInfo mfi = {};
        mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
        mfi.audioFmt.nChannels = CHANNELS;
        mfi.audioFmt.nSampleRate = SAMPLE_RATE_HZ;
        ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("temp_%d.wav"), duration);
        mfi.uDurationMSec = duration;
        REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, TONE_HZ));
        mfis.push_back(mfi);
    }

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
    mfp.bPaused = FALSE;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    auto ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));

    for (auto mfi : mfis)
    {
        TTMessage msg;
        INT32 playid = 0;
        bool done = false;
        bool gotmsg;
        int started = 0;
        int waittime = DEFWAIT;

        playid = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);
        REQUIRE(playid > 0);
        constexpr auto FINISHED_WAIT_MULTIPLIER = 2;
        while ((gotmsg = WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, waittime)))
        {
            REQUIRE(msg.nSource == playid);
            switch (msg.mediafileinfo.nStatus)
            {
            case MFS_STARTED :
                started++;
                REQUIRE(started == 1);
                REQUIRE(!done);
                break;
            case MFS_FINISHED :
                REQUIRE(!done);
                done = true;
                waittime = mfi.uDurationMSec * FINISHED_WAIT_MULTIPLIER;
                break;
            default :
                // no event should take place after MFS_FINISHED
                REQUIRE(!done);
                break;
            }
        }
        REQUIRE(!gotmsg);
        REQUIRE(done);
        REQUIRE(started == 1);
    }
}

TEST_CASE("LocalPlaybackPerformance")
{
    constexpr auto IN_SAMPLERATE = 48000;
    constexpr auto IN_CHANNELS = 2;
    constexpr auto IN_FRAMESIZE = int(IN_SAMPLERATE * .01);
    constexpr auto MEDIA_FILE_DURATION_MSEC = 100;
    constexpr auto TONE_HZ = 500;

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = IN_CHANNELS;
    mfi.audioFmt.nSampleRate = IN_SAMPLERATE;
    mfi.uDurationMSec = MEDIA_FILE_DURATION_MSEC;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("TTPlayOpusOgg.wav"), TT_STRLEN);
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, TONE_HZ));

    TTInstPtr ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
    mfp.bPaused = FALSE;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

    bool stop = false;
    bool started = false;
    bool paused = false;
    TTMessage msg;
    INT32 session = 0;
    uint32_t durationMSec = GETTIMESTAMP();
    uint32_t startupMSec = GETTIMESTAMP();

    // test duration of OpusFileStreamer playback
    session = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);
    REQUIRE(session > 0);

    while (!stop && WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, DEFWAIT))
    {
        switch (msg.mediafileinfo.nStatus)
        {
        case MFS_STARTED:
            REQUIRE(msg.mediafileinfo.uElapsedMSec == 0);
            REQUIRE(!started);
            started = true;
            std::cout << "Startup time: " << GETTIMESTAMP() - startupMSec << std::endl;
            break;
        case MFS_PLAYING:
            break;
        case MFS_FINISHED:
            REQUIRE(!stop);
            stop = true;
            break;
        default:
            break;
        }
    }
    REQUIRE(started);
    REQUIRE(stop);
    std::cout << "Duration time: " << GETTIMESTAMP() - durationMSec << std::endl;

    started = stop = false;
    mfp.bPaused = TRUE;
    session = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);
    REQUIRE(session > 0);
    durationMSec = startupMSec = GETTIMESTAMP();
    mfp.bPaused = FALSE;
    REQUIRE(TT_UpdateLocalPlayback(ttclient, session, &mfp));

    while (!stop && WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, DEFWAIT))
    {
        switch (msg.mediafileinfo.nStatus)
        {
        case MFS_PAUSED :
            break;
        case MFS_STARTED:
            REQUIRE(msg.mediafileinfo.uElapsedMSec == 0);
            REQUIRE(!started);
            started = true;
            std::cout << "Startup time: " << GETTIMESTAMP() - startupMSec << std::endl;
            break;
        case MFS_PLAYING:
            break;
        case MFS_FINISHED:
            REQUIRE(!stop);
            stop = true;
            break;
        default:
            break;
        }
    }

    REQUIRE(started);
    REQUIRE(stop);
    std::cout << "Duration time: " << GETTIMESTAMP() - durationMSec << std::endl;

}

#if TEAMTALK_KNOWN_BUGS
TEST_CASE("SSLBlocking_BUG")
{
    ACE_SOCK_Acceptor acceptor(ACE_INET_Addr(u_short(0), "127.0.0.1"));
    ACE_INET_Addr bindd;
    REQUIRE(acceptor.get_local_addr(bindd) == 0);
    u_short port = bindd.get_port_number();

    TTInstPtr ttclient(TT_InitTeamTalkPoll());
    auto threadfunc = [&]()
    {
        Connect(ttclient, ACE_TEXT("127.0.0.1"), port, 10333, TRUE);
    };

    std::thread tr(threadfunc);
    tr.detach();

    WaitForEvent(ttclient, CLIENTEVENT_NONE, 1000);
    TT_Disconnect(ttclient);
    TT_CloseTeamTalk(ttclient);
}

TEST_CASE("ReactorLockedTimerStart_BUG")
{
    using namespace teamtalk;
    using namespace soundsystem;

    class MyClientNode : public ClientNodeBase
                       , public VoiceLogListener
    {
        VoiceLogger m_vlog;
    public:
        MyClientNode() : m_vlog(this) {}
        void OnMediaFileStatus(int , teamtalk::MediaFileStatus ,
            const teamtalk::VoiceLogFile& ) override {}

        bool SoundDuplexMode() override { return false; }

        int GetUserID() const override { return 0; }
        // Get ID of current channel (0 = not set)
        int GetChannelID() override { return 0; }

        bool QueuePacket(FieldPacket* ) override { return 0; }
        VoiceLogger& voicelogger() override { return m_vlog; }

        void AudioUserCallback(int , teamtalk::StreamType ,
                               const media::AudioFrame & ) override {}

        int TimerEvent(uint32_t , long ) override
        {
            return 0;
        }
        void StreamDuplexCb(const DuplexStreamer&, const short*, short*, int) override {}
        SoundDeviceFeatures GetDuplexFeatures() override { return soundsystem::SOUNDDEVICEFEATURE_NONE; }
    } myclient;

    std::condition_variable cv_locked, cv_hold;;
    std::mutex mtx;
    bool locked = false;
    auto reactorlock = [&]()
    {
        guard_t g(myclient.reactor_lock());
        {
            std::unique_lock<std::mutex> lck(mtx);
            cv_locked.notify_all();
            locked = true;
        }

        std::unique_lock<std::mutex> lck(mtx);
        REQUIRE(cv_hold.wait_for(lck, std::chrono::milliseconds(DEFWAIT)) == std::cv_status::no_timeout);
    };

    std::thread tr(reactorlock);
    tr.detach();

    {
        std::unique_lock<std::mutex> lck(mtx);
        REQUIRE(cv_locked.wait_for(lck, std::chrono::milliseconds(DEFWAIT), [&locked]() { return locked; }));
    }

    auto reactor = myclient.GetEventLoop();
    // Cannot schedule_timer because ClientNode::reactor_lock() is locked by 'tr'-thread
    myclient.StartUserTimer(1 | USER_TIMER_MASK, 123, 0, ACE_Time_Value::zero, ACE_Time_Value::zero);

    std::unique_lock<std::mutex> lck(mtx);
    cv_hold.notify_all();

    // Basically starting a timer should only be allowed by 
    // ACE_Reactor-thread or another thread currently holding ACE_Reactor's lock.
    //
    // This issue is currently present in:
    // - ClientNode::MediaPlaybackStatus()
    // - ClientNode::MediaStreamStatusCallback()
}
#endif

TEST_CASE("StreamMediaToAudioBlock")
{

    auto txclient = InitTeamTalk();
    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(txclient));

    auto rxclient = InitTeamTalk();
    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    constexpr auto SAMPLE_RATE_HZ = 48000;
    constexpr auto CHANNELS = 2;
    constexpr auto MEDIA_FILE_DURATION_MSEC = 10 * 1000;
    constexpr auto TONE_HZ = 500;
    mfi.audioFmt.nChannels = CHANNELS;
    mfi.audioFmt.nSampleRate = SAMPLE_RATE_HZ;
    mfi.uDurationMSec = MEDIA_FILE_DURATION_MSEC;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("streamfile.wav"), TT_STRLEN);
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, TONE_HZ));

    REQUIRE(TT_EnableAudioBlockEvent(txclient, TT_LOCAL_USERID, STREAMTYPE_MEDIAFILE_AUDIO, TRUE));
    REQUIRE(TT_StartStreamingMediaFileToChannel(txclient, mfi.szFileName, nullptr));

    TTMessage msg;
    REQUIRE(WaitForEvent(txclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
    REQUIRE(msg.nStreamType == STREAMTYPE_MEDIAFILE_AUDIO);
    REQUIRE(msg.nSource == TT_LOCAL_USERID);
    auto *ab = TT_AcquireUserAudioBlock(txclient, STREAMTYPE_MEDIAFILE_AUDIO, TT_LOCAL_USERID);
    REQUIRE(ab);
    REQUIRE(TT_ReleaseUserAudioBlock(txclient, ab));
}

TEST_CASE("LocalPlaybackToAudioBlock")
{
    constexpr auto SAMPLE_RATE_HZ = 48000;
    constexpr auto CHANNELS = 2;
    constexpr auto MEDIA_FILE_DURATION_MSEC = 100 * 1000;
    constexpr auto TONE_HZ = 500;

    auto txclient = InitTeamTalk();
    REQUIRE(InitSound(txclient));

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = CHANNELS;
    mfi.audioFmt.nSampleRate = SAMPLE_RATE_HZ;
    mfi.uDurationMSec = MEDIA_FILE_DURATION_MSEC;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("playbackfile.wav"), TT_STRLEN);
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, TONE_HZ));

    MediaFilePlayback mfp = {};
    mfp.bPaused = TRUE;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    auto sessionid = TT_InitLocalPlayback(txclient, mfi.szFileName, &mfp);
    REQUIRE(sessionid > 0);
    REQUIRE(TT_EnableAudioBlockEvent(txclient, sessionid, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, TRUE));

    mfp.bPaused = FALSE;
    REQUIRE(TT_UpdateLocalPlayback(txclient, sessionid, &mfp));

    TTMessage msg;
    REQUIRE(WaitForEvent(txclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
    REQUIRE(msg.nStreamType == STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO);
    REQUIRE(msg.nSource == sessionid);
    auto *ab = TT_AcquireUserAudioBlock(txclient, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, sessionid);
    REQUIRE(ab);
    REQUIRE(TT_ReleaseUserAudioBlock(txclient, ab));
    REQUIRE(TT_StopLocalPlayback(txclient, sessionid));

    REQUIRE(TT_EnableAudioBlockEvent(txclient, sessionid, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, FALSE));

    while (WaitForEvent(txclient, CLIENTEVENT_USER_AUDIOBLOCK, msg, 0))
    {
        REQUIRE(msg.nStreamType == STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO);
        REQUIRE(TT_AcquireUserAudioBlock(txclient, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, msg.nSource) == nullptr);
    }

    // get all playback sessions
    REQUIRE(TT_EnableAudioBlockEvent(txclient, TT_LOCAL_USERID, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, TRUE));

    std::set<INT32> allsessions;
    std::set<INT32> sessions;
    constexpr auto PLAYBACK_SESSIONS = 10;
    for (int i=0;i<PLAYBACK_SESSIONS;i++)
    {
        sessionid = TT_InitLocalPlayback(txclient, mfi.szFileName, &mfp);
        REQUIRE(sessionid > 0);
        sessions.insert(sessionid);
        allsessions.insert(sessionid);
    }

    while ((!sessions.empty()) && WaitForEvent(txclient, CLIENTEVENT_USER_AUDIOBLOCK, msg))
    {
        REQUIRE(msg.nStreamType == STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO);
        ab = TT_AcquireUserAudioBlock(txclient, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, msg.nSource);
        REQUIRE(ab);
        REQUIRE(TT_ReleaseUserAudioBlock(txclient, ab));
        sessions.erase(msg.nSource);
    }

    while (TT_InitLocalPlayback(txclient, mfi.szFileName, &mfp) > 0)
    {
        while (WaitForEvent(txclient, CLIENTEVENT_USER_AUDIOBLOCK, msg, 0))
        {
            REQUIRE(msg.nStreamType == STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO);
            ab = TT_AcquireUserAudioBlock(txclient, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, msg.nSource);
            REQUIRE(ab);
            REQUIRE(TT_ReleaseUserAudioBlock(txclient, ab));
        }
    }

    REQUIRE(TT_StopLocalPlayback(txclient, *allsessions.rbegin()));
    REQUIRE(TT_StopLocalPlayback(txclient, *allsessions.begin()));

    REQUIRE(TT_InitLocalPlayback(txclient, mfi.szFileName, &mfp) > 0);
    REQUIRE(TT_InitLocalPlayback(txclient, mfi.szFileName, &mfp) > 0);
}

TEST_CASE("LocalPlaybackDisconnect")
{
    constexpr auto SAMPLE_RATE_HZ = 48000;
    constexpr auto CHANNELS = 2;
    constexpr auto MEDIA_FILE_DURATION_MSEC = 1000;
    constexpr auto TONE_HZ = 500;

    auto txclient = InitTeamTalk();
    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(txclient));

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = CHANNELS;
    mfi.audioFmt.nSampleRate = SAMPLE_RATE_HZ;
    mfi.uDurationMSec = MEDIA_FILE_DURATION_MSEC;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("playbackfile.wav"), TT_STRLEN);
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, TONE_HZ));

    MediaFilePlayback mfp = {};
    mfp.bPaused = FALSE;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    auto sessionid = TT_InitLocalPlayback(txclient, mfi.szFileName, &mfp);
    REQUIRE(sessionid > 0);

    TTMessage msg;
    REQUIRE(WaitForEvent(txclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
    REQUIRE(msg.mediafileinfo.nStatus == MFS_STARTED);
    REQUIRE(WaitForEvent(txclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
    REQUIRE(msg.mediafileinfo.nStatus == MFS_PLAYING);
    REQUIRE(TT_Disconnect(txclient));

    while (WaitForEvent(txclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg) && msg.mediafileinfo.nStatus == MFS_PLAYING);
    REQUIRE(msg.mediafileinfo.nStatus == MFS_FINISHED);
}

TEST_CASE("LocalPlaybackSharedDevice")
{
    constexpr auto SAMPLE_RATE_HZ = 48000;
    constexpr auto CHANNELS = 2;
    constexpr auto MEDIA_FILE_DURATION_MSEC = 1000;
    constexpr auto TONE_HZ = 600;

    auto ttclient = InitTeamTalk();
    InitSound(ttclient, SHARED_INPUT_OUTPUT);

    MediaFileInfo mfi = {};;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("hest.wav"), TT_STRLEN);;
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nSampleRate = SAMPLE_RATE_HZ;
    mfi.audioFmt.nChannels = CHANNELS;
    mfi.uDurationMSec = MEDIA_FILE_DURATION_MSEC;
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, TONE_HZ));

    // Call TT_InitLocalPlayback for file 1, PAUSE=FALSE
    MediaFilePlayback mfp = {};
    mfp.bPaused = 0;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

    int sessionid = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);
    REQUIRE(sessionid > 0);

    TTMessage msg;
    while (WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, DEFWAIT) &&
           msg.mediafileinfo.nStatus != MFS_FINISHED);

    // Call TT_InitLocalPlayback for file 1, PAUSE=TRUE (session = X)
    mfp.bPaused = 1;
    sessionid = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);

    // Call TT_InitLocalPlayback for file 2, PAUSE=FALSE
    mfp.bPaused = 0;
    int sessionid2 = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);
    REQUIRE(sessionid2 > 0);
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, DEFWAIT));
    REQUIRE(sessionid2 == msg.nSource);
    REQUIRE(MFS_STARTED == msg.mediafileinfo.nStatus);
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, DEFWAIT));
    REQUIRE(MFS_PLAYING == msg.mediafileinfo.nStatus);
    while (WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, DEFWAIT) &&
           msg.mediafileinfo.nStatus != MFS_FINISHED) {
        REQUIRE(sessionid2 == msg.nSource);
    }

    // Call TT_InitLocalPlayback for file 2, PAUSE=TRUE
    mfp.bPaused = 1;
    sessionid2 = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);

    // Call TT_UpdateLocalPlayback for session X => Crash
    mfp.bPaused = 0;
    REQUIRE(TT_UpdateLocalPlayback(ttclient, sessionid, &mfp));
    while (WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, DEFWAIT)) {
        if (msg.nSource == sessionid && msg.mediafileinfo.nStatus == MFS_FINISHED)
            break;
    }
}

TEST_CASE("LocalPlaybackOnOffSound")
{
    constexpr auto PLAYBACK_DURATION_MSEC = 1000;

    auto ttclient = InitTeamTalk();
    InitSound(ttclient);

    // Call TT_InitLocalPlayback for file 1, PAUSE=FALSE
    MediaFilePlayback mfp = {};
    mfp.bPaused = 0;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    int i = 1;
    while ((i--) != 0)
    {
        int const onid = TT_InitLocalPlayback(ttclient, ACE_TEXT("testdata/Opus/on.ogg"), &mfp);
        REQUIRE(onid > 0);
        WaitForEvent(ttclient, CLIENTEVENT_NONE, PLAYBACK_DURATION_MSEC);
        int const offid = TT_InitLocalPlayback(ttclient, ACE_TEXT("testdata/Opus/off.ogg"), &mfp);
        REQUIRE(offid > 0);
        WaitForEvent(ttclient, CLIENTEVENT_NONE, PLAYBACK_DURATION_MSEC);
    }
}

TEST_CASE("LocalPlaybackOnOffPause")
{

    auto ttclient = InitTeamTalk();
    InitSound(ttclient);

    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TTClient")));
    REQUIRE(JoinRoot(ttclient));

    TTMessage msg;
    MediaFilePlayback mfp = {};
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    
    // cache both sounds for initial playback
    mfp.bPaused = 1;
    int onid = TT_InitLocalPlayback(ttclient, ACE_TEXT("testdata/Opus/on.ogg"), &mfp);
    REQUIRE(onid > 0);

    int offid = TT_InitLocalPlayback(ttclient, ACE_TEXT("testdata/Opus/off.ogg"), &mfp);
    REQUIRE(offid > 0);

    constexpr auto PLAYBACK_ITERATIONS = 20;
    int i = PLAYBACK_ITERATIONS;
    while ((i--) != 0)
    {
        // Play cached on.ogg
        mfp.bPaused = 0;
        REQUIRE(TT_UpdateLocalPlayback(ttclient, onid, &mfp));

        while (WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg) && msg.mediafileinfo.nStatus != MFS_FINISHED);
        REQUIRE(msg.nSource == onid);

        // cache on.ogg for next playback
        mfp.bPaused = 1;
        onid = TT_InitLocalPlayback(ttclient, ACE_TEXT("testdata/Opus/on.ogg"), &mfp);
        REQUIRE(onid > 0);

        constexpr auto CACHE_WAIT_MSEC = 1000;
        WaitForEvent(ttclient, CLIENTEVENT_NONE, msg, CACHE_WAIT_MSEC);

        // Play cached off.ogg
        mfp.bPaused = 0;
        REQUIRE(TT_UpdateLocalPlayback(ttclient, offid, &mfp));

        while (WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg) && msg.mediafileinfo.nStatus != MFS_FINISHED);
        REQUIRE(msg.nSource == offid);

        // cache off.ogg for next playback
        mfp.bPaused = 1;
        offid = TT_InitLocalPlayback(ttclient, ACE_TEXT("testdata/Opus/off.ogg"), &mfp);
        REQUIRE(offid > 0);

        WaitForEvent(ttclient, CLIENTEVENT_NONE, msg, CACHE_WAIT_MSEC);
    }
}

TEST_CASE("FirstVoiceStreamPacket")
{

    auto txclient = InitTeamTalk();
    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(txclient));

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    auto firstvoicepacket = [&](TTMessage msg)
    {
        return msg.nClientEvent == CLIENTEVENT_USER_FIRSTVOICESTREAMPACKET &&
            msg.user.nUserID == TT_GetMyUserID(txclient);
    };

    auto voicestart = [&](TTMessage msg)
    {
        return msg.nClientEvent == CLIENTEVENT_USER_STATECHANGE &&
            msg.user.nUserID == TT_GetMyUserID(txclient) &&
            (msg.user.uUserState & USERSTATE_VOICE) == USERSTATE_VOICE;
    };

    constexpr auto FIXED_DELAY_MSEC = 240;
    constexpr auto MAX_ADAPTIVE_DELAY_MSEC = 10000;
    constexpr auto ACTIVE_ADAPTIVE_DELAY_MSEC = 800;
    //Set fixed Jitter buffer config
    uint32_t const fixeddelay = FIXED_DELAY_MSEC;
    JitterConfig jitterconf = {};
    jitterconf.nFixedDelayMSec = fixeddelay;
    jitterconf.bUseAdativeDejitter = 0;
    jitterconf.nMaxAdaptiveDelayMSec = MAX_ADAPTIVE_DELAY_MSEC;
    jitterconf.nActiveAdaptiveDelayMSec = ACTIVE_ADAPTIVE_DELAY_MSEC;

    TT_SetUserJitterControl(rxclient, TT_GetMyUserID(txclient), STREAMTYPE_VOICE, &jitterconf);

    /************************************************/
    /*  Part one - fixed jitter buffer              */
    /************************************************/

    //start voice
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));

    //Wait for first packet notification
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_FIRSTVOICESTREAMPACKET, firstvoicepacket));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, voicestart));

    //Close voice and wait for it to end
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE));

    /************************************************/
    /* Part Two - fixed jitter buffer but set active adaptive control */
    /************************************************/

    constexpr auto ADAPTIVE_DELAY_MSEC = 760;
    //Set fixed + adaptive Jitter buffer config but don't allow adaptive jitter control
    uint32_t const adaptivedelay = ADAPTIVE_DELAY_MSEC;
    jitterconf.nActiveAdaptiveDelayMSec = adaptivedelay;

    TT_SetUserJitterControl(rxclient, TT_GetMyUserID(txclient), STREAMTYPE_VOICE, &jitterconf);

    //start voice
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    //Wait for first packet notification
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_FIRSTVOICESTREAMPACKET, firstvoicepacket));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, voicestart));

    //Close voice and wait for it to end
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE));

    /************************************************/
    /* Part Three - Allow adaptive jitter buffer    */
    /************************************************/

    //Set fixed + adaptive Jitter buffer config but don't allow adaptive jitter control
    jitterconf.bUseAdativeDejitter = 1;

    TT_SetUserJitterControl(rxclient, TT_GetMyUserID(txclient), STREAMTYPE_VOICE, &jitterconf);

    //start voice
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    //Wait for first packet notification
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_FIRSTVOICESTREAMPACKET, firstvoicepacket));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, voicestart));
}

TEST_CASE("SeeFilesAfterMove")
{

    auto admin = InitTeamTalk();
    REQUIRE(Connect(admin));
    REQUIRE(Login(admin, ACE_TEXT("admin"), ACE_TEXT("admin"), ACE_TEXT("admin")));
    REQUIRE(JoinRoot(admin));

    auto ttclient = InitTeamTalk();
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("guest")));
    REQUIRE(JoinRoot(ttclient));

    AudioCodec codec;
    codec.nCodec = NO_CODEC;
    auto chan = MakeChannel(admin, ACE_TEXT("SeeFilesAfterMove"), TT_GetRootChannelID(admin), codec);
    constexpr auto CHANNEL_MAX_USERS = 100;
    constexpr auto CHANNEL_DISK_QUOTA_BYTES = 1024*1024;
    chan.nMaxUsers = CHANNEL_MAX_USERS;
    chan.nDiskQuota = CHANNEL_DISK_QUOTA_BYTES;
    REQUIRE(WaitForCmdSuccess(admin, TT_DoMakeChannel(admin, &chan)));

    int const chanid = TT_GetChannelIDFromPath(admin, ACE_TEXT("SeeFilesAfterMove"));
    REQUIRE(chanid > 0);

    std::array<TTCHAR, TT_STRLEN> filename{};
    ACE_OS::strsncpy(filename.data(), ACE_TEXT("testdata/Opus/giana.ogg"), filename.size());

    REQUIRE(WaitForCmdSuccess(admin, TT_DoSendFile(admin, chanid, filename.data())));

    REQUIRE(WaitForEvent(admin, CLIENTEVENT_CMD_FILE_NEW));

    REQUIRE(WaitForCmdSuccess(admin, TT_DoMoveUser(admin, TT_GetMyUserID(ttclient), chanid)));

    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_CMD_FILE_NEW));
}

#if defined(ENABLE_OPUSTOOLS) && defined(ENABLE_OPUS)

static void CreateOpusFile(const MediaFileInfo& mfi, const ACE_TString& oggfilename, const int FRAMESIZE)
{
    constexpr auto TONE_HZ = 500;

    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, TONE_HZ));

    WavePCMFile wavfile;
    REQUIRE(wavfile.OpenFile(mfi.szFileName, true));

    OpusEncFile opusenc;
    REQUIRE(opusenc.Open(oggfilename, mfi.audioFmt.nChannels, mfi.audioFmt.nSampleRate, FRAMESIZE, OPUS_APPLICATION_AUDIO));

    std::vector<short> buf(size_t(mfi.audioFmt.nChannels) * FRAMESIZE);
    int samples = 0;
    while ((samples = wavfile.ReadSamples(buf.data(), FRAMESIZE)) > 0)
    {
        REQUIRE(opusenc.Encode(buf.data(), FRAMESIZE, samples != FRAMESIZE) >= 0);
    }
    opusenc.Close();
    wavfile.Close();
}

TEST_CASE("OPUSFileEncDec")
{
    constexpr auto CHANNELS = 2;
    constexpr auto MEDIA_FILE_DURATION_MSEC = 10 * 1000;

    for (auto SAMPLERATE : {8000, 12000, 24000, 48000})
    {
#if defined(OPUS_FRAMESIZE_120_MS)
        for (auto FRAMESIZE_SEC : {.0025, .005, .01, .02, .04, .06, .08, .1, .12})
#else
        for (auto FRAMESIZE_SEC : {.0025, .005, .01, .02, .04, .06})
#endif
        {
            MediaFileInfo mfi = {};
            mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
            mfi.audioFmt.nChannels = CHANNELS;
            mfi.audioFmt.nSampleRate = SAMPLERATE;
            mfi.uDurationMSec = MEDIA_FILE_DURATION_MSEC;
            const int FRAMESIZE = int(mfi.audioFmt.nSampleRate * FRAMESIZE_SEC);
            ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("originalfile_%d_%dmsec.wav"),
                             mfi.audioFmt.nSampleRate, PCM16_SAMPLES_DURATION(FRAMESIZE, mfi.audioFmt.nSampleRate));

            constexpr auto TONE_HZ = 500;
            REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, TONE_HZ));

            WavePCMFile wavfile;
            REQUIRE(wavfile.OpenFile(mfi.szFileName, true));

            OpusEncFile opusenc;
            std::array<ACE_TCHAR, TT_STRLEN> opusencfilename{};
            ACE_OS::snprintf(opusencfilename.data(), opusencfilename.size(), ACE_TEXT("opusencfile_%d_%dmsec.ogg"),
                             mfi.audioFmt.nSampleRate, PCM16_SAMPLES_DURATION(FRAMESIZE, mfi.audioFmt.nSampleRate));
            REQUIRE(opusenc.Open(opusencfilename.data(), mfi.audioFmt.nChannels, mfi.audioFmt.nSampleRate, FRAMESIZE, OPUS_APPLICATION_AUDIO));

            std::vector<short> buf(size_t(mfi.audioFmt.nChannels) * FRAMESIZE);
            int samples = 0;
            while ((samples = wavfile.ReadSamples(buf.data(), FRAMESIZE)) > 0)
            {
                REQUIRE(opusenc.Encode(buf.data(), FRAMESIZE, samples != FRAMESIZE) >= 0);
            }
            opusenc.Close();
            wavfile.Close();

            OpusFile opusread;
            REQUIRE(opusread.OpenFile(opusencfilename.data()));

            OpusDecode opusdec;
            REQUIRE(opusdec.Open(opusread.GetSampleRate(), opusread.GetChannels()));

            std::array<ACE_TCHAR, TT_STRLEN> opusdecfilename{};
            ACE_OS::snprintf(opusdecfilename.data(), opusdecfilename.size(), ACE_TEXT("opusdecfile_%d_%dmsec.wav"),
                             mfi.audioFmt.nSampleRate, PCM16_SAMPLES_DURATION(FRAMESIZE, mfi.audioFmt.nSampleRate));
            REQUIRE(wavfile.NewFile(opusdecfilename.data(), opusread.GetSampleRate(), opusread.GetChannels()));

            ogg_int64_t samplesduration = 0;
            while (true)
            {
                int bytes = 0;
                const auto *opusbuf = opusread.ReadEncoded(bytes, &samplesduration);
                if (opusbuf == nullptr)
                    break;

                REQUIRE(opusdec.Decode(reinterpret_cast<const char*>(opusbuf), bytes, buf.data(), FRAMESIZE) == FRAMESIZE);
                wavfile.AppendSamples(buf.data(), FRAMESIZE);
            }

            auto durationmsec = PCM16_SAMPLES_DURATION(samplesduration, mfi.audioFmt.nSampleRate);
            // std::cout << SAMPLERATE << "@" << FRAMESIZE_SEC << " diff: " << mfi.uDurationMSec - durationmsec << std::endl;
            REQUIRE(std::abs(durationmsec - (int)mfi.uDurationMSec) <= FRAMESIZE_SEC * 1000);
        }
    }
}

TEST_CASE("OPUSFileSeek")
{
    constexpr auto SAMPLERATE = 12000;
    constexpr auto FRAMESIZE_SEC = .04;
    constexpr auto CHANNELS = 2;
    constexpr auto MEDIA_FILE_DURATION_MSEC = 10 * 1000;

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = CHANNELS;
    mfi.audioFmt.nSampleRate = SAMPLERATE;
    mfi.uDurationMSec = MEDIA_FILE_DURATION_MSEC;

    const int FRAMESIZE = int(mfi.audioFmt.nSampleRate * FRAMESIZE_SEC);
    ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("orgseekfile_%d_%dmsec.wav"),
                     mfi.audioFmt.nSampleRate, PCM16_SAMPLES_DURATION(FRAMESIZE, mfi.audioFmt.nSampleRate));

    std::array<ACE_TCHAR, TT_STRLEN> opusencfilename{};
    ACE_OS::snprintf(opusencfilename.data(), opusencfilename.size(), ACE_TEXT("opusseekfile_%d_%dmsec.ogg"),
                     mfi.audioFmt.nSampleRate, PCM16_SAMPLES_DURATION(FRAMESIZE, mfi.audioFmt.nSampleRate));

    CreateOpusFile(mfi, opusencfilename.data(), FRAMESIZE);

    // ensure we come back to file's origin after search
    OggFile of;
    OggFile of1;
    ogg_page og;
    ogg_page og1;
    REQUIRE(of.Open(opusencfilename.data()));
    REQUIRE(of1.Open(opusencfilename.data()));
    constexpr auto PAGE_READ_COUNT = 10;
    int count = PAGE_READ_COUNT;
    while((count--) != 0)
    {
        REQUIRE(of.ReadOggPage(og) == 1);
        REQUIRE(of1.ReadOggPage(og1) == 1);
    }
    constexpr auto OPUS_GRANULE_SAMPLE_RATE_HZ = 48000;
    REQUIRE(of.LastGranulePos() == OPUS_GRANULE_SAMPLE_RATE_HZ * mfi.uDurationMSec / 1000);
    REQUIRE(of.ReadOggPage(og) == 1);
    REQUIRE(of1.ReadOggPage(og1) == 1);
    REQUIRE(ogg_page_granulepos(&og) == ogg_page_granulepos(&og1));

    // validate frame-size and total number of samples
    OpusFile opfile;
    REQUIRE(opfile.OpenFile(opusencfilename.data()));
    REQUIRE(PCM16_SAMPLES_DURATION(FRAMESIZE, opfile.GetSampleRate()) == 1000 * FRAMESIZE_SEC);
    double const duration_sec = mfi.uDurationMSec / 1000.;
    REQUIRE(opfile.GetTotalSamples() == SAMPLERATE * duration_sec);

    auto const halfsamples = ogg_int64_t((duration_sec * mfi.audioFmt.nSampleRate) / 2);
    REQUIRE(opfile.Seek(halfsamples));
    ogg_int64_t samplesduration = 0;
    int bytes = 0;
    REQUIRE(opfile.ReadEncoded(bytes, &samplesduration));
    REQUIRE(std::abs(halfsamples - samplesduration) <= FRAMESIZE);

    constexpr auto SEEK_FRAMES = 10;
    // seek using log2
    REQUIRE(of.Seek((FRAMESIZE * SEEK_FRAMES) * (OPUS_GRANULE_SAMPLE_RATE_HZ / SAMPLERATE), og));
    REQUIRE(of.CurrentGranulePos() / (OPUS_GRANULE_SAMPLE_RATE_HZ / SAMPLERATE) == FRAMESIZE * SEEK_FRAMES);
    REQUIRE(of1.SeekLog2((FRAMESIZE * SEEK_FRAMES) * (OPUS_GRANULE_SAMPLE_RATE_HZ / SAMPLERATE), og));
    REQUIRE(of1.CurrentGranulePos() / (OPUS_GRANULE_SAMPLE_RATE_HZ / SAMPLERATE) == FRAMESIZE * SEEK_FRAMES);
    REQUIRE(of.LastGranulePos() == of1.LastGranulePosLog2());
    REQUIRE(of.Seek(((FRAMESIZE * SEEK_FRAMES) + FRAMESIZE / 3) * (OPUS_GRANULE_SAMPLE_RATE_HZ / SAMPLERATE), og));
    REQUIRE(of1.SeekLog2(((FRAMESIZE * SEEK_FRAMES) + FRAMESIZE / 3) * (OPUS_GRANULE_SAMPLE_RATE_HZ / SAMPLERATE), og));
    REQUIRE(of.LastGranulePos() == of1.LastGranulePosLog2());
    std::vector<uint32_t> const seekpositions = { uint32_t(mfi.uDurationMSec * 0.31),
                                            uint32_t(mfi.uDurationMSec * 0.0),
                                            uint32_t(mfi.uDurationMSec * 1.0),
                                            uint32_t(mfi.uDurationMSec * 0.777),
                                            uint32_t(mfi.uDurationMSec * 0.333),
                                            uint32_t(mfi.uDurationMSec * 1.0),
                                            uint32_t(mfi.uDurationMSec * 0.0)};

    for (uint32_t const sp : seekpositions)
    {
        REQUIRE(of.Seek(OPUS_GRANULE_SAMPLE_RATE_HZ * sp / 1000, og));
        REQUIRE(of1.SeekLog2(OPUS_GRANULE_SAMPLE_RATE_HZ * sp / 1000, og));
        REQUIRE(of.LastGranulePos() == of1.LastGranulePosLog2());
    }

    // decode from 0% onwards
    OpusDecFile opusdecfile;
    REQUIRE(opusdecfile.Open(opusencfilename.data()));
    std::vector<short> frame(FRAMESIZE * CHANNELS);
    int frames = 0;
    while (opusdecfile.Decode(frame.data(), FRAMESIZE) == FRAMESIZE)frames++;
    uint32_t duration_msec = PCM16_SAMPLES_DURATION(frames * FRAMESIZE, SAMPLERATE);
    REQUIRE(duration_msec == mfi.uDurationMSec);
    REQUIRE(opusdecfile.GetDurationMSec() == mfi.uDurationMSec);
    opusdecfile.Close();

    // decode from 90% onwards
    REQUIRE(opusdecfile.Open(opusencfilename.data()));
    constexpr auto SEEK_OFFSET_FRACTION = .9;
    auto offset_msec = SEEK_OFFSET_FRACTION * mfi.uDurationMSec;
    REQUIRE(opusdecfile.Seek(uint32_t(offset_msec)));
    frames = 0;
    while (opusdecfile.Decode(frame.data(), FRAMESIZE) == FRAMESIZE)frames++;
    duration_msec = PCM16_SAMPLES_DURATION(frames * FRAMESIZE, SAMPLERATE);
    REQUIRE(duration_msec == mfi.uDurationMSec - offset_msec);
    REQUIRE(opusdecfile.GetDurationMSec() == mfi.uDurationMSec);

    // check special handling of seeking to granule position 0 in Ogg file
    REQUIRE(opusdecfile.Seek(0));
    frames = 0;
    while (opusdecfile.Decode(frame.data(), FRAMESIZE) == FRAMESIZE)frames++;
    duration_msec = PCM16_SAMPLES_DURATION(frames * FRAMESIZE, SAMPLERATE);
    REQUIRE(duration_msec == mfi.uDurationMSec);
    REQUIRE(opusdecfile.GetDurationMSec() == mfi.uDurationMSec);

    constexpr auto SEEK_OFFSET_MSEC = 555;
    // check seek offsets
    REQUIRE(opusdecfile.Seek(SEEK_OFFSET_MSEC));
    auto diffMSec = opusdecfile.GetElapsedMSec() - ((SEEK_OFFSET_MSEC / int(FRAMESIZE_SEC * 1000)) * int(FRAMESIZE_SEC * 1000));
    REQUIRE(std::abs(int(diffMSec)) <= PCM16_SAMPLES_DURATION(FRAMESIZE, SAMPLERATE));
    REQUIRE(opusdecfile.Seek(0));
    REQUIRE(opusdecfile.GetElapsedMSec() == 0);

    // check decode output after initial duration check
    opusdecfile.Close();
    REQUIRE(opusdecfile.Open(opusencfilename.data()));
    REQUIRE(opusdecfile.GetDurationMSec() == mfi.uDurationMSec);
    frames = 0;
    while (opusdecfile.Decode(frame.data(), FRAMESIZE) == FRAMESIZE)frames++;
    duration_msec = PCM16_SAMPLES_DURATION(frames * FRAMESIZE, SAMPLERATE);
    REQUIRE(duration_msec == mfi.uDurationMSec);
    REQUIRE(opusdecfile.GetDurationMSec() == mfi.uDurationMSec);
}

TEST_CASE("OPUSStreamer")
{
    constexpr auto IN_SAMPLERATE = 12000;
    constexpr auto IN_FRAMESIZE_SEC = .04;
    constexpr auto IN_CHANNELS = 2;
    constexpr auto MEDIA_FILE_DURATION_MSEC = 3 * 1000;

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = IN_CHANNELS;
    mfi.audioFmt.nSampleRate = IN_SAMPLERATE;
    mfi.uDurationMSec = MEDIA_FILE_DURATION_MSEC;

    const int IN_FRAMESIZE = int(mfi.audioFmt.nSampleRate * IN_FRAMESIZE_SEC);
    ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("opusstreamer_inputfile_%d_%dmsec.wav"),
                     mfi.audioFmt.nSampleRate, PCM16_SAMPLES_DURATION(IN_FRAMESIZE, mfi.audioFmt.nSampleRate));
    std::array<ACE_TCHAR, TT_STRLEN> opusencfilename{};
    ACE_OS::snprintf(opusencfilename.data(), opusencfilename.size(), ACE_TEXT("opusstreamer_inputfile_%d_%dmsec.ogg"),
                     mfi.audioFmt.nSampleRate, PCM16_SAMPLES_DURATION(IN_FRAMESIZE, mfi.audioFmt.nSampleRate));
    CreateOpusFile(mfi, opusencfilename.data(), IN_FRAMESIZE);

    constexpr auto OUT_SAMPLERATE = 48000;
    constexpr auto OUT_CHANNELS = 1;
    constexpr auto OUT_FRAMESIZE = int(OUT_SAMPLERATE * .06);

    std::array<ACE_TCHAR, TT_STRLEN> opusstreamerfilename{};
    ACE_OS::snprintf(opusstreamerfilename.data(), opusstreamerfilename.size(), ACE_TEXT("opusstreamer_outputfile_%d_%dmsec.wav"),
                     OUT_SAMPLERATE, PCM16_SAMPLES_DURATION(OUT_FRAMESIZE, OUT_SAMPLERATE));

    WavePCMFile wavfile;
    REQUIRE(wavfile.NewFile(opusstreamerfilename.data(), OUT_SAMPLERATE, OUT_CHANNELS));

    MediaStreamOutput const mso(media::AudioFormat(OUT_SAMPLERATE, OUT_CHANNELS), OUT_FRAMESIZE);

    std::condition_variable cv_finished;
    std::condition_variable cv_started;
    std::condition_variable cv_paused;
    std::condition_variable cv_playing;
    std::condition_variable cv_error;
    std::mutex mtx;

    auto statusfunc = [&](const MediaFileProp& mfp, MediaStreamStatus status)
    {
        switch (status)
        {
        case MEDIASTREAM_STARTED :
            cv_started.notify_all();
            MYTRACE(ACE_TEXT("Started\n"));
            break;
        case MEDIASTREAM_ERROR :
            REQUIRE(false);
            cv_error.notify_all();
            break;
        case MEDIASTREAM_PLAYING :
            cv_playing.notify_all();
            MYTRACE(ACE_TEXT("Playing %u msec\n"), mfp.elapsed_ms);
            break;
        case MEDIASTREAM_PAUSED :
            REQUIRE(false);
            cv_paused.notify_all();
            break;
        case MEDIASTREAM_FINISHED :
            cv_finished.notify_all();
            MYTRACE(ACE_TEXT("Finished %u msec\n"), mfp.elapsed_ms);
            break;
        default :
            REQUIRE(false);
            break;
        }
    };

    auto audiofunc = [&](media::AudioFrame& audio_frame, ACE_Message_Block* /*mb_audio*/)
    {
        wavfile.AppendSamples(audio_frame.input_buffer, audio_frame.input_samples);
        return false;
    };

    std::unique_lock<std::mutex> lck(mtx);

    std::unique_ptr<OpusFileStreamer> ofs;
    ofs = std::make_unique<OpusFileStreamer>(opusencfilename.data(), mso);
    ofs->RegisterStatusCallback(statusfunc, true);
    ofs->RegisterAudioCallback(audiofunc, true);
    REQUIRE(ofs->Open());
    REQUIRE(ofs->StartStream());
    constexpr auto FINISHED_WAIT_MULTIPLIER = 2;
    REQUIRE(cv_finished.wait_for(lck, std::chrono::milliseconds(mfi.uDurationMSec * FINISHED_WAIT_MULTIPLIER)) == std::cv_status::no_timeout);
}

TEST_CASE("TTPlayOpusOgg")
{
    constexpr auto IN_SAMPLERATE = 48000;
    constexpr auto IN_CHANNELS = 2;
    constexpr auto IN_FRAMESIZE = int(IN_SAMPLERATE * .01);
    constexpr auto MEDIA_FILE_DURATION_MSEC = 5 * 1000;

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = IN_CHANNELS;
    mfi.audioFmt.nSampleRate = IN_SAMPLERATE;
    mfi.uDurationMSec = MEDIA_FILE_DURATION_MSEC;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("TTPlayOpusOgg.wav"), TT_STRLEN);

    std::array<TTCHAR, TT_STRLEN> szFilename{};
    ACE_OS::strsncpy(szFilename.data(), ACE_TEXT("TTPlayOpusOgg.ogg"), szFilename.size());
    CreateOpusFile(mfi, szFilename.data(), IN_FRAMESIZE);

    TTInstPtr ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
    mfp.bPaused = FALSE;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

    bool stop = false;
    bool started = false;
    bool paused = false;
    TTMessage msg;
    INT32 session = 0;
    uint32_t durationMSec = 0;

    // test duration of OpusFileStreamer playback
    session = TT_InitLocalPlayback(ttclient, szFilename.data(), &mfp);
    REQUIRE(session > 0);

    while (!stop && WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, DEFWAIT))
    {
        switch(msg.mediafileinfo.nStatus)
        {
        case MFS_STARTED :
            REQUIRE(msg.mediafileinfo.uElapsedMSec == 0);
            REQUIRE(!started);
            started = true;
            break;
        case MFS_PLAYING :
            break;
        case MFS_FINISHED :
            REQUIRE(!stop);
            stop = true;
            break;
        default :
            break;
        }
    }
    REQUIRE(started);
    REQUIRE(stop);

    // test seek-feature of OpusFileStreamer playback
    mfp.uOffsetMSec = mfi.uDurationMSec / 2;
    session = TT_InitLocalPlayback(ttclient, szFilename.data(), &mfp);
    REQUIRE(session > 0);
    durationMSec = GETTIMESTAMP();
    stop = false;
    started = false;
    while (!stop && WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, DEFWAIT))
    {
        switch(msg.mediafileinfo.nStatus)
        {
        case MFS_STARTED :
            REQUIRE(std::abs(int(msg.mediafileinfo.uElapsedMSec - mfp.uOffsetMSec)) <= PCM16_SAMPLES_DURATION(IN_FRAMESIZE, IN_SAMPLERATE));
            REQUIRE(!started);
            started = true;
            break;
        case MFS_PLAYING :
            break;
        case MFS_FINISHED :
            REQUIRE(!stop);
            stop = true;
            break;
        default :
            break;
        }
    }
    REQUIRE(started);
    REQUIRE(stop);
    durationMSec = GETTIMESTAMP() - durationMSec;
    // precision reduced due to GitHub CI being slow
    REQUIRE(int(durationMSec) < int(mfi.uDurationMSec));

    // test pause-feature of OpusFileStreamer playback
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    session = TT_InitLocalPlayback(ttclient, szFilename.data(), &mfp);
    REQUIRE(session > 0);
    stop = false;
    started = false;
    paused = false;
    durationMSec = GETTIMESTAMP();
    constexpr int PAUSE_DURATION_MSEC = 1000;
    while (!stop && WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, DEFWAIT))
    {
        switch(msg.mediafileinfo.nStatus)
        {
        case MFS_STARTED :
            REQUIRE(!started);
            started = true;
            break;
        case MFS_PLAYING :
            if (!paused && msg.mediafileinfo.uElapsedMSec >= mfi.uDurationMSec / 2)
            {
                mfp.bPaused = TRUE;
                REQUIRE(TT_UpdateLocalPlayback(ttclient, session, &mfp));
            }
            break;
        case MFS_PAUSED :
            REQUIRE(!paused);
            paused = true;
            WaitForEvent(ttclient, CLIENTEVENT_NONE, msg, PAUSE_DURATION_MSEC);
            mfp.bPaused = FALSE;
            REQUIRE(TT_UpdateLocalPlayback(ttclient, session, &mfp));
            started = false;
            break;
        case MFS_FINISHED :
            REQUIRE(!stop);
            stop = true;
            break;
        default :
            break;
        }
    }
    REQUIRE(started);
    REQUIRE(stop);
    REQUIRE(paused);
    durationMSec = GETTIMESTAMP() - durationMSec;
    // precision reduced due to GitHub CI being slow
    constexpr int DURATION_TOLERANCE_MSEC = 500;
    REQUIRE(int(durationMSec) >= int(mfi.uDurationMSec + PAUSE_DURATION_MSEC - PAUSE_DURATION_MSEC));
}

TEST_CASE("TTPlayFFmpegOpus")
{
    std::array<TTCHAR, TT_STRLEN> filename{};
    ACE_OS::strsncpy(filename.data(), ACE_TEXT("testdata/Opus/giana.ogg"), filename.size());
    OpusDecFile odf;
    REQUIRE(odf.Open(filename.data()));

    WavePCMFile wavfile;
    REQUIRE(wavfile.NewFile(ACE_TEXT("giana.wav"), odf.GetSampleRate(), odf.GetChannels()));
    std::vector<short> buf(size_t(odf.GetSampleRate()) * odf.GetChannels());
    int samples;
    int framesize = 0;
    while ((samples = odf.Decode(buf.data(), odf.GetSampleRate())) > 0)
    {
        wavfile.AppendSamples(buf.data(), samples);
        REQUIRE((framesize == 0 || framesize == samples)); // assume same framesize
        framesize = samples;
    }
    wavfile.Close();

    MediaFileInfo mfi;
    REQUIRE(TT_GetMediaFileInfo(ACE_TEXT("giana.wav"), &mfi));
    REQUIRE(std::abs(int(mfi.uDurationMSec - odf.GetDurationMSec())) < PCM16_SAMPLES_DURATION(framesize, odf.GetSampleRate()));
    REQUIRE(TT_GetMediaFileInfo(filename.data(), &mfi));
    REQUIRE(mfi.uDurationMSec == odf.GetDurationMSec());
}
#endif

TEST_CASE("SeekPrecision")
{

    std::array<TTCHAR, TT_STRLEN> filename{};
    ACE_OS::strsncpy(filename.data(), ACE_TEXT("testdata/Opus/giana.ogg"), filename.size());

    auto ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));

    MediaFilePlayback mfp = {};
    mfp.bPaused = FALSE;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

    auto session = TT_InitLocalPlayback(ttclient, filename.data(), &mfp);
    REQUIRE(session > 0);

    TTMessage msg;
    bool stop = false;
    while (!stop)
    {
        constexpr auto PAUSE_ELAPSED_MSEC = 2440;
        constexpr auto SEEK_OFFSET_MSEC = 988;

        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
        switch (msg.mediafileinfo.nStatus)
        {
        case MFS_PLAYING :
            // std::cout << "Playing at " << msg.mediafileinfo.uElapsedMSec << std::endl;
            if (msg.mediafileinfo.uElapsedMSec >= PAUSE_ELAPSED_MSEC)
            {
                mfp.bPaused = TRUE;
                REQUIRE(TT_UpdateLocalPlayback(ttclient, session, &mfp));
            }
            break;
        case MFS_PAUSED :
            mfp.bPaused = FALSE;
            mfp.uOffsetMSec = SEEK_OFFSET_MSEC;
            REQUIRE(TT_UpdateLocalPlayback(ttclient, session, &mfp));
            stop = true;
            // std::cout << "Paused at " << msg.mediafileinfo.uElapsedMSec << std::endl;
            break;
        default :
            break;
        }
    }

    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
    REQUIRE(msg.mediafileinfo.nStatus == MFS_STARTED);
    constexpr auto MFS_STARTED_TOLERANCE_MSEC = 120;
    constexpr auto MFS_PLAYING_TOLERANCE_MSEC = 240;
    constexpr auto MFS_PLAYING_MAX_TOLERANCE_MSEC = 360;
    REQUIRE(std::abs(int32_t(msg.mediafileinfo.uElapsedMSec - mfp.uOffsetMSec)) <= MFS_STARTED_TOLERANCE_MSEC);
    // std::cout << "Started at " << msg.mediafileinfo.uElapsedMSec << std::endl;
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
    REQUIRE(msg.mediafileinfo.nStatus == MFS_PLAYING);
    REQUIRE(std::abs(int32_t(msg.mediafileinfo.uElapsedMSec - mfp.uOffsetMSec)) <= MFS_PLAYING_TOLERANCE_MSEC);
    // std::cout << "Playing at " << msg.mediafileinfo.uElapsedMSec << std::endl;
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
    REQUIRE(msg.mediafileinfo.nStatus == MFS_PLAYING);
    REQUIRE(std::abs(int32_t(msg.mediafileinfo.uElapsedMSec - mfp.uOffsetMSec)) <= MFS_PLAYING_MAX_TOLERANCE_MSEC);
    // std::cout << "Playing at " << msg.mediafileinfo.uElapsedMSec << std::endl;
}

TEST_CASE("TimeConvert")
{
    auto tv = ACE_OS::gettimeofday();
    ACE_TString line = ACE_TEXT("foo ");
    teamtalk::AppendProperty(ACE_TEXT("tm"), tv, line);
    line += EOL;
    teamtalk::mstrings_t props;
    teamtalk::ExtractProperties(line, props);
    ACE_Time_Value tv2;
    REQUIRE(teamtalk::GetProperty(props, ACE_TEXT("tm"), tv2));
    REQUIRE(tv.sec() == tv2.sec());
}


TEST_CASE("FileIO")
{
    constexpr auto CHUNK_SIZE = 10;

    {
        MyFile f;
        REQUIRE(f.NewFile(ACE_TEXT("file.foo")));

        for (char c = 'A'; c <= 'z'; ++c)
            REQUIRE(f.Write(&c, 1) == 1);
    }
    {
        MyFile f;
        REQUIRE(f.Open(ACE_TEXT("file.foo")));

        for (char c = 'A'; c <= 'z'; ++c)
        {
            char t = 0;
            REQUIRE(f.Read(&t, 1));
            REQUIRE(t == c);
        }
    }

    constexpr auto FILE_BUFFER_SIZE = 256;
    std::array<char, FILE_BUFFER_SIZE> buf_myfile{};
    std::array<char, FILE_BUFFER_SIZE> buf_ace{};
    int i = 0;
    for (char c = 'A'; c <= 'z'; ++c, i++)
    {
        buf_myfile[i] = c;
        buf_ace[i] = c;
    }
    auto *const ORG_BUF = buf_myfile.data();

    {
        MyFile f;
        ACE_FILE_IO file_ace;
        ACE_FILE_Connector con;
        REQUIRE(f.NewFile(ACE_TEXT("file.foo")));
        REQUIRE(f.Write(ORG_BUF, i));
        // do same with ACE_FILE_IO
        REQUIRE(con.connect(file_ace, ACE_FILE_Addr(ACE_TEXT("file.ace")), nullptr, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT | O_TRUNC | O_BINARY) >= 0);
        REQUIRE(file_ace.send(ORG_BUF, i));

        std::array<char, CHUNK_SIZE> buf2{};
        REQUIRE(f.Seek(CHUNK_SIZE, std::ios_base::beg));  // now index 10
        REQUIRE(f.Tell() == CHUNK_SIZE);
        REQUIRE(f.Read(buf2.data(), CHUNK_SIZE) == CHUNK_SIZE); // now index 20
        REQUIRE(memcmp(&ORG_BUF[CHUNK_SIZE], buf2.data(), CHUNK_SIZE) == 0);
        REQUIRE(f.Tell() == CHUNK_SIZE * 2);
        memset(buf2.data(), 0, buf2.size());
        // do same with ACE_FILE_IO
        REQUIRE(file_ace.seek(CHUNK_SIZE, SEEK_SET) >= 0);
        REQUIRE(file_ace.recv(buf2.data(), CHUNK_SIZE) == CHUNK_SIZE);
        REQUIRE(memcmp(&ORG_BUF[CHUNK_SIZE], buf2.data(), CHUNK_SIZE) == 0);
        REQUIRE(file_ace.tell() == CHUNK_SIZE * 2);
        memset(buf2.data(), 0, buf2.size());

        for (char & i : buf2)
            i = '%';

        REQUIRE(f.Seek(CHUNK_SIZE, std::ios_base::cur)); // now index 30
        REQUIRE(f.Tell() == CHUNK_SIZE * 3);
        REQUIRE(f.Write(buf2.data(), CHUNK_SIZE) == CHUNK_SIZE); // now index 40
        REQUIRE(f.Tell() == CHUNK_SIZE * 4);
        // do same with ACE_FILE_IO
        REQUIRE(file_ace.seek(CHUNK_SIZE, SEEK_CUR) >= 0);
        REQUIRE(file_ace.tell() == CHUNK_SIZE * 3);
        REQUIRE(file_ace.send(buf2.data(), CHUNK_SIZE) == CHUNK_SIZE);
        REQUIRE(file_ace.tell() == CHUNK_SIZE * 4);

        REQUIRE(f.Seek(0, std::ios_base::beg));
        memset(buf_myfile.data(), 0, buf_myfile.size());
        REQUIRE(f.Read(buf_myfile.data(), buf_myfile.size()) == i);
        REQUIRE(memcmp(buf_myfile.data(), ORG_BUF, CHUNK_SIZE * 2) == 0);
        REQUIRE(memcmp(&buf_myfile[CHUNK_SIZE * 3], buf2.data(), CHUNK_SIZE) == 0);
        // do same with ACE_FILE_IO
        REQUIRE(file_ace.seek(0, SEEK_SET) >= 0);
        memset(buf_ace.data(), 0, buf_ace.size());
        REQUIRE(file_ace.recv(buf_ace.data(), buf_ace.size()) == i);
        REQUIRE(memcmp(buf_ace.data(), ORG_BUF, CHUNK_SIZE * 2) == 0);
        REQUIRE(memcmp(&buf_ace[CHUNK_SIZE * 3], buf2.data(), CHUNK_SIZE) == 0);

        for (char & i : buf2)
            i = '@';
        REQUIRE(f.Seek(CHUNK_SIZE, std::ios_base::end));
        REQUIRE(f.Write(buf2.data(), CHUNK_SIZE) == CHUNK_SIZE);
        REQUIRE(f.Read(buf_myfile.data(), CHUNK_SIZE) == 0);
        // do same with ACE_FILE_IO
        REQUIRE(file_ace.seek(CHUNK_SIZE, SEEK_END) >= 0);
        REQUIRE(file_ace.send(buf2.data(), CHUNK_SIZE) == CHUNK_SIZE);
        REQUIRE(file_ace.recv(buf_ace.data(), CHUNK_SIZE) == 0);
    }

    {
        MyFile f;
        ACE_FILE_IO file_ace;
        ACE_FILE_Connector con;

        memset(buf_myfile.data(), 0, buf_myfile.size());
        memset(buf_ace.data(), 0, buf_ace.size());

        REQUIRE(f.Open(ACE_TEXT("file.foo"), true));
        REQUIRE(con.connect(file_ace, ACE_FILE_Addr(ACE_TEXT("file.ace")), nullptr, ACE_Addr::sap_any, 0, O_RDONLY | O_BINARY) >= 0);

        REQUIRE(f.Read(buf_myfile.data(), buf_myfile.size()) > 0);
        REQUIRE(file_ace.recv(buf_ace.data(), buf_ace.size()) > 0);
        REQUIRE(f.Tell() == file_ace.tell());
        for (std::size_t i=0;i<buf_ace.size();i++)
            REQUIRE(buf_myfile[i] == buf_ace[i]);

        REQUIRE(f.Seek(-5, std::ios_base::end));
        REQUIRE(file_ace.seek(-5, SEEK_END) >= 0);
        REQUIRE(f.Tell() == file_ace.tell());

        REQUIRE(f.Read(buf_myfile.data(), buf_myfile.size()) > 0);
#if defined(WIN32)
        if (sizeof(void*) == 4)
        {
            // Bug in ACE_FILE_IO
            REQUIRE(file_ace.recv(buf_ace.data(), buf_ace.size()) == 0);
            REQUIRE(f.Tell() != file_ace.tell());
        }
        else
#endif
        {
            REQUIRE(file_ace.recv(buf_ace.data(), buf_ace.size()) > 0);
            REQUIRE(f.Tell() == file_ace.tell());
        }
    }

    {
        MyFile f;
        ACE_FILE_IO file_ace;
        ACE_FILE_Connector con;

        REQUIRE(f.Open(ACE_TEXT("file.foo"), false));
        REQUIRE(f.Seek(CHUNK_SIZE, std::ios_base::cur));
        REQUIRE(f.Tell() == CHUNK_SIZE);
        // do same with ACE_FILE_IO
        REQUIRE(con.connect(file_ace, ACE_FILE_Addr(ACE_TEXT("file.ace")), nullptr, ACE_Addr::sap_any, 0, O_RDWR | O_BINARY) >= 0);
        REQUIRE(file_ace.seek(CHUNK_SIZE, SEEK_CUR));
        REQUIRE(file_ace.tell() == CHUNK_SIZE);

        REQUIRE(f.Write(ORG_BUF, i) == i);
        REQUIRE(f.Read(buf_myfile.data(), 1) == 1);
        // do same with ACE_FILE_IO
        REQUIRE(file_ace.send(ORG_BUF, i) == i);
        REQUIRE(file_ace.recv(buf_ace.data(), 1) == 1);
    }
}

TEST_CASE( "TextMessageStrLength" )
{
    auto client = InitTeamTalk();

    REQUIRE(Connect(client));
    REQUIRE(Login(client, ACE_TEXT("TxClient")));

    TextMessage txtmsg = {};
    txtmsg.nMsgType = MSGTYPE_USER;
    txtmsg.nToUserID = TT_GetMyUserID(client);

    constexpr auto CHAR_PATTERN_MODULO = 10;

    for (size_t i=0;i<TT_STRLEN-1;++i)
        txtmsg.szMessage[i] = TTCHAR('A') + (i % CHAR_PATTERN_MODULO);
    REQUIRE(TT_DoTextMessage(client, &txtmsg) >= 0);
    TTMessage msg;
    REQUIRE(WaitForEvent(client, CLIENTEVENT_CMD_USER_TEXTMSG, msg));
    REQUIRE(ACE_OS::strlen(msg.textmessage.szMessage) == TT_STRLEN-1);
    for (size_t i=0;i<TT_STRLEN;++i)
        REQUIRE(txtmsg.szMessage[i] == msg.textmessage.szMessage[i]);

    // text max string length for non-UTF-8
    ACE_TString str = txtmsg.szMessage;
    REQUIRE(TT_DoChangeNickname(client, str.c_str()) > 0);
    REQUIRE(WaitForEvent(client, CLIENTEVENT_CMD_USER_UPDATE, msg));
    REQUIRE(ACE_TString(msg.user.szNickname) == str);

    // text exceeded max string for non-UTF-8
    for (size_t i=0;i<TT_STRLEN-1;++i)
        str += TTCHAR('A') + (i % CHAR_PATTERN_MODULO);
    REQUIRE(TT_DoChangeNickname(client, str.c_str()) > 0);
    REQUIRE(WaitForEvent(client, CLIENTEVENT_CMD_USER_UPDATE, msg));
    str = str.substr(0, TT_STRLEN-1);
    REQUIRE(ACE_TString(msg.user.szNickname) == str);

    // ensure UTF-8 string is cut correctly
    str = txtmsg.szMessage;
    str = str.substr(0, str.length() - 1);
    size_t const orgsize = str.length();
    str += ACE_TEXT("🔥");
    REQUIRE(TT_DoChangeNickname(client, str.c_str()) > 0);
    REQUIRE(WaitForEvent(client, CLIENTEVENT_CMD_USER_UPDATE, msg));
    str = str.substr(0, orgsize);
    REQUIRE(ACE_TString(msg.user.szNickname) == str);
}

TEST_CASE( "TimeZoneConversion" )
{
    ACE_Time_Value const now = ACE_OS::gettimeofday();
    ACE_TString command = ACE_TEXT("foo ");
    teamtalk::AppendProperty(ACE_TEXT("tm"), now, command);

    teamtalk::mstrings_t properties;
    REQUIRE(teamtalk::ExtractProperties(command, properties) == 1);

    ACE_Time_Value tm;
    REQUIRE(teamtalk::GetProperty(properties, ACE_TEXT("tm"), tm));
    REQUIRE(now.sec() == tm.sec());

    time_t const t_old = tm.sec();
    struct tm* tm_old = std::localtime(&t_old);

    struct tm tm_now = {};
    tm_now.tm_isdst = -1;
    tm_now.tm_year = tm_old->tm_year;
    tm_now.tm_mon = tm_old->tm_mon;
    tm_now.tm_mday = tm_old->tm_mday;
    tm_now.tm_hour = tm_old->tm_hour;
    tm_now.tm_min = tm_old->tm_min;
    tm_now.tm_sec = tm_old->tm_sec;

    time_t const t_now = std::mktime(&tm_now);
    REQUIRE(t_now == t_old);
}

TEST_CASE( "IPExtract" )
{
    REQUIRE(InetAddrFamily(ACE_TEXT("192.168.1.1")) == AF_INET);
    REQUIRE(InetAddrFamily(ACE_TEXT("2a02:9b0:402b:eed8:849b:310c:25c:bdda")) == AF_INET6);
    REQUIRE(InetAddrFamily(ACE_TEXT("jsdofkjsodfj")) < 0);

    REQUIRE(INetAddrNetwork(ACE_TEXT("192.168.1.1"), 32) == ACE_TEXT("192.168.1.1"));
    REQUIRE(INetAddrNetwork(ACE_TEXT("192.168.1.255"), 31) == ACE_TEXT("192.168.1.254"));
    REQUIRE(INetAddrNetwork(ACE_TEXT("192.168.1.1"), 24) == ACE_TEXT("192.168.1.0"));
    REQUIRE(INetAddrNetwork(ACE_TEXT("192.168.1.1"), 23) == ACE_TEXT("192.168.0.0"));
    REQUIRE(INetAddrNetwork(ACE_TEXT("192.168.129.1"), 23) == ACE_TEXT("192.168.128.0"));

    REQUIRE(INetAddrNetwork(ACE_TEXT("2a02:9b0:402b:eed8:849b:310c:25c:bdda"), 32) == ACE_TEXT("2a02:9b0::"));
    REQUIRE(INetAddrNetwork(ACE_TEXT("2a81::bdda"), 9) == ACE_TEXT("2a80::"));
    REQUIRE(INetAddrNetwork(ACE_TEXT("2a02:9b0:402b:eed8:849b:310c:25c:bdda"), 128) == ACE_TEXT("2a02:9b0:402b:eed8:849b:310c:25c:bdda"));
}

TEST_CASE( "MatchBan" )
{
    teamtalk::BannedUser banned_ipv4;
    banned_ipv4.bantype = BANTYPE_IPADDR;
    banned_ipv4.ipaddr = ACE_TEXT("192.168.1.111");

    teamtalk::BannedUser bancheck;

    bancheck.ipaddr = ACE_TEXT("192.168.1.111");
    REQUIRE(banned_ipv4.Match(bancheck));

    banned_ipv4.ipaddr = ACE_TEXT("192.168.1.111/32");
    REQUIRE(banned_ipv4.Match(bancheck));

    banned_ipv4.ipaddr = ACE_TEXT("192.168.0.0/23");
    REQUIRE(banned_ipv4.Match(bancheck));

    bancheck.ipaddr = ACE_TEXT("192.168.2.111");
    REQUIRE(banned_ipv4.Match(bancheck) == false);

    teamtalk::BannedUser banned_ipv6;
    banned_ipv6.bantype = BANTYPE_IPADDR;
    banned_ipv6.ipaddr = ACE_TEXT("2a02:9b0:402b:eed8:849b:310c:25c:bdda");

    bancheck.ipaddr = ACE_TEXT("2a02:9b0:402b:eed8:849b:310c:25c:bdda");
    REQUIRE(banned_ipv6.Match(bancheck));

    banned_ipv6.ipaddr = ACE_TEXT("2a02:9b0:402b:eed8:849b:310c:25c:bdda/128");
    REQUIRE(banned_ipv6.Match(bancheck));

    banned_ipv6.ipaddr = ACE_TEXT("2a02:09b0:402b:eed8::/63");
    REQUIRE(banned_ipv6.Match(bancheck));

    banned_ipv6.ipaddr = ACE_TEXT("2a02:9b0:402b:eed8:849b:310c:25c:bdda/128");
    REQUIRE(banned_ipv6.Match(bancheck));

    bancheck.ipaddr = ACE_TEXT("192.168.1.111");
    REQUIRE(banned_ipv6.Match(bancheck) == false);
}
