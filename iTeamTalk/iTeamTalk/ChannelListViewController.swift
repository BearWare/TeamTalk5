//
//  ChannelListViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 12-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class ChannelListViewController : UITableViewController {

    var timer = NSTimer()
    var ttInst = UnsafeMutablePointer<Void>()
    var channels = [INT32 : Channel]()
    var curchannel = Channel()
    var server = Server()
    var currentCmdId : INT32 = 0
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Our one and only TT client instance
        ttInst = TT_InitTeamTalkPoll()

        timer = NSTimer.scheduledTimerWithTimeInterval(0.1, target: self, selector: "timerEvent", userInfo: nil, repeats: true)
        
        
        if TT_Connect(ttInst, server.ipaddr, INT32(server.tcpport), INT32(server.udpport), 0, 0, 0) == 0 {
            println("Failed to connect")
        }
        else {
//            let topItem = self.navigationController?.navigationBar.topItem
//            topItem?.title = "Connecting"
//            self.navigationController?.navigationBar.pushNavigationItem(topItem!, animated: false)
        }
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    func connectToServer(server: Server) {
        self.server = server
    }
    
    enum Command {
        case LoginCmd, JoinCmd
    }
    
    var activeCommands = [INT32: Command]()
    
    func timerEvent() {
        
        var m = TTMessage()
        var n : INT32 = 0
        while TT_GetMessage(ttInst, &m, &n) != 0 {
            
            switch(m.nClientEvent.value) {
            case CLIENTEVENT_CON_SUCCESS.value :
                println("We're connected")
                let cmdid = TT_DoLogin(ttInst, "iOS client", server.username, server.password)
                if cmdid > 0 {
                    activeCommands[cmdid] = .LoginCmd
                }
                
            case CLIENTEVENT_CON_FAILED.value :
                println("Connect failed")
                
            case CLIENTEVENT_CON_LOST.value :
                println("connection lost")
                
            case CLIENTEVENT_CMD_PROCESSING.value :
                if getBoolean(&m) {
                    // command active
                    self.currentCmdId = m.nSource
                }
                else {
                    // command complete
                    self.currentCmdId = 0
                    
                    commandComplete(m.nSource)
                }
            case CLIENTEVENT_CMD_MYSELF_LOGGEDIN.value :
                let u = getUserAccount(&m).memory
                println("User account, type: \(u.uUserType)")
                
            case CLIENTEVENT_CMD_SERVER_UPDATE.value :
                var srv = getServerProperties(&m).memory
                let name = String.fromCString(&srv.szServerName.0)!
                println("Server update: " + name)
                
            case CLIENTEVENT_CMD_CHANNEL_NEW.value :
                var channel = getChannel(&m).memory
                
                //show sub channels of root as default
                if channel.nParentID == 0 {
                    curchannel = channel
                }
                
                channels[channel.nChannelID] = channel
            default :
                println("Unhandled message \(m.nClientEvent.value)")
            }
        }
    }
    
    override func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 1
    }
    
    override func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        let subchans = channels.values.filter({$0.nParentID == self.curchannel.nChannelID})
        println("There's \(subchans.array.count) sub channels")
        return subchans.array.count
    }
    
    override func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {

        let subchans = channels.values.filter({$0.nParentID == self.curchannel.nChannelID})
        
        let cellIdentifier = "ChannelTableCell"
        let cell = tableView.dequeueReusableCellWithIdentifier(cellIdentifier, forIndexPath: indexPath) as! ChannelTableCell
        
        var channel = subchans.array[indexPath.item]
        let name = String.fromCString(&channel.szName.0)
        cell.channame.text = name
        
        return cell
    }

    func commandComplete(cmdid : INT32) {

        if activeCommands[cmdid] == .LoginCmd {
            self.tableView.reloadData()
        }

        activeCommands.removeValueForKey(cmdid)
    }
}