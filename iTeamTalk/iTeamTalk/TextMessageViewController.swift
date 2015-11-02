//
//  TextMessageViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 11-10-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class TextMessageViewController : UIViewController, UITableViewDataSource, UITableViewDelegate, TeamTalkEvent {
    
    @IBOutlet weak var tableView: UITableView!
    @IBOutlet weak var scrollView: UIScrollView!
    @IBOutlet weak var msgTextView: UITextView!
    
    //shared TTInstance between all view controllers
    var ttInst = UnsafeMutablePointer<Void>()
    var userid = 0
    
    var items = [TextMsgTableCell]()
    
    //var messages : [TextMessage] = []
    
    override func viewDidLoad() {
        super.viewDidLoad()

        let def = NSNotificationCenter.defaultCenter()
        
        def.addObserver(self, selector: "keyboardAppeared:", name: UIKeyboardDidShowNotification, object: nil)
        def.addObserver(self, selector: "keyboardDisappered:", name: UIKeyboardWillHideNotification, object: nil)
        
        tableView.dataSource = self
        tableView.delegate = self
        
        addToTTMessages(self)
    }
    
    override func viewWillDisappear(animated: Bool) {

        if isClosing(self) {
            removeFromTTMessages(self)
        }

        super.viewWillDisappear(animated)
    }
    
    func keyboardAppeared(notify: NSNotification) {
        print("Keyboard appeared")
        
        if let userInfo = notify.userInfo {
            if let keyboardHeight = userInfo[UIKeyboardFrameEndUserInfoKey]?.CGRectValue.size.height {
                
                let contentInsets = UIEdgeInsetsMake(0.0, 0.0, keyboardHeight, 0.0)
                
                scrollView.contentInset = contentInsets
                scrollView.scrollIndicatorInsets = contentInsets
                
                let rect = self.view.frame
//                rect.size.height -= 620 //keyboardHeight
                
                if !CGRectContainsPoint(rect, msgTextView.frame.origin) {
                    scrollView.scrollRectToVisible(msgTextView.frame, animated: true)
                }
            }
        }
    }
    
    func keyboardDisappered(notify: NSNotification) {
        print("Keyboard disappered")

        scrollView.contentInset = UIEdgeInsetsZero
        scrollView.scrollIndicatorInsets = UIEdgeInsetsZero

    }
    
    func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 1
    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return items.count
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        
//        let fixedWidth = cell.messageTextView.frame.size.width
//        let oldSize = cell.messageTextView.frame.size
//        let newSize = cell.messageTextView.sizeThatFits(CGSize(width: fixedWidth, height: CGFloat.max))
//        var newFrame = cell.messageTextView.frame
//        newFrame.size = CGSize(width: max(newSize.width, fixedWidth), height: newSize.height)
//        cell.messageTextView.frame = newFrame;
        return items[indexPath.row]
    }
    
//    func tableView(tableView: UITableView, heightForRowAtIndexPath indexPath: NSIndexPath) -> CGFloat {
//        let f = UIFont.systemFontOfSize(14)
//        let constraintSize = CGSizeMake(self.view.frame.width, CGFloat.max)
//
//        let cell = items[indexPath.row]
//        
////        let fixedWidth = cell.messageTextView.frame.size.width
////        let oldSize = cell.messageTextView.frame.size
////        let newSize = cell.messageTextView.sizeThatFits(CGSize(width: fixedWidth, height: CGFloat.max))
////        
////        println("Width is now \(fixedWidth), height increase \(newSize.height - oldSize.height)")
////        
////        return cell.frame.height + newSize.height - oldSize.height
//        
//        return cell.frame.height
//    }
    
    func handleTTMessage(var m: TTMessage) {
        
        switch m.nClientEvent {
        case CLIENTEVENT_CMD_USER_TEXTMSG :
            
            var txtmsg = getTextMessage(&m).memory
            if txtmsg.nMsgType.rawValue == MSGTYPE_USER.rawValue {
                var user = User()
                TT_GetUser(ttInst, txtmsg.nFromUserID, &user)
                let cell = tableView.dequeueReusableCellWithIdentifier("Text Msg Cell") as! TextMsgTableCell
                cell.authorLabel.text = String.fromCString(&user.szNickname.0)
                cell.messageTextView.text = String.fromCString(&txtmsg.szMessage.0)
                items.append(cell)
                
                tableView.reloadData()
            }
        default : break
        }
    }
}
