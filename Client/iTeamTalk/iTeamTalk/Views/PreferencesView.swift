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
        .navigationTitle("Preferences")
    }

    private var generalSection: some View {
        Section("General") {
            VStack(alignment: .leading, spacing: 4) {
                TextField("Nickname", text: Binding(
                    get: { model.nicknameText },
                    set: { model.nicknameChanged($0) }
                ))
                .multilineTextAlignment(.trailing)
                .autocorrectionDisabled()
                PreferenceSubtitle("Name displayed in channel list")
            }

            VStack(alignment: .leading, spacing: 4) {
                VStack(alignment: .leading, spacing: 8) {
                    Text("Gender")
                    Picker("Gender", selection: Binding(
                        get: { model.genderIndex },
                        set: { model.genderChanged($0) }
                    )) {
                        Text("Male").tag(0)
                        Text("Female").tag(1)
                    }
                    .pickerStyle(.segmented)
                }
                PreferenceSubtitle("Show male or female icon")
            }

            NavigationLink {
                WebLoginView()
            } label: {
                VStack(alignment: .leading, spacing: 2) {
                    Text("BearWare.dk Web Login")
                    Text("Login ID from BearWare.dk")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }

            Toggle(isOn: Binding(get: { model.pushToTalkLock }, set: { model.pttlockChanged($0) })) {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Push To Talk Lock")
                    Text("Double tap to lock TX button")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
                            }
            Toggle(isOn: Binding(get: { model.headsetTXToggle }, set: { model.headsetTxToggleChanged($0) })) {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Headset TX Toggle")
                    Text("Toggle voice transmission using headset")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
            Toggle(isOn: Binding(get: { model.sendOnReturn }, set: { model.sendonenterChanged($0) })) {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Return Sends Message")
                    Text("Pressing Return-key sends text message")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
        }
    }

    private var displaySection: some View {
        Section("Display") {
            Toggle(isOn: Binding(get: { model.proximitySensor }, set: { model.proximityChanged($0) })) {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Proximity Sensor")
                    Text("Turn off screen when holding phone near ear")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
            Toggle(isOn: Binding(get: { model.popupTextMessages }, set: { model.showtextmessagesChanged($0) })) {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Show Text Messages Instantly")
                    Text("Pop up text message when new messages are received")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
            VStack(alignment: .leading, spacing: 4) {
                Stepper(value: Binding(get: { model.limitText }, set: { model.limittextChanged($0) }),
                        in: 1...Double(TT_STRLEN - 1), step: 1) {
                    HStack(spacing: 12) {
                        Text("Maximum Text Length")
                        Spacer(minLength: 16)
                        Text("\(Int(model.limitText.rounded()))")
                            .font(.body.monospacedDigit())
                            .foregroundStyle(.secondary)
                    }
                }
                PreferenceSubtitle(verbatim: String(format: String(localized: "Limit length of names in channel list to %d characters", comment: "preferences"), Int(model.limitText)))
            }
            NavigationLink {
                PublicServerView()
            } label: {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Filter Server List")
                    Text("Limit types of servers to show")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
            Toggle(isOn: Binding(get: { model.showUsername }, set: { model.showusernameChanged($0) })) {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Show Usernames")
                    Text("Show usernames instead of nicknames")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
            VStack(alignment: .leading, spacing: 4) {
                VStack(alignment: .leading, spacing: 8) {
                    Text("Sort Channels")
                    Picker("Sort Channels", selection: Binding(
                        get: { model.channelSortIndex },
                        set: { model.channelSortChanged($0) }
                    )) {
                        Text("Ascending").tag(0)
                        Text("Popularity").tag(1)
                    }
                    .pickerStyle(.segmented)
                }
                PreferenceSubtitle("Order of channels in Channel List")
            }
        }
    }

    private var soundSection: some View {
        Section("Sound System") {
            sliderWithSubtitle(
                title: "Master Volume",
                subtitle: Text(verbatim: model.percentSubtitle(model.masterVolumePercent)),
                value: Binding(get: { model.masterVolumePercent }, set: { model.masterVolumeChanged($0) }),
                range: 0...100,
                step: 10,
                displayValue: { model.percentSubtitle($0) }
            )
            sliderWithSubtitle(
                title: "Media File Volume",
                subtitle: Text("Media file vs. voice volume"),
                value: Binding(get: { model.mediaFileVolumePercent }, set: { model.mediafileVolumeChanged($0) }),
                range: 0...100,
                step: 1,
                displayValue: { "\(Int($0.rounded())) %" }
            )
            sliderWithSubtitle(
                title: "Microphone Gain",
                subtitle: Text(verbatim: model.percentSubtitle(model.microphoneGainPercent)),
                value: Binding(get: { model.microphoneGainPercent }, set: { model.microphoneGainChanged($0) }),
                range: 0...100,
                step: 10,
                displayValue: { model.percentSubtitle($0) }
            )
            sliderWithSubtitle(
                title: "Voice Activation Level",
                subtitle: Text(verbatim: model.voiceActivationSubtitle(model.voiceActivationLevel)),
                value: Binding(get: { model.voiceActivationLevel }, set: { model.voiceactlevelChanged($0) }),
                range: 0...Double(VOICEACT_DISABLED),
                step: 1,
                displayValue: { model.voiceActivationValueText($0) }
            )
            NavigationLink {
                SoundDevicesView()
            } label: {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Setup Sound Devices")
                    Text("Choose input and output devices")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
        }
    }

    private var soundEventsSection: some View {
        Section("Sound Events") {
            NavigationLink {
                SoundEventsView()
            } label: {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Setup Sound Events")
                    Text("Choose sounds events to play")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
        }
    }

    private var ttsSection: some View {
        Section("Text To Speech Events") {
            NavigationLink {
                SpeechListView()
            } label: {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Speech")
                    Text("Select the text-to-speech voice to use")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
            sliderWithSubtitle(
                title: "Speech Rate",
                subtitle: Text(verbatim: String(format: String(localized: "The rate of the speaking voice is %.1f", comment: "preferences"), Float(model.ttsRate))),
                value: Binding(get: { model.ttsRate }, set: { model.ttsrateChanged($0) }),
                range: Double(AVSpeechUtteranceMinimumSpeechRate)...Double(AVSpeechUtteranceMaximumSpeechRate),
                step: 0.1,
                displayValue: { String(format: "%.1f", $0) }
            )
            sliderWithSubtitle(
                title: "Speech Volume",
                subtitle: Text(verbatim: String(format: String(localized: "The volume of the speaking voice is %.1f", comment: "preferences"), Float(model.ttsVolume))),
                value: Binding(get: { model.ttsVolume }, set: { model.ttsvolChanged($0) }),
                range: 0...1,
                step: 0.1,
                displayValue: { String(format: "%.1f", $0) }
            )
            NavigationLink {
                TextToSpeechEventsView()
            } label: {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Setup Announcements")
                    Text("Choose events to playback")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
        }
    }

    private var connectionSection: some View {
        Section("Connection") {
            Toggle(isOn: Binding(get: { model.joinRoot }, set: { model.joinrootChanged($0) })) {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Join Root Channel")
                    Text("Join root channel after login")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
        }
    }

    private var subscriptionsSection: some View {
        Section("Default Subscriptions") {
            ForEach(model.subscriptionRows) { row in
                Toggle(isOn: Binding(
                    get: { model.isSubscribed(to: row) },
                    set: { model.subscriptionChanged($0, row: row) }
                )) {
                    VStack(alignment: .leading, spacing: 2) {
                        Text(row.title)
                        Text(row.subtitle)
                            .font(.footnote)
                            .foregroundStyle(.secondary)
                    }
                }
            }
        }
    }

    private var versionSection: some View {
        Section("Version Information") {
            ForEach(model.versionRows) { row in
                LabeledContent(row.title, value: row.value)
            }
        }
    }

    private func sliderWithSubtitle(title: LocalizedStringKey,
                                    subtitle: Text,
                                    value: Binding<Double>,
                                    range: ClosedRange<Double>,
                                    step: Double,
                                    displayValue: @escaping (Double) -> String) -> some View {
        VStack(alignment: .leading, spacing: 4) {
            VStack(alignment: .leading, spacing: 8) {
                HStack(spacing: 12) {
                    Text(title)
                    Spacer(minLength: 16)
                    Text(displayValue(value.wrappedValue))
                        .font(.body.monospacedDigit())
                        .foregroundStyle(.secondary)
                }
                Slider(value: value, in: range, step: step)
            }
            PreferenceSubtitle(subtitle)
        }
    }
}

struct PreferenceSubtitle: View {
    let text: Text

    init(_ text: LocalizedStringKey) {
        self.text = Text(text)
    }

    init(verbatim text: String) {
        self.text = Text(verbatim: text)
    }

    init(_ text: Text) {
        self.text = text
    }

    var body: some View {
        text
            .font(.footnote)
            .foregroundStyle(.secondary)
    }
}
