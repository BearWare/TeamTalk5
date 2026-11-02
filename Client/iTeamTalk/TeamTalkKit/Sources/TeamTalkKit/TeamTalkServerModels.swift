import Foundation
import TeamTalkC

/// The connected server's configuration: name, limits, ports, and logging
/// settings.
public struct TeamTalkServerProperties: Equatable, Hashable, Sendable {
    public let rawValue: ServerProperties

    public init(_ rawValue: ServerProperties) {
        self.rawValue = rawValue
    }

    public var cValue: ServerProperties {
        rawValue
    }

    public static func == (lhs: TeamTalkServerProperties, rhs: TeamTalkServerProperties) -> Bool {
        rawStructsEqual(lhs.rawValue, rhs.rawValue)
    }

    public func hash(into hasher: inout Hasher) {
        hashRawStruct(rawValue, into: &hasher)
    }

    public var name: String {
        rawValue.name
    }

    /// The message of the day with variable placeholders substituted (e.g.
    /// server name/version). See ``rawMessageOfTheDay`` for the unexpanded
    /// template, which is what `TeamTalkServerPropertiesConfiguration` edits.
    public var messageOfTheDay: String {
        rawValue.messageOfTheDay
    }

    public var rawMessageOfTheDay: String {
        rawValue.rawMessageOfTheDay
    }

    public var version: String {
        rawValue.version
    }

    public var protocolVersion: String {
        rawValue.protocolVersion
    }

    public var accessToken: String {
        rawValue.accessToken
    }

    public var maxUsers: Int32 {
        rawValue.nMaxUsers
    }

    public var maxLoginAttempts: Int32 {
        rawValue.nMaxLoginAttempts
    }

    public var maxLoginsPerIPAddress: Int32 {
        rawValue.nMaxLoginsPerIPAddress
    }

    public var maxVoiceTransmissionBytesPerSecond: Int32 {
        rawValue.nMaxVoiceTxPerSecond
    }

    public var maxVideoCaptureTransmissionBytesPerSecond: Int32 {
        rawValue.nMaxVideoCaptureTxPerSecond
    }

    public var maxMediaFileTransmissionBytesPerSecond: Int32 {
        rawValue.nMaxMediaFileTxPerSecond
    }

    public var maxDesktopTransmissionBytesPerSecond: Int32 {
        rawValue.nMaxDesktopTxPerSecond
    }

    public var maxTotalTransmissionBytesPerSecond: Int32 {
        rawValue.nMaxTotalTxPerSecond
    }

    public var tcpPort: Int32 {
        rawValue.nTcpPort
    }

    public var udpPort: Int32 {
        rawValue.nUdpPort
    }

    public var userTimeout: Int32 {
        rawValue.nUserTimeout
    }

    public var loginDelayMilliseconds: Int32 {
        rawValue.nLoginDelayMSec
    }

    public var logEvents: TeamTalkServerLogEvents {
        rawValue.logEvents
    }

    public var autoSave: Bool {
        rawValue.autoSave
    }

    /// A placeholder to use before a real server properties snapshot is
    /// available.
    public static let empty = TeamTalkServerProperties(ServerProperties())
}

/// Mutable counterpart to ``TeamTalkServerProperties`` for
/// `TeamTalkSession.updateServer(_:)`. Requires the "update server
/// properties" right.
public struct TeamTalkServerPropertiesConfiguration {
    public var name: String
    public var rawMessageOfTheDay: String
    public var maxUsers: Int32
    public var maxLoginAttempts: Int32
    public var maxLoginsPerIPAddress: Int32
    public var maxVoiceTransmissionBytesPerSecond: Int32
    public var maxVideoCaptureTransmissionBytesPerSecond: Int32
    public var maxMediaFileTransmissionBytesPerSecond: Int32
    public var maxDesktopTransmissionBytesPerSecond: Int32
    public var maxTotalTransmissionBytesPerSecond: Int32
    public var tcpPort: Int32
    public var udpPort: Int32
    public var userTimeout: Int32
    public var loginDelayMilliseconds: Int32
    public var logEvents: TeamTalkServerLogEvents
    public var autoSave: Bool

