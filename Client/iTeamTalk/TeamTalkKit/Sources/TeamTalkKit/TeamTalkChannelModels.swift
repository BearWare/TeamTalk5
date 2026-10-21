import Foundation
import TeamTalkC

/// A channel's configuration as currently known to this client. See
/// TeamTalkModels.swift for the snapshot/configuration naming convention
/// this type and ``TeamTalkChannelConfiguration`` follow.
public struct TeamTalkChannel: Identifiable, Equatable, Hashable, Sendable {
    public let rawValue: Channel

    public init(_ rawValue: Channel) {
        self.rawValue = rawValue
    }

    /// A new, unsaved channel under `parentChannelID`, for a "create
    /// channel" UI to populate before submitting.
    public init(parentChannelID: TeamTalkChannelID) {
        var rawValue = Channel()
        rawValue.nParentID = parentChannelID.cValue
        self.rawValue = rawValue
    }

    public var cValue: Channel {
        rawValue
    }

    public static func == (lhs: TeamTalkChannel, rhs: TeamTalkChannel) -> Bool {
        rawStructsEqual(lhs.rawValue, rhs.rawValue)
    }

    public func hash(into hasher: inout Hasher) {
        hashRawStruct(rawValue, into: &hasher)
    }

    public var id: Int32 {
        rawValue.id
    }

    public var channelID: TeamTalkChannelID {
        TeamTalkChannelID(id)
    }

    public var parentID: Int32 {
        rawValue.parentID
    }

    public var parentChannelID: TeamTalkChannelID {
        TeamTalkChannelID(parentID)
    }

    public var name: String {
        rawValue.name
    }

    public var topic: String {
        rawValue.topic
    }

    public var password: String {
        rawValue.password
    }

    /// Lets a user claim operator status in this channel by supplying this
    /// password, via `TeamTalkSession.setChannelOperator(_:in:operatorPassword:enabled:)`,
    /// without already holding the right.
    public var operatorPassword: String {
        rawValue.operatorPassword
    }

    public var types: TeamTalkChannelTypes {
        rawValue.types
    }

    public var userData: Int32 {
        rawValue.nUserData
    }

    public var diskQuota: Int64 {
        rawValue.nDiskQuota
    }

    public var maxUsers: Int32 {
        rawValue.nMaxUsers
    }

    public var audioCodecType: TeamTalkCodec {
        rawValue.audioCodecType
    }

    /// Users currently allowed to transmit at once in this channel — how
    /// many depends on the channel's transmit mode (e.g. "No Interruptions"
    /// channels typically allow just one).
    public var transmitUsers: [TeamTalkChannelTransmitUser] {
        rawValue.transmitUserList
    }

    /// Users waiting their turn to transmit, in order; the first entry is
    /// next up once a current transmitter yields. Empty in channels that
    /// don't queue (e.g. normal free-for-all channels).
    public var transmitUsersQueue: [TeamTalkUserID] {
        rawValue.transmitQueueUsers
    }

    /// How long the channel waits after a transmitter stops before handing
    /// off to the next queued user.
    public var transmitUsersQueueDelayMilliseconds: Int32 {
        rawValue.transmitQueueDelayMilliseconds
    }

    /// How long a user may hold voice transmission in this channel before
    /// being cut off; `0` means unlimited.
    public var voiceTimeoutMilliseconds: Int32 {
        rawValue.voiceTimeoutMilliseconds
    }

    /// Like ``voiceTimeoutMilliseconds``, for media file playback streamed
    /// into the channel.
    public var mediaFileTimeoutMilliseconds: Int32 {
        rawValue.mediaFileTimeoutMilliseconds
    }

    public var isRoot: Bool {
        rawValue.isRoot
    }

    public var isPasswordProtected: Bool {
        rawValue.isPasswordProtected
    }

    /// Whether this channel automatically adjusts transmitting users'
    /// volume to a target level, instead of using each user's raw input
    /// gain. See ``gainLevel`` for the target.
    public var isAGCEnabled: Bool {
        rawValue.audiocfg.bEnableAGC != 0
    }

    /// The target volume ``isAGCEnabled`` normalizes transmitting users to.
    public var gainLevel: Int32 {
        rawValue.audiocfg.nGainLevel
    }

    /// A copy of this channel with `password` set, e.g. to carry a
    /// remembered join password into a "channel detail" UI that otherwise
    /// only sees the server's (unpopulated) `password` field.
    public func settingPassword(_ password: String) -> TeamTalkChannel {
        var rawValue = rawValue
        TeamTalkString.setChannel(.password, on: &rawValue, to: password)
        return TeamTalkChannel(rawValue)
    }

    /// A placeholder to use before a real channel snapshot is available.
    public static let empty = TeamTalkChannel(Channel())
}

/// Mutable counterpart to ``TeamTalkChannel`` for
/// `TeamTalkSession.createChannel(_:)`/`updateChannel(_:)`/`joinChannel(_:password:)`.
public struct TeamTalkChannelConfiguration {
    public var id: Int32
    public var parentID: Int32
    public var name: String
    public var topic: String
    public var password: String
    public var operatorPassword: String
    public var types: TeamTalkChannelTypes
    public var userData: Int32
    public var diskQuota: Int64
    public var maxUsers: Int32
    public var audioCodec: TeamTalkAudioCodecConfiguration
    public var transmitUsers: [TeamTalkChannelTransmitUser]
    public var transmitUsersQueue: [TeamTalkUserID]
    public var transmitUsersQueueDelayMilliseconds: Int32
    public var voiceTimeoutMilliseconds: Int32
    public var mediaFileTimeoutMilliseconds: Int32

