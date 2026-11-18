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

    public static func makeWavCodec() -> WavCodec {
        TTKitMakeWavCodec()
    }

    public static func makeFlacCodec() -> FlacCodec {
        TTKitMakeFlacCodec()
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

    public static func wavCodec(from audioCodec: AudioCodec) -> WavCodec {
        var audioCodec = audioCodec
        return TTKitGetWavCodec(&audioCodec)
    }

    public static func flacCodec(from audioCodec: AudioCodec) -> FlacCodec {
        var audioCodec = audioCodec
        return TTKitGetFlacCodec(&audioCodec)
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

    public static func setWavCodec(_ wavCodec: WavCodec, on audioCodec: inout AudioCodec) {
        var wavCodec = wavCodec
        TTKitSetWavCodec(&audioCodec, &wavCodec)
    }

    public static func setFlacCodec(_ flacCodec: FlacCodec, on audioCodec: inout AudioCodec) {
        var flacCodec = flacCodec
        TTKitSetFlacCodec(&audioCodec, &flacCodec)
    }
}
