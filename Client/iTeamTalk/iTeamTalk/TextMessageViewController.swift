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

    struct MessageSection {
        var fromuserid: INT32
        var nickname: String
        var msgtype: MsgType
        var messages: [MyTextMessage]
    }
    // Ordered list of message sections.
    private var sections: [MessageSection] = []
    private var totalMessageCount = 0 // real total (across all sections)
    private var newMessagesButton: UIButton!
    
    func generateKey(_ msg: TextMessage) -> Int {
        return (Int(msg.nMsgType.rawValue) << 16) | Int(msg.nFromUserID)
    }
    
    // For multi-part messages (bMore). Each key accumulates fragments until final part.
    var mergemessages = [Int : [TextMessage] ]()
    private var mergeTimestamps = [Int : Date]() // to purge stale fragment chains
    private let mergeFragmentStaleInterval: TimeInterval = 120 // seconds
    
    func getTextMessageContent(_ msg: TextMessage) -> String? {
        var msg = msg
        let key = generateKey(msg)

        // purge stale fragment chains
        let now = Date()
        for (k, ts) in mergeTimestamps where now.timeIntervalSince(ts) > mergeFragmentStaleInterval {
            mergemessages.removeValue(forKey: k)
            mergeTimestamps.removeValue(forKey: k)
        }

        if msg.bMore == TRUE {
            if mergemessages[key] == nil {
                mergemessages[key] = []
            }
            mergemessages[key]!.append(msg)
            mergeTimestamps[key] = now
            // safety: if a single chain grows absurdly large, discard it
            if mergemessages[key]!.count > 1000 {
                mergemessages.removeValue(forKey: key)
                mergeTimestamps.removeValue(forKey: key)
            }
            return nil
        } else if let fragments = mergemessages[key] {
            var content = fragments.reduce(into: "") { acc, part in
                var part = part
                acc += String(cString: getTextMessageString(MESSAGE, &part))
            }
            mergemessages.removeValue(forKey: key)
            mergeTimestamps.removeValue(forKey: key)
            content += String(cString: getTextMessageString(MESSAGE, &msg))
            return content
        } else {
            return String(cString: getTextMessageString(MESSAGE, &msg))
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let def = NotificationCenter.default
        def.addObserver(self, selector: #selector(TextMessageViewController.keyboardWillShow(_:)), name: UIResponder.keyboardWillShowNotification, object: nil)
        def.addObserver(self, selector: #selector(TextMessageViewController.keyboardWillHide(_:)), name: UIResponder.keyboardWillHideNotification, object: nil)
        
        resetText()
        
        tableView.dataSource = self
        tableView.delegate = self
        tableView.estimatedRowHeight = 60
        tableView.rowHeight = UITableView.automaticDimension

        // New messages button (appears when user is not at bottom)
        newMessagesButton = UIButton(type: .system)
        newMessagesButton.setTitle(NSLocalizedString("New Messages", comment: "chat"), for: .normal)
        newMessagesButton.backgroundColor = UIColor.systemBlue.withAlphaComponent(0.85)
        newMessagesButton.setTitleColor(.white, for: .normal)
        newMessagesButton.titleLabel?.font = UIFont.boldSystemFont(ofSize: 14)
        newMessagesButton.layer.cornerRadius = 14
        newMessagesButton.contentEdgeInsets = UIEdgeInsets(top: 4, left: 12, bottom: 4, right: 12)
        newMessagesButton.alpha = 0
        newMessagesButton.addTarget(self, action: #selector(tapNewMessagesButton), for: .touchUpInside)
        newMessagesButton.translatesAutoresizingMaskIntoConstraints = false
        view.addSubview(newMessagesButton)
        NSLayoutConstraint.activate([
            newMessagesButton.bottomAnchor.constraint(equalTo: msgTextView.topAnchor, constant: -8),
            newMessagesButton.centerXAnchor.constraint(equalTo: view.centerXAnchor)
        ])
        
        msgTextView.delegate = self
        
        msgTextView.accessibilityLabel = NSLocalizedString("Send empty message to close keyboard", comment: "text message")
    }

    override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)
        
        if self.isMovingFromParent {
            unreadmessages.remove(userid)
        }
    }

    func showLogMessages() -> Bool { userid == 0 }
    
    @discardableResult
    func appendEventMessage(_ m : MyTextMessage) -> (newSection: Bool, indexPath: IndexPath?) {
        var isNewSection = false
        if let last = sections.last, last.fromuserid == m.fromuserid && last.nickname == m.nickname && last.msgtype == m.msgtype {
            // append to existing
            sections[sections.count - 1].messages.append(m)
        } else {
            // new section
            let newSec = MessageSection(fromuserid: m.fromuserid, nickname: m.nickname, msgtype: m.msgtype, messages: [m])
            sections.append(newSec)
            isNewSection = true
        }
        totalMessageCount += 1
        if totalMessageCount > MAX_TEXTMESSAGES { pruneOldestMessages() }
        guard tableView != nil else { return (isNewSection, nil) }
        let secIndex = sections.count - 1
        let row = sections[secIndex].messages.count - 1
        return (isNewSection, IndexPath(row: row, section: secIndex))
    }

    private func pruneOldestMessages() {
        while totalMessageCount > MAX_TEXTMESSAGES && !sections.isEmpty {
            if sections[0].messages.isEmpty {
                sections.removeFirst()
                continue
            }
            sections[0].messages.removeFirst()
            totalMessageCount -= 1
            if sections[0].messages.isEmpty { sections.removeFirst() }
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
    
    private func isAtBottom() -> Bool {
        guard let visible = tableView.indexPathsForVisibleRows, !visible.isEmpty else { return true }
        let lastSection = sections.count - 1
        guard lastSection >= 0 else { return true }
        let lastRow = sections[lastSection].messages.count - 1
        if lastRow < 0 { return true }
        return visible.contains { $0.section == lastSection && $0.row == lastRow }
    }

    private func scrollToBottom(animated: Bool) {
        let lastSection = sections.count - 1
        guard lastSection >= 0 else { return }
        let lastRow = sections[lastSection].messages.count - 1
        guard lastRow >= 0 else { return }
        let ip = IndexPath(row: lastRow, section: lastSection)
        tableView.scrollToRow(at: ip, at: .bottom, animated: animated)
    }

    func updateTableViewFullReload() { // fallback (should rarely be needed)
        tableView.reloadData()
        scrollToBottom(animated: true)
    }

    func updateTableViewIncremental(newSection: Bool, indexPath: IndexPath?) {
        guard let indexPath = indexPath else { return }
        let shouldScroll = isAtBottom()
        tableView.performBatchUpdates({
            if newSection {
                tableView.insertSections(IndexSet(integer: indexPath.section), with: .none)
            } else {
                tableView.insertRows(at: [indexPath], with: .none)
            }
        }, completion: { _ in
            if shouldScroll {
                self.scrollToBottom(animated: true)
                UIView.animate(withDuration: 0.2) { self.newMessagesButton.alpha = 0 }
            } else {
                UIView.animate(withDuration: 0.25) { self.newMessagesButton.alpha = 1 }
            }
        })
    }

    @objc private func tapNewMessagesButton() {
        scrollToBottom(animated: true)
        UIView.animate(withDuration: 0.2) { self.newMessagesButton.alpha = 0 }
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
                
                updateTableViewFullReload() // keyboard shift: keep simple full reload
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

            let res = appendEventMessage(mymsg)
            delegate?.appendTextMessage(userid, txtmsg: mymsg)
            updateTableViewIncremental(newSection: res.newSection, indexPath: res.indexPath)
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
                    let res = appendEventMessage(mymsg)
                    if tableView != nil { updateTableViewIncremental(newSection: res.newSection, indexPath: res.indexPath) }
                    
                    speakTextMessage(txtmsg.nMsgType, mymsg: mymsg)
                }
            }
        case CLIENTEVENT_CMD_USER_LOGGEDIN :
            
            let user = getUser(&m).pointee
            if showLogMessages() && TT_GetMyUserID(ttInst) == user.nUserID {
                let logmsg = MyTextMessage(logmsg: NSLocalizedString("Logged on to server", comment: "log entry"))
                let res = appendEventMessage(logmsg)
                if tableView != nil { updateTableViewIncremental(newSection: res.newSection, indexPath: res.indexPath) }
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
                
                let res = appendEventMessage(logmsg!)
                if tableView != nil { updateTableViewIncremental(newSection: res.newSection, indexPath: res.indexPath) }
            }
        case CLIENTEVENT_CMD_USER_LEFT :
            
            let user = getUser(&m).pointee
            if showLogMessages() && TT_GetMyChannelID(ttInst) == m.nSource {
                let name = getDisplayName(user)
                let txt = String(format: NSLocalizedString("%@ left channel", comment: "log entry"), name)
                let logmsg = MyTextMessage(logmsg: txt)
                let res = appendEventMessage(logmsg)
                if tableView != nil { updateTableViewIncremental(newSection: res.newSection, indexPath: res.indexPath) }
            }
        case CLIENTEVENT_CMD_ERROR :
            
            let errmsg = getClientErrorMsg(m.clienterrormsg, strprop: ERRMESSAGE)
            let txt = String(format: NSLocalizedString("Command failed: %@", comment: "log entry"),  errmsg)
            let logmsg = MyTextMessage(logmsg: txt)
            let res = appendEventMessage(logmsg)
            if tableView != nil { updateTableViewIncremental(newSection: res.newSection, indexPath: res.indexPath) }
        default : break
        }
    }
    
    func numberOfSections(in tableView: UITableView) -> Int { sections.count }
    
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return sections[section].messages.count
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {

        let cell = tableView.dequeueReusableCell(withIdentifier: "Text Msg Cell") as! TextMsgTableCell
        getEventMessage(indexPath).drawCell(cell)
        return cell
    }
    
    // Using automatic dimension – no manual height implementation required.

    func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        let em = sections[section].messages.first!
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
