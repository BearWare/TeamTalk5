JOBS?=4
TOOLCHAIN_ROOT?=$(PWD)/../Library/TeamTalkLib/toolchain
TEAMTALK_ROOT?=$(PWD)/..

help:

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

android-build: toolchain-env
	@echo "Testing if NDK environment variable is set"
	test -n "$NDK"
	make CONFIGTYPE=release \
	CMAKE_EXTRA="-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) -DBUILD_TEAMTALK_CORE=ON -DBUILD_TEAMTALK_LIBRARIES=OFF -DBUILD_TEAMTALK_CLIENTS=OFF -DBUILD_TEAMTALKPRO_SERVERS=OFF -DBUILD_TT5SRV=OFF -DBUILD_TT5PROSRV=OFF" buildlib
	cd $(TEAMTALK_ROOT)/Library/TeamTalkJNI/jni && $(NDK)/ndk-build

android-armeabi-v7a:
	. $(TOOLCHAIN_ROOT)/toolchain.sh android armeabi-v7a && make TEAMTALK_ROOT=$(TEAMTALK_ROOT) TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-android-armeabi-v7a.cmake BUILDDIR=build-armeabi-v7a android-build

android-arm64-v8a:
	. $(TOOLCHAIN_ROOT)/toolchain.sh android arm64-v8a && make TEAMTALK_ROOT=$(TEAMTALK_ROOT) TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-android-arm64-v8a.cmake BUILDDIR=build-arm64-v8a android-build

android-x86:
	. $(TOOLCHAIN_ROOT)/toolchain.sh android x86 && make TEAMTALK_ROOT=$(TEAMTALK_ROOT) TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-android-x86.cmake BUILDDIR=build-x86 android-build

deb32dbg: toolchain-env
	make BUILDDIR=build CONFIGTYPE=debug CMAKE_EXTRA="-DBUILD_TEAMTALK_CORE=ON" buildlib

deb32: toolchain-env
	make BUILDDIR=build CONFIGTYPE=release CMAKE_EXTRA="-DBUILD_TEAMTALK_CORE=ON" buildlib

deb64dbg: deb32dbg

deb64: deb32

ubuntu64:
	make JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64 deb64

rasp:
	make JOBS=2 BUILDDIR=build CONFIGTYPE=release CMAKE_EXTRA="-DBUILD_TEAMTALK_CORE=ON" JAVA_HOME=/usr/lib/jvm/java-8-openjdk-armhf buildlib

raspdbg:
	make JOBS=2 BUILDDIR=build CONFIGTYPE=debug CMAKE_EXTRA="-DBUILD_TEAMTALK_CORE=ON" JAVA_HOME=/usr/lib/jvm/java-8-openjdk-armhf buildlib

ios-build:
	make CONFIGTYPE=release \
	CMAKE_EXTRA="-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) -DBUILD_TEAMTALK_CORE=ON -DBUILD_TEAMTALK_LIBRARIES=OFF -DBUILD_TEAMTALK_CLIENTS=OFF -DBUILD_TEAMTALKPRO_SERVERS=OFF -DBUILD_TT5SRV=OFF -DBUILD_TT5PROSRV=OFF -DZLIB_STATIC=OFF" buildlib

ios-armv7:
	make TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-ios-armv7.cmake BUILDDIR=build-armv7 ios-build

ios-arm64:
	make TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-ios-arm64.cmake BUILDDIR=build-arm64 ios-build

ios-i386:
	make TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-ios-i386.cmake BUILDDIR=build-i386 ios-build

ios-x64:
	make TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-ios-x86_64.cmake BUILDDIR=build-x86_64 ios-build
