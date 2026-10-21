import Foundation
import TeamTalkC

/// Receives raw `TTMessage` values as they're polled off the client's message
/// queue. Most callers want ``TeamTalkEventObserver`` instead, which decodes
/// messages into typed ``TeamTalkEvent`` cases.
public protocol TeamTalkMessageObserver: AnyObject {
    func handleTeamTalkMessage(_ message: TTMessage)
}

/// Receives decoded ``TeamTalkEvent`` values. Register with a session's
/// `addEventObserver(_:)` and unregister with `removeEventObserver(_:)` to
/// stop receiving callbacks. Observers are held weakly, so there's no need
/// to unregister purely to break a retain cycle.
public protocol TeamTalkEventObserver: AnyObject {
    func handleTeamTalkEvent(_ event: TeamTalkEvent)
}

final class TeamTalkMessageHandler {
    weak var value: TeamTalkMessageObserver?

    init(value: TeamTalkMessageObserver) {
        self.value = value
    }
}

final class TeamTalkEventHandler {
    weak var value: TeamTalkEventObserver?

    init(value: TeamTalkEventObserver) {
        self.value = value
    }
}

final class TeamTalkAsyncEventObserver: TeamTalkEventObserver {
    private let handler: (TeamTalkEvent) -> Void
    private let finishStream: () -> Void

    init(handler: @escaping (TeamTalkEvent) -> Void, finishStream: @escaping () -> Void) {
        self.handler = handler
        self.finishStream = finishStream
    }

    func handleTeamTalkEvent(_ event: TeamTalkEvent) {
        handler(event)
        if case .connectionLost = event.kind {
            finishStream()
        }
    }

    /// Ends the backing `AsyncStream` even when no `.connectionLost` event will
    /// ever arrive, e.g. when `close()` tears down the client synchronously.
    func finish() {
        finishStream()
    }
}

func withOptionalVideoCodecPointer<Result>(
    _ videoCodec: VideoCodec?,
    _ body: (UnsafePointer<VideoCodec>?) -> Result
) -> Result {
    guard var videoCodec else {
        return body(nil)
    }

    return withUnsafePointer(to: &videoCodec) { pointer in
        body(pointer)
    }
}

func withOptionalAudioFormatPointer<Result>(
    _ audioFormat: AudioFormat?,
    _ body: (UnsafePointer<AudioFormat>?) -> Result
) -> Result {
    guard var audioFormat else {
        return body(nil)
    }

    return withUnsafePointer(to: &audioFormat) { pointer in
        body(pointer)
    }
}

/// Snapshot of the client's connection/transmission state, mirroring the
/// SDK's `CLIENT_*` flags. Read via ``TeamTalkSession/flags``.
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

/// TLS material for an encrypted connection. Pass to `configureEncryption(_:)`
/// before connecting; leave fields empty to use the system trust store with
/// no client certificate.
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

/// One connection to a TeamTalk server, wrapping one native SDK instance.
/// Most of the public API lives in extensions across the package (commands,
/// events, audio, media, desktop/video, server state); this file holds the
/// instance itself and the small set of always-available properties.
///
/// The native SDK supports multiple independent instances in one process
/// (each SDK call takes an explicit instance handle, not implicit global
/// state), so nothing prevents creating more than one `TeamTalkSession` —
/// e.g. to connect to two servers at once, or to give each unit test its
/// own isolated instance. Most apps still only need one; hold it wherever
/// makes sense for that app (a single shared instance created at launch is
/// the common case) and pass it to whatever needs to issue commands or
/// observe events.
public final class TeamTalkSession {
    var instance: UnsafeMutableRawPointer?
    var observers = [TeamTalkMessageHandler]()
    var eventObservers = [TeamTalkEventHandler]()
    private let eventLoopQueue = DispatchQueue(label: "dk.bearware.TeamTalk5.TeamTalkSession.eventLoop")
    private let eventLoopLock = NSLock()
    let messagePollingLock = NSLock()
    private var eventLoopTimer: DispatchSourceTimer?

    public init() {}

    /// Whether a repeating timer is currently polling the SDK's message queue.
    /// See ``startEventDispatching(pollInterval:)``.
    public var isEventDispatching: Bool {
        eventLoopLock.lock()
        defer { eventLoopLock.unlock() }
        return eventLoopTimer != nil
    }

    /// Starts a repeating timer that drains the SDK's message queue and
    /// delivers decoded events to registered ``TeamTalkEventObserver``s.
    /// Idempotent: calling this again while already dispatching is a no-op.
    /// - Parameter pollInterval: How often to poll, in seconds.
    public func startEventDispatching(pollInterval: TimeInterval = 0.1) {
        eventLoopLock.lock()
        if eventLoopTimer != nil {
            eventLoopLock.unlock()
            return
        }

        let timer = DispatchSource.makeTimerSource(queue: eventLoopQueue)
        timer.schedule(deadline: .now(), repeating: pollInterval)
        timer.setEventHandler { [weak self] in
            self?.pollMessages()
        }
        eventLoopTimer = timer
        eventLoopLock.unlock()

        timer.resume()
    }

