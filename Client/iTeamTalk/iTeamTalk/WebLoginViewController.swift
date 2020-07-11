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

class WebLoginViewController : UITableViewController {
    
    var weblogin_items = [UITableViewCell]()
    var authentication_items = [UITableViewCell]()
    
    var usernameField : UITextField?
    var passwordField : UITextField?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        createTableItems()
    }
    
    func createTableItems() {
        
        weblogin_items.removeAll()
        authentication_items.removeAll()
        
        let settings = UserDefaults.standard
        let username = settings.string(forKey: PREF_GENERAL_BEARWARE_ID)
        
        if username == nil {
            let createcell = tableView.dequeueReusableCell(withIdentifier: "Create Web Login")
            weblogin_items.append(createcell!)
        }
        else {
            let resetcell = tableView.dequeueReusableCell(withIdentifier: "Reset Web Login")
            weblogin_items.append(resetcell!)
        }
        
        let usernamecell = UITableViewCell(style: .default, reuseIdentifier: nil)
        let usernameLabel = NSLocalizedString("Username", comment: "Web Login Controller")
        usernameField = newTableCellTextField(usernamecell, label: usernameLabel, initial: username ?? "")
        usernameField!.autocorrectionType = .no
        usernameField!.spellCheckingType = .no
        usernameField!.autocapitalizationType = .none
        usernameField!.isUserInteractionEnabled = username == nil
        
        authentication_items.append(usernamecell)
        
        if username == nil {
            let passwordcell = UITableViewCell(style: .default, reuseIdentifier: nil)
            let passwordLabel = NSLocalizedString("Password", comment: "Web Login Controller")
            passwordField = newTableCellTextField(passwordcell, label: passwordLabel, initial: "")
            passwordField!.autocorrectionType = .no
            passwordField!.spellCheckingType = .no
            passwordField!.autocapitalizationType = .none
            passwordField!.isSecureTextEntry = true
            
            authentication_items.append(passwordcell)
            
            let logincell = tableView.dequeueReusableCell(withIdentifier: "Exec Web Login")
            
            authentication_items.append(logincell!)
        }
    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        return 2
    }

    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch section {
        case 0 :
            return NSLocalizedString("Web Login ID", comment: "Web Login controller")
        case 1 :
            return NSLocalizedString("Authentication", comment: "Web Login controller")
        default:
            return ""
        }
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch section {
        case 0 : return weblogin_items.count
        case 1 : return authentication_items.count
        default:
            return 0
        }
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
     
        switch (indexPath.section) {
        case 0 :
            return weblogin_items[indexPath.row]
        case 1 :
            return authentication_items[indexPath.row]
        default:
            return UITableViewCell()
        }
    }
    
    @IBAction func validateLogin(_ sender: UIButton) {
        
        let username = usernameField!.text!.trimmingCharacters(in: .whitespacesAndNewlines)
        usernameField!.text = username
        
        let url = AppInfo.getBearWareTokenURL(username: username, passwd: passwordField!.text!)
        
        let authParser = WebLoginParser()
        if let parser = XMLParser(contentsOf: URL(string: url)!) {
            
            parser.delegate = authParser
            if parser.parse() && authParser.username.count > 0 {
                let fmtmsg = NSLocalizedString("%@, your username \"%@\" has been validated", comment: "Web Login Controller")
                let loginmsg = String(format: fmtmsg, authParser.nickname, authParser.username)
                let alert = UIAlertView(title: NSLocalizedString("Authenticate", comment: "Web Login Controller"),
                                        message: loginmsg, delegate: nil,
                                        cancelButtonTitle: NSLocalizedString("OK", comment: "Web Login Controller"))
                alert.show()
                
                let settings = UserDefaults.standard
                settings.set(authParser.username, forKey: PREF_GENERAL_BEARWARE_ID)
                settings.set(authParser.token, forKey: PREF_GENERAL_BEARWARE_TOKEN)
                
                createTableItems()
                tableView.reloadData()
            }
            else {
                let alert = UIAlertView(title: NSLocalizedString("Authenticate", comment: "Web Login Controller"),
                                        message: NSLocalizedString("Username or password incorrect", comment: "Web Login Controller"),
                                        delegate: nil, cancelButtonTitle: NSLocalizedString("OK", comment: "Web Login Controller"))
                alert.show()
            }
        }
    }
    
    @IBAction func createWebLogin(_ sender: UIButton) {
        if let url = URL(string: AppInfo.BEARWARE_REGISTRATION_WEBSITE) {
            UIApplication.shared.openURL(url)
        }
    }
    
    @IBAction func resetWebLogin(_ sender: UIButton) {
        
        let settings = UserDefaults.standard
        settings.set(nil, forKey: PREF_GENERAL_BEARWARE_ID)
        settings.set(nil, forKey: PREF_GENERAL_BEARWARE_TOKEN)
        
        usernameField?.text = ""
        passwordField?.text = ""
        
        createTableItems()
        tableView.reloadData()
    }
}


class WebLoginParser : NSObject, XMLParserDelegate {
    
    var elementStack = [String]()
    
    var username = "", nickname = "", token = ""
    
    func parser(_ parser: XMLParser, didStartElement elementName: String,
                namespaceURI: String?, qualifiedName qName: String?,
                attributes attributeDict: [String : String]) {
        
        elementStack.append(elementName)
    }
    
    func parser(_ parser: XMLParser, foundCharacters string: String) {

        let path = getXMLPath(elementStack: elementStack)
        switch path {
        case "/teamtalk/bearware/username" :
            username = string
        case "/teamtalk/bearware/nickname" :
            nickname = string
        case "/teamtalk/bearware/token" :
            token = string
        default :
            print("Unknown path " + path)
        }
    }
    
    func parser(_ parser: XMLParser, didEndElement elementName: String,
                namespaceURI: String?, qualifiedName qName: String?) {
        
        self.elementStack.removeLast()
    }
    
}
