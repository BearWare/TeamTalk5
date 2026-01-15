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

import Foundation
import AVFoundation

func refVolume(_ percent: Double) -> Int {
    //82.832*EXP(0.0508*x) - 50
    if percent == 0 {
        return 0
    }
    
    let d = 82.832 * exp(0.0508 * percent) - 50
    return Int(d)
}

func refVolumeToPercent(_ volume: Int) -> Int {
    if(volume == 0) {
        return 0
    }
    
    let d = (Double(volume) + 50.0) / 82.832
    let d1 = (log(d) / 0.0508) + 0.5
    return Int(d1)
}

enum Sounds : Int {
    case tx_ON = 1,
         tx_OFF = 2,
         chan_MSG = 3,
         broadcast_MSG = 4,
         user_MSG = 5,
         srv_LOST = 6,
         joined_CHAN = 7,
         left_CHAN = 8,
         voxtriggered_ON = 9,
         voxtriggered_OFF = 10,
         transmit_ON = 11,
         transmit_OFF = 12,
         logged_IN = 13,
         logged_OUT = 14
}

var player : AVAudioPlayer?

func getSoundFile(_ s: Sounds) -> String? {
    
    let settings = UserDefaults.standard
    
    switch s {
    case .tx_ON:
        if settings.object(forKey: PREF_SNDEVENT_VOICETX) == nil ||
           settings.bool(forKey: PREF_SNDEVENT_VOICETX) {
            return "on"
        }
    case .tx_OFF:
        if settings.object(forKey: PREF_SNDEVENT_VOICETX) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_VOICETX) {
                return "off"
        }
    case .chan_MSG:
        if settings.object(forKey: PREF_SNDEVENT_CHANMSG) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_CHANMSG) {
                return "channel_message"
        }
    case .user_MSG:
        if settings.object(forKey: PREF_SNDEVENT_USERMSG) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_USERMSG) {
                return "user_message"
        }
    case .broadcast_MSG:
        if settings.object(forKey: PREF_SNDEVENT_BCASTMSG) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_BCASTMSG) {
            return "broadcast_message"
        }
    case .srv_LOST:
        if settings.object(forKey: PREF_SNDEVENT_SERVERLOST) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_SERVERLOST) {
                return "serverlost"
        }
    case .joined_CHAN:
        if settings.object(forKey: PREF_SNDEVENT_JOINEDCHAN) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_JOINEDCHAN) {
                return "newuser"
        }
    case .left_CHAN:
        if settings.object(forKey: PREF_SNDEVENT_LEFTCHAN) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_LEFTCHAN) {
                return "removeuser"
        }
    case .voxtriggered_ON :
        if settings.object(forKey: PREF_SNDEVENT_VOXTRIGGER) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_VOXTRIGGER) {
            return "voiceact_on"
        }
    case .voxtriggered_OFF :
        if settings.object(forKey: PREF_SNDEVENT_VOXTRIGGER) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_VOXTRIGGER) {
            return "voiceact_off"
        }
    case .transmit_ON :
        if settings.object(forKey: PREF_SNDEVENT_TRANSMITREADY) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_TRANSMITREADY) {
            return "txqueue_start"
        }
    case .transmit_OFF :
        if settings.object(forKey: PREF_SNDEVENT_TRANSMITREADY) == nil ||
            settings.bool(forKey: PREF_SNDEVENT_TRANSMITREADY) {
            return "txqueue_stop"
        }
    case .logged_IN :
        if settings.object(forKey: PREF_SNDEVENT_LOGGEDIN) != nil &&
            settings.bool(forKey: PREF_SNDEVENT_LOGGEDIN) {
            return "logged_on"
        }
    case .logged_OUT :
        if settings.object(forKey: PREF_SNDEVENT_LOGGEDOUT) != nil &&
            settings.bool(forKey: PREF_SNDEVENT_LOGGEDOUT) {
            return "logged_off"
        }
    }

    return nil
}

func getCategory(_ opt: AVAudioSession.CategoryOptions) -> String {
    var str = ""
    if opt.contains(.defaultToSpeaker) {
        str += "defaultToSpeaker|"
    }
    if opt.contains(.mixWithOthers) {
        str += "mixWithOthers|"
    }
    if opt.contains(.allowBluetooth) {
        str += "allowBluetooth|"
    }
    if opt.contains(.duckOthers) {
        str += "duckOthers|"
    }
    if opt.contains(.interruptSpokenAudioAndMixWithOthers) {
        str += "interruptSpokenAudioAndMixWithOthers|"
    }
    if #available(iOS 14.5, *) {
        if opt.contains(.overrideMutedMicrophoneInterruption) {
            str += "overrideMutedMicrophoneInterruption|"
        }
    }
    if opt.contains(.allowAirPlay) {
        str += "allowAirPlay|"
    }
    if opt.contains(.allowBluetoothA2DP) {
        str += "allowBluetoothA2DP|"
    }
    return str
}

func getAudioPortDataSource(descr: AVAudioSessionPortDescription) -> NSNumber? {
    let defaults = UserDefaults.standard
    let prefname = PREF_SNDINPUT_PORT + "_" + descr.uid
    if let id = defaults.object(forKey: prefname) as? NSNumber {
        return id
    }
    return nil
}

