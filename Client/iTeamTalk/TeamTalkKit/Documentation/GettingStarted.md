# Getting Started

This guide walks through the API surface a new app needs: create a session,
connect, log in, do something, and tear down. For a complete, runnable
version of everything below, see
[`Examples/TeamTalkKitExample`](../Examples/TeamTalkKitExample) - run it with:

```sh
swift run TeamTalkKitExample <host> [tcpPort] [udpPort] [nickname]
```

## Import

```swift
import TeamTalkKit
```

TeamTalkKit does not re-export the underlying `TeamTalkC` module or any raw
SDK type. Application code talks to Swift model types only (`TeamTalkUser`,
`TeamTalkChannel`, `TeamTalkSession`, ...); if you ever need the raw C value
behind one, it's on `rawValue`/`cValue` (see [Advanced Usage](Advanced.md)).

## Create A Session

Unlike the pre-1.0 API, there's no shared singleton - create a
`TeamTalkSession` wherever your app manages connection lifecycle (a session
model, a dependency-injected service, etc.):

```swift
let session = TeamTalkSession()
```

Nothing stops you from creating more than one (e.g. to connect to two
servers at once); most apps only need one, held for the app's lifetime.

## Start And Close

`start` allocates the native SDK instance and applies your license. Call it
once, before anything else:

```swift
session.start(licenseName: REGISTRATION_NAME, licenseKey: REGISTRATION_KEY)
```

An empty name/key (`""`, `""`) runs the SDK unregistered - the same default
iTeamTalk itself ships with. BearWare.dk issues a real license key for
production use; see their site for terms.

Tear the instance down when you're done with it (app shutdown, or before
reusing the session with a fresh `start` call):

```swift
session.close()
```

## Poll Events

The TeamTalk C SDK is poll based - it queues messages internally and expects
the host app to drain that queue. `TeamTalkSession` can do this for you with
a repeating timer:

```swift
session.startEventDispatching()   // default: every 0.1s
session.stopEventDispatching()    // e.g. around session.close()
```

If you'd rather drive polling yourself (a custom run loop, a specific
dispatch queue), call `session.pollMessages()` on whatever schedule you
choose instead - `startEventDispatching` is a convenience, not a requirement.

## Connect And Log In

Connecting starts the TCP/UDP handshake and returns immediately; the actual
result arrives later as an event.

```swift
let didStartConnection = session.connect(
    toHost: "example.org",
    tcpPort: 10333,
    udpPort: 10333,
    encrypted: false
)

guard didStartConnection else {
    return
}
```

Every command on `TeamTalkSession` comes in two flavors: a synchronous,
fire-and-forget one returning a `TeamTalkCommandID` you match against events
yourself, and an `async throws` one that awaits the matching completion
event for you (with a timeout, so it can't hang forever). Prefer the async
ones for new code:

```swift
// After .connectionSucceeded arrives (see "Observe Typed Events" below):
let me = try await session.logIn(
    nickname: "Alice",
    username: "alice",
    password: "secret",
    clientName: "MyApp"
)
print("Logged in as \(me.nickname), user #\(me.id)")
```

> **Top-level-code gotcha:** inside a normal function, as above, this always
> resolves correctly. Only if you write this directly as top-level code in a
> `main.swift` (a quick script, not a regular function) can `try await
> session.logIn(...)` bound to a `let` silently resolve to the sync overload
> instead - see [Advanced Usage](Advanced.md#top-level-code-overload-gotcha)
> for why, and the fix.

Or track the command ID yourself with the sync overload:

```swift
let loginCommandID = session.logIn(
    nickname: "Alice",
    username: "alice",
    password: "secret",
    clientName: "MyApp"
)
```

## Observe Typed Events

Use `TeamTalkEventObserver` for delegate-style callbacks:

```swift
final class SessionModel: TeamTalkEventObserver {
    func handleTeamTalkEvent(_ event: TeamTalkEvent) {
        switch event.kind {
        case .connectionSucceeded:
            print("Connected")

        case .commandError(let commandID, let error):
            print("Command \(commandID) failed: \(error.message)")

        case .myselfLoggedIn(let userID, let account):
            print("Logged in as \(userID), rights: \(account.rights)")

        default:
            break
        }
    }
}

let model = SessionModel()
session.addEventObserver(model)   // held weakly - no need to remove just to break a retain cycle
session.removeEventObserver(model)
```

Or consume events as an `AsyncStream`. Each access to `events` opens a fresh
stream backed by its own observer, cleaned up automatically when the
consuming loop ends:

```swift
Task {
    for await event in session.events {
        print(event.kind)
    }
}
```

`events` only receives events while something is polling - either
`startEventDispatching()` or your own `pollMessages()` calls.

## Query Server State

Once logged in, query snapshots straight from Swift models - no command
round-trip needed, these read the SDK's already-cached state:

```swift
let properties = session.serverProperties()
let me = session.currentUser()
let channels = session.channels()
let users = session.serverUsers()
let files = session.remoteFiles(in: someChannel)
```

Every snapshot model keeps its raw C value available through `cValue`/
`rawValue` for the rare case you need it - see
[Advanced Usage](Advanced.md#raw-values-and-c-values).

## Join A Channel

Join an existing channel:

```swift
try await session.joinChannel(channel, password: "")
```

Create-and-join with a configuration:

```swift
let configuration = TeamTalkChannelConfiguration(
    parentChannelID: session.channelIdentifier(fromPath: "/"),
    name: "Meeting",
    topic: "Weekly sync",
    maxUsers: 25
)

let created = try await session.createChannel(configuration)
try await session.joinChannel(created)
```

## Send Text Messages

`TeamTalkOutgoingTextMessage` splits long content across as many SDK
messages as needed automatically:

```swift
try await session.sendTextMessage(.channel(someChannel, content: "Hello"))
try await session.sendTextMessage(.user(to: someUser, content: "Hi"))
```

## Files

```swift
let files = session.remoteFiles(in: someChannel)

let uploadedFile = try await session.uploadFile(at: localURL, to: someChannel)
try await session.downloadFile(someFile, to: destinationURL)
```

Track transfer progress through `.fileTransfer` events:

```swift
case .fileTransfer(let transfer):
    print(transfer.status, transfer.progress)   // progress is normalized 0...1
```

## Rights And Options

Swift option sets wrap TeamTalk bitmasks:

```swift
if session.myRights.contains(.canUploadFiles) {
    print("Can upload")
}

if someUser.hasSubscription(.voice) {
    print("Receiving voice from user")
}
```

## Disconnect And Shut Down

```swift
try? await session.logOut()
session.disconnect()
session.stopEventDispatching()
session.close()
```
