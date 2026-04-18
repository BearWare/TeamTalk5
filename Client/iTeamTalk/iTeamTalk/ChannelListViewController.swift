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
import UIKit

enum ChannelListRow: Identifiable {
    case join
    case user(User)
    case channel(Channel)

    var id: String {
        switch self {
        case .join:
            return "join"
        case .user(let user):
            return "user-\(user.nUserID)"
        case .channel(let channel):
            return "channel-\(channel.nChannelID)"
        }
    }
}

final class ChannelListModel: ObservableObject {
    @Published var rows: [ChannelListRow] = []
    @Published var isTransmitting: Bool = false
    @Published var pttHint: String = NSLocalizedString("Toggle to enable/disable transmission", comment: "channel list")
}

struct ChannelListContainerView: View {
    @ObservedObject var model: ChannelListModel
    let selectRow: (ChannelListRow) -> Void
    let joinCurrentChannel: () -> Void
    let messageUser: (INT32) -> Void
    let editChannel: (INT32) -> Void
    let userDetails: (User) -> ChannelUserDetails
    let channelDetails: (Channel) -> ChannelDisplayDetails
    let onPTTDown: () -> Void
    let onPTTUp: () -> Void

    var body: some View {
        VStack(spacing: 0) {
            ChannelListView(
                model: model,
                selectRow: selectRow,
                joinCurrentChannel: joinCurrentChannel,
                messageUser: messageUser,
                editChannel: editChannel,
                userDetails: userDetails,
                channelDetails: channelDetails
            )

            Button {} label: {
                Text(NSLocalizedString("Talk", comment: "PTT button"))
                    .frame(maxWidth: .infinity)
                    .frame(height: 50)
                    .background(model.isTransmitting ? Color.red : Color.green)
                    .foregroundStyle(.white)
                    .fontWeight(.semibold)
            }
            .simultaneousGesture(
                DragGesture(minimumDistance: 0)
                    .onChanged { _ in onPTTDown() }
                    .onEnded { _ in onPTTUp() }
            )
            .accessibilityLabel(NSLocalizedString("Push to Talk", comment: "PTT button"))
            .accessibilityHint(model.pttHint)
            .accessibilityValue(model.isTransmitting
                ? NSLocalizedString("Active", comment: "PTT button")
                : NSLocalizedString("Inactive", comment: "PTT button"))
        }
    }
}

final class ChannelListViewController: UIHostingController<ChannelListContainerView>, MyTextMessageDelegate, TeamTalkEvent {

    // all channels on server
    var channels = [INT32 : Channel]()
    // channel passwords
    var chanpasswds = [INT32 : String]()
    // the channel being displayed (not nescessarily the same channel as we're in)
    var curchannel = Channel()
    // joined channel (the channel we're in)
    var mychannel = Channel()
    // channel to join if connection is lost
    var rejoinchannel = Channel()
    // all users on server
    var users = [INT32 : User]()
    // users which should be moved
    var moveusers = [INT32]()
    // the ID of the command for which we're expecting a result
    var cmdid : INT32 = 0
    // the command ID which is currently processing
    var currentCmdId : INT32 = 0
    // the commands awaiting reponse
    var activeCommands = [INT32: Command]()
    // properties of connected server
    var srvprop = ServerProperties()
    // local instance's user account
    var myuseraccount = UserAccount()
    // user to user text messages
    var textmessages = [INT32 : [MyTextMessage] ]()
    // timer for blinking unread messages
    var unreadTimer : Timer?
    // users being displayed in table-view
    var displayUsers = [User]()
    // channels (subchannels) being displayed in table-view
    var displayChans = [Channel]()
    // timeout for PTT lock
    var pttLockTimeout = Date()

    private var channelListModel: ChannelListModel

    init() {
        let model = ChannelListModel()
        self.channelListModel = model
        super.init(rootView: ChannelListContainerView(
            model: model,
            selectRow: { _ in },
            joinCurrentChannel: { },
            messageUser: { _ in },
            editChannel: { _ in },
            userDetails: { _ in ChannelUserDetails(title: "", subtitle: nil, iconName: "", iconAccessibilityLabel: "", messageIconName: "") },
            channelDetails: { _ in ChannelDisplayDetails(title: "", subtitle: nil, iconName: "", iconAccessibilityLabel: "", actionTitle: "", isParent: false) },
            onPTTDown: { },
            onPTTUp: { }
        ))
    }

    required init?(coder: NSCoder) { fatalError("use init()") }

