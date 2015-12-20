//
//  ServerViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 8-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class ServerDetailViewController : UIViewController, UITableViewDataSource, UITableViewDelegate, UITextFieldDelegate {

    var server = Server()

    var nameItems = [UITableViewCell]()
    var conItems = [UITableViewCell]()
    var authItems = [UITableViewCell]()
    var actionItems = [UITableViewCell]()
    
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
        namefield = newTableCellTextField(namecell, label: "Name", initial: server.name)
        namefield!.delegate = self
        nameItems.append(namecell)

        // Connection section
        let ipaddrcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        ipaddrfield = newTableCellTextField(ipaddrcell, label: "IP-address", initial: server.ipaddr)
        ipaddrfield!.delegate = self
        ipaddrfield!.keyboardType = .URL
        conItems.append(ipaddrcell)

        let tcpportcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        tcpportfield = newTableCellTextField(tcpportcell, label: "TCP Port", initial: String(server.tcpport))
        tcpportfield!.delegate = self
        tcpportfield!.keyboardType = .NumberPad
        conItems.append(tcpportcell)

        let udpportcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        udpportfield = newTableCellTextField(udpportcell, label: "UDP Port", initial: String(server.udpport))
        udpportfield!.delegate = self
        udpportfield!.keyboardType = .NumberPad
        conItems.append(udpportcell)

        // Authentication section
        let usernamecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        usernamefield = newTableCellTextField(usernamecell, label: "Username", initial: server.username)
        usernamefield!.delegate = self
        usernamefield!.autocorrectionType = .No
        usernamefield!.autocapitalizationType = .None
        authItems.append(usernamecell)
        
        let passwdcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        passwdfield = newTableCellTextField(passwdcell, label: "Password", initial: server.password)
        passwdfield!.delegate = self
        passwdfield!.autocorrectionType = .No
        passwdfield!.autocapitalizationType = .None
        passwdfield!.secureTextEntry = true
        authItems.append(passwdcell)
        
        let connectcell = tableView.dequeueReusableCellWithIdentifier("Connect Server")!
        actionItems.append(connectcell)
        
        let deletecell = tableView.dequeueReusableCellWithIdentifier("Delete Server")!
        actionItems.append(deletecell)
        
        tableView.dataSource = self
        tableView.delegate = self
        
        let def = NSNotificationCenter.defaultCenter()
        def.addObserver(self, selector: "keyboardWillShow:", name: UIKeyboardWillShowNotification, object: nil)
        def.addObserver(self, selector: "keyboardWillHide:", name: UIKeyboardWillHideNotification, object: nil)
    }
    
    func keyboardWillShow(notify: NSNotification) {
        moveForKeyboard(notify, up: true)
    }
    
    func keyboardWillHide(notify: NSNotification) {
        moveForKeyboard(notify, up: false)
    }

    func moveForKeyboard(notify: NSNotification, up: Bool) {
        if let userInfo = notify.userInfo {
            if let keyboardFrame = userInfo[UIKeyboardFrameEndUserInfoKey]?.CGRectValue {
                
                let selfFrame = self.view.frame
                var newTableFrame = tableView.frame
                
                if up {
                    newTableFrame.size.height = selfFrame.height - keyboardFrame.height
                }
                else {
                    var tabBarHeight : CGFloat = 0.0
                    if self.tabBarController != nil {
                        tabBarHeight = (self.tabBarController?.tabBar.frame.size.height)!
                    }
                    
                    newTableFrame.size.height = selfFrame.height - tabBarHeight
                }
                
                tableView.frame = newTableFrame
            }
        }
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
        server.username = usernamefield!.text!
        server.password = passwdfield!.text!
        server.publicserver = false
    }
    
    func textFieldDidBeginEditing(textfield: UITextField) {
        let cell = textfield.superview as! UITableViewCell
        tableView.scrollToRowAtIndexPath(tableView.indexPathForCell(cell)!, atScrollPosition: .Top, animated: true)
    }
    
    //    func textFieldShouldEndEditing(textfield: UITextField) -> Bool {
    //        return true
    //    }
    
    func textFieldShouldReturn(textfield: UITextField) -> Bool {
        textfield.resignFirstResponder()
        return false
    }
    
    func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 4
    }
    
    func tableView(tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch section {
        case 0 :
            return "Server List Entry"
        case 1 :
            return "Connection"
        case 2 :
            return "Authentication"
        case 3 :
            return "Actions"
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
        case 3 :
            return actionItems.count
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
        case 3 :
            return actionItems[indexPath.row]
        default :
            break
        }
        
        return UITableViewCell()
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        if segue.identifier == "Connect From ServerDetail" {
            
            saveServerDetail()
            
            let vc = segue.destinationViewController as! MainTabBarController
            vc.setTeamTalkServer(server)
        }
    }
}
