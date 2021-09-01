TEAMTALK_ROOT?=$(PWD)/..
CONFIGTYPE?=release
CMAKE_GENERATOR?=Ninja

SHELL := /bin/bash

help:
	echo "To make debug configuration specify CONFIGTYPE=debug:"
	echo "    make CONFIGTYPE=debug android-armeabi-v7a"

teamtalk-env:
	@echo "Testing that TEAMTALK_ROOT is valid."
	@echo "Source env.sh in repository root to set up environment"
	test -d "$(TEAMTALK_ROOT)"

genlib:
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && cmake $(CMAKE_EXTRA) -DCMAKE_BUILD_TYPE=$(CONFIGTYPE) -G "$(CMAKE_GENERATOR)" ../../

buildlib: genlib
	cmake --build $(BUILDDIR) $(CMAKE_BUILD_EXTRA)

android-build:
	@echo "Testing if ANDROID_NDK_HOME environment variable is set: $(ANDROID_NDK_HOME)"
	test -n "$(ANDROID_NDK_HOME)"
	$(MAKE) CONFIGTYPE=$(CONFIGTYPE) \
	CMAKE_EXTRA+="-DCMAKE_TOOLCHAIN_FILE=$(ANDROID_NDK_HOME)/build/cmake/android.toolchain.cmake -DANDROID_ABI=$(ANDROID_ABI) -DANDROID_PLATFORM=android-21 -DBUILD_TEAMTALK_CORE=ON -DBUILD_TEAMTALK5LIB=ON -DBUILD_TEAMTALK5PROLIB=ON -DBUILD_TEAMTALK_LIBRARIES=ON -DBUILD_TEAMTALK_CLIENTS=OFF -DBUILD_TEAMTALKPRO_SERVERS=OFF -DBUILD_TT5SRV=OFF -DBUILD_TT5PROSRV=OFF -DTOOLCHAIN_ZLIB=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON" buildlib

android-armeabi-v7a: teamtalk-env
	$(MAKE) ANDROID_ABI=armeabi-v7a BUILDDIR=build-$@ android-build

android-arm64-v8a: teamtalk-env
	$(MAKE) ANDROID_ABI=arm64-v8a BUILDDIR=build-$@ android-build

android-x86: teamtalk-env
	$(MAKE) ANDROID_ABI=x86 BUILDDIR=build-$@ android-build

android-x64: teamtalk-env
	$(MAKE) ANDROID_ABI=x86_64 BUILDDIR=build-$@ android-build

generic:
	$(MAKE) CMAKE_EXTRA+="-DBUILD_TEAMTALK_CORE=ON" buildlib

deb32:
	$(MAKE) BUILDDIR=build-$@ generic

deb64:
	$(MAKE) BUILDDIR=build-$@ generic

ubuntu64:
	# May need to specify JAVA_HOME to help CMake find Java SDK
	$(MAKE) BUILDDIR=build-$@ CMAKE_EXTRA+="-DCMAKE_POSITION_INDEPENDENT_CODE=ON" generic

centos7:
	# TeamTalk depends on CMake 3.1 or later.
	# To install CMake 3:
	# $ yum --enablerepo=extras install epel-release
	# Setup CMake 3 as default cmake:
	# $ ln -s /bin/cmake3 /bin/cmake
	$(MAKE) CMAKE_EXTRA+="-DBUILD_TEAMTALK5DLL=OFF -DBUILD_TEAMTALK5PRODLL=OFF -DSTATICCPP=OFF -DBUILD_TEAMTALK_LIBRARIES=OFF -DBUILD_TEAMTALKPRO_SERVERS=OFF -DBUILD_TEAMTALK_CLIENTS=OFF -DBUILD_TEAMTALK_CORE=ON -DFEATURE_WEBRTC=OFF" BUILDDIR=build-$@ buildlib

rasp:
	$(MAKE) CMAKE_EXTRA+="-DFEATURE_WEBRTC=OFF" BUILDDIR=build-$@ generic

mac:
	$(MAKE) CMAKE_EXTRA+="-DCMAKE_OSX_DEPLOYMENT_TARGET=10.10" BUILDDIR=build-$@ generic

ios-build:
	$(MAKE) CMAKE_EXTRA+="-DCMAKE_SYSTEM_NAME=iOS -DBUILD_TEAMTALK_CORE=ON -DBUILD_TEAMTALK5LIB=ON -DBUILD_TEAMTALK5PROLIB=ON -DBUILD_TEAMTALK5DLL=OFF -DBUILD_TEAMTALK5PRODLL=OFF -DFEATURE_OGG=OFF -DFEATURE_FFMPEG=OFF -DFEATURE_OPUSTOOLS=OFF -DBUILD_TEAMTALK_LIBRARIES=OFF -DBUILD_TEAMTALK_CLIENTS=OFF -DBUILD_TEAMTALKPRO_SERVERS=OFF -DBUILD_TT5SRV=OFF -DBUILD_TT5PROSRV=OFF -DTOOLCHAIN_ZLIB=OFF" buildlib

ios-armv7:
	$(MAKE) CMAKE_EXTRA+="-DCMAKE_OSX_ARCHITECTURES=armv7 -DCMAKE_OSX_SYSROOT=iphoneos" BUILDDIR=build-$@ ios-build
	$(MAKE) BUILDDIR=build-$@ ARCH=armv7 lipo-arch

