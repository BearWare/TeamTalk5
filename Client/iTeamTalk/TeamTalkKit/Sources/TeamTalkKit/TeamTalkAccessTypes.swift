import Foundation
import TeamTalkC

/// Which platform audio backend to use for `TeamTalkSession.defaultSoundDevices(for:)`.
/// Most cases are platform-specific; only ``coreAudio``/``audioUnit`` are
/// relevant on iOS/macOS.
public struct TeamTalkSoundSystem: RawRepresentable, Hashable, Sendable {
    public let rawValue: SoundSystem

    public init(rawValue: SoundSystem) {
        self.rawValue = rawValue
    }

    public init(cValue: SoundSystem) {
        self.init(rawValue: cValue)
    }

    public var cValue: SoundSystem {
        rawValue
    }

    public static let none = TeamTalkSoundSystem(rawValue: SOUNDSYSTEM_NONE)
    public static let winMM = TeamTalkSoundSystem(rawValue: SOUNDSYSTEM_WINMM)
    public static let directSound = TeamTalkSoundSystem(rawValue: SOUNDSYSTEM_DSOUND)
    public static let alsa = TeamTalkSoundSystem(rawValue: SOUNDSYSTEM_ALSA)
    public static let coreAudio = TeamTalkSoundSystem(rawValue: SOUNDSYSTEM_COREAUDIO)
    public static let wasapi = TeamTalkSoundSystem(rawValue: SOUNDSYSTEM_WASAPI)
    public static let openSLESAndroid = TeamTalkSoundSystem(rawValue: SOUNDSYSTEM_OPENSLES_ANDROID)
    public static let audioUnit = TeamTalkSoundSystem(rawValue: SOUNDSYSTEM_AUDIOUNIT)
    public static let audioUnitIOS = TeamTalkSoundSystem.audioUnit
    public static let pulseAudio = TeamTalkSoundSystem(rawValue: SOUNDSYSTEM_PULSEAUDIO)
}

/// DSP capabilities a ``TeamTalkSoundDevice`` supports or currently has
/// enabled (see ``TeamTalkSoundDeviceEffects/enabledEffects``).
public struct TeamTalkSoundDeviceFeatures: OptionSet, Hashable, Sendable {
    public let rawValue: UInt32

    public init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    public init(cValue: SoundDeviceFeatures) {
        self.init(rawValue: cValue)
    }

    public var cValue: SoundDeviceFeatures {
        rawValue
    }

    public static let none = TeamTalkSoundDeviceFeatures(rawValue: SOUNDDEVICEFEATURE_NONE.rawValue)
    public static let echoCancellation = TeamTalkSoundDeviceFeatures(rawValue: SOUNDDEVICEFEATURE_AEC.rawValue)
    public static let automaticGainControl = TeamTalkSoundDeviceFeatures(rawValue: SOUNDDEVICEFEATURE_AGC.rawValue)
    public static let denoise = TeamTalkSoundDeviceFeatures(rawValue: SOUNDDEVICEFEATURE_DENOISE.rawValue)
    public static let threeDPosition = TeamTalkSoundDeviceFeatures(rawValue: SOUNDDEVICEFEATURE_3DPOSITION.rawValue)
    public static let duplexMode = TeamTalkSoundDeviceFeatures(rawValue: SOUNDDEVICEFEATURE_DUPLEXMODE.rawValue)
    public static let defaultCommunicationDevice = TeamTalkSoundDeviceFeatures(rawValue: SOUNDDEVICEFEATURE_DEFAULTCOMDEVICE.rawValue)
}

/// Identifies a sound input/output device. Unlike most IDs in this package,
/// this one packs flags into the raw value (``isShared``,
/// ``physicalDeviceID``) rather than being an opaque integer, and includes
/// well-known constants below for platform-specific virtual devices.
public struct TeamTalkSoundDeviceID: RawRepresentable, Hashable, Sendable, CustomStringConvertible {
    public let rawValue: Int32

