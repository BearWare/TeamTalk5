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
    
    var userid : INT32 = 0
    
    var delegate : MyTextMessageDelegate?

    let initial_text = NSLocalizedString("Type text here", comment: "text message")

    var messages : [MyTextMessage] = []
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let def = NotificationCenter.default
        def.addObserver(self, selector: #selector(TextMessageViewController.keyboardWillShow(_:)), name: UIResponder.keyboardWillShowNotification, object: nil)
        def.addObserver(self, selector: #selector(TextMessageViewController.keyboardWillHide(_:)), name: UIResponder.keyboardWillHideNotification, object: nil)
        
        resetText()
        
        tableView.dataSource = self
        tableView.delegate = self
        
        msgTextView.delegate = self
        
        msgTextView.accessibilityHint = NSLocalizedString("Send empty message to close keyboard", comment: "text message")
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
        
        if tableView.numberOfRows(inSection: 0) > 0 {
//            let ip = NSIndexPath(forRow: tableView.numberOfRowsInSection(0)-1, inSection: 0)
//            tableView.scrollToRowAtIndexPath(ip, atScrollPosition: .Top, animated: true)
//            let frame = tableView.frame
//            let content = tableView.contentSize
//            tableView.setContentOffset(CGPointMake(0, rect.height - frame.height), animated: true)
//            if content.height > frame.height {
                let ip = IndexPath(row: tableView.numberOfRows(inSection: 0)-1, section: 0)
                tableView.scrollToRow(at: ip, at: .bottom, animated: true)
//            }
        }
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
    }
    
    @objc func keyboardWillShow(_ notify: Notification) {
        moveForKeyboard(notify, up: true)
    }
    
    @objc func keyboardWillHide(_ notify: Notification) {
        moveForKeyboard(notify, up: false)
    }
    
    func moveForKeyboard(_ notify: Notification, up: Bool) {
        if let userInfo = notify.userInfo {
            if let keyboardFrame = (userInfo[UIResponder.keyboardFrameEndUserInfoKey] as AnyObject).cgRectValue {
                
                let selfFrame = self.view.frame
                var newTableFrame = tableView.frame
                var newTextViewFrame = msgTextView.frame
                var newSendBtnFrame = sendButton.frame
                
                if up {
                    newTextViewFrame.origin.y = keyboardFrame.origin.y - newTextViewFrame.height
                    newSendBtnFrame.origin.y = keyboardFrame.origin.y - newSendBtnFrame.height
                    
                    newTableFrame.size.height = selfFrame.height - keyboardFrame.height - newTextViewFrame.height
                }
                else {
                    var tabBarHeight : CGFloat = 0.0
                    if self.tabBarController != nil {
                        tabBarHeight = (self.tabBarController?.tabBar.frame.size.height)!
                    }
                    
                    newTextViewFrame.origin.y = selfFrame.height - newTextViewFrame.height - tabBarHeight
                    newSendBtnFrame.origin.y = selfFrame.height - newSendBtnFrame.height - tabBarHeight
                    
                    newTableFrame.size.height = selfFrame.height - newTextViewFrame.height - tabBarHeight
                    
                }
                
                tableView.frame = newTableFrame
                msgTextView.frame = newTextViewFrame
                sendButton.frame = newSendBtnFrame
                
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
        toTTString(msgTextView.text, dst: &msg.szMessage)
        
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
            let mymsg = MyTextMessage(m: msg, nickname: name, msgtype: .PRIV_IM_MYSELF)
            
            messages.append(mymsg)

            if delegate != nil {
                delegate!.appendTextMessage(userid, txtmsg: mymsg)
            }
            updateTableView()
        }
        
        let cmdid = TT_DoTextMessage(ttInst, &msg)
        
        if cmdid > 0 {
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
                    let mymsg = MyTextMessage(m: txtmsg, nickname: name,
                        msgtype: msgtype)
                    messages.append(mymsg)
                    
                    if messages.count > MAX_TEXTMESSAGES {
                        messages.removeFirst()
                    }
                    
                    if tableView != nil {
                        updateTableView()
                    }
                    
                    speakTextMessage(txtmsg.nMsgType, mymsg: mymsg)
            }
        case CLIENTEVENT_CMD_USER_LOGGEDIN :
            
            let user = getUser(&m).pointee
            if showLogMessages() && TT_GetMyUserID(ttInst) == user.nUserID {
                let logmsg = MyTextMessage(logmsg: NSLocalizedString("Logged on to server", comment: "log entry"))
                messages.append(logmsg)
                
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
                messages.append(logmsg!)
                
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
                messages.append(logmsg)
                
                if tableView != nil {
                    updateTableView()
                }
            }
        default : break
        }
    }
    
    func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }
    
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return messages.count
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "Text Msg Cell") as! TextMsgTableCell
        let txtmsg = messages[indexPath.row]
        
        txtmsg.drawCell(cell)
        
        //print("Cell height \(cell.frame.height) txt view height \(cell.messageTextView.frame.height)")
        
        return cell
    }
    
    func tableView(_ tableView: UITableView, heightForRowAt indexPath: IndexPath) -> CGFloat {
        
        let cell = tableView.dequeueReusableCell(withIdentifier: "Text Msg Cell") as! TextMsgTableCell
        let txtmsg = messages[indexPath.row]
        cell.messageTextView.text = txtmsg.message
        
        let fixedWidth = cell.messageTextView.frame.size.width
        let newSize = cell.messageTextView.sizeThatFits(CGSize(width: fixedWidth, height: CGFloat.greatestFiniteMagnitude))
        
        return newSize.height
    }

}
