# Advanced Usage

This document explains the pieces that matter once the basic connection flow
from [Getting Started](GettingStarted.md) is working.

## Package Layers

TeamTalkKit is organized around four layers:

- `TeamTalkNativeiOS` / `TeamTalkNativemacOS`: vendored TeamTalk native SDK
  binaries linked by SwiftPM for the matching platform;
- `TeamTalkC`: a small C bridge over the TeamTalk SDK header, mostly for
  fixed string arrays, C unions and helper constructors;
- `TeamTalkTypes.swift`, `TeamTalkModels.swift`, `TeamTalk*Models.swift`,
  `TeamTalk*Types.swift`: Swift wrappers for C enums, bitmasks, and
  snapshot/configuration models built from C structs;
- `TeamTalkSession.swift` and its extensions (`TeamTalkSession*.swift`):
  lifecycle, queries, command APIs and event dispatch, all hung off one
  `TeamTalkSession` instance.

`Exports.swift` is intentionally empty. Earlier versions of this package
re-exported raw `TeamTalkC` types/constants by name so app code could touch
them directly; every one of those touch points now has a Swift-native
counterpart instead (see `Documentation/TODO.md`'s "Application Migration"
section for the full list). A file inside this package that still needs a
raw `TeamTalkC` symbol imports that module directly rather than relying on
a re-export.

## Native SDK Artifacts

The TeamTalk SDK headers and libraries are vendored inside the package:

- `Sources/TeamTalkC/include/TeamTalk.h` is the public C SDK header used by
  the bridge;
- `Vendor/TeamTalkNativeiOS.xcframework` contains the iOS static library
  slices;
- `Vendor/TeamTalkNativemacOS.xcframework` contains the macOS dynamic
  library slice.

This keeps TeamTalkKit usable as a normal Swift package dependency. A
consumer should be able to add the package URL in Xcode without also
arranging a repository-relative `Library/TeamTalk_DLL` checkout.

## Raw Values And C Values

Most Swift wrappers expose both a typed API and the underlying C value:

```swift
let rights = TeamTalkUserRights.canUploadFiles
let raw: UInt32 = rights.rawValue

let channel: TeamTalkChannel = ...
let rawChannel: Channel = channel.cValue   // needs `import TeamTalkC`
```

Configuration models go the other direction and build C structs:

```swift
let config = TeamTalkChannelConfiguration(
    parentChannelID: session.rootChannelIdentifier,
    name: "Staff"
)

let rawChannel: Channel = config.cValue   // needs `import TeamTalkC`
```

Because raw C types aren't re-exported, reaching for `Channel`, `User`,
`AudioCodec`, `TTBOOL`, and so on from app code requires `import TeamTalkC`
alongside `import TeamTalkKit`. This is deliberate friction: it should be
rare, and reaching for it is a sign a Swift-native accessor might be worth
adding to the wrapper instead.

## Typed IDs

Lightweight wrappers exist for every SDK ID:

```swift
let userID = TeamTalkUserID(5)
let channelID = TeamTalkChannelID(10)
let fileID = TeamTalkFileID(3)
let transferID = TeamTalkTransferID(8)
let commandID = TeamTalkCommandID(12)
```

They preserve the C value through `cValue`, but make Swift call sites
clearer:

```swift
session.joinChannel(channel)
session.downloadFile(file, to: localURL)
session.cancelFileTransfer(transfer)
```

Snapshot models expose both the plain `Int32` ID (`channel.id`,
`user.id`, ...) and its typed companion (`channel.channelID`,
`user.userID`, ...).

## Command Tracking

Every mutating operation on `TeamTalkSession` comes in two forms:

- a synchronous, `@discardableResult` command that fires the request and
  returns a `TeamTalkCommandID` immediately - match it against events
  yourself;
- an `async throws` command (in `TeamTalkSessionAsyncCommands.swift`) that
  awaits the matching completion event (or a payload event, for commands
  like `createChannel`/`uploadFile` that return a value) and throws
  `TeamTalkCommandAsyncError` on failure or timeout.

```swift
// Sync: track the ID yourself.
let commandID = session.joinChannel(channel)

switch event.kind {
case .commandProcessing(let id, let isActive) where id == commandID:
    print(isActive ? "Started" : "Finished")
case .commandError(let id, let error) where id == commandID:
    print(error.message)
case .commandSucceeded(let id) where id == commandID:
    print("OK")
default:
    break
}

// Async: the call itself waits.
try await session.joinChannel(channel)
```

Async commands default to a 15-second timeout
(`defaultTeamTalkCommandTimeoutSeconds`), overridable per call via a
trailing `timeoutSeconds:` argument, so a command whose completion event
never arrives (dropped connection, unresponsive server) can't hang the
awaiting `Task` forever.

`TeamTalkCommandID.invalid` maps to `-1`, the TeamTalk SDK's error return
value for a command that failed to even start.

### Top-level-code overload gotcha

The sync and async overloads of the same command share argument labels
(`logIn`, `logOut`, `joinChannel`, ...). Inside a normal function body,
Swift's overload resolution reliably picks the async one when the call is
written with `try await` - this is the common case, and it just works:

