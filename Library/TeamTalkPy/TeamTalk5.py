import sys
import os
import ctypes
from time import sleep
from enum import IntEnum
from ctypes import *
from ctypes.util import find_library

if sys.platform == "win32":
    if (sys.version_info.major == 3 and sys.version_info.minor >= 8):
        os.add_dll_directory(os.getcwd())
    dll = cdll.TeamTalk5
    TTCHAR = c_wchar
    TTCHAR_P = c_wchar_p
    from ctypes.wintypes import BOOL
elif sys.platform == "darwin":
    # Darwin is not supported. Seems SIP is preventing this from
    # working. Setting DYLD_LIBRARY_PATH doesn't help.
    dll = cdll.LoadLibrary("libTeamTalk5.dylib")
    TTCHAR = c_char
    TTCHAR_P = c_char_p
    BOOL = c_int
else:
    dll = cdll.LoadLibrary("libTeamTalk5.so")
    TTCHAR = c_char
    TTCHAR_P = c_char_p
    BOOL = c_int
INT32 = c_int
INT64 = c_longlong
UINT32 = c_uint
TT_STRLEN = 512
TT_VIDEOFORMATS_MAX = 1024
TT_TRANSMITUSERS_MAX = 128
TT_CHANNELS_OPERATOR_MAX = 16
TT_TRANSMITQUEUE_MAX = 16
TT_SAMPLERATES_MAX = 16
_TTInstance = c_void_p
_TTSoundLoop = c_void_p

# make functionfactory accepting functionname, restype, argtypes

# bindings
class SoundSystem(INT32):
    SOUNDSYSTEM_NONE = 0
    SOUNDSYSTEM_WINMM = 1
    SOUNDSYSTEM_DSOUND = 2
    SOUNDSYSTEM_ALSA = 3
    SOUNDSYSTEM_COREAUDIO = 4
    SOUNDSYSTEM_WASAPI = 5
    SOUNDSYSTEM_OPENSLES_ANDROID = 7
    SOUNDSYSTEM_AUDIOUNIT = 8

class SoundDeviceFeature(UINT32):
    SOUNDDEVICEFEATURE_NONE = 0x0000
    SOUNDDEVICEFEATURE_AEC = 0x0001
    SOUNDDEVICEFEATURE_AGC = 0x0002
    SOUNDDEVICEFEATURE_DENOISE = 0x0004
    SOUNDDEVICEFEATURE_3DPOSITION = 0x0008
    SOUNDDEVICEFEATURE_DUPLEXMODE = 0x0010

class SoundDevice(Structure):
    _fields_ = [
    ("nDeviceID", INT32),
    ("nSoundSystem", INT32),
    ("szDeviceName", TTCHAR*TT_STRLEN),
    ("szDeviceID", TTCHAR*TT_STRLEN),
    ("nWaveDeviceID", INT32),
    ("bSupports3D", BOOL),
    ("nMaxInputChannels", INT32),
    ("nMaxOutputChannels", INT32),
    ("inputSampleRates", INT32 * TT_SAMPLERATES_MAX),
    ("outputSampleRates", INT32 * TT_SAMPLERATES_MAX),
    ("nDefaultSampleRate", INT32),
    ("uSoundDeviceFeatures", UINT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.SOUNDDEVICE) == ctypes.sizeof(SoundDevice))

class SoundDeviceEffects(Structure):
    _fields_ = [
    ("bEnableAGC", BOOL),
    ("bEnableDenoise", BOOL),
    ("bEnableEchoCancellation", BOOL)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.SoundDeviceEffects) == ctypes.sizeof(SoundDeviceEffects))

class SoundLevel(INT32):
    SOUND_VU_MAX = 100
    SOUND_VU_MIN = 0
    SOUND_VOLUME_MAX = 32000
    SOUND_VOLUME_DEFAULT = 1000
    SOUND_VOLUME_MIN = 0
    SOUND_GAIN_MAX = 32000
    SOUND_GAIN_DEFAULT = 1000
    SOUND_GAIN_MIN = 0

class AudioBlock(Structure):
    _fields_ = [
    ("nStreamID", INT32),
    ("nSampleRate", INT32),
    ("nChannels", INT32),
    ("lpRawAudio", c_void_p),
    ("nSamples", INT32),
    ("uSampleIndex", UINT32),
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.AUDIOBLOCK) == ctypes.sizeof(AudioBlock))

class MediaFileStatus(INT32):
    MFS_CLOSED = 0
    MFS_ERROR = 1
    MFS_STARTED = 2
    MFS_FINISHED = 3
    MFS_ABORTED = 4
    MFS_PAUSED = 5
    MFS_PLAYING = 6

class AudioFileFormat(INT32):
    AFF_NONE = 0
    AFF_CHANNELCODEC_FORMAT = 1
    AFF_WAVE_FORMAT = 2
    AFF_MP3_16KBIT_FORMAT = 3
    AFF_MP3_32KBIT_FORMAT = 4
    AFF_MP3_64KBIT_FORMAT = 5
    AFF_MP3_128KBIT_FORMAT = 6
    AFF_MP3_256KBIT_FORMAT = 7

class AudioFormat(Structure):
    _fields_ = [
    ("nAudioFmt", INT32),
    ("nSampleRate", INT32),
    ("nChannels", INT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.AUDIOFORMAT) == ctypes.sizeof(AudioFormat))

class FourCC(INT32):
    FOURCC_NONE =   0
    FOURCC_I420 = 100
    FOURCC_YUY2 = 101
    FOURCC_RGB32 = 102

class VideoFormat(Structure):
    _fields_ = [
    ("nWidth", INT32),
    ("nHeight", INT32),
    ("nFPS_Numerator", INT32),
    ("nFPS_Denominator", INT32),
    ("picFourCC", INT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.VIDEOFORMAT) == ctypes.sizeof(VideoFormat))

class VideoFrame(Structure):
    _fields_ = [
    ("nWidth", INT32),
    ("nHeight", INT32),
    ("nStreamID", INT32),
    ("bKeyFrame", BOOL),
    ("frameBuffer", c_void_p),
    ("nFrameBufferSize", INT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.VIDEOFRAME) == ctypes.sizeof(VideoFrame))

class VideoCaptureDevice(Structure):
    _fields_ = [
    ("szDeviceID", TTCHAR*TT_STRLEN),
    ("szDeviceName", TTCHAR*TT_STRLEN),
    ("szCaptureAPI", TTCHAR*TT_STRLEN),
    ("videoFormats", VideoFormat*TT_VIDEOFORMATS_MAX),
    ("nVideoFormatsCount", INT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.VIDEOCAPTUREDEVICE) == ctypes.sizeof(VideoCaptureDevice))

class BitmapFormat(INT32):
    BMP_NONE = 0
    BMP_RGB8_PALETTE = 1
    BMP_RGB16_555 = 2
    BMP_RGB24 = 3
    BMP_RGB32 = 4

class DesktopProtocol(INT32):
    DESKTOPPROTOCOL_ZLIB_1 = 1

class DesktopWindow(Structure):
    _fields_ = [
    ("nWidth", INT32),
    ("nHeight", INT32),
    ("bmpFormat", INT32),
    ("nBytesPerLine", INT32),
    ("nSessionID", INT32),
    ("nProtocol", INT32),
    ("frameBuffer", c_void_p),
    ("nFrameBufferSize", INT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.DESKTOPWINDOW) == ctypes.sizeof(DesktopWindow))

class DesktopKeyState(UINT32):
    DESKTOPKEYSTATE_NONE = 0x00000000
    DESKTOPKEYSTATE_DOWN = 0x00000001
    DESKTOPKEYSTATE_UP = 0x00000002

class DesktopInput(Structure):
    _fields_ = [
    ("uMousePosX", c_uint16),
    ("uMousePosY", c_uint16),
    ("uKeyCode", UINT32),
    ("uKeyState", UINT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.DESKTOPINPUT) == ctypes.sizeof(DesktopInput))

class SpeexCodec(Structure):
    _fields_ = [
    ("nBandmode", INT32),
    ("nQuality", INT32),
    ("nTxIntervalMSec", INT32),
    ("bStereoPlayback", BOOL)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.SPEEXCODEC) == ctypes.sizeof(SpeexCodec))

class SpeexVBRCodec(Structure):
    _fields_ = [
    ("nBandmode", INT32),
    ("nQuality", INT32),
    ("nBitRate", INT32),
    ("nMaxBitRate", INT32),
    ("bDTX", BOOL),
    ("nTxIntervalMSec", INT32),
    ("bStereoPlayback", BOOL)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.SPEEXVBRCODEC) == ctypes.sizeof(SpeexVBRCodec))

SPEEX_NB_MIN_BITRATE = 2150
SPEEX_NB_MAX_BITRATE = 24600
SPEEX_WB_MIN_BITRATE = 3950
SPEEX_WB_MAX_BITRATE = 42200
SPEEX_UWB_MIN_BITRATE = 4150
SPEEX_UWB_MAX_BITRATE = 44000

class OpusCodec(Structure):
    _fields_ = [
    ("nSampleRate", INT32),
    ("nChannels", INT32),
    ("nApplication", INT32),
    ("nComplexity", INT32),
    ("bFEC", BOOL),
    ("bDTX", BOOL),
    ("nBitRate", INT32),
    ("bVBR", BOOL),
    ("bVBRConstraint", BOOL),
    ("nTxIntervalMSec", INT32),
    ("nFrameSizeMSec", INT32),
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.OPUSCODEC) == ctypes.sizeof(OpusCodec))

