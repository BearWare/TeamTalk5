TEAMTALK_ROOT?=$(PWD)/..
CONFIGTYPE?=release
CMAKE_GENERATOR?=Ninja

SHELL := /bin/bash

help:
	@echo "To make debug configuration specify CONFIGTYPE=debug:"
	@echo "    make CONFIGTYPE=debug android-armeabi-v7a"

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
	$(MAKE) ANDROID_ABI=armeabi-v7a BUILDDIR=build-$@ android-build

android-arm64-v8a: teamtalk-env
	$(MAKE) ANDROID_ABI=arm64-v8a BUILDDIR=build-$@ android-build

android-x86: teamtalk-env
	$(MAKE) ANDROID_ABI=x86 BUILDDIR=build-$@ android-build

android-x64: teamtalk-env
	$(MAKE) ANDROID_ABI=x86_64 BUILDDIR=build-$@ android-build

android-all: android-armeabi-v7a android-arm64-v8a android-x86 android-x64

generic:
	$(MAKE) buildlib

ubuntu:
	@echo May need to specify JAVA_HOME to help CMake find Java SDK
	$(MAKE) BUILDDIR=build-$(UBUNTUDIR) \
	CMAKE_EXTRA+="-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DTOOLCHAIN_OPENSSL=OFF" generic

ubuntu18:
	$(MAKE) CMAKE_EXTRA+="-DBUILD_TEAMTALK_LIBRARY_DLL=OFF \
                              -DBUILD_TEAMTALK_LIBRARY_DLLPRO=OFF \
                              -DBUILD_TEAMTALK_LIBRARIES=OFF \
                              -DBUILD_TEAMTALK_SERVERS=OFF \
                              -DBUILD_TEAMTALK_CLIENTS=OFF \
                              -DTOOLCHAIN_OPENSSL=OFF \
                              -DTOOLCHAIN_ZLIB=OFF \
                              -DTOOLCHAIN_CATCH2=OFF \
                              -DTOOLCHAIN_LIBVPX=OFF \
                              -DTOOLCHAIN_FFMPEG=OFF \
                              -DTOOLCHAIN_OGG=OFF \
                              -DTOOLCHAIN_OPUS=OFF \
                              -DTOOLCHAIN_PORTAUDIO=OFF \
                              -DTOOLCHAIN_SPEEX=OFF \
                              -DTOOLCHAIN_SPEEXDSP=OFF \
                              -DFEATURE_OPUSTOOLS=OFF \
                              -DFEATURE_WEBRTC=OFF \
                              -DBUILD_TEAMTALK_DOCUMENTATION=OFF" UBUNTUDIR=$@ ubuntu
ubuntu19:
	$(MAKE) UBUNTUDIR=$@ ubuntu
ubuntu20:
	$(MAKE) UBUNTUDIR=$@ ubuntu
ubuntu22:
	$(MAKE) UBUNTUDIR=$@ ubuntu
ubuntu24:
	$(MAKE) UBUNTUDIR=$@ ubuntu

rasp:
	$(MAKE) CMAKE_EXTRA+="-DFEATURE_WEBRTC=OFF \
                              -DTOOLCHAIN_OPENSSL=OFF" \
                              BUILDDIR=build-$@ generic

mac:
	$(MAKE) BUILDDIR=build-$@ CMAKE_EXTRA+="-DCMAKE_OSX_ARCHITECTURES=\"arm64;x86_64\"" generic

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

IOS_INSTALL_DIR?=$(PWD)/ios-install

ios-armv7:
	$(MAKE) CMAKE_EXTRA+="-DCMAKE_OSX_ARCHITECTURES=armv7 -DCMAKE_OSX_SYSROOT=iphoneos -DTOOLCHAIN_INSTALL_PREFIX=$(INSTALLDIR)/armv7" BUILDDIR=build-$@ ios-build
	$(MAKE) ARCH=armv7 lipo-arch

ios-arm64:
	$(MAKE) CMAKE_EXTRA+="-DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_SYSROOT=iphoneos -DTOOLCHAIN_INSTALL_PREFIX=$(INSTALLDIR)/arm64" BUILDDIR=build-$@ ios-build
	$(MAKE) ARCH=arm64 lipo-arch

