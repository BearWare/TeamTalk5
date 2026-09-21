import Foundation
import TeamTalkC

/// A raw PCM format request, e.g. for
/// `TeamTalkSession.enableAudioBlockEvent(sourceID:streamTypes:audioFormat:enabled:)`
/// to have the SDK resample/reformat delivered audio blocks.
public struct TeamTalkAudioFormatConfiguration {
    public var format: TeamTalkAudioFileFormat
    public var sampleRate: Int32
    public var channels: Int32

    public init(
        format: TeamTalkAudioFileFormat = .wave,
        sampleRate: Int32,
        channels: Int32
    ) {
        self.format = format
        self.sampleRate = sampleRate
        self.channels = channels
    }

    public init(_ format: TeamTalkAudioFormat) {
        self.init(
            format: format.format,
            sampleRate: format.sampleRate,
            channels: format.channels
        )
    }

    public var isValid: Bool {
        sampleRate > 0 && channels > 0 && format != .none
    }

    public var cValue: AudioFormat {
        var audioFormat = AudioFormat()
        audioFormat.format = format
        audioFormat.nSampleRate = sampleRate
        audioFormat.nChannels = channels
        return audioFormat
    }
}

/// One chunk of raw PCM audio, delivered via
/// `TeamTalkSession.acquireAudioBlock(sourceID:streamTypes:)` after enabling
/// audio block events for a stream. Owns its samples as `Data`/`rawAudio`,
/// safe to hold onto past the call that produced it.
public struct TeamTalkAudioBlock {
    public let streamID: Int32
    public let sampleRate: Int32
    public let channels: Int32
    public let samplesPerChannel: Int32
    public let sampleIndex: UInt32
    public let streamTypes: TeamTalkStreamTypes
    public let rawAudio: Data

    public init(
        streamID: Int32,
        sampleRate: Int32,
        channels: Int32,
        samplesPerChannel: Int32,
        sampleIndex: UInt32 = 0,
        streamTypes: TeamTalkStreamTypes,
        rawAudio: Data
    ) {
        self.streamID = streamID
        self.sampleRate = sampleRate
        self.channels = channels
        self.samplesPerChannel = samplesPerChannel
        self.sampleIndex = sampleIndex
        self.streamTypes = streamTypes
        self.rawAudio = rawAudio
    }

    public init(
        streamID: Int32,
        sampleRate: Int32,
        channels: Int32,
        sampleIndex: UInt32 = 0,
        streamTypes: TeamTalkStreamTypes,
        pcmSamples: [Int16]
    ) {
        let channels = max(1, channels)
        let samplesPerChannel = channels > 0 ? Int32(pcmSamples.count / Int(channels)) : 0
        self.init(
            streamID: streamID,
            sampleRate: sampleRate,
            channels: channels,
            samplesPerChannel: samplesPerChannel,
            sampleIndex: sampleIndex,
            streamTypes: streamTypes,
            rawAudio: pcmSamples.withUnsafeBufferPointer { buffer in
                Data(buffer: buffer)
            }
        )
    }

    public init(_ rawValue: AudioBlock) {
        self.init(
            streamID: rawValue.streamID,
            sampleRate: rawValue.sampleRate,
            channels: rawValue.channels,
            samplesPerChannel: rawValue.samplesPerChannel,
            sampleIndex: rawValue.sampleIndex,
            streamTypes: rawValue.streamTypes,
            rawAudio: rawValue.rawAudioData
        )
    }

    public var totalSampleCount: Int {
        max(0, Int(samplesPerChannel * max(0, channels)))
    }

    public var pcmSamples: [Int16] {
        rawAudio.withUnsafeBytes { rawBuffer in
            guard let baseAddress = rawBuffer.baseAddress else {
                return []
            }
            let buffer = baseAddress.bindMemory(to: Int16.self, capacity: totalSampleCount)
            return Array(UnsafeBufferPointer(start: buffer, count: totalSampleCount))
        }
    }

    public var durationMilliseconds: Double? {
        guard sampleRate > 0 else {
            return nil
        }
        return (Double(samplesPerChannel) * 1000.0) / Double(sampleRate)
    }