func setAudioPortDataSource(descr: AVAudioSessionPortDescription, dsrc: AVAudioSessionDataSourceDescription) {
    let defaults = UserDefaults.standard
    let prefname = PREF_SNDINPUT_PORT + "_" + descr.uid
    defaults.set(dsrc.dataSourceID, forKey: prefname)
}

func removeAudioPortDataSource(descr: AVAudioSessionPortDescription) {
    let defaults = UserDefaults.standard
    let prefname = PREF_SNDINPUT_PORT + "_" + descr.uid
    defaults.removeObject(forKey: prefname)
}

func closeSoundDevices() {
    TT_CloseSoundInputDevice(ttInst)
    TT_CloseSoundOutputDevice(ttInst)
}

func setupSoundDevices() {
    
    do {
        closeSoundDevices()
        
        let session = AVAudioSession.sharedInstance()

        print("preset: " + session.mode.rawValue)
        
        let defaults = UserDefaults.standard
        let speaker = defaults.object(forKey: PREF_SPEAKER_OUTPUT) != nil && defaults.bool(forKey: PREF_SPEAKER_OUTPUT)
        let preprocess = defaults.object(forKey: PREF_VOICEPROCESSINGIO) != nil && defaults.bool(forKey: PREF_VOICEPROCESSINGIO)
        let a2dp = defaults.object(forKey: PREF_BLUETOOTH_A2DP) != nil && defaults.bool(forKey: PREF_BLUETOOTH_A2DP)
        let headsettoggle = defaults.object(forKey: PREF_HEADSET_TXTOGGLE) != nil && defaults.bool(forKey: PREF_HEADSET_TXTOGGLE)
                
        // In 'voiceChat' mode stereo cannot be enabled on input devices.
        try session.setMode(preprocess ? .voiceChat : .default)

        var catoptions : AVAudioSession.CategoryOptions
        
        // Toggling 'speaker' on iPad has no effect since it can only output to speaker.
        // When Bluetooth headset is connected to iPad then toggling 'speaker' will have
        // no effect. However, on iPhone toggling 'speaker' has the desired effect both
        // when switching output from Receiver and Bluetooth to 'speaker'.
        if speaker {
            catoptions = [ .defaultToSpeaker ]
        }
        else {
            // Use .allowBluetooth for iOS 12.0+ (replaces deprecated .allowBluetoothHFP)
            catoptions = [ .allowBluetooth, .allowAirPlay, .allowBluetoothA2DP ]
            if #available(iOS 26.0, *) {
                catoptions.update(with: .bluetoothHighQualityRecording)
            }
            if a2dp {
                catoptions.remove(.allowBluetooth)
            }
        }
        // headset notifications, UIApplication.shared.beginReceivingRemoteControlEvents(),
        // will be ignored with .mixWithOthers
        if headsettoggle == false {
            catoptions.update(with: .mixWithOthers)
        }
        
        try session.setCategory(.playAndRecord, options: catoptions)

        // Note that Voice Preprocessing IO will disable ability to select
        // stereo microphone sources
        let sndid = preprocess ? TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO : TT_SOUNDDEVICE_ID_REMOTEIO
        if TT_InitSoundInputDevice(ttInst, sndid) == FALSE {
            print("Failed to initialize sound input device: \(sndid)")
        }
        else {
            print("Using sound input device: \(sndid)")
        }
        if TT_InitSoundOutputDevice(ttInst, sndid) == FALSE {
            print("Failed to initialize sound output device: \(sndid)")
        }
        else {
            print("Using sound output device: \(sndid)")
        }
        print("postset. Mode \(session.mode.rawValue), category \(session.category.rawValue), options \(getCategory(session.categoryOptions))")
        
        // enable stereo on all data sources that support it
        if #available(iOS 14.0, *) {
            if let availableInputs = session.availableInputs {
                for input in availableInputs {
                    // enable data source chosen by user (if any)
                    if let dataSourceID = getAudioPortDataSource(descr: input) {
                        if let dataSources = input.dataSources {
                            for datasrc in dataSources {
                                // enable stereo on selected audio input
                                if datasrc.dataSourceID == dataSourceID {
                                    if datasrc.supportedPolarPatterns != nil && datasrc.supportedPolarPatterns!.contains(.stereo) {
                                        try datasrc.setPreferredPolarPattern(.stereo)
                                        print("Setting \(datasrc.dataSourceName) to stereo")
                                    } else {
                                        print("No stereo on \(datasrc.dataSourceName)")
                                    }
                                }
                                // switch to selected audio input
                                if session.inputDataSource?.dataSourceID != dataSourceID {
                                    try input.setPreferredDataSource(datasrc)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch {
        print("Failed to set mode")
    }
}

func playSound(_ s: Sounds) {
    
    let filename = getSoundFile(s)
    
    if filename == nil {
        return
    }
    
    if let resPath = Bundle.main.path(forResource: filename, ofType: "mp3") {
        
        let url = URL(fileURLWithPath: resPath)
        
        do {
            player = try AVAudioPlayer(contentsOf: url)
            player!.prepareToPlay()
            player!.play()
        }
        catch {
            print("Failed to play")
        }
    }
}

