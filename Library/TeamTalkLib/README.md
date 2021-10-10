# TeamTalkLib

Building TeamTalkLib is enabled by toggling the CMake option
*BUILD_TEAMTALK_CORE=ON*. This will cause CMake to process the
TeamTalkLib folder and expose the build options for TeamTalkLib.

## TeamTalk Binaries

Which TeamTalk binaries to build are controlled by CMake options
prefixed by *BUILD_TEAMTALK_*.

Most interesting are:

* BUILD_TEAMTALK_DLL
  * TeamTalk 5 shared library
  * Builds TeamTalk5.dll/libTeamTalk5.so
* BUILD_TEAMTALK_PRODLL
  * TeamTalk 5 Pro shared library
  * Builds TeamTalk5Pro.dll/libTeamTalk5Pro.so
* BUILD_TEAMTALK_LIB
  * TeamTalk 5 static library used for iOS and Android
  * Builds libTeamTalk5.a  
* BUILD_TEAMTALK_PROLIB
  * TeamTalk 5 Pro static library used for iOS and Android
  * Builds libTeamTalk5Pro.a  
* BUILD_TEAMTALK_SRVEXE
  * TeamTalk 5 console/daemon server
  * Builds tt5srv.exe/tt5srv
* BUILD_TEAMTALK_PROSRVEXE
  * TeamTalk 5 Pro console/daemon server
  * Builds tt5prosrv.exe/tt5prosrv
* BUILD_TEAMTALK_SVCEXE
  * TeamTalk 5 NT service server
  * Builds tt5svc.exe
* BUILD_TEAMTALK_PROSVCEXE
  * TeamTalk 5 NT service server
  * Builds tt5prosvc.exe

## Feature Toggles for TeamTalk Binaries

What features to build into the TeamTalk binaries are controlled by
CMake options prefixed by *FEATURE_*.

The following feature toggles are available:

* FEATURE_OPUS
  * OPUS audio codec
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian, Windows
  * Toolchain mapping: *TOOLCHAIN_OPUS*
* FEATURE_OPUSTOOLS
  * OPUS .ogg file support
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian, Windows
* FEATURE_SPEEX
  * Speex audio codec
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian, Windows
  * Toolchain mapping: *TOOLCHAIN_SPEEX*
* FEATURE_SPEEXDSP
  * SpeexDSP for audio processing (resampler, denoiser, AGC)
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian, Windows
  * Toolchain mapping: *TOOLCHAIN_SPEEXDSP*
* FEATURE_FFMPEG
  * FFmpeg for streaming and audio resampling
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian
  * Toolchain mapping: *TOOLCHAIN_FFMPEG*
* FEATURE_V4L2
  * Video for Linux 2 for video capture support
  * Supported platforms: Ubuntu/Linux, Raspbian
* FEATURE_AVF
  * Audio Video Foundation for video capture support
  * Supported platforms: macOS
* FEATURE_WEBRTC
  * WebRTC for audio processing
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Windows
* FEATURE_MSDMO
  * Microsoft DirectX Media Object for audio resampling
  * Supported platforms: Windows
* FEATURE_MEDIAFOUNDATION
  * Microsoft Media Foundation for audio and video streaming
  * Supported platforms: Windows
* FEATURE_LIBVPX
  * libvpx (VP8) video codec
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian, Windows
  * Toolchain mapping: *TOOLCHAIN_LIBVPX*
* FEATURE_OGG
  * OGG file format
  * Supported platforms: macOS, iOS, Android, Ubuntu/Linux, Raspbian, Windows
  * Toolchain mapping: *TOOLCHAIN_OGG*
* FEATURE_PORTAUDIO
  * PortAudio sound system
  * Supported platforms: macOS, Ubuntu/Linux, Raspbian, Windows
  * Toolchain mapping: *TOOLCHAIN_PORTAUDIO*
* FEATURE_OPENSLES
  * OpenSL ES sound system
  * Supported platforms: Android
* FEATURE_AUDIOUNIT
  * AudioUnit sound system
  * Supported platforms: iOS

## Toolchain Toggles for TeamTalk Binaries

CMake is able to build all TeamTalk's dependencies using the CMake
options prefixed *TOOLCHAIN_*.

