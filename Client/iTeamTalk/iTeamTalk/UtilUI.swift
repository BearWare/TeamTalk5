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
    var fromuserid : INT32 = 0
    
    init(m: TextMessage, nickname: String, msgtype: MsgType) {
        var m = m
        message = String(cString: getTextMessageString(MESSAGE, &m))
        self.nickname = nickname
        self.msgtype = msgtype
        self.fromuserid = m.nFromUserID
    }

    init(fromuserid: INT32, nickname: String, msgtype: MsgType, content: String) {
        self.fromuserid = fromuserid
        self.message = content
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

func hasPTTLock() -> Bool {
    let defaults = UserDefaults.standard
    return defaults.object(forKey: PREF_GENERAL_PTTLOCK) != nil && defaults.bool(forKey: PREF_GENERAL_PTTLOCK)
}

func limitText(_ s: String) -> String {
    
    let settings = UserDefaults.standard
    let length = settings.object(forKey: PREF_DISPLAY_LIMITTEXT) == nil ? DEFAULT_LIMIT_TEXT : settings.integer(forKey: PREF_DISPLAY_LIMITTEXT)
    
    if s.count > length {
        return String(s.prefix(length))
    }
    return s
}

func getDisplayName(_ user: User) -> String {
    var user = user
    let settings = UserDefaults.standard
    if settings.object(forKey: PREF_DISPLAY_SHOWUSERNAME) != nil && settings.bool(forKey: PREF_DISPLAY_SHOWUSERNAME) {
        return limitText(String(cString: getUserString(USERNAME, &user)))
    }

    let nickname = getUser(user, strprop: NICKNAME)
    if nickname.isEmpty {
        return DEFAULT_NICKNAME + " - #\(user.nUserID)"
    }
    
    return limitText(nickname)
}

func buildTextMessages(_ msg: TextMessage, content: String) -> [TextMessage] {
    
    var result = [TextMessage]()
    
    var newmsg = msg;
    
    if content.lengthOfBytes(using: .utf8) <= TT_STRLEN - 1 {
        toTTString(content, dst: &newmsg.szMessage)
        newmsg.bMore = FALSE
        result.append(newmsg)
        return result
    }
    
    newmsg.bMore = TRUE
    var curlen = content.count
    while content.prefix(curlen).lengthOfBytes(using: .utf8) > TT_STRLEN - 1 {
        curlen /= 2
    }

    var half = Int(TT_STRLEN) / 2;
    while half > 0 {
        let utf8len = content.prefix(curlen + half).lengthOfBytes(using: .utf8)
        if utf8len <= TT_STRLEN - 1 {
            curlen += half;
        }
        if utf8len == TT_STRLEN - 1 {
            break;
        }
        half /= 2;
    }
    
    toTTString(String(content.prefix(curlen)), dst: &newmsg.szMessage)

    result.append(newmsg);
    let remain = content.count - curlen
    let messages = buildTextMessages(newmsg, content: String(content.suffix(remain)))
    for m in messages {
        result.append(m)
    }

    return result
}

func sendTextMessage(msg: TextMessage, content: String) -> Bool {
    var sent = true
    for m in buildTextMessages(msg, content: content) {
        var m = m
        sent = sent && TT_DoTextMessage(ttInst, &m) > 0
    }
    return sent
}
