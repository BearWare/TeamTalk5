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

 /* Catch unit-tests that are performance dependent. Typically unit-tests
  * that cannot run under Valgrind */

#include <catch.hpp>

#include <ace/OS.h>
#include "TTUnitTest.h"

#include <myace/MyACE.h>

TEST_CASE("AudioMuxerStreamRestart")
{
    auto rxclient = InitTeamTalk();
    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(rxclient));
    auto rxuserid = TT_GetMyUserID(rxclient);

    auto txclient = InitTeamTalk();
    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(txclient));
    auto txuserid = TT_GetMyUserID(txclient);

    AudioBlock* ab;
    int n_frames;
    const int RETRIES = 50;

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));

    for (int i = 0; i < 3; ++i)
    {
        REQUIRE(TT_EnableVoiceTransmission(txclient, TRUE));

        n_frames = RETRIES;
        while (n_frames-- && WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK))
        {
            ab = TT_AcquireUserAudioBlock(rxclient, STREAMTYPE_VOICE, TT_MUXED_USERID);
            REQUIRE(ab);
            if (ab->uStreamTypes == STREAMTYPE_VOICE)
                break;
            REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
            ab = nullptr;
        }
        REQUIRE(ab);
        REQUIRE(ab->uStreamTypes == STREAMTYPE_VOICE);
        REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));

        REQUIRE(TT_EnableVoiceTransmission(txclient, FALSE));

        n_frames = RETRIES;
        while (n_frames-- && WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK))
        {
            ab = TT_AcquireUserAudioBlock(rxclient, STREAMTYPE_VOICE, TT_MUXED_USERID);
            REQUIRE(ab);
            if (ab->uStreamTypes == STREAMTYPE_NONE)
                break;
            REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
            ab = nullptr;
        }
        REQUIRE(ab);
        REQUIRE(ab->uStreamTypes == STREAMTYPE_NONE);
        REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));

        REQUIRE(WaitForCmdSuccess(rxclient, TT_DoLeaveChannel(rxclient)));

        REQUIRE(JoinRoot(rxclient));
    }

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, FALSE));

    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = 2;
    mfi.audioFmt.nSampleRate = 44100;
    mfi.uDurationMSec = 60000;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("myfile.wav"), TT_STRLEN);
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 500));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_MEDIAFILE_AUDIO, TRUE));

    for (int i = 0; i < 3; ++i)
    {
        REQUIRE(TT_StartStreamingMediaFileToChannel(txclient, mfi.szFileName, nullptr));

        n_frames = RETRIES;
        while (n_frames-- && WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK))
        {
            ab = TT_AcquireUserAudioBlock(rxclient, STREAMTYPE_MEDIAFILE_AUDIO, TT_MUXED_USERID);
            REQUIRE(ab);
            if (ab->uStreamTypes == STREAMTYPE_MEDIAFILE_AUDIO)
                break;
            REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
            ab = nullptr;
        }
        REQUIRE(ab);
        REQUIRE(ab->uStreamTypes == STREAMTYPE_MEDIAFILE_AUDIO);
        REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));

        REQUIRE(TT_StopStreamingMediaFileToChannel(txclient));

        n_frames = RETRIES;
        while (n_frames-- && WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK))
        {
            ab = TT_AcquireUserAudioBlock(rxclient, STREAMTYPE_MEDIAFILE_AUDIO, TT_MUXED_USERID);
            REQUIRE(ab);
            if (ab->uStreamTypes == STREAMTYPE_NONE)
                break;
            REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
            ab = nullptr;
        }
        REQUIRE(ab);
        REQUIRE(ab->uStreamTypes == STREAMTYPE_NONE);
        REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
    }
}

