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

class SoundEventsViewController : UITableViewController {
    
    var soundevents_items = [UITableViewCell]()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let srvlostcell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let srvlostswitch = newTableCellSwitch(srvlostcell, label: NSLocalizedString("Server Connection Lost", comment: "preferences"), initial: getSoundFile(.srv_LOST) != nil)
        srvlostcell.detailTextLabel!.text = NSLocalizedString("Play sound when connection is dropped", comment: "preferences")
        srvlostswitch.tag = Sounds.srv_LOST.rawValue
        srvlostswitch.addTarget(self, action: #selector(SoundEventsViewController.soundeventChanged(_:)), for: .valueChanged)
        soundeventChanged(srvlostswitch)
        soundevents_items.append(srvlostcell)
        
        let voicetxcell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let voicetxswitch = newTableCellSwitch(voicetxcell, label: NSLocalizedString("Voice Transmission Toggled", comment: "preferences"), initial: getSoundFile(.tx_ON) != nil)
        voicetxcell.detailTextLabel!.text = NSLocalizedString("Play sound when voice transmission is toggled", comment: "preferences")
        voicetxswitch.tag = Sounds.tx_ON.rawValue
        voicetxswitch.addTarget(self, action: #selector(SoundEventsViewController.soundeventChanged(_:)), for: .valueChanged)
        soundeventChanged(voicetxswitch)
        soundevents_items.append(voicetxcell)
        
        let usermsgcell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let usermsgswitch = newTableCellSwitch(usermsgcell, label: NSLocalizedString("Private Text Message", comment: "preferences"), initial: getSoundFile(.user_MSG) != nil)
        usermsgcell.detailTextLabel!.text = NSLocalizedString("Play sound when private text message is received", comment: "preferences")
        usermsgswitch.tag = Sounds.user_MSG.rawValue
        usermsgswitch.addTarget(self, action: #selector(SoundEventsViewController.soundeventChanged(_:)), for: .valueChanged)
        soundeventChanged(usermsgswitch)
        soundevents_items.append(usermsgcell)
        
        let chanmsgcell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let chanmsgswitch = newTableCellSwitch(chanmsgcell, label: NSLocalizedString("Channel Text Message", comment: "preferences"), initial: getSoundFile(.chan_MSG) != nil)
        chanmsgcell.detailTextLabel!.text = NSLocalizedString("Play sound when channel text message is received", comment: "preferences")
        chanmsgswitch.tag = Sounds.chan_MSG.rawValue
        chanmsgswitch.addTarget(self, action: #selector(SoundEventsViewController.soundeventChanged(_:)), for: .valueChanged)
        soundeventChanged(chanmsgswitch)
        soundevents_items.append(chanmsgcell)
        
        let joinedchancell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let joinedchanswitch = newTableCellSwitch(joinedchancell, label: NSLocalizedString("User Joins Channel", comment: "preferences"), initial: getSoundFile(.joined_CHAN) != nil)
        joinedchancell.detailTextLabel!.text = NSLocalizedString("Play sound when a user joins the channel", comment: "preferences")
        joinedchanswitch.tag = Sounds.joined_CHAN.rawValue
        joinedchanswitch.addTarget(self, action: #selector(SoundEventsViewController.soundeventChanged(_:)), for: .valueChanged)
        soundeventChanged(joinedchanswitch)
        soundevents_items.append(joinedchancell)
        
        let leftchancell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let leftchanswitch = newTableCellSwitch(leftchancell, label: NSLocalizedString("User Leaves Channel", comment: "preferences"), initial: getSoundFile(.left_CHAN) != nil)
        leftchancell.detailTextLabel!.text = NSLocalizedString("Play sound when a user leaves the channel", comment: "preferences")
        leftchanswitch.tag = Sounds.left_CHAN.rawValue
        leftchanswitch.addTarget(self, action: #selector(SoundEventsViewController.soundeventChanged(_:)), for: .valueChanged)
        soundeventChanged(leftchanswitch)
        soundevents_items.append(leftchancell)
        
        let voxtriggercell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let voxtriggerswitch = newTableCellSwitch(voxtriggercell, label: NSLocalizedString("Voice Activation Triggered", comment: "preferences"), initial: getSoundFile(.voxtriggered_ON) != nil)
        voxtriggercell.detailTextLabel!.text = NSLocalizedString("Play sound when voice activation is triggered", comment: "preferences")
        voxtriggerswitch.tag = Sounds.voxtriggered_ON.rawValue
        voxtriggerswitch.addTarget(self, action: #selector(SoundEventsViewController.soundeventChanged(_:)), for: .valueChanged)
        soundeventChanged(voxtriggerswitch)
        soundevents_items.append(voxtriggercell)
        
        let transmitcell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        let transmitswitch = newTableCellSwitch(transmitcell, label: NSLocalizedString("Exclusive Mode Toggled", comment: "preferences"), initial: getSoundFile(.transmit_ON) != nil)
        transmitcell.detailTextLabel!.text = NSLocalizedString("Play sound when transmit ready in \"No Interruptions\" channel", comment: "preferences")
        transmitswitch.tag = Sounds.transmit_ON.rawValue
        transmitswitch.addTarget(self, action: #selector(SoundEventsViewController.soundeventChanged(_:)), for: .valueChanged)
        soundeventChanged(transmitswitch)
        soundevents_items.append(transmitcell)
    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }
    
    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        return NSLocalizedString("Sound Events", comment:"text to speech")
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return soundevents_items.count
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        
        return soundevents_items[indexPath.row]
    }
    
    @objc func soundeventChanged(_ sender: UISwitch) {
        
        let defaults = UserDefaults.standard
        
        switch sender.tag {
        case Sounds.tx_ON.rawValue :
            defaults.set(sender.isOn, forKey: PREF_SNDEVENT_VOICETX)
        case Sounds.srv_LOST.rawValue :
            defaults.set(sender.isOn, forKey: PREF_SNDEVENT_SERVERLOST)
        case Sounds.chan_MSG.rawValue :
            defaults.set(sender.isOn, forKey: PREF_SNDEVENT_CHANMSG)
        case Sounds.joined_CHAN.rawValue :
            defaults.set(sender.isOn, forKey: PREF_SNDEVENT_JOINEDCHAN)
            
        case Sounds.left_CHAN.rawValue :
            defaults.set(sender.isOn, forKey: PREF_SNDEVENT_LEFTCHAN)
        case Sounds.user_MSG.rawValue :
            defaults.set(sender.isOn, forKey: PREF_SNDEVENT_USERMSG)
        case Sounds.voxtriggered_ON.rawValue :
            defaults.set(sender.isOn, forKey: PREF_SNDEVENT_VOXTRIGGER)
        case Sounds.transmit_ON.rawValue :
            defaults.set(sender.isOn, forKey: PREF_SNDEVENT_TRANSMITREADY)
        default :
            break
        }
    }
}
