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

import UIKit
import XCTest
import iTeamTalk
import AVFoundation
import Foundation

class iTeamTalkTests: XCTestCase {
    
    let TRUE : TTBOOL = 1, FALSE : TTBOOL = 0
    let DEF_WAIT : INT32 = 5000
    let IPADDR = "tt5eu.bearware.dk"
    let TCPPORT : INT32 = 10335, UDPPORT : INT32 = 10335
    let ENCRYPTED : TTBOOL = 0
    let ADMIN_USERNAME = "admin", ADMIN_PASSWORD = "admin"
    let NICKNAME = "iOS test", USERNAME = "guest", PASSWORD = "guest"
    
    var clients = [UnsafeMutableRawPointer]()
    
    func newClient() -> UnsafeMutableRawPointer {
        let ttInst = TT_InitTeamTalkPoll()
        clients.append(ttInst!)
        return ttInst!
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
        clients.removeAll(keepingCapacity: true)
    }
    
    func testExample() {
        // This is an example of a functional test case.
        XCTAssert(true, "Pass")
        
//        let ttInst = newClient()
//        XCTAssert(TT_CloseTeamTalk(ttInst) != 0, "Closed TT instance")
    }
    
    func testPerformanceExample() {
        // This is an example of a performance test case.
        self.measure() {
            // Put the code you want to measure the time of here.
        }
    }
    
    func testLoopback() {
        let ttInst = newClient()
        var msg = TTMessage()

        let inst1 = TT_StartSoundLoopbackTest(TT_SOUNDDEVICE_ID_REMOTEIO, TT_SOUNDDEVICE_ID_REMOTEIO, 16000, 1, FALSE, nil)

        print("Sound loop is active now")
        
        XCTAssert(inst1 != nil, "inst1 started")
        
        waitForEvent(ttInst, e: CLIENTEVENT_NONE, waittimeout: 5000, msg: &msg)
        
        XCTAssert(TT_CloseSoundLoopbackTest(inst1) == TRUE)
        
        var agc = SpeexDSP()
        
        agc.bEnableAGC = TRUE
        agc.nGainLevel = 8000
        agc.nMaxIncDBSec = 12
        agc.nMaxDecDBSec = -40
        agc.nMaxGainDB = 30
        agc.bEnableDenoise = TRUE
        
        let instAGC = TT_StartSoundLoopbackTest(TT_SOUNDDEVICE_ID_REMOTEIO, TT_SOUNDDEVICE_ID_REMOTEIO, 48000, 1, FALSE, &agc)
        
        waitForEvent(ttInst, e: CLIENTEVENT_NONE, waittimeout: 5000, msg: &msg)
        
        XCTAssert(TT_CloseSoundLoopbackTest(instAGC) == TRUE)

        agc.nGainLevel = 2000
        let instAGC2 = TT_StartSoundLoopbackTest(TT_SOUNDDEVICE_ID_REMOTEIO, TT_SOUNDDEVICE_ID_REMOTEIO, 48000, 1, FALSE, &agc)
        
        waitForEvent(ttInst, e: CLIENTEVENT_NONE, waittimeout: 5000, msg: &msg)
        
        XCTAssert(TT_CloseSoundLoopbackTest(instAGC2) == TRUE)

        let instAGC3 = TT_StartSoundLoopbackTest(TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO, TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO, 48000, 1, FALSE, &agc)
        
        waitForEvent(ttInst, e: CLIENTEVENT_NONE, waittimeout: 5000, msg: &msg)
        
        XCTAssert(TT_CloseSoundLoopbackTest(instAGC3) == TRUE)
    }
    