TEST_CASE("AudioMuxerAllStreamTypesIntoAudioBlock")
{
    AudioBlock* ab;
    StreamTypes sts;
    INT32 session;

    const int N_WAIT_FRAMES = 50;
    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = 2;
    mfi.audioFmt.nSampleRate = 44100;
    mfi.uDurationMSec = 60000;
    ACE_OS::strncpy(mfi.szFileName, ACE_TEXT("myfile.wav"), TT_STRLEN);
    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 500));

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
    mfp.bPaused = FALSE;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

    auto rxclient = InitTeamTalk();
    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(rxclient));
    auto rxuserid = TT_GetMyUserID(rxclient);

    auto txclient = InitTeamTalk();
    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(txclient));
    auto txuserid = TT_GetMyUserID(txclient);

    // test STREAMTYPE_VOICE from TT_LOCAL_USERID
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_LOCAL_USERID, STREAMTYPE_VOICE, TRUE));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK));
    ab = TT_AcquireUserAudioBlock(rxclient, STREAMTYPE_VOICE, TT_LOCAL_USERID);
    REQUIRE(ab);
    REQUIRE(ab->uStreamTypes == STREAMTYPE_VOICE);
    REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_LOCAL_USERID, STREAMTYPE_VOICE, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // test STREAMTYPE_VOICE from TT_LOCAL_TX_USERID
    REQUIRE(TT_EnableVoiceTransmission(rxclient, TRUE));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_LOCAL_TX_USERID, STREAMTYPE_VOICE, TRUE));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK));
    ab = TT_AcquireUserAudioBlock(rxclient, STREAMTYPE_VOICE, TT_LOCAL_TX_USERID);
    REQUIRE(ab);
    REQUIRE(ab->uStreamTypes == STREAMTYPE_VOICE);
    REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_LOCAL_TX_USERID, STREAMTYPE_VOICE, FALSE));
    REQUIRE(TT_EnableVoiceTransmission(rxclient, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // test STREAMTYPE_VOICE from remote user
    REQUIRE(TT_EnableVoiceTransmission(txclient, TRUE));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, TRUE));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK));
    ab = TT_AcquireUserAudioBlock(rxclient, STREAMTYPE_VOICE, txuserid);
    REQUIRE(ab);
    REQUIRE(ab->uStreamTypes == STREAMTYPE_VOICE);
    REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_VOICE, FALSE));
    REQUIRE(TT_EnableVoiceTransmission(txclient, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // test STREAMTYPE_MEDIAFILE_AUDIO from remote user
    AudioFormat af;
    af.nAudioFmt = AFF_WAVE_FORMAT;
    af.nChannels = 1;
    af.nSampleRate = 12000;
    REQUIRE(TT_StartStreamingMediaFileToChannel(txclient, mfi.szFileName, nullptr));
    REQUIRE(TT_EnableAudioBlockEventEx(rxclient, txuserid, STREAMTYPE_MEDIAFILE_AUDIO, &af, TRUE));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK));
    ab = TT_AcquireUserAudioBlock(rxclient, STREAMTYPE_MEDIAFILE_AUDIO, txuserid);
    REQUIRE(ab);
    REQUIRE(ab->uStreamTypes == STREAMTYPE_MEDIAFILE_AUDIO);
    REQUIRE(TT_StopStreamingMediaFileToChannel(txclient));
    REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, txuserid, STREAMTYPE_MEDIAFILE_AUDIO, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // test STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO
    session = TT_InitLocalPlayback(rxclient, mfi.szFileName, &mfp);
    REQUIRE(session > 0);
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, session, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, TRUE));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK));
    ab = TT_AcquireUserAudioBlock(rxclient, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, session);
    REQUIRE(ab);
    REQUIRE(ab->uStreamTypes == STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO);
    REQUIRE(TT_StopLocalPlayback(rxclient, session));
    REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, session, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, FALSE));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, 0);

    // clear audio encoder queue
    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoLeaveChannel(rxclient)));
    REQUIRE(WaitForCmdSuccess(txclient, TT_DoLeaveChannel(txclient)));
    REQUIRE(JoinRoot(rxclient));
    REQUIRE(JoinRoot(txclient));

    // test mixing of STREAMTYPE_VOICE | STREAMTYPE_MEDIAFILE_AUDIO | STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO
    sts = STREAMTYPE_VOICE | STREAMTYPE_MEDIAFILE_AUDIO | STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO;
    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, sts, TRUE));
    for (int i = 0; i < 3; ++i)
    {
        REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK));
        ab = TT_AcquireUserAudioBlock(rxclient, sts, TT_MUXED_USERID);
        REQUIRE(ab);
        REQUIRE(ab->uStreamTypes == STREAMTYPE_NONE);
        REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
    }
    REQUIRE(TT_EnableVoiceTransmission(txclient, TRUE));
    int n_frames = N_WAIT_FRAMES;
    while (n_frames-- && WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK))
    {
        ab = TT_AcquireUserAudioBlock(rxclient, sts, TT_MUXED_USERID);
        REQUIRE(ab);
        if (ab->uStreamTypes == STREAMTYPE_VOICE)
            break;
        REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
        ab = nullptr;
    }
    REQUIRE(ab);
    REQUIRE(ab->uStreamTypes == STREAMTYPE_VOICE);
    REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
    REQUIRE(TT_StartStreamingMediaFileToChannel(txclient, mfi.szFileName, nullptr));
    n_frames = N_WAIT_FRAMES;
    while (n_frames-- && WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK))
    {
        ab = TT_AcquireUserAudioBlock(rxclient, sts, TT_MUXED_USERID);
        REQUIRE(ab);
        if (ab->uStreamTypes == (STREAMTYPE_VOICE | STREAMTYPE_MEDIAFILE_AUDIO))
            break;
        REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
        ab = nullptr;
    }
    REQUIRE(ab);
    REQUIRE(ab->uStreamTypes == (STREAMTYPE_VOICE | STREAMTYPE_MEDIAFILE_AUDIO));
    REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
    session = TT_InitLocalPlayback(rxclient, mfi.szFileName, &mfp);
    REQUIRE(session > 0);
    n_frames = N_WAIT_FRAMES;
    while (n_frames-- && WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK))
    {
        ab = TT_AcquireUserAudioBlock(rxclient, sts, TT_MUXED_USERID);
        REQUIRE(ab);
        if (ab->uStreamTypes == (STREAMTYPE_VOICE | STREAMTYPE_MEDIAFILE_AUDIO | STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO))
            break;
        REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
        ab = nullptr;
    }
    REQUIRE(ab);
    REQUIRE(ab->uStreamTypes == (STREAMTYPE_VOICE | STREAMTYPE_MEDIAFILE_AUDIO | STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO));
    REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
    REQUIRE(TT_EnableVoiceTransmission(txclient, FALSE));
    REQUIRE(TT_StopStreamingMediaFileToChannel(txclient));
    REQUIRE(TT_StopLocalPlayback(rxclient, session));
    n_frames = N_WAIT_FRAMES;
    while (n_frames-- && WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK))
    {
        ab = TT_AcquireUserAudioBlock(rxclient, sts, TT_MUXED_USERID);
        REQUIRE(ab);
        if (ab->uStreamTypes == (STREAMTYPE_NONE))
            break;
        REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
        ab = nullptr;
    }
    REQUIRE(ab);
    REQUIRE(ab->uStreamTypes == (STREAMTYPE_NONE));
    REQUIRE(TT_ReleaseUserAudioBlock(rxclient, ab));
}


