# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#LOCAL_PATH := $(call my-dir)

LOCAL_PATH := $(TTLIBS_ROOT)

#ACE static library
include $(CLEAR_VARS)

LOCAL_MODULE := ACE
LOCAL_SRC_FILES := ACE/lib/libACE.a
include $(PREBUILT_STATIC_LIBRARY)

#Speex static library
include $(CLEAR_VARS)

LOCAL_MODULE := Speex
LOCAL_SRC_FILES := speex/lib/libspeex.a
include $(PREBUILT_STATIC_LIBRARY)

#Speex DSB static library
include $(CLEAR_VARS)

LOCAL_MODULE := SpeexDSP
LOCAL_SRC_FILES := speex/lib/libspeexdsp.a
include $(PREBUILT_STATIC_LIBRARY)

#ogg static library
include $(CLEAR_VARS)

LOCAL_MODULE := Ogg
LOCAL_SRC_FILES := ogg/lib/libogg.a
include $(PREBUILT_STATIC_LIBRARY)

#OPUS static library
include $(CLEAR_VARS)

LOCAL_MODULE := Opus
LOCAL_SRC_FILES := opus/lib/libopus.a
include $(PREBUILT_STATIC_LIBRARY)

#libVPX static library
include $(CLEAR_VARS)

LOCAL_MODULE := VPX
LOCAL_SRC_FILES := libvpx/lib/libvpx.a
include $(PREBUILT_STATIC_LIBRARY)

#crypto static library
include $(CLEAR_VARS)

LOCAL_MODULE := Crypto
LOCAL_SRC_FILES := openssl/lib/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)

#ssl static library
include $(CLEAR_VARS)

LOCAL_MODULE := Ssl
LOCAL_SRC_FILES := openssl/lib/libssl.a
include $(PREBUILT_STATIC_LIBRARY)

#ACE SSL static library
include $(CLEAR_VARS)

LOCAL_MODULE := ACESSL
LOCAL_SRC_FILES := ACE/lib/libACE_SSL.a
include $(PREBUILT_STATIC_LIBRARY)

#ACE Inet static library
include $(CLEAR_VARS)

LOCAL_MODULE := ACEInet
LOCAL_SRC_FILES := ACE/lib/libACE_INet.a
include $(PREBUILT_STATIC_LIBRARY)

#ACE Inet SSL static library
include $(CLEAR_VARS)

LOCAL_MODULE := ACEInetSSL
LOCAL_SRC_FILES := ACE/lib/libACE_INet_SSL.a
include $(PREBUILT_STATIC_LIBRARY)


#TeamTalk static library
LOCAL_PATH := $(TEAMTALK_ROOT)

include $(CLEAR_VARS)

LOCAL_MODULE := TeamTalk5
LOCAL_SRC_FILES := Library/TeamTalk_DLL/$(ARCH)/libTeamTalk5.a
include $(PREBUILT_STATIC_LIBRARY)

#TeamTalk Pro static library
LOCAL_PATH := $(TEAMTALK_ROOT)

include $(CLEAR_VARS)

LOCAL_MODULE := TeamTalk5Pro
LOCAL_SRC_FILES := Library/TeamTalk_DLL/$(ARCH)/libTeamTalk5Pro.a
include $(PREBUILT_STATIC_LIBRARY)

#TeamTalk JNI shared library

LOCAL_PATH := ./

include $(CLEAR_VARS)
LOCAL_CPPFLAGS := -I$(TEAMTALK_ROOT)/Library/TeamTalk_DLL
LOCAL_MODULE    := TeamTalk5-jni
LOCAL_SRC_FILES := teamtalk-jni.cpp ttconvert-jni.cpp

LOCAL_STATIC_LIBRARIES := TeamTalk5 ACEInet ACE Speex SpeexDSP Ogg Opus VPX cpufeatures

LOCAL_LDLIBS += -lz -lOpenSLES -L$(SYSROOT)/usr/lib -llog

include $(BUILD_SHARED_LIBRARY)

#TeamTalk Pro JNI shared library

include $(CLEAR_VARS)
LOCAL_CPPFLAGS := -I$(TEAMTALK_ROOT)/Library/TeamTalk_DLL
LOCAL_MODULE    := TeamTalk5Pro-jni
LOCAL_SRC_FILES := teamtalk-jni.cpp ttconvert-jni.cpp teamtalksrv-jni.cpp

LOCAL_STATIC_LIBRARIES := TeamTalk5Pro Ssl Crypto ACESSL ACEInet ACEInetSSL ACE Speex SpeexDSP Ogg Opus VPX  cpufeatures

LOCAL_LDLIBS += -lz -lOpenSLES -L$(SYSROOT)/usr/lib -llog

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/cpufeatures)

