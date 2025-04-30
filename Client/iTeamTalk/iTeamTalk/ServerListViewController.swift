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

enum ServerType {
    case LOCAL,
    OFFICIAL,
    PUBLIC,
    UNOFFICIAL
}

// Properties of a TeamTalk server to connect to
class Server : NSObject {
    var name = ""
    var ipaddr = ""
    var tcpport = AppInfo.DEFAULT_TCPPORT
    var udpport = AppInfo.DEFAULT_UDPPORT
    var nickname = ""
    var username = ""
    var password = ""
    var channel = ""
    var chanpasswd = ""
    var servertype : ServerType = .LOCAL
    var encrypted = false
    var cacertdata = ""
    var certdata = ""
    var certprivkeydata = ""
    var certverifypeer = false
    
    // stats from public server list
    var stats_usercount = 0
    var stats_country = ""
    var stats_servername = ""
    var stats_motd = ""
    
    override init() {
        
    }
    
    @objc init(coder dec: NSCoder!) {
        name = dec.decodeObject(forKey: "name") as! String
        ipaddr = dec.decodeObject(forKey: "ipaddr") as! String
        tcpport = dec.decodeInteger(forKey: "tcpport")
        udpport = dec.decodeInteger(forKey: "udpport")
        username = dec.decodeObject(forKey: "username") as! String
        password = dec.decodeObject(forKey: "password") as! String
        nickname = dec.decodeObject(forKey: "nickname") as? String ?? ""
        channel = dec.decodeObject(forKey: "channel") as! String
        chanpasswd = dec.decodeObject(forKey: "chanpasswd") as! String
        encrypted = dec.decodeBool(forKey: "encrypted")
        cacertdata = dec.decodeObject(forKey: "cacertdata") as? String ?? ""
        certdata = dec.decodeObject(forKey: "certdata") as? String ?? ""
        certprivkeydata = dec.decodeObject(forKey: "certprivkeydata") as? String ?? ""
        certverifypeer = dec.decodeBool(forKey: "certverifypeer")
    }
    