    /// Stops the polling timer started by ``startEventDispatching(pollInterval:)``.
    /// Safe to call even if dispatching was never started.
    public func stopEventDispatching() {
        eventLoopLock.lock()
        let timer = eventLoopTimer
        eventLoopTimer = nil
        eventLoopLock.unlock()

        timer?.setEventHandler {}
        timer?.cancel()
    }

    func ensureEventDispatching() {
        startEventDispatching()
    }

    /// Never actually runs (the `TT_GetRootChannelID(nil) == 1` guard is
    /// always false): this exists purely so the linker sees every native SDK
    /// symbol referenced, keeping them from being dead-stripped out of test
    /// builds that never call them directly.
    public static func touchLinkerSymbolsForTests() {
        if TT_GetRootChannelID(nil) == 1 {
            TT_CloseSoundOutputDevice(nil)
            TT_StartSoundLoopbackTest(0, 0, 0, 0, 0, nil)
            TT_CloseSoundLoopbackTest(nil)
            TT_CloseSoundInputDevice(nil)
            TT_GetSoundDevices(nil, nil)
            var defaultInputDeviceID: Int32 = 0
            var defaultOutputDeviceID: Int32 = 0
            TT_GetDefaultSoundDevices(&defaultInputDeviceID, &defaultOutputDeviceID)
            TT_GetDefaultSoundDevicesEx(SOUNDSYSTEM_NONE, &defaultInputDeviceID, &defaultOutputDeviceID)
            TT_InitSoundInputSharedDevice(0, 0, 0)
            TT_InitSoundOutputSharedDevice(0, 0, 0)
            TT_InitSoundDuplexDevices(nil, 0, 0)
            TT_CloseSoundDuplexDevices(nil)
            var soundDeviceEffects = SoundDeviceEffects()
            TT_GetSoundDeviceEffects(nil, &soundDeviceEffects)
            TT_SetSoundDeviceEffects(nil, &soundDeviceEffects)
            var speexDSP = SpeexDSP()
            TT_GetSoundInputPreprocess(nil, &speexDSP)
            TT_SetSoundInputPreprocess(nil, &speexDSP)
            var audioPreprocessor = AudioPreprocessor()
            TT_GetSoundInputPreprocessEx(nil, &audioPreprocessor)
            TT_SetSoundInputPreprocessEx(nil, &audioPreprocessor)
            TT_StartStreamingMediaFileToChannel(nil, "", nil)
            var mediaPlayback = MediaFilePlayback()
            TT_StartStreamingMediaFileToChannelEx(nil, "", &mediaPlayback, nil)
            TT_UpdateStreamingMediaFileToChannel(nil, &mediaPlayback, nil)
            TT_StopStreamingMediaFileToChannel(nil)
            let playbackSessionID = TT_InitLocalPlayback(nil, "", &mediaPlayback)
            TT_UpdateLocalPlayback(nil, playbackSessionID, &mediaPlayback)
            TT_StopLocalPlayback(nil, playbackSessionID)
            var mediaFileInfo = MediaFileInfo()
            TT_GetMediaFileInfo("", &mediaFileInfo)
            var desktopWindow = DesktopWindow()
            TT_SendDesktopWindow(nil, &desktopWindow, BMP_NONE)
            TT_CloseDesktopWindow(nil)
            TT_SendDesktopCursorPosition(nil, 0, 0)
            var desktopInput = DesktopInput()
            TT_SendDesktopInput(nil, 0, &desktopInput, 1)
            let acquiredDesktopWindow = TT_AcquireUserDesktopWindow(nil, 0)
            let acquiredDesktopWindowEx = TT_AcquireUserDesktopWindowEx(nil, 0, BMP_NONE)
            TT_ReleaseUserDesktopWindow(nil, acquiredDesktopWindow)
            TT_ReleaseUserDesktopWindow(nil, acquiredDesktopWindowEx)
            var videoCaptureDeviceCount: Int32 = 0
            TT_GetVideoCaptureDevices(nil, &videoCaptureDeviceCount)
            var videoFormat = VideoFormat()
            TT_InitVideoCaptureDevice(nil, "", &videoFormat)
            TT_CloseVideoCaptureDevice(nil)
            TT_SetUserMediaStorageDir(nil, 0, "", "", AFF_NONE)
            TT_SetUserMediaStorageDirEx(nil, 0, "", "", AFF_NONE, 0)
            var audioCodec = AudioCodec()
            TT_StartRecordingMuxedAudioFile(nil, &audioCodec, "", AFF_NONE)
            TT_StartRecordingMuxedAudioFileEx(nil, 0, "", AFF_NONE)
            TT_StartRecordingMuxedStreams(nil, STREAMTYPE_NONE.rawValue, &audioCodec, "", AFF_NONE)
            TT_StopRecordingMuxedAudioFile(nil)
            TT_StopRecordingMuxedAudioFileEx(nil, 0)
            var audioFormat = AudioFormat()
            TT_EnableAudioBlockEventEx(nil, 0, STREAMTYPE_NONE.rawValue, &audioFormat, 0)
            var audioBlock = AudioBlock()
            TT_InsertAudioBlock(nil, &audioBlock)
            let acquiredAudioBlock = TT_AcquireUserAudioBlock(nil, STREAMTYPE_NONE.rawValue, 0)
            TT_ReleaseUserAudioBlock(nil, acquiredAudioBlock)
            TT_DoLeaveChannel(nil)
            TT_GetRootChannelID(nil)
            TT_DBG_SetSoundInputTone(nil, 0, 0)
            TT_DoLogin(nil, "", "", "")
            TT_RestartSoundSystem()
            TT_DoSendFile(nil, 0, "")
            TT_DoRecvFile(nil, 0, 0, "")
            TT_DoDeleteFile(nil, 0, 0)
            var count: Int32 = 0
            TT_GetChannelFiles(nil, 0, nil, &count)
            var remoteFile = RemoteFile()
            TT_GetChannelFile(nil, 0, 0, &remoteFile)
            var fileTransfer = FileTransfer()
            TT_GetFileTransferInfo(nil, 0, &fileTransfer)
            TT_CancelFileTransfer(nil, 0)
            var clientStatistics = ClientStatistics()
            TT_GetClientStatistics(nil, &clientStatistics)
            var clientKeepAlive = ClientKeepAlive()
            TT_GetClientKeepAlive(nil, &clientKeepAlive)
            TT_SetClientKeepAlive(nil, &clientKeepAlive)
            var userStatistics = UserStatistics()
            TT_GetUserStatistics(nil, 0, &userStatistics)
        }
    }

