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
                if let datasources = inputs[indexPath.section].dataSources {
                    if indexPath.row < datasources.count {
                        var srcName = datasources[indexPath.row].dataSourceName
                        if #available(iOS 14.0, *) {
                            if datasources[indexPath.row].supportedPolarPatterns != nil && datasources[indexPath.row].supportedPolarPatterns!.contains(.stereo) {
                                srcName += " (Stereo)"
                            }
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
                    if let datasources = inputs[indexPath.section].dataSources {
                        if indexPath.row < datasources.count {
                            print ("UID of \(inputs[indexPath.section].portName): \(inputs[indexPath.section].uid)");
                            print ("Data source ID: \(datasources[indexPath.row].dataSourceID)")
                            try inputs[indexPath.section].setPreferredDataSource(datasources[indexPath.row])
                            try session.setPreferredInput(inputs[indexPath.section])
                            try session.setInputDataSource(datasources[indexPath.row])
                        }
                        else {
                            try session.setPreferredInput(inputs[indexPath.section])
                        }
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

