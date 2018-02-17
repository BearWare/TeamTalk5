cmake_minimum_required(VERSION 2.8)
project (TeamTalk5SDK)

option (BUILD_TEAMTALK_EXAMPLES "Build TeamTalk client and server examples" ON)
option (BUILD_TEAMTALK_LIBRARIES "Build TeamTalk libraries" ON)

if (BUILD_TEAMTALK_LIBRARIES)
  add_subdirectory (Library/TeamTalkJNI)
endif()

if (BUILD_TEAMTALK_EXAMPLES)
  add_subdirectory (Server/TeamTalkServer)
  add_subdirectory (Client/ttserverlog)
endif()

if (MSVC)

  if (BUILD_TEAMTALK_LIBRARIES)
    add_subdirectory (Library/TeamTalk.NET)
  endif()

  if (BUILD_TEAMTALK_EXAMPLES)
    add_subdirectory (Client/TeamTalkClassic)
    add_subdirectory (Client/TeamTalkApp.NET)
    add_subdirectory (Client/TeamTalkServer.NET)
    add_subdirectory (Client/ttserverlog.NET)
  endif()
  
endif()
