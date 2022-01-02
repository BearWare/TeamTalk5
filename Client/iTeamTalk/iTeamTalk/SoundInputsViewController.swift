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

class SoundInputsViewController : UITableViewController {

    override func viewDidLoad() {
        super.viewDidLoad()
        
        let center = NotificationCenter.default
        center.addObserver(self, selector: #selector(SoundInputsViewController.audioRouteChange(_:)), name: AVAudioSession.routeChangeNotification, object: nil)
    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        let session = AVAudioSession.sharedInstance()
        if let inputs = session.availableInputs {
            return inputs.count
        }

        return 0
    }

    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        let session = AVAudioSession.sharedInstance()
        if let inputs = session.availableInputs {
            if section < inputs.count {
                return inputs[section].portName
            }
        }
        return ""
    }

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        let session = AVAudioSession.sharedInstance()
        if let inputs = session.availableInputs {
            if section < inputs.count {
                if let datasources = inputs[section].dataSources {
                    return max(datasources.count, 1)
                }
            }
        }
        return 0
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "SoundInput")
        let session = AVAudioSession.sharedInstance()
        if let inputs = session.availableInputs {
            if indexPath.section < inputs.count {
                let audioinput = inputs[indexPath.section]
                if let datasources = audioinput.dataSources {
                    if indexPath.row < datasources.count {
                        let audiodatasource = datasources[indexPath.row]
                        var srcName = audiodatasource.dataSourceName
                        if #available(iOS 14.0, *) {
                            if audiodatasource.supportedPolarPatterns != nil && audiodatasource.supportedPolarPatterns!.contains(.stereo) {
                                srcName += " (" + NSLocalizedString("Stereo", comment: "Sound Input") + ")"
                            }
                        }
                        
                        if getAudioPortDataSource(descr: audioinput) == audiodatasource.dataSourceID {
                            srcName += ", " + NSLocalizedString("Preferred", comment: "Sound Input")
                        }
                        
                        if session.inputDataSource?.dataSourceID == audiodatasource.dataSourceID {
                            srcName += ", " + NSLocalizedString("Active", comment: "Sound Input")
                        }

                        cell!.textLabel?.text = srcName
                    }
                    else {
                        cell!.textLabel?.text = NSLocalizedString("Default", comment: "Sound Input")
                    }
                }
            }
        }
        return cell!
    }

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {

        let session = AVAudioSession.sharedInstance()
        
        do {
            if let inputs = session.availableInputs {
                if indexPath.section < inputs.count {
                    
                    let audioinput = inputs[indexPath.section]
                    
                    if let datasources = audioinput.dataSources {
                        if indexPath.row < datasources.count {
                            let audiodatasource = datasources[indexPath.row]
                            print ("UID of \(audioinput.portName): \(audioinput.uid)");
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
                    
                }
            }
            print (session.currentRoute)
        }
        catch {
            print("Failed to select data source")
        }
    }
    
    @objc func audioRouteChange(_ notification: Notification) {
        print("Audio Route changed in Sound Inputs table")
        self.tableView.reloadData()
    }
}

