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
    case useWAV
    case useFLAC
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
            case WAV_CODEC:
                return "wav"
            case FLAC_CODEC:
                return "flac"
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
    let wavSampleRates: [INT32] = [8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000,
                                   88200, 96000, 176400, 192000, 352800, 384000]
    private let wavSampleFormats: [WavSampleFormat] = [WAV_SAMPLEFORMAT_UINT8, WAV_SAMPLEFORMAT_INT16]
    let flacCompressionLevels: [INT32] = Array(FLAC_MIN_COMPRESSIONLEVEL...FLAC_MAX_COMPRESSIONLEVEL)

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

    @Published var wavSampleRateIndex: Int {
        didSet { clampWavTransmitInterval() }
    }
    @Published var wavChannelsIndex: Int {
        didSet { clampWavTransmitInterval() }
    }
    @Published var wavSampleFormatIndex: Int {
        didSet { clampWavTransmitInterval() }
    }
    @Published var wavTransmitInterval: Double

    @Published var flacSampleRateIndex: Int {
        didSet { clampFlacTransmitInterval() }
    }
    @Published var flacChannelsIndex: Int {
        didSet { clampFlacTransmitInterval() }
    }
    @Published var flacCompressionLevelIndex: Int
    @Published var flacTransmitInterval: Double

    init(activeCodec: Codec, opuscodec: OpusCodec, speexcodec: SpeexCodec, speexvbrcodec: SpeexVBRCodec, wavcodec: WavCodec, flaccodec: FlacCodec) {
        self.activeCodec = activeCodec

        switch activeCodec {
        case OPUS_CODEC:
            sections = [OPUS_CODEC, SPEEX_CODEC, SPEEX_VBR_CODEC, WAV_CODEC, FLAC_CODEC, NO_CODEC].map(Section.init)
        case SPEEX_CODEC:
            sections = [SPEEX_CODEC, OPUS_CODEC, SPEEX_VBR_CODEC, WAV_CODEC, FLAC_CODEC, NO_CODEC].map(Section.init)
        case SPEEX_VBR_CODEC:
            sections = [SPEEX_VBR_CODEC, OPUS_CODEC, SPEEX_CODEC, WAV_CODEC, FLAC_CODEC, NO_CODEC].map(Section.init)
        case WAV_CODEC:
            sections = [WAV_CODEC, OPUS_CODEC, SPEEX_CODEC, SPEEX_VBR_CODEC, FLAC_CODEC, NO_CODEC].map(Section.init)
        case FLAC_CODEC:
            sections = [FLAC_CODEC, OPUS_CODEC, SPEEX_CODEC, SPEEX_VBR_CODEC, WAV_CODEC, NO_CODEC].map(Section.init)
        case NO_CODEC:
            fallthrough
        default:
            sections = [NO_CODEC, OPUS_CODEC, SPEEX_CODEC, SPEEX_VBR_CODEC, WAV_CODEC, FLAC_CODEC].map(Section.init)
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

        wavSampleRateIndex = wavSampleRates.firstIndex(of: wavcodec.nSampleRate) ?? 8
        wavChannelsIndex = wavcodec.nChannels == 2 ? 1 : 0
        wavSampleFormatIndex = wavSampleFormats.firstIndex(of: wavcodec.nSampleFormat) ?? 1
        wavTransmitInterval = Double(wavcodec.nTxIntervalMSec)

        flacSampleRateIndex = wavSampleRates.firstIndex(of: flaccodec.nSampleRate) ?? 8
        flacChannelsIndex = flaccodec.nChannels == 2 ? 1 : 0
        flacCompressionLevelIndex = flacCompressionLevels.firstIndex(of: flaccodec.nCompressionLevel) ?? 5
        flacTransmitInterval = Double(flaccodec.nTxIntervalMSec)
    }

    var wavBytesPerSecond: Int {
        let samplebytes = wavSampleFormats[wavSampleFormatIndex] == WAV_SAMPLEFORMAT_UINT8 ? 1 : 2
        return Int(wavSampleRates[wavSampleRateIndex]) * (wavChannelsIndex + 1) * samplebytes
    }

    var wavBitrate: Int {
        wavBytesPerSecond * 8 / 1000
    }

    var wavMaxTransmitInterval: Double {
        Double(min(1000, Int(WAV_MAX_PACKETSIZE) * 1000 / wavBytesPerSecond))
    }

    private func clampWavTransmitInterval() {
        if wavTransmitInterval > wavMaxTransmitInterval {
            wavTransmitInterval = wavMaxTransmitInterval
        }
    }

    var flacBytesPerSecond: Int {
        Int(wavSampleRates[flacSampleRateIndex]) * (flacChannelsIndex + 1) * 2
    }

    var flacMaxBitrate: Int {
        flacBytesPerSecond * 8 / 1000
    }

    var flacMaxTransmitInterval: Double {
        Double(min(1000, (Int(FLAC_MAX_PACKETSIZE) - Int(FLAC_MAX_PACKETOVERHEAD)) * 1000 / flacBytesPerSecond))
    }

    private func clampFlacTransmitInterval() {
        if flacTransmitInterval > flacMaxTransmitInterval {
            flacTransmitInterval = flacMaxTransmitInterval
        }
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

    func saveWavCodec(to wavcodec: inout WavCodec) {
        wavcodec.nSampleRate = wavSampleRates[wavSampleRateIndex]
        wavcodec.nChannels = INT32(wavChannelsIndex + 1)
        wavcodec.nSampleFormat = wavSampleFormats[wavSampleFormatIndex]
        wavcodec.nTxIntervalMSec = INT32(min(wavTransmitInterval, wavMaxTransmitInterval))
    }

    func saveFlacCodec(to flaccodec: inout FlacCodec) {
        flaccodec.nSampleRate = wavSampleRates[flacSampleRateIndex]
        flaccodec.nChannels = INT32(flacChannelsIndex + 1)
        flaccodec.nCompressionLevel = flacCompressionLevels[flacCompressionLevelIndex]
        flaccodec.nTxIntervalMSec = INT32(min(flacTransmitInterval, flacMaxTransmitInterval))
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
        case WAV_CODEC:
            title = String(localized: "WAV Codec", comment: "codec detail")
        case FLAC_CODEC:
            title = String(localized: "FLAC Codec", comment: "codec detail")
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
