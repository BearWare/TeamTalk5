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
import Foundation
import AVFoundation

enum StatusMode : UInt {
    case STATUSMODE_AVAILABLE   = 0x00000000,
    STATUSMODE_AWAY             = 0x00000001,
    STATUSMODE_QUESTION         = 0x00000002,
    STATUSMODE_MODE             = 0x000000FF,

    STATUSMODE_FLAGS            = 0xFFFFFF00,
    STATUSMODE_FEMALE           = 0x00000100,
    STATUSMODE_VIDEOTX          = 0x00000200,
    STATUSMODE_DESKTOP          = 0x00000400,
    STATUSMODE_STREAM_MEDIAFILE = 0x00000800
}

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

let DEFAULT_SUBSCRIPTION_USERMSG = true
let DEFAULT_SUBSCRIPTION_CHANMSG = true
let DEFAULT_SUBSCRIPTION_BCASTMSG = true
let DEFAULT_SUBSCRIPTION_VOICE = true
let DEFAULT_SUBSCRIPTION_VIDEOCAP = true
let DEFAULT_SUBSCRIPTION_MEDIAFILE = true
let DEFAULT_SUBSCRIPTION_DESKTOP = true
let DEFAULT_SUBSCRIPTION_DESKTOPINPUT = false

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

func getXMLPath(elementStack : [String]) -> String {
    var path = ""
    for s in elementStack {
        path += "/" + s
    }
    return path
}

//shared TTInstance between all view controllers
var ttInst : UnsafeMutableRawPointer? = nil

// messages received but no read (blinking)
var unreadmessages = Set<INT32>()

// types of responses
enum Command {
    case loginCmd, joinCmd, moveCmd, kickCmd, banCmd
}

enum Sounds : Int {
    case tx_ON = 1,
         tx_OFF = 2,
         chan_MSG = 3,
         broadcast_MSG = 4,
         user_MSG = 5,
         srv_LOST = 6,
         joined_CHAN = 7,
         left_CHAN = 8,
         voxtriggered_ON = 9,
         voxtriggered_OFF = 10,
         transmit_ON = 11,
         transmit_OFF = 12,
         logged_IN = 13,
         logged_OUT = 14
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
    case .broadcast_MSG:
        if settings.object(forKey: PREF_SNDEVENT_BCASTMSG) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_BCASTMSG) {
            return "broadcast_message"
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
    case .voxtriggered_ON :
        if settings.object(forKey: PREF_SNDEVENT_VOXTRIGGER) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_VOXTRIGGER) {
            return "voiceact_on"
        }
    case .voxtriggered_OFF :
        if settings.object(forKey: PREF_SNDEVENT_VOXTRIGGER) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_VOXTRIGGER) {
            return "voiceact_off"
        }
    case .transmit_ON :
        if settings.object(forKey: PREF_SNDEVENT_TRANSMITREADY) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_TRANSMITREADY) {
            return "txqueue_start"
        }
    case .transmit_OFF :
        if settings.object(forKey: PREF_SNDEVENT_TRANSMITREADY) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_TRANSMITREADY) {
            return "txqueue_stop"
        }
    case .logged_IN :
        if settings.object(forKey: PREF_SNDEVENT_LOGGEDIN) != nil &&
            settings.bool(forKey: PREF_SNDEVENT_LOGGEDIN) {
            return "logged_on"
        }
    case .logged_OUT :
        if settings.object(forKey: PREF_SNDEVENT_LOGGEDOUT) != nil &&
            settings.bool(forKey: PREF_SNDEVENT_LOGGEDOUT) {
            return "logged_off"
        }
    }

    return nil
}

