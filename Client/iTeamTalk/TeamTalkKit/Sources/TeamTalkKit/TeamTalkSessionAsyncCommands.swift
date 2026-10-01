import Foundation

public enum TeamTalkCommandAsyncError: Error, LocalizedError {
    case invalidCommand
    case commandFailed(commandID: TeamTalkCommandID, error: TeamTalkClientError)
    case eventStreamEnded(commandID: TeamTalkCommandID)

    public var errorDescription: String? {
        switch self {
        case .invalidCommand:
            return "The TeamTalk command could not be started."
        case .commandFailed(_, let error):
            return error.message
        case .eventStreamEnded(let commandID):
            return "The TeamTalk event stream ended before command \(commandID) completed."
        }
    }
}

/// Default timeout applied to async command helpers that don't override it.
/// Without a bound, a command that never receives a matching completion event
/// (dropped connection, unresponsive server) would hang the awaiting `Task` forever.
public let defaultTeamTalkCommandTimeoutSeconds: TimeInterval = 15

private func awaitCommandCompletion(_ commandID: TeamTalkCommandID, in events: AsyncStream<TeamTalkEvent>) async throws {
    for await event in events {
        switch event.kind {
        case .commandError(let observedCommandID, let error)
            where observedCommandID == commandID:
            throw TeamTalkCommandAsyncError.commandFailed(commandID: observedCommandID, error: error)

        case .commandProcessing(let observedCommandID, let isActive)
            where observedCommandID == commandID && !isActive:
            return

        case .commandSucceeded(let observedCommandID)
            where observedCommandID == commandID:
            return

        default:
            continue
        }
    }

    throw TeamTalkCommandAsyncError.eventStreamEnded(commandID: commandID)
}

private func awaitCommandCompletions(_ commandIDs: Set<TeamTalkCommandID>, in events: AsyncStream<TeamTalkEvent>) async throws {
    var pending = commandIDs

    for await event in events {
        switch event.kind {
        case .commandError(let observedCommandID, let error)
            where pending.contains(observedCommandID):
            throw TeamTalkCommandAsyncError.commandFailed(commandID: observedCommandID, error: error)

        case .commandProcessing(let observedCommandID, let isActive)
            where pending.contains(observedCommandID) && !isActive:
            pending.remove(observedCommandID)
            if pending.isEmpty {
                return
            }

        case .commandSucceeded(let observedCommandID)
            where pending.contains(observedCommandID):
            pending.remove(observedCommandID)
            if pending.isEmpty {
                return
            }

        default:
            continue
        }
    }

    throw TeamTalkCommandAsyncError.eventStreamEnded(commandID: pending.first ?? .invalid)
}

/// Races `operation` against a timeout, throwing `.eventStreamEnded(commandID:)` if the
/// timeout wins first, so a command awaiting a completion event that never arrives
/// doesn't hang its caller forever.
private func withCommandTimeout<T: Sendable>(
    seconds: TimeInterval,
    commandID: TeamTalkCommandID,
    operation: @escaping @Sendable () async throws -> T
) async throws -> T {
    try await withThrowingTaskGroup(of: T.self) { group in
        group.addTask { try await operation() }
        group.addTask {
            try await Task.sleep(nanoseconds: UInt64(seconds * 1_000_000_000))
            throw TeamTalkCommandAsyncError.eventStreamEnded(commandID: commandID)
        }
        let result = try await group.next()!
        group.cancelAll()
        return result
    }
}

private extension TeamTalkSession {
    func performCommand(
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds,
        _ start: () -> TeamTalkCommandID
    ) async throws {
        let eventStream = events
        let commandID = start()

        guard commandID.isValid else {
            throw TeamTalkCommandAsyncError.invalidCommand
        }

        try await withCommandTimeout(seconds: timeoutSeconds, commandID: commandID) {
            try await awaitCommandCompletion(commandID, in: eventStream)
        }
    }

    func performCommands(
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds,
        _ start: () -> [TeamTalkCommandID]
    ) async throws {
        let eventStream = events
        let commandIDs = start()

        guard !commandIDs.isEmpty else {
            throw TeamTalkCommandAsyncError.invalidCommand
        }

        let validCommandIDs = Set(commandIDs.filter(\.isValid))
        guard validCommandIDs.count == commandIDs.count else {
            throw TeamTalkCommandAsyncError.invalidCommand
        }

        try await withCommandTimeout(seconds: timeoutSeconds, commandID: validCommandIDs.first ?? .invalid) {
            try await awaitCommandCompletions(validCommandIDs, in: eventStream)
        }
    }

