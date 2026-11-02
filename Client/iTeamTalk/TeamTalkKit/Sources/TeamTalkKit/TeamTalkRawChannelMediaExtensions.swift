import Foundation
import TeamTalkC


// Each extension below bridges one raw C struct's fixed-size buffers and
// int/bit fields to Swift-friendly accessors. The corresponding `TeamTalkX`
// wrapper type (documented in the model files) wraps this extension rather
// than duplicating it, so see those for what each field means.
public extension Channel {
    var id: Int32 {
        nChannelID
    }

    var channelID: TeamTalkChannelID {
        TeamTalkChannelID(id)
    }

    var parentID: Int32 {
        nParentID
    }

    var parentChannelID: TeamTalkChannelID {
        TeamTalkChannelID(parentID)
    }

    var name: String {
        TeamTalkString.channel(.name, from: self)
    }

    var topic: String {
        TeamTalkString.channel(.topic, from: self)
    }

    var password: String {
        TeamTalkString.channel(.password, from: self)
    }

    var operatorPassword: String {
        TeamTalkString.channel(.operatorPassword, from: self)
    }

    var transmitUserList: [TeamTalkChannelTransmitUser] {
        get {
            withUnsafePointer(to: self) { channel in
                var entries: [TeamTalkChannelTransmitUser] = []
                entries.reserveCapacity(Int(TT_TRANSMITUSERS_MAX))

                for index in 0..<Int(TT_TRANSMITUSERS_MAX) {
                    let userID = TTKitGetChannelTransmitUserID(channel, Int32(index))
                    if userID == 0 {
                        break
                    }

                    let streamTypes = TeamTalkStreamTypes(cValue: TTKitGetChannelTransmitUserStreamTypes(channel, Int32(index)))
                    entries.append(TeamTalkChannelTransmitUser(userID: userID, streamTypes: streamTypes))
                }
                return entries
            }
        }
        set {
            for index in 0..<Int(TT_TRANSMITUSERS_MAX) {
                TTKitSetChannelTransmitUser(&self, Int32(index), 0, TeamTalkStreamTypes.none.cMask)
            }

            for (index, entry) in newValue.prefix(Int(TT_TRANSMITUSERS_MAX)).enumerated() {
                TTKitSetChannelTransmitUser(&self, Int32(index), entry.userID, entry.streamTypes.cMask)
            }
        }
    }

    var transmitQueueUsers: [TeamTalkUserID] {
        get {
            withUnsafePointer(to: self) { channel in
                var userIDs: [TeamTalkUserID] = []
                userIDs.reserveCapacity(Int(TT_TRANSMITQUEUE_MAX))

                for index in 0..<Int(TT_TRANSMITQUEUE_MAX) {
                    let userID = TTKitGetChannelTransmitQueueUserID(channel, Int32(index))
                    if userID == 0 {
                        break
                    }
                    userIDs.append(TeamTalkUserID(userID))
                }
                return userIDs
            }
        }
        set {
            for index in 0..<Int(TT_TRANSMITQUEUE_MAX) {
                TTKitSetChannelTransmitQueueUser(&self, Int32(index), 0)
            }

            for (index, userID) in newValue.prefix(Int(TT_TRANSMITQUEUE_MAX)).enumerated() {
                TTKitSetChannelTransmitQueueUser(&self, Int32(index), userID.cValue)
            }
        }
    }

    var transmitQueueDelayMilliseconds: Int32 {
        get { nTransmitUsersQueueDelayMSec }
        set { nTransmitUsersQueueDelayMSec = newValue }
    }

    var voiceTimeoutMilliseconds: Int32 {
        get { nTimeOutTimerVoiceMSec }
        set { nTimeOutTimerVoiceMSec = newValue }
    }

    var mediaFileTimeoutMilliseconds: Int32 {
        get { nTimeOutTimerMediaFileMSec }
        set { nTimeOutTimerMediaFileMSec = newValue }
    }

    var types: TeamTalkChannelTypes {
        get { TeamTalkChannelTypes(cValue: uChannelType) }
        set { uChannelType = newValue.cValue }
    }

    var isPasswordProtected: Bool {
        bPassword != 0
    }

    var audioCodecType: TeamTalkCodec {
        get { TeamTalkCodec(cValue: audiocodec.nCodec) }
        set { audiocodec.nCodec = newValue.cValue }
    }

    var isRoot: Bool {
        nParentID == 0
    }
}

public extension RemoteFile {
    var channelID: Int32 {
        nChannelID
    }

    var channelIdentifier: TeamTalkChannelID {
        TeamTalkChannelID(channelID)
    }

    var id: Int32 {
        nFileID
    }