    public init(
        name: String = "",
        rawMessageOfTheDay: String = "",
        maxUsers: Int32 = 0,
        maxLoginAttempts: Int32 = 0,
        maxLoginsPerIPAddress: Int32 = 0,
        maxVoiceTransmissionBytesPerSecond: Int32 = 0,
        maxVideoCaptureTransmissionBytesPerSecond: Int32 = 0,
        maxMediaFileTransmissionBytesPerSecond: Int32 = 0,
        maxDesktopTransmissionBytesPerSecond: Int32 = 0,
        maxTotalTransmissionBytesPerSecond: Int32 = 0,
        tcpPort: Int32 = 0,
        udpPort: Int32 = 0,
        userTimeout: Int32 = 0,
        loginDelayMilliseconds: Int32 = 0,
        logEvents: TeamTalkServerLogEvents = .none,
        autoSave: Bool = false
    ) {
        self.name = name
        self.rawMessageOfTheDay = rawMessageOfTheDay
        self.maxUsers = maxUsers
        self.maxLoginAttempts = maxLoginAttempts
        self.maxLoginsPerIPAddress = maxLoginsPerIPAddress
        self.maxVoiceTransmissionBytesPerSecond = maxVoiceTransmissionBytesPerSecond
        self.maxVideoCaptureTransmissionBytesPerSecond = maxVideoCaptureTransmissionBytesPerSecond
        self.maxMediaFileTransmissionBytesPerSecond = maxMediaFileTransmissionBytesPerSecond
        self.maxDesktopTransmissionBytesPerSecond = maxDesktopTransmissionBytesPerSecond
        self.maxTotalTransmissionBytesPerSecond = maxTotalTransmissionBytesPerSecond
        self.tcpPort = tcpPort
        self.udpPort = udpPort
        self.userTimeout = userTimeout
        self.loginDelayMilliseconds = loginDelayMilliseconds
        self.logEvents = logEvents
        self.autoSave = autoSave
    }

    public init(_ properties: TeamTalkServerProperties) {
        self.init(
            name: properties.name,
            rawMessageOfTheDay: properties.rawMessageOfTheDay,
            maxUsers: properties.maxUsers,
            maxLoginAttempts: properties.maxLoginAttempts,
            maxLoginsPerIPAddress: properties.maxLoginsPerIPAddress,
            maxVoiceTransmissionBytesPerSecond: properties.maxVoiceTransmissionBytesPerSecond,
            maxVideoCaptureTransmissionBytesPerSecond: properties.maxVideoCaptureTransmissionBytesPerSecond,
            maxMediaFileTransmissionBytesPerSecond: properties.maxMediaFileTransmissionBytesPerSecond,
            maxDesktopTransmissionBytesPerSecond: properties.maxDesktopTransmissionBytesPerSecond,
            maxTotalTransmissionBytesPerSecond: properties.maxTotalTransmissionBytesPerSecond,
            tcpPort: properties.tcpPort,
            udpPort: properties.udpPort,
            userTimeout: properties.userTimeout,
            loginDelayMilliseconds: properties.loginDelayMilliseconds,
            logEvents: properties.logEvents,
            autoSave: properties.autoSave
        )
    }

    public var cValue: ServerProperties {
        var properties = ServerProperties()
        TeamTalkString.setServerProperties(.name, on: &properties, to: name)
        TeamTalkString.setServerProperties(.rawMessageOfTheDay, on: &properties, to: rawMessageOfTheDay)
        properties.nMaxUsers = maxUsers
        properties.nMaxLoginAttempts = maxLoginAttempts
        properties.nMaxLoginsPerIPAddress = maxLoginsPerIPAddress
        properties.nMaxVoiceTxPerSecond = maxVoiceTransmissionBytesPerSecond
        properties.nMaxVideoCaptureTxPerSecond = maxVideoCaptureTransmissionBytesPerSecond
        properties.nMaxMediaFileTxPerSecond = maxMediaFileTransmissionBytesPerSecond
        properties.nMaxDesktopTxPerSecond = maxDesktopTransmissionBytesPerSecond
        properties.nMaxTotalTxPerSecond = maxTotalTransmissionBytesPerSecond
        properties.nTcpPort = tcpPort
        properties.nUdpPort = udpPort
        properties.nUserTimeout = userTimeout
        properties.nLoginDelayMSec = loginDelayMilliseconds
        properties.logEvents = logEvents
        properties.autoSave = autoSave
        return properties
    }
}

/// Server-wide aggregate statistics, from `TeamTalkSession.queryServerStatistics()`.
public struct TeamTalkServerStatistics {
    public let rawValue: ServerStatistics

    public init(_ rawValue: ServerStatistics) {
        self.rawValue = rawValue
    }

    public var cValue: ServerStatistics {
        rawValue
    }

