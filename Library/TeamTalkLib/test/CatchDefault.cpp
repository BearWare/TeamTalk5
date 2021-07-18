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

#include <ace/ACE.h>
#include <ace/OS.h>
#include <ace/Date_Time.h>

#include "TTUnitTest.h"

#include <myace/MyACE.h>
#include <teamtalk/server/ServerNode.h>
#include <teamtalk/client/ClientNodeBase.h>
#include <avstream/VideoCapture.h>
#include <avstream/MediaPlayback.h>
#include <teamtalk/client/AudioMuxer.h>

#include <map>
#include <iostream>
#include <future>
#include <thread>
#include <ctime>

#if defined(ENABLE_OGG)
#include <codec/OggFileIO.h>
#endif

#if defined(ENABLE_OPUS)
#include <codec/OpusEncoder.h>
#include <codec/OpusDecoder.h>
#endif
#include <codec/WaveFile.h>

#if defined(ENABLE_FFMPEG3)
#include <avstream/FFMpeg3Streamer.h>
#endif

#if defined (ENABLE_PORTAUDIO)
#include <avstream/PortAudioWrapper.h>
#endif

#if defined(WIN32)
#include <ace/Init_ACE.h>
#include <assert.h>
#include <Mmsystem.h>
#include <propsys.h>
#include <atlbase.h>
#include <MMDeviceApi.h>
#include <avstream/DMOResampler.h>
#include <avstream/PortAudioWrapper.h>
#include <mfapi.h>

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

TEST_CASE( "Init TT", "" ) {
    TTInstance* ttinst;
    REQUIRE( (ttinst = TT_InitTeamTalkPoll()) );
    REQUIRE( TT_CloseTeamTalk(ttinst) );
}

#if defined(ENABLE_OGG) && defined(ENABLE_SPEEX)
TEST_CASE( "Ogg Write", "" ) {
    SpeexEncFile spxfile;
    REQUIRE( spxfile.Open(ACE_TEXT("/foo.spx"), 1, DEFAULT_SPEEX_COMPLEXITY, 7, 32000, 48000, false) == false);
}
#endif

#if defined(ENABLE_OPUS)
TEST_CASE( "Record mux") {
    std::vector<ttinst> clients(2);
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
            audiocodec.opus.nTxIntervalMSec = 240;
#if defined(OPUS_FRAMESIZE_120_MS)
            audiocodec.opus.nFrameSizeMSec = 120;
#else
            audiocodec.opus.nFrameSizeMSec = 40;
#endif
            audiocodec.opus.nBitRate = OPUS_MIN_BITRATE;
            audiocodec.opus.nChannels = 2;
            audiocodec.opus.nComplexity = 10;
            audiocodec.opus.nSampleRate= 48000;
            audiocodec.opus.bDTX = true;
            audiocodec.opus.bFEC = true;
            audiocodec.opus.bVBR = false;
            audiocodec.opus.bVBRConstraint = false;

            Channel chan = MakeChannel(clients[i], ACE_TEXT("foo"), TT_GetRootChannelID(clients[i]), audiocodec);
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
    WaitForEvent(clients[0], CLIENTEVENT_NONE, 100);
    REQUIRE(TT_EnableVoiceTransmission(clients[0], false));

    REQUIRE(TT_StartRecordingMuxedAudioFile(clients[1], &chan.audiocodec, ACE_TEXT("MyMuxFile.wav"), AFF_WAVE_FORMAT));

    REQUIRE(TT_DBG_SetSoundInputTone(clients[0], STREAMTYPE_VOICE, 500));
    REQUIRE(TT_EnableVoiceTransmission(clients[0], true));
    WaitForEvent(clients[0], CLIENTEVENT_NONE, 2500);
    REQUIRE(TT_EnableVoiceTransmission(clients[0], false));

    REQUIRE(TT_DBG_SetSoundInputTone(clients[1], STREAMTYPE_VOICE, 600));
    REQUIRE(TT_EnableVoiceTransmission(clients[1], true));
    WaitForEvent(clients[1], CLIENTEVENT_NONE, 2500);
    REQUIRE(TT_EnableVoiceTransmission(clients[1], false));

    WaitForEvent(clients[1], CLIENTEVENT_NONE, 10000);

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
    audiocodec.opus.nTxIntervalMSec = 240;
#if defined(OPUS_FRAMESIZE_120_MS)
    audiocodec.opus.nFrameSizeMSec = 120;
#else
    audiocodec.opus.nFrameSizeMSec = 60;
#endif
    audiocodec.opus.nBitRate = OPUS_MIN_BITRATE;
    audiocodec.opus.nChannels = 2;
    audiocodec.opus.nComplexity = 10;
    audiocodec.opus.nSampleRate= 48000;
    audiocodec.opus.bDTX = true;
    audiocodec.opus.bFEC = true;
    audiocodec.opus.bVBR = false;
    audiocodec.opus.bVBRConstraint = false;

    Channel chan = MakeChannel(txclient, ACE_TEXT("foo"), TT_GetRootChannelID(txclient), audiocodec);
    REQUIRE(WaitForCmdSuccess(txclient, TT_DoJoinChannel(txclient, &chan)));

    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoJoinChannelByID(rxclient, TT_GetMyChannelID(txclient), ACE_TEXT(""))));

    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, 600));

    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, int(audiocodec.opus.nTxIntervalMSec * 5 + audiocodec.opus.nTxIntervalMSec * .5));
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));

    auto voicestop = [&](TTMessage msg)
    {
        if (msg.nClientEvent == CLIENTEVENT_USER_STATECHANGE &&
            msg.user.nUserID == TT_GetMyUserID(txclient) &&
            (msg.user.uUserState & USERSTATE_VOICE) == 0)
        {
            return true;
        }

        return false;
    };

    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, voicestop));
    //WaitForEvent(txclient, CLIENTEVENT_NONE, nullptr, audiocodec.opus.nTxIntervalMSec * 2);

    TTCHAR curdir[1024] = {};
    ACE_OS::getcwd(curdir, 1024);
    REQUIRE(TT_SetUserMediaStorageDir(rxclient, TT_GetMyUserID(txclient), curdir, ACE_TEXT(""), AFF_WAVE_FORMAT));

    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, 0));
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, 1000);
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

    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, 500));
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, 2000);
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));

    // This tone is not being stored in 'MyMuxFile.wav' because the
    // audio block will bypass the audio encoder.
    REQUIRE(TT_DBG_SetSoundInputTone(rxclient, STREAMTYPE_VOICE, 600));
    REQUIRE(TT_EnableVoiceTransmission(rxclient, true));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 2000);
    REQUIRE(TT_EnableVoiceTransmission(rxclient, false));

    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, 2000);
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));

    REQUIRE(TT_EnableVoiceTransmission(rxclient, true));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 2000);
    REQUIRE(TT_EnableVoiceTransmission(rxclient, false));

    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, 2000);
    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoUnsubscribe(rxclient, TT_GetMyUserID(txclient), SUBSCRIBE_VOICE)));

    WaitForEvent(txclient, CLIENTEVENT_NONE, 2000);

    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoSubscribe(rxclient, TT_GetMyUserID(txclient), SUBSCRIBE_VOICE)));

    REQUIRE(TT_EnableVoiceTransmission(rxclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, 2000);

    REQUIRE(TT_CloseSoundInputDevice(rxclient));
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, 2000);
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));

    REQUIRE(TT_StopRecordingMuxedAudioFile(rxclient));
}

int GetAudioBlockSamplesSum(TTInstance* ttinst, int userid, StreamTypes sts)
{
    TTMessage msg;
    REQUIRE(WaitForEvent(ttinst, CLIENTEVENT_USER_AUDIOBLOCK, msg));
    auto ab = TT_AcquireUserAudioBlock(ttinst, sts, userid);
    REQUIRE(ab);
    short* audiobuf = reinterpret_cast<short*>(ab->lpRawAudio);
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
    auto ab = TT_AcquireUserAudioBlock(rxclient, STREAMTYPE_VOICE, TT_MUXED_USERID);
    REQUIRE(ab != nullptr);
    REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));

    int sum = GetAudioBlockSamplesSum(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE);
    REQUIRE(sum == 0);

    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, 500));
    REQUIRE(TT_EnableVoiceTransmission(txclient, TRUE));

    int retries = 100;
    while (retries-- && GetAudioBlockSamplesSum(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE) == 0);
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

    int n_blocks = 10;
    do
    {
        TTMessage msg;
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
        REQUIRE(msg.nSource == TT_MUXED_USERID);
        AudioBlock* ab = TT_AcquireUserAudioBlock(ttclient, STREAMTYPE_VOICE, TT_MUXED_USERID);
        REQUIRE(ab);
        REQUIRE(ab->nSamples>0);
        REQUIRE(TT_ReleaseUserAudioBlock(ttclient, ab));
    } while (n_blocks--);
}

TEST_CASE("AudioMuxerInOutOfChannel")
{
    auto ttclient = InitTeamTalk();

    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(InitSound(ttclient));

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = 2;
    mfi.audioFmt.nSampleRate = 48000;
    mfi.uDurationMSec = 60 * 1000;
    ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("playfile.wav"));

    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 0));

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
    mfp.bPaused = FALSE;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

    StreamTypes sts = STREAMTYPE_VOICE | STREAMTYPE_MEDIAFILE_AUDIO | STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO;

    AudioFormat af = {};
    af.nAudioFmt = AFF_WAVE_FORMAT;
    af.nChannels = 2;
    af.nSampleRate = 12000;

    REQUIRE(TT_EnableAudioBlockEventEx(ttclient, TT_MUXED_USERID, sts, &af, TRUE));

    TTMessage msg;
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));

    // silence will appear
    REQUIRE(msg.nSource == TT_MUXED_USERID);
    abptr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, sts, TT_MUXED_USERID));
    REQUIRE(ab);
    REQUIRE(ab->nSamples>0);
    REQUIRE(ab->uStreamTypes == STREAMTYPE_NONE);

    // Local playback will appear
    auto playid = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);
    int n_blocks = 100;
    do
    {
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
        REQUIRE(msg.nSource == TT_MUXED_USERID);
        abptr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, sts, TT_MUXED_USERID));
        REQUIRE(ab);
        REQUIRE(ab->nSamples>0);
        if (ab->uStreamTypes == STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO)
            break;
    } while (n_blocks--);
    REQUIRE(n_blocks > 0);

    // Voice mixed with local playback will appear
    REQUIRE(TT_EnableVoiceTransmission(ttclient, true));
    REQUIRE(JoinRoot(ttclient));
    n_blocks = 100;
    do
    {
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
        REQUIRE(msg.nSource == TT_MUXED_USERID);
        abptr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, sts, TT_MUXED_USERID));
        REQUIRE(ab);
        REQUIRE(ab->nSamples>0);
        if (ab->uStreamTypes == (STREAMTYPE_VOICE | STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO))
            break;
    } while (n_blocks--);
    REQUIRE(n_blocks > 0);

    // Voice will disappear
    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoLeaveChannel(ttclient)));
    n_blocks = 100;
    do
    {
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
        REQUIRE(msg.nSource == TT_MUXED_USERID);
        abptr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, sts, TT_MUXED_USERID));
        REQUIRE(ab);
        REQUIRE(ab->nSamples>0);
        if (ab->uStreamTypes == STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO)
            break;
    } while (n_blocks--);
    REQUIRE(n_blocks > 0);

    // Local playback will disappear
    REQUIRE(TT_StopLocalPlayback(ttclient, playid));
    n_blocks = 100;
    do
    {
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
        REQUIRE(msg.nSource == TT_MUXED_USERID);
        abptr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, sts, TT_MUXED_USERID));
        REQUIRE(ab);
        REQUIRE(ab->nSamples>0);
        if (ab->uStreamTypes == STREAMTYPE_NONE)
            break;
    } while (n_blocks--);
    REQUIRE(n_blocks > 0);
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
        if (msg.nClientEvent == CLIENTEVENT_USER_STATECHANGE &&
            msg.user.nUserID == TT_GetMyUserID(txclient) &&
            (msg.user.uUserState & USERSTATE_VOICE) == USERSTATE_VOICE)
        {
            return true;
        }

        return false;
    };
    auto voicestop = [&](TTMessage msg)
    {
        if (msg.nClientEvent == CLIENTEVENT_USER_STATECHANGE &&
            msg.user.nUserID == TT_GetMyUserID(txclient) &&
            (msg.user.uUserState & USERSTATE_VOICE) == USERSTATE_NONE)
        {
            return true;
        }

        return false;
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
    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, 500));
    int txuserid = TT_GetMyUserID(txclient);

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    REQUIRE(TT_EnableVoiceTransmission(txclient, true));

    TTMessage msg;

    // calc default volume level of muxed audio block
    REQUIRE((WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, msg) && (msg.user.uUserState & USERSTATE_VOICE) == USERSTATE_VOICE));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));

    uint32_t sum_nogain;
    sum_nogain = GetAudioBlockSamplesSum(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE);

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);
    REQUIRE(TT_AcquireUserAudioBlock(rxclient,STREAMTYPE_VOICE, TT_MUXED_USERID) == nullptr);

    // double volume level of user
    User user;
    REQUIRE(TT_GetUser(rxclient, txuserid, &user));
    REQUIRE(TT_SetUserVolume(rxclient, txuserid, STREAMTYPE_VOICE, user.nVolumeVoice * 2));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));

    int retries = 5;
    uint32_t sum_gain;
    do
    {
        sum_gain = GetAudioBlockSamplesSum(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE);
    }
    while (sum_gain <= sum_nogain * 1.9 && retries);

    // volume level of muxed audio should now have doubled (roughly due to tone offset)
    REQUIRE(sum_gain > sum_nogain * 1.9);

    // reset
    REQUIRE(TT_SetUserVolume(rxclient, txuserid, STREAMTYPE_VOICE, SOUND_VOLUME_DEFAULT));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // volume level of muxed audio when changing master volume
    REQUIRE(TT_SetSoundOutputVolume(rxclient, SOUND_VOLUME_DEFAULT * 2));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));

    retries = 5;
    do
    {
        sum_gain = GetAudioBlockSamplesSum(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE);
    }
    while (sum_gain <= sum_nogain * 1.9 && retries--);

    // volume level of muxed audio should now have doubled (roughly due to tone offset)
    REQUIRE(sum_gain > sum_nogain * 1.9);

    // reset
    REQUIRE(TT_SetSoundOutputVolume(rxclient, SOUND_VOLUME_DEFAULT));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // calc default volume leve of single stream audioblock
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, TRUE));

    for (int i=0;i<2;++i)
        sum_nogain = GetAudioBlockSamplesSum(rxclient, txuserid, STREAMTYPE_VOICE);

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // double volume level
    REQUIRE(TT_SetUserVolume(rxclient, txuserid, STREAMTYPE_VOICE, user.nVolumeVoice * 2));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, TRUE));

    retries = 5;
    do
    {
        sum_gain = GetAudioBlockSamplesSum(rxclient, txuserid, STREAMTYPE_VOICE);
    }
    while (sum_gain <= sum_nogain * 1.9 && retries--);

    // volume level of muxed audio should now have doubled (roughly due to tone offset)
    REQUIRE(sum_gain > sum_nogain * 1.9);

    // reset
    REQUIRE(TT_SetUserVolume(rxclient, txuserid, STREAMTYPE_VOICE, SOUND_VOLUME_DEFAULT));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // test master mute
    REQUIRE(TT_SetSoundOutputMute(rxclient, true));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, TRUE));

    retries = 5;
    do
    {
        sum_gain = GetAudioBlockSamplesSum(rxclient, txuserid, STREAMTYPE_VOICE);
    }
    while (sum_gain != 0 && retries--);

    // mute master gives 0 sum
    REQUIRE(sum_gain == 0);

    REQUIRE(TT_SetSoundOutputMute(rxclient, true));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // test user mute
    REQUIRE(TT_SetUserMute(rxclient, txuserid, STREAMTYPE_VOICE, TRUE));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, TRUE));

    retries = 5;
    do
    {
        sum_gain = GetAudioBlockSamplesSum(rxclient, txuserid, STREAMTYPE_VOICE);
    }
    while (sum_gain != 0 && retries--);

    // mute user gives 0 sum
    REQUIRE(sum_gain == 0);
}

