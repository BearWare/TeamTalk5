import Foundation
import TeamTalkC
import TeamTalkKit
import XCTest

final class TeamTalkModelsTests: XCTestCase {
    func testModelTypedIdentifiersMirrorRawIDs() {
        var rawUser = User()
        rawUser.nUserID = 11
        rawUser.nChannelID = 22
        let user = TeamTalkUser(rawUser)
        XCTAssertEqual(user.userID, TeamTalkUserID(11))
        XCTAssertEqual(user.channelIdentifier, TeamTalkChannelID(22))
        XCTAssertEqual(rawUser.userID, TeamTalkUserID(11))
        XCTAssertEqual(rawUser.channelIdentifier, TeamTalkChannelID(22))

        var rawFile = RemoteFile()
        rawFile.nFileID = 33
        rawFile.nChannelID = 44
        let file = TeamTalkRemoteFile(rawFile)
        XCTAssertEqual(file.fileID, TeamTalkFileID(33))
        XCTAssertEqual(file.channelIdentifier, TeamTalkChannelID(44))
        XCTAssertEqual(rawFile.fileID, TeamTalkFileID(33))
        XCTAssertEqual(rawFile.channelIdentifier, TeamTalkChannelID(44))

        var rawTransfer = FileTransfer()
        rawTransfer.nTransferID = 55
        rawTransfer.nChannelID = 66
        let transfer = TeamTalkFileTransfer(rawTransfer)
        XCTAssertEqual(transfer.transferID, TeamTalkTransferID(55))
        XCTAssertEqual(transfer.channelIdentifier, TeamTalkChannelID(66))
        XCTAssertEqual(rawTransfer.transferID, TeamTalkTransferID(55))
        XCTAssertEqual(rawTransfer.channelIdentifier, TeamTalkChannelID(66))
    }

    func testSnapshotModelsAreEquatableAndHashable() {
        var rawUserA = User()
        rawUserA.nUserID = 11
        rawUserA.nChannelID = 22
        var rawUserB = rawUserA
        let userA = TeamTalkUser(rawUserA)
        let userB = TeamTalkUser(rawUserB)

        XCTAssertEqual(userA, userB)
        XCTAssertEqual(userA.hashValue, userB.hashValue)
        XCTAssertEqual(Set([userA, userB]).count, 1)

        rawUserB.nChannelID = 23
        let userC = TeamTalkUser(rawUserB)
        XCTAssertNotEqual(userA, userC)

        var rawChannelA = Channel()
        rawChannelA.nChannelID = 5
        var rawChannelB = rawChannelA
        XCTAssertEqual(TeamTalkChannel(rawChannelA), TeamTalkChannel(rawChannelB))

        rawChannelB.nMaxUsers = 25
        XCTAssertNotEqual(TeamTalkChannel(rawChannelA), TeamTalkChannel(rawChannelB))
    }

    func testEmptyPlaceholdersAndNewChannelConvenienceInit() {
        XCTAssertEqual(TeamTalkChannel.empty.channelID, .none)
        XCTAssertEqual(TeamTalkUser.empty.userID, .none)
        XCTAssertEqual(TeamTalkUserAccount.empty.username, "")
        XCTAssertEqual(TeamTalkServerProperties.empty.name, "")

        let parentID = TeamTalkChannelID(7)
        let newChannel = TeamTalkChannel(parentChannelID: parentID)
        XCTAssertEqual(newChannel.parentChannelID, parentID)
        XCTAssertFalse(newChannel.channelID.isValid)

        XCTAssertTrue(newChannel.password.isEmpty)
        let passworded = newChannel.settingPassword("secret")
        XCTAssertEqual(passworded.password, "secret")
        XCTAssertEqual(passworded.parentChannelID, parentID)
        XCTAssertTrue(newChannel.password.isEmpty)
    }

    func testUserAccountConfigurationRoundTrip() {
        let configuration = TeamTalkUserAccountConfiguration(
            username: "guest",
            password: "secret",
            types: [.defaultUser, .administrator],
            rights: [.canUploadFiles, .canDownloadFiles, .canSendChannelTextMessages],
            userData: 17,
            note: "trusted account",
            initialChannel: "/Lobby",
            autoOperatorChannelIDs: [10, 20],
            audioCodecBitrateLimit: 64_000,
            commandLimit: 12,
            commandIntervalMilliseconds: 1_500
        )

        let account = TeamTalkUserAccount(configuration.cValue)

        XCTAssertEqual(account.username, "guest")
        XCTAssertEqual(account.password, "secret")
        XCTAssertEqual(account.note, "trusted account")
        XCTAssertEqual(account.initialChannel, "/Lobby")
        XCTAssertEqual(account.types, [.defaultUser, .administrator])
        XCTAssertEqual(account.rights, [.canUploadFiles, .canDownloadFiles, .canSendChannelTextMessages])
        XCTAssertEqual(account.userData, 17)
        XCTAssertEqual(account.autoOperatorChannelIDs, [10, 20])
        XCTAssertEqual(account.autoOperatorChannelIdentifiers, [TeamTalkChannelID(10), TeamTalkChannelID(20)])
        XCTAssertEqual(account.audioCodecBitrateLimit, 64_000)
        XCTAssertEqual(account.abusePrevention.commandLimit, 12)
        XCTAssertEqual(account.abusePrevention.commandIntervalMilliseconds, 1_500)
        XCTAssertFalse(account.abusePrevention.isDisabled)
        XCTAssertEqual(account.commandLimit, 12)
        XCTAssertEqual(account.commandIntervalMilliseconds, 1_500)
        XCTAssertTrue(account.getUserRight(.canUploadFiles))
        XCTAssertEqual(account.cValue.autoOperatorChannelIdentifiers, [TeamTalkChannelID(10), TeamTalkChannelID(20)])
    }

    func testAbusePreventionConfigurationSupportsTypedAndCompatibilityAccessors() {
        var configuration = TeamTalkUserAccountConfiguration(
            username: "member",
            abusePrevention: TeamTalkAbusePreventionConfiguration(
                commandLimit: 5,
                commandIntervalMilliseconds: 2_000
            )
        )

        XCTAssertEqual(configuration.commandLimit, 5)
        XCTAssertEqual(configuration.commandIntervalMilliseconds, 2_000)
        XCTAssertFalse(configuration.abusePrevention.isDisabled)

        configuration.commandLimit = 7
        configuration.commandIntervalMilliseconds = 3_500
        configuration.autoOperatorChannelIdentifiers = [TeamTalkChannelID(11), TeamTalkChannelID(12)]

        let account = TeamTalkUserAccount(configuration.cValue)

        XCTAssertEqual(account.abusePrevention.commandLimit, 7)
        XCTAssertEqual(account.abusePrevention.commandIntervalMilliseconds, 3_500)
        XCTAssertEqual(account.commandLimit, 7)
        XCTAssertEqual(account.commandIntervalMilliseconds, 3_500)
        XCTAssertEqual(configuration.autoOperatorChannelIDs, [11, 12])
        XCTAssertEqual(account.autoOperatorChannelIdentifiers, [TeamTalkChannelID(11), TeamTalkChannelID(12)])

        let typedConfiguration = TeamTalkAbusePreventionConfiguration(account.abusePrevention)
        XCTAssertEqual(typedConfiguration.commandLimit, 7)
        XCTAssertEqual(typedConfiguration.commandIntervalMilliseconds, 3_500)

        let disabled = TeamTalkAbusePreventionConfiguration()
        XCTAssertTrue(disabled.isDisabled)
        XCTAssertTrue(TeamTalkAbusePrevention(disabled.cValue).isDisabled)
    }

