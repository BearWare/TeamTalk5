cmake_minimum_required(VERSION 2.8)
project (TeamTalk5SDK)

option (BUILD_TEAMTALK_EXAMPLES "Build TeamTalk client and server examples" ON)
option (BUILD_TEAMTALK_LIBRARIES "Build TeamTalk libraries" ON)

if (BUILD_TEAMTALK_LIBRARIES)
  add_subdirectory (TeamTalkJNI)
endif()

if (BUILD_TEAMTALK_EXAMPLES)
  add_subdirectory (Examples/TeamTalkServer)
  add_subdirectory (Examples/ttserverlog)
endif()

if (MSVC)

  if (BUILD_TEAMTALK_LIBRARIES)
    add_subdirectory (TeamTalk.NET)
  endif()

  if (BUILD_TEAMTALK_EXAMPLES)
    add_subdirectory (TeamTalkClassic)
    add_subdirectory (Examples/TeamTalkApp.NET)
    add_subdirectory (Examples/TeamTalkServer.NET)
    add_subdirectory (Examples/ttserverlog.NET)
  endif()
  
endif()
