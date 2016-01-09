/*
* Copyright (c) 2005-2016, BearWare.dk
*
* Contact Information:
*
* Bjoern D. Rasmussen
* Skanderborgvej 40 4-2
* DK-8000 Aarhus C
* Denmark
* Email: contact@bearware.dk
* Phone: +45 20 20 54 59
* Web: http://www.bearware.dk
*
* This source code is part of the TeamTalk 5 SDK owned by
* BearWare.dk. All copyright statements may not be removed
* or altered from any source distribution. If you use this
* software in a product, an acknowledgment in the product
* documentation is required.
*
*/

import UIKit

class MainTabBarController : UITabBarController, TeamTalkEvent {

    // our one and only TeamTalk client instance
    var ttInst = UnsafeMutablePointer<Void>()
    // timer for polling TeamTalk client events
    var polltimer : NSTimer?
    // reconnect timer
    var reconnecttimer : NSTimer?
    // ip-addr and login information for current server
    var server = Server()
    // active command
    var cmdid : INT32 = 0

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
        let chatTab = viewControllers?[1] as! TextMessageViewController
        let prefTab = viewControllers?[2] as! PreferencesViewController
        channelsTab.ttInst = self.ttInst
        chatTab.ttInst = self.ttInst
        prefTab.ttInst = self.ttInst
        addToTTMessages(channelsTab)
        addToTTMessages(chatTab)

        let defaults = NSUserDefaults.standardUserDefaults()

        let flags = TT_GetFlags(ttInst)
        if flags & CLIENT_SNDINPUT_READY.rawValue == 0 {
            TT_InitSoundInputDevice(ttInst, 0)
        }
        
        let speaker_output = defaults.objectForKey(PREF_SPEAKER_OUTPUT) != nil && defaults.boolForKey(PREF_SPEAKER_OUTPUT)
        let sndoutid : INT32 = (speaker_output) ? 1 : 0
        enableSpeakerOutput(speaker_output)
        
        if flags & CLIENT_SNDOUTPUT_READY.rawValue == 0 {
            TT_InitSoundOutputDevice(ttInst, sndoutid)
        }
        
        if defaults.objectForKey(PREF_MASTER_VOLUME) != nil {
            let vol = defaults.integerForKey(PREF_MASTER_VOLUME)
            TT_SetSoundOutputVolume(ttInst, INT32(refVolume(Double(vol))))
        }
        
        if defaults.objectForKey(PREF_VOICEACTIVATION) != nil {
            let voiceact = defaults.integerForKey(PREF_VOICEACTIVATION)
            if voiceact != VOICEACT_DISABLED {
                TT_EnableVoiceActivation(ttInst, 1)
                TT_SetVoiceActivationLevel(ttInst, INT32(voiceact))
            }
        }
        
        if defaults.objectForKey(PREF_MICROPHONE_GAIN) != nil {
            let vol = defaults.integerForKey(PREF_MICROPHONE_GAIN)
            TT_SetSoundInputGainLevel(ttInst, INT32(refVolume(Double(vol))))
        }
        
        polltimer = NSTimer.scheduledTimerWithTimeInterval(0.1, target: self, selector: "timerEvent", userInfo: nil, repeats: true)
        
        connectToServer()
    }
    
    override func viewDidDisappear(animated: Bool) {
        super.viewDidDisappear(animated)
        
        if self.isMovingFromParentViewController() {
            polltimer?.invalidate()
            reconnecttimer?.invalidate()
            TT_CloseTeamTalk(ttInst)

            ttMessageHandlers.removeAll()
            unreadmessages.removeAll()
        }
    }
    
    func setTeamTalkServer(server: Server) {
        self.server = server
    }
    
    func startReconnectTimer() {
        reconnecttimer = NSTimer.scheduledTimerWithTimeInterval(5.0, target: self, selector: "connectToServer", userInfo: nil, repeats: false)
    }
    
    func connectToServer() {
        
        if TT_Connect(ttInst, server.ipaddr, INT32(server.tcpport), INT32(server.udpport), 0, 0, 0) == 0 {
            TT_Disconnect(ttInst)
            startReconnectTimer()
        }
    }
    
    // run the TeamTalk event loop
    func timerEvent() {
        var m = TTMessage()
        var n : INT32 = 0
        while TT_GetMessage(ttInst, &m, &n) != 0 {

            for tt in ttMessageHandlers {
                if tt.value == nil {
                    removeFromTTMessages(tt)
                }
                else {
                    tt.value!.handleTTMessage(m)
                }
            }
        }
    }
    
    func handleTTMessage(var m: TTMessage) {
        
        let channelsTab = viewControllers?[0] as! ChannelListViewController

        switch(m.nClientEvent) {
            
        case CLIENTEVENT_CON_SUCCESS :
            
            var nickname = NSUserDefaults.standardUserDefaults().stringForKey(PREF_NICKNAME)
            if nickname == nil {
                nickname = ""
            }
            
            cmdid = TT_DoLogin(ttInst, nickname!, server.username, server.password)
            if cmdid > 0 {
                channelsTab.activeCommands[cmdid] = .LoginCmd
            }
            
            reconnecttimer?.invalidate()
            
        case CLIENTEVENT_CON_FAILED :
            
            TT_Disconnect(ttInst)
            startReconnectTimer()
            
        case CLIENTEVENT_CON_LOST :
            
            TT_Disconnect(ttInst)
            playSound(.SRV_LOST)
            
            startReconnectTimer()
            
        case CLIENTEVENT_CMD_ERROR :
            let errmsg = getClientErrorMsg(&m).memory
            print(fromTTString(errmsg.szErrorMsg))
            
            if m.nSource == cmdid {
                let s = fromTTString(errmsg.szErrorMsg)
                let alert = UIAlertController(title: NSLocalizedString("Error", comment: "message dialog"), message: s, preferredStyle: UIAlertControllerStyle.Alert)
                alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "message dialog"), style: UIAlertActionStyle.Default, handler: nil))
                self.presentViewController(alert, animated: true, completion: nil)
            }
        
        case CLIENTEVENT_CMD_USER_LOGGEDIN :
            let subs = getDefaultSubscriptions()
            let user = getUser(&m).memory
            if TT_GetMyUserID(ttInst) != user.nUserID && user.uLocalSubscriptions != subs {
                TT_DoUnsubscribe(ttInst, user.nUserID, user.uLocalSubscriptions ^ subs)
            }
        
        case CLIENTEVENT_CMD_USER_TEXTMSG :
            
            switch getTextMessage(&m).memory.nMsgType {
            case MSGTYPE_CHANNEL :
                playSound(.CHAN_MSG)
            case MSGTYPE_USER :
                playSound(.USER_MSG)
            default : break
            }
        default :
            break
        }
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
//        let channelsTab = viewControllers?[0] as! ChannelListViewController
//        channelsTab.prepareForSegue(segue, sender: sender)
        
        for v in viewControllers! {
            v.prepareForSegue(segue, sender: sender)
        }
    }
}
