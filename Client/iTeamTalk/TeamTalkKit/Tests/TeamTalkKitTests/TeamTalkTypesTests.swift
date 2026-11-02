import TeamTalkC
import TeamTalkKit
import XCTest

final class TeamTalkTypesTests: XCTestCase {
    func testCommandIDValidityAndDescription() {
        XCTAssertTrue(TeamTalkCommandID(42).isValid)
        XCTAssertFalse(TeamTalkCommandID.none.isValid)
        XCTAssertFalse(TeamTalkCommandID.invalid.isValid)
        XCTAssertEqual(TeamTalkCommandID(42).description, "42")
    }

    func testTypedIDsPreserveRawValues() {
        XCTAssertEqual(TeamTalkUserID(5).cValue, 5)
        XCTAssertTrue(TeamTalkUserID(5).isValid)
        XCTAssertFalse(TeamTalkUserID.none.isValid)
        XCTAssertEqual(TeamTalkUserID.invalid.description, "-1")

        XCTAssertEqual(TeamTalkChannelID(6).cValue, 6)
        XCTAssertTrue(TeamTalkChannelID(6).isValid)
        XCTAssertFalse(TeamTalkChannelID.none.isValid)
        XCTAssertEqual(TeamTalkChannelID.invalid.description, "-1")

        XCTAssertEqual(TeamTalkFileID(7).cValue, 7)
        XCTAssertTrue(TeamTalkFileID(7).isValid)
        XCTAssertFalse(TeamTalkFileID.none.isValid)
        XCTAssertEqual(TeamTalkFileID.invalid.description, "-1")

        XCTAssertEqual(TeamTalkTransferID(8).cValue, 8)
        XCTAssertTrue(TeamTalkTransferID(8).isValid)
        XCTAssertFalse(TeamTalkTransferID.none.isValid)
        XCTAssertEqual(TeamTalkTransferID.invalid.description, "-1")

        XCTAssertEqual(TeamTalkPlaybackSessionID(9).cValue, 9)
        XCTAssertTrue(TeamTalkPlaybackSessionID(9).isValid)
        XCTAssertFalse(TeamTalkPlaybackSessionID.none.isValid)
        XCTAssertEqual(TeamTalkPlaybackSessionID.invalid.description, "-1")

        XCTAssertEqual(TeamTalkDesktopSessionID(10).cValue, 10)
        XCTAssertTrue(TeamTalkDesktopSessionID(10).isValid)
        XCTAssertFalse(TeamTalkDesktopSessionID.none.isValid)
        XCTAssertEqual(TeamTalkDesktopSessionID.invalid.description, "-1")

        XCTAssertEqual(TeamTalkMediaStreamID(11).cValue, 11)
        XCTAssertTrue(TeamTalkMediaStreamID(11).isValid)
        XCTAssertFalse(TeamTalkMediaStreamID.none.isValid)
        XCTAssertEqual(TeamTalkMediaStreamID.invalid.description, "-1")

        XCTAssertEqual(TeamTalkAudioBlockSourceID.localUser.cValue, TT_LOCAL_USERID)
        XCTAssertEqual(TeamTalkAudioBlockSourceID.localTransmission.cValue, TT_LOCAL_TX_USERID)
        XCTAssertEqual(TeamTalkAudioBlockSourceID.muxed.cValue, TT_MUXED_USERID)
        XCTAssertTrue(TeamTalkAudioBlockSourceID.localUser.isSpecialSource)
        XCTAssertTrue(TeamTalkAudioBlockSourceID.localTransmission.isSpecialSource)
        XCTAssertTrue(TeamTalkAudioBlockSourceID.muxed.isSpecialSource)
        XCTAssertFalse(TeamTalkAudioBlockSourceID(userID: TeamTalkUserID(10)).isSpecialSource)
        XCTAssertEqual(TeamTalkAudioBlockSourceID(userID: TeamTalkUserID(10)).description, "10")
        XCTAssertEqual(
            TeamTalkAudioBlockSourceID(playbackSessionID: TeamTalkPlaybackSessionID(12)).cValue,
            12
        )
    }

    func testChannelPathNormalizesAndExposesComponents() {
        let empty = TeamTalkChannelPath.empty
        XCTAssertTrue(empty.isEmpty)
        XCTAssertFalse(empty.isRoot)
        XCTAssertEqual(empty.components, [])
        XCTAssertNil(empty.parent)

        let root = TeamTalkChannelPath.root
        XCTAssertFalse(root.isEmpty)
        XCTAssertTrue(root.isRoot)
        XCTAssertEqual(root.rawValue, "/")
        XCTAssertEqual(root.components, [])
        XCTAssertNil(root.parent)

        let path = TeamTalkChannelPath(rawValue: "///Lobby//Music///Rock/")
        XCTAssertEqual(path.rawValue, "/Lobby/Music/Rock")
        XCTAssertEqual(path.components.map(\.rawValue), ["Lobby", "Music", "Rock"])
        XCTAssertEqual(path.depth, 3)
        XCTAssertEqual(path.lastComponent?.rawValue, "Rock")
        XCTAssertEqual(path.parent?.rawValue, "/Lobby/Music")
        XCTAssertEqual(path.description, "/Lobby/Music/Rock")

        let rebuilt = TeamTalkChannelPath(components: ["Lobby", "Music", "Rock"])
        XCTAssertEqual(rebuilt, path)
        XCTAssertEqual(root.appending(component: "Lobby").rawValue, "/Lobby")
        XCTAssertEqual(path.appending("Live").rawValue, "/Lobby/Music/Rock/Live")

        let component: TeamTalkChannelPathComponent = "Mu/sic"
        XCTAssertEqual(component.rawValue, "Music")
        XCTAssertFalse(component.isEmpty)
        XCTAssertEqual(component.description, "Music")
    }

