//
//  MainTabBarController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 10-10-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class MainTabBarController : UITabBarController {

    // timer for polling TeamTalk client events
    var timer = NSTimer()
    // our one and only TeamTalk client instance
    var ttInst = UnsafeMutablePointer<Void>()
    // ip-addr and login information for current server
    var server = Server()

    override func viewDidLoad() {
        super.viewDidLoad()
        
        // Our one and only TT client instance
        ttInst = TT_InitTeamTalkPoll()
        
        let channelsTab = viewControllers?[0] as! ChannelListViewController
        channelsTab.ttInst = self.ttInst
        
        let flags = TT_GetFlags(ttInst)
        if flags & CLIENT_SNDINPUT_READY.value == 0 {
            TT_InitSoundInputDevice(ttInst, 0)
        }
        if flags & CLIENT_SNDOUTPUT_READY.value == 0 {
            TT_InitSoundOutputDevice(ttInst, 0)
        }
        
        timer = NSTimer.scheduledTimerWithTimeInterval(0.1, target: self, selector: "timerEvent", userInfo: nil, repeats: true)
        
        if TT_Connect(ttInst, server.ipaddr, INT32(server.tcpport), INT32(server.udpport), 0, 0, 0) == 0 {
            println("Failed to connect")
        }
        else {
            
        }
    }

    override func viewDidDisappear(animated: Bool) {
        super.viewDidDisappear(animated)
        
        if self.isMovingFromParentViewController() {
            timer.invalidate()
            TT_CloseTeamTalk(ttInst)
            println("Destroying TT instance")
        }
    }
    
    func connectToServer(server: Server) {
        self.server = server
    }
    
    func timerEvent() {
        let channelsTab = viewControllers?[0] as! ChannelListViewController
        var m = TTMessage()
        var n : INT32 = 0
        while TT_GetMessage(ttInst, &m, &n) != 0 {
            
            switch(m.nClientEvent.value) {
                
            case CLIENTEVENT_CON_SUCCESS.value :
                println("We're connected")
                var nickname = NSUserDefaults.standardUserDefaults().stringForKey("nickname_preference")
                if nickname == nil {
                    nickname = ""
                }
                
                let cmdid = TT_DoLogin(ttInst, nickname!, server.username, server.password)
                if cmdid > 0 {
                    channelsTab.activeCommands[cmdid] = .LoginCmd
                }
                
            case CLIENTEVENT_CON_FAILED.value :
                println("Connect failed")
                
            case CLIENTEVENT_CON_LOST.value :
                println("connection lost")
            case CLIENTEVENT_CMD_ERROR.value :
                var errmsg = getClientErrorMsg(&m).memory
                println(String.fromCString(&errmsg.szErrorMsg.0))
            default :
                break
            }
            
            channelsTab.handleTTMessage(m)
        }
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        let channelsTab = viewControllers?[0] as! ChannelListViewController
        channelsTab.prepareForSegue(segue, sender: sender)
    }
}