    public init(rawValue: Int32) {
        self.rawValue = rawValue
    }

    public init(_ rawValue: Int32) {
        self.init(rawValue: rawValue)
    }

    public init(cValue: Int32) {
        self.init(rawValue: cValue)
    }

    public var cValue: Int32 {
        rawValue
    }

    public var isShared: Bool {
        (rawValue & Int32(TT_SOUNDDEVICE_ID_SHARED_FLAG)) != 0
    }

    public var physicalDeviceID: TeamTalkSoundDeviceID {
        TeamTalkSoundDeviceID(rawValue & Int32(TT_SOUNDDEVICE_ID_MASK))
    }

    public var description: String {
        String(rawValue)
    }

    /// iOS's Audio Unit Remote I/O device, without built-in voice processing.
    public static let remoteIO = TeamTalkSoundDeviceID(TT_SOUNDDEVICE_ID_REMOTEIO)
    /// iOS's Audio Unit Voice-Processing I/O device: Remote I/O plus the
    /// system's own echo cancellation/AGC.
    public static let voiceProcessingIO = TeamTalkSoundDeviceID(TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO)
    public static let openSLESDefault = TeamTalkSoundDeviceID(TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT)
    public static let openSLESVoiceCommunication = TeamTalkSoundDeviceID(TT_SOUNDDEVICE_ID_OPENSLES_VOICECOM)
    public static let teamTalkVirtual = TeamTalkSoundDeviceID(TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL)
}

/// A command or internal SDK error code, carried by ``TeamTalkClientError``.
/// Three bands: ``success`` (0), command errors (1000–9999, rejected by the
/// server — see ``isCommandError``), and internal errors (10000+, local SDK
/// failures — see ``isInternalError``).
public struct TeamTalkErrorCode: RawRepresentable, Hashable, Sendable, ExpressibleByIntegerLiteral, CustomStringConvertible {
    public let rawValue: Int32

    public init(rawValue: Int32) {
        self.rawValue = rawValue
    }

    public init(_ rawValue: Int32) {
        self.init(rawValue: rawValue)
    }

    public init(cValue: ClientError) {
        self.init(rawValue: Int32(cValue.rawValue))
    }

    public init(integerLiteral value: Int32) {
        self.init(rawValue: value)
    }

    public var cValue: Int32 {
        rawValue
    }

    public var isSuccess: Bool {
        rawValue == TeamTalkErrorCode.success.rawValue
    }

    public var isCommandError: Bool {
        (1000..<10000).contains(rawValue)
    }

    public var isInternalError: Bool {
        rawValue >= 10000
    }

    public var description: String {
        String(rawValue)
    }