    func testChannelConfigurationRoundTrip() {
        let configuration = TeamTalkChannelConfiguration(
            channelID: TeamTalkChannelID(7),
            parentChannelID: TeamTalkChannelID(1),
            name: "Lobby",
            topic: "Daily chat",
            password: "room-password",
            operatorPassword: "op-password",
            types: [.permanent, .noRecording],
            userData: 99,
            diskQuota: 4_096,
            maxUsers: 25,
            transmitUsers: [
                TeamTalkChannelTransmitUser(userID: TeamTalkUserID(42), streamTypes: [.voice, .videoCapture]),
                .classroomFreeForAll(streamTypes: [.desktopInput])
            ],
            transmitUsersQueue: [TeamTalkUserID(5), TeamTalkUserID(6), TeamTalkUserID(7)],
            transmitUsersQueueDelayMilliseconds: 750,
            voiceTimeoutMilliseconds: 1_500,
            mediaFileTimeoutMilliseconds: 3_000
        )

        let channel = TeamTalkChannel(configuration.cValue)

        XCTAssertEqual(channel.id, 7)
        XCTAssertEqual(channel.parentID, 1)
        XCTAssertEqual(channel.channelID, TeamTalkChannelID(7))
        XCTAssertEqual(channel.parentChannelID, TeamTalkChannelID(1))
        XCTAssertEqual(channel.name, "Lobby")
        XCTAssertEqual(channel.topic, "Daily chat")
        XCTAssertEqual(channel.password, "room-password")
        XCTAssertEqual(channel.operatorPassword, "op-password")
        XCTAssertEqual(channel.types, [.permanent, .noRecording])
        XCTAssertEqual(channel.userData, 99)
        XCTAssertEqual(channel.diskQuota, 4_096)
        XCTAssertEqual(channel.maxUsers, 25)
        XCTAssertEqual(channel.transmitUsers.count, 2)
        XCTAssertEqual(channel.transmitUsers[0].userIdentifier, TeamTalkUserID(42))
        XCTAssertEqual(channel.transmitUsers[0].streamTypes, [.voice, .videoCapture])
        XCTAssertTrue(channel.transmitUsers[1].isClassroomFreeForAll)
        XCTAssertEqual(channel.transmitUsers[1].streamTypes, [.desktopInput])
        XCTAssertEqual(channel.transmitUsersQueue, [TeamTalkUserID(5), TeamTalkUserID(6), TeamTalkUserID(7)])
        XCTAssertEqual(channel.transmitUsersQueueDelayMilliseconds, 750)
        XCTAssertEqual(channel.voiceTimeoutMilliseconds, 1_500)
        XCTAssertEqual(channel.mediaFileTimeoutMilliseconds, 3_000)
        XCTAssertTrue(channel.isPasswordProtected)
    }

    func testChannelAGCAndUserVolumeStereoAccessors() {
        var rawChannel = Channel()
        rawChannel.audiocfg.bEnableAGC = 1
        rawChannel.audiocfg.nGainLevel = 15_000
        let channel = TeamTalkChannel(rawChannel)
        XCTAssertTrue(channel.isAGCEnabled)
        XCTAssertEqual(channel.gainLevel, 15_000)

        var rawUser = User()
        rawUser.nVolumeVoice = 1_000
        rawUser.nVolumeMediaFile = 2_000
        rawUser.stereoPlaybackVoice = (1, 0)
        rawUser.stereoPlaybackMediaFile = (0, 1)
        let user = TeamTalkUser(rawUser)
        XCTAssertEqual(user.voiceVolume, 1_000)
        XCTAssertEqual(user.mediaFileVolume, 2_000)
        XCTAssertTrue(user.voiceStereoLeftSpeaker)
        XCTAssertFalse(user.voiceStereoRightSpeaker)
        XCTAssertFalse(user.mediaFileStereoLeftSpeaker)
        XCTAssertTrue(user.mediaFileStereoRightSpeaker)
    }

    func testChannelTransmitUserAndQueueHelpers() {
        var rawChannel = Channel()
        rawChannel.transmitUserList = [
            TeamTalkChannelTransmitUser(userID: TeamTalkUserID(12), streamTypes: [.voice]),
            .classroomFreeForAll(streamTypes: [.videoCapture, .mediaFileAudio])
        ]
        rawChannel.transmitQueueUsers = [TeamTalkUserID(20), TeamTalkUserID(21)]
        rawChannel.transmitQueueDelayMilliseconds = 650
        rawChannel.voiceTimeoutMilliseconds = 2_500
        rawChannel.mediaFileTimeoutMilliseconds = 5_000

        XCTAssertEqual(rawChannel.transmitUserList.count, 2)
        XCTAssertEqual(rawChannel.transmitUserList[0].userIdentifier, TeamTalkUserID(12))
        XCTAssertEqual(rawChannel.transmitUserList[0].streamTypes, [.voice])
        XCTAssertTrue(rawChannel.transmitUserList[1].isClassroomFreeForAll)
        XCTAssertNil(rawChannel.transmitUserList[1].userIdentifier)
        XCTAssertEqual(rawChannel.transmitUserList[1].streamTypes, [.videoCapture, .mediaFileAudio])
        XCTAssertEqual(rawChannel.transmitQueueUsers, [TeamTalkUserID(20), TeamTalkUserID(21)])
        XCTAssertEqual(rawChannel.transmitQueueDelayMilliseconds, 650)
        XCTAssertEqual(rawChannel.voiceTimeoutMilliseconds, 2_500)
        XCTAssertEqual(rawChannel.mediaFileTimeoutMilliseconds, 5_000)
    }

    func testServerPropertiesConfigurationRoundTrip() {
        let configuration = TeamTalkServerPropertiesConfiguration(
            name: "Main Server",
            rawMessageOfTheDay: "Welcome <b>home</b>",
            maxUsers: 100,
            maxLoginAttempts: 5,
            maxLoginsPerIPAddress: 3,
            maxVoiceTransmissionBytesPerSecond: 8_000,
            maxVideoCaptureTransmissionBytesPerSecond: 16_000,
            maxMediaFileTransmissionBytesPerSecond: 24_000,
            maxDesktopTransmissionBytesPerSecond: 32_000,
            maxTotalTransmissionBytesPerSecond: 64_000,
            tcpPort: 10333,
            udpPort: 10333,
            userTimeout: 60,
            loginDelayMilliseconds: 250,
            logEvents: [.userLoggedIn, .fileUploaded],
            autoSave: true
        )

        let properties = TeamTalkServerProperties(configuration.cValue)

        XCTAssertEqual(properties.name, "Main Server")
        XCTAssertEqual(properties.rawMessageOfTheDay, "Welcome <b>home</b>")
        XCTAssertEqual(properties.maxUsers, 100)
        XCTAssertEqual(properties.maxLoginAttempts, 5)
        XCTAssertEqual(properties.maxLoginsPerIPAddress, 3)
        XCTAssertEqual(properties.maxVoiceTransmissionBytesPerSecond, 8_000)
        XCTAssertEqual(properties.maxVideoCaptureTransmissionBytesPerSecond, 16_000)
        XCTAssertEqual(properties.maxMediaFileTransmissionBytesPerSecond, 24_000)
        XCTAssertEqual(properties.maxDesktopTransmissionBytesPerSecond, 32_000)
        XCTAssertEqual(properties.maxTotalTransmissionBytesPerSecond, 64_000)
        XCTAssertEqual(properties.tcpPort, 10333)
        XCTAssertEqual(properties.udpPort, 10333)
        XCTAssertEqual(properties.userTimeout, 60)
        XCTAssertEqual(properties.loginDelayMilliseconds, 250)
        XCTAssertEqual(properties.logEvents, [.userLoggedIn, .fileUploaded])
        XCTAssertTrue(properties.hasLogEvent(.userLoggedIn))
        XCTAssertFalse(properties.hasLogEvent(.serverUpdated))
        XCTAssertTrue(configuration.hasLogEvent(.fileUploaded))
        XCTAssertTrue(properties.autoSave)
    }

    func testUserStatisticsExposeFriendlyProperties() {
        var rawStatistics = UserStatistics()
        rawStatistics.nVoicePacketsRecv = 101
        rawStatistics.nVoicePacketsLost = 7
        rawStatistics.nVideoCapturePacketsRecv = 88
        rawStatistics.nVideoCaptureFramesRecv = 44
        rawStatistics.nVideoCaptureFramesLost = 3
        rawStatistics.nVideoCaptureFramesDropped = 2
        rawStatistics.nMediaFileAudioPacketsRecv = 65
        rawStatistics.nMediaFileAudioPacketsLost = 5
        rawStatistics.nMediaFileVideoPacketsRecv = 54
        rawStatistics.nMediaFileVideoFramesRecv = 27
        rawStatistics.nMediaFileVideoFramesLost = 4
        rawStatistics.nMediaFileVideoFramesDropped = 1

        let statistics = TeamTalkUserStatistics(rawStatistics)

        XCTAssertEqual(statistics.voicePacketsReceived, 101)
        XCTAssertEqual(statistics.voicePacketsLost, 7)
        XCTAssertEqual(statistics.videoCapturePacketsReceived, 88)
        XCTAssertEqual(statistics.videoCaptureFramesReceived, 44)
        XCTAssertEqual(statistics.videoCaptureFramesLost, 3)
        XCTAssertEqual(statistics.videoCaptureFramesDropped, 2)
        XCTAssertEqual(statistics.mediaFileAudioPacketsReceived, 65)
        XCTAssertEqual(statistics.mediaFileAudioPacketsLost, 5)
        XCTAssertEqual(statistics.mediaFileVideoPacketsReceived, 54)
        XCTAssertEqual(statistics.mediaFileVideoFramesReceived, 27)
        XCTAssertEqual(statistics.mediaFileVideoFramesLost, 4)
        XCTAssertEqual(statistics.mediaFileVideoFramesDropped, 1)
        XCTAssertEqual(rawStatistics.voicePacketsReceived, 101)
    }

