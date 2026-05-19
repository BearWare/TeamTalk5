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

struct PublicServerView: View {
    private let rows = PublicServerRow.allCases

    var body: some View {
        Form {
            Section("Show in Server List") {
                ForEach(rows) { row in
                    Toggle(row.title, isOn: Binding(
                        get: { row.boolValue },
                        set: { UserDefaults.standard.set($0, forKey: row.preferenceKey) }
                    ))
                }
            }
        }
        .navigationTitle("Filter Server List")
    }
}

private enum PublicServerRow: CaseIterable, Identifiable {
    case official
    case unofficial

    var id: String {
        preferenceKey
    }

    var title: LocalizedStringKey {
        switch self {
        case .official:
            return "Official Servers"
        case .unofficial:
            return "Unofficial Servers"
        }
    }

    var preferenceKey: String {
        switch self {
        case .official:
            return PREF_DISPLAY_OFFICIALSERVERS
        case .unofficial:
            return PREF_DISPLAY_UNOFFICIALSERVERS
        }
    }

    var defaultValue: Bool {
        switch self {
        case .official:
            return true
        case .unofficial:
            return false
        }
    }

    var boolValue: Bool {
        let settings = UserDefaults.standard
        if settings.object(forKey: preferenceKey) == nil {
            return defaultValue
        }
        return settings.bool(forKey: preferenceKey)
    }
}
