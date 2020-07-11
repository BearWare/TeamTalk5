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
import AVFoundation
import FBSDKCoreKit
import FBSDKLoginKit

class MainTabBarController : UITabBarController, UIAlertViewDelegate, TeamTalkEvent {

    // timer for polling TeamTalk client events
    var polltimer : Timer?
    // reconnect timer
    var reconnecttimer : Timer?
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
        
        let defaults = UserDefaults.standard
        if defaults.object(forKey: PREF_MASTER_VOLUME) != nil {
            let vol = defaults.integer(forKey: PREF_MASTER_VOLUME)
            TT_SetSoundOutputVolume(ttInst, INT32(refVolume(Double(vol))))
        }
        
        if defaults.object(forKey: PREF_VOICEACTIVATION) != nil {
            let voiceact = defaults.integer(forKey: PREF_VOICEACTIVATION)
            if voiceact != VOICEACT_DISABLED {
                TT_EnableVoiceActivation(ttInst, TRUE)
                TT_SetVoiceActivationLevel(ttInst, INT32(voiceact))
            }
        }
        
        if defaults.object(forKey: PREF_MICROPHONE_GAIN) != nil {
            let vol = defaults.integer(forKey: PREF_MICROPHONE_GAIN)
            TT_SetSoundInputGainLevel(ttInst, INT32(refVolume(Double(vol))))
        }
        