    public func withUnsafeCValue<Result>(_ body: (AudioBlock) -> Result) -> Result {
        rawAudio.withUnsafeBytes { buffer in
            var audioBlock = AudioBlock()
            audioBlock.nStreamID = streamID
            audioBlock.nSampleRate = sampleRate
            audioBlock.nChannels = channels
            audioBlock.nSamples = samplesPerChannel
            audioBlock.uSampleIndex = sampleIndex
            audioBlock.uStreamTypes = streamTypes.cMask
            audioBlock.lpRawAudio = buffer.isEmpty ? nil : UnsafeMutableRawPointer(mutating: buffer.baseAddress)
            return body(audioBlock)
        }
    }
}

/// A PCM format as actually reported back by the SDK (e.g. on a media
/// file), as opposed to a format a caller is requesting — see
/// ``TeamTalkAudioFormatConfiguration``.
public struct TeamTalkAudioFormat {
    public let rawValue: AudioFormat

    public init(_ rawValue: AudioFormat) {
        self.rawValue = rawValue
    }

    public var cValue: AudioFormat {
        rawValue
    }

    public var format: TeamTalkAudioFileFormat {
        rawValue.format
    }

    public var sampleRate: Int32 {
        rawValue.sampleRate
    }

    public var channels: Int32 {
        rawValue.channels
    }

    public var isAvailable: Bool {
        rawValue.isAvailable
    }
}

/// A resolution/frame-rate/pixel-format combination a video capture device
/// or media file supports.
public struct TeamTalkVideoFormat {
    public let rawValue: VideoFormat

    public init(_ rawValue: VideoFormat) {
        self.rawValue = rawValue
    }

    public var cValue: VideoFormat {
        rawValue
    }

    public var width: Int32 {
        rawValue.width
    }

    public var height: Int32 {
        rawValue.height
    }

    public var frameRateNumerator: Int32 {
        rawValue.frameRateNumerator
    }

    public var frameRateDenominator: Int32 {
        rawValue.frameRateDenominator
    }

    public var pixelFormat: TeamTalkVideoPixelFormat {
        rawValue.pixelFormat
    }

    public var framesPerSecond: Double? {
        rawValue.framesPerSecond
    }
}

/// Format/duration/playback-progress info for a local media file, from
/// `TeamTalkSession.mediaFile(at:)`.
public struct TeamTalkMediaFileInfo {
    public let rawValue: MediaFileInfo

    public init(_ rawValue: MediaFileInfo) {
        self.rawValue = rawValue
    }

    public var cValue: MediaFileInfo {
        rawValue
    }

    public var status: TeamTalkMediaFileStatus {
        rawValue.status
    }

    public var fileName: String {
        rawValue.fileName
    }

    public var audioFormat: TeamTalkAudioFormat {
        TeamTalkAudioFormat(rawValue.audioFmt)
    }

    public var videoFormat: TeamTalkVideoFormat {
        TeamTalkVideoFormat(rawValue.videoFmt)
    }

    public var durationMilliseconds: UInt32 {
        rawValue.durationMilliseconds
    }

    public var elapsedMilliseconds: UInt32 {
        rawValue.elapsedMilliseconds
    }

    public var progress: Double {
        rawValue.progress
    }
}

/// Swift-native mirror of the Speex-based preprocessor's tunable fields (see
/// TeamTalkConvenienceExtensions.swift's `SpeexDSP` extension for what each
/// one configures), for use inside ``TeamTalkAudioPreprocessorConfiguration``.
public struct TeamTalkSpeexDSPConfiguration: Hashable, Sendable {
    public var automaticGainControlEnabled: Bool
    public var gainLevel: Int32
    public var maxGainIncreaseDecibelsPerSecond: Int32
    public var maxGainDecreaseDecibelsPerSecond: Int32
    public var maxGainDecibels: Int32
    public var denoiseEnabled: Bool
    public var maxNoiseSuppressionDecibels: Int32
    public var echoCancellationEnabled: Bool
    public var echoSuppressionDecibels: Int32
    public var echoSuppressionActiveDecibels: Int32

    public init(_ rawValue: SpeexDSP = TeamTalkAudioPreprocessor.makeSpeexDSP()) {
        automaticGainControlEnabled = rawValue.automaticGainControlEnabled
        gainLevel = rawValue.gainLevel
        maxGainIncreaseDecibelsPerSecond = rawValue.maxGainIncreaseDecibelsPerSecond
        maxGainDecreaseDecibelsPerSecond = rawValue.maxGainDecreaseDecibelsPerSecond
        maxGainDecibels = rawValue.maxGainDecibels
        denoiseEnabled = rawValue.denoiseEnabled
        maxNoiseSuppressionDecibels = rawValue.maxNoiseSuppressionDecibels
        echoCancellationEnabled = rawValue.echoCancellationEnabled
        echoSuppressionDecibels = rawValue.echoSuppressionDecibels
        echoSuppressionActiveDecibels = rawValue.echoSuppressionActiveDecibels
    }

