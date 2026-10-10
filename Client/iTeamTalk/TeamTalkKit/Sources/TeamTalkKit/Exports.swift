// No raw C types or constants are re-exported here. The app talks to Swift
// model types only (TeamTalkChannel, TeamTalkUser, TeamTalkAudioCodecConfiguration,
// ...); files inside this package that still need a raw TeamTalkC symbol
// import TeamTalkC directly rather than relying on a re-export from this
// file. See Documentation/TODO.md's "Decide how much of TeamTalkC should
// remain re-exported long term" for how this file's contents were retired.
