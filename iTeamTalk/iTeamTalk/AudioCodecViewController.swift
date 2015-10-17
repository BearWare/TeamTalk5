//
//  AudioCodecViewController.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 14-10-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

import UIKit

class AudioCodecViewController : UITableViewController {
    
    var audiocodec = AudioCodec()

    var opuscodec = OpusCodec(nSampleRate: DEFAULT_OPUS_SAMPLERATE,
        nChannels: DEFAULT_OPUS_CHANNELS, nApplication: DEFAULT_OPUS_APPLICATION,
        nComplexity: DEFAULT_OPUS_COMPLEXITY, bFEC: DEFAULT_OPUS_FEC,
        bDTX: DEFAULT_OPUS_DTX, nBitRate: DEFAULT_OPUS_BITRATE,
        bVBR: DEFAULT_OPUS_VBR, bVBRConstraint: DEFAULT_OPUS_VBRCONSTRAINT,
        nTxIntervalMSec: DEFAULT_MSEC_PER_PACKET)
    
    var speexcodec = SpeexCodec(nBandmode: DEFAULT_SPEEX_BANDMODE, nQuality: DEFAULT_SPEEX_QUALITY, nTxIntervalMSec: DEFAULT_SPEEX_DELAY, bStereoPlayback: DEFAULT_SPEEX_SIMSTEREO)
    
    var speexvbrcodec = SpeexVBRCodec(nBandmode: DEFAULT_SPEEX_VBR_BANDMODE,
        nQuality: DEFAULT_SPEEX_VBR_QUALITY,
        nBitRate: DEFAULT_SPEEX_VBR_BITRATE,
        nMaxBitRate: DEFAULT_SPEEX_VBR_MAXBITRATE,
        bDTX: DEFAULT_SPEEX_VBR_DTX,
        nTxIntervalMSec: DEFAULT_SPEEX_VBR_DELAY,
        bStereoPlayback: DEFAULT_SPEEX_VBR_SIMSTEREO)
    
    var opus_items = [UITableViewCell]()
    var speex_items = [UITableViewCell]()
    var speexvbr_items = [UITableViewCell]()
    var noaudio_items = [UITableViewCell]()
    
    // Opus section
    var opus_appSegCtrl: UISegmentedControl?
    var opus_srSegCtrl: UISegmentedControl!
    var opus_chansSegCtrl: UISegmentedControl?
    var opus_dtxSwitch: UISwitch?
    var opus_bitrateSlider: UISlider?
    var opus_bitrateCell : UITableViewCell?
    var opus_txintervalStepper: UIStepper?
    var opus_txintervalCell : UITableViewCell?
    
    // Speex section
    var spx_srSegCtrl: UISegmentedControl?
    var spx_qualitySlider: UISlider?
    var spx_txintervalStepper: UIStepper?
    var spx_txintervalCell: UITableViewCell?
    
    // Speex VBR section
    var spxvbr_srSegCtrl: UISegmentedControl?
    var spxvbr_qualitySlider: UISlider?
    var spxvbr_bitrateSlider: UISlider?
    var spxvbr_bitrateCell: UITableViewCell?
    var spxvbr_dtxSwitch: UISwitch?
    var spxvbr_txintervalStepper: UIStepper?
    var spxvbr_txintervalCell: UITableViewCell?
    
    let opus_applications: [Int32] = [OPUS_APPLICATION_VOIP, OPUS_APPLICATION_AUDIO]
    let opus_samplerates: [Int32] = [8000, 12000, 16000, 24000, 48000]
    
    var sections = [Int : Codec]()
    
