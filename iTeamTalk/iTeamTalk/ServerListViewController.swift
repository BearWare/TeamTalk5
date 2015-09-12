//
//  ServerListViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 3-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit


// Properties of a TeamTalk server to connect to
struct Server {
    var name = ""
    var ipaddr = ""
    var tcpport = 10333
    var udpport = 10333
    var username = ""
    var password = ""
    var channel = ""
    var chanpasswd = ""
}

class ServerListViewController: UITableViewController,
NSXMLParserDelegate {
    
    var timer = NSTimer()
    var ttInst = UnsafeMutablePointer<Void>()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        
        let version = String.fromCString(TT_GetVersion())!
//        appnameLabel.text = AppInfo.APPTITLE + " " + version
        
        // Our one and only TT client instance
        ttInst = TT_InitTeamTalkPoll()
        
        // get xml-list of public server
        var parser = NSXMLParser(contentsOfURL: NSURL(string: AppInfo.URL_FREESERVER))!
        parser.delegate = self
        parser.parse()
        
        timer = NSTimer.scheduledTimerWithTimeInterval(0.1, target: self, selector: "timerEvent", userInfo: nil, repeats: true)
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
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
        
        return cell
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        if segue.identifier == "Show ServerDetail" {
            let index = self.tableView.indexPathForSelectedRow()
            currentServer = servers[index!.item]
            let serverDetail = segue.destinationViewController as! ServerDetailViewController
            serverDetail.server = currentServer
        }
    }
    
    @IBAction func connectToServer(sender: UIButton) {
        TT_Disconnect(ttInst)
        
        currentServer = servers[sender.tag]
        
        if TT_Connect(ttInst, currentServer.ipaddr, INT32(currentServer.tcpport), INT32(currentServer.udpport), 0, 0, 0) != 0 {
            println("Failed to connect")
        }
    }
    
    var servers = [Server]()
    
    var currentServer = Server()
    var elementStack = [String]()
    
    func parser(parser: NSXMLParser, didStartElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?,
        attributes attributeDict: [NSObject : AnyObject]) {
            
            self.elementStack.append(elementName)
            if elementName == "host" {
                currentServer = Server()
            }
    }
    
    func parser(parser: NSXMLParser, foundCharacters string: String?) {
        
        if string == nil { return }
        
        switch elementStack.last! {
        case "name" :
            currentServer.name = string!
        case "address" :
            currentServer.ipaddr = string!
        case "tcpport" :
            let v : String = string!
            currentServer.tcpport = v.toInt()!
        case "udpport" :
            let v : String = string!
            currentServer.udpport = v.toInt()!
        case "username" :
            currentServer.username = string!
        case "password" :
            if find(elementStack, "auth") != nil {
                currentServer.password = string!
            }
            else if find(elementStack, "join") != nil {
                currentServer.chanpasswd = string!
            }
        default :
            println("Unknown tag " + self.elementStack.last!)
        }
    }
    
    func parser(parser: NSXMLParser, didEndElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?) {
            
            self.elementStack.removeLast()
            if elementName == "host" {
                servers.append(currentServer)
            }
    }
    
    func timerEvent() {
        
        var m = TTMessage()
        var n : INT32 = 0
        if TT_GetMessage(ttInst, &m, &n) != 0 {
            
            switch(m.nClientEvent.value) {
            case CLIENTEVENT_CON_SUCCESS.value :
                println("We're connected")
                TT_DoLogin(ttInst, "iOS client", currentServer.username, currentServer.password)
                
            case CLIENTEVENT_CON_FAILED.value :
                println("Connect failed")
                
            case CLIENTEVENT_CON_LOST.value :
                println("connection lost")
                
            case CLIENTEVENT_CMD_MYSELF_LOGGEDIN.value :
                let u = getUserAccount(&m).memory
                println("User account, type: \(u.uUserType)")
                
            case CLIENTEVENT_CMD_SERVER_UPDATE.value :
                var srv = getServerProperties(&m).memory
                let name = String.fromCString(&srv.szServerName.0)!
                println("Server update: " + name)
            default :
                println("Unhandled message \(m.nClientEvent.value)")
            }
        }
    }
}