    public static let success = TeamTalkErrorCode(cValue: CMDERR_SUCCESS)
    public static let syntaxError = TeamTalkErrorCode(cValue: CMDERR_SYNTAX_ERROR)
    public static let unknownCommand = TeamTalkErrorCode(cValue: CMDERR_UNKNOWN_COMMAND)
    public static let missingParameter = TeamTalkErrorCode(cValue: CMDERR_MISSING_PARAMETER)
    public static let incompatibleProtocols = TeamTalkErrorCode(cValue: CMDERR_INCOMPATIBLE_PROTOCOLS)
    public static let unknownAudioCodec = TeamTalkErrorCode(cValue: CMDERR_UNKNOWN_AUDIOCODEC)
    public static let invalidUsername = TeamTalkErrorCode(cValue: CMDERR_INVALID_USERNAME)
    public static let incorrectChannelPassword = TeamTalkErrorCode(cValue: CMDERR_INCORRECT_CHANNEL_PASSWORD)
    public static let invalidAccount = TeamTalkErrorCode(cValue: CMDERR_INVALID_ACCOUNT)
    public static let maxServerUsersExceeded = TeamTalkErrorCode(cValue: CMDERR_MAX_SERVER_USERS_EXCEEDED)
    public static let maxChannelUsersExceeded = TeamTalkErrorCode(cValue: CMDERR_MAX_CHANNEL_USERS_EXCEEDED)
    public static let serverBanned = TeamTalkErrorCode(cValue: CMDERR_SERVER_BANNED)
    public static let notAuthorized = TeamTalkErrorCode(cValue: CMDERR_NOT_AUTHORIZED)
    public static let maxDiskUsageExceeded = TeamTalkErrorCode(cValue: CMDERR_MAX_DISKUSAGE_EXCEEDED)
    public static let incorrectOperatorPassword = TeamTalkErrorCode(cValue: CMDERR_INCORRECT_OP_PASSWORD)
    public static let audioCodecBitrateLimitExceeded = TeamTalkErrorCode(cValue: CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED)
    public static let maxLoginsPerIPAddressExceeded = TeamTalkErrorCode(cValue: CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED)
    public static let maxChannelsExceeded = TeamTalkErrorCode(cValue: CMDERR_MAX_CHANNELS_EXCEEDED)
    public static let commandFlood = TeamTalkErrorCode(cValue: CMDERR_COMMAND_FLOOD)
    public static let channelBanned = TeamTalkErrorCode(cValue: CMDERR_CHANNEL_BANNED)
    public static let maxFileTransfersExceeded = TeamTalkErrorCode(cValue: CMDERR_MAX_FILETRANSFERS_EXCEEDED)
    public static let notLoggedIn = TeamTalkErrorCode(cValue: CMDERR_NOT_LOGGEDIN)
    public static let alreadyLoggedIn = TeamTalkErrorCode(cValue: CMDERR_ALREADY_LOGGEDIN)
    public static let notInChannel = TeamTalkErrorCode(cValue: CMDERR_NOT_IN_CHANNEL)
    public static let alreadyInChannel = TeamTalkErrorCode(cValue: CMDERR_ALREADY_IN_CHANNEL)
    public static let channelAlreadyExists = TeamTalkErrorCode(cValue: CMDERR_CHANNEL_ALREADY_EXISTS)
    public static let channelNotFound = TeamTalkErrorCode(cValue: CMDERR_CHANNEL_NOT_FOUND)
    public static let userNotFound = TeamTalkErrorCode(cValue: CMDERR_USER_NOT_FOUND)
    public static let banNotFound = TeamTalkErrorCode(cValue: CMDERR_BAN_NOT_FOUND)
    public static let fileTransferNotFound = TeamTalkErrorCode(cValue: CMDERR_FILETRANSFER_NOT_FOUND)
    public static let openFileFailed = TeamTalkErrorCode(cValue: CMDERR_OPENFILE_FAILED)
    public static let accountNotFound = TeamTalkErrorCode(cValue: CMDERR_ACCOUNT_NOT_FOUND)
    public static let fileNotFound = TeamTalkErrorCode(cValue: CMDERR_FILE_NOT_FOUND)
    public static let fileAlreadyExists = TeamTalkErrorCode(cValue: CMDERR_FILE_ALREADY_EXISTS)
    public static let fileSharingDisabled = TeamTalkErrorCode(cValue: CMDERR_FILESHARING_DISABLED)
    public static let channelHasUsers = TeamTalkErrorCode(cValue: CMDERR_CHANNEL_HAS_USERS)
    public static let loginServiceUnavailable = TeamTalkErrorCode(cValue: CMDERR_LOGINSERVICE_UNAVAILABLE)
    public static let channelCannotBeHidden = TeamTalkErrorCode(cValue: CMDERR_CHANNEL_CANNOT_BE_HIDDEN)
    public static let soundInputFailure = TeamTalkErrorCode(cValue: INTERR_SNDINPUT_FAILURE)
    public static let soundOutputFailure = TeamTalkErrorCode(cValue: INTERR_SNDOUTPUT_FAILURE)
    public static let audioCodecInitializationFailed = TeamTalkErrorCode(cValue: INTERR_AUDIOCODEC_INIT_FAILED)
    public static let speexDSPInitializationFailed = TeamTalkErrorCode(cValue: INTERR_SPEEXDSP_INIT_FAILED)
    public static let audioPreprocessorInitializationFailed = TeamTalkErrorCode(cValue: INTERR_AUDIOPREPROCESSOR_INIT_FAILED)
    public static let messageQueueOverflow = TeamTalkErrorCode(cValue: INTERR_TTMESSAGE_QUEUE_OVERFLOW)
    public static let soundEffectFailure = TeamTalkErrorCode(cValue: INTERR_SNDEFFECT_FAILURE)
}

