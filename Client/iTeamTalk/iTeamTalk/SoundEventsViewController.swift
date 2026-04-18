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
        SoundEventRow(sound: .srv_LOST, preferenceKey: PREF_SNDEVENT_SERVERLOST, title: NSLocalizedString("Server Connection Lost", comment: "preferences"), subtitle: NSLocalizedString("Play sound when connection is dropped", comment: "preferences")),
        SoundEventRow(sound: .tx_ON, preferenceKey: PREF_SNDEVENT_VOICETX, title: NSLocalizedString("Voice Transmission Toggled", comment: "preferences"), subtitle: NSLocalizedString("Play sound when voice transmission is toggled", comment: "preferences")),
        SoundEventRow(sound: .user_MSG, preferenceKey: PREF_SNDEVENT_USERMSG, title: NSLocalizedString("Private Text Message", comment: "preferences"), subtitle: NSLocalizedString("Play sound when private text message is received", comment: "preferences")),
        SoundEventRow(sound: .chan_MSG, preferenceKey: PREF_SNDEVENT_CHANMSG, title: NSLocalizedString("Channel Text Message", comment: "preferences"), subtitle: NSLocalizedString("Play sound when channel text message is received", comment: "preferences")),
        SoundEventRow(sound: .broadcast_MSG, preferenceKey: PREF_SNDEVENT_BCASTMSG, title: NSLocalizedString("Broadcast Text Message", comment: "preferences"), subtitle: NSLocalizedString("Play sound when broadcast text message is received", comment: "preferences")),
        SoundEventRow(sound: .logged_IN, preferenceKey: PREF_SNDEVENT_LOGGEDIN, title: NSLocalizedString("User Logged In", comment: "preferences"), subtitle: NSLocalizedString("Play sound when user logs on to the server", comment: "preferences")),
        SoundEventRow(sound: .logged_OUT, preferenceKey: PREF_SNDEVENT_LOGGEDOUT, title: NSLocalizedString("User Logged Out", comment: "preferences"), subtitle: NSLocalizedString("Play sound when user logs off to the server", comment: "preferences")),
        SoundEventRow(sound: .joined_CHAN, preferenceKey: PREF_SNDEVENT_JOINEDCHAN, title: NSLocalizedString("User Joins Channel", comment: "preferences"), subtitle: NSLocalizedString("Play sound when a user joins the channel", comment: "preferences")),
        SoundEventRow(sound: .left_CHAN, preferenceKey: PREF_SNDEVENT_LEFTCHAN, title: NSLocalizedString("User Leaves Channel", comment: "preferences"), subtitle: NSLocalizedString("Play sound when a user leaves the channel", comment: "preferences")),
        SoundEventRow(sound: .voxtriggered_ON, preferenceKey: PREF_SNDEVENT_VOXTRIGGER, title: NSLocalizedString("Voice Activation Triggered", comment: "preferences"), subtitle: NSLocalizedString("Play sound when voice activation is triggered", comment: "preferences")),
        SoundEventRow(sound: .transmit_ON, preferenceKey: PREF_SNDEVENT_TRANSMITREADY, title: NSLocalizedString("Exclusive Mode Toggled", comment: "preferences"), subtitle: NSLocalizedString("Play sound when transmit ready in \"No Interruptions\" channel", comment: "preferences"))
    ]

    var body: some View {
        Form {
            Section(NSLocalizedString("Sound Events", comment:"text to speech")) {
                ForEach(rows) { row in
                    TeamTalkToggleRow(title: row.title, subtitle: row.subtitle, isOn: Binding(
                        get: { getSoundFile(row.sound) != nil },
                        set: { isOn in
                            UserDefaults.standard.set(isOn, forKey: row.preferenceKey)
                            if isOn {
                                playSound(row.sound)
                            }
                        }
                    ))
                }
            }
        }
        .navigationTitle(NSLocalizedString("Setup Sound Events", comment: "preferences"))
    }
}

private struct SoundEventRow: Identifiable {
    let sound: Sounds
    let preferenceKey: String
    let title: String
    let subtitle: String

    var id: String {
        preferenceKey
    }
}
