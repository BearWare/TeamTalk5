import Foundation
import TeamTalkC

/// A decoded notification from the server or SDK, delivered to every
/// registered ``TeamTalkEventObserver``. Switch on ``kind``; ``rawMessage``
/// is kept around for cases ``Kind`` doesn't decode a payload for.
public struct TeamTalkEvent {
    public let rawMessage: TTMessage
    public let kind: Kind

    public init(_ rawMessage: TTMessage) {
        self.rawMessage = rawMessage
        self.kind = Kind(rawMessage)
    }
}

public extension TeamTalkEvent {
    /// What happened. Some cases only announce that new data is ready to
    /// pull (e.g. ``userAudioBlock(userID:streamType:)``,
    /// ``userDesktopWindow(userID:streamID:)``) rather than carrying the
    /// data itself — see the corresponding `acquire...` method on
    /// `TeamTalkSession` for those.
    enum Kind {
        case none
        case connectionSucceeded
        case connectionEncryptionError(TeamTalkClientError)
        case connectionFailed
        case connectionLost
        /// The server renegotiated the maximum single-message payload size
        /// mid-connection.
        case connectionMaxPayloadUpdated(maxPayloadSize: Int32)
        /// Marks the start (`isActive: true`) or end (`false`) of a
        /// long-running command; not itself a success/failure signal.
        case commandProcessing(commandID: TeamTalkCommandID, isActive: Bool)
        case commandError(commandID: TeamTalkCommandID, error: TeamTalkClientError)
        case commandSucceeded(commandID: TeamTalkCommandID)
        case myselfLoggedIn(userID: TeamTalkUserID, account: TeamTalkUserAccount)
        case myselfLoggedOut
        /// `by` is `nil` when the server doesn't attribute the kick to a
        /// specific user (e.g. a ban taking effect rather than a live kick).
        case myselfKicked(channelID: TeamTalkChannelID, by: TeamTalkUser?)
        case userLoggedIn(TeamTalkUser)
        case userLoggedOut(TeamTalkUser)
        case userUpdated(TeamTalkUser)
        case userJoined(TeamTalkUser)
        /// By the time this fires, `user`'s own channel has already changed;
        /// `previousChannelID` is the channel they just left.
        case userLeft(previousChannelID: TeamTalkChannelID, user: TeamTalkUser)
        case textMessage(TeamTalkTextMessage)
        case channelCreated(TeamTalkChannel)
        case channelUpdated(TeamTalkChannel)
        case channelRemoved(TeamTalkChannel)
        case serverUpdated(TeamTalkServerProperties)
        case serverStatistics(TeamTalkServerStatistics)
        case fileCreated(TeamTalkRemoteFile)
        case fileRemoved(TeamTalkRemoteFile)
        /// One page of results from `TeamTalkSession.listUserAccounts(startingAt:count:)`,
        /// delivered per account rather than as a single batched array.
        case userAccount(TeamTalkUserAccount)
        case userAccountCreated(TeamTalkUserAccount)
        case userAccountRemoved(TeamTalkUserAccount)
        case bannedUser(TeamTalkBannedUser)
        case userStateChanged(TeamTalkUser)
        case userVideoCapture(userID: TeamTalkUserID, streamID: TeamTalkMediaStreamID)
        case userMediaFileVideo(userID: TeamTalkUserID, streamID: TeamTalkMediaStreamID)
        case userDesktopWindow(userID: TeamTalkUserID, streamID: TeamTalkMediaStreamID)
        case userDesktopCursor(userID: TeamTalkUserID)
        case userDesktopInput(userID: TeamTalkUserID)
        case userRecordMediaFile(userID: TeamTalkUserID)
        case userAudioBlock(userID: TeamTalkUserID, streamType: TeamTalkStreamTypes)
        case internalError(TeamTalkClientError)
        case voiceActivation(isActive: Bool)
        /// Decoded for SDK message compatibility only: hotkey registration
        /// (`TT_HotKey_*`) is Windows-only in the native SDK, so nothing in
        /// this package can ever trigger this case on iOS or macOS.
        case hotkey(hotkeyID: Int32, isActive: Bool)
        /// See ``hotkey(hotkeyID:isActive:)`` — same Windows-only caveat.
        case hotkeyTest(keyCode: Int32, isActive: Bool)
        case fileTransfer(TeamTalkFileTransfer)
        case desktopWindowTransfer(sessionID: TeamTalkDesktopSessionID, bytesRemaining: Int32)
        /// A `startStreamingMediaFileToChannel`/`updateStreamingMediaFileToChannel`/
        /// `stopStreamingMediaFileToChannel` playback state change for this
        /// client's own channel stream. Carries no identifier since there's
        /// at most one such stream per client.
        case streamMediaFile
        /// A playback state change for a `initLocalPlayback(from:playback:)`
        /// session, identified by `sessionID`.
        case localMediaFile(sessionID: TeamTalkPlaybackSessionID)
        case audioInput(streamID: TeamTalkMediaStreamID)
        case userFirstVoiceStreamPacket(streamID: TeamTalkMediaStreamID, user: TeamTalkUser)
        case soundDeviceAdded
        case soundDeviceRemoved
        case soundDeviceUnplugged
        case soundDeviceNewDefaultInput
        case soundDeviceNewDefaultOutput
        case soundDeviceNewDefaultInputCommunicationDevice
        case soundDeviceNewDefaultOutputCommunicationDevice
        /// Catch-all for native SDK events without a dedicated case above
        /// yet. `event`/`payloadType` identify what arrived so a caller can
        /// fall back to ``TeamTalkEvent/rawMessage`` if needed.
        case unhandled(event: TeamTalkClientEvent, payloadType: TeamTalkPayloadType)
    }
}