TEST_CASE( "AudioMuxerRawSameStreamTypeSameAudioFormat" )
{
    media::AudioInputFormat inputfmt(media::AudioFormat(12000, 1), int(12000 * .01));
    const int TOTALSAMPLES = inputfmt.GetTotalSamples();
    const int FRAMESIZE = inputfmt.samples;
    const auto FMT = inputfmt.fmt;
    const int FRAMEBYTES = inputfmt.GetBytes();

    msg_queue_t mixed_frames;
    mixed_frames.high_water_mark(1024 * 1024);
    mixed_frames.low_water_mark(1024 * 1024);

    AudioMuxer muxer(teamtalk::STREAMTYPE_VOICE);
    int muxinterval_msec = inputfmt.GetDurationMSec() * 5;
    muxer.SetMuxInterval(muxinterval_msec);
    auto mixedfunc = [&] (teamtalk::StreamTypes sts, const media::AudioFrame& frm)
    {
        auto mb = AudioFrameToMsgBlock(frm);
        REQUIRE(mixed_frames.enqueue(mb) >= 0);
    };

    REQUIRE(muxer.RegisterMuxCallback(inputfmt, mixedfunc));

    std::vector<int> userids;
    userids.push_back(10);
    userids.push_back(20);
    userids.push_back(30);
    std::vector< std::vector<short> > user_bufs;
    std::vector<media::AudioFrame> user_frames;
    for (auto u : userids)
        user_bufs.push_back(std::vector<short>(inputfmt.GetTotalSamples(), u));
    for (size_t i=0;i<userids.size();++i)
    {
        media::AudioFrame frm(FMT, &user_bufs[i][0], inputfmt.samples, userids[i] * 1000);
        frm.streamid = userids[i];
        user_frames.push_back(frm);
    }

    REQUIRE(muxer.QueueUserAudio(user_frames[0].streamid, teamtalk::STREAMTYPE_VOICE, user_frames[0]));

    // test mixing of single stream
    ACE_Message_Block* mb;
    REQUIRE(mixed_frames.dequeue(mb) >= 0);
    {
        MBGuard g(mb);
        media::AudioFrame frm_mixed(mb);
        REQUIRE(std::memcmp(frm_mixed.input_buffer, user_frames[0].input_buffer, FRAMEBYTES) == 0);
    }

    ACE_Time_Value tm = ACE_OS::gettimeofday() + ToTimeValue(50);
    REQUIRE(mixed_frames.dequeue(mb, &tm) < 0);

    user_frames[0].sample_no += FRAMESIZE;
    REQUIRE(muxer.QueueUserAudio(user_frames[0].streamid, teamtalk::STREAMTYPE_VOICE, user_frames[0]));

    REQUIRE(mixed_frames.dequeue(mb) >= 0);
    {
        MBGuard g(mb);
        media::AudioFrame frm_mixed(mb);
        REQUIRE(std::memcmp(frm_mixed.input_buffer, user_frames[0].input_buffer, FRAMEBYTES) == 0);
    }

    // test mixing of two stream
    REQUIRE(muxer.QueueUserAudio(user_frames[1].streamid, teamtalk::STREAMTYPE_VOICE, user_frames[1]));
    user_frames[0].sample_no += FRAMESIZE;
    REQUIRE(muxer.QueueUserAudio(user_frames[0].streamid, teamtalk::STREAMTYPE_VOICE, user_frames[0]));
    REQUIRE(mixed_frames.dequeue(mb) >= 0);
    {
        MBGuard g(mb);
        media::AudioFrame frm_mixed(mb);
        for (int i=0;i<TOTALSAMPLES;++i)
            REQUIRE(frm_mixed.input_buffer[i] == userids[0] + userids[1]);
    }

    user_frames[1].sample_no += FRAMESIZE;
    REQUIRE(muxer.QueueUserAudio(user_frames[1].streamid, teamtalk::STREAMTYPE_VOICE, user_frames[1]));

    tm = ACE_OS::gettimeofday() + ToTimeValue(50);
    REQUIRE(mixed_frames.dequeue(mb, &tm) < 0);

    // terminate stream for #0
    REQUIRE(muxer.QueueUserAudio(user_frames[0].streamid, teamtalk::STREAMTYPE_VOICE, media::AudioFrame()));

    REQUIRE(mixed_frames.dequeue(mb) >= 0);
    {
        MBGuard g(mb);
        media::AudioFrame frm_mixed(mb);
        REQUIRE(std::memcmp(frm_mixed.input_buffer, user_frames[1].input_buffer, FRAMEBYTES) == 0);
    }

    // terminate stream for #1
    REQUIRE(muxer.QueueUserAudio(user_frames[1].streamid, teamtalk::STREAMTYPE_VOICE, media::AudioFrame()));

    // expect silent mix
    REQUIRE(mixed_frames.dequeue(mb) >= 0);
    {
        MBGuard g(mb);
        media::AudioFrame frm_mixed(mb);
        for (int i=0;i<TOTALSAMPLES;++i)
            REQUIRE(frm_mixed.input_buffer[i] == 0);
    }

    // start new stream for #0
    user_frames[0].sample_no += FRAMESIZE;
    REQUIRE(muxer.QueueUserAudio(user_frames[0].streamid, teamtalk::STREAMTYPE_VOICE, user_frames[0]));

    // wait for #0 audio to appear so it blocks further muxing
    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard g(mb);
        media::AudioFrame frm_mixed(mb);
        if (frm_mixed.input_buffer[0] == userids[0])
            break;
    }

    // overflow the buffer
    int n_flood = 10 * muxinterval_msec / inputfmt.GetDurationMSec();
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
        MBGuard g(mb);
        media::AudioFrame frm_mixed(mb);
        if (frm_mixed.input_buffer[0] == muxsum)
            break;
    }

    // wait for audio muxer to report delayer, i.e. drain muxer
    tm = ACE_OS::gettimeofday() + ToTimeValue(muxinterval_msec * 3);
    bool gotmux = false;
    while (mixed_frames.dequeue(mb, &tm) >= 0)
    {
        MBGuard g(mb);
        media::AudioFrame frm_mixed(mb);
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
        MBGuard g(mb);
        media::AudioFrame frm_mixed(mb);
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
        MBGuard g(mb);
        media::AudioFrame frm_mixed(mb);
        if (frm_mixed.input_buffer[0] == 0)
            break;
    }
}

TEST_CASE("MergeAudioBlocks")
{
    media::AudioFormat fmt(48000, 2);
    std::vector<short> rawaudio;
    short v = 1000;
    for (size_t i=0;i<77000*fmt.channels;++i)
        rawaudio.push_back(v++);

    std::vector<ACE_Message_Block*> mbs;
    media::AudioFrame frm(fmt, &rawaudio[0], 11000, 7000);
    mbs.push_back(AudioFrameToMsgBlock(frm));
    frm = media::AudioFrame(fmt, &rawaudio[11000 * fmt.channels], 22000, 7000 + 11000);
    mbs.push_back(AudioFrameToMsgBlock(frm));
    frm = media::AudioFrame(fmt, &rawaudio[(11000 + 22000) * fmt.channels], 44000, 7000 + 11000 + 22000);
    mbs.push_back(AudioFrameToMsgBlock(frm));
    auto mb = AudioFramesMerge(mbs);
    REQUIRE(mb->size() == mbs[0]->size() + mbs[1]->size() + mbs[2]->size() - sizeof(media::AudioFrame) * 2);
    MBGuard g(mb);
    frm = media::AudioFrame(mb);
    for (size_t i=0;i<rawaudio.size();++i)
        REQUIRE(frm.input_buffer[i] == rawaudio[i]);
    REQUIRE(frm.sample_no == 7000);
    REQUIRE(fmt == frm.inputfmt);

    for (auto m : mbs)
        m->release();
    mbs.clear();

    mbs.push_back(mb);
    mb = AudioFramesMerge(mbs);
    MBGuard gg(mb);
    frm = media::AudioFrame(mb);
    for (size_t i=0;i<rawaudio.size();++i)
        REQUIRE(frm.input_buffer[i] == rawaudio[i]);
    REQUIRE(frm.sample_no == 7000);
    REQUIRE(fmt == frm.inputfmt);
}

TEST_CASE("BuildAudioFrame")
{
    // Build from one big audio frame with remainder
    media::AudioFormat fmt(48000, 2);
    std::vector<short> rawaudio;
    short v = 1000;
    for (size_t i=0;i<77000*fmt.channels;++i)
        rawaudio.push_back(v++);

    std::vector<ACE_Message_Block*> mbs;
    media::AudioFrame frm(fmt, &rawaudio[0], 77000, 7000);
    mbs.push_back(AudioFrameToMsgBlock(frm));

    uint32_t sampleindex = 7000;
    v = 1000;
    while (auto mb = AudioFrameFromList(120, mbs))
    {
        MBGuard g(mb);
        REQUIRE(mbs.size());
        media::AudioFrame newfrm(mb);
        REQUIRE(newfrm.sample_no == sampleindex);
        sampleindex += 120;
        for (int j=0;j<newfrm.input_samples*fmt.channels;++j)
            REQUIRE(newfrm.input_buffer[j] == v++);
    }
    REQUIRE(mbs.size());
    media::AudioFrame remain(mbs[0]);
    MBGuard g(mbs[0]);
    for (int j=0;j<remain.input_samples*fmt.channels;++j)
        REQUIRE(remain.input_buffer[j] == v++);
    REQUIRE(remain.input_samples == 77000 % 120);
    mbs.clear();

    // Build from one big audio frame without remainder
    mbs.push_back(AudioFrameToMsgBlock(frm));
    sampleindex = 7000;
    v = 1000;
    std::vector<ACE_Message_Block*> mbs_next;
    while (auto mb = AudioFrameFromList(100, mbs))
    {
        MBGuard g(mb);
        media::AudioFrame newfrm(mb);
        REQUIRE(newfrm.sample_no == sampleindex);
        sampleindex += 100;
        for (int j=0;j<newfrm.input_samples*fmt.channels;++j)
            REQUIRE(newfrm.input_buffer[j] == v++);

        // store for next test-case
        mbs_next.push_back(AudioFrameToMsgBlock(newfrm));
    }
    REQUIRE(mbs.empty());

    // Build from many small audio frames
    mbs = mbs_next;
    sampleindex = 7000;
    v = 1000;
    while (auto mb = AudioFrameFromList(1554, mbs))
    {
        MBGuard g(mb);
        REQUIRE(mbs.size());
        media::AudioFrame newfrm(mb);
        REQUIRE(newfrm.sample_no == sampleindex);
        sampleindex += 1554;
        for (int j=0;j<newfrm.input_samples*fmt.channels;++j)
            REQUIRE(newfrm.input_buffer[j] == v++);
    }

    remain = media::AudioFrame(mbs[0]);

    for (int j=0;j<remain.input_samples*fmt.channels;++j)
        REQUIRE(remain.input_buffer[j] == v++);
    int remainsamples = 0;
    for (auto m : mbs)
    {
        remainsamples += media::AudioFrame(m).input_samples;
        m->release();
    }
    REQUIRE(remainsamples == 77000 % 1554);
}