    override func viewDidLoad() {
        super.viewDidLoad()

        rootView = ChannelListContainerView(
            model: channelListModel,
            selectRow: { [weak self] row in self?.selectRow(row) },
            joinCurrentChannel: { [weak self] in
                guard let self else { return }
                self.joinNewChannel(self.curchannel)
            },
            messageUser: { [weak self] userID in
                self?.showTextMessages(userid: userID)
            },
            editChannel: { [weak self] channelID in
                self?.showChannelDetail(channelID: channelID)
            },
            userDetails: { [weak self] user in
                self?.userDetails(user) ?? ChannelUserDetails(
                    title: getDisplayName(user),
                    subtitle: TeamTalkString.user(.statusMessage, from: user),
                    iconName: "man_blue.png",
                    iconAccessibilityLabel: NSLocalizedString("Silent", comment: "channel list"),
                    messageIconName: "message_blue"
                )
            },
            channelDetails: { [weak self] channel in
                self?.channelDetails(channel) ?? ChannelDisplayDetails(
                    title: TeamTalkString.channel(.name, from: channel),
                    subtitle: TeamTalkString.channel(.topic, from: channel),
                    iconName: "channel_orange.png",
                    iconAccessibilityLabel: NSLocalizedString("Channel", comment: "channel list"),
                    actionTitle: NSLocalizedString("View", comment: "channel list"),
                    isParent: false
                )
            },
            onPTTDown: { [weak self] in self?.txBtnDown() },
            onPTTUp: { [weak self] in self?.txBtnUp() }
        )

        refreshChannelList()
        updateTX()
    }

    private func refreshChannelList() {
        updateDisplayItems()
        channelListModel.rows = displayRows()
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        updateTX()
    }
    
