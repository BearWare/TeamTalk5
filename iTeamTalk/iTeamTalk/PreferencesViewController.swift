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
import AVFoundation

let PREF_NICKNAME = "nickname_preference"
let PREF_JOINROOTCHANNEL = "joinroot_preference"

let PREF_DISPLAY_PROXIMITY = "display_proximity_sensor"
let PREF_DISPLAY_POPUPTXTMSG = "display_popuptxtmsg_preference"
let PREF_DISPLAY_LIMITTEXT = "display_limittext_preference"
let PREF_DISPLAY_PUBSERVERS = "display_publicservers_preference"

let PREF_MASTER_VOLUME = "mastervolume_preference"
let PREF_MICROPHONE_GAIN = "microphonegain_preference"
let PREF_SPEAKER_OUTPUT = "speakeroutput_preference"
let PREF_VOICEACTIVATION = "voiceactivationlevel_preference"
let PREF_MEDIAFILE_VOLUME = "mediafile_volume_preference"

let PREF_SNDEVENT_SERVERLOST = "snd_srvlost_preference"
let PREF_SNDEVENT_VOICETX = "snd_voicetx_preference"
let PREF_SNDEVENT_CHANMSG = "snd_chanmsg_preference"
let PREF_SNDEVENT_USERMSG = "snd_usermsg_preference"
let PREF_SNDEVENT_JOINEDCHAN = "snd_joinedchan_preference"
let PREF_SNDEVENT_LEFTCHAN = "snd_leftchan_preference"

let PREF_SUB_USERMSG = "sub_usertextmsg_preference"
let PREF_SUB_CHANMSG = "sub_chantextmsg_preference"
let PREF_SUB_BROADCAST = "sub_broadcastmsg_preference"
let PREF_SUB_VOICE = "sub_voice_preference"
let PREF_SUB_VIDEOCAP = "sub_videocapture_preference"
let PREF_SUB_MEDIAFILE = "sub_mediafile_preference"
let PREF_SUB_DESKTOP = "sub_desktop_preference"
let PREF_SUB_DESKTOPINPUT = "sub_desktopinput_preference"

let PREF_TTSEVENT_JOINEDCHAN = "tts_joinedchan_preference"
let PREF_TTSEVENT_LEFTCHAN = "tts_leftchan_preference"
let PREF_TTSEVENT_CONLOST = "tts_conlost_preference"

class PreferencesViewController : UIViewController, UITableViewDataSource, UITableViewDelegate, UITextFieldDelegate {
    
    @IBOutlet weak var tableView: UITableView!
   
    var nicknamefield : UITextField?
    
    var ttInst = UnsafeMutablePointer<Void>()
    
    var limittextcell : UITableViewCell?
    var mastervolcell : UITableViewCell?
    var voiceactcell : UITableViewCell?
    var microphonecell : UITableViewCell?

    var general_items = [UITableViewCell]()
    var display_items = [UITableViewCell]()
    var soundevents_items = [UITableViewCell]()
    var sound_items  = [UITableViewCell]()
    var subscription_items = [UITableViewCell]()
    var connection_items = [UITableViewCell]()
    var ttsevents_items = [UITableViewCell]()
    
    let SECTION_GENERAL = 0,
        SECTION_DISPLAY = 1,
        SECTION_SOUND = 2,
        SECTION_SOUNDEVENTS = 3,
        SECTION_TTSEVENTS = 4,
        SECTION_CONNECTION = 5,
        SECTION_SUBSCRIPTIONS = 6,
        SECTIONS_COUNT = 7
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        tableView.dataSource = self
        tableView.delegate = self
        
        let settings = NSUserDefaults.standardUserDefaults()
        
        var nickname = settings.stringForKey(PREF_NICKNAME)
        if nickname == nil {
            nickname = DEFAULT_NICKNAME
        }
        
        // general items
        
        let nicknamecell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        nicknamefield = newTableCellTextField(nicknamecell, label: NSLocalizedString("Nickname", comment: "preferences"), initial: nickname!)
        nicknamecell.detailTextLabel!.text = NSLocalizedString("Name displayed in channel list", comment: "preferences")
        nicknamefield?.addTarget(self, action: "nicknameChanged:", forControlEvents: .EditingDidEnd)
        nicknamefield?.delegate = self
        general_items.append(nicknamecell)
        
        // display items