    func testTextMessageAssemblerRebuildsMultipartMessagesUsingTypedWrappers() {
        var assembler = TeamTalkTextMessageAssembler()

        var rawUser = User()
        rawUser.nUserID = 7
        let user = TeamTalkUser(rawUser)

        var part1 = TeamTalkOutgoingTextMessage.user(to: user, content: "Hello ").cValue
        part1.nFromUserID = 42
        part1.bMore = 1

        var part2 = TeamTalkOutgoingTextMessage.user(to: user, content: "world").cValue
        part2.nFromUserID = 42

        XCTAssertNil(assembler.append(TeamTalkTextMessage(part1)))
        XCTAssertEqual(assembler.append(TeamTalkTextMessage(part2)), "Hello world")
    }

    func testTextMessageAssemblerKeepsMessageTypesSeparated() {
        var assembler = TeamTalkTextMessageAssembler()

        var rawUser = User()
        rawUser.nUserID = 7
        let user = TeamTalkUser(rawUser)
        var privatePart = TeamTalkOutgoingTextMessage.user(to: user, content: "Private ").cValue
        privatePart.nFromUserID = 42
        privatePart.bMore = 1

        var rawChannel = Channel()
        rawChannel.nChannelID = 9
        let channel = TeamTalkChannel(rawChannel)
        var channelMessage = TeamTalkOutgoingTextMessage.channel(channel, content: "Channel").cValue
        channelMessage.nFromUserID = 42

        XCTAssertNil(assembler.append(TeamTalkTextMessage(privatePart)))
        XCTAssertEqual(assembler.append(TeamTalkTextMessage(channelMessage)), "Channel")
    }

    func testClientStatisticsExposeFriendlyProperties() {
        var rawStatistics = ClientStatistics()
        rawStatistics.nUdpBytesSent = 1000
        rawStatistics.nUdpBytesRecv = 1100
        rawStatistics.nVoiceBytesSent = 1200
        rawStatistics.nVoiceBytesRecv = 1300
        rawStatistics.nVideoCaptureBytesSent = 1400
        rawStatistics.nVideoCaptureBytesRecv = 1500
        rawStatistics.nMediaFileAudioBytesSent = 1600
        rawStatistics.nMediaFileAudioBytesRecv = 1700
        rawStatistics.nMediaFileVideoBytesSent = 1800
        rawStatistics.nMediaFileVideoBytesRecv = 1900
        rawStatistics.nDesktopBytesSent = 2000
        rawStatistics.nDesktopBytesRecv = 2100
        rawStatistics.nUdpPingTimeMs = -1
        rawStatistics.nTcpPingTimeMs = 42
        rawStatistics.nTcpServerSilenceSec = 9
        rawStatistics.nUdpServerSilenceSec = 11
        rawStatistics.nSoundInputDeviceDelayMSec = 17

        let statistics = TeamTalkClientStatistics(rawStatistics)

        XCTAssertEqual(statistics.udpBytesSent, 1000)
        XCTAssertEqual(statistics.udpBytesReceived, 1100)
        XCTAssertEqual(statistics.voiceBytesSent, 1200)
        XCTAssertEqual(statistics.voiceBytesReceived, 1300)
        XCTAssertEqual(statistics.videoCaptureBytesSent, 1400)
        XCTAssertEqual(statistics.videoCaptureBytesReceived, 1500)
        XCTAssertEqual(statistics.mediaFileAudioBytesSent, 1600)
        XCTAssertEqual(statistics.mediaFileAudioBytesReceived, 1700)
        XCTAssertEqual(statistics.mediaFileVideoBytesSent, 1800)
        XCTAssertEqual(statistics.mediaFileVideoBytesReceived, 1900)
        XCTAssertEqual(statistics.desktopBytesSent, 2000)
        XCTAssertEqual(statistics.desktopBytesReceived, 2100)
        XCTAssertNil(statistics.udpPingTimeMilliseconds)
        XCTAssertEqual(statistics.tcpPingTimeMilliseconds, 42)
        XCTAssertEqual(statistics.tcpServerSilenceSeconds, 9)
        XCTAssertEqual(statistics.udpServerSilenceSeconds, 11)
        XCTAssertEqual(statistics.soundInputDeviceDelayMilliseconds, 17)
        XCTAssertNil(rawStatistics.udpPingTimeMilliseconds)
        XCTAssertEqual(rawStatistics.tcpPingTimeMilliseconds, 42)
    }

    func testClientKeepAliveConfigurationRoundTrip() {
        let configuration = TeamTalkClientKeepAliveConfiguration(
            connectionLostMilliseconds: 15_000,
            tcpKeepAliveIntervalMilliseconds: 7_500,
            udpKeepAliveIntervalMilliseconds: 2_000,
            udpKeepAliveRetransmitMilliseconds: 500,
            udpConnectRetransmitMilliseconds: 750,
            udpConnectTimeoutMilliseconds: 10_000
        )

        let keepAlive = TeamTalkClientKeepAlive(configuration.cValue)

        XCTAssertEqual(keepAlive.connectionLostMilliseconds, 15_000)
        XCTAssertEqual(keepAlive.tcpKeepAliveIntervalMilliseconds, 7_500)
        XCTAssertEqual(keepAlive.udpKeepAliveIntervalMilliseconds, 2_000)
        XCTAssertEqual(keepAlive.udpKeepAliveRetransmitMilliseconds, 500)
        XCTAssertEqual(keepAlive.udpConnectRetransmitMilliseconds, 750)
        XCTAssertEqual(keepAlive.udpConnectTimeoutMilliseconds, 10_000)

        let roundTripped = TeamTalkClientKeepAliveConfiguration(keepAlive).cValue
        XCTAssertEqual(roundTripped.connectionLostMilliseconds, 15_000)
        XCTAssertEqual(roundTripped.tcpKeepAliveIntervalMilliseconds, 7_500)
        XCTAssertEqual(roundTripped.udpKeepAliveIntervalMilliseconds, 2_000)
        XCTAssertEqual(roundTripped.udpKeepAliveRetransmitMilliseconds, 500)
        XCTAssertEqual(roundTripped.udpConnectRetransmitMilliseconds, 750)
        XCTAssertEqual(roundTripped.udpConnectTimeoutMilliseconds, 10_000)
    }