OPUS_APPLICATION_VOIP = 2048
OPUS_APPLICATION_AUDIO = 2049
OPUS_MIN_BITRATE = 6000
OPUS_MAX_BITRATE = 510000
OPUS_MIN_FRAMESIZE = 2
OPUS_MAX_FRAMESIZE = 60
OPUS_REALMAX_FRAMESIZE = 120

class SpeexDSP(Structure):
    _fields_ = [
    ("bEnableAGC", BOOL),
    ("nGainLevel", INT32),
    ("nMaxIncDBSec", INT32),
    ("nMaxDecDBSec", INT32),
    ("nMaxGainDB", INT32),
    ("bEnableDenoise", BOOL),
    ("nMaxNoiseSuppressDB", INT32),
    ("bEnableEchoCancellation", BOOL),
    ("nEchoSuppress", INT32),
    ("nEchoSuppressActive", INT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.SPEEXDSP) == ctypes.sizeof(SpeexDSP))
        

class TTAudioPreprocessor(Structure):
    _fields_ = [
    ("nGainLevel", INT32),
    ("bMuteLeftSpeaker", BOOL),
    ("bMuteRightSpeaker", BOOL)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.TTAUDIOPREPROCESSOR) == ctypes.sizeof(TTAudioPreprocessor))

class AudioPreprocessorType(INT32):
    NO_AUDIOPREPROCESSOR = 0
    SPEEXDSP_AUDIOPREPROCESSOR = 1
    TEAMTALK_AUDIOPREPROCESSOR = 2

class AudioPreprocessorUnion(Union):
    _fields_ = [
    ("speexdsp", SpeexDSP),
    ("ttpreprocessor", TTAudioPreprocessor)
    ]

class AudioPreprocessor(Structure):
    _anonymous_ = ["u"]
    _fields_ = [
    ("nPreprocessor", INT32),
    ("u", AudioPreprocessorUnion)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.AUDIOPREPROCESSOR) == ctypes.sizeof(AudioPreprocessor))

class WebMVP8CodecUnion(Union):
    _fields_ = [
    ("nRcTargetBitrate", INT32),
    ("rc_target_bitrate", UINT32)
    ]

class WebMVP8Codec(Structure):
    _anonymous_ = ["u"]
    _fields_ = [
    ("u", WebMVP8CodecUnion),
    ("nEncodeDeadline", UINT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.WEBMVP8CODEC) == ctypes.sizeof(WebMVP8Codec))

WEBM_VPX_DL_REALTIME = 1
WEBM_VPX_DL_GOOD_QUALITY = 1000000
WEBM_VPX_DL_BEST_QUALITY = 0

class Codec(INT32):
    NO_CODEC = 0
    SPEEX_CODEC = 1
    SPEEX_VBR_CODEC = 2
    OPUS_CODEC = 3
    WEBM_VP8_CODEC = 128

class AudioCodecUnion(Union):
    _fields_ = [
    ("speex", SpeexCodec),
    ("speex_vbr", SpeexVBRCodec),
    ("opus", OpusCodec)
    ]

class AudioCodec(Structure):
    _anonymous_ = ["u"]
    _fields_ = [
    ("nCodec", INT32),
    ("u", AudioCodecUnion)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.AUDIOCODEC) == ctypes.sizeof(AudioCodec))

class AudioConfig(Structure):
    _fields_ = [
    ("bEnableAGC", BOOL),
    ("nGainLevel", INT32),
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.AUDIOCONFIG) == ctypes.sizeof(AudioConfig))

class VideoCodecUnion(Union):
    _fields_ = [
    ("webm_vp8", WebMVP8Codec)
    ]

class VideoCodec(Structure):
    _anonymous_ = ["u"]
    _fields_ = [
    ("nCodec", INT32),
    ("u", VideoCodecUnion)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.VIDEOCODEC) == ctypes.sizeof(VideoCodec))

class MediaFileInfo(Structure):
    _fields_ = [
    ("nStatus", INT32),
    ("szFileName", TTCHAR*TT_STRLEN),
    ("audioFmt", AudioFormat),
    ("videoFmt", VideoFormat),
    ("uDurationMSec", UINT32),
    ("uElapsedMSec", UINT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.MEDIAFILEINFO) == ctypes.sizeof(MediaFileInfo))

class MediaFilePlayback(Structure):
    _fields_ = [
    ("uOffsetMSec", UINT32),
    ("bPaused", BOOL),
    ("audioPreprocessor", AudioPreprocessor)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.MEDIAFILEPLAYBACK) == ctypes.sizeof(MediaFilePlayback))

class AudioInputProgress(Structure):
    _fields_ = [
    ("nStreamID", INT32),
    ("uQueueMSec", UINT32),
    ("uElapsedMSec", UINT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.AudioInputProgress) == ctypes.sizeof(AudioInputProgress))

class StreamType(UINT32):
    STREAMTYPE_NONE = 0x00000000
    STREAMTYPE_VOICE = 0x00000001
    STREAMTYPE_VIDEOCAPTURE = 0x00000002
    STREAMTYPE_MEDIAFILE_AUDIO = 0x00000004
    STREAMTYPE_MEDIAFILE_VIDEO = 0x00000008
    STREAMTYPE_DESKTOP = 0x00000010
    STREAMTYPE_DESKTOPINPUT = 0x00000020
    STREAMTYPE_MEDIAFILE = STREAMTYPE_MEDIAFILE_AUDIO | STREAMTYPE_MEDIAFILE_VIDEO
    STREAMTYPE_CLASSROOM_ALL = STREAMTYPE_VOICE | STREAMTYPE_VIDEOCAPTURE | STREAMTYPE_DESKTOP | STREAMTYPE_MEDIAFILE

class UserRight(UINT32):
    USERRIGHT_NONE = 0x00000000
    USERRIGHT_MULTI_LOGIN = 0x00000001
    USERRIGHT_VIEW_ALL_USERS = 0x00000002
    USERRIGHT_CREATE_TEMPORARY_CHANNEL = 0x00000004
    USERRIGHT_MODIFY_CHANNELS = 0x00000008
    USERRIGHT_TEXTMESSAGE_BROADCAST = 0x00000010
    USERRIGHT_KICK_USERS = 0x00000020
    USERRIGHT_BAN_USERS = 0x00000040
    USERRIGHT_MOVE_USERS = 0x00000080
    USERRIGHT_OPERATOR_ENABLE = 0x00000100
    USERRIGHT_UPLOAD_FILES = 0x00000200
    USERRIGHT_DOWNLOAD_FILES = 0x00000400
    USERRIGHT_UPDATE_SERVERPROPERTIES = 0x00000800
    USERRIGHT_TRANSMIT_VOICE = 0x00001000
    USERRIGHT_TRANSMIT_VIDEOCAPTURE = 0x00002000
    USERRIGHT_TRANSMIT_DESKTOP = 0x00004000
    USERRIGHT_TRANSMIT_DESKTOPINPUT = 0x00008000
    USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO = 0x00010000
    USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO = 0x00020000
    USERRIGHT_TRANSMIT_MEDIAFILE = USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO | USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO
    USERRIGHT_LOCKED_NICKNAME = 0x00040000
    USERRIGHT_LOCKED_STATUS = 0x00080000
    USERRIGHT_RECORD_VOICE = 0x00100000

class ServerProperties(Structure):
    _fields_ = [
    ("szServerName", TTCHAR*TT_STRLEN),
    ("szMOTD", TTCHAR*TT_STRLEN),
    ("szMOTDRaw", TTCHAR*TT_STRLEN),
    ("nMaxUsers", INT32),
    ("nMaxLoginAttempts", INT32),
    ("nMaxLoginsPerIPAddress", INT32),
    ("nMaxVoiceTxPerSecond", INT32),
    ("nMaxVideoCaptureTxPerSecond", INT32),
    ("nMaxMediaFileTxPerSecond", INT32),
    ("nMaxDesktopTxPerSecond", INT32),
    ("nMaxTotalTxPerSecond", INT32),
    ("bAutoSave", BOOL),
    ("nTcpPort", INT32),
    ("nUdpPort", INT32),
    ("nUserTimeout", INT32),
    ("szServerVersion", TTCHAR*TT_STRLEN),
    ("szServerProtocolVersion", TTCHAR*TT_STRLEN),
    ("nLoginDelayMSec", INT32),
    ("szAccessToken", TTCHAR*TT_STRLEN),
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.SERVERPROPERTIES) == ctypes.sizeof(ServerProperties))

class ServerStatistics(Structure):
    _fields_ = [
    ("nTotalBytesTX", INT64),
    ("nTotalBytesRX", INT64),
    ("nVoiceBytesTX", INT64),
    ("nVoiceBytesRX", INT64),
    ("nVideoCaptureBytesTX", INT64),
    ("nVideoCaptureBytesRX", INT64),
    ("nMediaFileBytesTX", INT64),
    ("nMediaFileBytesRX", INT64),
    ("nDesktopBytesTX", INT64),
    ("nDesktopBytesRX", INT64),
    ("nUsersServed", INT32),
    ("nUsersPeak", INT32),
    ("nFilesTx", INT64),
    ("nFilesRx", INT64),
    ("nUptimeMSec", INT64)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.SERVERSTATISTICS) == ctypes.sizeof(ServerStatistics))