    public var cValue: SpeexDSP {
        var speexDSP = TeamTalkAudioPreprocessor.makeSpeexDSP()
        speexDSP.automaticGainControlEnabled = automaticGainControlEnabled
        speexDSP.gainLevel = gainLevel
        speexDSP.maxGainIncreaseDecibelsPerSecond = maxGainIncreaseDecibelsPerSecond
        speexDSP.maxGainDecreaseDecibelsPerSecond = maxGainDecreaseDecibelsPerSecond
        speexDSP.maxGainDecibels = maxGainDecibels
        speexDSP.denoiseEnabled = denoiseEnabled
        speexDSP.maxNoiseSuppressionDecibels = maxNoiseSuppressionDecibels
        speexDSP.echoCancellationEnabled = echoCancellationEnabled
        speexDSP.echoSuppressionDecibels = echoSuppressionDecibels
        speexDSP.echoSuppressionActiveDecibels = echoSuppressionActiveDecibels
        return speexDSP
    }
}

/// Swift-native mirror of TeamTalk's own (non-Speex, non-WebRTC)
/// preprocessor: just gain and per-speaker muting.
public struct TeamTalkTTAudioPreprocessorConfiguration: Hashable, Sendable {
    public var gainLevel: Int32
    public var muteLeftSpeaker: Bool
    public var muteRightSpeaker: Bool

    public init(_ rawValue: TTAudioPreprocessor = TeamTalkAudioPreprocessor.makeTTAudioPreprocessor()) {
        gainLevel = rawValue.gainLevel
        muteLeftSpeaker = rawValue.muteLeftSpeaker
        muteRightSpeaker = rawValue.muteRightSpeaker
    }

    public var cValue: TTAudioPreprocessor {
        var preprocessor = TeamTalkAudioPreprocessor.makeTTAudioPreprocessor()
        preprocessor.gainLevel = gainLevel
        preprocessor.muteLeftSpeaker = muteLeftSpeaker
        preprocessor.muteRightSpeaker = muteRightSpeaker
        return preprocessor
    }
}

/// Swift-native mirror of the WebRTC-based preprocessor's tunable fields
/// (see TeamTalkConvenienceExtensions.swift's `WebRTCAudioPreprocessor`
/// extension for what each one configures).
public struct TeamTalkWebRTCAudioPreprocessorConfiguration: Hashable, Sendable {
    public var preamplifierEnabled: Bool
    public var preamplifierFixedGainFactor: Float
    public var echoCancellerEnabled: Bool
    public var noiseSuppressionEnabled: Bool
    public var noiseSuppressionLevel: TeamTalkWebRTCNoiseSuppressionLevel
    public var gainController2Enabled: Bool
    public var fixedDigitalGainDecibels: Float
    public var adaptiveDigitalGainEnabled: Bool
    public var adaptiveHeadRoomDecibels: Float
    public var adaptiveMaxGainDecibels: Float
    public var adaptiveInitialGainDecibels: Float
    public var adaptiveMaxGainChangeDecibelsPerSecond: Float
    public var adaptiveMaxOutputNoiseLevelDecibelsFS: Float

    public init(_ rawValue: WebRTCAudioPreprocessor = TeamTalkAudioPreprocessor.makeWebRTCAudioPreprocessor()) {
        preamplifierEnabled = rawValue.preamplifierEnabled
        preamplifierFixedGainFactor = rawValue.preamplifierFixedGainFactor
        echoCancellerEnabled = rawValue.echoCancellerEnabled
        noiseSuppressionEnabled = rawValue.noiseSuppressionEnabled
        noiseSuppressionLevel = rawValue.noiseSuppressionLevel
        gainController2Enabled = rawValue.gainController2Enabled
        fixedDigitalGainDecibels = rawValue.fixedDigitalGainDecibels
        adaptiveDigitalGainEnabled = rawValue.adaptiveDigitalGainEnabled
        adaptiveHeadRoomDecibels = rawValue.adaptiveHeadRoomDecibels
        adaptiveMaxGainDecibels = rawValue.adaptiveMaxGainDecibels
        adaptiveInitialGainDecibels = rawValue.adaptiveInitialGainDecibels
        adaptiveMaxGainChangeDecibelsPerSecond = rawValue.adaptiveMaxGainChangeDecibelsPerSecond
        adaptiveMaxOutputNoiseLevelDecibelsFS = rawValue.adaptiveMaxOutputNoiseLevelDecibelsFS
    }

