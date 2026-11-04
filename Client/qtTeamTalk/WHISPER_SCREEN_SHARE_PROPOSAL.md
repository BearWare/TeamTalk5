# Targeted whisper and screen audio proposal

This is a draft implementation for maintainer review, based on upstream commit
`227e4422d3c1bb67317cac6e616f637ebee37072`. It contains source code for two
features from a Windows Qt client prototype. It does not include that client's
branding, installer, server profiles, or global Opus and jitter-buffer changes.

## Targeted whisper

Select another user in the channel tree and hold plain Space to transmit voice
to that user. Releasing Space ends the whisper. Focus changes, application
deactivation, and opening a modal dialog also end it. The shortcut is a proposed
UI choice for review. Start and stop sounds are configurable.

Implementation entry points:

- `mainwindow.cpp`: application event filter, `slotWhisperPressed()` and
  `slotWhisperReleased()`.
- `Library/TeamTalk_DLL/TeamTalk.h`: `TT_SetVoiceTransmissionTarget()`,
  `TT_GetVoiceTransmissionTarget()`, and `USERSTATE_WHISPER`.
- `ClientNode`, `PacketLayout`, and `PacketHelper`: destination-user routing and
  preservation of the destination through audio fragmentation and reassembly.
- `ClientUser` and `Convert`: received whisper state.
- `Library/TeamTalkJNI`: matching Java/JNI whisper API and state constant.

The destination must be in the same channel. A target of zero restores channel
voice routing. This uses TeamTalk's existing server routing and interception
model; administrators with interception rights can receive voice. It is not
end-to-end encryption. Mixed-version interoperability and transitions involving
already queued audio need dedicated review before this is ready to merge.

## Screen share and computer audio

Windows capture offers process loopback for the selected application's process
tree, or desktop audio excluding the TeamTalk process. An explicitly selected
ReaStream route listens on local UDP port 58710 with the configured identifier
(default `TeamTalk_REAPER`). A failed route does not silently switch to another
capture source. Microphone transmission remains controlled by the existing
TeamTalk controls.

The quality presets request different image sizes, update intervals and Opus
bitrates:

| Preset | Maximum height | Update interval | Requested audio bitrate |
| --- | --- | --- | --- |
| Low | 560 | 100 ms | 128 kbit/s |
| Medium | 720 | 67 ms | 320 kbit/s |
| High | 1080 | 50 ms | 510 kbit/s |

Audio is captured into 48 kHz stereo PCM blocks. The media encoder still uses
the channel codec: a preset cannot supply stereo or a higher sample rate if the
channel codec does not provide them. Opus bitrate is capped to the codec packet
budget. Screen audio is transported as media-file audio so receiver volume is
independent of microphone voice. It shares the media stream resource with media
file playback. Sharing continues across channel moves and stops when disabled.

Implementation entry points:

- `screenshareaudio.cpp/.h`: Windows capture worker, explicit ReaStream route,
  gain control, and COM resource lifetime.
- `screensharequality.h`: Opus packet-budget calculation.
- `mainwindow.cpp`: capture lifecycle, quality settings and desktop scaling.
- `preferencesdlg.cpp` and `desktopsharedlg.cpp`: source and preset controls.
- `userdesktopwidget.cpp`: fullscreen display and scaled input coordinates.
- `ClientNode` and `AudioThread`: separate screen-audio media encoder and bitrate
  updates. The prototype recognizes `AudioBlock.nStreamID == 0x5343524E` as
  screen audio; this implicit SDK contract should become an explicit API before
  acceptance.

## Validation and remaining work

On Windows x64 with MSVC 2022 and Qt 6.10.3, the SDK DLL and Qt client compile in
Release mode. The client links against the SDK built from this branch. Cached
third-party libraries were reused. No Linux, macOS, Qt 5, or JNI build has been
performed for this proposal.

The standalone test in `tests` checks Low/Medium/High bitrate targets, the
160 ms packet-size limit, an invalid interval and a non-Opus codec. It passes
with CTest. Build it separately with Qt Widgets available:

```sh
cmake -S Client/qtTeamTalk/tests -B build-quality
cmake --build build-quality --config Release
ctest --test-dir build-quality -C Release --output-on-failure
```

Compilation and the bitrate test do not establish end-to-end media correctness.
Before merging, validate three-client whisper isolation, fragmentation,
queued-audio transitions, normal voice restoration, keyboard/focus behavior,
channel moves, repeated capture start/stop and shutdown, ReaStream isolation,
and supported Windows versions with real devices and NVDA.

The prototype's PCM sample-rate conversion is stateless between capture packets
and uses linear interpolation. Non-48-kHz sources, especially 44.1 kHz timing
and downsampling from 96 kHz without anti-alias filtering, need a streaming
resampler before claiming maximum fidelity. Desktop capture performance,
unsupported/protected windows, protocol compatibility, API naming, localization,
and the screen-audio/media-file exclusivity also need maintainer review.