class BanType(UINT32):
    BANTYPE_NONE = 0x00
    BANTYPE_CHANNEL = 0x01
    BANTYPE_IPADDR = 0x02
    BANTYPE_USERNAME = 0x04

class BannedUser(Structure):
    _fields_ = [
    ("szIPAddress", TTCHAR*TT_STRLEN),
    ("szChannelPath", TTCHAR*TT_STRLEN),
    ("szBanTime", TTCHAR*TT_STRLEN),
    ("szNickname", TTCHAR*TT_STRLEN),
    ("szUsername", TTCHAR*TT_STRLEN),
    ("uBanTypes", UINT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.BANNEDUSER) == ctypes.sizeof(BannedUser))

class UserType(UINT32):
    USERTYPE_NONE = 0x0
    USERTYPE_DEFAULT = 0x01
    USERTYPE_ADMIN = 0x02

class AbusePrevention(Structure):
    _fields_ = [
    ("nCommandsLimit", INT32),
    ("nCommandsIntervalMSec", INT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.ABUSEPREVENTION) == ctypes.sizeof(AbusePrevention))

class UserAccount(Structure):
    _fields_ = [
    ("szUsername", TTCHAR*TT_STRLEN),
    ("szPassword", TTCHAR*TT_STRLEN),
    ("uUserType", UINT32),
    ("uUserRights", UINT32),
    ("nUserData", INT32),
    ("szNote", TTCHAR*TT_STRLEN),
    ("szInitChannel", TTCHAR*TT_STRLEN),
    ("autoOperatorChannels", INT32*TT_CHANNELS_OPERATOR_MAX),
    ("nAudioCodecBpsLimit", INT32),
    ("abusePrevent", AbusePrevention)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.USERACCOUNT) == ctypes.sizeof(UserAccount))

class Subscription(UINT32):
    SUBSCRIBE_NONE = 0x00000000
    SUBSCRIBE_USER_MSG = 0x00000001
    SUBSCRIBE_CHANNEL_MSG = 0x00000002
    SUBSCRIBE_BROADCAST_MSG = 0x00000004
    SUBSCRIBE_CUSTOM_MSG = 0x00000008
    SUBSCRIBE_VOICE = 0x00000010
    SUBSCRIBE_VIDEOCAPTURE = 0x00000020
    SUBSCRIBE_DESKTOP = 0x00000040
    SUBSCRIBE_DESKTOPINPUT = 0x00000080
    SUBSCRIBE_MEDIAFILE = 0x00000100
    SUBSCRIBE_INTERCEPT_USER_MSG = 0x00010000
    SUBSCRIBE_INTERCEPT_CHANNEL_MSG = 0x00020000
    SUBSCRIBE_INTERCEPT_CUSTOM_MSG = 0x00080000
    SUBSCRIBE_INTERCEPT_VOICE = 0x00100000
    SUBSCRIBE_INTERCEPT_VIDEOCAPTURE = 0x00200000
    SUBSCRIBE_INTERCEPT_DESKTOP = 0x00400000
    SUBSCRIBE_INTERCEPT_MEDIAFILE = 0x01000000

class UserState(UINT32):
    USERSTATE_NONE = 0x0000000
    USERSTATE_VOICE = 0x00000001
    USERSTATE_MUTE_VOICE = 0x00000002
    USERSTATE_MUTE_MEDIAFILE = 0x00000004
    USERSTATE_DESKTOP = 0x00000008
    USERSTATE_VIDEOCAPTURE = 0x00000010
    USERSTATE_MEDIAFILE_AUDIO = 0x00000020
    USERSTATE_MEDIAFILE_VIDEO = 0x00000040
    USERSTATE_MEDIAFILE = USERSTATE_MEDIAFILE_AUDIO | USERSTATE_MEDIAFILE_VIDEO

class User(Structure):
    _fields_ = [
    ("nUserID", INT32),
    ("szUsername", TTCHAR * TT_STRLEN),
    ("nUserData", INT32),
    ("uUserType", UINT32),
    ("szIPAddress", TTCHAR * TT_STRLEN),
    ("uVersion", UINT32),
    ("nChannelID", INT32),
    ("uLocalSubscriptions", UINT32),
    ("uPeerSubscriptions", UINT32),
    ("szNickname", TTCHAR * TT_STRLEN),
    ("nStatusMode", INT32),
    ("szStatusMsg", TTCHAR * TT_STRLEN),
    ("uUserState", UINT32),
    ("szMediaStorageDir", TTCHAR * TT_STRLEN),
    ("nVolumeVoice", INT32),
    ("nVolumeMediaFile", INT32),
    ("nStoppedDelayVoice", INT32),
    ("nStoppedDelayMediaFile", INT32),
    ("soundPositionVoice", c_float*3),
    ("soundPositionMediaFile", c_float*3),
    ("stereoPlaybackVoice", BOOL*2),
    ("stereoPlaybackMediaFile", BOOL*2),
    ("nBufferMSecVoice", INT32),
    ("nBufferMSecMediaFile", INT32),
    ("szClientName", TTCHAR * TT_STRLEN)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.USER) == ctypes.sizeof(User))

class UserStatistics(Structure):
    _fields_ = [
    ("nVoicePacketsRecv", INT64),
    ("nVoicePacketsLost", INT64),
    ("nVideoCapturePacketsRecv", INT64),
    ("nVideoCaptureFramesRecv", INT64),
    ("nVideoCaptureFramesLost", INT64),
    ("nVideoCaptureFramesDropped", INT64),
    ("nMediaFileAudioPacketsRecv", INT64),
    ("nMediaFileAudioPacketsLost", INT64),
    ("nMediaFileVideoPacketsRecv", INT64),
    ("nMediaFileVideoFramesRecv", INT64),
    ("nMediaFileVideoFramesLost", INT64),
    ("nMediaFileVideoFramesDropped", INT64),
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.USERSTATISTICS) == ctypes.sizeof(UserStatistics))

class TextMsgType(INT32):
    MSGTYPE_USER = 1
    MSGTYPE_CHANNEL = 2
    MSGTYPE_BROADCAST = 3
    MSGTYPE_CUSTOM = 4

class TextMessage(Structure):
    _fields_ = [
    ("nMsgType", INT32),
    ("nFromUserID", INT32),
    ("szFromUsername", TTCHAR*TT_STRLEN),
    ("nToUserID", INT32),
    ("nChannelID", INT32),
    ("szMessage", TTCHAR*TT_STRLEN),
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.TEXTMESSAGE) == ctypes.sizeof(TextMessage))

class ChannelType(UINT32):
    CHANNEL_DEFAULT = 0x0000
    CHANNEL_PERMANENT = 0x0001
    CHANNEL_SOLO_TRANSMIT = 0x0002
    CHANNEL_CLASSROOM = 0x0004
    CHANNEL_OPERATOR_RECVONLY = 0x0008
    CHANNEL_NO_VOICEACTIVATION = 0x0010
    CHANNEL_NO_RECORDING = 0x0020

class Channel(Structure):
    _fields_ = [
    ("nParentID", INT32),
    ("nChannelID", INT32),
    ("szName", TTCHAR*TT_STRLEN),
    ("szTopic", TTCHAR*TT_STRLEN),
    ("szPassword", TTCHAR*TT_STRLEN),
    ("bPassword", BOOL),
    ("uChannelType", UINT32),
    ("nUserData", INT32),
    ("nDiskQuota", INT64),
    ("szOpPassword", TTCHAR*TT_STRLEN),
    ("nMaxUsers", INT32),
    ("audiocodec", AudioCodec),
    ("audiocfg", AudioConfig),
    ("transmitUsers", (INT32*2)*TT_TRANSMITUSERS_MAX),
    ("transmitUsersQueue", INT32*TT_TRANSMITQUEUE_MAX)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.CHANNEL) == ctypes.sizeof(Channel))

class FileTransferStatus(INT32):
    FILETRANSFER_CLOSED = 0
    FILETRANSFER_ERROR = 1
    FILETRANSFER_ACTIVE = 2
    FILETRANSFER_FINISHED = 3

class FileTransfer(Structure):
    _fields_ = [
    ("nStatus", INT32),
    ("nTransferID", INT32),
    ("nChannelID", INT32),
    ("szLocalFilePath", TTCHAR*TT_STRLEN),
    ("szRemoteFileName", TTCHAR*TT_STRLEN),
    ("nFileSize", INT64),
    ("nTransferred", INT64),
    ("bInbound", BOOL)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.FILETRANSFER) == ctypes.sizeof(FileTransfer))

class RemoteFile(Structure):
    _fields_ = [
    ("nChannelID", INT32),
    ("nFileID", INT32),
    ("szFileName", TTCHAR*TT_STRLEN),
    ("nFileSize", INT64),
    ("szUsername", TTCHAR*TT_STRLEN)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.REMOTEFILE) == ctypes.sizeof(RemoteFile))

class EncryptionContext(Structure):
    _fields_ = [
    ("szCertificateFile", TTCHAR*TT_STRLEN),
    ("szPrivateKeyFile", TTCHAR*TT_STRLEN),
    ("szCAFile", TTCHAR*TT_STRLEN),
    ("szCADir", TTCHAR*TT_STRLEN),
    ("bVerifyPeer", BOOL),
    ("bVerifyClientOnce", BOOL),
    ("nVerifyDepth", INT32)
    ]

