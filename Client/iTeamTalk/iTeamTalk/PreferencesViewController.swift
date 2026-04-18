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

import AVFoundation
import SwiftUI
import TeamTalkKit
import UIKit

let PREF_GENERAL_NICKNAME = "nickname_preference"
let PREF_GENERAL_GENDER = "gender_preference"
let PREF_GENERAL_BEARWARE_ID = "general_bearwareid_preference"
let PREF_GENERAL_BEARWARE_TOKEN = "general_bearwaretoken_preference"
let PREF_GENERAL_PTTLOCK = "general_pttlock_preference"
let PREF_GENERAL_SENDONRETURN = "general_sendonreturn_preference"
let PREF_JOINROOTCHANNEL = "joinroot_preference"

let PREF_DISPLAY_SHOWUSERNAME = "display_showusername_preference"
let PREF_DISPLAY_PROXIMITY = "display_proximity_sensor"
let PREF_DISPLAY_POPUPTXTMSG = "display_popuptxtmsg_preference"
let PREF_DISPLAY_LIMITTEXT = "display_limittext_preference"
let PREF_DISPLAY_SORTCHANNELS = "display_sortchannels_preference"
let PREF_DISPLAY_OFFICIALSERVERS = "display_officialservers_preference"
let PREF_DISPLAY_PUBLICSERVERS = "display_publicservers_preference"
let PREF_DISPLAY_UNOFFICIALSERVERS = "display_unofficialservers_preference"

let PREF_MASTER_VOLUME = "mastervolume_preference"
let PREF_MICROPHONE_GAIN = "microphonegain_preference"
let PREF_SPEAKER_OUTPUT = "speakeroutput_preference"
let PREF_BLUETOOTH_A2DP = "bluetooth_a2dp_preference"
let PREF_VOICEACTIVATION = "voiceactivationlevel_preference"
let PREF_MEDIAFILE_VOLUME = "mediafile_volume_preference"
let PREF_HEADSET_TXTOGGLE = "headset_tx_preference"
let PREF_VOICEPROCESSINGIO = "voiceprocessing_preference"
let PREF_SNDINPUT_PORT = "sndinput_port_preference" // postfix of key is AVAudioSessionPortDescription.uid. Value is dataSourceID

let PREF_SNDEVENT_SERVERLOST = "snd_srvlost_preference"
let PREF_SNDEVENT_VOICETX = "snd_voicetx_preference"
let PREF_SNDEVENT_CHANMSG = "snd_chanmsg_preference"
let PREF_SNDEVENT_USERMSG = "snd_usermsg_preference"
let PREF_SNDEVENT_BCASTMSG = "snd_bcastmsg_preference"
let PREF_SNDEVENT_JOINEDCHAN = "snd_joinedchan_preference"
let PREF_SNDEVENT_LEFTCHAN = "snd_leftchan_preference"
let PREF_SNDEVENT_VOXTRIGGER = "snd_vox_triggered_preference"
let PREF_SNDEVENT_TRANSMITREADY = "snd_transmitready_preference"
let PREF_SNDEVENT_LOGGEDIN = "snd_loggedin_preference"
let PREF_SNDEVENT_LOGGEDOUT = "snd_loggedout_preference"

let PREF_SUB_USERMSG = "sub_usertextmsg_preference"
let PREF_SUB_CHANMSG = "sub_chantextmsg_preference"
let PREF_SUB_BROADCAST = "sub_broadcastmsg_preference"
let PREF_SUB_VOICE = "sub_voice_preference"
let PREF_SUB_VIDEOCAP = "sub_videocapture_preference"
let PREF_SUB_MEDIAFILE = "sub_mediafile_preference"
let PREF_SUB_DESKTOP = "sub_desktop_preference"
let PREF_SUB_DESKTOPINPUT = "sub_desktopinput_preference"

let PREF_TTSEVENT_VOICEID = "tts_voiceid_preference"
let PREF_TTSEVENT_VOICELANG = "tts_voicelang_preference"
let PREF_TTSEVENT_JOINEDCHAN = "tts_joinedchan_preference"
let PREF_TTSEVENT_LEFTCHAN = "tts_leftchan_preference"
let PREF_TTSEVENT_CONLOST = "tts_conlost_preference"
let PREF_TTSEVENT_TEXTMSG = "tts_usertxtmsg_preference"
let PREF_TTSEVENT_CHANTEXTMSG = "tts_chantxtmsg_preference"
let PREF_TTSEVENT_RATE = "tts_rate_preference"
let PREF_TTSEVENT_VOL = "tts_volume_preference"
let PREF_TTSEVENT_USERLOGIN = "tts_user_login"
let PREF_TTSEVENT_USERLOGOUT = "tts_user_logout"

