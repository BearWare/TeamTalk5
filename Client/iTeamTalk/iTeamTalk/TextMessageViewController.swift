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
import UIKit

final class TextMessageViewController: UIHostingController<TextMessageView>, TeamTalkEvent {

    // userid > 0 means private message session, i.e. MSGTYPE_USER
    var userid: INT32 = 0
    var delegate: MyTextMessageDelegate?

    private let initialText = NSLocalizedString("Type text here", comment: "text message")
    private let model: TextMessageModel

    private var messages = [Int: [MyTextMessage]]()
    private var curMessageSection = 0
    private var mergemessages = [Int: [TextMessage]]()

    init() {
        let model = TextMessageModel()
        self.model = model
        super.init(rootView: TextMessageView(
            model: model,
            placeholder: NSLocalizedString("Type text here", comment: "text message"),
            sendMessage: { },
            dismissKeyboard: { }
        ))
    }

    required init?(coder: NSCoder) {
        fatalError("TextMessageViewController is always created programmatically")
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        rootView = TextMessageView(
            model: model,
            placeholder: initialText,
            sendMessage: { [weak self] in self?.sendCurrentMessage() },
            dismissKeyboard: { [weak self] in self?.view.endEditing(true) }
        )
    }

    override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)

        if isMovingFromParent {
            unreadmessages.remove(userid)
        }
    }

    func showLogMessages() -> Bool {
        userid == 0
    }

    func appendEventMessage(_ message: MyTextMessage) {
        if messages[curMessageSection] == nil ||
            messages[curMessageSection]?.last?.fromuserid != message.fromuserid ||
            messages[curMessageSection]?.last?.nickname != message.nickname ||
            messages[curMessageSection]?.last?.msgtype != message.msgtype {
            curMessageSection += 1
            messages[curMessageSection] = [MyTextMessage]()
        }
        messages[curMessageSection]!.append(message)

        if messages.values.count > MAX_TEXTMESSAGES {
            let key = messages.keys.sorted().first
            messages[key!]!.removeFirst()
            if messages[key!]!.isEmpty {
                messages.removeValue(forKey: key!)
            }
        }
        updateMessages()
    }

    func getLastEventMessage() -> MyTextMessage? {
        messages[curMessageSection]?.last
    }

    private func generateKey(_ msg: TextMessage) -> Int {
        (Int(msg.nMsgType.rawValue) << 16) | Int(msg.nFromUserID)
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

    private func sendCurrentMessage() {
        let content = model.composedText
        if content.isEmpty {
            view.endEditing(true)
            return
        }

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
            model.composedText = ""
        }
    }

    private func updateMessages() {
        let sections = messages.keys.sorted().compactMap { key -> TextMessageSection? in
            guard let values = messages[key], let first = values.first else {
                return nil
            }
            return TextMessageSection(title: sectionTitle(for: first), messages: values)
        }
        model.sections = sections
    }

    private func sectionTitle(for message: MyTextMessage) -> String {
        switch message.msgtype {
        case .PRIV_IM, .PRIV_IM_MYSELF, .CHAN_IM, .CHAN_IM_MYSELF, .BCAST:
            return message.nickname
        case .LOGMSG:
            return NSLocalizedString("Status Event", comment: "Text message view")
        }
    }

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
            if showLogMessages() && TeamTalkClient.shared.myUserID == user.nUserID {
                appendEventMessage(MyTextMessage(logmsg: NSLocalizedString("Logged on to server", comment: "log entry")))
            }

        case CLIENTEVENT_CMD_USER_JOINED:
            let user = TeamTalkMessagePayload.user(from: m)
            if showLogMessages() && TeamTalkClient.shared.myChannelID == user.nChannelID {
                let logmsg: MyTextMessage
                if TeamTalkClient.shared.myUserID == user.nUserID {
                    let channame = TeamTalkClient.shared.withChannel(id: user.nChannelID) { channel in
                        if channel.nParentID == 0 {
                            return NSLocalizedString("root channel", comment: "log entry")
                        }
                        return TeamTalkString.channel(.name, from: channel)
                    }
                    let txt = String(format: NSLocalizedString("Joined %@", comment: "log entry"), channame)
                    logmsg = MyTextMessage(logmsg: txt)
                } else {
                    let name = getDisplayName(user)
                    let txt = String(format: NSLocalizedString("%@ joined channel", comment: "log entry"), name)
                    logmsg = MyTextMessage(logmsg: txt)
                }
                appendEventMessage(logmsg)
            }

        case CLIENTEVENT_CMD_USER_LEFT:
            let user = TeamTalkMessagePayload.user(from: m)
            if showLogMessages() && TeamTalkClient.shared.myChannelID == m.nSource {
                let name = getDisplayName(user)
                let txt = String(format: NSLocalizedString("%@ left channel", comment: "log entry"), name)
                appendEventMessage(MyTextMessage(logmsg: txt))
            }

        case CLIENTEVENT_CMD_ERROR:
            let errmsg = TeamTalkString.clientError(TeamTalkMessagePayload.clientError(from: m))
            let txt = String(format: NSLocalizedString("Command failed: %@", comment: "log entry"), errmsg)
            appendEventMessage(MyTextMessage(logmsg: txt))

        default:
            break
        }
    }
}

