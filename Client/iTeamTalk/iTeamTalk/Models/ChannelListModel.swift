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

import AVFoundation
import SwiftUI
import TeamTalkKit

// MARK: - Navigation destination

enum ChannelListDestination: Hashable {
    case userDetail(UserDetailModel)
    case textMessage(TextMessageModel)

    static func == (lhs: Self, rhs: Self) -> Bool {
        switch (lhs, rhs) {
        case (.userDetail(let a), .userDetail(let b)): return a === b
        case (.textMessage(let a), .textMessage(let b)): return a === b
        default: return false
        }
    }

    func hash(into hasher: inout Hasher) {
        switch self {
        case .userDetail(let m):  hasher.combine(0); hasher.combine(ObjectIdentifier(m))
        case .textMessage(let m): hasher.combine(1); hasher.combine(ObjectIdentifier(m))
        }
    }
}

// MARK: - Row model

enum ChannelListRow: Identifiable {
    case join
    case user(User)
    case channel(Channel)

    var id: String {
        switch self {
        case .join: return "join"
        case .user(let user): return "user-\(user.nUserID)"
        case .channel(let channel): return "channel-\(channel.nChannelID)"
        }
    }
}

// MARK: - Channel List Model

final class ChannelListModel: ObservableObject {

    // MARK: Published state for the channel list view
    @Published var rows: [ChannelListRow] = []
    @Published var isTransmitting: Bool = false
    @Published var pttHint: String = String(localized: "Toggle to enable/disable transmission", comment: "channel list")
    @Published var navigationTitle: String = ""

    // MARK: Published navigation state
    @Published var navigationPath: [ChannelListDestination] = []
    @Published var channelDetailModel: ChannelDetailModel?

    // MARK: Published alert state
    @Published var showingJoinPasswordAlert = false
    @Published var joinPassword = ""
    @Published var errorMessage: String?

    // MARK: Server / channel state
    var channels = [INT32: Channel]()
    var chanpasswds = [INT32: String]()
    var curchannel = Channel()
    var mychannel = Channel()
    var rejoinchannel = Channel()
    var users = [INT32: User]()
    var moveusers = Set<INT32>()
    var cmdid: INT32 = 0
    var currentCmdId: INT32 = 0
    var activeCommands = [INT32: Command]()
    var srvprop = ServerProperties()
    var myuseraccount = UserAccount()
    var textmessages = [INT32: [MyTextMessage]]()
    var unreadTimer: Timer?
    var displayUsers = [User]()
    var displayChans = [Channel]()
    var pttLockTimeout = Date()

    // MARK: Private state
    private var joiningChannel: Channel?
    private weak var currentTextMessageModel: TextMessageModel?

    // MARK: Deinit
    deinit {
        for (_, user) in users {
            syncToUserCache(user: user)
        }
    }

    // MARK: - Display helpers

    func updateDisplayItems() {
        let subchans: [Channel] = channels.values.filter { $0.nParentID == curchannel.nChannelID }
        let chanusers: [User] = users.values.filter { $0.nChannelID == curchannel.nChannelID }

        let settings = UserDefaults.standard
        let chansort = settings.object(forKey: PREF_DISPLAY_SORTCHANNELS) == nil
            ? ChanSort.ASCENDING.rawValue
            : settings.integer(forKey: PREF_DISPLAY_SORTCHANNELS)

        switch chansort {
        case ChanSort.POPULARITY.rawValue:
            displayChans = subchans.sorted { lhs, rhs in
                let au = users.values.filter { $0.nChannelID == lhs.nChannelID }
                let bu = users.values.filter { $0.nChannelID == rhs.nChannelID }
                let aname = TeamTalkString.channel(.name, from: lhs)
                let bname = TeamTalkString.channel(.name, from: rhs)
                return au.count == bu.count
                    ? aname.caseInsensitiveCompare(bname) == .orderedAscending
                    : au.count > bu.count
            }
        default:
            displayChans = subchans.sorted {
                let aname = TeamTalkString.channel(.name, from: $0)
                let bname = TeamTalkString.channel(.name, from: $1)
                return aname.caseInsensitiveCompare(bname) == .orderedAscending
            }
        }
        displayUsers = chanusers.sorted {
            getDisplayName($0).caseInsensitiveCompare(getDisplayName($1)) == .orderedAscending
        }
    }