final class PreferencesModel: ObservableObject {

    struct SubscriptionRow: Identifiable {
        let title: String
        let subtitle: String
        let type: Subscription
        let key: String

        var id: String {
            key
        }
    }

    struct VersionRow: Identifiable {
        let title: String
        let value: String

        var id: String {
            title
        }
    }

    @Published var nicknameText: String
    @Published var genderIndex: Int
    @Published var pushToTalkLock: Bool
    @Published var headsetTXToggle: Bool
    @Published var sendOnReturn: Bool
    @Published var proximitySensor: Bool
    @Published var popupTextMessages: Bool
    @Published var limitText: Double
    @Published var showUsername: Bool
    @Published var channelSortIndex: Int
    @Published var joinRoot: Bool
    @Published var defaultSubscriptions: Subscriptions
    @Published var masterVolumePercent: Double
    @Published var mediaFileVolumePercent: Double
    @Published var microphoneGainPercent: Double
    @Published var voiceActivationLevel: Double
    @Published var ttsRate: Double
    @Published var ttsVolume: Double

    var users = Set<INT32>()

    let subscriptionRows: [SubscriptionRow]
    let versionRows: [VersionRow]

    init() {
        let settings = UserDefaults.standard

        nicknameText = settings.string(forKey: PREF_GENERAL_NICKNAME) ?? ""
        genderIndex = settings.integer(forKey: PREF_GENERAL_GENDER)
        pushToTalkLock = settings.object(forKey: PREF_GENERAL_PTTLOCK) != nil && settings.bool(forKey: PREF_GENERAL_PTTLOCK)
        headsetTXToggle = settings.object(forKey: PREF_HEADSET_TXTOGGLE) != nil && settings.bool(forKey: PREF_HEADSET_TXTOGGLE)
        sendOnReturn = settings.object(forKey: PREF_GENERAL_SENDONRETURN) == nil || settings.bool(forKey: PREF_GENERAL_SENDONRETURN)
        proximitySensor = settings.object(forKey: PREF_DISPLAY_PROXIMITY) != nil && settings.bool(forKey: PREF_DISPLAY_PROXIMITY)
        popupTextMessages = settings.object(forKey: PREF_DISPLAY_POPUPTXTMSG) == nil || settings.bool(forKey: PREF_DISPLAY_POPUPTXTMSG)
        limitText = Double(settings.object(forKey: PREF_DISPLAY_LIMITTEXT) == nil ? DEFAULT_LIMIT_TEXT : settings.integer(forKey: PREF_DISPLAY_LIMITTEXT))
        showUsername = settings.object(forKey: PREF_DISPLAY_SHOWUSERNAME) != nil && settings.bool(forKey: PREF_DISPLAY_SHOWUSERNAME)
        channelSortIndex = settings.object(forKey: PREF_DISPLAY_SORTCHANNELS) == nil ? ChanSort.ASCENDING.rawValue : settings.integer(forKey: PREF_DISPLAY_SORTCHANNELS)
        joinRoot = settings.object(forKey: PREF_JOINROOTCHANNEL) == nil || settings.bool(forKey: PREF_JOINROOTCHANNEL)
        defaultSubscriptions = getDefaultSubscriptions()

        masterVolumePercent = Double(refVolumeToPercent(Int(TeamTalkClient.shared.soundOutputVolume)))

        var mediaVolume = DEFAULT_MEDIAFILE_VOLUME
        if settings.value(forKey: PREF_MEDIAFILE_VOLUME) != nil {
            mediaVolume = settings.float(forKey: PREF_MEDIAFILE_VOLUME)
        }
        mediaFileVolumePercent = Double(mediaVolume * 100)

        microphoneGainPercent = Double(refVolumeToPercent(Int(TeamTalkClient.shared.soundInputGainLevel)))

        var voiceActivation = VOICEACT_DISABLED
        if settings.object(forKey: PREF_VOICEACTIVATION) != nil {
            voiceActivation = settings.integer(forKey: PREF_VOICEACTIVATION)
        }
        voiceActivationLevel = Double(voiceActivation)

        var storedTTSRate = AVSpeechUtteranceDefaultSpeechRate
        if settings.value(forKey: PREF_TTSEVENT_RATE) != nil {
            storedTTSRate = settings.float(forKey: PREF_TTSEVENT_RATE)
        }
        ttsRate = Double(storedTTSRate)

        var storedTTSVolume = DEFAULT_TTS_VOL
        if settings.value(forKey: PREF_TTSEVENT_VOL) != nil {
            storedTTSVolume = settings.float(forKey: PREF_TTSEVENT_VOL)
        }
        ttsVolume = Double(storedTTSVolume)

        subscriptionRows = [
            SubscriptionRow(title: NSLocalizedString("User Messages", comment: "preferences"), subtitle: NSLocalizedString("Receive text messages by default", comment: "preferences"), type: SUBSCRIBE_USER_MSG, key: PREF_SUB_USERMSG),
            SubscriptionRow(title: NSLocalizedString("Channel Messages", comment: "preferences"), subtitle: NSLocalizedString("Receive channel messages by default", comment: "preferences"), type: SUBSCRIBE_CHANNEL_MSG, key: PREF_SUB_CHANMSG),
            SubscriptionRow(title: NSLocalizedString("Broadcast Messages", comment: "preferences"), subtitle: NSLocalizedString("Receive broadcast messages by default", comment: "preferences"), type: SUBSCRIBE_BROADCAST_MSG, key: PREF_SUB_BROADCAST),
            SubscriptionRow(title: NSLocalizedString("Voice", comment: "preferences"), subtitle: NSLocalizedString("Receive voice streams by default", comment: "preferences"), type: SUBSCRIBE_VOICE, key: PREF_SUB_VOICE),
            SubscriptionRow(title: NSLocalizedString("WebCam", comment: "preferences"), subtitle: NSLocalizedString("Receive webcam streams by default", comment: "preferences"), type: SUBSCRIBE_VIDEOCAPTURE, key: PREF_SUB_VIDEOCAP),
            SubscriptionRow(title: NSLocalizedString("Media File", comment: "preferences"), subtitle: NSLocalizedString("Receive media file streams by default", comment: "preferences"), type: SUBSCRIBE_MEDIAFILE, key: PREF_SUB_MEDIAFILE),
            SubscriptionRow(title: NSLocalizedString("Desktop", comment: "preferences"), subtitle: NSLocalizedString("Receive desktop sessions by default", comment: "preferences"), type: SUBSCRIBE_DESKTOP, key: PREF_SUB_DESKTOP)
        ]

        let version = TeamTalkClient.shared.version
        versionRows = [
            VersionRow(
                title: NSLocalizedString("Translator", comment: "preferences"),
                value: NSLocalizedString("Bjoern D. Rasmussen, contact@bearware.dk", comment: "preferences")
            ),
            VersionRow(
                title: NSLocalizedString("App Version", comment: "preferences"),
                value: "\(AppInfo.getAppName()) v\(AppInfo.getAppVersionLong()), Library v\(version)"
            )
        ]
    }

