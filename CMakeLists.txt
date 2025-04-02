# Relative paths are not supported pre-2.8.1, target_link_libraries seems to require 3.0.2
cmake_minimum_required(VERSION 3.10)

project (TeamTalk5SDK)

# To build TeamTalk libraries, clients and servers including all their
# dependencies follow the instructions below for each platform
#
# Windows 32-bit
# --------------
# Open "x86 Native Tools Command Prompt for VS 2019" and change to
# directory of TeamTalk5 checkout, e.g. C:\TeamTalk5
#
# Now configure an output directory, here teamtalk-win32, for
# TeamTalk5 repository in C:\TeamTalk5:
#
# $ cmake -S C:/TeamTalk5 -B teamtalk-win32 -A Win32
#
# Afterwards build configured projects:
#
# $ cmake --build teamtalk-win32
#
# A Visual Studio 2019 solution file will also be available in output
# directory teamtalk-win32.
#
# Windows 64-bit
# --------------
# Open "x64 Native Tools Command Prompt for VS 2022" and change to
# directory of TeamTalk5 checkout, e.g. C:\TeamTalk5
#
# Now configure an output directory, here teamtalk-win64, for
# TeamTalk5 repository in C:\TeamTalk5:
#
# $ cmake -S C:/TeamTalk5 -B teamtalk-win64 -A x64
#
# Afterwards build configured projects:
#
# $ cmake --build teamtalk-win64
#
# A Visual Studio 2022 solution file will also be available in output
# directory teamtalk-win64.

option (BUILD_TEAMTALK_CORE "Build TeamTalk core library" ON)
option (BUILD_TEAMTALK_LIBRARIES "Build TeamTalk libraries" ON)
option (BUILD_TEAMTALK_CLIENTS "Build TeamTalk client examples" ON)
option (BUILD_TEAMTALK_SERVERS "Build TeamTalk server examples" ON)
option (BUILD_TEAMTALK_DOCUMENTATION "Build TeamTalk documentation" ON)

set (TEAMTALK_ROOT ${CMAKE_CURRENT_LIST_DIR})

if (BUILD_TEAMTALK_CORE)
  add_subdirectory (Library/TeamTalkLib)
endif()

if (MSVC)
  include(CheckLanguage)
  check_language(CSharp)
endif()

if (BUILD_TEAMTALK_LIBRARIES)
  add_subdirectory (Library/TeamTalkJNI)

  if (CMAKE_CSharp_COMPILER)
    add_subdirectory (Library/TeamTalk.NET)
  else()
    message(WARNING "No C# compiler detected. Skipping .NET libraries")
  endif()
endif()

if (BUILD_TEAMTALK_CLIENTS)
  add_subdirectory (Client)
endif()

if (BUILD_TEAMTALK_SERVERS)
  add_subdirectory (Server/TeamTalkServer)
  if (CMAKE_CSharp_COMPILER)
    add_subdirectory (Server/TeamTalkServer.NET)
  else()
    message(WARNING "No C# compiler detected. Skipping .NET Servers")
  endif()
endif()

if (BUILD_TEAMTALK_DOCUMENTATION)
  add_subdirectory (Documentation/TeamTalk)
  add_subdirectory (Documentation/TeamTalkSDK)
endif()
