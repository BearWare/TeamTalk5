//
//  TextMessageViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 11-10-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class TextMessageViewController :
    UIViewController, UITableViewDataSource,
    UITableViewDelegate, TeamTalkEvent,
    UITextViewDelegate {
    
    @IBOutlet weak var tableView: UITableView!
    @IBOutlet weak var msgTextView: UITextView!
    @IBOutlet weak var sendButton: UIButton!
    
    //shared TTInstance between all view controllers
    var ttInst = UnsafeMutablePointer<Void>()
    var userid : INT32 = 0
    
    var delegate : MyTextMessageDelegate?

    let initial_text = "Type text here"

    var messages : [MyTextMessage] = []
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let def = NSNotificationCenter.defaultCenter()
        def.addObserver(self, selector: "keyboardWillShow:", name: UIKeyboardWillShowNotification, object: nil)
        def.addObserver(self, selector: "keyboardWillHide:", name: UIKeyboardWillHideNotification, object: nil)
        
        resetText()
        
        tableView.dataSource = self
        tableView.delegate = self
        
        msgTextView.delegate = self
        
        let swipe = UISwipeGestureRecognizer(target: self, action: "dismissKeyboard")
        swipe.direction = .Down
        self.view.addGestureRecognizer(swipe)
    }

    override func viewDidDisappear(animated: Bool) {
        super.viewDidDisappear(animated)
        
        if self.isMovingFromParentViewController() {
            unreadmessages.remove(userid)
        }
    }

    
    func dismissKeyboard() {
        if msgTextView.isFirstResponder() {
           msgTextView.resignFirstResponder()
        }
    }
    
    func resetText() {
        msgTextView.text = initial_text
        msgTextView.textColor = UIColor.lightGrayColor()
    }
    
    func updateTableView() {
        
        tableView.reloadData()
        
        if tableView.numberOfRowsInSection(0) > 0 {
//            let ip = NSIndexPath(forRow: tableView.numberOfRowsInSection(0)-1, inSection: 0)
//            tableView.scrollToRowAtIndexPath(ip, atScrollPosition: .Top, animated: true)
//            let frame = tableView.frame
//            let content = tableView.contentSize
//            tableView.setContentOffset(CGPointMake(0, rect.height - frame.height), animated: true)
//            if content.height > frame.height {
                let ip = NSIndexPath(forRow: tableView.numberOfRowsInSection(0)-1, inSection: 0)
                tableView.scrollToRowAtIndexPath(ip, atScrollPosition: .Bottom, animated: true)
//            }
        }
    }
    
    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)
    }
    
    func keyboardWillShow(notify: NSNotification) {
        moveForKeyboard(notify, up: true)
    }
    
    func keyboardWillHide(notify: NSNotification) {
        moveForKeyboard(notify, up: false)
    }
    
    func moveForKeyboard(notify: NSNotification, up: Bool) {
        if let userInfo = notify.userInfo {
            if let keyboardFrame = userInfo[UIKeyboardFrameEndUserInfoKey]?.CGRectValue {
                
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
    
    @IBAction func sendTextMessage(sender: UIButton) {
        
        var msg = TextMessage()
        msg.nFromUserID = TT_GetMyUserID(ttInst)
        toTTString(msgTextView.text, &msg.szMessage.0)
        
        if userid == 0 {
            msg.nMsgType = MSGTYPE_CHANNEL
            msg.nChannelID = TT_GetMyChannelID(ttInst)
        }
        else {
            msg.nMsgType = MSGTYPE_USER
            msg.nToUserID = userid
            
            var user = User()
            TT_GetUser(ttInst, msg.nFromUserID, &user)
            let name = String.fromCString(&user.szNickname.0)!
            let mymsg = MyTextMessage(m: msg, nickname: name)
            
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
    
    func textViewShouldBeginEditing(textView: UITextView) -> Bool {
        
        if msgTextView.text == initial_text {
            msgTextView.text = ""
        }
        msgTextView.textColor = UIColor.darkTextColor()
        
        return true
    }
    
    func textViewDidEndEditing(textView: UITextView) {
        if msgTextView.text.isEmpty {
            resetText()
        }
    }
    
    func handleTTMessage(var m: TTMessage) {

        switch m.nClientEvent {
            
        case CLIENTEVENT_CMD_USER_TEXTMSG :
            
            let txtmsg = getTextMessage(&m).memory
            
            if (txtmsg.nMsgType == MSGTYPE_USER && txtmsg.nFromUserID == userid) || txtmsg.nMsgType == MSGTYPE_CHANNEL {
                
                var user = User()
                TT_GetUser(ttInst, txtmsg.nFromUserID, &user)
                
                let mymsg = MyTextMessage(m: txtmsg, nickname: String.fromCString(&user.szNickname.0)!)
                messages.append(mymsg)
                
                if messages.count > MAX_TEXTMESSAGES {
                    messages.removeFirst()
                }
                
                if tableView != nil {
                    updateTableView()
                }
            }
        case CLIENTEVENT_CMD_USER_LOGGEDIN :
            
            let user = getUser(&m).memory
            if TT_GetMyUserID(ttInst) == user.nUserID {
                let logmsg = MyTextMessage(logmsg: "Logged on to server")
                messages.append(logmsg)
                
                if tableView != nil {
                    updateTableView()
                }
            }
        case CLIENTEVENT_CMD_USER_JOINED :
            
            var user = getUser(&m).memory
            if TT_GetMyChannelID(ttInst) == user.nChannelID {
                var logmsg : MyTextMessage?
                if TT_GetMyUserID(ttInst) == user.nUserID {
                    var channel = Channel()
                    TT_GetChannel(ttInst, user.nChannelID, &channel)
                    var channame : String
                    if channel.nParentID == 0 {
                        channame = "root channel"
                    }
                    else {
                        channame = String.fromCString(&channel.szName.0)!
                    }
                    logmsg = MyTextMessage(logmsg: "Joined \(channame)")
                }
                else {
                    let nickname = String.fromCString(&user.szNickname.0)!
                    logmsg = MyTextMessage(logmsg: "\(nickname) joined channel")
                }
                messages.append(logmsg!)
                
                if tableView != nil {
                    updateTableView()
                }
            }
        case CLIENTEVENT_CMD_USER_LEFT :
            
            var user = getUser(&m).memory
            if TT_GetMyChannelID(ttInst) == m.nSource {
                let nickname = String.fromCString(&user.szNickname.0)!
                let logmsg = MyTextMessage(logmsg: "\(nickname) left channel")
                messages.append(logmsg)
                
                if tableView != nil {
                    updateTableView()
                }
            }
        default : break
        }
    }
    
    func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 1
    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return messages.count
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCellWithIdentifier("Text Msg Cell") as! TextMsgTableCell
        let txtmsg = messages[indexPath.row]
        
        txtmsg.drawCell(cell)
        
        //print("Cell height \(cell.frame.height) txt view height \(cell.messageTextView.frame.height)")
        
        return cell
    }
    
    func tableView(tableView: UITableView, heightForRowAtIndexPath indexPath: NSIndexPath) -> CGFloat {
        
        let cell = tableView.dequeueReusableCellWithIdentifier("Text Msg Cell") as! TextMsgTableCell
        let txtmsg = messages[indexPath.row]
        cell.messageTextView.text = txtmsg.message
        
        let fixedWidth = cell.messageTextView.frame.size.width
        let newSize = cell.messageTextView.sizeThatFits(CGSize(width: fixedWidth, height: CGFloat.max))
        
        return newSize.height
    }

}
