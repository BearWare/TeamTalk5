//
//  UtilTT.swift
//  iTeamTalk
//
//  Created by Bjørn Damstedt Rasmussen on 10/01/2021.
//  Copyright © 2021 BearWare.dk. All rights reserved.
//

import Foundation

protocol TeamTalkEvent : AnyObject {
    func handleTTMessage(_ m: TTMessage)
}

class TeamTalkEventHandler {
    weak var value : TeamTalkEvent?
    init (value: TeamTalkEvent) {
        self.value = value
    }
}

var ttMessageHandlers = [TeamTalkEventHandler]()

func addToTTMessages(_ p: TeamTalkEvent) {
    
    for m in ttMessageHandlers {
        if m.value === p {
            return
        }
    }
    
    let new = TeamTalkEventHandler(value: p)
    
    ttMessageHandlers.append(new)
}

func removeFromTTMessages(_ p: TeamTalkEventHandler) {

    var i = 0
    
    for m in ttMessageHandlers {
        if m === p {
            ttMessageHandlers.remove(at: i)
            break
        }
        else {
            i += 1
        }
    }
}

func runTeamTalkEventHandler() {
    var m = TTMessage()
    var n : INT32 = 0
    while TT_GetMessage(ttInst, &m, &n) != FALSE {

        for tt in ttMessageHandlers {
            if tt.value == nil {
                removeFromTTMessages(tt)
            }
            else {
                tt.value!.handleTTMessage(m)
            }
        }
    }
}

func setupEncryption(_ ttInst: UnsafeMutableRawPointer, server: Server) -> Bool {
    if server.encrypted == false {
        return true
    }
    
    do {
        var encryption = EncryptionContext()
        let itemReplaceDir = try FileManager.default.url(for: .autosavedInformationDirectory, in: .userDomainMask, appropriateFor: nil, create: true)

        let cacertPath = itemReplaceDir.appendingPathComponent("ca_cert.pem")
        defer {
            do {
                try FileManager.default.removeItem(at: cacertPath)
            } catch {}
        }
        if server.cacertdata.isEmpty == false {
            try server.cacertdata.write(to: cacertPath, atomically: true, encoding: .utf8)
            toTTString(cacertPath.path, dst: &encryption.szCAFile)
        }

        let certPath = itemReplaceDir.appendingPathComponent("cert.pem")
        defer {
            do {
                try FileManager.default.removeItem(at: certPath)
            } catch{}
        }
        if server.certdata.isEmpty == false {
            try server.certdata.write(to: certPath, atomically: true, encoding: .utf8)
            toTTString(certPath.path, dst: &encryption.szCertificateFile)
        }

        let keyPath = itemReplaceDir.appendingPathComponent("key.pem")
        defer {
            do {
                try FileManager.default.removeItem(at: keyPath)
            } catch {}
        }
        if server.certprivkeydata.isEmpty == false {
            try server.certprivkeydata.write(to: keyPath, atomically: true, encoding: .utf8)
            toTTString(keyPath.path, dst: &encryption.szPrivateKeyFile)
        }
        
        encryption.bVerifyPeer = server.certverifypeer ? TRUE : FALSE
        encryption.nVerifyDepth = encryption.bVerifyPeer == TRUE ? 0 : -1

        let result = TT_SetEncryptionContext(ttInst, &encryption) == TRUE
        if result {
           print("Encryption activated")
        }
        else {
            print("Failed to set encryption")
        }
        return result
    } catch {
        print("Exception thrown trying to create directory")
        return false
    }
}

func isTransmitting(_ ttInst: UnsafeMutableRawPointer, stream: StreamType) -> Bool {
    let flags = TT_GetFlags(ttInst)
    
    switch stream {
    case STREAMTYPE_VOICE:
        if flags & CLIENT_TX_VOICE.rawValue != 0 {
            return true
        }
        if flags & CLIENT_SNDINPUT_VOICEACTIVATED.rawValue != 0 &&
            flags & CLIENT_SNDINPUT_VOICEACTIVE.rawValue != 0 {
                return true
        }
    default : break
    }
    return false
}

func getChannel(_ chan: Channel, strprop : CExt) -> String {
    return String(cString: withUnsafePointer(to: chan) { getChannelString(strprop, $0) })
}