    func testSoundSystemWrapperPreservesCValues() {
        assertSoundSystemMappings([
            (.none, SOUNDSYSTEM_NONE),
            (.winMM, SOUNDSYSTEM_WINMM),
            (.directSound, SOUNDSYSTEM_DSOUND),
            (.alsa, SOUNDSYSTEM_ALSA),
            (.coreAudio, SOUNDSYSTEM_COREAUDIO),
            (.wasapi, SOUNDSYSTEM_WASAPI),
            (.openSLESAndroid, SOUNDSYSTEM_OPENSLES_ANDROID),
            (.audioUnit, SOUNDSYSTEM_AUDIOUNIT),
            (.pulseAudio, SOUNDSYSTEM_PULSEAUDIO)
        ])

        XCTAssertEqual(TeamTalkSoundSystem.audioUnitIOS, .audioUnit)
        XCTAssertEqual(TeamTalkSoundSystem(cValue: SOUNDSYSTEM_AUDIOUNIT), .audioUnit)
    }

    func testSoundDeviceFeaturesMapToCBitmask() {
        let features: TeamTalkSoundDeviceFeatures = [.echoCancellation, .denoise]
        let expected = SOUNDDEVICEFEATURE_AEC.rawValue | SOUNDDEVICEFEATURE_DENOISE.rawValue

        XCTAssertTrue(features.contains(.echoCancellation))
        XCTAssertTrue(features.contains(.denoise))
        XCTAssertEqual(features.cValue, expected)

        let mappings: [(TeamTalkSoundDeviceFeatures, UInt32)] = [
            (.none, SOUNDDEVICEFEATURE_NONE.rawValue),
            (.echoCancellation, SOUNDDEVICEFEATURE_AEC.rawValue),
            (.automaticGainControl, SOUNDDEVICEFEATURE_AGC.rawValue),
            (.denoise, SOUNDDEVICEFEATURE_DENOISE.rawValue),
            (.threeDPosition, SOUNDDEVICEFEATURE_3DPOSITION.rawValue),
            (.duplexMode, SOUNDDEVICEFEATURE_DUPLEXMODE.rawValue),
            (.defaultCommunicationDevice, SOUNDDEVICEFEATURE_DEFAULTCOMDEVICE.rawValue)
        ]
        assertUInt32Mappings(mappings)
    }

    func testSoundDeviceIDWrapperPreservesRawValuesAndHelpers() {
        XCTAssertEqual(TeamTalkSoundDeviceID(42).cValue, 42)
        XCTAssertEqual(TeamTalkSoundDeviceID(42).description, "42")

        XCTAssertEqual(TeamTalkSoundDeviceID.remoteIO.rawValue, TT_SOUNDDEVICE_ID_REMOTEIO)
        XCTAssertEqual(TeamTalkSoundDeviceID.voiceProcessingIO.rawValue, TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO)
        XCTAssertEqual(TeamTalkSoundDeviceID.openSLESDefault.rawValue, TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT)
        XCTAssertEqual(TeamTalkSoundDeviceID.openSLESVoiceCommunication.rawValue, TT_SOUNDDEVICE_ID_OPENSLES_VOICECOM)
        XCTAssertEqual(TeamTalkSoundDeviceID.teamTalkVirtual.rawValue, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL)

        XCTAssertFalse(TeamTalkSoundDeviceID.remoteIO.isShared)
        XCTAssertFalse(TeamTalkSoundDeviceID.openSLESVoiceCommunication.isShared)
        XCTAssertTrue(TeamTalkSoundDeviceID.voiceProcessingIO.isShared)
        XCTAssertEqual(TeamTalkSoundDeviceID.voiceProcessingIO.physicalDeviceID, TeamTalkSoundDeviceID(1))
        XCTAssertEqual(TeamTalkSoundDeviceID.teamTalkVirtual.physicalDeviceID, .teamTalkVirtual)
    }