    override func viewDidLoad() {
        super.viewDidLoad()

        switch audiocodec.nCodec.value {
        case OPUS_CODEC.value :
            sections[0] = OPUS_CODEC
            sections[1] = SPEEX_CODEC
            sections[2] = SPEEX_VBR_CODEC
            sections[3] = NO_CODEC
        case SPEEX_CODEC.value :
            sections[1] = OPUS_CODEC
            sections[0] = SPEEX_CODEC
            sections[2] = SPEEX_VBR_CODEC
            sections[3] = NO_CODEC
        case SPEEX_VBR_CODEC.value :
            sections[1] = OPUS_CODEC
            sections[2] = SPEEX_CODEC
            sections[0] = SPEEX_VBR_CODEC
            sections[3] = NO_CODEC
        case NO_CODEC.value :
            fallthrough
        default :
            sections[1] = OPUS_CODEC
            sections[2] = SPEEX_CODEC
            sections[3] = SPEEX_VBR_CODEC
            sections[0] = NO_CODEC
        }
        
        setupOpus()
        setupSpeex()
        setupSpeexVBR()
        setupNoAudio()
    }
    
    func setupNoAudio() {
        let cell = tableView.dequeueReusableCellWithIdentifier("Use No Audio") as! UITableViewCell
        noaudio_items.append(cell)
        
        let blank = tableView.dequeueReusableCellWithIdentifier("Blank") as! UITableViewCell
        noaudio_items.append(blank)
    }
    
    func setupOpus() {

        let opus_appcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        opus_appSegCtrl = newTableCellSegCtrl(opus_appcell, "Application", ["VoIP", "Music"])
        opus_items.append(opus_appcell)
        
        let opus_srcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        opus_srSegCtrl = newTableCellSegCtrl(opus_srcell, "Sample Rate", ["8 KHz", "12 KHz", "16 KHz", "24 KHz", "48 KHz"])
        opus_items.append(opus_srcell)
        
        let opus_chanscell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        opus_chansSegCtrl = newTableCellSegCtrl(opus_chanscell, "Audio Channels", ["Mono", "Stereo"])
        opus_items.append(opus_chanscell)
        
        let bitrate = within(OPUS_MIN_BITRATE, OPUS_MAX_BITRATE, opuscodec.nBitRate)
        opus_bitrateCell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        opus_bitrateSlider = newTableCellSlider(opus_bitrateCell!, "Bitrate", Float(OPUS_MIN_BITRATE) / 1000.0, Float(OPUS_MAX_BITRATE) / 1000.0, Float(bitrate) / 1000)
        opus_bitrateSlider?.addTarget(self, action: "opus_bitrateChanged:", forControlEvents: .ValueChanged)
        opus_items.append(opus_bitrateCell!)
        
        let opus_dtxcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        opus_dtxSwitch = newTableCellSwitch(opus_dtxcell, "DTX", opuscodec.bDTX != 0)
        opus_items.append(opus_dtxcell)
        
        opus_txintervalCell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        opus_txintervalStepper = newTableCellStepper(opus_txintervalCell!, "Transmit Interval", 20, 60, 20, Double(opuscodec.nTxIntervalMSec))
        opus_txintervalStepper?.addTarget(self, action: "opus_txintervalChanged:", forControlEvents: .ValueChanged)
        opus_items.append(opus_txintervalCell!)

        let opus_savecell = tableView.dequeueReusableCellWithIdentifier("Use OPUS") as! UITableViewCell
        opus_items.append(opus_savecell)
        
        if let i = find(opus_applications, opuscodec.nApplication) {
            opus_appSegCtrl!.selectedSegmentIndex = i
        }
        else {
            opus_appSegCtrl!.selectedSegmentIndex = 0
        }
        
        if let i = find(opus_samplerates, opuscodec.nSampleRate) {
            opus_srSegCtrl!.selectedSegmentIndex = i
        }
        else {
            opus_srSegCtrl!.selectedSegmentIndex = 4
        }
        
        switch opuscodec.nChannels {
        case 2 :
            opus_chansSegCtrl!.selectedSegmentIndex = 1
        case 1 :
            fallthrough
        default :
            opus_chansSegCtrl!.selectedSegmentIndex = 0
        }
        
        opus_bitrateChanged(opus_bitrateSlider!)
        
        opus_txintervalChanged(opus_txintervalStepper!)

        let blank = tableView.dequeueReusableCellWithIdentifier("Blank") as! UITableViewCell
        opus_items.append(blank)
    }
    
