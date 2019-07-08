JOBS?=4

help:

teamtalk-env:
	@echo "Testing that TEAMTALK_ROOT is valid."
	@echo "Source env.sh in repository root to set up environment"
	test -d "$(TEAMTALK_ROOT)"

toolchain-env:
	@echo "Testing that TOOLCHAIN_ROOT and TTLIBS_ROOT are valid."
	@echo "Source env.sh in repository root to set up environment"
	test -d "$(TOOLCHAIN_ROOT)"
	test -d "$(TTLIBS_ROOT)"

genlib: teamtalk-env
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && cmake $(CMAKE_EXTRA) -DCMAKE_BUILD_TYPE=$(CONFIGTYPE) $(TEAMTALK_ROOT)

buildlib: genlib
	make -C $(BUILDDIR) VERBOSE=1 all -j$(JOBS)

android-build: toolchain-env
	make CONFIGTYPE=release \
	CMAKE_EXTRA="-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) -DBUILD_TEAMTALK_CORE=ON -DBUILD_TEAMTALK_LIBRARIES=OFF -DBUILD_TEAMTALK_CLIENTS=OFF -DBUILD_TEAMTALKPRO_SERVERS=OFF -DBUILD_TT5SRV=OFF -DBUILD_TT5PROSRV=OFF" buildlib
	cd $(TEAMTALK_ROOT)/Library/TeamTalkJNI && make android-jni

android-armeabi-v7a:
	make TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-android-armeabi-v7a.cmake BUILDDIR=build-armeabi-v7a android-build

android-arm64-v8a:
	make TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-android-arm64-v8a.cmake BUILDDIR=build-arm64-v8a android-build

android-x86:
	make TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-android-x86.cmake BUILDDIR=build-x86 android-build

deb32dbg:
	make BUILDDIR=build CONFIGTYPE=debug CMAKE_EXTRA="-DBUILD_TEAMTALK_CORE=ON" buildlib

deb32:
	make BUILDDIR=build CONFIGTYPE=release CMAKE_EXTRA="-DBUILD_TEAMTALK_CORE=ON" buildlib

deb64dbg: deb32dbg

deb64: deb32