TEST_CASE("GenerateToneStereoWaveFile")
{
    media::AudioFormat fmt(32000, 2);
    std::vector<short> buf(fmt.samplerate * fmt.channels);
    media::AudioFrame frm(fmt, &buf[0], 32000);
    WavePCMFile wavfile;
    REQUIRE(wavfile.NewFile(ACE_TEXT("stereo.wav"), fmt));
    for (int i=0;i<5;++i)
    {
        frm.sample_no = GenerateTone(frm, frm.sample_no, 500, 8000, false, true);
        REQUIRE(wavfile.AppendSamples(frm.input_buffer, frm.input_samples));
        frm.sample_no = GenerateTone(frm, frm.sample_no, 500, 8000, true, false);
        REQUIRE(wavfile.AppendSamples(frm.input_buffer, frm.input_samples));
    }
    wavfile.Close();
    REQUIRE(wavfile.OpenFile(ACE_TEXT("stereo.wav"), true));
    for (int i=0;i<5;++i)
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
    media::AudioInputFormat inputfmt(media::AudioFormat(12000, 1), int(12000 * .01));
    const int TOTALSAMPLES = inputfmt.GetTotalSamples();
    const int FRAMESIZE = inputfmt.samples;
    const auto FMT = inputfmt.fmt;
    const int FRAMEBYTES = inputfmt.GetBytes();

    msg_queue_t mixed_frames, mixed_ticker;
    AudioMuxer muxer(teamtalk::STREAMTYPE_VOICE | teamtalk::STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO | teamtalk::STREAMTYPE_MEDIAFILE_AUDIO);
    int muxinterval_msec = inputfmt.GetDurationMSec() * 5;
    muxer.SetMuxInterval(muxinterval_msec);
    auto mixedfunc = [&] (teamtalk::StreamTypes sts, const media::AudioFrame& frm)
    {
        auto mb = AudioFrameToMsgBlock(frm);
        REQUIRE(mixed_frames.enqueue(mb) >= 0);
    };

    auto tickfunc = [&] (teamtalk::StreamTypes sts, uint32_t sample_no)
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
    user_fmts.push_back(media::AudioFormat(48000, 2));
    user_fmts.push_back(media::AudioFormat(32000, 2));
    user_fmts.push_back(FMT);
    std::vector< std::vector<short> > user_bufs;
    std::vector<media::AudioFrame> user_frames;
    user_bufs.push_back(std::vector<short>(inputfmt.GetTotalSamples(), userids[0]));
    user_bufs.push_back(std::vector<short>((user_fmts[1].channels * user_fmts[1].samplerate) * .01, userids[1]));
    user_bufs.push_back(std::vector<short>((user_fmts[2].channels * user_fmts[2].samplerate) * .015, userids[2]));
    user_bufs.push_back(std::vector<short>((user_fmts[3].channels * user_fmts[3].samplerate) * .005, userids[3]));
    std::vector<teamtalk::StreamType> user_sts;
    user_sts.push_back(teamtalk::STREAMTYPE_VOICE);
    user_sts.push_back(teamtalk::STREAMTYPE_MEDIAFILE_AUDIO);
    user_sts.push_back(teamtalk::STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO);
    user_sts.push_back(teamtalk::STREAMTYPE_VOICE);

    for (size_t i=0;i<userids.size();++i)
    {
        media::AudioFrame frm(user_fmts[i], &user_bufs[i][0], user_bufs[i].size() / user_fmts[i].channels, user_bufs[i].size() * 1000);
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
        MBGuard g(mb);
    }

    REQUIRE(mixed_frames.message_count() == 0);

    // submit to generate full mux frame
    user_frames[3].sample_no += user_frames[3].input_samples;
    REQUIRE(muxer.QueueUserAudio(userids[3], user_sts[3], user_frames[3]));

    {
        REQUIRE(mixed_frames.dequeue(mb) >= 0);
        MBGuard g(mb);
    }

    // test overflow of resampler buffer
    int n_overflow = 3 * ((muxinterval_msec / 1000.) / .015);
    while (n_overflow--)
    {
        user_frames[2].sample_no += user_frames[2].input_samples;
        REQUIRE(muxer.QueueUserAudio(userids[2], user_sts[2], user_frames[2]));
    }

    // not enough audio to generate mux frame
    {
        mixed_ticker.dequeue(mb);
        MBGuard g(mb);
    }
    REQUIRE(mixed_frames.message_count() == 0);
}

TEST_CASE( "AudioMuxerRawOverflow" )
{
    media::AudioInputFormat inputfmt(media::AudioFormat(48000, 2), int(48000 * .01));
    const int TOTALSAMPLES = inputfmt.GetTotalSamples();
    const int FRAMESIZE = inputfmt.samples;
    const auto FMT = inputfmt.fmt;

    msg_queue_t mixed_frames, mixed_ticker;
    auto QSIZE = 1024*1024*10;
    mixed_frames.high_water_mark(QSIZE);
    mixed_frames.low_water_mark(QSIZE);
    mixed_ticker.high_water_mark(QSIZE);
    mixed_ticker.low_water_mark(QSIZE);
    AudioMuxer muxer(teamtalk::STREAMTYPE_VOICE);
    auto mixedfunc = [&] (teamtalk::StreamTypes sts, const media::AudioFrame& frm)
    {
        auto mb = AudioFrameToMsgBlock(frm);
        REQUIRE(mixed_frames.enqueue(mb) >= 0);
    };
    auto tickfunc = [&] (teamtalk::StreamTypes sts, uint32_t sample_no)
    {
        media::AudioFrame tickfrm;
        tickfrm.sample_no = sample_no;
        REQUIRE(mixed_ticker.enqueue(AudioFrameToMsgBlock(tickfrm)) >= 0);
    };

    muxer.RegisterMuxTick(tickfunc);
    REQUIRE(muxer.RegisterMuxCallback(inputfmt, mixedfunc));

    std::vector<short> buffer(TOTALSAMPLES, short(1));
    media::AudioFrame frm(FMT, &buffer[0], FRAMESIZE);

    ACE_Message_Block* mb = nullptr;
    {
        REQUIRE(mixed_frames.dequeue(mb) >= 0);
        MBGuard g(mb);
    }

    // overflow AudioMuxer's queue
    short v = 2;
    while (frm.sample_no < frm.inputfmt.samplerate * 2)
    {
        REQUIRE(muxer.QueueUserAudio(16, teamtalk::STREAMTYPE_VOICE, frm));
        frm.sample_no += FRAMESIZE;
        for (size_t i=0;i<buffer.size();++i)
            buffer[i] = v;
        ++v;
    }

    REQUIRE(muxer.QueueUserAudio(16, teamtalk::STREAMTYPE_VOICE, media::AudioFrame()));

    // verify AudioMuxer survives by ensuring AudioMuxer starts generating silence again.
    auto c = mixed_ticker.message_count();
    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard g(mb);
        if (media::AudioFrame(mb).input_buffer[0] == 0 && c != mixed_ticker.message_count())
            break;
    }

    // AudioContainer has a 3 second buffer @ 48KHz, so lets overflow that
    auto offset = frm.sample_no;
    while (frm.sample_no - offset < 48000 * 10)
    {
        REQUIRE(muxer.QueueUserAudio(16, teamtalk::STREAMTYPE_VOICE, frm));
        frm.sample_no += FRAMESIZE;
        for (size_t i=0;i<buffer.size();++i)
            buffer[i] = v;
        ++v;
    }

    REQUIRE(muxer.QueueUserAudio(16, teamtalk::STREAMTYPE_VOICE, media::AudioFrame()));

    // verify AudioMuxer survives by ensuring AudioMuxer starts generating silence again.
    c = mixed_ticker.message_count();
    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard g(mb);
        if (media::AudioFrame(mb).input_buffer[0] == 0 && c != mixed_ticker.message_count())
            break;
    }
}

TEST_CASE( "AudioMuxerRawSampleIndex" )
{
    media::AudioInputFormat inputfmt(media::AudioFormat(48000, 2), int(48000 * .01));
    const int TOTALSAMPLES = inputfmt.GetTotalSamples();
    const int FRAMESIZE = inputfmt.samples;
    const auto FMT = inputfmt.fmt;

    ACE_Time_Value zero;
    msg_queue_t mixed_frames, mixed_ticker;
    auto QSIZE = 1024*1024*10;
    mixed_frames.high_water_mark(QSIZE);
    mixed_frames.low_water_mark(QSIZE);
    mixed_ticker.high_water_mark(QSIZE);
    mixed_ticker.low_water_mark(QSIZE);
    AudioMuxer muxer(teamtalk::STREAMTYPE_VOICE);
    auto mixedfunc = [&] (teamtalk::StreamTypes sts, const media::AudioFrame& frm)
    {
        auto mb = AudioFrameToMsgBlock(frm);
        REQUIRE(mixed_frames.enqueue(mb, &zero) >= 0);
    };
    auto tickfunc = [&] (teamtalk::StreamTypes sts, uint32_t sample_no)
    {
        media::AudioFrame tickfrm;
        tickfrm.sample_no = sample_no;
        REQUIRE(mixed_ticker.enqueue(AudioFrameToMsgBlock(tickfrm), &zero) >= 0);
    };

    muxer.RegisterMuxTick(tickfunc);
    REQUIRE(muxer.RegisterMuxCallback(inputfmt, mixedfunc));

    std::vector<short> buffer(TOTALSAMPLES, short(1));
    media::AudioFrame frm(FMT, &buffer[0], FRAMESIZE);

    REQUIRE(muxer.QueueUserAudio(16, teamtalk::STREAMTYPE_VOICE, frm));

    ACE_Message_Block* mb = nullptr;
    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard g(mb);
        if (media::AudioFrame(mb).input_buffer[0] == 1)
            break;
    }

    buffer.assign(buffer.size(), short(2));
    frm.sample_no += FRAMESIZE * 2; // sample index mismatch

    REQUIRE(muxer.QueueUserAudio(16, teamtalk::STREAMTYPE_VOICE, frm));
    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard g(mb);
        // audio frame (2) is lost
        if (media::AudioFrame(mb).input_buffer[0] == 0)
            break;
    }

    buffer.assign(buffer.size(), short(3));
    frm.sample_no += FRAMESIZE * 3;

    REQUIRE(muxer.QueueUserAudio(16, teamtalk::STREAMTYPE_VOICE, frm));
    while (mixed_frames.dequeue(mb) >= 0)
    {
        MBGuard g(mb);
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
    mfi.audioFmt.nChannels = 2;
    mfi.audioFmt.nSampleRate = 48000;
    mfi.uDurationMSec = 10 * 1000;
    ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("muxtone.wav"));

    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 500));

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    mfp.bPaused = FALSE;

    auto session = TT_InitLocalPlayback(rxclient, mfi.szFileName, &mfp);
    REQUIRE(session > 0);

    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, 800));

    TTMessage msg;

    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
    REQUIRE(msg.mediafileinfo.nStatus == MFS_STARTED);

    StreamTypes sts = STREAMTYPE_VOICE | STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO;
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, sts, TRUE));

    std::vector<int> premux, aftermux;
    uint32_t sum_mux_mf = 0, sum_mux_mf_voice = 0;
    sum_mux_mf = GetAudioBlockSamplesSum(rxclient, TT_MUXED_USERID, sts);

    REQUIRE(TT_EnableVoiceTransmission(txclient, true));

    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, msg));
    REQUIRE((msg.user.uUserState & USERSTATE_VOICE) == USERSTATE_VOICE);

    // drain
    while (WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK, msg, 0))
    {
        auto ab = TT_AcquireUserAudioBlock(rxclient, sts, TT_MUXED_USERID);
        REQUIRE(ab);
        REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
    }

    int n_frames = 10;
    do
    {
        sum_mux_mf_voice = GetAudioBlockSamplesSum(rxclient, TT_MUXED_USERID, sts);
    }
    while (sum_mux_mf_voice <= sum_mux_mf * 1.2 && --n_frames);

    REQUIRE(sum_mux_mf_voice > sum_mux_mf * 1.2);
}

TEST_CASE( "AudioMuxerStreamTypeRecording" )
{
    auto rxclient = InitTeamTalk();

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = 2;
    mfi.audioFmt.nSampleRate = 48000;
    mfi.uDurationMSec = 1000;
    ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("tone_100.wav"));
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 100));
    mfi.audioFmt.nChannels = 1;
    mfi.audioFmt.nSampleRate = 44100;
    ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("tone_200.wav"));
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 200));
    mfi.audioFmt.nChannels = 2;
    mfi.audioFmt.nSampleRate = 12000;
    ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("tone_300.wav"));
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 300));

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
    while (sessions.size())
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

    // resume playback
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 1000);
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
    ac.opus.nSampleRate = 48000;
    ac.opus.nChannels = 2;
    ac.opus.nTxIntervalMSec = 240;
