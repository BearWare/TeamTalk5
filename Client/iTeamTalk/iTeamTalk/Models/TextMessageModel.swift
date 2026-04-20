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

import SwiftUI
import TeamTalkKit

final class TextMessageModel: ObservableObject {

    // userid > 0 means private message session (MSGTYPE_USER), 0 = channel chat
    let userid: INT32
    let title: String
    weak var delegate: MyTextMessageDelegate?

    @Published var sections = [TextMessageSection]()
    @Published var composedText = ""

    private var messages = [Int: [MyTextMessage]]()
    private var curMessageSection = 0
    private var mergemessages = [Int: [TextMessage]]()

    init(userid: INT32, title: String) {
        self.userid = userid
        self.title = title
    }

    var showLogMessages: Bool { userid == 0 }

    func appendEventMessage(_ message: MyTextMessage) {
        if messages[curMessageSection] == nil ||
            messages[curMessageSection]?.last?.fromuserid != message.fromuserid ||
            messages[curMessageSection]?.last?.nickname != message.nickname ||
            messages[curMessageSection]?.last?.msgtype != message.msgtype {
            curMessageSection += 1
            messages[curMessageSection] = [MyTextMessage]()
        }
        messages[curMessageSection]?.append(message)

        trimMessagesIfNeeded()
        updateMessages()
    }

    func getLastEventMessage() -> MyTextMessage? {
        messages[curMessageSection]?.last
    }

    func clearUnreadMessages() {
        unreadmessages.remove(userid)
    }

    func sendMessage() {
        let content = composedText
        guard !content.isEmpty else { return }

        var msg = TextMessage()
        msg.nFromUserID = TeamTalkClient.shared.myUserID

        if userid == 0 {
            msg.nMsgType = MSGTYPE_CHANNEL
            msg.nChannelID = TeamTalkClient.shared.myChannelID
        } else {
            msg.nMsgType = MSGTYPE_USER
            msg.nToUserID = userid

            let user = TeamTalkClient.shared.withUser(id: msg.nFromUserID) { $0 }
            let name = getDisplayName(user)
            let mymsg = MyTextMessage(fromuserid: msg.nFromUserID, nickname: name, msgtype: .PRIV_IM_MYSELF, content: content)
            appendEventMessage(mymsg)
            delegate?.appendTextMessage(userid, txtmsg: mymsg)
        }

        if TeamTalkClient.shared.sendTextMessage(msg, content: content) {
            composedText = ""
        }
    }

    private func getTextMessageContent(_ msg: TextMessage) -> String? {
        let key = generateKey(msg)
        if msg.bMore == TRUE {
            if mergemessages[key] == nil {
                mergemessages[key] = [TextMessage]()
            }
            mergemessages[key]!.append(msg)
            if mergemessages[key]!.count > 1000 {
                mergemessages.removeValue(forKey: key)
            }
        } else if mergemessages[key] != nil {
            var content = ""
            for m in mergemessages[key]! {
                content += TeamTalkString.textMessage(m)
            }
            mergemessages.removeValue(forKey: key)
            return content + TeamTalkString.textMessage(msg)
        } else {
            return TeamTalkString.textMessage(msg)
        }
        return nil
    }

    private func generateKey(_ msg: TextMessage) -> Int {
        (Int(msg.nMsgType.rawValue) << 16) | Int(msg.nFromUserID)
    }

    private func updateMessages() {
        let updatedSections = messages.keys.sorted().compactMap { key -> TextMessageSection? in
            guard let values = messages[key], let first = values.first else { return nil }
            return TextMessageSection(title: sectionTitle(for: first), messages: values)
        }
        sections = updatedSections
    }

    private func trimMessagesIfNeeded() {
        while messageCount > MAX_TEXTMESSAGES, let key = messages.keys.sorted().first {
            messages[key]?.removeFirst()
            if messages[key]?.isEmpty != false {
                messages.removeValue(forKey: key)
            }
        }
    }

    private var messageCount: Int {
        messages.values.reduce(0) { $0 + $1.count }
    }

