import Foundation
import TeamTalkC

/// A file uploaded to a channel's server-side file storage.
public struct TeamTalkRemoteFile: Identifiable, Equatable, Hashable, Sendable {
    public let rawValue: RemoteFile

    public init(_ rawValue: RemoteFile) {
        self.rawValue = rawValue
    }

    public var cValue: RemoteFile {
        rawValue
    }

    public static func == (lhs: TeamTalkRemoteFile, rhs: TeamTalkRemoteFile) -> Bool {
        rawStructsEqual(lhs.rawValue, rhs.rawValue)
    }

    public func hash(into hasher: inout Hasher) {
        hashRawStruct(rawValue, into: &hasher)
    }

    public var id: Int32 {
        rawValue.id
    }

    public var fileID: TeamTalkFileID {
        TeamTalkFileID(id)
    }

    public var channelID: Int32 {
        rawValue.channelID
    }

    public var channelIdentifier: TeamTalkChannelID {
        TeamTalkChannelID(channelID)
    }

    public var name: String {
        rawValue.name
    }

    public var username: String {
        rawValue.username
    }

    public var uploadTime: String {
        rawValue.uploadTime
    }

    public var size: Int64 {
        rawValue.size
    }
}

/// One ban entry, as returned by `TeamTalkSession.listBans(in:startingAt:count:)`.
/// Has no single server-assigned ID; ``id`` is synthesized from its fields
/// for `Identifiable` conformance.
public struct TeamTalkBannedUser: Identifiable, Equatable, Hashable, Sendable {
    public let rawValue: BannedUser

    public init(_ rawValue: BannedUser) {
        self.rawValue = rawValue
    }

    public var cValue: BannedUser {
        rawValue
    }

    public static func == (lhs: TeamTalkBannedUser, rhs: TeamTalkBannedUser) -> Bool {
        rawStructsEqual(lhs.rawValue, rhs.rawValue)
    }

    public func hash(into hasher: inout Hasher) {
        hashRawStruct(rawValue, into: &hasher)
    }

    public var id: String {
        "\(ipAddress)|\(username)|\(channelPath)|\(types.rawValue)"
    }

    public var ipAddress: String {
        rawValue.ipAddress
    }

    /// Empty for a ban that isn't scoped to one channel (see ``isLoginBan``).
    public var channelPath: String {
        rawValue.channelPath
    }

    public var banTime: String {
        rawValue.banTime
    }

    public var nickname: String {
        rawValue.nickname
    }

    public var username: String {
        rawValue.username
    }

    /// Username of the operator who issued this ban.
    public var owner: String {
        rawValue.owner
    }

    public var channelPathValue: TeamTalkChannelPath? {
        rawValue.channelPathValue
    }

    public var types: TeamTalkBanTypes {
        rawValue.types
    }

    public var isChannelBan: Bool {
        rawValue.isChannelBan
    }

    /// A ban that isn't scoped to one channel, i.e. blocks logging into the
    /// server at all rather than just joining a specific channel.
    public var isLoginBan: Bool {
        rawValue.isLoginBan
    }

    public var isIPAddressBan: Bool {
        rawValue.isIPAddressBan
    }

    public var isUsernameBan: Bool {
        rawValue.isUsernameBan
    }
}

/// Describes a ban to create via `TeamTalkSession.ban(_:)`/remove via
/// `unban(_:)`, independent of any currently-online ``TeamTalkUser``.
public struct TeamTalkBanConfiguration {
    public var ipAddress: String
    public var channelPath: String
    public var username: String
    public var types: TeamTalkBanTypes

    public init(
        ipAddress: String = "",
        channelPath: String = "",
        username: String = "",
        types: TeamTalkBanTypes
    ) {
        self.ipAddress = ipAddress
        self.channelPath = channelPath
        self.username = username
        self.types = types
    }

    public init(
        ipAddress: String = "",
        channelPathValue: TeamTalkChannelPath?,
        username: String = "",
        types: TeamTalkBanTypes
    ) {
        self.init(
            ipAddress: ipAddress,
            channelPath: channelPathValue?.rawValue ?? "",
            username: username,
            types: types
        )
    }

    public init(_ bannedUser: TeamTalkBannedUser) {
        self.init(
            ipAddress: bannedUser.ipAddress,
            channelPath: bannedUser.channelPath,
            username: bannedUser.username,
            types: bannedUser.types
        )
    }

    public var channelPathValue: TeamTalkChannelPath? {
        get { channelPath.isEmpty ? nil : TeamTalkChannelPath(rawValue: channelPath) }
        set { channelPath = newValue?.rawValue ?? "" }
    }

    public var isChannelBan: Bool {
        types.contains(.channel)
    }

    public var isLoginBan: Bool {
        !isChannelBan
    }

    public var isIPAddressBan: Bool {
        types.contains(.ipAddress)
    }

    public var isUsernameBan: Bool {
        types.contains(.username)
    }

    public var cValue: BannedUser {
        var bannedUser = BannedUser()
        TeamTalkString.setBannedUser(.ipAddress, on: &bannedUser, to: ipAddress)
        TeamTalkString.setBannedUser(.channelPath, on: &bannedUser, to: channelPath)
        TeamTalkString.setBannedUser(.username, on: &bannedUser, to: username)
        bannedUser.types = types
        return bannedUser
    }
}
