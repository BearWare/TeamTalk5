//
//  TeamTalkTestTests.m
//  TeamTalkTestTests
//
//  Created by Bjoern Rasmussen on 4-01-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <XCTest/XCTest.h>

#include "../TeamTalk_DLL/TeamTalk.h"

@interface TeamTalkTestTests : XCTestCase
{
    NSMutableArray* clients;
}
- (TTInstance*)newClient;
- (void)initSound:(TTInstance*)ttInst;
- (void)connect:(TTInstance*)ttInst
        ipaddr:(const TTCHAR*)ip
        tcpport:(INT32)tcpport
        udpport:(INT32)udpport
        encrypted:(TTBOOL)encrypted;
- (INT32)login:  (TTInstance*)ttInst
        nickname:(const TTCHAR*)nick
        username:(const TTCHAR*)usr
        password:(const TTCHAR*)pw;
- (void)joinRootChannel:(TTInstance*)ttInst;
@end

const int DEF_WAIT = 5000;
const TTCHAR IPADDR[] = "192.168.1.110";
const INT32 TCPPORT = 10333, UDPPORT = 10333;
const TTBOOL ENCRYPTED = FALSE;
const TTCHAR ADMIN_USERNAME[] = "admin", ADMIN_PASSWORD[] = "admin";

@implementation TeamTalkTestTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.

    clients = [[NSMutableArray alloc] init];
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    
    for(id ttInst in clients) {
        TT_CloseTeamTalk([ttInst pointerValue]);
    }
    
    [clients removeAllObjects];
    
    [super tearDown];
}

- (void)testExample {
    // This is an example of a functional test case.
    XCTAssert(YES, @"Pass");
    NSString* str = [[NSString alloc]initWithUTF8String:TT_GetVersion()];
    NSLog(@"This is some TTT messsage %@", str);
}

- (void)testSoundLoop {
    
    SoundDevice devs[10];
    int how_many = 0;
    
    XCTAssert(TT_GetSoundDevices(NULL, &how_many), "Queried sound devices");
    
    XCTAssert(TT_GetSoundDevices(devs, &how_many), "Filled devices");
    
    XCTAssertGreaterThan(how_many, 0, "Got one or more devices");
    
    NSString* str = [[NSString alloc]initWithUTF8String:devs[0].szDeviceName];
    NSLog(@"Device name is %@", str);
    
    TTSoundLoop* sndLoopInst = TT_StartSoundLoopbackTest(devs[0].nDeviceID, devs[0].nDeviceID,
                                                         devs[0].nDefaultSampleRate,
                                                         1, FALSE, NULL);
    XCTAssert(sndLoopInst != NULL, "Start Sound Loop");

    NSLog(@"Run main event loop for 5 seconds to process audio...");
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 5, false);
    NSLog(@"Finished event looping...");
    
    XCTAssert(TT_CloseSoundLoopbackTest(sndLoopInst), "Close sound loop");
    
    SpeexDSP spxdsp;
    spxdsp.bEnableAGC = false;
    spxdsp.nGainLevel = 8000;
    spxdsp.nMaxIncDBSec = 12;
    spxdsp.nMaxDecDBSec = -40;
    spxdsp.nMaxGainDB = 30;
    
    spxdsp.bEnableDenoise = true;
    spxdsp.nMaxNoiseSuppressDB = -30;
    
    spxdsp.bEnableEchoCancellation = false;
    spxdsp.nEchoSuppress = -40;
    spxdsp.nEchoSuppressActive = -15;
    
    sndLoopInst = TT_StartSoundLoopbackTest(devs[0].nDeviceID, devs[0].nDeviceID,
                                            devs[0].nDefaultSampleRate,
                                            1, FALSE, &spxdsp);
    XCTAssert(sndLoopInst != NULL, "Start Sound Loop SpxDSP");
    
    NSLog(@"Run main event loop for 5 seconds to process audio...");
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 5, false);
    NSLog(@"Finished event looping...");
    
    XCTAssert(TT_CloseSoundLoopbackTest(sndLoopInst), "Close sound loop SpxDsp");

}

