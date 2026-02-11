# TeamTalkLib

Building the TeamTalk server and DLL binaries requires that the user
is familiar with [CMake](http://www.cmake.org). CMake is a build
system that is able to build TeamTalk binaries and generate project
files for various IDEs.

The following secions explain how to build TeamTalk binaries using CMake:
* [Install TeamTalk Toolchain Dependencies](#install-teamTalk-toolchain-dependencies)
  * Install the tools required to build TeamTalk on the host build platform:
    * [Windows](#install-teamtalk-dependencies-on-windows)
    * [Ubuntu 22](#install-teamtalk-dependencies-on-ubuntu-22)
    * [Android on Ubuntu 22](#install-teamtalk-dependencies-for-android-on-ubuntu-22)
    * [macOS](#install-teamtalk-dependencies-on-macos)
    * [iOS on macOS](#install-teamtalk-dependencies-for-ios-on-macos)
* [Build TeamTalk Binaries](#build-teamTalk-binaries)
  * Invoke CMake to start building the TeamTalk binaries on the host build platform:
    * [Windows](#build-teamtalk-binaries-for-windows)
    * [Ubuntu 22](#build-teamtalk-binaries-for-ubuntu-22)
    * [Android on Ubuntu 22](#build-teamtalk-binaries-for-android-on-ubuntu-22)
    * [macOS](#build-teamtalk-binaries-for-macos)
    * [iOS on macOS](#build-teamtalk-binaries-for-ios-on-macos)
* [Toolchain Toggles for TeamTalk Build Targets](#toolchain-toggles-for-teamTalk-build-targets)
  * Choose if a tool required by TeamTalk should be built by CMake or use
    the tool already installed on the host build machine. E.g. use OPUS
    installed by `apt install libopus-dev` instead of manually building OPUS.
* [Feature Toggles for TeamTalk Build Targets](#feature-toggles-for-teamTalk-build-targets)
  * Choose what features should be compiled into the TeamTalk binaries.
    E.g. disable Speex in the TeamTalk DLL.
* [Toggles for TeamTalk Build Targets](#teamTalk-build-targets)
  * Toggle what TeamTalk binaries should be built by CMake. E.g. avoid
    building TeamTalk server.

## Install TeamTalk Toolchain Dependencies

TeamTalk depends on many external libraries, e.g. OpenSSL and OPUS,
and these libraries require a certain set of tools to be installed on
the build host in order to compile.

The following sections explain what tools to install on the build host
depending on the platform.

### Install TeamTalk Dependencies on Windows

Tools for building TeamTalk dependencies on Windows must be installed
manually.

* Install [ActivePerl](https://www.activestate.com/products/perl/) or
  [Strawberry Perl](https://strawberryperl.com/) on Windows
  * Place `perl.exe` in environment variable %PATH%.
    * OpenSSL and ACE Framework uses Perl
* Install [MSYS2](https://www.msys2.org/) in `C:/MSYS64` (recommended)
  * Run `pacman -S base-devel pkg-config` to install build tools
  * FFmpeg and LibVPX requires *MSYS2* or *Cygwin*
* Alternatively, install [Cygwin](https://www.cygwin.com/) in `C:/cygwin64`
  * Install `make` package during Cygwin setup
  * Cygwin is supported but MSYS2 is preferred
* Install [NASM](https://www.nasm.us)
    * Install to default location (`C:/Program Files/NASM`) or add to PATH
    * LibVPX requires *NASM*
    * NASM will be auto-downloaded if not found during CMake configuration

### Install TeamTalk Dependencies on Ubuntu

A Makefile is available for Ubuntu 22 and 24 to install all the dependencies
required to build TeamTalk binaries.

* To install build dependencies for Ubuntu 22 run the following in
  TEAMTALK_ROOT:
  * `sudo make -C Build depend-ubuntu22`
    * `sudo` is required because `apt install` is called.

* To install build dependencies for Ubuntu 24 run the following in
  TEAMTALK_ROOT:
  * `sudo make -C Build depend-ubuntu24`
    * `sudo` is required because `apt install` is called.

### Install TeamTalk Dependencies for Android on Ubuntu 24

Building for Android platform is supported on Ubuntu 24.

* To install build dependencies for Android on Ubuntu 24 run the
  following in TEAMTALK_ROOT:
  * `sudo make -C Build depend-ubuntu24-android`
    * `sudo` is required because `apt install` is called.
* Download [Android NDK r29](https://developer.android.com/ndk) and
  unzip it. Make environment variable `ANDROID_NDK_HOME` point to the
  unzipped location.

### Install TeamTalk Dependencies on macOS

Build for macOS platform is supported by using [Homebrew](https://brew.sh).

* To install build dependencies for macOS run the following in
  TEAMTALK_ROOT:
  * `make -C Build depend-mac`

### Install TeamTalk Dependencies for iOS on macOS

Build for iOS platform is supported by using [Homebrew](https://brew.sh) on macOS.

* To install build dependencies for iOS on macOS run the following in
  TEAMTALK_ROOT:
  * `make -C Build depend-mac`


## Build TeamTalk Binaries

Now that all required tool dependencies have been
[installed](#install-teamtalk-toolchain-dependencies) it is possible
to build the TeamTalk binaries. Again depending on the platform
there's different ways of doing this.

### Build TeamTalk Binaries for Windows

Building TeamTalk for Windows is supported by
[Visual Studio 2022](https://visualstudio.microsoft.com).

To build TeamTalk for Windows first start *x64 Native Tools Command
Prompt for VS 2022*. Use Git to clone
[TeamTalk5](https://github.com/BearWare/TeamTalk5) repository into
`C:\TeamTalk5`.

Use CMake to generate a valid build configuration in `C:\builddir`
that will install binaries into `C:\installdir`:

`cmake -DCMAKE_INSTALL_PREFIX=C:/installdir -S C:/TeamTalk5 -B C:/builddir -A x64`

Given that CMake managed to create a valid build configuration now
start the build process:

`cmake --build C:/builddir --config Release --target install`

To get a Visual Studio solution file for building TeamTalk from Visual
Studio 2022 run CMake like this:

`cmake -G "Visual Studio 17 2022" -S C:/TeamTalk5 -B C:/builddir -A x64`

Note that WebRTC dependency will create a folder in `C:\webrtc` where
it downloads its repository.

When both `FEATURE_FFMPEG` and `FEATURE_MEDIAFOUNDATION` are enabled,
FFmpeg is tried first and Media Foundation is used as fallback.

### Build TeamTalk Binaries for Ubuntu 22

Run the following command in TEAMTALK_ROOT:

`make -C Build ubuntu22`

This will cause `make` to call CMake to generate a valid build
configuration and afterwards build the binaries.

### Build TeamTalk Binaries for Android on Ubuntu 22

Run the following command in TEAMTALK_ROOT:

`make -C Build android-all`

This will build TeamTalk binaries for architectures *armeabi-v7a*,
*arm64-v8a*, *x86* and *x64*.

### Build TeamTalk Binaries for macOS

Run the following command in TEAMTALK_ROOT:

`make -C Build mac`

This will cause `make` to call CMake to generate a valid build
configuration and afterwards build the binaries.

### Build TeamTalk Binaries for iOS on macOS

Run the following command in TEAMTALK_ROOT:

`make -C Build ios-all`

This will build TeamTalk binaries for architectures *armv7*,
*arm64*, *i386* and *x64*.


## Toolchain Toggles for TeamTalk Build Targets

CMake is able to build all TeamTalk's dependencies as so-called
*ExternalProjects*. All dependencies in TeamTalk's toolchain
can be activated using the CMake options prefixed `TOOLCHAIN_`.

To e.g. have TeamTalk avoid building OPUS and instead use OPUS
already installed on the host machine, call CMake like this:

`cmake -DTOOLCHAIN_OPUS=OFF -S TeamTalk5 -B builddir`

The following toolchain toggles are available:

* `TOOLCHAIN_BUILD_PREFIX`
  * Build toolchain dependencies in the specified directory
* `TOOLCHAIN_INSTALL_PREFIX`
  * Install dependencies in the specified directory
* `TOOLCHAIN_BUILD_EXTERNALPROJECTS`
  * When `ON` builds all TeamTalk's dependencies and installs them into
    `TOOLCHAIN_INSTALL_PREFIX`
  * When `OFF` will make TeamTalk binaries build using the libraries
    installed in `TOOLCHAIN_INSTALL_PREFIX`. This is useful to avoid
    building the dependencies again after a new checkout or switching
    branch.
* `TOOLCHAIN_OPENSSL`
  * When `ON` builds [OpenSSL](https://github.com/openssl/openssl)
  * When `OFF` uses OpenSSL installed on host
    * `OFF` is only supported on Linux distributions and is recommended,
      since linking different versions of OpenSSL can cause problems.
* `TOOLCHAIN_ACE`
  * When `ON` builds [ACE Framework](https://github.com/DOCGroup/ACE_TAO)
  * When `OFF` uses ACE Framework installed on host
    * `OFF` is only supported on Linux distributions
  * Build ACE on Windows requires *ActivePerl* or *Strawberry Perl*
    * Place `perl.exe` in %PATH%.
* `TOOLCHAIN_TINYXML2`
  * When `ON` builds [TinyXML2](https://github.com/leethomason/tinyxml2)
  * When `OFF` uses TinyXML2 installed on host
    * `OFF` is only supported on Linux distributions
* `TOOLCHAIN_ZLIB`
  * When `ON` builds [ZLib](https://github.com/madler/zlib)
  * When `OFF` uses ZLib installed on host
    * `OFF` is recommended on Linux, Android, iOS
    * `OFF` is only supported on Linux distributions
* `TOOLCHAIN_CATCH2`
  * When `ON` enables [Catch2](https://github.com/catchorg/Catch2) for unit tests
  * When `OFF` ignore Catch2 unit tests
* `TOOLCHAIN_LIBVPX`
  * When `ON` enables [LibVPX](https://github.com/webmproject/libvpx)
  * When `OFF` uses LibVPX installed on host
    * `OFF` is only supported on Linux distributions
  * Building LibVPX on Windows requires MSYS2 or Cygwin
    * MSYS2 (recommended): Install in `C:/MSYS64` and run `pacman -S base-devel`
    * Cygwin (alternative): Install in `C:/cygwin64` with `make` package
  * Building LibVPX on Windows requires NASM, https://www.nasm.us/
    * Install to default location (`C:/Program Files/NASM`) or add to PATH
    * NASM will be auto-downloaded if not found
* `TOOLCHAIN_FFMPEG`
  * When `ON` builds [FFmpeg](https://github.com/FFmpeg/FFmpeg)
  * When `OFF` uses FFmpeg installed on host.
    * `OFF` is only supported on Linux distributions
* `TOOLCHAIN_OGG`
  * When `ON` builds [OGG](https://github.com/xiph/ogg)
  * When `OFF` uses OGG installed on host.
    * `OFF` is only supported on Linux distributions
* `TOOLCHAIN_OPUS`
  * When `ON` builds [OPUS](https://github.com/xiph/opus)
  * When `OFF` uses OPUS installed on host.
    * `OFF` is only supported on Linux distributions
* `TOOLCHAIN_PORTAUDIO`
  * When `ON` builds [PortAudio](https://github.com/PortAudio/portaudio)
  * When `OFF` uses PortAudio installed on host.
    * `OFF` is only supported on Linux distributions
* `TOOLCHAIN_SPEEX`
  * When `ON` builds [Speex](https://github.com/xiph/speex)
  * When `OFF` uses Speex installed on host.
    * `OFF` is only supported on Linux distributions
* `TOOLCHAIN_SPEEXDSP`
  * When `ON` builds [SpeexDSP](https://github.com/xiph/speexdsp)
  * When `OFF` uses SpeexDSP installed on host.
    * `OFF` is only supported on Linux distributions

## Feature Toggles for TeamTalk Build Targets

What features to build into the TeamTalk binaries are controlled by
CMake options prefixed by `FEATURE_`.

To e.g. have TeamTalk built without OPUS support invoke CMake like this:
`cmake -DFEATURE_OPUS=OFF -S TeamTalk5 -B builddir`

The following feature toggles are available:

* `FEATURE_OPUS`
  * OPUS audio codec
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian, Windows
  * Toolchain mapping: `TOOLCHAIN_OPUS`
* `FEATURE_OPUSTOOLS`
  * OPUS .ogg file support
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian, Windows
* `FEATURE_SPEEX`
  * Speex audio codec
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian, Windows
  * Toolchain mapping: `TOOLCHAIN_SPEEX`
* `FEATURE_SPEEXDSP`
  * SpeexDSP for audio processing (resampler, denoiser, AGC)
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian, Windows
  * Toolchain mapping: `TOOLCHAIN_SPEEXDSP`
* `FEATURE_FFMPEG`
  * FFmpeg for streaming and audio resampling
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian, Windows
  * Toolchain mapping: `TOOLCHAIN_FFMPEG`
  * Building FFmpeg on Windows requires MSYS2
    * Install in `C:/MSYS64` and run `pacman -S base-devel`
* `FEATURE_V4L2`
  * Video for Linux 2 for video capture support
  * Supported platforms: Ubuntu/Linux, Raspbian
* `FEATURE_AVF`
  * Audio Video Foundation for video capture support
  * Supported platforms: macOS
* `FEATURE_WEBRTC`
  * WebRTC for audio processing
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Windows
* `FEATURE_MSDMO`
  * Microsoft DirectX Media Object for audio resampling
  * Supported platforms: Windows
* `FEATURE_MEDIAFOUNDATION`
  * Microsoft Media Foundation for audio and video streaming
  * Supported platforms: Windows
* `FEATURE_LIBVPX`
  * libvpx (VP8) video codec
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian, Windows
  * Toolchain mapping: `TOOLCHAIN_LIBVPX`
* `FEATURE_OGG`
  * OGG file format
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian, Windows
  * Toolchain mapping: `TOOLCHAIN_OGG`
* `FEATURE_PORTAUDIO`
  * PortAudio sound system
  * Supported platforms: macOS, Ubuntu/Linux, Raspbian, Windows
  * Toolchain mapping: `TOOLCHAIN_PORTAUDIO`
* `FEATURE_OPENSLES`
  * OpenSL ES sound system
  * Supported platforms: Android
* `FEATURE_AUDIOUNIT`
  * AudioUnit sound system
  * Supported platforms: iOS

## TeamTalk Build Targets

Which TeamTalk binaries to build are controlled by CMake options
prefixed by `BUILD_TEAMTALK_`.

Most interesting are:

* `BUILD_TEAMTALK_LIBRARY_DLL`
  * TeamTalk 5 shared library
  * Builds binary `TEAMTALK_ROOT/Library/TeamTalk_DLL/TeamTalk5.dll` or `libTeamTalk5.so`
* `BUILD_TEAMTALK_LIBRARY_DLLPRO`
  * TeamTalk 5 Pro shared library
  * Builds binary `TEAMTALK_ROOT/Library/TeamTalk_DLL/TeamTalk5Pro.dll` or `libTeamTalk5Pro.so`
* `BUILD_TEAMTALK_LIBRARY_LIB`
  * TeamTalk 5 static library used for iOS and Android
  * Builds binary `TEAMTALK_ROOT/Library/TeamTalk_DLL/libTeamTalk5.a`
* `BUILD_TEAMTALK_LIBRARY_LIBPRO`
  * TeamTalk 5 Pro static library used for iOS and Android
  * Builds binary `TEAMTALK_ROOT/Library/TeamTalk_DLL/libTeamTalk5Pro.a`
* `BUILD_TEAMTALK_SERVER_SRVEXE`
  * TeamTalk 5 console/daemon server
  * Builds binary `TEAMTALK_ROOT/Server/tt5srv.exe` or `tt5srv`
* `BUILD_TEAMTALK_SERVER_SRVEXEPRO`
  * TeamTalk 5 Pro console/daemon server
  * Builds binary `TEAMTALK_ROOT/Server/tt5prosrv.exe` or `tt5prosrv`
* `BUILD_TEAMTALK_SERVER_SVCEXE`
  * TeamTalk 5 NT service server
  * Builds binary `TEAMTALK_ROOT/Server/tt5svc.exe`
* `BUILD_TEAMTALK_SERVER_SVCEXEPRO`
  * TeamTalk 5 NT service server
  * Builds binary `TEAMTALK_ROOT/Server/tt5prosvc.exe`
