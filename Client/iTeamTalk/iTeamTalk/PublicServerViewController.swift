//
//  PublicServerViewController.swift
//  iTeamTalk
//
//  Created by Bjørn Damstedt Rasmussen on 07/03/2022.
//  Copyright © 2022 BearWare.dk. All rights reserved.
//

import UIKit
import Foundation

class PublicServerViewController : UITableViewController {

    let SECTION_PUBLICSERVERS = 0,
        SECTION_SORTING = 1,
        SECTIONS_COUNT = 1

    var publicserver_items = [UITableViewCell]()
    
    override func viewDidLoad() {
        super.viewDidLoad()
                
        let officialservercell = tableView.dequeueReusableCell(withIdentifier: "ShowOfficialServers")
        let officialserverswitch = UISwitch()
        officialserverswitch.addTarget(self, action: #selector(handleSwitchActionOfficial), for: .valueChanged)
        officialservercell?.accessoryView = officialserverswitch
        publicserver_items.append(officialservercell!)

        let publicservercell = tableView.dequeueReusableCell(withIdentifier: "ShowPublicServers")
        let publicserverswitch = UISwitch()
        publicserverswitch.addTarget(self, action: #selector(handleSwitchActionPublic), for: .valueChanged)
        publicservercell?.accessoryView = publicserverswitch
        publicserver_items.append(publicservercell!)

        let unofficialservercell = tableView.dequeueReusableCell(withIdentifier: "ShowUnofficialServers")
        let unofficialserverswitch = UISwitch()
        unofficialserverswitch.addTarget(self, action: #selector(handleSwitchActionUnofficial), for: .valueChanged)
        unofficialservercell?.accessoryView = unofficialserverswitch
        publicserver_items.append(unofficialservercell!)

        let settings = UserDefaults.standard
        officialserverswitch.isOn = settings.object(forKey: PREF_DISPLAY_OFFICIALSERVERS) == nil || settings.bool(forKey: PREF_DISPLAY_OFFICIALSERVERS)
        publicserverswitch.isOn = settings.object(forKey: PREF_DISPLAY_PUBLICSERVERS) == nil || settings.bool(forKey: PREF_DISPLAY_PUBLICSERVERS)
        unofficialserverswitch.isOn = settings.object(forKey: PREF_DISPLAY_UNOFFICIALSERVERS) != nil && settings.bool(forKey: PREF_DISPLAY_UNOFFICIALSERVERS)
    }
    
    @objc func handleSwitchActionOfficial(sender: UISwitch) {
        let settings = UserDefaults.standard
        settings.set(sender.isOn, forKey: PREF_DISPLAY_OFFICIALSERVERS)
    }

    @objc func handleSwitchActionPublic(sender: UISwitch) {
        let settings = UserDefaults.standard
        settings.set(sender.isOn, forKey: PREF_DISPLAY_PUBLICSERVERS)
    }

    @objc func handleSwitchActionUnofficial(sender: UISwitch) {
        let settings = UserDefaults.standard
        settings.set(sender.isOn, forKey: PREF_DISPLAY_UNOFFICIALSERVERS)
    }

    override func numberOfSections(in tableView: UITableView) -> Int {
        return SECTIONS_COUNT
    }
    
    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch section {
        case SECTION_PUBLICSERVERS :
            return NSLocalizedString("Show in Server List", comment: "preferences")
        default : return ""
        }
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch section {
        case SECTION_PUBLICSERVERS :
            return publicserver_items.count
        default : return 0
        }
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        switch indexPath.section {
        case SECTION_PUBLICSERVERS :
            return publicserver_items[indexPath.row]
        default : return UITableViewCell()
        }
    }
}