    var fileID: TeamTalkFileID {
        TeamTalkFileID(id)
    }

    var name: String {
        TeamTalkString.remoteFile(.fileName, from: self)
    }

    var username: String {
        TeamTalkString.remoteFile(.username, from: self)
    }

    var uploadTime: String {
        TeamTalkString.remoteFile(.uploadTime, from: self)
    }

    var size: Int64 {
        nFileSize
    }
}

public extension FileTransfer {
    var status: TeamTalkFileTransferStatus {
        get { TeamTalkFileTransferStatus(cValue: nStatus) }
        set { nStatus = newValue.cValue }
    }

    var id: Int32 {
        nTransferID
    }

    var transferID: TeamTalkTransferID {
        TeamTalkTransferID(id)
    }

    var channelID: Int32 {
        nChannelID
    }

    var channelIdentifier: TeamTalkChannelID {
        TeamTalkChannelID(channelID)
    }

    var localFilePath: String {
        TeamTalkString.fileTransfer(.localFilePath, from: self)
    }

    var remoteFileName: String {
        TeamTalkString.fileTransfer(.remoteFileName, from: self)
    }

    var isDownload: Bool {
        bInbound != 0
    }

    var progress: Double {
        guard nFileSize > 0 else { return 0 }
        return min(1, max(0, Double(nTransferred) / Double(nFileSize)))
    }
}

public extension DesktopWindow {
    var width: Int32 {
        nWidth
    }

    var height: Int32 {
        nHeight
    }

    var bitmapFormat: TeamTalkBitmapFormat {
        get { TeamTalkBitmapFormat(cValue: bmpFormat) }
        set { bmpFormat = newValue.cValue }
    }

    var bytesPerLine: Int32 {
        nBytesPerLine
    }

    var sessionID: Int32 {
        nSessionID
    }

    var sessionIdentifier: TeamTalkDesktopSessionID {
        get { TeamTalkDesktopSessionID(sessionID) }
        set { nSessionID = newValue.cValue }
    }

    var desktopProtocol: TeamTalkDesktopProtocol {
        get { TeamTalkDesktopProtocol(cValue: nProtocol) }
        set { nProtocol = newValue.cValue }
    }

    var frameBufferSize: Int32 {
        nFrameBufferSize
    }

    var hasFrameBuffer: Bool {
        frameBuffer != nil && frameBufferSize > 0
    }

    var frameBufferData: Data {
        guard let frameBuffer, frameBufferSize > 0 else {
            return Data()
        }
        return Data(bytes: frameBuffer, count: Int(frameBufferSize))
    }
}

public extension DesktopInput {
    var mousePositionX: UInt16? {
        get { uMousePosX == TT_DESKTOPINPUT_MOUSEPOS_IGNORE ? nil : uMousePosX }
        set { uMousePosX = newValue ?? UInt16(TT_DESKTOPINPUT_MOUSEPOS_IGNORE) }
    }

    var mousePositionY: UInt16? {
        get { uMousePosY == TT_DESKTOPINPUT_MOUSEPOS_IGNORE ? nil : uMousePosY }
        set { uMousePosY = newValue ?? UInt16(TT_DESKTOPINPUT_MOUSEPOS_IGNORE) }
    }

    var mousePosition: (x: UInt16, y: UInt16)? {
        get {
            guard let x = mousePositionX, let y = mousePositionY else {
                return nil
            }
            return (x, y)
        }
        set {
            mousePositionX = newValue?.x
            mousePositionY = newValue?.y
        }
    }

    var keyCode: TeamTalkDesktopKeyCode {
        get { TeamTalkDesktopKeyCode(cValue: uKeyCode) }
        set { uKeyCode = newValue.cValue }
    }

    var keyState: TeamTalkDesktopKeyState {
        get { TeamTalkDesktopKeyState(cValue: uKeyState) }
        set { uKeyState = newValue.cValue }
    }

    var hasMousePosition: Bool {
        mousePosition != nil
    }

    var ignoresKeyCode: Bool {
        keyCode.isIgnored
    }
}

public extension VideoCaptureDevice {
    var deviceIdentifier: String {
        teamTalkString(from: szDeviceID)
    }

    var name: String {
        teamTalkString(from: szDeviceName)
    }

    var captureAPI: String {
        teamTalkString(from: szCaptureAPI)
    }

    var videoFormatsCount: Int32 {
        nVideoFormatsCount
    }

    var supportedFormats: [VideoFormat] {
        teamTalkArray(from: videoFormats, count: videoFormatsCount, as: VideoFormat.self)
    }

    var hasFormats: Bool {
        !supportedFormats.isEmpty
    }
}

public extension AudioBlock {
    var streamID: Int32 {
        nStreamID
    }

