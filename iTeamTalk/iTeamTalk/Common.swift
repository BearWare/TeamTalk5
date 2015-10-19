//
//  Common.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 23-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import Foundation
import UIKit

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

func newTableCellTextField(cell: UITableViewCell, label: String, initial: String) -> UITextField {
    cell.selectionStyle = .None
    cell.textLabel?.text = label
    let textfield = UITextField(frame: CGRectMake(150, 7, 150, 30))
    textfield.text = initial
    textfield.placeholder = "Type text here"
    textfield.borderStyle = .Line
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
    
    stepper.value = within(min, max, initial)
    cell.accessoryView = stepper
    
    return stepper
}

protocol TeamTalkEvent : class {
    func handleTTMessage(var m: TTMessage)
}

var ttMessageHandlers = [TeamTalkEvent]()

func addToTTMessages(p: TeamTalkEvent) {
    for m in ttMessageHandlers {
        if m === p {
            return
        }
    }
    ttMessageHandlers.append(p)
}

func removeFromTTMessages(p: TeamTalkEvent) {

    for var i = 0; i < ttMessageHandlers.count; {
        if ttMessageHandlers[i] === p {
            ttMessageHandlers.removeAtIndex(i)
        }
        else {
            ++i
        }
    }
}

func isClosing(vc: UIViewController) -> Bool {
    let s : NSArray = vc.navigationController!.viewControllers
    
    return s.indexOfObject(vc) == NSNotFound
}

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

func within<T : Comparable>(min_v: T, max_v: T, value: T) -> T {
    if value < min_v {
        return min_v
    }
    if value > max_v {
        return max_v
    }
    return value
}
