JOBS?=4
TOOLCHAIN_ROOT?=$(PWD)/../Library/TeamTalkLib/toolchain
TEAMTALK_ROOT?=$(PWD)/..
CONFIGTYPE?=release

SHELL := /bin/bash

help:
	echo "To make debug configuration specify CONFIGTYPE=debug:"
	echo "    make CONFIGTYPE=debug android-armeabi-v7a"

teamtalk-env:
	@echo "Testing that TEAMTALK_ROOT is valid."
	@echo "Source env.sh in repository root to set up environment"
	test -d "$(TEAMTALK_ROOT)"

toolchain-env:
	@echo "Testing that TOOLCHAIN_ROOT is valid, i.e. source Library/TeamTalkLib/toolchain/toolchain.sh"
	test -d "$(TOOLCHAIN_ROOT)"

genlib:
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && cmake $(CMAKE_EXTRA) -DCMAKE_BUILD_TYPE=$(CONFIGTYPE) ../../

buildlib: genlib
	make -C $(BUILDDIR) VERBOSE=1 all -j$(JOBS)

android-build:
	@echo "Testing if NDK environment variable is set"
	test -n "$NDK"
	make CONFIGTYPE=$(CONFIGTYPE) \
	CMAKE_EXTRA="-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) -DBUILD_TEAMTALK_CORE=ON -DBUILD_TEAMTALK_LIBRARIES=OFF -DBUILD_TEAMTALK_CLIENTS=OFF -DBUILD_TEAMTALKPRO_SERVERS=OFF -DBUILD_TT5SRV=OFF -DBUILD_TT5PROSRV=OFF" buildlib
	cd $(TEAMTALK_ROOT)/Library/TeamTalkJNI/jni && $(NDK)/ndk-build

android-armeabi-v7a: teamtalk-env toolchain-env
	. $(TOOLCHAIN_ROOT)/toolchain.sh android armeabi-v7a && make TEAMTALK_ROOT=$(TEAMTALK_ROOT) TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-android-armeabi-v7a.cmake BUILDDIR=build-$@ android-build

android-arm64-v8a: teamtalk-env toolchain-env
	. $(TOOLCHAIN_ROOT)/toolchain.sh android arm64-v8a && make TEAMTALK_ROOT=$(TEAMTALK_ROOT) TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-android-arm64-v8a.cmake BUILDDIR=build-$@ android-build

android-x86: teamtalk-env toolchain-env
	. $(TOOLCHAIN_ROOT)/toolchain.sh android x86 && make TEAMTALK_ROOT=$(TEAMTALK_ROOT) TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-android-x86.cmake BUILDDIR=build-$@ android-build

android-x64: teamtalk-env toolchain-env
	. $(TOOLCHAIN_ROOT)/toolchain.sh android x86_64 && make TEAMTALK_ROOT=$(TEAMTALK_ROOT) TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-android-x86_64.cmake BUILDDIR=build-$@ android-build

generic:
	make CMAKE_EXTRA="-DBUILD_TEAMTALK_CORE=ON $(CMAKE_EXTRA)" buildlib

deb32:
	make BUILDDIR=build-$@ generic

deb64:
	make BUILDDIR=build-$@ generic

ubuntu64:
	# Need to specify JAVA_HOME to help CMake find Java SDK
	make JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64 BUILDDIR=build-$@ generic

centos7:
	make CMAKE_EXTRA="-DBUILD_TEAMTALK5DLL=OFF -DBUILD_TEAMTALK5PRODLL=OFF -DSTATICCPP=OFF -DBUILD_TEAMTALK_LIBRARIES=OFF -DBUILD_TEAMTALKPRO_SERVERS=OFF -DBUILD_TEAMTALK_CLIENTS=OFF -DBUILD_TEAMTALK_CORE=ON" BUILDDIR=build-$@ buildlib

rasp:
	make JOBS=2 JAVA_HOME=/usr/lib/jvm/java-8-openjdk-armhf BUILDDIR=build-$@ generic

mac:
	make BUILDDIR=build-$@ generic

ios-build:
	make CMAKE_EXTRA="$(CMAKE_EXTRA) -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_DEPLOYMENT_TARGET=7.0 -DBUILD_TEAMTALK_CORE=ON -DOGG=OFF -DFFMPEG=OFF -DOPUSTOOLS=OFF -DBUILD_TEAMTALK_LIBRARIES=OFF -DBUILD_TEAMTALK_CLIENTS=OFF -DBUILD_TEAMTALKPRO_SERVERS=OFF -DBUILD_TT5SRV=OFF -DBUILD_TT5PROSRV=OFF -DZLIB_STATIC=OFF" buildlib

