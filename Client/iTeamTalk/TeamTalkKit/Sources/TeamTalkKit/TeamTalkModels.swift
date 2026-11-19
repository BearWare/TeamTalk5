import Foundation
import TeamTalkC

// Model files in this package generally come in two shapes:
// - `TeamTalkX`: a read-only snapshot wrapping the SDK's raw `X` struct
//   (exposed as `rawValue`/`cValue`), returned by queries and events.
// - `TeamTalkXConfiguration`: a plain mutable struct used to build a
//   command's payload (`cValue` converts it back to the raw C struct);
//   pass one to the matching `create`/`update` command.
// Equatable/Hashable snapshots compare/hash the raw struct byte-for-byte
// (see TeamTalkSnapshotEquatable.swift), so equality tracks every field the
// SDK reports, not just the ones this wrapper exposes accessors for.

/// A live user session: who's logged in, in which channel, with what
/// status. Contrast with ``TeamTalkUserAccount``, which is the persistent
/// account data (rights, password, ...) behind a login.
public struct TeamTalkUser: Identifiable, Equatable, Hashable, Sendable {
    public let rawValue: User

    public init(_ rawValue: User) {
        self.rawValue = rawValue
    }

    public var cValue: User {
        rawValue
    }

    public static func == (lhs: TeamTalkUser, rhs: TeamTalkUser) -> Bool {
        rawStructsEqual(lhs.rawValue, rhs.rawValue)
    }

    public func hash(into hasher: inout Hasher) {
        hashRawStruct(rawValue, into: &hasher)
    }

    public var id: Int32 {
        rawValue.id
    }

    public var userID: TeamTalkUserID {
        TeamTalkUserID(id)
    }

    public var channelID: Int32 {
        rawValue.channelID
    }

    public var channelIdentifier: TeamTalkChannelID {
        TeamTalkChannelID(channelID)
    }

    public var username: String {
        rawValue.username
    }

    public var nickname: String {
        rawValue.nickname
    }

    public var statusMessage: String {
        rawValue.statusMessage
    }

    public var ipAddress: String {
        rawValue.ipAddress
    }

    public var clientName: String {
        rawValue.clientName
    }

    public var types: TeamTalkUserTypes {
        rawValue.types
    }

    /// What this client receives from `user` (set via
    /// `TeamTalkSession.subscribe(_:to:)`/`unsubscribe(_:from:)`).
    public var localSubscriptions: TeamTalkSubscriptions {
        rawValue.localSubscriptions
    }

    /// What `user` receives from this client.
    public var peerSubscriptions: TeamTalkSubscriptions {
        rawValue.peerSubscriptions
    }

    public var statusMode: TeamTalkStatusMode {
        rawValue.statusMode
    }

    public var states: TeamTalkUserStates {
        rawValue.states
    }

    public var isAdministrator: Bool {
        rawValue.isAdministrator
    }

    /// Whether `channelID`/`channelIdentifier` currently point at a real
    /// channel, e.g. `false` right after logging in before joining anywhere.
    public var isInChannel: Bool {
        rawValue.isInChannel
    }

    /// This client's locally set playback volume for `user`'s voice - see
    /// `TeamTalkSession.setUserVolume(_:stream:volume:)`.
    public var voiceVolume: Int32 {
        rawValue.nVolumeVoice
    }

    /// Like ``voiceVolume``, for `user`'s media file playback.
    public var mediaFileVolume: Int32 {
        rawValue.nVolumeMediaFile
    }

    public var voiceStereoLeftSpeaker: Bool {
        rawValue.stereoPlaybackVoice.0 != 0
    }

    public var voiceStereoRightSpeaker: Bool {
        rawValue.stereoPlaybackVoice.1 != 0
    }

    public var mediaFileStereoLeftSpeaker: Bool {
        rawValue.stereoPlaybackMediaFile.0 != 0
    }

    public var mediaFileStereoRightSpeaker: Bool {
        rawValue.stereoPlaybackMediaFile.1 != 0
    }

