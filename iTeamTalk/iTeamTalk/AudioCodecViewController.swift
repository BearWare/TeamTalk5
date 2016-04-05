/*
* Copyright (c) 2005-2016, BearWare.dk
*
* Contact Information:
*
* Bjoern D. Rasmussen
* Skanderborgvej 40 4-2
* DK-8000 Aarhus C
* Denmark
* Email: contact@bearware.dk
* Phone: +45 20 20 54 59
* Web: http://www.bearware.dk
*
* This source code is part of the TeamTalk 5 SDK owned by
* BearWare.dk. All copyright statements may not be removed
* or altered from any source distribution. If you use this
* software in a product, an acknowledgment in the product
* documentation is required.
*
*/

import UIKit

class AudioCodecViewController : UITableViewController {
    
    var audiocodec = newAudioCodec(DEFAULT_AUDIOCODEC)

    var opuscodec = newOpusCodec()
    var speexcodec = newSpeexCodec()
    var speexvbrcodec = newSpeexVBRCodec()
    
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

        switch audiocodec.nCodec {
        case OPUS_CODEC :
            sections[0] = OPUS_CODEC
            sections[1] = SPEEX_CODEC
            sections[2] = SPEEX_VBR_CODEC
            sections[3] = NO_CODEC
        case SPEEX_CODEC :
            sections[1] = OPUS_CODEC
            sections[0] = SPEEX_CODEC
            sections[2] = SPEEX_VBR_CODEC
            sections[3] = NO_CODEC
        case SPEEX_VBR_CODEC :
            sections[1] = OPUS_CODEC
            sections[2] = SPEEX_CODEC
            sections[0] = SPEEX_VBR_CODEC
            sections[3] = NO_CODEC
        case NO_CODEC :
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
        let cell = tableView.dequeueReusableCellWithIdentifier("Use No Audio")!
        noaudio_items.append(cell)
        
        let blank = tableView.dequeueReusableCellWithIdentifier("Blank")!
        noaudio_items.append(blank)
    }
    