func getUser(_ user: User, strprop : UExt) -> String {
    return String(cString: withUnsafePointer(to: user) { getUserString(strprop, $0) })
}

func getClientErrorMsg(_ clienterr: ClientErrorMsg, strprop: CEExt) -> String {
    return String(cString: withUnsafePointer(to: clienterr) { getClientErrorMsgString(strprop, $0) })
}

let TRUE : TTBOOL = 1
let FALSE : TTBOOL = 0

let DEFAULT_AUDIOCODEC = OPUS_CODEC
let DEFAULT_AGC_ENABLE = FALSE
let DEFAULT_DENOISE_ENABLE = FALSE

let CHANNEL_AUDIOCONFIG_MAX = INT32(32000)
let DEFAULT_CHANNEL_AUDIOCONFIG_ENABLE = FALSE
let DEFAULT_CHANNEL_AUDIOCONFIG_LEVEL = INT32(9600) /* CHANNEL_AUDIOCONFIG_MAX * (DEFAULT_WEBRTC_GAINDB / WEBRTC_GAINCONTROLLER2_FIXEDGAIN_MAX) */

let DEFAULT_TEAMTALK_GAINLEVEL = INT32(SOUND_GAIN_DEFAULT.rawValue)
let DEFAULT_TEAMTALK_MUTELEFT = FALSE
let DEFAULT_TEAMTALK_MUTERIGHT = FALSE

let DEFAULT_SPEEXDSP_AGC_ENABLE = DEFAULT_AGC_ENABLE
let DEFAULT_SPEEXDSP_AGC_GAINLEVEL = INT32(8000)
let DEFAULT_SPEEXDSP_AGC_INC_MAXDB = INT32(-12)
let DEFAULT_SPEEXDSP_AGC_DEC_MAXDB = INT32(-40)
let DEFAULT_SPEEXDSP_AGC_GAINMAXDB = INT32(30)
let DEFAULT_SPEEXDSP_DENOISE_ENABLE = DEFAULT_DENOISE_ENABLE
let DEFAULT_SPEEXDSP_DENOISE_SUPPRESS = INT32(-30)
let DEFAULT_SPEEXDSP_ECHO_ENABLE = FALSE /* requires duplex mode */
let DEFAULT_SPEEXDSP_ECHO_SUPPRESS = INT32(-40)
let DEFAULT_SPEEXDSP_ECHO_SUPPRESSACTIVE = INT32(-15)

let DEFAULT_WEBRTC_PREAMPLIFIER_ENABLE = FALSE
let DEFAULT_WEBRTC_PREAMPLIFIER_GAINFACTOR = Float(1)
let DEFAULT_WEBRTC_GAINCTL_ENABLE = DEFAULT_AGC_ENABLE
let DEFAULT_WEBRTC_GAINDB = Float(15)
let DEFAULT_WEBRTC_SAT_PROT_ENABLE = TRUE
let DEFAULT_WEBRTC_HEADROOM_DB = Float(5)
let DEFAULT_WEBRTC_MAXGAIN_DB = Float(50)
let DEFAULT_WEBRTC_INITIAL_GAIN_DB = Float(15)
let DEFAULT_WEBRTC_MAXGAIN_DBSEC = Float(6)
let DEFAULT_WEBRTC_MAX_OUT_NOISE = Float(-50)
let DEFAULT_WEBRTC_NOISESUPPRESS_ENABLE = DEFAULT_DENOISE_ENABLE
let DEFAULT_WEBRTC_NOISESUPPRESS_LEVEL = INT32(2)
let DEFAULT_WEBRTC_ECHO_CANCEL_ENABLE = FALSE /* requires duplex mode */

let DEFAULT_MSEC_PER_PACKET = INT32(40)

//Default OPUS codec settings
let DEFAULT_OPUS_SAMPLERATE = INT32(48000)
let DEFAULT_OPUS_CHANNELS = INT32(1)
let DEFAULT_OPUS_APPLICATION = INT32(OPUS_APPLICATION_VOIP)
let DEFAULT_OPUS_COMPLEXITY = INT32(10)
let DEFAULT_OPUS_FEC = TRUE
let DEFAULT_OPUS_DTX = FALSE
let DEFAULT_OPUS_VBR = TRUE
let DEFAULT_OPUS_VBRCONSTRAINT = FALSE
let DEFAULT_OPUS_BITRATE = INT32(32000)
let DEFAULT_OPUS_DELAY = INT32(DEFAULT_MSEC_PER_PACKET)
let DEFAULT_OPUS_FRAMESIZE = INT32(0) // implies same as DEFAULT_OPUS_DELAY

