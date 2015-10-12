//
//  TextMessageViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 11-10-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class TextMessageViewController : UIViewController, UITableViewDataSource, UITableViewDelegate {
    
    @IBOutlet weak var tableView: UITableView!
    
    var items = [UITableViewCell]()
    
    override func viewDidLoad() {
        super.viewDidLoad()

        let cell = UITableViewCell(style: .Default, reuseIdentifier: "onwrow")
        cell.selectionStyle = .None
        cell.textLabel?.text = "The line of test"
        
//        let textfield = UITextField(frame: CGRectZero)
//        cell.accessoryView = textfield
//        textfield.text = "FOOOO"
        
        let uiswitch = UISwitch(frame: CGRectZero)
        cell.accessoryView = uiswitch
        uiswitch.setOn(false, animated:false)
        
        items.append(cell)
        
        let cell2 = UITableViewCell(style: .Subtitle, reuseIdentifier: "tworow")
        cell2.selectionStyle = .None
        cell2.textLabel?.text = "Svend"
        cell2.detailTextLabel?.text = "Here's some smaller text"
        
        items.append(cell2)
        
        tableView.dataSource = self
        tableView.delegate = self
        
        
    }
    
    func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 2
    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        if section == 0 {
            return 1
        }
        return items.count
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        
        if indexPath.section == 0 {
            let cell = UITableViewCell(style: .Default, reuseIdentifier: "section0")
            cell.selectionStyle = .None
            cell.textLabel?.text = "My category"
            return cell
        }
        
        let prevCell = tableView.dequeueReusableCellWithIdentifier("switchcell")
        if prevCell == nil {
            return items[indexPath.row]
        }
        
        return prevCell as! UITableViewCell
    }
}
