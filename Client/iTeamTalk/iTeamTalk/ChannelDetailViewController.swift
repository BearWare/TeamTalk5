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
import UIKit

final class ChannelDetailViewController: UIHostingController<ChannelDetailView>, TeamTalkEvent {

    var channel = Channel()
    var cmdid: INT32 = 0

    private var model: ChannelDetailModel

    init() {
        let model = ChannelDetailModel(channel: Channel())
        self.model = model
        super.init(rootView: ChannelDetailView(model: model, setupCodec: { }, joinChannel: { }, deleteChannel: { }))
    }

    required init?(coder: NSCoder) { fatalError("use init()") }

    override func viewDidLoad() {
        super.viewDidLoad()

        if channel.nChannelID == 0 {
            channel.audiocodec = newAudioCodec(DEFAULT_AUDIOCODEC)
        }

        let model = ChannelDetailModel(channel: channel)
        self.model = model
        navigationItem.title = model.navigationTitle
        navigationItem.rightBarButtonItem = UIBarButtonItem(
            barButtonSystemItem: .save,
            target: self,
            action: #selector(createChannel(_:))
        )

        rootView = ChannelDetailView(
            model: model,
            setupCodec: { [weak self] in self?.pushAudioCodecViewController() },
            joinChannel: { [weak self] in self?.joinChannelPressed() },
            deleteChannel: { [weak self] in self?.deleteChannelPressed() }
        )
        addToTTMessages(self)
    }

    @objc func createChannel(_ sender: UIBarButtonItem) {
        saveChannelDetail()

        if channel.nChannelID == 0 {
            cmdid = TeamTalkClient.shared.join(channel: &channel)
        } else {
            cmdid = TeamTalkClient.shared.update(channel: &channel)
        }
    }

    private func joinChannelPressed() {
        if channel.bPassword == TRUE {
            showJoinChannelAlert()
        } else {
            cmdid = TeamTalkClient.shared.joinChannel(id: channel.nChannelID)
        }
    }

    private func showJoinChannelAlert() {
        let alert = UIAlertController(
            title: NSLocalizedString("Enter Password", comment: "Dialog message"),
            message: NSLocalizedString("Password", comment: "Dialog message"),
            preferredStyle: .alert
        )
        alert.addTextField { [weak self] textField in
            textField.isSecureTextEntry = true
            textField.text = self?.model.passwordText
        }
        alert.addAction(UIAlertAction(title: NSLocalizedString("Join", comment: "Dialog message"), style: .default) { [weak self, weak alert] _ in
            guard let self else { return }
            let passwd = alert?.textFields?.first?.text ?? ""
            cmdid = TeamTalkClient.shared.joinChannel(id: channel.nChannelID, password: passwd)
        })
        present(alert, animated: true)
    }

    private func deleteChannelPressed() {
        cmdid = TeamTalkClient.shared.removeChannel(id: channel.nChannelID)
    }

    func handleTTMessage(_ m: TTMessage) {
        switch m.nClientEvent {
        case CLIENTEVENT_CMD_SUCCESS:
            if m.nSource == cmdid {
                navigationController?.popViewController(animated: true)
            }
        case CLIENTEVENT_CMD_ERROR:
            if m.nSource == cmdid {
                let errmsg = TeamTalkString.clientError(TeamTalkMessagePayload.clientError(from: m))
                let alert = UIAlertController(title: NSLocalizedString("Error", comment: "Dialog message"), message: errmsg, preferredStyle: .alert)
                alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "Dialog message"), style: .default, handler: nil))
                present(alert, animated: true)
            }
        case CLIENTEVENT_CMD_PROCESSING:
            if !TeamTalkMessagePayload.isActive(m) && cmdid == m.nSource {
                cmdid = 0
            }
        default:
            break
        }
    }

    private func pushAudioCodecViewController() {
        let vc = AudioCodecViewController()
        vc.audiocodec = channel.audiocodec
        switch channel.audiocodec.nCodec {
        case SPEEX_CODEC:
            vc.speexcodec = TeamTalkAudioCodec.speexCodec(from: channel.audiocodec)
        case SPEEX_VBR_CODEC:
            vc.speexvbrcodec = TeamTalkAudioCodec.speexVBRCodec(from: channel.audiocodec)
        case OPUS_CODEC:
            vc.opuscodec = TeamTalkAudioCodec.opusCodec(from: channel.audiocodec)
        case NO_CODEC:
            if channel.nChannelID == 0 {
                vc.audiocodec.nCodec = OPUS_CODEC
            }
        default:
            vc.audiocodec.nCodec = NO_CODEC
        }
        vc.onAction = { [weak self] action, codecVC in
            guard let self else { return }
            switch action {
            case .useNoAudio:
                channel.audiocodec.nCodec = NO_CODEC
            case .useOPUS:
                codecVC.saveOPUSCodec()
                TeamTalkAudioCodec.setOpusCodec(codecVC.opuscodec, on: &channel.audiocodec)
            case .useSpeex:
                codecVC.saveSpeexCodec()
                TeamTalkAudioCodec.setSpeexCodec(codecVC.speexcodec, on: &channel.audiocodec)
            case .useSpeexVBR:
                codecVC.saveSpeexVBRCodec()
                TeamTalkAudioCodec.setSpeexVBRCodec(codecVC.speexvbrcodec, on: &channel.audiocodec)
            }
            model.refreshCodecDescription(channel.audiocodec)
            navigationController?.popViewController(animated: true)
        }
        navigationController?.pushViewController(vc, animated: true)
    }

    func saveChannelDetail() {
        model.apply(to: &channel)
        if !model.nameText.isEmpty {
            navigationItem.title = model.nameText
        }
    }
}

