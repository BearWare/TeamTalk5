include (ttlib)
include (tinyxml2)

set (TTSPAMBOT_INCLUDE_DIR
  ${TEAMTALKLIB_ROOT}
  ${TEAMTALKLIB_ROOT}/bin/ttsrv
  ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)

set (TTSPAMBOT_COMPILE_FLAGS
  ${ACE_COMPILE_FLAGS}
  -DENABLE_ENCRYPTION
  -DENABLE_TEAMTALKPRO
  -DBUILD_SPAMBOT)

set (TTSPAMBOT_LINK_FLAGS
  ${ACE_SSL_LINK_FLAGS}
  ${ACE_LINK_FLAGS}
  ${OPENSSL_LINK_FLAGS}
  ${TINYXML2_LINK_FLAGS})

set (TTSPAMBOT_SOURCES
  ${TEAMTALKLIB_ROOT}/myace/MyACE.cpp
  ${TEAMTALKLIB_ROOT}/myace/MyINet.cpp
  ${TEAMTALKLIB_ROOT}/mystd/MyStd.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/TTAssert.cpp
  ${TEAMTALKLIB_ROOT}/settings/Settings.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerUtil.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/Abuse.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/AbuseDB.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/BadWords.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/IPBan.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/SpamBotConfig.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/SpamBotSession.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/SpamBotXML.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/Main.cpp
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/ttspambot.rc)

set (TTSPAMBOT_HEADERS
  ${TEAMTALKLIB_ROOT}/TeamTalkDefs.h
  ${TEAMTALKLIB_ROOT}/myace/MyACE.h
  ${TEAMTALKLIB_ROOT}/myace/MyINet.h
  ${TEAMTALKLIB_ROOT}/mystd/MyStd.h
  ${TEAMTALKLIB_ROOT}/settings/Settings.h
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/AppInfo.h
  ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerUtil.h
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/Abuse.h
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/AbuseDB.h
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/BadWords.h
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/IPBan.h
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/SpamBotConfig.h
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/SpamBotSession.h
  ${TEAMTALKLIB_ROOT}/bin/ttspambot/SpamBotXML.h)
