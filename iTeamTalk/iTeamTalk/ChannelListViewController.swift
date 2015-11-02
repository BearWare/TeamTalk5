//
//  ChannelListViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 12-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class ChannelListViewController : UIViewController, UITableViewDataSource, UITableViewDelegate, TeamTalkEvent  {

    //shared TTInstance between all view controllers
    var ttInst = UnsafeMutablePointer<Void>()
    // all channels on server
    var channels = [INT32 : Channel]()
    // the channel being displayed (not nescessarily the same channel as we're in)
    var curchannel = Channel()
    // all users on server
    var users = [INT32 : User]()
    // the command ID which is currently processing
    var currentCmdId : INT32 = 0
    // properties of connected server
    var srvprop = ServerProperties()
    // local instance's user account
    var myuseraccount = UserAccount()
    // double tab is lock TX
    @IBOutlet var tabGesture: UITapGestureRecognizer!
    // long press is TX
    @IBOutlet var pressGesture: UILongPressGestureRecognizer!
    //list of channels and users
    @IBOutlet weak var tableView: UITableView!
    @IBOutlet weak var txButton: UIButton!

    override func viewDidLoad() {
        super.viewDidLoad()

        addToTTMessages(self)
        
        tableView.dataSource = self
        tableView.delegate = self
    
//        tabGesture.numberOfTapsRequired = 2
//        
//        pressGesture.minimumPressDuration = 0.1
        
        updateTX()
    }
    
    deinit {
        print("Destroyed chan list ctrl")
    }
    
    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)
        
        tableView.reloadData()
    }
    
    override func viewDidDisappear(animated: Bool) {
        super.viewDidDisappear(animated)
        
        if isClosing(self) {
            removeFromTTMessages(self)
        }
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @IBAction func joinChannel(sender: UIButton) {
        
        if let chan = channels[INT32(sender.tag)] {
            let cmdid = TT_DoJoinChannelByID(ttInst, chan.nChannelID, "")
            activeCommands[cmdid] = .JoinCmd
        }
    }
    
    enum Command {
        case LoginCmd, JoinCmd
    }
    
    var activeCommands = [INT32: Command]()
    
    func getDisplayItems() -> ([Channel], [User]) {
        let subchans : [Channel] = channels.values.filter({$0.nParentID == self.curchannel.nChannelID})
        let chanusers : [User] = users.values.filter({$0.nChannelID == self.curchannel.nChannelID})
        
        return (subchans, chanusers)
    }
    
    func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 1
    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        
        let (subchans, chanusers) = getDisplayItems()
        
        if curchannel.nParentID != 0 {
            return subchans.count + chanusers.count + 1 //+1 for 'Back' to parent channel
        }
        return subchans.count + chanusers.count
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {

        let (subchans, chanusers) = getDisplayItems()

        let chan_count = (curchannel.nParentID != 0 ? subchans.count + 1 : subchans.count)
        
        print("row = \(indexPath.row) cur channel = \(curchannel.nChannelID) subs = \(subchans.count) users = \(chanusers.count)")
        
        // display channels first
        if indexPath.row < chan_count {

            let cellIdentifier = "ChannelTableCell"
            let cell = tableView.dequeueReusableCellWithIdentifier(cellIdentifier, forIndexPath: indexPath) as! ChannelTableCell
            
            var channel = Channel()
            var textcolor : UIColor? = nil
            var name : String?, topic : String?
            
            if indexPath.row == 0 && curchannel.nParentID != 0 {
                // display previous channel if not in root channel
                channel = channels[curchannel.nParentID]!
                
                if channel.nParentID == 0 {
                    name = String.fromCString(&srvprop.szServerName.0)!
                }
                else {
                    name = String.fromCString(&channel.szName.0)
                    topic = String.fromCString(&channel.szTopic.0)
                }
                
                textcolor = UIColor.grayColor()
                cell.chanimage.image = UIImage(named: "back_orange.png")
            }
            else if curchannel.nChannelID == 0 {
                // display only the root channel
                channel = subchans[indexPath.row]
                
                name = String.fromCString(&srvprop.szServerName.0)
                topic = String.fromCString(&channel.szTopic.0)
                
                if channel.bPassword != 0 {
                    cell.chanimage.image = UIImage(named: "channel_pink.png")
                }
                else {
                    cell.chanimage.image = UIImage(named: "channel_orange.png")
                }
            }
            else  {
                // display sub channels
                if curchannel.nParentID != 0 {
                    // root channel doesn't display access to parent
                    channel = subchans[indexPath.row - 1]
                }
                else {
                    channel = subchans[indexPath.row]
                }
                
                name = String.fromCString(&channel.szName.0)
                topic = String.fromCString(&channel.szTopic.0)
                
                if channel.bPassword != 0 {
                    cell.chanimage.image = UIImage(named: "channel_pink.png")
                }
                else {
                    cell.chanimage.image = UIImage(named: "channel_orange.png")
                }

            }

            cell.channame.textColor = textcolor
            cell.chantopicLabel.textColor = textcolor

            cell.channame.text = name
            cell.chantopicLabel.text = topic
            
            cell.editBtn.tag = Int(channel.nChannelID)
            cell.joinBtn.tag = Int(channel.nChannelID)
            cell.tag = Int(channel.nChannelID)
            
            return cell
        }
        else {
            let cellIdentifier = "UserTableCell"
            let cell = tableView.dequeueReusableCellWithIdentifier(cellIdentifier, forIndexPath: indexPath) as! UserTableCell
            var user = chanusers[indexPath.row - chan_count]
            let nickname = String.fromCString(&user.szNickname.0)
            let statusmsg = String.fromCString(&user.szStatusMsg.0)
            cell.nicknameLabel.text = nickname
            cell.statusmsgLabel.text = statusmsg
            
            if (user.uUserState & USERSTATE_VOICE.rawValue) != 0 {
                cell.userImage.image = UIImage(named: "man_green.png")
            }
            else {
                cell.userImage.image = UIImage(named: "man_blue.png")
            }
            
            cell.messageBtn.tag = Int(user.nUserID)
            cell.tag = Int(user.nUserID)
            
            return cell
        }
    }
    
    func tableView(tableView: UITableView, didSelectRowAtIndexPath indexPath: NSIndexPath) {
        let cell = self.tableView.cellForRowAtIndexPath(indexPath)
        if cell is ChannelTableCell {
            curchannel = channels[INT32(cell!.tag)]!
            tableView.reloadData()
            updateTitle()
        }
    }
    
    func updateTitle() {
        var title = ""
        if curchannel.nParentID == 0 {
            
            title = String.fromCString(&srvprop.szServerName.0)!
        }
        else {
            title = String.fromCString(&curchannel.szName.0)!
        }
        
        self.tabBarController?.navigationItem.title = title
    }

    func commandComplete(cmdid : INT32) {

        let cmd = activeCommands[cmdid]
        
        if cmd == nil {
            return
        }
        
        switch cmd! {
            
        case .LoginCmd :
            self.tableView.reloadData()
            
            let flags = TT_GetFlags(ttInst)
            
            if (flags & CLIENT_AUTHORIZED.rawValue) != 0 && NSUserDefaults.standardUserDefaults().boolForKey("joinroot_preference") {
                
                let cmdid = TT_DoJoinChannelByID(ttInst, TT_GetRootChannelID(ttInst), "")
                if cmdid > 0 {
                    activeCommands[cmdid] = .JoinCmd
                }
            }
            
        case .JoinCmd :
            self.tableView.reloadData()
        }

        activeCommands.removeValueForKey(cmdid)
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {

        if segue.identifier == "Show User" {
            let index = self.tableView.indexPathForSelectedRow
            let cell = self.tableView.cellForRowAtIndexPath(index!)

            let userDetail = segue.destinationViewController as! UserDetailViewController
            userDetail.userid = INT32(cell!.tag)
            userDetail.ttInst = self.ttInst
        }
        else if segue.identifier == "New Channel" {
            let chanDetail = segue.destinationViewController as! ChannelDetailViewController
            chanDetail.ttInst = ttInst
            chanDetail.channel.nParentID = curchannel.nChannelID
            
            if chanDetail.channel.nParentID == 0 {
                let subchans = channels.values.filter({$0.nParentID == 0})
                if let root = subchans.first {
                    chanDetail.channel.nParentID = root.nChannelID
                }
            }
        }
        else if segue.identifier == "Edit Channel" {
            
            let btn = sender as! UIButton

            let channel = channels[INT32(btn.tag)]
            
            let chanDetail = segue.destinationViewController as! ChannelDetailViewController
            chanDetail.ttInst = ttInst
            chanDetail.channel = channel!
        }
        else if segue.identifier == "New TextMessage" {

            let btn = sender as! UIButton
            
            let txtmsgView = segue.destinationViewController as! TextMessageViewController
            txtmsgView.ttInst = self.ttInst
            txtmsgView.userid = btn.tag
        }
    }

    func updateTX() {
        let flags = TT_GetFlags(ttInst)
        
        switch flags & CLIENT_TX_VOICE.rawValue {
        case CLIENT_TX_VOICE.rawValue :
            txButton.backgroundColor = UIColor.redColor()
        default :
            txButton.backgroundColor = UIColor.greenColor()
        }
    }
    
    @IBAction func longPressGesture(sender: UILongPressGestureRecognizer) {
        
        let p = sender.locationInView(txButton)
        
        switch sender.state {
        case .Began :
            if txButton.pointInside(p, withEvent: nil) {
                TT_EnableVoiceTransmission(ttInst, 1)
                updateTX()
            }
        case .Ended :
            TT_EnableVoiceTransmission(ttInst, 0)
            updateTX()
        default :
            break
        }
        
        TT_DBG_SetSoundInputTone(ttInst, STREAMTYPE_NONE.rawValue, 0)
    }
    
    @IBAction func tabGesture(sender: UITapGestureRecognizer) {
        
        let flags = TT_GetFlags(ttInst)
        
        switch flags & CLIENT_TX_VOICE.rawValue {
        case CLIENT_TX_VOICE.rawValue :
            TT_EnableVoiceTransmission(ttInst, 0)
        default :
            TT_EnableVoiceTransmission(ttInst, 1)
        }
        
        updateTX()
    }
    
    func handleTTMessage(var m: TTMessage) {
        switch(m.nClientEvent) {

        case CLIENTEVENT_CON_LOST :
            //TODO: reset channel lists?
            break
            
        case CLIENTEVENT_CMD_PROCESSING :
            if getBoolean(&m) {
                // command active
                self.currentCmdId = m.nSource
            }
            else {
                // command complete
                self.currentCmdId = 0
                
                commandComplete(m.nSource)
            }
        case CLIENTEVENT_CMD_SERVER_UPDATE :
            srvprop = getServerProperties(&m).memory
            
        case CLIENTEVENT_CMD_MYSELF_LOGGEDIN :
            myuseraccount = getUserAccount(&m).memory
            
        case CLIENTEVENT_CMD_CHANNEL_NEW :
            let channel = getChannel(&m).memory
            
            channels[channel.nChannelID] = channel
            
            // initial title will be server name
            if channel.nParentID == 0 {
                updateTitle()
            }
            
            if currentCmdId == 0 {
                self.tableView.reloadData()
            }
            
        case CLIENTEVENT_CMD_CHANNEL_UPDATE :
            let channel = getChannel(&m).memory
            channels[channel.nChannelID] = channel
            
            if currentCmdId == 0 {
                self.tableView.reloadData()
            }
            
        case CLIENTEVENT_CMD_CHANNEL_REMOVE :
            let channel = getChannel(&m).memory
            channels.removeValueForKey(channel.nChannelID)
            
            if currentCmdId == 0 {
                self.tableView.reloadData()
            }
            
        case CLIENTEVENT_CMD_USER_LOGGEDIN :
            let user = getUser(&m).memory
            users[user.nUserID] = user
            
        case CLIENTEVENT_CMD_USER_LOGGEDOUT :
            let user = getUser(&m).memory
            users.removeValueForKey(user.nUserID)
            
        case CLIENTEVENT_CMD_USER_JOINED :
            let user = getUser(&m).memory
            users[user.nUserID] = user
            
            // we joined a new channel so update table view
            if user.nUserID == TT_GetMyUserID(ttInst) {
                curchannel = channels[user.nChannelID]!
                updateTitle()
            }
            
            if currentCmdId == 0 {
                self.tableView.reloadData()
            }
        case CLIENTEVENT_CMD_USER_UPDATE :
            let user = getUser(&m).memory
            users[user.nUserID] = user
            
            if currentCmdId == 0 {
                self.tableView.reloadData()
            }
            
        case CLIENTEVENT_CMD_USER_LEFT :
            let user = getUser(&m).memory
            
            if myuseraccount.uUserRights & USERRIGHT_VIEW_ALL_USERS.rawValue == 0 {
                users.removeValueForKey(user.nUserID)
            }
            else {
                users[user.nUserID] = user
            }
            
            if currentCmdId == 0 {
                self.tableView.reloadData()
            }
            
        case CLIENTEVENT_USER_STATECHANGE :
            let user = getUser(&m).memory
            users[user.nUserID] = user
            self.tableView.reloadData()
            
        default :
            print("Unhandled message \(m.nClientEvent.rawValue)")
        }

    }
}