    @objc func encodeWithCoder(_ enc: NSCoder!) {
        enc.encode(name, forKey: "name")
        enc.encode(ipaddr, forKey: "ipaddr")
        enc.encode(tcpport, forKey: "tcpport")
        enc.encode(udpport, forKey: "udpport")
        enc.encode(username, forKey: "username")
        enc.encode(password, forKey: "password")
        enc.encode(nickname, forKey: "nickname")
        enc.encode(channel, forKey: "channel")
        enc.encode(chanpasswd, forKey: "chanpasswd")
        enc.encode(encrypted, forKey: "encrypted")
        enc.encode(cacertdata, forKey: "cacertdata")
        enc.encode(certdata, forKey: "certdata")
        enc.encode(certprivkeydata, forKey: "certprivkeydata")
        enc.encode(certverifypeer, forKey: "certverifypeer")
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
            addbtn.accessibilityLabel = NSLocalizedString("Add new server entry", comment: "serverlist")
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
        var downloadServers = defaults.object(forKey: PREF_DISPLAY_OFFICIALSERVERS) == nil || defaults.bool(forKey: PREF_DISPLAY_OFFICIALSERVERS)
        downloadServers = downloadServers || (defaults.object(forKey: PREF_DISPLAY_PUBLICSERVERS) == nil || defaults.bool(forKey: PREF_DISPLAY_PUBLICSERVERS))
        downloadServers = downloadServers || (defaults.object(forKey: PREF_DISPLAY_UNOFFICIALSERVERS) != nil && defaults.bool(forKey: PREF_DISPLAY_UNOFFICIALSERVERS))
        if downloadServers {
            Timer.scheduledTimer(timeInterval: 1.0, target: self, selector: #selector(ServerListViewController.downloadServerList), userInfo: nil, repeats: false)
        }

        tableView.reloadData()
        
        if (nextappupdate as NSDate).earlierDate(Date()) == nextappupdate {
            Timer.scheduledTimer(timeInterval: 2.0, target: self, selector: #selector(ServerListViewController.checkAppUpdate), userInfo: nil, repeats: false)
        }
        
    }
    
    @objc func checkAppUpdate() {
        
        // check for new version
        if let url = URL(string: AppInfo.getUpdateURL()) {
            let task = URLSession.shared.dataTask(with: url) { data, response, error in
                if error != nil {
                    print ("Failed to check app update \(url.absoluteString)")
                    return
                }
                guard let httpResponse = response as? HTTPURLResponse,
                      (200...299).contains(httpResponse.statusCode) else {
                    print ("Failed to check app update \(url.absoluteString)")
                    return
                }
                if let data = data {
                    let parser = XMLParser(data: data)
                    let updateparser = AppUpdateParser()
                    parser.delegate = updateparser
                    parser.parse()
                    if updateparser.registerUrl.isEmpty == false {
                        AppInfo.BEARWARE_REGISTRATION_WEBSITE = updateparser.registerUrl
                    }
                }
            }
            task.resume()
        }
        nextappupdate = nextappupdate.addingTimeInterval(60 * 60 * 24)
    }
    
    @objc func downloadServerList() {

        // get xml-list of public server
       
        let defaults = UserDefaults.standard
        let official = defaults.object(forKey: PREF_DISPLAY_OFFICIALSERVERS) == nil || defaults.bool(forKey: PREF_DISPLAY_OFFICIALSERVERS)
        let unofficial = defaults.object(forKey: PREF_DISPLAY_UNOFFICIALSERVERS) != nil && defaults.bool(forKey: PREF_DISPLAY_UNOFFICIALSERVERS)

        if let serversurl = URL(string: AppInfo.getServersURL(officialservers: official, unofficialservers: unofficial)) {
        
            let task = URLSession.shared.dataTask(with: serversurl) { data, response, error in
                if error != nil {
                    print ("Failed to download server list \(serversurl.absoluteString)")
                    return
                }
                guard let httpResponse = response as? HTTPURLResponse,
                      (200...299).contains(httpResponse.statusCode) else {
                    print ("Failed to download server list \(serversurl.absoluteString)")
                    return
                }
                if let data = data {
                    DispatchQueue.main.async {
                        self.insertServerList(data: data)
                    }
                }
            }
            task.resume()
        }
    }
    
    func insertServerList(data: Data) {
        let parser = XMLParser(data: data)
        let serverparser = ServerParser()
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
        
        saveLocalServers(servers.filter({$0.servertype == .LOCAL}))
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
        cell.detailLabel.text = "\(server.ipaddr):\(server.tcpport)"
        if server.servertype != .LOCAL {
            let detail = cell.detailLabel.text! + ", " +
            String(format: NSLocalizedString("Users: %d, Country: %@", comment: "serverlist"), server.stats_usercount, server.stats_country)
            cell.detailLabel.text = detail
        }
        switch server.servertype {
        case .LOCAL :
            cell.iconImageView.image = UIImage(named: "teamtalk_yellow.png")
            cell.iconImageView.accessibilityLabel = NSLocalizedString("Local server", comment: "serverlist")
        case .OFFICIAL :
            cell.iconImageView.image = UIImage(named: "teamtalk_blue.png")
            cell.iconImageView.accessibilityLabel = NSLocalizedString("Official server", comment: "serverlist")
        case .PUBLIC :
            cell.iconImageView.image = UIImage(named: "teamtalk_green.png")
            cell.iconImageView.accessibilityLabel = NSLocalizedString("Public server", comment: "serverlist")
        case .UNOFFICIAL :
            cell.iconImageView.image = UIImage(named: "teamtalk_orange.png")
            cell.iconImageView.accessibilityLabel = NSLocalizedString("Unofficial server", comment: "serverlist")
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

    @objc @available(iOS 8.0, *)
    func connectServer(_ action: UIAccessibilityCustomAction) -> Bool {
        
        if let ac = action as? MyCustomAction {
            currentServer = servers[ac.tag]
            performSegue(withIdentifier: "Show ChannelList", sender: self)
        }
        return true
    }
    
    @objc @available(iOS 8.0, *)
    func deleteServer(_ action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            let alertController = UIAlertController(
                title: NSLocalizedString("deletion_title", comment: "serverlist"),
                message: NSLocalizedString("deletion_message", comment: "serverlist"),
                preferredStyle: .alert
            )

            let cancelAction = UIAlertAction(title: NSLocalizedString("cancel_deletion", comment: "serverlist"), style: .cancel, handler: nil)
            let deleteAction = UIAlertAction(title: NSLocalizedString("confirm_deletion", comment: "serverlist"), style: .destructive) { _ in
                self.servers.remove(at: ac.tag)
                self.saveServerList()
                self.tableView.reloadData()
            }

            alertController.addAction(cancelAction)
            alertController.addAction(deleteAction)

            if let viewController = self.tableView.window?.rootViewController {
                viewController.present(alertController, animated: true, completion: nil)
            }
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
        
        if let found = servers.map({$0.name}).firstIndex(of: name) {
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
            
            if let parser = XMLParser(contentsOf: url) {
                parser.delegate = serverparser
                parser.parse()
            }
            
            for s in serverparser.servers {
                currentServer = s
            }
        }
        else if url.absoluteString.starts(with: AppInfo.TTLINK_PREFIX){
            do {
                // assume TT url
                let url_str = url.absoluteString
                let ns_str = url_str as NSString
                let url_range = NSMakeRange(0, url_str.count)

                // ip-addr
                let host = AppInfo.TTLINK_PREFIX + "([^\\??!/]*)/?\\??"
                let host_regex = try NSRegularExpression(pattern: host, options: .caseInsensitive)
                let host_matches = host_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = host_matches.first {
                    currentServer.ipaddr = ns_str.substring(with: m.range(at: 1))
                }
                
                //tcp port
                let tcpport = "[&|\\?]tcpport=(\\d+)"
                let tcpport_regex = try NSRegularExpression(pattern: tcpport, options: .caseInsensitive)
                let tcpport_matches = tcpport_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = tcpport_matches.first {
                    let s = ns_str.substring(with: m.range(at: 1))
                    currentServer.tcpport = Int(s)!
                }
                
                // udp port
                let udpport = "[&|\\?]udpport=(\\d+)"
                let udpport_regex = try NSRegularExpression(pattern: udpport, options: .caseInsensitive)
                let udpport_matches = udpport_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = udpport_matches.first {
                    let s = ns_str.substring(with: m.range(at: 1))
                    currentServer.udpport = Int(s)!
                }

                // encrypted
                let encrypted = "[&|\\?]encrypted=([^&]*)"
                let encrypted_regex = try NSRegularExpression(pattern: encrypted, options: .caseInsensitive)
                let encrypted_matches = encrypted_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = encrypted_matches.first {
                    let str = ns_str.substring(with: m.range(at: 1))
                    currentServer.encrypted = str == "true" || str == "1"
                }

                // username
                let username = "[&|\\?]username=([^&]*)"
                let username_regex = try NSRegularExpression(pattern: username, options: .caseInsensitive)
                let username_matches = username_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = username_matches.first {
                    let urlusername = ns_str.substring(with: m.range(at: 1))
                    currentServer.username = urlusername.removingPercentEncoding!
                }
                
                // password
                let password = "[&|\\?]password=([^&]*)"
                let password_regex = try NSRegularExpression(pattern: password, options: .caseInsensitive)
                let password_matches = password_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = password_matches.first {
                    let urlpasswd = ns_str.substring(with: m.range(at: 1))
                    currentServer.password = urlpasswd.removingPercentEncoding!
                }

                // nickname
                let nickname = "[&|\\?]nickname=([^&]*)"
                let nickname_regex = try NSRegularExpression(pattern: nickname, options: .caseInsensitive)
                let nickname_matches = nickname_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = nickname_matches.first {
                    let urlnickname = ns_str.substring(with: m.range(at: 1))
                    currentServer.nickname = urlnickname.removingPercentEncoding!
                }

                // channel
                let channel = "[&|\\?]channel=([^&]*)"
                let channel_regex = try NSRegularExpression(pattern: channel, options: .caseInsensitive)
                let channel_matches = channel_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = channel_matches.first {
                    let urlchannel = ns_str.substring(with: m.range(at: 1))
                    currentServer.channel = urlchannel.removingPercentEncoding!
                }
                
                // channel password
                let chpasswd = "[&|\\?]chanpasswd=([^&]*)"
                let chpasswd_regex = try NSRegularExpression(pattern: chpasswd, options: .caseInsensitive)
                let chpasswd_matches = chpasswd_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = chpasswd_matches.first {
                    let urlchpasswd = ns_str.substring(with: m.range(at: 1))
                    currentServer.chanpasswd = urlchpasswd.removingPercentEncoding!
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

    var elementStack = [String]()
    var update = ""
    var registerUrl = ""
    
    func parser(_ parser: XMLParser, didStartElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?,
        attributes attributeDict: [String : String]) {
        
        elementStack.append(elementName)
    }

    func parser(_ parser: XMLParser, foundCharacters string: String) {
        let path = getXMLPath(elementStack: elementStack)
        switch path {
        case "/teamtalk/update/name" :
            update = string
        case "/teamtalk/bearware/register-url" :
            registerUrl = string
        default :
             print("Unknown path " + path)
        }
    }

    func parser(_ parser: XMLParser, didEndElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?) {
        
        self.elementStack.removeLast()
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
        // <host>
        case "id" : break
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
        case "listing" :
            if string == "official" {
                currentServer.servertype = .OFFICIAL
            }
            else if string == "public" {
                currentServer.servertype = .PUBLIC
            }
            else if string == "private" {
                currentServer.servertype = .UNOFFICIAL
            }
        case "auth" : break
        case "join" : break
        // <auth>
        case "username" :
            currentServer.username = string
        // <auth> or <join>
        case "password" :
            if elementStack.firstIndex(of: "auth") != nil {
                currentServer.password = string
            }
            else if elementStack.firstIndex(of: "join") != nil {
                currentServer.chanpasswd = string
            }
        // <auth>
        case "nickname" :
            currentServer.nickname = string
        case "channel" :
            currentServer.channel = string

        // <stats>
        case "stats" : break
        case "user-count" :
            currentServer.stats_usercount = Int(string) ?? 0
        case "country" :
            currentServer.stats_country = string
        case "servername" :
            currentServer.stats_servername += string
        case "motd" :
            currentServer.stats_motd += string
        case "active" :
            break
            
        // <trusted-certificate>
        case "trusted-certificate" : break
        case "certificate-authority-pem" :
            currentServer.cacertdata += string
        case "client-certificate-pem" :
            currentServer.certdata += string
        case "client-private-key-pem" :
            currentServer.certprivkeydata += string
        case "verify-peer" :
            currentServer.certverifypeer = string == "true"
        default :
            print("Unknown tag " + self.elementStack.last!)
        }
    }
    
    func parser(_ parser: XMLParser, didEndElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?) {
            
            self.elementStack.removeLast()
            if elementName == "host" {
                servers.append(currentServer)
            }
    }
    
}
