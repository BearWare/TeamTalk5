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
    
    var ttInst = UnsafeMutablePointer<Void>()
    var userid : INT32 = 0
    
    @IBOutlet weak var tableView: UITableView!
    var items = [UITableViewCell]()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        var user = User()
        
        TT_GetUser(ttInst, userid, &user)
        navtitle.title = String.fromCString(&user.szNickname.0)
        
        let (usernamecell, usernamefield) = newTableCell("Username", String.fromCString(&user.szUsername.0)!)
        self.usernamefield = usernamefield
        items.append(usernamecell)
        
        let (voicevolcell, voiceslider) = newTableCell("Voice Volume", 0, 100, Float(refVolumeToPercent(Int(user.nVolumeVoice))))
        voiceslider.addTarget(self, action: "voiceVolumeChanged:", forControlEvents: .ValueChanged)
        self.voiceslider = voiceslider
        items.append(voicevolcell)
        
        let (voicemutecell, voiceswitch) = newTableCell("Mute Voice", (user.uUserState & USERSTATE_MUTE_VOICE.value) != 0)
        voiceswitch.addTarget(self, action: "muteVoice:", forControlEvents: .ValueChanged)
        self.voiceswitch = voiceswitch
        items.append(voicemutecell)
        
        let (mediavolcell, mediaslider) = newTableCell("Media File Volume", 0, 100, Float(refVolumeToPercent(Int(user.nVolumeMediaFile))))
        mediaslider.addTarget(self, action: "mediaVolumeChanged:", forControlEvents: .ValueChanged)
        self.mediaslider = mediaslider
        items.append(mediavolcell)
        
        let (mediamutecell, mediaswitch) = newTableCell("Mute Media File", (user.uUserState & USERSTATE_MUTE_MEDIAFILE.value) != 0)
        mediaswitch.addTarget(self, action: "muteMediaStream:", forControlEvents: .ValueChanged)
        self.mediaswitch = mediaswitch
        items.append(mediamutecell)
        
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
    
    func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 1
    }
    
//    func tableView(tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
//    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        
        return items.count
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        
        return items[indexPath.row]
    }

    
}
