import Foundation
import TeamTalkC

// Fire-and-forget commands: each sends a request to the server and returns
// immediately with a `TeamTalkCommandID`. Success/failure arrives later as a
// `TeamTalkEvent.Kind.commandSucceeded`/`.commandError` carrying that same ID
// (or as a more specific event, e.g. `.channelCreated`), not through this
// call. For a `try await` alternative that resolves once the matching event
// arrives, see the wrappers in TeamTalkSessionAsyncCommands.swift.
extension TeamTalkSession {
@discardableResult
public func ping() -> TeamTalkCommandID {
    TeamTalkCommandID(TT_DoPing(instance))
}

@discardableResult
public func logIn(nickname: String, username: String, password: String, clientName: String = "") -> TeamTalkCommandID {
    TeamTalkCommandID(login(nickname: nickname, username: username, password: password, clientName: clientName))
}

@discardableResult
public func logOut() -> TeamTalkCommandID {
    TeamTalkCommandID(TT_DoLogout(instance))
}

@discardableResult
public func joinChannel(_ channel: TeamTalkChannel, password: String = "") -> TeamTalkCommandID {
    joinChannel(withID: channel.channelID.cValue, password: password)
}

@discardableResult
public func joinChannel(_ configuration: TeamTalkChannelConfiguration, password: String = "") -> TeamTalkCommandID {
    var channel = configuration.cValue
    return TeamTalkCommandID(TT_DoJoinChannel(instance, &channel))
}

@discardableResult
internal func joinChannel(withID channelID: Int32, password: String = "") -> TeamTalkCommandID {
    TeamTalkCommandID(joinChannel(id: channelID, password: password))
}

@discardableResult
public func leaveChannel() -> TeamTalkCommandID {
    TeamTalkCommandID(TT_DoLeaveChannel(instance))
}

@discardableResult
public func createChannel(_ configuration: TeamTalkChannelConfiguration) -> TeamTalkCommandID {
    var channel = configuration.cValue
    return TeamTalkCommandID(TT_DoMakeChannel(instance, &channel))
}

@discardableResult
public func updateChannel(_ channel: TeamTalkChannel) -> TeamTalkCommandID {
    var channel = channel.cValue
    return TeamTalkCommandID(TT_DoUpdateChannel(instance, &channel))
}

@discardableResult
public func updateChannel(_ configuration: TeamTalkChannelConfiguration) -> TeamTalkCommandID {
    var channel = configuration.cValue
    return TeamTalkCommandID(TT_DoUpdateChannel(instance, &channel))
}

@discardableResult
public func removeChannel(_ channel: TeamTalkChannel) -> TeamTalkCommandID {
    TeamTalkCommandID(removeChannel(id: channel.channelID.cValue))
}

@discardableResult
public func setNickname(_ nickname: String) -> TeamTalkCommandID {
    TeamTalkCommandID(changeNickname(nickname))
}

@discardableResult
public func setStatus(mode: TeamTalkStatusMode, message: String = "") -> TeamTalkCommandID {
    TeamTalkCommandID(changeStatus(mode: mode.rawValue, message: message))
}

@discardableResult
public func sendTextMessage(_ message: TeamTalkOutgoingTextMessage) -> [TeamTalkCommandID] {
    TeamTalkTextMessageFactory.messages(from: message.cValue, content: message.content).map { textMessage in
        var textMessage = textMessage
        return TeamTalkCommandID(TT_DoTextMessage(instance, &textMessage))
    }
}

@discardableResult
public func sendTextMessage(to user: TeamTalkUser, content: String) -> [TeamTalkCommandID] {
    sendTextMessage(.user(to: user, content: content))
}

@discardableResult
public func sendTextMessage(to channel: TeamTalkChannel, content: String) -> [TeamTalkCommandID] {
    sendTextMessage(.channel(channel, content: content))
}

@discardableResult
public func sendChannelMessage(_ content: String) -> [TeamTalkCommandID] {
    guard let channel = currentChannel() else {
        return []
    }
    return sendTextMessage(to: channel, content: content)
}

@discardableResult
public func reply(to message: TeamTalkTextMessage, content: String) -> [TeamTalkCommandID] {
    sendTextMessage(.reply(to: message, content: content))
}

// Plain overload: only works if this client is already an operator of
// `channel` (or has the server-wide right). Use the `operatorPassword:`
// overload below to claim operator status with the channel's op password
// instead.
@discardableResult
public func setChannelOperator(_ user: TeamTalkUser, in channel: TeamTalkChannel, enabled: Bool) -> TeamTalkCommandID {
    TeamTalkCommandID(TT_DoChannelOp(instance, user.userID.cValue, channel.channelID.cValue, enabled ? 1 : 0))
}

@discardableResult
public func setChannelOperator(
    _ user: TeamTalkUser,
    in channel: TeamTalkChannel,
    operatorPassword: String,
    enabled: Bool
) -> TeamTalkCommandID {
    TeamTalkCommandID(TT_DoChannelOpEx(instance, user.userID.cValue, channel.channelID.cValue, operatorPassword, enabled ? 1 : 0))
}

@discardableResult
public func kickUser(_ user: TeamTalkUser, from channel: TeamTalkChannel? = nil) -> TeamTalkCommandID {
    TeamTalkCommandID(kickUser(id: user.userID.cValue, fromChannelID: channel?.channelID.cValue ?? 0))
}

@discardableResult
public func banUser(_ user: TeamTalkUser, from channel: TeamTalkChannel? = nil) -> TeamTalkCommandID {
    TeamTalkCommandID(banUser(id: user.userID.cValue, fromChannelID: channel?.channelID.cValue ?? 0))
}

// Bans by channel, IP, and/or username, per `types`, instead of the plain
// `banUser(_:from:)` overload's default (IP-based channel ban).
@discardableResult
public func banUser(_ user: TeamTalkUser, types: TeamTalkBanTypes) -> TeamTalkCommandID {
    TeamTalkCommandID(TT_DoBanUserEx(instance, user.userID.cValue, types.cValue))
}

/// Bans an address/account that isn't necessarily an online user right now,
/// via a fully-specified ``TeamTalkBanConfiguration`` rather than an existing
/// ``TeamTalkUser``.
@discardableResult
public func ban(_ configuration: TeamTalkBanConfiguration) -> TeamTalkCommandID {
    var bannedUser = configuration.cValue
    return TeamTalkCommandID(TT_DoBan(instance, &bannedUser))
}

@discardableResult
public func banIPAddress(_ ipAddress: String, in channel: TeamTalkChannel? = nil) -> TeamTalkCommandID {
    TeamTalkCommandID(TT_DoBanIPAddress(instance, ipAddress, channel?.channelID.cValue ?? 0))
}

@discardableResult
public func unbanIPAddress(_ ipAddress: String, in channel: TeamTalkChannel? = nil) -> TeamTalkCommandID {
    TeamTalkCommandID(TT_DoUnBanUser(instance, ipAddress, channel?.channelID.cValue ?? 0))
}

@discardableResult
public func unban(_ configuration: TeamTalkBanConfiguration) -> TeamTalkCommandID {
    var bannedUser = configuration.cValue
    return TeamTalkCommandID(TT_DoUnBanUserEx(instance, &bannedUser))
}

/// Requests a page of ban entries; pass `nil` for `channel` to list
/// server-wide bans instead of one channel's. Results arrive as a series of
/// `TeamTalkEvent.Kind.bannedUser` events tagged with this command's ID.
@discardableResult
public func listBans(in channel: TeamTalkChannel? = nil, startingAt index: Int32 = 0, count: Int32 = 100) -> TeamTalkCommandID {
    TeamTalkCommandID(TT_DoListBans(instance, channel?.channelID.cValue ?? 0, index, count))
}

@discardableResult
public func moveUser(_ user: TeamTalkUser, to channel: TeamTalkChannel) -> TeamTalkCommandID {
    TeamTalkCommandID(moveUser(id: user.userID.cValue, toChannelID: channel.channelID.cValue))
}

@discardableResult
public func uploadFile(at localURL: URL, to channel: TeamTalkChannel) -> TeamTalkCommandID {
    TeamTalkCommandID(uploadFile(at: localURL, toChannelID: channel.channelID.cValue))
}

@discardableResult
public func downloadFile(_ file: TeamTalkRemoteFile, to localURL: URL) -> TeamTalkCommandID {
    TeamTalkCommandID(downloadFile(channelID: file.channelIdentifier.cValue, fileID: file.fileID.cValue, to: localURL))
}

@discardableResult
public func deleteFile(_ file: TeamTalkRemoteFile) -> TeamTalkCommandID {
    TeamTalkCommandID(deleteFile(channelID: file.channelIdentifier.cValue, fileID: file.fileID.cValue))
}

// Unlike the rest of this file, file transfers are tracked locally by the
// SDK rather than round-tripped to the server, so these resolve synchronously
// (`Bool`/an optional snapshot) instead of returning a `TeamTalkCommandID`.
@discardableResult
public func cancelFileTransfer(_ transfer: TeamTalkFileTransfer) -> Bool {
    cancelFileTransfer(id: transfer.transferID.cValue)
}

public func fileTransfer(_ transfer: TeamTalkFileTransfer) -> TeamTalkFileTransfer? {
    fileTransfer(id: transfer.transferID.cValue)
}

@discardableResult
public func subscribe(_ subscriptions: TeamTalkSubscriptions, to user: TeamTalkUser) -> TeamTalkCommandID {
    TeamTalkCommandID(subscribe(userID: user.userID.cValue, subscriptions: subscriptions.cValue))
}

@discardableResult
public func unsubscribe(_ subscriptions: TeamTalkSubscriptions, from user: TeamTalkUser) -> TeamTalkCommandID {
    TeamTalkCommandID(unsubscribe(userID: user.userID.cValue, subscriptions: subscriptions.cValue))
}

@discardableResult
public func updateServer(_ configuration: TeamTalkServerPropertiesConfiguration) -> TeamTalkCommandID {
    var properties = configuration.cValue
    return TeamTalkCommandID(TT_DoUpdateServer(instance, &properties))
}

@discardableResult
public func updateServer(_ properties: TeamTalkServerProperties) -> TeamTalkCommandID {
    var properties = properties.cValue
    return TeamTalkCommandID(TT_DoUpdateServer(instance, &properties))
}

@discardableResult
public func listUserAccounts(startingAt index: Int32 = 0, count: Int32 = 100) -> TeamTalkCommandID {
    TeamTalkCommandID(TT_DoListUserAccounts(instance, index, count))
}

@discardableResult
public func createUserAccount(_ configuration: TeamTalkUserAccountConfiguration) -> TeamTalkCommandID {
    var account = configuration.cValue
    return TeamTalkCommandID(TT_DoNewUserAccount(instance, &account))
}

@discardableResult
public func createUserAccount(_ account: TeamTalkUserAccount) -> TeamTalkCommandID {
    var account = account.cValue
    return TeamTalkCommandID(TT_DoNewUserAccount(instance, &account))
}

@discardableResult
public func deleteUserAccount(username: String) -> TeamTalkCommandID {
    TeamTalkCommandID(TT_DoDeleteUserAccount(instance, username))
}

@discardableResult
public func saveServerConfiguration() -> TeamTalkCommandID {
    TeamTalkCommandID(TT_DoSaveConfig(instance))
}

@discardableResult
public func queryServerStatistics() -> TeamTalkCommandID {
    TeamTalkCommandID(TT_DoQueryServerStats(instance))
}

@discardableResult
public func quitServer() -> TeamTalkCommandID {
    TeamTalkCommandID(TT_DoQuit(instance))
}

}
