//
//  ChannelChatViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 2-11-15.
//  Copyright © 2015 BearWare.dk. All rights reserved.
//

import UIKit

class ChannelChatViewController : UIViewController, UITableViewDataSource, UITableViewDelegate {

    @IBOutlet weak var textmsgTextView: UITextView!
    
    @IBOutlet weak var sendButton: UIButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
//        let def = NSNotificationCenter.defaultCenter()
//
//        def.addObserver(self, selector: "keyboardAppeared:", name: UIKeyboardDidShowNotification, object: nil)
//        def.addObserver(self, selector: "keyboardDisappered:", name: UIKeyboardWillHideNotification, object: nil)

    }
    
    func keyboardAppeared(notification: NSNotification) {

    }
    
    func keyboardDisappered(notification: NSNotification) {
        
    }

    
    func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 1
    }
    
//    func tableView(tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
//        
//    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return 0
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        return UITableViewCell()
    }
}
