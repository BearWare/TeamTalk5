//
//  UtilTT.swift
//  iTeamTalk
//
//  Created by Bjørn Damstedt Rasmussen on 10/01/2021.
//  Copyright © 2021 BearWare.dk. All rights reserved.
//

import Foundation
import TeamTalkKit

protocol TeamTalkEvent : AnyObject {
    func handleTTMessage(_ m: TTMessage)
}

class TeamTalkEventHandler: TeamTalkMessageObserver {
    weak var value : TeamTalkEvent?
    init (value: TeamTalkEvent) {
        self.value = value
    }

    func handleTeamTalkMessage(_ message: TTMessage) {
        value?.handleTTMessage(message)
    }
}

var ttMessageHandlers = [TeamTalkEventHandler]()

func addToTTMessages(_ p: TeamTalkEvent) {
    if ttMessageHandlers.contains(where: { $0.value === p }) {
        return
    }

    let handler = TeamTalkEventHandler(value: p)
    ttMessageHandlers.append(handler)
    TeamTalkClient.shared.addObserver(handler)
}

func removeFromTTMessages(_ p: TeamTalkEventHandler) {
    ttMessageHandlers.removeAll { $0 === p || $0.value == nil }
    TeamTalkClient.shared.removeObserver(p)
}

func removeAllTTMessageHandlers() {
    ttMessageHandlers.removeAll()
    TeamTalkClient.shared.removeAllObservers()
}

func runTeamTalkEventHandler() {
    TeamTalkClient.shared.pollMessages()
}

func setupEncryption(server: Server) -> Bool {
    if server.encrypted == false {
        return true
    }
    
    do {
        let configuration = TeamTalkEncryptionConfiguration(
            caCertificate: server.cacertdata,
            certificate: server.certdata,
            privateKey: server.certprivkeydata,
            verifyPeer: server.certverifypeer
        )
        let result = try TeamTalkClient.shared.configureEncryption(configuration)
        if result {
           print("Encryption activated")
        }
        else {
            print("Failed to set encryption")
        }
        return result
    } catch {
        print("Exception thrown trying to create directory")
        return false
    }
}

let TRUE : TTBOOL = 1
let FALSE : TTBOOL = 0

let DEFAULT_AUDIOCODEC = OPUS_CODEC

let DEFAULT_MSEC_PER_PACKET = INT32(40)

//Default OPUS codec settings
let DEFAULT_OPUS_SAMPLERATE = INT32(48000)
let DEFAULT_OPUS_CHANNELS = INT32(1)
let DEFAULT_OPUS_APPLICATION = INT32(OPUS_APPLICATION_VOIP)
let DEFAULT_OPUS_COMPLEXITY = INT32(10)
let DEFAULT_OPUS_FEC = TRUE
let DEFAULT_OPUS_DTX = FALSE
let DEFAULT_OPUS_VBR = TRUE
let DEFAULT_OPUS_VBRCONSTRAINT = FALSE
let DEFAULT_OPUS_BITRATE = INT32(32000)
let DEFAULT_OPUS_DELAY = INT32(DEFAULT_MSEC_PER_PACKET)
let DEFAULT_OPUS_FRAMESIZE = INT32(0) // implies same as DEFAULT_OPUS_DELAY

//Default Speex codec settings
let DEFAULT_SPEEX_BANDMODE = INT32(1)
let DEFAULT_SPEEX_QUALITY = INT32(4)
let DEFAULT_SPEEX_DELAY = INT32(DEFAULT_MSEC_PER_PACKET)
let DEFAULT_SPEEX_SIMSTEREO = FALSE

//Default Speex VBR codec settings
let DEFAULT_SPEEX_VBR_BANDMODE : INT32 = 1
let DEFAULT_SPEEX_VBR_QUALITY : INT32 = 4
let DEFAULT_SPEEX_VBR_BITRATE : INT32 = 0
let DEFAULT_SPEEX_VBR_MAXBITRATE : INT32 = 0
let DEFAULT_SPEEX_VBR_DTX = TRUE
let DEFAULT_SPEEX_VBR_DELAY : INT32 = DEFAULT_MSEC_PER_PACKET
let DEFAULT_SPEEX_VBR_SIMSTEREO = FALSE

func newAudioCodec(_ codec: Codec) -> AudioCodec {
    TeamTalkAudioCodec.makeAudioCodec(codec)
}

func newOpusCodec() -> OpusCodec {
    TeamTalkAudioCodec.makeOpusCodec()
}

func newSpeexCodec() -> SpeexCodec {
    TeamTalkAudioCodec.makeSpeexCodec()
}

func newSpeexVBRCodec() -> SpeexVBRCodec {
    TeamTalkAudioCodec.makeSpeexVBRCodec()
}