    private func sectionTitle(for message: MyTextMessage) -> String {
        switch message.msgtype {
        case .PRIV_IM, .PRIV_IM_MYSELF, .CHAN_IM, .CHAN_IM_MYSELF, .BCAST:
            return message.nickname
        case .LOGMSG:
            return String(localized: "Status Event", comment: "Text message view")
        }
    }
}

extension TextMessageModel: TeamTalkEvent {
    func handleTTMessage(_ m: TTMessage) {
        switch m.nClientEvent {
        case CLIENTEVENT_CMD_USER_TEXTMSG:
            let txtmsg = TeamTalkMessagePayload.textMessage(from: m)

            if (txtmsg.nMsgType == MSGTYPE_USER && txtmsg.nFromUserID == userid) ||
                (txtmsg.nMsgType == MSGTYPE_CHANNEL && userid == 0) ||
                (txtmsg.nMsgType == MSGTYPE_BROADCAST && userid == 0) {

                if let content = getTextMessageContent(txtmsg) {
                    let user = TeamTalkClient.shared.withUser(id: txtmsg.nFromUserID) { $0 }
                    var msgtype = MsgType.PRIV_IM
                    switch txtmsg.nMsgType {
                    case MSGTYPE_USER:
                        msgtype = TeamTalkClient.shared.myUserID == txtmsg.nFromUserID ? .PRIV_IM_MYSELF : .PRIV_IM
                    case MSGTYPE_CHANNEL:
                        msgtype = TeamTalkClient.shared.myUserID == txtmsg.nFromUserID ? .CHAN_IM_MYSELF : .CHAN_IM
                    case MSGTYPE_BROADCAST:
                        msgtype = .BCAST
                    default:
                        break
                    }
                    let name = getDisplayName(user)
                    let mymsg = MyTextMessage(fromuserid: txtmsg.nFromUserID, nickname: name, msgtype: msgtype, content: content)
                    appendEventMessage(mymsg)
                    speakTextMessage(txtmsg.nMsgType, mymsg: mymsg)
                }
            }

        case CLIENTEVENT_CMD_USER_LOGGEDIN:
            let user = TeamTalkMessagePayload.user(from: m)
            if showLogMessages && TeamTalkClient.shared.myUserID == user.nUserID {
                appendEventMessage(MyTextMessage(logmsg: String(localized: "Logged on to server", comment: "log entry")))
            }

        case CLIENTEVENT_CMD_USER_JOINED:
            let user = TeamTalkMessagePayload.user(from: m)
            if showLogMessages && TeamTalkClient.shared.myChannelID == user.nChannelID {
                let logmsg: MyTextMessage
                if TeamTalkClient.shared.myUserID == user.nUserID {
                    let channame = TeamTalkClient.shared.withChannel(id: user.nChannelID) { channel in
                        if channel.nParentID == 0 {
                            return String(localized: "root channel", comment: "log entry")
                        }
                        return TeamTalkString.channel(.name, from: channel)
                    }
                    let txt = String(format: String(localized: "Joined %@", comment: "log entry"), channame)
                    logmsg = MyTextMessage(logmsg: txt)
                } else {
                    let name = getDisplayName(user)
                    let txt = String(format: String(localized: "%@ joined channel", comment: "log entry"), name)
                    logmsg = MyTextMessage(logmsg: txt)
                }
                appendEventMessage(logmsg)
            }

        case CLIENTEVENT_CMD_USER_LEFT:
            let user = TeamTalkMessagePayload.user(from: m)
            if showLogMessages && TeamTalkClient.shared.myChannelID == m.nSource {
                let name = getDisplayName(user)
                let txt = String(format: String(localized: "%@ left channel", comment: "log entry"), name)
                appendEventMessage(MyTextMessage(logmsg: txt))
            }

        case CLIENTEVENT_CMD_ERROR:
            let errmsg = TeamTalkString.clientError(TeamTalkMessagePayload.clientError(from: m))
            let txt = String(format: String(localized: "Command failed: %@", comment: "log entry"), errmsg)
            appendEventMessage(MyTextMessage(logmsg: txt))

        default:
            break
        }
    }
}

struct TextMessageSection: Identifiable {
    let id = UUID()
    let title: String
    let messages: [MyTextMessage]
}
