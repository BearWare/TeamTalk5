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

struct ServerListView: View {
    @ObservedObject var model: ServerListModel

    var body: some View {
        Group {
            if let mainTabModel = model.activeMainTabModel {
                MainTabView(model: mainTabModel, close: {
                    model.closeActiveServer()
                })
            } else {
                serverList
            }
        }
        .onReceive(NotificationCenter.default.publisher(for: .iTeamTalkOpenURL)) { notification in
            guard let url = notification.object as? URL else { return }
            model.openUrl(url)
        }
    }

    private var serverList: some View {
        NavigationStack(path: $model.navigationPath) {
            List {
                Button {
                    model.showJoinCodeAlert = true
                } label: {
                    Text("Enter Join Code")
                        .frame(maxWidth: .infinity, alignment: .center)
                }

                ForEach(model.servers, id: \.self) { server in
                    HStack(spacing: 10) {
                        Image(iconName(for: server))
                            .resizable()
                            .frame(width: 36, height: 36)
                            .accessibilityLabel(iconAccessibilityLabel(for: server))

                        VStack(alignment: .leading, spacing: 2) {
                            Text(server.name)
                                .font(.body)
                                .lineLimit(1)
                            Text(detail(for: server))
                                .font(.footnote)
                                .foregroundStyle(.secondary)
                                .lineLimit(2)
                        }

                        Spacer(minLength: 12)

                        Button("Connect") {
                            model.connect(to: server)
                        }
                        .buttonStyle(.bordered)
                    }
                    .accessibilityElement(children: .contain)
                    .contentShape(Rectangle())
                    .onTapGesture {
                        model.showServerDetail(for: server)
                    }
                    .swipeActions(edge: .trailing) {
                        Button(role: .destructive) {
                            model.deleteServer(server)
                        } label: {
                            Label("Delete", systemImage: "trash")
                        }
                    }
                    .accessibilityAction(named: "Connect to server") {
                        model.connect(to: server)
                    }
                    /*.accessibilityAction(named: "Delete server from list") {
                        model.deleteServer(server)
                    }*/
                }
            }
            .navigationTitle("TeamTalk Servers")
            .toolbar {
                ToolbarItem(placement: .navigationBarLeading) {
                    Button {
                        model.openPreferences()
                    } label: {
                        Image("setup")
                            .accessibilityLabel("Preferences")
                            .accessibilityHint("Access preferences")
                    }
                }
                ToolbarItem(placement: .navigationBarTrailing) {
                    Button {
                        model.addServer()
                    } label: {
                        Image(systemName: "plus")
                            .accessibilityLabel("Add new server entry")
                    }
                }
            }
            .navigationDestination(for: ServerListDestination.self) { destination in
                destinationView(destination)
            }
            .alert("Connect to Server",
                isPresented: $model.showJoinCodeAlert
            ) {
                TextField("Type Join Code",
                    text: $model.joinCodeInput
                )
                .autocorrectionDisabled()
                Button("OK") {
                    model.submitJoinCode()
                }
                Button("Cancel", role: .cancel) {
                    model.joinCodeInput = ""
                }
            } message: {
                Text("Enter Join Code")
            }
            .alert("Connect to Server",
                isPresented: Binding(
                    get: { model.errorMessage != nil },
                    set: { if !$0 { model.errorMessage = nil } }
                )
            ) {
                Button("OK", role: .cancel) {}
            } message: {
                Text(model.errorMessage ?? "")
            }
            .onAppear {
                model.onAppear()
            }
            .sheet(item: $model.serverDetailModel) { detailModel in
                ServerDetailSheetView(detailModel: detailModel, listModel: model)
                    .presentationDragIndicator(.visible)
            }
        }
    }

    @ViewBuilder
    private func destinationView(_ destination: ServerListDestination) -> some View {
        switch destination {
        case .preferences(let m):
            PreferencesView(model: m)
        }
    }

    private func detail(for server: Server) -> String {
        var detail = "\(server.ipaddr):\(server.tcpport)"
        if server.servertype != .LOCAL {
            detail += ", " + String(format: String(localized: "Users: %d, Country: %@", comment: "serverlist"), server.stats_usercount, server.stats_country)
        }
        return detail
    }

    private func iconName(for server: Server) -> String {
        switch server.servertype {
        case .LOCAL:
            return "teamtalk_yellow.png"
        case .OFFICIAL:
            return "teamtalk_blue.png"
        case .PUBLIC:
            return "teamtalk_green.png"
        case .UNOFFICIAL:
            return "teamtalk_orange.png"
        }
    }

    private func iconAccessibilityLabel(for server: Server) -> String {
        switch server.servertype {
        case .LOCAL:
            return String(localized: "Local server", comment: "serverlist")
        case .OFFICIAL:
            return String(localized: "Official server", comment: "serverlist")
        case .PUBLIC:
            return String(localized: "Public server", comment: "serverlist")
        case .UNOFFICIAL:
            return String(localized: "Unofficial server", comment: "serverlist")
        }
    }
}

// MARK: - Server detail sheet

private struct ServerDetailSheetView: View {
    @Environment(\.dismiss) private var dismiss
    let detailModel: ServerDetailModel
    let listModel: ServerListModel

    var body: some View {
        NavigationStack {
            ServerDetailView(
                model: detailModel,
                copyJoinCode: {
                    UIPasteboard.general.string = detailModel.server.joincode
                },
                connect: {
                    dismiss()
                    detailModel.apply(to: detailModel.server)
                    listModel.connect(to: detailModel.server)
                },
                delete: {
                    listModel.deleteServer(detailModel.server)
                    dismiss()
                },
                save: {
                    detailModel.apply(to: detailModel.server)
                    listModel.upsertServer(detailModel.server)
                    dismiss()
                }
            )

        }
    }
}