    func refreshChannelList() {
        moveusers = Set(moveusers.filter { users[$0] != nil })
        updateDisplayItems()
        rows = displayRows()
    }

    private func displayRows() -> [ChannelListRow] {
        let showJoin = curchannel.nChannelID != mychannel.nChannelID && curchannel.nChannelID > 0
        var result = [ChannelListRow]()
        if showJoin { result.append(.join) }
        for user in displayUsers { result.append(.user(user)) }
        if curchannel.nParentID != 0, let parent = channels[curchannel.nParentID] {
            result.append(.channel(parent))
        }
        for channel in displayChans { result.append(.channel(channel)) }
        return result
    }

    func updateTitle() {
        if curchannel.nParentID == 0 {
            navigationTitle = TeamTalkString.serverProperties(.name, from: srvprop)
        } else {
            navigationTitle = TeamTalkString.channel(.name, from: curchannel)
        }
    }

    // MARK: - User / channel detail providers

    func userDetails(_ user: User) -> ChannelUserDetails {
        let female = (UInt(user.nStatusMode) & StatusMode.STATUSMODE_FEMALE.rawValue) != 0
        let isTalking = user.uUserState & USERSTATE_VOICE.rawValue != 0 ||
            (TeamTalkClient.shared.myUserID == user.nUserID && TeamTalkClient.shared.isVoiceTransmitting)
        let iconName = isTalking
            ? (female ? "woman_green" : "man_green")
            : (female ? "woman_blue" : "man_blue")
        let iconAccessibilityLabel = isTalking
            ? String(localized: "Talking", comment: "channel list")
            : String(localized: "Silent", comment: "channel list")
        let messageIcon = unreadmessages.contains(user.nUserID) && Int(Date().timeIntervalSince1970) % 2 == 0
            ? "message_red"
            : "message_blue"
        return ChannelUserDetails(
            title: getDisplayName(user),
            subtitle: TeamTalkString.user(.statusMessage, from: user),
            iconName: iconName,
            iconAccessibilityLabel: iconAccessibilityLabel,
            messageIconName: messageIcon
        )
    }

    func channelDetails(_ channel: Channel) -> ChannelDisplayDetails {
        let op = TeamTalkClient.shared.isChannelOperator(channelID: channel.nChannelID)
        let canEdit = (myuseraccount.uUserRights & USERRIGHT_MODIFY_CHANNELS.rawValue) != 0 || op
        let actionTitle = canEdit
            ? String(localized: "Edit", comment: "channel list")
            : String(localized: "View", comment: "channel list")

        if curchannel.nChannelID == 0 {
            let iconName = channel.bPassword != 0 ? "channel_pink" : "channel_orange"
            let iconLabel = channel.bPassword != 0
                ? String(localized: "Password protected", comment: "channel list")
                : String(localized: "No password", comment: "channel list")
            return ChannelDisplayDetails(
                title: TeamTalkString.serverProperties(.name, from: srvprop),
                subtitle: TeamTalkString.channel(.topic, from: channel),
                iconName: iconName,
                iconAccessibilityLabel: iconLabel,
                actionTitle: actionTitle,
                isParent: false
            )
        }

        if channel.nChannelID == curchannel.nParentID {
            let subtitle = channel.nParentID == 0
                ? TeamTalkString.serverProperties(.name, from: srvprop)
                : TeamTalkString.channel(.name, from: channel)
            return ChannelDisplayDetails(
                title: String(localized: "Parent channel", comment: "channel list"),
                subtitle: subtitle,
                iconName: "back_orange",
                iconAccessibilityLabel: String(localized: "Return to previous channel", comment: "channel list"),
                actionTitle: actionTitle,
                isParent: true
            )
        }

        let userCount = getUsersCount(channel.nChannelID)
        let iconName = channel.bPassword != 0 ? "channel_pink" : "channel_orange"
        let iconLabel = String(format: String(localized: "Channel. %d users", comment: "channel list"), userCount)
        return ChannelDisplayDetails(
            title: TeamTalkString.channel(.name, from: channel) + " (\(userCount))",
            subtitle: TeamTalkString.channel(.topic, from: channel),
            iconName: iconName,
            iconAccessibilityLabel: iconLabel,
            actionTitle: actionTitle,
            isParent: false
        )
    }

