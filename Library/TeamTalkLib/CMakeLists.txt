cmake_minimum_required(VERSION 3.1)
project (TeamTalk5Lib)

set (TEAMTALK_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../)
set (TEAMTALKLIB_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

option (BUILD_TEAMTALK_STATIC "Build TeamTalk using static libraries" ON)

if (BUILD_TEAMTALK_STATIC)
  if (NOT DEFINED ${TTLIBS_ROOT})
    set (TTLIBS_ROOT $ENV{TTLIBS_ROOT})
  endif()
  file (TO_CMAKE_PATH ${TTLIBS_ROOT} TTLIBS_ROOT)
endif()

if (${CMAKE_VERSION} VERSION_GREATER "3.1")
  set (CMAKE_CXX_STANDARD 11)
else()
  if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    list (APPEND COMPILE_FLAGS -std=c++11)
  endif()
endif()

list (APPEND CMAKE_MODULE_PATH "${TEAMTALKLIB_ROOT}/build")

include (ttsrv)
include (ttdll)

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG -D_DEBUG")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  option (STATICCPP "Link static C and C++ libraries" ON)
  if (STATICCPP)
    set (LINK_LIBS -static-libgcc -static-libstdc++)
  endif()
endif()

if (MSVC)
  list (APPEND COMPILE_FLAGS -DUNICODE -D_UNICODE
    -D_CRT_NONSTDC_NO_WARNINGS
    -D_CRT_SECURE_NO_WARNINGS
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_DEPRECATE
    -D_WINSOCK_DEPRECATED_NO_WARNINGS)

  foreach (flag_var
      CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if (${flag_var} MATCHES "/MD")
      STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach(flag_var)
endif()

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

######################
# libTeamTalk5.so #
######################

option (BUILD_TEAMTALK5DLL "Build TeamTalk library" ON)

if (BUILD_TEAMTALK5DLL)
  
  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    add_library (TeamTalk5DLL STATIC
                 ${TTCLIENTDLL_SOURCES} ${TTCLIENTDLL_HEADERS})
  else()
    add_library (TeamTalk5DLL SHARED
                 ${TTCLIENTDLL_SOURCES} ${TTCLIENTDLL_HEADERS})
  endif()

  target_include_directories (TeamTalk5DLL PUBLIC
    ${TTCLIENTDLL_INCLUDE_DIR})

  target_compile_options (TeamTalk5DLL PUBLIC
    ${TTCLIENTDLL_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (TeamTalk5DLL
    ${TTCLIENTDLL_LINK_FLAGS}
    ${LINK_LIBS})

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5DLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
  else()
    set_output_dir(TeamTalk5DLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  endif()

  set_target_properties (TeamTalk5DLL PROPERTIES OUTPUT_NAME TeamTalk5)

  if (MSVS_UNITTEST)
    set_target_properties (TeamTalk5DLL PROPERTIES LINK_FLAGS "/LIBPATH:$(VCInstallDir)UnitTest/lib")
    set_target_properties (TeamTalk5DLL PROPERTIES COMPILE_FLAGS "/I$(VCInstallDir)UnitTest/include")
  endif()

endif()

######################
# libTeamTalk5Pro.so #
######################

option (BUILD_TEAMTALK5PRODLL "Build TeamTalk shared library" ON)

if (BUILD_TEAMTALK5PRODLL)

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    add_library (TeamTalk5ProDLL STATIC
      ${TTCLIENTPRODLL_SOURCES} ${TTCLIENTPRODLL_HEADERS})
  else()
    add_library (TeamTalk5ProDLL SHARED
      ${TTCLIENTPRODLL_SOURCES} ${TTCLIENTPRODLL_HEADERS})
  endif()

  target_include_directories (TeamTalk5ProDLL PUBLIC
    ${TTCLIENTPRODLL_INCLUDE_DIR})

  target_compile_options (TeamTalk5ProDLL PUBLIC
    ${TTCLIENTPRODLL_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (TeamTalk5ProDLL
    ${TTCLIENTPRODLL_LINK_FLAGS}
    ${LINK_LIBS})

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5ProDLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
  else()
    set_output_dir(TeamTalk5ProDLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  endif()

  set_target_properties (TeamTalk5ProDLL PROPERTIES OUTPUT_NAME TeamTalk5Pro)

  if (MSVS_UNITTEST)
    set_target_properties (TeamTalk5ProDLL PROPERTIES LINK_FLAGS "/LIBPATH:$(VCInstallDir)UnitTest/lib")
    set_target_properties (TeamTalk5ProDLL PROPERTIES COMPILE_FLAGS "/I$(VCInstallDir)UnitTest/include")
  endif()

endif()

##########
# tt5srv #
##########

option (BUILD_TT5SRV "Build TeamTalk server" ON)

if (BUILD_TT5SRV)
  add_executable (tt5srv
    ${TTSRV_SOURCES} ${TTSRV_HEADERS})

  target_include_directories (tt5srv PUBLIC
    ${TTSRV_INCLUDE_DIR})

  target_compile_options (tt5srv PUBLIC
    ${TTSRV_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (tt5srv
    ${TTSRV_LINK_FLAGS}
    ${LINK_LIBS})

  set_output_dir(tt5srv ${TEAMTALK_ROOT}/Server)

  if (MSVC)
    set_property(TARGET tt5srv PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Server")
  endif()
endif()

#############
# tt5prosrv #
#############

option (BUILD_TT5PROSRV "Build TeamTalk server" ON)

if (BUILD_TT5PROSRV)

  add_executable (tt5prosrv
    ${TTSRV_SOURCES} ${TTSRV_HEADERS})

  target_include_directories (tt5prosrv PUBLIC
    ${TTSRVPRO_INCLUDE_DIR})

  target_compile_options (tt5prosrv PUBLIC
    ${TTSRVPRO_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (tt5prosrv
    ${TTSRVPRO_LINK_FLAGS}
    ${LINK_LIBS})

  set_output_dir(tt5prosrv ${TEAMTALK_ROOT}/Server)

  if (MSVC)
    set_property(TARGET tt5prosrv PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Server")
  endif()
  
endif()

if (MSVC)

  ##############
  # tt5svc.exe #
  ##############
  option (BUILD_TT5SVC "Build TeamTalk service" ON)

  if (BUILD_TT5SVC)
    add_executable (tt5svc
      ${TTSRV_SOURCES} ${TTSRV_HEADERS})

    target_include_directories (tt5svc PUBLIC
      ${TTSRV_INCLUDE_DIR})

    target_compile_options (tt5svc PUBLIC
      ${TTSRV_COMPILE_FLAGS} ${COMPILE_FLAGS}
      -DBUILD_NT_SERVICE)

    target_link_libraries (tt5svc
      ${TTSRV_LINK_FLAGS}
      ${LINK_LIBS})

    add_custom_command (
      TARGET tt5svc
      POST_BUILD
      COMMAND mt
      ARGS -manifest tt5svc.exe.manifest "-outputresource:$(TargetPath);#1"
      WORKING_DIRECTORY ${TEAMTALKLIB_ROOT}/build)

    set_output_dir(tt5svc ${TEAMTALK_ROOT}/Server)
    
    if (MSVC)
      set_property(TARGET tt5svc PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Server")
    endif()
  endif()

  #################
  # tt5prosvc.exe #
  #################
  option (BUILD_TT5PROSVC "Build TeamTalk Professional service" ON)

  if (BUILD_TT5PROSVC)
    add_executable (tt5prosvc
      ${TTSRV_SOURCES} ${TTSRV_HEADERS})

    target_include_directories (tt5prosvc PUBLIC
      ${TTSRVPRO_INCLUDE_DIR})

    target_compile_options (tt5prosvc PUBLIC
      ${TTSRVPRO_COMPILE_FLAGS} ${COMPILE_FLAGS}
      -DBUILD_NT_SERVICE)

    target_link_libraries (tt5prosvc
      ${TTSRVPRO_LINK_FLAGS}
      ${LINK_LIBS})

    add_custom_command (TARGET tt5prosvc
      POST_BUILD
      COMMAND mt
      ARGS -manifest tt5svc.exe.manifest "-outputresource:$(TargetPath);#1"
      WORKING_DIRECTORY ${TEAMTALKLIB_ROOT}/build)

    set_output_dir (tt5prosvc ${TEAMTALK_ROOT}/Server)

    if (MSVC)
      set_property(TARGET tt5prosvc PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Server")
    endif()
    
  endif()
endif()