/// Permissions granted to a ``TeamTalkUserAccount``, checked via
/// `TeamTalkSession.myRights`/`getUserRight(_:)`.
public struct TeamTalkUserRights: OptionSet, Hashable, Sendable {
    public let rawValue: UInt32

    public init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    public init(cValue: UserRights) {
        self.init(rawValue: cValue)
    }

    public var cValue: UserRights {
        rawValue
    }

    public static let none = TeamTalkUserRights(rawValue: USERRIGHT_NONE.rawValue)
    public static let multipleLogins = TeamTalkUserRights(rawValue: USERRIGHT_MULTI_LOGIN.rawValue)
    public static let canViewAllUsers = TeamTalkUserRights(rawValue: USERRIGHT_VIEW_ALL_USERS.rawValue)
    public static let canCreateTemporaryChannels = TeamTalkUserRights(rawValue: USERRIGHT_CREATE_TEMPORARY_CHANNEL.rawValue)
    public static let canModifyChannels = TeamTalkUserRights(rawValue: USERRIGHT_MODIFY_CHANNELS.rawValue)
    public static let canBroadcastTextMessages = TeamTalkUserRights(rawValue: USERRIGHT_TEXTMESSAGE_BROADCAST.rawValue)
    public static let canKickUsers = TeamTalkUserRights(rawValue: USERRIGHT_KICK_USERS.rawValue)
    public static let canBanUsers = TeamTalkUserRights(rawValue: USERRIGHT_BAN_USERS.rawValue)
    public static let canMoveUsers = TeamTalkUserRights(rawValue: USERRIGHT_MOVE_USERS.rawValue)
    public static let canEnableOperators = TeamTalkUserRights(rawValue: USERRIGHT_OPERATOR_ENABLE.rawValue)
    public static let canUploadFiles = TeamTalkUserRights(rawValue: USERRIGHT_UPLOAD_FILES.rawValue)
    public static let canDownloadFiles = TeamTalkUserRights(rawValue: USERRIGHT_DOWNLOAD_FILES.rawValue)
    public static let canUpdateServerProperties = TeamTalkUserRights(rawValue: USERRIGHT_UPDATE_SERVERPROPERTIES.rawValue)
    public static let canTransmitVoice = TeamTalkUserRights(rawValue: USERRIGHT_TRANSMIT_VOICE.rawValue)
    public static let canTransmitVideoCapture = TeamTalkUserRights(rawValue: USERRIGHT_TRANSMIT_VIDEOCAPTURE.rawValue)
    public static let canTransmitDesktop = TeamTalkUserRights(rawValue: USERRIGHT_TRANSMIT_DESKTOP.rawValue)
    public static let canTransmitDesktopInput = TeamTalkUserRights(rawValue: USERRIGHT_TRANSMIT_DESKTOPINPUT.rawValue)
    public static let canTransmitMediaFileAudio = TeamTalkUserRights(rawValue: USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO.rawValue)
    public static let canTransmitMediaFileVideo = TeamTalkUserRights(rawValue: USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO.rawValue)
    public static let canTransmitMediaFiles = TeamTalkUserRights(rawValue: USERRIGHT_TRANSMIT_MEDIAFILE.rawValue)
    /// Unlike the other cases, this is a restriction, not a permission: the
    /// account may not change its own nickname.
    public static let lockedNickname = TeamTalkUserRights(rawValue: USERRIGHT_LOCKED_NICKNAME.rawValue)
    /// Restriction: the account may not change its own status mode/message.
    public static let lockedStatus = TeamTalkUserRights(rawValue: USERRIGHT_LOCKED_STATUS.rawValue)
    public static let canRecordVoice = TeamTalkUserRights(rawValue: USERRIGHT_RECORD_VOICE.rawValue)
    public static let canViewHiddenChannels = TeamTalkUserRights(rawValue: USERRIGHT_VIEW_HIDDEN_CHANNELS.rawValue)
    public static let canSendPrivateTextMessages = TeamTalkUserRights(rawValue: USERRIGHT_TEXTMESSAGE_USER.rawValue)
    public static let canSendChannelTextMessages = TeamTalkUserRights(rawValue: USERRIGHT_TEXTMESSAGE_CHANNEL.rawValue)
}