    func testSoundDeviceWrapperExposesFriendlyProperties() {
        var rawDevice = SoundDevice()
        rawDevice.nDeviceID = TeamTalkSoundDeviceID.voiceProcessingIO.cValue
        rawDevice.nSoundSystem = SOUNDSYSTEM_AUDIOUNIT
        rawDevice.nWaveDeviceID = -1
        rawDevice.nMaxInputChannels = 2
        rawDevice.nMaxOutputChannels = 2
        rawDevice.nDefaultSampleRate = 48_000
        rawDevice.uSoundDeviceFeatures = TeamTalkSoundDeviceFeatures([
            .echoCancellation,
            .automaticGainControl,
            .denoise,
            .defaultCommunicationDevice
        ]).cValue

        writeCString("Voice Processing I/O", to: &rawDevice.szDeviceName)
        writeCString("com.apple.audio.voice-processing", to: &rawDevice.szDeviceID)
        writeInt32Array([48_000, 44_100], to: &rawDevice.inputSampleRates)
        writeInt32Array([48_000], to: &rawDevice.outputSampleRates)

        let device = TeamTalkSoundDevice(rawDevice)

        XCTAssertEqual(rawDevice.id, TeamTalkSoundDeviceID.voiceProcessingIO.cValue)
        XCTAssertEqual(rawDevice.soundDeviceID, .voiceProcessingIO)
        XCTAssertEqual(rawDevice.physicalDeviceID, TeamTalkSoundDeviceID(1))
        XCTAssertEqual(rawDevice.name, "Voice Processing I/O")
        XCTAssertEqual(rawDevice.deviceIdentifier, "com.apple.audio.voice-processing")
        XCTAssertEqual(rawDevice.soundSystem, .audioUnit)
        XCTAssertEqual(rawDevice.waveDeviceID, -1)
        XCTAssertEqual(rawDevice.maxInputChannels, 2)
        XCTAssertEqual(rawDevice.maxOutputChannels, 2)
        XCTAssertEqual(rawDevice.supportedInputSampleRates, [48_000, 44_100])
        XCTAssertEqual(rawDevice.supportedOutputSampleRates, [48_000])
        XCTAssertEqual(rawDevice.defaultSampleRate, 48_000)
        XCTAssertEqual(rawDevice.features, [.echoCancellation, .automaticGainControl, .denoise, .defaultCommunicationDevice])
        XCTAssertTrue(rawDevice.isShared)
        XCTAssertTrue(rawDevice.supportsEchoCancellation)
        XCTAssertTrue(rawDevice.supportsAutomaticGainControl)
        XCTAssertTrue(rawDevice.supportsDenoise)
        XCTAssertFalse(rawDevice.supportsDuplexMode)
        XCTAssertTrue(rawDevice.isDefaultCommunicationDevice)
        XCTAssertTrue(rawDevice.hasFeature(.denoise))

        XCTAssertEqual(device.id, TeamTalkSoundDeviceID.voiceProcessingIO.cValue)
        XCTAssertEqual(device.soundDeviceID, .voiceProcessingIO)
        XCTAssertEqual(device.physicalDeviceID, TeamTalkSoundDeviceID(1))
        XCTAssertEqual(device.name, "Voice Processing I/O")
        XCTAssertEqual(device.deviceIdentifier, "com.apple.audio.voice-processing")
        XCTAssertEqual(device.soundSystem, .audioUnit)
        XCTAssertEqual(device.waveDeviceID, -1)
        XCTAssertEqual(device.maxInputChannels, 2)
        XCTAssertEqual(device.maxOutputChannels, 2)
        XCTAssertEqual(device.inputSampleRates, [48_000, 44_100])
        XCTAssertEqual(device.outputSampleRates, [48_000])
        XCTAssertEqual(device.defaultSampleRate, 48_000)
        XCTAssertEqual(device.features, [.echoCancellation, .automaticGainControl, .denoise, .defaultCommunicationDevice])
        XCTAssertTrue(device.isShared)
        XCTAssertTrue(device.supportsEchoCancellation)
        XCTAssertTrue(device.supportsAutomaticGainControl)
        XCTAssertTrue(device.supportsDenoise)
        XCTAssertFalse(device.supportsDuplexMode)
        XCTAssertTrue(device.isDefaultCommunicationDevice)
    }

    func testSoundDeviceEffectsConfigurationRoundTrip() {
        var rawEffects = SoundDeviceEffects()
        XCTAssertFalse(rawEffects.automaticGainControlEnabled)
        XCTAssertFalse(rawEffects.denoiseEnabled)
        XCTAssertFalse(rawEffects.echoCancellationEnabled)

        rawEffects.automaticGainControlEnabled = true
        rawEffects.denoiseEnabled = true
        XCTAssertEqual(rawEffects.enabledEffects, [.automaticGainControl, .denoise])
        XCTAssertTrue(rawEffects.hasEffect(.automaticGainControl))
        XCTAssertFalse(rawEffects.hasEffect(.echoCancellation))

        rawEffects.enabledEffects = [.echoCancellation]
        XCTAssertFalse(rawEffects.automaticGainControlEnabled)
        XCTAssertFalse(rawEffects.denoiseEnabled)
        XCTAssertTrue(rawEffects.echoCancellationEnabled)

        var configuration = TeamTalkSoundDeviceEffectsConfiguration(
            automaticGainControlEnabled: true,
            denoiseEnabled: false,
            echoCancellationEnabled: true
        )
        XCTAssertEqual(configuration.enabledEffects, [.automaticGainControl, .echoCancellation])

        configuration.enabledEffects = [.denoise]
        XCTAssertFalse(configuration.automaticGainControlEnabled)
        XCTAssertTrue(configuration.denoiseEnabled)
        XCTAssertFalse(configuration.echoCancellationEnabled)

        let effects = TeamTalkSoundDeviceEffects(configuration.cValue)
        XCTAssertFalse(effects.automaticGainControlEnabled)
        XCTAssertTrue(effects.denoiseEnabled)
        XCTAssertFalse(effects.echoCancellationEnabled)
        XCTAssertEqual(effects.enabledEffects, [.denoise])
        XCTAssertTrue(effects.hasEffect(.denoise))

        let roundTripped = TeamTalkSoundDeviceEffectsConfiguration(effects)
        XCTAssertFalse(roundTripped.automaticGainControlEnabled)
        XCTAssertTrue(roundTripped.denoiseEnabled)
        XCTAssertFalse(roundTripped.echoCancellationEnabled)
    }

    func testSharedSoundDeviceConfigurationHelpers() {
        let defaults = TeamTalkSharedSoundDeviceConfiguration()
        XCTAssertEqual(defaults.sampleRate, 0)
        XCTAssertEqual(defaults.channels, 0)
        XCTAssertEqual(defaults.frameSize, 0)
        XCTAssertTrue(defaults.usesDefaultSampleRate)
        XCTAssertTrue(defaults.usesDefaultChannels)
        XCTAssertTrue(defaults.usesDefaultFrameSize)
        XCTAssertTrue(defaults.usesDefaultValues)

        let custom = TeamTalkSharedSoundDeviceConfiguration(sampleRate: 48_000, channels: 2, frameSize: 1_920)
        XCTAssertEqual(custom.sampleRate, 48_000)
        XCTAssertEqual(custom.channels, 2)
        XCTAssertEqual(custom.frameSize, 1_920)
        XCTAssertFalse(custom.usesDefaultSampleRate)
        XCTAssertFalse(custom.usesDefaultChannels)
        XCTAssertFalse(custom.usesDefaultFrameSize)
        XCTAssertFalse(custom.usesDefaultValues)
    }

    func testSoundDuplexConfigurationPreservesTypedDeviceIDs() {
        let rawInput = TeamTalkSoundDeviceID.voiceProcessingIO
        let rawOutput = TeamTalkSoundDeviceID.remoteIO

        let identifiersConfiguration = TeamTalkSoundDuplexConfiguration(
            inputDeviceID: rawInput,
            outputDeviceID: rawOutput
        )

        XCTAssertEqual(identifiersConfiguration.inputDeviceID, rawInput)
        XCTAssertEqual(identifiersConfiguration.outputDeviceID, rawOutput)
        XCTAssertEqual(identifiersConfiguration.inputDeviceRawID, rawInput.cValue)
        XCTAssertEqual(identifiersConfiguration.outputDeviceRawID, rawOutput.cValue)

        var rawInputDevice = SoundDevice()
        rawInputDevice.nDeviceID = rawInput.cValue
        var rawOutputDevice = SoundDevice()
        rawOutputDevice.nDeviceID = rawOutput.cValue

        let devicesConfiguration = TeamTalkSoundDuplexConfiguration(
            inputDevice: TeamTalkSoundDevice(rawInputDevice),
            outputDevice: TeamTalkSoundDevice(rawOutputDevice)
        )

        XCTAssertEqual(devicesConfiguration.inputDeviceID, rawInput)
        XCTAssertEqual(devicesConfiguration.outputDeviceID, rawOutput)
    }

