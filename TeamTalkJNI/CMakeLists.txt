cmake_minimum_required(VERSION 3.1)
project (TeamTalk5JNI)

find_package(JNI)

if (NOT JNI_FOUND)
  message("Specify JAVA_HOME environment variable to help cmake")
endif()

include_directories (../TeamTalk_DLL ${JNI_INCLUDE_DIRS})
link_directories (../TeamTalk_DLL)

option (TEAMTALKSTD "Build TeamTalk 5 JNI DLL for TeamTalk 5 Standard Edition" ON)
option (TEAMTALKPRO "Build TeamTalk 5 Pro JNI DLL for TeamTalk 5 Professional Edition" ON)

if (MSVC)
  add_definitions (-DUNICODE -D_UNICODE)

  if (TEAMTALKSTD)
    add_library (TeamTalk5-jni SHARED
      jni/teamtalk-jni.cpp
      jni/ttconvert-jni.cpp
      jni/ttconvert-jni.h)
    target_link_libraries (TeamTalk5-jni TeamTalk5 ${JNI_LIBRARIES})
  endif()

  if (TEAMTALKPRO)
    add_library (TeamTalk5Pro-jni SHARED
      jni/teamtalk-jni.cpp
      jni/teamtalksrv-jni.cpp
      jni/ttconvert-jni.cpp
      jni/ttconvert-jni.h)
    target_link_libraries (TeamTalk5Pro-jni TeamTalk5Pro ${JNI_LIBRARIES})
  endif()
endif()
