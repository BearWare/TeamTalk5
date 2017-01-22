/*
* Copyright (c) 2005-2016, BearWare.dk
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
* This source code is part of the TeamTalk 5 SDK owned by
* BearWare.dk. All copyright statements may not be removed
* or altered from any source distribution. If you use this
* software in a product, an acknowledgment in the product
* documentation is required.
*
*/

import UIKit

// Properties of a TeamTalk server to connect to
class Server : NSObject {
    var name = ""
    var ipaddr = ""
    var tcpport = AppInfo.DEFAULT_TCPPORT
    var udpport = AppInfo.DEFAULT_UDPPORT
    var username = ""
    var password = ""
    var channel = ""
    var chanpasswd = ""
    var publicserver = false
    var encrypted = false
    
    override init() {
        
    }
    
    init(coder dec: NSCoder!) {
        name = dec.decodeObject(forKey: "name") as! String
        ipaddr = dec.decodeObject(forKey: "ipaddr") as! String
        tcpport = dec.decodeInteger(forKey: "tcpport")
        udpport = dec.decodeInteger(forKey: "udpport")
        username = dec.decodeObject(forKey: "username") as! String
        password = dec.decodeObject(forKey: "password") as! String
        channel = dec.decodeObject(forKey: "channel") as! String
        chanpasswd = dec.decodeObject(forKey: "chanpasswd") as! String
    }
    
    func encodeWithCoder(_ enc: NSCoder!) {
        enc.encode(name, forKey: "name")
        enc.encode(ipaddr, forKey: "ipaddr")
        enc.encode(tcpport, forKey: "tcpport")
        enc.encode(udpport, forKey: "udpport")
        enc.encode(username, forKey: "username")
        enc.encode(password, forKey: "password")
        enc.encode(channel, forKey: "channel")
        enc.encode(chanpasswd, forKey: "chanpasswd")
    }
}

func loadLocalServers() -> [Server] {
    
    var servers = [Server]()
    
    let defaults = UserDefaults.standard
    if let stored = defaults.array(forKey: "ServerList") {
        for e in stored {
            let data = e as! Data
            
            let server = NSKeyedUnarchiver.unarchiveObject(with: data) as! Server
            
            servers.append(server)
        }
    }
    return servers
}

func saveLocalServers(_ servers : [Server]) {
    //Store local servers
    let defaults = UserDefaults.standard
    var s_array = [Data]()
    for s in servers {
        let data = NSKeyedArchiver.archivedData(withRootObject: s)
        s_array.append(data)
    }
    defaults.set(s_array, forKey: "ServerList")
    defaults.synchronize()
}

