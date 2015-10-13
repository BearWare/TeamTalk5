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
    
    var namefield: UITextField?
    var passwdfield: UITextField?
    var topicfield: UITextField?
    var permanentswitch: UISwitch?
    var nointerruptionsswitch: UISwitch?
    var novoiceactivationswitch: UISwitch?
    var noaudiorecordingswitch: UISwitch?
    
    @IBOutlet weak var createBtn: UIButton!
    @IBOutlet weak var deleteBtn: UIButton!
    
    @IBOutlet weak var tableView: UITableView!
    
    var items = [UITableViewCell]()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let (namecell, namefield) = newTableCell("Name", String.fromCString(&channel.szName.0)!)
        self.namefield = namefield
        items.append(namecell)
        
        let (passwdcell, passwdfield) = newTableCell("Password", String.fromCString(&channel.szPassword.0)!)
        self.passwdfield = passwdfield
        items.append(passwdcell)
        
        let (topiccell, topicfield) = newTableCell("Topic", String.fromCString(&channel.szTopic.0)!)
        self.topicfield = topicfield
        items.append(topiccell)
        
        let codeccell = tableView.dequeueReusableCellWithIdentifier("Setup Codec Cell") as! UITableViewCell
        codeccell.selectionStyle = .None
        codeccell.textLabel!.text = "Audio Codec"
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
        codeccell.detailTextLabel?.text = codecdetail
        items.append(codeccell)
        
        let (permanentcell, permanentswitch) = newTableCell("Permanent Channel", (channel.uChannelType & CHANNEL_PERMANENT.value) != 0)
        self.permanentswitch = permanentswitch
        items.append(permanentcell)
        
        let (nointerruptcell, nointerruptionsswitch) = newTableCell("No Interruptions", (channel.uChannelType & CHANNEL_SOLO_TRANSMIT.value) != 0)
        self.nointerruptionsswitch = nointerruptionsswitch
        items.append(nointerruptcell)
        
        let (novoiceactcell, novoiceactivationswitch) = newTableCell("No Voice Activation", (channel.uChannelType & CHANNEL_NO_VOICEACTIVATION.value) != 0)
        self.novoiceactivationswitch = novoiceactivationswitch
        items.append(novoiceactcell)
        
        let (noaudiorecordcell, noaudiorecordingswitch) = newTableCell("No Audio Recording", (channel.uChannelType & CHANNEL_NO_RECORDING.value) != 0)
        self.noaudiorecordingswitch = noaudiorecordingswitch
        items.append(noaudiorecordcell)
        
        if !namefield.text.isEmpty {
            navitem.title = namefield.text
        }
        
        if channel.nChannelID == 0 {
            createBtn.setTitle("Create Channel", forState: .Normal)
            navitem.title = "Create Channel"
            deleteBtn.hidden = true
        }
        else {
            createBtn.setTitle("Update Channel", forState: .Normal)
        }
        
        tableView.dataSource = self
        tableView.delegate = self
    }
    
    @IBAction func createChannel(sender: UIButton) {
        saveChannelDetail()
        
        //TODO: UIAlertView on failure
        
        if channel.nChannelID != 0 {
            let cmdid = TT_DoJoinChannel(ttInst, &channel)
        }
        else {
            let cmdid = TT_DoUpdateChannel(ttInst, &channel)
        }
    }
    
    @IBAction func deleteChannel(sender: UIButton) {
        //TODO: UIAlertView on failure
        let cmdid = TT_DoRemoveChannel(ttInst, channel.nChannelID)
    }

    @IBAction func saveNoAudioCodec(segue:UIStoryboardSegue) {
        channel.audiocodec.nCodec = NO_CODEC
    }

    @IBAction func saveOpusCodec(segue:UIStoryboardSegue) {
        
        if segue.sourceViewController is OpusCodecDetailViewController {
            let vc = segue.sourceViewController as! OpusCodecDetailViewController
            vc.saveOPUSCodec()
            setOpusCodec(&channel.audiocodec, &vc.codec)
        }
    }
    
    @IBAction func saveSpeexCodec(segue:UIStoryboardSegue) {
        
        if segue.sourceViewController is SpeexCodecDetailViewController {
            let vc = segue.sourceViewController as! SpeexCodecDetailViewController
            vc.saveSpeexCodec()
            setSpeexCodec(&channel.audiocodec, &vc.codec)
        }
    }

    @IBAction func saveSpeexVBRCodec(segue:UIStoryboardSegue) {
        
        if segue.sourceViewController is SpeexVBRCodecDetailViewController {
            let vc = segue.sourceViewController as! SpeexVBRCodecDetailViewController
            vc.saveSpeexCodec()
            setSpeexVBRCodec(&channel.audiocodec, &vc.codec)
        }
    }

    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        
        if segue.identifier == "Setup Audio Codec" {
            
            let vc = segue.destinationViewController as! UITabBarController
            
            switch channel.audiocodec.nCodec.value {
            case NO_CODEC.value :
                if channel.nChannelID == 0 {
                    // we're creating a new channel (set Opus as default)
                    vc.selectedIndex = 1
                }
                else {
                    vc.selectedIndex = 0
                }
            case SPEEX_CODEC.value :
                let spxview = vc.viewControllers![2] as! SpeexCodecDetailViewController
                spxview.codec = getSpeexCodec(&channel.audiocodec).memory
                vc.selectedIndex = 2
            case SPEEX_VBR_CODEC.value :
                let spxview = vc.viewControllers![3] as! SpeexVBRCodecDetailViewController
                spxview.codec = getSpeexVBRCodec(&channel.audiocodec).memory
                vc.selectedIndex = 3
            case OPUS_CODEC.value :
                let opusview = vc.viewControllers![1] as! OpusCodecDetailViewController
                opusview.codec = getOpusCodec(&channel.audiocodec).memory
                fallthrough
            default :
                vc.selectedIndex = 1
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
