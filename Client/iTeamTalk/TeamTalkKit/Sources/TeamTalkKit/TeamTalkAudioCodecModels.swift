import Foundation
import TeamTalkC

/// Settings for the OPUS audio codec. See `OpusCodec` in the C SDK.
public struct TeamTalkOpusCodecConfiguration: Hashable, Sendable {
    public var sampleRate: Int32
    public var channels: Int32
    public var application: Int32
    public var complexity: Int32
    public var forwardErrorCorrectionEnabled: Bool
    public var discontinuousTransmissionEnabled: Bool
    public var bitrate: Int32
    public var variableBitrateEnabled: Bool
    public var constrainedVariableBitrateEnabled: Bool
    public var transmitIntervalMilliseconds: Int32
    public var frameSizeMilliseconds: Int32

    public init(_ rawValue: OpusCodec = TeamTalkAudioCodec.makeOpusCodec()) {
        sampleRate = rawValue.nSampleRate
        channels = rawValue.nChannels
        application = rawValue.nApplication
        complexity = rawValue.nComplexity
        forwardErrorCorrectionEnabled = rawValue.bFEC != 0
        discontinuousTransmissionEnabled = rawValue.bDTX != 0
        bitrate = rawValue.nBitRate
        variableBitrateEnabled = rawValue.bVBR != 0
        constrainedVariableBitrateEnabled = rawValue.bVBRConstraint != 0
        transmitIntervalMilliseconds = rawValue.nTxIntervalMSec
        frameSizeMilliseconds = rawValue.nFrameSizeMSec
    }

    public var cValue: OpusCodec {
        var codec = TeamTalkAudioCodec.makeOpusCodec()
        codec.nSampleRate = sampleRate
        codec.nChannels = channels
        codec.nApplication = application
        codec.nComplexity = complexity
        codec.bFEC = forwardErrorCorrectionEnabled ? 1 : 0
        codec.bDTX = discontinuousTransmissionEnabled ? 1 : 0
        codec.nBitRate = bitrate
        codec.bVBR = variableBitrateEnabled ? 1 : 0
        codec.bVBRConstraint = constrainedVariableBitrateEnabled ? 1 : 0
        codec.nTxIntervalMSec = transmitIntervalMilliseconds
        codec.nFrameSizeMSec = frameSizeMilliseconds
        return codec
    }

    public static let bitrateRange = Int32(OPUS_MIN_BITRATE)...Int32(OPUS_MAX_BITRATE)
    public static let maxFrameSizeMilliseconds = Int32(OPUS_REALMAX_FRAMESIZE)
    public static let applicationVOIP = Int32(OPUS_APPLICATION_VOIP)
    public static let applicationAudio = Int32(OPUS_APPLICATION_AUDIO)
}

/// Settings for the (non-VBR) Speex audio codec. See `SpeexCodec` in the C SDK.
public struct TeamTalkSpeexCodecConfiguration: Hashable, Sendable {
    public var bandmode: Int32
    public var quality: Int32
    public var transmitIntervalMilliseconds: Int32
    public var stereoPlaybackEnabled: Bool

    public init(_ rawValue: SpeexCodec = TeamTalkAudioCodec.makeSpeexCodec()) {
        bandmode = rawValue.nBandmode
        quality = rawValue.nQuality
        transmitIntervalMilliseconds = rawValue.nTxIntervalMSec
        stereoPlaybackEnabled = rawValue.bStereoPlayback != 0
    }

    public var cValue: SpeexCodec {
        var codec = TeamTalkAudioCodec.makeSpeexCodec()
        codec.nBandmode = bandmode
        codec.nQuality = quality
        codec.nTxIntervalMSec = transmitIntervalMilliseconds
        codec.bStereoPlayback = stereoPlaybackEnabled ? 1 : 0
        return codec
    }
}

