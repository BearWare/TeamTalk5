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

struct SoundEventsView: View {
    private let rows = [
        SoundEventRow(sound: .srv_LOST, preferenceKey: PREF_SNDEVENT_SERVERLOST, defaultValue: true, title: "Server Connection Lost", subtitle: "Play sound when connection is dropped"),
        SoundEventRow(sound: .tx_ON, preferenceKey: PREF_SNDEVENT_VOICETX, defaultValue: true, title: "Voice Transmission Toggled", subtitle: "Play sound when voice transmission is toggled"),
        SoundEventRow(sound: .user_MSG, preferenceKey: PREF_SNDEVENT_USERMSG, defaultValue: true, title: "Private Text Message", subtitle: "Play sound when private text message is received"),
        SoundEventRow(sound: .chan_MSG, preferenceKey: PREF_SNDEVENT_CHANMSG, defaultValue: true, title: "Channel Text Message", subtitle: "Play sound when channel text message is received"),
        SoundEventRow(sound: .broadcast_MSG, preferenceKey: PREF_SNDEVENT_BCASTMSG, defaultValue: true, title: "Broadcast Text Message", subtitle: "Play sound when broadcast text message is received"),
        SoundEventRow(sound: .logged_IN, preferenceKey: PREF_SNDEVENT_LOGGEDIN, defaultValue: false, title: "User Logged In", subtitle: "Play sound when user logs on to the server"),
        SoundEventRow(sound: .logged_OUT, preferenceKey: PREF_SNDEVENT_LOGGEDOUT, defaultValue: false, title: "User Logged Out", subtitle: "Play sound when user logs off to the server"),
        SoundEventRow(sound: .joined_CHAN, preferenceKey: PREF_SNDEVENT_JOINEDCHAN, defaultValue: true, title: "User Joins Channel", subtitle: "Play sound when a user joins the channel"),
        SoundEventRow(sound: .left_CHAN, preferenceKey: PREF_SNDEVENT_LEFTCHAN, defaultValue: true, title: "User Leaves Channel", subtitle: "Play sound when a user leaves the channel"),
        SoundEventRow(sound: .voxtriggered_ON, preferenceKey: PREF_SNDEVENT_VOXTRIGGER, defaultValue: true, title: "Voice Activation Triggered", subtitle: "Play sound when voice activation is triggered"),
        SoundEventRow(sound: .transmit_ON, preferenceKey: PREF_SNDEVENT_TRANSMITREADY, defaultValue: true, title: "Exclusive Mode Toggled", subtitle: "Play sound when transmit ready in \"No Interruptions\" channel")
    ]

    var body: some View {
        Form {
            Section("Sound Events") {
                ForEach(rows) { row in
                    SoundEventToggle(row: row)
                }
            }
        }
        .navigationTitle("Setup Sound Events")
    }
}

private struct SoundEventToggle: View {
    let row: SoundEventRow
    @AppStorage private var isOn: Bool

    init(row: SoundEventRow) {
        self.row = row
        _isOn = AppStorage(wrappedValue: row.defaultValue, row.preferenceKey)
    }

    var body: some View {
        Toggle(isOn: $isOn) {
            VStack(alignment: .leading, spacing: 2) {
                Text(row.title)
                Text(row.subtitle)
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }
        }
        .onChange(of: isOn) { newValue in
            if newValue {
                playSound(row.sound)
            }
        }
    }
}

private struct SoundEventRow: Identifiable {
    let sound: Sounds
    let preferenceKey: String
    let defaultValue: Bool
    let title: LocalizedStringKey
    let subtitle: LocalizedStringKey

    var id: String {
        preferenceKey
    }
}
