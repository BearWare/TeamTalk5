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
import AVFoundation

class ChannelListViewController :
    UIViewController, UITableViewDataSource,
    UITableViewDelegate, UIAlertViewDelegate,
    MyTextMessageDelegate, TeamTalkEvent  {

    // all channels on server
    var channels = [INT32 : Channel]()
    // channel passwords
    var chanpasswds = [INT32 : String]()
    // the channel being displayed (not nescessarily the same channel as we're in)
    var curchannel = Channel()
    // joined channel
    var mychannel = Channel()
    // all users on server
    var users = [INT32 : User]()
    // the ID of the command for which we're expecting a result
    var cmdid : INT32 = 0
    // the command ID which is currently processing
    var currentCmdId : INT32 = 0
    // properties of connected server
    var srvprop = ServerProperties()
    // local instance's user account
    var myuseraccount = UserAccount()
    // user to user text messages
    var textmessages = [INT32 : [MyTextMessage] ]()
    // timer for blinking unread messages
    var unreadTimer : NSTimer?
    // list of channels and users
    @IBOutlet weak var tableView: UITableView!
    // PTT button
    @IBOutlet weak var txButton: UIButton!
    // timeout for PTT lock
    var pttLockTimeout = NSDate()

    override func viewDidLoad() {
        super.viewDidLoad()

        tableView.dataSource = self
        tableView.delegate = self
    
        updateTX()
    }
    
    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)
        
        tableView.reloadData()
        updateTX()
    }
    
    override func viewDidDisappear(animated: Bool) {
        super.viewDidDisappear(animated)
        
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @IBAction func joinChannel(sender: UIButton) {
        joinNewChannel(curchannel)
    }
    
    func joinNewChannel(channel: Channel) {
        if channel.bPassword != 0 {
            let alertView = UIAlertView(title: NSLocalizedString("Enter Password", comment: "Dialog message"), message: NSLocalizedString("Password", comment: "Dialog message"), delegate: self, cancelButtonTitle: NSLocalizedString("Join", comment: "Dialog message"))
            alertView.alertViewStyle = .SecureTextInput
            alertView.tag = Int(channel.nChannelID)
            if let passwd = chanpasswds[channel.nChannelID] {
                alertView.textFieldAtIndex(0)?.text = passwd
            }
            alertView.show()
        }
        else {
            let cmdid = TT_DoJoinChannelByID(ttInst, channel.nChannelID, "")
            activeCommands[cmdid] = .JoinCmd
        }
        
    }
    
    func alertView(alertView: UIAlertView, clickedButtonAtIndex buttonIndex: Int) {
        let passwd = (alertView.textFieldAtIndex(0)?.text)!
        chanpasswds[INT32(alertView.tag)] = passwd
        let cmdid = TT_DoJoinChannelByID(ttInst, INT32(alertView.tag), passwd)
        activeCommands[cmdid] = .JoinCmd
    }
    
    enum Command {
        case LoginCmd, JoinCmd
    }
    
    var activeCommands = [INT32: Command]()
    
    func appendTextMessage(userid: INT32, txtmsg: MyTextMessage) {
        
        if textmessages[userid] == nil {
            textmessages[userid] = [MyTextMessage]()
        }
        textmessages[userid]?.append(txtmsg)
        
        if textmessages[userid]?.count > MAX_TEXTMESSAGES {
            textmessages[userid]?.removeFirst()
        }
    }
    
    func getUsersCount(chanid: INT32) -> Int {
        
        let chanusers : [User] = users.values.filter({$0.nChannelID == chanid})
        
        return chanusers.count
    }
    
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
        
        var n_items = subchans.count + chanusers.count
        
        if curchannel.nChannelID != mychannel.nChannelID && curchannel.nChannelID > 0 {
            n_items += 1 // +1 for 'Join this channel'
        }
        
        if curchannel.nParentID != 0 {
            n_items += 1 //+1 for 'Back' to parent channel
        }
        return n_items
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {

        let (subchans, chanusers) = getDisplayItems()

        //print("row = \(indexPath.row) cur channel = \(curchannel.nChannelID) subs = \(subchans.count) users = \(chanusers.count)")

        let button_index = curchannel.nChannelID != mychannel.nChannelID && curchannel.nChannelID > 0 ? 0 : -1
        
        // current index for users
        var user_index = indexPath.row
        if button_index >= 0 {
            user_index -= 1
        }
        
        // current index for channels
        var chan_index = indexPath.row - chanusers.count
        if button_index >= 0 {
            chan_index -= 1
        }

        if button_index == indexPath.row {
            let cellIdentifier = "JoinChannelCell"
            let cell = tableView.dequeueReusableCellWithIdentifier(cellIdentifier, forIndexPath: indexPath)
            return cell
        }
        else if user_index < chanusers.count {
            
            let cellIdentifier = "UserTableCell"
            let cell = tableView.dequeueReusableCellWithIdentifier(cellIdentifier, forIndexPath: indexPath) as! UserTableCell
            let user = chanusers[user_index]
            let name = getDisplayName(user)
            let statusmsg = fromTTString(user.szStatusMsg)
            
            cell.nicknameLabel.text = name
            cell.statusmsgLabel.text = statusmsg
            
            cell.userImage.accessibilityLabel = NSLocalizedString("User", comment: "channel list")
            if user.uUserState & USERSTATE_VOICE.rawValue != 0 ||
                (TT_GetMyUserID(ttInst) == user.nUserID &&
                    isTransmitting(ttInst, stream: STREAMTYPE_VOICE)) {
                        
                cell.userImage.image = UIImage(named: "man_green.png")
                cell.userImage.accessibilityHint = NSLocalizedString("Talking", comment: "channel list")
            }
            else {
                cell.userImage.image = UIImage(named: "man_blue.png")
                cell.userImage.accessibilityHint = NSLocalizedString("Silent", comment: "channel list")
            }
            
            cell.messageBtn.tag = Int(user.nUserID)
            cell.tag = Int(user.nUserID)
            
            if #available(iOS 8.0, *) {
                let action_msg = MyCustomAction(name: NSLocalizedString("Send private message", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.messageUser(_:)), tag: cell.tag)
                let action_mute = MyCustomAction(name: NSLocalizedString("Mute", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.muteUser(_:)), tag: cell.tag)
                let action_kick = MyCustomAction(name: NSLocalizedString("Kick user", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.kickUser(_:)), tag: cell.tag)
                
                if (myuseraccount.uUserRights & USERRIGHT_KICK_USERS.rawValue) != 0 {
                    cell.accessibilityCustomActions = [ action_msg, action_mute, action_kick ]
                }
                else {
                    cell.accessibilityCustomActions = [ action_msg, action_mute ]
                }
            } else {
                // Fallback on earlier versions
            }
            
            return cell
        }
        else {

            let cellIdentifier = "ChannelTableCell"
            let cell = tableView.dequeueReusableCellWithIdentifier(cellIdentifier, forIndexPath: indexPath) as! ChannelTableCell
            
            var channel = Channel()
            var textcolor : UIColor? = nil
            var title : String?, subtitle : String?
            
            cell.chanimage.accessibilityLabel = NSLocalizedString("Channel", comment: "channel list")

            if chan_index == 0 && curchannel.nParentID != 0 {
                
                // display previous channel if not in root channel
                
                channel = channels[curchannel.nParentID]!
                
                title = NSLocalizedString("Parent channel", comment: "channel list")
                if channel.nParentID == 0 {
                    subtitle = fromTTString(srvprop.szServerName)
                }
                else {
                    subtitle = fromTTString(channel.szName)
                }
                
                textcolor = UIColor.grayColor()
                cell.chanimage.image = UIImage(named: "back_orange.png")
                cell.chanimage.accessibilityHint = NSLocalizedString("Return to previous channel", comment: "channel list")
            }
            else if curchannel.nChannelID == 0 {
                
                // display only the root channel
                
                channel = subchans[chan_index]
                
                title = fromTTString(srvprop.szServerName)
                subtitle = fromTTString(channel.szTopic)
                
                if channel.bPassword != 0 {
                    cell.chanimage.image = UIImage(named: "channel_pink.png")
                    cell.chanimage.accessibilityHint = NSLocalizedString("Password protected", comment: "channel list")
                }
                else {
                    cell.chanimage.image = UIImage(named: "channel_orange.png")
                    cell.chanimage.accessibilityHint = NSLocalizedString("No password", comment: "channel list")
                }
            }
            else  {
                
                // display sub channels
                
                if curchannel.nParentID != 0 {
                    // root channel doesn't display access to parent
                    channel = subchans[chan_index - 1]
                }
                else {
                    channel = subchans[chan_index]
                }
                
                let user_count = getUsersCount(channel.nChannelID)
                title = fromTTString(channel.szName) + " (\(user_count))"
                subtitle = fromTTString(channel.szTopic)
                
                if channel.bPassword != 0 {
                    cell.chanimage.image = UIImage(named: "channel_pink.png")
                    cell.chanimage.accessibilityHint = NSLocalizedString("Password protected", comment: "channel list")
                }
                else {
                    cell.chanimage.image = UIImage(named: "channel_orange.png")
                    cell.chanimage.accessibilityHint = NSLocalizedString("No password", comment: "channel list")
                }

                cell.chanimage.accessibilityLabel =
                    String(format: NSLocalizedString("Channel. %d users", comment: "channel list"), user_count)             
            }

            cell.channame.textColor = textcolor
            cell.chantopicLabel.textColor = textcolor

            cell.channame.text = limitText(title!)
            cell.chantopicLabel.text = subtitle
            
            cell.editBtn.tag = Int(channel.nChannelID)
            cell.tag = Int(channel.nChannelID)

            if #available(iOS 8.0, *) {
                let action_join = MyCustomAction(name: NSLocalizedString("Join channel", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.joinThisChannel(_:)), tag: cell.tag)
                var action_edit : MyCustomAction?
                if (myuseraccount.uUserRights & USERRIGHT_MODIFY_CHANNELS.rawValue) == 0 {
                    cell.editBtn.setTitle(NSLocalizedString("View", comment: "channel list"), forState: .Normal)
                    action_edit = MyCustomAction(name: NSLocalizedString("View properties", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.editChannel(_:)), tag: cell.tag)
                }
                else {
                    action_edit = MyCustomAction(name: NSLocalizedString("Edit properties", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.editChannel(_:)), tag: cell.tag)
                }
                
                cell.accessibilityCustomActions = [ action_join, action_edit! ]
            } else {
                // Fallback on earlier versions
            }
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
            title = fromTTString(srvprop.szServerName)
        }
        else {
            title = fromTTString(curchannel.szName)
        }
        
        self.tabBarController?.navigationItem.title = title
    }
    
    @available(iOS 8.0, *)
    func messageUser(action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            performSegueWithIdentifier("New TextMessage", sender: ac)
        }
        return true
    }

 @available(iOS 8.0, *)
       func muteUser(action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            let userid = INT32(ac.tag)
            if let user = users[userid] {
                TT_SetUserMute(ttInst, userid, STREAMTYPE_MEDIAFILE_AUDIO,
                    (user.uUserState & USERSTATE_MUTE_MEDIAFILE.rawValue) == 0 ? TRUE : FALSE )
                TT_SetUserMute(ttInst, userid, STREAMTYPE_VOICE,
                    (user.uUserState & USERSTATE_MUTE_VOICE.rawValue) == 0 ? TRUE : FALSE )
                // tell TeamTalk event loop to send us an updated User-struct
                TT_PumpMessage(ttInst, CLIENTEVENT_USER_STATECHANGE, userid)
            }

        }
        return true
    }
    
    @available(iOS 8.0, *)
    func kickUser(action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            
            cmdid = TT_DoKickUser(ttInst, INT32(ac.tag), curchannel.nChannelID)
        }
        return true
    }

    @available(iOS 8.0, *)
    func joinThisChannel(action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            if let channel = channels[INT32(ac.tag)] {
                joinNewChannel(channel)
            }
        }
        return true
    }

    @available(iOS 8.0, *)
    func editChannel(action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            performSegueWithIdentifier("Edit Channel", sender: ac)
        }
        return true
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
            
            if (flags & CLIENT_AUTHORIZED.rawValue) != 0 {
                
                // if we were previously in a channel then rejoin
                if mychannel.nChannelID > 0 {
                    let passwd = chanpasswds[mychannel.nChannelID] != nil ? chanpasswds[mychannel.nChannelID] : ""
                    
                    let cmdid = TT_DoJoinChannelByID(ttInst, mychannel.nChannelID, passwd!)
                    if cmdid > 0 {
                        activeCommands[cmdid] = .JoinCmd
                    }
                }
                else if NSUserDefaults.standardUserDefaults().objectForKey(PREF_JOINROOTCHANNEL) == nil ||
                    NSUserDefaults.standardUserDefaults().boolForKey(PREF_JOINROOTCHANNEL) {
                    
                    let cmdid = TT_DoJoinChannelByID(ttInst, TT_GetRootChannelID(ttInst), "")
                    if cmdid > 0 {
                        activeCommands[cmdid] = .JoinCmd
                    }
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
        }
        else if segue.identifier == "New Channel" {
            let chanDetail = segue.destinationViewController as! ChannelDetailViewController
            chanDetail.channel.nParentID = curchannel.nChannelID
            
            if chanDetail.channel.nParentID == 0 {
                let subchans = channels.values.filter({$0.nParentID == 0})
                if let root = subchans.first {
                    chanDetail.channel.nParentID = root.nChannelID
                }
            }
        }
        else if segue.identifier == "Edit Channel" {
            
            var chanid : INT32 = 0
            
            if let btn = sender as? UIButton {
                chanid = INT32(btn.tag)
            }
            if #available(iOS 8.0, *) {
                if let ac = sender as? MyCustomAction {
                    chanid = INT32(ac.tag)
                }
            } else {
                // Fallback on earlier versions
            }

            let channel = channels[chanid]
            
            let chanDetail = segue.destinationViewController as! ChannelDetailViewController
            chanDetail.channel = channel!
        }
        else if segue.identifier == "New TextMessage" {

            var userid : INT32 = -1
            if let btn = sender as? UIButton {
                userid = INT32(btn.tag)
            }
            if #available(iOS 8.0, *) {
                if let action = sender as? MyCustomAction {
                    userid = INT32(action.tag)
                }
            } else {
                // Fallback on earlier versions
            }
            
            let txtmsgView = segue.destinationViewController as! TextMessageViewController
            openTextMessages(txtmsgView, userid: userid)
        }
    }
    
    @IBAction func openTextMessages(segue:UIStoryboardSegue) {

        let src_vc = segue.sourceViewController as! UserDetailViewController
        
        let vc = self.storyboard?.instantiateViewControllerWithIdentifier("Text Message") as! TextMessageViewController
        openTextMessages(vc, userid: src_vc.userid)
        self.navigationController?.pushViewController(vc, animated: true)

    }

    func openTextMessages(sender: TextMessageViewController, userid: INT32) {
        sender.userid = userid
        sender.delegate = self
        addToTTMessages(sender)
        if (self.textmessages[userid] != nil) {
            sender.messages = self.textmessages[userid]!
        }
    }
    
    @IBAction func closeTextMessages(segue:UIStoryboardSegue) {
        
        print("Closed messages")
        
    }
    
    @IBAction func txBtnDown(sender: UIButton) {
        
        if hasPTTLock() {
            enableVoiceTx(true)
        }
        else {
            enableVoiceTx(!isTransmitting(ttInst, stream: STREAMTYPE_VOICE))
        }
    }
    
    func enableVoiceTx(enable: Bool) {

        TT_EnableVoiceTransmission(ttInst, enable ? TRUE : FALSE)
        playSound(enable ? .TX_ON : .TX_OFF)
        updateTX()

    }
    
    func txBtnUp() {
        
        if hasPTTLock() {
            
            let now = NSDate()
            
            if pttLockTimeout.earlierDate(now) == now {
                enableVoiceTx(true)
            }
            else {
                enableVoiceTx(false)
            }
            // PTT lock is 0.5 sec
            pttLockTimeout = now.dateByAddingTimeInterval(0.5)
        }
    }
    
    @IBAction func txBtnUpInside(sender: UIButton) {
        txBtnUp()
    }
    
    @IBAction func txBtnUpOutside(sender: UIButton) {
        txBtnUp()
    }
    
    func updateTX() {
        
        let flags = TT_GetFlags(ttInst)
        
        switch flags & CLIENT_TX_VOICE.rawValue {
        case CLIENT_TX_VOICE.rawValue :
            txButton.backgroundColor = UIColor.redColor()
            txButton.accessibilityLabel = NSLocalizedString("Stop transmit", comment: "channel list")
        default :
            txButton.backgroundColor = UIColor.greenColor()
            txButton.accessibilityLabel = NSLocalizedString("Transmit", comment: "channel list")
        }
        
        if hasPTTLock() {
            txButton.accessibilityHint = NSLocalizedString("Double tap and hold to transmit. Triple tap fast to lock transmission.", comment: "channel list")
        }
        else {
            txButton.accessibilityHint = NSLocalizedString("Toggle to enable transmission", comment: "channel list")
        }
        
        tableView.reloadData()
        
    }
    
    func timerUnread() {
        let cells = tableView.visibleCells
        for c in cells {
            if c.reuseIdentifier == "UserTableCell"  {
                let cell = c as! UserTableCell
                if unreadmessages.contains(INT32(c.tag)) {
                    let time = Int(NSDate().timeIntervalSince1970)
                    if time % 2 == 0 {
                        cell.messageBtn.setImage(UIImage(named: "message_red"), forState: .Normal)
                    }
                    else {
                        cell.messageBtn.setImage(UIImage(named: "message_blue"), forState: .Normal)
                    }
                }
                else {
                    cell.messageBtn.setImage(UIImage(named: "message_blue"), forState: .Normal)
                }
            }
        }
        if unreadmessages.isEmpty {
            unreadTimer?.invalidate()
        }
    }
    
    func handleTTMessage(var m: TTMessage) {
        
        switch(m.nClientEvent) {

        case CLIENTEVENT_CON_LOST :
            
            channels.removeAll()
            users.removeAll()
            curchannel = Channel()
            activeCommands.removeAll()
            
            tableView.reloadData()
            break
            
        case CLIENTEVENT_CMD_PROCESSING :
            if getTTBOOL(&m) != 0 {
                // command active
                self.currentCmdId = m.nSource
            }
            else {
                // command complete
                self.currentCmdId = 0
                
                commandComplete(m.nSource)
            }
        case CLIENTEVENT_CMD_ERROR :
            if activeCommands[m.nSource] != nil {
                let errmsg = getClientErrorMsg(&m).memory
                let s = fromTTString(errmsg.szErrorMsg)
                if #available(iOS 8.0, *) {
                    let alert = UIAlertController(title: NSLocalizedString("Error", comment: "Dialog"), message: s, preferredStyle: UIAlertControllerStyle.Alert)
                    alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "Dialog"), style: UIAlertActionStyle.Default, handler: nil))
                    self.presentViewController(alert, animated: true, completion: nil)
                } else {
                    // Fallback on earlier versions
                }
            }
        case CLIENTEVENT_CMD_SERVER_UPDATE :
            srvprop = getServerProperties(&m).memory
            
        case CLIENTEVENT_CMD_MYSELF_LOGGEDIN :
            myuseraccount = getUserAccount(&m).memory
            if (myuseraccount.uUserType & USERTYPE_ADMIN.rawValue) != 0 {
                // an admin user type can do everything
                myuseraccount.uUserRights = 0xFFFFFFFF
            }
            
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
                mychannel = channels[user.nChannelID]!
                updateTitle()
            }
            if user.nChannelID == mychannel.nChannelID && mychannel.nChannelID > 0 {
                playSound(.JOINED_CHAN)
                let defaults = NSUserDefaults.standardUserDefaults()
                
                if defaults.objectForKey(PREF_TTSEVENT_JOINEDCHAN) == nil || defaults.boolForKey(PREF_TTSEVENT_JOINEDCHAN) {
                    let name = getDisplayName(user)
                    newUtterance(name + " " +  NSLocalizedString("has joined the channel", comment: "TTS EVENT"))
                }
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
            users[user.nUserID] = user
            
            if myuseraccount.uUserRights & USERRIGHT_VIEW_ALL_USERS.rawValue == 0 {
                users.removeValueForKey(user.nUserID)
            }
            else {
                users[user.nUserID] = user
            }
    
            if user.nUserID == TT_GetMyUserID(ttInst) {
                mychannel = Channel()
            }
            
            if m.nSource == mychannel.nChannelID && mychannel.nChannelID > 0 {
                playSound(.LEFT_CHAN)
                let defaults = NSUserDefaults.standardUserDefaults()
                if defaults.objectForKey(PREF_TTSEVENT_LEFTCHAN) == nil || defaults.boolForKey(PREF_TTSEVENT_LEFTCHAN) {
                    let name = getDisplayName(user)
                    newUtterance(name + " " + NSLocalizedString("has left the channel", comment: "TTS EVENT"))
                }
            }
            
            if currentCmdId == 0 {
                self.tableView.reloadData()
            }
            
        case CLIENTEVENT_CMD_USER_TEXTMSG :
            let txtmsg = getTextMessage(&m).memory
            
            if txtmsg.nMsgType == MSGTYPE_USER {
                
                let settings = NSUserDefaults.standardUserDefaults()
                if let user = users[txtmsg.nFromUserID] {
                    let name = getDisplayName(user)
                    let newmsg = MyTextMessage(m: txtmsg, nickname: name,
                        msgtype: TT_GetMyUserID(ttInst) == txtmsg.nFromUserID ? .IM_MYSELF : .IM)
                    appendTextMessage(txtmsg.nFromUserID, txtmsg: newmsg)
                    
                    if unreadmessages.count == 0 {
                        unreadTimer = NSTimer.scheduledTimerWithTimeInterval(1.0, target: self, selector: #selector(ChannelListViewController.timerUnread), userInfo: nil, repeats: true)
                    }
                    unreadmessages.insert(txtmsg.nFromUserID)                    
                }
                
                //ignore incoming message if text message view controller is already open
                if self.navigationController?.topViewController is TextMessageViewController {
                    let vc = self.navigationController?.topViewController as! TextMessageViewController
                    if vc.userid == txtmsg.nFromUserID {
                        break
                    }
                }
                
                if settings.objectForKey(PREF_DISPLAY_POPUPTXTMSG) == nil || settings.boolForKey(PREF_DISPLAY_POPUPTXTMSG) {
                    let vc = self.storyboard?.instantiateViewControllerWithIdentifier("Text Message") as! TextMessageViewController
                    openTextMessages(vc, userid: txtmsg.nFromUserID)
                    self.navigationController?.pushViewController(vc, animated: true)
                    if vc.messages.count > 0 {
                        speakTextMessage(txtmsg.nMsgType, mymsg: vc.messages.last!)
                    }
                }
            }
            
        case CLIENTEVENT_CMD_ERROR :
            if m.nSource == cmdid {
                let errmsg = getClientErrorMsg(&m).memory
                let s = fromTTString(errmsg.szErrorMsg)
                if #available(iOS 8.0, *) {
                    let alert = UIAlertController(title: NSLocalizedString("Error", comment: "Dialog message"), message: s, preferredStyle: UIAlertControllerStyle.Alert)
                    alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "Dialog message"), style: UIAlertActionStyle.Default, handler: nil))
                    self.presentViewController(alert, animated: true, completion: nil)
                } else {
                    // Fallback on earlier versions
                }
            }

        case CLIENTEVENT_USER_STATECHANGE :
            let user = getUser(&m).memory
            users[user.nUserID] = user
            self.tableView.reloadData()
        
        case CLIENTEVENT_VOICE_ACTIVATION :
            
            tableView.reloadData()

        default :
            //print("Unhandled message \(m.nClientEvent.rawValue)")
            break
        }

    }
}