/// Settings for the Speex audio codec in Variable Bitrate mode. See
/// `SpeexVBRCodec` in the C SDK.
public struct TeamTalkSpeexVBRCodecConfiguration: Hashable, Sendable {
    public var bandmode: Int32
    public var quality: Int32
    public var bitrate: Int32
    public var maxBitrate: Int32
    public var discontinuousTransmissionEnabled: Bool
    public var transmitIntervalMilliseconds: Int32
    public var stereoPlaybackEnabled: Bool

    public init(_ rawValue: SpeexVBRCodec = TeamTalkAudioCodec.makeSpeexVBRCodec()) {
        bandmode = rawValue.nBandmode
        quality = rawValue.nQuality
        bitrate = rawValue.nBitRate
        maxBitrate = rawValue.nMaxBitRate
        discontinuousTransmissionEnabled = rawValue.bDTX != 0
        transmitIntervalMilliseconds = rawValue.nTxIntervalMSec
        stereoPlaybackEnabled = rawValue.bStereoPlayback != 0
    }

    public var cValue: SpeexVBRCodec {
        var codec = TeamTalkAudioCodec.makeSpeexVBRCodec()
        codec.nBandmode = bandmode
        codec.nQuality = quality
        codec.nBitRate = bitrate
        codec.nMaxBitRate = maxBitrate
        codec.bDTX = discontinuousTransmissionEnabled ? 1 : 0
        codec.nTxIntervalMSec = transmitIntervalMilliseconds
        codec.bStereoPlayback = stereoPlaybackEnabled ? 1 : 0
        return codec
    }

    public static let maxBitrateUpperBound = Int32(SPEEX_UWB_MAX_BITRATE)
}

/// Which audio codec a channel uses, and its settings — a type-safe union
/// over the C SDK's `AudioCodec` struct, which is a true C union keyed by
/// `nCodec`: only the matching Opus/Speex/SpeexVBR sub-struct is meaningful,
/// and switching codecs doesn't preserve the other two's settings (neither
/// does the underlying C union, which physically overlaps their storage).
public enum TeamTalkAudioCodecConfiguration: Hashable, Sendable {
    case none
    case opus(TeamTalkOpusCodecConfiguration)
    case speex(TeamTalkSpeexCodecConfiguration)
    case speexVBR(TeamTalkSpeexVBRCodecConfiguration)

    public init(_ rawValue: AudioCodec) {
        switch rawValue.nCodec {
        case OPUS_CODEC:
            self = .opus(TeamTalkOpusCodecConfiguration(TeamTalkAudioCodec.opusCodec(from: rawValue)))
        case SPEEX_CODEC:
            self = .speex(TeamTalkSpeexCodecConfiguration(TeamTalkAudioCodec.speexCodec(from: rawValue)))
        case SPEEX_VBR_CODEC:
            self = .speexVBR(TeamTalkSpeexVBRCodecConfiguration(TeamTalkAudioCodec.speexVBRCodec(from: rawValue)))
        default:
            self = .none
        }
    }

    public var codec: TeamTalkCodec {
        switch self {
        case .none: return .none
        case .opus: return .opus
        case .speex: return .speex
        case .speexVBR: return .speexVBR
        }
    }

    public var cValue: AudioCodec {
        switch self {
        case .none:
            return TeamTalkAudioCodec.makeAudioCodec(NO_CODEC)
        case .opus(let configuration):
            var audioCodec = TeamTalkAudioCodec.makeAudioCodec(OPUS_CODEC)
            TeamTalkAudioCodec.setOpusCodec(configuration.cValue, on: &audioCodec)
            return audioCodec
        case .speex(let configuration):
            var audioCodec = TeamTalkAudioCodec.makeAudioCodec(SPEEX_CODEC)
            TeamTalkAudioCodec.setSpeexCodec(configuration.cValue, on: &audioCodec)
            return audioCodec
        case .speexVBR(let configuration):
            var audioCodec = TeamTalkAudioCodec.makeAudioCodec(SPEEX_VBR_CODEC)
            TeamTalkAudioCodec.setSpeexVBRCodec(configuration.cValue, on: &audioCodec)
            return audioCodec
        }
    }
}
