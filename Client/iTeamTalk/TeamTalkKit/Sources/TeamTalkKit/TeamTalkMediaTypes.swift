import Foundation
import TeamTalkC

/// Which audio codec a channel/stream uses.
public struct TeamTalkCodec: RawRepresentable, Hashable, Sendable {
    public let rawValue: Codec

    public init(rawValue: Codec) {
        self.rawValue = rawValue
    }

    public init(cValue: Codec) {
        self.init(rawValue: cValue)
    }

    public var cValue: Codec {
        rawValue
    }

    public static let none = TeamTalkCodec(rawValue: NO_CODEC)
    public static let speex = TeamTalkCodec(rawValue: SPEEX_CODEC)
    public static let speexVBR = TeamTalkCodec(rawValue: SPEEX_VBR_CODEC)
    public static let opus = TeamTalkCodec(rawValue: OPUS_CODEC)
    public static let webMVP8 = TeamTalkCodec(rawValue: WEBM_VP8_CODEC)
}

/// Which sound-input preprocessing backend is active — the discriminant
/// behind ``TeamTalkAudioPreprocessorConfiguration``.
public struct TeamTalkAudioPreprocessorType: RawRepresentable, Hashable, Sendable {
    public let rawValue: AudioPreprocessorType

    public init(rawValue: AudioPreprocessorType) {
        self.rawValue = rawValue
    }

    public init(cValue: AudioPreprocessorType) {
        self.init(rawValue: cValue)
    }

    public var cValue: AudioPreprocessorType {
        rawValue
    }

    public static let none = TeamTalkAudioPreprocessorType(rawValue: NO_AUDIOPREPROCESSOR)
    public static let speexDSP = TeamTalkAudioPreprocessorType(rawValue: SPEEXDSP_AUDIOPREPROCESSOR)
    public static let teamTalk = TeamTalkAudioPreprocessorType(rawValue: TEAMTALK_AUDIOPREPROCESSOR)
    public static let obsoleteWebRTC = TeamTalkAudioPreprocessorType(rawValue: WEBRTC_AUDIOPREPROCESSOR_OBSOLETE_R4332)
    public static let webRTC = TeamTalkAudioPreprocessorType(rawValue: WEBRTC_AUDIOPREPROCESSOR)
}

/// How aggressively the WebRTC preprocessor suppresses background noise.
public struct TeamTalkWebRTCNoiseSuppressionLevel: RawRepresentable, Hashable, Sendable {
    public let rawValue: Int32

    public init(rawValue: Int32) {
        self.rawValue = rawValue
    }

    public init(_ rawValue: Int32) {
        self.init(rawValue: rawValue)
    }

    public var cValue: Int32 {
        rawValue
    }

    public static let low = TeamTalkWebRTCNoiseSuppressionLevel(rawValue: 0)
    public static let moderate = TeamTalkWebRTCNoiseSuppressionLevel(rawValue: 1)
    public static let high = TeamTalkWebRTCNoiseSuppressionLevel(rawValue: 2)
    public static let veryHigh = TeamTalkWebRTCNoiseSuppressionLevel(rawValue: 3)
}

/// One entry in ``TeamTalkChannel/transmitUsers``: who may transmit which
/// stream types right now. In a classroom channel, an entry can instead
/// mean "anyone may transmit" — see ``isClassroomFreeForAll``/``classroomFreeForAll(streamTypes:)``
/// — rather than naming a specific user.
public struct TeamTalkChannelTransmitUser: Hashable, Sendable {
    public var userID: Int32
    public var streamTypes: TeamTalkStreamTypes

    public init(userID: Int32, streamTypes: TeamTalkStreamTypes) {
        self.userID = userID
        self.streamTypes = streamTypes
    }

    public init(userID: TeamTalkUserID, streamTypes: TeamTalkStreamTypes) {
        self.init(userID: userID.cValue, streamTypes: streamTypes)
    }

    public var userIdentifier: TeamTalkUserID? {
        guard userID > 0, !isClassroomFreeForAll else {
            return nil
        }
        return TeamTalkUserID(userID)
    }

    public var isClassroomFreeForAll: Bool {
        userID == TT_CLASSROOM_FREEFORALL
    }