#if defined(OPUS_FRAMESIZE_120_MS)
    ac.opus.nFrameSizeMSec = 120;
#else
    ac.opus.nFrameSizeMSec = 40;
#endif

    TTCHAR filename[TT_STRLEN] = ACE_TEXT("testdata/Opus/on.ogg");

    auto ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient, DEFAULT, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient"), ACE_TEXT("admin"), ACE_TEXT("admin")));

    auto chan = MakeChannel(ttclient, ACE_TEXT("Channel5"), TT_GetRootChannelID(ttclient), ac);
    chan.uChannelType = CHANNEL_PERMANENT | CHANNEL_HIDDEN;
    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoJoinChannel(ttclient, &chan)));

    AudioFormat af;
    af.nAudioFmt = AFF_WAVE_FORMAT;
    af.nChannels = 1;
    af.nSampleRate = 8000;
    REQUIRE(TT_EnableAudioBlockEventEx(ttclient, TT_MUXED_USERID, STREAMTYPE_VOICE | STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, &af, TRUE));

    WavePCMFile wavfile;
    REQUIRE(wavfile.NewFile(ACE_TEXT("channel5mix.wav"), af.nSampleRate, af.nChannels));

    uint32_t starttime = GETTIMESTAMP();
    uint32_t durationMSec = 0;

    int silenceBlockCount = 0;
    bool playoutstarted = false;
    TTMessage msg;

    while (durationMSec < 3000)
    {
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
        StreamType streamtype = static_cast<StreamType>(msg.nStreamType);
        AudioBlock* ttAudioblock = TT_AcquireUserAudioBlock(ttclient, streamtype, msg.nSource);
        REQUIRE(ttAudioblock);
        REQUIRE(ttAudioblock->nSamples == int(ttAudioblock->nSampleRate * .02));
        REQUIRE(ttAudioblock->nSampleRate == af.nSampleRate);
        REQUIRE(ttAudioblock->nChannels == af.nChannels);
        wavfile.AppendSamples(reinterpret_cast<const short*>(ttAudioblock->lpRawAudio), ttAudioblock->nSamples);

        if (ttAudioblock->uStreamTypes == STREAMTYPE_NONE)
        {
            silenceBlockCount++;
        }

        // Start playout after receving a few block without audio. 
        // Not required to reproduce the problem, but the mechanism might be useful
        // for debugging purpose
        if (!playoutstarted && (silenceBlockCount > 1))
        {
            MediaFilePlayback mfp = {};
            mfp.bPaused = FALSE;
            mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

            INT32 session = TT_InitLocalPlayback(ttclient, filename, &mfp);
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
    codec.opus.nBitRate = 64000;
    codec.opus.nChannels = 2;
    codec.opus.nComplexity = 9;
    codec.opus.nFrameSizeMSec = 5;
    codec.opus.nSampleRate = 24000;
    codec.opus.nTxIntervalMSec = 400;
    Channel chan = MakeChannel(rxclient, ACE_TEXT("opustools"), TT_GetMyChannelID(rxclient), codec);
    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoJoinChannel(rxclient, &chan)));
#else
    codec.nCodec = SPEEX_VBR_CODEC;
    codec.speex_vbr = {};
    codec.speex_vbr.nBandmode = 1;
    codec.speex_vbr.nBitRate = 16000;
    codec.speex_vbr.nMaxBitRate = 32000;
    codec.speex_vbr.nQuality = 5;
    codec.speex_vbr.nTxIntervalMSec = 400;
    Channel chan = MakeChannel(rxclient, ACE_TEXT("speex"), TT_GetMyChannelID(rxclient), codec);
    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoJoinChannel(rxclient, &chan)));
#endif
    const TTCHAR FILENAME[] = ACE_TEXT("MyMuxFile.ogg");
    REQUIRE(TT_GetChannel(rxclient, TT_GetMyChannelID(rxclient), &chan));
    REQUIRE(TT_StartRecordingMuxedAudioFile(rxclient, &chan.audiocodec, FILENAME, AFF_CHANNELCODEC_FORMAT));

    WaitForEvent(rxclient, CLIENTEVENT_NONE, 2000);

    OggFile of;
    REQUIRE(of.Open(FILENAME));
    ogg_page op;
    int pages = 0;
    REQUIRE(of.ReadOggPage(op));
    REQUIRE(op.header_len>0);
    REQUIRE(op.body_len>0);
    pages++;
    while (of.ReadOggPage(op))pages++;
}
#endif

#if defined(ENABLE_ENCRYPTION) && 0 /* doesn't work on GitHub */
TEST_CASE("TestHTTPS")
{
    //ACE::HTTPS::Context::set_default_ssl_mode(ACE_SSL_Context::SSLv23);
    //ACE::HTTPS::Context::set_default_verify_mode(true);
    //ACE::HTTPS::Context::instance().use_default_ca();
    //ACE::INet::SSL_CallbackManager::instance()->set_certificate_callback(new ACE::INet::SSL_CertificateAcceptor);

    std::string response1, response2, response3;
    REQUIRE(1 == HttpRequest("http://www.bearware.dk/teamtalk/weblogin.php?ping=1", response1));
    REQUIRE(1 == HttpRequest("https://www.bearware.dk/teamtalk/weblogin.php?ping=1", response2));
    REQUIRE(response1 == response2);
    REQUIRE(1 == HttpRequest("https://www.google.com", response3));
}

TEST_CASE("TestWebLogin")
{
    REQUIRE(teamtalk::LoginBearWareAccount("", "") > 0);
}
#endif

#if defined(WIN32)

TEST_CASE("TT_AEC")
{
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
    int abCount = 20;
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

    abCount = 20;
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
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_INTERNAL_ERROR, 500) == false);

    // cannot disable sound effects either
    effects.bEnableAGC = effects.bEnableDenoise = effects.bEnableEchoCancellation = FALSE;
    REQUIRE(TT_SetSoundDeviceEffects(ttclient, &effects) == FALSE);
}
#endif

#if defined(ENABLE_FFMPEG3)
TEST_CASE("testThumbnail")
{
    // ffmpeg -i in.mp3 -i teamtalk.png -map 0:0 -map 1:0 -c copy -id3v2_version 3 -metadata:s:v title="Album cover" -metadata:s:v comment="Cover (front)" out.mp3

    TTCHAR filename[TT_STRLEN] = ACE_TEXT("testdata/mp3/thumbnail.mp3");

    MediaFileProp mfp;
    REQUIRE(GetMediaFileProp(filename, mfp));
    REQUIRE(!mfp.video.IsValid());

    MediaStreamOutput prop(media::AudioFormat(16000, 2), 1600, media::FOURCC_NONE);
    FFMpegStreamer ffmpeg(filename, prop);

    REQUIRE(ffmpeg.Open());

    std::promise<bool> done;
    auto sig_done = done.get_future();

    auto status = [&] (const MediaFileProp& mfp, MediaStreamStatus status) {
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

#if defined(ENABLE_ENCRYPTION) && 0
// Encryption context should apparently not be set on client unless it
// is meant for peer verification
TEST_CASE("testSSLSetup")
{
    auto ttclient = InitTeamTalk();

    EncryptionContext context = {};
    ACE_OS::strsncpy(context.szCertificateFile, ACE_TEXT("ttclientcert.pem"), TT_STRLEN);
    ACE_OS::strsncpy(context.szPrivateKeyFile, ACE_TEXT("ttclientkey.pem"), TT_STRLEN);
    ACE_OS::strsncpy(context.szCAFile, ACE_TEXT("ca.cer"), TT_STRLEN);
    context.bVerifyPeer = FALSE;
    context.bVerifyClientOnce = TRUE;
    context.nVerifyDepth = 0;

    REQUIRE(TT_SetEncryptionContext(ttclient, &context));
    REQUIRE(Connect(ttclient, ACE_TEXT("127.0.0.1"), DEFAULT_ENCRYPTED_TCPPORT, DEFAULT_ENCRYPTED_UDPPORT, TRUE));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
}
#endif

#if 0 // this unit-test is too unstable under Valgrind
TEST_CASE("Last voice packet - wav files")
{
    TTCHAR curdir[1024] = {};
    ACE_OS::getcwd(curdir, 1024);


    //clean up wav files from previous runs
    //they are not deleted after the test, so they are available for inspection afterwards
    ACE_TCHAR delim = ACE_DIRECTORY_SEPARATOR_CHAR;
    ACE_OS::strncat(curdir, &delim, 1);
    ACE_OS::strncat(curdir, ACE_TEXT("wav"), 4);

    ACE_stat fileInfo;
    ACE_DIR* dir = ACE_OS::opendir(curdir);
    if (!dir)
    {
        ACE_OS::mkdir(curdir);
        dir = ACE_OS::opendir(curdir);
    }
    REQUIRE(dir);

    if (ACE_OS::stat(curdir, &fileInfo) == -1)
    {
        ACE_OS::mkdir(curdir);
        dir = ACE_OS::opendir(curdir);
    }
    dirent* dirInfo = ACE_OS::readdir(dir);
    ACE_TCHAR fileToDelete[1024]{};
    do
    {
        if ((ACE_OS::strcmp(dirInfo->d_name, ACE_TEXT(".")) == 0) || (ACE_OS::strcmp(dirInfo->d_name, ACE_TEXT("..")) == 0))
        {
            continue;
        }

        TTCHAR buf[1024]{};
        ACE_OS::strncpy(buf, dirInfo->d_name + ACE_OS::strlen(dirInfo->d_name) - 4, 4);
        int index = ACE_OS::strncmp(buf, ACE_TEXT(".wav"), 4);

        if (index == 0)
        {
            ACE_OS::strncpy(fileToDelete, curdir, 1024);
            ACE_OS::strncat(fileToDelete, &delim, 1);
            ACE_OS::strncat(fileToDelete, dirInfo->d_name, ACE_OS::strlen(dirInfo->d_name));
            ACE_OS::unlink(fileToDelete);
        }


    } while ((dirInfo = ACE_OS::readdir(dir)) != NULL);

    ACE_OS::closedir(dir);

    SoundDevice indev, outdev;
    REQUIRE(GetSoundDevices(indev, outdev));
    class SharedInputReset
    {
        SoundDevice indev;
    public:
        SharedInputReset(SoundDevice in) : indev(in)
        {
            // By default a shared device uses default sample rate, max channels and 40 msec framesize.
            // We reduce it to 20 msec in order to get the 240 msec frame faster.
            REQUIRE(TT_InitSoundInputSharedDevice(indev.nDefaultSampleRate, indev.nMaxInputChannels, int(indev.nDefaultSampleRate * 0.02)));
        }
        ~SharedInputReset()
        {
            // reset back to default
            REQUIRE(TT_InitSoundInputSharedDevice(indev.nDefaultSampleRate, indev.nMaxInputChannels, int(indev.nDefaultSampleRate * 0.04)));
        }
    } a(indev);

    // run multiple times to get the wav output files with different number of frames.
    // running 10 times, gives me 1-4 files with a missing frame, sometimes I get no
    // file with a missing frame, but then you can run this test again or increase the
    // number of runs from 10 to e.g. 20.
    for (int i = 0; i < 10; i++)
    {
        auto txclient = InitTeamTalk();
        auto rxclient = InitTeamTalk();

        REQUIRE(InitSound(txclient, SHARED_INPUT, indev.nDeviceID, outdev.nDeviceID));
        REQUIRE(Connect(txclient));
        REQUIRE(Login(txclient, ACE_TEXT("TxClient")));

        REQUIRE(InitSound(rxclient, SHARED_INPUT, indev.nDeviceID, outdev.nDeviceID));
        REQUIRE(Connect(rxclient));
        REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));

        AudioCodec audiocodec = {};
        audiocodec.nCodec = OPUS_CODEC;
        audiocodec.opus.nApplication = OPUS_APPLICATION_VOIP;
        audiocodec.opus.nTxIntervalMSec = 240;
#if defined(OPUS_FRAMESIZE_120_MS)
        audiocodec.opus.nFrameSizeMSec = 120;
#else
        audiocodec.opus.nFrameSizeMSec = 60;
#endif
        audiocodec.opus.nBitRate = OPUS_MIN_BITRATE;
        audiocodec.opus.nChannels = 2;
        audiocodec.opus.nComplexity = 10;
        audiocodec.opus.nSampleRate = 48000;
        audiocodec.opus.bDTX = true;
        audiocodec.opus.bFEC = true;
        audiocodec.opus.bVBR = false;
        audiocodec.opus.bVBRConstraint = false;

        Channel chan = MakeChannel(txclient, ACE_TEXT("foo"), TT_GetRootChannelID(txclient), audiocodec);
        REQUIRE(WaitForCmdSuccess(txclient, TT_DoJoinChannel(txclient, &chan)));
        REQUIRE(WaitForCmdSuccess(rxclient, TT_DoJoinChannelByID(rxclient, TT_GetMyChannelID(txclient), ACE_TEXT(""))));

        REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, 600));
        REQUIRE(TT_SetUserMediaStorageDir(rxclient, TT_GetMyUserID(txclient), curdir, ACE_TEXT(""), AFF_WAVE_FORMAT));

        auto voicestop = [&](TTMessage msg)
            {
                if (msg.nClientEvent == CLIENTEVENT_USER_STATECHANGE &&
                    msg.user.nUserID == TT_GetMyUserID(txclient) &&
                    (msg.user.uUserState & USERSTATE_VOICE) == 0)
                {
                    return true;
                }

                return false;
            };

        REQUIRE(TT_EnableVoiceTransmission(txclient, true));
        /*
         * A duration which ends on a third of a package size, will produce different wav outputs (files are generated which last 1220 ms (5x nTxIntervalMSec)
         * and files are generated which last 1440ms (6x nTxIntervalMSec)
         */
        int duration = int(audiocodec.opus.nTxIntervalMSec * 5 + audiocodec.opus.nTxIntervalMSec * 0.33);
        WaitForEvent(txclient, CLIENTEVENT_NONE, duration);
        ClientStatistics stats = {};
        REQUIRE(TT_GetClientStatistics(txclient, &stats));
        // take sound device start into account before stopping PTT
        WaitForEvent(txclient, CLIENTEVENT_NONE, stats.nSoundInputDeviceDelayMSec);

        REQUIRE(TT_EnableVoiceTransmission(txclient, false));

        // std::cout << "Initial audio frame delay: " << stats.nSoundInputDeviceDelayMSec << " msec. ";

        REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, voicestop));

        REQUIRE(TT_GetClientStatistics(txclient, &stats));
        // std::cout << "Encoded voice sent: " << stats.nVoiceBytesSent << " bytes. ";

        UserStatistics ustats;
        REQUIRE(TT_GetUserStatistics(rxclient, TT_GetMyUserID(txclient), &ustats));
        // std::cout << "Voice packets received: " << ustats.nVoicePacketsRecv << std::endl;
    }

    //check file sizes. All files should hvae the same size (but some are missing the last frame, so this test fails)
    long long fileSize = -1;
    dir = ACE_OS::opendir(curdir);
    dirInfo = ACE_OS::readdir(dir);
    do
    {
        if ((ACE_OS::strcmp(dirInfo->d_name, ACE_TEXT(".")) == 0) || (ACE_OS::strcmp(dirInfo->d_name, ACE_TEXT("..")) == 0))
        {
            continue;
        }

        TTCHAR buf[1024]{};
        ACE_OS::strncpy(buf, dirInfo->d_name + ACE_OS::strlen(dirInfo->d_name) - 4, 4);
        int index = ACE_OS::strncmp(buf, ACE_TEXT(".wav"), 4);

        ACE_TCHAR fileToCheck[1024]{};
        if (index == 0)
        {
            ACE_OS::strncpy(fileToCheck, curdir, 1024);
            ACE_OS::strncat(fileToCheck, &delim, 1);
            ACE_OS::strncat(fileToCheck, dirInfo->d_name, ACE_OS::strlen(dirInfo->d_name));

            if (ACE_OS::stat(fileToCheck, &fileInfo) != -1)
            {
                if (fileSize == -1)
                {
                    fileSize = fileInfo.st_size;
                }

                REQUIRE(fileSize == fileInfo.st_size);
            }
        }
    } while ((dirInfo = ACE_OS::readdir(dir)) != NULL);

    ACE_OS::closedir(dir);
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
    auto ab = TT_AcquireUserAudioBlock(txclient, STREAMTYPE_VOICE, TT_LOCAL_TX_USERID);
    REQUIRE(ab);
    int streamid = ab->nStreamID;
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
    ttinst ttclient = InitTeamTalk();

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
    ttinst ttclient = InitTeamTalk();

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
    SoundDevice indev, outdev;
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
    ttinst ttclient = InitTeamTalk();

    AudioPreprocessor preprocess = {};

    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;
    preprocess.webrtc.gaincontroller2.bEnable = TRUE;
    preprocess.webrtc.gaincontroller2.fixeddigital.fGainDB = 25;

    preprocess.webrtc.noisesuppression.bEnable = FALSE;
    preprocess.webrtc.noisesuppression.nLevel = 0;

    auto sndloop = TT_StartSoundLoopbackTestEx(indev.nDeviceID, outdev.nDeviceID, indev.nDefaultSampleRate,
                                          1, FALSE, &preprocess, nullptr);
    REQUIRE(sndloop);

    std::cout << "Recording...." << std::endl;

    WaitForEvent(ttclient, CLIENTEVENT_NONE, 30000);

    REQUIRE(TT_CloseSoundLoopbackTest(sndloop));
}

