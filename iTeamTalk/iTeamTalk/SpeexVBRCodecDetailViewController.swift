//
//  SpeexVBRCodecDetailViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 5-10-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class SpeexVBRCodecDetailViewController : UIViewController {
    
    var codec = SpeexVBRCodec(nBandmode: DEFAULT_SPEEX_VBR_BANDMODE,
        nQuality: DEFAULT_SPEEX_VBR_QUALITY,
        nBitRate: DEFAULT_SPEEX_VBR_BITRATE,
        nMaxBitRate: DEFAULT_SPEEX_VBR_MAXBITRATE,
        bDTX: DEFAULT_SPEEX_VBR_DTX,
        nTxIntervalMSec: DEFAULT_SPEEX_VBR_DELAY,
        bStereoPlayback: DEFAULT_SPEEX_VBR_SIMSTEREO)
    
    @IBOutlet weak var sampleratesSegCtrl: UISegmentedControl!
    @IBOutlet weak var qualitySlider: UISlider!
    @IBOutlet weak var bitrateSlider: UISlider!
    @IBOutlet weak var bitrateLabel: UILabel!
    @IBOutlet weak var dtxSwitch: UISwitch!
    @IBOutlet weak var txintervalStepper: UIStepper!
    @IBOutlet weak var txintervalLabel: UILabel!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.tabBarController?.navigationItem.title = "Speex VBR"
        
        sampleratesSegCtrl.selectedSegmentIndex = Int(codec.nBandmode)
        
        qualitySlider.minimumValue = 0
        qualitySlider.maximumValue = 10
        qualitySlider.value = Float(codec.nQuality)
        
        bitrateSlider.minimumValue = 0
        bitrateSlider.maximumValue = Float(SPEEX_UWB_MAX_BITRATE) / 1000.0
        bitrateSlider.value = within(0,Float(SPEEX_UWB_MAX_BITRATE), Float(codec.nMaxBitRate) / 1000)
        bitrateChanged(bitrateSlider)
        
        dtxSwitch.on = codec.bDTX != 0
        
        txintervalStepper.minimumValue = 20
        txintervalStepper.maximumValue = 100
        txintervalStepper.stepValue = 20
        txintervalStepper.value = within(20, 100, Double(codec.nTxIntervalMSec))
        txintervalChanged(txintervalStepper)
    }
    
    @IBAction func bitrateChanged(sender: UISlider) {
        bitrateLabel.text = "\(Int(bitrateSlider.value)) KB/s"
    }
    
    @IBAction func txintervalChanged(sender: UIStepper) {
        txintervalLabel.text = "\(Int(sender.value)) ms"
    }
    
    func saveSpeexCodec() {
        codec.nBandmode = INT32(sampleratesSegCtrl.selectedSegmentIndex)
        codec.nQuality = INT32(qualitySlider.value)
        codec.nMaxBitRate = INT32(bitrateSlider.value * 1000)
        codec.bDTX = (dtxSwitch.on ? 1 : 0)
        codec.nTxIntervalMSec = INT32(txintervalStepper.value)
    }
}