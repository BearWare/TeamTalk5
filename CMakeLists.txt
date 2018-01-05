cmake_minimum_required(VERSION 3.1)
project (TeamTalk5SDK)

add_subdirectory (TeamTalkJNI)
add_subdirectory (TeamTalkClassic)
add_subdirectory (Examples/TeamTalkServer)
add_subdirectory (Examples/ttserverlog)

if (MSVC)
  add_subdirectory (TeamTalk.NET)
  add_subdirectory (Examples/TeamTalkApp.NET)
  add_subdirectory (Examples/TeamTalkServer.NET)
  add_subdirectory (Examples/ttserverlog.NET)
endif()
