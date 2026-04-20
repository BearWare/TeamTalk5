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
import OSLog
import SwiftUI
import TeamTalkKit
import UIKit

final class MainTabModel: ObservableObject, TeamTalkEvent {

    let channelListModel: ChannelListModel
    let channelChatModel: TextMessageModel
    let preferencesModel: PreferencesModel

    var server: Server
    var cmdid: INT32 = 0

    @Published var alertMessage: String?
    @Published var fatalAlertMessage: String?   // dismisses the view when OK tapped
    @Published var showSaveAlert = false

    private var pendingDismiss: (() -> Void)?
    private var polltimer: Timer?
    private var reconnecttimer: Timer?
    private var didSetup = false

    init(server: Server) {
        self.server = server
        channelListModel = ChannelListModel()
        channelChatModel = TextMessageModel(
            userid: 0,
            title: String(localized: "Messages", comment: "tab")
        )
        preferencesModel = PreferencesModel()
        channelListModel.openTextMessages(channelChatModel)
    }

    deinit {
        TeamTalkClient.shared.disconnect()
        closeSoundDevices()
        runTeamTalkEventHandler()
        print("Destroyed main view controller")
    }

    func setup() {
        guard !didSetup else { return }
        didSetup = true

        addToTTMessages(self)
        addToTTMessages(channelListModel)
        addToTTMessages(channelChatModel)
        addToTTMessages(preferencesModel)

        setupSoundDevices()

        let defaults = UserDefaults.standard
        if defaults.object(forKey: PREF_MASTER_VOLUME) != nil {
            let vol = defaults.integer(forKey: PREF_MASTER_VOLUME)
            TeamTalkClient.shared.setSoundOutputVolume(INT32(refVolume(Double(vol))))
        }
        if defaults.object(forKey: PREF_VOICEACTIVATION) != nil {
            let voiceact = defaults.integer(forKey: PREF_VOICEACTIVATION)
            if voiceact != VOICEACT_DISABLED {
                TeamTalkClient.shared.enableVoiceActivation(true)
                TeamTalkClient.shared.setVoiceActivationLevel(INT32(voiceact))
            }
        }
        if defaults.object(forKey: PREF_MICROPHONE_GAIN) != nil {
            let vol = defaults.integer(forKey: PREF_MICROPHONE_GAIN)
            TeamTalkClient.shared.setSoundInputGainLevel(INT32(refVolume(Double(vol))))
        }

        polltimer = Timer.scheduledTimer(
            timeInterval: 0.1,
            target: self,
            selector: #selector(timerEvent),
            userInfo: nil,
            repeats: true
        )

        let center = NotificationCenter.default
        center.addObserver(
            self, selector: #selector(proximityChanged(_:)),
            name: UIDevice.proximityStateDidChangeNotification,
            object: UIDevice.current
        )
        center.addObserver(
            self, selector: #selector(audioRouteChange(_:)),
            name: AVAudioSession.routeChangeNotification,
            object: nil
        )
        center.addObserver(
            self, selector: #selector(audioInterruption(_:)),
            name: AVAudioSession.interruptionNotification,
            object: nil
        )

        connectToServer()
    }

    func teardown() {
        polltimer?.invalidate()
        reconnecttimer?.invalidate()
        removeAllTTMessageHandlers()
        unreadmessages.removeAll()
        UIDevice.current.isProximityMonitoringEnabled = false
        UIApplication.shared.endReceivingRemoteControlEvents()
    }

    func onVisibleAppear() {
        let defaults = UserDefaults.standard
        if defaults.object(forKey: PREF_DISPLAY_PROXIMITY) != nil &&
            defaults.bool(forKey: PREF_DISPLAY_PROXIMITY) {
            UIDevice.current.isProximityMonitoringEnabled = true
        }
        if defaults.object(forKey: PREF_HEADSET_TXTOGGLE) != nil &&
            defaults.bool(forKey: PREF_HEADSET_TXTOGGLE) {
            UIApplication.shared.beginReceivingRemoteControlEvents()
        }
    }

    func remoteControl(_ event: UIEvent?) {
        guard let rc = event?.subtype else { return }
        switch rc {
        case .remoteControlPause, .remoteControlTogglePlayPause:
            channelListModel.enableVoiceTx(false)
        case .remoteControlPreviousTrack, .remoteControlNextTrack:
            channelListModel.enableVoiceTx(true)
        default:
            break
        }
    }

    func disconnectTapped(dismiss: @escaping () -> Void) {
        let servers = loadLocalServers()
        let found = servers.filter {
            $0.ipaddr == server.ipaddr &&
            $0.tcpport == server.tcpport &&
            $0.udpport == server.udpport &&
            $0.username == server.username
        }
        if found.isEmpty && server.servertype == .LOCAL {
            pendingDismiss = { dismiss() }
            showSaveAlert = true
        } else {
            dismiss()
        }
    }

    func saveAndDisconnect(name: String) {
        var servers = loadLocalServers()
        servers = servers.filter { $0.name != name }
        server.name = name
        servers.append(server)
        saveLocalServers(servers)
        pendingDismiss?()
        pendingDismiss = nil
    }