    func nicknameChanged(_ nickname: String) {
        nicknameText = nickname
        TeamTalkClient.shared.changeNickname(nickname)
        UserDefaults.standard.set(nickname, forKey: PREF_GENERAL_NICKNAME)
    }

    func genderChanged(_ index: Int) {
        genderIndex = index
        UserDefaults.standard.set(index, forKey: PREF_GENERAL_GENDER)

        let gender = index != 0 ? StatusMode.STATUSMODE_FEMALE : StatusMode.STATUSMODE_AVAILABLE
        TeamTalkClient.shared.changeStatus(mode: INT32(gender.rawValue))
    }

    func pttlockChanged(_ enabled: Bool) {
        pushToTalkLock = enabled
        UserDefaults.standard.set(enabled, forKey: PREF_GENERAL_PTTLOCK)
    }

    func sendonenterChanged(_ enabled: Bool) {
        sendOnReturn = enabled
        UserDefaults.standard.set(enabled, forKey: PREF_GENERAL_SENDONRETURN)
    }

    func headsetTxToggleChanged(_ enabled: Bool) {
        headsetTXToggle = enabled
        UserDefaults.standard.set(enabled, forKey: PREF_HEADSET_TXTOGGLE)

        if enabled {
            UIApplication.shared.beginReceivingRemoteControlEvents()
        } else {
            UIApplication.shared.endReceivingRemoteControlEvents()
        }

        setupSoundDevices()
    }

