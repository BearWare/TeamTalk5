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

// MARK: - SwiftUI Tab View

struct MainTabView: View {
    let channelNav: UINavigationController
    let chatVC: TextMessageViewController
    @ObservedObject var preferencesModel: PreferencesModel

    var body: some View {
        TabView {
            UINavControllerView(channelNav)
                .tabItem {
                    Label(NSLocalizedString("Channels", comment: "tab"), image: "channels")
                }
                .tag(0)

            UIViewControllerView(chatVC)
                .tabItem {
                    Label(NSLocalizedString("Messages", comment: "tab"), image: "messages")
                }
                .tag(1)

            NavigationStack {
                PreferencesView(model: preferencesModel)
            }
            .tabItem {
                Label(NSLocalizedString("Preferences", comment: "tab"), image: "setup")
            }
            .tag(2)
        }
    }
}

private struct UINavControllerView: UIViewControllerRepresentable {
    private let navController: UINavigationController
    init(_ navController: UINavigationController) { self.navController = navController }
    func makeUIViewController(context: Context) -> UINavigationController { navController }
    func updateUIViewController(_ uiViewController: UINavigationController, context: Context) {}
}

private struct UIViewControllerView<VC: UIViewController>: UIViewControllerRepresentable {
    private let viewController: VC
    init(_ viewController: VC) { self.viewController = viewController }
    func makeUIViewController(context: Context) -> VC { viewController }
    func updateUIViewController(_ uiViewController: VC, context: Context) {}
}

// MARK: - Main Tab Bar Controller

final class MainTabBarController: UIHostingController<MainTabView>, TeamTalkEvent {

    // timer for polling TeamTalk client events
    var polltimer: Timer?
    // reconnect timer
    var reconnecttimer: Timer?
    // ip-addr and login information for current server
    var server: Server
    // active command
    var cmdid: INT32 = 0

    private let channelListVC: ChannelListViewController
    private let channelChatVC: TextMessageViewController
    private let preferencesModel: PreferencesModel

    init(server: Server) {
        self.server = server

        let channelListVC = ChannelListViewController()
        let channelChatVC = TextMessageViewController()
        let preferencesModel = PreferencesModel()

        self.channelListVC = channelListVC
        self.channelChatVC = channelChatVC
        self.preferencesModel = preferencesModel

        let channelNav = UINavigationController(rootViewController: channelListVC)

        super.init(rootView: MainTabView(
            channelNav: channelNav,
            chatVC: channelChatVC,
            preferencesModel: preferencesModel
        ))
    }

    required init?(coder: NSCoder) { fatalError("use init(server:)") }

    override func viewDidLoad() {
        super.viewDidLoad()

        navigationItem.leftBarButtonItem = UIBarButtonItem(
            title: NSLocalizedString("Disconnect", comment: "main-tab"),
            style: .plain,
            target: self,
            action: #selector(disconnectButtonPressed(_:))
        )
        let addBtn = UIBarButtonItem(
            barButtonSystemItem: .add,
            target: self,
            action: #selector(newChannelPressed(_:))
        )
        addBtn.accessibilityLabel = NSLocalizedString("Create new channel", comment: "main-tab")
        navigationItem.rightBarButtonItem = addBtn

        addToTTMessages(self)
        addToTTMessages(channelListVC)
        addToTTMessages(channelChatVC)
        addToTTMessages(preferencesModel)

        // wire up the channel chat tab
        channelChatVC.navigationItem.title = NSLocalizedString("Messages", comment: "tab")
        channelListVC.openTextMessages(channelChatVC, userid: 0)

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

        polltimer = Timer.scheduledTimer(timeInterval: 0.1, target: self, selector: #selector(MainTabBarController.timerEvent), userInfo: nil, repeats: true)

        let center = NotificationCenter.default
        center.addObserver(self, selector: #selector(MainTabBarController.proximityChanged(_:)), name: UIDevice.proximityStateDidChangeNotification, object: UIDevice.current)
        center.addObserver(self, selector: #selector(MainTabBarController.audioRouteChange(_:)), name: AVAudioSession.routeChangeNotification, object: nil)
        center.addObserver(self, selector: #selector(MainTabBarController.audioInterruption(_:)), name: AVAudioSession.interruptionNotification, object: nil)

        connectToServer()
    }

    deinit {
        TeamTalkClient.shared.disconnect()
        closeSoundDevices()
        runTeamTalkEventHandler()
        print("Destroyed main view controller")
    }

    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)

        let defaults = UserDefaults.standard
        if defaults.object(forKey: PREF_DISPLAY_PROXIMITY) != nil && defaults.bool(forKey: PREF_DISPLAY_PROXIMITY) {
            UIDevice.current.isProximityMonitoringEnabled = true
        }
        if defaults.object(forKey: PREF_HEADSET_TXTOGGLE) != nil && defaults.bool(forKey: PREF_HEADSET_TXTOGGLE) {
            UIApplication.shared.beginReceivingRemoteControlEvents()
        }
    }

    override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)

        if self.isMovingFromParent {
            polltimer?.invalidate()
            reconnecttimer?.invalidate()
            removeAllTTMessageHandlers()
            unreadmessages.removeAll()
        }

        UIDevice.current.isProximityMonitoringEnabled = false
        UIApplication.shared.endReceivingRemoteControlEvents()
    }

