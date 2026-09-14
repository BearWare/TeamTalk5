# Building the iOS / macOS XCFrameworks for TeamTalkKit

The Swift package `Client/iTeamTalk/TeamTalkKit` links the native TeamTalk
client library through two binary targets declared in its `Package.swift`:

| Binary target            | File (relative to the package)               |
| ------------------------ | -------------------------------------------- |
| `TeamTalkNativeiOS`      | `Vendor/TeamTalkNativeiOS.xcframework`       |
| `TeamTalkNativemacOS`    | `Vendor/TeamTalkNativemacOS.xcframework`     |

Both `.xcframework` bundles are **git-ignored** (see the package's
`.gitignore`). They are build products, not source, and have to be produced
locally (or in CI) from `Library/TeamTalkLib` before the package will build.
This document is the procedure for producing them.

## Prerequisites

- macOS with Xcode and the command-line tools (`xcode-select -p` should point
  into `Xcode.app`).
- The repository environment sourced from the repo root:

  ```sh
  source env.sh
  ```

- Disk space and time for the first build: it clones WebRTC into
  `$HOME/webrtc/iOS` (and `$HOME/webrtc/Darwin` for macOS), which is a large
  checkout.

## 1. Build the static archives

All commands are run from `TEAMTALK_ROOT` (the repository root).

### iOS (device + simulator)

```sh
make -C Build ios-all
```

`ios-all` runs three sub-builds and two `lipo` steps and writes the results
to `Library/TeamTalk_DLL/`:

| Archive                              | Platform / arch                    | SDK               |
| ------------------------------------ | ---------------------------------- | ----------------- |
| `libTeamTalk5-arm64.a`               | iOS **device** arm64              | `iphoneos`        |
| `libTeamTalk5-x86_64.a`              | iOS **simulator** x86_64 (Intel)  | `iphonesimulator` |
| `libTeamTalk5-arm64-simulator.a`     | iOS **simulator** arm64 (Apple Si)| `iphonesimulator` |
| `libTeamTalk5-simulator.a`           | fat: x86_64 + arm64 **simulator** | —                 |
| `libTeamTalk5.a`                     | legacy fat: x86_64 sim + arm64 dev| —                 |

Each also has a `libTeamTalk5Pro-*.a` counterpart (Professional Edition, which
additionally exposes the server API in `TeamTalkSrv.h`).

Use `libTeamTalk5-arm64.a` for the device and `libTeamTalk5-simulator.a` for
the simulator. **Do not** use `libTeamTalk5.a` for the XCFramework — it mixes
a device slice and a simulator slice under the same `arm64`/`x86_64` names and
`xcodebuild -create-xcframework` will reject it.

### macOS

```sh
make -C Build mac
```

This builds a universal (`arm64;x86_64`) archive into `Library/TeamTalk_DLL/`
as `libTeamTalk5.a` / `libTeamTalk5Pro.a`. If you have just run `ios-all`,
run `mac` into a clean checkout or move the iOS archives aside first, because
both write the same `libTeamTalk5.a` filename.

## 2. Assemble the XCFrameworks

The native archives ship **without** headers; the SPM `TeamTalkC` target
provides the C headers (`Sources/TeamTalkC/include/`), so the XCFrameworks are
library-only.

```sh
DLL="$TEAMTALK_ROOT/Library/TeamTalk_DLL"
VENDOR="$TEAMTALK_ROOT/Client/iTeamTalk/TeamTalkKit/Vendor"

# iOS: one device slice + one simulator slice
rm -rf "$VENDOR/TeamTalkNativeiOS.xcframework"
xcodebuild -create-xcframework \
  -library "$DLL/libTeamTalk5-arm64.a" \
  -library "$DLL/libTeamTalk5-simulator.a" \
  -output  "$VENDOR/TeamTalkNativeiOS.xcframework"

# macOS: the universal slice
rm -rf "$VENDOR/TeamTalkNativemacOS.xcframework"
xcodebuild -create-xcframework \
  -library "$DLL/libTeamTalk5.a" \
  -output  "$VENDOR/TeamTalkNativemacOS.xcframework"
```

For the Professional Edition, repeat with the `libTeamTalk5Pro-*.a` archives
and whatever output names your package variant expects.

## 3. Verify

```sh
# slices present and correctly tagged
xcodebuild -checkFirstLaunchStatus >/dev/null 2>&1 || true
lipo -info "$VENDOR/TeamTalkNativeiOS.xcframework"/ios-arm64/libTeamTalk5-arm64.a
lipo -info "$VENDOR/TeamTalkNativeiOS.xcframework"/ios-arm64_x86_64-simulator/libTeamTalk5-simulator.a

# package resolves and builds against the device slice
cd "$TEAMTALK_ROOT/Client/iTeamTalk/TeamTalkKit"
swift build -Xswiftc -sdk -Xswiftc "$(xcrun --sdk iphoneos --show-sdk-path)" \
            -Xswiftc -target -Xswiftc arm64-apple-ios18.0
```

The exact `Info.plist` slice directory names inside the `.xcframework`
(`ios-arm64`, `ios-arm64_x86_64-simulator`, `macos-arm64_x86_64`) are chosen
by `xcodebuild`; check them with `plutil -p .../Info.plist` if a path above
does not match.

## The "same CPU, different platform" gotcha

An `arm64` iOS **device** binary and an `arm64` iOS **simulator** binary have
the same Mach-O CPU type. `lipo` will happily fat-join them, but the result is
unusable: the linker can't tell which slice is for which platform, and
`-create-xcframework` refuses it. Keep them in separate XCFramework library
entries (that's the whole point of an XCFramework). Only ever `lipo` together
slices that target the **same** platform — which is why `lipo-simulator`
combines `x86_64` + `arm64-simulator` and nothing else.

## When to rebuild

Regenerate the XCFrameworks whenever the native SDK changes: a bump of the
vendored `TeamTalk.h` / `TeamTalkC.*`, a change under `Library/TeamTalkLib`,
or a new dependency version in a `build/*/CMakeLists.txt`. There is no
automatic dependency between the Swift package and `Library/TeamTalkLib`, so a
stale XCFramework will link and silently run old native code.
