import TeamTalkC
import XCTest
@testable import TeamTalkKit

final class TeamTalkEventResolverTests: XCTestCase {
    func testResolvedUserPicksEmbeddedUserWithoutQueryingSession() {
        var rawUser = User()
        rawUser.nUserID = 5
        let user = TeamTalkUser(rawUser)
        let event = TeamTalkEvent.Kind.userJoined(user)

        // Passing an unstarted session never gets queried because the event already carries the user.
        let session = TeamTalkSession()
        let resolved = event.resolvedUser(in: session)
        XCTAssertEqual(resolved?.userID, TeamTalkUserID(5))
    }

    func testResolvedChannelPicksEmbeddedChannelWithoutQueryingSession() {
        var rawChannel = Channel()
        rawChannel.nChannelID = 12
        let channel = TeamTalkChannel(rawChannel)
        let event = TeamTalkEvent.Kind.channelCreated(channel)

        let session = TeamTalkSession()
        let resolved = event.resolvedChannel(in: session)
        XCTAssertEqual(resolved?.channelID, TeamTalkChannelID(12))
    }

    func testResolvedUserOnEventWithoutUserPayloadReturnsNil() {
        let event = TeamTalkEvent.Kind.connectionSucceeded
        let session = TeamTalkSession()
        XCTAssertNil(event.resolvedUser(in: session))
        XCTAssertNil(event.resolvedChannel(in: session))
    }
}
