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
        TextToSpeechEventRow(preferenceKey: PREF_TTSEVENT_USERLOGIN, defaultValue: false, title: NSLocalizedString("User logged in", comment: "preferences"), subtitle: NSLocalizedString("Announce user logged onto server", comment: "preferences")),
        TextToSpeechEventRow(preferenceKey: PREF_TTSEVENT_USERLOGOUT, defaultValue: false, title: NSLocalizedString("User logged out", comment: "preferences"), subtitle: NSLocalizedString("Announce user logged out of server", comment: "preferences")),
        TextToSpeechEventRow(preferenceKey: PREF_TTSEVENT_JOINEDCHAN, defaultValue: true, title: NSLocalizedString("User joins channel", comment: "preferences"), subtitle: NSLocalizedString("Announce user joining channel", comment: "preferences")),
        TextToSpeechEventRow(preferenceKey: PREF_TTSEVENT_LEFTCHAN, defaultValue: true, title: NSLocalizedString("User leaves channel", comment: "preferences"), subtitle: NSLocalizedString("Announce user leaving channel", comment: "preferences")),
        TextToSpeechEventRow(preferenceKey: PREF_TTSEVENT_CONLOST, defaultValue: true, title: NSLocalizedString("Connection lost", comment: "preferences"), subtitle: NSLocalizedString("Announce lost server connection", comment: "preferences")),
        TextToSpeechEventRow(preferenceKey: PREF_TTSEVENT_TEXTMSG, defaultValue: false, title: NSLocalizedString("Private Text Message", comment: "preferences"), subtitle: NSLocalizedString("Announce content of text message", comment: "preferences")),
        TextToSpeechEventRow(preferenceKey: PREF_TTSEVENT_CHANTEXTMSG, defaultValue: false, title: NSLocalizedString("Channel Text Message", comment: "preferences"), subtitle: NSLocalizedString("Announce content of text message", comment: "preferences"))
    ]

    var body: some View {
        Form {
            Section(NSLocalizedString("Announcements", comment:"text to speech")) {
                ForEach(rows) { row in
                    TeamTalkToggleRow(title: row.title, subtitle: row.subtitle, isOn: Binding(
                        get: { row.preferenceValue },
                        set: { UserDefaults.standard.set($0, forKey: row.preferenceKey) }
                    ))
                }
            }
        }
        .navigationTitle(NSLocalizedString("Text To Speech Events", comment: "preferences"))
    }
}

private struct TextToSpeechEventRow: Identifiable {
    let preferenceKey: String
    let defaultValue: Bool
    let title: String
    let subtitle: String

    var id: String {
        preferenceKey
    }

    var preferenceValue: Bool {
        let settings = UserDefaults.standard
        if settings.object(forKey: preferenceKey) == nil {
            return defaultValue
        }
        return settings.bool(forKey: preferenceKey)
    }
}
