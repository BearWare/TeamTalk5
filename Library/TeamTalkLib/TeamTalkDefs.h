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

#define TEAMTALK_VERSION            "5.6.0.5000"
#define TEAMTALK_RC_VERSION         5,6,0,5000
#define TEAMTALK_RC_FILEVERSION     "5, 6, 0, 5000"
#define TEAMTALK_VERSION_POSTFIX    " - Unreleased"

#if defined(ENABLE_TEAMTALKPRO)
#define TEAMTALK_LIB_NAME "TeamTalk5Pro"
#else
#define TEAMTALK_LIB_NAME "TeamTalk5"
#endif

#define TT_MAX_ID 0xFFE
#define USER_TIMEOUT 60 //secs before disconnecting users without keepalive (60 seconds)
#define MAX_STRING_LENGTH 511
#define MAX_USERS 1000
#define MAX_USERS_IN_CHANNEL MAX_USERS
#define MAX_CHANNELS TT_MAX_ID

#define WEBLOGIN_FACEBOOK_USERNAME  "facebook"
#define WEBLOGIN_FACEBOOK_POSTFIX   "@facebook.com"

#define WEBLOGIN_BEARWARE_USERNAME  "bearware"
#define WEBLOGIN_BEARWARE_POSTFIX   "@bearware.dk"
#define WEBLOGIN_URL                "https://login.bearware.dk/teamtalk/weblogin.php?"

// sanity check
#if defined(ENABLE_TEAMTALKPRO)
#if !defined(ENABLE_ENCRYPTION)
#error Encryption not enabled
#endif
#endif

#endif