    func skipSaveAndDisconnect() {
        pendingDismiss?()
        pendingDismiss = nil
    }

    func startReconnectTimer() {
        reconnecttimer = Timer.scheduledTimer(
            timeInterval: 5.0,
            target: self,
            selector: #selector(connectToServer),
            userInfo: nil,
            repeats: false
        )
    }

    @objc func connectToServer() {
        if !setupEncryption(server: server) {
            fatalAlertMessage = String(localized: "Failed to setup encryption", comment: "connect to a server")
        } else if !TeamTalkClient.shared.connect(
            toHost: server.ipaddr,
            tcpPort: INT32(server.tcpport),
            udpPort: INT32(server.udpport),
            encrypted: server.encrypted
        ) {
            TeamTalkClient.shared.disconnect()
            startReconnectTimer()
        }
    }

    @objc private func timerEvent() {
        runTeamTalkEventHandler()
    }

    @objc private func proximityChanged(_ notification: Notification) {}

    @objc private func audioRouteChange(_ notification: Notification) {
        guard let reasonValue = notification.userInfo?[AVAudioSessionRouteChangeReasonKey] as? UInt,
              let reason = AVAudioSession.RouteChangeReason(rawValue: reasonValue) else { return }
        switch reason {
        case .oldDeviceUnavailable:
            setupSoundDevices()
        default:
            break
        }
        print(AVAudioSession.sharedInstance().currentRoute)
    }

    @objc private func audioInterruption(_ notification: Notification) {
        guard let optionValue = notification.userInfo?[AVAudioSessionInterruptionOptionKey] as? UInt else { return }
        let options = AVAudioSession.InterruptionOptions(rawValue: optionValue)
        if options.contains(.shouldResume) {
            setupSoundDevices()
        }
    }

    func handleTTMessage(_ m: TTMessage) {
        switch m.nClientEvent {

        case CLIENTEVENT_CON_SUCCESS:
            os_log("Connected to \(self.server.ipaddr)")

            if AppInfo.isBearWareWebLogin(self.server.username) {
                let settings = UserDefaults.standard
                let username = settings.string(forKey: PREF_GENERAL_BEARWARE_ID) ?? ""
                let token = settings.string(forKey: PREF_GENERAL_BEARWARE_TOKEN) ?? ""
                let accesstoken = TeamTalkClient.shared.withServerProperties {
                    TeamTalkString.serverProperties(.accessToken, from: $0)
                }
                let url = AppInfo.getBearWareServerTokenURL(
                    username: username, token: token, accesstoken: accesstoken
                )
                let authParser = WebLoginParser()
                if let tokenURL = URL(string: url), let parser = XMLParser(contentsOf: tokenURL) {
                    parser.delegate = authParser
                    if parser.parse() && authParser.username.count > 0 {
                        self.server.username = authParser.username
                        self.server.password = AppInfo.WEBLOGIN_BEARWARE_PASSWDPREFIX + authParser.token
                    }
                }
                if self.server.username == AppInfo.WEBLOGIN_BEARWARE_USERNAME {
                    alertMessage = String(localized: "BearWare.dk Web Login failed to authenticate. Check BearWare.dk Web Login in Preferences", comment: "weblogin event")
                }
            }
            login()

        case CLIENTEVENT_CON_FAILED:
            TeamTalkClient.shared.disconnect()
            startReconnectTimer()
            os_log("Connect to \(self.server.ipaddr) failed")

        case CLIENTEVENT_CON_LOST:
            os_log("Connection to \(self.server.ipaddr) lost")
            TeamTalkClient.shared.disconnect()
            playSound(.srv_LOST)
            if UserDefaults.standard.object(forKey: PREF_TTSEVENT_CONLOST) == nil ||
                UserDefaults.standard.bool(forKey: PREF_TTSEVENT_CONLOST) {
                newUtterance(String(localized: "Connection lost", comment: "tts event"))
            }
            startReconnectTimer()

        case CLIENTEVENT_VOICE_ACTIVATION:
            playSound(TeamTalkMessagePayload.isActive(m) ? .voxtriggered_ON : .voxtriggered_OFF)

        case CLIENTEVENT_CMD_PROCESSING:
            if !TeamTalkMessagePayload.isActive(m) {
                commandComplete(m.nSource)
            }

        case CLIENTEVENT_CMD_MYSELF_LOGGEDIN:
            let account = TeamTalkMessagePayload.userAccount(from: m)
            let initchan = TeamTalkString.userAccount(.initialChannel, from: account)
            if !initchan.isEmpty {
                server.channel = initchan
            }

        case CLIENTEVENT_CMD_MYSELF_KICKED:
            let msg: String
            if TeamTalkMessagePayload.hasUserPayload(m) {
                let kicker = getDisplayName(TeamTalkMessagePayload.user(from: m))
                msg = m.nSource == 0
                    ? String(format: String(localized: "You have been kicked from server by %@", comment: "Dialog"), kicker)
                    : String(format: String(localized: "You have been kicked from channel by %@", comment: "Dialog"), kicker)
            } else {
                msg = m.nSource == 0
                    ? String(localized: "You have been kicked from server", comment: "Dialog")
                    : String(localized: "You have been kicked from channel", comment: "Dialog")
            }
            if m.nSource == 0 { playSound(.srv_LOST) }
            alertMessage = msg

        case CLIENTEVENT_CMD_ERROR:
            if m.nSource == cmdid {
                alertMessage = TeamTalkString.clientError(TeamTalkMessagePayload.clientError(from: m))
            }

        case CLIENTEVENT_CMD_USER_LOGGEDIN:
            let subs = getDefaultSubscriptions()
            let user = TeamTalkMessagePayload.user(from: m)
            if TeamTalkClient.shared.myUserID != user.nUserID && user.uLocalSubscriptions != subs {
                TeamTalkClient.shared.unsubscribe(userID: user.nUserID, subscriptions: user.uLocalSubscriptions ^ subs)
            }
            syncFromUserCache(user: user)

        case CLIENTEVENT_CMD_USER_LOGGEDOUT:
            syncToUserCache(user: TeamTalkMessagePayload.user(from: m))

        case CLIENTEVENT_CMD_USER_JOINED:
            let user = TeamTalkMessagePayload.user(from: m)
            let defaults = UserDefaults.standard
            if let mfvol = defaults.object(forKey: PREF_MEDIAFILE_VOLUME) as? Double {
                let vol = refVolume(100.0 * mfvol)
                TeamTalkClient.shared.setUserVolume(
                    userID: user.nUserID, stream: STREAMTYPE_MEDIAFILE_AUDIO, volume: INT32(vol)
                )
                TeamTalkClient.shared.pump(CLIENTEVENT_USER_STATECHANGE, source: user.nUserID)
            }
            if (TeamTalkClient.shared.myUserRights & USERRIGHT_VIEW_ALL_USERS.rawValue) != USERRIGHT_VIEW_ALL_USERS.rawValue {
                syncFromUserCache(user: user)
            }

        case CLIENTEVENT_CMD_USER_LEFT:
            if (TeamTalkClient.shared.myUserRights & USERRIGHT_VIEW_ALL_USERS.rawValue) != USERRIGHT_VIEW_ALL_USERS.rawValue {
                syncToUserCache(user: TeamTalkMessagePayload.user(from: m))
            }

        case CLIENTEVENT_CMD_USER_TEXTMSG:
            switch TeamTalkMessagePayload.textMessage(from: m).nMsgType {
            case MSGTYPE_CHANNEL:   playSound(.chan_MSG)
            case MSGTYPE_USER:      playSound(.user_MSG)
            case MSGTYPE_BROADCAST: playSound(.broadcast_MSG)
            default: break
            }

        default:
            break
        }
    }

