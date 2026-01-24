project (TeamTalk5Classic)

include(CMakeDependentOption)
include(ExternalProject)

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

  ExternalProject_Add(tinyxml2-classic-src
      GIT_REPOSITORY    https://github.com/leethomason/tinyxml2.git
      GIT_TAG           11.0.0
      UPDATE_COMMAND    ""
      CMAKE_ARGS        -DCMAKE_DEBUG_POSTFIX=d -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug> -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -Dtinyxml2_INSTALL_PKGCONFIG=OFF
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --config Release
      INSTALL_COMMAND   ""
      BUILD_BYPRODUCTS  ${BINARY_DIR}/Debug/tinyxml2d.lib
                        ${BINARY_DIR}/Release/tinyxml2.lib
      )
  ExternalProject_Get_Property(tinyxml2-classic-src SOURCE_DIR)
  ExternalProject_Get_Property(tinyxml2-classic-src BINARY_DIR)

  add_library(tinyxml2-classic STATIC IMPORTED GLOBAL)
  add_dependencies(tinyxml2-classic tinyxml2-classic-src)
  target_include_directories (tinyxml2-classic INTERFACE ${SOURCE_DIR})
  set_target_properties(tinyxml2-classic PROPERTIES
    IMPORTED_LOCATION_DEBUG ${BINARY_DIR}/Debug/tinyxml2d.lib
    IMPORTED_LOCATION ${BINARY_DIR}/Release/tinyxml2.lib)

  if ("${CMAKE_GENERATOR}" MATCHES "Visual Studio")
    set (VS_BUILD 1)
  else()
    set (VS_BUILD 0)
  endif()

  cmake_dependent_option (BUILD_TEAMTALK_CLIENT_TEAMTALKCLASSIC "Build MFC TeamTalk 5 Classic for client example" ON "VS_BUILD" OFF)

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
    if (BUILD_TEAMTALK_CLIENT_TEAMTALKCLASSIC)
      add_executable (TeamTalk5Classic WIN32 ${TTCLASSIC_SOURCES} ${TTCLASSIC_HEADERS} res/TeamTalk_amd64.manifest)
    endif()
  else()
    if (BUILD_TEAMTALK_CLIENT_TEAMTALKCLASSIC)
      add_executable (TeamTalk5Classic WIN32 ${TTCLASSIC_SOURCES} ${TTCLASSIC_HEADERS} res/TeamTalk.manifest)
    endif()
  endif()

  set (TTCLASSIC_COMPILE_FLAGS -DUNICODE -D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS -D_UNICODE -wd4800)

  if (BUILD_TEAMTALK_CLIENT_TEAMTALKCLASSIC)
    set_property(TARGET TeamTalk5Classic PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Library/TeamTalk_DLL")

    if (BUILD_TEAMTALK_CLIENT_TOLK)
      target_compile_options (TeamTalk5Classic PUBLIC -DENABLE_TOLK ${TTCLASSIC_COMPILE_FLAGS})
      install (FILES ${TOLK_DLL_FILES} DESTINATION Client/TeamTalkClassic)
      if (BUILD_TEAMTALK_LIBRARY_DLL)
        target_include_directories (TeamTalk5Classic PUBLIC ./)
        target_link_libraries (TeamTalk5Classic TeamTalk5DLL Tolk tinyxml2-classic)
      else()
        target_include_directories (TeamTalk5Classic PUBLIC ./ ../../Library/TeamTalk_DLL)
        target_link_libraries (TeamTalk5Classic TeamTalk5 Tolk tinyxml2-classic)
      endif()
    else()
      target_compile_options (TeamTalk5Classic PUBLIC ${TTCLASSIC_COMPILE_FLAGS})
      if (BUILD_TEAMTALK_LIBRARY_DLL)
        target_include_directories (TeamTalk5Classic PUBLIC ./)
        target_link_libraries (TeamTalk5Classic TeamTalk5DLL tinyxml2-classic)
      else()
        target_include_directories (TeamTalk5Classic PUBLIC ./ ../../Library/TeamTalk_DLL)
        target_link_libraries (TeamTalk5Classic TeamTalk5 tinyxml2-classic)
      endif()
    endif()
    set_output_dir(TeamTalk5Classic "${CMAKE_CURRENT_SOURCE_DIR}/")
    install (TARGETS TeamTalk5Classic DESTINATION Client/TeamTalkClassic)
    install (DIRECTORY lng_classic DESTINATION Client/TeamTalkClassic)
  endif()

endif()