class ClientKeepAlive(Structure):
    _fields_ = [
    ("nConnectionLostMSec", INT32),
    ("nTcpKeepAliveIntervalMSec", INT32),
    ("nUdpKeepAliveIntervalMSec", INT32),
    ("nUdpKeepAliveRTXMSec", INT32),
    ("nUdpConnectRTXMSec", INT32),
    ("nUdpConnectTimeoutMSec", INT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.CLIENTKEEPALIVE) == ctypes.sizeof(ClientKeepAlive))

class ClientStatistics(Structure):
    _fields_ = [
    ("nUdpBytesSent", INT64),
    ("nUdpBytesRecv", INT64),
    ("nVoiceBytesSent", INT64),
    ("nVoiceBytesRecv", INT64),
    ("nVideoCaptureBytesSent", INT64),
    ("nVideoCaptureBytesRecv", INT64),
    ("nMediaFileAudioBytesSent", INT64),
    ("nMediaFileAudioBytesRecv", INT64),
    ("nMediaFileVideoBytesSent", INT64),
    ("nMediaFileVideoBytesRecv", INT64),
    ("nDesktopBytesSent", INT64),
    ("nDesktopBytesRecv", INT64),
    ("nUdpPingTimeMs", INT32),
    ("nTcpPingTimeMs", INT32),
    ("nTcpServerSilenceSec", INT32),
    ("nUdpServerSilenceSec", INT32)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.CLIENTSTATISTICS) == ctypes.sizeof(ClientStatistics))

class ClientError(INT32):
    CMDERR_SUCCESS = 0
    CMDERR_SYNTAX_ERROR = 1000
    CMDERR_UNKNOWN_COMMAND = 1001
    CMDERR_MISSING_PARAMETER = 1002    CMDERR_INCOMPATIBLE_PROTOCOLS = 1003
    CMDERR_UNKNOWN_AUDIOCODEC = 1004
    CMDERR_INVALID_USERNAME = 1005
    CMDERR_INCORRECT_CHANNEL_PASSWORD = 2001
    CMDERR_INVALID_ACCOUNT = 2002
    CMDERR_MAX_SERVER_USERS_EXCEEDED = 2003
    CMDERR_MAX_CHANNEL_USERS_EXCEEDED = 2004
    CMDERR_SERVER_BANNED = 2005
    CMDERR_NOT_AUTHORIZED = 2006
    CMDERR_MAX_DISKUSAGE_EXCEEDED = 2008
    CMDERR_INCORRECT_OP_PASSWORD = 2010
    CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED = 2011
    CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED = 2012
    CMDERR_MAX_CHANNELS_EXCEEDED = 2013
    CMDERR_COMMAND_FLOOD = 2014
    CMDERR_CHANNEL_BANNED = 2015
    CMDERR_NOT_LOGGEDIN = 3000
    CMDERR_ALREADY_LOGGEDIN = 3001
    CMDERR_NOT_IN_CHANNEL = 3002
    CMDERR_ALREADY_IN_CHANNEL = 3003
    CMDERR_CHANNEL_ALREADY_EXISTS = 3004
    CMDERR_CHANNEL_NOT_FOUND = 3005
    CMDERR_USER_NOT_FOUND = 3006
    CMDERR_BAN_NOT_FOUND = 3007
    CMDERR_FILETRANSFER_NOT_FOUND = 3008
    CMDERR_OPENFILE_FAILED = 3009
    CMDERR_ACCOUNT_NOT_FOUND = 3010
    CMDERR_FILE_NOT_FOUND = 3011
    CMDERR_FILE_ALREADY_EXISTS = 3012
    CMDERR_FILESHARING_DISABLED = 3013
    CMDERR_CHANNEL_HAS_USERS = 3015
    CMDERR_LOGINSERVICE_UNAVAILABLE = 3016
    INTERR_SNDINPUT_FAILURE = 10000
    INTERR_SNDOUTPUT_FAILURE = 10001
    INTERR_AUDIOCODEC_INIT_FAILED = 10002
    INTERR_SPEEXDSP_INIT_FAILED = 10003
    INTERR_TTMESSAGE_QUEUE_OVERFLOW = 10004
    INTERR_SNDEFFECT_FAILURE = 10005

class ClientErrorMsg(Structure):
    _fields_ = [
    ("nErrorNo", INT32),
    ("szErrorMsg", TTCHAR*TT_STRLEN)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.CLIENTERRORMSG) == ctypes.sizeof(ClientErrorMsg))

class ClientEvent(UINT32):
    CLIENTEVENT_NONE = 0
    CLIENTEVENT_CON_SUCCESS = CLIENTEVENT_NONE + 10
    CLIENTEVENT_CON_FAILED = CLIENTEVENT_NONE + 20
    CLIENTEVENT_CON_LOST = CLIENTEVENT_NONE + 30
    CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED = CLIENTEVENT_NONE + 40
    CLIENTEVENT_CMD_PROCESSING = CLIENTEVENT_NONE + 200
    CLIENTEVENT_CMD_ERROR = CLIENTEVENT_NONE + 210
    CLIENTEVENT_CMD_SUCCESS = CLIENTEVENT_NONE + 220
    CLIENTEVENT_CMD_MYSELF_LOGGEDIN = CLIENTEVENT_NONE + 230
    CLIENTEVENT_CMD_MYSELF_LOGGEDOUT = CLIENTEVENT_NONE + 240
    CLIENTEVENT_CMD_MYSELF_KICKED = CLIENTEVENT_NONE + 250
    CLIENTEVENT_CMD_USER_LOGGEDIN = CLIENTEVENT_NONE + 260
    CLIENTEVENT_CMD_USER_LOGGEDOUT = CLIENTEVENT_NONE + 270
    CLIENTEVENT_CMD_USER_UPDATE = CLIENTEVENT_NONE + 280
    CLIENTEVENT_CMD_USER_JOINED = CLIENTEVENT_NONE + 290
    CLIENTEVENT_CMD_USER_LEFT = CLIENTEVENT_NONE + 300
    CLIENTEVENT_CMD_USER_TEXTMSG = CLIENTEVENT_NONE + 310
    CLIENTEVENT_CMD_CHANNEL_NEW = CLIENTEVENT_NONE + 320
    CLIENTEVENT_CMD_CHANNEL_UPDATE = CLIENTEVENT_NONE + 330
    CLIENTEVENT_CMD_CHANNEL_REMOVE = CLIENTEVENT_NONE + 340
    CLIENTEVENT_CMD_SERVER_UPDATE = CLIENTEVENT_NONE + 350
    CLIENTEVENT_CMD_SERVERSTATISTICS = CLIENTEVENT_NONE + 360
    CLIENTEVENT_CMD_FILE_NEW = CLIENTEVENT_NONE + 370
    CLIENTEVENT_CMD_FILE_REMOVE = CLIENTEVENT_NONE + 380
    CLIENTEVENT_CMD_USERACCOUNT = CLIENTEVENT_NONE + 390
    CLIENTEVENT_CMD_BANNEDUSER  = CLIENTEVENT_NONE + 400
    CLIENTEVENT_USER_STATECHANGE = CLIENTEVENT_NONE + 500
    CLIENTEVENT_USER_VIDEOCAPTURE = CLIENTEVENT_NONE + 510
    CLIENTEVENT_USER_MEDIAFILE_VIDEO = CLIENTEVENT_NONE + 520
    CLIENTEVENT_USER_DESKTOPWINDOW = CLIENTEVENT_NONE + 530
    CLIENTEVENT_USER_DESKTOPCURSOR = CLIENTEVENT_NONE + 540
    CLIENTEVENT_USER_DESKTOPINPUT = CLIENTEVENT_NONE + 550
    CLIENTEVENT_USER_RECORD_MEDIAFILE = CLIENTEVENT_NONE + 560
    CLIENTEVENT_USER_AUDIOBLOCK = CLIENTEVENT_NONE + 570
    CLIENTEVENT_INTERNAL_ERROR = CLIENTEVENT_NONE + 1000
    CLIENTEVENT_VOICE_ACTIVATION = CLIENTEVENT_NONE + 1010
    CLIENTEVENT_HOTKEY = CLIENTEVENT_NONE + 1020
    CLIENTEVENT_HOTKEY_TEST = CLIENTEVENT_NONE + 1030
    CLIENTEVENT_FILETRANSFER = CLIENTEVENT_NONE + 1040
    CLIENTEVENT_DESKTOPWINDOW_TRANSFER = CLIENTEVENT_NONE + 1050
    CLIENTEVENT_STREAM_MEDIAFILE = CLIENTEVENT_NONE + 1060
    CLIENTEVENT_LOCAL_MEDIAFILE = CLIENTEVENT_NONE + 1070
    CLIENTEVENT_AUDIOINPUT = CLIENTEVENT_NONE + 1080