/// Whether an account is a regular user or an administrator (who implicitly
/// holds every ``TeamTalkUserRights`` regardless of what's explicitly granted).
public struct TeamTalkUserTypes: OptionSet, Hashable, Sendable {
    public let rawValue: UInt32

    public init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    public init(cValue: UserTypes) {
        self.init(rawValue: cValue)
    }

    public var cValue: UserTypes {
        rawValue
    }

    public static let none = TeamTalkUserTypes(rawValue: USERTYPE_NONE.rawValue)
    public static let defaultUser = TeamTalkUserTypes(rawValue: USERTYPE_DEFAULT.rawValue)
    public static let administrator = TeamTalkUserTypes(rawValue: USERTYPE_ADMIN.rawValue)
}

/// What one client receives from another — set per-peer via
/// `TeamTalkSession.subscribe(_:to:)`/`unsubscribe(_:from:)`, independent of
/// whether the peer is actually transmitting that stream type.
public struct TeamTalkSubscriptions: OptionSet, Hashable, Sendable {
    public let rawValue: UInt32

    public init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    public init(cValue: Subscriptions) {
        self.init(rawValue: cValue)
    }

    public var cValue: Subscriptions {
        rawValue
    }

    public static let none = TeamTalkSubscriptions(rawValue: SUBSCRIBE_NONE.rawValue)
    public static let userMessages = TeamTalkSubscriptions(rawValue: SUBSCRIBE_USER_MSG.rawValue)
    public static let channelMessages = TeamTalkSubscriptions(rawValue: SUBSCRIBE_CHANNEL_MSG.rawValue)
    public static let broadcastMessages = TeamTalkSubscriptions(rawValue: SUBSCRIBE_BROADCAST_MSG.rawValue)
    public static let customMessages = TeamTalkSubscriptions(rawValue: SUBSCRIBE_CUSTOM_MSG.rawValue)
    public static let voice = TeamTalkSubscriptions(rawValue: SUBSCRIBE_VOICE.rawValue)
    public static let videoCapture = TeamTalkSubscriptions(rawValue: SUBSCRIBE_VIDEOCAPTURE.rawValue)
    public static let desktop = TeamTalkSubscriptions(rawValue: SUBSCRIBE_DESKTOP.rawValue)
    public static let desktopInput = TeamTalkSubscriptions(rawValue: SUBSCRIBE_DESKTOPINPUT.rawValue)
    public static let mediaFile = TeamTalkSubscriptions(rawValue: SUBSCRIBE_MEDIAFILE.rawValue)
    /// The `intercept*` cases below mirror the plain ones above but for
    /// silently receiving a stream meant for someone else — e.g. an admin
    /// tool auditing private messages — rather than being an ordinary recipient.
    public static let interceptUserMessages = TeamTalkSubscriptions(rawValue: SUBSCRIBE_INTERCEPT_USER_MSG.rawValue)
    public static let interceptChannelMessages = TeamTalkSubscriptions(rawValue: SUBSCRIBE_INTERCEPT_CHANNEL_MSG.rawValue)
    public static let interceptCustomMessages = TeamTalkSubscriptions(rawValue: SUBSCRIBE_INTERCEPT_CUSTOM_MSG.rawValue)
    public static let interceptVoice = TeamTalkSubscriptions(rawValue: SUBSCRIBE_INTERCEPT_VOICE.rawValue)
    public static let interceptVideoCapture = TeamTalkSubscriptions(rawValue: SUBSCRIBE_INTERCEPT_VIDEOCAPTURE.rawValue)
    public static let interceptDesktop = TeamTalkSubscriptions(rawValue: SUBSCRIBE_INTERCEPT_DESKTOP.rawValue)
    public static let interceptMediaFile = TeamTalkSubscriptions(rawValue: SUBSCRIBE_INTERCEPT_MEDIAFILE.rawValue)
}