- (void)testConnect {
    TTInstance* ttInst = [self newClient];
    
    TTBOOL b = TT_Connect(ttInst, IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED);

    XCTAssert(b, "Connect to server");

    int waitMSec = 5000;
    TTMessage msg;

    XCTAssert(TT_GetMessage(ttInst, &msg, &waitMSec), "Wait connect");

    XCTAssertEqual(CLIENTEVENT_CON_SUCCESS, msg.nClientEvent, "connect success");

    int cmdid = TT_DoLogin(ttInst, "iOS", "guest", "guest");
    XCTAssert(cmdid > 0, "do login");

    XCTAssert(TT_GetMessage(ttInst, &msg, &waitMSec), "Wait Login");
    
    XCTAssertEqual(CLIENTEVENT_CMD_PROCESSING, msg.nClientEvent, "login reply starting");
    
    b = waitForEvent(ttInst, CLIENTEVENT_CMD_MYSELF_LOGGEDIN, waitMSec, &msg);
    
    XCTAssertEqual(CLIENTEVENT_CMD_MYSELF_LOGGEDIN, msg.nClientEvent, "login completed with success");

    while(TT_GetMessage(ttInst, &msg, &waitMSec)) {
        switch(msg.nClientEvent) {
            case CLIENTEVENT_CMD_SERVER_UPDATE : {
                XCTAssertEqual(__SERVERPROPERTIES, msg.ttType, @"TTMessage contains ServerProperties");
                NSString* name = [[NSString alloc] initWithUTF8String:msg.serverproperties.szServerName];
                NSLog(@"Server name: %@", name);
                break;
            }
            case CLIENTEVENT_CMD_CHANNEL_NEW : {
                XCTAssertEqual(__CHANNEL, msg.ttType, @"TTMessage contains Channel");
                NSString* name = [[NSString alloc] initWithUTF8String:msg.channel.szName];
                NSLog(@"Channel #%d name: %@", msg.channel.nChannelID, name);
                break;
            }
            case CLIENTEVENT_CMD_USER_LOGGEDIN : {
                XCTAssertEqual(__USER, msg.ttType, @"TTMessage contains User");
                NSString* name = [[NSString alloc] initWithUTF8String:msg.user.szNickname];
                NSLog(@"User name: %@", name);
                break;
            }
            case CLIENTEVENT_CMD_USER_JOINED : {
                XCTAssertEqual(__USER, msg.ttType, @"TTMessage contains User");
                NSString* name = [[NSString alloc] initWithUTF8String:msg.user.szNickname];
                NSLog(@"User name: %@", name);
                break;
            }
            case CLIENTEVENT_CMD_PROCESSING : {
                XCTAssertEqual(__TTBOOL, msg.ttType, @"TTMessage contains TTBOOL");
                XCTAssertEqual(0, msg.bActive, @"Processing completed");
                break;
            }
            default : {
                
            }
        }
    }
}

- (void)testJoinChannel {
    TTInstance* ttInst = [self newClient];
    [self initSound:ttInst];
    [self connect:ttInst ipaddr:IPADDR tcpport:TCPPORT udpport:UDPPORT encrypted:ENCRYPTED];
    [self login:ttInst nickname:"testJoinChannel" username:"guest" password:"guest"];
    [self joinRootChannel:ttInst];
    
    XCTAssert(TT_DBG_SetSoundInputTone(ttInst, STREAMTYPE_VOICE, 440));
    
    XCTAssert(TT_EnableVoiceTransmission(ttInst, TRUE));
    
    
    TTMessage msg;
    waitForEvent(ttInst, CLIENTEVENT_NONE, 10000, &msg);
}

- (void)testSpeexChannel {

    TTInstance* ttInst = [self newClient];
    [self initSound:ttInst];
    [self connect:ttInst ipaddr:IPADDR tcpport:TCPPORT udpport:UDPPORT encrypted:ENCRYPTED];
    [self login:ttInst nickname:"testJoinChannel" username:"guest" password:"guest"];

    int myuserid = TT_GetMyUserID(ttInst);
    
    Channel chan;
    chan.nParentID = TT_GetRootChannelID(ttInst);
    chan.nChannelID = 0;
    chan.uChannelType = CHANNEL_DEFAULT;
    chan.nDiskQuota = 0;
    chan.nMaxUsers = 100;
    strncpy(chan.szName, "Speex Channel", TT_STRLEN);
    strncpy(chan.szOpPassword, "", TT_STRLEN);
    strncpy(chan.szPassword, "", TT_STRLEN);
    strncpy(chan.szTopic, "This is the topic", TT_STRLEN);
    
    memset(&chan.audiocfg, 0, sizeof(chan.audiocfg));
    chan.audiocodec.nCodec = SPEEX_CODEC;
    chan.audiocodec.speex.nQuality = 4;
    chan.audiocodec.speex.nTxIntervalMSec = 40;
    chan.audiocodec.speex.nBandmode = 1;
    chan.audiocodec.speex.bStereoPlayback = FALSE;
    
    int cmdid = TT_DoJoinChannel(ttInst, &chan);
    XCTAssert(waitCmdSuccess(ttInst, cmdid, DEF_WAIT), "Join channel");
    
    TTMessage msg;
    
    //drain message queue
    waitForEvent(ttInst, CLIENTEVENT_NONE, 0, &msg);

    TT_DBG_SetSoundInputTone(ttInst, STREAMTYPE_VOICE, 440);
    
    XCTAssert(TT_EnableVoiceTransmission(ttInst, TRUE));

    cmdid = TT_DoSubscribe(ttInst, myuserid, SUBSCRIBE_VOICE);
    XCTAssert(waitCmdSuccess(ttInst, cmdid, DEF_WAIT));
    
    XCTAssert(TT_EnableAudioBlockEvent(ttInst, myuserid, STREAMTYPE_VOICE, TRUE));
    
    XCTAssert(waitForEvent(ttInst, CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, &msg));
    
    waitForEvent(ttInst, CLIENTEVENT_NONE, 5000, &msg);
}