        let proximitycell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let proximity = settings.objectForKey(PREF_DISPLAY_PROXIMITY) != nil && settings.boolForKey(PREF_DISPLAY_PROXIMITY)
        let proximitywitch = newTableCellSwitch(proximitycell, label: NSLocalizedString("Proximity sensor", comment: "preferences"), initial: proximity)
        proximitycell.detailTextLabel!.text = NSLocalizedString("Turn off screen when holding phone near ear", comment: "preferences")
        proximitywitch.addTarget(self, action: "proximityChanged:", forControlEvents: .ValueChanged)
        display_items.append(proximitycell)
        
        let txtmsgpopcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let txtmsgpopup = settings.objectForKey(PREF_DISPLAY_POPUPTXTMSG) == nil || settings.boolForKey(PREF_DISPLAY_POPUPTXTMSG)
        let txtmsgswitch = newTableCellSwitch(txtmsgpopcell, label: NSLocalizedString("Show text messages instantly", comment: "preferences"), initial: txtmsgpopup)
        txtmsgpopcell.detailTextLabel!.text = NSLocalizedString("Pop up text message when new messages are received", comment: "preferences")
        txtmsgswitch.addTarget(self, action: "showtextmessagesChanged:", forControlEvents: .ValueChanged)
        display_items.append(txtmsgpopcell)
        
        limittextcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let limittext = settings.objectForKey(PREF_DISPLAY_LIMITTEXT) == nil ? DEFAULT_LIMIT_TEXT : settings.integerForKey(PREF_DISPLAY_LIMITTEXT)
        let limittextstepper = newTableCellStepper(limittextcell!, label: NSLocalizedString("Maximum Text Length", comment: "preferences"), min: 1, max: Double(TT_STRLEN-1), step: 1, initial: Double(limittext))
        limittextChanged(limittextstepper)
        limittextstepper.addTarget(self, action: "limittextChanged:", forControlEvents: .ValueChanged)
        display_items.append(limittextcell!)
        
        let pubservercell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let pubsrv = settings.objectForKey(PREF_DISPLAY_PUBSERVERS) == nil || settings.boolForKey(PREF_DISPLAY_PUBSERVERS)
        let pubserverswitch = newTableCellSwitch(pubservercell, label: NSLocalizedString("Show Public Servers", comment: "preferences"), initial: pubsrv)
        pubservercell.detailTextLabel!.text = NSLocalizedString("Show public servers in server list", comment: "preferences")
        pubserverswitch.addTarget(self, action: "showpublicserversChanged:", forControlEvents: .ValueChanged)
        display_items.append(pubservercell)

        
        // sound preferences
        
        mastervolcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        var outputvol = Int(SOUND_VOLUME_DEFAULT.rawValue)
        if ttInst != nil {
            outputvol = Int(TT_GetSoundOutputVolume(ttInst))
        }
        else if settings.objectForKey(PREF_MASTER_VOLUME) != nil {
            let output_pct = settings.integerForKey(PREF_MASTER_VOLUME)
            outputvol = refVolume(Double(output_pct))
        }
        let output_pct = refVolumeToPercent(outputvol)
        let mastervolslider = newTableCellSlider(mastervolcell!, label: NSLocalizedString("Master Volume", comment: "preferences"), min: 0, max: 1, initial: Float(output_pct) / 100)
        mastervolslider.addTarget(self, action: "masterVolumeChanged:", forControlEvents: .ValueChanged)
        masterVolumeChanged(mastervolslider)
        sound_items.append(mastervolcell!)
        
        let mfvolumecell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        var mfvol = DEFAULT_MEDIAFILE_VOLUME
        if settings.valueForKey(PREF_MEDIAFILE_VOLUME) != nil {
            mfvol = settings.floatForKey(PREF_MEDIAFILE_VOLUME)
        }
        let mfvolumeslider = newTableCellSlider(mfvolumecell, label: NSLocalizedString("Media File Volume", comment: "preferences"), min: 0, max: 1, initial: mfvol)
        mfvolumeslider.addTarget(self, action: "mediafileVolumeChanged:", forControlEvents: .ValueChanged)
        mfvolumecell.detailTextLabel?.text = NSLocalizedString("Media file vs. voice volume (requires reconnect)", comment: "preferences")
        sound_items.append(mfvolumecell)