On Ubuntu, Raspbian, etc. it may be desirable to use ```apt install```
to install TeamTalk dependencies instead of manually building
them. E.g. to build TeamTalk binaries using an installed version of
OpenSSL pass ```-DTOOLCHAIN_OPENSSL=OFF```. This will make CMake
compile using the the OpenSSL version installed in
```/usr/include/openssl``` instead of building OpenSSL manually.

Being able to build TeamTalk's toolchain requires several dependencies
to be installed on the build machine. Go to TEAMTALK_ROOT/Build and
install the dependencies:

* For macOS run
* ```make depend-mac```
* For iOS on macOS run
* ```make depend-mac```
* For Ubuntu 18 run:
* ```make depend-ubuntu18```
* For Android on Ubuntu 18 run:
* ```make depend-ubuntu18```

The following toolchain toggles are available:

* TOOLCHAIN_BUILD_PREFIX
  * Build toolchain dependencies in the specified directory
* TOOLCHAIN_INSTALL_PREFIX
  * Install dependencies in the specified directory  
* TOOLCHAIN_BUILD_EXTERNALPROJECTS
  * When *ON* builds all TeamTalk's dependencies and installs them into
    *TOOLCHAIN_INSTALL_PREFIX*
  * When *OFF* will make TeamTalk binaries build using the libraries
    installed in *TOOLCHAIN_INSTALL_PREFIX*. This is useful to avoid
    building the dependencies again after a new checkout or switching
    branch.
* TOOLCHAIN_OPENSSL
  * When *ON* builds OpenSSL.
  * When *OFF* uses OpenSSL installed on host.
  * *OFF* is only supported on Linux distributions and is recommended,
     since linking different versions of OpenSSL can cause problems.
* TOOLCHAIN_ACE
  * When *ON* builds ACE Framework
  * When *OFF* uses ACE Framework installed on host.
  * *OFF* is only supported on Linux distributions
  * Build ACE on Windows requires *ActivePerl* or *Strawberry Perl*
    * Place ```perl.exe``` in %PATH%.
* TOOLCHAIN_TINYXML
  * When *ON* builds TinyXML
  * When *OFF* uses TinyXML installed on host.
  * *OFF* is only supported on Linux distributions
* TOOLCHAIN_ZLIB
  * When *ON* builds ZLib
  * When *OFF* uses ZLib installed on host.
  * *OFF* is recommended on Linux, Android, iOS
* TOOLCHAIN_CATCH2
  * When *ON* enables Catch2 for unit tests
  * When *OFF* ignore Catch2 unit tests
* TOOLCHAIN_LIBVPX
  * When *ON* enables LibVPX
  * When *OFF* uses TinyXML installed on host.
  * Building LibVPX on Windows requires Cygwin, https://www.cygwin.com/
    * Install Cygwin in ```C:/cygwin64```
  * Building LibVPX on Windows requires yasm, http://yasm.tortall.net/
    * Place ```yasm.exe``` for x64 in ```C:/tt5dist/yasm/x64```
    * Place ```yasm.exe``` for Win32 in ```C:/tt5dist/yasm/win32```
* TOOLCHAIN_FFMPEG
  * When *ON* builds FFmpeg
  * When *OFF* uses FFmpeg installed on host.
  * *OFF* is only supported on Linux distributions
* TOOLCHAIN_OGG
  * When *ON* builds OGG
  * When *OFF* uses OGG installed on host.
  * *OFF* is only supported on Linux distributions
* TOOLCHAIN_OPUS
  * When *ON* builds OPUS
  * When *OFF* uses OPUS installed on host.
  * *OFF* is only supported on Linux distributions
* TOOLCHAIN_PORTAUDIO
  * When *ON* builds PortAudio
  * When *OFF* uses PortAudio installed on host.
  * *OFF* is only supported on Linux distributions
* TOOLCHAIN_SPEEX
  * When *ON* builds Speex
  * When *OFF* uses Speex installed on host.
  * *OFF* is only supported on Linux distributions
* TOOLCHAIN_SPEEXDSP
  * When *ON* builds SpeexDSP
  * When *OFF* uses SpeexDSP installed on host.
  * *OFF* is only supported on Linux distributions
