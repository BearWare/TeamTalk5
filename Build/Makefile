JOBS?=4

help:

pre:
	test -d "$(TEAMTALK_ROOT)"

pre-toolchain:
	test -d "$(TOOLCHAIN_ROOT)"
	test -d "$(TTLIBS_ROOT)"

genlib: pre-toolchain
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && cmake $(CMAKE_EXTRA) -DCMAKE_BUILD_TYPE=$(CONFIGTYPE) $(TEAMTALK_ROOT)

buildlib: genlib
	make -C $(BUILDDIR) VERBOSE=1 all -j$(JOBS)

android-build:
	make CONFIGTYPE=release \
	CMAKE_EXTRA="-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) -DBUILD_TEAMTALK_CORE=ON -DBUILD_TEAMTALK_LIBRARIES=OFF -DBUILD_TEAMTALK_CLIENTS=OFF -DBUILD_TEAMTALKPRO_SERVERS=OFF -DBUILD_TT5SRV=OFF -DBUILD_TT5PROSRV=OFF" buildlib
	cd $(TEAMTALK_ROOT)/Library/TeamTalkJNI && make android-jni

android-arm32:
	make TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-android-armeabi-v7a.cmake BUILDDIR=build-armeabi-v7a android-build

android-arm64:
	make TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-android-arm64-v8a.cmake BUILDDIR=build-arm64-v8a android-build

android-x86:
	make TOOLCHAIN_FILE=$(TOOLCHAIN_ROOT)/toolchain-android-x86.cmake BUILDDIR=build-x86 android-build

