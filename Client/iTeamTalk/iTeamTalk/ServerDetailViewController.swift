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
import UIKit

final class ServerDetailViewController: UIHostingController<ServerDetailView> {

    var server = Server()
    var onConnect: ((Server) -> Void)?
    var onDelete: (() -> Void)?
    var onSave: ((Server) -> Void)?
    private var model: ServerDetailModel

    init() {
        let model = ServerDetailModel(server: Server())
        self.model = model
        super.init(rootView: ServerDetailView(model: model, copyJoinCode: { }, connect: { }, delete: { }, save: { }))
    }

    required init?(coder: NSCoder) { fatalError("use init()") }

    override func viewDidLoad() {
        super.viewDidLoad()
        navigationItem.title = NSLocalizedString("Server Entry", comment: "server entry")

        let model = ServerDetailModel(server: server)
        self.model = model
        rootView = ServerDetailView(
            model: model,
            copyJoinCode: { [weak self] in self?.copyJoinCode() },
            connect: { [weak self] in self?.connectServer() },
            delete: { [weak self] in self?.deleteServer() },
            save: { [weak self] in self?.saveServerDetail() }
        )
    }

    @objc func copyJoinCode() {
        UIPasteboard.general.string = server.joincode
    }

    func saveServerDetail() {
        model.apply(to: server)
        onSave?(server)
    }

    private func connectServer() {
        //saveServerDetail()
        onConnect?(server)
    }

    private func deleteServer() {
        onDelete?()
    }
}

final class ServerDetailModel: ObservableObject {
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
                ServerStatusRow(title: NSLocalizedString("Users Online", comment: "server entry"), value: String(server.stats_usercount)),
                ServerStatusRow(title: NSLocalizedString("Message of the Day", comment: "server entry"), value: server.stats_motd),
                ServerStatusRow(title: NSLocalizedString("Country", comment: "server entry"), value: server.stats_country)
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

struct ServerDetailView: View {
    @ObservedObject var model: ServerDetailModel

    let copyJoinCode: () -> Void
    let connect: () -> Void
    let delete: () -> Void
    let save: () -> Void
    var body: some View {
        Form {
            Section(NSLocalizedString("Server List Entry", comment: "server entry")) {
                TeamTalkTextFieldRow(
                    title: NSLocalizedString("Name", comment: "server entry"),
                    placeholder: NSLocalizedString("Name", comment: "server entry"),
                    text: $model.nameText
                )
            }

            if !model.statusRows.isEmpty {
                Section(NSLocalizedString("Server Status", comment: "server entry")) {
                    ForEach(model.statusRows) { row in
                        TeamTalkValueRow(title: row.title, value: row.value)
                    }
                }
            }

            Section(NSLocalizedString("Connection", comment: "server entry")) {
                TeamTalkTextFieldRow(
                    title: NSLocalizedString("Host address", comment: "server entry"),
                    placeholder: NSLocalizedString("Host address", comment: "server entry"),
                    text: $model.hostText
                )
                .keyboardType(.URL)
                TeamTalkTextFieldRow(
                    title: NSLocalizedString("TCP Port", comment: "server entry"),
                    placeholder: NSLocalizedString("TCP Port", comment: "server entry"),
                    text: $model.tcpPortText
                )
                .keyboardType(.numberPad)
                TeamTalkTextFieldRow(
                    title: NSLocalizedString("UDP Port", comment: "server entry"),
                    placeholder: NSLocalizedString("UDP Port", comment: "server entry"),
                    text: $model.udpPortText
                )
                .keyboardType(.numberPad)
                TeamTalkToggleRow(
                    title: NSLocalizedString("Encrypted", comment: "server entry"),
                    isOn: $model.isEncrypted
                )
            }

            Section(NSLocalizedString("Authentication", comment: "server entry")) {
                if !model.isWebLogin {
                    TeamTalkTextFieldRow(
                        title: NSLocalizedString("Username", comment: "server entry"),
                        placeholder: NSLocalizedString("Username", comment: "server entry"),
                        text: $model.usernameText
                    )
                    TeamTalkTextFieldRow(
                        title: NSLocalizedString("Password", comment: "server entry"),
                        placeholder: NSLocalizedString("Password", comment: "server entry"),
                        text: $model.passwordText,
                        isSecure: true
                    )
                }
                TeamTalkToggleRow(
                    title: NSLocalizedString("BearWare.dk Web Login", comment: "server entry"),
                    subtitle: NSLocalizedString("Check only if this server has enabled BearWare.dk Web Login", comment: "server entry"),
                    isOn: $model.isWebLogin
                )
                TeamTalkTextFieldRow(
                    title: NSLocalizedString("Nickname (optional)", comment: "server entry"),
                    placeholder: NSLocalizedString("Nickname (optional)", comment: "server entry"),
                    text: $model.nicknameText
                )
            }

            Section(NSLocalizedString("Join Channel", comment: "server entry")) {
                TeamTalkTextFieldRow(
                    title: NSLocalizedString("Channel", comment: "server entry"),
                    placeholder: NSLocalizedString("Channel", comment: "server entry"),
                    text: $model.channelText
                )
                TeamTalkTextFieldRow(
                    title: NSLocalizedString("Password", comment: "server entry"),
                    placeholder: NSLocalizedString("Password", comment: "server entry"),
                    text: $model.channelPasswordText,
                    isSecure: true
                )
            }

            Section(NSLocalizedString("Actions", comment: "server entry")) {
                if !model.joinCode.isEmpty {
                    TeamTalkValueRow(
                        title: NSLocalizedString("Join Code", comment: "server entry"),
                        value: model.joinCode
                    )
                    TeamTalkActionRow(title: NSLocalizedString("Copy Join Code", comment: "server entry"), action: copyJoinCode)
                }
                TeamTalkActionRow(title: NSLocalizedString("Connect", comment: "server entry"), action: connect)
                TeamTalkActionRow(title: NSLocalizedString("Delete Server", comment: "server entry"), role: .destructive, action: delete)
                TeamTalkActionRow(title: NSLocalizedString("Save server", comment: "server entry"), action: save)
            }
        }
    }
}

struct ServerStatusRow: Identifiable {
    let title: String
    let value: String

    var id: String {
        title
    }
}
