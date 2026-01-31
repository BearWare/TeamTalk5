project (TeamTalk5Lib LANGUAGES CXX)

# Build Multi-threaded instead of Multi-Treaded DLL to avoid MSVC
# dependency hell
set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include (FeatureSummary)

set (TEAMTALKLIB_ROOT ${CMAKE_CURRENT_LIST_DIR})

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG -D_DEBUG")

# WebRTC makes use of std::optional which is C++17.
# Also ACE requires C++ 17.
set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  option (STATICCPP "Link static C and C++ libraries" ON)
  if (STATICCPP)
    set (LINK_LIBS -static-libgcc -static-libstdc++)
  endif()
endif()

if (MSVC)
  set (COMPILE_FLAGS -DUNICODE -D_UNICODE
    -D_CRT_NONSTDC_NO_WARNINGS
    -D_CRT_SECURE_NO_WARNINGS
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_DEPRECATE
    -D_WINSOCK_DEPRECATED_NO_WARNINGS)

  # Use for building TeamTalk toolchain
  set (PLATFORMTOOLSET "/property:PlatformToolset=v143")

else()
  set (COMPILE_FLAGS -Wno-deprecated-declarations)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set (CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
    message(WARNING "No deployment target specified. Defaulting to ${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endif()

  if (NOT CMAKE_OSX_ARCHITECTURES)
    message(WARNING "No architecture specified. Defaulting to ${CMAKE_OSX_ARCHITECTURES}")
  endif()

  # Prevent clang from including /usr/local (where Homebrew installs
  # its header files and libraries)
  list (APPEND COMPILE_FLAGS -Xclang -nostdsysteminc)
  set (LINK_LIBS -Xclang -nostdsysteminc)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set (CMAKE_OSX_DEPLOYMENT_TARGET 8.0)
    message(WARNING "No deployment target specified. Defaulting to ${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endif()
  list (APPEND COMPILE_FLAGS -fembed-bitcode)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  if (NOT ANDROID_PLATFORM)
    message(WARNING "No Android API level specified.")
  endif()

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

option (FEATURE_LIBVPX "Build TeamTalk DLL with libVPX support" ON)
option (FEATURE_OGG "Build TeamTalk DLL with libOgg support" ON)
option (FEATURE_OPUS "Build TeamTalk DLL with OPUS codec support" ON)
option (FEATURE_OPUSTOOLS "Build TeamTalk DLL with Opus-tools support" ON)
option (FEATURE_SPEEX "Build TeamTalk DLL with Speex codec support" ON)
option (FEATURE_SPEEXDSP "Build TeamTalk DLL with SpeexDSP audio processing support" ON)
option (FEATURE_WEBRTC "Build TeamTalk DLL with WebRTC audio processing support" ON)

add_feature_info (FEATURE_LIBVPX FEATURE_LIBVPX "libVPX video codec")
add_feature_info (FEATURE_OGG FEATURE_OGG "Ogg file format")
add_feature_info (FEATURE_OPUS FEATURE_OPUS "OPUS codec")
add_feature_info (FEATURE_OPUSTOOLS FEATURE_OPUSTOOLS "OPUS-tools for storing OPUS in .ogg files")
add_feature_info (FEATURE_SPEEX FEATURE_SPEEX "Speex codec")
add_feature_info (FEATURE_SPEEXDSP FEATURE_SPEEXDSP "SpeexDSP for audio processing (AGC, echo cancellation, etc.)")
add_feature_info (FEATURE_WEBRTC FEATURE_WEBRTC "WebRTC for audio processing (AGC, echo cancellation, voice detection, etc.)")

if (MSVC)
  option (FEATURE_DSHOW "Build TeamTalk DLL with Microsoft DirectShow Streaming support (obsolete)" OFF)
  option (FEATURE_MEDIAFOUNDATION "Build TeamTalk DLL with Media Foundation Streaming support" ON)
  option (FEATURE_MSDMO "Build TeamTalk DLL with Microsoft DirectX Media Objects (DMO) Resampler support" ON)
  option (FEATURE_VIDCAP "Build TeamTalk DLL with DirectShow Video Capture support (obsolete)" OFF)
  add_feature_info (FEATURE_MEDIAFOUNDATION FEATURE_MEDIAFOUNDATION "Microsoft Media Foundation for A/V processing")
  add_feature_info (FEATURE_MSDMO FEATURE_MSDMO "Microsoft DirectX for audio resampling")
else()
  option (FEATURE_FFMPEG "Build TeamTalk DLL with FFmpeg support" ON)
  add_feature_info (FEATURE_FFMPEG FEATURE_FFMPEG "FFmpeg A/V library")
endif()
if (FEATURE_FFMPEG)
  if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    option(FEATURE_V4L2 "Build TeamTalk DLL with Video for Linux 2 (V4L2) support (video capture)" ON)
    add_feature_info (FEATURE_V4L2 FEATURE_V4L2 "Video4Linux v2 (V4L2) for video capture")
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    option(FEATURE_AVF "Build TeamTalk DLL with AVFoundation support (video capture)" ON)
    add_feature_info (FEATURE_AVF FEATURE_AVF "Audio/Video Foundation (AVF) for video capture")
  endif()
endif()

# Sound system options
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR
    ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR
    ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  option (FEATURE_PORTAUDIO "Build using PortAudio as sound system" ON)
  add_feature_info (FEATURE_PORTAUDIO FEATURE_PORTAUDIO "PortAudio as sound system")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
  option (FEATURE_OPENSLES "Build using OpenSL ES as sound system" ON)
  add_feature_info (FEATURE_OPENSLES FEATURE_OPENSLES "OpenSL ES as sound system")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
  option (FEATURE_AUDIOUNIT "Build using AudioUnit as sound system" ON)
  add_feature_info (FEATURE_AUDIOUNIT FEATURE_AUDIOUNIT "AudioUnit as sound system")
endif()

add_subdirectory (build)

list (APPEND CMAKE_MODULE_PATH "${TEAMTALKLIB_ROOT}/build")

include (ttsrv)
include (ttdll)
include (unittest)


######################
# libTeamTalk5.so #
######################

option (BUILD_TEAMTALK_LIBRARY_DLL "Build TeamTalk5 shared library" ON)
add_feature_info (BUILD_TEAMTALK_LIBRARY_DLL BUILD_TEAMTALK_LIBRARY_DLL "Build shared library [TeamTalk5.dll | libTeamTalk5.so | libTeamTalk5.dylib] in Library/TeamTalk_DLL")
if (BUILD_TEAMTALK_LIBRARY_DLL)

  if (MSVS_UNITTEST)
    list (APPEND TTCLIENTDLL_SOURCES ${MSVS_UNITTEST_SOURCES})
  endif()

  add_library (TeamTalk5DLL SHARED ${TTCLIENTDLL_SOURCES} ${TTCLIENTDLL_HEADERS})

  target_include_directories (TeamTalk5DLL PRIVATE ${TTCLIENTDLL_INCLUDE_DIR} INTERFACE ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  target_compile_options (TeamTalk5DLL PRIVATE ${TTCLIENTDLL_COMPILE_FLAGS} ${COMPILE_FLAGS})
  target_link_libraries (TeamTalk5DLL PRIVATE ${TTCLIENTDLL_LINK_FLAGS} ${LINK_LIBS})
  set_target_properties (TeamTalk5DLL PROPERTIES OUTPUT_NAME TeamTalk5)

  if (MSVC AND ${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    target_link_options (TeamTalk5DLL PRIVATE /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:vcruntime.lib)
  endif()

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5DLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set_output_dir(TeamTalk5DLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_OSX_ARCHITECTURES})
  else()
    set_output_dir(TeamTalk5DLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  endif()

  install (FILES ${CMAKE_CURRENT_LIST_DIR}/../TeamTalk_DLL/TeamTalk.h DESTINATION Library/TeamTalk_DLL)
  install (TARGETS TeamTalk5DLL DESTINATION Library/TeamTalk_DLL)

  if (MSVS_UNITTEST)
    set_target_properties (TeamTalk5DLL PROPERTIES LINK_FLAGS "/LIBPATH:$(VCInstallDir)UnitTest/lib")
    set_target_properties (TeamTalk5DLL PROPERTIES COMPILE_FLAGS "/I$(VCInstallDir)UnitTest/include")
  endif()

endif()

######################
# libTeamTalk5.a #
######################

option (BUILD_TEAMTALK_LIBRARY_LIB "Build TeamTalk5 static library" OFF)
add_feature_info (BUILD_TEAMTALK_LIBRARY_LIB BUILD_TEAMTALK_LIBRARY_LIB "Build static library [TeamTalk5.lib | libTeamTalk5.a ] in Library/TeamTalk_DLL")

if (BUILD_TEAMTALK_LIBRARY_LIB)

  if (MSVS_UNITTEST)
    list (APPEND TTCLIENTDLL_SOURCES ${MSVS_UNITTEST_SOURCES})
  endif()

  add_library (TeamTalk5Lib STATIC ${TTCLIENTDLL_SOURCES} ${TTCLIENTDLL_HEADERS})

  target_include_directories (TeamTalk5Lib PRIVATE ${TTCLIENTDLL_INCLUDE_DIR} INTERFACE ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  target_compile_options (TeamTalk5Lib PRIVATE ${TTCLIENTDLL_COMPILE_FLAGS} ${COMPILE_FLAGS})
  target_link_libraries (TeamTalk5Lib PRIVATE ${TTCLIENTDLL_LINK_FLAGS} ${LINK_LIBS})
  set_target_properties (TeamTalk5Lib PROPERTIES OUTPUT_NAME TeamTalk5)

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5Lib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set_output_dir(TeamTalk5Lib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_OSX_ARCHITECTURES})
  else()
    set_output_dir(TeamTalk5Lib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  endif()

  install (TARGETS TeamTalk5Lib DESTINATION Library/TeamTalk_DLL)

endif()



######################
# libTeamTalk5Pro.so #
######################

option (BUILD_TEAMTALK_LIBRARY_DLLPRO "Build TeamTalk5Pro shared library" ON)
add_feature_info (BUILD_TEAMTALK_LIBRARY_DLLPRO BUILD_TEAMTALK_LIBRARY_DLLPRO "Build shared library [TeamTalk5Pro.dll | libTeamTalk5Pro.so | libTeamTalk5Pro.dylib] in Library/TeamTalk_DLL")

if (BUILD_TEAMTALK_LIBRARY_DLLPRO)

  if (MSVS_UNITTEST)
    list (APPEND TTCLIENTPRODLL_SOURCES ${MSVS_UNITTEST_SOURCES})
  endif()

  add_library (TeamTalk5ProDLL SHARED
    ${TTCLIENTPRODLL_SOURCES} ${TTCLIENTPRODLL_HEADERS})

  target_include_directories (TeamTalk5ProDLL PRIVATE
    ${TTCLIENTPRODLL_INCLUDE_DIR} INTERFACE ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)

  target_compile_options (TeamTalk5ProDLL PRIVATE
    ${TTCLIENTPRODLL_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (TeamTalk5ProDLL PRIVATE ${TTCLIENTPRODLL_LINK_FLAGS} ${LINK_LIBS})

  if (MSVC AND ${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    target_link_options (TeamTalk5ProDLL PRIVATE /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:vcruntime.lib)
  endif()

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5ProDLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set_output_dir(TeamTalk5ProDLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_OSX_ARCHITECTURES})
  else()
    set_output_dir(TeamTalk5ProDLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  endif()

  install (FILES ${CMAKE_CURRENT_LIST_DIR}/../TeamTalk_DLL/TeamTalk.h ${CMAKE_CURRENT_LIST_DIR}/../TeamTalk_DLL/TeamTalkSrv.h DESTINATION Library/TeamTalk_DLL)
  install (TARGETS TeamTalk5ProDLL DESTINATION Library/TeamTalk_DLL)

  set_target_properties (TeamTalk5ProDLL PROPERTIES OUTPUT_NAME TeamTalk5Pro)

  if (MSVS_UNITTEST)
    set_target_properties (TeamTalk5ProDLL PROPERTIES LINK_FLAGS "/LIBPATH:$(VCInstallDir)UnitTest/lib")
    set_target_properties (TeamTalk5ProDLL PROPERTIES COMPILE_FLAGS "/I$(VCInstallDir)UnitTest/include")
  endif()

endif()

######################
# libTeamTalk5Pro.a #
######################

option (BUILD_TEAMTALK_LIBRARY_LIBPRO "Build TeamTalk5Pro static library" OFF)
add_feature_info (BUILD_TEAMTALK_LIBRARY_LIBPRO BUILD_TEAMTALK_LIBRARY_LIBPRO "Build static library [TeamTalk5Pro.lib | libTeamTalk5Pro.a ] in Library/TeamTalk_DLL")

if (BUILD_TEAMTALK_LIBRARY_LIBPRO)

  if (MSVS_UNITTEST)
    list (APPEND TTCLIENTPRODLL_SOURCES ${MSVS_UNITTEST_SOURCES})
  endif()

  add_library (TeamTalk5ProLib STATIC
    ${TTCLIENTPRODLL_SOURCES} ${TTCLIENTPRODLL_HEADERS})

  target_include_directories (TeamTalk5ProLib PRIVATE
    ${TTCLIENTPRODLL_INCLUDE_DIR} INTERFACE ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)

  target_compile_options (TeamTalk5ProLib PRIVATE
    ${TTCLIENTPRODLL_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (TeamTalk5ProLib PRIVATE
    ${TTCLIENTPRODLL_LINK_FLAGS} ${LINK_LIBS})

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5ProLib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set_output_dir(TeamTalk5ProLib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_OSX_ARCHITECTURES})
  else()
    set_output_dir(TeamTalk5ProLib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  endif()

  install (TARGETS TeamTalk5ProLib DESTINATION Library/TeamTalk_DLL)

  set_target_properties (TeamTalk5ProLib PROPERTIES OUTPUT_NAME TeamTalk5Pro)

endif()


##########
# tt5srv #
##########

option (BUILD_TEAMTALK_SERVER_SRVEXE "Build TeamTalk server" ON)
add_feature_info (BUILD_TEAMTALK_SERVER_SRVEXE BUILD_TEAMTALK_SERVER_SRVEXE "Build TeamTalk server executable [tt5srv.exe | tt5srv ] in Server")

if (BUILD_TEAMTALK_SERVER_SRVEXE)
  add_executable (tt5srv
    ${TTSRV_SOURCES} ${TTSRV_HEADERS})

  target_include_directories (tt5srv PRIVATE ${TTSRV_INCLUDE_DIR})

  target_compile_options (tt5srv PRIVATE ${TTSRV_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (tt5srv PRIVATE ${TTSRV_LINK_FLAGS} ${LINK_LIBS})

  set_output_dir(tt5srv ${TEAMTALK_ROOT}/Server)

  install (TARGETS tt5srv DESTINATION Server)

  if (MSVC)
    set_property(TARGET tt5srv PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Server")
  endif()
endif()

#############
# tt5prosrv #
#############

option (BUILD_TEAMTALK_SERVER_SRVEXEPRO "Build TeamTalk server" ON)
add_feature_info (BUILD_TEAMTALK_SERVER_SRVEXEPRO BUILD_TEAMTALK_SERVER_SRVEXEPRO "Build TeamTalk Pro server executable [tt5prosrv.exe | tt5prosrv ] in Server")

if (BUILD_TEAMTALK_SERVER_SRVEXEPRO)

  add_executable (tt5prosrv ${TTSRV_SOURCES} ${TTSRV_HEADERS})

  target_include_directories (tt5prosrv PRIVATE ${TTSRVPRO_INCLUDE_DIR})

  target_compile_options (tt5prosrv PRIVATE ${TTSRVPRO_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (tt5prosrv PRIVATE ${TTSRVPRO_LINK_FLAGS} ${LINK_LIBS})

  set_output_dir(tt5prosrv ${TEAMTALK_ROOT}/Server)

  install (TARGETS tt5prosrv DESTINATION Server)

  if (MSVC)
    set_property(TARGET tt5prosrv PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Server")
  endif()

endif()

if (MSVC)

  ##############
  # tt5svc.exe #
  ##############
  option (BUILD_TEAMTALK_SERVER_SVCEXE "Build TeamTalk service" ON)
  add_feature_info (BUILD_TEAMTALK_SERVER_SVCEXE BUILD_TEAMTALK_SERVER_SVCEXE "Build TeamTalk service executable [tt5svc.exe ] in Server")

  if (BUILD_TEAMTALK_SERVER_SVCEXE)
    add_executable (tt5svc ${TTSRV_SOURCES} ${TTSRV_HEADERS})

    target_include_directories (tt5svc PRIVATE ${TTSRV_INCLUDE_DIR})

    target_compile_options (tt5svc PRIVATE ${TTSRV_COMPILE_FLAGS} ${COMPILE_FLAGS}
      -DBUILD_NT_SERVICE)

    target_link_libraries (tt5svc PRIVATE ${TTSRV_LINK_FLAGS} ${LINK_LIBS})

    set_output_dir(tt5svc ${TEAMTALK_ROOT}/Server)

    set_property(TARGET tt5svc PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Server")

    file (TO_NATIVE_PATH "${TEAMTALK_ROOT}/Server/tt5svc.exe" TT5SVC_EXE_PATH)

    add_custom_command (
      TARGET tt5svc
      POST_BUILD
      COMMAND mt
      ARGS -manifest tt5svc.exe.manifest "-outputresource:${TT5SVC_EXE_PATH};#1"
      WORKING_DIRECTORY ${TEAMTALKLIB_ROOT}/build)

    install (TARGETS tt5svc DESTINATION Server)

  endif()

  #################
  # tt5prosvc.exe #
  #################
  option (BUILD_TEAMTALK_SERVER_SVCEXEPRO "Build TeamTalk Professional service" ON)
  add_feature_info (BUILD_TEAMTALK_SERVER_SVCEXEPRO BUILD_TEAMTALK_SERVER_SVCEXEPRO "Build TeamTalk Pro service executable [tt5prosvc.exe ] in Server")

  if (BUILD_TEAMTALK_SERVER_SVCEXEPRO)
    add_executable (tt5prosvc ${TTSRV_SOURCES} ${TTSRV_HEADERS})

    target_include_directories (tt5prosvc PRIVATE ${TTSRVPRO_INCLUDE_DIR})

    target_compile_options (tt5prosvc PRIVATE ${TTSRVPRO_COMPILE_FLAGS} ${COMPILE_FLAGS}
      -DBUILD_NT_SERVICE)

    target_link_libraries (tt5prosvc PRIVATE ${TTSRVPRO_LINK_FLAGS} ${LINK_LIBS})

    set_output_dir (tt5prosvc ${TEAMTALK_ROOT}/Server)

    set_property(TARGET tt5prosvc PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Server")

    file (TO_NATIVE_PATH "${TEAMTALK_ROOT}/Server/tt5prosvc.exe" TT5PROSVC_EXE_PATH)

    add_custom_command (TARGET tt5prosvc
      POST_BUILD
      COMMAND mt
      ARGS -manifest tt5svc.exe.manifest "-outputresource:${TT5PROSVC_EXE_PATH};#1"
      WORKING_DIRECTORY ${TEAMTALKLIB_ROOT}/build)

    install (TARGETS tt5prosvc DESTINATION Server)

  endif()
endif()

####################
# Catch unit-tests #
####################

if (BUILD_TEAMTALK_LIBRARY_UNITTEST_CATCH2)

  add_executable (catchtt ${CATCH_UNITTEST_SOURCES} ${TTCLIENTPRODLL_SOURCES} ${TTCLIENTPRODLL_HEADERS}
      ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerConfig.cpp ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerConfig.h
      ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerGuard.cpp ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerGuard.h
      ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerUtil.cpp ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerUtil.h
      ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerXML.cpp ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerXML.h
      ${TEAMTALKLIB_ROOT}/bin/ttsrv/AppInfo.h ${TEAMTALKLIB_ROOT}/settings/Settings.cpp
      ${TEAMTALKLIB_ROOT}/settings/Settings.h)

  if (TOOLCHAIN_CATCH2)
    target_link_libraries (catchtt catch2main catch2)
  else()
    find_package(Catch2 3 REQUIRED)
    target_link_libraries (catchtt Catch2::Catch2WithMain)
  endif()

  target_include_directories (catchtt PRIVATE
    ${TTCLIENTPRODLL_INCLUDE_DIR})

  target_compile_options (catchtt PRIVATE
    ${TTCLIENTPRODLL_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (catchtt ${TTCLIENTPRODLL_LINK_FLAGS}
    ${LINK_LIBS} ${CATCH_LINK_FLAGS} ${TINYXML2_LINK_FLAGS})

  if (MSVC AND ${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    target_link_options (catchtt PRIVATE /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:vcruntime.lib)
  endif()

  set_output_dir(catchtt ${TEAMTALK_ROOT}/Library/TeamTalkLib/test)

  set_property(TARGET catchtt PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test")

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set_target_properties (catchtt PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${TEAMTALK_ROOT}/Library/TeamTalkLib/test/Info.plist)
  endif()

endif()

feature_summary(WHAT ALL)
