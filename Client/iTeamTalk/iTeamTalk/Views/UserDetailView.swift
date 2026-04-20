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

struct UserDetailView: View {
    @ObservedObject var model: UserDetailModel

    var body: some View {
        Form {
            Section("General") {
                TextField("Username", text: $model.usernameText)
                    .multilineTextAlignment(.trailing)
                    .autocorrectionDisabled()
                    .textInputAutocapitalization(.never)
                    .disabled(true)
            }

            Section("Volume Controls") {
                VStack(alignment: .leading, spacing: 8) {
                    HStack(spacing: 12) {
                        Text("Voice Volume")
                        Spacer(minLength: 16)
                        Text("\(Int(model.voiceVolume.rounded()))")
                            .font(.body.monospacedDigit())
                            .foregroundStyle(.secondary)
                    }
                    Slider(value: Binding(get: { model.voiceVolume }, set: { model.voiceVolumeChanged($0) }),
                           in: 0...100, step: 1)
                }
                Toggle("Mute Voice", isOn: Binding(
                    get: { model.isVoiceMuted },
                    set: { model.muteVoice($0) }
                ))
                VStack(alignment: .leading, spacing: 8) {
                    HStack(spacing: 12) {
                        Text("Media File Volume")
                        Spacer(minLength: 16)
                        Text("\(Int(model.mediaVolume.rounded()))")
                            .font(.body.monospacedDigit())
                            .foregroundStyle(.secondary)
                    }
                    Slider(value: Binding(get: { model.mediaVolume }, set: { model.mediaVolumeChanged($0) }),
                           in: 0...100, step: 1)
                }
                Toggle("Mute Media File", isOn: Binding(
                    get: { model.isMediaMuted },
                    set: { model.muteMediaStream($0) }
                ))
            }

            Section("Subscriptions") {
                ForEach(model.subscriptionRows) { row in
                    Toggle(row.title, isOn: Binding(
                        get: { model.isSubscribed(to: row.type) },
                        set: { model.setSubscription(row.type, enabled: $0) }
                    ))
                }
            }

            Section("Actions") {
                Button(action: model.kickUser) {
                    Text("Kick User")
                        .frame(maxWidth: .infinity, alignment: .center)
                }
                Button(role: .destructive, action: model.kickAndBanUser) {
                    Text("Kick and Ban User")
                        .frame(maxWidth: .infinity, alignment: .center)
                }
            }
        }
        .navigationTitle(model.displayName)
        .alert("Error", isPresented: Binding(
            get: { model.errorMessage != nil },
            set: { if !$0 { model.errorMessage = nil } }
        )) {
            Button("OK", role: .cancel) { }
        } message: {
            Text(model.errorMessage ?? "")
        }
    }
}