final class TextMessageModel: ObservableObject {
    @Published var sections = [TextMessageSection]()
    @Published var composedText = ""
}

struct TextMessageSection: Identifiable {
    let id = UUID()
    let title: String
    let messages: [MyTextMessage]
}

struct TextMessageView: View {
    @ObservedObject var model: TextMessageModel
    let placeholder: String
    let sendMessage: () -> Void
    let dismissKeyboard: () -> Void

    @FocusState private var isComposing: Bool

    var body: some View {
        VStack(spacing: 0) {
            ScrollViewReader { proxy in
                List {
                    ForEach(model.sections) { section in
                        Section(section.title) {
                            ForEach(section.messages.indices, id: \.self) { index in
                                let message = section.messages[index]
                                TeamTalkMessageRow(message: message)
                                    .listRowInsets(EdgeInsets(top: 4, leading: 8, bottom: 4, trailing: 8))
                                    .listRowBackground(message.backgroundColor)
                            }
                        }
                    }
                }
                .listStyle(.plain)
                .onChange(of: model.sections.count) { _ in
                    scrollToBottom(proxy)
                }
                .onChange(of: model.sections.last?.messages.count ?? 0) { _ in
                    scrollToBottom(proxy)
                }
            }

            Divider()

            HStack(alignment: .bottom, spacing: 8) {
                ZStack(alignment: .topLeading) {
                    TextEditor(text: $model.composedText)
                        .focused($isComposing)
                        .frame(minHeight: 40, maxHeight: 96)
                        .textInputAutocapitalization(.sentences)
                        .accessibilityLabel(NSLocalizedString("Send empty message to close keyboard", comment: "text message"))
                        .onChange(of: model.composedText) { text in
                            sendOnReturnIfNeeded(text)
                        }

                    if model.composedText.isEmpty {
                        Text(placeholder)
                            .foregroundStyle(.secondary)
                            .padding(.horizontal, 5)
                            .padding(.vertical, 8)
                            .allowsHitTesting(false)
                    }
                }

                Button(NSLocalizedString("Send", comment: "text message")) {
                    if model.composedText.isEmpty {
                        isComposing = false
                        dismissKeyboard()
                    } else {
                        sendMessage()
                    }
                }
                .buttonStyle(.borderedProminent)
            }
            .padding(.horizontal, 8)
            .padding(.vertical, 6)
            .background(.bar)
        }
    }

    private func sendOnReturnIfNeeded(_ text: String) {
        let defaults = UserDefaults.standard
        let sendOnReturn = defaults.object(forKey: PREF_GENERAL_SENDONRETURN) == nil || defaults.bool(forKey: PREF_GENERAL_SENDONRETURN)
        guard sendOnReturn, text.contains("\n") else {
            return
        }
        model.composedText = text.replacingOccurrences(of: "\n", with: "")
        sendMessage()
    }

    private func scrollToBottom(_ proxy: ScrollViewProxy) {
        guard let section = model.sections.last, !section.messages.isEmpty else {
            return
        }
        DispatchQueue.main.async {
            proxy.scrollTo(section.id, anchor: .bottom)
        }
    }
}
