TEAMTALK_ROOT?=$(PWD)/..
CONFIGTYPE?=release
CMAKE_GENERATOR?=Ninja

SHELL := /bin/bash

help:
	@echo "To make debug configuration specify CONFIGTYPE=debug:"
	@echo "    make CONFIGTYPE=debug android-armeabi-v7a"

teamtalk-env:
	@if [ -n "$(TEAMTALK_ROOT)" ] && [ -d "$(TEAMTALK_ROOT)" ]; then \
		echo "Using TEAMTALK_ROOT: $(TEAMTALK_ROOT)"; \
	else \
		echo "Error: TEAMTALK_ROOT is not set or directory does not exist"; \
		echo "Source env.sh in repository root to set up environment"; \
		exit 1; \
	fi

genlib:
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && cmake $(CMAKE_EXTRA) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=$(CONFIGTYPE) -G "$(CMAKE_GENERATOR)" ../../

buildlib: genlib
	cmake --build $(BUILDDIR) $(CMAKE_BUILD_EXTRA)

android-build:
	@if [ -n "$(ANDROID_NDK_HOME)" ] && [ -d "$(ANDROID_NDK_HOME)" ]; then \
		echo "Using ANDROID_NDK_HOME: $(ANDROID_NDK_HOME)"; \
	else \
		echo "Error: ANDROID_NDK_HOME is not set or directory does not exist"; \
		exit 1; \
	fi; \
	$(MAKE) CONFIGTYPE=$(CONFIGTYPE) \
        CMAKE_EXTRA+="-DCMAKE_TOOLCHAIN_FILE=$(ANDROID_NDK_HOME)/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI=$(ANDROID_ABI) \
        -DANDROID_PLATFORM=android-21 \
        -DBUILD_TEAMTALK_LIBRARY_LIB=ON \
        -DBUILD_TEAMTALK_LIBRARY_LIBPRO=ON \
        -DBUILD_TEAMTALK_LIBRARIES=ON \
        -DBUILD_TEAMTALK_CLIENTS=OFF \
        -DBUILD_TEAMTALK_SERVERS=OFF \
        -DBUILD_TEAMTALK_SERVER_SRVEXE=OFF \
        -DBUILD_TEAMTALK_SERVER_SRVEXEPRO=OFF \
        -DTOOLCHAIN_ZLIB=OFF \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON" \
        buildlib

android-armeabi-v7a: teamtalk-env
	$(MAKE) ANDROID_ABI=armeabi-v7a BUILDDIR=build-$(CONFIGTYPE)-$@ android-build

android-arm64-v8a: teamtalk-env
	$(MAKE) ANDROID_ABI=arm64-v8a BUILDDIR=build-$(CONFIGTYPE)-$@ android-build

android-x86: teamtalk-env
	$(MAKE) ANDROID_ABI=x86 BUILDDIR=build-$(CONFIGTYPE)-$@ android-build

android-x64: teamtalk-env
	$(MAKE) ANDROID_ABI=x86_64 BUILDDIR=build-$(CONFIGTYPE)-$@ android-build

android-all: android-armeabi-v7a android-arm64-v8a android-x86 android-x64

generic:
	$(MAKE) buildlib

ubuntu:
	@echo May need to specify JAVA_HOME to help CMake find Java SDK
	$(MAKE) BUILDDIR=build-$(CONFIGTYPE)-$(UBUNTUDIR) \
	CMAKE_EXTRA+="-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DTOOLCHAIN_OPENSSL=OFF" generic

ubuntu22:
	$(MAKE) UBUNTUDIR=$@ ubuntu

ubuntu24:
	$(MAKE) UBUNTUDIR=$@ ubuntu

rasp:
	$(MAKE) CMAKE_EXTRA+="-DFEATURE_WEBRTC=OFF \
                              -DTOOLCHAIN_OPENSSL=OFF" \
                              BUILDDIR=build-$(CONFIGTYPE)-$@ generic

