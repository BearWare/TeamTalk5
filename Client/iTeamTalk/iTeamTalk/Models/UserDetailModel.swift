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

final class UserDetailModel: ObservableObject {

    struct SubscriptionRow: Identifiable {
        let title: String
        let type: Subscription

        var id: Subscription.RawValue {
            type.rawValue
        }
    }

    let userid: INT32
    let displayName: String
    let subscriptionRows: [SubscriptionRow]

    private var kick_cmdid: INT32 = 0
    private var kickban_cmdid: INT32 = 0

    @Published var errorMessage: String?
    @Published var usernameText: String
    @Published var voiceVolume: Double
    @Published var mediaVolume: Double
    @Published var isVoiceMuted: Bool
    @Published var isMediaMuted: Bool
    @Published private var subscriptions: Subscriptions

    init(user: User) {
        userid = user.nUserID
        displayName = getDisplayName(user)
        usernameText = TeamTalkString.user(.username, from: user)
        voiceVolume = Double(refVolumeToPercent(Int(user.nVolumeVoice)))
        mediaVolume = Double(refVolumeToPercent(Int(user.nVolumeMediaFile)))
        isVoiceMuted = (user.uUserState & USERSTATE_MUTE_VOICE.rawValue) != 0
        isMediaMuted = (user.uUserState & USERSTATE_MUTE_MEDIAFILE.rawValue) != 0
        subscriptions = user.uLocalSubscriptions
        subscriptionRows = [
            SubscriptionRow(title: String(localized: "User Messages", comment: "user detail"), type: SUBSCRIBE_USER_MSG),
            SubscriptionRow(title: String(localized: "Channel Messages", comment: "user detail"), type: SUBSCRIBE_CHANNEL_MSG),
            SubscriptionRow(title: String(localized: "Broadcast Messages", comment: "user detail"), type: SUBSCRIBE_BROADCAST_MSG),
            SubscriptionRow(title: String(localized: "Voice", comment: "user detail"), type: SUBSCRIBE_VOICE),
            SubscriptionRow(title: String(localized: "WebCam", comment: "user detail"), type: SUBSCRIBE_VIDEOCAPTURE),
            SubscriptionRow(title: String(localized: "Media File", comment: "user detail"), type: SUBSCRIBE_MEDIAFILE),
            SubscriptionRow(title: String(localized: "Desktop", comment: "user detail"), type: SUBSCRIBE_DESKTOP)
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

    func kickUser() {
        let channelID = TeamTalkClient.shared.withUser(id: userid) { $0.nChannelID }
        kick_cmdid = TeamTalkClient.shared.kickUser(id: userid, fromChannelID: channelID)
    }

    func kickAndBanUser() {
        kickban_cmdid = TeamTalkClient.shared.kickUser(id: userid, fromChannelID: 0)
    }
}

extension UserDetailModel: TeamTalkEvent {
    func handleTTMessage(_ m: TTMessage) {
        switch m.nClientEvent {
        case CLIENTEVENT_CMD_SUCCESS:
            if m.nSource == kickban_cmdid {
                TeamTalkClient.shared.banUser(id: userid, fromChannelID: 0)
            }
        case CLIENTEVENT_CMD_ERROR:
            if m.nSource == kick_cmdid || m.nSource == kickban_cmdid {
                errorMessage = TeamTalkString.clientError(TeamTalkMessagePayload.clientError(from: m))
            }
        default:
            break
        }
    }
}