    override func remoteControlReceived(with event: UIEvent?) {
        guard let rc = event?.subtype else { return }
        switch rc {
        case .remoteControlPause, .remoteControlTogglePlayPause:
            channelListVC.enableVoiceTx(false)
        case .remoteControlPreviousTrack, .remoteControlNextTrack:
            channelListVC.enableVoiceTx(true)
        default:
            break
        }
    }

    func startReconnectTimer() {
        reconnecttimer = Timer.scheduledTimer(timeInterval: 5.0, target: self, selector: #selector(MainTabBarController.connectToServer), userInfo: nil, repeats: false)
    }

    @objc func connectToServer() {
        if setupEncryption(server: server) == false {
            let alert = UIAlertController(
                title: NSLocalizedString("Connect to Server", comment: "connect to a server"),
                message: NSLocalizedString("Failed to setup encryption", comment: "connect to a server"),
                preferredStyle: .alert
            )
            alert.addAction(UIAlertAction(title: "OK", style: .default) { [weak self] _ in
                self?.navigationController?.popViewController(animated: true)
            })
            present(alert, animated: true)
        } else {
            if !TeamTalkClient.shared.connect(toHost: server.ipaddr,
                                              tcpPort: INT32(server.tcpport),
                                              udpPort: INT32(server.udpport),
                                              encrypted: server.encrypted) {
                TeamTalkClient.shared.disconnect()
                startReconnectTimer()
            }
        }
    }

    @objc func timerEvent() {
        runTeamTalkEventHandler()
    }

    @objc func proximityChanged(_ notification: Notification) {}

    @objc func audioRouteChange(_ notification: Notification) {
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

    @objc func audioInterruption(_ notification: Notification) {
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

                let url = AppInfo.getBearWareServerTokenURL(username: username, token: token, accesstoken: accesstoken)
                let authParser = WebLoginParser()
                if let parser = XMLParser(contentsOf: URL(string: url)!) {
                    parser.delegate = authParser
                    if parser.parse() && authParser.username.count > 0 {
                        self.server.username = authParser.username
                        self.server.password = AppInfo.WEBLOGIN_BEARWARE_PASSWDPREFIX + authParser.token
                    }
                }

                if self.server.username == AppInfo.WEBLOGIN_BEARWARE_USERNAME {
                    let err = NSLocalizedString("BearWare.dk Web Login failed to authenticate. Check BearWare.dk Web Login in Preferences", comment: "weblogin event")
                    let alert = UIAlertController(title: NSLocalizedString("Error", comment: "message dialog"), message: err, preferredStyle: .alert)
                    alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "Dialog message"), style: .default, handler: nil))
                    present(alert, animated: true)
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
            if UserDefaults.standard.object(forKey: PREF_TTSEVENT_CONLOST) == nil || UserDefaults.standard.bool(forKey: PREF_TTSEVENT_CONLOST) {
                newUtterance(NSLocalizedString("Connection lost", comment: "tts event"))
            }
            startReconnectTimer()
            os_log("Connection to \(self.server.ipaddr) lost")

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
                    ? String(format: NSLocalizedString("You have been kicked from server by %@", comment: "Dialog"), kicker)
                    : String(format: NSLocalizedString("You have been kicked from channel by %@", comment: "Dialog"), kicker)
            } else {
                msg = m.nSource == 0
                    ? NSLocalizedString("You have been kicked from server", comment: "Dialog")
                    : NSLocalizedString("You have been kicked from channel", comment: "Dialog")
            }
            if m.nSource == 0 { playSound(.srv_LOST) }
            let alert = UIAlertController(title: NSLocalizedString("Error", comment: "Dialog"), message: msg, preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "Dialog"), style: .default, handler: nil))
            present(alert, animated: true)

        case CLIENTEVENT_CMD_ERROR:
            if m.nSource == cmdid {
                let errmsg = TeamTalkString.clientError(TeamTalkMessagePayload.clientError(from: m))
                let alert = UIAlertController(title: NSLocalizedString("Error", comment: "message dialog"), message: errmsg, preferredStyle: .alert)
                alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "message dialog"), style: .default, handler: nil))
                present(alert, animated: true)
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
                TeamTalkClient.shared.setUserVolume(userID: user.nUserID, stream: STREAMTYPE_MEDIAFILE_AUDIO, volume: INT32(vol))
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

    func commandComplete(_ active_cmdid: INT32) {
        let cmd = channelListVC.activeCommands[active_cmdid]
        guard let cmd else { return }

        switch cmd {
        case .loginCmd:
            if !server.channel.isEmpty {
                var tokens = server.channel.components(separatedBy: "/")
                let chanid = TeamTalkClient.shared.channelID(fromPath: server.channel)
                if chanid > 0 {
                    channelListVC.rejoinchannel.nChannelID = chanid
                    TeamTalkString.setChannel(.password, on: &channelListVC.rejoinchannel, to: server.chanpasswd)
                } else if tokens.count > 0 {
                    let channame = tokens.removeLast()
                    let chanpath = tokens.map { "/" + $0 }.joined()
                    let parentid = TeamTalkClient.shared.channelID(fromPath: chanpath)
                    if parentid > 0 {
                        channelListVC.rejoinchannel.nParentID = parentid
                        TeamTalkString.setChannel(.name, on: &channelListVC.rejoinchannel, to: channame)
                        TeamTalkString.setChannel(.password, on: &channelListVC.rejoinchannel, to: server.chanpasswd)
                        channelListVC.rejoinchannel.audiocodec = newAudioCodec(DEFAULT_AUDIOCODEC)
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

    func login() {
        let nickname = server.nickname.isEmpty
            ? (UserDefaults.standard.string(forKey: PREF_GENERAL_NICKNAME) ?? "")
            : server.nickname

        cmdid = TeamTalkClient.shared.login(
            nickname: nickname,
            username: server.username,
            password: server.password,
            clientName: AppInfo.getAppName()
        )
        channelListVC.activeCommands[cmdid] = .loginCmd
        reconnecttimer?.invalidate()
    }

    @objc private func newChannelPressed(_ sender: UIBarButtonItem) {
        channelListVC.showNewChannelDetail()
    }

    @objc func disconnectButtonPressed(_ sender: UIBarButtonItem) {
        let servers = loadLocalServers()
        let found = servers.filter {
            $0.ipaddr == server.ipaddr &&
            $0.tcpport == server.tcpport &&
            $0.udpport == server.udpport &&
            $0.username == server.username
        }

        if found.isEmpty && server.servertype == .LOCAL {
            showSaveServerAlert()
        } else {
            navigationController?.popViewController(animated: true)
        }
    }

    private func showSaveServerAlert() {
        let alert = UIAlertController(
            title: NSLocalizedString("Save server to server list?", comment: "Dialog message"),
            message: NSLocalizedString("Save Server", comment: "Dialog message"),
            preferredStyle: .alert
        )
        alert.addTextField { textField in
            textField.text = NSLocalizedString("New Server", comment: "Dialog message")
        }
        alert.addAction(UIAlertAction(title: NSLocalizedString("No", comment: "Dialog message"), style: .cancel) { [weak self] _ in
            self?.navigationController?.popViewController(animated: true)
        })
        alert.addAction(UIAlertAction(title: NSLocalizedString("Yes", comment: "Dialog message"), style: .default) { [weak self, weak alert] _ in
            guard let self else { return }
            let name = alert?.textFields?.first?.text ?? NSLocalizedString("New Server", comment: "Dialog message")
            var servers = loadLocalServers()
            servers = servers.filter { $0.name != name }
            server.name = name
            servers.append(server)
            saveLocalServers(servers)
            navigationController?.popViewController(animated: true)
        })
        present(alert, animated: true)
    }

}
