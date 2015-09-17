//
//  ChannelDetailViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 16-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class ChannelDetailViewController : UIViewController {

    var channel = Channel()
    
    @IBOutlet weak var navitem: UINavigationItem!
    @IBOutlet weak var name: UITextField!
    @IBOutlet weak var password: UITextField!
    @IBOutlet weak var topic: UITextField!
    @IBOutlet weak var permanentchannel: UISwitch!
    @IBOutlet weak var nointerruptions: UISwitch!
    @IBOutlet weak var novoiceactivation: UISwitch!
    @IBOutlet weak var noaudiorecording: UISwitch!

    override func viewDidLoad() {
        super.viewDidLoad()
        
        name.text = String.fromCString(&channel.szName.0)
        password.text = String.fromCString(&channel.szPassword.0)
        topic.text = String.fromCString(&channel.szTopic.0)
        permanentchannel.on = (channel.uChannelType & CHANNEL_PERMANENT.value) != 0
        nointerruptions.on = (channel.uChannelType & CHANNEL_SOLO_TRANSMIT.value) != 0
        novoiceactivation.on = (channel.uChannelType & CHANNEL_NO_VOICEACTIVATION.value) != 0
        noaudiorecording.on = (channel.uChannelType & CHANNEL_NO_RECORDING.value) != 0
        
        if !name.text.isEmpty {
            navitem.title = name.text
        }
    }
    
    func saveChannelDetail() {
        toTTString(name.text!, &channel.szName.0)
        toTTString(password.text!, &channel.szPassword.0)
        toTTString(topic.text!, &channel.szTopic.0)
        if permanentchannel.on {
            channel.uChannelType |= CHANNEL_PERMANENT.value
        }
        if nointerruptions.on {
            channel.uChannelType |= CHANNEL_SOLO_TRANSMIT.value
        }
        if novoiceactivation.on {
            channel.uChannelType |= CHANNEL_NO_VOICEACTIVATION.value
        }
        if noaudiorecording.on {
            channel.uChannelType |= CHANNEL_NO_RECORDING.value
        }
    }
    
}
