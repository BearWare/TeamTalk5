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
    
    var items = [UITableViewCell]()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let namecell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        namefield = newTableCellTextField(namecell, "Name", String.fromCString(&channel.szName.0)!)
        items.append(namecell)
        
        let passwdcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        passwdfield = newTableCellTextField(passwdcell, "Password", String.fromCString(&channel.szPassword.0)!)
        items.append(passwdcell)
        
        let topiccell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        topicfield = newTableCellTextField(topiccell, "Topic", String.fromCString(&channel.szTopic.0)!)
        items.append(topiccell)
        
        codeccell = tableView.dequeueReusableCellWithIdentifier("Setup Codec Cell") as? UITableViewCell
        codeccell!.selectionStyle = .None
        codeccell!.textLabel!.text = "Audio Codec"
        showCodecDetail()
        items.append(codeccell!)
        
        let permanentcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        permanentswitch = newTableCellSwitch(permanentcell, "Permanent Channel", (channel.uChannelType & CHANNEL_PERMANENT.value) != 0)
        items.append(permanentcell)
        
        let nointerruptcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        nointerruptionsswitch = newTableCellSwitch(nointerruptcell, "No Interruptions", (channel.uChannelType & CHANNEL_SOLO_TRANSMIT.value) != 0)
        items.append(nointerruptcell)
        
        let novoiceactcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        novoiceactivationswitch = newTableCellSwitch(novoiceactcell, "No Voice Activation", (channel.uChannelType & CHANNEL_NO_VOICEACTIVATION.value) != 0)
        items.append(novoiceactcell)
        
        let noaudiorecordcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        noaudiorecordingswitch = newTableCellSwitch(noaudiorecordcell, "No Audio Recording", (channel.uChannelType & CHANNEL_NO_RECORDING.value) != 0)
        items.append(noaudiorecordcell)
        
        let blankcell = tableView.dequeueReusableCellWithIdentifier("Blank") as! UITableViewCell
        items.append(blankcell)
        
        let deletechan = tableView.dequeueReusableCellWithIdentifier("Delete Channel") as! UITableViewCell
        items.append(deletechan)
        
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
            codecdetail = "OPUS \(opus.nSampleRate / 1000) KHz " + chans
        case SPEEX_CODEC.value :
            let speex = getSpeexCodec(&channel.audiocodec).memory
            var sr = ""
            switch speex.nBandmode {
            case 2 :
                sr = "32 KHz"
            case 1 :
                sr = "16 KHz"
            case 0 :
                fallthrough
            default :
                sr = "8 KHz"
            }
            codecdetail = "Speex " + sr
        case SPEEX_VBR_CODEC.value :
            codecdetail = "Speex VBR"
        case NO_CODEC.value :
            fallthrough
        default :
            codecdetail = "No Audio"
        }
        codeccell!.detailTextLabel?.text = codecdetail
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
        return 1
    }
    
    //    func tableView(tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
    //    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        
        return items.count
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        
        return items[indexPath.row]
    }
    
}