    public static func classroomFreeForAll(streamTypes: TeamTalkStreamTypes) -> TeamTalkChannelTransmitUser {
        TeamTalkChannelTransmitUser(userID: TT_CLASSROOM_FREEFORALL, streamTypes: streamTypes)
    }
}

/// Who a ``TeamTalkTextMessage``/``TeamTalkOutgoingTextMessage`` is addressed to.
public struct TeamTalkTextMessageType: RawRepresentable, Hashable, Sendable {
    public let rawValue: TextMsgType

    public init(rawValue: TextMsgType) {
        self.rawValue = rawValue
    }

    public init(cValue: TextMsgType) {
        self.init(rawValue: cValue)
    }

    public var cValue: TextMsgType {
        rawValue
    }

    public static let none = TeamTalkTextMessageType(rawValue: MSGTYPE_NONE)
    public static let user = TeamTalkTextMessageType(rawValue: MSGTYPE_USER)
    public static let channel = TeamTalkTextMessageType(rawValue: MSGTYPE_CHANNEL)
    public static let broadcast = TeamTalkTextMessageType(rawValue: MSGTYPE_BROADCAST)
    public static let custom = TeamTalkTextMessageType(rawValue: MSGTYPE_CUSTOM)
}

/// A ``TeamTalkFileTransfer``'s current state.
public struct TeamTalkFileTransferStatus: RawRepresentable, Hashable, Sendable {
    public let rawValue: FileTransferStatus

    public init(rawValue: FileTransferStatus) {
        self.rawValue = rawValue
    }

    public init(cValue: FileTransferStatus) {
        self.init(rawValue: cValue)
    }

    public var cValue: FileTransferStatus {
        rawValue
    }

    public static let closed = TeamTalkFileTransferStatus(rawValue: FILETRANSFER_CLOSED)
    public static let error = TeamTalkFileTransferStatus(rawValue: FILETRANSFER_ERROR)
    public static let active = TeamTalkFileTransferStatus(rawValue: FILETRANSFER_ACTIVE)
    public static let finished = TeamTalkFileTransferStatus(rawValue: FILETRANSFER_FINISHED)
}

/// Pixel encoding to convert a ``TeamTalkDesktopWindow`` frame to, e.g. via
/// `TeamTalkSession.sendDesktopWindow(_:convertTo:)`/`acquireDesktopWindow(for:convertTo:)`.
public struct TeamTalkBitmapFormat: RawRepresentable, Hashable, Sendable {
    public let rawValue: BitmapFormat

    public init(rawValue: BitmapFormat) {
        self.rawValue = rawValue
    }

    public init(cValue: BitmapFormat) {
        self.init(rawValue: cValue)
    }

    public var cValue: BitmapFormat {
        rawValue
    }

    public static let none = TeamTalkBitmapFormat(rawValue: BMP_NONE)
    public static let rgb8Palette = TeamTalkBitmapFormat(rawValue: BMP_RGB8_PALETTE)
    public static let rgb16_555 = TeamTalkBitmapFormat(rawValue: BMP_RGB16_555)
    public static let rgb24 = TeamTalkBitmapFormat(rawValue: BMP_RGB24)
    public static let rgb32 = TeamTalkBitmapFormat(rawValue: BMP_RGB32)
}

/// How a ``TeamTalkDesktopWindow``'s frame buffer is compressed.
public struct TeamTalkDesktopProtocol: RawRepresentable, Hashable, Sendable {
    public let rawValue: DesktopProtocol

    public init(rawValue: DesktopProtocol) {
        self.rawValue = rawValue
    }

    public init(cValue: DesktopProtocol) {
        self.init(rawValue: cValue)
    }

    public var cValue: DesktopProtocol {
        rawValue
    }

    public static let zlib1 = TeamTalkDesktopProtocol(rawValue: DESKTOPPROTOCOL_ZLIB_1)
    public static let zlib = TeamTalkDesktopProtocol.zlib1
}

/// Whether a forwarded ``TeamTalkDesktopInput`` key/button is being pressed
/// or released.
public struct TeamTalkDesktopKeyState: OptionSet, Hashable, Sendable {
    public let rawValue: UInt32

    public init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    public init(cValue: DesktopKeyStates) {
        self.init(rawValue: cValue)
    }

    public var cValue: DesktopKeyStates {
        rawValue
    }

