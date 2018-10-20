include (ttlib)

if (MSVC)
  set (UNITTEST_INCLUDE_DIR $(VCInstallDir)UnitTest/include)
  set (UNITTEST_LINK_FLAGS $(VCInstallDir)UnitTest/lib)
  set (UNITTEST_SOURCES ${TEAMTALKLIB_ROOT}/test/UnitTestWin.cpp)
endif()