    func setupOpus() {

        let opus_appcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        opus_appSegCtrl = newTableCellSegCtrl(opus_appcell, label: NSLocalizedString("Application", comment:"codec detail"),
            values: [NSLocalizedString("VoIP", comment:"codec detail"),
                NSLocalizedString("Music", comment:"codec detail")])
        opus_items.append(opus_appcell)
        
        let opus_srcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        opus_srSegCtrl = newTableCellSegCtrl(opus_srcell, label: NSLocalizedString("Sample Rate", comment: "codec detail"),
            values: [NSLocalizedString("8 KHz", comment:"codec detail"),
                NSLocalizedString("12 KHz", comment:"codec detail"),
                NSLocalizedString("16 KHz", comment:"codec detail"),
                NSLocalizedString("24 KHz", comment:"codec detail"),
                NSLocalizedString("48 KHz", comment:"codec detail")])
        opus_items.append(opus_srcell)
        
        let opus_chanscell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        opus_chansSegCtrl = newTableCellSegCtrl(opus_chanscell, label: NSLocalizedString("Audio Channels", comment: "codec detail"),
            values: [NSLocalizedString("Mono", comment:"codec detail"),
                NSLocalizedString("Stereo", comment:"codec detail")])
        opus_items.append(opus_chanscell)
        
        let bitrate = within(OPUS_MIN_BITRATE, max_v: OPUS_MAX_BITRATE, value: opuscodec.nBitRate)
        opus_bitrateCell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        opus_bitrateSlider = newTableCellSlider(opus_bitrateCell!, label: NSLocalizedString("Bitrate", comment:"codec detail"), min: Float(OPUS_MIN_BITRATE) / 1000.0, max: Float(OPUS_MAX_BITRATE) / 1000.0, initial: Float(bitrate) / 1000)
        opus_bitrateSlider?.addTarget(self, action: #selector(AudioCodecViewController.opus_bitrateChanged(_:)), forControlEvents: .ValueChanged)
        opus_items.append(opus_bitrateCell!)
        
        let opus_dtxcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        opus_dtxSwitch = newTableCellSwitch(opus_dtxcell, label: NSLocalizedString("DTX", comment:"codec detail"), initial: opuscodec.bDTX != 0)
        opus_items.append(opus_dtxcell)
        
        opus_txintervalCell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        opus_txintervalStepper = newTableCellStepper(opus_txintervalCell!, label: NSLocalizedString("Transmit Interval", comment:"codec detail"), min: 20, max: 60, step: 20, initial: Double(opuscodec.nTxIntervalMSec))
        opus_txintervalStepper?.addTarget(self, action: #selector(AudioCodecViewController.opus_txintervalChanged(_:)), forControlEvents: .ValueChanged)
        opus_items.append(opus_txintervalCell!)

        let opus_savecell = tableView.dequeueReusableCellWithIdentifier("Use OPUS")!
        opus_items.append(opus_savecell)
        
        if let i = opus_applications.indexOf(opuscodec.nApplication) {
            opus_appSegCtrl!.selectedSegmentIndex = i
        }
        else {
            opus_appSegCtrl!.selectedSegmentIndex = 0
        }
        
        if let i = opus_samplerates.indexOf(opuscodec.nSampleRate) {
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

        let blank = tableView.dequeueReusableCellWithIdentifier("Blank")!
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
        spx_srSegCtrl = newTableCellSegCtrl(srcell,
            label: NSLocalizedString("Sample Rate", comment:"codec detail"),
            values: [NSLocalizedString("8 KHz", comment:"codec detail"), NSLocalizedString("16 KHz", comment:"codec detail"), NSLocalizedString("32 KHz", comment:"codec detail")])
        spx_srSegCtrl!.selectedSegmentIndex = Int(speexcodec.nBandmode)
        speex_items.append(srcell)
        
        let qcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        spx_qualitySlider = newTableCellSlider(qcell,
            label: NSLocalizedString("Quality", comment:"codec detail"),
            min: 0, max: 10, initial: Float(speexcodec.nQuality))
        speex_items.append(qcell)
        
        spx_txintervalCell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        spx_txintervalStepper = newTableCellStepper(spx_txintervalCell!,
            label: NSLocalizedString("Transmit Interval", comment:"codec detail"),
            min: 20, max: 100, step: 20, initial: Double(speexcodec.nTxIntervalMSec))
        speex_txintervalChanged(spx_txintervalStepper!)
        spx_txintervalStepper?.addTarget(self, action: #selector(AudioCodecViewController.speex_txintervalChanged(_:)), forControlEvents: .ValueChanged)
        speex_items.append(spx_txintervalCell!)
        
        let savebtn = tableView.dequeueReusableCellWithIdentifier("Use Speex")!
        speex_items.append(savebtn)

        let blank = tableView.dequeueReusableCellWithIdentifier("Blank")!
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
        spxvbr_srSegCtrl = newTableCellSegCtrl(srcell, label: NSLocalizedString("Sample Rate", comment:"codec detail"), values: ["8 KHz", "16 KHz", "32 KHz"])
        spxvbr_srSegCtrl!.selectedSegmentIndex = Int(speexvbrcodec.nBandmode)
        speexvbr_items.append(srcell)
        
        let qcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        spxvbr_qualitySlider = newTableCellSlider(qcell, label: NSLocalizedString("Quality", comment:"codec detail"), min: 0, max: 10, initial: Float(speexvbrcodec.nQuality))
        speexvbr_items.append(qcell)
        
        spxvbr_bitrateCell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        spxvbr_bitrateSlider = newTableCellSlider(spxvbr_bitrateCell!,
            label: NSLocalizedString("Bitrate", comment:"codec detail"), min: 0, max: Float(SPEEX_UWB_MAX_BITRATE) / 1000.0,
            initial: within(0, max_v: Float(SPEEX_UWB_MAX_BITRATE), value: Float(speexvbrcodec.nMaxBitRate) / 1000))
        speexvbr_bitrateChanged(spxvbr_bitrateSlider!)
        spxvbr_bitrateSlider?.addTarget(self, action: #selector(AudioCodecViewController.speexvbr_bitrateChanged(_:)), forControlEvents: .ValueChanged)
        speexvbr_items.append(spxvbr_bitrateCell!)
        
        let dtxcell = UITableViewCell(style: .Default, reuseIdentifier: nil)
        spxvbr_dtxSwitch = newTableCellSwitch(dtxcell, label: "DTX", initial: speexvbrcodec.bDTX != 0)
        speexvbr_items.append(dtxcell)
        
        spxvbr_txintervalCell = UITableViewCell(style: .Subtitle, reuseIdentifier: nil)
        spxvbr_txintervalStepper = newTableCellStepper(spxvbr_txintervalCell!,
            label: NSLocalizedString("Transmit Interval", comment:"codec detail"), min: 20, max: 100, step: 20, initial: Double(speexvbrcodec.nTxIntervalMSec))
        speexvbr_txintervalChanged(spxvbr_txintervalStepper!)
        spxvbr_txintervalStepper?.addTarget(self, action: #selector(AudioCodecViewController.speexvbr_txintervalChanged(_:)), forControlEvents: .ValueChanged)
        speexvbr_items.append(spxvbr_txintervalCell!)
        
        let savebtn = tableView.dequeueReusableCellWithIdentifier("Use Speex VBR")!
        speexvbr_items.append(savebtn)
        
        let blank = tableView.dequeueReusableCellWithIdentifier("Blank")!
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
        
        switch sections[section]! {
        case OPUS_CODEC :
            title = NSLocalizedString("OPUS Codec", comment:"codec detail")
            active = audiocodec.nCodec == OPUS_CODEC
        case SPEEX_CODEC :
            title = NSLocalizedString("Speex Codec", comment:"codec detail")
            active = audiocodec.nCodec == SPEEX_CODEC
        case SPEEX_VBR_CODEC :
            title = NSLocalizedString("Speex Variable Bitrate Codec", comment:"codec detail")
            active = audiocodec.nCodec == SPEEX_VBR_CODEC
        case NO_CODEC :
            title = NSLocalizedString("No Audio", comment:"codec detail")
            active = audiocodec.nCodec == NO_CODEC
        default :
            return nil
        }
        
        if active {
            return title + " " + NSLocalizedString("(Active)", comment:"codec detail")
        }
        return title
    }
    
    override func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        
        switch sections[section]! {
        case OPUS_CODEC :
            return opus_items.count
        case SPEEX_CODEC :
            return speex_items.count
        case SPEEX_VBR_CODEC :
            return speexvbr_items.count
        case NO_CODEC :
            return noaudio_items.count
        default :
            return 0
        }
    }
    
    override func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        
        switch sections[indexPath.section]! {
        case OPUS_CODEC :
            return opus_items[indexPath.row]
        case SPEEX_CODEC :
            return speex_items[indexPath.row]
        case SPEEX_VBR_CODEC :
            return speexvbr_items[indexPath.row]
        case NO_CODEC :
            return noaudio_items[indexPath.row]
        default :
            return UITableViewCell()
        }
    }
}