    public static let none = TeamTalkDesktopKeyState(rawValue: DESKTOPKEYSTATE_NONE.rawValue)
    public static let down = TeamTalkDesktopKeyState(rawValue: DESKTOPKEYSTATE_DOWN.rawValue)
    public static let up = TeamTalkDesktopKeyState(rawValue: DESKTOPKEYSTATE_UP.rawValue)
}

/// A key or mouse button forwarded in a ``TeamTalkDesktopInput``.
public struct TeamTalkDesktopKeyCode: RawRepresentable, Hashable, Sendable, CustomStringConvertible {
    public let rawValue: UInt32

    public init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    public init(_ rawValue: UInt32) {
        self.init(rawValue: rawValue)
    }

    public init(cValue: UInt32) {
        self.init(rawValue: cValue)
    }

    public var cValue: UInt32 {
        rawValue
    }

    public var isIgnored: Bool {
        rawValue == TT_DESKTOPINPUT_KEYCODE_IGNORE
    }

    public var isMouseButton: Bool {
        self == .leftMouseButton || self == .rightMouseButton || self == .middleMouseButton
    }

    public var description: String {
        String(rawValue)
    }

    public static let ignore = TeamTalkDesktopKeyCode(UInt32(TT_DESKTOPINPUT_KEYCODE_IGNORE))
    public static let leftMouseButton = TeamTalkDesktopKeyCode(UInt32(TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN))
    public static let rightMouseButton = TeamTalkDesktopKeyCode(UInt32(TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN))
    public static let middleMouseButton = TeamTalkDesktopKeyCode(UInt32(TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN))
}

/// A ``TeamTalkMediaFileInfo``'s current playback/probe state.
public struct TeamTalkMediaFileStatus: RawRepresentable, Hashable, Sendable {
    public let rawValue: MediaFileStatus

    public init(rawValue: MediaFileStatus) {
        self.rawValue = rawValue
    }

    public init(cValue: MediaFileStatus) {
        self.init(rawValue: cValue)
    }

    public var cValue: MediaFileStatus {
        rawValue
    }

    public static let closed = TeamTalkMediaFileStatus(rawValue: MFS_CLOSED)
    public static let error = TeamTalkMediaFileStatus(rawValue: MFS_ERROR)
    public static let started = TeamTalkMediaFileStatus(rawValue: MFS_STARTED)
    public static let finished = TeamTalkMediaFileStatus(rawValue: MFS_FINISHED)
    public static let aborted = TeamTalkMediaFileStatus(rawValue: MFS_ABORTED)
    public static let paused = TeamTalkMediaFileStatus(rawValue: MFS_PAUSED)
    public static let playing = TeamTalkMediaFileStatus(rawValue: MFS_PLAYING)
}

/// Output format for recorded audio files (`startRecordingMuxedAudioFile`,
/// `setUserMediaStorage`, ...).
public struct TeamTalkAudioFileFormat: RawRepresentable, Hashable, Sendable {
    public let rawValue: AudioFileFormat

    public init(rawValue: AudioFileFormat) {
        self.rawValue = rawValue
    }

    public init(cValue: AudioFileFormat) {
        self.init(rawValue: cValue)
    }

    public var cValue: AudioFileFormat {
        rawValue
    }

    public static let none = TeamTalkAudioFileFormat(rawValue: AFF_NONE)
    /// Records raw encoded audio in the channel's own codec, rather than
    /// transcoding to a fixed output format.
    public static let channelCodec = TeamTalkAudioFileFormat(rawValue: AFF_CHANNELCODEC_FORMAT)
    public static let wave = TeamTalkAudioFileFormat(rawValue: AFF_WAVE_FORMAT)
    public static let mp3_16kbit = TeamTalkAudioFileFormat(rawValue: AFF_MP3_16KBIT_FORMAT)
    public static let mp3_32kbit = TeamTalkAudioFileFormat(rawValue: AFF_MP3_32KBIT_FORMAT)
    public static let mp3_64kbit = TeamTalkAudioFileFormat(rawValue: AFF_MP3_64KBIT_FORMAT)
    public static let mp3_128kbit = TeamTalkAudioFileFormat(rawValue: AFF_MP3_128KBIT_FORMAT)
    public static let mp3_256kbit = TeamTalkAudioFileFormat(rawValue: AFF_MP3_256KBIT_FORMAT)
    public static let mp3_320kbit = TeamTalkAudioFileFormat(rawValue: AFF_MP3_320KBIT_FORMAT)
}