TEST_CASE("WebRTC_SampleRates")
{
    ttinst ttclient = InitTeamTalk();

    AudioPreprocessor preprocess = {};

    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;
    preprocess.webrtc.gaincontroller2.bEnable = TRUE;
    preprocess.webrtc.gaincontroller2.fixeddigital.fGainDB = 25;

    const std::vector<int> standardSampleRates = {8000, 12000, 16000, 24000, 32000, 44100, 48000};

    for (auto samplerate : standardSampleRates)
    {
        auto sndloop = TT_StartSoundLoopbackTestEx(TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL,
                                                   TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL,
                                                   samplerate, 2, TRUE, &preprocess, nullptr);
        REQUIRE(sndloop);

        REQUIRE(TT_CloseSoundLoopbackTest(sndloop));
    }
}

TEST_CASE("WebRTCPreprocessor")
{
    ttinst ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(ttclient));
    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoSubscribe(ttclient, TT_GetMyUserID(ttclient), SUBSCRIBE_VOICE)));

    TTCHAR curdir[1024] = {};
    ACE_OS::getcwd(curdir, 1024);
    REQUIRE(TT_SetUserMediaStorageDir(ttclient, TT_GetMyUserID(ttclient), curdir, ACE_TEXT(""), AFF_WAVE_FORMAT));

    REQUIRE(TT_EnableVoiceTransmission(ttclient, true));
    WaitForEvent(ttclient, CLIENTEVENT_NONE, 5000);

    AudioPreprocessor preprocess = {};

    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;

    preprocess.webrtc.noisesuppression.bEnable = TRUE;
    preprocess.webrtc.noisesuppression.nLevel = 3;

    REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess));
    WaitForEvent(ttclient, CLIENTEVENT_NONE, 5000);

    preprocess.webrtc.gaincontroller2.bEnable = TRUE;
    preprocess.webrtc.gaincontroller2.fixeddigital.fGainDB = 10;

    REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess));
    WaitForEvent(ttclient, CLIENTEVENT_NONE, 5000);

    REQUIRE(TT_EnableVoiceTransmission(ttclient, false));

}

#if 0 /* gain_controller1 doesn't work */
TEST_CASE("WebRTC_gaincontroller1")
{
    ttinst ttclient = InitTeamTalk();
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
    ttinst ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;

    mfp.audioPreprocessor.webrtc.gaincontroller2.bEnable = FALSE;
    mfp.audioPreprocessor.webrtc.gaincontroller2.fixeddigital.fGainDB = 0;

    mfp.audioPreprocessor.webrtc.noisesuppression.bEnable = FALSE;
    mfp.audioPreprocessor.webrtc.noisesuppression.nLevel = 3;

    TTCHAR filename[TT_STRLEN] = ACE_TEXT("testdata/AGC/input_16k_mono_low.wav");

    auto session = TT_InitLocalPlayback(ttclient, filename, &mfp);
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
                mfp.audioPreprocessor.webrtc.gaincontroller2.bEnable = TRUE;
                mfp.audioPreprocessor.webrtc.gaincontroller2.fixeddigital.fGainDB = 25;
                REQUIRE(TT_UpdateLocalPlayback(ttclient, session, &mfp));
                toggled = true;
                // std::cout << "Toggled: " << msg.mediafileinfo.uElapsedMSec << std::endl;
            }
            if (msg.mediafileinfo.uElapsedMSec >= 10000)
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
    ttinst ttclient = InitTeamTalk();
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
    codec.speex_vbr.nBitRate = 16000;
    codec.speex_vbr.nMaxBitRate = SPEEX_UWB_MAX_BITRATE;
    codec.speex_vbr.nQuality = 10;
    codec.speex_vbr.nTxIntervalMSec = 40;
    Channel chan = MakeChannel(ttclient, ACE_TEXT("speex"), TT_GetRootChannelID(ttclient), codec);
    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoJoinChannel(ttclient, &chan)));

    AudioPreprocessor preprocess = {};

    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;

    switch (preprocess.nPreprocessor)
    {
    case WEBRTC_AUDIOPREPROCESSOR :
        preprocess.webrtc.echocanceller.bEnable = TRUE;

        preprocess.webrtc.noisesuppression.bEnable = TRUE;
        preprocess.webrtc.noisesuppression.nLevel = 2;

        preprocess.webrtc.gaincontroller2.bEnable = TRUE;
        preprocess.webrtc.gaincontroller2.fixeddigital.fGainDB = 25;
        break;
    case SPEEXDSP_AUDIOPREPROCESSOR :
#define DEFAULT_AGC_ENABLE              TRUE
#define DEFAULT_AGC_GAINLEVEL           8000
#define DEFAULT_AGC_INC_MAXDB           12
#define DEFAULT_AGC_DEC_MAXDB           -40
#define DEFAULT_AGC_GAINMAXDB           30
#define DEFAULT_DENOISE_ENABLE          TRUE
#define DEFAULT_DENOISE_SUPPRESS        -30
#define DEFAULT_ECHO_ENABLE             TRUE
#define DEFAULT_ECHO_SUPPRESS           -40
#define DEFAULT_ECHO_SUPPRESSACTIVE     -15
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
    case NO_AUDIOPREPROCESSOR :
    case TEAMTALK_AUDIOPREPROCESSOR :
        REQUIRE(false);
        break;
    }

    REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess));

    REQUIRE(TT_EnableVoiceTransmission(ttclient, true));

    WaitForEvent(ttclient, CLIENTEVENT_NONE, 5000);
}

TEST_CASE("WebRTC_Preamplifier")
{
    ttinst ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(ttclient));

    int level = 0;
    
    REQUIRE(TT_EnableVoiceTransmission(ttclient, TRUE));
    REQUIRE(TT_DBG_SetSoundInputTone(ttclient, STREAMTYPE_VOICE, 300));
    REQUIRE(TT_EnableAudioBlockEvent(ttclient, TT_LOCAL_TX_USERID, STREAMTYPE_VOICE, TRUE));

    // no gain
    AudioPreprocessor preprocess = {};
    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;
    REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess));

    TTMessage msg = {};
    int streamid = 0;
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
    auto ab = TT_AcquireUserAudioBlock(ttclient, STREAMTYPE_VOICE, TT_LOCAL_TX_USERID);
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
    preprocess.webrtc.preamplifier.fFixedGainFactor = .5f;
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

TEST_CASE("WebRTC_LevelEstimation")
{
    ttinst ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(ttclient));

    REQUIRE(TT_DBG_SetSoundInputTone(ttclient, STREAMTYPE_VOICE, 500));

    REQUIRE(TT_EnableAudioBlockEvent(ttclient, TT_LOCAL_USERID, STREAMTYPE_VOICE, TRUE));

    AudioPreprocessor preprocess = {};
    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;
    preprocess.webrtc.levelestimation.bEnable = TRUE;
    REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess));
    int n = 10;
    do
    {
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK));
        auto ab = TT_AcquireUserAudioBlock(ttclient, STREAMTYPE_VOICE, TT_LOCAL_USERID);
        REQUIRE(ab);
        REQUIRE(TT_ReleaseUserAudioBlock(ttclient, ab));
        REQUIRE(TT_GetSoundInputLevel(ttclient) >= 88);
    } while (n-- > 0);
}

TEST_CASE("WebRTC_VAD")
{
    ttinst ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(ttclient));

    AudioPreprocessor preprocess = {};
    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;
    preprocess.webrtc.voicedetection.bEnable = TRUE;
    REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess));

    REQUIRE(TT_SetVoiceActivationStopDelay(ttclient, 200));
    REQUIRE(TT_DBG_SetSoundInputTone(ttclient, STREAMTYPE_VOICE, 500));

    REQUIRE(TT_EnableVoiceActivation(ttclient, TRUE));
    TTMessage msg = {};
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_VOICE_ACTIVATION, msg));
    REQUIRE(msg.bActive);

    REQUIRE(TT_DBG_SetSoundInputTone(ttclient, STREAMTYPE_VOICE, 0));
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_VOICE_ACTIVATION, msg));
    REQUIRE(!msg.bActive);
}

