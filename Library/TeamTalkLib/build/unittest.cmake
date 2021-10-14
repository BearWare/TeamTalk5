option (MSVS_UNITTEST "Build Visual Studio Unit Tests" OFF)
if (MSVS_UNITTEST)
  set (MSVS_UNITTEST_SOURCES ${TEAMTALKLIB_ROOT}/test/UnitTestWin.cpp
	   ${TEAMTALKLIB_ROOT}/test/TTUnitTest.cpp ${TEAMTALKLIB_ROOT}/test/TTUnitTest.h)
endif()

option (CATCH_UNITTEST "Build Catch Unit Tests" OFF)
if (CATCH_UNITTEST)
  set (CATCH_UNITTEST_SOURCES ${TEAMTALKLIB_ROOT}/test/CatchDefault.cpp
    ${TEAMTALKLIB_ROOT}/test/CatchMain.cpp ${TEAMTALKLIB_ROOT}/test/TTUnitTest.cpp
    ${TEAMTALKLIB_ROOT}/test/TTUnitTest.h)

  if (MSVC)
    list (APPEND CATCH_UNITTEST_SOURCES ${TEAMTALKLIB_ROOT}/test/CatchWin.cpp)
  endif()
  
  if (FEATURE_WEBRTC)
    list (APPEND CATCH_UNITTEST_SOURCES ${TEAMTALKLIB_ROOT}/test/CatchWebRTC.cpp)
  endif()

  option (CATCH_UNITTEST_PERF "Include performance dependent Catch Unit Tests" ON)
  if (CATCH_UNITTEST_PERF)
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
