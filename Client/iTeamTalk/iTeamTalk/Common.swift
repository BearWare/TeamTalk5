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

let MAX_TEXTMESSAGES = 100
let DEFAULT_SOUND_VU_MAX = 20 // real max is SOUND_VU_MAX
let VOICEACT_DISABLED = 21 //DEFAULT_SOUND_VU_MAX + 1
let DEFAULT_VOICEACT = 2
let DEFAULT_MEDIAFILE_VOLUME : Float = 0.5
let DEFAULT_POPUP_TEXTMESSAGE = true
let DEFAULT_LIMIT_TEXT = 25
// Auto-set Away on phone/FaceTime calls
let PREF_GENERAL_AUTO_CALL_AWAY = "general.autocallaway"
let DEFAULT_AUTO_CALL_AWAY = true

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