ios-i386:
	$(MAKE) CMAKE_EXTRA+="-DCMAKE_OSX_ARCHITECTURES=i386 -DCMAKE_OSX_SYSROOT=iphonesimulator -DTOOLCHAIN_INSTALL_PREFIX=$(INSTALLDIR)/i386" BUILDDIR=build-$@ ios-build
	$(MAKE) ARCH=i386 lipo-arch

ios-x64:
	$(MAKE) CMAKE_EXTRA+="-DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_OSX_SYSROOT=iphonesimulator -DTOOLCHAIN_INSTALL_PREFIX=$(INSTALLDIR)/x86_64" BUILDDIR=build-$@ ios-build
	$(MAKE) ARCH=x86_64 lipo-arch

ios-all: ios-armv7 ios-arm64 ios-i386 ios-x64 lipo

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
	$(MAKE) BUILDDIR=build-$@ CMAKE_EXTRA+="-DBUILD_TEAMTALK_CLIENTS=OFF \
        -DBUILD_TEAMTALK_SERVERS=OFF \
        -DBUILD_TEAMTALK_LIBRARY_JNI=OFF \
        -DBUILD_TEAMTALK_LIBRARY_JNIPRO=OFF \
        -DBUILD_TEAMTALK_LIBRARY_DLL_DOTNET=OFF \
        -DBUILD_TEAMTALK_LIBRARY_DLLPRO_DOTNET=OFF \
        -DBUILD_TEAMTALK_LIBRARY_DLL_DOTNET=OFF \
        -DBUILD_TEAMTALK_LIBRARY_DLLPRO_DOTNET=OFF" \
        buildlib

# TeamTalk 5 dependencies for Ubuntu 18
depend-ubuntu18:
# Duplicate install in Docker/Dockerfile_ubuntu18
	apt install \
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
		libxss-dev
		ninja-build \
		openjdk-11-jdk \
		pkg-config \
		python \
		python3-pytest \
		qt5-default \
		qtmultimedia5-dev \
		qttools5-dev \
		qttools5-dev-tools \
		yasm

depend-ubuntu20:
# Duplicate install in Docker/Dockerfile_ubuntu20
	apt install \
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
		ninja-build \
		openjdk-17-jdk \
		pkg-config \
		python \
		qt5-default \
		qtmultimedia5-dev \
		qttools5-dev \
		qttools5-dev-tools \
		yasm

depend-ubuntu22:
# Duplicate install in Docker/Dockerfile_ubuntu22
	apt install \
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
		ninja-build \
		openjdk-18-jdk \
		p7zip-full \
		pkg-config \
		python2 \
		python3-pytest \
		qtbase5-dev \
		qtmultimedia5-dev \
		qttools5-dev \
		qttools5-dev-tools \
		yasm
	@echo "Ensure a default python is set up for WebRTC to compile:"
	@echo "# sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10"

depend-ubuntu24:
# Duplicate install in Docker/Dockerfile_ubuntu24
	apt install \
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
		ninja-build \
		openjdk-21-jdk \
		p7zip-full \
		pkg-config \
		python3-pytest \
		qt6-base-dev \
		qt6-declarative-dev \
		qt6-multimedia-dev \
		qt6-speech-dev \
		qt6-tools-dev \
		qt6-tools-dev-tools \
		yasm
	@echo "Ensure a default python is set up for WebRTC to compile:"
	@echo "# sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 12"

depend-ubuntu18-android:
	apt install \
		autoconf \
		cmake \
		doxygen \
		junit4 \
		libtool \
		ninja-build \
		openjdk-11-jdk \
		pkg-config \
		python

depend-ubuntu22-android:
	apt install \
		autoconf \
		cmake \
		doxygen \
		junit4 \
		libtool \
		ninja-build \
		openjdk-18-jdk \
		pkg-config \
		python2 \
		python3-pytest

depend-debian12:
	apt install \
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
		qt6-tools-dev-tools \
		yasm

depend-raspios12:
	apt install \
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
		qt6-tools-dev-tools \
		libxss-dev \
		yasm

depend-mac:
	brew install \
		autoconf \
		automake \
		libtool \
		ninja \
		pkg-config \
		python-setuptools \
		yasm
