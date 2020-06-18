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

#pragma once

#define COMPANYNAME             _T("BearWare.dk")
#define APPVERSION_SHORT        _T("5.6.0")
#define APPVERSION              _T( TEAMTALK_VERSION ) _T(" - Unreleased")

#ifdef ENABLE_TEAMTALKPRO
#define APPTITLE                _T("TeamTalk 5 Classic Professional v. ") APPVERSION
#define APPNAME                 _T("TeamTalk 5 Classic Professional")
#define APPTITLE_SHORT          _T("TeamTalk5ClassicPro")
#define SETTINGS_FILE           "TeamTalk5ClassicPro.xml"
#define SETTINGS_DEFAULT_FILE   "TeamTalk5ClassicPro.xml.default"
#else
#define APPTITLE                _T("TeamTalk 5 Classic v. ") APPVERSION
#define APPNAME                 _T("TeamTalk 5 Classic")
#define APPTITLE_SHORT          _T("TeamTalk5Classic")
#define SETTINGS_FILE           "TeamTalk5Classic.xml"
#define SETTINGS_DEFAULT_FILE   "TeamTalk5Classic.xml.default"
#endif

#define MANUALFILE              _T("TeamTalk5.chm")
#define WEBSITE                 _T("http://www.bearware.dk")
#define TEAMTALK_INSTALLDIR     _T("BearWare.dk")
#define TTURL                   _T("tt://")
#define TT_XML_ROOTNAME         "teamtalk"
#define TTFILE_EXT              ".tt"

#define URL_PUBLICSERVER        _T("http://www.bearware.dk/teamtalk/tt5servers.php?client=") APPTITLE_SHORT _T("&version=") APPVERSION_SHORT _T("&dllversion=") _T( TEAMTALK_VERSION ) _T("&os=Windows")
#define URL_APPUPDATE           _T("http://www.bearware.dk/teamtalk/tt5update.php?client=")  APPTITLE_SHORT _T("&version=") APPVERSION_SHORT _T("&dllversion=") _T( TEAMTALK_VERSION ) _T("&os=Windows")

#define WEBLOGIN_FACEBOOK_USERNAME          _T("facebook")
#define WEBLOGIN_FACEBOOK_CLIENTID          _T("611833262539381")
#define WEBLOGIN_FACEBOOK_REDIRECT          _T("https://www.facebook.com/connect/login_success.html")
#define WEBLOGIN_FACEBOOK_PASSWDPREFIX      _T("code=")
#define WEBLOGIN_FACEBOOK_USERNAMEPOSTFIX   _T("@facebook.com")
#define WEBLOGIN_FACEBOOK_PROFILE_URL       _T("https://facebook.com/")
#define WEBLOGIN_FACEBOOK_LOGOUT_URL        _T("https://www.facebook.com/logout.php?")
#define WEBLOGIN_FACEBOOK_LOGOUT_REDIRECT   WEBLOGIN_FACEBOOK_REDIRECT

#define WEBLOGIN_BEARWARE_USERNAME              "bearware"
#define WEBLOGIN_BEARWARE_USERNAMEPOSTFIX       "@bearware.dk"
#define WEBLOGIN_URL                            _T("https://www.bearware.dk/teamtalk/weblogin.php?client=") APPTITLE_SHORT _T("&version=") APPVERSION_SHORT _T("&dllversion=") _T( TEAMTALK_VERSION ) _T("&os=Windows")
#define WEBLOGIN_BEARWARE_URLAUTH(uid, passwd)  WEBLOGIN_URL _T("&service=bearware&action=auth&username=") + CString(uid) + _T("&password=") + CString(passwd)
#define WEBLOGIN_BEARWARE_URLTOKEN(uid, token, accesstoken)  WEBLOGIN_URL _T("&service=bearware&action=clientauth&username=") + CString(uid) + _T("&token=") + CString(token) + _T("&accesstoken=") + CString(accesstoken)
