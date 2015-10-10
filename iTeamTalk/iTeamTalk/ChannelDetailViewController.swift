//
//  ChannelDetailViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 16-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class ChannelDetailViewController : UIViewController {

    //shared TTInstance between all view controllers
    var ttInst = UnsafeMutablePointer<Void>()

    var channel = Channel()
    
    @IBOutlet weak var navitem: UINavigationItem!
    @IBOutlet weak var name: UITextField!
    @IBOutlet weak var password: UITextField!
    @IBOutlet weak var topic: UITextField!
    @IBOutlet weak var permanentchannel: UISwitch!
    @IBOutlet weak var nointerruptions: UISwitch!
    @IBOutlet weak var novoiceactivation: UISwitch!
    @IBOutlet weak var noaudiorecording: UISwitch!
    @IBOutlet weak var createBtn: UIButton!
    @IBOutlet weak var deleteBtn: UIButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        name.text = String.fromCString(&channel.szName.0)
        password.text = String.fromCString(&channel.szPassword.0)
        topic.text = String.fromCString(&channel.szTopic.0)
        permanentchannel.on = (channel.uChannelType & CHANNEL_PERMANENT.value) != 0
        nointerruptions.on = (channel.uChannelType & CHANNEL_SOLO_TRANSMIT.value) != 0
        novoiceactivation.on = (channel.uChannelType & CHANNEL_NO_VOICEACTIVATION.value) != 0
        noaudiorecording.on = (channel.uChannelType & CHANNEL_NO_RECORDING.value) != 0
        
        if !name.text.isEmpty {
            navitem.title = name.text
        }
        
        if channel.nChannelID == 0 {
            createBtn.setTitle("Create Channel", forState: .Normal)
            navitem.title = "Create Channel"
            deleteBtn.hidden = true
        }
        else {
            createBtn.setTitle("Update Channel", forState: .Normal)
        }
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
        toTTString(name.text!, &channel.szName.0)
        toTTString(password.text!, &channel.szPassword.0)
        toTTString(topic.text!, &channel.szTopic.0)
        if permanentchannel.on {
            channel.uChannelType |= CHANNEL_PERMANENT.value
        }
        if nointerruptions.on {
            channel.uChannelType |= CHANNEL_SOLO_TRANSMIT.value
        }
        if novoiceactivation.on {
            channel.uChannelType |= CHANNEL_NO_VOICEACTIVATION.value
        }
        if noaudiorecording.on {
            channel.uChannelType |= CHANNEL_NO_RECORDING.value
        }
    }
    
}
