import Foundation
import TeamTalkC

public extension TeamTalkClientFlags {
    init(cValue: ClientFlags) {
        self.init(rawValue: cValue)
    }

    var cValue: ClientFlags {
        rawValue
    }

    static let closed = TeamTalkClientFlags(rawValue: CLIENT_CLOSED.rawValue)
    static let soundOutputReady = TeamTalkClientFlags(rawValue: CLIENT_SNDOUTPUT_READY.rawValue)
    static let soundInputOutputDuplex = TeamTalkClientFlags(rawValue: CLIENT_SNDINOUTPUT_DUPLEX.rawValue)
    static let soundOutputMuted = TeamTalkClientFlags(rawValue: CLIENT_SNDOUTPUT_MUTE.rawValue)
    static let soundOutputAuto3DPosition = TeamTalkClientFlags(rawValue: CLIENT_SNDOUTPUT_AUTO3DPOSITION.rawValue)
    static let videoCaptureReady = TeamTalkClientFlags(rawValue: CLIENT_VIDEOCAPTURE_READY.rawValue)
    static let transmittingVideoCapture = TeamTalkClientFlags(rawValue: CLIENT_TX_VIDEOCAPTURE.rawValue)
    static let transmittingDesktop = TeamTalkClientFlags(rawValue: CLIENT_TX_DESKTOP.rawValue)
    static let desktopActive = TeamTalkClientFlags(rawValue: CLIENT_DESKTOP_ACTIVE.rawValue)
    static let muxingAudioFile = TeamTalkClientFlags(rawValue: CLIENT_MUX_AUDIOFILE.rawValue)
    static let connecting = TeamTalkClientFlags(rawValue: CLIENT_CONNECTING.rawValue)
    static let connection = TeamTalkClientFlags(rawValue: CLIENT_CONNECTION.rawValue)
    static let streamingAudio = TeamTalkClientFlags(rawValue: CLIENT_STREAM_AUDIO.rawValue)
    static let streamingVideo = TeamTalkClientFlags(rawValue: CLIENT_STREAM_VIDEO.rawValue)
}

public extension TeamTalkAudioCodec {
    /// A default raw `AudioCodec` configuration for `codec`, ready to pass
    /// into a C-level call or wrap in a ``TeamTalkAudioCodecConfiguration``.
    static func makeAudioCodec(_ codec: TeamTalkCodec) -> AudioCodec {
        makeAudioCodec(codec.cValue)
    }
}

public extension TeamTalkAudioPreprocessor {
    /// A default raw `AudioPreprocessor` configuration for `preprocessor`,
    /// ready to pass into a C-level call or wrap in a
    /// ``TeamTalkAudioPreprocessorConfiguration``.
    static func makeAudioPreprocessor(_ preprocessor: TeamTalkAudioPreprocessorType) -> AudioPreprocessor {
        makeAudioPreprocessor(preprocessor.cValue)
    }
}

public extension TeamTalkSession {
    /// This client's own user ID, typed. See ``TeamTalkSession/myUserID``.
    var myUserIdentifier: TeamTalkUserID {
        TeamTalkUserID(myUserID)
    }

    /// The channel this client currently occupies, typed. See ``TeamTalkSession/myChannelID``.
    var myChannelIdentifier: TeamTalkChannelID {
        TeamTalkChannelID(myChannelID)
    }

    /// The server's root channel ID, typed. See ``TeamTalkSession/rootChannelID``.
    var rootChannelIdentifier: TeamTalkChannelID {
        TeamTalkChannelID(rootChannelID)
    }

    /// This client's effective user rights, typed for `.contains(_:)` checks.
    /// See ``TeamTalkSession/myUserRights``.
    var myRights: TeamTalkUserRights {
        TeamTalkUserRights(cValue: myUserRights)
    }

    /// Whether this client currently holds `right`.
    func getUserRight(_ right: TeamTalkUserRights) -> Bool {
        myRights.contains(right)
    }

    /// Alias for ``getUserRight(_:)``.
    func hasUserRight(_ right: TeamTalkUserRights) -> Bool {
        getUserRight(right)
    }

    /// Sets this client's presence (available/away/...) and optional status
    /// message, visible to other users.
    @discardableResult
    func changeStatus(mode: TeamTalkStatusMode, message: String = "") -> Int32 {
        changeStatus(mode: mode.rawValue, message: message)
    }
}

// Bridges raw C struct fields to Swift-friendly Bool/enum types with the
// SDK's own naming (nGainLevel, bEnableAGC, ...); names below mirror the
// underlying DSP concept each field configures.
public extension AudioCodec {
    var type: TeamTalkCodec {
        get { TeamTalkCodec(cValue: nCodec) }
        set { nCodec = newValue.cValue }
    }
}

public extension AudioPreprocessor {
    var type: TeamTalkAudioPreprocessorType {
        get { TeamTalkAudioPreprocessorType(cValue: nPreprocessor) }
        set { nPreprocessor = newValue.cValue }
    }

    var speexDSP: SpeexDSP {
        get { speexdsp }
        set { speexdsp = newValue }
    }

