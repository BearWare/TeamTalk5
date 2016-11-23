/*
* Copyright (c) 2005-2016, BearWare.dk
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
* This source code is part of the TeamTalk 5 SDK owned by
* BearWare.dk. All copyright statements may not be removed
* or altered from any source distribution. If you use this
* software in a product, an acknowledgment in the product
* documentation is required.
*
*/

import UIKit
import Foundation
import AVFoundation

func refVolume(_ percent: Double) -> Int {
    //82.832*EXP(0.0508*x) - 50
    if percent == 0 {
        return 0
    }
    
    let d = 82.832 * exp(0.0508 * percent) - 50
    return Int(d)
}

func refVolumeToPercent(_ volume: Int) -> Int {
    if(volume == 0) {
        return 0
    }
    
    let d = (Double(volume) + 50.0) / 82.832
    let d1 = (log(d) / 0.0508) + 0.5
    return Int(d1)
}

func getDefaultSubscriptions() -> Subscriptions {
    
    let settings = UserDefaults.standard
    
    var sub_usermsg = DEFAULT_SUBSCRIPTION_USERMSG
    if settings.object(forKey: PREF_SUB_USERMSG) != nil {
        sub_usermsg = settings.bool(forKey: PREF_SUB_USERMSG)
    }
    var sub_chanmsg = DEFAULT_SUBSCRIPTION_CHANMSG
    if settings.object(forKey: PREF_SUB_CHANMSG) != nil {
        sub_chanmsg = settings.bool(forKey: PREF_SUB_CHANMSG)
    }
    var sub_bcastmsg = DEFAULT_SUBSCRIPTION_BCASTMSG
    if settings.object(forKey: PREF_SUB_BROADCAST) != nil {
        sub_bcastmsg = settings.bool(forKey: PREF_SUB_BROADCAST)
    }
    var sub_voice = DEFAULT_SUBSCRIPTION_VOICE
    if settings.object(forKey: PREF_SUB_VOICE) != nil {
        sub_voice = settings.bool(forKey: PREF_SUB_VOICE)
    }
    var sub_vidcap = DEFAULT_SUBSCRIPTION_VIDEOCAP
    if settings.object(forKey: PREF_SUB_VIDEOCAP) != nil {
        sub_vidcap = settings.bool(forKey: PREF_SUB_VIDEOCAP)
    }
    var sub_mediafile = DEFAULT_SUBSCRIPTION_MEDIAFILE
    if settings.object(forKey: PREF_SUB_MEDIAFILE) != nil {
        sub_mediafile = settings.bool(forKey: PREF_SUB_MEDIAFILE)
    }
    var sub_desktop = DEFAULT_SUBSCRIPTION_DESKTOP
    if settings.object(forKey: PREF_SUB_DESKTOP) != nil {
        sub_desktop = settings.bool(forKey: PREF_SUB_DESKTOP)
    }
    var sub_deskinput = DEFAULT_SUBSCRIPTION_DESKTOPINPUT
    if settings.object(forKey: PREF_SUB_DESKTOPINPUT) != nil {
        sub_deskinput = settings.bool(forKey: PREF_SUB_DESKTOPINPUT)
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

func newTableCellSwitch(_ cell: UITableViewCell, label: String, initial: Bool) -> UISwitch {
    cell.selectionStyle = .none
    cell.textLabel?.text = label
    let switchfield = UISwitch(frame: CGRect.zero)
    switchfield.isOn = initial
    cell.accessoryView = switchfield
    
    return switchfield
}

func newTableCellBtn(_ cell: UITableViewCell, label: String, btntext: String) -> UIButton {
    cell.selectionStyle = .none
    cell.textLabel?.text = label
    let buttonfield = UIButton(frame: CGRect(x: 40, y: 0, width: 150, height: 31))
    buttonfield.setTitle(btntext, for: UIControlState())
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

protocol TeamTalkEvent : class {
    func handleTTMessage(_ m: TTMessage)
}

class TeamTalkEventHandler {
    weak var value : TeamTalkEvent?
    init (value: TeamTalkEvent) {
        self.value = value
    }
}

//shared TTInstance between all view controllers
var ttInst : UnsafeMutableRawPointer? = nil

var ttMessageHandlers = [TeamTalkEventHandler]()

func addToTTMessages(_ p: TeamTalkEvent) {
    
    for m in ttMessageHandlers {
        if m.value === p {
            return
        }
    }
    
    let new = TeamTalkEventHandler(value: p)
    
    ttMessageHandlers.append(new)
}

func removeFromTTMessages(_ p: TeamTalkEventHandler) {

    var i = 0
    
    for m in ttMessageHandlers {
        if m === p {
            ttMessageHandlers.remove(at: i)
            break
        }
        else {
            i += 1
        }
    }
}

enum MsgType {
    case im
    case im_MYSELF
    case logmsg
    case bcast
}

struct MyTextMessage {
    var nickname = ""
    var message : String
    var date = Date()
    var msgtype : MsgType
    
    init(m: TextMessage, nickname: String, msgtype: MsgType) {
        message = fromTTString(m.szMessage)
        self.nickname = nickname
        self.msgtype = msgtype
    }
    
    init(logmsg: String) {
        message = logmsg
        msgtype = .logmsg
    }

    
    func drawCell(_ cell: TextMsgTableCell) {
        
        let dateFormatter = DateFormatter()
        dateFormatter.locale = Locale.current
        dateFormatter.dateFormat = "HH:mm:ss"
        let time = dateFormatter.string(from: date)
        
        switch msgtype {
        case .im :
            let source = limitText(nickname)
            cell.authorLabel.text = "\(source), \(time)"
            cell.backgroundColor = UIColor(red: 1.0, green:0.627, blue:0.882, alpha: 1.0)
            
        case .im_MYSELF :
            let source = limitText(nickname)
            cell.authorLabel.text = "\(source), \(time)"
            cell.backgroundColor = UIColor(red: 0.54, green: 0.82, blue: 0.94, alpha: 1.0)
            
        case .bcast :
            let source = limitText(nickname)
            cell.authorLabel.text = "\(source), \(time)"
            cell.backgroundColor = UIColor(red: 0.831, green: 0.376, blue: 1.0, alpha:1.0)
            
        case .logmsg :
            cell.backgroundColor = UIColor(red: 0.86, green: 0.86, blue: 0.86, alpha: 1.0)
            cell.authorLabel.text = "\(time)"
        }
        cell.messageTextView.text = message

        cell.accessibilityHint = cell.authorLabel.text
        cell.accessibilityLabel = message
        //cell.messageTextView.textContainerInset = UIEdgeInsetsZero
        //cell.messageTextView.textContainer.lineFragmentPadding = 0.0
    }
}

protocol MyTextMessageDelegate {
    func appendTextMessage(_ userid: INT32, txtmsg: MyTextMessage)
}

// messages received but no read (blinking)
var unreadmessages = Set<INT32>()

func isTransmitting(_ ttInst: UnsafeMutableRawPointer, stream: StreamType) -> Bool {
    let flags = TT_GetFlags(ttInst)
    
    switch stream {
    case STREAMTYPE_VOICE:
        if flags & CLIENT_TX_VOICE.rawValue != 0 {
            return true
        }
        if flags & CLIENT_SNDINPUT_VOICEACTIVATED.rawValue != 0 &&
            flags & CLIENT_SNDINPUT_VOICEACTIVE.rawValue != 0 {
                return true
        }
    default : break
    }
    return false
}

func hasPTTLock() -> Bool {
    let defaults = UserDefaults.standard
    return defaults.object(forKey: PREF_GENERAL_PTTLOCK) != nil && defaults.bool(forKey: PREF_GENERAL_PTTLOCK)
}

func limitText(_ s: String) -> String {
    
    let settings = UserDefaults.standard
    let length = settings.object(forKey: PREF_DISPLAY_LIMITTEXT) == nil ? DEFAULT_LIMIT_TEXT : settings.integer(forKey: PREF_DISPLAY_LIMITTEXT)
    
    if s.characters.count > length {
        return s.substring(with: (s.startIndex ..< s.characters.index(s.startIndex, offsetBy: length)))
    }
    return s
}

func getDisplayName(_ user: User) -> String {
    let settings = UserDefaults.standard
    if settings.object(forKey: PREF_DISPLAY_SHOWUSERNAME) != nil && settings.bool(forKey: PREF_DISPLAY_SHOWUSERNAME) {
        return limitText(fromTTString(user.szUsername))
    }
    return limitText(fromTTString(user.szNickname))
}

enum Sounds : Int {
    case tx_ON = 1, tx_OFF = 2, chan_MSG = 3,
         user_MSG = 4, srv_LOST = 5, joined_CHAN = 6, left_CHAN = 7
}

var player : AVAudioPlayer?

func getSoundFile(_ s: Sounds) -> String? {
    
    let settings = UserDefaults.standard
    
    switch s {
    case .tx_ON:
        if settings.object(forKey: PREF_SNDEVENT_VOICETX) == nil ||
           settings.bool(forKey: PREF_SNDEVENT_VOICETX) {
            return "on"
        }
    case .tx_OFF:
        if settings.object(forKey: PREF_SNDEVENT_VOICETX) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_VOICETX) {
                return "off"
        }
    case .chan_MSG:
        if settings.object(forKey: PREF_SNDEVENT_CHANMSG) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_CHANMSG) {
                return "channel_message"
        }
    case .user_MSG:
        if settings.object(forKey: PREF_SNDEVENT_USERMSG) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_USERMSG) {
                return "user_message"
        }
    case .srv_LOST:
        if settings.object(forKey: PREF_SNDEVENT_SERVERLOST) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_SERVERLOST) {
                return "serverlost"
        }
    case .joined_CHAN:
        if settings.object(forKey: PREF_SNDEVENT_JOINEDCHAN) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_JOINEDCHAN) {
                return "newuser"
        }
    case .left_CHAN:
        if settings.object(forKey: PREF_SNDEVENT_LEFTCHAN) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_LEFTCHAN) {
                return "removeuser"
        }
	    }

    return nil
}

