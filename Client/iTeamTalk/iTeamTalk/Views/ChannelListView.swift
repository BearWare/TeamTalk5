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

// MARK: - Container view

struct ChannelListContainerView: View {
    @ObservedObject var model: ChannelListModel
    @State private var isPressingTalkButton = false

    var body: some View {
        VStack(spacing: 0) {
            ChannelListView(model: model)

            Text("Talk")
                .frame(maxWidth: .infinity)
                .frame(height: 50)
                .background(model.isTransmitting ? Color.red : Color.green)
                .foregroundStyle(.white)
                .fontWeight(.semibold)
                .contentShape(Rectangle())
            .simultaneousGesture(
                DragGesture(minimumDistance: 0)
                    .onChanged { _ in
                        guard !isPressingTalkButton else { return }
                        isPressingTalkButton = true
                        model.txBtnDown()
                    }
                    .onEnded { _ in
                        guard isPressingTalkButton else { return }
                        isPressingTalkButton = false
                        model.txBtnUp()
                    }
            )
            .accessibilityLabel("Push to Talk")
            .accessibilityHint(model.pttHint)
            .accessibilityValue(model.isTransmitting
                ? Text("Active")
                : Text("Inactive"))
            .accessibilityAddTraits(.isButton)
            .accessibilityAction {
                model.txBtnAccessibilityAction()
            }
        }
        .navigationTitle(model.navigationTitle)
        .alert("Enter Password", isPresented: $model.showingJoinPasswordAlert) {
            SecureField("Password", text: $model.joinPassword)
            Button("Join") { model.confirmJoinWithPassword() }
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
    }
}

// MARK: - List view

struct ChannelListView: View {
    @ObservedObject var model: ChannelListModel

    var body: some View {
        List(model.rows) { row in
            switch row {
            case .join:
                Button(action: model.joinCurrentChannel) {
                    Text("Join this channel")
                        .frame(maxWidth: .infinity, alignment: .center)
                }

            case .user(let user):
                let details = model.userDetails(user)
                HStack(spacing: 10) {
                    Image(details.iconName)
                        .resizable()
                        .frame(width: 36, height: 36)
                        .accessibilityLabel(details.iconAccessibilityLabel)

                    VStack(alignment: .leading, spacing: 2) {
                        Text(details.title)
                            .font(.body)
                            .lineLimit(1)
                        if let subtitle = details.subtitle {
                            Text(subtitle)
                                .font(.footnote)
                                .foregroundStyle(.secondary)
                                .lineLimit(2)
                        }
                    }

                    Spacer(minLength: 12)

                    Button {
                        model.showTextMessages(userid: user.nUserID)
                    } label: {
                        Image(details.messageIconName)
                            .resizable()
                            .frame(width: 24, height: 24)
                    }
                    .buttonStyle(.borderless)
                    .accessibilityLabel("Text Messaging")
                }
                .accessibilityElement(children: .contain)
                .contentShape(Rectangle())
                .onTapGesture {
                    model.selectRow(.user(user))
                }
                .accessibilityAction(named: "Mute") {
                    model.muteUser(userid: user.nUserID)
                }
                .accessibilityAction(named: "Move user") {
                    model.moveUser(userid: user.nUserID)
                }
                .accessibilityAction(named: "Kick user") {
                    model.kickUser(userid: user.nUserID)
                }
                .accessibilityAction(named: "Ban user") {
                    model.banUser(userid: user.nUserID)
                }

            case .channel(let channel):
                let details = model.channelDetails(channel)
                HStack(spacing: 10) {
                    Image(details.iconName)
                        .resizable()
                        .frame(width: 36, height: 36)
                        .accessibilityLabel(details.iconAccessibilityLabel)

                    VStack(alignment: .leading, spacing: 2) {
                        Text(limitText(details.title))
                            .font(.body)
                            .foregroundStyle(details.isParent ? .secondary : .primary)
                            .lineLimit(1)
                        if let subtitle = details.subtitle {
                            Text(subtitle)
                                .font(.footnote)
                                .foregroundStyle(.secondary)
                                .lineLimit(2)
                        }
                    }

                    Spacer(minLength: 12)

                    Button(details.actionTitle) {
                        model.showChannelDetail(channelID: channel.nChannelID)
                    }
                    .buttonStyle(.borderless)
                }
                .accessibilityElement(children: .contain)
                .contentShape(Rectangle())
                .onTapGesture {
                    model.selectRow(.channel(channel))
                }
                .accessibilityAction(named: "Move users here") {
                    model.moveIntoChannel(channelID: channel.nChannelID)
                }
                .accessibilityAction(named: "Join channel") {
                    model.joinChannelFromAccessibility(channelID: channel.nChannelID)
                }
            }
        }
    }
}

// MARK: - Detail structs

struct ChannelUserDetails {
    let title: String
    let subtitle: String?
    let iconName: String
    let iconAccessibilityLabel: String
    let messageIconName: String
}

struct ChannelDisplayDetails {
    let title: String
    let subtitle: String?
    let iconName: String
    let iconAccessibilityLabel: String
    let actionTitle: String
    let isParent: Bool
}