    var sampleRate: Int32 {
        nSampleRate
    }

    var channels: Int32 {
        nChannels
    }

    var samplesPerChannel: Int32 {
        nSamples
    }

    var totalSampleCount: Int {
        max(0, Int(samplesPerChannel * max(0, channels)))
    }

    var rawAudioByteCount: Int {
        totalSampleCount * MemoryLayout<Int16>.size
    }

    var rawAudioData: Data {
        guard let lpRawAudio, rawAudioByteCount > 0 else {
            return Data()
        }
        return Data(bytes: lpRawAudio, count: rawAudioByteCount)
    }

    var pcmSamples: [Int16] {
        teamTalkPCM16Samples(from: lpRawAudio, sampleCount: totalSampleCount)
    }

    var sampleIndex: UInt32 {
        uSampleIndex
    }

    var streamTypes: TeamTalkStreamTypes {
        get { TeamTalkStreamTypes(cValue: uStreamTypes) }
        set { uStreamTypes = newValue.cMask }
    }

    var durationMilliseconds: Double? {
        guard sampleRate > 0 else {
            return nil
        }
        return (Double(samplesPerChannel) * 1000.0) / Double(sampleRate)
    }
}

public extension AudioFormat {
    var format: TeamTalkAudioFileFormat {
        get { TeamTalkAudioFileFormat(cValue: nAudioFmt) }
        set { nAudioFmt = newValue.cValue }
    }

    var sampleRate: Int32 {
        nSampleRate
    }

    var channels: Int32 {
        nChannels
    }

    var isAvailable: Bool {
        sampleRate > 0 && channels > 0 && format != .none
    }
}

public extension VideoFormat {
    var width: Int32 {
        nWidth
    }

    var height: Int32 {
        nHeight
    }

    var frameRateNumerator: Int32 {
        nFPS_Numerator
    }

    var frameRateDenominator: Int32 {
        nFPS_Denominator
    }

    var pixelFormat: TeamTalkVideoPixelFormat {
        get { TeamTalkVideoPixelFormat(cValue: picFourCC) }
        set { picFourCC = newValue.cValue }
    }

    var framesPerSecond: Double? {
        guard frameRateDenominator > 0 else {
            return nil
        }
        return Double(frameRateNumerator) / Double(frameRateDenominator)
    }
}

public extension MediaFileInfo {
    var status: TeamTalkMediaFileStatus {
        get { TeamTalkMediaFileStatus(cValue: nStatus) }
        set { nStatus = newValue.cValue }
    }

    var fileName: String {
        teamTalkString(from: szFileName)
    }

    var durationMilliseconds: UInt32 {
        uDurationMSec
    }

    var elapsedMilliseconds: UInt32 {
        uElapsedMSec
    }

    var progress: Double {
        guard durationMilliseconds > 0 else {
            return 0
        }
        return min(1, max(0, Double(elapsedMilliseconds) / Double(durationMilliseconds)))
    }
}

public extension MediaFilePlayback {
    var offsetMilliseconds: UInt32? {
        get { uOffsetMSec == TT_MEDIAPLAYBACK_OFFSET_IGNORE ? nil : uOffsetMSec }
        set { uOffsetMSec = newValue ?? TT_MEDIAPLAYBACK_OFFSET_IGNORE }
    }

    var isPaused: Bool {
        get { bPaused != 0 }
        set { bPaused = newValue ? 1 : 0 }
    }

    var preprocessor: AudioPreprocessor {
        get { audioPreprocessor }
        set { audioPreprocessor = newValue }
    }

    var preprocessorType: TeamTalkAudioPreprocessorType {
        preprocessor.type
    }
}

public extension TextMessage {
    var type: TeamTalkTextMessageType {
        get { TeamTalkTextMessageType(cValue: nMsgType) }
        set { nMsgType = newValue.cValue }
    }

    var content: String {
        TeamTalkString.textMessage(self)
    }
}

public extension TTMessage {
    var source: Int32 {
        nSource
    }

    var payloadSize: Int32 {
        nPayloadSize
    }

    var event: TeamTalkClientEvent {
        get { TeamTalkClientEvent(cValue: nClientEvent) }
        set { nClientEvent = newValue.cValue }
    }

    var payloadType: TeamTalkPayloadType {
        get { TeamTalkPayloadType(cValue: ttType) }
        set { ttType = newValue.cValue }
    }

    var streamType: TeamTalkStreamTypes {
        get { TeamTalkStreamTypes(cValue: nStreamType) }
        set { nStreamType = newValue.cValue }
    }

    var isActive: Bool {
        TeamTalkMessagePayload.isActive(self)
    }
}
