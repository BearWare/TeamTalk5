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
#include <codec/WaveFile.h>
#include <avstream/MediaPlayback.h>

#include <myace/MyACE.h>

TEST_CASE("AudioMuxerStreamRestart")
{
    auto rxclient = InitTeamTalk();
    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(rxclient));
    auto rxuserid = TT_GetMyUserID(rxclient);

    auto txclient = InitTeamTalk();
    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
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
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(rxclient));
    auto rxuserid = TT_GetMyUserID(rxclient);

    auto txclient = InitTeamTalk();
    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
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

TEST_CASE( "AudioMuxerPauseLocalPlayback" )
{
    // pause local playback and ensure we continue to receive AudioBlocks with uStreamTypes == STREAMTYPE_NONE
    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = 2;
    mfi.audioFmt.nSampleRate = 48000;
    mfi.uDurationMSec = 10 * 1000;
    ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("tone500hz.wav"));

    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 500));

    auto ttclient = InitTeamTalk();

    REQUIRE(InitSound(ttclient));
    REQUIRE(Connect(ttclient));
    REQUIRE(Login(ttclient, ACE_TEXT("RxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(ttclient));

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    mfp.bPaused = FALSE;

    WavePCMFile wavfile;
    REQUIRE(wavfile.NewFile(ACE_TEXT("tone500hz_mix.wav"), mfi.audioFmt.nSampleRate, mfi.audioFmt.nChannels));

    auto session = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);
    REQUIRE(session > 0);

    auto sts = STREAMTYPE_VOICE | STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO;
    REQUIRE(TT_EnableAudioBlockEventEx(ttclient, TT_MUXED_USERID, sts, &mfi.audioFmt, TRUE));

    TTMessage msg;
    int oneSecPlayback = mfi.audioFmt.nSampleRate;
    while (oneSecPlayback > 0 && WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg))
    {
        abptr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, sts, TT_MUXED_USERID));
        wavfile.AppendSamples(reinterpret_cast<const short*>(ab->lpRawAudio), ab->nSamples);
        if (ab->uStreamTypes == STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO)
            oneSecPlayback -= ab->nSamples;
    }

    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
    abptr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, sts, TT_MUXED_USERID));
    wavfile.AppendSamples(reinterpret_cast<const short*>(ab->lpRawAudio), ab->nSamples);
    REQUIRE(ab->uStreamTypes == STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO);

    mfp.bPaused = TRUE;
    REQUIRE(TT_UpdateLocalPlayback(ttclient, session, &mfp));

    int oneSecSilence = mfi.audioFmt.nSampleRate;
    while (oneSecSilence > 0 && WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg))
    {
        abptr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, sts, TT_MUXED_USERID));
        wavfile.AppendSamples(reinterpret_cast<const short*>(ab->lpRawAudio), ab->nSamples);
        if (ab->uStreamTypes == STREAMTYPE_NONE)
            oneSecSilence -= ab->nSamples;
    }

    mfp.bPaused = FALSE;
    REQUIRE(TT_UpdateLocalPlayback(ttclient, session, &mfp));

    oneSecPlayback = mfi.audioFmt.nSampleRate;
    while (oneSecPlayback > 0 && WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg))
    {
        abptr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, sts, TT_MUXED_USERID));
        wavfile.AppendSamples(reinterpret_cast<const short*>(ab->lpRawAudio), ab->nSamples);
        if (ab->uStreamTypes == STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO)
            oneSecPlayback -= ab->nSamples;
    }

}

TEST_CASE("AudioMuxerMixedVoiceAndLocalPlayback")
{
    // Write output-wavefile containing a mix of voice-stream from remote user and local-playback

    for (int txinterval : {10, 20, 240})
    {
        AudioCodec ac = MakeDefaultAudioCodec(OPUS_CODEC);
        ac.opus.nSampleRate = 48000;
        ac.opus.nChannels = 2;
        ac.opus.nTxIntervalMSec = txinterval;
        ac.opus.nFrameSizeMSec = 10;

        MediaFileInfo mfi = {};
        mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
        mfi.audioFmt.nChannels = 2;
        mfi.audioFmt.nSampleRate = 48000;
        mfi.uDurationMSec = 1000;
        ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("tone_1000.wav"));
        REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 1000));

#if 0
        // mix music with tone
        ACE_OS::snprintf(mfi.szFileName, TT_STRLEN, ACE_TEXT("testdata/Opus/giana.ogg"));
