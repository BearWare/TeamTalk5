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

struct WebLoginView: View {
    @Environment(\.openURL) private var openURL
    @State private var usernameText = ""
    @State private var passwordText = ""
    @State private var hasStoredUsername = false
    @State private var alertMessage: String?

    var body: some View {
        Form {
            Section("Web Login ID") {
                if hasStoredUsername {
                    Button(role: .destructive, action: resetWebLogin) {
                        Text("Reset Web Login")
                            .frame(maxWidth: .infinity, alignment: .center)
                    }
                } else {
                    Button(action: openWebLoginSignup) {
                        Text("Create Web Login")
                            .frame(maxWidth: .infinity, alignment: .center)
                    }
                }
            }

            Section("Authentication") {
                TextField("Username", text: $usernameText)
                    .multilineTextAlignment(.trailing)
                    .autocorrectionDisabled()
                    .textInputAutocapitalization(.never)
                    .disabled(hasStoredUsername)

                if !hasStoredUsername {
                    SecureField("Password", text: $passwordText)
                        .textInputAutocapitalization(.never)

                    Button(action: validateWebLogin) {
                        Text("Authenticate")
                            .frame(maxWidth: .infinity, alignment: .center)
                    }
                }
            }
        }
        .navigationTitle("BearWare.dk Web Login")
        .onAppear(perform: loadWebLogin)
        .alert("Authenticate",
               isPresented: Binding(get: { alertMessage != nil }, set: { isPresented in
                   if !isPresented {
                       alertMessage = nil
                   }
               })) {
            Button("OK", role: .cancel) {
                alertMessage = nil
            }
        } message: {
            Text(alertMessage ?? "")
        }
    }

    private func loadWebLogin() {
        let settings = UserDefaults.standard
        let username = settings.string(forKey: PREF_GENERAL_BEARWARE_ID)
        usernameText = username ?? ""
        passwordText = ""
        hasStoredUsername = username != nil
    }

    private func validateWebLogin() {
        let username = usernameText.trimmingCharacters(in: .whitespacesAndNewlines)
        usernameText = username

        let tokenURL = AppInfo.getBearWareTokenURL(username: username, passwd: passwordText)

        guard let url = URL(string: tokenURL), let parser = XMLParser(contentsOf: url) else {
            alertMessage = String(localized: "Username or password incorrect", comment: "Web Login Controller")
            return
        }

        let authParser = WebLoginParser()
        parser.delegate = authParser
        if parser.parse() && authParser.username.count > 0 {
            let fmtmsg = String(localized: "%@, your username \"%@\" has been validated", comment: "Web Login Controller")
            alertMessage = String(format: fmtmsg, authParser.nickname, authParser.username)

            let settings = UserDefaults.standard
            settings.set(authParser.username, forKey: PREF_GENERAL_BEARWARE_ID)
            settings.set(authParser.token, forKey: PREF_GENERAL_BEARWARE_TOKEN)

            loadWebLogin()
        } else {
            alertMessage = String(localized: "Username or password incorrect", comment: "Web Login Controller")
        }
    }

    private func openWebLoginSignup() {
        if let url = URL(string: AppInfo.BEARWARE_REGISTRATION_WEBSITE) {
            openURL(url)
        }
    }

    private func resetWebLogin() {
        let settings = UserDefaults.standard
        settings.set(nil, forKey: PREF_GENERAL_BEARWARE_ID)
        settings.set(nil, forKey: PREF_GENERAL_BEARWARE_TOKEN)

        loadWebLogin()
    }
}
