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
        
//        let version = String.fromCString(TT_GetVersion())!
//        appnameLabel.text = AppInfo.APPTITLE + " " + version
        
        // get xml-list of public server
        let parser = NSXMLParser(contentsOfURL: NSURL(string: AppInfo.URL_FREESERVER)!)!
        parser.delegate = self
        parser.parse()
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
        
        let server = servers[indexPath.item]
        cell.connectBtn.tag = indexPath.item
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
            currentServer = servers[index!.item]
            let serverDetail = segue.destinationViewController as! ServerDetailViewController
            serverDetail.server = currentServer
        }
        else if segue.identifier == "New Server" {
            
        }
        else if segue.identifier == "Show ChannelList" {
            let vc = segue.destinationViewController as! MainTabBarController
            vc.connectToServer(currentServer)
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
        
        self.tableView.reloadData()
        
        saveServerList()
    }
    
    @IBAction func connectToServer(sender: UIButton) {
        currentServer = servers[sender.tag]
    }
    
    var servers = [Server]()
    
    var currentServer = Server()
    var elementStack = [String]()
    
    func parser(parser: NSXMLParser, didStartElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?,
        attributes attributeDict: [String : String]) {
            
            self.elementStack.append(elementName)
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

