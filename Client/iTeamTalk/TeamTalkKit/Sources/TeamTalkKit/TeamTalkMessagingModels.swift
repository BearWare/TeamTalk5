import Foundation
import TeamTalkC

/// Progress/state for one upload or download in flight, from
/// `TeamTalkSession.uploadFile(at:to:)`/`downloadFile(_:to:)`.
public struct TeamTalkFileTransfer: Identifiable, Equatable, Hashable, Sendable {
    public let rawValue: FileTransfer

    public init(_ rawValue: FileTransfer) {
        self.rawValue = rawValue
    }

    public var cValue: FileTransfer {
        rawValue
    }

    public static func == (lhs: TeamTalkFileTransfer, rhs: TeamTalkFileTransfer) -> Bool {
        rawStructsEqual(lhs.rawValue, rhs.rawValue)
    }

    public func hash(into hasher: inout Hasher) {
        hashRawStruct(rawValue, into: &hasher)
    }

    public var id: Int32 {
        rawValue.id
    }

    public var transferID: TeamTalkTransferID {
        TeamTalkTransferID(id)
    }

    public var channelID: Int32 {
        rawValue.channelID
    }

    public var channelIdentifier: TeamTalkChannelID {
        TeamTalkChannelID(channelID)
    }

    public var status: TeamTalkFileTransferStatus {
        rawValue.status
    }

    public var localFilePath: String {
        rawValue.localFilePath
    }

    public var remoteFileName: String {
        rawValue.remoteFileName
    }

    public var fileSize: Int64 {
        rawValue.nFileSize
    }

    public var transferredBytes: Int64 {
        rawValue.nTransferred
    }

    public var isDownload: Bool {
        rawValue.isDownload
    }

    /// `transferredBytes / fileSize`, from `0.0` to `1.0`.
    public var progress: Double {
        rawValue.progress
    }
}

/// One text message as delivered by the SDK. May be only a fragment of a
/// longer message — see ``hasMoreContent`` and ``TeamTalkTextMessageAssembler``.
public struct TeamTalkTextMessage: Equatable, Hashable, Sendable {
    public let rawValue: TextMessage

    public init(_ rawValue: TextMessage) {
        self.rawValue = rawValue
    }

    public var cValue: TextMessage {
        rawValue
    }

    public static func == (lhs: TeamTalkTextMessage, rhs: TeamTalkTextMessage) -> Bool {
        rawStructsEqual(lhs.rawValue, rhs.rawValue)
    }

    public func hash(into hasher: inout Hasher) {
        hashRawStruct(rawValue, into: &hasher)
    }

    public var type: TeamTalkTextMessageType {
        rawValue.type
    }

    public var fromUserID: Int32 {
        rawValue.nFromUserID
    }

    public var fromUserIdentifier: TeamTalkUserID {
        TeamTalkUserID(fromUserID)
    }

    public var toUserID: Int32 {
        rawValue.nToUserID
    }

    public var toUserIdentifier: TeamTalkUserID {
        TeamTalkUserID(toUserID)
    }

    public var channelID: Int32 {
        rawValue.nChannelID
    }

    public var channelIdentifier: TeamTalkChannelID {
        TeamTalkChannelID(channelID)
    }

    public var content: String {
        rawValue.content
    }

    /// Whether more fragments of this same logical message follow. The SDK
    /// splits long messages across multiple `TeamTalkTextMessage` values;
    /// feed them through ``TeamTalkTextMessageAssembler`` to reassemble.
    public var hasMoreContent: Bool {
        rawValue.bMore != 0
    }
}

/// Identifies which in-flight multipart message a fragment belongs to, for
/// ``TeamTalkTextMessageAssembler``.
public struct TeamTalkTextMessageMultipartKey: Hashable, Sendable {
    public let type: TeamTalkTextMessageType
    public let fromUserID: TeamTalkUserID

    public init(type: TeamTalkTextMessageType, fromUserID: TeamTalkUserID) {
        self.type = type
        self.fromUserID = fromUserID
    }

    public init(_ message: TeamTalkTextMessage) {
        self.init(type: message.type, fromUserID: message.fromUserIdentifier)
    }
}

