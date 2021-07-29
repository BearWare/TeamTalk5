cmake_minimum_required(VERSION 3.1)
project (TeamTalk5SDK)

option (BUILD_TEAMTALK_CORE "Build TeamTalk core library" OFF)
option (BUILD_TEAMTALK_LIBRARIES "Build TeamTalk libraries" ON)
option (BUILD_TEAMTALK_CLIENTS "Build TeamTalk client examples" ON)
option (BUILD_TEAMTALKPRO_SERVERS "Build TeamTalk server examples" ON)

set (TEAMTALK_ROOT ${CMAKE_CURRENT_LIST_DIR})

if (BUILD_TEAMTALK_CORE)
  add_subdirectory (Library/TeamTalkLib)
endif()

if (BUILD_TEAMTALK_LIBRARIES)
  add_subdirectory (Library/TeamTalkJNI)
endif()

if (BUILD_TEAMTALK_CLIENTS)
  add_subdirectory (Client/qtTeamTalk)
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
  
  if (BUILD_TEAMTALK5DLL)
    if (BUILD_TEAMTALK_LIBRARIES AND BUILD_TEAMTALK_LIBRARY_JNI)
      add_dependencies (TeamTalk5-jni TeamTalk5DLL)
    endif()
    if (BUILD_TEAMTALK_CLIENTS AND BUILD_TEAMTALK_CLIENT_LOG_EXAMPLE)
      add_dependencies (ttserverlog TeamTalk5DLL)
    endif()
  endif()
  
  if (BUILD_TEAMTALK5PRODLL)
    if (BUILD_TEAMTALK_LIBRARIES AND BUILD_TEAMTALKPRO_LIBRARY_JNI)
      add_dependencies (TeamTalk5Pro-jni TeamTalk5ProDLL)
    endif()
    
    if (BUILD_TEAMTALKPRO_SERVERS AND BUILD_TEAMTALKPRO_SERVER_EXAMPLE)
      add_dependencies (TeamTalk5ProServer TeamTalk5ProDLL)
    endif()
    
    if (BUILD_TEAMTALK_CLIENTS AND BUILD_TEAMTALKPRO_CLIENT_LOG_EXAMPLE)
      add_dependencies (ttproserverlog TeamTalk5ProDLL)
    endif()
  endif()
    
endif()
