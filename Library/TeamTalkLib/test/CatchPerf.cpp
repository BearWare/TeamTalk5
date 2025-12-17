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

#include <catch2/catch_test_macros.hpp>

#include "TTUnitTest.h"

#include "avstream/MediaPlayback.h"
#include "codec/WaveFile.h"
#include "myace/MyACE.h"

#include <cstring>

TEST_CASE("AudioMuxerStreamRestart")
{
    auto rxclient = InitTeamTalk();
    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));
    auto rxuserid = TT_GetMyUserID(rxclient);

    auto txclient = InitTeamTalk();
    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
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
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));
    auto rxuserid = TT_GetMyUserID(rxclient);

    auto txclient = InitTeamTalk();
    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient")));
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
    REQUIRE(Login(ttclient, ACE_TEXT("RxClient")));
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
        ABPtr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, sts, TT_MUXED_USERID));
        wavfile.AppendSamples(reinterpret_cast<const short*>(ab->lpRawAudio), ab->nSamples);
        if (ab->uStreamTypes == STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO)
            oneSecPlayback -= ab->nSamples;
    }

    REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));
    ABPtr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, sts, TT_MUXED_USERID));
    wavfile.AppendSamples(reinterpret_cast<const short*>(ab->lpRawAudio), ab->nSamples);
    REQUIRE(ab->uStreamTypes == STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO);

    mfp.bPaused = TRUE;
    REQUIRE(TT_UpdateLocalPlayback(ttclient, session, &mfp));

    int oneSecSilence = mfi.audioFmt.nSampleRate;
    while (oneSecSilence > 0 && WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg))
    {
        ABPtr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, sts, TT_MUXED_USERID));
        wavfile.AppendSamples(reinterpret_cast<const short*>(ab->lpRawAudio), ab->nSamples);
        if (ab->uStreamTypes == STREAMTYPE_NONE)
            oneSecSilence -= ab->nSamples;
    }

    mfp.bPaused = FALSE;
    REQUIRE(TT_UpdateLocalPlayback(ttclient, session, &mfp));

    oneSecPlayback = mfi.audioFmt.nSampleRate;
    while (oneSecPlayback > 0 && WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg))
    {
        ABPtr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, sts, TT_MUXED_USERID));
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
                ABPtr ab(rxclient, TT_AcquireUserAudioBlock(rxclient, sts, TT_MUXED_USERID));
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

TEST_CASE("LocalPlaybackLatency")
{
    // Call TT_InitLocalPlayback for file 1, PAUSE=FALSE
    MediaFilePlayback mfp = {};
    mfp.bPaused = false;
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

    MediaFileInfo mfi;
    REQUIRE(TT_GetMediaFileInfo(ACE_TEXT("testdata/Opus/off.ogg"), &mfi));

    std::vector< std::vector<short> > buffers;
    std::vector< std::vector<AudioBlock> > abs_shared;

    for (int i = 0; i < 3; ++i)
    {
        auto ttclient = InitTeamTalk();
        REQUIRE(InitSound(ttclient, SHARED_INPUT_OUTPUT));

        auto starttime = GETTIMESTAMP();
        int onid = TT_InitLocalPlayback(ttclient, mfi.szFileName, &mfp);
        REQUIRE(onid > 0);
        REQUIRE(TT_EnableAudioBlockEvent(ttclient, onid, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, TRUE));

        TTMessage msg;
        REQUIRE(WaitForEvent(ttclient, CLIENTEVENT_USER_AUDIOBLOCK, msg));

        MYTRACE(ACE_TEXT("Latency for SHARED_INPUT_OUTPUT: %u"), GETTIMESTAMP() - starttime);

        while (WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg) && msg.mediafileinfo.nStatus != MFS_FINISHED);

        abs_shared.resize(abs_shared.size() + 1);
        while (true)
        {
            ABPtr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, onid));
            if (!ab)
                break;

            buffers.resize(buffers.size() + 1);
            int index = buffers.size() - 1;
            buffers[index].resize(ab->nSamples * ab->nChannels);
            std::memcpy(&buffers[index][0], ab->lpRawAudio, ab->nSamples * ab->nChannels * sizeof(short));
            AudioBlock store = *ab;
            store.lpRawAudio = &buffers[index][0];
            abs_shared[i].push_back(store);
        }

        WaitForEvent(ttclient, CLIENTEVENT_NONE, 500);
    }

    // ensure all playbacks outputs the same audio
    for (size_t i = 0; i < abs_shared.size(); ++i)
    {
        REQUIRE(abs_shared[0].size() == abs_shared[i].size());
        for (size_t a = 0; a < abs_shared[0].size(); ++a)
        {
            REQUIRE(abs_shared[0][a].nSamples == abs_shared[i][a].nSamples);
            REQUIRE(std::memcmp(abs_shared[0][a].lpRawAudio, abs_shared[i][a].lpRawAudio, abs_shared[0][a].nSamples * abs_shared[0][a].nChannels * sizeof(short)) == 0);
        }
    }

    for (int i = 0; i < 3; ++i)
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

        MYTRACE(ACE_TEXT("Latency for DEFAULT: %u\n"), GETTIMESTAMP() - starttime);

        while (WaitForEvent(ttclient, CLIENTEVENT_LOCAL_MEDIAFILE, msg) && msg.mediafileinfo.nStatus != MFS_FINISHED);

        // Ensure DEFAULT mode outputs the same audio as SHARED mode
        size_t index = 0;
        while (true)
        {
            ABPtr ab(ttclient, TT_AcquireUserAudioBlock(ttclient, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, onid));
            if (!ab)
                break;

            REQUIRE(index < abs_shared.size());
            REQUIRE(abs_shared[0][index].nSamples == ab->nSamples);
            REQUIRE(std::memcmp(abs_shared[0][index].lpRawAudio, ab->lpRawAudio, ab->nSamples * ab->nChannels * sizeof(short)) == 0);
            index++;
        }
        REQUIRE(index == abs_shared[0].size());

        WaitForEvent(ttclient, CLIENTEVENT_NONE, 500);
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
        ACE_TString name = ACE_TEXT("Client - #") + I2String(TT_GetMyUserID(txclient));
        REQUIRE(Login(txclient, name.c_str()));
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
    //    ACE_TString name = ACE_TEXT("Client - #") + I2String(TT_GetMyUserID(txclient));
    //    REQUIRE(Login(txclient, name.c_str()));

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
        ACE_TString name = ACE_TEXT("TestChannel - #") + I2String(i);
        Channel chan = MakeChannel(txclient, name.c_str(), TT_GetRootChannelID(txclient), codec);
        REQUIRE(WaitForCmdSuccess(txclient, TT_DoMakeChannel(txclient, &chan)));
    }

    auto rxclient = InitTeamTalk();
    REQUIRE(Connect(rxclient));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient")));
    REQUIRE(JoinRoot(rxclient));

    for (int i = 0; i < n_create; ++i)
    {
        ACE_TString name = ACE_TEXT("TestChannel - #") + I2String(i);
        int chanid = TT_GetChannelIDFromPath(txclient, name.c_str());
        REQUIRE(WaitForCmdSuccess(txclient, TT_DoRemoveChannel(txclient, chanid)));
    }
}

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
