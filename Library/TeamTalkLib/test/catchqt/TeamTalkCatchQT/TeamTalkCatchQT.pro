TEMPLATE = app
QT += gui   # needed to compile for Android
CONFIG += c++14
android: QT+= androidextras
DEFINES += CATCH_MAIN_ALREADY_PROVIDED DEBUG _DEBUG

# Paths - $$PWD is path to QT .pro file
TEAMTALKLIB_ROOT = $$PWD/../../../../TeamTalkLib
TTLIBS_ROOT = $$TEAMTALKLIB_ROOT/toolchain
android: TTLIBS_ROOT = $$TEAMTALKLIB_ROOT/toolchain/armeabi-v7a #TODO: add all architectures

###################################################
#   INCLUDES
###################################################
# Catch2
INCLUDEPATH += $$TTLIBS_ROOT/Catch2/single_include

# Includes for the 3rd parth libs
INCLUDEPATH += $$TTLIBS_ROOT/ACE/include
INCLUDEPATH += $$TTLIBS_ROOT/opus-tools/src
INCLUDEPATH += $$TTLIBS_ROOT/openssl/include
INCLUDEPATH += $$TTLIBS_ROOT/speex/include
INCLUDEPATH += $$TTLIBS_ROOT/ffmpeg/include
INCLUDEPATH += $$TTLIBS_ROOT/webrtc/include
INCLUDEPATH += $$TTLIBS_ROOT/webrtc/include/modules/audio_processing/include
INCLUDEPATH += $$TTLIBS_ROOT/webrtc/include/third_party/abseil-cpp
INCLUDEPATH += $$TTLIBS_ROOT/ogg/include
INCLUDEPATH += $$TTLIBS_ROOT/opus/include
INCLUDEPATH += $$TTLIBS_ROOT/libvpx/include
INCLUDEPATH += $$TTLIBS_ROOT/zlib/include

# Includes for TeamTalk project headers
INCLUDEPATH += $$TEAMTALKLIB_ROOT
INCLUDEPATH += $$TEAMTALKLIB_ROOT/../TeamTalk_DLL

###################################################
#   Libs
###################################################
# ACE
LIBS += $$TTLIBS_ROOT/ACE/lib/libACE_INet_SSL.a
LIBS += $$TTLIBS_ROOT/ACE/lib/libACE_SSL.a
LIBS += $$TTLIBS_ROOT/ACE/lib/libACE_INet.a
LIBS += $$TTLIBS_ROOT/ACE/lib/libACE.a
LIBS += -ldl
# Speex
LIBS += $$TTLIBS_ROOT/speex/lib/libspeex.a
LIBS += $$TTLIBS_ROOT/speex/lib/libspeexdsp.a
# ffmpeg, use static
LIBS += $$TTLIBS_ROOT/ffmpeg/lib/libavdevice.a
LIBS += $$TTLIBS_ROOT/ffmpeg/lib/libavfilter.a
LIBS += $$TTLIBS_ROOT/ffmpeg/lib/libavformat.a
LIBS += $$TTLIBS_ROOT/ffmpeg/lib/libavcodec.a
LIBS += $$TTLIBS_ROOT/ffmpeg/lib/libswresample.a
LIBS += $$TTLIBS_ROOT/ffmpeg/lib/libswscale.a
LIBS += $$TTLIBS_ROOT/ffmpeg/lib/libavutil.a
unix:!android: LIBS += -lasound # Linux only
#webrtc
LIBS += $$TTLIBS_ROOT/webrtc/obj/modules/audio_processing/libteamtalk.a
# ogg
LIBS += $$TTLIBS_ROOT/ogg/lib/libogg.a
# opus
LIBS += $$TTLIBS_ROOT/opus/lib/libopus.a
# vpx
LIBS += $$TTLIBS_ROOT/libvpx/lib/libvpx.a
# zlib, use system lib on Android
!android: LIBS += $$TTLIBS_ROOT/zlib/lib/libz.a
android:  LIBS += -lz

# openssl use static ssl libs on Android, system libs on linux
android: LIBS += $$TTLIBS_ROOT/openssl/lib/libssl.a
android: LIBS += $$TTLIBS_ROOT/openssl/lib/libcrypto.a
!android: LIBS += -lssl -lcrypto

# System libs. OpenSLES and log lib on Android.
android: LIBS += -lOpenSLES -llog


###################################################
#   Build flags
###################################################

#QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -DENABLE_ENCRYPTION -D__ACE_INLINE__  # ace
QMAKE_CXXFLAGS += -D__STDC_CONSTANT_MACROS # ffmpeg
QMAKE_CXXFLAGS += -DENABLE_OPUS -DENABLE_OGG -DENABLE_OPUSTOOLS -DENABLE_SPEEX \
                  -DENABLE_FFMPEG3 -DENABLE_VPX -DENABLE_WEBRTC

android: DEFINES += ENABLE_OPENSLES

###################################################
#   Sources
###################################################