    public init(
        id: Int32 = 0,
        parentID: Int32,
        name: String,
        topic: String = "",
        password: String = "",
        operatorPassword: String = "",
        types: TeamTalkChannelTypes = .default,
        userData: Int32 = 0,
        diskQuota: Int64 = 0,
        maxUsers: Int32 = 0,
        audioCodec: TeamTalkAudioCodecConfiguration = .opus(TeamTalkOpusCodecConfiguration()),
        transmitUsers: [TeamTalkChannelTransmitUser] = [],
        transmitUsersQueue: [TeamTalkUserID] = [],
        transmitUsersQueueDelayMilliseconds: Int32 = 500,
        voiceTimeoutMilliseconds: Int32 = 0,
        mediaFileTimeoutMilliseconds: Int32 = 0
    ) {
        self.id = id
        self.parentID = parentID
        self.name = name
        self.topic = topic
        self.password = password
        self.operatorPassword = operatorPassword
        self.types = types
        self.userData = userData
        self.diskQuota = diskQuota
        self.maxUsers = maxUsers
        self.audioCodec = audioCodec
        self.transmitUsers = transmitUsers
        self.transmitUsersQueue = transmitUsersQueue
        self.transmitUsersQueueDelayMilliseconds = transmitUsersQueueDelayMilliseconds
        self.voiceTimeoutMilliseconds = voiceTimeoutMilliseconds
        self.mediaFileTimeoutMilliseconds = mediaFileTimeoutMilliseconds
    }

    public init(
        channelID: TeamTalkChannelID = .none,
        parentChannelID: TeamTalkChannelID,
        name: String,
        topic: String = "",
        password: String = "",
        operatorPassword: String = "",
        types: TeamTalkChannelTypes = .default,
        userData: Int32 = 0,
        diskQuota: Int64 = 0,
        maxUsers: Int32 = 0,
        audioCodec: TeamTalkAudioCodecConfiguration = .opus(TeamTalkOpusCodecConfiguration()),
        transmitUsers: [TeamTalkChannelTransmitUser] = [],
        transmitUsersQueue: [TeamTalkUserID] = [],
        transmitUsersQueueDelayMilliseconds: Int32 = 500,
        voiceTimeoutMilliseconds: Int32 = 0,
        mediaFileTimeoutMilliseconds: Int32 = 0
    ) {
        self.init(
            id: channelID.cValue,
            parentID: parentChannelID.cValue,
            name: name,
            topic: topic,
            password: password,
            operatorPassword: operatorPassword,
            types: types,
            userData: userData,
            diskQuota: diskQuota,
            maxUsers: maxUsers,
            audioCodec: audioCodec,
            transmitUsers: transmitUsers,
            transmitUsersQueue: transmitUsersQueue,
            transmitUsersQueueDelayMilliseconds: transmitUsersQueueDelayMilliseconds,
            voiceTimeoutMilliseconds: voiceTimeoutMilliseconds,
            mediaFileTimeoutMilliseconds: mediaFileTimeoutMilliseconds
        )
    }

    public init(_ channel: TeamTalkChannel) {
        self.init(
            id: channel.id,
            parentID: channel.parentID,
            name: channel.name,
            topic: channel.topic,
            password: channel.password,
            operatorPassword: channel.operatorPassword,
            types: channel.types,
            userData: channel.userData,
            diskQuota: channel.diskQuota,
            maxUsers: channel.maxUsers,
            audioCodec: TeamTalkAudioCodecConfiguration(channel.rawValue.audiocodec),
            transmitUsers: channel.transmitUsers,
            transmitUsersQueue: channel.transmitUsersQueue,
            transmitUsersQueueDelayMilliseconds: channel.transmitUsersQueueDelayMilliseconds,
            voiceTimeoutMilliseconds: channel.voiceTimeoutMilliseconds,
            mediaFileTimeoutMilliseconds: channel.mediaFileTimeoutMilliseconds
        )
    }

    public var cValue: Channel {
        var channel = Channel()
        channel.nChannelID = id
        channel.nParentID = parentID
        TeamTalkString.setChannel(.name, on: &channel, to: name)
        TeamTalkString.setChannel(.topic, on: &channel, to: topic)
        TeamTalkString.setChannel(.password, on: &channel, to: password)
        TeamTalkString.setChannel(.operatorPassword, on: &channel, to: operatorPassword)
        channel.bPassword = password.isEmpty ? 0 : 1
        channel.types = types
        channel.nUserData = userData
        channel.nDiskQuota = diskQuota
        channel.nMaxUsers = maxUsers
        channel.audiocodec = audioCodec.cValue
        channel.transmitUserList = transmitUsers
        channel.transmitQueueUsers = transmitUsersQueue
        channel.transmitQueueDelayMilliseconds = transmitUsersQueueDelayMilliseconds
        channel.voiceTimeoutMilliseconds = voiceTimeoutMilliseconds
        channel.mediaFileTimeoutMilliseconds = mediaFileTimeoutMilliseconds
        return channel
    }
}
