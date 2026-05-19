import Foundation
import TeamTalkC

public protocol TeamTalkMessageObserver: AnyObject {
    func handleTeamTalkMessage(_ message: TTMessage)
}

final class TeamTalkMessageHandler {
    weak var value: TeamTalkMessageObserver?

    init(value: TeamTalkMessageObserver) {
        self.value = value
    }
}

public struct TeamTalkClientFlags: OptionSet {
    public let rawValue: UInt32

    public init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    public static let connected = TeamTalkClientFlags(rawValue: CLIENT_CONNECTED.rawValue)
    public static let authorized = TeamTalkClientFlags(rawValue: CLIENT_AUTHORIZED.rawValue)
    public static let soundInputReady = TeamTalkClientFlags(rawValue: CLIENT_SNDINPUT_READY.rawValue)
    public static let transmittingVoice = TeamTalkClientFlags(rawValue: CLIENT_TX_VOICE.rawValue)
    public static let voiceActivated = TeamTalkClientFlags(rawValue: CLIENT_SNDINPUT_VOICEACTIVATED.rawValue)
    public static let voiceActive = TeamTalkClientFlags(rawValue: CLIENT_SNDINPUT_VOICEACTIVE.rawValue)
}

public struct TeamTalkEncryptionConfiguration {
    public var caCertificate: String
    public var certificate: String
    public var privateKey: String
    public var verifyPeer: Bool

    public init(
        caCertificate: String = "",
        certificate: String = "",
        privateKey: String = "",
        verifyPeer: Bool = false
    ) {
        self.caCertificate = caCertificate
        self.certificate = certificate
        self.privateKey = privateKey
        self.verifyPeer = verifyPeer
    }
}

public enum TeamTalkSoundDeviceID {
    public static let voiceProcessingIO = TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO
    public static let remoteIO = TT_SOUNDDEVICE_ID_REMOTEIO
}
public final class TeamTalkClient {
    public static let shared = TeamTalkClient()

    private var instance: UnsafeMutableRawPointer?
    private var observers = [TeamTalkMessageHandler]()

    private init() {}

    public static func touchLinkerSymbolsForTests() {
        if TT_GetRootChannelID(nil) == 1 {
            TT_CloseSoundOutputDevice(nil)
            TT_StartSoundLoopbackTest(0, 0, 0, 0, 0, nil)
            TT_CloseSoundLoopbackTest(nil)
            TT_CloseSoundInputDevice(nil)
            TT_GetSoundDevices(nil, nil)
            TT_DoLeaveChannel(nil)
            TT_GetRootChannelID(nil)
            TT_DBG_SetSoundInputTone(nil, 0, 0)
            TT_DoLogin(nil, "", "", "")
            TT_RestartSoundSystem()
        }
    }

    public var flags: TeamTalkClientFlags {
        TeamTalkClientFlags(rawValue: TT_GetFlags(instance))
    }

    public var isConnected: Bool {
        flags.contains(.connected)
    }

    public var isSoundInputReady: Bool {
        flags.contains(.soundInputReady)
    }

    public var isAuthorized: Bool {
        flags.contains(.authorized)
    }

    public var isVoiceTransmitting: Bool {
        isTransmitting(STREAMTYPE_VOICE)
    }

    public var myUserID: Int32 {
        TT_GetMyUserID(instance)
    }

    public var myChannelID: Int32 {
        TT_GetMyChannelID(instance)
    }

    public var myUserRights: UInt32 {
        TT_GetMyUserRights(instance)
    }

    public var rootChannelID: Int32 {
        TT_GetRootChannelID(instance)
    }

    public var soundOutputVolume: Int32 {
        TT_GetSoundOutputVolume(instance)
    }

    public var soundInputGainLevel: Int32 {
        TT_GetSoundInputGainLevel(instance)
    }

    public var version: String {
        String(cString: TT_GetVersion())
    }

    public func start(licenseName: String, licenseKey: String) {
        guard instance == nil else {
            return
        }

        TT_SetLicenseInformation(licenseName, licenseKey)
        instance = TT_InitTeamTalkPoll()
    }

    public func close() {
        guard let instance else {
            return
        }

        TT_CloseTeamTalk(instance)
        self.instance = nil
        observers.removeAll()
    }

