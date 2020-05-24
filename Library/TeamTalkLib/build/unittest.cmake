option (MSVS_UNITTEST "Build Visual Studio Unit Tests" OFF)
if (MSVS_UNITTEST)
  set (MSVS_UNITTEST_SOURCES ${TEAMTALKLIB_ROOT}/test/UnitTestWin.cpp
	   ${TEAMTALKLIB_ROOT}/test/TTUnitTest.cpp ${TEAMTALKLIB_ROOT}/test/TTUnitTest.h)
endif()

option (CATCH_UNITTEST "Build Catch Unit Tests" OFF)
if (CATCH_UNITTEST)
  set (CATCH_UNITTEST_SOURCES ${TEAMTALKLIB_ROOT}/test/CatchUnitTest.cpp
    ${TEAMTALKLIB_ROOT}/test/CatchUnitTestWin.cpp
    ${TEAMTALKLIB_ROOT}/test/CatchMain.cpp ${TEAMTALKLIB_ROOT}/test/TTUnitTest.cpp
    ${TEAMTALKLIB_ROOT}/test/TTUnitTest.h)
endif()
