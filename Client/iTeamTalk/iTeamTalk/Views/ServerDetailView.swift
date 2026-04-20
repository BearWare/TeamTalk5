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
    @State private var showPassword = false

    let copyJoinCode: () -> Void
    let connect: () -> Void
    let delete: () -> Void
    let save: () -> Void

    var body: some View {
        Form {
            Section("Server List Entry") {
                TextField("Name", text: $model.nameText)
                    .multilineTextAlignment(.trailing)
                    .autocorrectionDisabled()
                    .textInputAutocapitalization(.never)
            }

            if !model.statusRows.isEmpty {
                Section("Server Status") {
                    ForEach(model.statusRows) { row in
                        LabeledContent(row.title, value: row.value)
                    }
                }
            }

            Section("Connection") {
                TextField("Host address", text: $model.hostText)
                    .multilineTextAlignment(.trailing)
                    .autocorrectionDisabled()
                    .textInputAutocapitalization(.never)
                    .keyboardType(.URL)
                TextField("TCP Port", text: $model.tcpPortText)
                    .multilineTextAlignment(.trailing)
                    .autocorrectionDisabled()
                    .textInputAutocapitalization(.never)
                    .keyboardType(.numberPad)
                TextField("UDP Port", text: $model.udpPortText)
                    .multilineTextAlignment(.trailing)
                    .autocorrectionDisabled()
                    .textInputAutocapitalization(.never)
                    .keyboardType(.numberPad)
                Toggle("Encrypted", isOn: $model.isEncrypted)
            }

            Section("Authentication") {
                if !model.isWebLogin {
                    TextField("Username", text: $model.usernameText)
                        .multilineTextAlignment(.trailing)
                        .autocorrectionDisabled()
                        .textInputAutocapitalization(.never)
                    if !showPassword {
                        SecureField("Password", text: $model.passwordText)
                            .textInputAutocapitalization(.never)
                    } else {
                        TextField("Password", text: $model.passwordText)
                            .multilineTextAlignment(.trailing)
                            .textInputAutocapitalization(.never)
                            .autocorrectionDisabled()
                    }
                    Toggle(isOn: $showPassword) {
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
                TextField("Nickname (optional)", text: $model.nicknameText)
                    .multilineTextAlignment(.trailing)
                    .autocorrectionDisabled()
                    .textInputAutocapitalization(.never)
            }

            Section("Join Channel") {
                TextField("Channel", text: $model.channelText)
                    .multilineTextAlignment(.trailing)
                    .autocorrectionDisabled()
                    .textInputAutocapitalization(.never)
                if !showPassword {
                    SecureField("Password", text: $model.channelPasswordText)
                        .textInputAutocapitalization(.never)
                } else {
                    TextField("Channel password", text: $model.channelPasswordText)
                        .multilineTextAlignment(.trailing)
                        .autocorrectionDisabled()
                        .textInputAutocapitalization(.never)
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
                    .disabled(model.hostText.isEmpty)
            }
        }
    }
}
