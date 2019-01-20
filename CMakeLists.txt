cmake_minimum_required(VERSION 2.8)
project (TeamTalk5SDK)

option (BUILD_TEAMTALK_CORE "Build TeamTalk core library" OFF)
option (BUILD_TEAMTALK_LIBRARIES "Build TeamTalk libraries" ON)
option (BUILD_TEAMTALK_CLIENTS "Build TeamTalk client examples" ON)
option (BUILD_TEAMTALKPRO_SERVERS "Build TeamTalk server examples" ON)

if (BUILD_TEAMTALK_CORE)
  add_subdirectory (Library/TeamTalkLib)
endif()

if (BUILD_TEAMTALK_LIBRARIES)
  add_subdirectory (Library/TeamTalkJNI)
endif()

if (BUILD_TEAMTALK_CLIENTS)
  add_subdirectory (Client/ttserverlog)
endif()

if (BUILD_TEAMTALKPRO_SERVERS)
  add_subdirectory (Server/TeamTalkServer)
endif()

if (MSVC)

  if (BUILD_TEAMTALK_LIBRARIES)
    add_subdirectory (Library/TeamTalk.NET)
  endif()

  if (BUILD_TEAMTALK_CLIENTS)
    add_subdirectory (Client/TeamTalkClassic)
    add_subdirectory (Client/TeamTalkApp.NET)
    add_subdirectory (Client/ttserverlog.NET)
  endif()

  if (BUILD_TEAMTALKPRO_SERVERS)
    add_subdirectory (Server/TeamTalkServer.NET)
  endif()
endif()

if (BUILD_TEAMTALK_CORE)
    add_dependencies (TeamTalk5-jni TeamTalk5DLL)
    add_dependencies (ttserverlog TeamTalk5DLL)
    
    add_dependencies (TeamTalk5Pro-jni TeamTalk5ProDLL)
    add_dependencies (TeamTalk5ProServer TeamTalk5ProDLL)
    add_dependencies (ttproserverlog TeamTalk5ProDLL)
    
endif()