/// Reassembles multipart ``TeamTalkTextMessage`` sequences into complete
/// message text. Fragments are buffered per `(type, sender)` key, so a
/// single assembler can track several senders' in-progress messages at
/// once — feed it every incoming message, in order per sender.
public struct TeamTalkTextMessageAssembler {
    private var messageFragments = [TeamTalkTextMessageMultipartKey: [String]]()

    public init() {}

    /// Feeds one message in. Returns the complete text once the final
    /// fragment (`hasMoreContent == false`) arrives, `nil` while still
    /// buffering.
    public mutating func append(_ message: TeamTalkTextMessage) -> String? {
        let key = TeamTalkTextMessageMultipartKey(message)

        if message.hasMoreContent {
            if messageFragments[key] == nil {
                messageFragments[key] = []
            }
            messageFragments[key]?.append(message.content)
            if messageFragments[key]?.count ?? 0 > 1000 {
                messageFragments.removeValue(forKey: key)
            }
            return nil
        }

        guard let fragments = messageFragments.removeValue(forKey: key), !fragments.isEmpty else {
            return message.content
        }

        return fragments.joined() + message.content
    }

    public mutating func clear() {
        messageFragments.removeAll()
    }
}

/// A message to send via `TeamTalkSession.sendTextMessage(_:)`. Prefer the
/// `user(to:content:)`/`channel(_:content:)`/`broadcast(content:)`/
/// `reply(to:content:)` factory methods over the raw ID-based initializer.
public struct TeamTalkOutgoingTextMessage {
    public var type: TeamTalkTextMessageType
    public var toUserID: Int32
    public var channelID: Int32
    public var content: String

    public init(
        type: TeamTalkTextMessageType,
        toUserID: Int32 = 0,
        channelID: Int32 = 0,
        content: String
    ) {
        self.type = type
        self.toUserID = toUserID
        self.channelID = channelID
        self.content = content
    }

    public static func user(to user: TeamTalkUser, content: String) -> TeamTalkOutgoingTextMessage {
        TeamTalkOutgoingTextMessage(type: .user, toUserID: user.userID.cValue, content: content)
    }

    public static func channel(_ channel: TeamTalkChannel, content: String) -> TeamTalkOutgoingTextMessage {
        TeamTalkOutgoingTextMessage(type: .channel, channelID: channel.channelID.cValue, content: content)
    }

    public static func broadcast(content: String) -> TeamTalkOutgoingTextMessage {
        TeamTalkOutgoingTextMessage(type: .broadcast, content: content)
    }

    /// Constructs an outgoing reply matching the original message's scope.
    /// `.user`/`.custom` reply privately to the sender; `.channel` replies in the same channel; `.broadcast` rebroadcasts.
    public static func reply(to message: TeamTalkTextMessage, content: String) -> TeamTalkOutgoingTextMessage {
        if message.type == .channel {
            return TeamTalkOutgoingTextMessage(type: .channel, channelID: message.channelID, content: content)
        }
        if message.type == .broadcast {
            return TeamTalkOutgoingTextMessage(type: .broadcast, content: content)
        }
        return TeamTalkOutgoingTextMessage(type: message.type, toUserID: message.fromUserID, content: content)
    }

    public var toUserIdentifier: TeamTalkUserID {
        get { TeamTalkUserID(toUserID) }
        set { toUserID = newValue.cValue }
    }

    public var channelIdentifier: TeamTalkChannelID {
        get { TeamTalkChannelID(channelID) }
        set { channelID = newValue.cValue }
    }

    public var cValue: TextMessage {
        var message = TextMessage()
        message.type = type
        message.nToUserID = toUserID
        message.nChannelID = channelID
        TeamTalkString.setTextMessage(&message, to: content)
        return message
    }
}

/// Wraps a raw SDK error message with a typed ``errorCode`` and a
/// human-readable ``message``.
public struct TeamTalkClientError {
    public let rawValue: ClientErrorMsg

    public init(_ rawValue: ClientErrorMsg) {
        self.rawValue = rawValue
    }

    public var cValue: ClientErrorMsg {
        rawValue
    }

    public var code: Int32 {
        rawValue.nErrorNo
    }

    public var errorCode: TeamTalkErrorCode {
        TeamTalkErrorCode(rawValue: code)
    }

    public var message: String {
        TeamTalkString.clientError(rawValue)
    }
}
