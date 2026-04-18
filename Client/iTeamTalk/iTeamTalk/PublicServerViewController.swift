//
//  PublicServerViewController.swift
//  iTeamTalk
//
//  Created by Bjørn Damstedt Rasmussen on 07/03/2022.
//  Copyright © 2022 BearWare.dk. All rights reserved.
//

import SwiftUI

struct PublicServerView: View {
    private let rows = PublicServerRow.allCases

    var body: some View {
        Form {
            Section(NSLocalizedString("Show in Server List", comment: "preferences")) {
                ForEach(rows) { row in
                    TeamTalkToggleRow(title: row.title, isOn: Binding(
                        get: { row.boolValue },
                        set: { UserDefaults.standard.set($0, forKey: row.preferenceKey) }
                    ))
                }
            }
        }
        .navigationTitle(NSLocalizedString("Filter Server List", comment: "preferences"))
    }
}

private enum PublicServerRow: CaseIterable, Identifiable {
    case official
    case unofficial

    var id: String {
        preferenceKey
    }

    var title: String {
        switch self {
        case .official:
            return NSLocalizedString("Official Servers", comment: "preferences")
        case .unofficial:
            return NSLocalizedString("Unofficial Servers", comment: "preferences")
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