    func opus_bitrateChanged(sender: UISlider) {
        opus_bitrateCell?.detailTextLabel!.text =  "\(sender.value) KB/s"
    }
    
    func opus_txintervalChanged(sender: UIStepper) {
         opus_txintervalCell?.detailTextLabel!.text = String(Int(sender.value)) + " ms"
    }
    
    func saveOPUSCodec() {
        opuscodec.nApplication = opus_applications[opus_appSegCtrl!.selectedSegmentIndex]
        opuscodec.nBitRate = Int32(opus_bitrateSlider!.value) * 1000
        opuscodec.nSampleRate = opus_samplerates[opus_srSegCtrl!.selectedSegmentIndex]
        opuscodec.nChannels = opus_chansSegCtrl!.selectedSegmentIndex + 1
        opuscodec.nTxIntervalMSec = Int32(opus_txintervalStepper!.value)
        opuscodec.bDTX = (opus_dtxSwitch!.on ? 1 : 0)
    }
    
    func setupSpeex() {
        
        let srcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        spx_srSegCtrl = newTableCellSegCtrl(srcell, "Sample Rate", ["8 KHz", "16 KHz", "32 KHz"])
        spx_srSegCtrl!.selectedSegmentIndex = Int(speexcodec.nBandmode)
        speex_items.append(srcell)
        
        let qcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        spx_qualitySlider = newTableCellSlider(qcell, "Quality", 0, 10, Float(speexcodec.nQuality))
        speex_items.append(qcell)
        
        spx_txintervalCell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        spx_txintervalStepper = newTableCellStepper(spx_txintervalCell!, "Transmit Interval", 20, 100, 20, Double(speexcodec.nTxIntervalMSec))
        speex_txintervalChanged(spx_txintervalStepper!)
        spx_txintervalStepper?.addTarget(self, action: "speex_txintervalChanged:", forControlEvents: .ValueChanged)
        speex_items.append(spx_txintervalCell!)
        
        let savebtn = tableView.dequeueReusableCellWithIdentifier("Use Speex") as! UITableViewCell
        speex_items.append(savebtn)

        let blank = tableView.dequeueReusableCellWithIdentifier("Blank") as! UITableViewCell
        speex_items.append(blank)

    }

    func speex_txintervalChanged(sender: UIStepper) {
        spx_txintervalCell?.detailTextLabel!.text = String(Int(sender.value)) + " ms"
    }
    
    func saveSpeexCodec() {
        speexcodec.nBandmode = INT32(spx_srSegCtrl!.selectedSegmentIndex)
        speexcodec.nQuality = INT32(spx_qualitySlider!.value)
        speexcodec.nTxIntervalMSec = INT32(spx_txintervalStepper!.value)
    }
    
