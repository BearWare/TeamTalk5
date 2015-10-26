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
        usernamefield = newTableCellTextField(usernamecell, "Username", String.fromCString(&user.szUsername.0)!)
        general_items.append(usernamecell)
        
        let voicevolcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        voiceslider = newTableCellSlider(voicevolcell, "Voice Volume", 0, 100, Float(refVolumeToPercent(Int(user.nVolumeVoice))))
        voiceslider!.addTarget(self, action: "voiceVolumeChanged:", forControlEvents: .ValueChanged)
        volume_items.append(voicevolcell)
        
        
        let voicemutecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        voiceswitch = newTableCellSwitch(voicemutecell, "Mute Voice", (user.uUserState & USERSTATE_MUTE_VOICE.value) != 0)
        voiceswitch!.addTarget(self, action: "muteVoice:", forControlEvents: .ValueChanged)
        volume_items.append(voicemutecell)

        let mediavolcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        mediaslider = newTableCellSlider(mediavolcell, "Media File Volume", 0, 100, Float(refVolumeToPercent(Int(user.nVolumeMediaFile))))
        mediaslider!.addTarget(self, action: "mediaVolumeChanged:", forControlEvents: .ValueChanged)
        volume_items.append(mediavolcell)
        
        let mediamutecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        mediaswitch = newTableCellSwitch(mediamutecell, "Mute Media File", (user.uUserState & USERSTATE_MUTE_MEDIAFILE.value) != 0)
        mediaswitch!.addTarget(self, action: "muteMediaStream:", forControlEvents: .ValueChanged)
        volume_items.append(mediamutecell)
        
        let subusermsgcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subusermsgswitch = newTableCellSwitch(subusermsgcell, "User Messages", (user.uLocalSubscriptions & SUBSCRIBE_USER_MSG.value) != 0)
        subusermsgswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subusermsgcell)
        
        let subchanmsgcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subchanmsgswitch = newTableCellSwitch(subchanmsgcell, "Channel Messages", (user.uLocalSubscriptions & SUBSCRIBE_CHANNEL_MSG.value) != 0)
        subchanmsgswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subchanmsgcell)
        
        let subvoicecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subvoiceswitch = newTableCellSwitch(subvoicecell, "Voice", (user.uLocalSubscriptions & SUBSCRIBE_VOICE.value) != 0)
        subvoiceswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subvoicecell)
        
        let subwebcamcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subwebcamswitch = newTableCellSwitch(subwebcamcell, "WebCam", (user.uLocalSubscriptions & SUBSCRIBE_VIDEOCAPTURE.value) != 0)
        subwebcamswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subwebcamcell)
        
        let submediafilecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        submediafileswitch = newTableCellSwitch(submediafilecell, "Media File", (user.uLocalSubscriptions & SUBSCRIBE_MEDIAFILE.value) != 0)
        submediafileswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(submediafilecell)
        
        let subdesktopcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subdesktopswitch = newTableCellSwitch(subdesktopcell, "Desktop", (user.uLocalSubscriptions & SUBSCRIBE_DESKTOP.value) != 0)
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
        var sub : UINT32 = SUBSCRIBE_NONE.value
        
        switch sender {
        case subusermsgswitch! :
            sub = SUBSCRIBE_USER_MSG.value
        case subchanmsgswitch! :
            sub = SUBSCRIBE_CHANNEL_MSG.value
        case subvoiceswitch! :
            sub = SUBSCRIBE_VOICE.value
        case subwebcamswitch! :
            sub = SUBSCRIBE_VIDEOCAPTURE.value
        case submediafileswitch! :
            sub = SUBSCRIBE_MEDIAFILE.value
        case subdesktopswitch! :
            sub = SUBSCRIBE_DESKTOP.value
        default :
            break
        }
        
        if sender.on {
            TT_DoSubscribe(ttInst, userid, sub)
        }
        else {
            TT_DoUnsubscribe(ttInst, userid, sub)
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