        microphonecell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        var inputvol = Int(SOUND_GAIN_DEFAULT.rawValue)
        if ttInst != nil {
            inputvol = Int(TT_GetSoundInputGainLevel(ttInst))
        }
        else if settings.objectForKey(PREF_MICROPHONE_GAIN) != nil {
            let input_pct = settings.integerForKey(PREF_MICROPHONE_GAIN)
            inputvol = refVolume(Double(input_pct))
        }
        let input_pct = refVolumeToPercent(inputvol)
        let microphoneslider = newTableCellSlider(microphonecell!, label: NSLocalizedString("Microphone Gain", comment: "preferences"), min: 0, max: 1, initial: Float(input_pct) / 100)
        microphoneslider.addTarget(self, action: "microphoneGainChanged:", forControlEvents: .ValueChanged)
        microphoneGainChanged(microphoneslider)
        sound_items.append(microphonecell!)
        
        // use SOUND_VU_MAX + 1 as voice activation disabled
        var voiceact = VOICEACT_DISABLED
        if settings.objectForKey(PREF_VOICEACTIVATION) != nil {
            voiceact = settings.integerForKey(PREF_VOICEACTIVATION)
        }
        voiceactcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let voiceactslider = newTableCellSlider(voiceactcell!, label: NSLocalizedString("Voice Activation Level", comment: "preferences"),
            min: 0, max: 1, initial: Float(voiceact) / Float(VOICEACT_DISABLED))
        voiceactslider.addTarget(self, action: "voiceactlevelChanged:", forControlEvents: .ValueChanged)
        voiceactlevelChanged(voiceactslider)
        sound_items.append(voiceactcell!)
        
        let speakercell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let speakerswitch = newTableCellSwitch(speakercell, label: NSLocalizedString("Speaker Output", comment: "preferences"),
            initial: settings.objectForKey(PREF_SPEAKER_OUTPUT) != nil && settings.boolForKey(PREF_SPEAKER_OUTPUT))
        speakercell.detailTextLabel!.text = NSLocalizedString("Use iPhone's speaker instead of earpiece", comment: "preferences")
        speakerswitch.addTarget(self, action: "speakeroutputChanged:", forControlEvents: .ValueChanged)
        sound_items.append(speakercell)

        
        // sound events
        
        let srvlostcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let srvlostswitch = newTableCellSwitch(srvlostcell, label: NSLocalizedString("Server Connection Lost", comment: "preferences"), initial: getSoundFile(.SRV_LOST) != nil)
        srvlostcell.detailTextLabel!.text = NSLocalizedString("Play sound when connection is dropped", comment: "preferences")
        srvlostswitch.tag = Sounds.SRV_LOST.rawValue
        srvlostswitch.addTarget(self, action: "soundeventChanged:", forControlEvents: .ValueChanged)
        soundeventChanged(srvlostswitch)
        soundevents_items.append(srvlostcell)
        
        let voicetxcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let voicetxswitch = newTableCellSwitch(voicetxcell, label: NSLocalizedString("Voice Transmission Toggled", comment: "preferences"), initial: getSoundFile(.TX_ON) != nil)
        voicetxcell.detailTextLabel!.text = NSLocalizedString("Play sound when voice transmission is toggled", comment: "preferences")
        voicetxswitch.tag = Sounds.TX_ON.rawValue
        voicetxswitch.addTarget(self, action: "soundeventChanged:", forControlEvents: .ValueChanged)
        soundeventChanged(voicetxswitch)
        soundevents_items.append(voicetxcell)

        let usermsgcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let usermsgswitch = newTableCellSwitch(usermsgcell, label: NSLocalizedString("Private Text Message", comment: "preferences"), initial: getSoundFile(.USER_MSG) != nil)
        usermsgcell.detailTextLabel!.text = NSLocalizedString("Play sound when private text message is received", comment: "preferences")
        usermsgswitch.tag = Sounds.USER_MSG.rawValue
        usermsgswitch.addTarget(self, action: "soundeventChanged:", forControlEvents: .ValueChanged)
        soundeventChanged(usermsgswitch)
        soundevents_items.append(usermsgcell)
        