func setupSoundDevices() {
    
    let defaults = UserDefaults.standard
    let on = defaults.object(forKey: PREF_VOICEPROCESSINGIO) != nil && defaults.bool(forKey: PREF_VOICEPROCESSINGIO)
    
    let flags = TT_GetFlags(ttInst)
    if flags & CLIENT_SNDINPUT_READY.rawValue == 0 {
        TT_InitSoundInputDevice(ttInst, on ? 1 : 0)
    }
    if flags & CLIENT_SNDOUTPUT_READY.rawValue == 0 {
        TT_InitSoundOutputDevice(ttInst, on ? 1 : 0)
    }
    
    setupSpeakerOutput()
}

func setupSpeakerOutput() {
    
    let defaults = UserDefaults.standard
    let on = defaults.object(forKey: PREF_SPEAKER_OUTPUT) != nil && defaults.bool(forKey: PREF_SPEAKER_OUTPUT)

    if on {
        enableSpeakerOutput(on)
    }
}

func enableSpeakerOutput(_ on: Bool) {
    
    let session = AVAudioSession.sharedInstance()
    
    do {
        print("preset: " + session.mode)
        if on {
            try session.setMode(AVAudioSessionModeVideoChat)
        }
        else {
            try session.setMode(AVAudioSessionModeDefault)
            try session.setCategory(AVAudioSessionCategoryPlayAndRecord)
//            try session.overrideOutputAudioPort(AVAudioSessionPortOverride.None)
        }
//        try session.setActive(true)
        print("post set: "  + session.mode)
    }
    catch {
        print("Failed")
    }
    
}

