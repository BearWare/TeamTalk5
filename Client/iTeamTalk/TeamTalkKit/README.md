# TeamTalkKit

TeamTalkKit is the Swift package used by iTeamTalk to wrap the TeamTalk C SDK.
Application code talks to Swift model types only - `TeamTalkUser`,
`TeamTalkChannel`, `TeamTalkAudioCodecConfiguration`, `TeamTalkCommandID`,
`TeamTalkSubscriptions`, `TeamTalkUserRights`, and so on - with `rawValue`/
`cValue` available on those types for the rare case a caller needs the
underlying C value. No raw C type or SDK function is re-exported to
`import TeamTalkKit` consumers; files inside this package that still need a
raw `TeamTalkC` symbol import that module directly instead.

## Platform Status

`Package.swift` declares:

- iOS 18 and newer
- macOS 10.15 and newer

The package vendors the TeamTalk native SDK artifacts it needs under
`Vendor/`, so consumers should not need a sibling `Library/TeamTalk_DLL`
checkout just to build the Swift package. SwiftPM links the bundled native
artifact conditionally per platform:

- `TeamTalkNativeiOS.xcframework` for iOS device and iOS simulator;
- `TeamTalkNativemacOS.xcframework` for macOS.

The iOS artifact contains an `arm64` device slice and a universal simulator
slice with both `x86_64` and `arm64`, so it can be used on Intel and Apple
Silicon simulator hosts.

## Documentation

- [Getting Started](Documentation/GettingStarted.md)
- [Advanced Usage](Documentation/Advanced.md)
- [API Audit](Documentation/APIAudit.md)
- [TODO](Documentation/TODO.md)
- [Examples](Examples/) - a runnable console client, `swift run TeamTalkKitExample <host>`

## Current API Shape

TeamTalkKit currently contains:

- typed wrappers for user rights, user types, subscriptions, stream types,
  channel types, server log events, ban types, client events and payload types;
- snapshot models for users, channels, server properties, user accounts, remote
  files, file transfers, text messages, bans and server statistics;
- configuration models for creating/updating user accounts, channels, server
  properties, bans and outgoing text messages;
- `TeamTalkCommandID` for tracking command processing;
- a typed event system through `TeamTalkEventObserver` and
  `AsyncStream<TeamTalkEvent>`;
- Swift command APIs on `TeamTalkSession` for login, channels, files, bans,
  user accounts, server settings and server statistics - each as both a
  synchronous, command-ID-returning call and an `async throws` call that
  awaits the matching completion event for you;
- no raw C re-exports (`Sources/TeamTalkKit/Exports.swift` is empty) — every
  raw C type the wrapper's own API used to hand back has a Swift-native
  counterpart (e.g. `TeamTalkAudioCodecConfiguration` instead of the raw
  `AudioCodec` union), so neither raw SDK types nor functions (`TT_*`) are
  reachable from `import TeamTalkKit` alone.

The package is not complete yet. See [TODO](Documentation/TODO.md) for the
remaining wrapper work.

## Build Checks

From the package directory:

```sh
swift build
swift test
swift run TeamTalkKitExample <host> [tcpPort] [udpPort] [nickname]
```

From the iTeamTalk workspace:

```sh
xcodebuild -scheme iTeamTalk -configuration Debug -destination 'generic/platform=iOS Simulator' CODE_SIGNING_ALLOWED=NO build
```

Native TeamTalk SDK warnings from the bundled artifacts are currently unrelated
to the Swift wrapper layer.
