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

final class ChannelDetailModel: ObservableObject {
    var channel: Channel
    let isExistingChannel: Bool

    private var cmdid: INT32 = 0

    @Published var nameText: String
    @Published var passwordText: String
    @Published var topicText: String
    @Published var isPermanent: Bool
    @Published var hasNoInterruptions: Bool
    @Published var hasNoVoiceActivation: Bool
    @Published var hasNoAudioRecording: Bool
    @Published var isHidden: Bool
    @Published var codecDescription: String
    @Published var errorMessage: String?
    @Published var shouldDismiss = false
    @Published var showingJoinAlert = false
    @Published var joinPassword = ""
    @Published var audioCodecModel: AudioCodecModel?

    init(channel: Channel) {
        var channel = channel
        if channel.nChannelID == 0 {
            channel.audiocodec = newAudioCodec(DEFAULT_AUDIOCODEC)
        }
        self.channel = channel
        isExistingChannel = channel.nChannelID != 0
        nameText = TeamTalkString.channel(.name, from: channel)
        passwordText = TeamTalkString.channel(.password, from: channel)
        topicText = TeamTalkString.channel(.topic, from: channel)
        isPermanent = (channel.uChannelType & CHANNEL_PERMANENT.rawValue) != 0
        hasNoInterruptions = (channel.uChannelType & CHANNEL_SOLO_TRANSMIT.rawValue) != 0
        hasNoVoiceActivation = (channel.uChannelType & CHANNEL_NO_VOICEACTIVATION.rawValue) != 0
        hasNoAudioRecording = (channel.uChannelType & CHANNEL_NO_RECORDING.rawValue) != 0
        isHidden = (channel.uChannelType & CHANNEL_HIDDEN.rawValue) != 0
        codecDescription = Self.codecDescription(for: channel.audiocodec)
    }

    var navigationTitle: String {
        if !nameText.isEmpty {
            return nameText
        }
        if isExistingChannel {
            return String(localized: "Channel Detail", comment: "View Title")
        }
        return String(localized: "Create Channel", comment: "View Title")
    }

    func refreshCodecDescription(_ codec: AudioCodec) {
        codecDescription = Self.codecDescription(for: codec)
    }

    func createOrUpdate() {
        apply()
        if channel.nChannelID == 0 {
            cmdid = TeamTalkClient.shared.join(channel: &channel)
        } else {
            cmdid = TeamTalkClient.shared.update(channel: &channel)
        }
    }

    func joinChannelPressed() {
        if channel.bPassword == TRUE {
            joinPassword = passwordText
            showingJoinAlert = true
        } else {
            cmdid = TeamTalkClient.shared.joinChannel(id: channel.nChannelID)
        }
    }

    func joinWithPassword() {
        cmdid = TeamTalkClient.shared.joinChannel(id: channel.nChannelID, password: joinPassword)
    }

    func deleteChannel() {
        cmdid = TeamTalkClient.shared.removeChannel(id: channel.nChannelID)
    }

    func makeAudioCodecModel() -> AudioCodecModel {
        var opuscodec = newOpusCodec()
        var speexcodec = newSpeexCodec()
        var speexvbrcodec = newSpeexVBRCodec()
        var activeCodec = channel.audiocodec

        switch channel.audiocodec.nCodec {
        case SPEEX_CODEC:
            speexcodec = TeamTalkAudioCodec.speexCodec(from: channel.audiocodec)
        case SPEEX_VBR_CODEC:
            speexvbrcodec = TeamTalkAudioCodec.speexVBRCodec(from: channel.audiocodec)
        case OPUS_CODEC:
            opuscodec = TeamTalkAudioCodec.opusCodec(from: channel.audiocodec)
        case NO_CODEC:
            if channel.nChannelID == 0 {
                activeCodec.nCodec = OPUS_CODEC
            }
        default:
            activeCodec.nCodec = NO_CODEC
        }

        return AudioCodecModel(
            activeCodec: activeCodec.nCodec,
            opuscodec: opuscodec,
            speexcodec: speexcodec,
            speexvbrcodec: speexvbrcodec
        )
    }

