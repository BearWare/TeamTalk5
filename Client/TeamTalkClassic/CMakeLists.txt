cmake_minimum_required(VERSION 3.1)
project (TeamTalk5Classic)

function(set_output_dir target dir)
  set_target_properties (${target} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${dir}
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${dir}
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${dir}
    RUNTIME_OUTPUT_DIRECTORY ${dir}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${dir}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${dir}
    ARCHIVE_OUTPUT_DIRECTORY ${dir}
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${dir}
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${dir})
endfunction()

include (ttclassic.cmake)

if (MSVC)

  option (BUILD_TEAMTALK_CLIENT_MFC_EXAMPLE "Build TeamTalk 5 Classic for TeamTalk 5 Standard Edition" ON)
  option (BUILD_TEAMTALK_PROCLIENT_MFC_EXAMPLE "Build TeamTalk 5 Classic Pro for TeamTalk 5 Professional Edition" ON)

  set(CMAKE_MFC_FLAG 1)

  link_directories (../../Library/TeamTalk_DLL)

  foreach (flag_var
      CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if (${flag_var} MATCHES "/MD")
      STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach(flag_var)

  set_source_files_properties (stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h")

  foreach (src_file ${TTCLASSIC_SOURCES})
    set_source_files_properties (${src_file} PROPERTIES COMPILE_FLAGS "/Yustdafx.h")
  endforeach( src_file ${TTCLASSIC_SOURCES} )

  list(APPEND TTCLASSIC_SOURCES stdafx.cpp)

  if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    add_definitions (-D_WIN64)
    if (BUILD_TEAMTALK_CLIENT_MFC_EXAMPLE)
      add_executable (TeamTalk5Classic WIN32 ${TTCLASSIC_SOURCES} ${TTCLASSIC_HEADERS}
        ${TINYXML_SOURCES} ${TINYXML_HEADERS} res/TeamTalk_amd64.manifest)
    endif()
    if (BUILD_TEAMTALK_PROCLIENT_MFC_EXAMPLE)
      add_executable (TeamTalk5ClassicPro WIN32 ${TTCLASSIC_SOURCES} ${TTCLASSIC_HEADERS}
        ${TINYXML_SOURCES}  ${TINYXML_HEADERS} res/TeamTalk_amd64.manifest)
    endif()
  else()
    if (BUILD_TEAMTALK_CLIENT_MFC_EXAMPLE)
      add_executable (TeamTalk5Classic WIN32 ${TTCLASSIC_SOURCES} ${TTCLASSIC_HEADERS}
        ${TINYXML_SOURCES} ${TINYXML_HEADERS} res/TeamTalk.manifest)
    endif()
    if (BUILD_TEAMTALK_PROCLIENT_MFC_EXAMPLE)
      add_executable (TeamTalk5ClassicPro WIN32 ${TTCLASSIC_SOURCES} ${TTCLASSIC_HEADERS}
        ${TINYXML_SOURCES} ${TINYXML_HEADERS} res/TeamTalk.manifest)
    endif()
  endif()

  set (TTCLASSIC_COMPILE_FLAGS -DUNICODE -D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS -D_UNICODE -wd4800)

  if (BUILD_TEAMTALK_CLIENT_MFC_EXAMPLE)
    set_property(TARGET TeamTalk5Classic PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Library/TeamTalk_DLL")

    if (BUILD_TEAMTALK_CLIENT_TOLK)
      target_compile_options (TeamTalk5Classic PUBLIC -DENABLE_TOLK ${TTCLASSIC_COMPILE_FLAGS})
      if (BUILD_TEAMTALK_DLL)
        target_include_directories (TeamTalk5Classic PUBLIC ./ tinyxml)
        target_link_libraries (TeamTalk5Classic TeamTalk5DLL Tolk)
      else()
        target_include_directories (TeamTalk5Classic PUBLIC ./ ../../Library/TeamTalk_DLL tinyxml)
        target_link_libraries (TeamTalk5Classic TeamTalk5 Tolk)
      endif()
    else()
      target_compile_options (TeamTalk5Classic PUBLIC ${TTCLASSIC_COMPILE_FLAGS})
      if (BUILD_TEAMTALK_DLL)
        target_include_directories (TeamTalk5Classic PUBLIC ./ tinyxml)
        target_link_libraries (TeamTalk5Classic TeamTalk5DLL)
      else()
        target_include_directories (TeamTalk5Classic PUBLIC ./ ../../Library/TeamTalk_DLL tinyxml)
        target_link_libraries (TeamTalk5Classic TeamTalk5)
      endif()
    endif()
    set_output_dir(TeamTalk5Classic "${CMAKE_CURRENT_SOURCE_DIR}/")
  endif()

  if (BUILD_TEAMTALK_PROCLIENT_MFC_EXAMPLE)
    set_property(TARGET TeamTalk5ClassicPro PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Library/TeamTalk_DLL")

    if (BUILD_TEAMTALK_CLIENT_TOLK)
      target_compile_options (TeamTalk5ClassicPro PUBLIC -DENABLE_TEAMTALKPRO -DENABLE_TOLK ${TTCLASSIC_COMPILE_FLAGS})
      if (BUILD_TEAMTALK_PRODLL)
        target_include_directories (TeamTalk5ClassicPro PUBLIC ./ tinyxml)
        target_link_libraries (TeamTalk5ClassicPro TeamTalk5ProDLL Tolk)
      else()
        target_include_directories (TeamTalk5ClassicPro PUBLIC ./ ../../Library/TeamTalk_DLL tinyxml)
        target_link_libraries (TeamTalk5ClassicPro TeamTalk5Pro Tolk)
      endif()
    else()
      target_compile_options (TeamTalk5ClassicPro PUBLIC -DENABLE_TEAMTALKPRO ${TTCLASSIC_COMPILE_FLAGS})
      if (BUILD_TEAMTALK_PRODLL)
        target_include_directories (TeamTalk5ClassicPro PUBLIC ./ tinyxml)
        target_link_libraries (TeamTalk5ClassicPro TeamTalk5ProDLL)
      else()
        target_link_libraries (TeamTalk5ClassicPro TeamTalk5Pro)
        target_include_directories (TeamTalk5ClassicPro PUBLIC ./ ../../Library/TeamTalk_DLL tinyxml)
      endif()
    endif()
    set_output_dir(TeamTalk5ClassicPro "${CMAKE_CURRENT_SOURCE_DIR}/")
  endif()
endif()
