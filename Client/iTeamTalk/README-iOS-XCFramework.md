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

### iOS (arm64 device + x64 simulator)

```sh
make -C Build lipo
```

`lipo` runs two sub-builds and writes the results to
`Library/TeamTalk_DLL/`:

| Archive                              | Platform / arch                    | SDK                        |
| ------------------------------------ | ---------------------------------- | -------------------------- |
| `libTeamTalk5-arm64.a`               | iOS **device** arm64               | `iphoneos`                 |
| `libTeamTalk5-x86_64-simulator.a`    | iOS **simulator** x86_64 (Intel)   | `iphonesimulator`          |
| `libTeamTalk5.a`                     | iOS: x86_64 sim + arm64 device     | `iphoneos/iphonesimulator` |

Each also has a `libTeamTalk5Pro-*.a` counterpart (Professional Edition, which
additionally exposes the server API in `TeamTalkSrv.h`).

### iOS (arm64 simulator and x64 simulator)

```sh
make -C Build lipo-simulator
```

`lipo-simulator` runs two sub-builds and writes the results to
`Library/TeamTalk_DLL/`:

| Archive                              | Platform / arch                    | SDK               |
| ------------------------------------ | ---------------------------------- | ----------------- |
| `libTeamTalk5-x86_64-simulator.a`    | iOS **simulator** x86_64 (Intel)   | `iphonesimulator` |
| `libTeamTalk5-arm64-simulator.a`     | iOS **simulator** arm64 (Apple Si) | `iphonesimulator` |
| `libTeamTalk5-simulator.a`           | iOS: x86_64 sim + arm64 sim        | `iphonesimulator` |

Each also has a `libTeamTalk5Pro-*.a` counterpart (Professional Edition, which
additionally exposes the server API in `TeamTalkSrv.h`).

### macOS

```sh
make -C Build mac-lib
```

Plain `make -C Build mac` will **not** produce what the XCFramework step
below needs: `BUILD_TEAMTALK_LIBRARY_LIB` defaults to `OFF` and
`BUILD_TEAMTALK_LIBRARY_DLL` to `ON` (see
`Library/TeamTalkLib/CMakeLists.txt`), so `mac` only ever builds
`libTeamTalk5.dylib`, never the static `libTeamTalk5.a` this document
assembles into an XCFramework. `mac-lib` sets those flags the other way,
building a universal (`arm64;x86_64`) static archive into
`Library/TeamTalk_DLL/` as `libTeamTalk5.a` / `libTeamTalk5Pro.a`. If you
have just run `lipo` or `lipo-simulator`, run `mac-lib` into a clean
checkout or move the iOS archives aside first, because both write the
same `libTeamTalk5.a` filename.

## 2. Assemble the XCFrameworks

The native archives ship **without** headers; the SPM `TeamTalkC` target
provides the C headers (`Sources/TeamTalkC/include/`), so the XCFrameworks are
library-only.

Assemble TeamTalkNativeiOS.xcframework for arm64 device and x86_64 simulator

```sh
make -C Client/iTeamTalk/TeamTalkKit teamtalkkit-ios
```

Assemble TeamTalkNativeiOS.xcframework for arm64 simulator and x86_64 simulator

```sh
make -C Client/iTeamTalk/TeamTalkKit teamtalkkit-ios-simulator
```

Assemble TeamTalkNativemacOS.xcframework for arm64 simulator and x86_64 simulator

```sh
make -C Client/iTeamTalk/TeamTalkKit teamtalkkit-macos
```

For the Professional Edition, repeat with the `libTeamTalk5Pro-*.a` archives
and whatever output names your package variant expects.

## 3. Verify

```sh
# slices present and correctly tagged
xcodebuild -checkFirstLaunchStatus >/dev/null 2>&1 || true
lipo -info $TEAMTALK_ROOT/Client/iTeamTalk/TeamTalkKit/Vendor/TeamTalkNativeiOS.xcframework/ios-arm64/libTeamTalk5-arm64.a
lipo -info $TEAMTALK_ROOT/Client/iTeamTalk/TeamTalkKit/Vendor/TeamTalkNativeiOS.xcframework/ios-x86_64-simulator/libTeamTalk5-x86_64-simulator.a

# package resolves and builds against the device slice
cd "$TEAMTALK_ROOT/Client/iTeamTalk/TeamTalkKit"
swift build -Xswiftc -sdk -Xswiftc "$(xcrun --sdk iphoneos --show-sdk-path)" \
            -Xswiftc -target -Xswiftc arm64-apple-ios18.0
```

The exact `Info.plist` slice directory names inside the `.xcframework`
(`ios-arm64`, `ios-x86_64-simulator`, `macos-arm64_x86_64`) are chosen
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