mac:
	$(MAKE) BUILDDIR=build-$(CONFIGTYPE)-$@ CMAKE_EXTRA+="-DCMAKE_OSX_ARCHITECTURES=\"arm64;x86_64\"" generic

ios-build:
	$(MAKE) CMAKE_EXTRA+="-DCMAKE_SYSTEM_NAME=iOS \
        -DBUILD_TEAMTALK_LIBRARY_LIB=ON \
        -DBUILD_TEAMTALK_LIBRARY_LIBPRO=ON \
        -DBUILD_TEAMTALK_LIBRARY_DLL=OFF \
        -DBUILD_TEAMTALK_LIBRARY_DLLPRO=OFF \
        -DFEATURE_OGG=OFF \
        -DFEATURE_FFMPEG=OFF \
        -DFEATURE_OPUSTOOLS=OFF \
        -DBUILD_TEAMTALK_LIBRARIES=OFF \
        -DBUILD_TEAMTALK_CLIENTS=OFF \
        -DBUILD_TEAMTALK_SERVERS=OFF \
        -DBUILD_TEAMTALK_SERVER_SRVEXE=OFF \
        -DBUILD_TEAMTALK_SERVER_SRVEXEPRO=OFF \
        -DTOOLCHAIN_ZLIB=OFF" \
        buildlib

ios-arm64:
	$(MAKE) CMAKE_EXTRA+="-DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_SYSROOT=iphoneos -DTOOLCHAIN_INSTALL_PREFIX=$(INSTALLDIR)/arm64" BUILDDIR=build-$(CONFIGTYPE)-$@ ios-build
	$(MAKE) ARCH=arm64 lipo-arch

ios-x64:
	$(MAKE) CMAKE_EXTRA+="-DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_OSX_SYSROOT=iphonesimulator -DTOOLCHAIN_INSTALL_PREFIX=$(INSTALLDIR)/x86_64" BUILDDIR=build-$(CONFIGTYPE)-$@ ios-build
	$(MAKE) ARCH=x86_64 lipo-arch

ios-all: ios-arm64 ios-x64 lipo

lipo-arch: teamtalk-env
	test -d "$(INSTALLDIR)"
	libtool -static -o $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5-$(ARCH).a \
	$(INSTALLDIR)/$(ARCH)/speex/lib/libspeex.a \
	$(INSTALLDIR)/$(ARCH)/speexdsp/lib/libspeexdsp.a \
	$(INSTALLDIR)/$(ARCH)/opus/lib/libopus.a \
	$(INSTALLDIR)/$(ARCH)/libvpx/lib/libvpx.a \
	$(INSTALLDIR)/$(ARCH)/ace/lib/libACE.a \
	$(INSTALLDIR)/$(ARCH)/ace/lib/libACE_SSL.a \
	$(INSTALLDIR)/$(ARCH)/ace/lib/libACE_INet.a \
	$(INSTALLDIR)/$(ARCH)/ace/lib/libACE_INet_SSL.a \
	$(INSTALLDIR)/$(ARCH)/openssl/lib/libcrypto.a \
	$(INSTALLDIR)/$(ARCH)/openssl/lib/libssl.a \
	$(INSTALLDIR)/$(ARCH)/webrtc/obj/modules/audio_processing/libteamtalk.a \
	$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/$(ARCH)/libTeamTalk5.a

	libtool -static -o $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5Pro-$(ARCH).a \
	$(INSTALLDIR)/$(ARCH)/speex/lib/libspeex.a \
	$(INSTALLDIR)/$(ARCH)/speexdsp/lib/libspeexdsp.a \
	$(INSTALLDIR)/$(ARCH)/opus/lib/libopus.a \
	$(INSTALLDIR)/$(ARCH)/libvpx/lib/libvpx.a \
	$(INSTALLDIR)/$(ARCH)/ace/lib/libACE.a \
	$(INSTALLDIR)/$(ARCH)/ace/lib/libACE_SSL.a \
	$(INSTALLDIR)/$(ARCH)/ace/lib/libACE_INet.a \
	$(INSTALLDIR)/$(ARCH)/ace/lib/libACE_INet_SSL.a \
	$(INSTALLDIR)/$(ARCH)/openssl/lib/libcrypto.a \
	$(INSTALLDIR)/$(ARCH)/openssl/lib/libssl.a \
	$(INSTALLDIR)/$(ARCH)/webrtc/obj/modules/audio_processing/libteamtalk.a \
	$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/$(ARCH)/libTeamTalk5Pro.a

