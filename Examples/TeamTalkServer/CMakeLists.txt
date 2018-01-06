cmake_minimum_required(VERSION 3.1)
project (TeamTalk5ProServer)

include_directories (../../TeamTalk_DLL)
link_directories (../../TeamTalk_DLL)

option (BUILD_TEAMTALKSERVER_EXAMPLE "Build TeamTalk Server Example for TeamTalk 5 Professional Edition" ON)

if (MSVC)
  add_definitions (-DUNICODE -D_UNICODE)
endif()

if (BUILD_TEAMTALKSERVER_EXAMPLE)
  add_executable (TeamTalk5ProServer Main.cpp)
  target_link_libraries (TeamTalk5ProServer TeamTalk5Pro)
endif()