//Default Speex codec settings
let DEFAULT_SPEEX_BANDMODE = INT32(1)
let DEFAULT_SPEEX_QUALITY = INT32(4)
let DEFAULT_SPEEX_DELAY = INT32(DEFAULT_MSEC_PER_PACKET)
let DEFAULT_SPEEX_SIMSTEREO = FALSE

//Default Speex VBR codec settings
let DEFAULT_SPEEX_VBR_BANDMODE : INT32 = 1
let DEFAULT_SPEEX_VBR_QUALITY : INT32 = 4
let DEFAULT_SPEEX_VBR_BITRATE : INT32 = 0
let DEFAULT_SPEEX_VBR_MAXBITRATE : INT32 = 0
let DEFAULT_SPEEX_VBR_DTX = TRUE
let DEFAULT_SPEEX_VBR_DELAY : INT32 = DEFAULT_MSEC_PER_PACKET
let DEFAULT_SPEEX_VBR_SIMSTEREO = FALSE

func newAudioCodec(_ codec: Codec) -> iTeamTalk.AudioCodec {
    var audiocodec = AudioCodec()
    
    switch codec {
    case OPUS_CODEC :
        var opus = newOpusCodec()
        setOpusCodec(&audiocodec, &opus)
    case SPEEX_CODEC :
        var speex = newSpeexCodec()
        setSpeexCodec(&audiocodec, &speex)
    case SPEEX_VBR_CODEC :
        var speexvbr = newSpeexVBRCodec()
        setSpeexVBRCodec(&audiocodec, &speexvbr)
    case NO_CODEC :
        fallthrough
    default :
        break
    }
    return audiocodec
}

func newOpusCodec() -> OpusCodec {
    return OpusCodec(nSampleRate: DEFAULT_OPUS_SAMPLERATE,
        nChannels: DEFAULT_OPUS_CHANNELS, nApplication: DEFAULT_OPUS_APPLICATION,
        nComplexity: DEFAULT_OPUS_COMPLEXITY, bFEC: DEFAULT_OPUS_FEC,
        bDTX: DEFAULT_OPUS_DTX, nBitRate: DEFAULT_OPUS_BITRATE,
        bVBR: DEFAULT_OPUS_VBR, bVBRConstraint: DEFAULT_OPUS_VBRCONSTRAINT,
        nTxIntervalMSec: DEFAULT_MSEC_PER_PACKET, nFrameSizeMSec: DEFAULT_OPUS_FRAMESIZE)
}

func newSpeexCodec() -> SpeexCodec {
    return SpeexCodec(nBandmode: DEFAULT_SPEEX_BANDMODE, nQuality: DEFAULT_SPEEX_QUALITY, nTxIntervalMSec: DEFAULT_SPEEX_DELAY, bStereoPlayback: DEFAULT_SPEEX_SIMSTEREO)
}

func newSpeexVBRCodec() -> SpeexVBRCodec {
    return SpeexVBRCodec(nBandmode: DEFAULT_SPEEX_VBR_BANDMODE,
        nQuality: DEFAULT_SPEEX_VBR_QUALITY,
        nBitRate: DEFAULT_SPEEX_VBR_BITRATE,
        nMaxBitRate: DEFAULT_SPEEX_VBR_MAXBITRATE,
        bDTX: DEFAULT_SPEEX_VBR_DTX,
        nTxIntervalMSec: DEFAULT_SPEEX_VBR_DELAY,
        bStereoPlayback: DEFAULT_SPEEX_VBR_SIMSTEREO)
}

