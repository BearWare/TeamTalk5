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

class ChannelDetailViewController :
    UITableViewController, TeamTalkEvent,
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
    
    var chan_items = [UITableViewCell]()
    var cmd_items = [UITableViewCell]()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        if channel.nChannelID == 0 {
            channel.audiocodec = newAudioCodec(DEFAULT_AUDIOCODEC)
        }
        
        let namecell = UITableViewCell(style: .default, reuseIdentifier: nil)
        namefield = newTableCellTextField(namecell, label: NSLocalizedString("Name", comment: "create channel"), initial: String(cString: getChannelString(NAME, &channel)))
        namefield?.delegate = self
        chan_items.append(namecell)
        
        let passwdcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        passwdfield = newTableCellTextField(passwdcell, label: NSLocalizedString("Password", comment: "create channel"), initial: String (cString: getChannelString(PASSWORD, &channel)))
        passwdfield?.delegate = self
        passwdfield?.autocorrectionType = .no
        passwdfield?.spellCheckingType = .no
        passwdfield?.autocapitalizationType = .none
        chan_items.append(passwdcell)
        
        let topiccell = UITableViewCell(style: .default, reuseIdentifier: nil)
        topicfield = newTableCellTextField(topiccell, label: NSLocalizedString("Topic", comment: "create channel"), initial: String(cString: getChannelString(TOPIC, &channel)))
        topicfield?.delegate = self
        chan_items.append(topiccell)
        
        codeccell = tableView.dequeueReusableCell(withIdentifier: "Setup Codec Cell")!
        codeccell!.selectionStyle = .none
        codeccell!.textLabel!.text = NSLocalizedString("Audio Codec", comment: "create channel")
        showCodecDetail()
        chan_items.append(codeccell!)
        
        let permanentcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        permanentswitch = newTableCellSwitch(permanentcell, label: NSLocalizedString("Permanent Channel", comment: "create channel"), initial: (channel.uChannelType & CHANNEL_PERMANENT.rawValue) != 0)
        chan_items.append(permanentcell)
        
        let nointerruptcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        nointerruptionsswitch = newTableCellSwitch(nointerruptcell, label: NSLocalizedString("No Interruptions", comment: "create channel"), initial: (channel.uChannelType & CHANNEL_SOLO_TRANSMIT.rawValue) != 0)
        chan_items.append(nointerruptcell)
        
        let novoiceactcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        novoiceactivationswitch = newTableCellSwitch(novoiceactcell, label: NSLocalizedString("No Voice Activation", comment: "create channel"), initial: (channel.uChannelType & CHANNEL_NO_VOICEACTIVATION.rawValue) != 0)
        chan_items.append(novoiceactcell)
        
        let noaudiorecordcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        noaudiorecordingswitch = newTableCellSwitch(noaudiorecordcell, label: NSLocalizedString("No Audio Recording", comment: "create channel"), initial: (channel.uChannelType & CHANNEL_NO_RECORDING.rawValue) != 0)
        chan_items.append(noaudiorecordcell)
        
        if channel.nChannelID != 0 {
            let joinchan = tableView.dequeueReusableCell(withIdentifier: "Join Channel")!
            cmd_items.append(joinchan)
            
            let deletechan = tableView.dequeueReusableCell(withIdentifier: "Delete Channel")!
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
    
    func textFieldDidBeginEditing(_ textfield: UITextField) {
        let cell = textfield.superview as! UITableViewCell
        tableView.scrollToRow(at: tableView.indexPath(for: cell)!, at: .top, animated: true)
    }
    
//    func textFieldShouldEndEditing(textfield: UITextField) -> Bool {
//        return true
//    }
    
    func textFieldShouldReturn(_ textfield: UITextField) -> Bool {
        textfield.resignFirstResponder()
        return false
    }

    func showCodecDetail() {
        var codecdetail = ""
        switch channel.audiocodec.nCodec {
        case OPUS_CODEC :
            let opus = getOpusCodec(&channel.audiocodec).pointee
            let chans = (opus.nChannels>1 ? NSLocalizedString("Stereo", comment: "create channel") : NSLocalizedString("Mono", comment: "create channel") )
            codecdetail = "OPUS \(opus.nSampleRate / 1000) KHz \(opus.nBitRate / 1000) KB/s " + chans
        case SPEEX_CODEC :
            let speex = getSpeexCodec(&channel.audiocodec).pointee
            codecdetail = "Speex " + getBandmodeString(speex.nBandmode)
        case SPEEX_VBR_CODEC :
            let speexvbr = getSpeexVBRCodec(&channel.audiocodec).pointee
            codecdetail = "Speex VBR " + getBandmodeString(speexvbr.nBandmode)
        case NO_CODEC :
            fallthrough
        default :
            codecdetail = NSLocalizedString("No Audio", comment: "create channel")
        }
        codeccell!.detailTextLabel?.text = codecdetail
    }
    
    func getBandmodeString(_ bandmode : INT32) -> String {
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
    
    @IBAction func createChannel(_ sender: UIBarButtonItem) {
        saveChannelDetail()
        
        if channel.nChannelID == 0 {
            cmdid = TT_DoJoinChannel(ttInst, &channel)
        }
        else {
            cmdid = TT_DoUpdateChannel(ttInst, &channel)
        }
    }
    
    @IBAction func joinChannelPressed(_ sender: UIButton) {
        
        if channel.bPassword == TRUE {
            let alert = UIAlertView(title: NSLocalizedString("Enter Password", comment: "Dialog message"),
                                    message: NSLocalizedString("Password", comment: "Dialog message"), delegate: self,
                                                               cancelButtonTitle: NSLocalizedString("Join", comment: "Dialog message"))
            alert.alertViewStyle = .secureTextInput
            alert.textField(at: 0)?.text = self.passwdfield?.text
            alert.show()
        }
        else {
            cmdid = TT_DoJoinChannelByID(ttInst, channel.nChannelID, "")
        }
    }
    
    
    func alertView(_ alertView: UIAlertView, clickedButtonAt buttonIndex: Int) {
        let passwd = (alertView.textField(at: 0)?.text)!
        cmdid = TT_DoJoinChannelByID(ttInst, channel.nChannelID, passwd)
    }

    
    @IBAction func deleteChannelPressed(_ sender: UIButton) {
        cmdid = TT_DoRemoveChannel(ttInst, channel.nChannelID)
    }
    
    func handleTTMessage(_ m: TTMessage) {
        var m = m
        
        switch m.nClientEvent {
            
        case CLIENTEVENT_CMD_SUCCESS :
            if m.nSource == cmdid {
                self.navigationController!.popViewController(animated: true)
            }
        case CLIENTEVENT_CMD_ERROR :
            if m.nSource == cmdid {
                var errmsg = getClientErrorMsg(&m).pointee
                let s = String(cString: getClientErrorMsgString(ERRMESSAGE, &errmsg))
                if #available(iOS 8.0, *) {
                    let alert = UIAlertController(title: NSLocalizedString("Error", comment: "Dialog message"), message: s, preferredStyle: UIAlertController.Style.alert)
                    alert.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "Dialog message"), style: UIAlertAction.Style.default, handler: nil))
                    self.present(alert, animated: true, completion: nil)
                    
                } else {
                    // Fallback on earlier versions
                }
            }
        case CLIENTEVENT_CMD_PROCESSING :
            if getTTBOOL(&m) == FALSE && cmdid == m.nSource {
                cmdid = 0
            }
            
        default : break
        }
    }
    
    @IBAction func saveNoAudioCodec(_ segue:UIStoryboardSegue) {
        channel.audiocodec.nCodec = NO_CODEC
        showCodecDetail()
    }

    @IBAction func saveOpusCodec(_ segue:UIStoryboardSegue) {
        
        if segue.source is AudioCodecViewController {
            let vc = segue.source as! AudioCodecViewController
            vc.saveOPUSCodec()
            setOpusCodec(&channel.audiocodec, &vc.opuscodec)
            showCodecDetail()
        }
    }
    
    @IBAction func saveSpeexCodec(_ segue:UIStoryboardSegue) {
        
        if segue.source is AudioCodecViewController {
            let vc = segue.source as! AudioCodecViewController
            vc.saveSpeexCodec()
            setSpeexCodec(&channel.audiocodec, &vc.speexcodec)
            showCodecDetail()
        }
    }

    @IBAction func saveSpeexVBRCodec(_ segue:UIStoryboardSegue) {
        
        if segue.source is AudioCodecViewController {
            let vc = segue.source as! AudioCodecViewController
            vc.saveSpeexVBRCodec()
            setSpeexVBRCodec(&channel.audiocodec, &vc.speexvbrcodec)
            showCodecDetail()
        }
    }

    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        
        if segue.identifier == "Setup Audio Codec" {
            
            let vc = segue.destination as! AudioCodecViewController

            vc.audiocodec = channel.audiocodec
            
            switch channel.audiocodec.nCodec {
            case SPEEX_CODEC :
                vc.speexcodec = getSpeexCodec(&channel.audiocodec).pointee
            case SPEEX_VBR_CODEC :
                vc.speexvbrcodec = getSpeexVBRCodec(&channel.audiocodec).pointee
            case OPUS_CODEC :
                vc.opuscodec = getOpusCodec(&channel.audiocodec).pointee
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
        if permanentswitch!.isOn {
            channel.uChannelType |= CHANNEL_PERMANENT.rawValue
        }
        else {
            channel.uChannelType &= ~CHANNEL_PERMANENT.rawValue
        }
        if nointerruptionsswitch!.isOn {
            channel.uChannelType |= CHANNEL_SOLO_TRANSMIT.rawValue
        }
        else {
            channel.uChannelType &= ~CHANNEL_SOLO_TRANSMIT.rawValue
        }
        if novoiceactivationswitch!.isOn {
            channel.uChannelType |= CHANNEL_NO_VOICEACTIVATION.rawValue
        }
        else {
            channel.uChannelType &= ~CHANNEL_NO_VOICEACTIVATION.rawValue
        }
        if noaudiorecordingswitch!.isOn {
            channel.uChannelType |= CHANNEL_NO_RECORDING.rawValue
        }
        else {
            channel.uChannelType &= ~CHANNEL_NO_RECORDING.rawValue
        }
        
    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        if cmd_items.count > 0 {
            return 2
        }
        else {
            return 1
        }
    }
    
    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch section {
        case 0 :
            return NSLocalizedString("Channel Properties", comment: "create channel")
        case 1 :
            return NSLocalizedString("Commands", comment: "create channel")
        default :
            return nil
        }
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch section {
        case 0 :
            return chan_items.count
        case 1 :
            return cmd_items.count
        default :
            return 0
        }
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
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