    func getUsersCount(_ chanid: INT32) -> Int {
        var count = users.values.filter { $0.nChannelID == chanid }.count
        for c in channels.values.filter({ $0.nParentID == chanid }) {
            count += getUsersCount(c.nChannelID)
        }
        return count
    }

    // MARK: - Row selection

    func selectRow(_ row: ChannelListRow) {
        switch row {
        case .join:
            joinCurrentChannel()
        case .user(let user):
            showUserDetail(userid: user.nUserID)
        case .channel(let channel):
            curchannel = channel
            refreshChannelList()
            updateTitle()
        }
    }

    func joinCurrentChannel() {
        joinNewChannel(curchannel)
    }

    // MARK: - Channel joining

    func joinNewChannel(_ channel: Channel) {
        if channel.bPassword == TRUE {
            joiningChannel = channel
            //joinPassword = TeamTalkString.channel(.password, from: channel)
            showingJoinPasswordAlert = true
        } else {
            cmdid = TeamTalkClient.shared.joinChannel(id: channel.nChannelID)
            activeCommands[cmdid] = .joinCmd
        }
    }

    func confirmJoinWithPassword() {
        guard let channel = joiningChannel else { return }
        chanpasswds[channel.nChannelID] = joinPassword
        cmdid = TeamTalkClient.shared.joinChannel(id: channel.nChannelID, password: joinPassword)
        activeCommands[cmdid] = .joinCmd
        joiningChannel = nil
    }

    func joinChannelFromAccessibility(channelID: INT32) {
        if let channel = channels[channelID] {
            joinNewChannel(channel)
        }
    }

    // MARK: - Accessibility actions

    func muteUser(userid: INT32) {
        guard let user = users[userid] else { return }
        TeamTalkClient.shared.setUserMute(
            userID: userid,
            stream: STREAMTYPE_MEDIAFILE_AUDIO,
            muted: (user.uUserState & USERSTATE_MUTE_MEDIAFILE.rawValue) == 0
        )
        TeamTalkClient.shared.setUserMute(
            userID: userid,
            stream: STREAMTYPE_VOICE,
            muted: (user.uUserState & USERSTATE_MUTE_VOICE.rawValue) == 0
        )
        TeamTalkClient.shared.pump(CLIENTEVENT_USER_STATECHANGE, source: userid)
    }

    func moveUser(userid: INT32) {
        guard let user = users[userid] else { return }

        let isSelected = moveusers.contains(userid)
        if isSelected {
            moveusers.remove(userid)
        } else {
            moveusers.insert(userid)
        }

        refreshChannelList()
        announceForAccessibility(
            String(
                format: isSelected
                    ? String(localized: "%@ deselected", comment: "channel list")
                    : String(localized: "%@ selected", comment: "channel list"),
                getDisplayName(user)
            )
        )
    }

    func kickUser(userid: INT32) {
        let op = TeamTalkClient.shared.isChannelOperator(channelID: curchannel.nChannelID)
        guard (myuseraccount.uUserRights & USERRIGHT_KICK_USERS.rawValue) != 0 || op else { return }
        cmdid = TeamTalkClient.shared.kickUser(id: userid, fromChannelID: curchannel.nChannelID)
        activeCommands[cmdid] = .kickCmd
    }

    func banUser(userid: INT32) {
        let op = TeamTalkClient.shared.isChannelOperator(channelID: curchannel.nChannelID)
        guard (myuseraccount.uUserRights & USERRIGHT_BAN_USERS.rawValue) != 0 || op else { return }
        cmdid = TeamTalkClient.shared.banUser(id: userid, fromChannelID: curchannel.nChannelID)
        activeCommands[cmdid] = .banCmd
        cmdid = TeamTalkClient.shared.kickUser(id: userid, fromChannelID: curchannel.nChannelID)
        activeCommands[cmdid] = .kickCmd
    }

