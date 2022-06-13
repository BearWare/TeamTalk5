# TeamTalkLib

Building the TeamTalk server and DLL binaries requires that the user
is familiar with [CMake](http://www.cmake.org). CMake is a build
system that is able to build TeamTalk binaries and generate project
files for various IDEs.

Prior to invoking CMake and start building the tools required to build
the TeamTalk dependencies (toolchain) must be installed. This is
explained in the following section.

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
* Install [Cygwin](https://www.cygwin.com) in `C:/cygwin64`
  * LibVPX requires *cygwin*
* Download [yasm](http://yasm.tortall.net)
    * Place `yasm.exe` for x64 in `C:/tt5dist/yasm/x64`
    * Place `yasm.exe` for Win32 in `C:/tt5dist/yasm/win32`
    * LibVPX requires *yasm*

### Install TeamTalk Dependencies on Ubuntu 18

A Makefile is available for Ubuntu 18 to install all the dependencies
required to build TeamTalk binaries.

* To install build dependencies for Ubuntu 18 run the following in
  TEAMTALK_ROOT:
  * `sudo make -C Build depend-ubuntu18`
    * `sudo` is required because `apt install` is called.

### Install TeamTalk Dependencies for Android on Ubuntu 18

Building for Android platform is supported on Ubuntu 18.

* To install build dependencies for Android on Ubuntu 18 run the
  following in TEAMTALK_ROOT:
  * `sudo make -C Build depend-ubuntu18`
  * `sudo` is required because `apt install` is called.
* Download [Android NDK r21e](https://developer.android.com/ndk) and
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

To build TeamTalk for Windows first start *x86 Native Tools Command
Prompt for VS 2019*. Use Git to clone
[TeamTalk5](https://github.com/BearWare/TeamTalk5) repository into
`C:\TeamTalk5`.

Now generate a valid build configuration using CMake:

`cmake -DBUILD_TEAMTALK_CORE=ON -S C:/TeamTalk5 -B C:/builddir -A Win32`

Given that CMake managed to create a valid build configuration now
start the build process:

`cmake --build C:/builddir --config Release --target install`

Note that WebRTC dependency will create a folder in `C:\webrtc` where
it downloads its repository.

### Build TeamTalk Binaries for Ubuntu 18

Run the following command in TEAMTALK_ROOT:

`make -C Build ubuntu18`

This will cause `make` to call CMake to generate a valid build
configuration and afterwards build the binaries.

### Build TeamTalk Binaries for Android on Ubuntu 18

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
can be activated using using the CMake options prefixed `TOOLCHAIN_`.

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
  * When `ON` builds OpenSSL.
  * When `OFF` uses OpenSSL installed on host.
  * `OFF` is only supported on Linux distributions and is recommended,
     since linking different versions of OpenSSL can cause problems.
* `TOOLCHAIN_ACE`
  * When `ON` builds ACE Framework
  * When `OFF` uses ACE Framework installed on host.
  * `OFF` is only supported on Linux distributions
  * Build ACE on Windows requires *ActivePerl* or *Strawberry Perl*
    * Place `perl.exe` in %PATH%.
* `TOOLCHAIN_TINYXML`
  * When `ON` builds TinyXML
  * When `OFF` uses TinyXML installed on host.
  * `OFF` is only supported on Linux distributions
* `TOOLCHAIN_ZLIB`
  * When `ON` builds ZLib
  * When `OFF` uses ZLib installed on host.
  * `OFF` is recommended on Linux, Android, iOS
* `TOOLCHAIN_CATCH2`
  * When `ON` enables Catch2 for unit tests
  * When `OFF` ignore Catch2 unit tests
* `TOOLCHAIN_LIBVPX`
  * When `ON` enables LibVPX
  * When `OFF` uses LibVPX installed on host.
  * Building LibVPX on Windows requires Cygwin, https://www.cygwin.com/
    * Install Cygwin in `C:/cygwin64`
  * Building LibVPX on Windows requires yasm, http://yasm.tortall.net/
    * Place `yasm.exe` for x64 in `C:/tt5dist/yasm/x64`
    * Place `yasm.exe` for Win32 in `C:/tt5dist/yasm/win32`
* `TOOLCHAIN_FFMPEG`
  * When `ON` builds FFmpeg
  * When `OFF` uses FFmpeg installed on host.
  * `OFF` is only supported on Linux distributions
* `TOOLCHAIN_OGG`
  * When `ON` builds OGG
  * When `OFF` uses OGG installed on host.
  * `OFF` is only supported on Linux distributions
* `TOOLCHAIN_OPUS`
  * When `ON` builds OPUS
  * When `OFF` uses OPUS installed on host.
  * `OFF` is only supported on Linux distributions
* `TOOLCHAIN_PORTAUDIO`
  * When `ON` builds PortAudio
  * When `OFF` uses PortAudio installed on host.
  * `OFF` is only supported on Linux distributions
* `TOOLCHAIN_SPEEX`
  * `OFF` is only supported on Linux distributions
  * When `ON` builds Speex
  * When `OFF` uses Speex installed on host.
* `TOOLCHAIN_SPEEXDSP`
  * When `ON` builds SpeexDSP
  * When `OFF` uses SpeexDSP installed on host.
  * `OFF` is only supported on Linux distributions

## Feature Toggles for TeamTalk Build Targets

What features to build into the TeamTalk binaries are controlled by
CMake options prefixed by `FEATURE_`.

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
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian
  * Toolchain mapping: `TOOLCHAIN_FFMPEG`
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

* `BUILD_TEAMTALK_DLL`
  * TeamTalk 5 shared library
  * Builds binary `TEAMTALK_ROOT/Library/TeamTalk_DLL/TeamTalk5.dll` or `libTeamTalk5.so`
* `BUILD_TEAMTALK_PRODLL`
  * TeamTalk 5 Pro shared library
  * Builds binary `TEAMTALK_ROOT/Library/TeamTalk_DLL/TeamTalk5Pro.dll` or `libTeamTalk5Pro.so`
* `BUILD_TEAMTALK_LIB`
  * TeamTalk 5 static library used for iOS and Android
  * Builds binary `TEAMTALK_ROOT/Library/TeamTalk_DLL/libTeamTalk5.a`
* `BUILD_TEAMTALK_PROLIB`
  * TeamTalk 5 Pro static library used for iOS and Android
  * Builds binary `TEAMTALK_ROOT/Library/TeamTalk_DLL/libTeamTalk5Pro.a`
* `BUILD_TEAMTALK_SRVEXE`
  * TeamTalk 5 console/daemon server
  * Builds binary `TEAMTALK_ROOT/Server/tt5srv.exe` or `tt5srv`
* `BUILD_TEAMTALK_PROSRVEXE`
  * TeamTalk 5 Pro console/daemon server
  * Builds binary `TEAMTALK_ROOT/Server/tt5prosrv.exe` or `tt5prosrv`
* `BUILD_TEAMTALK_SVCEXE`
  * TeamTalk 5 NT service server
  * Builds binary `TEAMTALK_ROOT/Server/tt5svc.exe`
* `BUILD_TEAMTALK_PROSVCEXE`
  * TeamTalk 5 NT service server
  * Builds binary `TEAMTALK_ROOT/Server/tt5prosvc.exe`
