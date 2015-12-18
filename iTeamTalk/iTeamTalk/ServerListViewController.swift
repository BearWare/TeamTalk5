//
//  ServerListViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 3-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

// Properties of a TeamTalk server to connect to
class Server : NSObject {
    var name = ""
    var ipaddr = ""
    var tcpport = 10333
    var udpport = 10333
    var username = ""
    var password = ""
    var channel = ""
    var chanpasswd = ""
    var publicserver = false
    var encrypted = false
    
    override init() {
        
    }
    
    init(coder dec: NSCoder!) {
        name = dec.decodeObjectForKey("name") as! String
        ipaddr = dec.decodeObjectForKey("ipaddr") as! String
        tcpport = dec.decodeIntegerForKey("tcpport")
        udpport = dec.decodeIntegerForKey("udpport")
        username = dec.decodeObjectForKey("username") as! String
        password = dec.decodeObjectForKey("password") as! String
        channel = dec.decodeObjectForKey("channel") as! String
        chanpasswd = dec.decodeObjectForKey("chanpasswd") as! String
    }
    
    func encodeWithCoder(enc: NSCoder!) {
        enc.encodeObject(name, forKey: "name")
        enc.encodeObject(ipaddr, forKey: "ipaddr")
        enc.encodeInteger(tcpport, forKey: "tcpport")
        enc.encodeInteger(udpport, forKey: "udpport")
        enc.encodeObject(username, forKey: "username")
        enc.encodeObject(password, forKey: "password")
        enc.encodeObject(channel, forKey: "channel")
        enc.encodeObject(chanpasswd, forKey: "chanpasswd")
    }
}

class ServerListViewController: UITableViewController,
NSXMLParserDelegate {
    
    var currentServer = Server()
    var servers = [Server]()

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        
        let defaults = NSUserDefaults.standardUserDefaults()
        
        if let stored = defaults.arrayForKey("ServerList") {
            for e in stored {
                let data = e as! NSData
                
                let server = NSKeyedUnarchiver.unarchiveObjectWithData(data) as! Server
                
                servers.append(server)
            }
        }
        
        NSTimer.scheduledTimerWithTimeInterval(1.0, target: self, selector: "downloadServerList", userInfo: nil, repeats: false)
        NSTimer.scheduledTimerWithTimeInterval(2.0, target: self, selector: "checkAppUpdate", userInfo: nil, repeats: false)
    }
    
    func checkAppUpdate() {
        
        // check for new version
        let updateparser = AppUpdateParser()
        
        let parser = NSXMLParser(contentsOfURL: NSURL(string: AppInfo.URL_APPUPDATE)!)!
        parser.delegate = updateparser
        parser.parse()
    }
    
    func downloadServerList() {

        // get xml-list of public server
        let serverparser = ServerParser()
        
        let parser = NSXMLParser(contentsOfURL: NSURL(string: AppInfo.URL_FREESERVER)!)!
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
        //Store local servers
        let localservers = servers.filter({$0.publicserver == false})
        let defaults = NSUserDefaults.standardUserDefaults()
        var s_array = [NSData]()
        for s in localservers {
            let data = NSKeyedArchiver.archivedDataWithRootObject(s)
            s_array.append(data)
        }
        defaults.setObject(s_array, forKey: "ServerList")
        defaults.synchronize()
    }
    
    override func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 1
    }
    
    override func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return servers.count
    }
    
    override func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        
        let cellIdentifier = "ServerTableCell"
        
        let cell = tableView.dequeueReusableCellWithIdentifier(cellIdentifier, forIndexPath: indexPath) as! ServerTableCell
        
        let server = servers[indexPath.row]
        cell.connectBtn.tag = indexPath.row
        cell.nameLabel.text = server.name
        cell.ipaddrLabel.text = server.ipaddr
        if server.publicserver {
            cell.iconImageView.image = UIImage(named: "teamtalk_green.png")
        }
        else {
            cell.iconImageView.image = UIImage(named: "teamtalk_yellow.png")
        }
        
        return cell
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        if segue.identifier == "Show Server" {
            let index = self.tableView.indexPathForSelectedRow
            currentServer = servers[index!.row]
            let serverDetail = segue.destinationViewController as! ServerDetailViewController
            serverDetail.server = currentServer
        }
        else if segue.identifier == "New Server" {
            
        }
        else if segue.identifier == "Show ChannelList" {
            let vc = segue.destinationViewController as! MainTabBarController
            vc.setTeamTalkServer(currentServer)
        }
    }
    
    @IBAction func deleteServerDetail(segue:UIStoryboardSegue) {
        let vc = segue.sourceViewController as! ServerDetailViewController
        
        vc.saveServerDetail()
        let name = vc.namefield?.text
        
        servers = servers.filter({$0.name != name})
        
        saveServerList()
        
        self.tableView.reloadData()
    }
    
    @IBAction func saveServerDetail(segue:UIStoryboardSegue) {
        let vc = segue.sourceViewController as! ServerDetailViewController
        
        vc.saveServerDetail()
        let name = vc.server.name
        
        if let found = servers.map({$0.name}).indexOf(name) {
            servers[found] = vc.server
        }
        else {
            servers.append(vc.server)
        }
        
        self.currentServer = vc.server
        
        saveServerList()
        
        self.tableView.reloadData()
    }
    
    @IBAction func connectToServer(sender: UIButton) {
        currentServer = servers[sender.tag]
    }

}

class AppUpdateParser : NSObject, NSXMLParserDelegate {

    var update = ""
    var updatefound = false
    
    func parser(parser: NSXMLParser, didStartElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?,
        attributes attributeDict: [String : String]) {
            
            if elementName == "name" {
                updatefound = true
            }
    }

    func parser(parser: NSXMLParser, foundCharacters string: String) {
        update = string
    }

    func parser(parser: NSXMLParser, didEndElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?) {
            
    }

}

class ServerParser : NSObject, NSXMLParserDelegate {
    
    var currentServer = Server()
    var elementStack = [String]()
    var servers = [Server]()
    
    func parser(parser: NSXMLParser, didStartElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?,
        attributes attributeDict: [String : String]) {
            
            elementStack.append(elementName)
            if elementName == "host" {
                currentServer = Server()
            }
    }
    
    func parser(parser: NSXMLParser, foundCharacters string: String) {
        
        switch elementStack.last! {
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
        case "username" :
            currentServer.username = string
        case "password" :
            if elementStack.indexOf("auth") != nil {
                currentServer.password = string
            }
            else if elementStack.indexOf("join") != nil {
                currentServer.chanpasswd = string
            }
        case "channel" :
            currentServer.channel = string
        case "encrypted" :
            currentServer.encrypted = string == "true"
        default :
            print("Unknown tag " + self.elementStack.last!)
        }
    }
    
    func parser(parser: NSXMLParser, didEndElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?) {
            
            self.elementStack.removeLast()
            if elementName == "host" {
                currentServer.publicserver = true
                servers.append(currentServer)
            }
    }
    
}
