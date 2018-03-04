
if (MSVC)
  set ( DSHOW_INCLUDE_DIR ${TTLIBS_ROOT}/DirectShow/BaseClasses )

  set ( DSHOW_STATIC_LIB optimized ${TTLIBS_ROOT}/DirectShow/BaseClasses/lib/$(PlatformName)/strmbase.lib
    debug ${TTLIBS_ROOT}/DirectShow/BaseClasses/lib/$(PlatformName)/strmbased.lib)
endif()