    func moveIntoChannel(channelID: INT32) {
        guard !moveusers.isEmpty else {
            announceForAccessibility(String(localized: "No users selected to move", comment: "channel list"))
            return
        }

        for userid in moveusers {
            cmdid = TeamTalkClient.shared.moveUser(id: userid, toChannelID: channelID)
            activeCommands[cmdid] = .moveCmd
        }
        moveusers.removeAll()
        refreshChannelList()
    }

    func isMoveUserSelected(userid: INT32) -> Bool {
        moveusers.contains(userid)
    }

    func moveUserActionTitle(userid: INT32) -> String {
        if isMoveUserSelected(userid: userid) {
            return String(localized: "Deselect user", comment: "channel list")
        }
        return String(localized: "Move user", comment: "channel list")
    }

    /*func moveUserAccessibilityValue(userid: INT32) -> String {
        if isMoveUserSelected(userid: userid) {
            return String(localized: "Selected for moving", comment: "channel list")
        }
        return String(localized: "Not selected for moving", comment: "channel list")
    }*/

    func moveDestinationAccessibilityHint() -> String {
        switch moveusers.count {
        case 0:
            return String(localized: "No users selected to move", comment: "channel list")
        case 1:
            return String(localized: "1 user selected to move here", comment: "channel list")
        default:
            return String(
                format: String(localized: "%d users selected to move here", comment: "channel list"),
                moveusers.count
            )
        }
    }

    // MARK: - Navigation

    func showUserDetail(userid: INT32) {
        let user = TeamTalkClient.shared.withUser(id: userid) { $0 }
        let model = UserDetailModel(user: user)
        addToTTMessages(model)
        navigationPath.append(.userDetail(model))
    }

    func showChannelDetail(channelID: INT32) {
        guard var channel = channels[channelID] else { return }
        if TeamTalkString.channel(.password, from: channel).isEmpty {
            if let passwd = chanpasswds[channelID] {
                TeamTalkString.setChannel(.password, on: &channel, to: passwd)
            }
        }
        let model = ChannelDetailModel(channel: channel)
        addToTTMessages(model)
        channelDetailModel = model
    }

    func showNewChannel() {
        var newChannel = Channel()
        newChannel.nParentID = curchannel.nChannelID
        if newChannel.nParentID == 0 {
            let subchans = channels.values.filter { $0.nParentID == 0 }
            if let root = subchans.first {
                newChannel.nParentID = root.nChannelID
            }
        }
        let model = ChannelDetailModel(channel: newChannel)
        addToTTMessages(model)
        channelDetailModel = model
    }

    func showTextMessages(userid: INT32) {
        let model = makeTextMessageModel(userid: userid)
        currentTextMessageModel = model
        navigationPath.append(.textMessage(model))
    }

    private func makeTextMessageModel(userid: INT32) -> TextMessageModel {
        let model = TextMessageModel(
            userid: userid,
            title: String(localized: "Private Text Message", comment: "text message navigation title")
        )
        openTextMessages(model)
        return model
    }

    func openTextMessages(_ model: TextMessageModel) {
        model.delegate = self
        addToTTMessages(model)
        if let msgs = textmessages[model.userid] {
            for m in msgs { model.appendEventMessage(m) }
        }
    }

    // MARK: - PTT

    func txBtnDown() {
        if hasPTTLock() {
            enableVoiceTx(true)
        } else {
            enableVoiceTx(!TeamTalkClient.shared.isVoiceTransmitting)
        }
    }

    func enableVoiceTx(_ enable: Bool) {
        TeamTalkClient.shared.enableVoiceTransmission(enable)
        playSound(enable ? .tx_ON : .tx_OFF)
        updateTX()
    }

    func txBtnUp() {
        if hasPTTLock() {
            let now = Date()
            if (pttLockTimeout as NSDate).earlierDate(now) == now {
                enableVoiceTx(true)
            } else {
                enableVoiceTx(false)
            }
            pttLockTimeout = now.addingTimeInterval(0.5)
        }
    }

    func txBtnAccessibilityAction() {
        enableVoiceTx(!TeamTalkClient.shared.isVoiceTransmitting)
    }