    /// Awaits commandSucceeded for `commandID` AND a payload event matched by `extract`,
    /// emitted in either order. Times out to avoid hangs when the SDK skips one event.
    func performCommand<T: Sendable>(
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds,
        _ start: () -> TeamTalkCommandID,
        extract: @escaping @Sendable (TeamTalkEvent.Kind, TeamTalkCommandID) -> T?
    ) async throws -> T {
        let stream = events
        let commandID = start()
        guard commandID.isValid else {
            throw TeamTalkCommandAsyncError.invalidCommand
        }

        return try await withCommandTimeout(seconds: timeoutSeconds, commandID: commandID) {
            var payload: T? = nil
            var succeeded = false
            for await event in stream {
                if let value = extract(event.kind, commandID) {
                    payload = value
                }
                switch event.kind {
                case .commandError(let observedID, let err) where observedID == commandID:
                    throw TeamTalkCommandAsyncError.commandFailed(commandID: observedID, error: err)
                case .commandSucceeded(let observedID) where observedID == commandID:
                    succeeded = true
                case .commandProcessing(let observedID, let isActive) where observedID == commandID && !isActive:
                    succeeded = true
                default:
                    break
                }
                if succeeded, let p = payload {
                    return p
                }
            }
            throw TeamTalkCommandAsyncError.eventStreamEnded(commandID: commandID)
        }
    }
}

extension TeamTalkSession {
    public func ping(timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { ping() }
    }