    func testErrorCodeCategoriesAndRawValues() {
        XCTAssertTrue(TeamTalkErrorCode.success.isSuccess)
        XCTAssertFalse(TeamTalkErrorCode.notAuthorized.isSuccess)
        XCTAssertTrue(TeamTalkErrorCode.notAuthorized.isCommandError)
        XCTAssertFalse(TeamTalkErrorCode.notAuthorized.isInternalError)
        XCTAssertTrue(TeamTalkErrorCode.soundInputFailure.isInternalError)
        XCTAssertEqual(TeamTalkErrorCode.notAuthorized.cValue, Int32(CMDERR_NOT_AUTHORIZED.rawValue))
        XCTAssertEqual(TeamTalkErrorCode.soundInputFailure.cValue, Int32(INTERR_SNDINPUT_FAILURE.rawValue))

        assertErrorCodeMappings([
            (.success, CMDERR_SUCCESS),
            (.syntaxError, CMDERR_SYNTAX_ERROR),
            (.unknownCommand, CMDERR_UNKNOWN_COMMAND),
            (.missingParameter, CMDERR_MISSING_PARAMETER),
            (.incompatibleProtocols, CMDERR_INCOMPATIBLE_PROTOCOLS),
            (.unknownAudioCodec, CMDERR_UNKNOWN_AUDIOCODEC),
            (.invalidUsername, CMDERR_INVALID_USERNAME),
            (.incorrectChannelPassword, CMDERR_INCORRECT_CHANNEL_PASSWORD),
            (.invalidAccount, CMDERR_INVALID_ACCOUNT),
            (.maxServerUsersExceeded, CMDERR_MAX_SERVER_USERS_EXCEEDED),
            (.maxChannelUsersExceeded, CMDERR_MAX_CHANNEL_USERS_EXCEEDED),
            (.serverBanned, CMDERR_SERVER_BANNED),
            (.notAuthorized, CMDERR_NOT_AUTHORIZED),
            (.maxDiskUsageExceeded, CMDERR_MAX_DISKUSAGE_EXCEEDED),
            (.incorrectOperatorPassword, CMDERR_INCORRECT_OP_PASSWORD),
            (.audioCodecBitrateLimitExceeded, CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED),
            (.maxLoginsPerIPAddressExceeded, CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED),
            (.maxChannelsExceeded, CMDERR_MAX_CHANNELS_EXCEEDED),
            (.commandFlood, CMDERR_COMMAND_FLOOD),
            (.channelBanned, CMDERR_CHANNEL_BANNED),
            (.maxFileTransfersExceeded, CMDERR_MAX_FILETRANSFERS_EXCEEDED),
            (.notLoggedIn, CMDERR_NOT_LOGGEDIN),
            (.alreadyLoggedIn, CMDERR_ALREADY_LOGGEDIN),
            (.notInChannel, CMDERR_NOT_IN_CHANNEL),
            (.alreadyInChannel, CMDERR_ALREADY_IN_CHANNEL),
            (.channelAlreadyExists, CMDERR_CHANNEL_ALREADY_EXISTS),
            (.channelNotFound, CMDERR_CHANNEL_NOT_FOUND),
            (.userNotFound, CMDERR_USER_NOT_FOUND),
            (.banNotFound, CMDERR_BAN_NOT_FOUND),
            (.fileTransferNotFound, CMDERR_FILETRANSFER_NOT_FOUND),
            (.openFileFailed, CMDERR_OPENFILE_FAILED),
            (.accountNotFound, CMDERR_ACCOUNT_NOT_FOUND),
            (.fileNotFound, CMDERR_FILE_NOT_FOUND),
            (.fileAlreadyExists, CMDERR_FILE_ALREADY_EXISTS),
            (.fileSharingDisabled, CMDERR_FILESHARING_DISABLED),
            (.channelHasUsers, CMDERR_CHANNEL_HAS_USERS),
            (.loginServiceUnavailable, CMDERR_LOGINSERVICE_UNAVAILABLE),
            (.channelCannotBeHidden, CMDERR_CHANNEL_CANNOT_BE_HIDDEN),
            (.soundInputFailure, INTERR_SNDINPUT_FAILURE),
            (.soundOutputFailure, INTERR_SNDOUTPUT_FAILURE),
            (.audioCodecInitializationFailed, INTERR_AUDIOCODEC_INIT_FAILED),
            (.speexDSPInitializationFailed, INTERR_SPEEXDSP_INIT_FAILED),
            (.audioPreprocessorInitializationFailed, INTERR_AUDIOPREPROCESSOR_INIT_FAILED),
            (.messageQueueOverflow, INTERR_TTMESSAGE_QUEUE_OVERFLOW),
            (.soundEffectFailure, INTERR_SNDEFFECT_FAILURE)
        ])
    }

    func testUserRightsMapToCBitmask() {
        let rights: TeamTalkUserRights = [.canUploadFiles, .canDownloadFiles, .canTransmitVoice]
        let expected = USERRIGHT_UPLOAD_FILES.rawValue
            | USERRIGHT_DOWNLOAD_FILES.rawValue
            | USERRIGHT_TRANSMIT_VOICE.rawValue

        XCTAssertTrue(rights.contains(.canUploadFiles))
        XCTAssertTrue(rights.contains(.canDownloadFiles))
        XCTAssertTrue(rights.contains(.canTransmitVoice))
        XCTAssertEqual(rights.cValue, expected)

        let mappings: [(TeamTalkUserRights, UInt32)] = [
            (.none, USERRIGHT_NONE.rawValue),
            (.multipleLogins, USERRIGHT_MULTI_LOGIN.rawValue),
            (.canViewAllUsers, USERRIGHT_VIEW_ALL_USERS.rawValue),
            (.canCreateTemporaryChannels, USERRIGHT_CREATE_TEMPORARY_CHANNEL.rawValue),
            (.canModifyChannels, USERRIGHT_MODIFY_CHANNELS.rawValue),
            (.canBroadcastTextMessages, USERRIGHT_TEXTMESSAGE_BROADCAST.rawValue),
            (.canKickUsers, USERRIGHT_KICK_USERS.rawValue),
            (.canBanUsers, USERRIGHT_BAN_USERS.rawValue),
            (.canMoveUsers, USERRIGHT_MOVE_USERS.rawValue),
            (.canEnableOperators, USERRIGHT_OPERATOR_ENABLE.rawValue),
            (.canUploadFiles, USERRIGHT_UPLOAD_FILES.rawValue),
            (.canDownloadFiles, USERRIGHT_DOWNLOAD_FILES.rawValue),
            (.canUpdateServerProperties, USERRIGHT_UPDATE_SERVERPROPERTIES.rawValue),
            (.canTransmitVoice, USERRIGHT_TRANSMIT_VOICE.rawValue),
            (.canTransmitVideoCapture, USERRIGHT_TRANSMIT_VIDEOCAPTURE.rawValue),
            (.canTransmitDesktop, USERRIGHT_TRANSMIT_DESKTOP.rawValue),
            (.canTransmitDesktopInput, USERRIGHT_TRANSMIT_DESKTOPINPUT.rawValue),
            (.canTransmitMediaFileAudio, USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO.rawValue),
            (.canTransmitMediaFileVideo, USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO.rawValue),
            (.canTransmitMediaFiles, USERRIGHT_TRANSMIT_MEDIAFILE.rawValue),
            (.lockedNickname, USERRIGHT_LOCKED_NICKNAME.rawValue),
            (.lockedStatus, USERRIGHT_LOCKED_STATUS.rawValue),
            (.canRecordVoice, USERRIGHT_RECORD_VOICE.rawValue),
            (.canViewHiddenChannels, USERRIGHT_VIEW_HIDDEN_CHANNELS.rawValue),
            (.canSendPrivateTextMessages, USERRIGHT_TEXTMESSAGE_USER.rawValue),
            (.canSendChannelTextMessages, USERRIGHT_TEXTMESSAGE_CHANNEL.rawValue)
        ]
        assertUInt32Mappings(mappings)
    }