# Underscore has special meaning in Python, so we remove it
class TTType(INT32):
    NONE = 0
    AUDIOCODEC = 1
    BANNEDUSER = 2
    VIDEOFORMAT =  3
    OPUSCODEC =  4
    CHANNEL =  5
    CLIENTSTATISTICS =  6
    REMOTEFILE =  7
    FILETRANSFER =  8
    MEDIAFILESTATUS =  9
    SERVERPROPERTIES = 10
    SERVERSTATISTICS = 11
    SOUNDDEVICE = 12
    SPEEXCODEC = 13
    TEXTMESSAGE = 14
    WEBMVP8CODEC = 15
    TTMESSAGE = 16
    USER = 17
    USERACCOUNT = 18
    USERSTATISTICS = 19
    VIDEOCAPTUREDEVICE = 20
    VIDEOCODEC = 21
    AUDIOCONFIG = 22
    SPEEXVBRCODEC = 23
    VIDEOFRAME = 24
    AUDIOBLOCK = 25
    AUDIOFORMAT = 26
    MEDIAFILEINFO = 27
    CLIENTERRORMSG = 28
    TTBOOL = 29
    INT32 = 30
    DESKTOPINPUT = 31
    SPEEXDSP = 32
    STREAMTYPE = 33
    AUDIOPREPROCESSORTYPE = 34
    AUDIOPREPROCESSOR = 35
    TTAUDIOPREPROCESSOR = 36
    MEDIAFILEPLAYBACK = 37
    CLIENTKEEPALIVE = 38
    UINT32 = 39
    AUDIOINPUTPROGRESS = 40

class TTMessageUnion(Union):
    _fields_ = [
    ("channel", Channel),
    ("clienterrormsg", ClientErrorMsg),
    ("desktopinput", DesktopInput),
    ("filetransfer", FileTransfer),
    ("mediafileinfo", MediaFileInfo),
    ("remotefile", RemoteFile),
    ("serverproperties", ServerProperties),
    ("serverstatistics", ServerStatistics),
    ("textmessage", TextMessage),
    ("user", User),
    ("useraccount", UserAccount),
    ("banneduser", BannedUser),
    ("bActive", BOOL),
    ("nBytesRemain", INT32),
    ("nStreamID", INT32),
    ("nPayloadSize", INT32),
    ("nStreamType", INT32),
    ("audioinputprogress", AudioInputProgress),
    ("data", c_char*1)
    ]

class TTMessage(Structure):
    _anonymous_ = ["u"]

    _fields_ = [
    ("nClientEvent", UINT32),
    ("nSource", INT32),
    ("ttType", INT32),
    ("uReserved", UINT32),
    ("u", TTMessageUnion)
    ]
    def __init__(self):
        assert(DBG_SIZEOF(TTType.TTMESSAGE) == ctypes.sizeof(TTMessage))

class ClientFlags(UINT32):
    CLIENT_CLOSED = 0x00000000
    CLIENT_SNDINPUT_READY = 0x00000001
    CLIENT_SNDOUTPUT_READY = 0x00000002
    CLIENT_SNDINOUTPUT_DUPLEX = 0x00000004
    CLIENT_SNDINPUT_VOICEACTIVATED = 0x00000008
    CLIENT_SNDINPUT_VOICEACTIVE = 0x00000010
    CLIENT_SNDOUTPUT_MUTE = 0x00000020
    CLIENT_SNDOUTPUT_AUTO3DPOSITION = 0x00000040
    CLIENT_VIDEOCAPTURE_READY = 0x00000080
    CLIENT_TX_VOICE = 0x00000100
    CLIENT_TX_VIDEOCAPTURE = 0x00000200
    CLIENT_TX_DESKTOP = 0x00000400
    CLIENT_DESKTOP_ACTIVE = 0x00000800
    CLIENT_MUX_AUDIOFILE = 0x00001000
    CLIENT_CONNECTING = 0x00002000
    CLIENT_CONNECTED = 0x00004000
    CLIENT_CONNECTION               = CLIENT_CONNECTING or CLIENT_CONNECTED
    CLIENT_AUTHORIZED = 0x00008000
    CLIENT_STREAM_AUDIO = 0x00010000
    CLIENT_STREAM_VIDEO = 0x00020000

