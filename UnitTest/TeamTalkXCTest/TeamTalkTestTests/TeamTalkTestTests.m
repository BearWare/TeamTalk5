//
//  TeamTalkTestTests.m
//  TeamTalkTestTests
//
//  Created by Bjoern Rasmussen on 4-01-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <XCTest/XCTest.h>

#include <TeamTalk.h>

@interface TeamTalkTestTests : XCTestCase
{
    NSMutableArray* clients;
}
- (TTInstance*)newClient;
- (void)connect:(TTInstance*)ttInst
        ipaddr:(const TTCHAR*)ip
        tcpport:(INT32)tcpport
        udpport:(INT32)udpport
        encrypted:(TTBOOL)encrypted;
- (void)login:  (TTInstance*)ttInst
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
    [self newClient];
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
}

- (void)testConnect {
    TTInstance* ttInst = [self newClient];
    
    TTBOOL b = TT_Connect(ttInst, "tt5eu.bearware.dk", 10335, 10335, 0, 0, FALSE);

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
    
    [self connect:ttInst ipaddr:IPADDR tcpport:TCPPORT udpport:UDPPORT encrypted:ENCRYPTED];
    [self login:ttInst nickname:"testJoinChannel" username:"guest" password:"guest"];
    [self joinRootChannel:ttInst];
    
}

- (TTInstance*)newClient {
    TTInstance* ttInst = TT_InitTeamTalkPoll();
    [clients addObject:[NSValue valueWithPointer:ttInst]];
    return ttInst;
}

- (void)connect:(TTInstance *)ttInst ipaddr:(const TTCHAR *)ip
        tcpport:(INT32)tcpport udpport:(INT32)udpport encrypted:(TTBOOL)encrypted {

    XCTAssert(TT_Connect(ttInst, ip, tcpport, udpport, 0, 0, encrypted), "Connect to server");
    
    TTMessage msg;
    XCTAssert(waitForEvent(ttInst, CLIENTEVENT_CON_SUCCESS, DEF_WAIT, &msg), "Wait connect");
}

- (void)login:(TTInstance*)ttInst
     nickname:(const TTCHAR*)nick
     username:(const TTCHAR*)usr
     password:(const TTCHAR*)pw {
    
    TTMessage msg;
    
    XCTAssertGreaterThan(TT_DoLogin(ttInst, nick, usr, pw), 0, "do login");
        
    XCTAssert(waitForEvent(ttInst, CLIENTEVENT_CMD_MYSELF_LOGGEDIN, DEF_WAIT, &msg), "Wait Login");
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

    double tm_out = waittimeout;
    tm_out /= 1000.0;

    NSDate* now = [NSDate date];
    NSDate *end = [now dateByAddingTimeInterval:tm_out];
    do {
        int wait = 0;
        TT_GetMessage(ttInst, msg, &wait);

        //we need to run the iOS event loop to process audio and video device
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false);
        now = [[NSDate date] laterDate:end];
    }
    while(now == end && msg->nClientEvent != e);

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