    func showtextmessagesChanged(_ enabled: Bool) {
        popupTextMessages = enabled
        UserDefaults.standard.set(enabled, forKey: PREF_DISPLAY_POPUPTXTMSG)
    }

    func proximityChanged(_ enabled: Bool) {
        proximitySensor = enabled
        UserDefaults.standard.set(enabled, forKey: PREF_DISPLAY_PROXIMITY)
        UIDevice.current.isProximityMonitoringEnabled = enabled
    }

    func limittextChanged(_ value: Double) {
        limitText = value
        UserDefaults.standard.set(Int(value), forKey: PREF_DISPLAY_LIMITTEXT)
    }

    func showusernameChanged(_ enabled: Bool) {
        showUsername = enabled
        UserDefaults.standard.set(enabled, forKey: PREF_DISPLAY_SHOWUSERNAME)
    }

    func channelSortChanged(_ index: Int) {
        channelSortIndex = index
        UserDefaults.standard.set(index == 0 ? ChanSort.ASCENDING.rawValue : ChanSort.POPULARITY.rawValue, forKey: PREF_DISPLAY_SORTCHANNELS)
    }

    func joinrootChanged(_ enabled: Bool) {
        joinRoot = enabled
        UserDefaults.standard.set(enabled, forKey: PREF_JOINROOTCHANNEL)
    }

    func subscriptionChanged(_ enabled: Bool, row: SubscriptionRow) {
        if enabled {
            defaultSubscriptions |= row.type.rawValue
        } else {
            defaultSubscriptions &= ~row.type.rawValue
        }
        UserDefaults.standard.set(enabled, forKey: row.key)
    }

    func masterVolumeChanged(_ percent: Double) {
        let roundedPercent = Double(Int(percent / 10.0) * 10)
        masterVolumePercent = roundedPercent
        let vol = refVolume(roundedPercent)
        TeamTalkClient.shared.setSoundOutputVolume(INT32(vol))
        UserDefaults.standard.set(Int(roundedPercent), forKey: PREF_MASTER_VOLUME)
    }

    func mediafileVolumeChanged(_ percent: Double) {
        mediaFileVolumePercent = percent
        let normalized = Float(percent / 100.0)
        UserDefaults.standard.set(normalized, forKey: PREF_MEDIAFILE_VOLUME)

        let vol = refVolume(percent)
        for userID in users {
            TeamTalkClient.shared.setUserVolume(userID: userID, stream: STREAMTYPE_MEDIAFILE_AUDIO, volume: INT32(vol))
            TeamTalkClient.shared.pump(CLIENTEVENT_USER_STATECHANGE, source: userID)
        }
    }

    func microphoneGainChanged(_ percent: Double) {
        let roundedPercent = Double(Int(percent / 10.0) * 10)
        microphoneGainPercent = roundedPercent
        let vol = refVolume(roundedPercent)
        TeamTalkClient.shared.setSoundInputGainLevel(INT32(vol))
        UserDefaults.standard.set(Int(roundedPercent), forKey: PREF_MICROPHONE_GAIN)
    }

    func voiceactlevelChanged(_ levelValue: Double) {
        let level = Int(levelValue)
        voiceActivationLevel = Double(level)

        if level == VOICEACT_DISABLED {
            TeamTalkClient.shared.enableVoiceActivation(false)
        } else {
            TeamTalkClient.shared.enableVoiceActivation(true)
            TeamTalkClient.shared.setVoiceActivationLevel(INT32(level))
        }
        UserDefaults.standard.set(level, forKey: PREF_VOICEACTIVATION)
    }

    func ttsrateChanged(_ value: Double) {
        ttsRate = value
        UserDefaults.standard.set(Float(value), forKey: PREF_TTSEVENT_RATE)
    }

