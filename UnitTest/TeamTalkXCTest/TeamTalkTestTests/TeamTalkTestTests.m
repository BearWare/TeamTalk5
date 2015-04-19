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
    TTInstance* ttInst;
}
@end

@implementation TeamTalkTestTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
    ttInst = TT_InitTeamTalkPoll();
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    TT_CloseTeamTalk(ttInst);
    [super tearDown];
}

- (void)testExample {
    // This is an example of a functional test case.
    XCTAssert(YES, @"Pass");
    
    NSString* str = [[NSString alloc]initWithUTF8String:TT_GetVersion()];
    NSLog(@"This is some TTT messsage %@", str);
}

- (void)testConnect {
    TTBOOL b = TT_Connect(ttInst, "tt4eu.bearware.dk", 10335, 10335, 0, 0, FALSE);

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
        }
    }
    

}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

bool waitForEvent(TTInstance* ttInst, ClientEvent e, int waittimeout, TTMessage* msg) {

    while (TT_GetMessage(ttInst, msg, &waittimeout) && msg->nClientEvent != e) {
        
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