- (void)testOpusChannel {
    
    TTInstance* ttInst = [self newClient];
    [self initSound:ttInst];
    [self connect:ttInst ipaddr:IPADDR tcpport:TCPPORT udpport:UDPPORT encrypted:ENCRYPTED];
    [self login:ttInst nickname:"testJoinChannel" username:"guest" password:"guest"];
    
    int myuserid = TT_GetMyUserID(ttInst);
    
    Channel chan;
    chan.nParentID = TT_GetRootChannelID(ttInst);
    chan.nChannelID = 0;
    chan.uChannelType = CHANNEL_DEFAULT;
    chan.nDiskQuota = 0;
    chan.nMaxUsers = 100;
    strncpy(chan.szName, "Opus Channel", TT_STRLEN);
    strncpy(chan.szOpPassword, "", TT_STRLEN);
    strncpy(chan.szPassword, "", TT_STRLEN);
    strncpy(chan.szTopic, "This is the topic", TT_STRLEN);
    
    memset(&chan.audiocfg, 0, sizeof(chan.audiocfg));
    chan.audiocodec.nCodec = OPUS_CODEC;
    chan.audiocodec.opus.nApplication = OPUS_APPLICATION_VOIP;
    chan.audiocodec.opus.nBitRate = 32000;
    chan.audiocodec.opus.nChannels = 1;
    chan.audiocodec.opus.nComplexity = 2;
    chan.audiocodec.opus.nSampleRate = 16000;
    chan.audiocodec.opus.nTxIntervalMSec = 40;
    chan.audiocodec.opus.bVBRConstraint = FALSE;
    chan.audiocodec.opus.bVBR = FALSE;
    chan.audiocodec.opus.bFEC = TRUE;
    chan.audiocodec.opus.bDTX = TRUE;

    int cmdid = TT_DoJoinChannel(ttInst, &chan);
    XCTAssert(waitCmdSuccess(ttInst, cmdid, DEF_WAIT), "Join channel");
    
    TTMessage msg;
    
    //drain message queue
    waitForEvent(ttInst, CLIENTEVENT_NONE, 0, &msg);
    
    TT_DBG_SetSoundInputTone(ttInst, STREAMTYPE_VOICE, 440);
    
    XCTAssert(TT_EnableVoiceTransmission(ttInst, TRUE));
    
    cmdid = TT_DoSubscribe(ttInst, myuserid, SUBSCRIBE_VOICE);
    XCTAssert(waitCmdSuccess(ttInst, cmdid, DEF_WAIT));
    
    //XCTAssert(TT_EnableAudioBlockEvent(ttInst, myuserid, STREAMTYPE_VOICE, TRUE));
    
    //XCTAssert(waitForEvent(ttInst, CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, &msg));
    
    waitForEvent(ttInst, CLIENTEVENT_NONE, 10000, &msg);
}