        let chanmsgcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let chanmsgswitch = newTableCellSwitch(chanmsgcell, label: NSLocalizedString("Channel Text Message", comment: "preferences"), initial: getSoundFile(.CHAN_MSG) != nil)
        chanmsgcell.detailTextLabel!.text = NSLocalizedString("Play sound when channel text message is received", comment: "preferences")
        chanmsgswitch.tag = Sounds.CHAN_MSG.rawValue
        chanmsgswitch.addTarget(self, action: "soundeventChanged:", forControlEvents: .ValueChanged)
        soundeventChanged(chanmsgswitch)
        soundevents_items.append(chanmsgcell)
        
        let joinedchancell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let joinedchanswitch = newTableCellSwitch(joinedchancell, label: NSLocalizedString("User Joins Channel", comment: "preferences"), initial: getSoundFile(.JOINED_CHAN) != nil)
        joinedchancell.detailTextLabel!.text = NSLocalizedString("Play sound when a user joins the channel", comment: "preferences")
        joinedchanswitch.tag = Sounds.JOINED_CHAN.rawValue
        joinedchanswitch.addTarget(self, action: "soundeventChanged:", forControlEvents: .ValueChanged)
        soundeventChanged(joinedchanswitch)
        soundevents_items.append(joinedchancell)
        
        let leftchancell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let leftchanswitch = newTableCellSwitch(leftchancell, label: NSLocalizedString("User Leaves Channel", comment: "preferences"), initial: getSoundFile(.LEFT_CHAN) != nil)
        leftchancell.detailTextLabel!.text = NSLocalizedString("Play sound when a user leaves the channel", comment: "preferences")
        leftchanswitch.tag = Sounds.LEFT_CHAN.rawValue
        leftchanswitch.addTarget(self, action: "soundeventChanged:", forControlEvents: .ValueChanged)
        soundeventChanged(leftchanswitch)
        soundevents_items.append(leftchancell)

        // connection items
        
        let joinroot = settings.objectForKey(PREF_JOINROOTCHANNEL) == nil || settings.boolForKey(PREF_JOINROOTCHANNEL)
        let joinrootcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let joinrootswitch = newTableCellSwitch(joinrootcell, label: NSLocalizedString("Join Root Channel", comment: "preferences"), initial: joinroot)
        joinrootcell.detailTextLabel!.text = NSLocalizedString("Join root channel after login", comment: "preferences")
        joinrootswitch.addTarget(self, action: "joinrootChanged:", forControlEvents: .ValueChanged)
        connection_items.append(joinrootcell)
        
        // subscription items
        
        let subs = getDefaultSubscriptions()