    var teamTalkPreprocessor: TTAudioPreprocessor {
        get { ttpreprocessor }
        set { ttpreprocessor = newValue }
    }

    var webRTCAudioPreprocessor: WebRTCAudioPreprocessor {
        get { webrtc }
        set { webrtc = newValue }
    }
}

public extension SpeexDSP {
    var automaticGainControlEnabled: Bool {
        get { bEnableAGC != 0 }
        set { bEnableAGC = newValue ? 1 : 0 }
    }

    var gainLevel: Int32 {
        get { nGainLevel }
        set { nGainLevel = newValue }
    }

    var maxGainIncreaseDecibelsPerSecond: Int32 {
        get { nMaxIncDBSec }
        set { nMaxIncDBSec = newValue }
    }

    var maxGainDecreaseDecibelsPerSecond: Int32 {
        get { nMaxDecDBSec }
        set { nMaxDecDBSec = newValue }
    }

    var maxGainDecibels: Int32 {
        get { nMaxGainDB }
        set { nMaxGainDB = newValue }
    }

    var denoiseEnabled: Bool {
        get { bEnableDenoise != 0 }
        set { bEnableDenoise = newValue ? 1 : 0 }
    }

    var maxNoiseSuppressionDecibels: Int32 {
        get { nMaxNoiseSuppressDB }
        set { nMaxNoiseSuppressDB = newValue }
    }

    var echoCancellationEnabled: Bool {
        get { bEnableEchoCancellation != 0 }
        set { bEnableEchoCancellation = newValue ? 1 : 0 }
    }

    var echoSuppressionDecibels: Int32 {
        get { nEchoSuppress }
        set { nEchoSuppress = newValue }
    }

    var echoSuppressionActiveDecibels: Int32 {
        get { nEchoSuppressActive }
        set { nEchoSuppressActive = newValue }
    }
}

public extension TTAudioPreprocessor {
    var gainLevel: Int32 {
        get { nGainLevel }
        set { nGainLevel = newValue }
    }

    var muteLeftSpeaker: Bool {
        get { bMuteLeftSpeaker != 0 }
        set { bMuteLeftSpeaker = newValue ? 1 : 0 }
    }

    var muteRightSpeaker: Bool {
        get { bMuteRightSpeaker != 0 }
        set { bMuteRightSpeaker = newValue ? 1 : 0 }
    }
}

public extension WebRTCAudioPreprocessor {
    var preamplifierEnabled: Bool {
        get { preamplifier.bEnable != 0 }
        set { preamplifier.bEnable = newValue ? 1 : 0 }
    }

    var preamplifierFixedGainFactor: Float {
        get { preamplifier.fFixedGainFactor }
        set { preamplifier.fFixedGainFactor = newValue }
    }

    var echoCancellerEnabled: Bool {
        get { echocanceller.bEnable != 0 }
        set { echocanceller.bEnable = newValue ? 1 : 0 }
    }

    var noiseSuppressionEnabled: Bool {
        get { noisesuppression.bEnable != 0 }
        set { noisesuppression.bEnable = newValue ? 1 : 0 }
    }

    var noiseSuppressionLevel: TeamTalkWebRTCNoiseSuppressionLevel {
        get { TeamTalkWebRTCNoiseSuppressionLevel(noisesuppression.nLevel) }
        set { noisesuppression.nLevel = newValue.cValue }
    }

    var gainController2Enabled: Bool {
        get { gaincontroller2.bEnable != 0 }
        set { gaincontroller2.bEnable = newValue ? 1 : 0 }
    }

    var fixedDigitalGainDecibels: Float {
        get { gaincontroller2.fixeddigital.fGainDB }
        set { gaincontroller2.fixeddigital.fGainDB = newValue }
    }

    var adaptiveDigitalGainEnabled: Bool {
        get { gaincontroller2.adaptivedigital.bEnable != 0 }
        set { gaincontroller2.adaptivedigital.bEnable = newValue ? 1 : 0 }
    }

    var adaptiveHeadRoomDecibels: Float {
        get { gaincontroller2.adaptivedigital.fHeadRoomDB }
        set { gaincontroller2.adaptivedigital.fHeadRoomDB = newValue }
    }

    var adaptiveMaxGainDecibels: Float {
        get { gaincontroller2.adaptivedigital.fMaxGainDB }
        set { gaincontroller2.adaptivedigital.fMaxGainDB = newValue }
    }

    var adaptiveInitialGainDecibels: Float {
        get { gaincontroller2.adaptivedigital.fInitialGainDB }
        set { gaincontroller2.adaptivedigital.fInitialGainDB = newValue }
    }

    var adaptiveMaxGainChangeDecibelsPerSecond: Float {
        get { gaincontroller2.adaptivedigital.fMaxGainChangeDBPerSecond }
        set { gaincontroller2.adaptivedigital.fMaxGainChangeDBPerSecond = newValue }
    }

    var adaptiveMaxOutputNoiseLevelDecibelsFS: Float {
        get { gaincontroller2.adaptivedigital.fMaxOutputNoiseLevelDBFS }
        set { gaincontroller2.adaptivedigital.fMaxOutputNoiseLevelDBFS = newValue }
    }
}