/// What a user is currently doing (talking, sharing desktop, ...). The
/// `*Muted` cases reflect this *client's own* local mute setting for that
/// user (see `TeamTalkSession.setUserMute(_:stream:muted:)`), not whether
/// the user has muted themselves.
public struct TeamTalkUserStates: OptionSet, Hashable, Sendable {
    public let rawValue: UInt32

    public init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    public init(cValue: UserStates) {
        self.init(rawValue: cValue)
    }

    public var cValue: UserStates {
        rawValue
    }

    public static let none = TeamTalkUserStates(rawValue: USERSTATE_NONE.rawValue)
    public static let voice = TeamTalkUserStates(rawValue: USERSTATE_VOICE.rawValue)
    public static let voiceMuted = TeamTalkUserStates(rawValue: USERSTATE_MUTE_VOICE.rawValue)
    public static let mediaFileMuted = TeamTalkUserStates(rawValue: USERSTATE_MUTE_MEDIAFILE.rawValue)
    public static let desktop = TeamTalkUserStates(rawValue: USERSTATE_DESKTOP.rawValue)
    public static let videoCapture = TeamTalkUserStates(rawValue: USERSTATE_VIDEOCAPTURE.rawValue)
    public static let mediaFileAudio = TeamTalkUserStates(rawValue: USERSTATE_MEDIAFILE_AUDIO.rawValue)
    public static let mediaFileVideo = TeamTalkUserStates(rawValue: USERSTATE_MEDIAFILE_VIDEO.rawValue)
    public static let mediaFile = TeamTalkUserStates(rawValue: USERSTATE_MEDIAFILE.rawValue)
}

/// Which kind(s) of media/data stream an operation applies to — used both
/// for querying/muting a specific stream and, combined, for audio block
/// subscriptions and muxed recording.
public struct TeamTalkStreamTypes: OptionSet, Hashable, Sendable {
    public let rawValue: UInt32

    public init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    public init(cValue: StreamTypes) {
        self.init(rawValue: cValue)
    }

    public init(cValue: StreamType) {
        self.init(rawValue: cValue.rawValue)
    }

    public var cValue: StreamType {
        StreamType(rawValue: rawValue)
    }

    public var cMask: StreamTypes {
        rawValue
    }