    func ttsvolChanged(_ value: Double) {
        ttsVolume = value
        UserDefaults.standard.set(Float(value), forKey: PREF_TTSEVENT_VOL)
    }

    func isSubscribed(to row: SubscriptionRow) -> Bool {
        (defaultSubscriptions & row.type.rawValue) != 0
    }

    func percentSubtitle(_ value: Double) -> String {
        let percent = Int(value.rounded())
        let vol = refVolume(Double(percent))
        if UInt32(vol) == SOUND_VOLUME_DEFAULT.rawValue {
            return String(format: NSLocalizedString("%d %% - Default", comment: "preferences"), percent)
        }
        return "\(percent) %"
    }

    func voiceActivationValueText(_ value: Double) -> String {
        let level = Int(value.rounded())
        if level == VOICEACT_DISABLED {
            return NSLocalizedString("Disabled", comment: "preferences")
        }
        return "\(level)"
    }

    func voiceActivationSubtitle(_ value: Double) -> String {
        let level = Int(value.rounded())
        if level == VOICEACT_DISABLED {
            return NSLocalizedString("Voice Activation Level: Disabled", comment: "preferences")
        }
        return String(format: NSLocalizedString("Voice Activation Level: %d. Recommended: %d", comment: "preferences"), level, DEFAULT_VOICEACT)
    }
}

extension PreferencesModel: TeamTalkEvent {
    func handleTTMessage(_ m: TTMessage) {
        switch m.nClientEvent {
        case CLIENTEVENT_CMD_USER_JOINED:
            users.insert(TeamTalkMessagePayload.user(from: m).nUserID)
        case CLIENTEVENT_CMD_USER_LEFT:
            users.remove(TeamTalkMessagePayload.user(from: m).nUserID)
        default:
            break
        }
    }
}

struct PreferencesView: View {
    @ObservedObject var model: PreferencesModel

    var body: some View {
        Form {
            generalSection
            displaySection
            soundSection
            soundEventsSection
            ttsSection
            connectionSection
            subscriptionsSection
            versionSection
        }
        .navigationTitle(NSLocalizedString("Preferences", comment: "tab"))
    }

    private var generalSection: some View {
        Section(NSLocalizedString("General", comment: "preferences")) {
            VStack(alignment: .leading, spacing: 4) {
                TeamTalkTextFieldRow(
                    title: NSLocalizedString("Nickname", comment: "preferences"),
                    placeholder: NSLocalizedString("Nickname", comment: "preferences"),
                    text: Binding(
                        get: { model.nicknameText },
                        set: { model.nicknameChanged($0) }
                    )
                )
                PreferenceSubtitle(NSLocalizedString("Name displayed in channel list", comment: "preferences"))
            }

            VStack(alignment: .leading, spacing: 4) {
                TeamTalkSegmentedRow(
                    title: NSLocalizedString("Gender", comment: "preferences"),
                    values: [
                        NSLocalizedString("Male", comment: "preferences"),
                        NSLocalizedString("Female", comment: "preferences")
                    ],
                    selectedIndex: Binding(
                        get: { model.genderIndex },
                        set: { model.genderChanged($0) }
                    )
                )
                PreferenceSubtitle(NSLocalizedString("Show male or female icon", comment: "preferences"))
            }

            NavigationLink {
                WebLoginView()
            } label: {
                TeamTalkValueRow(
                    title: NSLocalizedString("BearWare.dk Web Login", comment: "preferences"),
                    subtitle: NSLocalizedString("Login ID from BearWare.dk", comment: "preferences")
                )
            }

            TeamTalkToggleRow(
                title: NSLocalizedString("Push To Talk Lock", comment: "preferences"),
                subtitle: NSLocalizedString("Double tap to lock TX button", comment: "preferences"),
                isOn: Binding(get: { model.pushToTalkLock }, set: { model.pttlockChanged($0) })
            )
            TeamTalkToggleRow(
                title: NSLocalizedString("Headset TX Toggle", comment: "preferences"),
                subtitle: NSLocalizedString("Toggle voice transmission using headset", comment: "preferences"),
                isOn: Binding(get: { model.headsetTXToggle }, set: { model.headsetTxToggleChanged($0) })
            )
            TeamTalkToggleRow(
                title: NSLocalizedString("Return Sends Message", comment: "preferences"),
                subtitle: NSLocalizedString("Pressing Return-key sends text message", comment: "preferences"),
                isOn: Binding(get: { model.sendOnReturn }, set: { model.sendonenterChanged($0) })
            )
        }
    }

