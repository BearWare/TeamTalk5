import Foundation
import TeamTalkC

// Each protocol below adds one `hasX(_:)`/`getX(_:)` OptionSet-membership
// check, then both the raw C struct and its `TeamTalkX` wrapper conform to
// it at the bottom of this file — so the check is written once and works
// identically on `Channel`/`TeamTalkChannel`, `User`/`TeamTalkUser`, etc.,
// instead of each model duplicating its own `.contains(_:)` wrapper.
public protocol TeamTalkUserTypeContaining {
    var types: TeamTalkUserTypes { get }
}

public extension TeamTalkUserTypeContaining {
    func hasUserType(_ type: TeamTalkUserTypes) -> Bool {
        types.contains(type)
    }
}

public protocol TeamTalkLocalSubscriptionContaining {
    var localSubscriptions: TeamTalkSubscriptions { get }
}

public extension TeamTalkLocalSubscriptionContaining {
    func hasSubscription(_ subscription: TeamTalkSubscriptions) -> Bool {
        localSubscriptions.contains(subscription)
    }
}

public protocol TeamTalkPeerSubscriptionContaining {
    var peerSubscriptions: TeamTalkSubscriptions { get }
}

public extension TeamTalkPeerSubscriptionContaining {
    func hasPeerSubscription(_ subscription: TeamTalkSubscriptions) -> Bool {
        peerSubscriptions.contains(subscription)
    }
}

public protocol TeamTalkUserStateContaining {
    var states: TeamTalkUserStates { get }
}

public extension TeamTalkUserStateContaining {
    func hasState(_ state: TeamTalkUserStates) -> Bool {
        states.contains(state)
    }
}

public protocol TeamTalkUserRightsContaining {
    var rights: TeamTalkUserRights { get }
}

public extension TeamTalkUserRightsContaining {
    func getUserRight(_ right: TeamTalkUserRights) -> Bool {
        rights.contains(right)
    }

    func hasUserRight(_ right: TeamTalkUserRights) -> Bool {
        getUserRight(right)
    }
}

public protocol TeamTalkSoundDeviceEffectContaining {
    var enabledEffects: TeamTalkSoundDeviceFeatures { get }
}

public extension TeamTalkSoundDeviceEffectContaining {
    func hasEffect(_ effect: TeamTalkSoundDeviceFeatures) -> Bool {
        enabledEffects.contains(effect)
    }
}

public protocol TeamTalkBanTypeContaining {
    var types: TeamTalkBanTypes { get }
}

public extension TeamTalkBanTypeContaining {
    func hasBanType(_ type: TeamTalkBanTypes) -> Bool {
        types.contains(type)
    }
}

public protocol TeamTalkChannelTypeContaining {
    var types: TeamTalkChannelTypes { get }
}

public extension TeamTalkChannelTypeContaining {
    func hasChannelType(_ type: TeamTalkChannelTypes) -> Bool {
        types.contains(type)
    }
}

public protocol TeamTalkServerLogEventContaining {
    var logEvents: TeamTalkServerLogEvents { get }
}

public extension TeamTalkServerLogEventContaining {
    func hasLogEvent(_ event: TeamTalkServerLogEvents) -> Bool {
        logEvents.contains(event)
    }
}

extension User: TeamTalkUserTypeContaining, TeamTalkLocalSubscriptionContaining, TeamTalkPeerSubscriptionContaining, TeamTalkUserStateContaining {}

extension TeamTalkUser: TeamTalkUserTypeContaining, TeamTalkLocalSubscriptionContaining, TeamTalkPeerSubscriptionContaining, TeamTalkUserStateContaining {}

extension UserAccount: TeamTalkUserRightsContaining {}

extension TeamTalkUserAccount: TeamTalkUserRightsContaining {}

extension SoundDeviceEffects: TeamTalkSoundDeviceEffectContaining {}

extension TeamTalkSoundDeviceEffects: TeamTalkSoundDeviceEffectContaining {}

extension BannedUser: TeamTalkBanTypeContaining {}

extension TeamTalkBannedUser: TeamTalkBanTypeContaining {}

extension Channel: TeamTalkChannelTypeContaining {}

extension TeamTalkChannel: TeamTalkChannelTypeContaining {}

extension ServerProperties: TeamTalkServerLogEventContaining {}

extension TeamTalkServerProperties: TeamTalkServerLogEventContaining {}

extension TeamTalkServerPropertiesConfiguration: TeamTalkServerLogEventContaining {}
