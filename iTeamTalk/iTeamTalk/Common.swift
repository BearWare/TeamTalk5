//
//  Common.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 23-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit
import Foundation
import AVFoundation

func refVolume(percent: Double) -> Int {
    //82.832*EXP(0.0508*x) - 50
    if percent == 0 {
        return 0
    }
    
    let d = 82.832 * exp(0.0508 * percent) - 50
    return Int(d)
}

func refVolumeToPercent(volume: Int) -> Int {
    if(volume == 0) {
        return 0
    }
    
    let d = (Double(volume) + 50.0) / 82.832
    let d1 = (log(d) / 0.0508) + 0.5
    return Int(d1)
}

func getDefaultSubscriptions() -> Subscriptions {
    
    let settings = NSUserDefaults.standardUserDefaults()
    
    var sub_usermsg = DEFAULT_SUBSCRIPTION_USERMSG
    if settings.objectForKey(PREF_SUB_USERMSG) != nil {
        sub_usermsg = settings.boolForKey(PREF_SUB_USERMSG)
    }
    var sub_chanmsg = DEFAULT_SUBSCRIPTION_CHANMSG
    if settings.objectForKey(PREF_SUB_CHANMSG) != nil {
        sub_chanmsg = settings.boolForKey(PREF_SUB_CHANMSG)
    }
    var sub_bcastmsg = DEFAULT_SUBSCRIPTION_BCASTMSG
    if settings.objectForKey(PREF_SUB_BROADCAST) != nil {
        sub_bcastmsg = settings.boolForKey(PREF_SUB_BROADCAST)
    }
    var sub_voice = DEFAULT_SUBSCRIPTION_VOICE
    if settings.objectForKey(PREF_SUB_VOICE) != nil {
        sub_voice = settings.boolForKey(PREF_SUB_VOICE)
    }
    var sub_vidcap = DEFAULT_SUBSCRIPTION_VIDEOCAP
    if settings.objectForKey(PREF_SUB_VIDEOCAP) != nil {
        sub_vidcap = settings.boolForKey(PREF_SUB_VIDEOCAP)
    }
    var sub_mediafile = DEFAULT_SUBSCRIPTION_MEDIAFILE
    if settings.objectForKey(PREF_SUB_MEDIAFILE) != nil {
        sub_mediafile = settings.boolForKey(PREF_SUB_MEDIAFILE)
    }
    var sub_desktop = DEFAULT_SUBSCRIPTION_DESKTOP
    if settings.objectForKey(PREF_SUB_DESKTOP) != nil {
        sub_desktop = settings.boolForKey(PREF_SUB_DESKTOP)
    }
    var sub_deskinput = DEFAULT_SUBSCRIPTION_DESKTOPINPUT
    if settings.objectForKey(PREF_SUB_DESKTOPINPUT) != nil {
        sub_deskinput = settings.boolForKey(PREF_SUB_DESKTOPINPUT)
    }
    
    var subs : Subscriptions = SUBSCRIBE_CUSTOM_MSG.rawValue
    if sub_usermsg {
        subs |= SUBSCRIBE_USER_MSG.rawValue
    }
    if sub_chanmsg {
        subs |= SUBSCRIBE_CHANNEL_MSG.rawValue
    }
    if sub_bcastmsg {
        subs |= SUBSCRIBE_BROADCAST_MSG.rawValue
    }
    if sub_voice {
        subs |= SUBSCRIBE_VOICE.rawValue
    }
    if sub_vidcap {
        subs |= SUBSCRIBE_VIDEOCAPTURE.rawValue
    }
    if sub_mediafile {
        subs |= SUBSCRIBE_MEDIAFILE.rawValue
    }
    if sub_desktop {
        subs |= SUBSCRIBE_DESKTOP.rawValue
    }
    if sub_deskinput {
        subs |= SUBSCRIBE_DESKTOPINPUT.rawValue
    }
    
    return subs
}

func newTableCellTextField(cell: UITableViewCell, label: String, initial: String) -> UITextField {
    cell.selectionStyle = .None
    cell.textLabel?.text = label
    let textfield = UITextField(frame: CGRectMake(150, 7, 150, 30))
    textfield.text = initial
    textfield.placeholder = "Type text here"
    //textfield.borderStyle = .Line
    cell.accessoryView = textfield
    
    return textfield
}

func newTableCellSlider(cell: UITableViewCell, label: String, min: Float, max: Float, initial: Float) -> UISlider {
    cell.selectionStyle = .None
    cell.textLabel?.text = label
    let sliderfield = UISlider(frame: CGRectMake(150, 7, 150, 31))
    sliderfield.minimumValue = min
    sliderfield.maximumValue = max
    sliderfield.value = initial
    cell.accessoryView = sliderfield
    
    return sliderfield
}

