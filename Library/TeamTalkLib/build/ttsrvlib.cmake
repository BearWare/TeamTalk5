include (ttlib)

set (TTSRVLIB_INCLUDE_DIR ${ACE_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${TEAMTALKLIB_ROOT})
set (TTSRVLIB_COMPILE_FLAGS ${ACE_COMPILE_FLAGS} -DENABLE_HTTP_AUTH -DENABLE_ENCRYPTION)
# SSL required for HTTPS weblogin (Facebook/BearWare.dk)
set (TTSRVLIB_LINK_FLAGS ${ACE_LINK_FLAGS} ${OPENSSL_LINK_FLAGS})

set (TTSRVLIB_HEADERS 
  ${TEAMTALKLIB_ROOT}/TeamTalkDefs.h
  ${TEAMTALKLIB_ROOT}/myace/MyACE.h
  ${TEAMTALKLIB_ROOT}/myace/TimerHandler.h
  ${TEAMTALKLIB_ROOT}/mystd/MyStd.h
  ${TEAMTALKLIB_ROOT}/teamtalk/Channel.h
  ${TEAMTALKLIB_ROOT}/teamtalk/CodecCommon.h
  ${TEAMTALKLIB_ROOT}/teamtalk/Commands.h
  ${TEAMTALKLIB_ROOT}/teamtalk/Common.h
  ${TEAMTALKLIB_ROOT}/teamtalk/DesktopSession.h
  ${TEAMTALKLIB_ROOT}/teamtalk/Log.h
  ${TEAMTALKLIB_ROOT}/teamtalk/PacketHandler.h
  ${TEAMTALKLIB_ROOT}/teamtalk/PacketHelper.h
  ${TEAMTALKLIB_ROOT}/teamtalk/PacketLayout.h
  ${TEAMTALKLIB_ROOT}/teamtalk/StreamHandler.h
  ${TEAMTALKLIB_ROOT}/teamtalk/ttassert.h
  ${TEAMTALKLIB_ROOT}/teamtalk/User.h
  ${TEAMTALKLIB_ROOT}/teamtalk/server/AcceptHandler.h
  ${TEAMTALKLIB_ROOT}/teamtalk/server/DesktopCache.h
  ${TEAMTALKLIB_ROOT}/teamtalk/server/ServerChannel.h
  ${TEAMTALKLIB_ROOT}/teamtalk/server/Server.h
  ${TEAMTALKLIB_ROOT}/teamtalk/server/ServerNode.h
  ${TEAMTALKLIB_ROOT}/teamtalk/server/ServerUser.h )

set (TTSRVLIB_SOURCES
  ${TEAMTALKLIB_ROOT}/myace/MyACE.cpp
  ${TEAMTALKLIB_ROOT}/myace/TimerHandler.cpp
  ${TEAMTALKLIB_ROOT}/mystd/MyStd.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/Channel.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/CodecCommon.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/Commands.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/Common.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/DesktopSession.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/PacketHandler.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/PacketHelper.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/PacketLayout.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/StreamHandler.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/ttassert.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/User.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/server/AcceptHandler.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/server/DesktopCache.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/server/ServerChannel.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/server/ServerNode.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/server/ServerUser.cpp )
