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

struct SoundDevicesView: View {
    @StateObject private var model = SoundDevicesModel()

    var body: some View {
        Form {
            Section("General") {
                ForEach(model.toggleRows) { row in
                    Toggle(isOn: Binding(
                        get: { model.preferenceValue(forKey: row.preferenceKey) },
                        set: { model.setPreference($0, forKey: row.preferenceKey) }
                    )) {
                        VStack(alignment: .leading, spacing: 2) {
                            Text(row.title)
                            Text(row.subtitle)
                                .font(.footnote)
                                .foregroundStyle(.secondary)
                        }
                    }
                }
            }

            ForEach(model.audioInputSections) { section in
                Section(section.title) {
                    ForEach(section.dataSources.indices, id: \.self) { index in
                        Button {
                            model.selectDataSource(at: index, for: section.input)
                        } label: {
                            Text(model.title(for: section.dataSources, at: index, input: section.input))
                                .frame(maxWidth: .infinity, alignment: .leading)
                        }
                        .buttonStyle(.plain)
                    }
                }
            }
        }
        .navigationTitle("Setup Sound Devices")
    }
}