ios-armv7: toolchain-env
	make CMAKE_EXTRA="-DCMAKE_OSX_ARCHITECTURES=armv7 -DCMAKE_OSX_SYSROOT=iphoneos" BUILDDIR=build-$@ ios-build
	. $(TOOLCHAIN_ROOT)/toolchain.sh ios armv7 && make lipo-arch

ios-arm64: toolchain-env
	make CMAKE_EXTRA="-DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_SYSROOT=iphoneos" BUILDDIR=build-$@ ios-build
	. $(TOOLCHAIN_ROOT)/toolchain.sh ios arm64 && make lipo-arch

ios-i386: toolchain-env
	make CMAKE_EXTRA="-DCMAKE_OSX_ARCHITECTURES=i386 -DCMAKE_OSX_SYSROOT=iphonesimulator" BUILDDIR=build-$@ ios-build
	. $(TOOLCHAIN_ROOT)/toolchain.sh ios i386 && make lipo-arch

ios-x64: toolchain-env
	make CMAKE_EXTRA="-DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_OSX_SYSROOT=iphonesimulator" BUILDDIR=build-$@ ios-build
	. $(TOOLCHAIN_ROOT)/toolchain.sh ios x86_64 && make lipo-arch

ios-all: ios-armv7 ios-arm64 ios-i386 ios-x64 lipo

lipo-arch: toolchain-env teamtalk-env
	libtool -static -o $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5-$(ARCH).a \
	$(TTLIBS_ROOT)/speex/lib/libspeex.a \
	$(TTLIBS_ROOT)/speex/lib/libspeexdsp.a \
	$(TTLIBS_ROOT)/opus/lib/libopus.a \
	$(TTLIBS_ROOT)/libvpx/lib/libvpx.a \
	$(TTLIBS_ROOT)/ACE/lib/libACE.a \
	$(TTLIBS_ROOT)/ACE/lib/libACE_SSL.a \
	$(TTLIBS_ROOT)/ACE/lib/libACE_INet.a \
	$(TTLIBS_ROOT)/ACE/lib/libACE_INet_SSL.a \
	$(TTLIBS_ROOT)/openssl/lib/libcrypto.a \
	$(TTLIBS_ROOT)/openssl/lib/libssl.a \
	$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/$(ARCH)/libTeamTalk5.a

	libtool -static -o $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5Pro-$(ARCH).a \
	$(TTLIBS_ROOT)/speex/lib/libspeex.a \
	$(TTLIBS_ROOT)/speex/lib/libspeexdsp.a \
	$(TTLIBS_ROOT)/opus/lib/libopus.a \
	$(TTLIBS_ROOT)/libvpx/lib/libvpx.a \
	$(TTLIBS_ROOT)/ACE/lib/libACE.a \
	$(TTLIBS_ROOT)/ACE/lib/libACE_SSL.a \
	$(TTLIBS_ROOT)/ACE/lib/libACE_INet.a \
	$(TTLIBS_ROOT)/ACE/lib/libACE_INet_SSL.a \
	$(TTLIBS_ROOT)/openssl/lib/libcrypto.a \
	$(TTLIBS_ROOT)/openssl/lib/libssl.a \
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

xcodeswap-toolchain: toolchain-env
	@echo "Changing to $(TOOLCHAIN_ROOT)/Xcode.app/Contents/Developer"
	sudo xcode-select -s $(TOOLCHAIN_ROOT)/Xcode.app/Contents/Developer

teamtalk-jar:
	make BUILDDIR=build-$@ CMAKE_EXTRA="-DBUILD_TEAMTALK_CLIENTS=OFF -DBUILD_TEAMTALKPRO_SERVERS=OFF -DBUILD_TEAMTALK_LIBRARY_JNI=OFF -DBUILD_TEAMTALKPRO_LIBRARY_JNI=OFF -DBUILD_TEAMTALK_LIBRARY_DOTNET=OFF -DBUILD_TEAMTALKPRO_LIBRARY_DOTNET=OFF -DBUILD_TEAMTALK_LIBRARY_DOTNET=OFF -DBUILD_TEAMTALKPRO_LIBRARY_DOTNET=OFF" buildlib

ubuntu18-depend:
	# TeamTalk 5 dependencies
	apt install qt5-default libqt5x11extras5-dev qtwebengine5-dev qtmultimedia5-dev doxygen openjdk-11-jdk qttools5-dev-tools

debian9-depend:
	# TeamTalk 5 dependencies
	apt install qt5-default qtbase5-dev libqt5x11extras5-dev qtmultimedia5-dev qtwebengine5-dev doxygen g++ openjdk-8-jdk qttools5-dev-tools
