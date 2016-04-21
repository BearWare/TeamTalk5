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
import AVFoundation

class MainTabBarController : UITabBarController, TeamTalkEvent {

    // timer for polling TeamTalk client events
    var polltimer : NSTimer?
    // reconnect timer
    var reconnecttimer : NSTimer?
    // ip-addr and login information for current server
    var server = Server()
    // active command
    var cmdid : INT32 = 0

    let CHANNELTAB = 0, TEXTMSGTAB = 1, PREFTAB = 2

    override func viewDidLoad() {
        super.viewDidLoad()
        
        if let addbtn = self.navigationItem.rightBarButtonItem {
            addbtn.accessibilityHint = NSLocalizedString("Create new channel", comment: "main-tab")
        }

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
        
        let channelsTab = viewControllers?[CHANNELTAB] as! ChannelListViewController
        let chatTab = viewControllers?[TEXTMSGTAB] as! TextMessageViewController
        let prefTab = viewControllers?[PREFTAB] as! PreferencesViewController
        addToTTMessages(channelsTab)
        addToTTMessages(chatTab)
        addToTTMessages(prefTab)

        setupSoundDevices()
        
        let defaults = NSUserDefaults.standardUserDefaults()
        if defaults.objectForKey(PREF_MASTER_VOLUME) != nil {
            let vol = defaults.integerForKey(PREF_MASTER_VOLUME)
            TT_SetSoundOutputVolume(ttInst, INT32(refVolume(Double(vol))))
        }
        
        if defaults.objectForKey(PREF_VOICEACTIVATION) != nil {
            let voiceact = defaults.integerForKey(PREF_VOICEACTIVATION)
            if voiceact != VOICEACT_DISABLED {
                TT_EnableVoiceActivation(ttInst, TRUE)
                TT_SetVoiceActivationLevel(ttInst, INT32(voiceact))
            }
        }
        
        if defaults.objectForKey(PREF_MICROPHONE_GAIN) != nil {
            let vol = defaults.integerForKey(PREF_MICROPHONE_GAIN)
            TT_SetSoundInputGainLevel(ttInst, INT32(refVolume(Double(vol))))
        }
        
        polltimer = NSTimer.scheduledTimerWithTimeInterval(0.1, target: self, selector: #selector(MainTabBarController.timerEvent), userInfo: nil, repeats: true)
        
        let device = UIDevice.currentDevice()
        
        let center = NSNotificationCenter.defaultCenter()
        center.addObserver(self, selector: #selector(MainTabBarController.proximityChanged(_:)), name: UIDeviceProximityStateDidChangeNotification, object: device)

        // detect device changes, e.g. headset plugged in
        center.addObserver(self, selector: #selector(MainTabBarController.audioRouteChange(_:)), name: AVAudioSessionRouteChangeNotification, object: nil)
        
        center.addObserver(self, selector: #selector(MainTabBarController.audioInterruption(_:)), name: AVAudioSessionInterruptionNotification, object: nil)
        
        connectToServer()
    }
    
    deinit {
        // print("Destroyed main view controller")
        if ttInst != nil {
            TT_CloseTeamTalk(ttInst)
            ttInst = nil
        }
    }
    
    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)
        
        let defaults = NSUserDefaults.standardUserDefaults()
        
        let proximity = defaults.objectForKey(PREF_DISPLAY_PROXIMITY) != nil && defaults.boolForKey(PREF_DISPLAY_PROXIMITY)
        if proximity {
            let device = UIDevice.currentDevice()
            device.proximityMonitoringEnabled = true
        }
        
        UIApplication.sharedApplication().beginReceivingRemoteControlEvents()

    }
    
    override func viewDidDisappear(animated: Bool) {
        super.viewDidDisappear(animated)
        
        if self.isMovingFromParentViewController() {
            polltimer?.invalidate()
            reconnecttimer?.invalidate()
            TT_CloseTeamTalk(ttInst)
            ttInst = nil

            ttMessageHandlers.removeAll()
            unreadmessages.removeAll()
        }
        
        let device = UIDevice.currentDevice()
        device.proximityMonitoringEnabled = false
        
        UIApplication.sharedApplication().endReceivingRemoteControlEvents()
    }
    
    override func remoteControlReceivedWithEvent(event: UIEvent?) { // *
        let rc = event!.subtype
        switch rc {
        case .RemoteControlTogglePlayPause:
            let channelsTab = viewControllers?[CHANNELTAB] as! ChannelListViewController
            channelsTab.enableVoiceTx(false)
        case .RemoteControlNextTrack:
            let channelsTab = viewControllers?[CHANNELTAB] as! ChannelListViewController
            channelsTab.enableVoiceTx(true)
        default:
            break
        }
    }
    
    func setTeamTalkServer(server: Server) {
        self.server = server
    }
    
    func startReconnectTimer() {
        reconnecttimer = NSTimer.scheduledTimerWithTimeInterval(5.0, target: self, selector: #selector(MainTabBarController.connectToServer), userInfo: nil, repeats: false)
    }
    
    func connectToServer() {
        
        if TT_Connect(ttInst, server.ipaddr, INT32(server.tcpport), INT32(server.udpport), 0, 0, server.encrypted ? TRUE : FALSE) == FALSE {
            TT_Disconnect(ttInst)
            startReconnectTimer()
        }
    }
    
    // run the TeamTalk event loop
    func timerEvent() {
        var m = TTMessage()
        var n : INT32 = 0
        while TT_GetMessage(ttInst, &m, &n) != FALSE {

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
    
    func proximityChanged(notification: NSNotification) {
        let device = notification.object as! UIDevice
        
        if device.proximityState {
//            print("Proximity state 1")
        }
        else {
//            print("Proximity state 0")
        }
    }
    
    func audioRouteChange(notification: NSNotification) {
//        let session = AVAudioSession.sharedInstance()
//        print("Audio route: " + session.currentRoute.debugDescription)
        if let reason = notification.userInfo![AVAudioSessionRouteChangeReasonKey] {
            
            switch reason as! UInt {
            case AVAudioSessionRouteChangeReason.Unknown.rawValue :
                //print("ChangeReason Unknown")
                break
            case AVAudioSessionRouteChangeReason.NewDeviceAvailable.rawValue :
                //print("ChangeReason NewDeviceAvailable")
                break
            case AVAudioSessionRouteChangeReason.OldDeviceUnavailable.rawValue:
//                print("ChangeReason Unknown")
                setupSpeakerOutput()
            case AVAudioSessionRouteChangeReason.CategoryChange.rawValue:
//                let session = AVAudioSession.sharedInstance()
//                print("ChangeReason CategoryChange, new category: " + session.category)
                break
            case AVAudioSessionRouteChangeReason.Override.rawValue :
//                let session = AVAudioSession.sharedInstance()
//                print("ChangeReason Override, new route: " + session.currentRoute.description)
                break
            case AVAudioSessionRouteChangeReason.RouteConfigurationChange.rawValue :
//                print("ChangeReason RouteConfigurationChange")
                break
            case AVAudioSessionRouteChangeReason.WakeFromSleep.rawValue:
//                print("ChangeReason WakeFromSleep")
                break
            case AVAudioSessionRouteChangeReason.NoSuitableRouteForCategory.rawValue:
//                print("ChangeReason NoSuitableRouteForCategory")
                break
            default :
//                print("ChangeReason Default")
                break
            }
        }
    }

    func audioInterruption(notification: NSNotification) {
        
        // Phone call active/inactive
        if let reason = notification.userInfo![AVAudioSessionInterruptionTypeKey] {
            
            switch reason as! UInt {
            case AVAudioSessionInterruptionType.Began.rawValue :
                //print("Audio interruption begin")
                break
            case AVAudioSessionInterruptionType.Ended.rawValue :
                //print("Audio interruption ended")
                break
            default :
                break
            }
        }
        
        if let reason = notification.userInfo![AVAudioSessionInterruptionOptionKey] {
            
            // when phone call is complete we restart the sound devices
            switch reason as! UInt {
            case AVAudioSessionInterruptionOptions.ShouldResume.rawValue :
                //print("Audio session can now resume")
                
                TT_CloseSoundInputDevice(ttInst)
                TT_CloseSoundOutputDevice(ttInst)
                
                setupSoundDevices()
                
                break
            default :
                break
            }
        }
    }
    
    func handleTTMessage(var m: TTMessage) {
        
        let channelsTab = viewControllers?[CHANNELTAB] as! ChannelListViewController

        switch(m.nClientEvent) {
            
        case CLIENTEVENT_CON_SUCCESS :
            
            var nickname = NSUserDefaults.standardUserDefaults().stringForKey(PREF_NICKNAME)
            if nickname == nil {
                nickname = DEFAULT_NICKNAME
            }
            
            cmdid = TT_DoLoginEx(ttInst, nickname!, server.username, server.password, AppInfo.getAppName())
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
            
            let defaults = NSUserDefaults.standardUserDefaults()
            
            if defaults.objectForKey(PREF_TTSEVENT_CONLOST) == nil || defaults.boolForKey(PREF_TTSEVENT_CONLOST) {
                newUtterance(NSLocalizedString("Connection lost", comment: "tts event"))
            }

            startReconnectTimer()
            
        case CLIENTEVENT_CMD_ERROR :
            let errmsg = getClientErrorMsg(&m).memory
            print(fromTTString(errmsg.szErrorMsg))
            
            if m.nSource == cmdid {
                let s = fromTTString(errmsg.szErrorMsg)
                if #available(iOS 8.0, *) {
                    let alert = UIAlertController(title: NSLocalizedString("Error", comment: "message dialog"), message: s, preferredStyle: UIAlertControllerStyle.Alert)
                    alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "message dialog"), style: UIAlertActionStyle.Default, handler: nil))
                    self.presentViewController(alert, animated: true, completion: nil)
                } else {
                    // Fallback on earlier versions
                }
            }
        
        case CLIENTEVENT_CMD_USER_LOGGEDIN :
            let subs = getDefaultSubscriptions()
            let user = getUser(&m).memory
            if TT_GetMyUserID(ttInst) != user.nUserID && user.uLocalSubscriptions != subs {
                TT_DoUnsubscribe(ttInst, user.nUserID, user.uLocalSubscriptions ^ subs)
            }
            
        case CLIENTEVENT_CMD_USER_JOINED :
            let defaults = NSUserDefaults.standardUserDefaults()
            if let mfvol = defaults.valueForKey(PREF_MEDIAFILE_VOLUME) {
                let user = getUser(&m).memory
                let vol = refVolume(100.0 * Double(mfvol.floatValue!))
                TT_SetUserVolume(ttInst, user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO, INT32(vol))
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
    
    @IBAction func disconnectButtonPressed(sender: UIBarButtonItem) {
        self.navigationController?.popViewControllerAnimated(true)
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
//        let channelsTab = viewControllers?[CHANNELTAB] as! ChannelListViewController
//        channelsTab.prepareForSegue(segue, sender: sender)
        
        for v in viewControllers! {
            v.prepareForSegue(segue, sender: sender)
        }
    }
}
