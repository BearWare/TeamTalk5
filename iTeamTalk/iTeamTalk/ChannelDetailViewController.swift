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

class ChannelDetailViewController :
    UIViewController, UITableViewDataSource,
    UITableViewDelegate, TeamTalkEvent,
    UITextFieldDelegate, UIAlertViewDelegate {

    var channel = Channel()
    
    var cmdid : INT32 = 0
    
    @IBOutlet weak var navitem: UINavigationItem!
    @IBOutlet weak var doneBtn: UIBarButtonItem!
    
    var namefield: UITextField?
    var passwdfield: UITextField?
    var topicfield: UITextField?
    var codeccell : UITableViewCell?
    var permanentswitch: UISwitch?
    var nointerruptionsswitch: UISwitch?
    var novoiceactivationswitch: UISwitch?
    var noaudiorecordingswitch: UISwitch?
    
    @IBOutlet weak var tableView: UITableView!
    
    var chan_items = [UITableViewCell]()
    var cmd_items = [UITableViewCell]()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        if channel.nChannelID == 0 {
            channel.audiocodec = newAudioCodec(DEFAULT_AUDIOCODEC)
        }
        
        let namecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        namefield = newTableCellTextField(namecell, label: NSLocalizedString("Name", comment: "create channel"), initial: fromTTString(channel.szName))
        namefield?.delegate = self
        chan_items.append(namecell)
        
        let passwdcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        passwdfield = newTableCellTextField(passwdcell, label: NSLocalizedString("Password", comment: "create channel"), initial: fromTTString(channel.szPassword))
        passwdfield?.delegate = self
        passwdfield?.autocorrectionType = .No
        passwdfield?.spellCheckingType = .No
        passwdfield?.autocapitalizationType = .None
        chan_items.append(passwdcell)
        
        let topiccell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        topicfield = newTableCellTextField(topiccell, label: NSLocalizedString("Topic", comment: "create channel"), initial: fromTTString(channel.szTopic))
        topicfield?.delegate = self
        chan_items.append(topiccell)
        
        codeccell = tableView.dequeueReusableCellWithIdentifier("Setup Codec Cell")!
        codeccell!.selectionStyle = .None
        codeccell!.textLabel!.text = NSLocalizedString("Audio Codec", comment: "create channel")
        showCodecDetail()
        chan_items.append(codeccell!)
        
        let permanentcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        permanentswitch = newTableCellSwitch(permanentcell, label: NSLocalizedString("Permanent Channel", comment: "create channel"), initial: (channel.uChannelType & CHANNEL_PERMANENT.rawValue) != 0)
        chan_items.append(permanentcell)
        
        let nointerruptcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        nointerruptionsswitch = newTableCellSwitch(nointerruptcell, label: NSLocalizedString("No Interruptions", comment: "create channel"), initial: (channel.uChannelType & CHANNEL_SOLO_TRANSMIT.rawValue) != 0)
        chan_items.append(nointerruptcell)
        
        let novoiceactcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        novoiceactivationswitch = newTableCellSwitch(novoiceactcell, label: NSLocalizedString("No Voice Activation", comment: "create channel"), initial: (channel.uChannelType & CHANNEL_NO_VOICEACTIVATION.rawValue) != 0)
        chan_items.append(novoiceactcell)
        
        let noaudiorecordcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        noaudiorecordingswitch = newTableCellSwitch(noaudiorecordcell, label: NSLocalizedString("No Audio Recording", comment: "create channel"), initial: (channel.uChannelType & CHANNEL_NO_RECORDING.rawValue) != 0)
        chan_items.append(noaudiorecordcell)
        
        if channel.nChannelID != 0 {
            let joinchan = tableView.dequeueReusableCellWithIdentifier("Join Channel")!
            cmd_items.append(joinchan)
            
            let deletechan = tableView.dequeueReusableCellWithIdentifier("Delete Channel")!
            cmd_items.append(deletechan)
        }
        
        if !namefield!.text!.isEmpty {
            navitem.title = namefield!.text
        }
        
        if channel.nChannelID == 0 {
            navitem.title = NSLocalizedString("Create Channel", comment: "View Title")
        }
        else {
        }
        
        addToTTMessages(self)
        
        tableView.dataSource = self
        tableView.delegate = self
    }
    
    func textFieldDidBeginEditing(textfield: UITextField) {
        let cell = textfield.superview as! UITableViewCell
        tableView.scrollToRowAtIndexPath(tableView.indexPathForCell(cell)!, atScrollPosition: .Top, animated: true)
    }
    
