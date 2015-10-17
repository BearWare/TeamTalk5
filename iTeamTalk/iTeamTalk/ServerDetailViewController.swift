//
//  ServerViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 8-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class ServerDetailViewController : UIViewController, UITableViewDataSource, UITableViewDelegate {

    var server = Server()

    var nameItems = [UITableViewCell]()
    var conItems = [UITableViewCell]()
    var authItems = [UITableViewCell]()
    
    var namefield : UITextField?
    var ipaddrfield : UITextField?
    var tcpportfield : UITextField?
    var udpportfield : UITextField?
    var usernamefield : UITextField?
    var passwdfield : UITextField?
    
    @IBOutlet weak var tableView: UITableView!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // ServerList Entry section
        let namecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        namefield = newTableCellTextField(namecell, "Name", server.name)
        nameItems.append(namecell)

        // Connection section
        let ipaddrcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        ipaddrfield = newTableCellTextField(ipaddrcell, "IP-address", server.ipaddr)
        conItems.append(ipaddrcell)

        let tcpportcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        tcpportfield = newTableCellTextField(tcpportcell, "TCP Port", String(server.tcpport))
        tcpportfield!.keyboardType = .NumberPad
        conItems.append(tcpportcell)

        let udpportcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        udpportfield = newTableCellTextField(udpportcell, "UDP Port", String(server.udpport))
        udpportfield!.keyboardType = .NumberPad
        conItems.append(udpportcell)

        // Authentication section
        let usernamecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        usernamefield = newTableCellTextField(usernamecell, "Username", server.username)
        usernamefield!.autocorrectionType = .No
        usernamefield!.autocapitalizationType = .None
        authItems.append(usernamecell)
        
        let passwdcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        passwdfield = newTableCellTextField(passwdcell, "Password", server.password)
        passwdfield!.autocorrectionType = .No
        passwdfield!.autocapitalizationType = .None
        passwdfield!.secureTextEntry = true
        authItems.append(passwdcell)
        
        tableView.dataSource = self
        tableView.delegate = self
    }
    
    func saveServerDetail() {
        server.name = namefield!.text
        server.ipaddr = ipaddrfield!.text
        if let n = tcpportfield!.text.toInt() {
            server.tcpport = n
        }
        if let n = udpportfield!.text.toInt() {
            server.udpport = n
        }
        server.username = usernamefield!.text
        server.password = passwdfield!.text
    }
    
    func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 3
    }
    
    func tableView(tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch section {
        case 0 :
            return "Server List Entry"
        case 1 :
            return "Connection"
        case 2 :
            return "Authentication"
        default :
            return nil
        }
    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        
        switch section {
        case 0 :
            return nameItems.count
        case 1 :
            return conItems.count
        case 2 :
            return authItems.count
        default :
            return 0
        }
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        
        switch indexPath.section {
        case 0 :
            return nameItems[indexPath.row]
        case 1 :
            return conItems[indexPath.row]
        case 2 :
            return authItems[indexPath.row]
        default :
            break
        }
        
        return UITableViewCell()
    }
    
}