#endif

        auto rxclient = InitTeamTalk();
        REQUIRE(InitSound(rxclient));
        REQUIRE(Connect(rxclient));
        REQUIRE(Login(rxclient, ACE_TEXT("RxClient"), ACE_TEXT("admin"), ACE_TEXT("admin")));

        auto chan = MakeChannel(rxclient, ACE_TEXT("Channel6"), TT_GetRootChannelID(rxclient), ac);
        chan.uChannelType = CHANNEL_HIDDEN;
        REQUIRE(WaitForCmdSuccess(rxclient, TT_DoJoinChannel(rxclient, &chan)));

        auto txclient = InitTeamTalk();
        REQUIRE(InitSound(txclient));
        REQUIRE(Connect(txclient));
        REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
        REQUIRE(WaitForCmdSuccess(txclient, TT_DoJoinChannel(txclient, &chan)));
        REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, 500));
        REQUIRE(TT_EnableVoiceTransmission(txclient, true));

        StreamTypes sts = STREAMTYPE_VOICE | STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO;
        AudioFormat af;
        af.nAudioFmt = AFF_WAVE_FORMAT;
        af.nChannels = 1;
        af.nSampleRate = 8000;
        REQUIRE(TT_EnableAudioBlockEventEx(rxclient, TT_MUXED_USERID, sts, &af, TRUE));

        WavePCMFile wavfile;
        TTCHAR wavfilename[TT_STRLEN];
        ACE_OS::snprintf(wavfilename, TT_STRLEN, ACE_TEXT("mixedvoicelocalplayback_%d.wav"), ac.opus.nTxIntervalMSec);
        REQUIRE(wavfile.NewFile(wavfilename, af.nSampleRate, af.nChannels));

        TTCHAR recordfilename[TT_STRLEN];
        ACE_OS::snprintf(recordfilename, TT_STRLEN, ACE_TEXT("recordvoicelocalplayback_%d.wav"), ac.opus.nTxIntervalMSec);
        REQUIRE(TT_StartRecordingMuxedStreams(rxclient, sts, &ac, recordfilename, AFF_WAVE_FORMAT));

        MediaFilePlayback mfp = {};
        mfp.bPaused = FALSE;
        mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

        INT32 session = TT_InitLocalPlayback(rxclient, mfi.szFileName, &mfp);
        REQUIRE(session > 0);

        bool stop = false;
        TTMessage msg;
        auto waittime = DEFWAIT;
        while (TT_GetMessage(rxclient, &msg, &waittime) && !stop)
        {
            switch (msg.nClientEvent)
            {
            case CLIENTEVENT_USER_AUDIOBLOCK :
            {
                abptr ab(rxclient, TT_AcquireUserAudioBlock(rxclient, sts, TT_MUXED_USERID));
                REQUIRE(ab->nSamples == int(ab->nSampleRate * .02));
                REQUIRE(ab->nChannels == af.nChannels);
                REQUIRE(ab->nSampleRate == af.nSampleRate);
                wavfile.AppendSamples(reinterpret_cast<const short*>(ab->lpRawAudio), ab->nSamples);
                break;
            }
            case CLIENTEVENT_LOCAL_MEDIAFILE :
                stop = msg.mediafileinfo.nStatus == MFS_FINISHED;
                break;
            default :
                break;
            }
        }

        REQUIRE(TT_StopRecordingMuxedAudioFile(rxclient));
        REQUIRE(WaitForCmdSuccess(rxclient, TT_DoRemoveChannel(rxclient, TT_GetMyChannelID(rxclient))));

        wavfile.Close();

        MediaFileInfo mfi1, mfi2, mfi3;
        REQUIRE(TT_GetMediaFileInfo(mfi.szFileName, &mfi1));
        REQUIRE(TT_GetMediaFileInfo(wavfilename, &mfi2));
        REQUIRE(TT_GetMediaFileInfo(recordfilename, &mfi3));

        // local playback uses a 40 msec
        int localplayback_interval = txinterval * (PB_FRAMEDURATION_MSEC / txinterval);
        REQUIRE(mfi2.uDurationMSec >= mfi1.uDurationMSec - (2 * txinterval + localplayback_interval));
        REQUIRE(mfi3.uDurationMSec >= mfi1.uDurationMSec - (2 * txinterval + localplayback_interval));
    }
}

#if 0 // too many open files
TEST_CASE("MaxUsersAndMaxChannels")
{
    std::vector<ttinst> clients;
    for (int i = 1; i < 1000; ++i)
    {
        auto txclient = InitTeamTalk();

        REQUIRE(Connect(txclient));
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

    //    REQUIRE(Connect(txclient));
    //    ACE_TString name = ACE_TEXT("Client - #") + i2string(TT_GetMyUserID(txclient));
    //    REQUIRE(Login(txclient, name.c_str(), ACE_TEXT("guest"), ACE_TEXT("guest")));

}
#endif

TEST_CASE("MaxChannels")
{
    auto txclient = InitTeamTalk();

    REQUIRE(Connect(txclient));

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
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(rxclient));

    for (int i = 0; i < n_create; ++i)
    {
        ACE_TString name = ACE_TEXT("TestChannel - #") + i2string(i);
        int chanid = TT_GetChannelIDFromPath(txclient, name.c_str());
        REQUIRE(WaitForCmdSuccess(txclient, TT_DoRemoveChannel(txclient, chanid)));
    }
}