/// A video frame's pixel layout (as a FourCC code).
public struct TeamTalkVideoPixelFormat: RawRepresentable, Hashable, Sendable {
    public let rawValue: FourCC

    public init(rawValue: FourCC) {
        self.rawValue = rawValue
    }

    public init(cValue: FourCC) {
        self.init(rawValue: cValue)
    }

    public var cValue: FourCC {
        rawValue
    }

    public static let none = TeamTalkVideoPixelFormat(rawValue: FOURCC_NONE)
    public static let i420 = TeamTalkVideoPixelFormat(rawValue: FOURCC_I420)
    public static let yuy2 = TeamTalkVideoPixelFormat(rawValue: FOURCC_YUY2)
    public static let rgb32 = TeamTalkVideoPixelFormat(rawValue: FOURCC_RGB32)
}

/// The native SDK's own event tag, 1:1 with each ``TeamTalkEvent/Kind``
/// case. Only surfaced through ``TeamTalkEvent/Kind/unhandled(event:payloadType:)``
/// for events without a dedicated case; everything else decodes straight to
/// `Kind` and callers shouldn't need to compare against this directly.
public struct TeamTalkClientEvent: RawRepresentable, Hashable, Sendable {
    public let rawValue: ClientEvent

    public init(rawValue: ClientEvent) {
        self.rawValue = rawValue
    }

    public init(cValue: ClientEvent) {
        self.init(rawValue: cValue)
    }

    public var cValue: ClientEvent {
        rawValue
    }