    func testUserTypesMapToCBitmask() {
        let mappings: [(TeamTalkUserTypes, UInt32)] = [
            (TeamTalkUserTypes.none, USERTYPE_NONE.rawValue),
            (.defaultUser, USERTYPE_DEFAULT.rawValue),
            (.administrator, USERTYPE_ADMIN.rawValue)
        ]
        assertUInt32Mappings(mappings)
    }

    func testSubscriptionsMapToCBitmask() {
        let subscriptions: TeamTalkSubscriptions = [.voice, .channelMessages, .interceptUserMessages]
        let expected = SUBSCRIBE_VOICE.rawValue
            | SUBSCRIBE_CHANNEL_MSG.rawValue
            | SUBSCRIBE_INTERCEPT_USER_MSG.rawValue

        XCTAssertTrue(subscriptions.contains(.voice))
        XCTAssertTrue(subscriptions.contains(.channelMessages))
        XCTAssertTrue(subscriptions.contains(.interceptUserMessages))
        XCTAssertEqual(subscriptions.cValue, expected)

        let mappings: [(TeamTalkSubscriptions, UInt32)] = [
            (.none, SUBSCRIBE_NONE.rawValue),
            (.userMessages, SUBSCRIBE_USER_MSG.rawValue),
            (.channelMessages, SUBSCRIBE_CHANNEL_MSG.rawValue),
            (.broadcastMessages, SUBSCRIBE_BROADCAST_MSG.rawValue),
            (.customMessages, SUBSCRIBE_CUSTOM_MSG.rawValue),
            (.voice, SUBSCRIBE_VOICE.rawValue),
            (.videoCapture, SUBSCRIBE_VIDEOCAPTURE.rawValue),
            (.desktop, SUBSCRIBE_DESKTOP.rawValue),
            (.desktopInput, SUBSCRIBE_DESKTOPINPUT.rawValue),
            (.mediaFile, SUBSCRIBE_MEDIAFILE.rawValue),
            (.interceptUserMessages, SUBSCRIBE_INTERCEPT_USER_MSG.rawValue),
            (.interceptChannelMessages, SUBSCRIBE_INTERCEPT_CHANNEL_MSG.rawValue),
            (.interceptCustomMessages, SUBSCRIBE_INTERCEPT_CUSTOM_MSG.rawValue),
            (.interceptVoice, SUBSCRIBE_INTERCEPT_VOICE.rawValue),
            (.interceptVideoCapture, SUBSCRIBE_INTERCEPT_VIDEOCAPTURE.rawValue),
            (.interceptDesktop, SUBSCRIBE_INTERCEPT_DESKTOP.rawValue),
            (.interceptMediaFile, SUBSCRIBE_INTERCEPT_MEDIAFILE.rawValue)
        ]
        assertUInt32Mappings(mappings)
    }

    func testUserStatesMapToCBitmask() {
        let mappings: [(TeamTalkUserStates, UInt32)] = [
            (TeamTalkUserStates.none, USERSTATE_NONE.rawValue),
            (.voice, USERSTATE_VOICE.rawValue),
            (.voiceMuted, USERSTATE_MUTE_VOICE.rawValue),
            (.mediaFileMuted, USERSTATE_MUTE_MEDIAFILE.rawValue),
            (.desktop, USERSTATE_DESKTOP.rawValue),
            (.videoCapture, USERSTATE_VIDEOCAPTURE.rawValue),
            (.mediaFileAudio, USERSTATE_MEDIAFILE_AUDIO.rawValue),
            (.mediaFileVideo, USERSTATE_MEDIAFILE_VIDEO.rawValue),
            (.mediaFile, USERSTATE_MEDIAFILE.rawValue)
        ]
        assertUInt32Mappings(mappings)
    }

    func testStreamTypesExposeSingleValueAndMaskRepresentations() {
        let stream: TeamTalkStreamTypes = .voice
        let streams: TeamTalkStreamTypes = [.voice, .desktop]

        XCTAssertEqual(stream.cValue.rawValue, STREAMTYPE_VOICE.rawValue)
        XCTAssertEqual(streams.cMask, STREAMTYPE_VOICE.rawValue | STREAMTYPE_DESKTOP.rawValue)

        let mappings: [(TeamTalkStreamTypes, UInt32)] = [
            (TeamTalkStreamTypes.none, STREAMTYPE_NONE.rawValue),
            (.voice, STREAMTYPE_VOICE.rawValue),
            (.videoCapture, STREAMTYPE_VIDEOCAPTURE.rawValue),
            (.mediaFileAudio, STREAMTYPE_MEDIAFILE_AUDIO.rawValue),
            (.mediaFileVideo, STREAMTYPE_MEDIAFILE_VIDEO.rawValue),
            (.desktop, STREAMTYPE_DESKTOP.rawValue),
            (.desktopInput, STREAMTYPE_DESKTOPINPUT.rawValue),
            (.mediaFile, STREAMTYPE_MEDIAFILE.rawValue),
            (.channelMessage, STREAMTYPE_CHANNELMSG.rawValue),
            (.localMediaPlaybackAudio, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO.rawValue),
            (.classroomAll, STREAMTYPE_CLASSROOM_ALL.rawValue)
        ]
        assertUInt32Mappings(mappings)
    }

