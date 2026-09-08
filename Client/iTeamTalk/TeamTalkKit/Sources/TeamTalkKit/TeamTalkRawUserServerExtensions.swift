import Foundation
import TeamTalkC

// Each extension below bridges one raw C struct's fixed-size buffers and
// int/bit fields to Swift-friendly accessors. The corresponding `TeamTalkX`
// wrapper type (documented in the model files) wraps this extension rather
// than duplicating it, so see those for what each field means.
public extension User {
    var id: Int32 {
        nUserID
    }

    var userID: TeamTalkUserID {
        TeamTalkUserID(id)
    }

    var channelID: Int32 {
        nChannelID
    }

    var channelIdentifier: TeamTalkChannelID {
        TeamTalkChannelID(channelID)
    }

    var username: String {
        TeamTalkString.user(.username, from: self)
    }

    var nickname: String {
        TeamTalkString.user(.nickname, from: self)
    }

    var statusMessage: String {
        TeamTalkString.user(.statusMessage, from: self)
    }

    var ipAddress: String {
        TeamTalkString.user(.ipAddress, from: self)
    }

    var clientName: String {
        TeamTalkString.user(.clientName, from: self)
    }

    var types: TeamTalkUserTypes {
        get { TeamTalkUserTypes(cValue: uUserType) }
        set { uUserType = newValue.cValue }
    }

    var localSubscriptions: TeamTalkSubscriptions {
        get { TeamTalkSubscriptions(cValue: uLocalSubscriptions) }
        set { uLocalSubscriptions = newValue.cValue }
    }

    var peerSubscriptions: TeamTalkSubscriptions {
        get { TeamTalkSubscriptions(cValue: uPeerSubscriptions) }
        set { uPeerSubscriptions = newValue.cValue }
    }

    var statusMode: TeamTalkStatusMode {
        get { TeamTalkStatusMode(rawValue: nStatusMode) }
        set { nStatusMode = newValue.rawValue }
    }

    var states: TeamTalkUserStates {
        get { TeamTalkUserStates(cValue: uUserState) }
        set { uUserState = newValue.cValue }
    }

    var isAdministrator: Bool {
        types.contains(.administrator)
    }

    var isInChannel: Bool {
        nChannelID > 0
    }
}

public extension AbusePrevention {
    var commandLimit: Int32 {
        get { nCommandsLimit }
        set { nCommandsLimit = newValue }
    }

    var commandIntervalMilliseconds: Int32 {
        get { nCommandsIntervalMSec }
        set { nCommandsIntervalMSec = newValue }
    }

    var isDisabled: Bool {
        commandLimit <= 0 || commandIntervalMilliseconds <= 0
    }
}

public extension UserAccount {
    var username: String {
        TeamTalkString.userAccount(.username, from: self)
    }

    var password: String {
        TeamTalkString.userAccount(.password, from: self)
    }

    var initialChannel: String {
        TeamTalkString.userAccount(.initialChannel, from: self)
    }

    var note: String {
        TeamTalkString.userAccount(.note, from: self)
    }

    var lastModified: String {
        TeamTalkString.userAccount(.lastModified, from: self)
    }

    var lastLoginTime: String {
        TeamTalkString.userAccount(.lastLoginTime, from: self)
    }

    var types: TeamTalkUserTypes {
        get { TeamTalkUserTypes(cValue: uUserType) }
        set { uUserType = newValue.cValue }
    }

    var rights: TeamTalkUserRights {
        get { TeamTalkUserRights(cValue: uUserRights) }
        set { uUserRights = newValue.cValue }
    }

    var isAdministrator: Bool {
        types.contains(.administrator)
    }

    var autoOperatorChannelIDs: [Int32] {
        (0..<Int(TT_CHANNELS_OPERATOR_MAX)).compactMap { index in
            var account = self
            let channelID = TTKitGetUserAccountAutoOperatorChannel(&account, Int32(index))
            return channelID > 0 ? channelID : nil
        }
    }

    var autoOperatorChannelIdentifiers: [TeamTalkChannelID] {
        autoOperatorChannelIDs.map { TeamTalkChannelID($0) }
    }

    var abusePrevention: AbusePrevention {
        get { abusePrevent }
        set { abusePrevent = newValue }
    }

    var commandLimit: Int32 {
        get { abusePrevention.commandLimit }
        set { abusePrevention.commandLimit = newValue }
    }

    var commandIntervalMilliseconds: Int32 {
        get { abusePrevention.commandIntervalMilliseconds }
        set { abusePrevention.commandIntervalMilliseconds = newValue }
    }
}

public extension BannedUser {
    var ipAddress: String {
        TeamTalkString.bannedUser(.ipAddress, from: self)
    }

    var channelPath: String {
        TeamTalkString.bannedUser(.channelPath, from: self)
    }

    var banTime: String {
        TeamTalkString.bannedUser(.banTime, from: self)
    }

    var nickname: String {
        TeamTalkString.bannedUser(.nickname, from: self)
    }

    var username: String {
        TeamTalkString.bannedUser(.username, from: self)
    }

    var owner: String {
        TeamTalkString.bannedUser(.owner, from: self)
    }

    var channelPathValue: TeamTalkChannelPath? {
        channelPath.isEmpty ? nil : TeamTalkChannelPath(rawValue: channelPath)
    }

    var types: TeamTalkBanTypes {
        get { TeamTalkBanTypes(cValue: uBanTypes) }
        set { uBanTypes = newValue.cValue }
    }

