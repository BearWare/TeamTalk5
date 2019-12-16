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

#include "TTUnitTest.h"

#include <codec/OggOutput.h>

#if defined(WIN32)
#include <ace/Init_ACE.h>
#include <assert.h>
static class WinInit
{
public:
    WinInit()
    {
        int ret = ACE::init();
        assert(ret >= 0);
    }
    ~WinInit()
    {
        int ret = ACE::fini();
        assert(ret >= 0);
    }
} wininit;
#endif

TEST_CASE( "Init TT", "" ) {
    TTInstance* ttinst;
    REQUIRE( (ttinst = TT_InitTeamTalkPoll()) );
    REQUIRE( TT_CloseTeamTalk(ttinst) );
}

TEST_CASE( "Ogg Write", "" ) {
    SpeexEncFile spxfile;
    REQUIRE( spxfile.Open(ACE_TEXT("/foo.spx"), 1, DEFAULT_SPEEX_COMPLEXITY, 7, 32000, 48000, false) == false);
}

TEST_CASE( "Record mux") {
    std::vector<TTInstance*> clients(2);
    for (auto i=0;i<clients.size();++i)
    {
        REQUIRE((clients[i] = TT_InitTeamTalkPoll()));
        REQUIRE(InitSound(clients[i]));
        REQUIRE(Connect(clients[i], ACE_TEXT("127.0.0.1"), 10333, 10333));
        REQUIRE(Login(clients[i], ACE_TEXT("MyNickname"), ACE_TEXT("guest"), ACE_TEXT("guest")));
        REQUIRE(JoinRoot(clients[i]));
    }

    Channel rootchan;
    REQUIRE(TT_GetChannel(clients[1], TT_GetMyChannelID(clients[1]), &rootchan));
    REQUIRE(TT_StartRecordingMuxedAudioFile(clients[1], &rootchan.audiocodec, ACE_TEXT("MyMuxFile.wav"), AFF_WAVE_FORMAT));
    
    REQUIRE(TT_DBG_SetSoundInputTone(clients[0], STREAMTYPE_VOICE, 500));
    REQUIRE(TT_EnableVoiceActivation(clients[0], true));
    WaitForEvent(clients[1], CLIENTEVENT_NONE, TTMessage(), 5000);
    REQUIRE(TT_StopRecordingMuxedAudioFile(clients[1]));

    for(auto c : clients)
        REQUIRE(TT_CloseTeamTalk(c));
}