//
//  ServerViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 8-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class ServerDetailViewController : UIViewController {

    var server = Server()

    @IBOutlet weak var serverName: UITextField!
    @IBOutlet weak var ipaddr: UITextField!
    @IBOutlet weak var tcpport: UITextField!
    @IBOutlet weak var udpport: UITextField!
    @IBOutlet weak var username: UITextField!
    @IBOutlet weak var password: UITextField!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        serverName.text = server.name
        ipaddr.text = server.ipaddr
        tcpport.text = String(server.tcpport)
        udpport.text = String(server.udpport)
        username.text = server.username
        password.text = server.password
    }
}