#if 0 // too many open files
TEST_CASE("MaxUsersAndMaxChannels")
{
    std::vector<ttinst> clients;
    for (int i = 1; i < 1000; ++i)
    {
        auto txclient = InitTeamTalk();

        REQUIRE(Connect(txclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
        ACE_TString name = ACE_TEXT("Client - #") + i2string(TT_GetMyUserID(txclient));
        REQUIRE(Login(txclient, name.c_str(), ACE_TEXT("guest"), ACE_TEXT("guest")));
        int n_users;
        REQUIRE(TT_GetServerUsers(txclient, nullptr, &n_users));
        REQUIRE(n_users == i); // no other users should be on server.

        AudioCodec codec;
        codec.nCodec = NO_CODEC;
        int n_channels;
        REQUIRE(TT_GetServerChannels(txclient, nullptr, &n_channels));
        if (n_channels < 0xFFE) // compensate for existing channels
        {
            Channel chan = MakeChannel(txclient, name.c_str(), TT_GetRootChannelID(txclient), codec);
            REQUIRE(WaitForCmdComplete(txclient, TT_DoJoinChannel(txclient, &chan)));
        }
        else
            JoinRoot(txclient);

        clients.push_back(txclient);
    }

    //    auto txclient = InitTeamTalk();

    //    REQUIRE(Connect(txclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    //    ACE_TString name = ACE_TEXT("Client - #") + i2string(TT_GetMyUserID(txclient));
    //    REQUIRE(Login(txclient, name.c_str(), ACE_TEXT("guest"), ACE_TEXT("guest")));

}
#endif

TEST_CASE("MaxChannels")
{
    auto txclient = InitTeamTalk();

    REQUIRE(Connect(txclient, ACE_TEXT("127.0.0.1"), 10333, 10333));

    REQUIRE(Login(txclient, ACE_TEXT("admin"), ACE_TEXT("admin"), ACE_TEXT("admin")));

    int n_channels;
    REQUIRE(TT_GetServerChannels(txclient, nullptr, &n_channels));

    //int n_create = 0xffe - n_channels; //takes too long
    int n_create = 1000;

    for (int i = 0; i < n_create; ++i)
    {
        AudioCodec codec;
        codec.nCodec = NO_CODEC;
        ACE_TString name = ACE_TEXT("TestChannel - #") + i2string(i);
        Channel chan = MakeChannel(txclient, name.c_str(), TT_GetRootChannelID(txclient), codec);
        REQUIRE(WaitForCmdSuccess(txclient, TT_DoMakeChannel(txclient, &chan)));
    }

    auto rxclient = InitTeamTalk();
    REQUIRE(Connect(rxclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(rxclient));

    for (int i = 0; i < n_create; ++i)
    {
        ACE_TString name = ACE_TEXT("TestChannel - #") + i2string(i);
        int chanid = TT_GetChannelIDFromPath(txclient, name.c_str());
        REQUIRE(WaitForCmdSuccess(txclient, TT_DoRemoveChannel(txclient, chanid)));
    }
}