SOURCES +=  main.cpp \
            $$TEAMTALKLIB_ROOT/test/CatchDefault.cpp \
            $$TEAMTALKLIB_ROOT/test/CatchMain.cpp \
            $$TEAMTALKLIB_ROOT/test/TTUnitTest.cpp \
            $$TEAMTALKLIB_ROOT/toolchain/armeabi-v7a/opus-tools/src/opus_header.c \
            $$TEAMTALKLIB_ROOT/myace/MyACE.cpp \
            $$TEAMTALKLIB_ROOT/myace/TimerHandler.cpp \
            $$TEAMTALKLIB_ROOT/mystd/MyStd.cpp \
            $$TEAMTALKLIB_ROOT/avstream/AudioResampler.cpp \
            $$TEAMTALKLIB_ROOT/avstream/VideoCapture.cpp \
            $$TEAMTALKLIB_ROOT/avstream/SoundLoopback.cpp \
            $$TEAMTALKLIB_ROOT/avstream/SoundSystem.cpp \
            $$TEAMTALKLIB_ROOT/avstream/SoundSystemEx.cpp \
            $$TEAMTALKLIB_ROOT/avstream/MediaStreamer.cpp \
            $$TEAMTALKLIB_ROOT/avstream/MediaPlayback.cpp \
            $$TEAMTALKLIB_ROOT/avstream/AudioInputStreamer.cpp \
            $$TEAMTALKLIB_ROOT/avstream/SpeexPreprocess.cpp \
            $$TEAMTALKLIB_ROOT/avstream/SpeexResampler.cpp \
            $$TEAMTALKLIB_ROOT/avstream/FFmpegStreamer.cpp \
            $$TEAMTALKLIB_ROOT/avstream/FFmpegResampler.cpp \
            $$TEAMTALKLIB_ROOT/avstream/WebRTCPreprocess.cpp \
            $$TEAMTALKLIB_ROOT/avstream/WebRTCPreprocess.h \
            $$TEAMTALKLIB_ROOT/avstream/OpusFileStreamer.cpp \
            $$TEAMTALKLIB_ROOT/codec/BmpFile.cpp \
            $$TEAMTALKLIB_ROOT/codec/WaveFile.cpp \
            $$TEAMTALKLIB_ROOT/codec/MediaUtil.cpp \
            $$TEAMTALKLIB_ROOT/codec/SpeexEncoder.cpp \
            $$TEAMTALKLIB_ROOT/codec/SpeexDecoder.cpp \
            $$TEAMTALKLIB_ROOT/codec/OpusEncoder.cpp \
            $$TEAMTALKLIB_ROOT/codec/OpusDecoder.cpp \
            $$TEAMTALKLIB_ROOT/codec/VpxEncoder.cpp \
            $$TEAMTALKLIB_ROOT/codec/VpxDecoder.cpp \
            $$TEAMTALKLIB_ROOT/codec/OggFileIO.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/Channel.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/CodecCommon.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/Commands.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/Common.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/DesktopSession.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/PacketHandler.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/PacketHelper.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/PacketLayout.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/StreamHandler.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/ttassert.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/User.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/client/AudioContainer.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/client/AudioThread.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/client/ClientChannel.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/client/ClientNode.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/client/ClientNodeBase.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/client/ClientUser.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/client/FileNode.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/client/StreamPlayers.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/client/VideoThread.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/client/VoiceLogger.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/client/AudioMuxer.cpp \
            $$TEAMTALKLIB_ROOT/teamtalk/client/DesktopShare.cpp \
            $$TEAMTALKLIB_ROOT/bin/dll/Convert.cpp \
            $$TEAMTALKLIB_ROOT/bin/dll/TTClientMsg.cpp \
            $$TEAMTALKLIB_ROOT/bin/dll/TeamTalk.cpp \

android: SOURCES +=  $$TEAMTALKLIB_ROOT/avstream/OpenSLESWrapper.cpp

###################################################
#   Deployment of test data. Can be accessed from Android as "assets:/File"
###################################################

# - setup the 'make install' step to make the INSTALLS work!
# the dir "assets" is special because it will be copied into the APK
AGC.path = /assets/testdata/AGC
AGC.files += $$TEAMTALKLIB_ROOT/test/testdata/AGC/*
INSTALLS += AGC

Jitter.path = /assets/testdata/Jitter
Jitter.files += $$TEAMTALKLIB_ROOT/test/testdata/Jitter/*
INSTALLS += Jitter

mp3.path = /assets/testdata/mp3
mp3.files += $$TEAMTALKLIB_ROOT/test/testdata/mp3/*
INSTALLS += mp3

Noise.path = /assets/testdata/Noise
Noise.files += $$TEAMTALKLIB_ROOT/test/testdata/Noise/*
INSTALLS += Noise

Opus.path = /assets/testdata/Opus
Opus.files += $$TEAMTALKLIB_ROOT/test/testdata/Opus/*
INSTALLS += Opus

Video.path = /assets/testdata/Video
Video.files += $$TEAMTALKLIB_ROOT/test/testdata/Video/*
INSTALLS += Video

###################################################
#   Deployment of Android stuff for building.
###################################################
DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