    public var totalBytesTransmitted: Int64 { rawValue.nTotalBytesTX }
    public var totalBytesReceived: Int64 { rawValue.nTotalBytesRX }
    public var voiceBytesTransmitted: Int64 { rawValue.nVoiceBytesTX }
    public var voiceBytesReceived: Int64 { rawValue.nVoiceBytesRX }
    public var videoCaptureBytesTransmitted: Int64 { rawValue.nVideoCaptureBytesTX }
    public var videoCaptureBytesReceived: Int64 { rawValue.nVideoCaptureBytesRX }
    public var mediaFileBytesTransmitted: Int64 { rawValue.nMediaFileBytesTX }
    public var mediaFileBytesReceived: Int64 { rawValue.nMediaFileBytesRX }
    public var desktopBytesTransmitted: Int64 { rawValue.nDesktopBytesTX }
    public var desktopBytesReceived: Int64 { rawValue.nDesktopBytesRX }
    public var usersServed: Int32 { rawValue.nUsersServed }
    public var usersPeak: Int32 { rawValue.nUsersPeak }
    public var filesBytesTransmitted: Int64 { rawValue.nFilesTx }
    public var filesBytesReceived: Int64 { rawValue.nFilesRx }
    public var uptimeMilliseconds: Int64 { rawValue.nUptimeMSec }
}

/// Per-user network statistics, from `TeamTalkSession.userStatistics(_:)`.
public struct TeamTalkUserStatistics {
    public let rawValue: UserStatistics

    public init(_ rawValue: UserStatistics) {
        self.rawValue = rawValue
    }

    public var cValue: UserStatistics {
        rawValue
    }

    public var voicePacketsReceived: Int64 { rawValue.voicePacketsReceived }
    public var voicePacketsLost: Int64 { rawValue.voicePacketsLost }
    public var videoCapturePacketsReceived: Int64 { rawValue.videoCapturePacketsReceived }
    public var videoCaptureFramesReceived: Int64 { rawValue.videoCaptureFramesReceived }
    public var videoCaptureFramesLost: Int64 { rawValue.videoCaptureFramesLost }
    public var videoCaptureFramesDropped: Int64 { rawValue.videoCaptureFramesDropped }
    public var mediaFileAudioPacketsReceived: Int64 { rawValue.mediaFileAudioPacketsReceived }
    public var mediaFileAudioPacketsLost: Int64 { rawValue.mediaFileAudioPacketsLost }
    public var mediaFileVideoPacketsReceived: Int64 { rawValue.mediaFileVideoPacketsReceived }
    public var mediaFileVideoFramesReceived: Int64 { rawValue.mediaFileVideoFramesReceived }
    public var mediaFileVideoFramesLost: Int64 { rawValue.mediaFileVideoFramesLost }
    public var mediaFileVideoFramesDropped: Int64 { rawValue.mediaFileVideoFramesDropped }
}

/// This client's own connection statistics, from `TeamTalkSession.clientStatistics()`.
public struct TeamTalkClientStatistics {
    public let rawValue: ClientStatistics

    public init(_ rawValue: ClientStatistics) {
        self.rawValue = rawValue
    }

    public var cValue: ClientStatistics {
        rawValue
    }

    public var udpBytesSent: Int64 { rawValue.udpBytesSent }
    public var udpBytesReceived: Int64 { rawValue.udpBytesReceived }
    public var voiceBytesSent: Int64 { rawValue.voiceBytesSent }
    public var voiceBytesReceived: Int64 { rawValue.voiceBytesReceived }
    public var videoCaptureBytesSent: Int64 { rawValue.videoCaptureBytesSent }
    public var videoCaptureBytesReceived: Int64 { rawValue.videoCaptureBytesReceived }
    public var mediaFileAudioBytesSent: Int64 { rawValue.mediaFileAudioBytesSent }
    public var mediaFileAudioBytesReceived: Int64 { rawValue.mediaFileAudioBytesReceived }
    public var mediaFileVideoBytesSent: Int64 { rawValue.mediaFileVideoBytesSent }
    public var mediaFileVideoBytesReceived: Int64 { rawValue.mediaFileVideoBytesReceived }
    public var desktopBytesSent: Int64 { rawValue.desktopBytesSent }
    public var desktopBytesReceived: Int64 { rawValue.desktopBytesReceived }
    public var udpPingTimeMilliseconds: Int32? { rawValue.udpPingTimeMilliseconds }
    public var tcpPingTimeMilliseconds: Int32? { rawValue.tcpPingTimeMilliseconds }
    public var tcpServerSilenceSeconds: Int32 { rawValue.tcpServerSilenceSeconds }
    public var udpServerSilenceSeconds: Int32 { rawValue.udpServerSilenceSeconds }
    public var soundInputDeviceDelayMilliseconds: Int32 { rawValue.soundInputDeviceDelayMilliseconds }
}

