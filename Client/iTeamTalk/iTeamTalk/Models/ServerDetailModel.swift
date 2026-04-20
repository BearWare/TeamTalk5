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

final class ServerDetailModel: ObservableObject {
    let server: Server
    let joinCode: String
    let statusRows: [ServerStatusRow]

    @Published var nameText: String
    @Published var hostText: String
    @Published var tcpPortText: String
    @Published var udpPortText: String
    @Published var isEncrypted: Bool
    @Published var usernameText: String
    @Published var passwordText: String
    @Published var nicknameText: String
    @Published var isWebLogin: Bool
    @Published var channelText: String
    @Published var channelPasswordText: String

    init(server: Server) {
        self.server = server
        joinCode = server.joincode
        nameText = server.name
        hostText = server.ipaddr
        tcpPortText = String(server.tcpport)
        udpPortText = String(server.udpport)
        isEncrypted = server.encrypted
        usernameText = server.username
        passwordText = server.password
        nicknameText = server.nickname
        isWebLogin = AppInfo.isBearWareWebLogin(server.username)
        channelText = server.channel
        channelPasswordText = server.chanpasswd

        if server.servertype != .LOCAL {
            statusRows = [
                ServerStatusRow(title: String(localized: "Users Online", comment: "server entry"), value: String(server.stats_usercount)),
                ServerStatusRow(title: String(localized: "Message of the Day", comment: "server entry"), value: server.stats_motd),
                ServerStatusRow(title: String(localized: "Country", comment: "server entry"), value: server.stats_country)
            ]
        } else {
            statusRows = []
        }
    }

    func apply(to server: Server) {
        server.name = nameText
        server.ipaddr = hostText
        if let n = Int(tcpPortText) {
            server.tcpport = n
        }
        if let n = Int(udpPortText) {
            server.udpport = n
        }
        server.encrypted = isEncrypted
        server.username = isWebLogin ? AppInfo.WEBLOGIN_BEARWARE_USERNAME : usernameText.trimmingCharacters(in: .whitespacesAndNewlines)
        server.password = isWebLogin ? "" : passwordText
        server.nickname = nicknameText
        server.servertype = .LOCAL
        server.channel = channelText.trimmingCharacters(in: .whitespacesAndNewlines)
        server.chanpasswd = channelPasswordText
    }
}

extension ServerDetailModel: Identifiable {
    var id: ObjectIdentifier { ObjectIdentifier(self) }
}

struct ServerStatusRow: Identifiable {
    let title: String
    let value: String

    var id: String {
        title
    }
}