    func testChannelTypesMapToCBitmask() {
        let mappings: [(TeamTalkChannelTypes, UInt32)] = [
            (TeamTalkChannelTypes.default, CHANNEL_DEFAULT.rawValue),
            (.permanent, CHANNEL_PERMANENT.rawValue),
            (.soloTransmit, CHANNEL_SOLO_TRANSMIT.rawValue),
            (.classroom, CHANNEL_CLASSROOM.rawValue),
            (.operatorReceiveOnly, CHANNEL_OPERATOR_RECVONLY.rawValue),
            (.noVoiceActivation, CHANNEL_NO_VOICEACTIVATION.rawValue),
            (.noRecording, CHANNEL_NO_RECORDING.rawValue),
            (.hidden, CHANNEL_HIDDEN.rawValue)
        ]
        assertUInt32Mappings(mappings)
    }

    func testServerLogEventsMapToCBitmask() {
        let mappings: [(TeamTalkServerLogEvents, UInt32)] = [
            (TeamTalkServerLogEvents.none, SERVERLOGEVENT_NONE.rawValue),
            (.userConnected, SERVERLOGEVENT_USER_CONNECTED.rawValue),
            (.userDisconnected, SERVERLOGEVENT_USER_DISCONNECTED.rawValue),
            (.userLoggedIn, SERVERLOGEVENT_USER_LOGGEDIN.rawValue),
            (.userLoggedOut, SERVERLOGEVENT_USER_LOGGEDOUT.rawValue),
            (.userLoginFailed, SERVERLOGEVENT_USER_LOGINFAILED.rawValue),
            (.userTimedOut, SERVERLOGEVENT_USER_TIMEDOUT.rawValue),
            (.userKicked, SERVERLOGEVENT_USER_KICKED.rawValue),
            (.userBanned, SERVERLOGEVENT_USER_BANNED.rawValue),
            (.userUnbanned, SERVERLOGEVENT_USER_UNBANNED.rawValue),
            (.userUpdated, SERVERLOGEVENT_USER_UPDATED.rawValue),
            (.userJoinedChannel, SERVERLOGEVENT_USER_JOINEDCHANNEL.rawValue),
            (.userLeftChannel, SERVERLOGEVENT_USER_LEFTCHANNEL.rawValue),
            (.userMoved, SERVERLOGEVENT_USER_MOVED.rawValue),
            (.userPrivateTextMessage, SERVERLOGEVENT_USER_TEXTMESSAGE_PRIVATE.rawValue),
            (.userCustomTextMessage, SERVERLOGEVENT_USER_TEXTMESSAGE_CUSTOM.rawValue),
            (.userChannelTextMessage, SERVERLOGEVENT_USER_TEXTMESSAGE_CHANNEL.rawValue),
            (.userBroadcastTextMessage, SERVERLOGEVENT_USER_TEXTMESSAGE_BROADCAST.rawValue),
            (.channelCreated, SERVERLOGEVENT_CHANNEL_CREATED.rawValue),
            (.channelUpdated, SERVERLOGEVENT_CHANNEL_UPDATED.rawValue),
            (.channelRemoved, SERVERLOGEVENT_CHANNEL_REMOVED.rawValue),
            (.fileUploaded, SERVERLOGEVENT_FILE_UPLOADED.rawValue),
            (.fileDownloaded, SERVERLOGEVENT_FILE_DOWNLOADED.rawValue),
            (.fileDeleted, SERVERLOGEVENT_FILE_DELETED.rawValue),
            (.serverUpdated, SERVERLOGEVENT_SERVER_UPDATED.rawValue),
            (.serverSavedConfig, SERVERLOGEVENT_SERVER_SAVECONFIG.rawValue),
            (.userEncryptionError, SERVERLOGEVENT_USER_CRYPTERROR.rawValue),
            (.userNewStream, SERVERLOGEVENT_USER_NEW_STREAM.rawValue)
        ]
        assertUInt32Mappings(mappings)
    }

    func testBanTypesMapToCBitmask() {
        let mappings: [(TeamTalkBanTypes, UInt32)] = [
            (TeamTalkBanTypes.none, BANTYPE_NONE.rawValue),
            (.channel, BANTYPE_CHANNEL.rawValue),
            (.ipAddress, BANTYPE_IPADDR.rawValue),
            (.username, BANTYPE_USERNAME.rawValue)
        ]
        assertUInt32Mappings(mappings)
    }

    func testStatusModeRawValues() {
        let mappings: [(TeamTalkStatusMode, Int32)] = [
            (TeamTalkStatusMode.available, Int32(0)),
            (.away, 0x00000001),
            (.question, 0x00000002),
            (.modeMask, 0x000000FF),
            (.flagsMask, Int32(bitPattern: 0xFFFFFF00)),
            (.female, 0x00000100),
            (.videoTransmit, 0x00000200),
            (.desktop, 0x00000400),
            (.streamMediaFile, 0x00000800)
        ]
        assertInt32Mappings(mappings)
    }