    private var displaySection: some View {
        Section(NSLocalizedString("Display", comment: "preferences")) {
            TeamTalkToggleRow(
                title: NSLocalizedString("Proximity Sensor", comment: "preferences"),
                subtitle: NSLocalizedString("Turn off screen when holding phone near ear", comment: "preferences"),
                isOn: Binding(get: { model.proximitySensor }, set: { model.proximityChanged($0) })
            )
            TeamTalkToggleRow(
                title: NSLocalizedString("Show Text Messages Instantly", comment: "preferences"),
                subtitle: NSLocalizedString("Pop up text message when new messages are received", comment: "preferences"),
                isOn: Binding(get: { model.popupTextMessages }, set: { model.showtextmessagesChanged($0) })
            )
            VStack(alignment: .leading, spacing: 4) {
                TeamTalkStepperRow(
                    title: NSLocalizedString("Maximum Text Length", comment: "preferences"),
                    value: Binding(get: { model.limitText }, set: { model.limittextChanged($0) }),
                    range: 1...Double(TT_STRLEN - 1),
                    step: 1
                )
                PreferenceSubtitle(String(format: NSLocalizedString("Limit length of names in channel list to %d characters", comment: "preferences"), Int(model.limitText)))
            }
            NavigationLink {
                PublicServerView()
            } label: {
                TeamTalkValueRow(
                    title: NSLocalizedString("Filter Server List", comment: "preferences"),
                    subtitle: NSLocalizedString("Limit types of servers to show", comment: "preferences")
                )
            }
            TeamTalkToggleRow(
                title: NSLocalizedString("Show Usernames", comment: "preferences"),
                subtitle: NSLocalizedString("Show usernames instead of nicknames", comment: "preferences"),
                isOn: Binding(get: { model.showUsername }, set: { model.showusernameChanged($0) })
            )
            VStack(alignment: .leading, spacing: 4) {
                TeamTalkSegmentedRow(
                    title: NSLocalizedString("Sort Channels", comment: "preferences"),
                    values: [
                        NSLocalizedString("Ascending", comment: "preferences"),
                        NSLocalizedString("Popularity", comment: "preferences")
                    ],
                    selectedIndex: Binding(get: { model.channelSortIndex }, set: { model.channelSortChanged($0) })
                )
                PreferenceSubtitle(NSLocalizedString("Order of channels in Channel List", comment: "preferences"))
            }
        }
    }

    private var soundSection: some View {
        Section(NSLocalizedString("Sound System", comment: "preferences")) {
            sliderWithSubtitle(
                title: NSLocalizedString("Master Volume", comment: "preferences"),
                subtitle: model.percentSubtitle(model.masterVolumePercent),
                value: Binding(get: { model.masterVolumePercent }, set: { model.masterVolumeChanged($0) }),
                range: 0...100,
                step: 10,
                displayValue: { model.percentSubtitle($0) }
            )
            sliderWithSubtitle(
                title: NSLocalizedString("Media File Volume", comment: "preferences"),
                subtitle: NSLocalizedString("Media file vs. voice volume", comment: "preferences"),
                value: Binding(get: { model.mediaFileVolumePercent }, set: { model.mediafileVolumeChanged($0) }),
                range: 0...100,
                step: 1,
                displayValue: { "\(Int($0.rounded())) %" }
            )
            sliderWithSubtitle(
                title: NSLocalizedString("Microphone Gain", comment: "preferences"),
                subtitle: model.percentSubtitle(model.microphoneGainPercent),
                value: Binding(get: { model.microphoneGainPercent }, set: { model.microphoneGainChanged($0) }),
                range: 0...100,
                step: 10,
                displayValue: { model.percentSubtitle($0) }
            )
            sliderWithSubtitle(
                title: NSLocalizedString("Voice Activation Level", comment: "preferences"),
                subtitle: model.voiceActivationSubtitle(model.voiceActivationLevel),
                value: Binding(get: { model.voiceActivationLevel }, set: { model.voiceactlevelChanged($0) }),
                range: 0...Double(VOICEACT_DISABLED),
                step: 1,
                displayValue: { model.voiceActivationValueText($0) }
            )
            NavigationLink {
                SoundDevicesView()
            } label: {
                TeamTalkValueRow(
                    title: NSLocalizedString("Setup Sound Devices", comment: "preferences"),
                    subtitle: NSLocalizedString("Choose input and output devices", comment: "preferences")
                )
            }
        }
    }

