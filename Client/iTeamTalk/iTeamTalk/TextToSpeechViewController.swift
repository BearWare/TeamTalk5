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

enum TTSEvents : Int {
    case USERLOGIN,
    USERLOGOUT,
    USERJOINED,
    USERLEFT,
    USERTEXTMSG,
    USERCHANMSG,
    CONLOST
    
}

class TextToSpeechViewController : UITableViewController {
    
    var ttsevents_items = [UITableViewCell]()

    override func viewDidLoad() {
        super.viewDidLoad()
        
        let settings = UserDefaults.standard

        let ttsuserlogincell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let ttsuserlogin = settings.object(forKey: PREF_TTSEVENT_USERLOGIN) != nil && settings.bool(forKey: PREF_TTSEVENT_USERLOGIN)
        let ttsuserloginswitch = newTableCellSwitch(ttsuserlogincell, label: NSLocalizedString("User logged in", comment: "preferences"), initial: ttsuserlogin, tag: TTSEvents.USERLOGIN.rawValue)
        ttsuserlogincell.detailTextLabel!.text = NSLocalizedString("Announce user logged onto server", comment: "preferences")
        ttsuserloginswitch.addTarget(self, action: #selector(TextToSpeechViewController.ttsSwitchChanged(_:)), for: .valueChanged)
        ttsevents_items.append(ttsuserlogincell)

        let ttsuserlogoutcell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let ttsuserlogout = settings.object(forKey: PREF_TTSEVENT_USERLOGOUT) != nil && settings.bool(forKey: PREF_TTSEVENT_USERLOGOUT)
        let ttsuserlogoutswitch = newTableCellSwitch(ttsuserlogoutcell, label: NSLocalizedString("User logged out", comment: "preferences"), initial: ttsuserlogout, tag: TTSEvents.USERLOGOUT.rawValue)
        ttsuserlogoutcell.detailTextLabel!.text = NSLocalizedString("Announce user logged out of server", comment: "preferences")
        ttsuserlogoutswitch.addTarget(self, action: #selector(TextToSpeechViewController.ttsSwitchChanged(_:)), for: .valueChanged)
        ttsevents_items.append(ttsuserlogoutcell)

        let ttsjoinedchancell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let ttsjoinedchan = settings.object(forKey: PREF_TTSEVENT_JOINEDCHAN) == nil || settings.bool(forKey: PREF_TTSEVENT_JOINEDCHAN)
        let ttsjoinedchanswitch = newTableCellSwitch(ttsjoinedchancell, label: NSLocalizedString("User joins channel", comment: "preferences"), initial: ttsjoinedchan, tag: TTSEvents.USERJOINED.rawValue)
        ttsjoinedchancell.detailTextLabel!.text = NSLocalizedString("Announce user joining channel", comment: "preferences")
        ttsjoinedchanswitch.addTarget(self, action: #selector(TextToSpeechViewController.ttsSwitchChanged(_:)), for: .valueChanged)
        ttsevents_items.append(ttsjoinedchancell)
        
        let ttsleftchancell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let ttsleftchan = settings.object(forKey: PREF_TTSEVENT_LEFTCHAN) == nil || settings.bool(forKey: PREF_TTSEVENT_LEFTCHAN)
        let ttsleftchanswitch = newTableCellSwitch(ttsleftchancell, label: NSLocalizedString("User leaves channel", comment: "preferences"), initial: ttsleftchan, tag: TTSEvents.USERLEFT.rawValue)
        ttsleftchancell.detailTextLabel!.text = NSLocalizedString("Announce user leaving channel", comment: "preferences")
        ttsleftchanswitch.addTarget(self, action: #selector(TextToSpeechViewController.ttsSwitchChanged(_:)), for: .valueChanged)
        ttsevents_items.append(ttsleftchancell)
        
        let ttsconlostcell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let ttsconlost = settings.object(forKey: PREF_TTSEVENT_CONLOST) == nil || settings.bool(forKey: PREF_TTSEVENT_CONLOST)
        let ttsconlostswitch = newTableCellSwitch(ttsconlostcell, label: NSLocalizedString("Connection lost", comment: "preferences"), initial: ttsconlost, tag: TTSEvents.CONLOST.rawValue)
        ttsconlostcell.detailTextLabel!.text = NSLocalizedString("Announce lost server connection", comment: "preferences")
        ttsconlostswitch.addTarget(self, action: #selector(TextToSpeechViewController.ttsSwitchChanged(_:)), for: .valueChanged)
        ttsevents_items.append(ttsconlostcell)
        
        let ttstxtmsgcell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let ttstxtmsg = settings.object(forKey: PREF_TTSEVENT_TEXTMSG) != nil && settings.bool(forKey: PREF_TTSEVENT_TEXTMSG)
        let ttstxtmsgswitch = newTableCellSwitch(ttstxtmsgcell, label: NSLocalizedString("Private Text Message", comment: "preferences"), initial: ttstxtmsg, tag: TTSEvents.USERTEXTMSG.rawValue)
        ttstxtmsgcell.detailTextLabel!.text = NSLocalizedString("Announce content of text message", comment: "preferences")
        ttstxtmsgswitch.addTarget(self, action: #selector(TextToSpeechViewController.ttsSwitchChanged(_:)), for: .valueChanged)
        ttsevents_items.append(ttstxtmsgcell)
        
        let ttschantxtmsgcell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let ttschantxtmsg = settings.object(forKey: PREF_TTSEVENT_CHANTEXTMSG) != nil && settings.bool(forKey: PREF_TTSEVENT_CHANTEXTMSG)
        let ttschantxtmsgswitch = newTableCellSwitch(ttschantxtmsgcell, label: NSLocalizedString("Channel Text Message", comment: "preferences"), initial: ttschantxtmsg, tag: TTSEvents.USERCHANMSG.rawValue)
        ttschantxtmsgcell.detailTextLabel!.text = NSLocalizedString("Announce content of text message", comment: "preferences")
        ttschantxtmsgswitch.addTarget(self, action: #selector(TextToSpeechViewController.ttsSwitchChanged(_:)), for: .valueChanged)
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

    @objc func ttsSwitchChanged(_ sender: UISwitch) {
        let defaults = UserDefaults.standard
        let s = TTSEvents(rawValue: sender.tag)!
        switch s {
        case .USERLOGIN :
            defaults.set(sender.isOn, forKey: PREF_TTSEVENT_USERLOGIN)
        case .USERLOGOUT :
            defaults.set(sender.isOn, forKey: PREF_TTSEVENT_USERLOGOUT)
        case .USERJOINED :
            defaults.set(sender.isOn, forKey: PREF_TTSEVENT_JOINEDCHAN)
        case .USERLEFT :
            defaults.set(sender.isOn, forKey: PREF_TTSEVENT_LEFTCHAN)
        case .USERTEXTMSG :
            defaults.set(sender.isOn, forKey: PREF_TTSEVENT_TEXTMSG)
        case .USERCHANMSG :
            defaults.set(sender.isOn, forKey: PREF_TTSEVENT_CHANTEXTMSG)
        case .CONLOST :
            defaults.set(sender.isOn, forKey: PREF_TTSEVENT_CONLOST)
        }
    }
}
