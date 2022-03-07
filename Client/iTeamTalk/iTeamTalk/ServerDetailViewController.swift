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

class ServerDetailViewController : UITableViewController, UITextFieldDelegate {

    var server = Server()

    var nameItems = [UITableViewCell]()
    var conItems = [UITableViewCell]()
    var authItems = [UITableViewCell]()
    var chanItems = [UITableViewCell]()
    var actionItems = [UITableViewCell]()
    
    var namefield : UITextField?
    var ipaddrfield : UITextField?
    var tcpportfield : UITextField?
    var udpportfield : UITextField?
    var encryptedfield : UISwitch?
    var usernamefield : UITextField?
    var passwdfield : UITextField?
    var nicknamefield : UITextField?
    var webloginfield : UISwitch?
    var chanfield : UITextField?
    var chpasswdfield : UITextField?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // ServerList Entry section
        let namecell = UITableViewCell(style: .default, reuseIdentifier: nil)
        namefield = newTableCellTextField(namecell, label: NSLocalizedString("Name", comment: "server entry"), initial: server.name)
        namefield!.delegate = self
        nameItems.append(namecell)

        if server.servertype != .LOCAL {
            let usercountcell = UITableViewCell(style: .value1, reuseIdentifier: nil)
            usercountcell.selectionStyle = .none
            usercountcell.textLabel?.text = NSLocalizedString("Users Online", comment: "server entry")
            usercountcell.detailTextLabel?.text = String(server.stats_usercount)
            nameItems.append(usercountcell)
            
            let motdcell = UITableViewCell(style: .value1, reuseIdentifier: nil)
            motdcell.selectionStyle = .none
            motdcell.textLabel?.text = NSLocalizedString("Message of the Day", comment: "server entry")
            motdcell.detailTextLabel?.text = server.stats_motd
            nameItems.append(motdcell)
            
            let countrycell = UITableViewCell(style: .value1, reuseIdentifier: nil)
            countrycell.selectionStyle = .none
            countrycell.textLabel?.text = NSLocalizedString("Country", comment: "server entry")
            countrycell.detailTextLabel?.text = server.stats_country
            nameItems.append(countrycell)
        }

        // Connection section
        let ipaddrcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        ipaddrfield = newTableCellTextField(ipaddrcell, label: NSLocalizedString("Host address", comment: "server entry"), initial: server.ipaddr)
        ipaddrfield!.delegate = self
        ipaddrfield!.keyboardType = .URL
        ipaddrfield!.spellCheckingType = .no
        ipaddrfield!.autocorrectionType = .no
        ipaddrfield!.autocapitalizationType = .none
        conItems.append(ipaddrcell)

        let tcpportcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        tcpportfield = newTableCellTextField(tcpportcell, label: NSLocalizedString("TCP Port", comment: "server entry"), initial: String(server.tcpport))
        tcpportfield!.delegate = self
        tcpportfield!.keyboardType = .numberPad
        conItems.append(tcpportcell)

        let udpportcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        udpportfield = newTableCellTextField(udpportcell, label: NSLocalizedString("UDP Port", comment: "server entry"), initial: String(server.udpport))
        udpportfield!.delegate = self
        udpportfield!.keyboardType = .numberPad
        conItems.append(udpportcell)

        let encryptedcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        encryptedfield = newTableCellSwitch(encryptedcell, label: NSLocalizedString("Encrypted", comment: "server entry"), initial: server.encrypted)
        conItems.append(encryptedcell)

        // create auth items
        refreshAuthorizationItems(weblogin: AppInfo.isBearWareWebLogin(self.server.username))
        
        //initial channel
        let chancell = UITableViewCell(style: .default, reuseIdentifier: nil)
        chanfield = newTableCellTextField(chancell, label: NSLocalizedString("Channel", comment: "server entry"), initial: server.channel)
        chanfield!.delegate = self
        chanfield!.autocorrectionType = .no
        chanfield!.spellCheckingType = .no
        chanfield!.autocapitalizationType = .none
        chanItems.append(chancell)

        let chpasswdcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        chpasswdfield = newTableCellTextField(chpasswdcell, label: NSLocalizedString("Password", comment: "server entry"), initial: server.chanpasswd)
        chpasswdfield!.delegate = self
        chpasswdfield!.autocorrectionType = .no
        chpasswdfield!.spellCheckingType = .no
        chpasswdfield!.autocapitalizationType = .none
        chpasswdfield!.isSecureTextEntry = true
        chanItems.append(chpasswdcell)

        let connectcell = tableView.dequeueReusableCell(withIdentifier: "Connect Server")!
        actionItems.append(connectcell)
        