    func testDesktopWindowAndInputWrappersExposeFriendlyProperties() {
        let pixels = Data([0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80])
        var rawWindow = DesktopWindow()
        rawWindow.nWidth = 640
        rawWindow.nHeight = 480
        rawWindow.bmpFormat = BMP_RGB32
        rawWindow.nBytesPerLine = 2560
        rawWindow.nSessionID = 13
        rawWindow.nProtocol = DESKTOPPROTOCOL_ZLIB_1
        rawWindow.nFrameBufferSize = Int32(pixels.count)

        pixels.withUnsafeBytes { buffer in
            rawWindow.frameBuffer = UnsafeMutableRawPointer(mutating: buffer.baseAddress)

            XCTAssertEqual(rawWindow.width, 640)
            XCTAssertEqual(rawWindow.height, 480)
            XCTAssertEqual(rawWindow.bitmapFormat, .rgb32)
            XCTAssertEqual(rawWindow.bytesPerLine, 2560)
            XCTAssertEqual(rawWindow.sessionID, 13)
            XCTAssertEqual(rawWindow.sessionIdentifier, TeamTalkDesktopSessionID(13))
            XCTAssertEqual(rawWindow.desktopProtocol, .zlib1)
            XCTAssertEqual(rawWindow.frameBufferSize, Int32(pixels.count))
            XCTAssertTrue(rawWindow.hasFrameBuffer)
            XCTAssertEqual(rawWindow.frameBufferData, pixels)

            let window = TeamTalkDesktopWindow(rawWindow)
            XCTAssertEqual(window.width, 640)
            XCTAssertEqual(window.height, 480)
            XCTAssertEqual(window.bitmapFormat, .rgb32)
            XCTAssertEqual(window.bytesPerLine, 2560)
            XCTAssertEqual(window.sessionID, 13)
            XCTAssertEqual(window.sessionIdentifier, TeamTalkDesktopSessionID(13))
            XCTAssertEqual(window.desktopProtocol, .zlib1)
            XCTAssertEqual(window.frameBuffer, pixels)
            XCTAssertEqual(window.frameBufferSize, Int32(pixels.count))
            XCTAssertTrue(window.hasFrameBuffer)

            window.withUnsafeCValue { roundTrippedWindow in
                XCTAssertEqual(roundTrippedWindow.width, 640)
                XCTAssertEqual(roundTrippedWindow.height, 480)
                XCTAssertEqual(roundTrippedWindow.bitmapFormat, .rgb32)
                XCTAssertEqual(roundTrippedWindow.bytesPerLine, 2560)
                XCTAssertEqual(roundTrippedWindow.sessionID, 13)
                XCTAssertEqual(roundTrippedWindow.sessionIdentifier, TeamTalkDesktopSessionID(13))
                XCTAssertEqual(roundTrippedWindow.desktopProtocol, .zlib1)
                XCTAssertEqual(roundTrippedWindow.frameBufferData, pixels)
            }
        }

        var rawInput = DesktopInput()
        rawInput.mousePosition = nil
        rawInput.keyCode = .ignore
        rawInput.keyState = .none
        XCTAssertNil(rawInput.mousePosition)
        XCTAssertTrue(rawInput.ignoresKeyCode)
        XCTAssertFalse(rawInput.hasMousePosition)

        rawInput.mousePosition = (x: 320, y: 240)
        rawInput.keyCode = .leftMouseButton
        rawInput.keyState = [.down]
        XCTAssertEqual(rawInput.mousePositionX, 320)
        XCTAssertEqual(rawInput.mousePositionY, 240)
        XCTAssertEqual(rawInput.mousePosition?.x, 320)
        XCTAssertEqual(rawInput.mousePosition?.y, 240)
        XCTAssertEqual(rawInput.keyCode, .leftMouseButton)
        XCTAssertEqual(rawInput.keyState, [.down])
        XCTAssertTrue(rawInput.hasMousePosition)
        XCTAssertFalse(rawInput.ignoresKeyCode)

        let input = TeamTalkDesktopInput(rawInput)
        XCTAssertEqual(TeamTalkDesktopInput.maximumCount, Int(TT_DESKTOPINPUT_MAX))
        XCTAssertEqual(input.mousePositionX, 320)
        XCTAssertEqual(input.mousePositionY, 240)
        XCTAssertEqual(input.mousePosition?.x, 320)
        XCTAssertEqual(input.mousePosition?.y, 240)
        XCTAssertEqual(input.keyCode, .leftMouseButton)
        XCTAssertEqual(input.keyState, [.down])
        XCTAssertTrue(input.hasMousePosition)
        XCTAssertFalse(input.ignoresKeyCode)

        let configuredInput = TeamTalkDesktopInput(
            mousePositionX: 100,
            mousePositionY: 200,
            keyCode: .rightMouseButton,
            keyState: [.up]
        )
        XCTAssertEqual(configuredInput.cValue.mousePositionX, 100)
        XCTAssertEqual(configuredInput.cValue.mousePositionY, 200)
        XCTAssertEqual(configuredInput.cValue.keyCode, .rightMouseButton)
        XCTAssertEqual(configuredInput.cValue.keyState, [.up])
    }

    func testVideoCaptureDeviceWrapperExposesFriendlyProperties() {
        var hdFormat = VideoFormat()
        hdFormat.nWidth = 1280
        hdFormat.nHeight = 720
        hdFormat.nFPS_Numerator = 30
        hdFormat.nFPS_Denominator = 1
        hdFormat.picFourCC = FOURCC_RGB32

        var sdFormat = VideoFormat()
        sdFormat.nWidth = 640
        sdFormat.nHeight = 480
        sdFormat.nFPS_Numerator = 15
        sdFormat.nFPS_Denominator = 1
        sdFormat.picFourCC = FOURCC_I420

        var rawDevice = VideoCaptureDevice()
        writeCString("camera-1", to: &rawDevice.szDeviceID)
        writeCString("FaceTime HD Camera", to: &rawDevice.szDeviceName)
        writeCString("AVFoundation", to: &rawDevice.szCaptureAPI)
        rawDevice.nVideoFormatsCount = 2
        writeStructArray([hdFormat, sdFormat], to: &rawDevice.videoFormats)

        let device = TeamTalkVideoCaptureDevice(rawDevice)

        XCTAssertEqual(rawDevice.deviceIdentifier, "camera-1")
        XCTAssertEqual(rawDevice.name, "FaceTime HD Camera")
        XCTAssertEqual(rawDevice.captureAPI, "AVFoundation")
        XCTAssertEqual(rawDevice.videoFormatsCount, 2)
        XCTAssertTrue(rawDevice.hasFormats)
        XCTAssertEqual(rawDevice.supportedFormats.count, 2)
        XCTAssertEqual(rawDevice.supportedFormats[0].width, 1280)
        XCTAssertEqual(rawDevice.supportedFormats[0].height, 720)
        XCTAssertEqual(rawDevice.supportedFormats[0].pixelFormat, .rgb32)
        XCTAssertEqual(rawDevice.supportedFormats[1].width, 640)
        XCTAssertEqual(rawDevice.supportedFormats[1].height, 480)
        XCTAssertEqual(rawDevice.supportedFormats[1].pixelFormat, .i420)

        XCTAssertEqual(device.id, "camera-1")
        XCTAssertEqual(device.deviceIdentifier, "camera-1")
        XCTAssertEqual(device.name, "FaceTime HD Camera")
        XCTAssertEqual(device.captureAPI, "AVFoundation")
        XCTAssertEqual(device.videoFormatsCount, 2)
        XCTAssertTrue(device.hasVideoFormats)
        XCTAssertEqual(device.videoFormats.count, 2)
        XCTAssertEqual(device.defaultVideoFormat?.width, 1280)
        XCTAssertEqual(device.defaultVideoFormat?.height, 720)
        XCTAssertEqual(device.defaultVideoFormat?.pixelFormat, .rgb32)
        XCTAssertEqual(device.videoFormats[1].width, 640)
        XCTAssertEqual(device.videoFormats[1].height, 480)
        XCTAssertEqual(device.videoFormats[1].pixelFormat, .i420)
    }