TEST_CASE("WebRTC-reinit")
{
    ttinst ttclient = InitTeamTalk();
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
    audiocodec.opus.nTxIntervalMSec = 240;
#if defined(OPUS_FRAMESIZE_120_MS)
    audiocodec.opus.nFrameSizeMSec = 120;
#else
    audiocodec.opus.nFrameSizeMSec = 40;
#endif
    audiocodec.opus.nBitRate = OPUS_MIN_BITRATE;
    audiocodec.opus.nChannels = 2;
    audiocodec.opus.nComplexity = 10;
    audiocodec.opus.nSampleRate = 48000;
    audiocodec.opus.bDTX = true;
    audiocodec.opus.bFEC = true;
    audiocodec.opus.bVBR = false;
    audiocodec.opus.bVBRConstraint = false;

    Channel chan = MakeChannel(ttclient, ACE_TEXT("speex"), TT_GetRootChannelID(ttclient), audiocodec);
    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoJoinChannel(ttclient, &chan)));

    AudioPreprocessor preprocess = {};

    preprocess.nPreprocessor = WEBRTC_AUDIOPREPROCESSOR;
    preprocess.webrtc.echocanceller.bEnable = TRUE;

    preprocess.webrtc.noisesuppression.bEnable = TRUE;
    preprocess.webrtc.noisesuppression.nLevel = 2;

    preprocess.webrtc.gaincontroller2.bEnable = TRUE;
    preprocess.webrtc.gaincontroller2.fixeddigital.fGainDB = 25;


    AudioPreprocessor preprocess_reinit = {};
    preprocess_reinit.nPreprocessor = NO_AUDIOPREPROCESSOR;

    for (int i = 0; i < 100; i++)
    {
        REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess));
        WaitForEvent(ttclient, CLIENTEVENT_NONE, 20);

        REQUIRE(TT_SetSoundInputPreprocessEx(ttclient, &preprocess_reinit));
        WaitForEvent(ttclient, CLIENTEVENT_NONE, 20);

    }
}
#endif /* ENABLE_WEBRTC */

TEST_CASE("TeamTalk_VAD")
{
    ttinst ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(ttclient));

    REQUIRE(TT_EnableVoiceActivation(ttclient, TRUE));
    REQUIRE(TT_SetVoiceActivationLevel(ttclient, 63));
    REQUIRE(TT_SetVoiceActivationStopDelay(ttclient, 100));

    TTMessage msg = {};
    REQUIRE(TT_DBG_SetSoundInputTone(ttclient, STREAMTYPE_VOICE, 500));
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
    err = Pa_OpenStream(&outstream, nullptr, &outputParameters,
                        ininfo->defaultSampleRate, uint32_t(ininfo->defaultSampleRate * .04),
                        paClipOff, Foo_StreamCallback, static_cast<void*> (0));

    REQUIRE(Pa_StartStream(outstream) == paNoError);
    while (paSamples < ininfo->defaultSampleRate * 2)
    {
        Pa_Sleep(1000);

        auto samplesDurationMSec = PCM16_SAMPLES_DURATION(paSamples, int(ininfo->defaultSampleRate));
        auto durationMSec = GETTIMESTAMP() - paTimeStamp;
        auto skew = int(samplesDurationMSec - durationMSec);
        // std::cout << "Samples duration: " << samplesDurationMSec << " / " << durationMSec << "  " << skew << std::endl;

        REQUIRE(skew < 0.08 * 1000);
    }

    Pa_Terminate();
}

TEST_CASE("PortAudio_SamplesPerSec")
{
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
    const int SAMPLERATE = 48000, CHANNELS = 1;

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

    REQUIRE(snd->OpenOutputStream(&player, outputdeviceid, grp, SAMPLERATE, CHANNELS, uint32_t(SAMPLERATE * 0.04)));
    REQUIRE(snd->StartStream(&player));

    while (samples < uint32_t(outdev.default_samplerate * 2))
    {
        Pa_Sleep(1000);

        auto samplesDurationMSec = PCM16_SAMPLES_DURATION(samples, int(outdev.default_samplerate));
        auto durationMSec = GETTIMESTAMP() - starttime;
        auto skew = int(samplesDurationMSec - durationMSec);

        // std::cout << "Samples duration: " << samplesDurationMSec << " / " << durationMSec << "  " << skew << std::endl;

        REQUIRE(skew < 0.08 * 1000);
    }
    REQUIRE(snd->CloseOutputStream(&player));
    snd->RemoveSoundGroup(grp);
}

#endif

TEST_CASE("InjectAudio")
{
    ttinst ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient, DEFAULT, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(ttclient));

    REQUIRE(WaitForCmdSuccess(ttclient, TT_DoSubscribe(ttclient, TT_GetMyUserID(ttclient), SUBSCRIBE_VOICE)));

    int SAMPLERATE = 48000, CHANNELS = 2;
    std::vector<short> buf(SAMPLERATE * CHANNELS);
    AudioBlock ab = {};
    ab.nStreamID = 1;
    ab.nSampleRate = SAMPLERATE;
    ab.nChannels = CHANNELS;
    ab.lpRawAudio = &buf[0];
    ab.nSamples = SAMPLERATE;

    // 3 secs
    int samples = SAMPLERATE * 3;

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
    ttinst ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient, DEFAULT, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(ttclient));

    Channel chan = {};
    REQUIRE(TT_GetChannel(ttclient, TT_GetMyChannelID(ttclient), &chan));
    REQUIRE(chan.audiocodec.nCodec == OPUS_CODEC);
    auto const N_FRAMES = 5;
    int samples = PCM16_DURATION_SAMPLES(chan.audiocodec.opus.nTxIntervalMSec, chan.audiocodec.opus.nSampleRate) * N_FRAMES;
    std::vector<short> buffer(samples * chan.audiocodec.opus.nChannels);
    media::AudioFrame frm(media::AudioFormat(chan.audiocodec.opus.nSampleRate, chan.audiocodec.opus.nChannels), &buffer[0], samples);
    int sampleindex = GenerateTone(frm, 0, 500);

    AudioBlock ab = {};
    ab.lpRawAudio = frm.input_buffer;
    ab.nChannels = frm.inputfmt.channels;
    ab.nSampleRate = frm.inputfmt.samplerate;
    ab.uSampleIndex = frm.sample_no;
    ab.nSamples = frm.input_samples;

    REQUIRE(TT_EnableAudioBlockEvent(ttclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));
    REQUIRE(TT_InsertAudioBlock(ttclient, &ab));

    // wait for 'ab' to appear
    int sumsamples_pregain;
    while ((sumsamples_pregain = GetAudioBlockSamplesSum(ttclient, TT_MUXED_USERID, STREAMTYPE_VOICE)) == 0);
    int recv_frames = 1;
    for (;recv_frames < N_FRAMES;++recv_frames)
        sumsamples_pregain += GetAudioBlockSamplesSum(ttclient, TT_MUXED_USERID, STREAMTYPE_VOICE);

    // AudioMuxer underflow
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, chan.audiocodec.opus.nTxIntervalMSec * 2) == FALSE);

    // double gain
    REQUIRE(TT_SetSoundInputGainLevel(ttclient, TT_GetSoundInputGainLevel(ttclient) * 2));
    ab.uSampleIndex = ab.nSamples;
    REQUIRE(TT_InsertAudioBlock(ttclient, &ab));

    int sumsamples_postgain = 0;
    for (recv_frames=0;recv_frames < N_FRAMES;++recv_frames)
        sumsamples_postgain += GetAudioBlockSamplesSum(ttclient, TT_MUXED_USERID, STREAMTYPE_VOICE);

    REQUIRE(sumsamples_pregain * 1.9 < sumsamples_postgain);
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

    uint32_t fixeddelay = 240;

    JitterConfig jitterconf{};
    jitterconf.nFixedDelayMSec = fixeddelay;
    jitterconf.bUseAdativeDejitter = false;
    jitterconf.nMaxAdaptiveDelayMSec = 10000;

    TT_SetUserJitterControl(rxclient, TT_GetMyUserID(txclient), STREAMTYPE_VOICE, &jitterconf);

    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, 500));
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));

    auto voicestart = [&](TTMessage msg)
    {
        if (msg.nClientEvent == CLIENTEVENT_USER_STATECHANGE &&
            msg.user.nUserID == TT_GetMyUserID(txclient) &&
            (msg.user.uUserState & USERSTATE_VOICE) == USERSTATE_VOICE)
        {
            return true;
        }

        return false;
    };

    uint32_t starttime = GETTIMESTAMP();
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, voicestart));

    uint32_t endtime = GETTIMESTAMP();
    uint32_t delay = (endtime - starttime);
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

    uint32_t fixeddelay = 240;

    JitterConfig jitterconf_in{};
    jitterconf_in.nFixedDelayMSec = fixeddelay;
    jitterconf_in.bUseAdativeDejitter = false;
    jitterconf_in.nMaxAdaptiveDelayMSec = 10000;
    jitterconf_in.nActiveAdaptiveDelayMSec = 800;

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
        if (devs[i].vidcapformats.size())
            dev = devs[i];
    }

    if (dev.deviceid.empty())
        return;

    auto capformat = dev.vidcapformats.at(0);

    std::condition_variable cv;

    int frames = 10;
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
    vid.webm_vp8.nRcTargetBitrate = 128;
    vid.webm_vp8.nEncodeDeadline = WEBM_VPX_DL_REALTIME;

    TTCHAR filename[TT_STRLEN] = ACE_TEXT("testdata/Video/MOV03830.MPG");

    REQUIRE(TT_StartStreamingMediaFileToChannel(txclient, filename, &vid));

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
            auto vb = TT_AcquireUserMediaVideoFrame(rxclient, msg.nSource);
            if (vb)
            {
                REQUIRE(TT_ReleaseUserMediaVideoFrame(rxclient, vb));
            }
        }
    }
}
#endif /* ENABLE_VPX */

TEST_CASE("ReactorDeadlock_BUG")
{
    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = 2;
    mfi.audioFmt.nSampleRate = 48000;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("temp.wav"), TT_STRLEN);
    mfi.uDurationMSec = 1000;
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 500));
    
    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
    mfp.bPaused = FALSE;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    TTMessage msg;
    INT32 playid;

    ttinst ttclient(TT_InitTeamTalkPoll());
    REQUIRE(InitSound(ttclient));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(ttclient));
    playid = TT_InitLocalPlayback(ttclient, ACE_TEXT("temp.wav"), &mfp);
    REQUIRE(playid > 0);
    for (int i=0;i<10;++i)
    {
        REQUIRE(TT_InitLocalPlayback(ttclient, ACE_TEXT("temp.wav"), &mfp) > 0);
        WaitForEvent(ttclient, CLIENTEVENT_NONE, 10);
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
    std::vector<MediaFileInfo> mfis;
    for (int duration : {10, 20, 50, 80, 120})
    {
        MediaFileInfo mfi = {};
        mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
        mfi.audioFmt.nChannels = 2;
        mfi.audioFmt.nSampleRate = 48000;
        ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("temp_%d.wav"), duration);
        mfi.uDurationMSec = duration;
        REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 500));
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
        INT32 playid;
        bool done = false, gotmsg;
        int started = 0;
        int waittime = DEFWAIT;

        playid = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);
        REQUIRE(playid > 0);
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
                waittime = mfi.uDurationMSec * 2;
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
    const auto IN_SAMPLERATE = 48000;
    const auto IN_CHANNELS = 2;
    const auto IN_FRAMESIZE = int(IN_SAMPLERATE * .01);

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = IN_CHANNELS;
    mfi.audioFmt.nSampleRate = IN_SAMPLERATE;
    mfi.uDurationMSec = 100;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("TTPlayOpusOgg.wav"), TT_STRLEN);
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 500));

    ttinst ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
    mfp.bPaused = FALSE;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

    bool stop = false, started = false, paused = false;
    TTMessage msg;
    INT32 session;
    uint32_t durationMSec = GETTIMESTAMP(), startupMSec = GETTIMESTAMP();

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

    ttinst ttclient(TT_InitTeamTalkPoll());
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

        int TimerEvent(ACE_UINT32 , long ) override
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
    mfi.audioFmt.nChannels = 2;
    mfi.audioFmt.nSampleRate = 48000;
    mfi.uDurationMSec = 10 * 1000;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("streamfile.wav"), TT_STRLEN);
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 500));

    REQUIRE(TT_EnableAudioBlockEvent(txclient, TT_LOCAL_USERID, STREAMTYPE_MEDIAFILE_AUDIO, TRUE));
    REQUIRE(TT_StartStreamingMediaFileToChannel(txclient, mfi.szFileName, nullptr));

    TTMessage msg;
    REQUIRE(WaitForEvent(txclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
    REQUIRE(msg.nStreamType == STREAMTYPE_MEDIAFILE_AUDIO);
    REQUIRE(msg.nSource == TT_LOCAL_USERID);
    auto ab = TT_AcquireUserAudioBlock(txclient, STREAMTYPE_MEDIAFILE_AUDIO, TT_LOCAL_USERID);
    REQUIRE(ab);
    REQUIRE(TT_ReleaseUserAudioBlock(txclient, ab));
}

TEST_CASE("LocalPlaybackToAudioBlock")
{
    auto txclient = InitTeamTalk();
    REQUIRE(InitSound(txclient));

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = 2;
    mfi.audioFmt.nSampleRate = 48000;
    mfi.uDurationMSec = 100 * 1000;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("playbackfile.wav"), TT_STRLEN);
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 500));

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
    auto ab = TT_AcquireUserAudioBlock(txclient, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, sessionid);
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
    for (int i=0;i<10;i++)
    {
        sessionid = TT_InitLocalPlayback(txclient, mfi.szFileName, &mfp);
        REQUIRE(sessionid > 0);
        sessions.insert(sessionid);
        allsessions.insert(sessionid);
    }

    while (sessions.size() && WaitForEvent(txclient, CLIENTEVENT_USER_AUDIOBLOCK, msg))
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
    auto txclient = InitTeamTalk();
    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
    REQUIRE(JoinRoot(txclient));

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = 2;
    mfi.audioFmt.nSampleRate = 48000;
    mfi.uDurationMSec = 1000;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("playbackfile.wav"), TT_STRLEN);
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 500));

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
    auto ttclient = InitTeamTalk();
    InitSound(ttclient, SHARED_INPUT_OUTPUT);

    MediaFileInfo mfi = {};;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("hest.wav"), TT_STRLEN);;
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nSampleRate = 48000;
    mfi.audioFmt.nChannels = 2;
    mfi.uDurationMSec = 1000;
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 600));

    // Call TT_InitLocalPlayback for file 1, PAUSE=FALSE
    MediaFilePlayback mfp = {};
    mfp.bPaused = false;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

    int sessionid = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);
    REQUIRE(sessionid > 0);

    TTMessage msg;
    while (WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, DEFWAIT) &&
           msg.mediafileinfo.nStatus != MFS_FINISHED);

    // Call TT_InitLocalPlayback for file 1, PAUSE=TRUE (session = X)
    mfp.bPaused = true;
    sessionid = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);

    // Call TT_InitLocalPlayback for file 2, PAUSE=FALSE
    mfp.bPaused = false;
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
    mfp.bPaused = true;
    sessionid2 = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);

    // Call TT_UpdateLocalPlayback for session X => Crash
    mfp.bPaused = false;
    REQUIRE(TT_UpdateLocalPlayback(ttclient, sessionid, &mfp));
    while (WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg, DEFWAIT)) {
        if (msg.nSource == sessionid && msg.mediafileinfo.nStatus == MFS_FINISHED)
            break;
    }
}

