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
    
    @IBOutlet weak var name: UITextField!
    @IBOutlet weak var password: UITextField!
    @IBOutlet weak var topic: UITextField!
    @IBOutlet weak var permanentchannel: UISwitch!
    @IBOutlet weak var nointerruptions: UISwitch!
    @IBOutlet weak var novoiceactivation: UISwitch!
    @IBOutlet weak var noaudiorecording: UISwitch!

    override func viewDidLoad() {
        super.viewDidLoad()
    }
    
    
}