ios-arm64:
	$(MAKE) CMAKE_EXTRA+="-DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_SYSROOT=iphoneos" BUILDDIR=build-$@ ios-build
	$(MAKE) BUILDDIR=build-$@ ARCH=arm64 lipo-arch

ios-i386:
	$(MAKE) CMAKE_EXTRA+="-DCMAKE_OSX_ARCHITECTURES=i386 -DCMAKE_OSX_SYSROOT=iphonesimulator" BUILDDIR=build-$@ ios-build
	$(MAKE) BUILDDIR=build-$@ ARCH=i386 lipo-arch

ios-x64:
	$(MAKE) CMAKE_EXTRA+="-DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_OSX_SYSROOT=iphonesimulator" BUILDDIR=build-$@ ios-build
	$(MAKE) BUILDDIR=build-$@ ARCH=x86_64 lipo-arch

ios-all: ios-armv7 ios-arm64 ios-i386 ios-x64 lipo

lipo-arch: teamtalk-env
	test -d "$(BUILDDIR)"
	libtool -static -o $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5-$(ARCH).a \
	$(BUILDDIR)/Library/TeamTalkLib/build/speex/install/lib/libspeex.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/speexdsp/install/lib/libspeexdsp.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/opus/install/lib/libopus.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/libvpx/install/lib/libvpx.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/ace/install/lib/libACE.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/ace/install/lib/libACE_SSL.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/ace/install/lib/libACE_INet.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/ace/install/lib/libACE_INet_SSL.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/openssl/install/lib/libcrypto.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/openssl/install/lib/libssl.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/webrtc/install/obj/modules/audio_processing/libteamtalk.a \
	$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/$(ARCH)/libTeamTalk5.a

	libtool -static -o $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5Pro-$(ARCH).a \
	$(BUILDDIR)/Library/TeamTalkLib/build/speex/install/lib/libspeex.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/speexdsp/install/lib/libspeexdsp.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/opus/install/lib/libopus.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/libvpx/install/lib/libvpx.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/ace/install/lib/libACE.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/ace/install/lib/libACE_SSL.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/ace/install/lib/libACE_INet.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/ace/install/lib/libACE_INet_SSL.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/openssl/install/lib/libcrypto.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/openssl/install/lib/libssl.a \
	$(BUILDDIR)/Library/TeamTalkLib/build/webrtc/install/obj/modules/audio_processing/libteamtalk.a \
	$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/$(ARCH)/libTeamTalk5Pro.a

lipo: teamtalk-env
	lipo $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5-i386.a \
	$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5-x86_64.a \
	$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5-armv7.a \
	$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5-arm64.a \
	-create -output $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5.a

	lipo $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5Pro-i386.a \
	$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5Pro-x86_64.a \
	$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5Pro-armv7.a \
	$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5Pro-arm64.a \
	-create -output $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5Pro.a

xcodeswap-apps:
	@echo "Changing to /Applications/Xcode.app/Contents/Developer"
	sudo xcode-select -s /Applications/Xcode.app/Contents/Developer

teamtalk-jar:
	$(MAKE) BUILDDIR=build-$@ CMAKE_EXTRA+="-DBUILD_TEAMTALK_CLIENTS=OFF -DBUILD_TEAMTALKPRO_SERVERS=OFF -DBUILD_TEAMTALK_LIBRARY_JNI=OFF -DBUILD_TEAMTALKPRO_LIBRARY_JNI=OFF -DBUILD_TEAMTALK_LIBRARY_DOTNET=OFF -DBUILD_TEAMTALKPRO_LIBRARY_DOTNET=OFF -DBUILD_TEAMTALK_LIBRARY_DOTNET=OFF -DBUILD_TEAMTALKPRO_LIBRARY_DOTNET=OFF" buildlib

depend-ubuntu18:
	# TeamTalk 5 dependencies
	apt install qt5-default libqt5x11extras5-dev qtmultimedia5-dev \
                    libqt5texttospeech5-dev qttools5-dev-tools doxygen \
                    openjdk-11-jdk ninja-build libpcap-dev junit4 cmake \
                    libssl-dev yasm autoconf libtool pkg-config \
                    libasound2-dev wget python

depend-debian9:
	# TeamTalk 5 dependencies
	apt install qt5-default qtbase5-dev libqt5x11extras5-dev qtmultimedia5-dev \
                    doxygen g++ openjdk-8-jdk \
                    qttools5-dev-tools ninja-build libpcap-dev junit4 cmake

depend-mac:
	brew install autoconf automake libtool pkg-config yasm

dox: teamtalk-env
	cd $(TEAMTALK_ROOT)/Documentation/TeamTalk && doxygen TeamTalk5.dox
	cd $(TEAMTALK_ROOT)/Documentation/TeamTalkSDK && doxygen TeamTalk.dox
	cd $(TEAMTALK_ROOT)/Documentation/TeamTalkSDK && doxygen TeamTalk.NET.dox
	cd $(TEAMTALK_ROOT)/Documentation/TeamTalkSDK && doxygen TeamTalk.Java.dox
