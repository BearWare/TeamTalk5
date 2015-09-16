//
//  ChannelListViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 12-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class ChannelListViewController : UITableViewController {

    // timer for polling TeamTalk client events
    var timer = NSTimer()
    // our one and only TeamTalk client instance
    var ttInst = UnsafeMutablePointer<Void>()
    // ip-addr and login information for current server
    var server = Server()
    // all channels on server
    var channels = [INT32 : Channel]()
    // the channel being displayed (not nescessarily the same channel as we're in)
    var curchannel = Channel()
    // all users on server
    var users = [INT32 : User]()
    // the command ID which is currently processing
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
                
                if currentCmdId == 0 {
                    self.tableView.reloadData()
                }

            case CLIENTEVENT_CMD_CHANNEL_UPDATE.value :
                var channel = getChannel(&m).memory
                channels[channel.nChannelID] = channel

                if currentCmdId == 0 {
                    self.tableView.reloadData()
                }

            case CLIENTEVENT_CMD_CHANNEL_REMOVE.value :
                let channel = getChannel(&m).memory
                channels.removeValueForKey(channel.nChannelID)

                if currentCmdId == 0 {
                    self.tableView.reloadData()
                }

            case CLIENTEVENT_CMD_USER_JOINED.value :
                var user = getUser(&m).memory
                users[user.nUserID] = user

                if currentCmdId == 0 {
                    self.tableView.reloadData()
                }

            case CLIENTEVENT_CMD_USER_UPDATE.value :
                var user = getUser(&m).memory
                users[user.nUserID] = user

                if currentCmdId == 0 {
                    self.tableView.reloadData()
                }

            case CLIENTEVENT_CMD_USER_LEFT.value :
                let user = getUser(&m).memory
                users.removeValueForKey(user.nUserID)
                
                if currentCmdId == 0 {
                    self.tableView.reloadData()
                }
                
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
        let chanusers = users.values.filter({$0.nChannelID == self.curchannel.nChannelID})

        return subchans.array.count + chanusers.array.count
    }
    
    override func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {

        let subchans = channels.values.filter({$0.nParentID == self.curchannel.nChannelID})
        let chanusers = users.values.filter({$0.nChannelID == self.curchannel.nChannelID})

        // display users first
        if indexPath.item < chanusers.array.count {

            let cellIdentifier = "UserTableCell"
            let cell = tableView.dequeueReusableCellWithIdentifier(cellIdentifier, forIndexPath: indexPath) as! UserTableCell
            
            var user = chanusers.array[indexPath.item]
            let nickname = String.fromCString(&user.szNickname.0)
            let statusmsg = String.fromCString(&user.szStatusMsg.0)
            cell.nicknameLabel.text = nickname
            cell.statusmsgLabel.text = statusmsg
            
            if (user.uUserState & USERSTATE_VOICE.value) != 0 {
                cell.userImage.image = UIImage(named: "man_green.png")
            }
            else {
                cell.userImage.image = UIImage(named: "man_blue.png")
            }
            
            return cell
        }
        else {

            let cellIdentifier = "ChannelTableCell"
            let cell = tableView.dequeueReusableCellWithIdentifier(cellIdentifier, forIndexPath: indexPath) as! ChannelTableCell
            
            var channel = subchans.array[indexPath.item - chanusers.array.count]
            let name = String.fromCString(&channel.szName.0)
            let topic = String.fromCString(&channel.szTopic.0)
            cell.channame.text = name
            cell.chantopicLabel.text = topic
            if channel.bPassword != 0 {
                cell.chanimage.image = UIImage(named: "channel_pink.png")
            }
            else {
                cell.chanimage.image = UIImage(named: "channel_orange.png")
            }
            
            return cell
        }
    }

    func commandComplete(cmdid : INT32) {

        if activeCommands[cmdid] == .LoginCmd {
            self.tableView.reloadData()
        }

        activeCommands.removeValueForKey(cmdid)
    }
}
