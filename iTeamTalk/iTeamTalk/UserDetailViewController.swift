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
    
    var user = User()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        navtitle.title = String.fromCString(&user.szNickname.0)
        
        username.text = String.fromCString(&user.szUsername.0)
        voicevolume.minimumValue = Float(SOUND_VOLUME_MIN.value)
        voicevolume.maximumValue = Float(SOUND_VOLUME_MAX.value)
        voicevolume.value = Float(user.nVolumeVoice)
        mediafilevolume.minimumValue = Float(SOUND_VOLUME_MIN.value)
        mediafilevolume.maximumValue = Float(SOUND_VOLUME_MAX.value)
        mediafilevolume.value = Float(user.nVolumeMediaFile)
    }

}
