import TeamTalkC
import TeamTalkKit
import XCTest

final class TeamTalkEventTests: XCTestCase {
    func testCommandProcessingEventDecoding() {
        var message = TTMessage()
        message.event = .commandProcessing
        message.nSource = 42
        message.payloadType = .boolean
        message.bActive = 1

        switch TeamTalkEvent(message).kind {
        case .commandProcessing(let commandID, let isActive):
            XCTAssertEqual(commandID, TeamTalkCommandID(42))
            XCTAssertTrue(isActive)
        default:
            XCTFail("Expected commandProcessing event")
        }
    }

    func testConnectionMaxPayloadUpdatedUsesPayloadSize() {
        var message = TTMessage()
        message.event = .connectionMaxPayloadUpdated
        message.nSource = 999
        message.nPayloadSize = 1_472

        switch TeamTalkEvent(message).kind {
        case .connectionMaxPayloadUpdated(let maxPayloadSize):
            XCTAssertEqual(maxPayloadSize, 1_472)
        default:
            XCTFail("Expected connectionMaxPayloadUpdated event")
        }
    }

    func testCommandErrorEventDecoding() {
        var message = TTMessage()
        message.event = .commandError
        message.nSource = 77
        message.payloadType = .clientError

        var error = ClientErrorMsg()
        error.nErrorNo = TeamTalkErrorCode.notAuthorized.cValue
        message.clienterrormsg = error

        switch TeamTalkEvent(message).kind {
        case .commandError(let commandID, let error):
            XCTAssertEqual(commandID, TeamTalkCommandID(77))
            XCTAssertEqual(error.errorCode, .notAuthorized)
            XCTAssertEqual(error.code, TeamTalkErrorCode.notAuthorized.cValue)
        default:
            XCTFail("Expected commandError event")
        }
    }

    func testServerStatisticsEventDecoding() {
        var message = TTMessage()
        message.event = .serverStatistics
        message.payloadType = .serverStatistics

        var statistics = ServerStatistics()
        statistics.nUsersServed = 12
        statistics.nUsersPeak = 5
        statistics.nUptimeMSec = 123_456
        message.serverstatistics = statistics

        switch TeamTalkEvent(message).kind {
        case .serverStatistics(let statistics):
            XCTAssertEqual(statistics.usersServed, 12)
            XCTAssertEqual(statistics.usersPeak, 5)
            XCTAssertEqual(statistics.uptimeMilliseconds, 123_456)
        default:
            XCTFail("Expected serverStatistics event")
        }
    }

    func testTextMessageEventDecoding() {
        var message = TTMessage()
        message.event = .userTextMessage
        message.payloadType = .textMessage

        var rawChannel = Channel()
        rawChannel.nChannelID = 9
        var textMessage = TeamTalkOutgoingTextMessage.channel(TeamTalkChannel(rawChannel), content: "Hello").cValue
        textMessage.nFromUserID = 4
        message.textmessage = textMessage

        switch TeamTalkEvent(message).kind {
        case .textMessage(let textMessage):
            XCTAssertEqual(textMessage.type, .channel)
            XCTAssertEqual(textMessage.fromUserID, 4)
            XCTAssertEqual(textMessage.channelID, 9)
            XCTAssertEqual(textMessage.content, "Hello")
        default:
            XCTFail("Expected textMessage event")
        }
    }

    func testBannedUserEventDecoding() {
        var message = TTMessage()
        message.event = .bannedUser
        message.payloadType = .bannedUser
        message.banneduser = TeamTalkBanConfiguration(
            ipAddress: "10.0.0.*",
            channelPath: "/Lobby",
            username: "blocked",
            types: [.ipAddress, .username]
        ).cValue

        switch TeamTalkEvent(message).kind {
        case .bannedUser(let bannedUser):
            XCTAssertEqual(bannedUser.ipAddress, "10.0.0.*")
            XCTAssertEqual(bannedUser.channelPath, "/Lobby")
            XCTAssertEqual(bannedUser.username, "blocked")
            XCTAssertEqual(bannedUser.types, [.ipAddress, .username])
        default:
            XCTFail("Expected bannedUser event")
        }
    }

    func testUserAndChannelIdentifierEventsDecodeTypedWrappers() {
        var loginMessage = TTMessage()
        loginMessage.event = .myselfLoggedIn
        loginMessage.nSource = 42
        loginMessage.payloadType = .userAccount

        var account = UserAccount()
        TeamTalkString.setUserAccount(.username, on: &account, to: "alice")
        loginMessage.useraccount = account

        switch TeamTalkEvent(loginMessage).kind {
        case .myselfLoggedIn(let userID, let account):
            XCTAssertEqual(userID, TeamTalkUserID(42))
            XCTAssertEqual(account.username, "alice")
        default:
            XCTFail("Expected myselfLoggedIn event")
        }

        var kickedMessage = TTMessage()
        kickedMessage.event = .myselfKicked
        kickedMessage.nSource = 7

        switch TeamTalkEvent(kickedMessage).kind {
        case .myselfKicked(let channelID, let by):
            XCTAssertEqual(channelID, TeamTalkChannelID(7))
            XCTAssertNil(by)
        default:
            XCTFail("Expected myselfKicked event")
        }

        var desktopMessage = TTMessage()
        desktopMessage.event = .userDesktopWindow
        desktopMessage.nSource = 19
        desktopMessage.nStreamID = 88

        switch TeamTalkEvent(desktopMessage).kind {
        case .userDesktopWindow(let userID, let streamID):
            XCTAssertEqual(userID, TeamTalkUserID(19))
            XCTAssertEqual(streamID, TeamTalkMediaStreamID(88))
        default:
            XCTFail("Expected userDesktopWindow event")
        }
    }

    func testSessionIdentifierEventsDecodeTypedWrappers() {
        var desktopTransferMessage = TTMessage()
        desktopTransferMessage.event = .desktopWindowTransfer
        desktopTransferMessage.nSource = 15
        desktopTransferMessage.nBytesRemain = 2_048

        switch TeamTalkEvent(desktopTransferMessage).kind {
        case .desktopWindowTransfer(let sessionID, let bytesRemaining):
            XCTAssertEqual(sessionID, TeamTalkDesktopSessionID(15))
            XCTAssertEqual(bytesRemaining, 2_048)
        default:
            XCTFail("Expected desktopWindowTransfer event")
        }

        var playbackMessage = TTMessage()
        playbackMessage.event = .localMediaFile
        playbackMessage.nSource = 23

        switch TeamTalkEvent(playbackMessage).kind {
        case .localMediaFile(let sessionID):
            XCTAssertEqual(sessionID, TeamTalkPlaybackSessionID(23))
        default:
            XCTFail("Expected localMediaFile event")
        }
    }

    func testHotkeyEventsUseClearLabels() {
        var hotkeyMessage = TTMessage()
        hotkeyMessage.event = .hotkey
        hotkeyMessage.nSource = 17
        hotkeyMessage.bActive = 1

        switch TeamTalkEvent(hotkeyMessage).kind {
        case .hotkey(let hotkeyID, let isActive):
            XCTAssertEqual(hotkeyID, 17)
            XCTAssertTrue(isActive)
        default:
            XCTFail("Expected hotkey event")
        }
    }
}