    public static let none = TeamTalkClientEvent(rawValue: CLIENTEVENT_NONE)
    public static let connectionSucceeded = TeamTalkClientEvent(rawValue: CLIENTEVENT_CON_SUCCESS)
    public static let connectionEncryptionError = TeamTalkClientEvent(rawValue: CLIENTEVENT_CON_CRYPT_ERROR)
    public static let connectionFailed = TeamTalkClientEvent(rawValue: CLIENTEVENT_CON_FAILED)
    public static let connectionLost = TeamTalkClientEvent(rawValue: CLIENTEVENT_CON_LOST)
    public static let connectionMaxPayloadUpdated = TeamTalkClientEvent(rawValue: CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED)
    public static let commandProcessing = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_PROCESSING)
    public static let commandError = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_ERROR)
    public static let commandSucceeded = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_SUCCESS)
    public static let myselfLoggedIn = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_MYSELF_LOGGEDIN)
    public static let myselfLoggedOut = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_MYSELF_LOGGEDOUT)
    public static let myselfKicked = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_MYSELF_KICKED)
    public static let userLoggedIn = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_USER_LOGGEDIN)
    public static let userLoggedOut = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_USER_LOGGEDOUT)
    public static let userUpdated = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_USER_UPDATE)
    public static let userJoined = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_USER_JOINED)
    public static let userLeft = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_USER_LEFT)
    public static let userTextMessage = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_USER_TEXTMSG)
    public static let channelCreated = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_CHANNEL_NEW)
    public static let channelUpdated = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_CHANNEL_UPDATE)
    public static let channelRemoved = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_CHANNEL_REMOVE)
    public static let serverUpdated = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_SERVER_UPDATE)
    public static let serverStatistics = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_SERVERSTATISTICS)
    public static let fileCreated = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_FILE_NEW)
    public static let fileRemoved = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_FILE_REMOVE)
    public static let userAccount = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_USERACCOUNT)
    public static let bannedUser = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_BANNEDUSER)
    public static let userAccountCreated = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_USERACCOUNT_NEW)
    public static let userAccountRemoved = TeamTalkClientEvent(rawValue: CLIENTEVENT_CMD_USERACCOUNT_REMOVE)
    public static let userStateChanged = TeamTalkClientEvent(rawValue: CLIENTEVENT_USER_STATECHANGE)
    public static let userVideoCapture = TeamTalkClientEvent(rawValue: CLIENTEVENT_USER_VIDEOCAPTURE)
    public static let userMediaFileVideo = TeamTalkClientEvent(rawValue: CLIENTEVENT_USER_MEDIAFILE_VIDEO)
    public static let userDesktopWindow = TeamTalkClientEvent(rawValue: CLIENTEVENT_USER_DESKTOPWINDOW)
    public static let userDesktopCursor = TeamTalkClientEvent(rawValue: CLIENTEVENT_USER_DESKTOPCURSOR)
    public static let userDesktopInput = TeamTalkClientEvent(rawValue: CLIENTEVENT_USER_DESKTOPINPUT)
    public static let userRecordMediaFile = TeamTalkClientEvent(rawValue: CLIENTEVENT_USER_RECORD_MEDIAFILE)
    public static let userAudioBlock = TeamTalkClientEvent(rawValue: CLIENTEVENT_USER_AUDIOBLOCK)
    public static let internalError = TeamTalkClientEvent(rawValue: CLIENTEVENT_INTERNAL_ERROR)
    public static let voiceActivation = TeamTalkClientEvent(rawValue: CLIENTEVENT_VOICE_ACTIVATION)
    public static let hotkey = TeamTalkClientEvent(rawValue: CLIENTEVENT_HOTKEY)
    public static let hotkeyTest = TeamTalkClientEvent(rawValue: CLIENTEVENT_HOTKEY_TEST)
    public static let fileTransfer = TeamTalkClientEvent(rawValue: CLIENTEVENT_FILETRANSFER)
    public static let desktopWindowTransfer = TeamTalkClientEvent(rawValue: CLIENTEVENT_DESKTOPWINDOW_TRANSFER)
    public static let streamMediaFile = TeamTalkClientEvent(rawValue: CLIENTEVENT_STREAM_MEDIAFILE)
    public static let localMediaFile = TeamTalkClientEvent(rawValue: CLIENTEVENT_LOCAL_MEDIAFILE)
    public static let audioInput = TeamTalkClientEvent(rawValue: CLIENTEVENT_AUDIOINPUT)
    public static let userFirstVoiceStreamPacket = TeamTalkClientEvent(rawValue: CLIENTEVENT_USER_FIRSTVOICESTREAMPACKET)
    public static let soundDeviceAdded = TeamTalkClientEvent(rawValue: CLIENTEVENT_SOUNDDEVICE_ADDED)
    public static let soundDeviceRemoved = TeamTalkClientEvent(rawValue: CLIENTEVENT_SOUNDDEVICE_REMOVED)
    public static let soundDeviceUnplugged = TeamTalkClientEvent(rawValue: CLIENTEVENT_SOUNDDEVICE_UNPLUGGED)
    public static let soundDeviceNewDefaultInput = TeamTalkClientEvent(rawValue: CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_INPUT)
    public static let soundDeviceNewDefaultOutput = TeamTalkClientEvent(rawValue: CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_OUTPUT)
    public static let soundDeviceNewDefaultInputCommunicationDevice = TeamTalkClientEvent(rawValue: CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_INPUT_COMDEVICE)
    public static let soundDeviceNewDefaultOutputCommunicationDevice = TeamTalkClientEvent(rawValue: CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_OUTPUT_COMDEVICE)
}

/// Which C struct type a raw `TTMessage`'s payload actually is. Mainly
/// useful alongside ``TeamTalkEvent/Kind/unhandled(event:payloadType:)`` to
/// know how to interpret ``TeamTalkEvent/rawMessage`` yourself.
public struct TeamTalkPayloadType: RawRepresentable, Hashable, Sendable {
    public let rawValue: TTType

    public init(rawValue: TTType) {
        self.rawValue = rawValue
    }

    public init(cValue: TTType) {
        self.init(rawValue: cValue)
    }

    public var cValue: TTType {
        rawValue
    }