func newTableCellSwitch(cell: UITableViewCell, label: String, initial: Bool) -> UISwitch {
    cell.selectionStyle = .None
    cell.textLabel?.text = label
    let switchfield = UISwitch(frame: CGRectZero)
    switchfield.on = initial
    cell.accessoryView = switchfield
    
    return switchfield
}

func newTableCellBtn(cell: UITableViewCell, label: String, btntext: String) -> UIButton {
    cell.selectionStyle = .None
    cell.textLabel?.text = label
    let buttonfield = UIButton(frame: CGRectMake(150, 7, 150, 31))
    buttonfield.setTitle(btntext, forState: .Normal)
    cell.accessoryView = buttonfield
    
    return buttonfield
}

func newTableCellSegCtrl(cell: UITableViewCell, label: String, values: [String]) -> UISegmentedControl {
    cell.selectionStyle = .None
    cell.textLabel?.text = label
    let field = UISegmentedControl(items: values)
    cell.accessoryView = field
    
    return field
}

func newTableCellStepper(cell: UITableViewCell, label: String,
                         min: Double, max: Double,
                         step: Double, initial: Double) -> UIStepper {

    cell.selectionStyle = .None
    cell.textLabel?.text = label
    let stepper = UIStepper()
    
    stepper.minimumValue = min
    stepper.maximumValue = max
    stepper.stepValue = step
    
    stepper.value = within(min, max_v: max, value: initial)
    cell.accessoryView = stepper
    
    return stepper
}

protocol TeamTalkEvent : class {
    func handleTTMessage(var m: TTMessage)
}

class TeamTalkEventHandler {
    weak var value : TeamTalkEvent?
    init (value: TeamTalkEvent) {
        self.value = value
    }
}

var ttMessageHandlers = [TeamTalkEventHandler]()

func addToTTMessages(p: TeamTalkEvent) {
    
    for m in ttMessageHandlers {
        if m.value === p {
            return
        }
    }
    
    let new = TeamTalkEventHandler(value: p)
    
    ttMessageHandlers.append(new)
}

func removeFromTTMessages(p: TeamTalkEventHandler) {

    for var i = 0; i < ttMessageHandlers.count; {
        if ttMessageHandlers[i] === p {
            ttMessageHandlers.removeAtIndex(i)
        }
        else {
            ++i
        }
    }
}

enum MsgType {
    case IM, LOGMSG
}

struct MyTextMessage {
    var nickname = ""
    var message : String
    var date = NSDate()
    var msgtype : MsgType
    
    init(var m : TextMessage, nickname: String) {
        message = String.fromCString(&m.szMessage.0)!
        self.nickname = nickname
        msgtype = .IM
    }
    
    init(logmsg: String) {
        message = logmsg
        msgtype = .LOGMSG
    }
    
    func drawCell(cell: TextMsgTableCell) {
        
        let dateFormatter = NSDateFormatter()
        dateFormatter.locale = NSLocale.currentLocale()
        dateFormatter.dateFormat = "HH:mm:ss"
        let time = dateFormatter.stringFromDate(date)
        
        switch msgtype {
        case .IM :
            cell.authorLabel.text = "\(nickname), \(time)"
        case .LOGMSG :
            cell.backgroundColor = UIColor.lightGrayColor()
            cell.authorLabel.text = "\(time)"
        }
        cell.messageTextView.text = message
        cell.messageTextView.textContainerInset = UIEdgeInsetsZero
        //cell.messageTextView.textContainer.lineFragmentPadding = 0.0
    }
}

protocol MyTextMessageDelegate {
    func appendTextMessage(userid: INT32, txtmsg: MyTextMessage)
}

// messages received but no read (blinking)
var unreadmessages = Set<INT32>()

enum Sounds : Int {
    case TX_ON = 1, TX_OFF = 2, CHAN_MSG = 3,
         USER_MSG = 4, SRV_LOST = 5
}

var player : AVAudioPlayer?