_GetVersion = dll.TT_GetVersion
_GetVersion.restype = TTCHAR_P
_InitTeamTalkPoll = dll.TT_InitTeamTalkPoll
_InitTeamTalkPoll.restype = _TTInstance
_CloseTeamTalk = dll.TT_CloseTeamTalk
_CloseTeamTalk.restype = BOOL
_CloseTeamTalk.argtypes = [_TTInstance]
_GetMessage = dll.TT_GetMessage
_GetMessage.restype = BOOL
_GetMessage.argtypes = [_TTInstance, POINTER(TTMessage), POINTER(INT32)]
_PumpMessage = dll.TT_PumpMessage
_PumpMessage.restype = BOOL
_PumpMessage.argtypes = [_TTInstance, ClientEvent, INT32]
_GetFlags = dll.TT_GetFlags
_GetFlags.restype = UINT32
_GetFlags.argtypes = [_TTInstance]
_SetLicenseInformation = dll.TT_SetLicenseInformation
_SetLicenseInformation.restype = BOOL
_SetLicenseInformation.argtypes = [TTCHAR_P, TTCHAR_P]
_GetDefaultSoundDevices = dll.TT_GetDefaultSoundDevices
_GetDefaultSoundDevices.restype = BOOL
_GetDefaultSoundDevices.argtypes = [POINTER(INT32), POINTER(INT32)]
_GetDefaultSoundDevicesEx = dll.TT_GetDefaultSoundDevicesEx
_GetDefaultSoundDevicesEx.restype = BOOL
_GetDefaultSoundDevicesEx.argtypes = [SoundSystem, POINTER(INT32), POINTER(INT32)]
_GetSoundDevices = dll.TT_GetSoundDevices
_GetSoundDevices.restype = BOOL
_GetSoundDevices.argtypes = [POINTER(SoundDevice), POINTER(INT32)]
_RestartSoundSystem = dll.TT_RestartSoundSystem
_RestartSoundSystem.restype = BOOL
_StartSoundLoopbackTest = dll.TT_StartSoundLoopbackTest
_StartSoundLoopbackTest.restype = _TTSoundLoop
_StartSoundLoopbackTest.argtypes = [INT32, INT32, INT32, INT32, BOOL, POINTER(SpeexDSP)]
_StartSoundLoopbackTestEx = dll.TT_StartSoundLoopbackTestEx
_StartSoundLoopbackTestEx.restype = _TTSoundLoop
_StartSoundLoopbackTestEx.argtypes = [INT32, INT32, INT32, INT32, BOOL, POINTER(AudioPreprocessor), POINTER(SoundDeviceEffects)]
_CloseSoundLoopbackTest = dll.TT_CloseSoundLoopbackTest
_CloseSoundLoopbackTest.restype = BOOL
_CloseSoundLoopbackTest.argtypes = [_TTSoundLoop]
_InitSoundInputDevice = dll.TT_InitSoundInputDevice
_InitSoundInputDevice.restype = BOOL
_InitSoundInputDevice.argtypes = [_TTInstance, INT32]
_InitSoundInputSharedDevice = dll.TT_InitSoundInputSharedDevice
_InitSoundInputSharedDevice.restype = BOOL
_InitSoundInputSharedDevice.argtypes = [INT32, INT32, INT32]
_InitSoundOutputDevice = dll.TT_InitSoundOutputDevice
_InitSoundOutputDevice.restype = BOOL
_InitSoundOutputDevice.argtypes = [_TTInstance, INT32]
_InitSoundOutputSharedDevice = dll.TT_InitSoundOutputSharedDevice
_InitSoundOutputSharedDevice.restype = BOOL
_InitSoundOutputSharedDevice.argtypes = [INT32, INT32, INT32]
_InitSoundDuplexDevices = dll.TT_InitSoundDuplexDevices
_InitSoundDuplexDevices.restype = BOOL
_InitSoundDuplexDevices.argtypes = [_TTInstance, INT32, INT32]
_CloseSoundInputDevice = dll.TT_CloseSoundInputDevice
_CloseSoundInputDevice.restype = BOOL
_CloseSoundInputDevice.argtypes = [_TTInstance]
_CloseSoundOutputDevice = dll.TT_CloseSoundOutputDevice
_CloseSoundOutputDevice.restype = BOOL
_CloseSoundOutputDevice.argtypes = [_TTInstance]
_CloseSoundDuplexDevices = dll.TT_CloseSoundDuplexDevices
_CloseSoundDuplexDevices.restype = BOOL
_CloseSoundDuplexDevices.argtypes = [_TTInstance]
_SetSoundDeviceEffects = dll.TT_SetSoundDeviceEffects
_SetSoundDeviceEffects.restype = BOOL
_SetSoundDeviceEffects.argtypes = [_TTInstance, POINTER(SoundDeviceEffects)]
_GetSoundDeviceEffects = dll.TT_GetSoundDeviceEffects
_GetSoundDeviceEffects.restype = BOOL
_GetSoundDeviceEffects.argtypes = [_TTInstance, POINTER(SoundDeviceEffects)]
_GetSoundInputLevel = dll.TT_GetSoundInputLevel
_GetSoundInputLevel.restype = INT32
_GetSoundInputLevel.argtypes = [_TTInstance]
_SetSoundInputGainLevel = dll.TT_SetSoundInputGainLevel
_SetSoundInputGainLevel.restype = BOOL
_SetSoundInputGainLevel.argtypes = [_TTInstance, INT32]
_GetSoundInputGainLevel = dll.TT_GetSoundInputGainLevel
_GetSoundInputGainLevel.restype = INT32
_GetSoundInputGainLevel.argtypes = [_TTInstance]
_SetSoundInputPreprocess = dll.TT_SetSoundInputPreprocess
_SetSoundInputPreprocess.restype = BOOL
_SetSoundInputPreprocess.argtypes = [_TTInstance, POINTER(SpeexDSP)]
_GetSoundInputPreprocess = dll.TT_GetSoundInputPreprocess
_GetSoundInputPreprocess.restype = BOOL
_GetSoundInputPreprocess.argtypes = [_TTInstance, POINTER(SpeexDSP)]
_SetSoundInputPreprocessEx = dll.TT_SetSoundInputPreprocessEx
_SetSoundInputPreprocessEx.restype = BOOL
_SetSoundInputPreprocessEx.argtypes = [_TTInstance, POINTER(AudioPreprocessor)]
_GetSoundInputPreprocessEx = dll.TT_GetSoundInputPreprocessEx
_GetSoundInputPreprocessEx.restype = BOOL
_GetSoundInputPreprocessEx.argtypes = [_TTInstance, POINTER(AudioPreprocessor)]
_SetSoundOutputVolume = dll.TT_SetSoundOutputVolume
_SetSoundOutputVolume.restype = BOOL
_SetSoundOutputVolume.argtypes = [_TTInstance, INT32]
_GetSoundOutputVolume = dll.TT_GetSoundOutputVolume
_GetSoundOutputVolume.restype = INT32
_GetSoundOutputVolume.argtypes = [_TTInstance]
_SetSoundOutputMute = dll.TT_SetSoundOutputMute
_SetSoundOutputMute.restype = BOOL
_SetSoundOutputMute.argtypes = [_TTInstance, BOOL]
_Enable3DSoundPositioning = dll.TT_Enable3DSoundPositioning
_Enable3DSoundPositioning.restype = BOOL
_Enable3DSoundPositioning.argtypes = [_TTInstance, BOOL]
_AutoPositionUsers = dll.TT_AutoPositionUsers
_AutoPositionUsers.restype = BOOL
_AutoPositionUsers.argtypes = [_TTInstance]
_EnableAudioBlockEvent = dll.TT_EnableAudioBlockEvent
_EnableAudioBlockEvent.restype = BOOL
_EnableAudioBlockEvent.argtypes = [_TTInstance, INT32, INT32, BOOL]
_EnableAudioBlockEventEx = dll.TT_EnableAudioBlockEventEx
_EnableAudioBlockEventEx.restype = BOOL
_EnableAudioBlockEventEx.argtypes = [_TTInstance, INT32, INT32, POINTER(AudioFormat), BOOL]
_InsertAudioBlock = dll.TT_InsertAudioBlock
_InsertAudioBlock.restype = BOOL
_InsertAudioBlock.argtypes = [_TTInstance, POINTER(AudioBlock)]
_EnableVoiceTransmission = dll.TT_EnableVoiceTransmission
_EnableVoiceTransmission.restype = BOOL
_EnableVoiceTransmission.argtypes = [_TTInstance, BOOL]
_EnableVoiceActivation = dll.TT_EnableVoiceActivation
_EnableVoiceActivation.restype = BOOL
_EnableVoiceActivation.argtypes = [_TTInstance, BOOL]
_SetVoiceActivationLevel = dll.TT_SetVoiceActivationLevel
_SetVoiceActivationLevel.restype = BOOL
_SetVoiceActivationLevel.argtypes = [_TTInstance, INT32]
_GetVoiceActivationLevel = dll.TT_GetVoiceActivationLevel
_GetVoiceActivationLevel.restype = INT32
_GetVoiceActivationLevel.argtypes = [_TTInstance]
_SetVoiceActivationStopDelay = dll.TT_SetVoiceActivationStopDelay
_SetVoiceActivationStopDelay.restype = BOOL
_SetVoiceActivationStopDelay.argtypes = [_TTInstance, INT32]
_GetVoiceActivationStopDelay = dll.TT_GetVoiceActivationStopDelay
_GetVoiceActivationStopDelay.restype = INT32
_GetVoiceActivationStopDelay.argtypes = [_TTInstance]
_StartRecordingMuxedAudioFile = dll.TT_StartRecordingMuxedAudioFile
_StartRecordingMuxedAudioFile.restype = BOOL
_StartRecordingMuxedAudioFile.argtypes = [_TTInstance, POINTER(AudioCodec), TTCHAR_P, UINT32]
_StartRecordingMuxedAudioFileEx = dll.TT_StartRecordingMuxedAudioFileEx
_StartRecordingMuxedAudioFileEx.restype = BOOL
_StartRecordingMuxedAudioFileEx.argtypes = [_TTInstance, INT32, TTCHAR_P, UINT32]
_StopRecordingMuxedAudioFile = dll.TT_StopRecordingMuxedAudioFile
_StopRecordingMuxedAudioFile.restype = BOOL
_StopRecordingMuxedAudioFile.argtypes = [_TTInstance]
_StopRecordingMuxedAudioFileEx = dll.TT_StopRecordingMuxedAudioFileEx
_StopRecordingMuxedAudioFileEx.restype = BOOL
_StopRecordingMuxedAudioFileEx.argtypes = [_TTInstance, INT32]
_StartVideoCaptureTransmission = dll.TT_StartVideoCaptureTransmission
_StartVideoCaptureTransmission.restype = BOOL
_StartVideoCaptureTransmission.argtypes = [_TTInstance, POINTER(VideoCodec)]
_StopVideoCaptureTransmission = dll.TT_StopVideoCaptureTransmission
_StopVideoCaptureTransmission.restype = BOOL
_StopVideoCaptureTransmission.argtypes = [_TTInstance]
_GetVideoCaptureDevices = dll.TT_GetVideoCaptureDevices
_GetVideoCaptureDevices.restype = BOOL
_GetVideoCaptureDevices.argtypes = [POINTER(VideoCaptureDevice), POINTER(INT32)]
_InitVideoCaptureDevice = dll.TT_InitVideoCaptureDevice
_InitVideoCaptureDevice.restype = BOOL
_InitVideoCaptureDevice.argtypes = [_TTInstance, TTCHAR_P, POINTER(VideoFormat)]
_CloseVideoCaptureDevice = dll.TT_CloseVideoCaptureDevice
_CloseVideoCaptureDevice.restype = BOOL
_CloseVideoCaptureDevice.argtypes = [_TTInstance]
_StartStreamingMediaFileToChannel = dll.TT_StartStreamingMediaFileToChannel
_StartStreamingMediaFileToChannel.restype = BOOL
_StartStreamingMediaFileToChannel.argtypes = [_TTInstance, TTCHAR_P, POINTER(VideoFormat)]
_StartStreamingMediaFileToChannelEx = dll.TT_StartStreamingMediaFileToChannelEx
_StartStreamingMediaFileToChannelEx.restype = BOOL
_StartStreamingMediaFileToChannelEx.argtypes = [_TTInstance, TTCHAR_P, POINTER(MediaFilePlayback), POINTER(VideoFormat)]
_UpdateStreamingMediaFileToChannel = dll.TT_UpdateStreamingMediaFileToChannel
_UpdateStreamingMediaFileToChannel.restype = BOOL
_UpdateStreamingMediaFileToChannel.argtypes = [_TTInstance, POINTER(MediaFilePlayback), POINTER(VideoFormat)]
_StopStreamingMediaFileToChannel = dll.TT_StopStreamingMediaFileToChannel
_StopStreamingMediaFileToChannel.restype = BOOL
_StopStreamingMediaFileToChannel.argtypes = [_TTInstance]
_InitLocalPlayback = dll.TT_InitLocalPlayback
_InitLocalPlayback.restype = INT32
_InitLocalPlayback.argtypes = [_TTInstance, TTCHAR_P, POINTER(MediaFilePlayback)]
_UpdateLocalPlayback = dll.TT_UpdateLocalPlayback
_UpdateLocalPlayback.restype = BOOL
_UpdateLocalPlayback.argtypes = [_TTInstance, INT32, POINTER(MediaFilePlayback)]
_StopLocalPlayback = dll.TT_StopLocalPlayback
_StopLocalPlayback.restype = BOOL
_StopLocalPlayback.argtypes = [_TTInstance, INT32]
_GetMediaFileInfo = dll.TT_GetMediaFileInfo
_GetMediaFileInfo.restype = BOOL
_GetMediaFileInfo.argtypes = [_TTInstance, TTCHAR_P, POINTER(MediaFileInfo)]
_SetEncryptionContext = dll.TT_SetEncryptionContext
_SetEncryptionContext.restype = BOOL
_SetEncryptionContext.argtypes = [_TTInstance, POINTER(EncryptionContext)]
_Connect = dll.TT_Connect
_Connect.restype = BOOL
_Connect.argtypes = [_TTInstance, TTCHAR_P, INT32, INT32, INT32, INT32, BOOL]
_ConnectSysID = dll.TT_ConnectSysID
_ConnectSysID.restype = BOOL
_ConnectSysID.argtypes = [_TTInstance, TTCHAR_P, INT32, INT32, INT32, INT32, BOOL, TTCHAR_P]
_ConnectEx = dll.TT_ConnectEx
_ConnectEx.restype = BOOL
_ConnectEx.argtypes = [_TTInstance, TTCHAR_P, INT32, INT32, TTCHAR_P, INT32, INT32, BOOL]
_Disconnect = dll.TT_Disconnect
_Disconnect.restype = BOOL
_Disconnect.argtypes = [_TTInstance]
_QueryMaxPayload = dll.TT_QueryMaxPayload
_QueryMaxPayload.restype = BOOL
_QueryMaxPayload.argtypes = [_TTInstance, INT32]
_GetClientStatistics = dll.TT_GetClientStatistics
_GetClientStatistics.restype = BOOL
_GetClientStatistics.argtypes = [_TTInstance, POINTER(ClientStatistics)]
_SetClientKeepAlive = dll.TT_SetClientKeepAlive
_SetClientKeepAlive.restype = BOOL
_SetClientKeepAlive.argtypes = [_TTInstance, POINTER(ClientKeepAlive)]
_GetClientKeepAlive = dll.TT_GetClientKeepAlive
_GetClientKeepAlive.restype = BOOL
_GetClientKeepAlive.argtypes = [_TTInstance, POINTER(ClientKeepAlive)]
_DoPing = dll.TT_DoPing
_DoPing.restype = INT32
_DoPing.argtypes = [_TTInstance]
_DoLogin = dll.TT_DoLogin
_DoLogin.restype = INT32
_DoLogin.argtypes = [_TTInstance, TTCHAR_P, TTCHAR_P, TTCHAR_P]
_DoLoginEx = dll.TT_DoLoginEx
_DoLoginEx.restype = INT32
_DoLoginEx.argtypes = [_TTInstance, TTCHAR_P, TTCHAR_P, TTCHAR_P, TTCHAR_P]
_DoLogout = dll.TT_DoLogout
_DoLogout.restype = INT32
_DoLogout.argtypes = [_TTInstance]
_DoJoinChannel = dll.TT_DoJoinChannel
_DoJoinChannel.restype = INT32
_DoJoinChannel.argtypes = [_TTInstance, POINTER(Channel)]
_DoJoinChannelByID = dll.TT_DoJoinChannelByID
_DoJoinChannelByID.restype = INT32
_DoJoinChannelByID.argtypes = [_TTInstance, INT32, TTCHAR_P]
_DoLeaveChannel = dll.TT_DoLeaveChannel
_DoLeaveChannel.restype = INT32
_DoLeaveChannel.argtypes = [_TTInstance]
_DoChangeNickname = dll.TT_DoChangeNickname
_DoChangeNickname.restype = INT32
_DoChangeNickname.argtypes = [_TTInstance, TTCHAR_P]
_DoChangeStatus = dll.TT_DoChangeStatus
_DoChangeStatus.restype = INT32
_DoChangeStatus.argtypes = [_TTInstance, INT32, TTCHAR_P]
_DoTextMessage = dll.TT_DoTextMessage
_DoTextMessage.restype = INT32
_DoTextMessage.argtypes = [_TTInstance, POINTER(TextMessage)]
_DoChannelOp = dll.TT_DoChannelOp
_DoChannelOp.restype = INT32
_DoChannelOp.argtypes = [_TTInstance, INT32, INT32, BOOL]
_DoChannelOpEx = dll.TT_DoChannelOpEx
_DoChannelOpEx.restype = INT32
_DoChannelOpEx.argtypes = [_TTInstance, INT32, INT32, TTCHAR_P, BOOL]
_DoKickUser = dll.TT_DoKickUser
_DoKickUser.restype = INT32
_DoKickUser.argtypes = [_TTInstance, INT32, INT32]
_DoSendFile = dll.TT_DoSendFile
_DoSendFile.restype = INT32
_DoSendFile.argtypes = [_TTInstance, INT32, TTCHAR_P]
_DoRecvFile = dll.TT_DoRecvFile
_DoRecvFile.restype = INT32
_DoRecvFile.argtypes = [_TTInstance, INT32, INT32, TTCHAR_P]
_DoDeleteFile = dll.TT_DoDeleteFile
_DoDeleteFile.restype = INT32
_DoDeleteFile.argtypes = [_TTInstance, INT32, INT32]
_DoSubscribe = dll.TT_DoSubscribe
_DoSubscribe.restype = INT32
_DoSubscribe.argtypes = [_TTInstance, INT32, UINT32]
_DoUnsubscribe = dll.TT_DoUnsubscribe
_DoUnsubscribe.restype = INT32
_DoUnsubscribe.argtypes = [_TTInstance, INT32, UINT32]
_DoMakeChannel = dll.TT_DoMakeChannel
_DoMakeChannel.restype = INT32
_DoMakeChannel.argtypes = [_TTInstance, POINTER(Channel)]
_DoUpdateChannel = dll.TT_DoUpdateChannel
_DoUpdateChannel.restype = INT32
_DoUpdateChannel.argtypes = [_TTInstance, POINTER(Channel)]
_DoRemoveChannel = dll.TT_DoRemoveChannel
_DoRemoveChannel.restype = INT32
_DoRemoveChannel.argtypes = [_TTInstance, INT32]
_DoMoveUser = dll.TT_DoMoveUser
_DoMoveUser.restype = INT32
_DoMoveUser.argtypes = [_TTInstance, INT32, INT32]
_DoUpdateServer = dll.TT_DoUpdateServer
_DoUpdateServer.restype = INT32
_DoUpdateServer.argtypes = [_TTInstance, POINTER(ServerProperties)]
_DoListUserAccounts = dll.TT_DoListUserAccounts
_DoListUserAccounts.restype = INT32
_DoListUserAccounts.argtypes = [_TTInstance, INT32, INT32]
_DoNewUserAccount = dll.TT_DoNewUserAccount
_DoNewUserAccount.restype = INT32
_DoNewUserAccount.argtypes = [_TTInstance, POINTER(UserAccount)]
_DoDeleteUserAccount = dll.TT_DoDeleteUserAccount
_DoDeleteUserAccount.restype = INT32
_DoDeleteUserAccount.argtypes = [_TTInstance, TTCHAR_P]
_DoBanUser = dll.TT_DoBanUser
_DoBanUser.restype = INT32
_DoBanUser.argtypes = [_TTInstance, INT32, INT32]
_DoBanUserEx = dll.TT_DoBanUserEx
_DoBanUserEx.restype = INT32
_DoBanUserEx.argtypes = [_TTInstance, INT32, UINT32]
_DoBan = dll.TT_DoBan
_DoBan.restype = INT32
_DoBan.argtypes = [_TTInstance, POINTER(BannedUser)]
_DoBanIPAddress = dll.TT_DoBanIPAddress
_DoBanIPAddress.restype = INT32
_DoBanIPAddress.argtypes = [_TTInstance, TTCHAR_P, INT32]
_DoUnBanUser = dll.TT_DoUnBanUser
_DoUnBanUser.restype = INT32
_DoUnBanUser.argtypes = [_TTInstance, TTCHAR_P, INT32]
_DoUnBanUserEx = dll.TT_DoUnBanUserEx
_DoUnBanUserEx.restype = INT32
_DoUnBanUserEx.argtypes = [_TTInstance, POINTER(BannedUser)]
_DoListBans = dll.TT_DoListBans
_DoListBans.restype = INT32
_DoListBans.argtypes = [_TTInstance, INT32, INT32, INT32]
_DoSaveConfig = dll.TT_DoSaveConfig
_DoSaveConfig.restype = INT32
_DoSaveConfig.argtypes = [_TTInstance]
_DoQueryServerStats = dll.TT_DoQueryServerStats
_DoQueryServerStats.restype = INT32
_DoQueryServerStats.argtypes = [_TTInstance]
_DoQuit = dll.TT_DoQuit
_DoQuit.restype = INT32
_DoQuit.argtypes = [_TTInstance]
_GetServerProperties = dll.TT_GetServerProperties
_GetServerProperties.restype = BOOL
_GetServerProperties.argtypes = [_TTInstance, POINTER(ServerProperties)]
_GetServerUsers = dll.TT_GetServerUsers
_GetServerUsers.restype = BOOL
_GetServerUsers.argtypes = [_TTInstance, POINTER(User), POINTER(INT32)]
_GetRootChannelID = dll.TT_GetRootChannelID
_GetRootChannelID.restype = INT32
_GetRootChannelID.argtypes = [_TTInstance]
_GetMyChannelID = dll.TT_GetMyChannelID
_GetMyChannelID.restype = INT32
_GetMyChannelID.argtypes = [_TTInstance]
_GetChannel = dll.TT_GetChannel
_GetChannel.restype = BOOL
_GetChannel.argtypes = [_TTInstance, INT32, POINTER(Channel)]
_GetChannelPath = dll.TT_GetChannelPath
_GetChannelPath.restype = BOOL
_GetChannelPath.argtypes = [_TTInstance, INT32, POINTER(TTCHAR*TT_STRLEN)]
_GetChannelIDFromPath = dll.TT_GetChannelIDFromPath
_GetChannelIDFromPath.restype = INT32
_GetChannelIDFromPath.argtypes = [_TTInstance, TTCHAR_P]
_GetChannelUsers = dll.TT_GetChannelUsers
_GetChannelUsers.restype = BOOL
_GetChannelUsers.argtypes = [_TTInstance, INT32, POINTER(User), POINTER(INT32)]
_GetChannelFiles = dll.TT_GetChannelFiles
_GetChannelFiles.restype = BOOL
_GetChannelFiles.argtypes = [_TTInstance, INT32, POINTER(RemoteFile), POINTER(INT32)]
_GetChannelFile = dll.TT_GetChannelFile
_GetChannelFile.restype = BOOL
_GetChannelFile.argtypes = [_TTInstance, INT32, INT32, POINTER(RemoteFile)]
_IsChannelOperator = dll.TT_IsChannelOperator
_IsChannelOperator.restype = BOOL
_IsChannelOperator.argtypes = [_TTInstance, INT32, INT32]
_GetServerChannels = dll.TT_GetServerChannels
_GetServerChannels.restype = BOOL
_GetServerChannels.argtypes = [_TTInstance, POINTER(Channel), POINTER(INT32)]
_GetMyUserID = dll.TT_GetMyUserID
_GetMyUserID.restype = INT32
_GetMyUserID.argtypes = [_TTInstance]
_GetMyUserAccount = dll.TT_GetMyUserAccount
_GetMyUserAccount.restype = BOOL
_GetMyUserAccount.argtypes = [_TTInstance, POINTER(UserAccount)]
_GetMyUserData = dll.TT_GetMyUserData
_GetMyUserData.restype = INT32
_GetMyUserData.argtypes = [_TTInstance]
_GetUser = dll.TT_GetUser
_GetUser.restype = BOOL
_GetUser.argtypes = [_TTInstance, INT32, POINTER(User)]
_GetUserStatistics = dll.TT_GetUserStatistics
_GetUserStatistics.restype = BOOL
_GetUserStatistics.argtypes = [_TTInstance, INT32, POINTER(UserStatistics)]
_GetUserByUsername = dll.TT_GetUserByUsername
_GetUserByUsername.restype = BOOL
_GetUserByUsername.argtypes = [_TTInstance, TTCHAR_P, POINTER(User)]
_SetUserVolume = dll.TT_SetUserVolume
_SetUserVolume.restype = BOOL
_SetUserVolume.argtypes = [_TTInstance, INT32, INT32, INT32]
_SetUserMute = dll.TT_SetUserMute
_SetUserMute.restype = BOOL
_SetUserMute.argtypes = [_TTInstance, INT32, INT32, BOOL]
_SetUserStoppedPlaybackDelay = dll.TT_SetUserStoppedPlaybackDelay
_SetUserStoppedPlaybackDelay.restype = BOOL
_SetUserStoppedPlaybackDelay.argtypes = [_TTInstance, INT32, INT32, INT32]
_SetUserPosition = dll.TT_SetUserPosition
_SetUserPosition.restype = BOOL
_SetUserPosition.argtypes = [_TTInstance, INT32, INT32, c_float, c_float, c_float]
_SetUserStereo = dll.TT_SetUserStereo
_SetUserStereo.restype = BOOL
_SetUserStereo.argtypes = [_TTInstance, INT32, INT32, BOOL, BOOL]
_SetUserMediaStorageDir = dll.TT_SetUserMediaStorageDir
_SetUserMediaStorageDir.restype = BOOL
_SetUserMediaStorageDir.argtypes = [_TTInstance, INT32, TTCHAR_P, TTCHAR_P, UINT32]
_SetUserAudioStreamBufferSize = dll.TT_SetUserAudioStreamBufferSize
_SetUserAudioStreamBufferSize.restype = BOOL
_SetUserAudioStreamBufferSize.argtypes = [_TTInstance, INT32, UINT32, INT32]
_GetFileTransferInfo = dll.TT_GetFileTransferInfo
_GetFileTransferInfo.restype = BOOL
_GetFileTransferInfo.argtypes = [_TTInstance, INT32, POINTER(FileTransfer)]
_CancelFileTransfer = dll.TT_CancelFileTransfer
_CancelFileTransfer.restype = BOOL
_CancelFileTransfer.argtypes = [_TTInstance, INT32]
_GetErrorMessage = dll.TT_GetErrorMessage
_GetErrorMessage.restype = c_void_p
_GetErrorMessage.argtypes = [INT32, POINTER(TTCHAR*TT_STRLEN)]
_DBG_SIZEOF = dll.TT_DBG_SIZEOF
_DBG_SIZEOF.restype = INT32
_DBG_SIZEOF.argtypes = [TTType]