    // test is invalid. Sound device 1 no longer exists.
    func testSpeaker() {
        
        var n : INT32 = 0
        TT_GetSoundDevices(nil, &n)

        var devs : [SoundDevice] = []
        
        for _ in 1...n {
            let dev = SoundDevice()
            devs.append(dev)
        }
        
        TT_GetSoundDevices(&devs, &n)
        
        for d in devs {
            //to get name for 'd' it has to be mutable
            print("Sound device #\(d.nDeviceID), input channels=\(d.nMaxInputChannels) output channels=\(d.nMaxOutputChannels)")
        }
        
        let ttInst = newClient()
        XCTAssert(TT_InitSoundInputDevice(ttInst, TT_SOUNDDEVICE_ID_REMOTEIO) != 0, "Init sound input")
        XCTAssert(TT_InitSoundOutputDevice(ttInst, TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO) != 0, "Init speaker device")
        
        connect(ttInst, ipaddr: IPADDR, tcpport: TCPPORT, udpport: UDPPORT, encrypted: ENCRYPTED)
        login(ttInst, nickname: NICKNAME, username: USERNAME, password: PASSWORD)
        joinRootChannel(ttInst)
        
        XCTAssert(TT_DBG_SetSoundInputTone(ttInst, UInt32(STREAMTYPE_VOICE.rawValue), 500) != 0, "Set tone")
        let cmdid = TT_DoSubscribe(ttInst, TT_GetMyUserID(ttInst), SUBSCRIBE_VOICE.rawValue)
        XCTAssert(waitCmdComplete(ttInst, cmdid: cmdid, waittimeout: DEF_WAIT), "hear myself")
        
        XCTAssert(TT_EnableVoiceTransmission(ttInst, TRUE) != 0, "Enable voice tx")
        
        var msg = TTMessage()
        waitForEvent(ttInst, e: CLIENTEVENT_NONE, waittimeout: 5000, msg: &msg)

        let ttInst2 = newClient()
        XCTAssert(TT_InitSoundInputDevice(ttInst2, TT_SOUNDDEVICE_ID_REMOTEIO) != 0, "Init sound input")
        XCTAssert(TT_InitSoundOutputDevice(ttInst2, TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO) != 0, "Init speaker device")
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
        
        XCTAssert(TT_InitSoundInputDevice(src1, TT_SOUNDDEVICE_ID_REMOTEIO) != 0, "init sound src1")
        XCTAssert(TT_InitSoundInputDevice(src2, TT_SOUNDDEVICE_ID_REMOTEIO) != 0, "init sound src2")
        
        connect(src1, ipaddr: IPADDR, tcpport: TCPPORT, udpport: UDPPORT, encrypted: ENCRYPTED)
        connect(src2, ipaddr: IPADDR, tcpport: TCPPORT, udpport: UDPPORT, encrypted: ENCRYPTED)
        login(src1, nickname: NICKNAME, username: USERNAME, password: PASSWORD)
        login(src2, nickname: NICKNAME, username: USERNAME, password: PASSWORD)
        joinRootChannel(src1)
        joinRootChannel(src2)
        
        XCTAssert(TT_DBG_SetSoundInputTone(src1, UInt32(STREAMTYPE_VOICE.rawValue), 300) != 0, "Set tone src1")
        XCTAssert(TT_DBG_SetSoundInputTone(src2, UInt32(STREAMTYPE_VOICE.rawValue), 800) != 0, "Set tone src2")
        
        XCTAssert(TT_EnableVoiceTransmission(src1, TRUE) != 0, "Enable voice tx src1")
        XCTAssert(TT_EnableVoiceTransmission(src2, TRUE) != 0, "Enable voice tx src2")

        let player = newClient()
        
        XCTAssert(TT_InitSoundOutputDevice(player, TT_SOUNDDEVICE_ID_REMOTEIO) != 0, "init sound player")
        connect(player, ipaddr: IPADDR, tcpport: TCPPORT, udpport: UDPPORT, encrypted: ENCRYPTED)
        login(player, nickname: NICKNAME, username: USERNAME, password: PASSWORD)
        joinRootChannel(player)

        var msg = TTMessage()
        waitForEvent(player, e: CLIENTEVENT_NONE, waittimeout: 2000, msg: &msg)

        //both are talking
        var user1 = User(), user2 = User()
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src1), &user1) != 0, "get src1 user")
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src2), &user2) != 0, "get src2 user")
        
        XCTAssert(((user1.uUserState & USERSTATE_VOICE.rawValue) != 0), "src1 tx")
        XCTAssert(((user2.uUserState & USERSTATE_VOICE.rawValue) != 0), "src2 tx")

        let cmdid = TT_DoLeaveChannel(player)
        waitCmdSuccess(player, cmdid: cmdid, waittimeout: 2000)

        // stopped talking
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src1), &user1) != 0, "get src1 user")
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src2), &user2) != 0, "get src2 user")
        
        XCTAssert(((user1.uUserState & USERSTATE_VOICE.rawValue) == 0), "src1 tx")
        XCTAssert(((user2.uUserState & USERSTATE_VOICE.rawValue) == 0), "src2 tx")
        
        joinRootChannel(player)

        waitForEvent(player, e: CLIENTEVENT_NONE, waittimeout: 2000, msg: &msg)

        //both are talking
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src1), &user1) != 0, "get src1 user")
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src2), &user2) != 0, "get src2 user")
        
        XCTAssert(((user1.uUserState & USERSTATE_VOICE.rawValue) != 0), "src1 tx")
        XCTAssert(((user2.uUserState & USERSTATE_VOICE.rawValue) != 0), "src2 tx")

        XCTAssert(TT_EnableVoiceTransmission(src1, 0) != 0, "Disable voice tx src1")
        XCTAssert(TT_EnableVoiceTransmission(src2, 0) != 0, "Disable voice tx src2")

        waitForEvent(player, e: CLIENTEVENT_NONE, waittimeout: 5000, msg: &msg)

        // stopped talking
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src1), &user1) != 0, "get src1 user")
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src2), &user2) != 0, "get src2 user")
        
        XCTAssert(((user1.uUserState & USERSTATE_VOICE.rawValue) == 0), "src1 tx")
        XCTAssert(((user2.uUserState & USERSTATE_VOICE.rawValue) == 0), "src2 tx")

        //stream players will reset after 30 seconds
        waitForEvent(player, e: CLIENTEVENT_NONE, waittimeout: 35000, msg: &msg)

        // stopped talking
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src1), &user1) != 0, "get src1 user")
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src2), &user2) != 0, "get src2 user")
        
        XCTAssert(((user1.uUserState & USERSTATE_VOICE.rawValue) == 0), "src1 tx")
        XCTAssert(((user2.uUserState & USERSTATE_VOICE.rawValue) == 0), "src2 tx")

        XCTAssert(TT_EnableVoiceTransmission(src1, 1) != 0, "Disable voice tx src1")
        XCTAssert(TT_EnableVoiceTransmission(src2, 1) != 0, "Disable voice tx src2")
        
        waitForEvent(player, e: CLIENTEVENT_NONE, waittimeout: 2000, msg: &msg)
        
        //both are talking
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src1), &user1) != 0, "get src1 user")
        XCTAssert(TT_GetUser(player, TT_GetMyUserID(src2), &user2) != 0, "get src2 user")
        
        XCTAssert(((user1.uUserState & USERSTATE_VOICE.rawValue) != 0), "src1 tx")
        XCTAssert(((user2.uUserState & USERSTATE_VOICE.rawValue) != 0), "src2 tx")

        XCTAssert(TT_CloseSoundOutputDevice(player) != 0, "Close sound output")
    }

    
    func testAudioInputs() {
        let session = AVAudioSession.sharedInstance()
        
        do {
            try session.setCategory(AVAudioSession.Category.playAndRecord /*, withOptions: AVAudioSessionCategoryOptions.AllowBluetooth*/)
            
            //            try session.setMode(AVAudioSessionModeVoiceChat)
            
            //session.setActive(true, withOptions: AllowBluetooth)
            //            try session.overrideOutputAudioPort( ( on ? AVAudioSessionPortOverride.Speaker : AVAudioSessionPortOverride.None ) )
            
            if let def_input = session.preferredInput {
                print("Preferred input is: " + def_input.portName)
            }
            
            let inputs = session.availableInputs
            
            for a in inputs! {
                print("--- An input ---")
                print("PortName: " + a.portName)
                print("UID: " + a.uid)
                print("PortType: " + a.portType.rawValue)
                
                // only input
                if a.portType == AVAudioSession.Port.lineIn {
                    print("This is line in")
                }
                if a.portType == AVAudioSession.Port.builtInMic {
                    print("This is build in mic")
                }
                if a.portType == AVAudioSession.Port.headsetMic {
                    print("This is headset mic")
                }
                
                // input and output
                if a.portType == AVAudioSession.Port.bluetoothHFP {
                    print("Bluetooth input")
                }
                if a.portType == AVAudioSession.Port.usbAudio {
                    print("USB audio input")
                }
                
                if a.portType == AVAudioSession.Port.headphones {
                    print("This is headphones")
                }
                
                //data sources
                let datasources = a.dataSources!
                
                for d in datasources {
                    print("Data source ID: \(d.dataSourceID)")
                    print("Data source name: " + d.dataSourceName)
                }
                
                if let pref_ds = a.preferredDataSource {
                    print("Preferred data source is " + pref_ds.dataSourceName)
                }
                
                if let sel_ds = a.selectedDataSource {
                    print("Selected data source is " + sel_ds.dataSourceName)
                }
            }
            
            
            print("------- Outputs -------");

            let outputs = session.currentRoute.outputs
            for a in outputs {
                print("--- An output ---")
                print("PortName: " + a.portName)
                print("UID: " + a.uid)
                print("PortType: " + a.portType.rawValue)
                
                if a.portType == AVAudioSession.Port.lineOut {
                    print("This is line out")
                }
                if a.portType == AVAudioSession.Port.headphones {
                    print("This is headphones")
                }
                if a.portType == AVAudioSession.Port.bluetoothA2DP {
                    print("This is Bluetooth A2DP")
                }
                if a.portType == AVAudioSession.Port.builtInReceiver {
                    print("This is BuiltInReceiver")
                }
                if a.portType == AVAudioSession.Port.builtInSpeaker {
                    print("This is Speaker")
                }
                if a.portType == AVAudioSession.Port.HDMI {
                    print("This is HDMI")
                }
                if a.portType == AVAudioSession.Port.airPlay {
                    print("This is AirPlay")
                }
                if a.portType == AVAudioSession.Port.bluetoothLE {
                    print("This is Bluetooth LE")
                }
                
                if let datasources = a.dataSources {
                    for d in datasources {
                        print("Data source ID: \(d.dataSourceID)")
                        print("Data source name: " + d.dataSourceName)
                    }
                }
                
                if let sel_ds = session.outputDataSource {
                    print("Selected data source is " + sel_ds.dataSourceName)
                }
                
            }
            
            
            
            //            let outputs = session.avail
            
            try session.setActive(true)
            
            let ttInst = newClient()
            var msg = TTMessage()
            
            let inst1 = TT_StartSoundLoopbackTest(TT_SOUNDDEVICE_ID_REMOTEIO, TT_SOUNDDEVICE_ID_REMOTEIO, 48000, 1, 0, nil)
            
            print("Sound loop is active now")
            
            waitForEvent(ttInst, e: CLIENTEVENT_NONE, waittimeout: 5000, msg: &msg)
            
            print("Switching to speaker")
            
            try session.overrideOutputAudioPort(AVAudioSession.PortOverride.speaker)
            
            waitForEvent(ttInst, e: CLIENTEVENT_NONE, waittimeout: 5000, msg: &msg)
            
            TT_CloseSoundLoopbackTest(inst1)
            
            
            //print(session.currentRoute)
        }
        catch {
            XCTAssert(false, "Failed")
        }
    }
    
    func testPumpMessage() {

        let ttInst = newClient()
        connect(ttInst, ipaddr: IPADDR, tcpport: TCPPORT, udpport: UDPPORT, encrypted: ENCRYPTED)
        login(ttInst, nickname: NICKNAME, username: USERNAME, password: PASSWORD)
        joinRootChannel(ttInst)
        let myid = TT_GetMyUserID(ttInst)
        var m = TTMessage()
        waitForEvent(ttInst, e: CLIENTEVENT_NONE, waittimeout: 500, msg: &m)
        XCTAssert(TT_PumpMessage(ttInst, CLIENTEVENT_USER_STATECHANGE, myid) == TRUE, "Pump message")
        XCTAssert(waitForEvent(ttInst, e: CLIENTEVENT_USER_STATECHANGE, waittimeout: 500, msg: &m), "Got Pump message")
    }

    func testRestartSoundSystem() {
        let ttInst = newClient()
        initSound(ttInst)
        XCTAssertTrue(TT_RestartSoundSystem() == TRUE, "Restart sound where there are no active clients")
        connect(ttInst, ipaddr: IPADDR, tcpport: TCPPORT, udpport: UDPPORT, encrypted: ENCRYPTED)
        login(ttInst, nickname: NICKNAME, username: USERNAME, password: PASSWORD)
        joinRootChannel(ttInst)
        XCTAssertTrue(TT_RestartSoundSystem() == FALSE, "Cannot restart sound where there are active clients")
        TT_CloseTeamTalk(ttInst)
        
        let ttInst2 = newClient()
        initSound(ttInst2)
        connect(ttInst2, ipaddr: IPADDR, tcpport: TCPPORT, udpport: UDPPORT, encrypted: ENCRYPTED)
        login(ttInst2, nickname: NICKNAME, username: USERNAME, password: PASSWORD)
        joinRootChannel(ttInst2)
        TT_CloseSoundInputDevice(ttInst2)
        TT_CloseSoundOutputDevice(ttInst2)
        XCTAssertTrue(TT_RestartSoundSystem() == TRUE, "Restart sound where there are no active clients")
    }
    
    func testHearMyself() {
        
        do {
            
            let ttInst = newClient()
            
            var msg = TTMessage()

            waitForEvent(ttInst, e: CLIENTEVENT_NONE, waittimeout: 5000, msg: &msg)
            
            initSound(ttInst)
            
            connect(ttInst, ipaddr: IPADDR, tcpport: TCPPORT, udpport: UDPPORT, encrypted: ENCRYPTED)
            login(ttInst, nickname: NICKNAME, username: USERNAME, password: PASSWORD)
            joinRootChannel(ttInst)
            
            XCTAssert(TT_DBG_SetSoundInputTone(ttInst, UInt32(STREAMTYPE_VOICE.rawValue), 800) != 0, "Set tone src1")
            
            XCTAssert(TT_EnableVoiceTransmission(ttInst, TRUE) != 0, "Enable voice tx src1")
            
            waitCmdSuccess(ttInst, cmdid: TT_DoSubscribe(ttInst, TT_GetMyUserID(ttInst), SUBSCRIBE_VOICE.rawValue), waittimeout: 5000)
            
            print("Waiting 5 sec")
            
            waitForEvent(ttInst, e: CLIENTEVENT_NONE, waittimeout: 20000, msg: &msg)
                
            let session = AVAudioSession.sharedInstance()
            print("Switching to speaker")
            try session.overrideOutputAudioPort(AVAudioSession.PortOverride.speaker)
        
            waitForEvent(ttInst, e: CLIENTEVENT_NONE, waittimeout: 5000, msg: &msg)

        }
        catch {
            XCTAssert(false, "Failed")
        }
    }

    func testBluetooth() {
        
        do {
            print("0 -------------------")
            
            let session = AVAudioSession.sharedInstance()
            print("Audio route: " + session.currentRoute.debugDescription)
            
            NotificationCenter.default.addObserver(self, selector: #selector(iTeamTalkTests.audioRouteChangeListenerCallback(_:)), name: AVAudioSession.routeChangeNotification, object: nil)
            
            playSound()
            
            print("1 -------------------")
            
            try session.setMode(AVAudioSession.Mode.default)
            try session.setCategory(AVAudioSession.Category.playAndRecord, options: AVAudioSession.CategoryOptions.allowBluetooth)
            
            playSound()
            
            print("2 -------------------")
            
            let _ = newClient()
            
            playSound()
            
            print("3 -------------------")
        }
        catch {
            print("Error")
        }
    }
    
    func playSound() {
        if let resPath = Bundle.main.path(forResource: "newuser", ofType: "mp3") {
            
            let url = URL(fileURLWithPath: resPath)
            
            do {
                let player = try AVAudioPlayer(contentsOf: url)
                player.prepareToPlay()
                player.play()
                
                while player.isPlaying {
                    sleep(1)
                }
            }
            catch {
                print("Failed to play")
            }
        }
    }
    
    
    func audioRouteChangeListenerCallback(_ notification: Notification) {
        let session = AVAudioSession.sharedInstance()
        print("Audio route: " + session.currentRoute.debugDescription)
        if let reason = notification.userInfo![AVAudioSessionRouteChangeReasonKey] {
            
            switch reason as! UInt {
            case AVAudioSession.RouteChangeReason.unknown.rawValue :
                print("ChangeReason Unknown")
                break
            case AVAudioSession.RouteChangeReason.newDeviceAvailable.rawValue :
                print("ChangeReason NewDeviceAvailable")
                break
            case AVAudioSession.RouteChangeReason.oldDeviceUnavailable.rawValue:
                print("ChangeReason Unknown")
                break
            case AVAudioSession.RouteChangeReason.categoryChange.rawValue:
                let session = AVAudioSession.sharedInstance()
                print("ChangeReason CategoryChange, new category: " + session.category.rawValue)
                break
            case AVAudioSession.RouteChangeReason.override.rawValue :
                let session = AVAudioSession.sharedInstance()
                print("ChangeReason Override, new route: " + session.currentRoute.description)
                break
            case AVAudioSession.RouteChangeReason.routeConfigurationChange.rawValue :
                print("ChangeReason RouteConfigurationChange")
                break
            case AVAudioSession.RouteChangeReason.wakeFromSleep.rawValue:
                print("ChangeReason WakeFromSleep")
                break
            case AVAudioSession.RouteChangeReason.noSuitableRouteForCategory.rawValue:
                print("ChangeReason NoSuitableRouteForCategory")
                break
            default :
                print("ChangeReason Default")
                break
            }
        }
    }
    
    func testProximitySensor() {
        let ttInst = newClient()
        
        initSound(ttInst)
        
        let device = UIDevice.current
        device.isProximityMonitoringEnabled = true

        XCTAssert(device.isProximityMonitoringEnabled, "Proximity sensor ok")
        
        NotificationCenter.default.addObserver(self, selector: #selector(iTeamTalkTests.proximityChanged(_:)), name: UIDevice.proximityStateDidChangeNotification, object: device)

        
        connect(ttInst, ipaddr: IPADDR, tcpport: TCPPORT, udpport: UDPPORT, encrypted: ENCRYPTED)
        login(ttInst, nickname: NICKNAME, username: USERNAME, password: PASSWORD)
        joinRootChannel(ttInst)
        
        var msg = TTMessage()
        waitForEvent(ttInst, e: CLIENTEVENT_NONE, waittimeout: 10000, msg: &msg)

    }
    
    func proximityChanged(_ notification: Notification) {
        let device = notification.object as! UIDevice
        
        if device.proximityState {
            print("Proximity state 1")
        }
        else {
            print("Proximity state 0")
        }
    }
    
    func initSound(_ ttInst: UnsafeMutableRawPointer) {
        XCTAssert(TT_InitSoundInputDevice(ttInst, TT_SOUNDDEVICE_ID_REMOTEIO) != 0, "Init input sound device");
        XCTAssert(TT_InitSoundOutputDevice(ttInst, TT_SOUNDDEVICE_ID_REMOTEIO) != 0, "Init output sound device");
    }
    
    func connect(_ ttInst: UnsafeMutableRawPointer, ipaddr: String, tcpport: INT32, udpport: INT32, encrypted: TTBOOL) {
        
        XCTAssert(TT_Connect(ttInst, ipaddr, tcpport, udpport, 0, 0, encrypted) != 0, "Connect to server")
        
        var msg = TTMessage()

        XCTAssert(waitForEvent(ttInst, e: CLIENTEVENT_CON_SUCCESS, waittimeout: DEF_WAIT, msg: &msg), "Wait connect")
    }
    
    func login(_ ttInst: UnsafeMutableRawPointer, nickname: String, username: String, password: String) {
        
        var msg = TTMessage()
        
        let cmdid = TT_DoLogin(ttInst, nickname, username, password);
        XCTAssertGreaterThan(cmdid, 0, "do login");
        
        XCTAssert(waitForEvent(ttInst, e: CLIENTEVENT_CMD_MYSELF_LOGGEDIN, waittimeout: DEF_WAIT, msg: &msg), "Wait Login")
        let userid = msg.nSource;
        XCTAssertGreaterThan(userid, 0, "user id ok");
        XCTAssert(waitCmdComplete(ttInst, cmdid: cmdid, waittimeout: DEF_WAIT), "cmd complete")
    }
    
    func joinRootChannel(_ ttInst: UnsafeMutableRawPointer) {
        let rootid = TT_GetRootChannelID(ttInst)
        let cmdid = TT_DoJoinChannelByID(ttInst, rootid, "")
        XCTAssertGreaterThan(cmdid, 0, "Join root channel command")
        
        XCTAssert(waitCmdSuccess(ttInst, cmdid: cmdid, waittimeout: DEF_WAIT), "Join root successful")
    }
    
    @discardableResult
    func waitForEvent(_ ttInst : UnsafeMutableRawPointer, e: ClientEvent, waittimeout: INT32, msg: inout TTMessage) -> Bool {
        
        var tmout = waittimeout
        
        while TT_GetMessage(ttInst, &msg, &tmout) != 0 && msg.nClientEvent.rawValue != e.rawValue {
            
        }
        
        return msg.nClientEvent.rawValue == e.rawValue
    }
    
    @discardableResult
    func waitCmdComplete(_ ttInst: UnsafeMutableRawPointer, cmdid: INT32, waittimeout: INT32) -> Bool {
        
        var msg = TTMessage()
        while waitForEvent(ttInst, e: CLIENTEVENT_CMD_PROCESSING, waittimeout: waittimeout, msg: &msg)  {
            if msg.nSource == cmdid && getTTBOOL(&msg) == 0 {
                return true
            }
        }
        return false
    }
    
    @discardableResult
    func waitCmdSuccess(_ ttInst: UnsafeMutableRawPointer, cmdid: INT32, waittimeout: INT32) -> Bool {
        
        var msg = TTMessage()
        while waitForEvent(ttInst, e: CLIENTEVENT_CMD_SUCCESS, waittimeout: waittimeout, msg: &msg) {
            if msg.nSource == cmdid {
                return true
            }
        }
        return false
    }
}
