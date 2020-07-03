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

class UserDetailViewController : UITableViewController, TeamTalkEvent {
    
    @IBOutlet weak var navtitle: UINavigationItem!
    var usernamefield: UITextField?
    var voiceslider: UISlider?
    var voiceswitch: UISwitch?
    var mediaslider: UISlider?
    var mediaswitch: UISwitch?
    
    var subusermsgswitch: UISwitch?
    var subchanmsgswitch: UISwitch?
    var subbcastmsgswitch: UISwitch?
    var subvoiceswitch: UISwitch?
    var subwebcamswitch: UISwitch?
    var submediafileswitch: UISwitch?
    var subdesktopswitch: UISwitch?

    
    var userid : INT32 = 0, kick_cmdid : INT32 = 0, kickban_cmdid : INT32 = 0
    
    let SECTION_GENERAL = 0,
        SECTION_VOLUME = 1,
        SECTION_SUBSCRIPTIONS = 2,
        SECTION_ACTIONS = 3,
        SECTION_COUNT = 4
    
    var general_items = [UITableViewCell]()
    var action_items = [UITableViewCell]()
    var volume_items = [UITableViewCell]()
    var subscription_items = [UITableViewCell]()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        var user = User()
        
        TT_GetUser(ttInst, userid, &user)
        navtitle.title = getDisplayName(user)
        
        addToTTMessages(self)
        
        // general items
        let usernamecell = UITableViewCell(style: .default, reuseIdentifier: nil)
        usernamefield = newTableCellTextField(usernamecell, label: NSLocalizedString("Username", comment: "user detail"), initial: getUser(user, strprop: USERNAME))
        general_items.append(usernamecell)
        