- (void)testTextMessage {

    TTInstance* ttInst1 = [self newClient];
    TTInstance* ttInst2 = [self newClient];

    TTMessage msg;

    int userid1;
    [self connect:ttInst1 ipaddr:IPADDR tcpport:TCPPORT udpport:UDPPORT encrypted:ENCRYPTED];
    userid1 = [self login:ttInst1 nickname:"testTextMessage" username:"guest" password:"guest"];
    [self joinRootChannel:ttInst1];

    int userid2;
    [self connect:ttInst2 ipaddr:IPADDR tcpport:TCPPORT udpport:UDPPORT encrypted:ENCRYPTED];
    userid2 = [self login:ttInst2 nickname:"testTextMessage" username:"guest" password:"guest"];
    [self joinRootChannel:ttInst2];

    TextMessage txtmsg;
    txtmsg.nChannelID = 0;
    txtmsg.nFromUserID = 0;
    txtmsg.nMsgType = MSGTYPE_USER;
    txtmsg.nToUserID = userid2;
    strncpy(txtmsg.szMessage, "Hello World", TT_STRLEN);
    strncpy(txtmsg.szFromUsername, "", TT_STRLEN);
    
    int cmdid = TT_DoTextMessage(ttInst1, &txtmsg);
    XCTAssertGreaterThan(cmdid, 0, "Sent user2user text message");
    
    XCTAssert(waitForEvent(ttInst2, CLIENTEVENT_CMD_USER_TEXTMSG, DEF_WAIT, &msg));
    
    txtmsg.nChannelID = TT_GetMyChannelID(ttInst1);
    txtmsg.nFromUserID = 0;
    txtmsg.nMsgType = MSGTYPE_CHANNEL;
    cmdid = TT_DoTextMessage(ttInst1, &txtmsg);
    XCTAssertGreaterThan(cmdid, 0, "Sent user2user text message");
    
    XCTAssert(waitForEvent(ttInst1, CLIENTEVENT_CMD_USER_TEXTMSG, DEF_WAIT, &msg));
    XCTAssertEqual(msg.textmessage.nFromUserID, userid1, "Received own channel message");
    XCTAssert(waitForEvent(ttInst2, CLIENTEVENT_CMD_USER_TEXTMSG, DEF_WAIT, &msg));
    XCTAssertEqual(msg.textmessage.nFromUserID, userid1, "Received userid1 channel message");
}


- (TTInstance*)newClient {
    TTInstance* ttInst = TT_InitTeamTalkPoll();
    [clients addObject:[NSValue valueWithPointer:ttInst]];
    return ttInst;
}

- (void)initSound:(TTInstance*)ttInst {
    XCTAssert(TT_InitSoundInputDevice(ttInst, 0), "Init input sound device");
    XCTAssert(TT_InitSoundOutputDevice(ttInst, 0), "Init output sound device");
}

- (void)connect:(TTInstance *)ttInst ipaddr:(const TTCHAR *)ip
        tcpport:(INT32)tcpport udpport:(INT32)udpport encrypted:(TTBOOL)encrypted {

    XCTAssert(TT_Connect(ttInst, ip, tcpport, udpport, 0, 0, encrypted), "Connect to server");
    
    TTMessage msg;
    XCTAssert(waitForEvent(ttInst, CLIENTEVENT_CON_SUCCESS, DEF_WAIT, &msg), "Wait connect");
}

- (INT32)login:(TTInstance*)ttInst
     nickname:(const TTCHAR*)nick
     username:(const TTCHAR*)usr
     password:(const TTCHAR*)pw {
    
    TTMessage msg;
    
    int cmdid = TT_DoLogin(ttInst, nick, usr, pw);
    XCTAssertGreaterThan(cmdid, 0, "do login");
        
    XCTAssert(waitForEvent(ttInst, CLIENTEVENT_CMD_MYSELF_LOGGEDIN, DEF_WAIT, &msg), "Wait Login");
    int userid = msg.nSource;
    XCTAssert(waitCmdComplete(ttInst, cmdid, DEF_WAIT), "cmd complete");
    
    return userid;
}

- (void)joinRootChannel:(TTInstance*)ttInst {
    INT32 rootid = TT_GetRootChannelID(ttInst);
    INT32 cmdid = TT_DoJoinChannelByID(ttInst, rootid, "");
    XCTAssertGreaterThan(cmdid, 0, "Join root channel command");
    
    XCTAssert(waitCmdSuccess(ttInst, cmdid, DEF_WAIT), "Join root successfull");
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

bool waitForEvent(TTInstance* ttInst, ClientEvent e, int waittimeout, TTMessage* msg) {

    while(TT_GetMessage(ttInst, msg, &waittimeout) && msg->nClientEvent != e) {
        
    }

    return msg->nClientEvent == e;
}

bool waitCmdComplete(TTInstance* ttInst, int cmdid, int waittimeout) {
    TTMessage msg;
    while(waitForEvent(ttInst, CLIENTEVENT_CMD_PROCESSING, waittimeout, &msg)) {
        if(msg.nSource == cmdid && !msg.bActive)
            return true;
    }
    return false;
}

bool waitCmdSuccess(TTInstance* ttInst, int cmdid, int waittimeout) {
    TTMessage msg;
    while(waitForEvent(ttInst, CLIENTEVENT_CMD_SUCCESS, waittimeout, &msg)) {
        if(msg.nSource == cmdid)
            return true;
    }
    return false;
}

@end