/// Timing thresholds for detecting a lost connection and keeping the TCP/UDP
/// sockets alive through NATs/firewalls.
public struct TeamTalkClientKeepAlive {
    public let rawValue: ClientKeepAlive

    public init(_ rawValue: ClientKeepAlive) {
        self.rawValue = rawValue
    }

    public var cValue: ClientKeepAlive {
        rawValue
    }

    public var connectionLostMilliseconds: Int32 { rawValue.connectionLostMilliseconds }
    public var tcpKeepAliveIntervalMilliseconds: Int32 { rawValue.tcpKeepAliveIntervalMilliseconds }
    public var udpKeepAliveIntervalMilliseconds: Int32 { rawValue.udpKeepAliveIntervalMilliseconds }
    public var udpKeepAliveRetransmitMilliseconds: Int32 { rawValue.udpKeepAliveRetransmitMilliseconds }
    public var udpConnectRetransmitMilliseconds: Int32 { rawValue.udpConnectRetransmitMilliseconds }
    public var udpConnectTimeoutMilliseconds: Int32 { rawValue.udpConnectTimeoutMilliseconds }
}

/// Mutable counterpart to ``TeamTalkClientKeepAlive`` for
/// `TeamTalkSession.setClientKeepAlive(_:)`.
public struct TeamTalkClientKeepAliveConfiguration {
    public var connectionLostMilliseconds: Int32
    public var tcpKeepAliveIntervalMilliseconds: Int32
    public var udpKeepAliveIntervalMilliseconds: Int32
    public var udpKeepAliveRetransmitMilliseconds: Int32
    public var udpConnectRetransmitMilliseconds: Int32
    public var udpConnectTimeoutMilliseconds: Int32

    public init(
        connectionLostMilliseconds: Int32,
        tcpKeepAliveIntervalMilliseconds: Int32 = 0,
        udpKeepAliveIntervalMilliseconds: Int32 = 0,
        udpKeepAliveRetransmitMilliseconds: Int32 = 0,
        udpConnectRetransmitMilliseconds: Int32 = 0,
        udpConnectTimeoutMilliseconds: Int32 = 0
    ) {
        self.connectionLostMilliseconds = connectionLostMilliseconds
        self.tcpKeepAliveIntervalMilliseconds = tcpKeepAliveIntervalMilliseconds
        self.udpKeepAliveIntervalMilliseconds = udpKeepAliveIntervalMilliseconds
        self.udpKeepAliveRetransmitMilliseconds = udpKeepAliveRetransmitMilliseconds
        self.udpConnectRetransmitMilliseconds = udpConnectRetransmitMilliseconds
        self.udpConnectTimeoutMilliseconds = udpConnectTimeoutMilliseconds
    }

    public init(_ keepAlive: TeamTalkClientKeepAlive) {
        self.init(
            connectionLostMilliseconds: keepAlive.connectionLostMilliseconds,
            tcpKeepAliveIntervalMilliseconds: keepAlive.tcpKeepAliveIntervalMilliseconds,
            udpKeepAliveIntervalMilliseconds: keepAlive.udpKeepAliveIntervalMilliseconds,
            udpKeepAliveRetransmitMilliseconds: keepAlive.udpKeepAliveRetransmitMilliseconds,
            udpConnectRetransmitMilliseconds: keepAlive.udpConnectRetransmitMilliseconds,
            udpConnectTimeoutMilliseconds: keepAlive.udpConnectTimeoutMilliseconds
        )
    }

    public var cValue: ClientKeepAlive {
        var keepAlive = ClientKeepAlive()
        keepAlive.connectionLostMilliseconds = connectionLostMilliseconds
        keepAlive.tcpKeepAliveIntervalMilliseconds = tcpKeepAliveIntervalMilliseconds
        keepAlive.udpKeepAliveIntervalMilliseconds = udpKeepAliveIntervalMilliseconds
        keepAlive.udpKeepAliveRetransmitMilliseconds = udpKeepAliveRetransmitMilliseconds
        keepAlive.udpConnectRetransmitMilliseconds = udpConnectRetransmitMilliseconds
        keepAlive.udpConnectTimeoutMilliseconds = udpConnectTimeoutMilliseconds
        return keepAlive
    }
}
