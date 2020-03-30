# Relative paths are not supported pre-2.8.1, target_link_libraries seems to require 3.0.2
cmake_minimum_required(VERSION 3.1)
project (TeamTalk5JNI)

if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
  # CMake cannot find JNI package in Android NDK
  set(JAVA_AWT_LIBRARY "")
  set(JAVA_JVM_LIBRARY "")
  set(JAVA_INCLUDE_PATH2 "")
  set(JAVA_AWT_INCLUDE_PATH "")
endif()

find_package(JNI)

if (NOT JNI_FOUND)
  message("Specify JAVA_HOME environment variable to help cmake")
endif()

include_directories (../../Library/TeamTalk_DLL ${JNI_INCLUDE_DIRS})

if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
  link_directories (../../Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
else()
  link_directories (../../Library/TeamTalk_DLL)
endif()

option (BUILD_TEAMTALK_LIBRARY_JNI "Build TeamTalk 5 JNI DLL for TeamTalk 5 Standard Edition" ON)
option (BUILD_TEAMTALKPRO_LIBRARY_JNI "Build TeamTalk 5 Pro JNI DLL for TeamTalk 5 Professional Edition" ON)

set (JAVA_SOURCES
  src/dk/bearware/AbusePrevention.java
  src/dk/bearware/AudioBlock.java
  src/dk/bearware/AudioCodec.java
  src/dk/bearware/AudioConfig.java
  src/dk/bearware/AudioConfigConstants.java
  src/dk/bearware/AudioFileFormat.java
  src/dk/bearware/AudioFormat.java
  src/dk/bearware/AudioInputProgress.java
  src/dk/bearware/AudioPreprocessor.java
  src/dk/bearware/AudioPreprocessorType.java
  src/dk/bearware/BannedUser.java
  src/dk/bearware/BanType.java
  src/dk/bearware/BitmapFormat.java
  src/dk/bearware/Channel.java
  src/dk/bearware/ChannelType.java
  src/dk/bearware/ClientError.java
  src/dk/bearware/ClientErrorMsg.java
  src/dk/bearware/ClientEvent.java
  src/dk/bearware/ClientFlag.java
  src/dk/bearware/ClientKeepAlive.java
  src/dk/bearware/ClientStatistics.java
  src/dk/bearware/Codec.java
  src/dk/bearware/Constants.java
  src/dk/bearware/DesktopInput.java
  src/dk/bearware/DesktopProtocol.java
  src/dk/bearware/DesktopWindow.java
  src/dk/bearware/events/ClientListener.java
  src/dk/bearware/events/CommandListener.java
  src/dk/bearware/events/ConnectionListener.java
  src/dk/bearware/events/TeamTalkEventHandler.java
  src/dk/bearware/events/UserListener.java
  src/dk/bearware/FileTransfer.java
  src/dk/bearware/FileTransferStatus.java
  src/dk/bearware/FourCC.java
  src/dk/bearware/IntPtr.java
  src/dk/bearware/MediaFileInfo.java
  src/dk/bearware/MediaFilePlayback.java
  src/dk/bearware/MediaFileStatus.java
  src/dk/bearware/OpusCodec.java
  src/dk/bearware/OpusConstants.java
  src/dk/bearware/PlatformHelper.java
  src/dk/bearware/RemoteFile.java
  src/dk/bearware/ServerCallback.java
  src/dk/bearware/ServerLogger.java
  src/dk/bearware/ServerProperties.java
  src/dk/bearware/ServerStatistics.java
  src/dk/bearware/SoundDevice.java
  src/dk/bearware/SoundDeviceConstants.java
  src/dk/bearware/SoundLevel.java
  src/dk/bearware/SoundSystem.java
  src/dk/bearware/SpeexCodec.java
  src/dk/bearware/SpeexConstants.java
  src/dk/bearware/SpeexDSP.java
  src/dk/bearware/SpeexDSPConstants.java
  src/dk/bearware/SpeexVBRCodec.java
  src/dk/bearware/StreamType.java
  src/dk/bearware/Subscription.java
  src/dk/bearware/TeamTalkBase.java
  src/dk/bearware/TextMessage.java
  src/dk/bearware/TextMsgType.java
  src/dk/bearware/TTAudioPreprocessor.java
  src/dk/bearware/TTMessage.java
  src/dk/bearware/TTType.java
  src/dk/bearware/User.java
  src/dk/bearware/UserAccount.java
  src/dk/bearware/UserRight.java
  src/dk/bearware/UserState.java
  src/dk/bearware/UserStatistics.java
  src/dk/bearware/UserType.java
  src/dk/bearware/VideoCaptureDevice.java
  src/dk/bearware/VideoCodec.java
  src/dk/bearware/VideoFormat.java
  src/dk/bearware/VideoFrame.java
  src/dk/bearware/WebMVP8Codec.java
  src/dk/bearware/WindowsHelper.java)

if (MSVC)
  add_definitions (-DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS)
endif()

function(set_output_dir target dir)
  set_target_properties (${target} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${dir}
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${dir}
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${dir}
    RUNTIME_OUTPUT_DIRECTORY ${dir}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${dir}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${dir}
    ARCHIVE_OUTPUT_DIRECTORY ${dir}
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${dir}
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${dir})
endfunction()

if (BUILD_TEAMTALKPRO_LIBRARY_JNI OR BUILD_TEAMTALK_LIBRARY_JNI)
  set (CMAKE_CXX_STANDARD 11)
endif()

if (BUILD_TEAMTALK_LIBRARY_JNI)
  
  add_library (TeamTalk5-jni SHARED
    jni/teamtalk-jni.cpp
    jni/ttconvert-jni.cpp
    jni/ttconvert-jni.h)

  if (BUILD_TEAMTALK_CORE)
    if (BUILD_TEAMTALK5LIB)
      target_link_libraries (TeamTalk5-jni TeamTalk5Lib ${JNI_LIBRARIES})
    elseif (BUILD_TEAMTALK5DLL)
      target_link_libraries (TeamTalk5-jni TeamTalk5DLL ${JNI_LIBRARIES})
    else()
      target_link_libraries (TeamTalk5-jni TeamTalk5 ${JNI_LIBRARIES})
    endif()
  else()
    target_link_libraries (TeamTalk5-jni TeamTalk5 ${JNI_LIBRARIES})
  endif()

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5-jni "${CMAKE_CURRENT_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}")
  else()
    set_output_dir(TeamTalk5-jni "${CMAKE_CURRENT_SOURCE_DIR}/libs")
  endif()
  