    /// A placeholder to use before a real user snapshot is available.
    public static let empty = TeamTalkUser(User())
}

/// Server-side rate limiting for one user account: how many commands are
/// allowed per interval before the server starts rejecting them.
public struct TeamTalkAbusePrevention {
    public let rawValue: AbusePrevention

    public init(_ rawValue: AbusePrevention) {
        self.rawValue = rawValue
    }

    public var cValue: AbusePrevention {
        rawValue
    }

    public var commandLimit: Int32 {
        rawValue.commandLimit
    }

    public var commandIntervalMilliseconds: Int32 {
        rawValue.commandIntervalMilliseconds
    }

    public var isDisabled: Bool {
        rawValue.isDisabled
    }
}

/// Mutable counterpart to ``TeamTalkAbusePrevention`` for building a
/// `TeamTalkUserAccountConfiguration`. A limit of `0` for either field
/// disables rate limiting (see ``isDisabled``).
public struct TeamTalkAbusePreventionConfiguration {
    public var commandLimit: Int32
    public var commandIntervalMilliseconds: Int32

    public init(
        commandLimit: Int32 = 0,
        commandIntervalMilliseconds: Int32 = 0
    ) {
        self.commandLimit = commandLimit
        self.commandIntervalMilliseconds = commandIntervalMilliseconds
    }

    public init(_ abusePrevention: TeamTalkAbusePrevention) {
        self.init(
            commandLimit: abusePrevention.commandLimit,
            commandIntervalMilliseconds: abusePrevention.commandIntervalMilliseconds
        )
    }

    public var isDisabled: Bool {
        commandLimit <= 0 || commandIntervalMilliseconds <= 0
    }

    public var cValue: AbusePrevention {
        var abusePrevention = AbusePrevention()
        abusePrevention.commandLimit = commandLimit
        abusePrevention.commandIntervalMilliseconds = commandIntervalMilliseconds
        return abusePrevention
    }
}

/// A server account's persistent data: credentials, granted rights, and
/// defaults applied on login. Contrast with ``TeamTalkUser``, the live
/// session data visible while that account is logged in.
public struct TeamTalkUserAccount: Equatable, Hashable, Sendable {
    public let rawValue: UserAccount

    public init(_ rawValue: UserAccount) {
        self.rawValue = rawValue
    }

    public var cValue: UserAccount {
        rawValue
    }

    public static func == (lhs: TeamTalkUserAccount, rhs: TeamTalkUserAccount) -> Bool {
        rawStructsEqual(lhs.rawValue, rhs.rawValue)
    }

    public func hash(into hasher: inout Hasher) {
        hashRawStruct(rawValue, into: &hasher)
    }

    public var username: String {
        rawValue.username
    }

    /// Slash-separated channel path this account is placed into
    /// automatically on login (see `TeamTalkSession.channel(path:)`), or
    /// empty for the server's default.
    public var initialChannel: String {
        rawValue.initialChannel
    }

    public var note: String {
        rawValue.note
    }

    public var password: String {
        rawValue.password
    }

    public var lastModified: String {
        rawValue.lastModified
    }

    public var lastLoginTime: String {
        rawValue.lastLoginTime
    }

    public var types: TeamTalkUserTypes {
        rawValue.types
    }

    public var rights: TeamTalkUserRights {
        rawValue.rights
    }

    public var userData: Int32 {
        rawValue.nUserData
    }

    public var audioCodecBitrateLimit: Int32 {
        rawValue.nAudioCodecBpsLimit
    }

    /// Channels this account is automatically made an operator of on
    /// joining.
    public var autoOperatorChannelIDs: [Int32] {
        rawValue.autoOperatorChannelIDs
    }

    public var autoOperatorChannelIdentifiers: [TeamTalkChannelID] {
        rawValue.autoOperatorChannelIdentifiers
    }

    public var abusePrevention: TeamTalkAbusePrevention {
        TeamTalkAbusePrevention(rawValue.abusePrevention)
    }

