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

final class AudioCodecViewController: UIHostingController<AudioCodecView> {

    var audiocodec = newAudioCodec(DEFAULT_AUDIOCODEC)
    var opuscodec = newOpusCodec()
    var speexcodec = newSpeexCodec()
    var speexvbrcodec = newSpeexVBRCodec()
    var onAction: ((AudioCodecAction, AudioCodecViewController) -> Void)?

    private var model: AudioCodecModel

    init() {
        let model = AudioCodecModel(
            activeCodec: newAudioCodec(DEFAULT_AUDIOCODEC).nCodec,
            opuscodec: newOpusCodec(),
            speexcodec: newSpeexCodec(),
            speexvbrcodec: newSpeexVBRCodec()
        )
        self.model = model
        super.init(rootView: AudioCodecView(model: model, performAction: { _ in }))
    }

    required init?(coder: NSCoder) { fatalError("use init()") }

    override func viewDidLoad() {
        super.viewDidLoad()
        let model = AudioCodecModel(
            activeCodec: audiocodec.nCodec,
            opuscodec: opuscodec,
            speexcodec: speexcodec,
            speexvbrcodec: speexvbrcodec
        )
        self.model = model
        rootView = AudioCodecView(model: model) { [weak self] action in
            guard let self else { return }
            onAction?(action, self)
        }
    }

    func saveOPUSCodec() {
        model.saveOPUSCodec(to: &opuscodec)
    }

    func saveSpeexCodec() {
        model.saveSpeexCodec(to: &speexcodec)
    }