```swift
func logIn(session: TeamTalkSession) async throws -> TeamTalkUser {
    let me = try await session.logIn(nickname: "Alice", username: "alice", password: "secret")
    return me   // `me` is TeamTalkUser, as expected - no extra annotation needed
}
```

The one place this *doesn't* hold is **top-level code** - statements
written directly in a `main.swift`, outside any function. There, Swift's
statement-by-statement type-checking doesn't reliably prefer the async
overload, and `let me = try await session.logIn(...)` can silently resolve
to the sync, `@discardableResult` overload instead: `me` ends up a
`TeamTalkCommandID`, not the `TeamTalkUser` you were expecting, and nothing
was actually awaited. It's a Swift top-level-code limitation, not something
specific to this API - but this package's own command pairs are exactly the
shape that triggers it.

If you do write top-level code (a quick script, a one-off tool), either
annotate the expected type explicitly:

```swift
let me: TeamTalkUser = try await session.logIn(nickname: "Alice", username: "alice", password: "secret")
```

or - the more robust fix, and what `Examples/TeamTalkKitExample/main.swift`
does - keep top-level code to a couple of lines that just call into a
regular `async` function holding the real logic. Inside that function, the
ambiguity doesn't arise at all.

## Event Dispatch

TeamTalkKit has two event APIs:

- `TeamTalkMessageObserver` receives raw `TTMessage` values;
- `TeamTalkEventObserver` receives decoded `TeamTalkEvent` values.

Both are fed by `pollMessages()`, which `startEventDispatching(pollInterval:)`
calls on a repeating timer for you - see
[Getting Started](GettingStarted.md#poll-events). The typed API is additive
and doesn't remove raw message support.

`events: AsyncStream<TeamTalkEvent>` is a thin wrapper over the same
observer system: it registers a `TeamTalkEventObserver` on first iteration
and unregisters it when the consuming `for await` loop ends. It does not
poll the SDK by itself - something still needs to call `pollMessages()`
(directly, or via `startEventDispatching`).

## Server And Admin APIs

```swift
try await session.listUserAccounts(startingAt: 0, count: 100)

try await session.createUserAccount(
    TeamTalkUserAccountConfiguration(
        username: "guest",
        password: "guest",
        types: .defaultUser,
        rights: [.canTransmitVoice, .canSendChannelTextMessages]
    )
)

try await session.deleteUserAccount(username: "guest")
```

Server settings update from a configuration built off the current snapshot:

```swift
if let current = session.serverProperties() {
    var config = TeamTalkServerPropertiesConfiguration(current)
    config.name = "New Server Name"
    config.logEvents.insert(.userLoggedIn)
    try await session.updateServer(config)
}
```

Server statistics arrive as an event after being requested:

```swift
try await session.queryServerStatistics()
// ...
case .serverStatistics(let statistics):
    print(statistics.usersServed)
```

## Bans

```swift
let ipBan = TeamTalkBanConfiguration(ipAddress: "192.168.1.*", types: [.ipAddress])
try await session.ban(ipBan)

let userBan = TeamTalkBanConfiguration(channelPath: "/Lobby", username: "guest", types: [.channel, .username])
try await session.ban(userBan)

try await session.listBans(in: nil, startingAt: 0, count: 100)
// ...
case .bannedUser(let bannedUser):
    print(bannedUser.username, bannedUser.ipAddress)
```

## Files And Transfer Progress

File listing reads the SDK's already-cached state, no command round-trip:

```swift
let files = session.remoteFiles(in: channel)
```

Upload/download/delete are commands:

```swift
let uploaded = try await session.uploadFile(at: localURL, to: channel)
try await session.downloadFile(file, to: destinationURL)
try await session.deleteFile(file)
```

Transfer state arrives through `.fileTransfer`:

```swift
case .fileTransfer(let transfer):
    print(transfer.id, transfer.status, transfer.progress)
```

`TeamTalkFileTransfer.progress` is normalized to `0...1`.

## Threading And Lifecycle

The native SDK is stateful and poll based, so app code should avoid issuing
commands against the same `TeamTalkSession` from many unrelated queues.
Recommended pattern:

- `start(licenseName:licenseKey:)` once, during app/feature initialization;
- `connect`/`logIn` from a session or connection model;
- `startEventDispatching()` (or your own `pollMessages()` loop) running
  continuously while connected;
- update UI state from decoded `TeamTalkEvent`s;
- `logOut`/`disconnect`/`stopEventDispatching`/`close` on shutdown.

Nothing in `TeamTalkSession` prevents holding more than one instance at
once (e.g. two servers, or one instance per test case) - each wraps its own
native SDK handle.

## macOS Notes

The package declares macOS 10.15 so `swift build`/`swift test` compile and
run TeamTalkKit on macOS, and `Examples/TeamTalkKitExample` is a macOS
console executable exercising the same API a real app would use. This is
useful for package validation, CI, and command-line tooling.

Runtime macOS support in a full app still needs validating end-to-end:

- audio device testing beyond what the console example exercises;
- file sandbox/security-scoped URL testing where applicable;
- linker settings for a packaged macOS app target consuming TeamTalkKit as
  a dependency, not just via `swift build`/`swift run` in this package.

Until those are validated in a real app, treat macOS as a supported build
and command-line target, not yet a fully certified GUI-app runtime target.