    public var cValue: WebRTCAudioPreprocessor {
        var preprocessor = TeamTalkAudioPreprocessor.makeWebRTCAudioPreprocessor()
        preprocessor.preamplifierEnabled = preamplifierEnabled
        preprocessor.preamplifierFixedGainFactor = preamplifierFixedGainFactor
        preprocessor.echoCancellerEnabled = echoCancellerEnabled
        preprocessor.noiseSuppressionEnabled = noiseSuppressionEnabled
        preprocessor.noiseSuppressionLevel = noiseSuppressionLevel
        preprocessor.gainController2Enabled = gainController2Enabled
        preprocessor.fixedDigitalGainDecibels = fixedDigitalGainDecibels
        preprocessor.adaptiveDigitalGainEnabled = adaptiveDigitalGainEnabled
        preprocessor.adaptiveHeadRoomDecibels = adaptiveHeadRoomDecibels
        preprocessor.adaptiveMaxGainDecibels = adaptiveMaxGainDecibels
        preprocessor.adaptiveInitialGainDecibels = adaptiveInitialGainDecibels
        preprocessor.adaptiveMaxGainChangeDecibelsPerSecond = adaptiveMaxGainChangeDecibelsPerSecond
        preprocessor.adaptiveMaxOutputNoiseLevelDecibelsFS = adaptiveMaxOutputNoiseLevelDecibelsFS
        return preprocessor
    }

    public static let fixedDigitalGainMaxDecibels = Float(WEBRTC_GAINCONTROLLER2_FIXEDGAIN_MAX)
}

/// Which audio preprocessor backend to use for sound input, and its
/// settings — a type-safe union over the C SDK's single `AudioPreprocessor`
/// struct, which otherwise only says which of its several embedded configs
/// (Speex, TeamTalk, WebRTC) is actually meaningful via a separate `type` tag.
public enum TeamTalkAudioPreprocessorConfiguration: Hashable, Sendable {
    case none
    case speexDSP(TeamTalkSpeexDSPConfiguration)
    case teamTalk(TeamTalkTTAudioPreprocessorConfiguration)
    /// Decoded for compatibility with older configurations; not selectable
    /// going forward — use ``webRTC(_:)`` instead.
    case obsoleteWebRTC
    case webRTC(TeamTalkWebRTCAudioPreprocessorConfiguration)

    public init(_ rawValue: AudioPreprocessor) {
        switch rawValue.type {
        case .none:
            self = .none
        case .speexDSP:
            self = .speexDSP(TeamTalkSpeexDSPConfiguration(rawValue.speexDSP))
        case .teamTalk:
            self = .teamTalk(TeamTalkTTAudioPreprocessorConfiguration(rawValue.teamTalkPreprocessor))
        case .obsoleteWebRTC:
            self = .obsoleteWebRTC
        case .webRTC:
            self = .webRTC(TeamTalkWebRTCAudioPreprocessorConfiguration(rawValue.webRTCAudioPreprocessor))
        default:
            self = .none
        }
    }

    public init(type: TeamTalkAudioPreprocessorType) {
        switch type {
        case .none:
            self = .none
        case .speexDSP:
            self = .speexDSP(TeamTalkSpeexDSPConfiguration())
        case .teamTalk:
            self = .teamTalk(TeamTalkTTAudioPreprocessorConfiguration())
        case .obsoleteWebRTC:
            self = .obsoleteWebRTC
        case .webRTC:
            self = .webRTC(TeamTalkWebRTCAudioPreprocessorConfiguration())
        default:
            self = .none
        }
    }

    public var type: TeamTalkAudioPreprocessorType {
        switch self {
        case .none:
            return .none
        case .speexDSP:
            return .speexDSP
        case .teamTalk:
            return .teamTalk
        case .obsoleteWebRTC:
            return .obsoleteWebRTC
        case .webRTC:
            return .webRTC
        }
    }