final class ChannelDetailModel: ObservableObject {
    let isExistingChannel: Bool

    @Published var nameText: String
    @Published var passwordText: String
    @Published var topicText: String
    @Published var isPermanent: Bool
    @Published var hasNoInterruptions: Bool
    @Published var hasNoVoiceActivation: Bool
    @Published var hasNoAudioRecording: Bool
    @Published var isHidden: Bool
    @Published var codecDescription: String

    init(channel: Channel) {
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
            return NSLocalizedString("Channel Detail", comment: "View Title")
        }
        return NSLocalizedString("Create Channel", comment: "View Title")
    }

    func refreshCodecDescription(_ codec: AudioCodec) {
        codecDescription = Self.codecDescription(for: codec)
    }

    func apply(to channel: inout Channel) {
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
            let chans = opus.nChannels > 1 ? NSLocalizedString("Stereo", comment: "create channel") : NSLocalizedString("Mono", comment: "create channel")
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
            return NSLocalizedString("No Audio", comment: "create channel")
        }
    }

    private static func bandmodeString(_ bandmode: INT32) -> String {
        switch bandmode {
        case 2:
            return NSLocalizedString("32 KHz", comment: "create channel")
        case 1:
            return NSLocalizedString("16 KHz", comment: "create channel")
        default:
            return NSLocalizedString("8 KHz", comment: "create channel")
        }
    }
}

struct ChannelDetailView: View {
    @ObservedObject var model: ChannelDetailModel

    let setupCodec: () -> Void
    let joinChannel: () -> Void
    let deleteChannel: () -> Void

    var body: some View {
        Form {
            Section(NSLocalizedString("Channel Properties", comment: "create channel")) {
                TeamTalkTextFieldRow(
                    title: NSLocalizedString("Name", comment: "create channel"),
                    placeholder: NSLocalizedString("Name", comment: "create channel"),
                    text: $model.nameText
                )
                TeamTalkTextFieldRow(
                    title: NSLocalizedString("Password", comment: "create channel"),
                    placeholder: NSLocalizedString("Password", comment: "create channel"),
                    text: $model.passwordText
                )
                TeamTalkTextFieldRow(
                    title: NSLocalizedString("Topic", comment: "create channel"),
                    placeholder: NSLocalizedString("Topic", comment: "create channel"),
                    text: $model.topicText
                )
                Button(action: setupCodec) {
                    TeamTalkValueRow(
                        title: NSLocalizedString("Audio Codec", comment: "create channel"),
                        value: model.codecDescription
                    )
                }
                .buttonStyle(.plain)
                TeamTalkToggleRow(title: NSLocalizedString("Permanent Channel", comment: "create channel"), isOn: $model.isPermanent)
                TeamTalkToggleRow(title: NSLocalizedString("No Interruptions", comment: "create channel"), isOn: $model.hasNoInterruptions)
                TeamTalkToggleRow(title: NSLocalizedString("No Voice Activation", comment: "create channel"), isOn: $model.hasNoVoiceActivation)
                TeamTalkToggleRow(title: NSLocalizedString("No Audio Recording", comment: "create channel"), isOn: $model.hasNoAudioRecording)
                TeamTalkToggleRow(title: NSLocalizedString("Hidden Channel", comment: "hidden channel"), isOn: $model.isHidden)
            }

            if model.isExistingChannel {
                Section(NSLocalizedString("Commands", comment: "create channel")) {
                    TeamTalkActionRow(title: NSLocalizedString("Join Channel", comment: "create channel"), action: joinChannel)
                    TeamTalkActionRow(title: NSLocalizedString("Delete Channel", comment: "create channel"), role: .destructive, action: deleteChannel)
                }
            }
        }
    }
}