endif()

if (BUILD_TEAMTALKPRO_LIBRARY_JNI)
  add_library (TeamTalk5Pro-jni SHARED
    jni/teamtalk-jni.cpp
    jni/teamtalksrv-jni.cpp
    jni/ttconvert-jni.cpp
    jni/ttconvert-jni.h)

  if (BUILD_TEAMTALK_CORE)
    if (BUILD_TEAMTALK5PROLIB)
      target_link_libraries (TeamTalk5Pro-jni TeamTalk5ProLib ${JNI_LIBRARIES})
    elseif (BUILD_TEAMTALK5PRODLL)
      target_link_libraries (TeamTalk5Pro-jni TeamTalk5ProDLL ${JNI_LIBRARIES})
    else()
      target_link_libraries (TeamTalk5Pro-jni TeamTalk5Pro ${JNI_LIBRARIES})
    endif()
  else()
    target_link_libraries (TeamTalk5Pro-jni TeamTalk5Pro ${JNI_LIBRARIES})
  endif()

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5Pro-jni "${CMAKE_CURRENT_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}")
  else()
    set_output_dir(TeamTalk5Pro-jni "${CMAKE_CURRENT_SOURCE_DIR}/libs")
  endif()

endif()

find_package (Java)

if (Java_FOUND)
  include (UseJava)
endif()

option (BUILD_TEAMTALK_LIBRARY_JAR "Build TeamTalk 5 jar file with Java classes for TeamTalk 5 Standard Edition" ON)
option (BUILD_TEAMTALKPRO_LIBRARY_JAR "Build TeamTalk 5 Pro jar file with Java classes for TeamTalk 5 Professional Edition" ON)

# Android 5.0 (API level 21) has Java 1.7 so it's minimum.
set(CMAKE_JAVA_COMPILE_FLAGS "-source" "1.7" "-target" "1.7")

if (BUILD_TEAMTALK_LIBRARY_JAR)
  add_jar (TeamTalk5JAR
    SOURCES ${JAVA_SOURCES} src/dk/bearware/TeamTalk5.java
    OUTPUT_NAME TeamTalk5
    OUTPUT_DIR libs)
endif()

if (BUILD_TEAMTALKPRO_LIBRARY_JAR)
  add_jar (TeamTalk5ProJAR
    SOURCES ${JAVA_SOURCES} src/dk/bearware/TeamTalk5Pro.java src/dk/bearware/TeamTalk5Srv.java src/dk/bearware/TeamTalkSrv.java
    OUTPUT_NAME TeamTalk5Pro
    OUTPUT_DIR libs)
endif()