        // volume items
        let voicevolcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        voiceslider = newTableCellSlider(voicevolcell, label: NSLocalizedString("Voice Volume", comment: "user detail"), min: 0, max: 100, initial: Float(refVolumeToPercent(Int(user.nVolumeVoice))))
        voiceslider!.addTarget(self, action: #selector(UserDetailViewController.voiceVolumeChanged(_:)), for: .valueChanged)
        volume_items.append(voicevolcell)
        
        let voicemutecell = UITableViewCell(style: .default, reuseIdentifier: nil)
        voiceswitch = newTableCellSwitch(voicemutecell, label: NSLocalizedString("Mute Voice", comment: "user detail"), initial: (user.uUserState & USERSTATE_MUTE_VOICE.rawValue) != 0)
        voiceswitch!.addTarget(self, action: #selector(UserDetailViewController.muteVoice(_:)), for: .valueChanged)
        volume_items.append(voicemutecell)

        let mediavolcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        mediaslider = newTableCellSlider(mediavolcell, label: NSLocalizedString("Media File Volume", comment: "user detail"), min: 0, max: 100, initial: Float(refVolumeToPercent(Int(user.nVolumeMediaFile))))
        mediaslider!.addTarget(self, action: #selector(UserDetailViewController.mediaVolumeChanged(_:)), for: .valueChanged)
        volume_items.append(mediavolcell)
        
        let mediamutecell = UITableViewCell(style: .default, reuseIdentifier: nil)
        mediaswitch = newTableCellSwitch(mediamutecell, label: NSLocalizedString("Mute Media File", comment: "user detail"), initial: (user.uUserState & USERSTATE_MUTE_MEDIAFILE.rawValue) != 0)
        mediaswitch!.addTarget(self, action: #selector(UserDetailViewController.muteMediaStream(_:)), for: .valueChanged)
        volume_items.append(mediamutecell)
        
        // subscription items
        let subusermsgcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        subusermsgswitch = newTableCellSwitch(subusermsgcell, label: NSLocalizedString("User Messages", comment: "user detail"), initial: (user.uLocalSubscriptions & SUBSCRIBE_USER_MSG.rawValue) != 0)
        subusermsgswitch!.addTarget(self, action: #selector(UserDetailViewController.subscriptionChanged(_:)), for: .valueChanged)
        subscription_items.append(subusermsgcell)
        
        let subchanmsgcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        subchanmsgswitch = newTableCellSwitch(subchanmsgcell, label: NSLocalizedString("Channel Messages", comment: "user detail"), initial: (user.uLocalSubscriptions & SUBSCRIBE_CHANNEL_MSG.rawValue) != 0)
        subchanmsgswitch!.addTarget(self, action: #selector(UserDetailViewController.subscriptionChanged(_:)), for: .valueChanged)
        subscription_items.append(subchanmsgcell)

        let subbcastmsgcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        subbcastmsgswitch = newTableCellSwitch(subbcastmsgcell, label: NSLocalizedString("Broadcast Messages", comment: "user detail"), initial: (user.uLocalSubscriptions & SUBSCRIBE_BROADCAST_MSG.rawValue) != 0)
        subbcastmsgswitch!.addTarget(self, action: #selector(UserDetailViewController.subscriptionChanged(_:)), for: .valueChanged)
        subscription_items.append(subbcastmsgcell)

        let subvoicecell = UITableViewCell(style: .default, reuseIdentifier: nil)
        subvoiceswitch = newTableCellSwitch(subvoicecell, label: NSLocalizedString("Voice", comment: "user detail"), initial: (user.uLocalSubscriptions & SUBSCRIBE_VOICE.rawValue) != 0)
        subvoiceswitch!.addTarget(self, action: #selector(UserDetailViewController.subscriptionChanged(_:)), for: .valueChanged)
        subscription_items.append(subvoicecell)
        
        let subwebcamcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        subwebcamswitch = newTableCellSwitch(subwebcamcell, label: NSLocalizedString("WebCam", comment: "user detail"), initial: (user.uLocalSubscriptions & SUBSCRIBE_VIDEOCAPTURE.rawValue) != 0)
        subwebcamswitch!.addTarget(self, action: #selector(UserDetailViewController.subscriptionChanged(_:)), for: .valueChanged)
        subscription_items.append(subwebcamcell)
        
        let submediafilecell = UITableViewCell(style: .default, reuseIdentifier: nil)
        submediafileswitch = newTableCellSwitch(submediafilecell, label: NSLocalizedString("Media File", comment: "user detail"), initial: (user.uLocalSubscriptions & SUBSCRIBE_MEDIAFILE.rawValue) != 0)
        submediafileswitch!.addTarget(self, action: #selector(UserDetailViewController.subscriptionChanged(_:)), for: .valueChanged)
        subscription_items.append(submediafilecell)
        
        let subdesktopcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        subdesktopswitch = newTableCellSwitch(subdesktopcell, label: NSLocalizedString("Desktop", comment: "user detail"), initial: (user.uLocalSubscriptions & SUBSCRIBE_DESKTOP.rawValue) != 0)
        subdesktopswitch!.addTarget(self, action: #selector(UserDetailViewController.subscriptionChanged(_:)), for: .valueChanged)
        subscription_items.append(subdesktopcell)

        // action items
        let kickusercell = tableView.dequeueReusableCell(withIdentifier: "kickcell")
        action_items.append(kickusercell!)
        
        let kickbancell = tableView.dequeueReusableCell(withIdentifier: "kickbancell")
        action_items.append(kickbancell!)

        //        let sendmsgcell = tableView.dequeueReusableCellWithIdentifier("sendmessagecell")
        //        action_items.append(sendmsgcell!)
        
        tableView.dataSource = self
        tableView.delegate = self

    }

    @objc func voiceVolumeChanged(_ sender: UISlider) {
        TT_SetUserVolume(ttInst, userid, STREAMTYPE_VOICE, INT32(refVolume(Double(sender.value))))
    }
    
    @objc func mediaVolumeChanged(_ sender: UISlider) {
        TT_SetUserVolume(ttInst, userid, STREAMTYPE_MEDIAFILE_AUDIO, INT32(refVolume(Double(sender.value))))
    }
    
    @objc func muteVoice(_ sender: UISwitch) {
        TT_SetUserMute(ttInst, userid, STREAMTYPE_VOICE, (sender.isOn ? TRUE : FALSE))
    }
    
    @objc func muteMediaStream(_ sender: UISwitch) {
        TT_SetUserMute(ttInst, userid, STREAMTYPE_MEDIAFILE_AUDIO, (sender.isOn ? TRUE : FALSE))
    }
    
    @objc func subscriptionChanged(_ sender: UISwitch) {
        var sub = SUBSCRIBE_NONE
        
        switch sender {
        case subusermsgswitch! :
            sub = SUBSCRIBE_USER_MSG
        case subchanmsgswitch! :
            sub = SUBSCRIBE_CHANNEL_MSG
        case subbcastmsgswitch! :
            sub = SUBSCRIBE_BROADCAST_MSG
        case subvoiceswitch! :
            sub = SUBSCRIBE_VOICE
        case subwebcamswitch! :
            sub = SUBSCRIBE_VIDEOCAPTURE
        case submediafileswitch! :
            sub = SUBSCRIBE_MEDIAFILE
        case subdesktopswitch! :
            sub = SUBSCRIBE_DESKTOP
        default :
            break
        }
        
        if sender.isOn {
            TT_DoSubscribe(ttInst, userid, sub.rawValue)
        }
        else {
            TT_DoUnsubscribe(ttInst, userid, sub.rawValue)
        }
    }
    
    @IBAction func kickandbanUser(_ sender: UIButton) {
        var user = User()
        TT_GetUser(ttInst, userid, &user)
        
        kickban_cmdid = TT_DoKickUser(ttInst, userid, 0)
    }
    
    @IBAction func kickUser(_ sender: UIButton) {
        var user = User()
        TT_GetUser(ttInst, userid, &user)
        
        kick_cmdid = TT_DoKickUser(ttInst, userid, user.nChannelID)
    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        return SECTION_COUNT
    }
    
    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch section {
        case SECTION_GENERAL :
            return NSLocalizedString("General", comment: "user detail")
        case SECTION_ACTIONS :
            return NSLocalizedString("Actions", comment: "user detail")
        case SECTION_VOLUME :
            return NSLocalizedString("Volume Controls", comment: "user detail")
        case SECTION_SUBSCRIPTIONS :
            return NSLocalizedString("Subscriptions", comment: "user detail")
        default :
            return nil
        }
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch section {
        case SECTION_GENERAL :
            return general_items.count
        case SECTION_ACTIONS :
            return action_items.count
        case SECTION_VOLUME :
            return volume_items.count
        case SECTION_SUBSCRIPTIONS :
            return subscription_items.count
        default : return 0
        }
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        switch indexPath.section {
        case SECTION_GENERAL :
            return general_items[indexPath.row]
        case SECTION_ACTIONS :
            return action_items[indexPath.row]
        case SECTION_VOLUME :
            return volume_items[indexPath.row]
        case SECTION_SUBSCRIPTIONS :
            return subscription_items[indexPath.row]
        default : return UITableViewCell()
        }
    }

    func handleTTMessage(_ m: TTMessage) {
        var m = m
        
        switch m.nClientEvent {
            
        case CLIENTEVENT_CMD_SUCCESS :
            
            if m.nSource == kickban_cmdid {
                TT_DoBanUser(ttInst, userid, 0)
            }
            
        case CLIENTEVENT_CMD_ERROR :
            
            if m.nSource == kick_cmdid || m.nSource == kickban_cmdid {
                var errmsg = getClientErrorMsg(&m).pointee
                let s = String(cString: getClientErrorMsgString(ERRMESSAGE, &errmsg))
                if #available(iOS 8.0, *) {
                    let alert = UIAlertController(title: NSLocalizedString("Error", comment: "user detail"), message: s, preferredStyle: UIAlertController.Style.alert)
                    alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "user detail"), style: UIAlertAction.Style.default, handler: nil))
                    self.present(alert, animated: true, completion: nil)
                    
                } else {
                    // Fallback on earlier versions
                }
            }
        default :
            break
        }
    }
    
}