    public var commandLimit: Int32 {
        abusePrevention.commandLimit
    }

    public var commandIntervalMilliseconds: Int32 {
        abusePrevention.commandIntervalMilliseconds
    }

    public var isAdministrator: Bool {
        rawValue.isAdministrator
    }

    /// A placeholder to use before a real user account snapshot is
    /// available.
    public static let empty = TeamTalkUserAccount(UserAccount())
}

/// Mutable counterpart to ``TeamTalkUserAccount`` for
/// `TeamTalkSession.createUserAccount(_:)`/updates.
public struct TeamTalkUserAccountConfiguration {
    public var username: String
    public var password: String
    public var types: TeamTalkUserTypes
    public var rights: TeamTalkUserRights
    public var userData: Int32
    public var note: String
    public var initialChannel: String
    public var autoOperatorChannelIDs: [Int32]
    public var audioCodecBitrateLimit: Int32
    public var abusePrevention: TeamTalkAbusePreventionConfiguration

    public init(
        username: String,
        password: String = "",
        types: TeamTalkUserTypes = .defaultUser,
        rights: TeamTalkUserRights = .none,
        userData: Int32 = 0,
        note: String = "",
        initialChannel: String = "",
        autoOperatorChannelIDs: [Int32] = [],
        audioCodecBitrateLimit: Int32 = 0,
        commandLimit: Int32 = 0,
        commandIntervalMilliseconds: Int32 = 0,
        abusePrevention: TeamTalkAbusePreventionConfiguration? = nil
    ) {
        self.username = username
        self.password = password
        self.types = types
        self.rights = rights
        self.userData = userData
        self.note = note
        self.initialChannel = initialChannel
        self.autoOperatorChannelIDs = autoOperatorChannelIDs
        self.audioCodecBitrateLimit = audioCodecBitrateLimit
        self.abusePrevention = abusePrevention ?? TeamTalkAbusePreventionConfiguration(
            commandLimit: commandLimit,
            commandIntervalMilliseconds: commandIntervalMilliseconds
        )
    }

    public init(_ account: TeamTalkUserAccount, password: String = "") {
        self.init(
            username: account.username,
            password: password,
            types: account.types,
            rights: account.rights,
            userData: account.userData,
            note: account.note,
            initialChannel: account.initialChannel,
            autoOperatorChannelIDs: account.autoOperatorChannelIDs,
            audioCodecBitrateLimit: account.audioCodecBitrateLimit,
            abusePrevention: TeamTalkAbusePreventionConfiguration(account.abusePrevention)
        )
    }

    public var commandLimit: Int32 {
        get { abusePrevention.commandLimit }
        set { abusePrevention.commandLimit = newValue }
    }

    public var autoOperatorChannelIdentifiers: [TeamTalkChannelID] {
        get { autoOperatorChannelIDs.map { TeamTalkChannelID($0) } }
        set { autoOperatorChannelIDs = newValue.map(\.cValue) }
    }

    public var commandIntervalMilliseconds: Int32 {
        get { abusePrevention.commandIntervalMilliseconds }
        set { abusePrevention.commandIntervalMilliseconds = newValue }
    }

    public var cValue: UserAccount {
        var account = UserAccount()
        TeamTalkString.setUserAccount(.username, on: &account, to: username)
        TeamTalkString.setUserAccount(.password, on: &account, to: password)
        TeamTalkString.setUserAccount(.note, on: &account, to: note)
        TeamTalkString.setUserAccount(.initialChannel, on: &account, to: initialChannel)
        account.types = types
        account.rights = rights
        account.nUserData = userData
        account.nAudioCodecBpsLimit = audioCodecBitrateLimit
        account.abusePrevention = abusePrevention.cValue
        autoOperatorChannelIDs.withUnsafeBufferPointer { buffer in
            TTKitSetUserAccountAutoOperatorChannels(&account, buffer.baseAddress, Int32(buffer.count))
        }
        return account
    }
}