    override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)
        
    }
    
    deinit {
        for (_, user) in users {
            syncToUserCache(user: user)
        }
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    func joinNewChannel(_ channel: Channel) {
        if channel.bPassword == TRUE {
            
            var passwdField : UITextField?
            
            let passwdAlert = UIAlertController(title: NSLocalizedString("Enter Password", comment: "Dialog message"),
                                                message: NSLocalizedString("Password", comment: "Dialog message"),
                                                preferredStyle: .alert)
            passwdAlert.addTextField {(textField) in
                passwdField = textField
                passwdField?.autocorrectionType = .no
                passwdField?.spellCheckingType = .no
                passwdField?.autocapitalizationType = .none
                passwdField?.text = TeamTalkString.channel(.password, from: channel)
            }
            
            passwdAlert.addAction(UIAlertAction(title: NSLocalizedString("Cancel", comment: "Dialog message"), style: .cancel, handler: { (pAction) in
                passwdAlert.dismiss(animated: true, completion: nil)
            }))
            
            passwdAlert.addAction(UIAlertAction(title: NSLocalizedString("Join", comment: "Dialog message"),
                                                style: .default, handler: { (pAction) in

                                                    let passwd = passwdField?.text ?? ""
                                                    self.chanpasswds[channel.nChannelID] = passwd
                                                    self.cmdid = TeamTalkClient.shared.joinChannel(id: channel.nChannelID, password: passwd)
                                                    self.activeCommands[self.cmdid] = .joinCmd

                                                    passwdAlert.dismiss(animated: true, completion: nil)
            }))
            
            self.present(passwdAlert, animated: true, completion: nil)
        }
        else {
            cmdid = TeamTalkClient.shared.joinChannel(id: channel.nChannelID)
            activeCommands[cmdid] = .joinCmd
        }
        
    }
    
    func appendTextMessage(_ userid: INT32, txtmsg: MyTextMessage) {
        
        if textmessages[userid] == nil {
            textmessages[userid] = [MyTextMessage]()
        }
        textmessages[userid]!.append(txtmsg)
        
        if textmessages[userid]!.count > MAX_TEXTMESSAGES {
            textmessages[userid]!.removeFirst()
        }
    }
    
    func getUsersCount(_ chanid: INT32) -> Int {
        
        var count = users.values.filter({$0.nChannelID == chanid}).count
        
        for c in channels.values.filter({$0.nParentID == chanid}) {
            count += getUsersCount(c.nChannelID)
        }
        
        return count
    }
    
    func updateDisplayItems()  {
        let subchans : [Channel] = channels.values.filter({$0.nParentID == self.curchannel.nChannelID})
        let chanusers : [User] = users.values.filter({$0.nChannelID == self.curchannel.nChannelID})
        
        let settings = UserDefaults.standard
        let chansort = settings.object(forKey: PREF_DISPLAY_SORTCHANNELS) == nil ? ChanSort.ASCENDING.rawValue : settings.integer(forKey: PREF_DISPLAY_SORTCHANNELS)
        switch chansort {
        case ChanSort.POPULARITY.rawValue :
            displayChans = subchans.sorted() {
                let aid = $0.nChannelID
                let bid = $1.nChannelID
                let au = users.values.filter({$0.nChannelID == aid})
                let bu = users.values.filter({$0.nChannelID == bid})
                let aname = TeamTalkString.channel(.name, from: $0)
                let bname = TeamTalkString.channel(.name, from: $1)
                return au.count == bu.count ?
                    aname.caseInsensitiveCompare(bname) == ComparisonResult.orderedAscending : au.count > bu.count
            }
        case ChanSort.ASCENDING.rawValue :
            fallthrough
        default :
            displayChans = subchans.sorted() {
                let aname = TeamTalkString.channel(.name, from: $0)
                let bname = TeamTalkString.channel(.name, from: $1)
                return aname.caseInsensitiveCompare(bname) == ComparisonResult.orderedAscending
            }
        }
        displayUsers = chanusers.sorted() {
            getDisplayName($0).caseInsensitiveCompare(getDisplayName($1)) == ComparisonResult.orderedAscending
        }
    }
    
    private func displayRows() -> [ChannelListRow] {
        let show_join = curchannel.nChannelID != mychannel.nChannelID && curchannel.nChannelID > 0
        let show_parent = curchannel.nParentID != 0

        var rows = [ChannelListRow]()
        if show_join {
            rows.append(.join)
        }

        for user in displayUsers {
            rows.append(.user(user))
        }

        if show_parent, let channel = channels[curchannel.nParentID] {
            rows.append(.channel(channel))
        }

        for channel in displayChans {
            rows.append(.channel(channel))
        }

        return rows
    }

    private func userDetails(_ user: User) -> ChannelUserDetails {
        let female = (UInt(user.nStatusMode) & StatusMode.STATUSMODE_FEMALE.rawValue) != 0
        let isTalking = user.uUserState & USERSTATE_VOICE.rawValue != 0 ||
            (TeamTalkClient.shared.myUserID == user.nUserID && TeamTalkClient.shared.isVoiceTransmitting)
        let iconName = isTalking ? (female ? "woman_green.png" : "man_green.png") : (female ? "woman_blue.png" : "man_blue.png")
        let iconAccessibilityLabel = isTalking ? NSLocalizedString("Talking", comment: "channel list") : NSLocalizedString("Silent", comment: "channel list")
        let messageIcon = unreadmessages.contains(user.nUserID) && Int(Date().timeIntervalSince1970) % 2 == 0 ? "message_red" : "message_blue"
        return ChannelUserDetails(
            title: getDisplayName(user),
            subtitle: TeamTalkString.user(.statusMessage, from: user),
            iconName: iconName,
            iconAccessibilityLabel: iconAccessibilityLabel,
            messageIconName: messageIcon
        )
    }

    private func userActions(for user: User) -> [MyCustomAction] {
        let tag = Int(user.nUserID)
        let action_msg = MyCustomAction(name: NSLocalizedString("Send private message", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.messageUser(_:)), tag: tag)
        let action_mute = MyCustomAction(name: NSLocalizedString("Mute", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.muteUser(_:)), tag: tag)

        var actions = [action_msg, action_mute]

        if (myuseraccount.uUserRights & USERRIGHT_MOVE_USERS.rawValue) != 0 {
            let action_move = MyCustomAction(name: NSLocalizedString("Move user", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.moveUser(_:)), tag: tag)
            actions.append(action_move)
        }

        let op = TeamTalkClient.shared.isChannelOperator(channelID: user.nChannelID)
        if (myuseraccount.uUserRights & USERRIGHT_KICK_USERS.rawValue) != 0 || op {
            let action_kick = MyCustomAction(name: NSLocalizedString("Kick user", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.kickUser(_:)), tag: tag)
            actions.append(action_kick)
        }

        if (myuseraccount.uUserRights & USERRIGHT_BAN_USERS.rawValue) != 0 || op {
            let action_ban = MyCustomAction(name: NSLocalizedString("Ban user", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.banUser(_:)), tag: tag)
            actions.append(action_ban)
        }

        return actions
    }

    private func channelActions(for channel: Channel) -> [MyCustomAction] {
        let tag = Int(channel.nChannelID)
        var actions = [MyCustomAction]()

        if moveusers.count > 0 {
            let action_move = MyCustomAction(name: NSLocalizedString("Move users here", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.moveIntoChannel(_:)), tag: tag)
            actions.append(action_move)
        }

        if channel.nChannelID != mychannel.nChannelID {
            let action_join = MyCustomAction(name: NSLocalizedString("Join channel", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.joinThisChannel(_:)), tag: tag)
            actions.append(action_join)
        }

        let op = TeamTalkClient.shared.isChannelOperator(channelID: channel.nChannelID)
        if (myuseraccount.uUserRights & USERRIGHT_MODIFY_CHANNELS.rawValue) != 0 || op {
            let action_edit = MyCustomAction(name: NSLocalizedString("Edit properties", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.editChannel(_:)), tag: tag)
            actions.append(action_edit)
        } else {
            let action_view = MyCustomAction(name: NSLocalizedString("View properties", comment: "channel list"), target: self, selector: #selector(ChannelListViewController.editChannel(_:)), tag: tag)
            actions.append(action_view)
        }

        return actions
    }

    private func channelDetails(_ channel: Channel) -> ChannelDisplayDetails {
        let op = TeamTalkClient.shared.isChannelOperator(channelID: channel.nChannelID)
        let canEdit = (myuseraccount.uUserRights & USERRIGHT_MODIFY_CHANNELS.rawValue) != 0 || op
        let actionTitle = canEdit ? NSLocalizedString("Edit", comment: "channel list") : NSLocalizedString("View", comment: "channel list")

        if curchannel.nChannelID == 0 {
            let iconName = channel.bPassword != 0 ? "channel_pink.png" : "channel_orange.png"
            let iconAccessibilityLabel = channel.bPassword != 0 ? NSLocalizedString("Password protected", comment: "channel list") : NSLocalizedString("No password", comment: "channel list")
            return ChannelDisplayDetails(title: TeamTalkString.serverProperties(.name, from: srvprop), subtitle: TeamTalkString.channel(.topic, from: channel), iconName: iconName, iconAccessibilityLabel: iconAccessibilityLabel, actionTitle: actionTitle, isParent: false)
        }

        if channel.nChannelID == curchannel.nParentID {
            let subtitle: String
            if channel.nParentID == 0 {
                subtitle = TeamTalkString.serverProperties(.name, from: srvprop)
            } else {
                subtitle = TeamTalkString.channel(.name, from: channel)
            }
            return ChannelDisplayDetails(title: NSLocalizedString("Parent channel", comment: "channel list"),
                                         subtitle: subtitle,
                                         iconName: "back_orange.png",
                                         iconAccessibilityLabel: NSLocalizedString("Return to previous channel", comment: "channel list"),
                                         actionTitle: actionTitle,
                                         isParent: true)
        }

        let userCount = getUsersCount(channel.nChannelID)
        let iconName = channel.bPassword != 0 ? "channel_pink.png" : "channel_orange.png"
        let iconAccessibilityLabel = String(format: NSLocalizedString("Channel. %d users", comment: "channel list"), userCount)
        return ChannelDisplayDetails(title: TeamTalkString.channel(.name, from: channel) + " (\(userCount))",
                                     subtitle: TeamTalkString.channel(.topic, from: channel),
                                     iconName: iconName,
                                     iconAccessibilityLabel: iconAccessibilityLabel,
                                     actionTitle: actionTitle,
                                     isParent: false)
    }

    private func selectRow(_ row: ChannelListRow) {
        switch row {
        case .join:
            joinNewChannel(curchannel)
        case .user(let user):
            showUserDetail(userid: user.nUserID)
        case .channel(let channel):
            curchannel = channel
            refreshChannelList()
            updateTitle()
        }
    }
    
    func updateTitle() {
        var title = ""
        if curchannel.nParentID == 0 {
            title = TeamTalkString.serverProperties(.name, from: srvprop)
        }
        else {
            title = TeamTalkString.channel(.name, from: curchannel)
        }
        
        self.navigationItem.title = title
    }
    
    @objc
    func messageUser(_ action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            showTextMessages(userid: INT32(ac.tag))
        }
        return true
    }

    @objc
    func muteUser(_ action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            let userid = INT32(ac.tag)
            if let user = users[userid] {
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
                // tell TeamTalk event loop to send us an updated User-struct
                TeamTalkClient.shared.pump(CLIENTEVENT_USER_STATECHANGE, source: userid)
            }
            
        }
        return true
    }

    @objc
    func moveUser(_ action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            moveusers.append(INT32(ac.tag))
            self.refreshChannelList() //need to update accessible actions on channels
        }
        return true
    }

    @objc
    func kickUser(_ action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            
            cmdid = TeamTalkClient.shared.kickUser(id: INT32(ac.tag), fromChannelID: curchannel.nChannelID)
            activeCommands[cmdid] = .kickCmd
        }
        return true
    }

    @objc
    func banUser(_ action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            
            cmdid = TeamTalkClient.shared.banUser(id: INT32(ac.tag), fromChannelID: curchannel.nChannelID)
            activeCommands[cmdid] = .banCmd
            cmdid = TeamTalkClient.shared.kickUser(id: INT32(ac.tag), fromChannelID: curchannel.nChannelID)
            activeCommands[cmdid] = .kickCmd
        }
        return true
    }

    @objc
    func joinThisChannel(_ action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            if let channel = channels[INT32(ac.tag)] {
                joinNewChannel(channel)
            }
        }
        return true
    }

    @objc
    func editChannel(_ action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            showChannelDetail(channelID: INT32(ac.tag))
        }
        return true
    }
    
    @objc
    func moveIntoChannel(_ action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            for userid in moveusers {
                cmdid = TeamTalkClient.shared.moveUser(id: userid, toChannelID: INT32(ac.tag))
                activeCommands[cmdid] = .moveCmd
            }
            moveusers.removeAll()
        }
        return true
    }
    
    func commandComplete(_ active_cmdid : INT32) {

        let cmd = activeCommands[active_cmdid]
        
        if cmd == nil {
            return
        }
        
        switch cmd! {
            
        case .loginCmd :
            if TeamTalkClient.shared.isAuthorized {
                
                if rejoinchannel.nChannelID > 0 {
                    // if we were previously in a channel then rejoin
                    let passwd = chanpasswds[rejoinchannel.nChannelID] != nil ? chanpasswds[rejoinchannel.nChannelID] : TeamTalkString.channel(.password, from: rejoinchannel)
                    if chanpasswds[rejoinchannel.nChannelID] == nil {
                        // if channel password is from initial login (Server-struct) then we need to store it
                       chanpasswds[rejoinchannel.nChannelID] = TeamTalkString.channel(.password, from: rejoinchannel)
                    }
                    TeamTalkString.setChannel(.password, on: &rejoinchannel, to: passwd!)
                    cmdid = TeamTalkClient.shared.join(channel: &rejoinchannel)
                    activeCommands[cmdid] = .joinCmd
                }
                else if TeamTalkString.channel(.name, from: rejoinchannel).isEmpty == false {
                    // join from initial login
                    let passwd = TeamTalkString.channel(.password, from: rejoinchannel)
                    TeamTalkString.setChannel(.password, on: &rejoinchannel, to: passwd)
                    cmdid = TeamTalkClient.shared.join(channel: &rejoinchannel)
                    activeCommands[cmdid] = .joinCmd
                }
                else if UserDefaults.standard.object(forKey: PREF_JOINROOTCHANNEL) == nil ||
                    UserDefaults.standard.bool(forKey: PREF_JOINROOTCHANNEL) {
                    //join root channel automatically (if enabled)
                    cmdid = TeamTalkClient.shared.joinChannel(id: TeamTalkClient.shared.rootChannelID)
                    activeCommands[cmdid] = .joinCmd
                }
            }
        case .kickCmd :
            fallthrough
        case .joinCmd :
            fallthrough
        case .banCmd :
            fallthrough
        case .moveCmd :
            break
//        default :
//            print("Command #\(active_cmdid) is not a completion command")
        }

        activeCommands.removeValue(forKey: active_cmdid)
        
        self.refreshChannelList()
    }
    
    private func showUserDetail(userid: INT32) {
        let vc = UserDetailViewController()
        vc.userid = userid
        navigationController?.pushViewController(vc, animated: true)
    }

    private func showChannelDetail(channelID: INT32) {
        guard let channel = channels[channelID] else { return }
        let vc = ChannelDetailViewController()
        vc.channel = channel
        if TeamTalkString.channel(.password, from: channel).isEmpty {
            if let passwd = chanpasswds[channelID] {
                TeamTalkString.setChannel(.password, on: &vc.channel, to: passwd)
            }
        }
        navigationController?.pushViewController(vc, animated: true)
    }

    func showNewChannelDetail() {
        let vc = ChannelDetailViewController()
        vc.channel.nParentID = curchannel.nChannelID
        if vc.channel.nParentID == 0 {
            let subchans = channels.values.filter({ $0.nParentID == 0 })
            if let root = subchans.first {
                vc.channel.nParentID = root.nChannelID
            }
        }
        navigationController?.pushViewController(vc, animated: true)
    }

    private func makeTextMessageViewController(userid: INT32) -> TextMessageViewController {
        let viewController = TextMessageViewController()
        openTextMessages(viewController, userid: userid)
        return viewController
    }

    private func showTextMessages(userid: INT32) {
        let viewController = makeTextMessageViewController(userid: userid)
        navigationController?.pushViewController(viewController, animated: true)
    }

    func openTextMessages(_ sender: TextMessageViewController, userid: INT32) {
        sender.userid = userid
        sender.delegate = self
        sender.navigationItem.title = NSLocalizedString("Private Text Message", comment: "text message navigation title")
        addToTTMessages(sender)
        if let msgs = self.textmessages[userid] {
            for m in msgs {
                sender.appendEventMessage(m)
            }
        }
    }
    
    func txBtnDown() {

        if hasPTTLock() {
            enableVoiceTx(true)
        }
        else {
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
            }
            else {
                enableVoiceTx(false)
            }
            // PTT lock is 0.5 sec
            pttLockTimeout = now.addingTimeInterval(0.5)
        }
    }
    
    func updateTX() {
        channelListModel.isTransmitting = TeamTalkClient.shared.isVoiceTransmitting
        channelListModel.pttHint = hasPTTLock()
            ? NSLocalizedString("Double tap and hold to transmit. Triple tap fast to lock transmission.", comment: "channel list")
            : NSLocalizedString("Toggle to enable/disable transmission", comment: "channel list")
        refreshChannelList()
    }
    
    @objc func timerUnreadBlinker(_ timer: Timer) {
        refreshChannelList()
        if unreadmessages.isEmpty {
            unreadTimer?.invalidate()
        }
    }
    
    func handleTTMessage(_ m: TTMessage) {
        switch(m.nClientEvent) {

        case CLIENTEVENT_CON_LOST :
            
            channels.removeAll()
            users.removeAll()
            curchannel = Channel()
            mychannel = Channel()
            activeCommands.removeAll()
            
            refreshChannelList()
            break
            
        case CLIENTEVENT_CMD_PROCESSING :
            if TeamTalkMessagePayload.isActive(m) {
                // command active
                self.currentCmdId = m.nSource
            }
            else {
                // command complete
                self.currentCmdId = 0
                
                commandComplete(m.nSource)
            }
        case CLIENTEVENT_CMD_ERROR :
            if activeCommands[m.nSource] != nil {
                let errmsg = TeamTalkString.clientError(TeamTalkMessagePayload.clientError(from: m))
                let alert = UIAlertController(title: NSLocalizedString("Error", comment: "Dialog"), message: errmsg, preferredStyle: UIAlertController.Style.alert)
                alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "Dialog"), style: UIAlertAction.Style.default, handler: nil))
                self.present(alert, animated: true, completion: nil)
            }
        case CLIENTEVENT_CMD_SERVER_UPDATE :
            srvprop = TeamTalkMessagePayload.serverProperties(from: m)
            
        case CLIENTEVENT_CMD_MYSELF_LOGGEDIN :
            myuseraccount = TeamTalkMessagePayload.userAccount(from: m)
            if (myuseraccount.uUserType & USERTYPE_ADMIN.rawValue) != 0 {
                // an admin user type can do everything
                myuseraccount.uUserRights = 0xFFFFFFFF
            }
            
        case CLIENTEVENT_CMD_CHANNEL_NEW :
            let channel = TeamTalkMessagePayload.channel(from: m)
            
            channels[channel.nChannelID] = channel
            
            // initial title will be server name
            if channel.nParentID == 0 {
                updateTitle()
            }
            
            if currentCmdId == 0 {
                self.refreshChannelList()
            }
            
        case CLIENTEVENT_CMD_CHANNEL_UPDATE :
            let channel = TeamTalkMessagePayload.channel(from: m)
            channels[channel.nChannelID] = channel
            
            if mychannel.nChannelID == channel.nChannelID {
                
                let myuserid = TeamTalkClient.shared.myUserID
                if channel.transmitUsersQueue.0 == myuserid &&
                    mychannel.transmitUsersQueue.0 != myuserid {
                    playSound(.transmit_ON)
                }
                
                if mychannel.transmitUsersQueue.0 == myuserid &&
                    channel.transmitUsersQueue.0 != myuserid {
                    playSound(.transmit_OFF)
                }
                
                mychannel = channel
                
                updateAudioConfig()
            }
            
            if currentCmdId == 0 {
                self.refreshChannelList()
            }
            
        case CLIENTEVENT_CMD_CHANNEL_REMOVE :
            let channel = TeamTalkMessagePayload.channel(from: m)
            channels.removeValue(forKey: channel.nChannelID)
            
            if currentCmdId == 0 {
                self.refreshChannelList()
            }
            
        case CLIENTEVENT_CMD_USER_LOGGEDIN :
            
            playSound(.logged_IN)
            
            let user = TeamTalkMessagePayload.user(from: m)
            users[user.nUserID] = user
            
            if currentCmdId == 0 {
                if user.nChannelID == curchannel.nChannelID {
                    self.refreshChannelList()
                }
                if TeamTalkClient.shared.myUserID != user.nUserID {
                    let defaults = UserDefaults.standard
                    
                    if defaults.object(forKey: PREF_TTSEVENT_USERLOGIN) != nil && defaults.bool(forKey: PREF_TTSEVENT_USERLOGIN) {
                        let name = getDisplayName(user)
                        newUtterance(name + " " + NSLocalizedString("has logged on", comment: "TTS EVENT"))
                    }
                }
            }
            
        case CLIENTEVENT_CMD_USER_LOGGEDOUT :
            
            playSound(.logged_OUT)
            
            let user = TeamTalkMessagePayload.user(from: m)
            users.removeValue(forKey: user.nUserID)

            if currentCmdId == 0 {
                if user.nChannelID == curchannel.nChannelID {
                    self.refreshChannelList()
                }
                if TeamTalkClient.shared.myUserID != user.nUserID {
                    let defaults = UserDefaults.standard
                    
                    if defaults.object(forKey: PREF_TTSEVENT_USERLOGOUT) != nil && defaults.bool(forKey: PREF_TTSEVENT_USERLOGOUT) {
                        let name = getDisplayName(user)
                        newUtterance(name + " " + NSLocalizedString("has logged out", comment: "TTS EVENT"))
                    }
                }
            }
            
        case CLIENTEVENT_CMD_USER_JOINED :
            let user = TeamTalkMessagePayload.user(from: m)
            users[user.nUserID] = user
            
            // we joined a new channel so update table view
            if user.nUserID == TeamTalkClient.shared.myUserID {
                curchannel = channels[user.nChannelID]!
                mychannel = channels[user.nChannelID]!

                //store password if it's from initial login (Server-struct)
                if rejoinchannel.nChannelID == 0 && chanpasswds[user.nChannelID] == nil {
                   chanpasswds[user.nChannelID] = TeamTalkString.channel(.password, from: rejoinchannel)
                }
                rejoinchannel = channels[user.nChannelID]! //join this on connection lost

                updateTitle()

                updateAudioConfig()
            }

            if user.nChannelID == mychannel.nChannelID && mychannel.nChannelID > 0 {
                playSound(.joined_CHAN)
                let defaults = UserDefaults.standard
                
                if defaults.object(forKey: PREF_TTSEVENT_JOINEDCHAN) == nil || defaults.bool(forKey: PREF_TTSEVENT_JOINEDCHAN) {
                    let name = getDisplayName(user)
                    newUtterance(name + " " +  NSLocalizedString("has joined the channel", comment: "TTS EVENT"))
                }
            }

            if currentCmdId == 0 {
                self.refreshChannelList()
            }
        case CLIENTEVENT_CMD_USER_UPDATE :
            let user = TeamTalkMessagePayload.user(from: m)
            users[user.nUserID] = user
            
            if currentCmdId == 0 {
                self.refreshChannelList()
            }
            
        case CLIENTEVENT_CMD_USER_LEFT :
            let user = TeamTalkMessagePayload.user(from: m)
            users[user.nUserID] = user
            
            if myuseraccount.uUserRights & USERRIGHT_VIEW_ALL_USERS.rawValue == 0 {
                users.removeValue(forKey: user.nUserID)
            }
            else {
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
                    let name = getDisplayName(user)
                    newUtterance(name + " " + NSLocalizedString("has left the channel", comment: "TTS EVENT"))
                }
            }
            
            if currentCmdId == 0 {
                self.refreshChannelList()
            }
            
        case CLIENTEVENT_CMD_USER_TEXTMSG :
            let txtmsg = TeamTalkMessagePayload.textMessage(from: m)
            
            if txtmsg.nMsgType == MSGTYPE_USER {
                
                let settings = UserDefaults.standard
                if let user = users[txtmsg.nFromUserID] {
                    let name = getDisplayName(user)
                    let newmsg = MyTextMessage(m: txtmsg, nickname: name,
                        msgtype: TeamTalkClient.shared.myUserID == txtmsg.nFromUserID ? .PRIV_IM_MYSELF : .PRIV_IM)
                    appendTextMessage(txtmsg.nFromUserID, txtmsg: newmsg)
                    
                    if unreadmessages.count == 0 {
                        self.unreadTimer = Timer.scheduledTimer(withTimeInterval: 1.0, repeats: true) { s in
                            self.timerUnreadBlinker(s)
                        }
                    }
                    unreadmessages.insert(txtmsg.nFromUserID)                    
                }
                
                //ignore incoming message if text message view controller is already open
                if self.navigationController?.topViewController is TextMessageViewController {
                    let vc = self.navigationController?.topViewController as! TextMessageViewController
                    if vc.userid == txtmsg.nFromUserID {
                        break
                    }
                }
                
                if settings.object(forKey: PREF_DISPLAY_POPUPTXTMSG) == nil || settings.bool(forKey: PREF_DISPLAY_POPUPTXTMSG) {
                    let vc = makeTextMessageViewController(userid: txtmsg.nFromUserID)
                    self.navigationController?.pushViewController(vc, animated: true)
                    if let m = vc.getLastEventMessage() {
                        speakTextMessage(txtmsg.nMsgType, mymsg: m)
                    }
                }
            }
            
        case CLIENTEVENT_CMD_ERROR :
            if m.nSource == cmdid {
                let errmsg = TeamTalkString.clientError(TeamTalkMessagePayload.clientError(from: m))
                let alert = UIAlertController(title: NSLocalizedString("Error", comment: "Dialog message"), message: errmsg, preferredStyle: UIAlertController.Style.alert)
                alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "Dialog message"), style: UIAlertAction.Style.default, handler: nil))
                self.present(alert, animated: true, completion: nil)
            }

        case CLIENTEVENT_USER_STATECHANGE :
            let user = TeamTalkMessagePayload.user(from: m)
            users[user.nUserID] = user
            self.refreshChannelList()
        
        case CLIENTEVENT_VOICE_ACTIVATION :
            
            refreshChannelList()

        default :
            //print("Unhandled message \(m.nClientEvent.rawValue)")
            break
        }
    }

    func updateAudioConfig() {
        if mychannel.audiocfg.bEnableAGC == TRUE {
            TeamTalkClient.shared.setSoundInputGainLevel(INT32(SOUND_GAIN_DEFAULT.rawValue))
            var ap = TeamTalkAudioPreprocessor.makeWebRTCPreprocessor()
            let gain = Float(mychannel.audiocfg.nGainLevel) / Float(TeamTalkAudioPreprocessor.channelAudioConfigMax)
            ap.webrtc.gaincontroller2.fixeddigital.fGainDB = WEBRTC_GAINCONTROLLER2_FIXEDGAIN_MAX * gain
            ap.webrtc.gaincontroller2.bEnable = TRUE
            TeamTalkClient.shared.setSoundInputPreprocess(&ap)
        }
        else {
            var ap = TeamTalkAudioPreprocessor.makeTeamTalkPreprocessor()
            TeamTalkClient.shared.setSoundInputPreprocess(&ap)
            
            let defaults = UserDefaults.standard
            let vol = defaults.integer(forKey: PREF_MICROPHONE_GAIN)
            TeamTalkClient.shared.setSoundInputGainLevel(INT32(refVolume(Double(vol))))
        }
    }
}

