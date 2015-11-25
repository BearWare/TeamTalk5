//
//  TextMessageViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 11-10-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class TextMessageViewController : UIViewController, UITableViewDataSource, UITableViewDelegate, TeamTalkEvent, UITextViewDelegate {
    
    @IBOutlet weak var tableView: UITableView!
    @IBOutlet weak var msgTextView: UITextView!
    @IBOutlet weak var sendButton: UIButton!
    
    //shared TTInstance between all view controllers
    var ttInst = UnsafeMutablePointer<Void>()
    var userid = 0

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
        
        addToTTMessages(self)
        
    }
    
    func resetText() {
        msgTextView.text = initial_text
        msgTextView.textColor = UIColor.lightGrayColor()
    }
    
    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)
        
        print("view size W \(self.view.frame.width) \(self.view.frame.height)")
        print("Table height \(self.tableView.frame.height)")
        print("View bounds W \(self.view.bounds.width) \(self.view.bounds.height)")
        let h = CGRectGetHeight(self.view.bounds)
        print("View height \(h)")
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
            }
        }
    }
    
    @IBAction func sendTextMessage(sender: UIButton) {
        var msg = TextMessage()
        
        msg.nChannelID = TT_GetMyChannelID(ttInst)
        msg.nMsgType = MSGTYPE_CHANNEL
        toTTString(msgTextView.text, &msg.szMessage.0)
        
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
            if txtmsg.nMsgType == MSGTYPE_USER && txtmsg.nFromUserID == INT32(userid) {
                
                var user = User()
                TT_GetUser(ttInst, txtmsg.nFromUserID, &user)
                
                let mymsg = MyTextMessage(m: txtmsg, nickname: String.fromCString(&user.szNickname.0)!)
                messages.append(mymsg)
                
                tableView.reloadData()
                
                let ip = NSIndexPath(forRow: tableView.numberOfRowsInSection(0)-1, inSection: 0)
                tableView.scrollToRowAtIndexPath(ip, atScrollPosition: .Top, animated: true)
                
                print("Scrolled to row \(ip.row)")
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
        let currentDate = NSDate()
        let dateFormatter = NSDateFormatter()
        dateFormatter.locale = NSLocale.currentLocale()
        dateFormatter.dateFormat = "HH:mm:ss"
        let time = dateFormatter.stringFromDate(currentDate)
        
        cell.authorLabel.text = "\(txtmsg.nickname), \(time)"
        cell.messageTextView.text = txtmsg.message
        cell.messageTextView.textContainerInset = UIEdgeInsetsZero
        //cell.messageTextView.textContainer.lineFragmentPadding = 0.0
        
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
