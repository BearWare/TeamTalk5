cmake_minimum_required(VERSION 3.1)
project (TeamTalk5Lib)

set (TEAMTALKLIB_ROOT ${CMAKE_CURRENT_LIST_DIR})

option (BUILD_TEAMTALK_STATIC "Build TeamTalk libraries and executables from toolchain" ON)

if (BUILD_TEAMTALK_STATIC)

  add_subdirectory (toolchain)

  # if ($ENV{TTLIBS_ROOT})
  #   message("Overriding TTLIBS_ROOT with environment variable TTLIBS_ROOT=$ENV{TTLIBS_ROOT}")
  #   set (TTLIBS_ROOT $ENV{TTLIBS_ROOT})
  #   file (TO_CMAKE_PATH ${TTLIBS_ROOT} TTLIBS_ROOT)
  # endif()
  
else()
  message("Building TeamTalk from shared libraries")
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
include (unittest)

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
else()
  list (APPEND COMPILE_FLAGS -Wno-deprecated-declarations)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
  list (APPEND COMPILE_FLAGS -fembed-bitcode)
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

option (BUILD_TEAMTALK5DLL "Build TeamTalk5 shared library" ON)

if (BUILD_TEAMTALK5DLL)

  if (MSVS_UNITTEST)
    list (APPEND TTCLIENTDLL_SOURCES ${MSVS_UNITTEST_SOURCES})
  endif()

  add_library (TeamTalk5DLL SHARED ${TTCLIENTDLL_SOURCES} ${TTCLIENTDLL_HEADERS})

  target_include_directories (TeamTalk5DLL PUBLIC ${TTCLIENTDLL_INCLUDE_DIR})
  target_compile_options (TeamTalk5DLL PUBLIC ${TTCLIENTDLL_COMPILE_FLAGS} ${COMPILE_FLAGS})
  target_link_libraries (TeamTalk5DLL ${TTCLIENTDLL_LINK_FLAGS} ${LINK_LIBS})
  set_target_properties (TeamTalk5DLL PROPERTIES OUTPUT_NAME TeamTalk5)
  
  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5DLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set_output_dir(TeamTalk5DLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_OSX_ARCHITECTURES})
  else()
    set_output_dir(TeamTalk5DLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  endif()

  if (MSVS_UNITTEST)
    set_target_properties (TeamTalk5DLL PROPERTIES LINK_FLAGS "/LIBPATH:$(VCInstallDir)UnitTest/lib")
    set_target_properties (TeamTalk5DLL PROPERTIES COMPILE_FLAGS "/I$(VCInstallDir)UnitTest/include")
  endif()

endif()

######################
# libTeamTalk5.a #
######################

option (BUILD_TEAMTALK5LIB "Build TeamTalk5 static library" OFF)

if (BUILD_TEAMTALK5LIB)

  if (MSVS_UNITTEST)
    list (APPEND TTCLIENTDLL_SOURCES ${MSVS_UNITTEST_SOURCES})
  endif()

  add_library (TeamTalk5Lib STATIC ${TTCLIENTDLL_SOURCES} ${TTCLIENTDLL_HEADERS})

  target_include_directories (TeamTalk5Lib PUBLIC ${TTCLIENTDLL_INCLUDE_DIR})
  target_compile_options (TeamTalk5Lib PUBLIC ${TTCLIENTDLL_COMPILE_FLAGS} ${COMPILE_FLAGS})
  target_link_libraries (TeamTalk5Lib ${TTCLIENTDLL_LINK_FLAGS} ${LINK_LIBS})
  set_target_properties (TeamTalk5Lib PROPERTIES OUTPUT_NAME TeamTalk5)
  
  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5Lib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set_output_dir(TeamTalk5Lib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_OSX_ARCHITECTURES})
  else()
    set_output_dir(TeamTalk5Lib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  endif()

endif()



######################
# libTeamTalk5Pro.so #
######################

option (BUILD_TEAMTALK5PRODLL "Build TeamTalk5Pro shared library" ON)

if (BUILD_TEAMTALK5PRODLL)

  if (MSVS_UNITTEST)
    list (APPEND TTCLIENTPRODLL_SOURCES ${MSVS_UNITTEST_SOURCES})
  endif()

  add_library (TeamTalk5ProDLL SHARED
    ${TTCLIENTPRODLL_SOURCES} ${TTCLIENTPRODLL_HEADERS})

  target_include_directories (TeamTalk5ProDLL PUBLIC
    ${TTCLIENTPRODLL_INCLUDE_DIR})

  target_compile_options (TeamTalk5ProDLL PUBLIC
    ${TTCLIENTPRODLL_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (TeamTalk5ProDLL
    ${TTCLIENTPRODLL_LINK_FLAGS}
    ${LINK_LIBS})

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5ProDLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set_output_dir(TeamTalk5ProDLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_OSX_ARCHITECTURES})
  else()
    set_output_dir(TeamTalk5ProDLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  endif()

  set_target_properties (TeamTalk5ProDLL PROPERTIES OUTPUT_NAME TeamTalk5Pro)
  
  if (MSVS_UNITTEST)
    set_target_properties (TeamTalk5ProDLL PROPERTIES LINK_FLAGS "/LIBPATH:$(VCInstallDir)UnitTest/lib")
    set_target_properties (TeamTalk5ProDLL PROPERTIES COMPILE_FLAGS "/I$(VCInstallDir)UnitTest/include")
  endif()

endif()

######################
# libTeamTalk5Pro.a #
######################

option (BUILD_TEAMTALK5PROLIB "Build TeamTalk5Pro static library" OFF)

if (BUILD_TEAMTALK5PROLIB)

  if (MSVS_UNITTEST)
    list (APPEND TTCLIENTPRODLL_SOURCES ${MSVS_UNITTEST_SOURCES})
  endif()

  add_library (TeamTalk5ProLib STATIC
    ${TTCLIENTPRODLL_SOURCES} ${TTCLIENTPRODLL_HEADERS})

  target_include_directories (TeamTalk5ProLib PUBLIC
    ${TTCLIENTPRODLL_INCLUDE_DIR})

  target_compile_options (TeamTalk5ProLib PUBLIC
    ${TTCLIENTPRODLL_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (TeamTalk5ProLib
    ${TTCLIENTPRODLL_LINK_FLAGS}
    ${LINK_LIBS})

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5ProLib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set_output_dir(TeamTalk5ProLib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_OSX_ARCHITECTURES})
  else()
    set_output_dir(TeamTalk5ProLib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  endif()

  set_target_properties (TeamTalk5ProLib PROPERTIES OUTPUT_NAME TeamTalk5Pro)
  
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

####################
# Catch unit-tests #
####################

if (CATCH_UNITTEST)
  add_executable (catchtt ${CATCH_UNITTEST_SOURCES} ${TTCLIENTDLL_SOURCES} ${TTCLIENTDLL_HEADERS})

  if (BUILD_TEAMTALK_STATIC)
    list (APPEND TTCLIENTDLL_INCLUDE_DIR ${TTLIBS_ROOT}/Catch2/include)
  else()
    # Assuming 'catch' package is installed
  endif()
  
  target_include_directories (catchtt PUBLIC
    ${TTCLIENTDLL_INCLUDE_DIR})

  target_compile_options (catchtt PUBLIC
    ${TTCLIENTDLL_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (catchtt
    ${TTCLIENTDLL_LINK_FLAGS}
    ${LINK_LIBS})

  set_output_dir(catchtt ${TEAMTALK_ROOT}/Library/TeamTalkLib/test)

endif()
