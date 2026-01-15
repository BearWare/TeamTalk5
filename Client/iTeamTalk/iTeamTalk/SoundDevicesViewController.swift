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

import UIKit
import Foundation
import AVFoundation

class SoundDevicesViewController : UITableViewController {

    let SECTION_GENERAL     = 0,
        SECTION_COUNT_MIN   = 1
    
    var sound_items = [UITableViewCell]()

    override func viewDidLoad() {
        // preferred device cannot be changed unless sound system is activated
        if TT_GetFlags(ttInst) & CLIENT_SNDINPUT_READY.rawValue == 0 {
            setupSoundDevices()
        }
        
        super.viewDidLoad()
        
        let settings = UserDefaults.standard
        
        let speakercell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let speakerswitch = newTableCellSwitch(speakercell, label: NSLocalizedString("Speaker Output", comment: "preferences"),
            initial: settings.object(forKey: PREF_SPEAKER_OUTPUT) != nil && settings.bool(forKey: PREF_SPEAKER_OUTPUT))
        speakercell.detailTextLabel!.text = NSLocalizedString("Use iPhone's speaker instead of earpiece", comment: "preferences")
        speakerswitch.addTarget(self, action: #selector(SoundDevicesViewController.speakeroutputChanged(_:)), for: .valueChanged)
        sound_items.append(speakercell)
        
        let voice_prepcell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let voiceprepswitch = newTableCellSwitch(voice_prepcell, label: NSLocalizedString("Voice Preprocessing", comment: "preferences"),
            initial: settings.object(forKey: PREF_VOICEPROCESSINGIO) != nil && settings.bool(forKey: PREF_VOICEPROCESSINGIO))
        voice_prepcell.detailTextLabel!.text = NSLocalizedString("Use echo cancellation and automatic gain control",
                                                                 comment: "Sound Devices")
        voiceprepswitch.addTarget(self, action: #selector(SoundDevicesViewController.voicepreprocessingChanged(_:)), for: .valueChanged)
        sound_items.append(voice_prepcell)

        let a2dpcell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let a2dpswitch = newTableCellSwitch(a2dpcell, label: NSLocalizedString("Bluetooth A2DP Playback", comment: "Sound Devices"),
                                            initial: settings.object(forKey: PREF_BLUETOOTH_A2DP) != nil && settings.bool(forKey: PREF_BLUETOOTH_A2DP))
        a2dpcell.detailTextLabel!.text = NSLocalizedString("Bluetooth playback should use Advanced Audio Distribution Profile",
                                                           comment: "Sound Devices")
        a2dpswitch.addTarget(self, action: #selector(SoundDevicesViewController.bluetoothA2DPChanged(_:)), for: .valueChanged)
        sound_items.append(a2dpcell)

        let center = NotificationCenter.default
        center.addObserver(self, selector: #selector(SoundDevicesViewController.audioRouteChange(_:)), name: AVAudioSession.routeChangeNotification, object: nil)
    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        let session = AVAudioSession.sharedInstance()
        if let inputs = session.availableInputs {
            return inputs.count + SECTION_COUNT_MIN
        }

        return SECTION_COUNT_MIN
    }

    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        
        switch section {
        case SECTION_GENERAL :
            return NSLocalizedString("General", comment: "Sound Devices")
        default :
            let audioPortIndex = section - SECTION_COUNT_MIN
            let session = AVAudioSession.sharedInstance()
            if let inputs = session.availableInputs {
                if audioPortIndex < inputs.count {
                    return inputs[audioPortIndex].portName
                }
            }
        }
        return ""
    }

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch section {
        case SECTION_GENERAL :
            return sound_items.count
        default :
            let audioPortIndex = section - SECTION_COUNT_MIN
            return getAudioInputDataSourcesCount(audioPortIndex: audioPortIndex)
        }
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        switch indexPath.section {
        case SECTION_GENERAL :
            return sound_items[indexPath.row]
        default :
            let audioPortIndex = indexPath.section - SECTION_COUNT_MIN
            return createAudioInputTableCell(audioPortIndex: audioPortIndex, dataSourceIndex: indexPath.row)
        }
    }

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        switch indexPath.section {
        case SECTION_GENERAL :
            break
        default :
            let audioPortIndex = indexPath.section - SECTION_COUNT_MIN
            
            do {
                let session = AVAudioSession.sharedInstance()
                if let inputs = session.availableInputs {
                    if audioPortIndex < inputs.count {
                        
                        let audioinput = inputs[audioPortIndex]
                        print ("UID of \(audioinput.portName): \(audioinput.uid)");
                        if let datasources = audioinput.dataSources {
                            if indexPath.row < datasources.count {
                                let audiodatasource = datasources[indexPath.row]
                                print ("Data source ID: \(audiodatasource.dataSourceID)")
                                try audioinput.setPreferredDataSource(audiodatasource)
                                try session.setPreferredInput(audioinput)
                                try session.setInputDataSource(audiodatasource)
                                setAudioPortDataSource(descr: audioinput, dsrc: audiodatasource)
                            }
                            else {
                                try session.setPreferredInput(audioinput)
                                removeAudioPortDataSource(descr: audioinput)
                            }
                        } // datasources
                        else {
                            print ("\(audioinput.portName) has no audio sources")
                        }
                    }
                }
                print (session.currentRoute)
            }
            catch {
                print("Failed to select data source")
            }
        }
    }
    
    func createAudioInputTableCell(audioPortIndex: Int, dataSourceIndex: Int) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "SoundDevices")
        let session = AVAudioSession.sharedInstance()
        if let inputs = session.availableInputs {
            if audioPortIndex < inputs.count {
                let audioinput = inputs[audioPortIndex]
                if let datasources = audioinput.dataSources {
                    if dataSourceIndex < datasources.count {
                        let audiodatasource = datasources[dataSourceIndex]
                        var srcName = audiodatasource.dataSourceName
                        if #available(iOS 14.0, *) {
                            if audiodatasource.supportedPolarPatterns != nil && audiodatasource.supportedPolarPatterns!.contains(.stereo) {
                                srcName += " (" + NSLocalizedString("Stereo", comment: "Sound Devices") + ")"
                            }
                        }
                        
                        if getAudioPortDataSource(descr: audioinput) == audiodatasource.dataSourceID {
                            srcName += ", " + NSLocalizedString("Preferred", comment: "Sound Devices")
                        }
                        
                        if session.inputDataSource?.dataSourceID == audiodatasource.dataSourceID {
                            srcName += ", " + NSLocalizedString("Active", comment: "Sound Devices")
                        }

                        cell!.textLabel?.text = srcName
                    }
                    else {
                        cell!.textLabel?.text = NSLocalizedString("Default", comment: "Sound Devices")
                    }
                }
            }
        }
        return cell!
    }
    
    func getAudioInputDataSourcesCount(audioPortIndex: Int) -> Int {
        let session = AVAudioSession.sharedInstance()
        if let inputs = session.availableInputs {
            if audioPortIndex < inputs.count {
                if let datasources = inputs[audioPortIndex].dataSources {
                    return max(datasources.count, 1)
                }
            }
        }
        return 0
    }

    @objc func audioRouteChange(_ notification: Notification) {
        print("Audio Route changed in Sound Devicess table")
        self.tableView.reloadData()
    }
    
    @objc func speakeroutputChanged(_ sender: UISwitch) {
        
        let defaults = UserDefaults.standard
        defaults.set(sender.isOn, forKey: PREF_SPEAKER_OUTPUT)
        
        setupSoundDevices()
    }

    @objc func voicepreprocessingChanged(_ sender: UISwitch) {
        
        let defaults = UserDefaults.standard
        defaults.set(sender.isOn, forKey: PREF_VOICEPROCESSINGIO)
        
        setupSoundDevices()
    }

    @objc func bluetoothA2DPChanged(_ sender: UISwitch) {
        
        let defaults = UserDefaults.standard
        defaults.set(sender.isOn, forKey: PREF_BLUETOOTH_A2DP)
        
        setupSoundDevices()
    }
}

