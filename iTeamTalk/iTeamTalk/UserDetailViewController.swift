//
//  UserDetailViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 16-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class UserDetailViewController : UIViewController, UITableViewDataSource, UITableViewDelegate {
    
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

    
    var ttInst = UnsafeMutablePointer<Void>()
    var userid : INT32 = 0
    
    let SECTION_GENERAL = 0, SECTION_VOLUME = 1, SECTION_SUBSCRIPTIONS = 2
    
    @IBOutlet weak var tableView: UITableView!
    var general_items = [UITableViewCell]()
    var volume_items = [UITableViewCell]()
    var subscription_items = [UITableViewCell]()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        var user = User()
        
        TT_GetUser(ttInst, userid, &user)
        navtitle.title = String.fromCString(&user.szNickname.0)
        
        let usernamecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        usernamefield = newTableCellTextField(usernamecell, label: "Username", initial: String.fromCString(&user.szUsername.0)!)
        general_items.append(usernamecell)
        
        let voicevolcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        voiceslider = newTableCellSlider(voicevolcell, label: "Voice Volume", min: 0, max: 100, initial: Float(refVolumeToPercent(Int(user.nVolumeVoice))))
        voiceslider!.addTarget(self, action: "voiceVolumeChanged:", forControlEvents: .ValueChanged)
        volume_items.append(voicevolcell)
        
        
        let voicemutecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        voiceswitch = newTableCellSwitch(voicemutecell, label: "Mute Voice", initial: (user.uUserState & USERSTATE_MUTE_VOICE.rawValue) != 0)
        voiceswitch!.addTarget(self, action: "muteVoice:", forControlEvents: .ValueChanged)
        volume_items.append(voicemutecell)

        let mediavolcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        mediaslider = newTableCellSlider(mediavolcell, label: "Media File Volume", min: 0, max: 100, initial: Float(refVolumeToPercent(Int(user.nVolumeMediaFile))))
        mediaslider!.addTarget(self, action: "mediaVolumeChanged:", forControlEvents: .ValueChanged)
        volume_items.append(mediavolcell)
        
        let mediamutecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        mediaswitch = newTableCellSwitch(mediamutecell, label: "Mute Media File", initial: (user.uUserState & USERSTATE_MUTE_MEDIAFILE.rawValue) != 0)
        mediaswitch!.addTarget(self, action: "muteMediaStream:", forControlEvents: .ValueChanged)
        volume_items.append(mediamutecell)
        
        
        let subusermsgcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subusermsgswitch = newTableCellSwitch(subusermsgcell, label: "User Messages", initial: (user.uLocalSubscriptions & SUBSCRIBE_USER_MSG.rawValue) != 0)
        subusermsgswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subusermsgcell)
        
        let subchanmsgcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subchanmsgswitch = newTableCellSwitch(subchanmsgcell, label: "Channel Messages", initial: (user.uLocalSubscriptions & SUBSCRIBE_CHANNEL_MSG.rawValue) != 0)
        subchanmsgswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subchanmsgcell)

        let subbcastmsgcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subbcastmsgswitch = newTableCellSwitch(subbcastmsgcell, label: "Broadcast Messages", initial: (user.uLocalSubscriptions & SUBSCRIBE_BROADCAST_MSG.rawValue) != 0)
        subbcastmsgswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subbcastmsgcell)

        let subvoicecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subvoiceswitch = newTableCellSwitch(subvoicecell, label: "Voice", initial: (user.uLocalSubscriptions & SUBSCRIBE_VOICE.rawValue) != 0)
        subvoiceswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subvoicecell)
        
        let subwebcamcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subwebcamswitch = newTableCellSwitch(subwebcamcell, label: "WebCam", initial: (user.uLocalSubscriptions & SUBSCRIBE_VIDEOCAPTURE.rawValue) != 0)
        subwebcamswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subwebcamcell)
        
        let submediafilecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        submediafileswitch = newTableCellSwitch(submediafilecell, label: "Media File", initial: (user.uLocalSubscriptions & SUBSCRIBE_MEDIAFILE.rawValue) != 0)
        submediafileswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(submediafilecell)
        
        let subdesktopcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subdesktopswitch = newTableCellSwitch(subdesktopcell, label: "Desktop", initial: (user.uLocalSubscriptions & SUBSCRIBE_DESKTOP.rawValue) != 0)
        subdesktopswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subdesktopcell)
        
        tableView.dataSource = self
        tableView.delegate = self

    }

    func voiceVolumeChanged(sender: UISlider) {
        TT_SetUserVolume(ttInst, userid, STREAMTYPE_VOICE, INT32(refVolume(Double(sender.value))))
    }
    
    func mediaVolumeChanged(sender: UISlider) {
        TT_SetUserVolume(ttInst, userid, STREAMTYPE_MEDIAFILE_AUDIO, INT32(refVolume(Double(sender.value))))
    }
    
    func muteVoice(sender: UISwitch) {
        TT_SetUserMute(ttInst, userid, STREAMTYPE_VOICE, (sender.on ? 1 : 0))
    }
    
    func muteMediaStream(sender: UISwitch) {
        TT_SetUserMute(ttInst, userid, STREAMTYPE_MEDIAFILE_AUDIO, (sender.on ? 1 : 0))
    }
    
    func subscriptionChanged(sender: UISwitch) {
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
        
        if sender.on {
            TT_DoSubscribe(ttInst, userid, sub.rawValue)
        }
        else {
            TT_DoUnsubscribe(ttInst, userid, sub.rawValue)
        }
    }
    
    func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 3
    }
    
    func tableView(tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch section {
        case SECTION_GENERAL :
            return "General"
        case SECTION_VOLUME :
            return "Volume Controls"
        case SECTION_SUBSCRIPTIONS :
            return "Subscriptions"
        default :
            return nil
        }
    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch section {
        case SECTION_GENERAL :
            return general_items.count
        case SECTION_VOLUME :
            return volume_items.count
        case SECTION_SUBSCRIPTIONS :
            return subscription_items.count
        default : return 0
        }
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        switch indexPath.section {
        case SECTION_GENERAL :
            return general_items[indexPath.row]
        case SECTION_VOLUME :
            return volume_items[indexPath.row]
        case SECTION_SUBSCRIPTIONS :
            return subscription_items[indexPath.row]
        default : return UITableViewCell()
        }
    }

    
}