        let subusermsgcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let subusermsgswitch = newTableCellSwitch(subusermsgcell, label: NSLocalizedString("User Messages", comment: "preferences"), initial: (subs & SUBSCRIBE_USER_MSG.rawValue) != 0)
        subusermsgcell.detailTextLabel!.text = NSLocalizedString("Receive text messages by default", comment: "preferences")
        subusermsgswitch.tag = Int(SUBSCRIBE_USER_MSG.rawValue)
        subusermsgswitch.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subusermsgcell)
        
        let subchanmsgcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let subchanmsgswitch = newTableCellSwitch(subchanmsgcell, label: NSLocalizedString("Channel Messages", comment: "preferences"), initial: (subs & SUBSCRIBE_CHANNEL_MSG.rawValue) != 0)
        subchanmsgcell.detailTextLabel!.text = NSLocalizedString("Receive channel messages by default", comment: "preferences")
        subchanmsgswitch.tag = Int(SUBSCRIBE_CHANNEL_MSG.rawValue)
        subchanmsgswitch.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subchanmsgcell)
        
        let subbcastmsgcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let subbcastmsgswitch = newTableCellSwitch(subbcastmsgcell, label: NSLocalizedString("Broadcast Messages", comment: "preferences"), initial: (subs & SUBSCRIBE_BROADCAST_MSG.rawValue) != 0)
        subbcastmsgcell.detailTextLabel!.text = NSLocalizedString("Receive broadcast messages by default", comment: "preferences")
        subbcastmsgswitch.tag = Int(SUBSCRIBE_BROADCAST_MSG.rawValue)
        subbcastmsgswitch.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subbcastmsgcell)

        let subvoicecell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let subvoiceswitch = newTableCellSwitch(subvoicecell, label: NSLocalizedString("Voice", comment: "preferences"), initial: (subs & SUBSCRIBE_VOICE.rawValue) != 0)
        subvoicecell.detailTextLabel!.text = NSLocalizedString("Receive voice streams by default", comment: "preferences")
        subvoiceswitch.tag = Int(SUBSCRIBE_VOICE.rawValue)
        subvoiceswitch.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subvoicecell)
        
        let subwebcamcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let subwebcamswitch = newTableCellSwitch(subwebcamcell, label: NSLocalizedString("WebCam", comment: "preferences"), initial: (subs & SUBSCRIBE_VIDEOCAPTURE.rawValue) != 0)
        subwebcamcell.detailTextLabel!.text = NSLocalizedString("Receive webcam streams by default", comment: "preferences")
        subwebcamswitch.tag = Int(SUBSCRIBE_VIDEOCAPTURE.rawValue)
        subwebcamswitch.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subwebcamcell)
        
        let submediafilecell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let submediafileswitch = newTableCellSwitch(submediafilecell, label: NSLocalizedString("Media File", comment: "preferences"), initial: (subs & SUBSCRIBE_MEDIAFILE.rawValue) != 0)
        submediafilecell.detailTextLabel?.text = NSLocalizedString("Receive media file streams by default", comment: "preferences")
        submediafileswitch.tag = Int(SUBSCRIBE_MEDIAFILE.rawValue)
        submediafileswitch.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(submediafilecell)
        
        let subdesktopcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let subdesktopswitch = newTableCellSwitch(subdesktopcell, label: NSLocalizedString("Desktop", comment: "preferences"), initial: (subs & SUBSCRIBE_DESKTOP.rawValue) != 0)
        subdesktopcell.detailTextLabel!.text = NSLocalizedString("Receive desktop sessions by default", comment: "preferences")
        subdesktopswitch.tag = Int(SUBSCRIBE_DESKTOP.rawValue)
        subdesktopswitch.addTarget(self, action: "subscriptionChanged:", forControlEvents: .ValueChanged)
        subscription_items.append(subdesktopcell)
        
        
        // text to speech events

        let ttsjoinedchancell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let ttsjoinedchan = settings.objectForKey(PREF_TTSEVENT_JOINEDCHAN) == nil || settings.boolForKey(PREF_TTSEVENT_JOINEDCHAN)
        let ttsjoinedchanswitch = newTableCellSwitch(ttsjoinedchancell, label: NSLocalizedString("User joins channel", comment: "preferences"), initial: ttsjoinedchan)
        ttsjoinedchancell.detailTextLabel!.text = NSLocalizedString("Announce user joining channel", comment: "preferences")
        ttsjoinedchanswitch.addTarget(self, action: "ttsjoinedchanChanged:", forControlEvents: .ValueChanged)
        ttsevents_items.append(ttsjoinedchancell)
        
        let ttsleftchancell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let ttsleftchan = settings.objectForKey(PREF_TTSEVENT_LEFTCHAN) == nil || settings.boolForKey(PREF_TTSEVENT_LEFTCHAN)
        let ttsleftchanswitch = newTableCellSwitch(ttsleftchancell, label: NSLocalizedString("User leaves channel", comment: "preferences"), initial: ttsleftchan)
        ttsleftchancell.detailTextLabel!.text = NSLocalizedString("Announce user leaving channel", comment: "preferences")
        ttsleftchanswitch.addTarget(self, action: "ttsleftchanChanged:", forControlEvents: .ValueChanged)
        ttsevents_items.append(ttsleftchancell)

        let ttsconlostcell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        let ttsconlost = settings.objectForKey(PREF_TTSEVENT_CONLOST) == nil || settings.boolForKey(PREF_TTSEVENT_CONLOST)
        let ttsconlostswitch = newTableCellSwitch(ttsconlostcell, label: NSLocalizedString("Connection lost", comment: "preferences"), initial: ttsconlost)
        ttsconlostcell.detailTextLabel!.text = NSLocalizedString("Announce lost server connection", comment: "preferences")
        ttsconlostswitch.addTarget(self, action: "ttsconlostChanged:", forControlEvents: .ValueChanged)
        ttsevents_items.append(ttsconlostcell)

    }
    
    func textFieldShouldReturn(textField: UITextField) -> Bool {
        textField.resignFirstResponder()
        return false
    }
    
    func soundeventChanged(sender: UISwitch) {
        
        let defaults = NSUserDefaults.standardUserDefaults()
        
        switch sender.tag {
        case Sounds.TX_ON.rawValue :
            defaults.setBool(sender.on, forKey: PREF_SNDEVENT_VOICETX)
        case Sounds.SRV_LOST.rawValue :
            defaults.setBool(sender.on, forKey: PREF_SNDEVENT_SERVERLOST)
        case Sounds.CHAN_MSG.rawValue :
            defaults.setBool(sender.on, forKey: PREF_SNDEVENT_CHANMSG)
        case Sounds.JOINED_CHAN.rawValue :
            defaults.setBool(sender.on, forKey: PREF_SNDEVENT_JOINEDCHAN)

        case Sounds.LEFT_CHAN.rawValue :
            defaults.setBool(sender.on, forKey: PREF_SNDEVENT_LEFTCHAN)
        case Sounds.USER_MSG.rawValue :
            defaults.setBool(sender.on, forKey: PREF_SNDEVENT_USERMSG)
        default :
            assert(false)
            break
        }
    }
    
    func subscriptionChanged(sender: UISwitch) {
        
        let defaults = NSUserDefaults.standardUserDefaults()
        
        switch UInt32(sender.tag) {
        case SUBSCRIBE_USER_MSG.rawValue :
            defaults.setBool(sender.on, forKey: PREF_SUB_USERMSG)
        case SUBSCRIBE_CHANNEL_MSG.rawValue :
            defaults.setBool(sender.on, forKey: PREF_SUB_CHANMSG)
        case SUBSCRIBE_BROADCAST_MSG.rawValue :
            defaults.setBool(sender.on, forKey: PREF_SUB_BROADCAST)
        case SUBSCRIBE_VOICE.rawValue :
            defaults.setBool(sender.on, forKey: PREF_SUB_VOICE)
        case SUBSCRIBE_VIDEOCAPTURE.rawValue :
            defaults.setBool(sender.on, forKey: PREF_SUB_VIDEOCAP)
        case SUBSCRIBE_MEDIAFILE.rawValue :
            defaults.setBool(sender.on, forKey: PREF_SUB_MEDIAFILE)
        case SUBSCRIBE_DESKTOP.rawValue :
            defaults.setBool(sender.on, forKey: PREF_SUB_DESKTOP)
        default :
            assert(false)
            break
        }
    }
    
    func nicknameChanged(sender: UITextField) {
        if ttInst != nil {
            TT_DoChangeNickname(ttInst, sender.text!)
        }
        
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.setValue(sender.text!, forKey: PREF_NICKNAME)
    }
    
    func masterVolumeChanged(sender: UISlider) {
        let percent : Int = Int(sender.value * 10.0) * 10
        let vol = refVolume(Double(percent))
        if ttInst != nil {
            TT_SetSoundOutputVolume(ttInst, INT32(vol))
        }
        
        if UInt32(vol) == SOUND_VOLUME_DEFAULT.rawValue {
            let txt = String(format: NSLocalizedString("%d %% - Default", comment: "preferences"), percent)
            mastervolcell!.detailTextLabel!.text = txt
        }
        else {
            mastervolcell!.detailTextLabel!.text = "\(percent) %"
        }
        
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.setInteger(percent, forKey: PREF_MASTER_VOLUME)
    }
    
    func showtextmessagesChanged(sender: UISwitch) {
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.setBool(sender.on, forKey: PREF_DISPLAY_POPUPTXTMSG)
    }

    func ttsjoinedchanChanged(sender: UISwitch) {
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.setBool(sender.on, forKey: PREF_TTSEVENT_JOINEDCHAN)
    }

    func ttsleftchanChanged(sender: UISwitch) {
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.setBool(sender.on, forKey: PREF_TTSEVENT_LEFTCHAN)
    }

    func ttsconlostChanged(sender: UISwitch) {
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.setBool(sender.on, forKey: PREF_TTSEVENT_CONLOST)
    }

    func proximityChanged(sender: UISwitch) {
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.setBool(sender.on, forKey: PREF_DISPLAY_PROXIMITY)
        
        let device = UIDevice.currentDevice()
        device.proximityMonitoringEnabled = sender.on
    }

    func limittextChanged(sender: UIStepper) {
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.setInteger(Int(sender.value), forKey: PREF_DISPLAY_LIMITTEXT)
        let txt = String(format: NSLocalizedString("Limit length of names in channel list to %d characters", comment: "preferences"), Int(sender.value))
        limittextcell!.detailTextLabel!.text = txt
    }
    
    func showpublicserversChanged(sender: UISwitch) {
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.setBool(sender.on, forKey: PREF_DISPLAY_PUBSERVERS)
    }
    
    func speakeroutputChanged(sender: UISwitch) {
        
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.setBool(sender.on, forKey: PREF_SPEAKER_OUTPUT)
        
        enableSpeakerOutput(sender.on)
    }
    
    func voiceactlevelChanged(sender: UISlider) {
        let level = Int(sender.value * Float(VOICEACT_DISABLED))
        
        if level == VOICEACT_DISABLED {
            if ttInst != nil {
                TT_EnableVoiceActivation(ttInst, FALSE)
            }
            voiceactcell?.detailTextLabel?.text = NSLocalizedString("Voice Activation Level: Disabled", comment: "preferences")
        }
        else {
            if ttInst != nil {
                TT_EnableVoiceActivation(ttInst, TRUE)
                TT_SetVoiceActivationLevel(ttInst, INT32(level))
            }
            let txt = String(format: NSLocalizedString("Voice Activation Level: %d. Recommended: %d", comment: "preferences"), level, DEFAULT_VOICEACT)
            voiceactcell?.detailTextLabel?.text = txt
        }
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.setInteger(level, forKey: PREF_VOICEACTIVATION)
    }
    
    func microphoneGainChanged(sender: UISlider) {
        let vol_pct : Int = Int(sender.value * 10.0) * 10
        let vol = refVolume(Double(vol_pct))
        if ttInst != nil {
            TT_SetSoundInputGainLevel(ttInst, INT32(vol))
        }
        
        if UInt32(vol) == SOUND_VOLUME_DEFAULT.rawValue {
            let txt = String(format: NSLocalizedString("%d %% - Default", comment: "preferences"), vol_pct)
            microphonecell!.detailTextLabel!.text = txt
        }
        else {
            microphonecell!.detailTextLabel!.text = "\(vol_pct) %"
        }
        
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.setInteger(vol_pct, forKey: PREF_MICROPHONE_GAIN)
    }

    func mediafileVolumeChanged(sender: UISlider) {
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.setFloat(sender.value, forKey: PREF_MEDIAFILE_VOLUME)
    }
    
    func joinrootChanged(sender: UISwitch) {
        
        let defaults = NSUserDefaults.standardUserDefaults()
        defaults.setBool(sender.on, forKey: PREF_JOINROOTCHANNEL)
    }
    
    func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return SECTIONS_COUNT
    }
    
    func tableView(tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch section {
        case SECTION_GENERAL :
            return NSLocalizedString("General", comment: "preferences")
        case SECTION_DISPLAY :
            return NSLocalizedString("Display", comment: "preferences")
        case SECTION_SOUNDEVENTS :
            return NSLocalizedString("Sound Events", comment: "preferences")
        case SECTION_SOUND :
            return NSLocalizedString("Sound System", comment: "preferences")
        case SECTION_CONNECTION :
            return NSLocalizedString("Connection", comment: "preferences")
        case SECTION_SUBSCRIPTIONS :
            return NSLocalizedString("Default Subscriptions", comment: "preferences")
        case SECTION_TTSEVENTS :
            return NSLocalizedString("Text To Speech Events", comment: "preferences")
        default :
            return nil
        }
    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch section {
        case SECTION_GENERAL :
            return general_items.count
        case SECTION_DISPLAY :
            return display_items.count
        case SECTION_SOUNDEVENTS :
            return soundevents_items.count
        case SECTION_SOUND :
            return sound_items.count
        case SECTION_CONNECTION :
            return connection_items.count
        case SECTION_SUBSCRIPTIONS :
            return subscription_items.count
        case SECTION_TTSEVENTS :
            return ttsevents_items.count
        default :
            return 0
        }
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        
        switch indexPath.section {
        case SECTION_GENERAL :
            return general_items[indexPath.row]
        case SECTION_DISPLAY :
            return display_items[indexPath.row]
        case SECTION_SOUNDEVENTS:
            return soundevents_items[indexPath.row]
        case SECTION_SOUND :
            return sound_items[indexPath.row]
        case SECTION_CONNECTION :
            return connection_items[indexPath.row]
        case SECTION_SUBSCRIPTIONS :
            return subscription_items[indexPath.row]
        case SECTION_TTSEVENTS:
            return ttsevents_items[indexPath.row]
        default :
            return UITableViewCell()
        }
    }
}