    func updateTX() {
        isTransmitting = TeamTalkClient.shared.isVoiceTransmitting
        pttHint = hasPTTLock()
            ? String(localized: "Double tap and hold to transmit. Triple tap fast to lock transmission.", comment: "channel list")
            : String(localized: "Toggle to enable/disable transmission", comment: "channel list")
        refreshChannelList()
    }

    func timerUnreadBlinker() {
        refreshChannelList()
        if unreadmessages.isEmpty {
            unreadTimer?.invalidate()
        }
    }

    // MARK: - Command completion

    func commandComplete(_ active_cmdid: INT32) {
        if let cmd = activeCommands[active_cmdid] {
            switch cmd {
            case .loginCmd:
                if TeamTalkClient.shared.isAuthorized {
                    if rejoinchannel.nChannelID > 0 {
                        let passwd = chanpasswds[rejoinchannel.nChannelID]
                            ?? TeamTalkString.channel(.password, from: rejoinchannel)
                        if chanpasswds[rejoinchannel.nChannelID] == nil {
                            chanpasswds[rejoinchannel.nChannelID] = passwd
                        }
                        TeamTalkString.setChannel(.password, on: &rejoinchannel, to: passwd)
                        cmdid = TeamTalkClient.shared.join(channel: &rejoinchannel)
                        activeCommands[cmdid] = .joinCmd
                    } else if !TeamTalkString.channel(.name, from: rejoinchannel).isEmpty {
                        let passwd = TeamTalkString.channel(.password, from: rejoinchannel)
                        TeamTalkString.setChannel(.password, on: &rejoinchannel, to: passwd)
                        cmdid = TeamTalkClient.shared.join(channel: &rejoinchannel)
                        activeCommands[cmdid] = .joinCmd
                    } else if UserDefaults.standard.object(forKey: PREF_JOINROOTCHANNEL) == nil
                        || UserDefaults.standard.bool(forKey: PREF_JOINROOTCHANNEL) {
                        cmdid = TeamTalkClient.shared.joinChannel(id: TeamTalkClient.shared.rootChannelID)
                        activeCommands[cmdid] = .joinCmd
                    }
                }
            case .kickCmd, .joinCmd, .banCmd, .moveCmd:
                break
            }
            activeCommands.removeValue(forKey: active_cmdid)
        }
        refreshChannelList()
    }

    // MARK: - Audio config

    func updateAudioConfig() {
        if mychannel.audiocfg.bEnableAGC == TRUE {
            TeamTalkClient.shared.setSoundInputGainLevel(INT32(SOUND_GAIN_DEFAULT.rawValue))
            var ap = TeamTalkAudioPreprocessor.makeWebRTCPreprocessor()
            let gain = Float(mychannel.audiocfg.nGainLevel) / Float(TeamTalkAudioPreprocessor.channelAudioConfigMax)
            ap.webrtc.gaincontroller2.fixeddigital.fGainDB = WEBRTC_GAINCONTROLLER2_FIXEDGAIN_MAX * gain
            ap.webrtc.gaincontroller2.bEnable = TRUE
            TeamTalkClient.shared.setSoundInputPreprocess(&ap)
        } else {
            var ap = TeamTalkAudioPreprocessor.makeTeamTalkPreprocessor()
            TeamTalkClient.shared.setSoundInputPreprocess(&ap)
            let vol = UserDefaults.standard.integer(forKey: PREF_MICROPHONE_GAIN)
            TeamTalkClient.shared.setSoundInputGainLevel(INT32(refVolume(Double(vol))))
        }
    }
}

// MARK: - MyTextMessageDelegate

extension ChannelListModel: MyTextMessageDelegate {
    func appendTextMessage(_ userid: INT32, txtmsg: MyTextMessage) {
        if textmessages[userid] == nil {
            textmessages[userid] = [MyTextMessage]()
        }
        textmessages[userid]!.append(txtmsg)
        if textmessages[userid]!.count > MAX_TEXTMESSAGES {
            textmessages[userid]!.removeFirst()
        }
    }
}

// MARK: - TeamTalkEvent