    func setupSpeexVBR() {
        let srcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        spxvbr_srSegCtrl = newTableCellSegCtrl(srcell, "Sample Rate", ["8 KHz", "16 KHz", "32 KHz"])
        spxvbr_srSegCtrl!.selectedSegmentIndex = Int(speexvbrcodec.nBandmode)
        speexvbr_items.append(srcell)
        
        let qcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        spxvbr_qualitySlider = newTableCellSlider(qcell, "Quality", 0, 10, Float(speexvbrcodec.nQuality))
        speexvbr_items.append(qcell)
        
        spxvbr_bitrateCell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        spxvbr_bitrateSlider = newTableCellSlider(spxvbr_bitrateCell!, "Bitrate", 0, Float(SPEEX_UWB_MAX_BITRATE) / 1000.0,
                                                  within(0, Float(SPEEX_UWB_MAX_BITRATE), Float(speexvbrcodec.nMaxBitRate) / 1000))
        speexvbr_bitrateChanged(spxvbr_bitrateSlider!)
        spxvbr_bitrateSlider?.addTarget(self, action: "speexvbr_bitrateChanged:", forControlEvents: .ValueChanged)
        speexvbr_items.append(spxvbr_bitrateCell!)
        
        let dtxcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        spxvbr_dtxSwitch = newTableCellSwitch(dtxcell, "DTX", speexvbrcodec.bDTX != 0)
        speexvbr_items.append(dtxcell)
        
        spxvbr_txintervalCell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        spxvbr_txintervalStepper = newTableCellStepper(spxvbr_txintervalCell!, "Transmit Interval", 20, 100, 20, Double(speexvbrcodec.nTxIntervalMSec))
        speexvbr_txintervalChanged(spxvbr_txintervalStepper!)
        spxvbr_txintervalStepper?.addTarget(self, action: "speexvbr_txintervalChanged:", forControlEvents: .ValueChanged)
        speexvbr_items.append(spxvbr_txintervalCell!)
        
        let savebtn = tableView.dequeueReusableCellWithIdentifier("Use Speex VBR") as! UITableViewCell
        speexvbr_items.append(savebtn)
        
        let blank = tableView.dequeueReusableCellWithIdentifier("Blank") as! UITableViewCell
        speexvbr_items.append(blank)
    }

    func speexvbr_bitrateChanged(sender: UISlider) {
        spxvbr_bitrateCell?.detailTextLabel!.text = String(Int(sender.value)) + " KB/s"
    }
    
    func speexvbr_txintervalChanged(sender: UIStepper) {
        spxvbr_txintervalCell?.detailTextLabel!.text = String(Int(sender.value)) + " ms"
    }

    func saveSpeexVBRCodec() {
        speexvbrcodec.nBandmode = INT32(spxvbr_srSegCtrl!.selectedSegmentIndex)
        speexvbrcodec.nQuality = INT32(spxvbr_qualitySlider!.value)
        speexvbrcodec.nMaxBitRate = INT32(spxvbr_bitrateSlider!.value * 1000)
        speexvbrcodec.bDTX = (spxvbr_dtxSwitch!.on ? 1 : 0)
        speexvbrcodec.nTxIntervalMSec = INT32(spxvbr_txintervalStepper!.value)
    }
    
    override func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return sections.count
    }
    
    override func tableView(tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        var title = ""
        var active = false
        
        switch sections[section]!.value {
        case OPUS_CODEC.value :
            title = "OPUS Codec"
            active = audiocodec.nCodec.value == OPUS_CODEC.value
        case SPEEX_CODEC.value :
            title = "Speex Codec"
            active = audiocodec.nCodec.value == SPEEX_CODEC.value
        case SPEEX_VBR_CODEC.value :
            title = "Speex Variable Bitrate Codec"
            active = audiocodec.nCodec.value == SPEEX_VBR_CODEC.value
        case NO_CODEC.value :
            title = "No Audio"
            active = audiocodec.nCodec.value == NO_CODEC.value
        default :
            return nil
        }
        
        if active {
            return title + " (Active)"
        }
        return title
    }
    
    override func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        
        switch sections[section]!.value {
        case OPUS_CODEC.value :
            return opus_items.count
        case SPEEX_CODEC.value :
            return speex_items.count
        case SPEEX_VBR_CODEC.value :
            return speexvbr_items.count
        case NO_CODEC.value :
            return noaudio_items.count
        default :
            return 0
        }
    }
    
    override func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        
        switch sections[indexPath.section]!.value {
        case OPUS_CODEC.value :
            return opus_items[indexPath.row]
        case SPEEX_CODEC.value :
            return speex_items[indexPath.row]
        case SPEEX_VBR_CODEC.value :
            return speexvbr_items[indexPath.row]
        case NO_CODEC.value :
            return noaudio_items[indexPath.row]
        default :
            return UITableViewCell()
        }
    }
}
