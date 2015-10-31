//
//  iTeamTalkTests.swift
//  iTeamTalkTests
//
//  Created by Bjoern Rasmussen on 3-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit
import XCTest
import iTeamTalk

class iTeamTalkTests: XCTestCase {
    
    let DEF_WAIT : INT32 = 5000
    let IPADDR = "tt5eu.bearware.dk"
    let TCPPORT : INT32 = 10335, UDPPORT : INT32 = 10335
    let ENCRYPTED : TTBOOL = 0
    let ADMIN_USERNAME = "admin", ADMIN_PASSWORD = "admin"
    let NICKNAME = "iOS test", USERNAME = "guest", PASSWORD = "guest"
    
    var clients = [UnsafeMutablePointer<Void>]()
    
    func newClient() -> UnsafeMutablePointer<Void> {
        let ttInst = TT_InitTeamTalkPoll()
        clients.append(ttInst)
        return ttInst
    }
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
        
        for m in clients {
            TT_CloseTeamTalk(m)
        }
        clients.removeAll(keepCapacity: true)
    }
    
    func testExample() {
        // This is an example of a functional test case.
        XCTAssert(true, "Pass")
        
//        let ttInst = newClient()
//        XCTAssert(TT_CloseTeamTalk(ttInst) != 0, "Closed TT instance")
    }
    
    func testPerformanceExample() {
        // This is an example of a performance test case.
        self.measureBlock() {
            // Put the code you want to measure the time of here.
        }
    }
    
    func testLoopback() {
        let ttInst = newClient()
        var msg = TTMessage()
        
        let inst1 = TT_StartSoundLoopbackTest(0, 1, 48000, 1, 0, nil)
        let inst2 = TT_StartSoundLoopbackTest(0, 1, 48000, 1, 0, nil)
        
        waitForEvent(ttInst, e: CLIENTEVENT_NONE, waittimeout: 5000, msg: &msg)
        
        TT_CloseSoundLoopbackTest(inst2)
        TT_CloseSoundLoopbackTest(inst1)
    }
    
    func testSpeaker() {
        
        var n : INT32 = 0
        TT_GetSoundDevices(nil, &n)

        var devs : [SoundDevice] = []
        
        for d in 1...n {
            let dev = SoundDevice()
            devs.append(dev)
        }
        
        TT_GetSoundDevices(&devs, &n)
        
        for d in devs {
            //to get name for 'd' it has to be mutable
            println("Sound device #\(d.nDeviceID), input channels=\(d.nMaxInputChannels) output channels=\(d.nMaxOutputChannels)")
        }
        
        let ttInst = newClient()
        XCTAssert(TT_InitSoundInputDevice(ttInst, 0) != 0, "Init sound input")
        XCTAssert(TT_InitSoundOutputDevice(ttInst, 1) != 0, "Init speaker device")
        
        connect(ttInst, ipaddr: IPADDR, tcpport: TCPPORT, udpport: UDPPORT, encrypted: ENCRYPTED)
        login(ttInst, nickname: NICKNAME, username: USERNAME, password: PASSWORD)
        joinRootChannel(ttInst)
        
        XCTAssert(TT_DBG_SetSoundInputTone(ttInst, UInt32(STREAMTYPE_VOICE.value), 500) != 0, "Set tone")
        let cmdid = TT_DoSubscribe(ttInst, TT_GetMyUserID(ttInst), SUBSCRIBE_VOICE.value)
        XCTAssert(waitCmdComplete(ttInst, cmdid: cmdid, waittimeout: DEF_WAIT), "hear myself")
        
        XCTAssert(TT_EnableVoiceTransmission(ttInst, 1) != 0, "Enable voice tx")
        
        var msg = TTMessage()
        waitForEvent(ttInst, e: CLIENTEVENT_NONE, waittimeout: 5000, msg: &msg)

        let ttInst2 = newClient()
        XCTAssert(TT_InitSoundInputDevice(ttInst2, 0) != 0, "Init sound input")
        XCTAssert(TT_InitSoundOutputDevice(ttInst2, 1) != 0, "Init speaker device")
        connect(ttInst2, ipaddr: IPADDR, tcpport: TCPPORT, udpport: UDPPORT, encrypted: ENCRYPTED)
        login(ttInst2, nickname: NICKNAME, username: USERNAME, password: PASSWORD)
        joinRootChannel(ttInst2)
        
        waitForEvent(ttInst2, e: CLIENTEVENT_NONE, waittimeout: 1000, msg: &msg)
        
        XCTAssert(TT_CloseSoundInputDevice(ttInst) != 0, "Close mic device")
        XCTAssert(TT_CloseSoundInputDevice(ttInst2) != 0, "Close mic device")

        XCTAssert(TT_CloseSoundOutputDevice(ttInst) != 0, "Close speaker device")
        XCTAssert(TT_CloseSoundOutputDevice(ttInst2) != 0, "Close speaker device")
    }
    
    func testMultiVoice() {
        let src1 = newClient(), src2 = newClient()
        
        XCTAssert(TT_InitSoundInputDevice(src1, 0) != 0, "init sound src1")
        XCTAssert(TT_InitSoundInputDevice(src2, 0) != 0, "init sound src2")
        
        connect(src1, ipaddr: IPADDR, tcpport: TCPPORT, udpport: UDPPORT, encrypted: ENCRYPTED)
        connect(src2, ipaddr: IPADDR, tcpport: TCPPORT, udpport: UDPPORT, encrypted: ENCRYPTED)
        login(src1, nickname: NICKNAME, username: USERNAME, password: PASSWORD)
        login(src2, nickname: NICKNAME, username: USERNAME, password: PASSWORD)
        joinRootChannel(src1)
        joinRootChannel(src2)
        
        XCTAssert(TT_DBG_SetSoundInputTone(src1, UInt32(STREAMTYPE_VOICE.value), 300) != 0, "Set tone src1")
        XCTAssert(TT_DBG_SetSoundInputTone(src2, UInt32(STREAMTYPE_VOICE.value), 800) != 0, "Set tone src2")
        
        XCTAssert(TT_EnableVoiceTransmission(src1, 1) != 0, "Enable voice tx src1")
        XCTAssert(TT_EnableVoiceTransmission(src2, 1) != 0, "Enable voice tx src2")

        let player = newClient()
        
        XCTAssert(TT_InitSoundOutputDevice(player, 1) != 0, "init sound player")
        connect(player, ipaddr: IPADDR, tcpport: TCPPORT, udpport: UDPPORT, encrypted: ENCRYPTED)
        login(player, nickname: NICKNAME, username: USERNAME, password: PASSWORD)
        joinRootChannel(player)

        var msg = TTMessage()
        waitForEvent(player, e: CLIENTEVENT_NONE, waittimeout: 2000, msg: &msg)

        //both are talking
        var user1 = User(), user2 = User()
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src1), &user1) != 0, "get src1 user")
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src2), &user2) != 0, "get src2 user")
        
        XCTAssert(((user1.uUserState & USERSTATE_VOICE.value) != 0), "src1 tx")
        XCTAssert(((user2.uUserState & USERSTATE_VOICE.value) != 0), "src2 tx")

        let cmdid = TT_DoLeaveChannel(player)
        waitCmdSuccess(player, cmdid: cmdid, waittimeout: 2000)

        // stopped talking
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src1), &user1) != 0, "get src1 user")
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src2), &user2) != 0, "get src2 user")
        
        XCTAssert(((user1.uUserState & USERSTATE_VOICE.value) == 0), "src1 tx")
        XCTAssert(((user2.uUserState & USERSTATE_VOICE.value) == 0), "src2 tx")
        
        joinRootChannel(player)

        waitForEvent(player, e: CLIENTEVENT_NONE, waittimeout: 2000, msg: &msg)

        //both are talking
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src1), &user1) != 0, "get src1 user")
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src2), &user2) != 0, "get src2 user")
        
        XCTAssert(((user1.uUserState & USERSTATE_VOICE.value) != 0), "src1 tx")
        XCTAssert(((user2.uUserState & USERSTATE_VOICE.value) != 0), "src2 tx")

        XCTAssert(TT_EnableVoiceTransmission(src1, 0) != 0, "Disable voice tx src1")
        XCTAssert(TT_EnableVoiceTransmission(src2, 0) != 0, "Disable voice tx src2")

        waitForEvent(player, e: CLIENTEVENT_NONE, waittimeout: 5000, msg: &msg)

        // stopped talking
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src1), &user1) != 0, "get src1 user")
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src2), &user2) != 0, "get src2 user")
        
        XCTAssert(((user1.uUserState & USERSTATE_VOICE.value) == 0), "src1 tx")
        XCTAssert(((user2.uUserState & USERSTATE_VOICE.value) == 0), "src2 tx")

        //stream players will reset after 30 seconds
        waitForEvent(player, e: CLIENTEVENT_NONE, waittimeout: 35000, msg: &msg)

        // stopped talking
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src1), &user1) != 0, "get src1 user")
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src2), &user2) != 0, "get src2 user")
        
        XCTAssert(((user1.uUserState & USERSTATE_VOICE.value) == 0), "src1 tx")
        XCTAssert(((user2.uUserState & USERSTATE_VOICE.value) == 0), "src2 tx")

        XCTAssert(TT_EnableVoiceTransmission(src1, 1) != 0, "Disable voice tx src1")
        XCTAssert(TT_EnableVoiceTransmission(src2, 1) != 0, "Disable voice tx src2")
        
        waitForEvent(player, e: CLIENTEVENT_NONE, waittimeout: 2000, msg: &msg)
        
        //both are talking
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src1), &user1) != 0, "get src1 user")
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src2), &user2) != 0, "get src2 user")
        
        XCTAssert(((user1.uUserState & USERSTATE_VOICE.value) != 0), "src1 tx")
        XCTAssert(((user2.uUserState & USERSTATE_VOICE.value) != 0), "src2 tx")

        XCTAssert(TT_CloseSoundOutputDevice(player) != 0, "Close sound output")
    }
    
    
    func initSound(ttInst: UnsafeMutablePointer<Void>) {
        XCTAssert(TT_InitSoundInputDevice(ttInst, 0) != 0, "Init input sound device");
        XCTAssert(TT_InitSoundOutputDevice(ttInst, 0) != 0, "Init output sound device");
    }
    
    func connect(ttInst: UnsafeMutablePointer<Void>, ipaddr: String, tcpport: INT32, udpport: INT32, encrypted: TTBOOL) {
        
        XCTAssert(TT_Connect(ttInst, ipaddr, tcpport, udpport, 0, 0, encrypted) != 0, "Connect to server")
        
        var msg = TTMessage()

        XCTAssert(waitForEvent(ttInst, e: CLIENTEVENT_CON_SUCCESS, waittimeout: DEF_WAIT, msg: &msg), "Wait connect")
    }
    
    func login(ttInst: UnsafeMutablePointer<Void>, nickname: String, username: String, password: String) -> INT32 {
        
        var msg = TTMessage()
        
        let cmdid = TT_DoLogin(ttInst, nickname, username, password);
        XCTAssertGreaterThan(cmdid, 0, "do login");
        
        XCTAssert(waitForEvent(ttInst, e: CLIENTEVENT_CMD_MYSELF_LOGGEDIN, waittimeout: DEF_WAIT, msg: &msg), "Wait Login")
        let userid = msg.nSource;
        XCTAssert(waitCmdComplete(ttInst, cmdid: cmdid, waittimeout: DEF_WAIT), "cmd complete")
        
        return userid;
    }
    
    func joinRootChannel(ttInst: UnsafeMutablePointer<Void>) {
        let rootid = TT_GetRootChannelID(ttInst)
        let cmdid = TT_DoJoinChannelByID(ttInst, rootid, "")
        XCTAssertGreaterThan(cmdid, 0, "Join root channel command")
        
        XCTAssert(waitCmdSuccess(ttInst, cmdid: cmdid, waittimeout: DEF_WAIT), "Join root successful")
    }
    
    func waitForEvent(ttInst : UnsafeMutablePointer<Void>, e: ClientEvent, waittimeout: INT32, inout msg: TTMessage) -> Bool {
        
        var tmout = waittimeout
        
        while TT_GetMessage(ttInst, &msg, &tmout) != 0 && msg.nClientEvent.value != e.value {
            
        }
        
        return msg.nClientEvent.value == e.value
    }
    
    func waitCmdComplete(ttInst: UnsafeMutablePointer<Void>, cmdid: INT32, waittimeout: INT32) -> Bool {
        
        var msg = TTMessage()
        while waitForEvent(ttInst, e: CLIENTEVENT_CMD_PROCESSING, waittimeout: waittimeout, msg: &msg)  {
            if msg.nSource == cmdid && !getBoolean(&msg) {
                return true
            }
        }
        return false
    }
    
    func waitCmdSuccess(ttInst: UnsafeMutablePointer<Void>, cmdid: INT32, waittimeout: INT32) -> Bool {
        
        var msg = TTMessage()
        while waitForEvent(ttInst, e: CLIENTEVENT_CMD_SUCCESS, waittimeout: waittimeout, msg: &msg) {
            if msg.nSource == cmdid {
                return true
            }
        }
        return false
    }
    
}