func newAudioPreprocessor(preprocessor: AudioPreprocessorType) -> AudioPreprocessor {
    
    var ap = AudioPreprocessor()
    ap.nPreprocessor = preprocessor

    switch preprocessor {
    case SPEEXDSP_AUDIOPREPROCESSOR :
        ap.speexdsp.bEnableAGC = DEFAULT_SPEEXDSP_AGC_ENABLE
        ap.speexdsp.nGainLevel = DEFAULT_SPEEXDSP_AGC_GAINLEVEL
        ap.speexdsp.nMaxIncDBSec = DEFAULT_SPEEXDSP_AGC_INC_MAXDB
        ap.speexdsp.nMaxDecDBSec = DEFAULT_SPEEXDSP_AGC_DEC_MAXDB
        ap.speexdsp.nMaxGainDB = DEFAULT_SPEEXDSP_AGC_GAINMAXDB
        ap.speexdsp.bEnableDenoise = DEFAULT_SPEEXDSP_DENOISE_ENABLE
        ap.speexdsp.nMaxNoiseSuppressDB = DEFAULT_SPEEXDSP_DENOISE_SUPPRESS
        ap.speexdsp.bEnableEchoCancellation = DEFAULT_SPEEXDSP_ECHO_ENABLE
        ap.speexdsp.nEchoSuppress = DEFAULT_SPEEXDSP_ECHO_SUPPRESS
        ap.speexdsp.nEchoSuppressActive = DEFAULT_SPEEXDSP_ECHO_SUPPRESSACTIVE
    case TEAMTALK_AUDIOPREPROCESSOR :
        ap.ttpreprocessor.nGainLevel = DEFAULT_TEAMTALK_GAINLEVEL
        ap.ttpreprocessor.bMuteLeftSpeaker = DEFAULT_TEAMTALK_MUTELEFT
        ap.ttpreprocessor.bMuteRightSpeaker = DEFAULT_TEAMTALK_MUTERIGHT
    case WEBRTC_AUDIOPREPROCESSOR :
        ap.webrtc.preamplifier.bEnable = DEFAULT_WEBRTC_PREAMPLIFIER_ENABLE
        ap.webrtc.preamplifier.fFixedGainFactor = DEFAULT_WEBRTC_PREAMPLIFIER_GAINFACTOR
        ap.webrtc.echocanceller.bEnable = DEFAULT_WEBRTC_ECHO_CANCEL_ENABLE
        ap.webrtc.noisesuppression.bEnable = DEFAULT_WEBRTC_NOISESUPPRESS_ENABLE
        ap.webrtc.noisesuppression.nLevel = DEFAULT_WEBRTC_NOISESUPPRESS_LEVEL
        ap.webrtc.gaincontroller2.bEnable = DEFAULT_WEBRTC_GAINCTL_ENABLE
        ap.webrtc.gaincontroller2.fixeddigital.fGainDB = DEFAULT_WEBRTC_GAINDB
        ap.webrtc.gaincontroller2.adaptivedigital.bEnable = DEFAULT_WEBRTC_SAT_PROT_ENABLE
        ap.webrtc.gaincontroller2.adaptivedigital.fHeadRoomDB = DEFAULT_WEBRTC_HEADROOM_DB
        ap.webrtc.gaincontroller2.adaptivedigital.fMaxGainDB = DEFAULT_WEBRTC_MAXGAIN_DB
        ap.webrtc.gaincontroller2.adaptivedigital.fInitialGainDB = DEFAULT_WEBRTC_INITIAL_GAIN_DB
        ap.webrtc.gaincontroller2.adaptivedigital.fMaxGainChangeDBPerSecond = DEFAULT_WEBRTC_MAXGAIN_DBSEC
        ap.webrtc.gaincontroller2.adaptivedigital.fMaxOutputNoiseLevelDBFS = DEFAULT_WEBRTC_MAX_OUT_NOISE
    case NO_AUDIOPREPROCESSOR :
        fallthrough
    default :
        break
    }

    return ap
}

//TODO: someone find a way out of this madness
func fromTTString(_ cs: (TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR)) -> String {
    var str = cs
    return withUnsafePointer(to: &str) {
        $0.withMemoryRebound(to: UInt8.self, capacity: MemoryLayout.size(ofValue: cs)) {
            String(cString: $0)
        }
    }
}

func toTTString(_ src: String, dst: inout (TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR, TTCHAR)) {

    var c = StringWrap()
    convertTuple(src, &c)
    dst = c.buf
}