    public static let none = TeamTalkStreamTypes(rawValue: STREAMTYPE_NONE.rawValue)
    public static let voice = TeamTalkStreamTypes(rawValue: STREAMTYPE_VOICE.rawValue)
    public static let videoCapture = TeamTalkStreamTypes(rawValue: STREAMTYPE_VIDEOCAPTURE.rawValue)
    public static let mediaFileAudio = TeamTalkStreamTypes(rawValue: STREAMTYPE_MEDIAFILE_AUDIO.rawValue)
    public static let mediaFileVideo = TeamTalkStreamTypes(rawValue: STREAMTYPE_MEDIAFILE_VIDEO.rawValue)
    public static let desktop = TeamTalkStreamTypes(rawValue: STREAMTYPE_DESKTOP.rawValue)
    public static let desktopInput = TeamTalkStreamTypes(rawValue: STREAMTYPE_DESKTOPINPUT.rawValue)
    public static let mediaFile = TeamTalkStreamTypes(rawValue: STREAMTYPE_MEDIAFILE.rawValue)
    public static let channelMessage = TeamTalkStreamTypes(rawValue: STREAMTYPE_CHANNELMSG.rawValue)
    public static let localMediaPlaybackAudio = TeamTalkStreamTypes(rawValue: STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO.rawValue)
    /// Every stream type relevant to a classroom-mode channel, combined.
    public static let classroomAll = TeamTalkStreamTypes(rawValue: STREAMTYPE_CLASSROOM_ALL.rawValue)
}

/// Behavioral flags for a channel, set via ``TeamTalkChannelConfiguration/types``.
public struct TeamTalkChannelTypes: OptionSet, Hashable, Sendable {
    public let rawValue: UInt32

    public init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    public init(cValue: ChannelTypes) {
        self.init(rawValue: cValue)
    }

    public var cValue: ChannelTypes {
        rawValue
    }

    public static let `default` = TeamTalkChannelTypes(rawValue: CHANNEL_DEFAULT.rawValue)
    /// Survives even when empty, instead of being deleted once the last
    /// user leaves.
    public static let permanent = TeamTalkChannelTypes(rawValue: CHANNEL_PERMANENT.rawValue)
    /// "No Interruptions" mode: only one user (or the front of
    /// ``TeamTalkChannel/transmitUsersQueue``) may transmit at a time.
    public static let soloTransmit = TeamTalkChannelTypes(rawValue: CHANNEL_SOLO_TRANSMIT.rawValue)
    /// Only the channel operator can be heard by everyone by default; other
    /// users' voice is limited to those the operator specifically enables.
    public static let classroom = TeamTalkChannelTypes(rawValue: CHANNEL_CLASSROOM.rawValue)
    /// The channel operator only receives audio/video, without being able
    /// to transmit into it themselves.
    public static let operatorReceiveOnly = TeamTalkChannelTypes(rawValue: CHANNEL_OPERATOR_RECVONLY.rawValue)
    public static let noVoiceActivation = TeamTalkChannelTypes(rawValue: CHANNEL_NO_VOICEACTIVATION.rawValue)
    public static let noRecording = TeamTalkChannelTypes(rawValue: CHANNEL_NO_RECORDING.rawValue)
    /// Not shown to users without the "view hidden channels" right.
    public static let hidden = TeamTalkChannelTypes(rawValue: CHANNEL_HIDDEN.rawValue)
}

/// Which categories of activity the server writes to its own log file. Set
/// via ``TeamTalkServerPropertiesConfiguration/logEvents``.
public struct TeamTalkServerLogEvents: OptionSet, Hashable, Sendable {
    public let rawValue: UInt32

    public init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    public init(cValue: ServerLogEvents) {
        self.init(rawValue: cValue)
    }

    public var cValue: ServerLogEvents {
        rawValue
    }

