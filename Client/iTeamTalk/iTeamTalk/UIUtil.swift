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

class UserTableCell : UITableViewCell {
    
    @IBOutlet weak var nicknameLabel: UILabel!
    @IBOutlet weak var userImage: UIImageView!
    @IBOutlet weak var statusmsgLabel: UILabel!
    @IBOutlet weak var messageBtn: UIButton!
}

func newTableCellTextField(_ cell: UITableViewCell, label: String, initial: String) -> UITextField {
    cell.selectionStyle = .none
    cell.textLabel?.text = label
    let textfield = UITextField(frame: CGRect(x: 150, y: 7, width: 150, height: 30))
    textfield.text = initial
    textfield.placeholder = NSLocalizedString("Type text here", comment: "text field hint")
    //textfield.borderStyle = .Line
    cell.accessoryView = textfield
    
    return textfield
}

func newTableCellSlider(_ cell: UITableViewCell, label: String, min: Float, max: Float, initial: Float) -> UISlider {
    cell.selectionStyle = .none
    cell.textLabel?.text = label
    let sliderfield = UISlider(frame: CGRect(x: 150, y: 7, width: 150, height: 31))
    sliderfield.minimumValue = min
    sliderfield.maximumValue = max
    sliderfield.value = initial
    cell.accessoryView = sliderfield
    
    return sliderfield
}

func newTableCellSwitch(_ cell: UITableViewCell, label: String, initial: Bool, tag: Int) -> UISwitch {
    cell.selectionStyle = .none
    cell.textLabel?.text = label
    let switchfield = UISwitch(frame: CGRect.zero)
    switchfield.isOn = initial
    switchfield.tag = tag
    cell.accessoryView = switchfield
    
    return switchfield
}

func newTableCellSwitch(_ cell: UITableViewCell, label: String, initial: Bool) -> UISwitch {
    return newTableCellSwitch(cell, label: label, initial: initial, tag: 0)
}

func newTableCellBtn(_ cell: UITableViewCell, label: String, btntext: String) -> UIButton {
    cell.selectionStyle = .none
    cell.textLabel?.text = label
    let buttonfield = UIButton(frame: CGRect(x: 40, y: 0, width: 150, height: 31))
    buttonfield.setTitle(btntext, for: UIControl.State())
    cell.accessoryView = buttonfield
    
    return buttonfield
}

func newTableCellSegCtrl(_ cell: UITableViewCell, label: String, values: [String]) -> UISegmentedControl {
    cell.selectionStyle = .none
    cell.textLabel?.text = label
    let field = UISegmentedControl(items: values)
    cell.accessoryView = field
    
    return field
}

func newTableCellPicker(_ cell: UITableViewCell, label: String) -> UIPickerView {
    cell.selectionStyle = .none
    cell.textLabel?.text = label
    let picker = UIPickerView()
    cell.accessoryView = picker

    return picker
}

func newTableCellStepper(_ cell: UITableViewCell, label: String,
                         min: Double, max: Double,
                         step: Double, initial: Double) -> UIStepper {

    cell.selectionStyle = .none
    cell.textLabel?.text = label
    let stepper = UIStepper()
    
    stepper.minimumValue = min
    stepper.maximumValue = max
    stepper.stepValue = step
    
    stepper.value = within(min, max_v: max, value: initial)
    cell.accessoryView = stepper
    
    return stepper
}

enum ChanSort : Int {
    case ASCENDING = 0
    case POPULARITY
    case COUNT
}

enum MsgType {
    case PRIV_IM
    case PRIV_IM_MYSELF
    case CHAN_IM
    case CHAN_IM_MYSELF
    case LOGMSG
    case BCAST
}

struct MyTextMessage {
    var nickname = ""
    var message : String
    var date = Date()
    var msgtype : MsgType
    
    init(m: TextMessage, nickname: String, msgtype: MsgType) {
        var m = m
        message = String(cString: getTextMessageString(MESSAGE, &m))
        self.nickname = nickname
        self.msgtype = msgtype
    }
    
    init(logmsg: String) {
        message = logmsg
        msgtype = .LOGMSG
    }

    
    func drawCell(_ cell: TextMsgTableCell) {
        
        let dateFormatter = DateFormatter()
        dateFormatter.locale = Locale.current
        dateFormatter.dateFormat = "HH:mm:ss"
        let time = dateFormatter.string(from: date)
        
        switch msgtype {
        case .PRIV_IM :
            fallthrough
        case .CHAN_IM :
            let source = limitText(nickname)
            cell.authorLabel.text = "\(source), \(time)"
            cell.backgroundColor = UIColor(red: 1.0, green:0.627, blue:0.882, alpha: 1.0)
        case .PRIV_IM_MYSELF :
            fallthrough
        case .CHAN_IM_MYSELF :
            let source = limitText(nickname)
            cell.authorLabel.text = "\(source), \(time)"
            cell.backgroundColor = UIColor(red: 0.54, green: 0.82, blue: 0.94, alpha: 1.0)
            
        case .BCAST :
            let source = limitText(nickname)
            cell.authorLabel.text = "\(source), \(time)"
            cell.backgroundColor = UIColor(red: 0.831, green: 0.376, blue: 1.0, alpha:1.0)
            
        case .LOGMSG :
            cell.backgroundColor = UIColor(red: 0.86, green: 0.86, blue: 0.86, alpha: 1.0)
            cell.authorLabel.text = "\(time)"
        }
        cell.messageTextView.text = message
        
        var hint = ""
        
        switch msgtype {
        case .PRIV_IM :
            fallthrough
        case .PRIV_IM_MYSELF :
            hint = NSLocalizedString("Private message", comment: "text message type")
        case .CHAN_IM :
            fallthrough
        case .CHAN_IM_MYSELF :
            hint = NSLocalizedString("Channel message", comment: "text message type")
        case .BCAST :
            hint = NSLocalizedString("Broadcast message", comment: "text message type")
        case .LOGMSG :
            hint = NSLocalizedString("Log message", comment: "text message type")
        }
        
        cell.accessibilityLabel = message
        cell.accessibilityHint = cell.authorLabel.text! + ". " + hint
        
        //cell.messageTextView.textContainerInset = UIEdgeInsetsZero
        //cell.messageTextView.textContainer.lineFragmentPadding = 0.0
    }
}

protocol MyTextMessageDelegate {
    func appendTextMessage(_ userid: INT32, txtmsg: MyTextMessage)
}

@available(iOS 8.0, *)
class MyCustomAction : UIAccessibilityCustomAction {
    
    var tag = 0
    
    init(name: String, target: AnyObject?, selector: Selector, tag: Int) {
        super.init(name: name, target: target, selector: selector)
        self.tag = tag
    }
}