    func testClientFlagsMapToCBitmask() {
        let mappings: [(TeamTalkClientFlags, UInt32)] = [
            (.connected, CLIENT_CONNECTED.rawValue),
            (.authorized, CLIENT_AUTHORIZED.rawValue),
            (.soundInputReady, CLIENT_SNDINPUT_READY.rawValue),
            (.transmittingVoice, CLIENT_TX_VOICE.rawValue),
            (.voiceActivated, CLIENT_SNDINPUT_VOICEACTIVATED.rawValue),
            (.voiceActive, CLIENT_SNDINPUT_VOICEACTIVE.rawValue),
            (.closed, CLIENT_CLOSED.rawValue),
            (.soundOutputReady, CLIENT_SNDOUTPUT_READY.rawValue),
            (.soundInputOutputDuplex, CLIENT_SNDINOUTPUT_DUPLEX.rawValue),
            (.soundOutputMuted, CLIENT_SNDOUTPUT_MUTE.rawValue),
            (.soundOutputAuto3DPosition, CLIENT_SNDOUTPUT_AUTO3DPOSITION.rawValue),
            (.videoCaptureReady, CLIENT_VIDEOCAPTURE_READY.rawValue),
            (.transmittingVideoCapture, CLIENT_TX_VIDEOCAPTURE.rawValue),
            (.transmittingDesktop, CLIENT_TX_DESKTOP.rawValue),
            (.desktopActive, CLIENT_DESKTOP_ACTIVE.rawValue),
            (.muxingAudioFile, CLIENT_MUX_AUDIOFILE.rawValue),
            (.connecting, CLIENT_CONNECTING.rawValue),
            (.connection, CLIENT_CONNECTION.rawValue),
            (.streamingAudio, CLIENT_STREAM_AUDIO.rawValue),
            (.streamingVideo, CLIENT_STREAM_VIDEO.rawValue)
        ]
        assertUInt32Mappings(mappings)
    }

    func testRawWrappersPreserveCValues() {
        assertCodecMappings([
            (.none, TeamTalkC.NO_CODEC),
            (.speex, TeamTalkC.SPEEX_CODEC),
            (.speexVBR, TeamTalkC.SPEEX_VBR_CODEC),
            (.opus, TeamTalkC.OPUS_CODEC),
            (.webMVP8, WEBM_VP8_CODEC)
        ])

        assertAudioPreprocessorMappings([
            (.none, NO_AUDIOPREPROCESSOR),
            (.speexDSP, SPEEXDSP_AUDIOPREPROCESSOR),
            (.teamTalk, TEAMTALK_AUDIOPREPROCESSOR),
            (.obsoleteWebRTC, WEBRTC_AUDIOPREPROCESSOR_OBSOLETE_R4332),
            (.webRTC, WEBRTC_AUDIOPREPROCESSOR)
        ])

        assertTextMessageMappings([
            (.none, MSGTYPE_NONE),
            (.user, TeamTalkC.MSGTYPE_USER),
            (.channel, TeamTalkC.MSGTYPE_CHANNEL),
            (.broadcast, MSGTYPE_BROADCAST),
            (.custom, MSGTYPE_CUSTOM)
        ])

        assertFileTransferStatusMappings([
            (.closed, FILETRANSFER_CLOSED),
            (.error, FILETRANSFER_ERROR),
            (.active, FILETRANSFER_ACTIVE),
            (.finished, FILETRANSFER_FINISHED)
        ])

        assertBitmapFormatMappings([
            (.none, BMP_NONE),
            (.rgb8Palette, BMP_RGB8_PALETTE),
            (.rgb16_555, BMP_RGB16_555),
            (.rgb24, BMP_RGB24),
            (.rgb32, BMP_RGB32)
        ])

        assertDesktopProtocolMappings([
            (.zlib1, DESKTOPPROTOCOL_ZLIB_1)
        ])

        assertUInt32Mappings([
            (.none, DESKTOPKEYSTATE_NONE.rawValue),
            (.down, DESKTOPKEYSTATE_DOWN.rawValue),
            (.up, DESKTOPKEYSTATE_UP.rawValue)
        ] as [(TeamTalkDesktopKeyState, UInt32)])

        assertMediaFileStatusMappings([
            (.closed, MFS_CLOSED),
            (.error, MFS_ERROR),
            (.started, MFS_STARTED),
            (.finished, MFS_FINISHED),
            (.aborted, MFS_ABORTED),
            (.paused, MFS_PAUSED),
            (.playing, MFS_PLAYING)
        ])

        assertAudioFileFormatMappings([
            (.none, AFF_NONE),
            (.channelCodec, AFF_CHANNELCODEC_FORMAT),
            (.wave, AFF_WAVE_FORMAT),
            (.mp3_16kbit, AFF_MP3_16KBIT_FORMAT),
            (.mp3_32kbit, AFF_MP3_32KBIT_FORMAT),
            (.mp3_64kbit, AFF_MP3_64KBIT_FORMAT),
            (.mp3_128kbit, AFF_MP3_128KBIT_FORMAT),
            (.mp3_256kbit, AFF_MP3_256KBIT_FORMAT),
            (.mp3_320kbit, AFF_MP3_320KBIT_FORMAT)
        ])

        assertVideoPixelFormatMappings([
            (.none, FOURCC_NONE),
            (.i420, FOURCC_I420),
            (.yuy2, FOURCC_YUY2),
            (.rgb32, FOURCC_RGB32)
        ])

        let combinedDesktopKeyState: TeamTalkDesktopKeyState = [.down, .up]
        XCTAssertEqual(TeamTalkDesktopProtocol.zlib, .zlib1)
        XCTAssertEqual(combinedDesktopKeyState.cValue, DESKTOPKEYSTATE_DOWN.rawValue | DESKTOPKEYSTATE_UP.rawValue)
        XCTAssertTrue(TeamTalkDesktopKeyCode.ignore.isIgnored)
        XCTAssertFalse(TeamTalkDesktopKeyCode.ignore.isMouseButton)
        XCTAssertEqual(TeamTalkDesktopKeyCode.leftMouseButton.rawValue, UInt32(TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN))
        XCTAssertEqual(TeamTalkDesktopKeyCode.rightMouseButton.rawValue, UInt32(TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN))
        XCTAssertEqual(TeamTalkDesktopKeyCode.middleMouseButton.rawValue, UInt32(TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN))
        XCTAssertTrue(TeamTalkDesktopKeyCode.leftMouseButton.isMouseButton)
        XCTAssertEqual(TeamTalkDesktopKeyCode(42).description, "42")
    }