//    func textFieldShouldEndEditing(textfield: UITextField) -> Bool {
//        return true
//    }
    
    func textFieldShouldReturn(textfield: UITextField) -> Bool {
        textfield.resignFirstResponder()
        return false
    }

    func showCodecDetail() {
        var codecdetail = ""
        switch channel.audiocodec.nCodec {
        case OPUS_CODEC :
            let opus = getOpusCodec(&channel.audiocodec).memory
            let chans = (opus.nChannels>1 ? NSLocalizedString("Stereo", comment: "create channel") : NSLocalizedString("Mono", comment: "create channel") )
            codecdetail = "OPUS \(opus.nSampleRate / 1000) KHz \(opus.nBitRate / 1000) KB/s " + chans
        case SPEEX_CODEC :
            let speex = getSpeexCodec(&channel.audiocodec).memory
            codecdetail = "Speex " + getBandmodeString(speex.nBandmode)
        case SPEEX_VBR_CODEC :
            let speexvbr = getSpeexVBRCodec(&channel.audiocodec).memory
            codecdetail = "Speex VBR " + getBandmodeString(speexvbr.nBandmode)
        case NO_CODEC :
            fallthrough
        default :
            codecdetail = NSLocalizedString("No Audio", comment: "create channel")
        }
        codeccell!.detailTextLabel?.text = codecdetail
    }
    
    func getBandmodeString(bandmode : INT32) -> String {
        switch bandmode {
        case 2 :
            return NSLocalizedString("32 KHz", comment: "create channel")
        case 1 :
            return NSLocalizedString("16 KHz", comment: "create channel")
        case 0 :
            fallthrough
        default :
            return NSLocalizedString("8 KHz", comment: "create channel")
        }
    }
    
    @IBAction func createChannel(sender: UIBarButtonItem) {
        saveChannelDetail()
        
        if channel.nChannelID == 0 {
            cmdid = TT_DoJoinChannel(ttInst, &channel)
            
        }
        else {
            cmdid = TT_DoUpdateChannel(ttInst, &channel)
        }
    }
    
    @IBAction func joinChannelPressed(sender: UIButton) {
        
        if channel.bPassword != 0 {
            let alert = UIAlertView(title: NSLocalizedString("Enter Password", comment: "Dialog message"), message: NSLocalizedString("Password", comment: "Dialog message"), delegate: self, cancelButtonTitle: NSLocalizedString("Join", comment: "Dialog message"))
            alert.alertViewStyle = .SecureTextInput
            alert.show()
        }
        else {
            cmdid = TT_DoJoinChannelByID(ttInst, channel.nChannelID, "")
        }
    }
    
    
    func alertView(alertView: UIAlertView, clickedButtonAtIndex buttonIndex: Int) {
        let passwd = (alertView.textFieldAtIndex(0)?.text)!
        cmdid = TT_DoJoinChannelByID(ttInst, channel.nChannelID, passwd)
    }

    
    @IBAction func deleteChannelPressed(sender: UIButton) {
        cmdid = TT_DoRemoveChannel(ttInst, channel.nChannelID)
    }
    
    func handleTTMessage(var m: TTMessage) {
        
        switch m.nClientEvent {
            
        case CLIENTEVENT_CMD_SUCCESS :
            if m.nSource == cmdid {
                self.navigationController?.popViewControllerAnimated(true)
            }
        case CLIENTEVENT_CMD_ERROR :
            if m.nSource == cmdid {
                let errmsg = getClientErrorMsg(&m).memory
                let s = fromTTString(errmsg.szErrorMsg)
                if #available(iOS 8.0, *) {
                    let alert = UIAlertController(title: NSLocalizedString("Error", comment: "Dialog message"), message: s, preferredStyle: UIAlertControllerStyle.Alert)
                    alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "Dialog message"), style: UIAlertActionStyle.Default, handler: nil))
                    self.presentViewController(alert, animated: true, completion: nil)
                    
                } else {
                    // Fallback on earlier versions
                }
            }
        case CLIENTEVENT_CMD_PROCESSING :
            if getTTBOOL(&m) == 0 && cmdid == m.nSource {
                cmdid = 0
            }
            
        default : break
        }
    }
    
    @IBAction func saveNoAudioCodec(segue:UIStoryboardSegue) {
        channel.audiocodec.nCodec = NO_CODEC
        showCodecDetail()
    }

    @IBAction func saveOpusCodec(segue:UIStoryboardSegue) {
        
        if segue.sourceViewController is AudioCodecViewController {
            let vc = segue.sourceViewController as! AudioCodecViewController
            vc.saveOPUSCodec()
            setOpusCodec(&channel.audiocodec, &vc.opuscodec)
            showCodecDetail()
        }
    }
    
    @IBAction func saveSpeexCodec(segue:UIStoryboardSegue) {
        
        if segue.sourceViewController is AudioCodecViewController {
            let vc = segue.sourceViewController as! AudioCodecViewController
            vc.saveSpeexCodec()
            setSpeexCodec(&channel.audiocodec, &vc.speexcodec)
            showCodecDetail()
        }
    }

    @IBAction func saveSpeexVBRCodec(segue:UIStoryboardSegue) {
        
        if segue.sourceViewController is AudioCodecViewController {
            let vc = segue.sourceViewController as! AudioCodecViewController
            vc.saveSpeexVBRCodec()
            setSpeexVBRCodec(&channel.audiocodec, &vc.speexvbrcodec)
            showCodecDetail()
        }
    }

    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        
        if segue.identifier == "Setup Audio Codec" {
            
            let vc = segue.destinationViewController as! AudioCodecViewController

            vc.audiocodec = channel.audiocodec
            
            switch channel.audiocodec.nCodec {
            case SPEEX_CODEC :
                vc.speexcodec = getSpeexCodec(&channel.audiocodec).memory
            case SPEEX_VBR_CODEC :
                vc.speexvbrcodec = getSpeexVBRCodec(&channel.audiocodec).memory
            case OPUS_CODEC :
                vc.opuscodec = getOpusCodec(&channel.audiocodec).memory
            case NO_CODEC :
                if channel.nChannelID == 0 {
                    vc.audiocodec.nCodec = OPUS_CODEC
                }
                else {
                    vc.audiocodec.nCodec = vc.audiocodec.nCodec
                }
            default :
                vc.audiocodec.nCodec = NO_CODEC
            }
            
        }
    }
    
    func saveChannelDetail() {
        toTTString(namefield!.text!, dst: &channel.szName)
        toTTString(passwdfield!.text!, dst: &channel.szPassword)
        toTTString(topicfield!.text!, dst: &channel.szTopic)
        if permanentswitch!.on {
            channel.uChannelType |= CHANNEL_PERMANENT.rawValue
        }
        else {
            channel.uChannelType &= ~CHANNEL_PERMANENT.rawValue
        }
        if nointerruptionsswitch!.on {
            channel.uChannelType |= CHANNEL_SOLO_TRANSMIT.rawValue
        }
        else {
            channel.uChannelType &= ~CHANNEL_SOLO_TRANSMIT.rawValue
        }
        if novoiceactivationswitch!.on {
            channel.uChannelType |= CHANNEL_NO_VOICEACTIVATION.rawValue
        }
        else {
            channel.uChannelType &= ~CHANNEL_NO_VOICEACTIVATION.rawValue
        }
        if noaudiorecordingswitch!.on {
            channel.uChannelType |= CHANNEL_NO_RECORDING.rawValue
        }
        else {
            channel.uChannelType &= ~CHANNEL_NO_RECORDING.rawValue
        }
        
    }
    
    func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        if cmd_items.count > 0 {
            return 2
        }
        else {
            return 1
        }
    }
    
    func tableView(tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch section {
        case 0 :
            return NSLocalizedString("Channel Properties", comment: "create channel")
        case 1 :
            return NSLocalizedString("Commands", comment: "create channel")
        default :
            return nil
        }
    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch section {
        case 0 :
            return chan_items.count
        case 1 :
            return cmd_items.count
        default :
            return 0
        }
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        switch indexPath.section {
        case 0 :
            return chan_items[indexPath.row]
        case 1 :
            return cmd_items[indexPath.row]
        default :
            return UITableViewCell()
        }
    }
}