        let deletecell = tableView.dequeueReusableCell(withIdentifier: "Delete Server")!
        actionItems.append(deletecell)
        
        tableView.dataSource = self
        tableView.delegate = self
    }
    
    func refreshAuthorizationItems(weblogin: Bool) {
        self.authItems.removeAll()
        
        // Authentication section
        let usernamecell = UITableViewCell(style: .default, reuseIdentifier: nil)
        usernamefield = newTableCellTextField(usernamecell, label: NSLocalizedString("Username", comment: "server entry"), initial: weblogin ? AppInfo.WEBLOGIN_BEARWARE_USERNAME : server.username)
        usernamefield!.delegate = self
        usernamefield!.autocorrectionType = .no
        usernamefield!.spellCheckingType = .no
        usernamefield!.autocapitalizationType = .none
        if weblogin == false {
            authItems.append(usernamecell)
        }
        
        let passwdcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        passwdfield = newTableCellTextField(passwdcell, label: NSLocalizedString("Password", comment: "server entry"), initial: weblogin ? "" : server.password)
        passwdfield!.delegate = self
        passwdfield!.autocorrectionType = .no
        passwdfield!.spellCheckingType = .no
        passwdfield!.autocapitalizationType = .none
        passwdfield!.isSecureTextEntry = true
        
        if weblogin == false {
            authItems.append(passwdcell)
        }

        let weblogincell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        webloginfield = newTableCellSwitch(weblogincell, label: NSLocalizedString("BearWare.dk Web Login", comment: "server entry"), initial: weblogin)
        weblogincell.detailTextLabel?.text = NSLocalizedString("Check only if this server has enabled BearWare.dk Web Login", comment: "server entry")
        webloginfield?.addTarget(self, action: #selector(bearwareWebLogin(_:)), for: .valueChanged)
        authItems.append(weblogincell)

        let nicknamecell = UITableViewCell(style: .default, reuseIdentifier: nil)
        nicknamefield = newTableCellTextField(nicknamecell, label: NSLocalizedString("Nickname (optional)", comment: "server entry"), initial: server.nickname)
        nicknamefield!.delegate = self
        nicknamefield!.autocorrectionType = .no
        nicknamefield!.spellCheckingType = .no
        nicknamefield!.autocapitalizationType = .none
        authItems.append(nicknamecell)
    }
    
    func saveServerDetail() {
        server.name = namefield!.text!
        server.ipaddr = ipaddrfield!.text!
        if let n = Int(tcpportfield!.text!) {
            server.tcpport = n
        }
        if let n = Int(udpportfield!.text!) {
            server.udpport = n
        }
        server.encrypted = encryptedfield!.isOn
        let username = usernamefield!.text!.trimmingCharacters(in: .whitespacesAndNewlines)
        server.username = username
        server.password = passwdfield!.text!
        server.nickname = nicknamefield!.text!
        server.servertype = .LOCAL
        let channame = chanfield!.text!.trimmingCharacters(in: .whitespacesAndNewlines)
        server.channel = channame
        server.chanpasswd = chpasswdfield!.text!
    }
    
    func textFieldShouldReturn(_ textfield: UITextField) -> Bool {
        textfield.resignFirstResponder()
        return false
    }
    
    @objc func bearwareWebLogin(_ sender: UISwitch) {
        refreshAuthorizationItems(weblogin: sender.isOn)
        tableView.reloadData()
    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        return 5
    }
    
    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch section {
        case 0 :
            return NSLocalizedString("Server List Entry", comment: "server entry")
        case 1 :
            return NSLocalizedString("Connection", comment: "server entry")
        case 2 :
            return NSLocalizedString("Authentication", comment: "server entry")
        case 3 :
            return NSLocalizedString("Join Channel", comment: "server entry")
        case 4 :
            return NSLocalizedString("Actions", comment: "server entry")
        default :
            return nil
        }
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        
        switch section {
        case 0 :
            return nameItems.count
        case 1 :
            return conItems.count
        case 2 :
            return authItems.count
        case 3 :
            return chanItems.count
        case 4 :
            return actionItems.count
        default :
            return 0
        }
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        
        switch indexPath.section {
        case 0 :
            return nameItems[indexPath.row]
        case 1 :
            return conItems[indexPath.row]
        case 2 :
            return authItems[indexPath.row]
        case 3 :
            return chanItems[indexPath.row]
        case 4 :
            return actionItems[indexPath.row]
        default :
            break
        }
        
        return UITableViewCell()
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "Connect From ServerDetail" {
            
            saveServerDetail()
            
            let vc = segue.destination as! MainTabBarController
            vc.setTeamTalkServer(server)
        }
    }
}