    /// The client's current connection/transmission state.
    public var flags: TeamTalkClientFlags {
        TeamTalkClientFlags(rawValue: TT_GetFlags(instance))
    }

    /// Whether the TCP control connection to the server is currently up.
    /// Does not by itself mean login succeeded — see ``isAuthorized``.
    public var isConnected: Bool {
        flags.contains(.connected)
    }

    /// Whether a sound input device has been successfully opened for capture.
    public var isSoundInputReady: Bool {
        flags.contains(.soundInputReady)
    }

    /// Whether the server has accepted this client's login.
    public var isAuthorized: Bool {
        flags.contains(.authorized)
    }

    /// Whether this client is currently transmitting voice.
    public var isVoiceTransmitting: Bool {
        isTransmitting(STREAMTYPE_VOICE)
    }

    /// The server-assigned user ID for this client's own login. Prefer
    /// `myUserIdentifier` (``TeamTalkUserID``) where a typed ID is expected.
    public var myUserID: Int32 {
        TT_GetMyUserID(instance)
    }

    /// The channel ID this client currently occupies. Prefer
    /// `myChannelIdentifier` (``TeamTalkChannelID``) where a typed ID is expected.
    public var myChannelID: Int32 {
        TT_GetMyChannelID(instance)
    }

    /// This client's effective user rights as a raw `USERRIGHT_*` bitmask.
    /// Prefer `myRights` (``TeamTalkUserRights``) for `.contains(_:)` checks.
    public var myUserRights: UInt32 {
        TT_GetMyUserRights(instance)
    }

    /// This client's account type (e.g. regular vs. admin).
    public var myTypes: TeamTalkUserTypes {
        TeamTalkUserTypes(cValue: TT_GetMyUserType(instance))
    }

    /// The opaque `nUserData` value associated with this client's own account.
    public var myUserData: Int32 {
        TT_GetMyUserData(instance)
    }

    /// The server's root channel ID.
    public var rootChannelID: Int32 {
        TT_GetRootChannelID(instance)
    }

    /// Master output volume, in the SDK's internal volume units (see
    /// `SOUND_VOLUME_MIN`/`MAX`/`DEFAULT`), not a 0–100 percentage.
    public var soundOutputVolume: Int32 {
        TT_GetSoundOutputVolume(instance)
    }

    /// Microphone gain level, in the SDK's internal volume units, not a
    /// 0–100 percentage.
    public var soundInputGainLevel: Int32 {
        TT_GetSoundInputGainLevel(instance)
    }

    /// The native TeamTalk SDK's version string.
    public var version: String {
        String(cString: TT_GetVersion())
    }

    /// The vendored TeamTalk SDK version this package was built against, as
    /// reported at compile time. See ``version`` for the linked library's
    /// own runtime-reported version, which normally matches.
    public static let sdkVersion = TeamTalkC.TEAMTALK_VERSION

}