    func saveSpeexVBRCodec() {
        model.saveSpeexVBRCodec(to: &speexvbrcodec)
    }
}

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
            title = NSLocalizedString("OPUS Codec", comment: "codec detail")
        case SPEEX_CODEC:
            title = NSLocalizedString("Speex Codec", comment: "codec detail")
        case SPEEX_VBR_CODEC:
            title = NSLocalizedString("Speex Variable Bitrate Codec", comment: "codec detail")
        case NO_CODEC:
            title = NSLocalizedString("No Audio", comment: "codec detail")
        default:
            title = ""
        }

        if codec == activeCodec {
            return title + " " + NSLocalizedString("(Active)", comment: "codec detail")
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

struct AudioCodecView: View {
    @ObservedObject var model: AudioCodecModel
    let performAction: (AudioCodecAction) -> Void

    var body: some View {
        Form {
            ForEach(model.sections) { section in
                Section(model.title(for: section.codec)) {
                    rows(for: section.codec)
                }
            }
        }
    }

    @ViewBuilder
    private func rows(for codec: Codec) -> some View {
        switch codec {
        case OPUS_CODEC:
            opusRows
        case SPEEX_CODEC:
            speexRows
        case SPEEX_VBR_CODEC:
            speexVBRRows
        case NO_CODEC:
            TeamTalkActionRow(title: NSLocalizedString("Use No Audio", comment: "codec detail")) {
                performAction(.useNoAudio)
            }
        default:
            EmptyView()
        }
    }

    private var opusRows: some View {
        Group {
            TeamTalkSegmentedRow(
                title: NSLocalizedString("Application", comment: "codec detail"),
                values: [
                    NSLocalizedString("VoIP", comment: "codec detail"),
                    NSLocalizedString("Music", comment: "codec detail")
                ],
                selectedIndex: $model.opusApplicationIndex
            )
            TeamTalkSegmentedRow(
                title: NSLocalizedString("Sample Rate", comment: "codec detail"),
                values: [
                    NSLocalizedString("8 KHz", comment: "codec detail"),
                    NSLocalizedString("12 KHz", comment: "codec detail"),
                    NSLocalizedString("16 KHz", comment: "codec detail"),
                    NSLocalizedString("24 KHz", comment: "codec detail"),
                    NSLocalizedString("48 KHz", comment: "codec detail")
                ],
                selectedIndex: $model.opusSampleRateIndex
            )
            TeamTalkSegmentedRow(
                title: NSLocalizedString("Audio Channels", comment: "codec detail"),
                values: [
                    NSLocalizedString("Mono", comment: "codec detail"),
                    NSLocalizedString("Stereo", comment: "codec detail")
                ],
                selectedIndex: $model.opusChannelsIndex
            )
            TeamTalkSliderRow(
                title: NSLocalizedString("Bitrate", comment: "codec detail"),
                value: $model.opusBitrate,
                range: Double(OPUS_MIN_BITRATE) / 1000.0...Double(OPUS_MAX_BITRATE) / 1000.0,
                step: 1,
                unit: "KB/s"
            )
            TeamTalkToggleRow(
                title: NSLocalizedString("Variable Bitrate", comment: "codec detail"),
                isOn: $model.opusVBR
            )
            TeamTalkToggleRow(
                title: NSLocalizedString("DTX", comment: "codec detail"),
                isOn: $model.opusDTX
            )
            TeamTalkStepperRow(
                title: NSLocalizedString("Frame Size", comment: "codec detail"),
                value: Binding(
                    get: { model.opusFrameSize },
                    set: { model.opusFrameSizeChanged($0) }
                ),
                range: 0...Double(OPUS_REALMAX_FRAMESIZE),
                step: 5,
                unit: "ms"
            )
            TeamTalkStepperRow(
                title: NSLocalizedString("Transmit Interval", comment: "codec detail"),
                value: Binding(
                    get: { model.opusTransmitInterval },
                    set: { model.opusTransmitIntervalChanged($0) }
                ),
                range: 20...500,
                step: 20,
                unit: "ms"
            )
            TeamTalkActionRow(title: NSLocalizedString("Use OPUS Codec", comment: "codec detail")) {
                performAction(.useOPUS)
            }
        }
    }

    private var speexRows: some View {
        Group {
            speexSampleRateRow(selectedIndex: $model.speexSampleRateIndex)
            TeamTalkSliderRow(
                title: NSLocalizedString("Quality", comment: "codec detail"),
                value: $model.speexQuality,
                range: 0...10,
                step: 1
            )
            TeamTalkStepperRow(
                title: NSLocalizedString("Transmit Interval", comment: "codec detail"),
                value: $model.speexTransmitInterval,
                range: 20...500,
                step: 20,
                unit: "ms"
            )
            TeamTalkActionRow(title: NSLocalizedString("Use Speex Codec", comment: "codec detail")) {
                performAction(.useSpeex)
            }
        }
    }

    private var speexVBRRows: some View {
        Group {
            speexSampleRateRow(selectedIndex: $model.speexVBRSampleRateIndex)
            TeamTalkSliderRow(
                title: NSLocalizedString("Quality", comment: "codec detail"),
                value: $model.speexVBRQuality,
                range: 0...10,
                step: 1
            )
            TeamTalkSliderRow(
                title: NSLocalizedString("Bitrate", comment: "codec detail"),
                value: $model.speexVBRBitrate,
                range: 0...Double(SPEEX_UWB_MAX_BITRATE) / 1000.0,
                step: 1,
                unit: "KB/s"
            )
            TeamTalkToggleRow(title: "DTX", isOn: $model.speexVBRDTX)
            TeamTalkStepperRow(
                title: NSLocalizedString("Transmit Interval", comment: "codec detail"),
                value: $model.speexVBRTransmitInterval,
                range: 20...500,
                step: 20,
                unit: "ms"
            )
            TeamTalkActionRow(title: NSLocalizedString("Use Speex VBR Codec", comment: "codec detail")) {
                performAction(.useSpeexVBR)
            }
        }
    }

    private func speexSampleRateRow(selectedIndex: Binding<Int>) -> some View {
        TeamTalkSegmentedRow(
            title: NSLocalizedString("Sample Rate", comment: "codec detail"),
            values: [
                NSLocalizedString("8 KHz", comment: "codec detail"),
                NSLocalizedString("16 KHz", comment: "codec detail"),
                NSLocalizedString("32 KHz", comment: "codec detail")
            ],
            selectedIndex: selectedIndex
        )
    }
}
