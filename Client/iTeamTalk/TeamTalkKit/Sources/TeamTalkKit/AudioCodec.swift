import Foundation
import TeamTalkC

public enum TeamTalkAudioCodec {
    public static func makeAudioCodec(_ codec: Codec) -> AudioCodec {
        TTKitMakeAudioCodec(codec)
    }

    public static func makeOpusCodec() -> OpusCodec {
        TTKitMakeOpusCodec()
    }

    public static func makeSpeexCodec() -> SpeexCodec {
        TTKitMakeSpeexCodec()
    }

    public static func makeSpeexVBRCodec() -> SpeexVBRCodec {
        TTKitMakeSpeexVBRCodec()
    }

    public static func opusCodec(from audioCodec: AudioCodec) -> OpusCodec {
        var audioCodec = audioCodec
        return TTKitGetOpusCodec(&audioCodec)
    }

    public static func speexCodec(from audioCodec: AudioCodec) -> SpeexCodec {
        var audioCodec = audioCodec
        return TTKitGetSpeexCodec(&audioCodec)
    }

    public static func speexVBRCodec(from audioCodec: AudioCodec) -> SpeexVBRCodec {
        var audioCodec = audioCodec
        return TTKitGetSpeexVBRCodec(&audioCodec)
    }

    public static func setOpusCodec(_ opusCodec: OpusCodec, on audioCodec: inout AudioCodec) {
        var opusCodec = opusCodec
        TTKitSetOpusCodec(&audioCodec, &opusCodec)
    }

    public static func setSpeexCodec(_ speexCodec: SpeexCodec, on audioCodec: inout AudioCodec) {
        var speexCodec = speexCodec
        TTKitSetSpeexCodec(&audioCodec, &speexCodec)
    }

    public static func setSpeexVBRCodec(_ speexVBRCodec: SpeexVBRCodec, on audioCodec: inout AudioCodec) {
        var speexVBRCodec = speexVBRCodec
        TTKitSetSpeexVBRCodec(&audioCodec, &speexVBRCodec)
    }
}