    @discardableResult
    public func connect(toHost host: String, tcpPort: Int32, udpPort: Int32, encrypted: Bool) -> Bool {
        TT_Connect(instance, host, tcpPort, udpPort, 0, 0, encrypted ? 1 : 0) != 0
    }

    public func disconnect() {
        TT_Disconnect(instance)
    }

    @discardableResult
    public func login(nickname: String, username: String, password: String, clientName: String) -> Int32 {
        TT_DoLoginEx(instance, nickname, username, password, clientName)
    }

    public func channelID(fromPath path: String) -> Int32 {
        TT_GetChannelIDFromPath(instance, path)
    }

    public func isChannelOperator(userID: Int32? = nil, channelID: Int32) -> Bool {
        TT_IsChannelOperator(instance, userID ?? myUserID, channelID) != 0
    }

    public func withServerProperties<T>(_ body: (inout ServerProperties) -> T) -> T {
        var properties = ServerProperties()
        TT_GetServerProperties(instance, &properties)
        return body(&properties)
    }

    public func withChannel<T>(id channelID: Int32, _ body: (inout Channel) -> T) -> T {
        var channel = Channel()
        TT_GetChannel(instance, channelID, &channel)
        return body(&channel)
    }

    public func withUser<T>(id userID: Int32, _ body: (inout User) -> T) -> T {
        var user = User()
        TT_GetUser(instance, userID, &user)
        return body(&user)
    }

    @discardableResult
    public func join(channel: inout Channel) -> Int32 {
        TT_DoJoinChannel(instance, &channel)
    }

    @discardableResult
    public func joinChannel(id channelID: Int32, password: String = "") -> Int32 {
        TT_DoJoinChannelByID(instance, channelID, password)
    }

    @discardableResult
    public func update(channel: inout Channel) -> Int32 {
        TT_DoUpdateChannel(instance, &channel)
    }

    @discardableResult
    public func removeChannel(id channelID: Int32) -> Int32 {
        TT_DoRemoveChannel(instance, channelID)
    }

    @discardableResult
    public func changeNickname(_ nickname: String) -> Int32 {
        TT_DoChangeNickname(instance, nickname)
    }

    @discardableResult
    public func changeStatus(mode: Int32, message: String = "") -> Int32 {
        TT_DoChangeStatus(instance, mode, message)
    }

    @discardableResult
    public func kickUser(id userID: Int32, fromChannelID channelID: Int32) -> Int32 {
        TT_DoKickUser(instance, userID, channelID)
    }

    @discardableResult
    public func banUser(id userID: Int32, fromChannelID channelID: Int32) -> Int32 {
        TT_DoBanUser(instance, userID, channelID)
    }

    @discardableResult
    public func moveUser(id userID: Int32, toChannelID channelID: Int32) -> Int32 {
        TT_DoMoveUser(instance, userID, channelID)
    }

    @discardableResult
    public func sendTextMessage(_ message: inout TextMessage) -> Int32 {
        TT_DoTextMessage(instance, &message)
    }

    @discardableResult
    public func sendTextMessage(_ message: TextMessage, content: String) -> Bool {
        TeamTalkTextMessageFactory.messages(from: message, content: content).reduce(true) { sent, textMessage in
            var textMessage = textMessage
            return sent && sendTextMessage(&textMessage) > 0
        }
    }

    @discardableResult
    public func setEncryptionContext(_ encryption: inout EncryptionContext) -> Bool {
        TT_SetEncryptionContext(instance, &encryption) != 0
    }

    @discardableResult
    public func configureEncryption(
        _ configuration: TeamTalkEncryptionConfiguration,
        temporaryDirectory: URL? = nil
    ) throws -> Bool {
        var encryption = EncryptionContext()
        let directory: URL
        if let temporaryDirectory {
            directory = temporaryDirectory
        } else {
            directory = try FileManager.default.url(
                for: .autosavedInformationDirectory,
                in: .userDomainMask,
                appropriateFor: nil,
                create: true
            )
        }

        let caCertificateURL = directory.appendingPathComponent("ca_cert.pem")
        let certificateURL = directory.appendingPathComponent("cert.pem")
        let privateKeyURL = directory.appendingPathComponent("key.pem")
        let temporaryFiles = [
            (configuration.caCertificate, caCertificateURL, TeamTalkEncryptionStringProperty.caFile),
            (configuration.certificate, certificateURL, TeamTalkEncryptionStringProperty.certificateFile),
            (configuration.privateKey, privateKeyURL, TeamTalkEncryptionStringProperty.privateKeyFile)
        ]

        defer {
            for (_, url, _) in temporaryFiles {
                try? FileManager.default.removeItem(at: url)
            }
        }

        for (contents, url, property) in temporaryFiles where contents.isEmpty == false {
            try contents.write(to: url, atomically: true, encoding: .utf8)
            TeamTalkString.setEncryption(property, on: &encryption, to: url.path)
        }

        encryption.bVerifyPeer = configuration.verifyPeer ? 1 : 0
        encryption.nVerifyDepth = configuration.verifyPeer ? 0 : -1

        return setEncryptionContext(&encryption)
    }