    public static let none = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_NONE.rawValue)
    public static let userConnected = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_CONNECTED.rawValue)
    public static let userDisconnected = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_DISCONNECTED.rawValue)
    public static let userLoggedIn = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_LOGGEDIN.rawValue)
    public static let userLoggedOut = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_LOGGEDOUT.rawValue)
    public static let userLoginFailed = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_LOGINFAILED.rawValue)
    public static let userTimedOut = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_TIMEDOUT.rawValue)
    public static let userKicked = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_KICKED.rawValue)
    public static let userBanned = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_BANNED.rawValue)
    public static let userUnbanned = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_UNBANNED.rawValue)
    public static let userUpdated = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_UPDATED.rawValue)
    public static let userJoinedChannel = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_JOINEDCHANNEL.rawValue)
    public static let userLeftChannel = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_LEFTCHANNEL.rawValue)
    public static let userMoved = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_MOVED.rawValue)
    public static let userPrivateTextMessage = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_TEXTMESSAGE_PRIVATE.rawValue)
    public static let userCustomTextMessage = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_TEXTMESSAGE_CUSTOM.rawValue)
    public static let userChannelTextMessage = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_TEXTMESSAGE_CHANNEL.rawValue)
    public static let userBroadcastTextMessage = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_TEXTMESSAGE_BROADCAST.rawValue)
    public static let channelCreated = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_CHANNEL_CREATED.rawValue)
    public static let channelUpdated = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_CHANNEL_UPDATED.rawValue)
    public static let channelRemoved = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_CHANNEL_REMOVED.rawValue)
    public static let fileUploaded = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_FILE_UPLOADED.rawValue)
    public static let fileDownloaded = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_FILE_DOWNLOADED.rawValue)
    public static let fileDeleted = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_FILE_DELETED.rawValue)
    public static let serverUpdated = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_SERVER_UPDATED.rawValue)
    public static let serverSavedConfig = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_SERVER_SAVECONFIG.rawValue)
    public static let userEncryptionError = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_CRYPTERROR.rawValue)
    public static let userNewStream = TeamTalkServerLogEvents(rawValue: SERVERLOGEVENT_USER_NEW_STREAM.rawValue)
}

/// What a ``TeamTalkBanConfiguration``/``TeamTalkBannedUser`` matches
/// against: a channel, an IP address, and/or a username, combinable.
public struct TeamTalkBanTypes: OptionSet, Hashable, Sendable {
    public let rawValue: UInt32

    public init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    public init(cValue: BanTypes) {
        self.init(rawValue: cValue)
    }

    public var cValue: BanTypes {
        rawValue
    }

    public static let none = TeamTalkBanTypes(rawValue: BANTYPE_NONE.rawValue)
    public static let channel = TeamTalkBanTypes(rawValue: BANTYPE_CHANNEL.rawValue)
    public static let ipAddress = TeamTalkBanTypes(rawValue: BANTYPE_IPADDR.rawValue)
    public static let username = TeamTalkBanTypes(rawValue: BANTYPE_USERNAME.rawValue)
}

/// A user's presence, packed as a low-byte "mode" (``available``/``away``/
/// ``question`` — mutually exclusive, despite this being an `OptionSet`)
/// plus independent high-byte flags (``female``, ``videoTransmit``, ...)
/// that combine with whichever mode is set. ``modeMask``/``flagsMask``
/// isolate one half from a combined value; they aren't states to set
/// themselves.
public struct TeamTalkStatusMode: OptionSet, Hashable, Sendable {
    public let rawValue: Int32

    public init(rawValue: Int32) {
        self.rawValue = rawValue
    }

    public static let available: TeamTalkStatusMode = []
    public static let away = TeamTalkStatusMode(rawValue: 0x00000001)
    public static let question = TeamTalkStatusMode(rawValue: 0x00000002)
    /// Isolates the mode bits from a combined value, e.g. `status.intersection(.modeMask)`.
    public static let modeMask = TeamTalkStatusMode(rawValue: 0x000000FF)
    /// Isolates the flag bits from a combined value.
    public static let flagsMask = TeamTalkStatusMode(rawValue: Int32(bitPattern: 0xFFFFFF00))
    public static let female = TeamTalkStatusMode(rawValue: 0x00000100)
    public static let videoTransmit = TeamTalkStatusMode(rawValue: 0x00000200)
    public static let desktop = TeamTalkStatusMode(rawValue: 0x00000400)
    public static let streamMediaFile = TeamTalkStatusMode(rawValue: 0x00000800)
}