    private var soundEventsSection: some View {
        Section(NSLocalizedString("Sound Events", comment: "preferences")) {
            NavigationLink {
                SoundEventsView()
            } label: {
                TeamTalkValueRow(
                    title: NSLocalizedString("Setup Sound Events", comment: "preferences"),
                    subtitle: NSLocalizedString("Choose sounds events to play", comment: "preferences")
                )
            }
        }
    }

    private var ttsSection: some View {
        Section(NSLocalizedString("Text To Speech Events", comment: "preferences")) {
            NavigationLink {
                SpeechListView()
            } label: {
                TeamTalkValueRow(
                    title: NSLocalizedString("Speech", comment: "preferences"),
                    subtitle: NSLocalizedString("Select the text-to-speech voice to use", comment: "preferences")
                )
            }
            sliderWithSubtitle(
                title: NSLocalizedString("Speech Rate", comment: "preferences"),
                subtitle: String(format: NSLocalizedString("The rate of the speaking voice is %.1f", comment: "preferences"), Float(model.ttsRate)),
                value: Binding(get: { model.ttsRate }, set: { model.ttsrateChanged($0) }),
                range: Double(AVSpeechUtteranceMinimumSpeechRate)...Double(AVSpeechUtteranceMaximumSpeechRate),
                step: 0.1,
                displayValue: { String(format: "%.1f", $0) }
            )
            sliderWithSubtitle(
                title: NSLocalizedString("Speech Volume", comment: "preferences"),
                subtitle: String(format: NSLocalizedString("The volume of the speaking voice is %.1f", comment: "preferences"), Float(model.ttsVolume)),
                value: Binding(get: { model.ttsVolume }, set: { model.ttsvolChanged($0) }),
                range: 0...1,
                step: 0.1,
                displayValue: { String(format: "%.1f", $0) }
            )
            NavigationLink {
                TextToSpeechEventsView()
            } label: {
                TeamTalkValueRow(
                    title: NSLocalizedString("Setup Announcements", comment: "preferences"),
                    subtitle: NSLocalizedString("Choose events to playback", comment: "preferences")
                )
            }
        }
    }

    private var connectionSection: some View {
        Section(NSLocalizedString("Connection", comment: "preferences")) {
            TeamTalkToggleRow(
                title: NSLocalizedString("Join Root Channel", comment: "preferences"),
                subtitle: NSLocalizedString("Join root channel after login", comment: "preferences"),
                isOn: Binding(get: { model.joinRoot }, set: { model.joinrootChanged($0) })
            )
        }
    }

    private var subscriptionsSection: some View {
        Section(NSLocalizedString("Default Subscriptions", comment: "preferences")) {
            ForEach(model.subscriptionRows) { row in
                TeamTalkToggleRow(
                    title: row.title,
                    subtitle: row.subtitle,
                    isOn: Binding(
                        get: { model.isSubscribed(to: row) },
                        set: { model.subscriptionChanged($0, row: row) }
                    )
                )
            }
        }
    }

    private var versionSection: some View {
        Section(NSLocalizedString("Version Information", comment: "preferences")) {
            ForEach(model.versionRows) { row in
                TeamTalkValueRow(title: row.title, value: row.value)
            }
        }
    }

    private func sliderWithSubtitle(title: String,
                                    subtitle: String,
                                    value: Binding<Double>,
                                    range: ClosedRange<Double>,
                                    step: Double,
                                    displayValue: @escaping (Double) -> String) -> some View {
        VStack(alignment: .leading, spacing: 4) {
            TeamTalkSliderRow(title: title, value: value, range: range, step: step, displayValue: displayValue)
            PreferenceSubtitle(subtitle)
        }
    }
}

private struct PreferenceSubtitle: View {
    let text: String

    init(_ text: String) {
        self.text = text
    }

    var body: some View {
        Text(text)
            .font(.footnote)
            .foregroundStyle(.secondary)
    }
}