TEST_CASE("LocalPlaybackOnOffSound")
{
    auto ttclient = InitTeamTalk();
    InitSound(ttclient);

    // Call TT_InitLocalPlayback for file 1, PAUSE=FALSE
    MediaFilePlayback mfp = {};
    mfp.bPaused = false;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    int i = 1;
    while (i--)
    {
        int onid = TT_InitLocalPlayback(ttclient, ACE_TEXT("testdata/Opus/on.ogg"), &mfp);
        REQUIRE(onid > 0);
        WaitForEvent(ttclient, CLIENTEVENT_NONE, 1000);
        int offid = TT_InitLocalPlayback(ttclient, ACE_TEXT("testdata/Opus/off.ogg"), &mfp);
        REQUIRE(offid > 0);
        WaitForEvent(ttclient, CLIENTEVENT_NONE, 1000);
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
    mfp.bPaused = true;
    int onid = TT_InitLocalPlayback(ttclient, ACE_TEXT("testdata/Opus/on.ogg"), &mfp);
    REQUIRE(onid > 0);

    int offid = TT_InitLocalPlayback(ttclient, ACE_TEXT("testdata/Opus/off.ogg"), &mfp);
    REQUIRE(offid > 0);

    int i = 20;
    while (i--)
    {
        // Play cached on.ogg
        mfp.bPaused = false;
        REQUIRE(TT_UpdateLocalPlayback(ttclient, onid, &mfp));

        while (WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg) && msg.mediafileinfo.nStatus != MFS_FINISHED);
        REQUIRE(msg.nSource == onid);

        // cache on.ogg for next playback
        mfp.bPaused = true;
        onid = TT_InitLocalPlayback(ttclient, ACE_TEXT("testdata/Opus/on.ogg"), &mfp);
        REQUIRE(onid > 0);

        WaitForEvent(ttclient, CLIENTEVENT_NONE, msg, 1000);

        // Play cached off.ogg
        mfp.bPaused = false;
        REQUIRE(TT_UpdateLocalPlayback(ttclient, offid, &mfp));

        while (WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg) && msg.mediafileinfo.nStatus != MFS_FINISHED);
        REQUIRE(msg.nSource == offid);

        // cache off.ogg for next playback
        mfp.bPaused = true;
        offid = TT_InitLocalPlayback(ttclient, ACE_TEXT("testdata/Opus/off.ogg"), &mfp);
        REQUIRE(offid > 0);

        WaitForEvent(ttclient, CLIENTEVENT_NONE, msg, 1000);
    }
}

TEST_CASE("LocalPlaybackLatency")
{
    // Call TT_InitLocalPlayback for file 1, PAUSE=FALSE
    MediaFilePlayback mfp = {};
    mfp.bPaused = false;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

    MediaFileInfo mfi;
    REQUIRE(TT_GetMediaFileInfo(ACE_TEXT("testdata/Opus/broadcast_msg.ogg"), &mfi));
    //
    {
        auto ttclient = InitTeamTalk();
        REQUIRE(InitSound(ttclient, SHARED_INPUT_OUTPUT));

        auto starttime = GETTIMESTAMP();
        int onid = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);
        REQUIRE(onid > 0);
        REQUIRE(TT_EnableAudioBlockEvent(ttclient, onid, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, TRUE));

        TTMessage msg;
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));

        std::cout << "Latency for SHARED_INPUT_OUTPUT: " << GETTIMESTAMP() - starttime << std::endl;
    }
    {
        auto ttclient = InitTeamTalk();
        // force non shared device mode on Android
#if defined(__ANDROID__)
        // InitSound with DEFAULT switches to shared-mode on android, so force device selection on Android
        REQUIRE(InitSound(ttclient, DEFAULT, TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT, TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT));
#else
        REQUIRE(InitSound(ttclient, DEFAULT));
#endif

        auto starttime = GETTIMESTAMP();
        int onid = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);
        REQUIRE(onid > 0);
        REQUIRE(TT_EnableAudioBlockEvent(ttclient, onid, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, TRUE));

        TTMessage msg;
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));

        std::cout << "Latency for DEFAULT: " << GETTIMESTAMP() - starttime << std::endl;
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
        if (msg.nClientEvent == CLIENTEVENT_USER_FIRSTVOICESTREAMPACKET &&
            msg.user.nUserID == TT_GetMyUserID(txclient))
        {
            return true;
        }

        return false;
    };

    auto voicestart = [&](TTMessage msg)
    {
        if (msg.nClientEvent == CLIENTEVENT_USER_STATECHANGE &&
            msg.user.nUserID == TT_GetMyUserID(txclient) &&
            (msg.user.uUserState & USERSTATE_VOICE) == USERSTATE_VOICE)
        {
            return true;
        }

        return false;
    };

    //Set fixed Jitter buffer config
    uint32_t fixeddelay = 240;
    JitterConfig jitterconf = {};
    jitterconf.nFixedDelayMSec = fixeddelay;
    jitterconf.bUseAdativeDejitter = false;
    jitterconf.nMaxAdaptiveDelayMSec = 10000;
    jitterconf.nActiveAdaptiveDelayMSec = 800;

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

    //Set fixed + adaptive Jitter buffer config but don't allow adaptive jitter control
    uint32_t adaptivedelay = 760;
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
    jitterconf.bUseAdativeDejitter = true;

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
    chan.nMaxUsers = 100;
    chan.nDiskQuota = 1024*1024;
    REQUIRE(WaitForCmdSuccess(admin, TT_DoMakeChannel(admin, &chan)));

    int chanid = TT_GetChannelIDFromPath(admin, ACE_TEXT("SeeFilesAfterMove"));
    REQUIRE(chanid > 0);

    TTCHAR filename[TT_STRLEN] = ACE_TEXT("testdata/Opus/giana.ogg");

    REQUIRE(WaitForCmdSuccess(admin, TT_DoSendFile(admin, chanid, filename)));

    REQUIRE(WaitForEvent(admin, CLIENTEVENT_CMD_FILE_NEW));

    REQUIRE(WaitForCmdSuccess(admin, TT_DoMoveUser(admin, TT_GetMyUserID(ttclient), chanid)));

    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_CMD_FILE_NEW));
}

#if defined(ENABLE_OPUSTOOLS) && defined(ENABLE_OPUS)

void CreateOpusFile(const MediaFileInfo& mfi, const ACE_TString& oggfilename, const int FRAMESIZE)
{
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 500));

    WavePCMFile wavfile;
    REQUIRE(wavfile.OpenFile(mfi.szFileName, true));

    OpusEncFile opusenc;
    REQUIRE(opusenc.Open(oggfilename, mfi.audioFmt.nChannels, mfi.audioFmt.nSampleRate, FRAMESIZE, OPUS_APPLICATION_AUDIO));

    std::vector<short> buf(mfi.audioFmt.nChannels * FRAMESIZE);
    int samples;
    while ((samples = wavfile.ReadSamples(&buf[0], FRAMESIZE)) > 0)
    {
        REQUIRE(opusenc.Encode(&buf[0], FRAMESIZE, samples != FRAMESIZE) >= 0);
    }
    opusenc.Close();
    wavfile.Close();
}

TEST_CASE("OPUSFileEncDec")
{
    for (auto SAMPLERATE : {8000, 12000, 24000, 48000})
    {
        for (auto FRAMESIZE_SEC : {.0025, .005, .01, .02, .04, .06, .08, .1, .12})
        {
            MediaFileInfo mfi = {};
            mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
            mfi.audioFmt.nChannels = 2;
            mfi.audioFmt.nSampleRate = SAMPLERATE;
            mfi.uDurationMSec = 10 * 1000;
            const int FRAMESIZE = int(mfi.audioFmt.nSampleRate * FRAMESIZE_SEC);
            ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("originalfile_%d_%dmsec.wav"),
                             mfi.audioFmt.nSampleRate, PCM16_SAMPLES_DURATION(FRAMESIZE, mfi.audioFmt.nSampleRate));

            REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 500));

            WavePCMFile wavfile;
            REQUIRE(wavfile.OpenFile(mfi.szFileName, true));

            OpusEncFile opusenc;
            ACE_TCHAR opusencfilename[TT_STRLEN];
            ACE_OS::snprintf(opusencfilename, TT_STRLEN, ACE_TEXT("opusencfile_%d_%dmsec.ogg"),
                             mfi.audioFmt.nSampleRate, PCM16_SAMPLES_DURATION(FRAMESIZE, mfi.audioFmt.nSampleRate));
            REQUIRE(opusenc.Open(opusencfilename, mfi.audioFmt.nChannels, mfi.audioFmt.nSampleRate, FRAMESIZE, OPUS_APPLICATION_AUDIO));

            std::vector<short> buf(mfi.audioFmt.nChannels * FRAMESIZE);
            int samples;
            while ((samples = wavfile.ReadSamples(&buf[0], FRAMESIZE)) > 0)
            {
                REQUIRE(opusenc.Encode(&buf[0], FRAMESIZE, samples != FRAMESIZE) >= 0);
            }
            opusenc.Close();
            wavfile.Close();

            OpusFile opusread;
            REQUIRE(opusread.OpenFile(opusencfilename));

            OpusDecode opusdec;
            REQUIRE(opusdec.Open(opusread.GetSampleRate(), opusread.GetChannels()));

            ACE_TCHAR opusdecfilename[TT_STRLEN];
            ACE_OS::snprintf(opusdecfilename, TT_STRLEN, ACE_TEXT("opusdecfile_%d_%dmsec.wav"),
                             mfi.audioFmt.nSampleRate, PCM16_SAMPLES_DURATION(FRAMESIZE, mfi.audioFmt.nSampleRate));
            REQUIRE(wavfile.NewFile(opusdecfilename, opusread.GetSampleRate(), opusread.GetChannels()));

            ogg_int64_t samplesduration = 0;
            while (true)
            {
                int bytes;
                auto opusbuf = opusread.ReadEncoded(bytes, &samplesduration);
                if (!opusbuf)
                    break;

                REQUIRE(opusdec.Decode(reinterpret_cast<const char*>(opusbuf), bytes, &buf[0], FRAMESIZE) == FRAMESIZE);
                wavfile.AppendSamples(&buf[0], FRAMESIZE);
            }

            auto durationmsec = PCM16_SAMPLES_DURATION(samplesduration, mfi.audioFmt.nSampleRate);
            // std::cout << SAMPLERATE << "@" << FRAMESIZE_SEC << " diff: " << mfi.uDurationMSec - durationmsec << std::endl;
            REQUIRE(std::abs(durationmsec - mfi.uDurationMSec) <= FRAMESIZE_SEC * 1000);
        }
    }
}

