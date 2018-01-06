cmake_minimum_required(VERSION 3.1)
project (ttserverlog)

include_directories (../../TeamTalk_DLL)
link_directories (../../TeamTalk_DLL)

option (BUILD_TEAMTALKCLIENT_LOG_EXAMPLE "Build ttserverlog for TeamTalk 5 Standard Edition" ON)
option (BUILD_TEAMTALKCLIENTPRO_LOG_EXAMPLE "Build ttproserverlog for TeamTalk 5 Professional Edition" ON)

if (MSVC)
  add_definitions (-DUNICODE -D_UNICODE)
endif()

if (BUILD_TEAMTALKCLIENT_LOG_EXAMPLE)
  add_executable (ttserverlog Main.cpp)
  target_link_libraries (ttserverlog TeamTalk5)
endif()

if (BUILD_TEAMTALKCLIENTPRO_LOG_EXAMPLE)
  add_executable (ttproserverlog Main.cpp)
  target_link_libraries (ttproserverlog TeamTalk5Pro)
endif()
