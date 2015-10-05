//
//  SpeexCodecDetailViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 4-10-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class SpeexCodecDetailViewController : UIViewController {

    var codec = SpeexCodec(nBandmode: DEFAULT_SPEEX_BANDMODE, nQuality: DEFAULT_SPEEX_QUALITY, nTxIntervalMSec: DEFAULT_SPEEX_DELAY, bStereoPlayback: DEFAULT_SPEEX_SIMSTEREO)
    
    @IBOutlet weak var sampleratesSegCtrl: UISegmentedControl!
    @IBOutlet weak var qualitySlider: UISlider!
    @IBOutlet weak var txintervalStepper: UIStepper!
    @IBOutlet weak var txintervalLabel: UILabel!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        sampleratesSegCtrl.selectedSegmentIndex = Int(codec.nBandmode)
        
        qualitySlider.minimumValue = 0
        qualitySlider.maximumValue = 10
        qualitySlider.value = Float(codec.nQuality)
        
        txintervalStepper.minimumValue = 20
        txintervalStepper.maximumValue = 100
        txintervalStepper.stepValue = 20
        txintervalStepper.value = within(20, 100, Double(codec.nTxIntervalMSec))
        txintervalChanged(txintervalStepper)
    }

    @IBAction func txintervalChanged(sender: UIStepper) {
        txintervalLabel.text = String(Int(sender.value)) + " ms"
    }
    
    func saveSpeexCodec() {
        codec.nBandmode = INT32(sampleratesSegCtrl.selectedSegmentIndex)
        codec.nQuality = INT32(qualitySlider.value)
        codec.nTxIntervalMSec = INT32(txintervalStepper.value)
    }
}