    func testWebRTCNoiseSuppressionLevelPreservesRawValues() {
        assertInt32Mappings([
            (.low, 0),
            (.moderate, 1),
            (.high, 2),
            (.veryHigh, 3)
        ] as [(TeamTalkWebRTCNoiseSuppressionLevel, Int32)])
    }

    func testClientEventWrapperPreservesCValues() {
        assertClientEventMappings([
            (.none, CLIENTEVENT_NONE),
            (.connectionSucceeded, CLIENTEVENT_CON_SUCCESS),
            (.connectionEncryptionError, CLIENTEVENT_CON_CRYPT_ERROR),
            (.connectionFailed, CLIENTEVENT_CON_FAILED),
            (.connectionLost, CLIENTEVENT_CON_LOST),
            (.connectionMaxPayloadUpdated, CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED),
            (.commandProcessing, CLIENTEVENT_CMD_PROCESSING),
            (.commandError, CLIENTEVENT_CMD_ERROR),
            (.commandSucceeded, CLIENTEVENT_CMD_SUCCESS),
            (.myselfLoggedIn, CLIENTEVENT_CMD_MYSELF_LOGGEDIN),
            (.myselfLoggedOut, CLIENTEVENT_CMD_MYSELF_LOGGEDOUT),
            (.myselfKicked, CLIENTEVENT_CMD_MYSELF_KICKED),
            (.userLoggedIn, CLIENTEVENT_CMD_USER_LOGGEDIN),
            (.userLoggedOut, CLIENTEVENT_CMD_USER_LOGGEDOUT),
            (.userUpdated, CLIENTEVENT_CMD_USER_UPDATE),
            (.userJoined, CLIENTEVENT_CMD_USER_JOINED),
            (.userLeft, CLIENTEVENT_CMD_USER_LEFT),
            (.userTextMessage, CLIENTEVENT_CMD_USER_TEXTMSG),
            (.channelCreated, CLIENTEVENT_CMD_CHANNEL_NEW),
            (.channelUpdated, CLIENTEVENT_CMD_CHANNEL_UPDATE),
            (.channelRemoved, CLIENTEVENT_CMD_CHANNEL_REMOVE),
            (.serverUpdated, CLIENTEVENT_CMD_SERVER_UPDATE),
            (.serverStatistics, CLIENTEVENT_CMD_SERVERSTATISTICS),
            (.fileCreated, CLIENTEVENT_CMD_FILE_NEW),
            (.fileRemoved, CLIENTEVENT_CMD_FILE_REMOVE),
            (.userAccount, CLIENTEVENT_CMD_USERACCOUNT),
            (.bannedUser, CLIENTEVENT_CMD_BANNEDUSER),
            (.userAccountCreated, CLIENTEVENT_CMD_USERACCOUNT_NEW),
            (.userAccountRemoved, CLIENTEVENT_CMD_USERACCOUNT_REMOVE),
            (.userStateChanged, CLIENTEVENT_USER_STATECHANGE),
            (.userVideoCapture, CLIENTEVENT_USER_VIDEOCAPTURE),
            (.userMediaFileVideo, CLIENTEVENT_USER_MEDIAFILE_VIDEO),
            (.userDesktopWindow, CLIENTEVENT_USER_DESKTOPWINDOW),
            (.userDesktopCursor, CLIENTEVENT_USER_DESKTOPCURSOR),
            (.userDesktopInput, CLIENTEVENT_USER_DESKTOPINPUT),
            (.userRecordMediaFile, CLIENTEVENT_USER_RECORD_MEDIAFILE),
            (.userAudioBlock, CLIENTEVENT_USER_AUDIOBLOCK),
            (.internalError, CLIENTEVENT_INTERNAL_ERROR),
            (.voiceActivation, CLIENTEVENT_VOICE_ACTIVATION),
            (.hotkey, CLIENTEVENT_HOTKEY),
            (.hotkeyTest, CLIENTEVENT_HOTKEY_TEST),
            (.fileTransfer, CLIENTEVENT_FILETRANSFER),
            (.desktopWindowTransfer, CLIENTEVENT_DESKTOPWINDOW_TRANSFER),
            (.streamMediaFile, CLIENTEVENT_STREAM_MEDIAFILE),
            (.localMediaFile, CLIENTEVENT_LOCAL_MEDIAFILE),
            (.audioInput, CLIENTEVENT_AUDIOINPUT),
            (.userFirstVoiceStreamPacket, CLIENTEVENT_USER_FIRSTVOICESTREAMPACKET),
            (.soundDeviceAdded, CLIENTEVENT_SOUNDDEVICE_ADDED),
            (.soundDeviceRemoved, CLIENTEVENT_SOUNDDEVICE_REMOVED),
            (.soundDeviceUnplugged, CLIENTEVENT_SOUNDDEVICE_UNPLUGGED),
            (.soundDeviceNewDefaultInput, CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_INPUT),
            (.soundDeviceNewDefaultOutput, CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_OUTPUT),
            (.soundDeviceNewDefaultInputCommunicationDevice, CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_INPUT_COMDEVICE),
            (.soundDeviceNewDefaultOutputCommunicationDevice, CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_OUTPUT_COMDEVICE)
        ])
    }