func getSoundFile(s: Sounds) -> String? {
    
    let settings = NSUserDefaults.standardUserDefaults()
    
    switch s {
    case .TX_ON:
        if settings.objectForKey(PREF_SNDEVENT_VOICETX) == nil ||
           settings.boolForKey(PREF_SNDEVENT_VOICETX) {
            return "on"
        }
    case .TX_OFF:
        if settings.objectForKey(PREF_SNDEVENT_VOICETX) == nil ||
            settings.boolForKey(PREF_SNDEVENT_VOICETX) {
                return "off"
        }
    case .CHAN_MSG:
        if settings.objectForKey(PREF_SNDEVENT_CHANMSG) == nil ||
            settings.boolForKey(PREF_SNDEVENT_CHANMSG) {
                return "channel_message"
        }
    case .USER_MSG:
        if settings.objectForKey(PREF_SNDEVENT_USERMSG) == nil ||
            settings.boolForKey(PREF_SNDEVENT_USERMSG) {
                return "user_message"
        }
    case .SRV_LOST:
        if settings.objectForKey(PREF_SNDEVENT_SERVERLOST) == nil ||
            settings.boolForKey(PREF_SNDEVENT_SERVERLOST) {
                return "serverlost"
        }
    }

    return nil
}

func enableSpeakerOutput(on: Bool) {
    let session = AVAudioSession.sharedInstance()
    
    do {
        try session.setCategory(AVAudioSessionCategoryPlayAndRecord)
        try session.overrideOutputAudioPort( ( on ? AVAudioSessionPortOverride.Speaker : AVAudioSessionPortOverride.None ) )
        try session.setActive(true)
    }
    catch {
        print("Failed")
    }
}

func playSound(s: Sounds) {
    
    let filename = getSoundFile(s)
    
    if filename == nil {
        return
    }
    
    if let resPath = NSBundle.mainBundle().pathForResource(filename, ofType: "mp3") {
        
        let url = NSURL(fileURLWithPath: resPath)
        
        do {
            player = try AVAudioPlayer(contentsOfURL: url)
            player!.prepareToPlay()
            player!.play()
        }
        catch {
            print("Failed to play")
        }
    }
}

let MAX_TEXTMESSAGES = 100
let DEFAULT_SOUND_VU_MAX = 20 // real max is SOUND_VU_MAX
let VOICEACT_DISABLED : Int = DEFAULT_SOUND_VU_MAX + 1


let DEFAULT_MSEC_PER_PACKET : INT32 = 40

//Default OPUS codec settings
let DEFAULT_OPUS_SAMPLERATE : INT32 = 48000
let DEFAULT_OPUS_CHANNELS : INT32 = 1
let DEFAULT_OPUS_APPLICATION : INT32 = OPUS_APPLICATION_VOIP
let DEFAULT_OPUS_COMPLEXITY : INT32 = 10
let DEFAULT_OPUS_FEC : TTBOOL = 1
let DEFAULT_OPUS_DTX : TTBOOL = 0
let DEFAULT_OPUS_VBR : TTBOOL = 1
let DEFAULT_OPUS_VBRCONSTRAINT : TTBOOL = 0
let DEFAULT_OPUS_BITRATE : INT32 = 32000
let DEFAULT_OPUS_DELAY : INT32 = DEFAULT_MSEC_PER_PACKET

//Default Speex codec settings
let DEFAULT_SPEEX_BANDMODE : INT32 = 1
let DEFAULT_SPEEX_QUALITY : INT32 = 4
let DEFAULT_SPEEX_DELAY : INT32 = DEFAULT_MSEC_PER_PACKET
let DEFAULT_SPEEX_SIMSTEREO : TTBOOL = 0

//Default Speex VBR codec settings
let DEFAULT_SPEEX_VBR_BANDMODE : INT32 = 1
let DEFAULT_SPEEX_VBR_QUALITY : INT32 = 4
let DEFAULT_SPEEX_VBR_BITRATE : INT32 = 0
let DEFAULT_SPEEX_VBR_MAXBITRATE : INT32 = 0
let DEFAULT_SPEEX_VBR_DTX : TTBOOL = 1
let DEFAULT_SPEEX_VBR_DELAY : INT32 = DEFAULT_MSEC_PER_PACKET
let DEFAULT_SPEEX_VBR_SIMSTEREO : TTBOOL = 0

let DEFAULT_SUBSCRIPTION_USERMSG = true
let DEFAULT_SUBSCRIPTION_CHANMSG = true
let DEFAULT_SUBSCRIPTION_BCASTMSG = true
let DEFAULT_SUBSCRIPTION_VOICE = true
let DEFAULT_SUBSCRIPTION_VIDEOCAP = false
let DEFAULT_SUBSCRIPTION_MEDIAFILE = true
let DEFAULT_SUBSCRIPTION_DESKTOP = false
let DEFAULT_SUBSCRIPTION_DESKTOPINPUT = false

func within<T : Comparable>(min_v: T, max_v: T, value: T) -> T {
    if value < min_v {
        return min_v
    }
    if value > max_v {
        return max_v
    }
    return value
}
