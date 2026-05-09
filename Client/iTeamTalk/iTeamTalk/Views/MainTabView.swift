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

struct MainTabView: View {
    @ObservedObject var model: MainTabModel
    let close: () -> Void
    @State private var saveAlertName = String(localized: "New Server", comment: "Dialog message")

    var body: some View {
        TabView {
            ChannelsTabView(mainModel: model, model: model.channelListModel, close: close)
                .accessibilityAction(.magicTap) {
                    model.channelListModel.txBtnAccessibilityAction()
                }
            .tabItem {
                Label("Channels", image: "channels")
            }
            .tag(0)

            // Messages tab
            NavigationStack {
                TextMessageView(model: model.channelChatModel)
                    .accessibilityAction(.magicTap) {
                        model.channelListModel.txBtnAccessibilityAction()
                    }
            }
            .tabItem {
                Label("Messages", image: "messages")
            }
            .tag(1)

            // Preferences tab
            NavigationStack {
                PreferencesView(model: model.preferencesModel)
                    .accessibilityAction(.magicTap) {
                        model.channelListModel.txBtnAccessibilityAction()
                    }
            }
            .tabItem {
                Label("Preferences", image: "setup")
            }
            .tag(2)
        }
        .onAppear {
            model.setup()
            model.onVisibleAppear()
        }
        .onDisappear {
            model.teardown()
        }
        .onReceive(NotificationCenter.default.publisher(for: .iTeamTalkRemoteControl)) { notification in
            model.remoteControl(notification.object as? UIEvent)
        }
        .alert("Error",
               isPresented: Binding(
                get: { model.alertMessage != nil },
                set: { if !$0 { model.alertMessage = nil } }
               )) {
            Button("OK", role: .cancel) {}
        } message: {
            Text(model.alertMessage ?? "")
        }
        .alert("Connect to Server",
            isPresented: Binding(
                get: { model.fatalAlertMessage != nil },
                set: { if !$0 { model.fatalAlertMessage = nil } }
            )
        ) {
            Button("OK", role: .cancel) {
                close()
            }
        } message: {
            Text(model.fatalAlertMessage ?? "")
        }
        .alert("Save Server",
            isPresented: $model.showSaveAlert
        ) {
            TextField("New Server",
                text: $saveAlertName
            )
            Button("No", role: .cancel) {
                model.skipSaveAndDisconnect()
            }
            Button("Yes") {
                model.saveAndDisconnect(name: saveAlertName)
            }
        } message: {
            Text("Save server to server list?")
        }
    }
}

// MARK: - Channels tab

private struct ChannelsTabView: View {
    @ObservedObject var mainModel: MainTabModel
    @ObservedObject var model: ChannelListModel
    let close: () -> Void

    var body: some View {
        NavigationStack(path: $model.navigationPath) {
            ChannelListContainerView(model: model)
                .navigationDestination(for: ChannelListDestination.self) { destination in
                    channelDestinationView(destination)
                }
                .toolbar {
                    ToolbarItem(placement: .navigationBarLeading) {
                        Button("Disconnect") {
                            mainModel.disconnectTapped(dismiss: close)
                        }
                    }
                    ToolbarItem(placement: .navigationBarTrailing) {
                        Button {
                            model.showNewChannel()
                        } label: {
                            Image(systemName: "plus")
                                .accessibilityLabel("Create new channel")
                        }
                    }
                }
                .sheet(item: $model.channelDetailModel) { detailModel in
                    ChannelDetailSheetView(model: detailModel)
                        .presentationDragIndicator(.visible)
                }
        }
    }

    @ViewBuilder
    private func channelDestinationView(_ destination: ChannelListDestination) -> some View {
        switch destination {
        case .userDetail(let m):
            UserDetailView(model: m)

        case .textMessage(let m):
            TextMessageView(model: m)
        }
    }
}

// MARK: - Channel detail sheet

private struct ChannelDetailSheetView: View {
    @ObservedObject var model: ChannelDetailModel

    var body: some View {
        NavigationStack {
            ChannelDetailView(model: model, setupCodec: {
                model.audioCodecModel = model.makeAudioCodecModel()
            })
            .navigationDestination(isPresented: Binding(
                get: { model.audioCodecModel != nil },
                set: { if !$0 { model.audioCodecModel = nil } }
            )) {
                if let codecModel = model.audioCodecModel {
                    AudioCodecView(model: codecModel, performAction: { action in
                        model.applyCodecAction(action, codecModel: codecModel)
                    })
                }
            }
        }
    }
}