TEST_CASE("OPUSFileSeek")
{
    const auto SAMPLERATE = 12000;
    const auto FRAMESIZE_SEC = .04;
    const auto CHANNELS = 2;
    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = CHANNELS;
    mfi.audioFmt.nSampleRate = SAMPLERATE;
    mfi.uDurationMSec = 10 * 1000;

    const int FRAMESIZE = int(mfi.audioFmt.nSampleRate * FRAMESIZE_SEC);
    ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("orgseekfile_%d_%dmsec.wav"),
                     mfi.audioFmt.nSampleRate, PCM16_SAMPLES_DURATION(FRAMESIZE, mfi.audioFmt.nSampleRate));

    ACE_TCHAR opusencfilename[TT_STRLEN];
    ACE_OS::snprintf(opusencfilename, TT_STRLEN, ACE_TEXT("opusseekfile_%d_%dmsec.ogg"),
                     mfi.audioFmt.nSampleRate, PCM16_SAMPLES_DURATION(FRAMESIZE, mfi.audioFmt.nSampleRate));

    CreateOpusFile(mfi, opusencfilename, FRAMESIZE);

    // ensure we come back to file's origin after search
    OggFile of, of1;
    ogg_page og, og1;
    REQUIRE(of.Open(opusencfilename));
    REQUIRE(of1.Open(opusencfilename));
    int count = 10;
    while(count--)
    {
        REQUIRE(of.ReadOggPage(og) == 1);
        REQUIRE(of1.ReadOggPage(og1) == 1);
    }
    REQUIRE(of.LastGranulePos() == 48000 * mfi.uDurationMSec / 1000);
    REQUIRE(of.ReadOggPage(og) == 1);
    REQUIRE(of1.ReadOggPage(og1) == 1);
    REQUIRE(ogg_page_granulepos(&og) == ogg_page_granulepos(&og1));

    // validate frame-size and total number of samples
    OpusFile opfile;
    REQUIRE(opfile.OpenFile(opusencfilename));
    REQUIRE(PCM16_SAMPLES_DURATION(FRAMESIZE, opfile.GetSampleRate()) == 1000 * FRAMESIZE_SEC);
    double duration_sec = mfi.uDurationMSec / 1000.;
    REQUIRE(opfile.GetTotalSamples() == SAMPLERATE * duration_sec);

    ogg_int64_t halfsamples = ogg_int64_t((duration_sec * mfi.audioFmt.nSampleRate) / 2);
    REQUIRE(opfile.Seek(halfsamples));
    ogg_int64_t samplesduration;
    int bytes;
    REQUIRE(opfile.ReadEncoded(bytes, &samplesduration));
    REQUIRE(std::abs(halfsamples - samplesduration) <= FRAMESIZE);

    // seek using log2
    REQUIRE(of.Seek((FRAMESIZE * 10) * (48000 / SAMPLERATE), og));
    REQUIRE(of.CurrentGranulePos() / (48000 / SAMPLERATE) == FRAMESIZE * 10);
    REQUIRE(of1.SeekLog2((FRAMESIZE * 10) * (48000 / SAMPLERATE), og));
    REQUIRE(of1.CurrentGranulePos() / (48000 / SAMPLERATE) == FRAMESIZE * 10);
    REQUIRE(of.LastGranulePos() == of1.LastGranulePosLog2());
    REQUIRE(of.Seek(((FRAMESIZE * 10) + FRAMESIZE / 3) * (48000 / SAMPLERATE), og));
    REQUIRE(of1.SeekLog2(((FRAMESIZE * 10) + FRAMESIZE / 3) * (48000 / SAMPLERATE), og));
    REQUIRE(of.LastGranulePos() == of1.LastGranulePosLog2());
    std::vector<uint32_t> seekpositions = { uint32_t(mfi.uDurationMSec * 0.31),
                                            uint32_t(mfi.uDurationMSec * 0.0),
                                            uint32_t(mfi.uDurationMSec * 1.0),
                                            uint32_t(mfi.uDurationMSec * 0.777),
                                            uint32_t(mfi.uDurationMSec * 0.333),
                                            uint32_t(mfi.uDurationMSec * 1.0),
                                            uint32_t(mfi.uDurationMSec * 0.0)};

    for (uint32_t sp : seekpositions)
    {
        REQUIRE(of.Seek(48000 * sp / 1000, og));
        REQUIRE(of1.SeekLog2(48000 * sp / 1000, og));
        REQUIRE(of.LastGranulePos() == of1.LastGranulePosLog2());
    }

    // decode from 0% onwards
    OpusDecFile opusdecfile;
    REQUIRE(opusdecfile.Open(opusencfilename));
    std::vector<short> frame(FRAMESIZE * CHANNELS);
    int frames = 0;
    while (opusdecfile.Decode(&frame[0], FRAMESIZE) == FRAMESIZE)frames++;
    uint32_t duration_msec = PCM16_SAMPLES_DURATION(frames * FRAMESIZE, SAMPLERATE);
    REQUIRE(duration_msec == mfi.uDurationMSec);
    REQUIRE(opusdecfile.GetDurationMSec() == mfi.uDurationMSec);
    opusdecfile.Close();

    // decode from 90% onwards
    REQUIRE(opusdecfile.Open(opusencfilename));
    auto offset_msec = .9 * mfi.uDurationMSec;
    REQUIRE(opusdecfile.Seek(uint32_t(offset_msec)));
    frames = 0;
    while (opusdecfile.Decode(&frame[0], FRAMESIZE) == FRAMESIZE)frames++;
    duration_msec = PCM16_SAMPLES_DURATION(frames * FRAMESIZE, SAMPLERATE);
    REQUIRE(duration_msec == mfi.uDurationMSec - offset_msec);
    REQUIRE(opusdecfile.GetDurationMSec() == mfi.uDurationMSec);

    // check special handling of seeking to granule position 0 in Ogg file
    REQUIRE(opusdecfile.Seek(0));
    frames = 0;
    while (opusdecfile.Decode(&frame[0], FRAMESIZE) == FRAMESIZE)frames++;
    duration_msec = PCM16_SAMPLES_DURATION(frames * FRAMESIZE, SAMPLERATE);
    REQUIRE(duration_msec == mfi.uDurationMSec);
    REQUIRE(opusdecfile.GetDurationMSec() == mfi.uDurationMSec);

    // check seek offsets
    REQUIRE(opusdecfile.Seek(555));
    auto diffMSec = opusdecfile.GetElapsedMSec() - (555 / int(FRAMESIZE_SEC * 1000)) * int(FRAMESIZE_SEC * 1000);
    REQUIRE(std::abs(int(diffMSec)) <= PCM16_SAMPLES_DURATION(FRAMESIZE, SAMPLERATE));
    REQUIRE(opusdecfile.Seek(0));
    REQUIRE(opusdecfile.GetElapsedMSec() == 0);

    // check decode output after initial duration check
    opusdecfile.Close();
    REQUIRE(opusdecfile.Open(opusencfilename));
    REQUIRE(opusdecfile.GetDurationMSec() == mfi.uDurationMSec);
    frames = 0;
    while (opusdecfile.Decode(&frame[0], FRAMESIZE) == FRAMESIZE)frames++;
    duration_msec = PCM16_SAMPLES_DURATION(frames * FRAMESIZE, SAMPLERATE);
    REQUIRE(duration_msec == mfi.uDurationMSec);
    REQUIRE(opusdecfile.GetDurationMSec() == mfi.uDurationMSec);
}

#include <avstream/OpusFileStreamer.h>

TEST_CASE("OPUSStreamer")
{
    const auto IN_SAMPLERATE = 12000;
    const auto IN_FRAMESIZE_SEC = .04;
    const auto IN_CHANNELS = 2;

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = IN_CHANNELS;
    mfi.audioFmt.nSampleRate = IN_SAMPLERATE;
    mfi.uDurationMSec = 3 * 1000;

    const int IN_FRAMESIZE = int(mfi.audioFmt.nSampleRate * IN_FRAMESIZE_SEC);
    ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("opusstreamer_inputfile_%d_%dmsec.wav"),
                     mfi.audioFmt.nSampleRate, PCM16_SAMPLES_DURATION(IN_FRAMESIZE, mfi.audioFmt.nSampleRate));
    ACE_TCHAR opusencfilename[TT_STRLEN];
    ACE_OS::snprintf(opusencfilename, TT_STRLEN, ACE_TEXT("opusstreamer_inputfile_%d_%dmsec.ogg"),
                     mfi.audioFmt.nSampleRate, PCM16_SAMPLES_DURATION(IN_FRAMESIZE, mfi.audioFmt.nSampleRate));
    CreateOpusFile(mfi, opusencfilename, IN_FRAMESIZE);

    const auto OUT_SAMPLERATE = 48000;
    const auto OUT_CHANNELS = 1;
    const auto OUT_FRAMESIZE = int(OUT_SAMPLERATE * .06);

    ACE_TCHAR opusstreamerfilename[TT_STRLEN];
    ACE_OS::snprintf(opusstreamerfilename, TT_STRLEN, ACE_TEXT("opusstreamer_outputfile_%d_%dmsec.wav"),
                     OUT_SAMPLERATE, PCM16_SAMPLES_DURATION(OUT_FRAMESIZE, OUT_SAMPLERATE));

    WavePCMFile wavfile;
    REQUIRE(wavfile.NewFile(opusstreamerfilename, OUT_SAMPLERATE, OUT_CHANNELS));

    MediaStreamOutput mso(media::AudioFormat(OUT_SAMPLERATE, OUT_CHANNELS), OUT_FRAMESIZE);

    std::condition_variable cv_finished, cv_started, cv_paused, cv_playing, cv_error;
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
    ofs.reset(new OpusFileStreamer(opusencfilename, mso));
    ofs->RegisterStatusCallback(statusfunc, true);
    ofs->RegisterAudioCallback(audiofunc, true);
    REQUIRE(ofs->Open());
    REQUIRE(ofs->StartStream());
    REQUIRE(cv_finished.wait_for(lck, std::chrono::milliseconds(mfi.uDurationMSec * 2)) == std::cv_status::no_timeout);
}

TEST_CASE("TTPlayOpusOgg")
{
    const auto IN_SAMPLERATE = 48000;
    const auto IN_CHANNELS = 2;
    const auto IN_FRAMESIZE = int(IN_SAMPLERATE * .01);

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = IN_CHANNELS;
    mfi.audioFmt.nSampleRate = IN_SAMPLERATE;
    mfi.uDurationMSec = 5 * 1000;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("TTPlayOpusOgg.wav"), TT_STRLEN);

    TTCHAR szFilename[TT_STRLEN] = ACE_TEXT("TTPlayOpusOgg.ogg");
    CreateOpusFile(mfi, szFilename, IN_FRAMESIZE);

    ttinst ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
    mfp.bPaused = FALSE;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

    bool stop = false, started = false, paused = false;
    TTMessage msg;
    INT32 session;
    uint32_t durationMSec;

    // test duration of OpusFileStreamer playback
    session = TT_InitLocalPlayback(ttclient, szFilename, &mfp);
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
    session = TT_InitLocalPlayback(ttclient, szFilename, &mfp);
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
    session = TT_InitLocalPlayback(ttclient, szFilename, &mfp);
    REQUIRE(session > 0);
    stop = false;
    started = false;
    paused = false;
    durationMSec = GETTIMESTAMP();
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
            WaitForEvent(ttclient, CLIENTEVENT_NONE, msg, 1000);
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
    REQUIRE(int(durationMSec) > int(mfi.uDurationMSec + 1000));
}

TEST_CASE("TTPlayFFmpegOpus")
{
    TTCHAR filename[TT_STRLEN] = ACE_TEXT("testdata/Opus/giana.ogg");
    OpusDecFile odf;
    REQUIRE(odf.Open(filename));

    WavePCMFile wavfile;
    REQUIRE(wavfile.NewFile(ACE_TEXT("giana.wav"), odf.GetSampleRate(), odf.GetChannels()));
    std::vector<short> buf(odf.GetSampleRate() * odf.GetChannels());
    int samples, framesize = 0;
    while ((samples = odf.Decode(&buf[0], odf.GetSampleRate())) > 0)
    {
        wavfile.AppendSamples(&buf[0], samples);
        REQUIRE((framesize == 0 || framesize == samples)); // assume same framesize
        framesize = samples;
    }
    wavfile.Close();

    MediaFileInfo mfi;
    REQUIRE(TT_GetMediaFileInfo(ACE_TEXT("giana.wav"), &mfi));
    REQUIRE(std::abs(int(mfi.uDurationMSec - odf.GetDurationMSec())) < PCM16_SAMPLES_DURATION(framesize, odf.GetSampleRate()));
    REQUIRE(TT_GetMediaFileInfo(filename, &mfi));
    REQUIRE(mfi.uDurationMSec == odf.GetDurationMSec());
}
#endif

TEST_CASE("SeekPrecision")
{
    TTCHAR filename[TT_STRLEN] = ACE_TEXT("testdata/Opus/giana.ogg");

    auto ttclient = InitTeamTalk();
    REQUIRE(InitSound(ttclient));

    MediaFilePlayback mfp = {};
    mfp.bPaused = FALSE;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

    auto session = TT_InitLocalPlayback(ttclient, filename, &mfp);
    REQUIRE(session > 0);

    TTMessage msg;
    bool stop = false;
    while (!stop)
    {
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
        switch (msg.mediafileinfo.nStatus)
        {
        case MFS_PLAYING :
            // std::cout << "Playing at " << msg.mediafileinfo.uElapsedMSec << std::endl;
            if (msg.mediafileinfo.uElapsedMSec >= 2440)
            {
                mfp.bPaused = TRUE;
                REQUIRE(TT_UpdateLocalPlayback(ttclient, session, &mfp));
            }
            break;
        case MFS_PAUSED :
            mfp.bPaused = FALSE;
            mfp.uOffsetMSec = 988;
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
    REQUIRE(std::abs(int32_t(msg.mediafileinfo.uElapsedMSec - mfp.uOffsetMSec)) <= 120);
    // std::cout << "Started at " << msg.mediafileinfo.uElapsedMSec << std::endl;
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
    REQUIRE(msg.mediafileinfo.nStatus == MFS_PLAYING);
    REQUIRE(std::abs(int32_t(msg.mediafileinfo.uElapsedMSec - mfp.uOffsetMSec)) <= 240);
    // std::cout << "Playing at " << msg.mediafileinfo.uElapsedMSec << std::endl;
    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
    REQUIRE(msg.mediafileinfo.nStatus == MFS_PLAYING);
    REQUIRE(std::abs(int32_t(msg.mediafileinfo.uElapsedMSec - mfp.uOffsetMSec)) <= 360);
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
    REQUIRE(teamtalk::GetProperty(props, ACE_TEXT("tm"), tv2) == 1);
    REQUIRE(tv.sec() == tv2.sec());
}
