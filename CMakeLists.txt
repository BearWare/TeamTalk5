cmake_minimum_required(VERSION 2.8)
project (TeamTalk5SDK)

option (BUILD_TEAMTALK_LIBRARIES "Build TeamTalk libraries" ON)
option (BUILD_TEAMTALK_CLIENTS "Build TeamTalk client examples" ON)
option (BUILD_TEAMTALK_SERVERS "Build TeamTalk server examples" ON)

if (BUILD_TEAMTALK_LIBRARIES)
  add_subdirectory (Library/TeamTalkJNI)
endif()

if (BUILD_TEAMTALK_CLIENTS)
  add_subdirectory (Client/ttserverlog)
endif()

if (BUILD_TEAMTALK_SERVERS)
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

  if (BUILD_TEAMTALK_SERVERS)
    add_subdirectory (Server/TeamTalkServer.NET)
  endif()
endif()