class ServerListViewController : UITableViewController,
    XMLParserDelegate {
    
    // server for segue
    var currentServer = Server()
    // list of available servers
    var servers = [Server]()
    
    var nextappupdate = Date()

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        
        if let addbtn = self.navigationItem.rightBarButtonItem {
            addbtn.accessibilityHint = NSLocalizedString("Add new server entry", comment: "serverlist")
        }
        if let setupbtn = self.navigationItem.leftBarButtonItem {
            setupbtn.accessibilityLabel = NSLocalizedString("Preferences", comment: "serverlist")
            setupbtn.accessibilityHint = NSLocalizedString("Access preferences", comment: "serverlist")
        }
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)

        servers.removeAll()
        
        servers = loadLocalServers()
        
        let defaults = UserDefaults.standard
        if defaults.object(forKey: PREF_DISPLAY_PUBSERVERS) == nil || defaults.bool(forKey: PREF_DISPLAY_PUBSERVERS) {
            Timer.scheduledTimer(timeInterval: 1.0, target: self, selector: #selector(ServerListViewController.downloadServerList), userInfo: nil, repeats: false)
        }

        tableView.reloadData()
        
        if (nextappupdate as NSDate).earlierDate(Date()) == nextappupdate {
            Timer.scheduledTimer(timeInterval: 2.0, target: self, selector: #selector(ServerListViewController.checkAppUpdate), userInfo: nil, repeats: false)
        }
        
    }
    
    func checkAppUpdate() {

        // check for new version
        let updateparser = AppUpdateParser()
        
        let parser = XMLParser(contentsOf: URL(string: AppInfo.getUpdateURL())!)!
        parser.delegate = updateparser
        parser.parse()
        
        nextappupdate = nextappupdate.addingTimeInterval(60 * 60 * 24)
    }
    
    func downloadServerList() {

        // get xml-list of public server
        let serverparser = ServerParser()
        
        let parser = XMLParser(contentsOf: URL(string: AppInfo.getServersURL())!)!
        parser.delegate = serverparser
        parser.parse()

        for s in serverparser.servers {
            servers.append(s)
        }
        tableView.reloadData()
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    func saveServerList() {
        
        saveLocalServers(servers.filter({$0.publicserver == false}))
    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return servers.count
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        
        let cellIdentifier = "ServerTableCell"
        
        let cell = tableView.dequeueReusableCell(withIdentifier: cellIdentifier, for: indexPath) as! ServerTableCell
        
        let server = servers[indexPath.row]
        cell.connectBtn.tag = indexPath.row
        cell.nameLabel.text = server.name
        cell.ipaddrLabel.text = server.ipaddr
        if server.publicserver {
            cell.iconImageView.image = UIImage(named: "teamtalk_green.png")
            cell.iconImageView.accessibilityLabel = NSLocalizedString("Public server", comment: "serverlist")
        }
        else {
            cell.iconImageView.image = UIImage(named: "teamtalk_yellow.png")
            cell.iconImageView.accessibilityLabel = NSLocalizedString("Private server", comment: "serverlist")
        }
        
        if #available(iOS 8.0, *) {
            let action_connect = MyCustomAction(name: NSLocalizedString("Connect to server", comment: "serverlist"), target: self, selector: #selector(ServerListViewController.connectServer(_:)), tag: indexPath.row)
            let action_delete = MyCustomAction(name: NSLocalizedString("Delete server from list", comment: "serverlist"), target: self, selector: #selector(ServerListViewController.deleteServer(_:)), tag: indexPath.row)
            cell.accessibilityCustomActions = [action_connect, action_delete]
        } else {
            // Fallback on earlier versions
        }
        
        return cell
    }

    @available(iOS 8.0, *)
    func connectServer(_ action: UIAccessibilityCustomAction) -> Bool {
        
        if let ac = action as? MyCustomAction {
            currentServer = servers[ac.tag]
            performSegue(withIdentifier: "Show ChannelList", sender: self)
        }
        return true
    }
    
    @available(iOS 8.0, *)
    func deleteServer(_ action: UIAccessibilityCustomAction) -> Bool {
        
        if let ac = action as? MyCustomAction {
            servers.remove(at: ac.tag)
            saveServerList()
            tableView.reloadData()
        }
        return true
    }

    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "Show Server" {
            let index = self.tableView.indexPathForSelectedRow
            currentServer = servers[index!.row]
            let serverDetail = segue.destination as! ServerDetailViewController
            serverDetail.server = currentServer
        }
        else if segue.identifier == "New Server" {
            
        }
        else if segue.identifier == "Show ChannelList" {
            let vc = segue.destination as! MainTabBarController
            vc.setTeamTalkServer(currentServer)
        }
    }
    
    @IBAction func deleteServerDetail(_ segue:UIStoryboardSegue) {
        let vc = segue.source as! ServerDetailViewController
        
        vc.saveServerDetail()
        let name = vc.namefield?.text
        
        servers = servers.filter({$0.name != name})
        
        saveServerList()
        
        tableView.reloadData()
    }
    
    @IBAction func saveServerDetail(_ segue:UIStoryboardSegue) {
        let vc = segue.source as! ServerDetailViewController
        
        vc.saveServerDetail()
        let name = vc.server.name
        
        if let found = servers.map({$0.name}).index(of: name) {
            servers[found] = vc.server
        }
        else {
            servers.append(vc.server)
        }
        
        self.currentServer = vc.server
        
        saveServerList()
        
        tableView.reloadData()
    }
    
    @IBAction func connectToServer(_ sender: UIButton) {
        currentServer = servers[sender.tag]
    }

    func openUrl(_ url: URL) {
        
        if url.isFileURL {
            // get server from either .tt file or tt-URL
            let serverparser = ServerParser()
            
            let parser = XMLParser(contentsOf: url)!
            parser.delegate = serverparser
            parser.parse()
            
            for s in serverparser.servers {
                currentServer = s
            }
        }
        else {
            do {
                // assume TT url
                let url_str = url.absoluteString
                let ns_str = url_str as NSString
                let url_range = NSMakeRange(0, url_str.characters.count)

                // ip-addr
                let host = AppInfo.TTLINK_PREFIX + "([^\\??!/]*)/?\\??"
                let host_regex = try NSRegularExpression(pattern: host, options: .caseInsensitive)
                let host_matches = host_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = host_matches.first {
                    currentServer.ipaddr = ns_str.substring(with: m.rangeAt(1))
                }
                
                //tcp port
                let tcpport = "[&|\\?]tcpport=(\\d+)"
                let tcpport_regex = try NSRegularExpression(pattern: tcpport, options: .caseInsensitive)
                let tcpport_matches = tcpport_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = tcpport_matches.first {
                    let s = ns_str.substring(with: m.rangeAt(1))
                    currentServer.tcpport = Int(s)!
                }
                
                // udp port
                let udpport = "[&|\\?]udpport=(\\d+)"
                let udpport_regex = try NSRegularExpression(pattern: udpport, options: .caseInsensitive)
                let udpport_matches = udpport_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = udpport_matches.first {
                    let s = ns_str.substring(with: m.rangeAt(1))
                    currentServer.udpport = Int(s)!
                }

                // username
                let username = "[&|\\?]username=([^&]*)"
                let username_regex = try NSRegularExpression(pattern: username, options: .caseInsensitive)
                let username_matches = username_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = username_matches.first {
                    currentServer.username = ns_str.substring(with: m.rangeAt(1))
                }
                
                // password
                let password = "[&|\\?]password=([^&]*)"
                let password_regex = try NSRegularExpression(pattern: password, options: .caseInsensitive)
                let password_matches = password_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = password_matches.first {
                    currentServer.password = ns_str.substring(with: m.rangeAt(1))
                }
                
                // channel
                let channel = "[&|\\?]channel=([^&]*)"
                let channel_regex = try NSRegularExpression(pattern: channel, options: .caseInsensitive)
                let channel_matches = channel_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = channel_matches.first {
                    currentServer.channel = ns_str.substring(with: m.rangeAt(1))
                }
                
                // channel password
                let chpasswd = "[&|\\?]chanpasswd=([^&]*)"
                let chpasswd_regex = try NSRegularExpression(pattern: chpasswd, options: .caseInsensitive)
                let chpasswd_matches = chpasswd_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = chpasswd_matches.first {
                    currentServer.chanpasswd = ns_str.substring(with: m.rangeAt(1))
                }
            }
            catch {
                
            }
        }
        
        if !currentServer.ipaddr.isEmpty {
            performSegue(withIdentifier: "Show ChannelList", sender: self)
        }
    }
}

class AppUpdateParser : NSObject, XMLParserDelegate {

    var update = ""
    var updatefound = false
    
    func parser(_ parser: XMLParser, didStartElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?,
        attributes attributeDict: [String : String]) {
            
            if elementName == "name" {
                updatefound = true
            }
    }

    func parser(_ parser: XMLParser, foundCharacters string: String) {
        update = string
    }

    func parser(_ parser: XMLParser, didEndElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?) {
            
    }

}

class ServerParser : NSObject, XMLParserDelegate {
    
    var currentServer = Server()
    var elementStack = [String]()
    var servers = [Server]()
    
    func parser(_ parser: XMLParser, didStartElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?,
        attributes attributeDict: [String : String]) {
            
            elementStack.append(elementName)
            if elementName == "host" {
                currentServer = Server()
            }
    }
    
    func parser(_ parser: XMLParser, foundCharacters string: String) {
        
        switch elementStack.last! {
        case "teamtalk" : break
        case "host" : break
            
        case "name" :
            currentServer.name = string
        case "address" :
            currentServer.ipaddr = string
        case "tcpport" :
            let v : String = string
            currentServer.tcpport = Int(v)!
        case "udpport" :
            let v : String = string
            currentServer.udpport = Int(v)!
        case "encrypted" :
            currentServer.encrypted = string == "true"
            
        case "auth" : break
        case "join" : break
            
        case "username" :
            currentServer.username = string
        case "password" :
            if elementStack.index(of: "auth") != nil {
                currentServer.password = string
            }
            else if elementStack.index(of: "join") != nil {
                currentServer.chanpasswd = string
            }
        case "channel" :
            currentServer.channel = string
        default :
            print("Unknown tag " + self.elementStack.last!)
        }
    }
    
    func parser(_ parser: XMLParser, didEndElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?) {
            
            self.elementStack.removeLast()
            if elementName == "host" {
                currentServer.publicserver = true
                servers.append(currentServer)
            }
    }
    
}
