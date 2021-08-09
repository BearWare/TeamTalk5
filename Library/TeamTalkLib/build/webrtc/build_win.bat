@ECHO ------------------------------------------------------------------
@ECHO Build WebRTC for TeamTalk
@ECHO ------------------------------------------------------------------
@ECHO Ensure 'Windows Search' (WSearch) service is Disabled and Stopped.
@ECHO Failure to do so may result in file system errors.
@ECHO ------------------------------------------------------------------
@ECHO This Command Prompt must run in Visual Studio x86 or x64 mode
@PAUSE
@ECHO ------------------------------------------------------------------
@ECHO Download 'depot_tools' from Chromium Projects and extract it in C:\webrtc\depot_tools.
@ECHO Trying to place WebRTC in another directory will most likely give problems.
@PAUSE

@SET INITWEBRTC=%CD%
@SET WEBRTC_ROOT=C:\webrtc
@SET WEBRTC_INSTALL=%WEBRTC_ROOT%\install\%Platform%
@SET DEPOT_TOOLS_WIN_TOOLCHAIN=0
@ECHO Adding depot_tools to PATH
CD %WEBRTC_ROOT%\depot_tools
@IF NOT %ERRORLEVEL% == 0 GOTO buildfail
SET PATH=%CD%;%PATH%
@ECHO ------------------------------------------------------------------
@ECHO Cloning WebRTC is a slow process so here's a menu to skip steps.
@ECHO Start at 1 if this is the first time running this BAT file.
@CHOICE /C:1234 /M "1 for Clone WebRTC, 2 for checkout WebRTC, 3 for Generate GN files, 4 for Build"
@IF %ERRORLEVEL% == 1 GOTO clone
@IF %ERRORLEVEL% == 2 GOTO checkout
@IF %ERRORLEVEL% == 3 GOTO gengnfiles
@IF %ERRORLEVEL% == 4 GOTO build

:clone
@ECHO ------------------------------------------------------------------
@ECHO Fetching WebRTC repository. This will take a long time.
CD %WEBRTC_ROOT%
@IF NOT %ERRORLEVEL% == 0 GOTO buildfail
CALL fetch --nohooks webrtc

:checkout
@ECHO ------------------------------------------------------------------
@ECHO Checking out branch
CD %WEBRTC_ROOT%\src
@IF NOT %ERRORLEVEL% == 0 GOTO buildfail
CALL git clean -xdf .
CALL git checkout modules/audio_processing/BUILD.gn
CALL git fetch -p -t
CALL git checkout branch-heads/4332
@ECHO Synchronizing checkout
CALL gclient sync -D --with_branch_heads --with_tags
@ECHO ------------------------------------------------------------------
@ECHO Applying patch to build TeamTalk.lib
CALL git apply %INITWEBRTC%\libteamtalk-r4332.patch
@IF NOT %ERRORLEVEL% == 0 GOTO buildfail

:gengnfiles
@ECHO ------------------------------------------------------------------
@ECHO Set up Visual Studio version for WebRTC
@IF %Platform% == x86 GOTO win32
@IF %Platform% == x64 GOTO win64

:win32
@SET ARCH=x86
@GOTO generate
:win64
@SET ARCH=x64
@GOTO generate

:generate
@ECHO ------------------------------------------------------------------
@ECHO Checking for OpenSSL installation using OPENSSL_DIR environment variable
@IF NOT EXIST %OPENSSL_DIR% GOTO buildfail

CD %WEBRTC_ROOT%\src
@IF NOT %ERRORLEVEL% == 0 GOTO buildfail
DEL /S /Q %WEBRTC_INSTALL%\lib\debug
CALL gn gen %WEBRTC_INSTALL%\lib\debug --args="is_clang=false target_cpu=\"%ARCH%\" rtc_disable_logging=true rtc_exclude_field_trial_default=true rtc_enable_protobuf=false rtc_enable_sctp=false rtc_include_tests=false rtc_build_examples=false rtc_build_libvpx=false rtc_libvpx_build_vp9=false rtc_include_opus=false rtc_build_ssl=false rtc_builtin_ssl_root_certificates=false rtc_ssl_root=\"%OPENSSL_DIR%\include\" is_debug=true use_custom_libcxx=false enable_iterator_debugging=true"
@IF NOT %ERRORLEVEL% == 0 GOTO buildfail

DEL /S /Q %WEBRTC_INSTALL%\lib\release
CALL gn gen %WEBRTC_INSTALL%\lib\release --args="is_clang=false target_cpu=\"%ARCH%\" rtc_disable_logging=true rtc_exclude_field_trial_default=true rtc_enable_protobuf=false rtc_enable_sctp=false rtc_include_tests=false rtc_build_examples=false rtc_build_libvpx=false rtc_libvpx_build_vp9=false rtc_include_opus=false rtc_build_ssl=false rtc_builtin_ssl_root_certificates=false rtc_ssl_root=\"%OPENSSL_DIR%\include\" is_debug=false use_custom_libcxx=false"
@IF NOT %ERRORLEVEL% == 0 GOTO buildfail

:build
@ECHO ------------------------------------------------------------------
CD %WEBRTC_ROOT%\src
CALL ninja -v -C %WEBRTC_INSTALL%\lib\debug teamtalk
@IF NOT %ERRORLEVEL% == 0 GOTO buildfail
CALL ninja -v -C %WEBRTC_INSTALL%\lib\release teamtalk
@IF NOT %ERRORLEVEL% == 0 GOTO buildfail

:copyfiles
CD %WEBRTC_ROOT%\src
DEL /S /Q %WEBRTC_INSTALL%\include
MKDIR %WEBRTC_INSTALL%\include
XCOPY *.h /S %WEBRTC_INSTALL%\include

@ECHO ------------------------------------------------------------------
@ECHO Finishing building WebRTC for TeamTalk
@ECHO You can now delete %WEBRTC_ROOT%
@GOTO quit

:buildfail
@ECHO A build step failed

:quit
@CD %INITWEBRTC%