    public func logIn(
        nickname: String,
        username: String,
        password: String,
        clientName: String = "",
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws -> TeamTalkUser {
        let userID = try await performCommand(timeoutSeconds: timeoutSeconds, {
            logIn(nickname: nickname, username: username, password: password, clientName: clientName)
        }) { kind, _ in
            if case .myselfLoggedIn(let id, _) = kind { return id } else { return nil }
        }
        guard let me = user(id: userID) else {
            throw TeamTalkCommandAsyncError.invalidCommand
        }
        return me
    }

    public func logOut(timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { logOut() }
    }

    public func joinChannel(
        _ channel: TeamTalkChannel,
        password: String = "",
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { joinChannel(channel, password: password) }
    }

    public func joinChannel(
        _ configuration: TeamTalkChannelConfiguration,
        password: String = "",
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { joinChannel(configuration, password: password) }
    }

    public func leaveChannel(timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { leaveChannel() }
    }

    public func createChannel(
        _ configuration: TeamTalkChannelConfiguration,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws -> TeamTalkChannel {
        try await performCommand(timeoutSeconds: timeoutSeconds, { createChannel(configuration) }) { kind, _ in
            if case .channelCreated(let ch) = kind,
               ch.parentID == configuration.parentID,
               ch.name == configuration.name {
                return ch
            }
            return nil
        }
    }

    public func updateChannel(
        _ channel: TeamTalkChannel,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { updateChannel(channel) }
    }

    public func updateChannel(
        _ configuration: TeamTalkChannelConfiguration,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { updateChannel(configuration) }
    }

    public func removeChannel(
        _ channel: TeamTalkChannel,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { removeChannel(channel) }
    }

    public func setNickname(
        _ nickname: String,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { setNickname(nickname) }
    }

    public func setStatus(
        mode: TeamTalkStatusMode,
        message: String = "",
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { setStatus(mode: mode, message: message) }
    }

    public func kickUser(
        _ user: TeamTalkUser,
        from channel: TeamTalkChannel? = nil,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { kickUser(user, from: channel) }
    }

    public func banUser(
        _ user: TeamTalkUser,
        from channel: TeamTalkChannel? = nil,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { banUser(user, from: channel) }
    }

    public func banUser(
        _ user: TeamTalkUser,
        types: TeamTalkBanTypes,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { banUser(user, types: types) }
    }

    public func ban(
        _ configuration: TeamTalkBanConfiguration,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { ban(configuration) }
    }

    public func banIPAddress(
        _ ipAddress: String,
        in channel: TeamTalkChannel? = nil,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { banIPAddress(ipAddress, in: channel) }
    }

    public func unbanIPAddress(
        _ ipAddress: String,
        in channel: TeamTalkChannel? = nil,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { unbanIPAddress(ipAddress, in: channel) }
    }

    public func unban(
        _ configuration: TeamTalkBanConfiguration,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { unban(configuration) }
    }

    public func listBans(
        in channel: TeamTalkChannel? = nil,
        startingAt index: Int32 = 0,
        count: Int32 = 100,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { listBans(in: channel, startingAt: index, count: count) }
    }

    public func moveUser(
        _ user: TeamTalkUser,
        to channel: TeamTalkChannel,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { moveUser(user, to: channel) }
    }

    public func setChannelOperator(
        _ user: TeamTalkUser,
        in channel: TeamTalkChannel,
        enabled: Bool,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { setChannelOperator(user, in: channel, enabled: enabled) }
    }

    public func setChannelOperator(
        _ user: TeamTalkUser,
        in channel: TeamTalkChannel,
        operatorPassword: String,
        enabled: Bool,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) {
            setChannelOperator(user, in: channel, operatorPassword: operatorPassword, enabled: enabled)
        }
    }

    public func subscribe(
        _ subscriptions: TeamTalkSubscriptions,
        to user: TeamTalkUser,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { subscribe(subscriptions, to: user) }
    }

    public func unsubscribe(
        _ subscriptions: TeamTalkSubscriptions,
        from user: TeamTalkUser,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { unsubscribe(subscriptions, from: user) }
    }

    public func sendTextMessage(
        _ message: TeamTalkOutgoingTextMessage,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommands(timeoutSeconds: timeoutSeconds) { sendTextMessage(message) }
    }

    public func sendTextMessage(
        to user: TeamTalkUser,
        content: String,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await sendTextMessage(.user(to: user, content: content), timeoutSeconds: timeoutSeconds)
    }

    public func sendTextMessage(
        to channel: TeamTalkChannel,
        content: String,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await sendTextMessage(.channel(channel, content: content), timeoutSeconds: timeoutSeconds)
    }

    public func sendChannelMessage(
        _ content: String,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        guard let channel = currentChannel() else {
            throw TeamTalkCommandAsyncError.invalidCommand
        }
        try await sendTextMessage(to: channel, content: content, timeoutSeconds: timeoutSeconds)
    }

    public func reply(
        to message: TeamTalkTextMessage,
        content: String,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await sendTextMessage(.reply(to: message, content: content), timeoutSeconds: timeoutSeconds)
    }

    public func uploadFile(
        at localURL: URL,
        to channel: TeamTalkChannel,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws -> TeamTalkRemoteFile {
        let targetChannelID = channel.channelID
        return try await performCommand(timeoutSeconds: timeoutSeconds, { uploadFile(at: localURL, to: channel) }) { kind, _ in
            if case .fileCreated(let f) = kind, f.channelIdentifier == targetChannelID {
                return f
            }
            return nil
        }
    }

    public func downloadFile(
        _ file: TeamTalkRemoteFile,
        to localURL: URL,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { downloadFile(file, to: localURL) }
    }

    public func deleteFile(
        _ file: TeamTalkRemoteFile,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { deleteFile(file) }
    }

    public func updateServer(
        _ configuration: TeamTalkServerPropertiesConfiguration,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { updateServer(configuration) }
    }

    public func updateServer(
        _ properties: TeamTalkServerProperties,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { updateServer(properties) }
    }

    public func listUserAccounts(
        startingAt index: Int32 = 0,
        count: Int32 = 100,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { listUserAccounts(startingAt: index, count: count) }
    }

    public func createUserAccount(
        _ configuration: TeamTalkUserAccountConfiguration,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { createUserAccount(configuration) }
    }

    public func createUserAccount(
        _ account: TeamTalkUserAccount,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { createUserAccount(account) }
    }

    public func deleteUserAccount(
        username: String,
        timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds
    ) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { deleteUserAccount(username: username) }
    }

    public func saveServerConfiguration(timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { saveServerConfiguration() }
    }

    public func queryServerStatistics(timeoutSeconds: TimeInterval = defaultTeamTalkCommandTimeoutSeconds) async throws {
        try await performCommand(timeoutSeconds: timeoutSeconds) { queryServerStatistics() }
    }
}
