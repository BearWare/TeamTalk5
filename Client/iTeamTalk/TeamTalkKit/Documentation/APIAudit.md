# API Audit

This document records the current split between TeamTalkKit's public Swift
API and its internal-only plumbing, and the reasoning behind it. It used to
track an in-progress migration away from a raw-C-exposing compatibility
layer (`TeamTalkClient` + `@_exported import TeamTalkC`); that migration is
now complete, so this reads as a snapshot of the finished design rather than
a punch list. See `Documentation/TODO.md`'s "Application Migration" section
for how each raw-C touch point was retired.

## Public API

Application code should use:

- `TeamTalkSession` - one instance wraps one native SDK connection. Its
  public surface is spread across `TeamTalkSession*.swift` extension files
  by concern: lifecycle/queries (`TeamTalkSessionServerState.swift`),
  commands (`TeamTalkSessionCommands.swift`), the `async throws` command
  wrappers (`TeamTalkSessionAsyncCommands.swift`), events
  (`TeamTalkSessionEvents.swift`), audio/media
  (`TeamTalkSessionAudio.swift`, `TeamTalkSessionMedia.swift`), and
  desktop/video (`TeamTalkSessionDesktopVideo.swift`);
- typed snapshot models: `TeamTalkUser`, `TeamTalkChannel`,
  `TeamTalkRemoteFile`, `TeamTalkFileTransfer`, `TeamTalkTextMessage`,
  `TeamTalkServerProperties`, `TeamTalkUserAccount`, and configuration
  counterparts (`TeamTalkChannelConfiguration`,
  `TeamTalkUserAccountConfiguration`, `TeamTalkServerPropertiesConfiguration`,
  `TeamTalkBanConfiguration`, `TeamTalkAudioCodecConfiguration`,
  `TeamTalkAudioPreprocessorConfiguration`,
  `TeamTalkMediaFilePlaybackConfiguration`,
  `TeamTalkUserMediaStorageConfiguration`, ...);
- typed IDs and option sets: `TeamTalkUserID`, `TeamTalkChannelID`,
  `TeamTalkFileID`, `TeamTalkTransferID`, `TeamTalkCommandID`,
  `TeamTalkSubscriptions`, `TeamTalkUserRights`, `TeamTalkStreamTypes`,
  `TeamTalkChannelTypes`, and the rest of `TeamTalkTypes.swift`;
- `TeamTalkEvent`, `TeamTalkEventObserver` and `events: AsyncStream<TeamTalkEvent>`
  instead of parsing raw `TTMessage` values directly.

`rawValue`/`cValue` remain on every wrapper type as an intentional escape
hatch - see [Advanced Usage](Advanced.md#raw-values-and-c-values). Reaching
for them from app code requires `import TeamTalkC` explicitly, since
`Exports.swift` re-exports nothing.

## Internal-Only Plumbing

The following exist to implement the public API above, not to be called
directly, and are already marked `internal`/`private` in source (not just
"discouraged by convention"):

- `TeamTalkSessionLegacyCommands.swift` - `Int32`-keyed command helpers
  (`joinChannel(id:password:)`, `kickUser(id:fromChannelID:)`, ...) that the
  typed, public overloads in `TeamTalkSessionCommands.swift` call into;
- raw C struct extensions on `Channel`, `User`, `RemoteFile`,
  `FileTransfer`, `TTMessage`, `AudioCodec`, `AudioPreprocessor` and related
  types, used by the snapshot/configuration models' own `init`/`cValue`
  implementations;
- `TeamTalkMessageObserver` for raw `TTMessage` observation - kept for
  completeness/debugging, but `TeamTalkEventObserver`/`events` are what app
  code should use.

Because these are already `internal` (or explicitly deprecated, in the
message-observer case), there's no further deprecation wave needed for
them - the compiler already keeps them out of a consumer's autocomplete.

## Investigated: Sync/Async Overload Pairs

Every mutating `TeamTalkSession` command exists as both a synchronous,
`@discardableResult` overload and an `async throws` overload with the same
argument labels (e.g. `logIn`, `joinChannel`, `logOut`). This looked like it
might be a design flaw - `try await session.logIn(...)` bound to a `let`
appeared to silently resolve to the sync overload in
`Examples/TeamTalkKitExample`. Investigated by reproducing the same call
inside a plain `async` function instead of top-level code: it resolved
correctly there with no annotation needed, and stayed correct across every
command pair in `TeamTalkSessionCommands.swift`/`TeamTalkSessionAsyncCommands.swift`.

So this isn't an API design problem - it's a Swift **top-level-code**
type-checking limitation (statements directly in a `main.swift`, outside
any function, don't get the same overload resolution as a function body).
`iTeamTalk` itself is unaffected, since none of its code is top-level.
See [Advanced Usage](Advanced.md#top-level-code-overload-gotcha) for the
mechanism and the fix for anyone writing a top-level script against this
package. No API change needed; closing this out rather than carrying it as
an open design question.

## Event Naming

Event naming is in good shape. Notable past cleanups:

- `connectionMaxPayloadUpdated(maxPayloadSize:)` reflects the actual SDK
  payload field instead of a misleading `source` label;
- `hotkey(hotkeyID:isActive:)` is clearer than `hotkey(id:isActive:)`.

Remaining raw scalar payloads in `TeamTalkEvent.Kind` (key codes, payload
sizes) are intentionally left as scalar values - they're not SDK identity
types, so a typed ID wrapper wouldn't add anything.

## Recommended Next Steps

1. Continue expanding `Examples/` as new API surface areas (files, admin,
   audio) get their own worked examples, rather than only documenting them
   in prose.
2. Validate macOS runtime behavior in a real (non-console) app target - see
   [Advanced Usage](Advanced.md#macos-notes).
