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

struct ChannelDetailView: View {
    @ObservedObject var model: ChannelDetailModel
    @Environment(\.dismiss) private var dismiss

    let setupCodec: () -> Void

    var body: some View {
        Form {
            Section("Channel Properties") {
                TextField("Name", text: $model.nameText)
                    .multilineTextAlignment(.trailing)
                    .autocorrectionDisabled()
                    .textInputAutocapitalization(.never)
                TextField("Password", text: $model.passwordText)
                    .multilineTextAlignment(.trailing)
                    .autocorrectionDisabled()
                    .textInputAutocapitalization(.never)
                TextField("Topic", text: $model.topicText)
                    .multilineTextAlignment(.trailing)
                    .autocorrectionDisabled()
                    .textInputAutocapitalization(.never)
                Button(action: setupCodec) {
                    LabeledContent("Audio Codec",
                                   value: model.codecDescription)
                }
                .buttonStyle(.plain)
                Toggle("Permanent Channel", isOn: $model.isPermanent)
                Toggle("No Interruptions", isOn: $model.hasNoInterruptions)
                Toggle("No Voice Activation", isOn: $model.hasNoVoiceActivation)
                Toggle("No Audio Recording", isOn: $model.hasNoAudioRecording)
                Toggle("Hidden Channel", isOn: $model.isHidden)
            }

            if model.isExistingChannel {
                Section("Commands") {
                    Button(action: model.joinChannelPressed) {
                        Text("Join Channel")
                            .frame(maxWidth: .infinity, alignment: .center)
                    }
                    Button(role: .destructive, action: model.deleteChannel) {
                        Text("Delete Channel")
                            .frame(maxWidth: .infinity, alignment: .center)
                    }
                }
            }
        }
        .navigationTitle(model.navigationTitle)
        .toolbar {
            ToolbarItem(placement: .cancellationAction) {
                Button("Cancel", role: .cancel) {
                    dismiss()
                }
            }
            ToolbarItem(placement: .confirmationAction) {
                Button("Save") {
                    model.createOrUpdate()
                }
            }
        }
        .alert("Enter Password", isPresented: $model.showingJoinAlert) {
            SecureField("Password", text: $model.joinPassword)
            Button("Join") { model.joinWithPassword() }
            Button("Cancel", role: .cancel) { }
        } message: {
            Text("Password")
        }
        .alert("Error", isPresented: Binding(
            get: { model.errorMessage != nil },
            set: { if !$0 { model.errorMessage = nil } }
        )) {
            Button("OK", role: .cancel) { }
        } message: {
            Text(model.errorMessage ?? "")
        }
        .onChange(of: model.shouldDismiss) { newValue in
            if newValue { dismiss() }
        }
    }
}
