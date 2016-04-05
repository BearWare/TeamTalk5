/*
* Copyright (c) 2005-2016, BearWare.dk
*
* Contact Information:
*
* Bjoern D. Rasmussen
* Skanderborgvej 40 4-2
* DK-8000 Aarhus C
* Denmark
* Email: contact@bearware.dk
* Phone: +45 20 20 54 59
* Web: http://www.bearware.dk
*
* This source code is part of the TeamTalk 5 SDK owned by
* BearWare.dk. All copyright statements may not be removed
* or altered from any source distribution. If you use this
* software in a product, an acknowledgment in the product
* documentation is required.
*
*/

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
        namefield = newTableCellTextField(namecell, label: NSLocalizedString("Name", comment: "server entry"), initial: server.name)
        namefield!.delegate = self
        nameItems.append(namecell)

        // Connection section
        let ipaddrcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        ipaddrfield = newTableCellTextField(ipaddrcell, label: NSLocalizedString("Host address", comment: "server entry"), initial: server.ipaddr)
        ipaddrfield!.delegate = self
        ipaddrfield!.keyboardType = .URL
        ipaddrfield!.spellCheckingType = .No
        ipaddrfield!.autocorrectionType = .No
        ipaddrfield!.autocapitalizationType = .None
        conItems.append(ipaddrcell)

        let tcpportcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        tcpportfield = newTableCellTextField(tcpportcell, label: NSLocalizedString("TCP Port", comment: "server entry"), initial: String(server.tcpport))
        tcpportfield!.delegate = self
        tcpportfield!.keyboardType = .NumberPad
        conItems.append(tcpportcell)

        let udpportcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        udpportfield = newTableCellTextField(udpportcell, label: NSLocalizedString("UDP Port", comment: "server entry"), initial: String(server.udpport))
        udpportfield!.delegate = self
        udpportfield!.keyboardType = .NumberPad
        conItems.append(udpportcell)

        // Authentication section
        let usernamecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        usernamefield = newTableCellTextField(usernamecell, label: NSLocalizedString("Username", comment: "server entry"), initial: server.username)
        usernamefield!.delegate = self
        usernamefield!.autocorrectionType = .No
        usernamefield!.spellCheckingType = .No
        usernamefield!.autocapitalizationType = .None
        authItems.append(usernamecell)
        
        let passwdcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        passwdfield = newTableCellTextField(passwdcell, label: NSLocalizedString("Password", comment: "server entry"), initial: server.password)
        passwdfield!.delegate = self
        passwdfield!.autocorrectionType = .No
        passwdfield!.spellCheckingType = .No
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
        
        def.addObserver(self, selector: #selector(ServerDetailViewController.keyboardWillShow(_:)), name: UIKeyboardWillShowNotification, object: nil)
        def.addObserver(self, selector: #selector(ServerDetailViewController.keyboardWillHide(_:)), name: UIKeyboardWillHideNotification, object: nil)
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
            return NSLocalizedString("Server List Entry", comment: "server entry")
        case 1 :
            return NSLocalizedString("Connection", comment: "server entry")
        case 2 :
            return NSLocalizedString("Authentication", comment: "server entry")
        case 3 :
            return NSLocalizedString("Actions", comment: "server entry")
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
