//
//  ChannelDetailViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 16-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class ChannelDetailViewController : UIViewController, UITableViewDataSource, UITableViewDelegate, TeamTalkEvent {

    //shared TTInstance between all view controllers
    var ttInst = UnsafeMutablePointer<Void>()

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
        
        let namecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        namefield = newTableCellTextField(namecell, label: "Name", initial: String.fromCString(&channel.szName.0)!)
        chan_items.append(namecell)
        
        let passwdcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        passwdfield = newTableCellTextField(passwdcell, label: "Password", initial: String.fromCString(&channel.szPassword.0)!)
        chan_items.append(passwdcell)
        
        let topiccell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        topicfield = newTableCellTextField(topiccell, label: "Topic", initial: String.fromCString(&channel.szTopic.0)!)
        chan_items.append(topiccell)
        
        codeccell = tableView.dequeueReusableCellWithIdentifier("Setup Codec Cell")!
        codeccell!.selectionStyle = .None
        codeccell!.textLabel!.text = "Audio Codec"
        showCodecDetail()
        chan_items.append(codeccell!)
        
        let permanentcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        permanentswitch = newTableCellSwitch(permanentcell, label: "Permanent Channel", initial: (channel.uChannelType & CHANNEL_PERMANENT.rawValue) != 0)
        chan_items.append(permanentcell)
        
        let nointerruptcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        nointerruptionsswitch = newTableCellSwitch(nointerruptcell, label: "No Interruptions", initial: (channel.uChannelType & CHANNEL_SOLO_TRANSMIT.rawValue) != 0)
        chan_items.append(nointerruptcell)
        
        let novoiceactcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        novoiceactivationswitch = newTableCellSwitch(novoiceactcell, label: "No Voice Activation", initial: (channel.uChannelType & CHANNEL_NO_VOICEACTIVATION.rawValue) != 0)
        chan_items.append(novoiceactcell)
        
        let noaudiorecordcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        noaudiorecordingswitch = newTableCellSwitch(noaudiorecordcell, label: "No Audio Recording", initial: (channel.uChannelType & CHANNEL_NO_RECORDING.rawValue) != 0)
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
            navitem.title = "Create Channel"
        }
        else {
        }
        
        addToTTMessages(self)
        
        tableView.dataSource = self
        tableView.delegate = self
    }
    
    override func viewWillDisappear(animated: Bool) {
        super.viewWillDisappear(animated)

        if isClosing(self) {
            removeFromTTMessages(self)
        }
        
    }
    
    func showCodecDetail() {
        var codecdetail = ""
        switch channel.audiocodec.nCodec {
        case OPUS_CODEC :
            let opus = getOpusCodec(&channel.audiocodec).memory
            let chans = (opus.nChannels>1 ? "Stereo" : "Mono" )
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
            codecdetail = "No Audio"
        }
        codeccell!.detailTextLabel?.text = codecdetail
    }
    
    func getBandmodeString(bandmode : INT32) -> String {
        switch bandmode {
        case 2 :
            return "32 KHz"
        case 1 :
            return "16 KHz"
        case 0 :
            fallthrough
        default :
            return "8 KHz"
        }
    }
    
    @IBAction func createChannel(sender: UIBarButtonItem) {
        saveChannelDetail()
        
        //TODO: UIAlertView on failure
        
        if channel.nChannelID == 0 {
            cmdid = TT_DoJoinChannel(ttInst, &channel)
            
        }
        else {
            cmdid = TT_DoUpdateChannel(ttInst, &channel)
        }
    }
    
    @IBAction func joinChannelPressed(sender: UIButton) {
        
        cmdid = TT_DoJoinChannelByID(ttInst, channel.nChannelID, "")
    }
    
    @IBAction func deleteChannelPressed(sender: UIButton) {
        //TODO: UIAlertView on failure
        cmdid = TT_DoRemoveChannel(ttInst, channel.nChannelID)
    }
    
    func handleTTMessage(var m: TTMessage) {
        
        switch m.nClientEvent {
            
        case CLIENTEVENT_CMD_SUCCESS :
            if m.nSource == cmdid {
                let vc = self.navigationController?.viewControllers[1]
                
                self.navigationController?.popToViewController(vc!, animated: true)
            }
        case CLIENTEVENT_CMD_ERROR :
            if m.nSource == cmdid {
                var errmsg = getClientErrorMsg(&m).memory
                let s = String.fromCString(&errmsg.szErrorMsg.0)
                let alert = UIAlertController(title: "Error", message: s, preferredStyle: UIAlertControllerStyle.Alert)
                alert.addAction(UIAlertAction(title: "Click", style: UIAlertActionStyle.Default, handler: nil))
                self.presentViewController(alert, animated: true, completion: nil)
            }
        case CLIENTEVENT_CMD_PROCESSING :
            if !getBoolean(&m) && cmdid == m.nSource {
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
        toTTString(namefield!.text!, &channel.szName.0)
        toTTString(passwdfield!.text!, &channel.szPassword.0)
        toTTString(topicfield!.text!, &channel.szTopic.0)
        if permanentswitch!.on {
            channel.uChannelType |= CHANNEL_PERMANENT.rawValue
        }
        if nointerruptionsswitch!.on {
            channel.uChannelType |= CHANNEL_SOLO_TRANSMIT.rawValue
        }
        if novoiceactivationswitch!.on {
            channel.uChannelType |= CHANNEL_NO_VOICEACTIVATION.rawValue
        }
        if noaudiorecordingswitch!.on {
            channel.uChannelType |= CHANNEL_NO_RECORDING.rawValue
        }
        
    }
    
    func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 2
    }
    
    func tableView(tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch section {
        case 0 :
            return "Channel Properties"
        case 1 :
            return "Commands"
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
