# DirectShow from Windows SDK
# https://msdn.microsoft.com/en-us/library/windows/desktop/dd407279(v=vs.85).aspx
if (MSVC)
  set ( DSHOW_INCLUDE_DIR ${TTLIBS_ROOT}/DirectShow/BaseClasses )

  set ( DSHOW_STATIC_LIB optimized ${TTLIBS_ROOT}/DirectShow/BaseClasses/lib/${CMAKE_VS_PLATFORM_NAME}/strmbase.lib
    debug ${TTLIBS_ROOT}/DirectShow/BaseClasses/lib/${CMAKE_VS_PLATFORM_NAME}/strmbased.lib)
endif()
