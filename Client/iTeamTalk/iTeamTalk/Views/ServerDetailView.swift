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

struct ServerDetailView: View {
    @Environment(\.dismiss) private var dismiss
    @ObservedObject var model: ServerDetailModel
    @State private var showServerPassword = false
    @State private var showChannelPassword = false

    let copyJoinCode: () -> Void
    let connect: () -> Void
    let delete: () -> Void
    let save: () -> Void

    var body: some View {
        Form {
            Section("Server List Entry") {
                formTextField("Name", text: $model.nameText, )
            }

            if !model.statusRows.isEmpty {
                Section("Server Status") {
                    ForEach(model.statusRows) { row in
                        LabeledContent(row.title, value: row.value)
                    }
                }
            }

            Section("Connection") {
                formTextField("Host address", text: $model.hostText, keyboardType: .URL)
                formTextField("TCP Port", text: $model.tcpPortText, keyboardType: .numberPad)
                formTextField("UDP Port", text: $model.udpPortText, keyboardType: .numberPad)
                Toggle("Encrypted", isOn: $model.isEncrypted)
            }

            Section("Authentication") {
                if !model.isWebLogin {
                    formTextField("Username", text: $model.usernameText)
                    formPasswordField("Password", text: $model.passwordText, isRevealed: showServerPassword)
                    Toggle(isOn: $showServerPassword) {
                        Text("Show password")
                    }
                }
                Toggle(isOn: $model.isWebLogin) {
                    VStack(alignment: .leading, spacing: 2) {
                        Text("BearWare.dk Web Login")
                        Text("Check only if this server has enabled BearWare.dk Web Login")
                            .font(.footnote)
                            .foregroundStyle(.secondary)
                    }
                }
                formTextField("Nickname (optional)", text: $model.nicknameText)
            }

            Section("Join Channel") {
                formTextField("Channel name", text: $model.channelText)
                formPasswordField("Channel password", text: $model.channelPasswordText, isRevealed: showChannelPassword)
                Toggle(isOn: $showChannelPassword) {
                    Text("Show password")
                }
            }

            Section("Actions") {
                if !model.joinCode.isEmpty {
                    LabeledContent("Join Code", value: model.joinCode)
                    Button(action: copyJoinCode) {
                        Text("Copy Join Code")
                            .frame(maxWidth: .infinity, alignment: .center)
                    }
                }
                Button(action: connect) {
                    Text("Connect")
                        .frame(maxWidth: .infinity, alignment: .center)
                }
                .disabled(model.hostText.isEmpty || model.tcpPortText.isEmpty || model.udpPortText.isEmpty)
                Button(role: .destructive, action: delete) {
                    Text("Delete Server")
                        .frame(maxWidth: .infinity, alignment: .center)
                }
                /*Button(action: save) {
                    Text("Save server")
                        .frame(maxWidth: .infinity, alignment: .center)
                }*/
            }
        }
        .navigationTitle("Server Entry")
        .toolbar {
            ToolbarItem(placement: .cancellationAction) {
                Button("Cancel", role: .cancel) {
                    dismiss()
                }
            }
            ToolbarItem(placement: .confirmationAction) {
                Button("Save server", action: save)
                    .disabled(model.hostText.isEmpty || model.tcpPortText.isEmpty || model.udpPortText.isEmpty)
            }
        }
    }

    private func formPasswordField(
        _ title: LocalizedStringKey,
        text: Binding<String>,
        isRevealed: Bool
    ) -> some View {
        LabeledContent {
            Group {
                if isRevealed {
                    TextField("", text: text)
                        .autocorrectionDisabled()
                } else {
                    SecureField("", text: text)
                }
            }
            .frame(maxWidth: .infinity, alignment: .trailing)
            .multilineTextAlignment(.trailing)
            .textInputAutocapitalization(.never)
            .accessibilityLabel(Text(title))
        } label: {
            Text(title)
                .accessibilityHidden(true)
        }
    }
    
}
