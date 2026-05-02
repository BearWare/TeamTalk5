import Foundation
import TeamTalkC

public enum TeamTalkAudioPreprocessor {
    public static let channelAudioConfigMax = Int32(32000)

    public static func makeTeamTalkPreprocessor() -> AudioPreprocessor {
        makeAudioPreprocessor(TEAMTALK_AUDIOPREPROCESSOR)
    }

    public static func makeWebRTCPreprocessor() -> AudioPreprocessor {
        makeAudioPreprocessor(WEBRTC_AUDIOPREPROCESSOR)
    }

    public static func makeAudioPreprocessor(_ preprocessor: AudioPreprocessorType) -> AudioPreprocessor {
        var audioPreprocessor = AudioPreprocessor()
        audioPreprocessor.nPreprocessor = preprocessor

        switch preprocessor {
        case SPEEXDSP_AUDIOPREPROCESSOR:
            audioPreprocessor.speexdsp.bEnableAGC = 0
            audioPreprocessor.speexdsp.nGainLevel = 8000
            audioPreprocessor.speexdsp.nMaxIncDBSec = -12
            audioPreprocessor.speexdsp.nMaxDecDBSec = -40
            audioPreprocessor.speexdsp.nMaxGainDB = 30
            audioPreprocessor.speexdsp.bEnableDenoise = 0
            audioPreprocessor.speexdsp.nMaxNoiseSuppressDB = -30
            audioPreprocessor.speexdsp.bEnableEchoCancellation = 0
            audioPreprocessor.speexdsp.nEchoSuppress = -40
            audioPreprocessor.speexdsp.nEchoSuppressActive = -15
        case TEAMTALK_AUDIOPREPROCESSOR:
            audioPreprocessor.ttpreprocessor.nGainLevel = Int32(SOUND_GAIN_DEFAULT.rawValue)
            audioPreprocessor.ttpreprocessor.bMuteLeftSpeaker = 0
            audioPreprocessor.ttpreprocessor.bMuteRightSpeaker = 0
        case WEBRTC_AUDIOPREPROCESSOR:
            audioPreprocessor.webrtc.preamplifier.bEnable = 0
            audioPreprocessor.webrtc.preamplifier.fFixedGainFactor = 1
            audioPreprocessor.webrtc.echocanceller.bEnable = 0
            audioPreprocessor.webrtc.noisesuppression.bEnable = 0
            audioPreprocessor.webrtc.noisesuppression.nLevel = 2
            audioPreprocessor.webrtc.gaincontroller2.bEnable = 0
            audioPreprocessor.webrtc.gaincontroller2.fixeddigital.fGainDB = 15
            audioPreprocessor.webrtc.gaincontroller2.adaptivedigital.bEnable = 1
            audioPreprocessor.webrtc.gaincontroller2.adaptivedigital.fHeadRoomDB = 5
            audioPreprocessor.webrtc.gaincontroller2.adaptivedigital.fMaxGainDB = 50
            audioPreprocessor.webrtc.gaincontroller2.adaptivedigital.fInitialGainDB = 15
            audioPreprocessor.webrtc.gaincontroller2.adaptivedigital.fMaxGainChangeDBPerSecond = 6
            audioPreprocessor.webrtc.gaincontroller2.adaptivedigital.fMaxOutputNoiseLevelDBFS = -50
        case NO_AUDIOPREPROCESSOR:
            fallthrough
        default:
            break
        }

        return audioPreprocessor
    }
}
