//
//  MainTabBarController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 10-10-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class MainTabBarController : UITabBarController, TeamTalkEvent {

    // timer for polling TeamTalk client events
    var timer = NSTimer()
    // our one and only TeamTalk client instance
    var ttInst = UnsafeMutablePointer<Void>()
    // ip-addr and login information for current server
    var server = Server()

    override func viewDidLoad() {
        super.viewDidLoad()
        
//        let img = UIImage(named: "channel_pink.png")
//        let tabImgSize = CGSize(width: 30, height: 30)
//        UIGraphicsBeginImageContextWithOptions(tabImgSize, false, 0.0)
//        img?.drawInRect(CGRectMake(0,0,tabImgSize.width, tabImgSize.height))
//        let newImg = UIGraphicsGetImageFromCurrentImageContext()
//        UIGraphicsEndImageContext()
//        
//        let tab0 = self.tabBar.items?.first
//        
//        tab0?.setBackgroundImage(newImg, forState: .Normal)
//        tab0?.setBackgroundImage(newImg, forState: .Selected)
        
        // Our one and only TT client instance
        ttInst = TT_InitTeamTalkPoll()
        
        addToTTMessages(self)
        
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
    
    deinit {
        println("Destroyed tab ctrl")
    }
    
    override func viewDidDisappear(animated: Bool) {
        super.viewDidDisappear(animated)
        
        if self.isMovingFromParentViewController() {
            timer.invalidate()
            TT_CloseTeamTalk(ttInst)
            println("Destroying TT instance")

            removeFromTTMessages(self)
            
            ttMessageHandlers.removeAll(keepCapacity: false)
        }
    }
    
    func connectToServer(server: Server) {
        self.server = server
    }
    
    // run the TeamTalk event loop
    func timerEvent() {
        var m = TTMessage()
        var n : INT32 = 0
        while TT_GetMessage(ttInst, &m, &n) != 0 {

            for tt in ttMessageHandlers {
                tt.handleTTMessage(m)
            }
        }
    }
    
    func handleTTMessage(var m: TTMessage) {
        
        let channelsTab = viewControllers?[0] as! ChannelListViewController

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
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        let channelsTab = viewControllers?[0] as! ChannelListViewController
        channelsTab.prepareForSegue(segue, sender: sender)
    }
}