    func applyCodecAction(_ action: AudioCodecAction, codecModel: AudioCodecModel) {
        switch action {
        case .useNoAudio:
            channel.audiocodec.nCodec = NO_CODEC
        case .useOPUS:
            var opuscodec = newOpusCodec()
            codecModel.saveOPUSCodec(to: &opuscodec)
            TeamTalkAudioCodec.setOpusCodec(opuscodec, on: &channel.audiocodec)
        case .useSpeex:
            var speexcodec = newSpeexCodec()
            codecModel.saveSpeexCodec(to: &speexcodec)
            TeamTalkAudioCodec.setSpeexCodec(speexcodec, on: &channel.audiocodec)
        case .useSpeexVBR:
            var speexvbrcodec = newSpeexVBRCodec()
            codecModel.saveSpeexVBRCodec(to: &speexvbrcodec)
            TeamTalkAudioCodec.setSpeexVBRCodec(speexvbrcodec, on: &channel.audiocodec)
        }
        refreshCodecDescription(channel.audiocodec)
    }

    private func apply() {
        let channame = nameText.trimmingCharacters(in: .whitespacesAndNewlines)
        TeamTalkString.setChannel(.name, on: &channel, to: channame)
        TeamTalkString.setChannel(.password, on: &channel, to: passwordText)
        TeamTalkString.setChannel(.topic, on: &channel, to: topicText)

        updateChannelType(&channel, flag: CHANNEL_PERMANENT.rawValue, enabled: isPermanent)
        updateChannelType(&channel, flag: CHANNEL_SOLO_TRANSMIT.rawValue, enabled: hasNoInterruptions)
        updateChannelType(&channel, flag: CHANNEL_NO_VOICEACTIVATION.rawValue, enabled: hasNoVoiceActivation)
        updateChannelType(&channel, flag: CHANNEL_NO_RECORDING.rawValue, enabled: hasNoAudioRecording)
        updateChannelType(&channel, flag: CHANNEL_HIDDEN.rawValue, enabled: isHidden)
    }

    private func updateChannelType(_ channel: inout Channel, flag: UInt32, enabled: Bool) {
        if enabled {
            channel.uChannelType |= flag
        } else {
            channel.uChannelType &= ~flag
        }
    }

    private static func codecDescription(for codec: AudioCodec) -> String {
        switch codec.nCodec {
        case OPUS_CODEC:
            let opus = TeamTalkAudioCodec.opusCodec(from: codec)
            let chans = opus.nChannels > 1 ? String(localized: "Stereo", comment: "create channel") : String(localized: "Mono", comment: "create channel")
            return "OPUS \(opus.nSampleRate / 1000) KHz \(opus.nBitRate / 1000) KB/s " + chans
        case SPEEX_CODEC:
            let speex = TeamTalkAudioCodec.speexCodec(from: codec)
            return "Speex " + bandmodeString(speex.nBandmode)
        case SPEEX_VBR_CODEC:
            let speexvbr = TeamTalkAudioCodec.speexVBRCodec(from: codec)
            return "Speex VBR " + bandmodeString(speexvbr.nBandmode)
        case NO_CODEC:
            fallthrough
        default:
            return String(localized: "No Audio", comment: "create channel")
        }
    }

    private static func bandmodeString(_ bandmode: INT32) -> String {
        switch bandmode {
        case 2:
            return String(localized: "32 KHz", comment: "create channel")
        case 1:
            return String(localized: "16 KHz", comment: "create channel")
        default:
            return String(localized: "8 KHz", comment: "create channel")
        }
    }
}

extension ChannelDetailModel: Identifiable {
    var id: ObjectIdentifier { ObjectIdentifier(self) }
}

extension ChannelDetailModel: TeamTalkEvent {
    func handleTTMessage(_ m: TTMessage) {
        switch m.nClientEvent {
        case CLIENTEVENT_CMD_SUCCESS:
            if m.nSource == cmdid {
                shouldDismiss = true
            }
        case CLIENTEVENT_CMD_ERROR:
            if m.nSource == cmdid {
                errorMessage = TeamTalkString.clientError(TeamTalkMessagePayload.clientError(from: m))
            }
        case CLIENTEVENT_CMD_PROCESSING:
            if !TeamTalkMessagePayload.isActive(m) && cmdid == m.nSource {
                cmdid = 0
            }
        default:
            break
        }
    }
}
