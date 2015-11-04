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
        let prefTab = viewControllers?[3] as! PreferencesViewController
        channelsTab.ttInst = self.ttInst
        prefTab.ttInst = self.ttInst
        
        let flags = TT_GetFlags(ttInst)
        if flags & CLIENT_SNDINPUT_READY.rawValue == 0 {
            TT_InitSoundInputDevice(ttInst, 0)
        }
        if flags & CLIENT_SNDOUTPUT_READY.rawValue == 0 {
            TT_InitSoundOutputDevice(ttInst, 0)
        }
        
        let defaults = NSUserDefaults.standardUserDefaults()
        if defaults.objectForKey(PREF_MASTER_VOLUME) != nil {
            let vol = defaults.integerForKey(PREF_MASTER_VOLUME)
            TT_SetSoundOutputVolume(ttInst, INT32(refVolume(Double(vol))))
        }
        
        if defaults.objectForKey(PREF_MICROPHONE_GAIN) != nil {
            let vol = defaults.integerForKey(PREF_MICROPHONE_GAIN)
            TT_SetSoundInputGainLevel(ttInst, INT32(refVolume(Double(vol))))
        }
        
        timer = NSTimer.scheduledTimerWithTimeInterval(0.1, target: self, selector: "timerEvent", userInfo: nil, repeats: true)
        
        if TT_Connect(ttInst, server.ipaddr, INT32(server.tcpport), INT32(server.udpport), 0, 0, 0) == 0 {
            print("Failed to connect")
        }
        else {
            
        }
    }
    
    override func viewWillDisappear(animated: Bool) {
        super.viewWillDisappear(animated)
        
    }
    
    override func viewDidDisappear(animated: Bool) {
        super.viewDidDisappear(animated)
        
        if self.isMovingFromParentViewController() {
            timer.invalidate()
            TT_CloseTeamTalk(ttInst)
            print("Destroying TT instance")

            removeFromTTMessages(self)
            assert(ttMessageHandlers.isEmpty)
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

        switch(m.nClientEvent) {
            
        case CLIENTEVENT_CON_SUCCESS :
            print("We're connected")
            var nickname = NSUserDefaults.standardUserDefaults().stringForKey(PREF_NICKNAME)
            if nickname == nil {
                nickname = ""
            }
            
            let cmdid = TT_DoLogin(ttInst, nickname!, server.username, server.password)
            if cmdid > 0 {
                channelsTab.activeCommands[cmdid] = .LoginCmd
            }
            
        case CLIENTEVENT_CON_FAILED :
            print("Connect failed")
            
        case CLIENTEVENT_CON_LOST :
            print("connection lost")
            
        case CLIENTEVENT_CMD_ERROR :
            var errmsg = getClientErrorMsg(&m).memory
            print(String.fromCString(&errmsg.szErrorMsg.0))
        
        case CLIENTEVENT_CMD_USER_LOGGEDIN :
            let subs = getDefaultSubscriptions()
            let user = getUser(&m).memory
            if TT_GetMyUserID(ttInst) != user.nUserID && user.uLocalSubscriptions != subs {
                TT_DoUnsubscribe(ttInst, user.nUserID, user.uLocalSubscriptions ^ subs)
            }
        
        default :
            break
        }
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        let channelsTab = viewControllers?[0] as! ChannelListViewController
        channelsTab.prepareForSegue(segue, sender: sender)
    }
}
