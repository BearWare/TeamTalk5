cmake_minimum_required(VERSION 2.8)
project (TeamTalk5SDK)

if (MSVC)
  # Directories already have Makefile so don't overwrite
  add_subdirectory (TeamTalkJNI)
  add_subdirectory (TeamTalkClassic)
  add_subdirectory (Examples/TeamTalkServer)
  add_subdirectory (Examples/ttserverlog)

  add_subdirectory (TeamTalk.NET)
  add_subdirectory (Examples/TeamTalkApp.NET)
  add_subdirectory (Examples/TeamTalkServer.NET)
  add_subdirectory (Examples/ttserverlog.NET)
endif()