    func testAudioFormatConfigurationAndAudioBlockSnapshotHelpers() {
        let audioFormatConfiguration = TeamTalkAudioFormatConfiguration(
            format: .wave,
            sampleRate: 48_000,
            channels: 2
        )
        XCTAssertTrue(audioFormatConfiguration.isValid)
        XCTAssertEqual(audioFormatConfiguration.cValue.format, .wave)
        XCTAssertEqual(audioFormatConfiguration.cValue.sampleRate, 48_000)
        XCTAssertEqual(audioFormatConfiguration.cValue.channels, 2)

        let pcmSamples: [Int16] = [100, -100, 200, -200]
        let rawAudio = pcmSamples.withUnsafeBufferPointer { buffer in Data(buffer: buffer) }
        var rawBlock = AudioBlock()
        rawBlock.nStreamID = 77
        rawBlock.nSampleRate = 48_000
        rawBlock.nChannels = 2
        rawBlock.nSamples = 2
        rawBlock.uSampleIndex = 123
        rawBlock.uStreamTypes = TeamTalkStreamTypes([.voice, .mediaFileAudio]).cMask
        rawAudio.withUnsafeBytes { buffer in
            rawBlock.lpRawAudio = UnsafeMutableRawPointer(mutating: buffer.baseAddress)

            XCTAssertEqual(rawBlock.streamID, 77)
            XCTAssertEqual(rawBlock.sampleRate, 48_000)
            XCTAssertEqual(rawBlock.channels, 2)
            XCTAssertEqual(rawBlock.samplesPerChannel, 2)
            XCTAssertEqual(rawBlock.totalSampleCount, 4)
            XCTAssertEqual(rawBlock.rawAudioByteCount, 8)
            XCTAssertEqual(rawBlock.rawAudioData, rawAudio)
            XCTAssertEqual(rawBlock.pcmSamples, pcmSamples)
            XCTAssertEqual(rawBlock.sampleIndex, 123)
            XCTAssertEqual(rawBlock.streamTypes, [.voice, .mediaFileAudio])
            XCTAssertEqual(rawBlock.durationMilliseconds ?? 0, 2.0 / 48.0, accuracy: 0.0001)

            let snapshot = TeamTalkAudioBlock(rawBlock)
            XCTAssertEqual(snapshot.streamID, 77)
            XCTAssertEqual(snapshot.sampleRate, 48_000)
            XCTAssertEqual(snapshot.channels, 2)
            XCTAssertEqual(snapshot.samplesPerChannel, 2)
            XCTAssertEqual(snapshot.sampleIndex, 123)
            XCTAssertEqual(snapshot.streamTypes, [.voice, .mediaFileAudio])
            XCTAssertEqual(snapshot.rawAudio, rawAudio)
            XCTAssertEqual(snapshot.totalSampleCount, 4)
            XCTAssertEqual(snapshot.pcmSamples, pcmSamples)
            XCTAssertEqual(snapshot.durationMilliseconds ?? 0, 2.0 / 48.0, accuracy: 0.0001)

            snapshot.withUnsafeCValue { roundTripped in
                XCTAssertEqual(roundTripped.streamID, 77)
                XCTAssertEqual(roundTripped.sampleRate, 48_000)
                XCTAssertEqual(roundTripped.channels, 2)
                XCTAssertEqual(roundTripped.samplesPerChannel, 2)
                XCTAssertEqual(roundTripped.sampleIndex, 123)
                XCTAssertEqual(roundTripped.streamTypes, [.voice, .mediaFileAudio])
                XCTAssertEqual(roundTripped.pcmSamples, pcmSamples)
            }
        }

        let insertedBlock = TeamTalkAudioBlock(
            streamID: 91,
            sampleRate: 44_100,
            channels: 1,
            sampleIndex: 456,
            streamTypes: [.voice],
            pcmSamples: [10, 20, 30, 40]
        )
        XCTAssertEqual(insertedBlock.samplesPerChannel, 4)
        XCTAssertEqual(insertedBlock.totalSampleCount, 4)
        XCTAssertEqual(insertedBlock.pcmSamples, [10, 20, 30, 40])
        XCTAssertEqual(insertedBlock.durationMilliseconds ?? 0, 4.0 * 1000.0 / 44_100.0, accuracy: 0.0001)
    }

    func testMediaFileInfoAndFormatsExposeFriendlyProperties() {
        var rawMediaInfo = MediaFileInfo()
        rawMediaInfo.nStatus = MFS_PLAYING
        rawMediaInfo.audioFmt.nAudioFmt = AFF_MP3_128KBIT_FORMAT
        rawMediaInfo.audioFmt.nSampleRate = 48_000
        rawMediaInfo.audioFmt.nChannels = 2
        rawMediaInfo.videoFmt.nWidth = 1_280
        rawMediaInfo.videoFmt.nHeight = 720
        rawMediaInfo.videoFmt.nFPS_Numerator = 30
        rawMediaInfo.videoFmt.nFPS_Denominator = 1
        rawMediaInfo.videoFmt.picFourCC = FOURCC_RGB32
        rawMediaInfo.uDurationMSec = 120_000
        rawMediaInfo.uElapsedMSec = 30_000
        writeCString("demo.mp4", to: &rawMediaInfo.szFileName)

        let mediaInfo = TeamTalkMediaFileInfo(rawMediaInfo)

        XCTAssertEqual(rawMediaInfo.status, .playing)
        XCTAssertEqual(rawMediaInfo.fileName, "demo.mp4")
        XCTAssertEqual(rawMediaInfo.durationMilliseconds, 120_000)
        XCTAssertEqual(rawMediaInfo.elapsedMilliseconds, 30_000)
        XCTAssertEqual(rawMediaInfo.progress, 0.25, accuracy: 0.0001)

        XCTAssertEqual(mediaInfo.status, .playing)
        XCTAssertEqual(mediaInfo.fileName, "demo.mp4")
        XCTAssertEqual(mediaInfo.durationMilliseconds, 120_000)
        XCTAssertEqual(mediaInfo.elapsedMilliseconds, 30_000)
        XCTAssertEqual(mediaInfo.progress, 0.25, accuracy: 0.0001)

        XCTAssertEqual(mediaInfo.audioFormat.format, .mp3_128kbit)
        XCTAssertEqual(mediaInfo.audioFormat.sampleRate, 48_000)
        XCTAssertEqual(mediaInfo.audioFormat.channels, 2)
        XCTAssertTrue(mediaInfo.audioFormat.isAvailable)

        XCTAssertEqual(mediaInfo.videoFormat.width, 1_280)
        XCTAssertEqual(mediaInfo.videoFormat.height, 720)
        XCTAssertEqual(mediaInfo.videoFormat.frameRateNumerator, 30)
        XCTAssertEqual(mediaInfo.videoFormat.frameRateDenominator, 1)
        XCTAssertEqual(mediaInfo.videoFormat.pixelFormat, .rgb32)
        XCTAssertEqual(mediaInfo.videoFormat.framesPerSecond ?? 0, 30, accuracy: 0.0001)
    }

    func testMediaFilePlaybackConfigurationRoundTrip() {
        var rawPlayback = MediaFilePlayback()
        rawPlayback.offsetMilliseconds = nil
        rawPlayback.isPaused = true
        rawPlayback.preprocessor = TeamTalkAudioPreprocessor.makeWebRTCPreprocessor()

        XCTAssertNil(rawPlayback.offsetMilliseconds)
        XCTAssertTrue(rawPlayback.isPaused)
        XCTAssertEqual(rawPlayback.preprocessorType, .webRTC)

        let playback = TeamTalkMediaFilePlayback(rawPlayback)
        XCTAssertNil(playback.offsetMilliseconds)
        XCTAssertTrue(playback.isPaused)
        XCTAssertEqual(playback.audioPreprocessorType, .webRTC)

        var configuration = TeamTalkMediaFilePlaybackConfiguration(playback)
        XCTAssertNil(configuration.offsetMilliseconds)
        XCTAssertTrue(configuration.isPaused)
        XCTAssertEqual(configuration.audioPreprocessor.type, .webRTC)

        configuration.offsetMilliseconds = 5_000
        configuration.isPaused = false
        configuration.audioPreprocessor = TeamTalkAudioPreprocessor.makeTeamTalkPreprocessor()

        let roundTripped = TeamTalkMediaFilePlayback(configuration.cValue)
        XCTAssertEqual(roundTripped.offsetMilliseconds, 5_000)
        XCTAssertFalse(roundTripped.isPaused)
        XCTAssertEqual(roundTripped.audioPreprocessorType, .teamTalk)
    }

