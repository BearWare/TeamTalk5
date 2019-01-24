/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#if defined(ENABLE_TEAMTALKPRO)

#define TEAMTALK_SETTINGSFILE "tt5prosrv.xml"
#define TEAMTALK_LOGFILE "tt5prosrv.log"

#if defined(BUILD_NT_SERVICE)
#define TEAMTALK_NAME "TeamTalk Professional NT Service"
#define TEAMTALK_EXE "tt5prosvc"
#else
#define TEAMTALK_NAME "TeamTalk Professional Server"
#define TEAMTALK_EXE "tt5prosrv"
#endif /* BUILD_NT_SERVICE */

#define TEAMTALK_DESCRIPTION "TeamTalk Professional Conferencing Server"

#else

#define TEAMTALK_SETTINGSFILE "tt5srv.xml"
#define TEAMTALK_LOGFILE "tt5srv.log"

#if defined(BUILD_NT_SERVICE)

#define TEAMTALK_NAME "TeamTalk NT Service"
#define TEAMTALK_EXE "tt5svc"

#else

#define TEAMTALK_NAME "TeamTalk Server"
#define TEAMTALK_EXE "tt5srv"

#endif /* BUILD_NT_SERVICE */

#define TEAMTALK_DESCRIPTION "TeamTalk Conferencing Server"

#endif /* ENABLE_TEAMTALKPRO */

#define TEAMTALK_VERSION_FRIENDLY TEAMTALK_VERSION TEAMTALK_VERSION_POSTFIX

#define TEAMTALK_XML_ROOTNAME "teamtalk"

#define WEBLOGIN_FACEBOOK               "facebook"
