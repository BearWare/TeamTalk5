//
//  OpusCodecDetailViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 1-10-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class OpusCodecDetailViewController : UIViewController {
    
    var codec = OpusCodec(nSampleRate: DEFAULT_OPUS_SAMPLERATE,
        nChannels: DEFAULT_OPUS_CHANNELS, nApplication: DEFAULT_OPUS_APPLICATION,
        nComplexity: DEFAULT_OPUS_COMPLEXITY, bFEC: DEFAULT_OPUS_FEC,
        bDTX: DEFAULT_OPUS_DTX, nBitRate: DEFAULT_OPUS_BITRATE,
        bVBR: DEFAULT_OPUS_VBR, bVBRConstraint: DEFAULT_OPUS_VBRCONSTRAINT,
        nTxIntervalMSec: DEFAULT_MSEC_PER_PACKET)
    
    @IBOutlet weak var applicationSegCtrl: UISegmentedControl!
    @IBOutlet weak var samplerateSegCtrl: UISegmentedControl!
    @IBOutlet weak var audiochannelsSegCtrl: UISegmentedControl!
    @IBOutlet weak var dtxSwitch: UISwitch!
    @IBOutlet weak var bitrateLabel: UILabel!
    @IBOutlet weak var bitrateSlider: UISlider!
    @IBOutlet weak var txintervalStepper: UIStepper!
    @IBOutlet weak var txintervalLabel: UILabel!

    let applications: [Int32] = [OPUS_APPLICATION_VOIP, OPUS_APPLICATION_AUDIO]
    let samplerates: [Int32] = [8000, 12000, 16000, 24000, 48000]
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        if let i = find(applications, codec.nApplication) {
            applicationSegCtrl.selectedSegmentIndex = i
        }
        else {
            applicationSegCtrl.selectedSegmentIndex = 0
        }
        
        if let i = find(samplerates, codec.nSampleRate) {
            samplerateSegCtrl.selectedSegmentIndex = i
        }
        else {
            samplerateSegCtrl.selectedSegmentIndex = 4
        }
        
        switch codec.nChannels {
        case 2 :
            audiochannelsSegCtrl.selectedSegmentIndex = 1
        case 1 :
            fallthrough
        default :
            audiochannelsSegCtrl.selectedSegmentIndex = 0
        }

        bitrateSlider.minimumValue = Float(OPUS_MIN_BITRATE) / 1000.0
        bitrateSlider.maximumValue = Float(OPUS_MAX_BITRATE) / 1000.0
        var bitrate = within(OPUS_MIN_BITRATE, OPUS_MAX_BITRATE, codec.nBitRate)
        bitrateSlider.value = Float(bitrate) / 1000
        bitrateChanged(bitrateSlider)
        
        dtxSwitch.on = codec.bDTX != 0
        
        let TX_MIN = 20.0, TX_MAX = 60.0
        txintervalStepper.minimumValue = TX_MIN
        txintervalStepper.maximumValue = TX_MAX
        txintervalStepper.stepValue = 20
        
        var txinterval = within(TX_MIN, TX_MAX, Double(codec.nTxIntervalMSec))
        txintervalStepper.value = txinterval
        txintervalChanged(txintervalStepper)
    }
    
    @IBAction func bitrateChanged(sender: UISlider) {
        codec.nBitRate = Int32(sender.value * 1000)
        bitrateLabel.text = String(codec.nBitRate / 1000) + " KB/s"
    }
    
    @IBAction func txintervalChanged(sender: UIStepper) {
        txintervalLabel.text = String(Int(sender.value)) + " ms"
    }
    
    func saveOPUSCodec() {
        codec.nApplication = applications[applicationSegCtrl.selectedSegmentIndex]
        codec.nBitRate = Int32(bitrateSlider.value) * 1000
        codec.nSampleRate = samplerates[samplerateSegCtrl.selectedSegmentIndex]
        codec.nChannels = audiochannelsSegCtrl.selectedSegmentIndex + 1
        codec.nTxIntervalMSec = Int32(txintervalStepper.value)
        codec.bDTX = (dtxSwitch.on ? 1 : 0)
    }
    
}
