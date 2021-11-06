cmake_minimum_required(VERSION 3.1)
project (TeamTalk5SDK)

# To build TeamTalk libraries, clients and servers including all their
# dependencies follow the instructions below for each platform
#
# Windows 32-bit
# --------------
# Open "x86 Native Tools Command Prompt for VS 2019" and change to
# directory of TeamTalk5 checkout.
#
# Now configure an output directory, i.e. build-win32 for TeamTalk5
# repository:
# $ cmake -S TeamTalk5 -B teamtalk-win32 -DBUILD_TEAMTALK_CORE=ON -A Win32
#
# Afterwards build configured projects:
# $ cmake --build teamtalk-win32

option (BUILD_TEAMTALK_CORE "Build TeamTalk core library" OFF)
option (BUILD_TEAMTALK_LIBRARIES "Build TeamTalk libraries" ON)
option (BUILD_TEAMTALK_CLIENTS "Build TeamTalk client examples" ON)
option (BUILD_TEAMTALK_SERVERS "Build TeamTalk server examples" ON)

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

add_subdirectory (Documentation/TeamTalk)
add_subdirectory (Documentation/TeamTalkSDK)