    var isChannelBan: Bool {
        hasBanType(.channel)
    }

    var isLoginBan: Bool {
        !isChannelBan
    }

    var isIPAddressBan: Bool {
        hasBanType(.ipAddress)
    }

    var isUsernameBan: Bool {
        hasBanType(.username)
    }
}

public extension ServerProperties {
    var name: String {
        TeamTalkString.serverProperties(.name, from: self)
    }

    var messageOfTheDay: String {
        TeamTalkString.serverProperties(.messageOfTheDay, from: self)
    }

    var rawMessageOfTheDay: String {
        TeamTalkString.serverProperties(.rawMessageOfTheDay, from: self)
    }

    var version: String {
        TeamTalkString.serverProperties(.version, from: self)
    }

    var protocolVersion: String {
        TeamTalkString.serverProperties(.protocolVersion, from: self)
    }

    var accessToken: String {
        TeamTalkString.serverProperties(.accessToken, from: self)
    }

    var logEvents: TeamTalkServerLogEvents {
        get { TeamTalkServerLogEvents(cValue: uServerLogEvents) }
        set { uServerLogEvents = newValue.cValue }
    }

    var autoSave: Bool {
        get { bAutoSave != 0 }
        set { bAutoSave = newValue ? 1 : 0 }
    }
}

public extension UserStatistics {
    var voicePacketsReceived: Int64 {
        nVoicePacketsRecv
    }

    var voicePacketsLost: Int64 {
        nVoicePacketsLost
    }

    var videoCapturePacketsReceived: Int64 {
        nVideoCapturePacketsRecv
    }

    var videoCaptureFramesReceived: Int64 {
        nVideoCaptureFramesRecv
    }

    var videoCaptureFramesLost: Int64 {
        nVideoCaptureFramesLost
    }

    var videoCaptureFramesDropped: Int64 {
        nVideoCaptureFramesDropped
    }

    var mediaFileAudioPacketsReceived: Int64 {
        nMediaFileAudioPacketsRecv
    }

    var mediaFileAudioPacketsLost: Int64 {
        nMediaFileAudioPacketsLost
    }

    var mediaFileVideoPacketsReceived: Int64 {
        nMediaFileVideoPacketsRecv
    }

    var mediaFileVideoFramesReceived: Int64 {
        nMediaFileVideoFramesRecv
    }

    var mediaFileVideoFramesLost: Int64 {
        nMediaFileVideoFramesLost
    }

    var mediaFileVideoFramesDropped: Int64 {
        nMediaFileVideoFramesDropped
    }
}

public extension ClientStatistics {
    var udpBytesSent: Int64 {
        nUdpBytesSent
    }

    var udpBytesReceived: Int64 {
        nUdpBytesRecv
    }

    var voiceBytesSent: Int64 {
        nVoiceBytesSent
    }

    var voiceBytesReceived: Int64 {
        nVoiceBytesRecv
    }

    var videoCaptureBytesSent: Int64 {
        nVideoCaptureBytesSent
    }

    var videoCaptureBytesReceived: Int64 {
        nVideoCaptureBytesRecv
    }

    var mediaFileAudioBytesSent: Int64 {
        nMediaFileAudioBytesSent
    }

    var mediaFileAudioBytesReceived: Int64 {
        nMediaFileAudioBytesRecv
    }

    var mediaFileVideoBytesSent: Int64 {
        nMediaFileVideoBytesSent
    }

    var mediaFileVideoBytesReceived: Int64 {
        nMediaFileVideoBytesRecv
    }

    var desktopBytesSent: Int64 {
        nDesktopBytesSent
    }

    var desktopBytesReceived: Int64 {
        nDesktopBytesRecv
    }

    var udpPingTimeMilliseconds: Int32? {
        nUdpPingTimeMs >= 0 ? nUdpPingTimeMs : nil
    }

    var tcpPingTimeMilliseconds: Int32? {
        nTcpPingTimeMs >= 0 ? nTcpPingTimeMs : nil
    }

    var tcpServerSilenceSeconds: Int32 {
        nTcpServerSilenceSec
    }

    var udpServerSilenceSeconds: Int32 {
        nUdpServerSilenceSec
    }

    var soundInputDeviceDelayMilliseconds: Int32 {
        nSoundInputDeviceDelayMSec
    }
}

public extension ClientKeepAlive {
    var connectionLostMilliseconds: Int32 {
        get { nConnectionLostMSec }
        set { nConnectionLostMSec = newValue }
    }

    var tcpKeepAliveIntervalMilliseconds: Int32 {
        get { nTcpKeepAliveIntervalMSec }
        set { nTcpKeepAliveIntervalMSec = newValue }
    }

    var udpKeepAliveIntervalMilliseconds: Int32 {
        get { nUdpKeepAliveIntervalMSec }
        set { nUdpKeepAliveIntervalMSec = newValue }
    }

    var udpKeepAliveRetransmitMilliseconds: Int32 {
        get { nUdpKeepAliveRTXMSec }
        set { nUdpKeepAliveRTXMSec = newValue }
    }

    var udpConnectRetransmitMilliseconds: Int32 {
        get { nUdpConnectRTXMSec }
        set { nUdpConnectRTXMSec = newValue }
    }

    var udpConnectTimeoutMilliseconds: Int32 {
        get { nUdpConnectTimeoutMSec }
        set { nUdpConnectTimeoutMSec = newValue }
    }
}
