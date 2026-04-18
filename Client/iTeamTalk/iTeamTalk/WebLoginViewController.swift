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
            Section(NSLocalizedString("Web Login ID", comment: "Web Login controller")) {
                if hasStoredUsername {
                    TeamTalkActionRow(
                        title: NSLocalizedString("Reset Web Login", comment: "Web Login Controller"),
                        role: .destructive,
                        action: resetWebLogin
                    )
                } else {
                    TeamTalkActionRow(
                        title: NSLocalizedString("Create Web Login", comment: "Web Login Controller"),
                        action: openWebLoginSignup
                    )
                }
            }

            Section(NSLocalizedString("Authentication", comment: "Web Login controller")) {
                TeamTalkTextFieldRow(
                    title: NSLocalizedString("Username", comment: "Web Login Controller"),
                    text: $usernameText,
                    isEnabled: !hasStoredUsername
                )

                if !hasStoredUsername {
                    TeamTalkTextFieldRow(
                        title: NSLocalizedString("Password", comment: "Web Login Controller"),
                        text: $passwordText,
                        isSecure: true
                    )

                    TeamTalkActionRow(
                        title: NSLocalizedString("Authenticate", comment: "Web Login Controller"),
                        action: validateWebLogin
                    )
                }
            }
        }
        .navigationTitle(NSLocalizedString("BearWare.dk Web Login", comment: "Web Login controller"))
        .onAppear(perform: loadWebLogin)
        .alert(NSLocalizedString("Authenticate", comment: "Web Login Controller"),
               isPresented: Binding(get: { alertMessage != nil }, set: { isPresented in
                   if !isPresented {
                       alertMessage = nil
                   }
               })) {
            Button(NSLocalizedString("OK", comment: "Web Login Controller"), role: .cancel) {
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
            alertMessage = NSLocalizedString("Username or password incorrect", comment: "Web Login Controller")
            return
        }

        let authParser = WebLoginParser()
        parser.delegate = authParser
        if parser.parse() && authParser.username.count > 0 {
            let fmtmsg = NSLocalizedString("%@, your username \"%@\" has been validated", comment: "Web Login Controller")
            alertMessage = String(format: fmtmsg, authParser.nickname, authParser.username)

            let settings = UserDefaults.standard
            settings.set(authParser.username, forKey: PREF_GENERAL_BEARWARE_ID)
            settings.set(authParser.token, forKey: PREF_GENERAL_BEARWARE_TOKEN)

            loadWebLogin()
        } else {
            alertMessage = NSLocalizedString("Username or password incorrect", comment: "Web Login Controller")
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

class WebLoginParser : NSObject, XMLParserDelegate {
    
    var elementStack = [String]()
    
    var username = "", nickname = "", token = ""
    
    func parser(_ parser: XMLParser, didStartElement elementName: String,
                namespaceURI: String?, qualifiedName qName: String?,
                attributes attributeDict: [String : String]) {
        
        elementStack.append(elementName)
    }
    
    func parser(_ parser: XMLParser, foundCharacters string: String) {

        let path = getXMLPath(elementStack: elementStack)
        switch path {
        case "/teamtalk/bearware/username" :
            username = string
        case "/teamtalk/bearware/nickname" :
            nickname = string
        case "/teamtalk/bearware/token" :
            token = string
        default :
            print("Unknown path " + path)
        }
    }
    
    func parser(_ parser: XMLParser, didEndElement elementName: String,
                namespaceURI: String?, qualifiedName qName: String?) {
        
        elementStack.removeLast()
    }
}