    func testAudioPreprocessorConfigurationsRoundTrip() {
        var rawSpeex = TeamTalkAudioPreprocessor.makeSpeexDSP()
        XCTAssertFalse(rawSpeex.automaticGainControlEnabled)
        XCTAssertFalse(rawSpeex.denoiseEnabled)
        XCTAssertFalse(rawSpeex.echoCancellationEnabled)

        rawSpeex.automaticGainControlEnabled = true
        rawSpeex.gainLevel = 9_000
        rawSpeex.maxGainIncreaseDecibelsPerSecond = 18
        rawSpeex.maxGainDecreaseDecibelsPerSecond = -24
        rawSpeex.maxGainDecibels = 40
        rawSpeex.denoiseEnabled = true
        rawSpeex.maxNoiseSuppressionDecibels = -35
        rawSpeex.echoCancellationEnabled = true
        rawSpeex.echoSuppressionDecibels = -42
        rawSpeex.echoSuppressionActiveDecibels = -18

        let speexConfiguration = TeamTalkSpeexDSPConfiguration(rawSpeex)
        XCTAssertTrue(speexConfiguration.automaticGainControlEnabled)
        XCTAssertEqual(speexConfiguration.gainLevel, 9_000)
        XCTAssertEqual(speexConfiguration.maxGainIncreaseDecibelsPerSecond, 18)
        XCTAssertEqual(speexConfiguration.maxGainDecreaseDecibelsPerSecond, -24)
        XCTAssertEqual(speexConfiguration.maxGainDecibels, 40)
        XCTAssertTrue(speexConfiguration.denoiseEnabled)
        XCTAssertEqual(speexConfiguration.maxNoiseSuppressionDecibels, -35)
        XCTAssertTrue(speexConfiguration.echoCancellationEnabled)
        XCTAssertEqual(speexConfiguration.echoSuppressionDecibels, -42)
        XCTAssertEqual(speexConfiguration.echoSuppressionActiveDecibels, -18)
        XCTAssertEqual(speexConfiguration.cValue.gainLevel, 9_000)
        XCTAssertTrue(speexConfiguration.cValue.echoCancellationEnabled)

        var rawTeamTalk = TeamTalkAudioPreprocessor.makeTTAudioPreprocessor()
        rawTeamTalk.gainLevel = 1_500
        rawTeamTalk.muteLeftSpeaker = true
        rawTeamTalk.muteRightSpeaker = false

        let teamTalkConfiguration = TeamTalkTTAudioPreprocessorConfiguration(rawTeamTalk)
        XCTAssertEqual(teamTalkConfiguration.gainLevel, 1_500)
        XCTAssertTrue(teamTalkConfiguration.muteLeftSpeaker)
        XCTAssertFalse(teamTalkConfiguration.muteRightSpeaker)
        XCTAssertTrue(teamTalkConfiguration.cValue.muteLeftSpeaker)

        var rawWebRTC = TeamTalkAudioPreprocessor.makeWebRTCAudioPreprocessor()
        rawWebRTC.preamplifierEnabled = true
        rawWebRTC.preamplifierFixedGainFactor = 1.75
        rawWebRTC.echoCancellerEnabled = true
        rawWebRTC.noiseSuppressionEnabled = true
        rawWebRTC.noiseSuppressionLevel = .veryHigh
        rawWebRTC.gainController2Enabled = true
        rawWebRTC.fixedDigitalGainDecibels = 20
        rawWebRTC.adaptiveDigitalGainEnabled = true
        rawWebRTC.adaptiveHeadRoomDecibels = 6
        rawWebRTC.adaptiveMaxGainDecibels = 55
        rawWebRTC.adaptiveInitialGainDecibels = 14
        rawWebRTC.adaptiveMaxGainChangeDecibelsPerSecond = 5
        rawWebRTC.adaptiveMaxOutputNoiseLevelDecibelsFS = -48

        let webRTCConfiguration = TeamTalkWebRTCAudioPreprocessorConfiguration(rawWebRTC)
        XCTAssertTrue(webRTCConfiguration.preamplifierEnabled)
        XCTAssertEqual(webRTCConfiguration.preamplifierFixedGainFactor, 1.75, accuracy: 0.0001)
        XCTAssertTrue(webRTCConfiguration.echoCancellerEnabled)
        XCTAssertTrue(webRTCConfiguration.noiseSuppressionEnabled)
        XCTAssertEqual(webRTCConfiguration.noiseSuppressionLevel, .veryHigh)
        XCTAssertTrue(webRTCConfiguration.gainController2Enabled)
        XCTAssertEqual(webRTCConfiguration.fixedDigitalGainDecibels, 20, accuracy: 0.0001)
        XCTAssertTrue(webRTCConfiguration.adaptiveDigitalGainEnabled)
        XCTAssertEqual(webRTCConfiguration.adaptiveHeadRoomDecibels, 6, accuracy: 0.0001)
        XCTAssertEqual(webRTCConfiguration.adaptiveMaxGainDecibels, 55, accuracy: 0.0001)
        XCTAssertEqual(webRTCConfiguration.adaptiveInitialGainDecibels, 14, accuracy: 0.0001)
        XCTAssertEqual(webRTCConfiguration.adaptiveMaxGainChangeDecibelsPerSecond, 5, accuracy: 0.0001)
        XCTAssertEqual(webRTCConfiguration.adaptiveMaxOutputNoiseLevelDecibelsFS, -48, accuracy: 0.0001)
        XCTAssertEqual(webRTCConfiguration.cValue.noiseSuppressionLevel, .veryHigh)
        XCTAssertTrue(webRTCConfiguration.cValue.echoCancellerEnabled)

        let teamTalkPreprocessor = TeamTalkAudioPreprocessorConfiguration.teamTalk(teamTalkConfiguration)
        XCTAssertEqual(teamTalkPreprocessor.type, .teamTalk)
        XCTAssertEqual(teamTalkPreprocessor.cValue.type, .teamTalk)

        let webRTCPreprocessor = TeamTalkAudioPreprocessorConfiguration.webRTC(webRTCConfiguration)
        XCTAssertEqual(webRTCPreprocessor.type, .webRTC)
        XCTAssertEqual(webRTCPreprocessor.cValue.type, .webRTC)
        XCTAssertEqual(webRTCPreprocessor.cValue.webRTCAudioPreprocessor.noiseSuppressionLevel, .veryHigh)

        let restoredPreprocessor = TeamTalkAudioPreprocessorConfiguration(webRTCPreprocessor.cValue)
        guard case .webRTC(let restoredWebRTCConfiguration) = restoredPreprocessor else {
            XCTFail("Expected WebRTC audio preprocessor configuration")
            return
        }
        XCTAssertEqual(restoredWebRTCConfiguration, webRTCConfiguration)

        XCTAssertEqual(TeamTalkAudioPreprocessorConfiguration(type: .none).type, .none)
        XCTAssertEqual(TeamTalkAudioPreprocessorConfiguration(type: .obsoleteWebRTC).type, .obsoleteWebRTC)

        var playbackConfiguration = TeamTalkMediaFilePlaybackConfiguration()
        playbackConfiguration.audioPreprocessorConfiguration = .speexDSP(speexConfiguration)
        XCTAssertEqual(playbackConfiguration.audioPreprocessor.type, .speexDSP)
        XCTAssertEqual(playbackConfiguration.audioPreprocessorConfiguration, .speexDSP(speexConfiguration))
    }

    func testAudioCodecConfigurationsRoundTrip() {
        var opusConfiguration = TeamTalkOpusCodecConfiguration()
        opusConfiguration.sampleRate = 48_000
        opusConfiguration.channels = 2
        opusConfiguration.application = TeamTalkOpusCodecConfiguration.applicationAudio
        opusConfiguration.bitrate = 32_000
        opusConfiguration.variableBitrateEnabled = true
        opusConfiguration.discontinuousTransmissionEnabled = true
        opusConfiguration.frameSizeMilliseconds = 40
        opusConfiguration.transmitIntervalMilliseconds = 40

        XCTAssertEqual(opusConfiguration.cValue.nSampleRate, 48_000)
        XCTAssertEqual(opusConfiguration.cValue.nChannels, 2)
        XCTAssertEqual(opusConfiguration.cValue.bVBR, 1)

        let restoredOpus = TeamTalkOpusCodecConfiguration(opusConfiguration.cValue)
        XCTAssertEqual(restoredOpus, opusConfiguration)

        var speexConfiguration = TeamTalkSpeexCodecConfiguration()
        speexConfiguration.bandmode = 1
        speexConfiguration.quality = 7
        speexConfiguration.transmitIntervalMilliseconds = 40
        speexConfiguration.stereoPlaybackEnabled = true

        let restoredSpeex = TeamTalkSpeexCodecConfiguration(speexConfiguration.cValue)
        XCTAssertEqual(restoredSpeex, speexConfiguration)

        var speexVBRConfiguration = TeamTalkSpeexVBRCodecConfiguration()
        speexVBRConfiguration.bandmode = 2
        speexVBRConfiguration.quality = 9
        speexVBRConfiguration.bitrate = 28_000
        speexVBRConfiguration.maxBitrate = 42_200
        speexVBRConfiguration.discontinuousTransmissionEnabled = true
        speexVBRConfiguration.transmitIntervalMilliseconds = 40

        let restoredSpeexVBR = TeamTalkSpeexVBRCodecConfiguration(speexVBRConfiguration.cValue)
        XCTAssertEqual(restoredSpeexVBR, speexVBRConfiguration)

        let opusCodec = TeamTalkAudioCodecConfiguration.opus(opusConfiguration)
        XCTAssertEqual(opusCodec.codec, .opus)
        XCTAssertEqual(opusCodec.cValue.nCodec, TeamTalkC.OPUS_CODEC)

        let restoredOpusCodec = TeamTalkAudioCodecConfiguration(opusCodec.cValue)
        guard case .opus(let restoredOpusConfiguration) = restoredOpusCodec else {
            XCTFail("Expected OPUS audio codec configuration")
            return
        }
        XCTAssertEqual(restoredOpusConfiguration, opusConfiguration)

        let speexVBRCodec = TeamTalkAudioCodecConfiguration.speexVBR(speexVBRConfiguration)
        XCTAssertEqual(speexVBRCodec.codec, .speexVBR)
        XCTAssertEqual(speexVBRCodec.cValue.nCodec, TeamTalkC.SPEEX_VBR_CODEC)

        XCTAssertEqual(TeamTalkAudioCodecConfiguration.none.codec, .none)
        XCTAssertEqual(TeamTalkAudioCodecConfiguration.none.cValue.nCodec, TeamTalkC.NO_CODEC)
    }