    public static let none = TeamTalkPayloadType(rawValue: __NONE)
    public static let audioCodec = TeamTalkPayloadType(rawValue: __AUDIOCODEC)
    public static let bannedUser = TeamTalkPayloadType(rawValue: __BANNEDUSER)
    public static let videoFormat = TeamTalkPayloadType(rawValue: __VIDEOFORMAT)
    public static let opusCodec = TeamTalkPayloadType(rawValue: __OPUSCODEC)
    public static let channel = TeamTalkPayloadType(rawValue: __CHANNEL)
    public static let clientStatistics = TeamTalkPayloadType(rawValue: __CLIENTSTATISTICS)
    public static let clientError = TeamTalkPayloadType(rawValue: __CLIENTERRORMSG)
    public static let fileTransfer = TeamTalkPayloadType(rawValue: __FILETRANSFER)
    public static let mediaFileStatus = TeamTalkPayloadType(rawValue: __MEDIAFILESTATUS)
    public static let remoteFile = TeamTalkPayloadType(rawValue: __REMOTEFILE)
    public static let serverProperties = TeamTalkPayloadType(rawValue: __SERVERPROPERTIES)
    public static let serverStatistics = TeamTalkPayloadType(rawValue: __SERVERSTATISTICS)
    public static let soundDevice = TeamTalkPayloadType(rawValue: __SOUNDDEVICE)
    public static let speexCodec = TeamTalkPayloadType(rawValue: __SPEEXCODEC)
    public static let textMessage = TeamTalkPayloadType(rawValue: __TEXTMESSAGE)
    public static let webMVP8Codec = TeamTalkPayloadType(rawValue: __WEBMVP8CODEC)
    public static let teamTalkMessage = TeamTalkPayloadType(rawValue: __TTMESSAGE)
    public static let user = TeamTalkPayloadType(rawValue: __USER)
    public static let userAccount = TeamTalkPayloadType(rawValue: __USERACCOUNT)
    public static let userStatistics = TeamTalkPayloadType(rawValue: __USERSTATISTICS)
    public static let videoCaptureDevice = TeamTalkPayloadType(rawValue: __VIDEOCAPTUREDEVICE)
    public static let videoCodec = TeamTalkPayloadType(rawValue: __VIDEOCODEC)
    public static let audioConfig = TeamTalkPayloadType(rawValue: __AUDIOCONFIG)
    public static let speexVBRCodec = TeamTalkPayloadType(rawValue: __SPEEXVBRCODEC)
    public static let videoFrame = TeamTalkPayloadType(rawValue: __VIDEOFRAME)
    public static let audioBlock = TeamTalkPayloadType(rawValue: __AUDIOBLOCK)
    public static let audioFormat = TeamTalkPayloadType(rawValue: __AUDIOFORMAT)
    public static let mediaFileInfo = TeamTalkPayloadType(rawValue: __MEDIAFILEINFO)
    public static let boolean = TeamTalkPayloadType(rawValue: __TTBOOL)
    public static let integer = TeamTalkPayloadType(rawValue: __INT32)
    public static let desktopInput = TeamTalkPayloadType(rawValue: __DESKTOPINPUT)
    public static let speexDSP = TeamTalkPayloadType(rawValue: __SPEEXDSP)
    public static let streamType = TeamTalkPayloadType(rawValue: __STREAMTYPE)
    public static let audioPreprocessorType = TeamTalkPayloadType(rawValue: __AUDIOPREPROCESSORTYPE)
    public static let audioPreprocessor = TeamTalkPayloadType(rawValue: __AUDIOPREPROCESSOR)
    public static let teamTalkAudioPreprocessor = TeamTalkPayloadType(rawValue: __TTAUDIOPREPROCESSOR)
    public static let mediaFilePlayback = TeamTalkPayloadType(rawValue: __MEDIAFILEPLAYBACK)
    public static let clientKeepAlive = TeamTalkPayloadType(rawValue: __CLIENTKEEPALIVE)
    public static let unsignedInteger = TeamTalkPayloadType(rawValue: __UINT32)
    public static let audioInputProgress = TeamTalkPayloadType(rawValue: __AUDIOINPUTPROGRESS)
    public static let jitterConfig = TeamTalkPayloadType(rawValue: __JITTERCONFIG)
    public static let webRTCAudioPreprocessor = TeamTalkPayloadType(rawValue: __WEBRTCAUDIOPREPROCESSOR)
    public static let encryptionContext = TeamTalkPayloadType(rawValue: __ENCRYPTIONCONTEXT)
    public static let soundDeviceEffects = TeamTalkPayloadType(rawValue: __SOUNDDEVICEEFFECTS)
    public static let desktopWindow = TeamTalkPayloadType(rawValue: __DESKTOPWINDOW)
    public static let abusePrevention = TeamTalkPayloadType(rawValue: __ABUSEPREVENTION)
}