struct ChannelUserDetails {
    let title: String
    let subtitle: String?
    let iconName: String
    let iconAccessibilityLabel: String
    let messageIconName: String
}

struct ChannelDisplayDetails {
    let title: String
    let subtitle: String?
    let iconName: String
    let iconAccessibilityLabel: String
    let actionTitle: String
    let isParent: Bool
}

struct ChannelListView: View {
    @ObservedObject var model: ChannelListModel

    let selectRow: (ChannelListRow) -> Void
    let joinCurrentChannel: () -> Void
    let messageUser: (INT32) -> Void
    let editChannel: (INT32) -> Void
    let userDetails: (User) -> ChannelUserDetails
    let channelDetails: (Channel) -> ChannelDisplayDetails

    var body: some View {
        List(model.rows) { row in
            switch row {
            case .join:
                TeamTalkActionRow(
                    title: NSLocalizedString("Join this channel", comment: "channel list"),
                    action: joinCurrentChannel
                )

            case .user(let user):
                let details = userDetails(user)
                TeamTalkIconActionRow(
                    title: details.title,
                    subtitle: details.subtitle,
                    iconName: details.iconName,
                    iconAccessibilityLabel: details.iconAccessibilityLabel,
                    actionTitle: NSLocalizedString("Text Messaging", comment: "channel list"),
                    actionImageName: details.messageIconName
                ) {
                    messageUser(user.nUserID)
                }
                .contentShape(Rectangle())
                .onTapGesture {
                    selectRow(row)
                }

            case .channel(let channel):
                let details = channelDetails(channel)
                TeamTalkIconActionRow(
                    title: limitText(details.title),
                    subtitle: details.subtitle,
                    iconName: details.iconName,
                    iconAccessibilityLabel: details.iconAccessibilityLabel,
                    actionTitle: details.actionTitle,
                    isDimmed: details.isParent
                ) {
                    editChannel(channel.nChannelID)
                }
                .contentShape(Rectangle())
                .onTapGesture {
                    selectRow(row)
                }
            }
        }
    }
}
