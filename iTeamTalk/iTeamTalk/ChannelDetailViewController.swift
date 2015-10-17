//
//  ChannelDetailViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 16-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class ChannelDetailViewController : UIViewController, UITableViewDataSource, UITableViewDelegate {

    //shared TTInstance between all view controllers
    var ttInst = UnsafeMutablePointer<Void>()

    var channel = Channel()
    
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
        namefield = newTableCellTextField(namecell, "Name", String.fromCString(&channel.szName.0)!)
        chan_items.append(namecell)
        
        let passwdcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        passwdfield = newTableCellTextField(passwdcell, "Password", String.fromCString(&channel.szPassword.0)!)
        chan_items.append(passwdcell)
        
        let topiccell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        topicfield = newTableCellTextField(topiccell, "Topic", String.fromCString(&channel.szTopic.0)!)
        chan_items.append(topiccell)
        
        codeccell = tableView.dequeueReusableCellWithIdentifier("Setup Codec Cell") as? UITableViewCell
        codeccell!.selectionStyle = .None
        codeccell!.textLabel!.text = "Audio Codec"
        showCodecDetail()
        chan_items.append(codeccell!)
        
        let permanentcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        permanentswitch = newTableCellSwitch(permanentcell, "Permanent Channel", (channel.uChannelType & CHANNEL_PERMANENT.value) != 0)
        chan_items.append(permanentcell)
        
        let nointerruptcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        nointerruptionsswitch = newTableCellSwitch(nointerruptcell, "No Interruptions", (channel.uChannelType & CHANNEL_SOLO_TRANSMIT.value) != 0)
        chan_items.append(nointerruptcell)
        
        let novoiceactcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        novoiceactivationswitch = newTableCellSwitch(novoiceactcell, "No Voice Activation", (channel.uChannelType & CHANNEL_NO_VOICEACTIVATION.value) != 0)
        chan_items.append(novoiceactcell)
        
        let noaudiorecordcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        noaudiorecordingswitch = newTableCellSwitch(noaudiorecordcell, "No Audio Recording", (channel.uChannelType & CHANNEL_NO_RECORDING.value) != 0)
        chan_items.append(noaudiorecordcell)
        
        if channel.nChannelID != 0 {
            let joinchan = tableView.dequeueReusableCellWithIdentifier("Join Channel") as! UITableViewCell
            cmd_items.append(joinchan)
            
            let deletechan = tableView.dequeueReusableCellWithIdentifier("Delete Channel") as! UITableViewCell
            cmd_items.append(deletechan)
        }
        
        if !namefield!.text.isEmpty {
            navitem.title = namefield!.text
        }
        
        if channel.nChannelID == 0 {
            navitem.title = "Create Channel"
        }
        else {
        }
        
        tableView.dataSource = self
        tableView.delegate = self
    }
    
    func showCodecDetail() {
        var codecdetail = ""
        switch channel.audiocodec.nCodec.value {
        case OPUS_CODEC.value :
            let opus = getOpusCodec(&channel.audiocodec).memory
            let chans = (opus.nChannels>1 ? "Stereo" : "Mono" )
            codecdetail = "OPUS \(opus.nSampleRate / 1000) KHz \(opus.nBitRate / 1000) KB/s " + chans
        case SPEEX_CODEC.value :
            let speex = getSpeexCodec(&channel.audiocodec).memory
            codecdetail = "Speex " + getBandmodeString(speex.nBandmode)
        case SPEEX_VBR_CODEC.value :
            let speexvbr = getSpeexVBRCodec(&channel.audiocodec).memory
            codecdetail = "Speex VBR " + getBandmodeString(speexvbr.nBandmode)
        case NO_CODEC.value :
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
            let cmdid = TT_DoJoinChannel(ttInst, &channel)
        }
        else {
            let cmdid = TT_DoUpdateChannel(ttInst, &channel)
        }
    }
    
    @IBAction func joinChannelPressed(sender: UIButton) {
        
        let cmdid = TT_DoJoinChannelByID(ttInst, channel.nChannelID, "")
    }
    
    
    @IBAction func deleteChannelPressed(sender: UIButton) {
        //TODO: UIAlertView on failure
        let cmdid = TT_DoRemoveChannel(ttInst, channel.nChannelID)
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
            
            switch channel.audiocodec.nCodec.value {
            case SPEEX_CODEC.value :
                vc.speexcodec = getSpeexCodec(&channel.audiocodec).memory
            case SPEEX_VBR_CODEC.value :
                vc.speexvbrcodec = getSpeexVBRCodec(&channel.audiocodec).memory
            case OPUS_CODEC.value :
                vc.opuscodec = getOpusCodec(&channel.audiocodec).memory
            case NO_CODEC.value :
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
        toTTString(namefield!.text, &channel.szName.0)
        toTTString(passwdfield!.text, &channel.szPassword.0)
        toTTString(topicfield!.text, &channel.szTopic.0)
        if permanentswitch!.on {
            channel.uChannelType |= CHANNEL_PERMANENT.value
        }
        if nointerruptionsswitch!.on {
            channel.uChannelType |= CHANNEL_SOLO_TRANSMIT.value
        }
        if novoiceactivationswitch!.on {
            channel.uChannelType |= CHANNEL_NO_VOICEACTIVATION.value
        }
        if noaudiorecordingswitch!.on {
            channel.uChannelType |= CHANNEL_NO_RECORDING.value
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