# main code

def getVersion():
    return _GetVersion()

def setLicense(name, key):
    return _SetLicenseInformation(name, key)

def DBG_SIZEOF(t):
    return _DBG_SIZEOF(t)

class TeamTalkError(Exception):
    pass

class TeamTalk(object):

    def __init__(self):
        self._tt = _InitTeamTalkPoll()
        if not self._tt:
            raise TeamTalkError("failed to initialize")

    def closeTeamTalk(self):
        return _CloseTeamTalk(self._tt)

    def __del__(self):
        self.closeTeamTalk()

    def getMessage(self, nWaitMS=-1):
        msg = TTMessage()
        nWaitMS = INT32(nWaitMS)
        _GetMessage(self._tt, byref(msg), byref(nWaitMS))
        return msg

    def getFlags(self):
        return _GetFlags(self._tt)

    def getDefaultSoundDevices(self):
        indevid = INT32()
        outdevid = INT32()
        _GetDefaultSoundDevices(byref(indevid), byref(outdevid))
        return indevid, outdevid

    def getSoundDevices(self):
        count = c_int()
        _GetSoundDevices(None, byref(count))
        soundDevs = (SoundDevice*count.value)()
        _GetSoundDevices(soundDevs, byref(count))
        return soundDevs

    def initSoundInputDevice(self, indev):
        return _InitSoundInputDevice(self._tt, indev)

    def initSoundOutputDevice(self, outdev):
        return _InitSoundOutputDevice(self._tt, outdev)

    def enableVoiceTransmission(self, bEnable):
        return _EnableVoiceTransmission(self._tt, bEnable)

    def connect(self, szHostAddress, nTcpPort, nUdpPort, nLocalTcpPort=0, nLocalUdpPort=0, bEncrypted=False):
        return _Connect(self._tt, szHostAddress, nTcpPort, nUdpPort, nLocalTcpPort, nLocalUdpPort, bEncrypted)

    def disconnect(self):
        return _Disconnect(self._tt)

    def doPing(self):
        return _DoPing(self._tt)

    def doLogin(self, szNickname, szUsername, szPassword, szClientname):
        return _DoLoginEx(self._tt, szNickname, szUsername, szPassword, szClientname)

    def doLogout(self):
        return _DoLogout(self._tt)

    def doJoinChannelByID(self, nChannelID, szPassword):
        return _DoJoinChannelByID(self._tt, nChannelID, szPassword)

    def doLeaveChannel(self):
        return _DoLeaveChannel(self._tt)

    def doChangeNickname(self, szNewNick):
        return _DoChangeNickname(self._tt, szNewNick)

    def doChangeStatus(self, nStatusMode, szStatusMessage):
        return _DoChangeStatus(self._tt, nStatusMode, szStatusMessage)

    def doTextMessage(self, msg):
        return _DoTextMessage(self._tt, msg)

    def getServerProperties(self):
        srvprops = ServerProperties()
        _GetServerProperties(self._tt, srvprops)
        return srvprops

    def getServerUsers(self):
        count = c_int()
        _GetServerUsers(self._tt, None, byref(count))
        users = (User*count.value)()
        _GetServerUsers(self._tt, users, byref(count))
        return users

    def getRootChannelID(self):
        return _GetRootChannelID(self._tt)

    def getMyChannelID(self):
        return _GetMyChannelID(self._tt)

    def getChannel(self, nChannelID):
        channel= Channel()
        _GetChannel(self._tt, nChannelID, channel)
        return channel

    def getChannelPath(self, nChannelID):
        szChannelPath = (TTCHAR*TT_STRLEN)()
        _GetChannelPath(self._tt, nChannelID, szChannelPath)
        return szChannelPath.value

    def getChannelIDFromPath(self, szChannelPath):
        return _GetChannelIDFromPath(self._tt, szChannelPath)

    def getChannelUsers(self, nChannelID):
        count = c_int()
        _GetChannelUsers(self._tt, nChannelID, None, byref(count))
        users = (User*count.value)()
        _GetChannelUsers(self._tt, nChannelID, users, byref(count))
        return users

    def getChannelFiles(self, nChannelID):
        count = c_int()
        _GetChannelFiles(self._tt, nChannelID, None, byref(count))
        files = (RemoteFile*count.value)()
        _GetChannelFiles(self._tt, nChannelID, files, byref(count))
        return files

    def getServerChannels(self):
        count = c_int()
        _GetServerChannels(self._tt, None, byref(count))
        channels = (Channel*count.value)()
        _GetServerChannels(self._tt, channels, byref(count))
        return channels

    def getMyUserID(self):
        return _GetMyUserID(self._tt)

    def getMyUserAccount(self):
        account = UserAccount()
        _GetMyUserAccount(self._tt, account)
        return account

    def getMyUserData(self):
        return _GetMyUserData(self._tt)

    def getUser(self, nUserID):
        user = User()
        _GetUser(self._tt, nUserID, user)
        return user

    def getUserStatistics(self, nUserID):
        stats = UserStatistics()
        _GetUserStatistics(self._tt, nUserID, stats)
        return stats

    def getUserByUsername(self, szUsername):
        user = User()
        _GetUserByUsername(self._tt, szUsername, user)
        return user

    def getErrorMessage(self, nError):
        szErrorMsg = (TTCHAR*TT_STRLEN)()
        _GetErrorMessage(nError, szErrorMsg)
        return szErrorMsg.value
