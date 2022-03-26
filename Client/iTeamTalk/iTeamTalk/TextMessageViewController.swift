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

class TextMessageViewController :
    UIViewController, UITableViewDataSource,
    UITableViewDelegate, TeamTalkEvent,
    UITextViewDelegate {
    
    @IBOutlet weak var tableView: UITableView!
    @IBOutlet weak var msgTextView: UITextView!
    @IBOutlet weak var sendButton: UIButton!
    
    @IBOutlet weak var msgTextViewBottomConstraint: NSLayoutConstraint!
    @IBOutlet weak var sendButtonBottomConstraint: NSLayoutConstraint!
    
    // userid > 0 means private message session, i.e. MSGTYPE_USER
    var userid : INT32 = 0
    
    var delegate : MyTextMessageDelegate?

    let initial_text = NSLocalizedString("Type text here", comment: "text message")

    var messages = [Int : [MyTextMessage] ]()
    var curMessageSection = 0
    
    func generateKey(_ msg: TextMessage) -> Int {
        return (Int(msg.nMsgType.rawValue) << 16) | Int(msg.nFromUserID)
    }
    
    var mergemessages = [Int : [TextMessage] ]()
    
    func getTextMessageContent(_ msg: TextMessage) -> String? {
        var msg = msg
        let key = generateKey(msg)
        if msg.bMore == TRUE {
            if mergemessages[key] == nil {
                mergemessages[key] = [TextMessage]()
            }
            mergemessages[key]!.append(msg)
            
            // prevent out-of-memory
            if mergemessages[key]!.count > 1000 {
                mergemessages.removeValue(forKey: key)
            }
        }
        else if mergemessages[key] != nil {
            var content = ""
            for m in mergemessages[key]! {
                var m = m
                content += String(cString: getTextMessageString(MESSAGE, &m))
            }
            mergemessages.removeValue(forKey: key)
            return content + String(cString: getTextMessageString(MESSAGE, &msg))
        }
        else {
            return String(cString: getTextMessageString(MESSAGE, &msg))
        }
        return nil
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let def = NotificationCenter.default
        def.addObserver(self, selector: #selector(TextMessageViewController.keyboardWillShow(_:)), name: UIResponder.keyboardWillShowNotification, object: nil)
        def.addObserver(self, selector: #selector(TextMessageViewController.keyboardWillHide(_:)), name: UIResponder.keyboardWillHideNotification, object: nil)
        
        resetText()
        
        tableView.dataSource = self
        tableView.delegate = self
        
        msgTextView.delegate = self
        
        msgTextView.accessibilityLabel = NSLocalizedString("Send empty message to close keyboard", comment: "text message")
    }

    override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)
        
        if self.isMovingFromParent {
            unreadmessages.remove(userid)
        }
    }

    func showLogMessages() -> Bool {
        return userid == 0
    }
    
    func appendEventMessage(_ m : MyTextMessage) {
        if messages[curMessageSection] == nil ||
            messages[curMessageSection]?.last?.fromuserid != m.fromuserid ||
            messages[curMessageSection]?.last?.nickname != m.nickname  ||
            messages[curMessageSection]?.last?.msgtype != m.msgtype {
            curMessageSection += 1
            messages[curMessageSection] = [ MyTextMessage ] ()
        }
        messages[curMessageSection]!.append(m)

        if messages.values.count > MAX_TEXTMESSAGES {
            
            let key = messages.keys.sorted().first
            messages[key!]!.removeFirst()
            if messages[key!]!.isEmpty {
                messages.removeValue(forKey: key!)
            }
        }
    }
    
    func getEventMessage(_ indexPath : IndexPath) -> MyTextMessage {
        let sortedKeys = messages.keys.sorted(by: <)
        let key = sortedKeys[indexPath.section]
        return messages[key]![indexPath.row]
    }
    
    func getLastEventMessage() -> MyTextMessage? {
        return messages[curMessageSection]?.last
    }
    
    func dismissKeyboard() {
        if msgTextView.isFirstResponder {
           msgTextView.resignFirstResponder()
        }
    }
    
    func resetText() {
        msgTextView.text = initial_text
        msgTextView.textColor = UIColor.lightGray
    }
    
    func updateTableView() {
        
        tableView.reloadData()
        
        let n_messages = messages.values.count
        if n_messages > 0 {
            let lastsection = messages.keys.count - 1
            let ip = IndexPath(row: tableView.numberOfRows(inSection: lastsection) - 1, section: lastsection)
            tableView.scrollToRow(at: ip, at: .bottom, animated: true)
        }
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        repositionControls()
    }
    
    @objc func keyboardWillShow(_ notify: Notification) {
        moveForKeyboard(notify, up: true)
    }
    
    @objc func keyboardWillHide(_ notify: Notification) {
        moveForKeyboard(notify, up: false)
    }
    
    func repositionControls() {
        if let tc = self.tabBarController {
            msgTextViewBottomConstraint.constant = tc.tabBar.frame.size.height
            sendButtonBottomConstraint.constant = tc.tabBar.frame.size.height
        }
        else {
            msgTextViewBottomConstraint.constant = 0
            sendButtonBottomConstraint.constant = 0
        }
    }
    
    func moveForKeyboard(_ notify: Notification, up: Bool) {
        if let userInfo = notify.userInfo {
            if let keyboardFrame = (userInfo[UIResponder.keyboardFrameEndUserInfoKey] as AnyObject).cgRectValue {
                
                if up {
                    msgTextViewBottomConstraint.constant = keyboardFrame.height
                    sendButtonBottomConstraint.constant = keyboardFrame.height
                }
                else {
                    repositionControls()
                }
                
                updateTableView()
            }
        }
    }
    
    @IBAction func sendTextMessage(_ sender: UIButton) {
        
        if msgTextView.text.isEmpty {
            msgTextView.resignFirstResponder()
            return
        }
        
        var msg = TextMessage()
        msg.nFromUserID = TT_GetMyUserID(ttInst)
        
        if userid == 0 {
            msg.nMsgType = MSGTYPE_CHANNEL
            msg.nChannelID = TT_GetMyChannelID(ttInst)
        }
        else {
            msg.nMsgType = MSGTYPE_USER
            msg.nToUserID = userid
            
            var user = User()
            TT_GetUser(ttInst, msg.nFromUserID, &user)
            let name = getDisplayName(user)
            let mymsg = MyTextMessage(fromuserid: msg.nFromUserID, nickname: name, msgtype: .PRIV_IM_MYSELF, content: msgTextView.text)

            appendEventMessage(mymsg)

            if delegate != nil {
                delegate!.appendTextMessage(userid, txtmsg: mymsg)
            }
            updateTableView()
        }
        
        if iTeamTalk.sendTextMessage(msg: msg, content: msgTextView.text) {
            msgTextView.text = ""
        }
    }
    
    func textViewShouldBeginEditing(_ textView: UITextView) -> Bool {
        
        if msgTextView.text == initial_text {
            msgTextView.text = ""
        }
        
        if #available(iOS 13, *) {
            msgTextView.textColor = .label
        } else {
            msgTextView.textColor = .black
        }
        
        return true
    }
    
    func textViewDidEndEditing(_ textView: UITextView) {
        if msgTextView.text.isEmpty {
            resetText()
        }
    }
    
    // send message on enter
    func textView(_ textView: UITextView,
                  shouldChangeTextIn range: NSRange,
                  replacementText text: String) -> Bool {
        
        let defaults = UserDefaults.standard
        if defaults.object(forKey: PREF_GENERAL_SENDONRETURN) == nil || defaults.bool(forKey: PREF_GENERAL_SENDONRETURN) {
            
            if text.contains("\n") {
                sendTextMessage(sendButton)
                return false
            }
        }
        return true
    }
    
    func handleTTMessage(_ m: TTMessage) {
        var m = m

        switch m.nClientEvent {
            
        case CLIENTEVENT_CMD_USER_TEXTMSG :
            
            let txtmsg = getTextMessage(&m).pointee
            
            if (txtmsg.nMsgType == MSGTYPE_USER && txtmsg.nFromUserID == userid /* private message to this view controller */) ||
                (txtmsg.nMsgType == MSGTYPE_CHANNEL && userid == 0 /* channel message to tab-bar chat */) ||
                (txtmsg.nMsgType == MSGTYPE_BROADCAST && userid == 0 /* broadcast to tab-bar chat */) {
                
                if let content = getTextMessageContent(txtmsg) {
                    var user = User()
                    TT_GetUser(ttInst, txtmsg.nFromUserID, &user)
                    
                    var msgtype = MsgType.PRIV_IM
                    
                    switch txtmsg.nMsgType {
                    case MSGTYPE_USER :
                        msgtype = TT_GetMyUserID(ttInst) == txtmsg.nFromUserID ? .PRIV_IM_MYSELF : .PRIV_IM
                    case MSGTYPE_CHANNEL :
                        msgtype = TT_GetMyUserID(ttInst) == txtmsg.nFromUserID ? .CHAN_IM_MYSELF : .CHAN_IM
                    case MSGTYPE_BROADCAST :
                        msgtype = .BCAST
                    default :
                        break
                    }

                    let name = getDisplayName(user)
                    let mymsg = MyTextMessage(fromuserid: txtmsg.nFromUserID, nickname: name, msgtype: msgtype, content: content)
                    appendEventMessage(mymsg)
                    
                    if tableView != nil {
                        updateTableView()
                    }
                    
                    speakTextMessage(txtmsg.nMsgType, mymsg: mymsg)
                }
            }
        case CLIENTEVENT_CMD_USER_LOGGEDIN :
            
            let user = getUser(&m).pointee
            if showLogMessages() && TT_GetMyUserID(ttInst) == user.nUserID {
                let logmsg = MyTextMessage(logmsg: NSLocalizedString("Logged on to server", comment: "log entry"))
                appendEventMessage(logmsg)
                
                if tableView != nil {
                    updateTableView()
                }
            }
            
        case CLIENTEVENT_CMD_USER_JOINED :
            
            let user = getUser(&m).pointee
            if showLogMessages() && TT_GetMyChannelID(ttInst) == user.nChannelID {
                var logmsg : MyTextMessage?
                if TT_GetMyUserID(ttInst) == user.nUserID {
                    var channel = Channel()
                    TT_GetChannel(ttInst, user.nChannelID, &channel)
                    var channame : String
                    if channel.nParentID == 0 {
                        channame = NSLocalizedString("root channel", comment: "log entry")
                    }
                    else {
                        channame = getChannel(channel, strprop: NAME)
                    }
                    let txt = String(format: NSLocalizedString("Joined %@", comment: "log entry"), channame)
                    logmsg = MyTextMessage(logmsg: txt)
                }
                else {
                    let name = getDisplayName(user)
                    let txt = String(format: NSLocalizedString("%@ joined channel", comment: "log entry"), name)
                    logmsg = MyTextMessage(logmsg: txt)
                }
                
                appendEventMessage(logmsg!)
                
                if tableView != nil {
                    updateTableView()
                }
            }
        case CLIENTEVENT_CMD_USER_LEFT :
            
            let user = getUser(&m).pointee
            if showLogMessages() && TT_GetMyChannelID(ttInst) == m.nSource {
                let name = getDisplayName(user)
                let txt = String(format: NSLocalizedString("%@ left channel", comment: "log entry"), name)
                let logmsg = MyTextMessage(logmsg: txt)
                appendEventMessage(logmsg)
                
                if tableView != nil {
                    updateTableView()
                }
            }
        case CLIENTEVENT_CMD_ERROR :
            
            let errmsg = getClientErrorMsg(m.clienterrormsg, strprop: ERRMESSAGE)
            let txt = String(format: NSLocalizedString("Command failed: %@", comment: "log entry"),  errmsg)
            let logmsg = MyTextMessage(logmsg: txt)
            appendEventMessage(logmsg)
            
            if tableView != nil {
                updateTableView()
            }
        default : break
        }
    }
    
    func numberOfSections(in tableView: UITableView) -> Int {

        return messages.keys.count
    }
    
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        let sortedKeys = messages.keys.sorted()
        let key = sortedKeys[section]
        return messages[key]!.count
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {

        let cell = tableView.dequeueReusableCell(withIdentifier: "Text Msg Cell") as! TextMsgTableCell
        getEventMessage(indexPath).drawCell(cell)
        
        return cell
    }
    
    func tableView(_ tableView: UITableView, heightForRowAt indexPath: IndexPath) -> CGFloat {
        let cell = tableView.dequeueReusableCell(withIdentifier: "Text Msg Cell") as! TextMsgTableCell

        cell.messageTextView.text = getEventMessage(indexPath).message
        
        let fixedWidth = cell.messageTextView.frame.size.width
        let newSize = cell.messageTextView.sizeThatFits(CGSize(width: fixedWidth, height: CGFloat.greatestFiniteMagnitude))
        
        return newSize.height
    }

    func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        let em = getEventMessage(IndexPath(row: 0, section: section))
        switch em.msgtype {
        case .PRIV_IM :
            fallthrough
        case .PRIV_IM_MYSELF :
            fallthrough
        case .CHAN_IM :
            fallthrough
        case .CHAN_IM_MYSELF :
            fallthrough
        case .BCAST :
            return em.nickname
        case .LOGMSG :
            return NSLocalizedString("Status Event", comment: "Text message view")
        }
    }
}
