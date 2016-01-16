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

    //shared TTInstance between all view controllers
    var ttInst = UnsafeMutablePointer<Void>()
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

    let synth = AVSpeechSynthesizer()
    var myUtterance = AVSpeechUtterance(string: "")
    
    override func viewDidLoad() {
        super.viewDidLoad()

        tableView.dataSource = self
        tableView.delegate = self
    
        updateTX()
    }
    
    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)
        
        tableView.reloadData()
    }
    
    override func viewDidDisappear(animated: Bool) {
        super.viewDidDisappear(animated)
        
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @IBAction func joinChannel(sender: UIButton) {
        
        if curchannel.bPassword != 0 {
            let alertView = UIAlertView(title: NSLocalizedString("Enter Password", comment: "Dialog message"), message: NSLocalizedString("Password", comment: "Dialog message"), delegate: self, cancelButtonTitle: NSLocalizedString("Join", comment: "Dialog message"))
            alertView.alertViewStyle = .SecureTextInput
            alertView.tag = Int(curchannel.nChannelID)
            if let passwd = chanpasswds[curchannel.nChannelID] {
                alertView.textFieldAtIndex(0)?.text = passwd
            }
            alertView.show()
        }
        else {
            let cmdid = TT_DoJoinChannelByID(ttInst, curchannel.nChannelID, "")
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
            let nickname = fromTTString(user.szNickname)
            let statusmsg = fromTTString(user.szStatusMsg)
            
            cell.nicknameLabel.text = limitText(nickname)
            cell.statusmsgLabel.text = statusmsg
            
            if user.uUserState & USERSTATE_VOICE.rawValue != 0 ||
                (TT_GetMyUserID(ttInst) == user.nUserID &&
                    isTransmitting(ttInst, stream: STREAMTYPE_VOICE)) {
                        cell.userImage.image = UIImage(named: "man_green.png")
            }
            else {
                cell.userImage.image = UIImage(named: "man_blue.png")
            }
            
            cell.messageBtn.tag = Int(user.nUserID)
            cell.tag = Int(user.nUserID)
            
            return cell
        }
        else {

            let cellIdentifier = "ChannelTableCell"
            let cell = tableView.dequeueReusableCellWithIdentifier(cellIdentifier, forIndexPath: indexPath) as! ChannelTableCell
            
            var channel = Channel()
            var textcolor : UIColor? = nil
            var title : String?, subtitle : String?
            
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
            }
            else if curchannel.nChannelID == 0 {
                // display only the root channel
                channel = subchans[chan_index]
                
                title = fromTTString(srvprop.szServerName)
                subtitle = fromTTString(channel.szTopic)
                
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
                    channel = subchans[chan_index - 1]
                }
                else {
                    channel = subchans[chan_index]
                }
                
                title = fromTTString(channel.szName)
                subtitle = fromTTString(channel.szTopic)
                
                if channel.bPassword != 0 {
                    cell.chanimage.image = UIImage(named: "channel_pink.png")
                }
                else {
                    cell.chanimage.image = UIImage(named: "channel_orange.png")
                }

            }

            cell.channame.textColor = textcolor
            cell.chantopicLabel.textColor = textcolor

            cell.channame.text = limitText(title!)
            cell.chantopicLabel.text = subtitle
            
            cell.editBtn.tag = Int(channel.nChannelID)
            cell.tag = Int(channel.nChannelID)
            
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
                else if NSUserDefaults.standardUserDefaults().boolForKey(PREF_JOINROOTCHANNEL) {
                    
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
            openTextMessages(txtmsgView, userid: INT32(btn.tag))
        }
    }
    
    @IBAction func openTextMessages(segue:UIStoryboardSegue) {

        let src_vc = segue.sourceViewController as! UserDetailViewController
        
        let vc = self.storyboard?.instantiateViewControllerWithIdentifier("Text Message") as! TextMessageViewController
        openTextMessages(vc, userid: src_vc.userid)
        self.navigationController?.pushViewController(vc, animated: true)

    }

    func openTextMessages(sender: TextMessageViewController, userid: INT32) {
        sender.ttInst = self.ttInst
        sender.userid = userid
        sender.delegate = self
        if (self.textmessages[userid] != nil) {
            sender.messages = self.textmessages[userid]!
        }
    }
    
    @IBAction func closeTextMessages(segue:UIStoryboardSegue) {
        
        print("Closed messages")
        
    }

    @IBAction func txBtnDown(sender: UIButton) {
        TT_EnableVoiceTransmission(ttInst, 1)
        playSound(.TX_ON)
        updateTX()
    }
    
    func txBtnUp() {
        
        let now = NSDate()
        
        if pttLockTimeout.earlierDate(now) == now {
            TT_EnableVoiceTransmission(ttInst, 1)
            playSound(.TX_ON)
        }
        else {
            TT_EnableVoiceTransmission(ttInst, 0)
            playSound(.TX_OFF)
        }
        updateTX()
        // PTT lock is 0.5 sec
        pttLockTimeout = now.dateByAddingTimeInterval(0.5)
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
        default :
            txButton.backgroundColor = UIColor.greenColor()
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
                let alert = UIAlertController(title: NSLocalizedString("Error", comment: "Dialog"), message: s, preferredStyle: UIAlertControllerStyle.Alert)
                alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "Dialog"), style: UIAlertActionStyle.Default, handler: nil))
                self.presentViewController(alert, animated: true, completion: nil)
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
                mychannel = channels[user.nChannelID]!
                updateTitle()
            }
            if user.nChannelID == mychannel.nChannelID {
                playSound(.JOINED_CHAN)
                let defaults = NSUserDefaults.standardUserDefaults()
                
                if defaults.objectForKey(PREF_TTSEVENT_JOINEDCHAN) == nil || defaults.boolForKey(PREF_TTSEVENT_JOINEDCHAN) {
                    let nickname = limitText(fromTTString(user.szNickname))
                    myUtterance = AVSpeechUtterance(string: nickname + " " +  NSLocalizedString("has joined the channel", comment: "TTS EVENT"))
                    synth.speakUtterance(myUtterance)
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
            
            if m.nSource == mychannel.nChannelID {
                playSound(.LEFT_CHAN)
                let defaults = NSUserDefaults.standardUserDefaults()
                if defaults.objectForKey(PREF_TTSEVENT_LEFTCHAN) == nil || defaults.boolForKey(PREF_TTSEVENT_LEFTCHAN) {
                    let nickname = limitText(fromTTString(user.szNickname))
                    myUtterance = AVSpeechUtterance(string: nickname + " " + NSLocalizedString("has left the channel", comment: "TTS EVENT"))
                    synth.speakUtterance(myUtterance)
                }
            }
            
            if currentCmdId == 0 {
                self.tableView.reloadData()
            }
            
        case CLIENTEVENT_CMD_USER_TEXTMSG :
            let txtmsg = getTextMessage(&m).memory
            
            if txtmsg.nMsgType == MSGTYPE_USER {
                if let user = users[txtmsg.nFromUserID] {
                    let newmsg = MyTextMessage(m: txtmsg, nickname: fromTTString(user.szNickname),
                        msgtype: TT_GetMyUserID(ttInst) == txtmsg.nFromUserID ? .IM_MYSELF : .IM)
                    appendTextMessage(txtmsg.nFromUserID, txtmsg: newmsg)
                    
                    if unreadmessages.count == 0 {
                        unreadTimer = NSTimer.scheduledTimerWithTimeInterval(1.0, target: self, selector: "timerUnread", userInfo: nil, repeats: true)
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
                
                let settings = NSUserDefaults.standardUserDefaults()
                if settings.objectForKey(PREF_DISPLAY_POPUPTXTMSG) == nil || settings.boolForKey(PREF_DISPLAY_POPUPTXTMSG) {
                    let vc = self.storyboard?.instantiateViewControllerWithIdentifier("Text Message") as! TextMessageViewController
                    openTextMessages(vc, userid: txtmsg.nFromUserID)
                    self.navigationController?.pushViewController(vc, animated: true)
                    addToTTMessages(vc)
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