lipo: teamtalk-env
	lipo $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5-x86_64.a \
	$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5-arm64.a \
	-create -output $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5.a

	lipo $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5Pro-x86_64.a \
	$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5Pro-arm64.a \
	-create -output $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5Pro.a

xcodeswap-apps:
	@echo "Changing to /Applications/Xcode.app/Contents/Developer"
	sudo xcode-select -s /Applications/Xcode.app/Contents/Developer

teamtalk-jar:
	$(MAKE) BUILDDIR=build-$(CONFIGTYPE)-$@ CMAKE_EXTRA+="-DBUILD_TEAMTALK_CORE=OFF \
        -DBUILD_TEAMTALK_CLIENTS=OFF \
        -DBUILD_TEAMTALK_SERVERS=OFF \
        -DBUILD_TEAMTALK_DOCUMENTATION=OFF \
        -DBUILD_TEAMTALK_LIBRARY_JNI=OFF \
        -DBUILD_TEAMTALK_LIBRARY_JNIPRO=OFF" \
        buildlib

depend-ubuntu22:
	apt-get install \
		autoconf \
		cmake \
		doxygen \
		g++ \
		junit4 \
		libasound2-dev \
		libpcap-dev \
		libpulse-dev \
		libqt5texttospeech5-dev \
		libqt5x11extras5-dev \
		libssl-dev \
		libtool \
		libxss-dev \
		nasm \
		ninja-build \
		openjdk-25-jdk \
		p7zip-full \
		pkg-config \
		python-is-python3 \
		python3-pytest \
		qtbase5-dev \
		qtmultimedia5-dev \
		qttools5-dev \
		qttools5-dev-tools \
		wget

depend-ubuntu24:
	apt-get install \
		autoconf \
		cmake \
		doxygen \
		g++ \
		junit4 \
		libasound2-dev \
		libpcap-dev \
		libpulse-dev \
		libssl-dev \
		libtool \
		libxss-dev \
		nasm \
		ninja-build \
		openjdk-25-jdk \
		p7zip-full \
		pkg-config \
		python-is-python3 \
		python3-pytest \
		qt6-base-dev \
		qt6-declarative-dev \
		qt6-multimedia-dev \
		qt6-speech-dev \
		qt6-tools-dev \
		qt6-tools-dev-tools \
		wget \
		xz-utils

depend-ubuntu24-android:
	apt-get install \
		autoconf \
		cmake \
		doxygen \
		junit4 \
		libtool \
		nasm \
		ninja-build \
		openjdk-25-jdk \
		p7zip-full \
		pkg-config \
		wget \
		xz-utils

depend-raspios12:
	apt-get install \
		autoconf \
		cmake \
		doxygen \
		g++ \
		git \
		junit4 \
		libasound2-dev \
		libpcap-dev \
		libpulse-dev \
		libssl-dev \
		libtool \
		libxss-dev \
		nasm \
		ninja-build \
		openjdk-17-jdk \
		p7zip-full \
		pkg-config \
		python3-pytest \
		qt6-base-dev \
		qt6-declarative-dev \
		qt6-multimedia-dev \
		qt6-speech-dev \
		qt6-tools-dev \
		qt6-tools-dev-tools

depend-mac:
	brew install \
		autoconf \
		automake \
		doxygen \
		libtool \
		nasm \
		ninja \
		pkg-config \
		python-setuptools
