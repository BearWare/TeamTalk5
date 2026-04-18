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

struct SpeechListView: View {
    private let sections: [String]
    @State private var selectedVoiceIdentifier = UserDefaults.standard.string(forKey: PREF_TTSEVENT_VOICEID)

    init() {
        var languages = [AVSpeechSynthesisVoice.currentLanguageCode()]
        for voice in AVSpeechSynthesisVoice.speechVoices() where !languages.contains(voice.language) {
            languages.append(voice.language)
        }
        sections = languages
    }

    var body: some View {
        List {
            ForEach(sections, id: \.self) { language in
                Section(language) {
                    ForEach(voices(for: language), id: \.identifier) { voice in
                        Button {
                            select(voice)
                        } label: {
                            TeamTalkValueRow(
                                title: voice.name,
                                subtitle: localName(for: language),
                                value: selectedVoiceIdentifier == voice.identifier ? NSLocalizedString("Selected", comment: "speech") : nil
                            )
                        }
                        .buttonStyle(.plain)
                    }
                }
            }
        }
        .navigationTitle(NSLocalizedString("Text-to-Speech Voice", comment: "speech"))
    }

    private func voices(for language: String) -> [AVSpeechSynthesisVoice] {
        AVSpeechSynthesisVoice.speechVoices().filter { $0.language == language }
    }

    private func localName(for language: String) -> String? {
        (Locale.current as NSLocale).displayName(forKey: NSLocale.Key.identifier, value: language)
    }

    private func select(_ voice: AVSpeechSynthesisVoice) {
        selectedVoiceIdentifier = voice.identifier
        UserDefaults.standard.setValue(voice.identifier, forKey: PREF_TTSEVENT_VOICEID)

        let utterance = String(format: NSLocalizedString("You have selected %@" , comment: "speech"), voice.name)
        newUtterance(utterance)
    }
}