    func testPayloadTypeWrapperPreservesCValues() {
        assertPayloadMappings([
            (.none, __NONE),
            (.audioCodec, __AUDIOCODEC),
            (.bannedUser, __BANNEDUSER),
            (.videoFormat, __VIDEOFORMAT),
            (.opusCodec, __OPUSCODEC),
            (.channel, __CHANNEL),
            (.clientStatistics, __CLIENTSTATISTICS),
            (.clientError, __CLIENTERRORMSG),
            (.fileTransfer, __FILETRANSFER),
            (.mediaFileStatus, __MEDIAFILESTATUS),
            (.remoteFile, __REMOTEFILE),
            (.serverProperties, __SERVERPROPERTIES),
            (.serverStatistics, __SERVERSTATISTICS),
            (.soundDevice, __SOUNDDEVICE),
            (.speexCodec, __SPEEXCODEC),
            (.textMessage, __TEXTMESSAGE),
            (.webMVP8Codec, __WEBMVP8CODEC),
            (.teamTalkMessage, __TTMESSAGE),
            (.user, __USER),
            (.userAccount, __USERACCOUNT),
            (.userStatistics, __USERSTATISTICS),
            (.videoCaptureDevice, __VIDEOCAPTUREDEVICE),
            (.videoCodec, __VIDEOCODEC),
            (.audioConfig, __AUDIOCONFIG),
            (.speexVBRCodec, __SPEEXVBRCODEC),
            (.videoFrame, __VIDEOFRAME),
            (.audioBlock, __AUDIOBLOCK),
            (.audioFormat, __AUDIOFORMAT),
            (.mediaFileInfo, __MEDIAFILEINFO),
            (.boolean, __TTBOOL),
            (.integer, __INT32),
            (.desktopInput, __DESKTOPINPUT),
            (.speexDSP, __SPEEXDSP),
            (.streamType, __STREAMTYPE),
            (.audioPreprocessorType, __AUDIOPREPROCESSORTYPE),
            (.audioPreprocessor, __AUDIOPREPROCESSOR),
            (.teamTalkAudioPreprocessor, __TTAUDIOPREPROCESSOR),
            (.mediaFilePlayback, __MEDIAFILEPLAYBACK),
            (.clientKeepAlive, __CLIENTKEEPALIVE),
            (.unsignedInteger, __UINT32),
            (.audioInputProgress, __AUDIOINPUTPROGRESS),
            (.jitterConfig, __JITTERCONFIG),
            (.webRTCAudioPreprocessor, __WEBRTCAUDIOPREPROCESSOR),
            (.encryptionContext, __ENCRYPTIONCONTEXT),
            (.soundDeviceEffects, __SOUNDDEVICEEFFECTS),
            (.desktopWindow, __DESKTOPWINDOW),
            (.abusePrevention, __ABUSEPREVENTION)
        ])
    }

    private func assertUInt32Mappings<T: RawRepresentable>(
        _ mappings: [(T, UInt32)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) where T.RawValue == UInt32 {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.rawValue, expected, file: file, line: line)
        }
    }

    private func assertInt32Mappings<T: RawRepresentable>(
        _ mappings: [(T, Int32)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) where T.RawValue == Int32 {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.rawValue, expected, file: file, line: line)
        }
    }

    private func assertErrorCodeMappings(
        _ mappings: [(TeamTalkErrorCode, ClientError)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.rawValue, Int32(expected.rawValue), file: file, line: line)
        }
    }

    private func assertSoundSystemMappings(
        _ mappings: [(TeamTalkSoundSystem, SoundSystem)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.cValue, expected, file: file, line: line)
        }
    }

    private func assertCodecMappings(
        _ mappings: [(TeamTalkCodec, Codec)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.cValue, expected, file: file, line: line)
        }
    }

    private func assertAudioPreprocessorMappings(
        _ mappings: [(TeamTalkAudioPreprocessorType, AudioPreprocessorType)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.cValue, expected, file: file, line: line)
        }
    }

    private func assertTextMessageMappings(
        _ mappings: [(TeamTalkTextMessageType, TextMsgType)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.cValue, expected, file: file, line: line)
        }
    }

    private func assertFileTransferStatusMappings(
        _ mappings: [(TeamTalkFileTransferStatus, FileTransferStatus)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.cValue, expected, file: file, line: line)
        }
    }

    private func assertBitmapFormatMappings(
        _ mappings: [(TeamTalkBitmapFormat, BitmapFormat)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.cValue, expected, file: file, line: line)
        }
    }

    private func assertDesktopProtocolMappings(
        _ mappings: [(TeamTalkDesktopProtocol, DesktopProtocol)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.cValue, expected, file: file, line: line)
        }
    }

    private func assertMediaFileStatusMappings(
        _ mappings: [(TeamTalkMediaFileStatus, MediaFileStatus)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.cValue, expected, file: file, line: line)
        }
    }

    private func assertAudioFileFormatMappings(
        _ mappings: [(TeamTalkAudioFileFormat, AudioFileFormat)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.cValue, expected, file: file, line: line)
        }
    }

    private func assertVideoPixelFormatMappings(
        _ mappings: [(TeamTalkVideoPixelFormat, FourCC)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.cValue, expected, file: file, line: line)
        }
    }

    private func assertClientEventMappings(
        _ mappings: [(TeamTalkClientEvent, ClientEvent)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.cValue, expected, file: file, line: line)
        }
    }

    private func assertPayloadMappings(
        _ mappings: [(TeamTalkPayloadType, TTType)],
        file: StaticString = #filePath,
        line: UInt = #line
    ) {
        for (actual, expected) in mappings {
            XCTAssertEqual(actual.cValue, expected, file: file, line: line)
        }
    }
}