    func testUserMediaStorageConfigurationHelpers() {
        let defaults = TeamTalkUserMediaStorageConfiguration()
        XCTAssertNil(defaults.directoryURL)
        XCTAssertEqual(defaults.fileNamePattern, "")
        XCTAssertEqual(defaults.audioFileFormat, .none)
        XCTAssertNil(defaults.stopRecordingExtraDelayMilliseconds)
        XCTAssertTrue(defaults.isDisabled)
        XCTAssertTrue(defaults.usesDefaultFileNamePattern)

        let directoryURL = URL(fileURLWithPath: "/tmp/recordings")
        let custom = TeamTalkUserMediaStorageConfiguration(
            directoryURL: directoryURL,
            fileNamePattern: "%username%-%counter%",
            audioFileFormat: .wave,
            stopRecordingExtraDelayMilliseconds: 750
        )
        XCTAssertEqual(custom.directoryURL, directoryURL)
        XCTAssertEqual(custom.fileNamePattern, "%username%-%counter%")
        XCTAssertEqual(custom.audioFileFormat, .wave)
        XCTAssertEqual(custom.stopRecordingExtraDelayMilliseconds, 750)
        XCTAssertFalse(custom.isDisabled)
        XCTAssertFalse(custom.usesDefaultFileNamePattern)
    }

    func testBanConfigurationRoundTrip() {
        var configuration = TeamTalkBanConfiguration(
            ipAddress: "192.168.1.*",
            channelPathValue: TeamTalkChannelPath("/Lobby"),
            username: "guest",
            types: [.ipAddress, .username, .channel]
        )

        XCTAssertEqual(configuration.channelPath, "/Lobby")
        XCTAssertEqual(configuration.channelPathValue, TeamTalkChannelPath("/Lobby"))
        XCTAssertTrue(configuration.isChannelBan)
        XCTAssertFalse(configuration.isLoginBan)
        XCTAssertTrue(configuration.isIPAddressBan)
        XCTAssertTrue(configuration.isUsernameBan)

        configuration.channelPathValue = "/Lobby/Sub"

        let bannedUser = TeamTalkBannedUser(configuration.cValue)

        XCTAssertEqual(bannedUser.ipAddress, "192.168.1.*")
        XCTAssertEqual(bannedUser.channelPath, "/Lobby/Sub")
        XCTAssertEqual(bannedUser.channelPathValue, TeamTalkChannelPath("/Lobby/Sub"))
        XCTAssertEqual(bannedUser.username, "guest")
        XCTAssertEqual(bannedUser.types, [.ipAddress, .username, .channel])
        XCTAssertTrue(bannedUser.isChannelBan)
        XCTAssertFalse(bannedUser.isLoginBan)
        XCTAssertTrue(bannedUser.isIPAddressBan)
        XCTAssertTrue(bannedUser.isUsernameBan)
        XCTAssertTrue(bannedUser.hasBanType(.username))
    }

    func testOutgoingTextMessageRoundTrip() {
        var rawChannel = Channel()
        rawChannel.nChannelID = 42
        let channel = TeamTalkChannel(rawChannel)
        let outgoing = TeamTalkOutgoingTextMessage.channel(channel, content: "Hello channel")
        let message = TeamTalkTextMessage(outgoing.cValue)

        XCTAssertEqual(message.type, .channel)
        XCTAssertEqual(message.channelID, 42)
        XCTAssertEqual(message.channelIdentifier, TeamTalkChannelID(42))
        XCTAssertEqual(message.content, "Hello channel")
        XCTAssertFalse(message.hasMoreContent)

        var rawUser = User()
        rawUser.nUserID = 21
        let typedUser = TeamTalkUser(rawUser)
        let outgoingToTypedUser = TeamTalkOutgoingTextMessage.user(to: typedUser, content: "Typed user")
        XCTAssertEqual(outgoingToTypedUser.toUserIdentifier, TeamTalkUserID(21))

        var typedOutgoing = outgoingToTypedUser
        typedOutgoing.channelIdentifier = TeamTalkChannelID(12)
        XCTAssertEqual(typedOutgoing.channelID, 12)

        var rawChannel2 = Channel()
        rawChannel2.nChannelID = 34
        let typedChannel = TeamTalkChannel(rawChannel2)
        let outgoingToTypedChannel = TeamTalkOutgoingTextMessage.channel(typedChannel, content: "Typed channel")
        XCTAssertEqual(outgoingToTypedChannel.channelIdentifier, TeamTalkChannelID(34))
    }

    func testOutgoingTextMessageReplyDerivesScope() {
        var raw = TextMessage()
        raw.nMsgType = TeamTalkC.MSGTYPE_USER
        raw.nFromUserID = 7
        raw.nChannelID = 0
        let userMessage = TeamTalkTextMessage(raw)
        let userReply = TeamTalkOutgoingTextMessage.reply(to: userMessage, content: "Pong")
        XCTAssertEqual(userReply.type, .user)
        XCTAssertEqual(userReply.toUserID, 7)

        raw.nMsgType = TeamTalkC.MSGTYPE_CHANNEL
        raw.nFromUserID = 7
        raw.nChannelID = 99
        let channelMessage = TeamTalkTextMessage(raw)
        let channelReply = TeamTalkOutgoingTextMessage.reply(to: channelMessage, content: "Public pong")
        XCTAssertEqual(channelReply.type, .channel)
        XCTAssertEqual(channelReply.channelID, 99)
        XCTAssertEqual(channelReply.toUserID, 0)

        raw.nMsgType = MSGTYPE_BROADCAST
        let broadcastMessage = TeamTalkTextMessage(raw)
        let broadcastReply = TeamTalkOutgoingTextMessage.reply(to: broadcastMessage, content: "Echo")
        XCTAssertEqual(broadcastReply.type, .broadcast)
    }

    func testTextMessageFactorySplitsLongContent() {
        let content = String(repeating: "a", count: Int(TeamTalkC.TT_STRLEN) + 10)
        let messages = TeamTalkTextMessageFactory.messages(from: TeamTalkOutgoingTextMessage.broadcast(content: "").cValue, content: content)

        XCTAssertGreaterThan(messages.count, 1)
        XCTAssertTrue(messages.dropLast().allSatisfy { $0.bMore != 0 })
        XCTAssertEqual(messages.last?.bMore, 0)
        XCTAssertEqual(messages.map(\.content).joined(), content)
    }

    func testClientErrorExposesTypedCode() {
        var rawError = ClientErrorMsg()
        rawError.nErrorNo = TeamTalkErrorCode.fileNotFound.cValue

        let error = TeamTalkClientError(rawError)

        XCTAssertEqual(error.code, TeamTalkErrorCode.fileNotFound.cValue)
        XCTAssertEqual(error.errorCode, .fileNotFound)
        XCTAssertTrue(error.errorCode.isCommandError)
    }

    private func writeCString<T>(_ string: String, to storage: inout T) {
        let utf8 = Array(string.utf8)
        withUnsafeMutableBytes(of: &storage) { rawBuffer in
            for index in rawBuffer.indices {
                rawBuffer[index] = 0
            }
            rawBuffer.copyBytes(from: utf8.prefix(max(0, rawBuffer.count - 1)))
        }
    }

    private func writeInt32Array<T>(_ values: [Int32], to storage: inout T) {
        withUnsafeMutableBytes(of: &storage) { rawBuffer in
            let buffer = rawBuffer.bindMemory(to: Int32.self)
            for index in buffer.indices {
                buffer[index] = 0
            }
            for (index, value) in values.prefix(buffer.count).enumerated() {
                buffer[index] = value
            }
        }
    }

    private func writeStructArray<T, Element>(_ values: [Element], to storage: inout T) {
        withUnsafeMutableBytes(of: &storage) { rawBuffer in
            rawBuffer.initializeMemory(as: UInt8.self, repeating: 0)
            let buffer = rawBuffer.bindMemory(to: Element.self)
            for (index, value) in values.prefix(buffer.count).enumerated() {
                buffer[index] = value
            }
        }
    }
}