func setupSoundDevices() {
    
    do {
        let session = AVAudioSession.sharedInstance()

        print("preset: " + session.mode.rawValue)
        
        let defaults = UserDefaults.standard
        let preprocess = defaults.object(forKey: PREF_VOICEPROCESSINGIO) != nil && defaults.bool(forKey: PREF_VOICEPROCESSINGIO)
        let speaker = defaults.object(forKey: PREF_SPEAKER_OUTPUT) != nil && defaults.bool(forKey: PREF_SPEAKER_OUTPUT)
        
        TT_CloseSoundInputDevice(ttInst)
        TT_CloseSoundOutputDevice(ttInst)
        
        // Something weird is going on here. If TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO is
        // initialized (along with preprocess and speaker-enabled) and afterwards replaced by
        // TT_SOUNDDEVICE_ID_REMOTEIO without preprocess and speaker-enabled then the voice
        // volume is gained at lot compared to initially with TT_SOUNDDEVICE_ID_REMOTEIO.
        
        let mode = preprocess ? AVAudioSession.Mode.voiceChat : AVAudioSession.Mode.default
        let catoptions = speaker ? AVAudioSession.CategoryOptions.defaultToSpeaker : AVAudioSession.CategoryOptions.allowBluetooth
        
        try session.setMode(mode)
        try session.setCategory(AVAudioSession.Category.playAndRecord, options: catoptions)
        
        let sndid = speaker && preprocess ? TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO : TT_SOUNDDEVICE_ID_REMOTEIO
        if TT_InitSoundInputDevice(ttInst, sndid) == FALSE {
            print("Failed to initialize sound input device: \(sndid)")
        }
        else {
            print("Using sound input device: \(sndid)")
        }
        if TT_InitSoundOutputDevice(ttInst, sndid) == FALSE {
            print("Failed to initialize sound output device: \(sndid)")
        }
        else {
            print("Using sound output device: \(sndid)")
        }
        print("postset. Mode \(session.mode.rawValue), category \(session.category.rawValue), options \(session.categoryOptions.rawValue)")
    }
    catch {
        print("Failed to set mode")
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
let VOICEACT_DISABLED = 21 //DEFAULT_SOUND_VU_MAX + 1
let DEFAULT_VOICEACT = 2
let DEFAULT_MEDIAFILE_VOLUME : Float = 0.5
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

func userCacheID(user: User) -> String {
    let username = getUser(user, strprop: USERNAME)
    if username.hasSuffix(AppInfo.WEBLOGIN_BEARWARE_USERNAMEPOSTFIX) {
        return username + "|" + getUser(user, strprop: CLIENTNAME)
    }
    return ""
}

class UserCached {
    var subscriptions : UINT32
    var voiceMute : Bool
    var mediaMute : Bool
    var voiceVolume : INT32
    var mediaVolume : INT32
    var voiceLeftSpeaker, voiceRightSpeaker,
    mediaLeftSpeaker, mediaRightSpeaker : TTBOOL
    
    init(user : User) {
        subscriptions = user.uLocalSubscriptions
        voiceMute = (user.uUserState & USERSTATE_MUTE_VOICE.rawValue) == USERSTATE_MUTE_VOICE.rawValue
        mediaMute = (user.uUserState & USERSTATE_MUTE_MEDIAFILE.rawValue) == USERSTATE_MUTE_MEDIAFILE.rawValue
        voiceVolume = user.nVolumeVoice
        mediaVolume = user.nVolumeMediaFile
        voiceLeftSpeaker = user.stereoPlaybackVoice.0
        voiceRightSpeaker = user.stereoPlaybackVoice.1
        mediaLeftSpeaker = user.stereoPlaybackMediaFile.0
        mediaRightSpeaker = user.stereoPlaybackMediaFile.1
    }
    
    func sync(user: User) {
        TT_SetUserMute(ttInst, user.nUserID, STREAMTYPE_VOICE, voiceMute ? TRUE : FALSE)
        TT_SetUserMute(ttInst, user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO, mediaMute ? TRUE : FALSE)
        TT_SetUserVolume(ttInst, user.nUserID, STREAMTYPE_VOICE, voiceVolume)
        TT_SetUserVolume(ttInst, user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO, mediaVolume)
        TT_SetUserStereo(ttInst, user.nUserID, STREAMTYPE_VOICE, voiceLeftSpeaker, voiceRightSpeaker)
        TT_SetUserStereo(ttInst, user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO, mediaLeftSpeaker, mediaRightSpeaker)
        if subscriptions != user.uLocalSubscriptions {
            TT_DoUnsubscribe(ttInst, user.nUserID, user.uLocalSubscriptions ^ subscriptions)
            TT_DoSubscribe(ttInst, user.nUserID, subscriptions)
        }
        TT_PumpMessage(ttInst, CLIENTEVENT_USER_STATECHANGE, user.nUserID)
    }
}

func syncFromUserCache(user: User) {
    let cacheid = userCacheID(user: user)
    if cacheid.isEmpty == false {
        if let cache = userCache[cacheid] {
            cache.sync(user: user)
        }
    }
}

func syncToUserCache(user: User) {
    let cacheid = userCacheID(user: user)
    if cacheid.isEmpty == false {
        userCache[cacheid] = UserCached(user: user)
    }
}

var userCache = [String : UserCached]()

let DEFAULT_NICKNAME = NSLocalizedString("Noname", comment: "default nickname")

func within<T : Comparable>(_ min_v: T, max_v: T, value: T) -> T {
    if value < min_v {
        return min_v
    }
    if value > max_v {
        return max_v
    }
    return value
}
