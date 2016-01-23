/*
* Copyright (c) 2005-2016, BearWare.dk
*
* Contact Information:
*
* Bjoern D. Rasmussen
* Skanderborgvej 40 4-2
* DK-8000 Aarhus C
* Denmark
* Email: contact@bearware.dk
* Phone: +45 20 20 54 59
* Web: http://www.bearware.dk
*
* This source code is part of the TeamTalk 5 SDK owned by
* BearWare.dk. All copyright statements may not be removed
* or altered from any source distribution. If you use this
* software in a product, an acknowledgment in the product
* documentation is required.
*
*/

import UIKit

class UserDetailViewController : UIViewController,
    UITableViewDataSource, UITableViewDelegate, TeamTalkEvent {
    
    @IBOutlet weak var navtitle: UINavigationItem!
    var usernamefield: UITextField?
    var voiceslider: UISlider?
    var voiceswitch: UISwitch?
    var mediaslider: UISlider?
    var mediaswitch: UISwitch?
    
    var subusermsgswitch: UISwitch?
    var subchanmsgswitch: UISwitch?
    var subbcastmsgswitch: UISwitch?
    var subvoiceswitch: UISwitch?
    var subwebcamswitch: UISwitch?
    var submediafileswitch: UISwitch?
    var subdesktopswitch: UISwitch?

    
    var ttInst = UnsafeMutablePointer<Void>()
    var userid : INT32 = 0, kick_cmdid : INT32 = 0, kickban_cmdid : INT32 = 0
    
    let SECTION_GENERAL = 0,
        SECTION_VOLUME = 1,
        SECTION_SUBSCRIPTIONS = 2,
        SECTION_ACTIONS = 3,
        SECTION_COUNT = 4
    
    @IBOutlet weak var tableView: UITableView!
    var general_items = [UITableViewCell]()
    var action_items = [UITableViewCell]()
    var volume_items = [UITableViewCell]()
    var subscription_items = [UITableViewCell]()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        var user = User()
        
        TT_GetUser(ttInst, userid, &user)
        navtitle.title = getDisplayName(user)
        
        addToTTMessages(self)
        
        // general items
        let usernamecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        usernamefield = newTableCellTextField(usernamecell, label: NSLocalizedString("Username", comment: "user detail"), initial: fromTTString(user.szUsername))
        general_items.append(usernamecell)
        
        // volume items
        let voicevolcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        voiceslider = newTableCellSlider(voicevolcell, label: NSLocalizedString("Voice Volume", comment: "user detail"), min: 0, max: 100, initial: Float(refVolumeToPercent(Int(user.nVolumeVoice))))
        voiceslider!.addTarget(self, action: "voiceVolumeChanged:", forControlEvents: .ValueChanged)
        volume_items.append(voicevolcell)
        
        let voicemutecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        voiceswitch = newTableCellSwitch(voicemutecell, label: NSLocalizedString("Mute Voice", comment: "user detail"), initial: (user.uUserState & USERSTATE_MUTE_VOICE.rawValue) != 0)
        voiceswitch!.addTarget(self, action: "muteVoice:", forControlEvents: .ValueChanged)
        volume_items.append(voicemutecell)

        let mediavolcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        mediaslider = newTableCellSlider(mediavolcell, label: NSLocalizedString("Media File Volume", comment: "user detail"), min: 0, max: 100, initial: Float(refVolumeToPercent(Int(user.nVolumeMediaFile))))
        mediaslider!.addTarget(self, action: "mediaVolumeChanged:", forControlEvents: .ValueChanged)
        volume_items.append(mediavolcell)
        
        let mediamutecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        mediaswitch = newTableCellSwitch(mediamutecell, label: NSLocalizedString("Mute Media File", comment: "user detail"), initial: (user.uUserState & USERSTATE_MUTE_MEDIAFILE.rawValue) != 0)
        mediaswitch!.addTarget(self, action: "muteMediaStream:", forControlEvents: .ValueChanged)
        volume_items.append(mediamutecell)
        
        // subscription items
        let subusermsgcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subusermsgswitch = newTableCellSwitch(subusermsgcell, label: NSLocalizedString("User Messages", comment: "user detail"), initial: (user.uLocalSubscriptions & SUBSCRIBE_USER_MSG.rawValue) != 0)
        subusermsgswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subusermsgcell)
        
        let subchanmsgcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subchanmsgswitch = newTableCellSwitch(subchanmsgcell, label: NSLocalizedString("Channel Messages", comment: "user detail"), initial: (user.uLocalSubscriptions & SUBSCRIBE_CHANNEL_MSG.rawValue) != 0)
        subchanmsgswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subchanmsgcell)

        let subbcastmsgcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subbcastmsgswitch = newTableCellSwitch(subbcastmsgcell, label: NSLocalizedString("Broadcast Messages", comment: "user detail"), initial: (user.uLocalSubscriptions & SUBSCRIBE_BROADCAST_MSG.rawValue) != 0)
        subbcastmsgswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subbcastmsgcell)

        let subvoicecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subvoiceswitch = newTableCellSwitch(subvoicecell, label: NSLocalizedString("Voice", comment: "user detail"), initial: (user.uLocalSubscriptions & SUBSCRIBE_VOICE.rawValue) != 0)
        subvoiceswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subvoicecell)
        
        let subwebcamcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subwebcamswitch = newTableCellSwitch(subwebcamcell, label: NSLocalizedString("WebCam", comment: "user detail"), initial: (user.uLocalSubscriptions & SUBSCRIBE_VIDEOCAPTURE.rawValue) != 0)
        subwebcamswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subwebcamcell)
        
        let submediafilecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        submediafileswitch = newTableCellSwitch(submediafilecell, label: NSLocalizedString("Media File", comment: "user detail"), initial: (user.uLocalSubscriptions & SUBSCRIBE_MEDIAFILE.rawValue) != 0)
        submediafileswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(submediafilecell)
        
        let subdesktopcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        subdesktopswitch = newTableCellSwitch(subdesktopcell, label: NSLocalizedString("Desktop", comment: "user detail"), initial: (user.uLocalSubscriptions & SUBSCRIBE_DESKTOP.rawValue) != 0)
        subdesktopswitch!.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subdesktopcell)

        // action items
        let kickusercell = tableView.dequeueReusableCellWithIdentifier("kickcell")
        action_items.append(kickusercell!)
        
        let kickbancell = tableView.dequeueReusableCellWithIdentifier("kickbancell")
        action_items.append(kickbancell!)

        //        let sendmsgcell = tableView.dequeueReusableCellWithIdentifier("sendmessagecell")
        //        action_items.append(sendmsgcell!)
        
        tableView.dataSource = self
        tableView.delegate = self

    }

    func voiceVolumeChanged(sender: UISlider) {
        TT_SetUserVolume(ttInst, userid, STREAMTYPE_VOICE, INT32(refVolume(Double(sender.value))))
    }
    
    func mediaVolumeChanged(sender: UISlider) {
        TT_SetUserVolume(ttInst, userid, STREAMTYPE_MEDIAFILE_AUDIO, INT32(refVolume(Double(sender.value))))
    }
    
    func muteVoice(sender: UISwitch) {
        TT_SetUserMute(ttInst, userid, STREAMTYPE_VOICE, (sender.on ? 1 : 0))
    }
    
    func muteMediaStream(sender: UISwitch) {
        TT_SetUserMute(ttInst, userid, STREAMTYPE_MEDIAFILE_AUDIO, (sender.on ? 1 : 0))
    }
    
    func subscriptionChanged(sender: UISwitch) {
        var sub = SUBSCRIBE_NONE
        
        switch sender {
        case subusermsgswitch! :
            sub = SUBSCRIBE_USER_MSG
        case subchanmsgswitch! :
            sub = SUBSCRIBE_CHANNEL_MSG
        case subbcastmsgswitch! :
            sub = SUBSCRIBE_BROADCAST_MSG
        case subvoiceswitch! :
            sub = SUBSCRIBE_VOICE
        case subwebcamswitch! :
            sub = SUBSCRIBE_VIDEOCAPTURE
        case submediafileswitch! :
            sub = SUBSCRIBE_MEDIAFILE
        case subdesktopswitch! :
            sub = SUBSCRIBE_DESKTOP
        default :
            break
        }
        
        if sender.on {
            TT_DoSubscribe(ttInst, userid, sub.rawValue)
        }
        else {
            TT_DoUnsubscribe(ttInst, userid, sub.rawValue)
        }
    }
    
    @IBAction func kickandbanUser(sender: UIButton) {
        var user = User()
        TT_GetUser(ttInst, userid, &user)
        
        kickban_cmdid = TT_DoKickUser(ttInst, userid, 0)
    }
    
    @IBAction func kickUser(sender: UIButton) {
        var user = User()
        TT_GetUser(ttInst, userid, &user)
        
        kick_cmdid = TT_DoKickUser(ttInst, userid, user.nChannelID)
    }
    
    func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return SECTION_COUNT
    }
    
    func tableView(tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch section {
        case SECTION_GENERAL :
            return NSLocalizedString("General", comment: "user detail")
        case SECTION_ACTIONS :
            return NSLocalizedString("Actions", comment: "user detail")
        case SECTION_VOLUME :
            return NSLocalizedString("Volume Controls", comment: "user detail")
        case SECTION_SUBSCRIPTIONS :
            return NSLocalizedString("Subscriptions", comment: "user detail")
        default :
            return nil
        }
    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch section {
        case SECTION_GENERAL :
            return general_items.count
        case SECTION_ACTIONS :
            return action_items.count
        case SECTION_VOLUME :
            return volume_items.count
        case SECTION_SUBSCRIPTIONS :
            return subscription_items.count
        default : return 0
        }
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        switch indexPath.section {
        case SECTION_GENERAL :
            return general_items[indexPath.row]
        case SECTION_ACTIONS :
            return action_items[indexPath.row]
        case SECTION_VOLUME :
            return volume_items[indexPath.row]
        case SECTION_SUBSCRIPTIONS :
            return subscription_items[indexPath.row]
        default : return UITableViewCell()
        }
    }

    func handleTTMessage(var m: TTMessage) {
        
        switch m.nClientEvent {
            
        case CLIENTEVENT_CMD_SUCCESS :
            
            if m.nSource == kickban_cmdid {
                TT_DoBanUser(ttInst, userid, 0)
            }
            
        case CLIENTEVENT_CMD_ERROR :
            
            if m.nSource == kick_cmdid || m.nSource == kickban_cmdid {
                let errmsg = getClientErrorMsg(&m).memory
                let s = fromTTString(errmsg.szErrorMsg)
                let alert = UIAlertController(title: NSLocalizedString("Error", comment: "user detail"), message: s, preferredStyle: UIAlertControllerStyle.Alert)
                alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "user detail"), style: UIAlertActionStyle.Default, handler: nil))
                self.presentViewController(alert, animated: true, completion: nil)
            }
        default :
            break
        }
    }
    
}
