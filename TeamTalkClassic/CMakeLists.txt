cmake_minimum_required(VERSION 3.1)
project (TeamTalk5Classic)

include (ttclassic.cmake)

if (MSVC)

  option (BUILD_TEAMTALK_CLIENT_MFC_EXAMPLE "Build TeamTalk 5 Classic for TeamTalk 5 Standard Edition" ON)
  option (BUILD_TEAMTALKPRO_CLIENT_MFC_EXAMPLE "Build TeamTalk 5 Classic Pro for TeamTalk 5 Professional Edition" ON)
  
  set(CMAKE_MFC_FLAG 1)

  include_directories (./ ../TeamTalk_DLL tinyxml tolk/src)

  add_definitions (-DUNICODE -D_UNICODE -DENABLE_TOLK)

  link_directories (../TeamTalk_DLL tolk/bin/$(PlatformName))

  foreach (flag_var
      CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if (${flag_var} MATCHES "/MD")
      STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach(flag_var)

  set_source_files_properties (stdafx.cpp
    PROPERTIES
    COMPILE_FLAGS "/Ycstdafx.h")

  foreach (src_file ${TTCLASSIC_SOURCES})
    set_source_files_properties (
      ${src_file}
      PROPERTIES
      COMPILE_FLAGS "/Yustdafx.h")
  endforeach( src_file ${TTCLASSIC_SOURCES} )

  list(APPEND TTCLASSIC_SOURCES stdafx.cpp)

  if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    add_definitions (-D_WIN64)
    if (BUILD_TEAMTALK_CLIENT_MFC_EXAMPLE)
      add_executable (TeamTalk5Classic WIN32 ${TTCLASSIC_SOURCES} ${TTCLASSIC_HEADERS}
        ${TINYXML_SOURCES} ${TINYXML_HEADERS} res/TeamTalk_amd64.manifest)
    endif()
    if (BUILD_TEAMTALKPRO_CLIENT_MFC_EXAMPLE)
      add_executable (TeamTalk5ClassicPro WIN32 ${TTCLASSIC_SOURCES} ${TTCLASSIC_HEADERS}
        ${TINYXML_SOURCES}  ${TINYXML_HEADERS} res/TeamTalk_amd64.manifest)
    endif()
  else()
    if (BUILD_TEAMTALK_CLIENT_MFC_EXAMPLE)
      add_executable (TeamTalk5Classic WIN32 ${TTCLASSIC_SOURCES} ${TTCLASSIC_HEADERS}
        ${TINYXML_SOURCES} ${TINYXML_HEADERS} res/TeamTalk.manifest)
    endif()
    if (BUILD_TEAMTALKPRO_CLIENT_MFC_EXAMPLE)
      add_executable (TeamTalk5ClassicPro WIN32 ${TTCLASSIC_SOURCES} ${TTCLASSIC_HEADERS}
        ${TINYXML_SOURCES} ${TINYXML_HEADERS} res/TeamTalk.manifest)
    endif()
  endif()

  target_link_libraries (TeamTalk5Classic TeamTalk5 Tolk)
  target_link_libraries (TeamTalk5ClassicPro TeamTalk5Pro Tolk)
  target_compile_options (TeamTalk5ClassicPro PUBLIC -DENABLE_ENCRYPTION)
endif()