    public var cValue: AudioPreprocessor {
        switch self {
        case .none:
            return TeamTalkAudioPreprocessor.makeAudioPreprocessor(.none)
        case .speexDSP(let configuration):
            var preprocessor = TeamTalkAudioPreprocessor.makeAudioPreprocessor(.speexDSP)
            preprocessor.speexDSP = configuration.cValue
            return preprocessor
        case .teamTalk(let configuration):
            var preprocessor = TeamTalkAudioPreprocessor.makeAudioPreprocessor(.teamTalk)
            preprocessor.teamTalkPreprocessor = configuration.cValue
            return preprocessor
        case .obsoleteWebRTC:
            return TeamTalkAudioPreprocessor.makeAudioPreprocessor(.obsoleteWebRTC)
        case .webRTC(let configuration):
            var preprocessor = TeamTalkAudioPreprocessor.makeAudioPreprocessor(.webRTC)
            preprocessor.webRTCAudioPreprocessor = configuration.cValue
            return preprocessor
        }
    }
}

/// Playback state for a media file streamed via
/// `TeamTalkSession.startStreamingMediaFileToChannel(from:playback:videoCodec:)`
/// or played locally via `initLocalPlayback(from:playback:)`.
public struct TeamTalkMediaFilePlayback {
    public let rawValue: MediaFilePlayback

    public init(_ rawValue: MediaFilePlayback) {
        self.rawValue = rawValue
    }

    public var cValue: MediaFilePlayback {
        rawValue
    }

    public var offsetMilliseconds: UInt32? {
        rawValue.offsetMilliseconds
    }

    public var isPaused: Bool {
        rawValue.isPaused
    }

    public var audioPreprocessor: AudioPreprocessor {
        rawValue.preprocessor
    }

    public var audioPreprocessorType: TeamTalkAudioPreprocessorType {
        rawValue.preprocessorType
    }

    public var audioPreprocessorConfiguration: TeamTalkAudioPreprocessorConfiguration {
        TeamTalkAudioPreprocessorConfiguration(rawValue.preprocessor)
    }
}

/// Mutable counterpart to ``TeamTalkMediaFilePlayback``.
public struct TeamTalkMediaFilePlaybackConfiguration {
    public var offsetMilliseconds: UInt32?
    public var isPaused: Bool
    public var audioPreprocessor: AudioPreprocessor

    public init(
        offsetMilliseconds: UInt32? = nil,
        isPaused: Bool = false,
        audioPreprocessor: AudioPreprocessor = TeamTalkAudioPreprocessor.makeAudioPreprocessor(NO_AUDIOPREPROCESSOR)
    ) {
        self.offsetMilliseconds = offsetMilliseconds
        self.isPaused = isPaused
        self.audioPreprocessor = audioPreprocessor
    }

    public init(_ playback: TeamTalkMediaFilePlayback) {
        self.init(
            offsetMilliseconds: playback.offsetMilliseconds,
            isPaused: playback.isPaused,
            audioPreprocessor: playback.audioPreprocessor
        )
    }

    public var cValue: MediaFilePlayback {
        var playback = MediaFilePlayback()
        playback.offsetMilliseconds = offsetMilliseconds
        playback.isPaused = isPaused
        playback.preprocessor = audioPreprocessor
        return playback
    }

    public var audioPreprocessorConfiguration: TeamTalkAudioPreprocessorConfiguration {
        get { TeamTalkAudioPreprocessorConfiguration(audioPreprocessor) }
        set { audioPreprocessor = newValue.cValue }
    }
}

/// Parameters for `TeamTalkSession.setUserMediaStorage(for:configuration:)`,
/// which auto-records a user's audio to per-user files as they speak.
public struct TeamTalkUserMediaStorageConfiguration {
    public var directoryURL: URL?
    public var fileNamePattern: String
    public var audioFileFormat: TeamTalkAudioFileFormat
    public var stopRecordingExtraDelayMilliseconds: Int32?

    public init(
        directoryURL: URL? = nil,
        fileNamePattern: String = "",
        audioFileFormat: TeamTalkAudioFileFormat = .none,
        stopRecordingExtraDelayMilliseconds: Int32? = nil
    ) {
        self.directoryURL = directoryURL
        self.fileNamePattern = fileNamePattern
        self.audioFileFormat = audioFileFormat
        self.stopRecordingExtraDelayMilliseconds = stopRecordingExtraDelayMilliseconds
    }

    public var isDisabled: Bool {
        directoryURL == nil || audioFileFormat == .none
    }

    public var usesDefaultFileNamePattern: Bool {
        fileNamePattern.isEmpty
    }
}