func playSound(_ s: Sounds) {
    
    let filename = getSoundFile(s)
    
    if filename == nil {
        return
    }
    
    if let resPath = Bundle.main.path(forResource: filename, ofType: "mp3") {
        
        let url = URL(fileURLWithPath: resPath)
        
        do {
            player = try AVAudioPlayer(contentsOf: url)
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
let DEFAULT_VOICEACT = 2
let DEFAULT_MEDIAFILE_VOLUME : Float = 1.0
let DEFAULT_POPUP_TEXTMESSAGE = true
let DEFAULT_LIMIT_TEXT = 25

// initialize TTS values globally
let synth = AVSpeechSynthesizer()
var myUtterance = AVSpeechUtterance(string: "")

let DEFAULT_TTS_VOL : Float = 0.5

func newUtterance(_ utterance: String) {
    let settings = UserDefaults.standard
    myUtterance = AVSpeechUtterance(string: utterance)
    if let rate = settings.value(forKey: PREF_TTSEVENT_RATE) {
        myUtterance.rate = (rate as AnyObject).floatValue!
    }
    if let vol = settings.value(forKey: PREF_TTSEVENT_VOL) {
        myUtterance.volume = (vol as AnyObject).floatValue!
    }
    if let voice = settings.string(forKey: PREF_TTSEVENT_VOICEID) {
        if #available(iOS 9.0, *) {
            myUtterance.voice = AVSpeechSynthesisVoice(identifier: voice)
        } else {
            // Fallback on earlier versions
        }
    }
    else if let lang = settings.string(forKey: PREF_TTSEVENT_VOICELANG) {
        myUtterance.voice = AVSpeechSynthesisVoice(language: lang)
    }
    
    synth.speak(myUtterance)
}

func speakTextMessage(_ msgtype: TextMsgType, mymsg: MyTextMessage) {
    
    let settings = UserDefaults.standard
    let tts_priv = settings.object(forKey: PREF_TTSEVENT_TEXTMSG) != nil && settings.bool(forKey: PREF_TTSEVENT_TEXTMSG) && msgtype == MSGTYPE_USER
    let tts_chan = settings.object(forKey: PREF_TTSEVENT_CHANTEXTMSG) != nil && settings.bool(forKey: PREF_TTSEVENT_CHANTEXTMSG) && msgtype == MSGTYPE_CHANNEL
    
    if tts_priv {
        let ttsmsg = String(format: NSLocalizedString("Private text message from %@. %@", comment: "TTS EVENT"),
            limitText(mymsg.nickname), mymsg.message)
        newUtterance(ttsmsg)
    }
    if tts_chan {
        let ttsmsg = String(format: NSLocalizedString("Channel message from %@. %@", comment: "TTS EVENT"),
            limitText(mymsg.nickname), mymsg.message)
        newUtterance(ttsmsg)
    }
}

@available(iOS 8.0, *)
class MyCustomAction : UIAccessibilityCustomAction {
    
    var tag = 0
    
    init(name: String, target: AnyObject?, selector: Selector, tag: Int) {
        super.init(name: name, target: target, selector: selector)
        self.tag = tag
    }
}

let DEFAULT_NICKNAME = NSLocalizedString("Noname", comment: "default nickname")

let TRUE : TTBOOL = 1
let FALSE : TTBOOL = 0

let DEFAULT_MSEC_PER_PACKET : INT32 = 40

let DEFAULT_AUDIOCODEC = OPUS_CODEC

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

func newAudioCodec(_ codec: Codec) -> AudioCodec {
    var audiocodec = AudioCodec()
    
    switch codec {
    case OPUS_CODEC :
        var opus = newOpusCodec()
        setOpusCodec(&audiocodec, &opus)
    case SPEEX_CODEC :
        var speex = newSpeexCodec()
        setSpeexCodec(&audiocodec, &speex)
    case SPEEX_VBR_CODEC :
        var speexvbr = newSpeexVBRCodec()
        setSpeexVBRCodec(&audiocodec, &speexvbr)
    case NO_CODEC :
        fallthrough
    default :
        break
    }
    return audiocodec
}
func newOpusCodec() -> OpusCodec {
    return OpusCodec(nSampleRate: DEFAULT_OPUS_SAMPLERATE,
        nChannels: DEFAULT_OPUS_CHANNELS, nApplication: DEFAULT_OPUS_APPLICATION,
        nComplexity: DEFAULT_OPUS_COMPLEXITY, bFEC: DEFAULT_OPUS_FEC,
        bDTX: DEFAULT_OPUS_DTX, nBitRate: DEFAULT_OPUS_BITRATE,
        bVBR: DEFAULT_OPUS_VBR, bVBRConstraint: DEFAULT_OPUS_VBRCONSTRAINT,
        nTxIntervalMSec: DEFAULT_MSEC_PER_PACKET)
}

func newSpeexCodec() -> SpeexCodec {
    return SpeexCodec(nBandmode: DEFAULT_SPEEX_BANDMODE, nQuality: DEFAULT_SPEEX_QUALITY, nTxIntervalMSec: DEFAULT_SPEEX_DELAY, bStereoPlayback: DEFAULT_SPEEX_SIMSTEREO)
}

func newSpeexVBRCodec() -> SpeexVBRCodec {
    return SpeexVBRCodec(nBandmode: DEFAULT_SPEEX_VBR_BANDMODE,
        nQuality: DEFAULT_SPEEX_VBR_QUALITY,
        nBitRate: DEFAULT_SPEEX_VBR_BITRATE,
        nMaxBitRate: DEFAULT_SPEEX_VBR_MAXBITRATE,
        bDTX: DEFAULT_SPEEX_VBR_DTX,
        nTxIntervalMSec: DEFAULT_SPEEX_VBR_DELAY,
        bStereoPlayback: DEFAULT_SPEEX_VBR_SIMSTEREO)
}

let DEFAULT_SUBSCRIPTION_USERMSG = true
let DEFAULT_SUBSCRIPTION_CHANMSG = true
let DEFAULT_SUBSCRIPTION_BCASTMSG = true
let DEFAULT_SUBSCRIPTION_VOICE = true
let DEFAULT_SUBSCRIPTION_VIDEOCAP = false
let DEFAULT_SUBSCRIPTION_MEDIAFILE = true
let DEFAULT_SUBSCRIPTION_DESKTOP = false
let DEFAULT_SUBSCRIPTION_DESKTOPINPUT = false

func within<T : Comparable>(_ min_v: T, max_v: T, value: T) -> T {
    if value < min_v {
        return min_v
    }
    if value > max_v {
        return max_v
    }
    return value
}

//TODO: someone find a way out of this madness
func fromTTString(_ cs: (TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR)) -> String {
    var cs = cs
    return withUnsafePointer(to: &cs) { String(cString: UnsafeRawPointer($0).assumingMemoryBound(to: CChar.self)) }
}

func toTTString(_ src: String, dst: inout (TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR)) {

    var c = StringWrap()
    convertTuple(src, &c)
    dst = c.buf
}
