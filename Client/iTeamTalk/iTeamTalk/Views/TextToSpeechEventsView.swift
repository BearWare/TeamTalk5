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

struct TextToSpeechEventsView: View {
    private let rows = [
        TextToSpeechEventRow(preferenceKey: PREF_TTSEVENT_USERLOGIN, defaultValue: false, title: "User logged in", subtitle: "Announce user logged onto server"),
        TextToSpeechEventRow(preferenceKey: PREF_TTSEVENT_USERLOGOUT, defaultValue: false, title: "User logged out", subtitle: "Announce user logged out of server"),
        TextToSpeechEventRow(preferenceKey: PREF_TTSEVENT_JOINEDCHAN, defaultValue: true, title: "User joins channel", subtitle: "Announce user joining channel"),
        TextToSpeechEventRow(preferenceKey: PREF_TTSEVENT_LEFTCHAN, defaultValue: true, title: "User leaves channel", subtitle: "Announce user leaving channel"),
        TextToSpeechEventRow(preferenceKey: PREF_TTSEVENT_CONLOST, defaultValue: true, title: "Connection lost", subtitle: "Announce lost server connection"),
        TextToSpeechEventRow(preferenceKey: PREF_TTSEVENT_TEXTMSG, defaultValue: false, title: "Private Text Message", subtitle: "Announce content of text message"),
        TextToSpeechEventRow(preferenceKey: PREF_TTSEVENT_CHANTEXTMSG, defaultValue: false, title: "Channel Text Message", subtitle: "Announce content of text message")
    ]

    var body: some View {
        Form {
            Section("Announcements") {
                ForEach(rows) { row in
                    TextToSpeechEventToggle(row: row)
                }
            }
        }
        .navigationTitle("Text To Speech Events")
    }
}

private struct TextToSpeechEventToggle: View {
    let row: TextToSpeechEventRow
    @AppStorage private var isOn: Bool

    init(row: TextToSpeechEventRow) {
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
    }
}

private struct TextToSpeechEventRow: Identifiable {
    let preferenceKey: String
    let defaultValue: Bool
    let title: LocalizedStringKey
    let subtitle: LocalizedStringKey

    var id: String {
        preferenceKey
    }
}