extension ChannelListModel: TeamTalkEvent {
    func handleTTMessage(_ m: TTMessage) {
        switch m.nClientEvent {

        case CLIENTEVENT_CON_LOST:
            channels.removeAll()
            users.removeAll()
            curchannel = Channel()
            mychannel = Channel()
            activeCommands.removeAll()
            refreshChannelList()

        case CLIENTEVENT_CMD_PROCESSING:
            if TeamTalkMessagePayload.isActive(m) {
                currentCmdId = m.nSource
            } else {
                currentCmdId = 0
                commandComplete(m.nSource)
            }

        case CLIENTEVENT_CMD_ERROR:
            if let cmd = activeCommands[m.nSource], cmd != .loginCmd {
                errorMessage = TeamTalkString.clientError(TeamTalkMessagePayload.clientError(from: m))
            }

        case CLIENTEVENT_CMD_SERVER_UPDATE:
            srvprop = TeamTalkMessagePayload.serverProperties(from: m)

        case CLIENTEVENT_CMD_MYSELF_LOGGEDIN:
            myuseraccount = TeamTalkMessagePayload.userAccount(from: m)
            if (myuseraccount.uUserType & USERTYPE_ADMIN.rawValue) != 0 {
                myuseraccount.uUserRights = 0xFFFFFFFF
            }

        case CLIENTEVENT_CMD_CHANNEL_NEW:
            let channel = TeamTalkMessagePayload.channel(from: m)
            channels[channel.nChannelID] = channel
            if channel.nParentID == 0 { updateTitle() }
            if currentCmdId == 0 { refreshChannelList() }

        case CLIENTEVENT_CMD_CHANNEL_UPDATE:
            let channel = TeamTalkMessagePayload.channel(from: m)
            channels[channel.nChannelID] = channel
            if mychannel.nChannelID == channel.nChannelID {
                let myuserid = TeamTalkClient.shared.myUserID
                if channel.transmitUsersQueue.0 == myuserid && mychannel.transmitUsersQueue.0 != myuserid {
                    playSound(.transmit_ON)
                }
                if mychannel.transmitUsersQueue.0 == myuserid && channel.transmitUsersQueue.0 != myuserid {
                    playSound(.transmit_OFF)
                }
                mychannel = channel
                updateAudioConfig()
            }
            if currentCmdId == 0 { refreshChannelList() }

        case CLIENTEVENT_CMD_CHANNEL_REMOVE:
            let channel = TeamTalkMessagePayload.channel(from: m)
            channels.removeValue(forKey: channel.nChannelID)
            if currentCmdId == 0 { refreshChannelList() }

        case CLIENTEVENT_CMD_USER_LOGGEDIN:
            playSound(.logged_IN)
            let user = TeamTalkMessagePayload.user(from: m)
            users[user.nUserID] = user
            if currentCmdId == 0 {
                if user.nChannelID == curchannel.nChannelID { refreshChannelList() }
                if TeamTalkClient.shared.myUserID != user.nUserID {
                    let defaults = UserDefaults.standard
                    if defaults.object(forKey: PREF_TTSEVENT_USERLOGIN) != nil && defaults.bool(forKey: PREF_TTSEVENT_USERLOGIN) {
                        newUtterance(getDisplayName(user) + " " + String(localized: "has logged on", comment: "TTS EVENT"))
                    }
                }
            }

        case CLIENTEVENT_CMD_USER_LOGGEDOUT:
            playSound(.logged_OUT)
            let user = TeamTalkMessagePayload.user(from: m)
            users.removeValue(forKey: user.nUserID)
            if currentCmdId == 0 {
                if user.nChannelID == curchannel.nChannelID { refreshChannelList() }
                if TeamTalkClient.shared.myUserID != user.nUserID {
                    let defaults = UserDefaults.standard
                    if defaults.object(forKey: PREF_TTSEVENT_USERLOGOUT) != nil && defaults.bool(forKey: PREF_TTSEVENT_USERLOGOUT) {
                        newUtterance(getDisplayName(user) + " " + String(localized: "has logged out", comment: "TTS EVENT"))
                    }
                }
            }

        case CLIENTEVENT_CMD_USER_JOINED:
            let user = TeamTalkMessagePayload.user(from: m)
            users[user.nUserID] = user
            if user.nUserID == TeamTalkClient.shared.myUserID, let joinedChannel = channels[user.nChannelID] {
                curchannel = joinedChannel
                mychannel = joinedChannel
                if rejoinchannel.nChannelID == 0 && chanpasswds[user.nChannelID] == nil {
                    chanpasswds[user.nChannelID] = TeamTalkString.channel(.password, from: rejoinchannel)
                }
                rejoinchannel = joinedChannel
                updateTitle()
                updateAudioConfig()
            }
            if user.nChannelID == mychannel.nChannelID && mychannel.nChannelID > 0 {
                playSound(.joined_CHAN)
                let defaults = UserDefaults.standard
                if defaults.object(forKey: PREF_TTSEVENT_JOINEDCHAN) == nil || defaults.bool(forKey: PREF_TTSEVENT_JOINEDCHAN) {
                    newUtterance(getDisplayName(user) + " " + String(localized: "has joined the channel", comment: "TTS EVENT"))
                }
            }
            if currentCmdId == 0 { refreshChannelList() }

        case CLIENTEVENT_CMD_USER_UPDATE:
            let user = TeamTalkMessagePayload.user(from: m)
            users[user.nUserID] = user
            if currentCmdId == 0 { refreshChannelList() }

        case CLIENTEVENT_CMD_USER_LEFT:
            let user = TeamTalkMessagePayload.user(from: m)
            if myuseraccount.uUserRights & USERRIGHT_VIEW_ALL_USERS.rawValue == 0 {
                users.removeValue(forKey: user.nUserID)
            } else {
                users[user.nUserID] = user
            }
            if user.nUserID == TeamTalkClient.shared.myUserID {
                mychannel = Channel()
                rejoinchannel = Channel()
            }
            if m.nSource == mychannel.nChannelID && mychannel.nChannelID > 0 {
                playSound(.left_CHAN)
                let defaults = UserDefaults.standard
                if defaults.object(forKey: PREF_TTSEVENT_LEFTCHAN) == nil || defaults.bool(forKey: PREF_TTSEVENT_LEFTCHAN) {
                    newUtterance(getDisplayName(user) + " " + String(localized: "has left the channel", comment: "TTS EVENT"))
                }
            }
            if currentCmdId == 0 { refreshChannelList() }

        case CLIENTEVENT_CMD_USER_TEXTMSG:
            let txtmsg = TeamTalkMessagePayload.textMessage(from: m)
            if txtmsg.nMsgType == MSGTYPE_USER {
                let settings = UserDefaults.standard
                if let user = users[txtmsg.nFromUserID] {
                    let name = getDisplayName(user)
                    let newmsg = MyTextMessage(
                        m: txtmsg,
                        nickname: name,
                        msgtype: TeamTalkClient.shared.myUserID == txtmsg.nFromUserID ? .PRIV_IM_MYSELF : .PRIV_IM
                    )
                    appendTextMessage(txtmsg.nFromUserID, txtmsg: newmsg)
                    if unreadmessages.isEmpty {
                        unreadTimer = Timer.scheduledTimer(withTimeInterval: 1.0, repeats: true) { [weak self] _ in
                            self?.timerUnreadBlinker()
                        }
                    }
                    unreadmessages.insert(txtmsg.nFromUserID)
                }

                // Don't auto-navigate if the private chat for this user is already visible
                if currentTextMessageModel?.userid == txtmsg.nFromUserID {
                    break
                }

                if settings.object(forKey: PREF_DISPLAY_POPUPTXTMSG) == nil || settings.bool(forKey: PREF_DISPLAY_POPUPTXTMSG) {
                    let model = makeTextMessageModel(userid: txtmsg.nFromUserID)
                    currentTextMessageModel = model
                    navigationPath.append(.textMessage(model))
                    if let msg = model.getLastEventMessage() {
                        speakTextMessage(txtmsg.nMsgType, mymsg: msg)
                    }
                }
            }

        case CLIENTEVENT_USER_STATECHANGE:
            let user = TeamTalkMessagePayload.user(from: m)
            users[user.nUserID] = user
            refreshChannelList()

        case CLIENTEVENT_VOICE_ACTIVATION:
            refreshChannelList()

        default:
            break
        }
    }
}
