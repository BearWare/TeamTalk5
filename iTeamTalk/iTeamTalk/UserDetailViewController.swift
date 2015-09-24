//
//  UserDetailViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 16-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class UserDetailViewController : UIViewController {
    
    @IBOutlet weak var navtitle: UINavigationItem!
    @IBOutlet weak var username: UITextField!
    @IBOutlet weak var voicevolume: UISlider!
    @IBOutlet weak var mutevoice: UISwitch!
    @IBOutlet weak var mediafilevolume: UISlider!
    @IBOutlet weak var mutemediafile: UISwitch!
    
    var ttInst = UnsafeMutablePointer<Void>()
    var userid : INT32 = 0
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        var user = User()
        
        TT_GetUser(ttInst, userid, &user)
        navtitle.title = String.fromCString(&user.szNickname.0)
        
        username.text = String.fromCString(&user.szUsername.0)
        voicevolume.minimumValue = 0
        voicevolume.maximumValue = 100
        voicevolume.value = Float(refVolumeToPercent(Int(user.nVolumeVoice)))
        mutevoice.setOn((user.uUserState & USERSTATE_MUTE_VOICE.value) == 0, animated: false)
        mediafilevolume.minimumValue = 0
        mediafilevolume.maximumValue = 100
        mediafilevolume.value = Float(refVolumeToPercent(Int(user.nVolumeMediaFile)))
        mutemediafile.setOn((user.uUserState & USERSTATE_MUTE_MEDIAFILE.value) == 0, animated: false)
    }

    @IBAction func voiceVolumeChanged(sender: UISlider) {
        TT_SetUserVolume(ttInst, userid, STREAMTYPE_VOICE, INT32(refVolume(Double(voicevolume.value))))
    }
    
    @IBAction func mediastreamVolumeChanged(sender: UISlider) {
        TT_SetUserVolume(ttInst, userid, STREAMTYPE_MEDIAFILE_AUDIO, INT32(refVolume(Double(voicevolume.value))))
    }
    
    @IBAction func muteVoice(sender: UISwitch) {
        TT_SetUserMute(ttInst, userid, STREAMTYPE_VOICE, (mutevoice.on ? 1 : 0))
    }
    
    @IBAction func muteMediaStream(sender: UISwitch) {
        TT_SetUserMute(ttInst, userid, STREAMTYPE_MEDIAFILE_AUDIO, (mutemediafile.on ? 1 : 0))
    }
    
}