    public func addObserver(_ observer: TeamTalkMessageObserver) {
        if observers.contains(where: { $0.value === observer }) {
            return
        }

        observers.append(TeamTalkMessageHandler(value: observer))
    }

    public func removeObserver(_ observer: TeamTalkMessageObserver) {
        observers.removeAll { $0.value === observer || $0.value == nil }
    }

    public func removeAllObservers() {
        observers.removeAll()
    }

    public func pollMessages() {
        var message = TTMessage()
        var waitMSec: Int32 = 0

        while TT_GetMessage(instance, &message, &waitMSec) != 0 {
            observers.removeAll { $0.value == nil }

            for observer in observers {
                observer.value?.handleTeamTalkMessage(message)
            }
        }
    }

    public func isTransmitting(_ stream: StreamType) -> Bool {
        switch stream {
        case STREAMTYPE_VOICE:
            let currentFlags = flags
            return currentFlags.contains(.transmittingVoice) ||
                (currentFlags.contains(.voiceActivated) && currentFlags.contains(.voiceActive))
        default:
            return false
        }
    }

    @discardableResult
    public func enableVoiceTransmission(_ enabled: Bool) -> Bool {
        TT_EnableVoiceTransmission(instance, enabled ? 1 : 0) != 0
    }

    @discardableResult
    public func enableVoiceActivation(_ enabled: Bool) -> Bool {
        TT_EnableVoiceActivation(instance, enabled ? 1 : 0) != 0
    }

    public func setVoiceActivationLevel(_ level: Int32) {
        TT_SetVoiceActivationLevel(instance, level)
    }

    public func setSoundInputGainLevel(_ level: Int32) {
        TT_SetSoundInputGainLevel(instance, level)
    }

    public func setSoundOutputVolume(_ volume: Int32) {
        TT_SetSoundOutputVolume(instance, volume)
    }

    public func closeSoundDevices() {
        TT_CloseSoundInputDevice(instance)
        TT_CloseSoundOutputDevice(instance)
    }

    @discardableResult
    public func initSoundInputDevice(id soundDeviceID: Int32) -> Bool {
        TT_InitSoundInputDevice(instance, soundDeviceID) != 0
    }

    @discardableResult
    public func initSoundOutputDevice(id soundDeviceID: Int32) -> Bool {
        TT_InitSoundOutputDevice(instance, soundDeviceID) != 0
    }

    @discardableResult
    public func setSoundInputPreprocess(_ preprocessor: inout AudioPreprocessor) -> Bool {
        TT_SetSoundInputPreprocessEx(instance, &preprocessor) != 0
    }

    public func setUserMute(userID: Int32, stream: StreamType, muted: Bool) {
        TT_SetUserMute(instance, userID, stream, muted ? 1 : 0)
    }

    public func setUserVolume(userID: Int32, stream: StreamType, volume: Int32) {
        TT_SetUserVolume(instance, userID, stream, volume)
    }

    public func setUserStereo(userID: Int32, stream: StreamType, leftSpeaker: TTBOOL, rightSpeaker: TTBOOL) {
        TT_SetUserStereo(instance, userID, stream, leftSpeaker, rightSpeaker)
    }

    @discardableResult
    public func subscribe(userID: Int32, subscriptions: UInt32) -> Int32 {
        TT_DoSubscribe(instance, userID, subscriptions)
    }

    @discardableResult
    public func unsubscribe(userID: Int32, subscriptions: UInt32) -> Int32 {
        TT_DoUnsubscribe(instance, userID, subscriptions)
    }

    public func pump(_ event: ClientEvent, source: Int32) {
        TT_PumpMessage(instance, event, source)
    }
}
