# Examples

Runnable, standalone demonstrations of the TeamTalkKit API, separate from
the full iTeamTalk app.

## TeamTalkKitExample

A macOS console client: connects to a server, logs in as a guest, lists and
joins the root channel, then prints text messages and join/leave events for
15 seconds before disconnecting.

```sh
swift run TeamTalkKitExample <host> [tcpPort] [udpPort] [nickname]

# e.g.
swift run TeamTalkKitExample talk.example.org 10333 10333 ExampleBot
```

It builds as part of this package (`swift build`/`swift run` from the
`TeamTalkKit/` directory) - no Xcode project needed. Read
[`TeamTalkKitExample/main.swift`](TeamTalkKitExample/main.swift) alongside
[`../Documentation/GettingStarted.md`](../Documentation/GettingStarted.md)
for a walkthrough of each step.

Guest login only works if the target server allows it; pass a real
`username`/`password` by editing the `logIn` call if yours doesn't.
