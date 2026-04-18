/*
 * Copyright (c) 2005-2018, BearWare.dk.
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

final class UserDetailViewController: UIHostingController<UserDetailView>, TeamTalkEvent {

    var userid: INT32 = 0
    var kick_cmdid: INT32 = 0
    var kickban_cmdid: INT32 = 0

    private var model: UserDetailModel

    init() {
        let model = UserDetailModel(user: User())
        self.model = model
        super.init(rootView: UserDetailView(model: model, kickUser: { }, kickAndBanUser: { }))
    }

    required init?(coder: NSCoder) { fatalError("use init()") }

    override func viewDidLoad() {
        super.viewDidLoad()

        let user = TeamTalkClient.shared.withUser(id: userid) { $0 }
        navigationItem.title = getDisplayName(user)

        let model = UserDetailModel(user: user)
        self.model = model
        rootView = UserDetailView(
            model: model,
            kickUser: { [weak self] in self?.kickUser() },
            kickAndBanUser: { [weak self] in self?.kickAndBanUser() }
        )
        addToTTMessages(self)
    }

    private func kickAndBanUser() {
        kickban_cmdid = TeamTalkClient.shared.kickUser(id: userid, fromChannelID: 0)
    }

    private func kickUser() {
        let channelID = TeamTalkClient.shared.withUser(id: userid) { $0.nChannelID }
        kick_cmdid = TeamTalkClient.shared.kickUser(id: userid, fromChannelID: channelID)
    }

    func handleTTMessage(_ m: TTMessage) {
        switch m.nClientEvent {
        case CLIENTEVENT_CMD_SUCCESS:
            if m.nSource == kickban_cmdid {
                TeamTalkClient.shared.banUser(id: userid, fromChannelID: 0)
            }

        case CLIENTEVENT_CMD_ERROR:
            if m.nSource == kick_cmdid || m.nSource == kickban_cmdid {
                let errmsg = TeamTalkString.clientError(TeamTalkMessagePayload.clientError(from: m))
                let alert = UIAlertController(title: NSLocalizedString("Error", comment: "user detail"), message: errmsg, preferredStyle: .alert)
                alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "user detail"), style: .default, handler: nil))
                present(alert, animated: true, completion: nil)
            }

        default:
            break
        }
    }
}

final class UserDetailModel: ObservableObject {

    struct SubscriptionRow: Identifiable {
        let title: String
        let type: Subscription

        var id: Subscription.RawValue {
            type.rawValue
        }
    }

    let userid: INT32
    let subscriptionRows: [SubscriptionRow]

    @Published var usernameText: String
    @Published var voiceVolume: Double
    @Published var mediaVolume: Double
    @Published var isVoiceMuted: Bool
    @Published var isMediaMuted: Bool
    @Published private var subscriptions: Subscriptions

    init(user: User) {
        userid = user.nUserID
        usernameText = TeamTalkString.user(.username, from: user)
        voiceVolume = Double(refVolumeToPercent(Int(user.nVolumeVoice)))
        mediaVolume = Double(refVolumeToPercent(Int(user.nVolumeMediaFile)))
        isVoiceMuted = (user.uUserState & USERSTATE_MUTE_VOICE.rawValue) != 0
        isMediaMuted = (user.uUserState & USERSTATE_MUTE_MEDIAFILE.rawValue) != 0
        subscriptions = user.uLocalSubscriptions
        subscriptionRows = [
            SubscriptionRow(title: NSLocalizedString("User Messages", comment: "user detail"), type: SUBSCRIBE_USER_MSG),
            SubscriptionRow(title: NSLocalizedString("Channel Messages", comment: "user detail"), type: SUBSCRIBE_CHANNEL_MSG),
            SubscriptionRow(title: NSLocalizedString("Broadcast Messages", comment: "user detail"), type: SUBSCRIBE_BROADCAST_MSG),
            SubscriptionRow(title: NSLocalizedString("Voice", comment: "user detail"), type: SUBSCRIBE_VOICE),
            SubscriptionRow(title: NSLocalizedString("WebCam", comment: "user detail"), type: SUBSCRIBE_VIDEOCAPTURE),
            SubscriptionRow(title: NSLocalizedString("Media File", comment: "user detail"), type: SUBSCRIBE_MEDIAFILE),
            SubscriptionRow(title: NSLocalizedString("Desktop", comment: "user detail"), type: SUBSCRIBE_DESKTOP)
        ]
    }

    func isSubscribed(to subscription: Subscription) -> Bool {
        (subscriptions & subscription.rawValue) != 0
    }

    func voiceVolumeChanged(_ value: Double) {
        voiceVolume = value
        TeamTalkClient.shared.setUserVolume(userID: userid, stream: STREAMTYPE_VOICE, volume: INT32(refVolume(value)))
        TeamTalkClient.shared.pump(CLIENTEVENT_USER_STATECHANGE, source: userid)
    }

    func mediaVolumeChanged(_ value: Double) {
        mediaVolume = value
        TeamTalkClient.shared.setUserVolume(userID: userid, stream: STREAMTYPE_MEDIAFILE_AUDIO, volume: INT32(refVolume(value)))
        TeamTalkClient.shared.pump(CLIENTEVENT_USER_STATECHANGE, source: userid)
    }

    func muteVoice(_ muted: Bool) {
        isVoiceMuted = muted
        TeamTalkClient.shared.setUserMute(userID: userid, stream: STREAMTYPE_VOICE, muted: muted)
        TeamTalkClient.shared.pump(CLIENTEVENT_USER_STATECHANGE, source: userid)
    }

    func muteMediaStream(_ muted: Bool) {
        isMediaMuted = muted
        TeamTalkClient.shared.setUserMute(userID: userid, stream: STREAMTYPE_MEDIAFILE_AUDIO, muted: muted)
        TeamTalkClient.shared.pump(CLIENTEVENT_USER_STATECHANGE, source: userid)
    }

    func setSubscription(_ subscription: Subscription, enabled: Bool) {
        if enabled {
            subscriptions |= subscription.rawValue
            TeamTalkClient.shared.subscribe(userID: userid, subscriptions: subscription.rawValue)
        } else {
            subscriptions &= ~subscription.rawValue
            TeamTalkClient.shared.unsubscribe(userID: userid, subscriptions: subscription.rawValue)
        }
    }
}

struct UserDetailView: View {
    @ObservedObject var model: UserDetailModel

    let kickUser: () -> Void
    let kickAndBanUser: () -> Void

    var body: some View {
        Form {
            Section(NSLocalizedString("General", comment: "user detail")) {
                TeamTalkTextFieldRow(
                    title: NSLocalizedString("Username", comment: "user detail"),
                    text: $model.usernameText,
                    isEnabled: false
                )
            }

            Section(NSLocalizedString("Volume Controls", comment: "user detail")) {
                TeamTalkSliderRow(
                    title: NSLocalizedString("Voice Volume", comment: "user detail"),
                    value: Binding(
                        get: { model.voiceVolume },
                        set: { model.voiceVolumeChanged($0) }
                    ),
                    range: 0...100,
                    step: 1
                )
                TeamTalkToggleRow(
                    title: NSLocalizedString("Mute Voice", comment: "user detail"),
                    isOn: Binding(
                        get: { model.isVoiceMuted },
                        set: { model.muteVoice($0) }
                    )
                )
                TeamTalkSliderRow(
                    title: NSLocalizedString("Media File Volume", comment: "user detail"),
                    value: Binding(
                        get: { model.mediaVolume },
                        set: { model.mediaVolumeChanged($0) }
                    ),
                    range: 0...100,
                    step: 1
                )
                TeamTalkToggleRow(
                    title: NSLocalizedString("Mute Media File", comment: "user detail"),
                    isOn: Binding(
                        get: { model.isMediaMuted },
                        set: { model.muteMediaStream($0) }
                    )
                )
            }

            Section(NSLocalizedString("Subscriptions", comment: "user detail")) {
                ForEach(model.subscriptionRows) { row in
                    TeamTalkToggleRow(
                        title: row.title,
                        isOn: Binding(
                            get: { model.isSubscribed(to: row.type) },
                            set: { model.setSubscription(row.type, enabled: $0) }
                        )
                    )
                }
            }

            Section(NSLocalizedString("Actions", comment: "user detail")) {
                TeamTalkActionRow(title: NSLocalizedString("Kick User", comment: "user detail"), action: kickUser)
                TeamTalkActionRow(title: NSLocalizedString("Kick and Ban User", comment: "user detail"), role: .destructive, action: kickAndBanUser)
            }
        }
    }
}