        polltimer = Timer.scheduledTimer(timeInterval: 0.1, target: self, selector: #selector(MainTabBarController.timerEvent), userInfo: nil, repeats: true)
        
        let device = UIDevice.current
        
        let center = NotificationCenter.default
        center.addObserver(self, selector: #selector(MainTabBarController.proximityChanged(_:)), name: UIDevice.proximityStateDidChangeNotification, object: device)

        // detect device changes, e.g. headset plugged in
        center.addObserver(self, selector: #selector(MainTabBarController.audioRouteChange(_:)), name: AVAudioSession.routeChangeNotification, object: nil)
        
        center.addObserver(self, selector: #selector(MainTabBarController.audioInterruption(_:)), name: AVAudioSession.interruptionNotification, object: nil)
        
        if server.username == AppInfo.WEBLOGIN_FACEBOOK {
            facebookLogin()
        }
        else {
            connectToServer()
        }
    }
    
    deinit {
        // print("Destroyed main view controller")
        if ttInst != nil {
            TT_CloseTeamTalk(ttInst)
            ttInst = nil
        }
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        
        let defaults = UserDefaults.standard
        
        let proximity = defaults.object(forKey: PREF_DISPLAY_PROXIMITY) != nil && defaults.bool(forKey: PREF_DISPLAY_PROXIMITY)
        if proximity {
            let device = UIDevice.current
            device.isProximityMonitoringEnabled = true
        }
        
        // headset can toggle tx
        if defaults.object(forKey: PREF_HEADSET_TXTOGGLE) == nil || defaults.bool(forKey: PREF_HEADSET_TXTOGGLE) {
            UIApplication.shared.beginReceivingRemoteControlEvents()
        }

    }
    
    override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)
        
        if self.isMovingFromParent {
            polltimer?.invalidate()
            reconnecttimer?.invalidate()
            TT_CloseTeamTalk(ttInst)
            ttInst = nil

            ttMessageHandlers.removeAll()
            unreadmessages.removeAll()
        }
        
        let device = UIDevice.current
        device.isProximityMonitoringEnabled = false
        
        UIApplication.shared.endReceivingRemoteControlEvents()
    }
    
    override func remoteControlReceived(with event: UIEvent?) { // *
        let rc = event!.subtype
        switch rc {
        case .remoteControlTogglePlayPause:
            let channelsTab = viewControllers?[CHANNELTAB] as! ChannelListViewController
            channelsTab.enableVoiceTx(false)
        case .remoteControlNextTrack:
            let channelsTab = viewControllers?[CHANNELTAB] as! ChannelListViewController
            channelsTab.enableVoiceTx(true)
        default:
            break
        }
    }
    
    func setTeamTalkServer(_ server: Server) {
        self.server = server
    }
    
    func facebookLogin() {
        if FBSDKAccessToken.current() != nil {
            FBSDKLoginManager().logOut()
        }
        
        let login:FBSDKLoginManager = FBSDKLoginManager()
        login.logIn(withReadPermissions: ["public_profile"], from: self,
                    handler: { (result:FBSDKLoginManagerLoginResult?, error:Error?) in
            
            if error != nil {
                FBSDKLoginManager().logOut()
            } else if (result?.isCancelled)! {
                FBSDKLoginManager().logOut()
                
                let alertView = UIAlertView(title: NSLocalizedString("Facebook login was cancelled", comment: "Dialog message"),
                                            message: NSLocalizedString("Try again?", comment: "Dialog message"), delegate: self,
                                            cancelButtonTitle: NSLocalizedString("No", comment: "Dialog message"),
                                            otherButtonTitles: NSLocalizedString("Yes", comment: "Dialog message"))
                alertView.tag = self.ALERTVIEW_FACEBOOK
                alertView.show()
            } else {
                
                self.server.password = AppInfo.WEBLOGIN_FACEBOOK_PASSWDPREFIX + FBSDKAccessToken.current().tokenString
                self.connectToServer()
            }
        } )
    }
    
    func startReconnectTimer() {
        reconnecttimer = Timer.scheduledTimer(timeInterval: 5.0, target: self, selector: #selector(MainTabBarController.connectToServer), userInfo: nil, repeats: false)
    }
    
    @objc func connectToServer() {
        
        if TT_Connect(ttInst, server.ipaddr, INT32(server.tcpport), INT32(server.udpport), 0, 0, server.encrypted ? TRUE : FALSE) == FALSE {
            TT_Disconnect(ttInst)
            startReconnectTimer()
        }
    }
    
    // run the TeamTalk event loop
    @objc func timerEvent() {
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
    
    @objc func proximityChanged(_ notification: Notification) {
        let device = notification.object as! UIDevice
        
        if device.proximityState {
//            print("Proximity state 1")
        }
        else {
//            print("Proximity state 0")
        }
    }
    
    @objc func audioRouteChange(_ notification: Notification) {
//        let session = AVAudioSession.sharedInstance()
//        print("Audio route: " + session.currentRoute.debugDescription)
        if let reason = notification.userInfo![AVAudioSessionRouteChangeReasonKey] {
            
            switch reason as! UInt {
            case AVAudioSession.RouteChangeReason.unknown.rawValue :
                //print("ChangeReason Unknown")
                break
            case AVAudioSession.RouteChangeReason.newDeviceAvailable.rawValue :
                //print("ChangeReason NewDeviceAvailable")
                break
            case AVAudioSession.RouteChangeReason.oldDeviceUnavailable.rawValue:
//                print("ChangeReason Unknown")
                setupSpeakerOutput()
            case AVAudioSession.RouteChangeReason.categoryChange.rawValue:
//                let session = AVAudioSession.sharedInstance()
//                print("ChangeReason CategoryChange, new category: " + session.category)
                break
            case AVAudioSession.RouteChangeReason.override.rawValue :
//                let session = AVAudioSession.sharedInstance()
//                print("ChangeReason Override, new route: " + session.currentRoute.description)
                break
            case AVAudioSession.RouteChangeReason.routeConfigurationChange.rawValue :
//                print("ChangeReason RouteConfigurationChange")
                break
            case AVAudioSession.RouteChangeReason.wakeFromSleep.rawValue:
//                print("ChangeReason WakeFromSleep")
                break
            case AVAudioSession.RouteChangeReason.noSuitableRouteForCategory.rawValue:
//                print("ChangeReason NoSuitableRouteForCategory")
                break
            default :
//                print("ChangeReason Default")
                break
            }
        }
    }

    @objc func audioInterruption(_ notification: Notification) {
        
        // Phone call active/inactive
        if let reason = notification.userInfo![AVAudioSessionInterruptionTypeKey] {
            
            switch reason as! UInt {
            case AVAudioSession.InterruptionType.began.rawValue :
                //print("Audio interruption begin")
                break
            case AVAudioSession.InterruptionType.ended.rawValue :
                //print("Audio interruption ended")
                break
            default :
                break
            }
        }
        
        if let reason = notification.userInfo![AVAudioSessionInterruptionOptionKey] {
            
            // when phone call is complete we restart the sound devices
            switch reason as! UInt {
            case AVAudioSession.InterruptionOptions.shouldResume.rawValue :
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
    
    func handleTTMessage(_ m: TTMessage) {
        var m = m
        
        switch(m.nClientEvent) {
            
        case CLIENTEVENT_CON_SUCCESS :

            if self.server.username == AppInfo.WEBLOGIN_BEARWARE_USERNAME ||
               self.server.username.hasSuffix(AppInfo.WEBLOGIN_BEARWARE_USERNAMEPOSTFIX) {
                
                var srvprop = ServerProperties()
                TT_GetServerProperties(ttInst, &srvprop)
                
                let settings = UserDefaults.standard
                let username = settings.string(forKey: PREF_GENERAL_BEARWARE_ID) ?? ""
                let token = settings.string(forKey: PREF_GENERAL_BEARWARE_TOKEN) ?? ""
                let accesstoken = String(cString: getServerPropertiesString(ACCESSTOKEN, &srvprop))
                
                let url = AppInfo.getBearWareServerTokenURL(username: username, token: token, accesstoken: accesstoken)
                
                let authParser = WebLoginParser()
                if let parser = XMLParser(contentsOf: URL(string: url)!) {
                    
                    parser.delegate = authParser
                    if parser.parse() && authParser.username.count > 0 {
                        self.server.username = authParser.username
                        self.server.password = AppInfo.WEBLOGIN_BEARWARE_PASSWDPREFIX + authParser.token
                    }
                }
                
                if self.server.username == AppInfo.WEBLOGIN_BEARWARE_USERNAME {
                    let err = NSLocalizedString("BearWare.dk Web Login failed to authenticate. Check BearWare.dk Web Login in Preferences", comment: "weblogin event")
                    let alert = UIAlertController(title: NSLocalizedString("Error", comment: "message dialog"), message: err, preferredStyle: UIAlertController.Style.alert)
                    alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "Dialog message"), style: UIAlertAction.Style.default, handler: nil))
                    self.present(alert, animated: true, completion: nil)
                }
            }
            
            login()
            
        case CLIENTEVENT_CON_FAILED :
            
            TT_Disconnect(ttInst)
            startReconnectTimer()
            
        case CLIENTEVENT_CON_LOST :
            
            TT_Disconnect(ttInst)
            playSound(.srv_LOST)
            
            let defaults = UserDefaults.standard
            
            if defaults.object(forKey: PREF_TTSEVENT_CONLOST) == nil || defaults.bool(forKey: PREF_TTSEVENT_CONLOST) {
                newUtterance(NSLocalizedString("Connection lost", comment: "tts event"))
            }

            startReconnectTimer()
            
        case CLIENTEVENT_VOICE_ACTIVATION :
            
            playSound(getTTBOOL(&m) != 0 ? .voxtriggered_ON : .voxtriggered_OFF)
            
        case CLIENTEVENT_CMD_PROCESSING :
            if getTTBOOL(&m) != 0 {
            }
            else {
                commandComplete(m.nSource)
            }
            
        case CLIENTEVENT_CMD_MYSELF_LOGGEDIN :
            var account = getUserAccount(&m).pointee
            let initchan = String(cString: getUserAccountString(INITCHANNEL, &account))
            if initchan.isEmpty == false {
                server.channel = initchan
            }
            break
            
        case CLIENTEVENT_CMD_ERROR :
            if m.nSource == cmdid {
                var errmsg = getClientErrorMsg(&m).pointee
                let s = String(cString: getClientErrorMsgString(ERRMESSAGE, &errmsg))
                if #available(iOS 8.0, *) {
                    let alert = UIAlertController(title: NSLocalizedString("Error", comment: "message dialog"), message: s, preferredStyle: UIAlertController.Style.alert)
                    alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "message dialog"), style: UIAlertAction.Style.default, handler: nil))
                    self.present(alert, animated: true, completion: nil)
                } else {
                    // Fallback on earlier versions
                }
            }
        
        case CLIENTEVENT_CMD_USER_LOGGEDIN :
            let subs = getDefaultSubscriptions()
            let user = getUser(&m).pointee
            if TT_GetMyUserID(ttInst) != user.nUserID && user.uLocalSubscriptions != subs {
                TT_DoUnsubscribe(ttInst, user.nUserID, user.uLocalSubscriptions ^ subs)
            }
            
        case CLIENTEVENT_CMD_USER_JOINED :
            let defaults = UserDefaults.standard
            if let mfvol = defaults.object(forKey: PREF_MEDIAFILE_VOLUME) {
                let mfvol_double = mfvol as! Double
                let user = getUser(&m).pointee
                let vol = refVolume(100.0 * mfvol_double)
                TT_SetUserVolume(ttInst, user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO, INT32(vol))
            }
            
            let user = getUser(&m).pointee
            if TT_GetMyUserID(ttInst) == user.nUserID {
            }
            
        case CLIENTEVENT_CMD_USER_TEXTMSG :
            
            switch getTextMessage(&m).pointee.nMsgType {
            case MSGTYPE_CHANNEL :
                playSound(.chan_MSG)
            case MSGTYPE_USER :
                playSound(.user_MSG)
            case MSGTYPE_BROADCAST :
                playSound(.broadcast_MSG)
            default : break
            }
        default :
            break
        }
    }
    
    func commandComplete(_ active_cmdid : INT32) {
        
        let channelsTab = viewControllers?[CHANNELTAB] as! ChannelListViewController
        let cmd = channelsTab.activeCommands[active_cmdid]
        
        if cmd == nil {
            return
        }
        
        switch cmd! {
            
        case .loginCmd :
            //setup initial channel
            if server.channel.isEmpty == false {
                
                var path = StringWrap();
                toTTString(server.channel, dst: &path.buf)

                var tokens = server.channel.components(separatedBy: "/")
                
                let chanid = TT_GetChannelIDFromPath(ttInst, fromStringWrap(&path))
                if chanid > 0 {
                    //join existing channel
                    channelsTab.rejoinchannel.nChannelID = chanid
                    toTTString(server.chanpasswd, dst: &channelsTab.rejoinchannel.szPassword)
                }
                else if tokens.count > 0 {
                    // extract path of parent channel
                    let channame = tokens.removeLast()
                    var chanpath = ""
                    for c in tokens {
                        chanpath += "/" + c
                    }
                    
                    toTTString(chanpath, dst: &path.buf)
                    let parentid = TT_GetChannelIDFromPath(ttInst, fromStringWrap(&path))
                    if parentid > 0 {
                        channelsTab.rejoinchannel.nParentID = parentid
                        setChannelString(NAME, &channelsTab.rejoinchannel, channame)
                        setChannelString(PASSWORD, &channelsTab.rejoinchannel, server.chanpasswd)
                        channelsTab.rejoinchannel.audiocodec = newAudioCodec(DEFAULT_AUDIOCODEC)
                    }
                }
                
                //only do the initial login once. ChannelListViewController will
                //handle rejoin
                server.channel.removeAll()
                server.chanpasswd.removeAll()
            }
        default :
            break
        }
    }
    
    func login() {
        
        let channelsTab = viewControllers?[CHANNELTAB] as! ChannelListViewController
        
        let nickname = UserDefaults.standard.string(forKey: PREF_NICKNAME) ?? DEFAULT_NICKNAME
        
        cmdid = TT_DoLoginEx(ttInst, nickname, server.username, server.password, AppInfo.getAppName())
        channelsTab.activeCommands[cmdid] = .loginCmd
        
        reconnecttimer?.invalidate()
    }

    
    @IBAction func disconnectButtonPressed(_ sender: UIBarButtonItem) {
        let servers = loadLocalServers()
        let found = servers.filter({$0.ipaddr == server.ipaddr &&
                                    $0.tcpport == server.tcpport &&
                                    $0.udpport == server.udpport &&
                                    $0.username == server.username})

        if found.count == 0 && server.publicserver == false {
            let alertView = UIAlertView(title: NSLocalizedString("Save server to server list?", comment: "Dialog message"),
                                        message: NSLocalizedString("Save Server", comment: "Dialog message"), delegate: self,
                                        cancelButtonTitle: NSLocalizedString("No", comment: "Dialog message"),
                                        otherButtonTitles: NSLocalizedString("Yes", comment: "Dialog message"))
            alertView.alertViewStyle = .plainTextInput
            alertView.textField(at: 0)?.text = NSLocalizedString("New Server", comment: "Dialog message")
            alertView.tag = ALERTVIEW_SAVESERVER
            alertView.show()
        }
        else {
            self.navigationController!.popViewController(animated: true)
        }
    }

    let ALERTVIEW_SAVESERVER = 1, ALERTVIEW_FACEBOOK = 2
    
    func alertView(_ alertView: UIAlertView, clickedButtonAt buttonIndex: Int) {
        
        switch alertView.tag {
            
        case ALERTVIEW_SAVESERVER :
            // save new server to the list of local servers
            if buttonIndex == 1 {
                
                let name = (alertView.textField(at: 0)?.text)!
                var servers = loadLocalServers()
                // ensure server name doesn't already exist
                servers = servers.filter({$0.name != name})
                server.name = name
                servers.append(server)
                
                saveLocalServers(servers)
            }
            self.navigationController!.popViewController(animated: true)
        case ALERTVIEW_FACEBOOK :
            if buttonIndex == 1 {
                facebookLogin()
            }
            else {
                self.navigationController!.popViewController(animated: true)
            }
        default : break
        }
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
//        let channelsTab = viewControllers?[CHANNELTAB] as! ChannelListViewController
//        channelsTab.prepareForSegue(segue, sender: sender)
        
        for v in viewControllers! {
            v.prepare(for: segue, sender: sender)
        }
    }
}
