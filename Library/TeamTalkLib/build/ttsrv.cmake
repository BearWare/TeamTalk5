include (ttsrvlib)
include (tinyxml)

set (TTSRV_INCLUDE_DIR ${TTSRVLIB_INCLUDE_DIR})
set (TTSRV_COMPILE_FLAGS ${TTSRVLIB_COMPILE_FLAGS})
set (TTSRV_LINK_FLAGS ${TTSRVLIB_LINK_FLAGS} ${TINYXML_LINK_FLAGS})

set (TTSRVPRO_INCLUDE_DIR ${TTSRV_INCLUDE_DIR})
set (TTSRVPRO_COMPILE_FLAGS ${TTSRV_COMPILE_FLAGS} -DENABLE_TEAMTALKPRO)
set (TTSRVPRO_LINK_FLAGS ${TTSRV_LINK_FLAGS})
    
set (TTSRV_SOURCES
  ${TTSRVLIB_SOURCES}
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerConfig.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerGuard.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerUtil.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerXML.cpp
  ${TEAMTALKLIB_ROOT}/settings/Settings.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/Main.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/ttsrv.rc)

set (TTSRV_HEADERS
  ${TTSRVLIB_HEADERS}
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/AppInfo.h
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/resource.h
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerConfig.h
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerGuard.h
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerUtil.h
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerXML.h
  ${TEAMTALKLIB_ROOT}/settings/Settings.h)

