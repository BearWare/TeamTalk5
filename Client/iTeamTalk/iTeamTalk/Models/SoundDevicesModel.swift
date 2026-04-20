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

final class SoundDevicesModel: ObservableObject {
    struct ToggleRow: Identifiable {
        let id: String
        let title: String
        let subtitle: String
        let preferenceKey: String
    }

    struct AudioInputSection: Identifiable {
        let id: String
        let title: String
        let input: AVAudioSessionPortDescription
        let dataSources: [AVAudioSessionDataSourceDescription?]
    }

    @Published private var revision = 0

    let toggleRows = [
        ToggleRow(
            id: PREF_SPEAKER_OUTPUT,
            title: String(localized: "Speaker Output", comment: "preferences"),
            subtitle: String(localized: "Use iPhone's speaker instead of earpiece", comment: "preferences"),
            preferenceKey: PREF_SPEAKER_OUTPUT
        ),
        ToggleRow(
            id: PREF_VOICEPROCESSINGIO,
            title: String(localized: "Voice Preprocessing", comment: "preferences"),
            subtitle: String(localized: "Use echo cancellation and automatic gain control", comment: "Sound Devices"),
            preferenceKey: PREF_VOICEPROCESSINGIO
        ),
        ToggleRow(
            id: PREF_BLUETOOTH_A2DP,
            title: String(localized: "Bluetooth A2DP Playback", comment: "Sound Devices"),
            subtitle: String(localized: "Bluetooth playback should use Advanced Audio Distribution Profile", comment: "Sound Devices"),
            preferenceKey: PREF_BLUETOOTH_A2DP
        )
    ]

    var audioInputSections: [AudioInputSection] {
        _ = revision

        let session = AVAudioSession.sharedInstance()
        return (session.availableInputs ?? []).map { input in
            let dataSources = input.dataSources ?? []
            return AudioInputSection(
                id: input.uid,
                title: input.portName,
                input: input,
                dataSources: dataSources.isEmpty ? [nil] : dataSources.map { Optional($0) }
            )
        }
    }

    private var routeObserver: NSObjectProtocol?

    init() {
        if !TeamTalkClient.shared.isSoundInputReady {
            setupSoundDevices()
        }

        routeObserver = NotificationCenter.default.addObserver(
            forName: AVAudioSession.routeChangeNotification,
            object: nil,
            queue: .main
        ) { [weak self] _ in
            print("Audio Route changed in Sound Devices view")
            self?.reload()
        }
    }

    deinit {
        if let routeObserver {
            NotificationCenter.default.removeObserver(routeObserver)
        }
    }

    func preferenceValue(forKey key: String) -> Bool {
        UserDefaults.standard.object(forKey: key) != nil && UserDefaults.standard.bool(forKey: key)
    }

    func setPreference(_ value: Bool, forKey key: String) {
        UserDefaults.standard.set(value, forKey: key)
        setupSoundDevices()
        reload()
    }

    func title(for dataSources: [AVAudioSessionDataSourceDescription?],
               at index: Int,
               input: AVAudioSessionPortDescription) -> String {
        guard index < dataSources.count, let dataSource = dataSources[index] else {
            return String(localized: "Default", comment: "Sound Devices")
        }

        var title = dataSource.dataSourceName
        if let supportedPolarPatterns = dataSource.supportedPolarPatterns,
           supportedPolarPatterns.contains(.stereo) {
            title += " (" + String(localized: "Stereo", comment: "Sound Devices") + ")"
        }

        if getAudioPortDataSource(descr: input) == dataSource.dataSourceID {
            title += ", " + String(localized: "Preferred", comment: "Sound Devices")
        }

        if AVAudioSession.sharedInstance().inputDataSource?.dataSourceID == dataSource.dataSourceID {
            title += ", " + String(localized: "Active", comment: "Sound Devices")
        }

        return title
    }

    func selectDataSource(at index: Int, for input: AVAudioSessionPortDescription) {
        do {
            let session = AVAudioSession.sharedInstance()
            print("UID of \(input.portName): \(input.uid)")

            if let dataSources = input.dataSources, index < dataSources.count {
                let dataSource = dataSources[index]
                print("Data source ID: \(dataSource.dataSourceID)")
                try input.setPreferredDataSource(dataSource)
                try session.setPreferredInput(input)
                try session.setInputDataSource(dataSource)
                setAudioPortDataSource(descr: input, dsrc: dataSource)
            } else {
                try session.setPreferredInput(input)
                removeAudioPortDataSource(descr: input)
            }

            print(session.currentRoute)
        } catch {
            print("Failed to select data source")
        }

        reload()
    }

    private func reload() {
        revision += 1
    }
}
