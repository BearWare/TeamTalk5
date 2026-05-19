/*
 * Copyright (c) 2005-2018, BearWare.dk
 *
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

import SwiftUI
import TeamTalkKit

enum AudioCodecAction {
    case useNoAudio
    case useOPUS
    case useSpeex
    case useSpeexVBR
}

final class AudioCodecModel: ObservableObject {

    struct Section: Identifiable {
        let codec: Codec

        var id: String {
            switch codec {
            case OPUS_CODEC:
                return "opus"
            case SPEEX_CODEC:
                return "speex"
            case SPEEX_VBR_CODEC:
                return "speex-vbr"
            case NO_CODEC:
                return "no-audio"
            default:
                return "unknown"
            }
        }
    }

    let activeCodec: Codec
    let sections: [Section]

    private let opusApplications: [Int32] = [OPUS_APPLICATION_VOIP, OPUS_APPLICATION_AUDIO]
    private let opusSampleRates: [Int32] = [8000, 12000, 16000, 24000, 48000]
    private let speexBandmodes: [INT32] = [0, 1, 2]

    @Published var opusApplicationIndex: Int
    @Published var opusSampleRateIndex: Int
    @Published var opusChannelsIndex: Int
    @Published var opusBitrate: Double
    @Published var opusVBR: Bool
    @Published var opusDTX: Bool
    @Published var opusFrameSize: Double
    @Published var opusTransmitInterval: Double

    @Published var speexSampleRateIndex: Int
    @Published var speexQuality: Double
    @Published var speexTransmitInterval: Double

    @Published var speexVBRSampleRateIndex: Int
    @Published var speexVBRQuality: Double
    @Published var speexVBRBitrate: Double
    @Published var speexVBRDTX: Bool
    @Published var speexVBRTransmitInterval: Double

    init(activeCodec: Codec, opuscodec: OpusCodec, speexcodec: SpeexCodec, speexvbrcodec: SpeexVBRCodec) {
        self.activeCodec = activeCodec

        switch activeCodec {
        case OPUS_CODEC:
            sections = [OPUS_CODEC, SPEEX_CODEC, SPEEX_VBR_CODEC, NO_CODEC].map(Section.init)
        case SPEEX_CODEC:
            sections = [SPEEX_CODEC, OPUS_CODEC, SPEEX_VBR_CODEC, NO_CODEC].map(Section.init)
        case SPEEX_VBR_CODEC:
            sections = [SPEEX_VBR_CODEC, OPUS_CODEC, SPEEX_CODEC, NO_CODEC].map(Section.init)
        case NO_CODEC:
            fallthrough
        default:
            sections = [NO_CODEC, OPUS_CODEC, SPEEX_CODEC, SPEEX_VBR_CODEC].map(Section.init)
        }

        opusApplicationIndex = opusApplications.firstIndex(of: opuscodec.nApplication) ?? 0
        opusSampleRateIndex = opusSampleRates.firstIndex(of: opuscodec.nSampleRate) ?? 4
        opusChannelsIndex = opuscodec.nChannels == 2 ? 1 : 0
        let bitrate = within(OPUS_MIN_BITRATE, max_v: OPUS_MAX_BITRATE, value: opuscodec.nBitRate)
        opusBitrate = Double(bitrate) / 1000.0
        opusVBR = opuscodec.bVBR == TRUE
        opusDTX = opuscodec.bDTX != 0
        opusFrameSize = Double(opuscodec.nFrameSizeMSec)
        opusTransmitInterval = Double(opuscodec.nTxIntervalMSec)

        speexSampleRateIndex = Int(speexcodec.nBandmode)
        speexQuality = Double(speexcodec.nQuality)
        speexTransmitInterval = Double(speexcodec.nTxIntervalMSec)

        speexVBRSampleRateIndex = Int(speexvbrcodec.nBandmode)
        speexVBRQuality = Double(speexvbrcodec.nQuality)
        speexVBRBitrate = Double(speexvbrcodec.nMaxBitRate) / 1000.0
        speexVBRDTX = speexvbrcodec.bDTX != 0
        speexVBRTransmitInterval = Double(speexvbrcodec.nTxIntervalMSec)
    }

    func saveOPUSCodec(to opuscodec: inout OpusCodec) {
        opuscodec.nApplication = opusApplications[opusApplicationIndex]
        opuscodec.nBitRate = INT32(opusBitrate) * 1000
        opuscodec.nSampleRate = opusSampleRates[opusSampleRateIndex]
        opuscodec.nChannels = INT32(opusChannelsIndex + 1)
        opuscodec.nTxIntervalMSec = INT32(opusTransmitInterval)
        opuscodec.bDTX = opusDTX ? TRUE : FALSE
        opuscodec.bVBR = opusVBR ? TRUE : FALSE
        opuscodec.nFrameSizeMSec = INT32(opusFrameSize)
    }

    func saveSpeexCodec(to speexcodec: inout SpeexCodec) {
        speexcodec.nBandmode = speexBandmodes[speexSampleRateIndex]
        speexcodec.nQuality = INT32(speexQuality)
        speexcodec.nTxIntervalMSec = INT32(speexTransmitInterval)
    }

    func saveSpeexVBRCodec(to speexvbrcodec: inout SpeexVBRCodec) {
        speexvbrcodec.nBandmode = speexBandmodes[speexVBRSampleRateIndex]
        speexvbrcodec.nQuality = INT32(speexVBRQuality)
        speexvbrcodec.nMaxBitRate = INT32(speexVBRBitrate * 1000)
        speexvbrcodec.bDTX = speexVBRDTX ? 1 : 0
        speexvbrcodec.nTxIntervalMSec = INT32(speexVBRTransmitInterval)
    }

    func title(for codec: Codec) -> String {
        let title: String
        switch codec {
        case OPUS_CODEC:
            title = String(localized: "OPUS Codec", comment: "codec detail")
        case SPEEX_CODEC:
            title = String(localized: "Speex Codec", comment: "codec detail")
        case SPEEX_VBR_CODEC:
            title = String(localized: "Speex Variable Bitrate Codec", comment: "codec detail")
        case NO_CODEC:
            title = String(localized: "No Audio", comment: "codec detail")
        default:
            title = ""
        }

        if codec == activeCodec {
            return title + " " + String(localized: "(Active)", comment: "codec detail")
        }
        return title
    }

    func opusFrameSizeChanged(_ value: Double) {
        opusFrameSize = value
    }

    func opusTransmitIntervalChanged(_ value: Double) {
        opusTransmitInterval = value
        if opusFrameSize == 0 && INT32(value) > OPUS_REALMAX_FRAMESIZE {
            opusFrameSize = Double(OPUS_REALMAX_FRAMESIZE)
        } else if opusFrameSize >= value {
            opusFrameSize = 0
        }
    }
}
