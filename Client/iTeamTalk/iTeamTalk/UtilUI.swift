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

import TeamTalkKit
import UIKit
import SwiftUI

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
        message = TeamTalkString.textMessage(m)
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
}

protocol MyTextMessageDelegate: AnyObject {
    func appendTextMessage(_ userid: INT32, txtmsg: MyTextMessage)
}

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
    let settings = UserDefaults.standard
    if settings.object(forKey: PREF_DISPLAY_SHOWUSERNAME) != nil && settings.bool(forKey: PREF_DISPLAY_SHOWUSERNAME) {
        return limitText(TeamTalkString.user(.username, from: user))
    }

    let nickname = TeamTalkString.user(.nickname, from: user)
    if nickname.isEmpty {
        return DEFAULT_NICKNAME + " - #\(user.nUserID)"
    }
    
    return limitText(nickname)
}


func formTextField(
    _ title: LocalizedStringKey,
    text: Binding<String>,
    keyboardType: UIKeyboardType = .default
) -> some View {
    LabeledContent {
        TextField("", text: text)
            .frame(maxWidth: .infinity, alignment: .trailing)
            .multilineTextAlignment(.trailing)
            .autocorrectionDisabled()
            .textInputAutocapitalization(.never)
            .keyboardType(keyboardType)
            .accessibilityLabel(Text(title))
    } label: {
        Text(title)
            .accessibilityHidden(true)
    }
}

func formPasswordField(
    _ title: LocalizedStringKey,
    text: Binding<String>,
    isRevealed: Bool=false
) -> some View {
    LabeledContent {
        Group {
            if isRevealed {
                TextField("", text: text)
                    .autocorrectionDisabled()
            } else {
                SecureField("", text: text)
            }
        }
        .frame(maxWidth: .infinity, alignment: .trailing)
        .multilineTextAlignment(.trailing)
        .textInputAutocapitalization(.never)
        .accessibilityLabel(Text(title))
    } label: {
        Text(title)
            .accessibilityHidden(true)
    }
}

func formTextField(
    _ title: LocalizedStringKey,
    text: Binding<String>,
    disabled: Bool = false
) -> some View {
    LabeledContent {
        TextField("", text: text)
            .frame(maxWidth: .infinity, alignment: .trailing)
            .multilineTextAlignment(.trailing)
            .autocorrectionDisabled()
            .textInputAutocapitalization(.never)
            .disabled(disabled)
            .accessibilityLabel(Text(title))
    } label: {
        Text(title)
            .accessibilityHidden(true)
    }
}

