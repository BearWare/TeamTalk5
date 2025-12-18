option (MSVS_UNITTEST "Build Visual Studio Unit Tests" OFF)
add_feature_info (MSVS_UNITTEST MSVS_UNITTEST "Visual Studio format unit tests for TeamTalk")
if (MSVS_UNITTEST)
  set (MSVS_UNITTEST_SOURCES ${TEAMTALKLIB_ROOT}/test/UnitTestWin.cpp
	   ${TEAMTALKLIB_ROOT}/test/TTUnitTest.cpp ${TEAMTALKLIB_ROOT}/test/TTUnitTest.h)
endif()

option (BUILD_TEAMTALK_LIBRARY_UNITTEST_CATCH2 "Build Catch Unit Tests" OFF)
add_feature_info (BUILD_TEAMTALK_LIBRARY_UNITTEST_CATCH2 BUILD_TEAMTALK_LIBRARY_UNITTEST_CATCH2 "Catch2 unit tests for TeamTalk")
if (BUILD_TEAMTALK_LIBRARY_UNITTEST_CATCH2)
  set (CATCH_UNITTEST_SOURCES ${TEAMTALKLIB_ROOT}/test/CatchDefault.cpp
    ${TEAMTALKLIB_ROOT}/test/TTUnitTest.cpp
    ${TEAMTALKLIB_ROOT}/test/TTUnitTest.h)

  if (MSVC)
    list (APPEND CATCH_UNITTEST_SOURCES ${TEAMTALKLIB_ROOT}/test/CatchWin.cpp)
  endif()
  
  if (FEATURE_WEBRTC)
    list (APPEND CATCH_UNITTEST_SOURCES ${TEAMTALKLIB_ROOT}/test/CatchWebRTC.cpp)
  endif()

  option (BUILD_TEAMTALK_LIBRARY_UNITTEST_CATCH2_PERF "Include performance dependent Catch Unit Tests" ON)
  add_feature_info (BUILD_TEAMTALK_LIBRARY_UNITTEST_CATCH2_PERF BUILD_TEAMTALK_LIBRARY_UNITTEST_CATCH2_PERF "Catch2 unit tests for TeamTalk that are performance dependent")
  if (BUILD_TEAMTALK_LIBRARY_UNITTEST_CATCH2_PERF)
    list (APPEND CATCH_UNITTEST_SOURCES ${TEAMTALKLIB_ROOT}/test/CatchPerf.cpp)
  endif()

  # macOS finds 'libpcap.tbd' and assumes it's libpcap.so
  if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    # sudo apt install libpcap-dev
    find_library(PCAP_LIBRARY pcap)
    if (PCAP_LIBRARY)
      list (APPEND CATCH_UNITTEST_SOURCES ${TEAMTALKLIB_ROOT}/test/CatchPcap.cpp)
      set (CATCH_LINK_FLAGS -lpcap)
    else()
      message("libpcap not found")
    endif()
  endif()
endif()