private extension TeamTalkEvent.Kind {
    init(_ message: TTMessage) {
        switch message.event {
        case .none:
            self = .none
        case .connectionSucceeded:
            self = .connectionSucceeded
        case .connectionEncryptionError:
            self = .connectionEncryptionError(TeamTalkClientError(TeamTalkMessagePayload.clientError(from: message)))
        case .connectionFailed:
            self = .connectionFailed
        case .connectionLost:
            self = .connectionLost
        case .connectionMaxPayloadUpdated:
            self = .connectionMaxPayloadUpdated(maxPayloadSize: message.payloadSize)
        case .commandProcessing:
            self = .commandProcessing(commandID: TeamTalkCommandID(message.source), isActive: message.isActive)
        case .commandError:
            self = .commandError(
                commandID: TeamTalkCommandID(message.source),
                error: TeamTalkClientError(TeamTalkMessagePayload.clientError(from: message))
            )
        case .commandSucceeded:
            self = .commandSucceeded(commandID: TeamTalkCommandID(message.source))
        case .myselfLoggedIn:
            self = .myselfLoggedIn(
                userID: TeamTalkUserID(message.source),
                account: TeamTalkUserAccount(TeamTalkMessagePayload.userAccount(from: message))
            )
        case .myselfLoggedOut:
            self = .myselfLoggedOut
        case .myselfKicked:
            let user = message.payloadType == .user ? TeamTalkUser(TeamTalkMessagePayload.user(from: message)) : nil
            self = .myselfKicked(channelID: TeamTalkChannelID(message.source), by: user)
        case .userLoggedIn:
            self = .userLoggedIn(TeamTalkUser(TeamTalkMessagePayload.user(from: message)))
        case .userLoggedOut:
            self = .userLoggedOut(TeamTalkUser(TeamTalkMessagePayload.user(from: message)))
        case .userUpdated:
            self = .userUpdated(TeamTalkUser(TeamTalkMessagePayload.user(from: message)))
        case .userJoined:
            self = .userJoined(TeamTalkUser(TeamTalkMessagePayload.user(from: message)))
        case .userLeft:
            self = .userLeft(
                previousChannelID: TeamTalkChannelID(message.source),
                user: TeamTalkUser(TeamTalkMessagePayload.user(from: message))
            )
        case .userTextMessage:
            self = .textMessage(TeamTalkTextMessage(TeamTalkMessagePayload.textMessage(from: message)))
        case .channelCreated:
            self = .channelCreated(TeamTalkChannel(TeamTalkMessagePayload.channel(from: message)))
        case .channelUpdated:
            self = .channelUpdated(TeamTalkChannel(TeamTalkMessagePayload.channel(from: message)))
        case .channelRemoved:
            self = .channelRemoved(TeamTalkChannel(TeamTalkMessagePayload.channel(from: message)))
        case .serverUpdated:
            self = .serverUpdated(TeamTalkServerProperties(TeamTalkMessagePayload.serverProperties(from: message)))
        case .serverStatistics:
            self = .serverStatistics(TeamTalkServerStatistics(TeamTalkMessagePayload.serverStatistics(from: message)))
        case .fileCreated:
            self = .fileCreated(TeamTalkRemoteFile(TeamTalkMessagePayload.remoteFile(from: message)))
        case .fileRemoved:
            self = .fileRemoved(TeamTalkRemoteFile(TeamTalkMessagePayload.remoteFile(from: message)))
        case .userAccount:
            self = .userAccount(TeamTalkUserAccount(TeamTalkMessagePayload.userAccount(from: message)))
        case .bannedUser:
            self = .bannedUser(TeamTalkBannedUser(TeamTalkMessagePayload.bannedUser(from: message)))
        case .userAccountCreated:
            self = .userAccountCreated(TeamTalkUserAccount(TeamTalkMessagePayload.userAccount(from: message)))
        case .userAccountRemoved:
            self = .userAccountRemoved(TeamTalkUserAccount(TeamTalkMessagePayload.userAccount(from: message)))
        case .userStateChanged:
            self = .userStateChanged(TeamTalkUser(TeamTalkMessagePayload.user(from: message)))
        case .userVideoCapture:
            self = .userVideoCapture(
                userID: TeamTalkUserID(message.source),
                streamID: TeamTalkMediaStreamID(message.nStreamID)
            )
        case .userMediaFileVideo:
            self = .userMediaFileVideo(
                userID: TeamTalkUserID(message.source),
                streamID: TeamTalkMediaStreamID(message.nStreamID)
            )
        case .userDesktopWindow:
            self = .userDesktopWindow(
                userID: TeamTalkUserID(message.source),
                streamID: TeamTalkMediaStreamID(message.nStreamID)
            )
        case .userDesktopCursor:
            self = .userDesktopCursor(userID: TeamTalkUserID(message.source))
        case .userDesktopInput:
            self = .userDesktopInput(userID: TeamTalkUserID(message.source))
        case .userRecordMediaFile:
            self = .userRecordMediaFile(userID: TeamTalkUserID(message.source))
        case .userAudioBlock:
            self = .userAudioBlock(userID: TeamTalkUserID(message.source), streamType: message.streamType)
        case .internalError:
            self = .internalError(TeamTalkClientError(TeamTalkMessagePayload.clientError(from: message)))
        case .voiceActivation:
            self = .voiceActivation(isActive: message.isActive)
        case .hotkey:
            self = .hotkey(hotkeyID: message.source, isActive: message.isActive)
        case .hotkeyTest:
            self = .hotkeyTest(keyCode: message.source, isActive: message.isActive)
        case .fileTransfer:
            self = .fileTransfer(TeamTalkFileTransfer(TeamTalkMessagePayload.fileTransfer(from: message)))
        case .desktopWindowTransfer:
            self = .desktopWindowTransfer(
                sessionID: TeamTalkDesktopSessionID(message.source),
                bytesRemaining: message.nBytesRemain
            )
        case .streamMediaFile:
            self = .streamMediaFile
        case .localMediaFile:
            self = .localMediaFile(sessionID: TeamTalkPlaybackSessionID(message.source))
        case .audioInput:
            self = .audioInput(streamID: TeamTalkMediaStreamID(message.source))
        case .userFirstVoiceStreamPacket:
            self = .userFirstVoiceStreamPacket(
                streamID: TeamTalkMediaStreamID(message.source),
                user: TeamTalkUser(TeamTalkMessagePayload.user(from: message))
            )
        case .soundDeviceAdded:
            self = .soundDeviceAdded
        case .soundDeviceRemoved:
            self = .soundDeviceRemoved
        case .soundDeviceUnplugged:
            self = .soundDeviceUnplugged
        case .soundDeviceNewDefaultInput:
            self = .soundDeviceNewDefaultInput
        case .soundDeviceNewDefaultOutput:
            self = .soundDeviceNewDefaultOutput
        case .soundDeviceNewDefaultInputCommunicationDevice:
            self = .soundDeviceNewDefaultInputCommunicationDevice
        case .soundDeviceNewDefaultOutputCommunicationDevice:
            self = .soundDeviceNewDefaultOutputCommunicationDevice
        default:
            self = .unhandled(event: message.event, payloadType: message.payloadType)
        }
    }
}
