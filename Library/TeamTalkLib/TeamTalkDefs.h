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

#if !defined(TEAMTALKDEFS_H)
#define TEAMTALKDEFS_H

#define TEAMTALK_VERSION            "5.21.2.5191"
#define TEAMTALK_RC_VERSION         5,21,2,5191
#define TEAMTALK_RC_FILEVERSION     "5, 21, 2, 5191"
#define TEAMTALK_VERSION_POSTFIX    " - Unreleased"

#if defined(ENABLE_TEAMTALKPRO)
#define TEAMTALK_LIB_NAME "TeamTalk5Pro"

#define WEBLOGIN_BEARWARE_USERNAME  "bearware"
#define WEBLOGIN_BEARWARE_POSTFIX   "@bearware.dk"
#define WEBLOGIN_URL                "https://login.bearware.dk/teamtalk/weblogin.php?"
#else
#define TEAMTALK_LIB_NAME "TeamTalk5"
#endif

constexpr auto TT_MAX_ID = 0xFFE;
constexpr auto USER_TIMEOUT = 60; //secs before disconnecting users without keepalive (60 seconds)
constexpr auto MAX_STRING_LENGTH = 511;
constexpr auto MAX_USERS = 1000;
constexpr auto MAX_USERS_IN_CHANNEL = MAX_USERS;
constexpr auto MAX_CHANNELS = TT_MAX_ID;

constexpr auto DEFAULT_ENCRYPTED_TCPPORT = 10443;
constexpr auto DEFAULT_ENCRYPTED_UDPPORT = 10443;
constexpr auto DEFAULT_TCPPORT = 10333;
constexpr auto DEFAULT_UDPPORT = 10333;

// sanity check
#if defined(ENABLE_TEAMTALKPRO)
#if !defined(ENABLE_ENCRYPTION)
#error Encryption not enabled
#endif
#endif

#endif