    private func commandComplete(_ active_cmdid: INT32) {
        let cmd = channelListModel.activeCommands[active_cmdid]
        guard let cmd else { return }

        switch cmd {
        case .loginCmd:
            if !server.channel.isEmpty {
                var tokens = server.channel.components(separatedBy: "/")
                let chanid = TeamTalkClient.shared.channelID(fromPath: server.channel)
                if chanid > 0 {
                    channelListModel.rejoinchannel.nChannelID = chanid
                    TeamTalkString.setChannel(.password, on: &channelListModel.rejoinchannel, to: server.chanpasswd)
                } else if tokens.count > 0 {
                    let channame = tokens.removeLast()
                    let chanpath = tokens.map { "/" + $0 }.joined()
                    let parentid = TeamTalkClient.shared.channelID(fromPath: chanpath)
                    if parentid > 0 {
                        channelListModel.rejoinchannel.nParentID = parentid
                        TeamTalkString.setChannel(.name, on: &channelListModel.rejoinchannel, to: channame)
                        TeamTalkString.setChannel(.password, on: &channelListModel.rejoinchannel, to: server.chanpasswd)
                        channelListModel.rejoinchannel.audiocodec = newAudioCodec(DEFAULT_AUDIOCODEC)
                    }
                }
                server.channel.removeAll()
                server.chanpasswd.removeAll()
            }
            let settings = UserDefaults.standard
            if settings.integer(forKey: PREF_GENERAL_GENDER) != 0 {
                TeamTalkClient.shared.changeStatus(mode: INT32(StatusMode.STATUSMODE_FEMALE.rawValue))
            }
        default:
            break
        }
    }

    private func login() {
        let nickname = server.nickname.isEmpty
            ? (UserDefaults.standard.string(forKey: PREF_GENERAL_NICKNAME) ?? "")
            : server.nickname
        cmdid = TeamTalkClient.shared.login(
            nickname: nickname,
            username: server.username,
            password: server.password,
            clientName: AppInfo.getAppName()
        )
        channelListModel.activeCommands[cmdid] = .loginCmd
        reconnecttimer?.invalidate()
    }
}
