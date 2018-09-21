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
import AVFoundation

class TextToSpeechViewController : UITableViewController {
    
    var ttsevents_items = [UITableViewCell]()

    override func viewDidLoad() {
        super.viewDidLoad()
        
        let settings = UserDefaults.standard
        
        let ttsjoinedchancell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let ttsjoinedchan = settings.object(forKey: PREF_TTSEVENT_JOINEDCHAN) == nil || settings.bool(forKey: PREF_TTSEVENT_JOINEDCHAN)
        let ttsjoinedchanswitch = newTableCellSwitch(ttsjoinedchancell, label: NSLocalizedString("User joins channel", comment: "preferences"), initial: ttsjoinedchan)
        ttsjoinedchancell.detailTextLabel!.text = NSLocalizedString("Announce user joining channel", comment: "preferences")
        ttsjoinedchanswitch.addTarget(self, action: #selector(TextToSpeechViewController.ttsjoinedchanChanged(_:)), for: .valueChanged)
        ttsevents_items.append(ttsjoinedchancell)
        
        let ttsleftchancell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let ttsleftchan = settings.object(forKey: PREF_TTSEVENT_LEFTCHAN) == nil || settings.bool(forKey: PREF_TTSEVENT_LEFTCHAN)
        let ttsleftchanswitch = newTableCellSwitch(ttsleftchancell, label: NSLocalizedString("User leaves channel", comment: "preferences"), initial: ttsleftchan)
        ttsleftchancell.detailTextLabel!.text = NSLocalizedString("Announce user leaving channel", comment: "preferences")
        ttsleftchanswitch.addTarget(self, action: #selector(TextToSpeechViewController.ttsleftchanChanged(_:)), for: .valueChanged)
        ttsevents_items.append(ttsleftchancell)
        
        let ttsconlostcell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let ttsconlost = settings.object(forKey: PREF_TTSEVENT_CONLOST) == nil || settings.bool(forKey: PREF_TTSEVENT_CONLOST)
        let ttsconlostswitch = newTableCellSwitch(ttsconlostcell, label: NSLocalizedString("Connection lost", comment: "preferences"), initial: ttsconlost)
        ttsconlostcell.detailTextLabel!.text = NSLocalizedString("Announce lost server connection", comment: "preferences")
        ttsconlostswitch.addTarget(self, action: #selector(TextToSpeechViewController.ttsconlostChanged(_:)), for: .valueChanged)
        ttsevents_items.append(ttsconlostcell)
        
        let ttstxtmsgcell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let ttstxtmsg = settings.object(forKey: PREF_TTSEVENT_TEXTMSG) != nil && settings.bool(forKey: PREF_TTSEVENT_TEXTMSG)
        let ttstxtmsgswitch = newTableCellSwitch(ttstxtmsgcell, label: NSLocalizedString("Private Text Message", comment: "preferences"), initial: ttstxtmsg)
        ttstxtmsgcell.detailTextLabel!.text = NSLocalizedString("Announce content of text message", comment: "preferences")
        ttstxtmsgswitch.addTarget(self, action: #selector(TextToSpeechViewController.ttsprivtxtmsgChanged(_:)), for: .valueChanged)
        ttsevents_items.append(ttstxtmsgcell)
        
        let ttschantxtmsgcell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let ttschantxtmsg = settings.object(forKey: PREF_TTSEVENT_CHANTEXTMSG) != nil && settings.bool(forKey: PREF_TTSEVENT_CHANTEXTMSG)
        let ttschantxtmsgswitch = newTableCellSwitch(ttschantxtmsgcell, label: NSLocalizedString("Channel Text Message", comment: "preferences"), initial: ttschantxtmsg)
        ttschantxtmsgcell.detailTextLabel!.text = NSLocalizedString("Announce content of text message", comment: "preferences")
        ttschantxtmsgswitch.addTarget(self, action: #selector(TextToSpeechViewController.ttschantxtmsgChanged(_:)), for: .valueChanged)
        ttsevents_items.append(ttschantxtmsgcell)

    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }
    
    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        return NSLocalizedString("Announcements", comment:"text to speech")
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return ttsevents_items.count
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        
        return ttsevents_items[indexPath.row]
    }
    
    @objc func ttsjoinedchanChanged(_ sender: UISwitch) {
        let defaults = UserDefaults.standard
        defaults.set(sender.isOn, forKey: PREF_TTSEVENT_JOINEDCHAN)
    }
    
    @objc func ttsleftchanChanged(_ sender: UISwitch) {
        let defaults = UserDefaults.standard
        defaults.set(sender.isOn, forKey: PREF_TTSEVENT_LEFTCHAN)
    }
    
    @objc func ttsconlostChanged(_ sender: UISwitch) {
        let defaults = UserDefaults.standard
        defaults.set(sender.isOn, forKey: PREF_TTSEVENT_CONLOST)
    }
    
    @objc func ttsprivtxtmsgChanged(_ sender: UISwitch) {
        let defaults = UserDefaults.standard
        defaults.set(sender.isOn, forKey: PREF_TTSEVENT_TEXTMSG)
    }
    
    @objc func ttschantxtmsgChanged(_ sender: UISwitch) {
        let defaults = UserDefaults.standard
        defaults.set(sender.isOn, forKey: PREF_TTSEVENT_CHANTEXTMSG)
    }
}
