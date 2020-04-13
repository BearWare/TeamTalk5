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

#ifndef APPINFO_H
#define APPINFO_H

#define COMPANYNAME         "BearWare.dk"
#define APPVERSION_SHORT    "5.5.0"
#define APPVERSION          "5.5" ""

#define APPICON             QString::fromUtf8(":/images/images/teamtalk.png")
#define APPTRAYICON         QString::fromUtf8(":/images/images/teamtalky.png")
#define APPTRAYICON_CON     QString::fromUtf8(":/images/images/teamtalk.png")
#define APPTRAYICON_ACTIVE  QString::fromUtf8(":/images/images/teamtalkg.png")
#define APPWEBSITE          "http://www.bearware.dk"

#ifdef ENABLE_TEAMTALKPRO
#define APPTITLE            "TeamTalk Professional v. " APPVERSION
#define APPNAME_SHORT       "TeamTalk5Pro"
#else
#define APPTITLE            "TeamTalk v. " APPVERSION
#define APPNAME_SHORT       "TeamTalk5"
#endif

#define APPINIFILE          (QApplication::applicationName() + ".ini")

#if defined(Q_OS_DARWIN)
#define APPDEFAULTINIFILE   (QApplication::applicationDirPath() + "/../Resources/Config/" + (QApplication::applicationName() + ".ini"))
#else
#define APPDEFAULTINIFILE   (QApplication::applicationDirPath() + "/" + QApplication::applicationName() + ".ini.default")
#endif

#if defined(Q_OS_DARWIN)
#define APPMANUAL           (QApplication::applicationDirPath() + "/../Resources/Help/index.html")
#define TRANSLATE_FOLDER    (QApplication::applicationDirPath() + "/../Resources/Languages")
#elif defined(Q_OS_WIN32)
#define APPMANUAL           (QApplication::applicationDirPath() + "/TeamTalk5.chm")
#define TRANSLATE_FOLDER    (QApplication::applicationDirPath() + "/" "languages")
#else
#define APPMANUAL           (QApplication::applicationDirPath() + "/help/index.html")
#define TRANSLATE_FOLDER    (QApplication::applicationDirPath() + "/" "languages")
#endif

#if defined(Q_OS_WIN32)
#define OSTYPE "Windows"
#elif defined(Q_OS_DARWIN)
#define OSTYPE "Mac"
#elif defined(Q_OS_LINUX)
#define OSTYPE "Linux"
#endif

#define URL_FREESERVER      ("http://www.bearware.dk/teamtalk/tt5servers.php?client=" APPNAME_SHORT "&version=" APPVERSION_SHORT "&dllversion=" TEAMTALK_VERSION "&os=" OSTYPE)
#define URL_APPUPDATE       ("http://www.bearware.dk/teamtalk/tt5update.php?client=" APPNAME_SHORT "&version=" APPVERSION_SHORT "&dllversion=" TEAMTALK_VERSION "&os=" OSTYPE)

#define TTFILE_EXT          ".tt"
#define TTFILE_ROOT         "teamtalk"
#define TTFILE_VERSION      "5.0"
#define TTLINK_PREFIX       "tt:"

#define WEBLOGIN_FACEBOOK_USERNAME              "facebook"
#define WEBLOGIN_FACEBOOK_CLIENTID              "611833262539381"
#define WEBLOGIN_FACEBOOK_REDIRECT              "https://www.facebook.com/connect/login_success.html"
#define WEBLOGIN_FACEBOOK_URL                   ("https://www.facebook.com/v2.10/dialog/oauth?client_id=" WEBLOGIN_FACEBOOK_CLIENTID \
                                                "&redirect_uri=" WEBLOGIN_FACEBOOK_REDIRECT "&response_type=code%20token")
#define WEBLOGIN_FACEBOOK_PASSWDPREFIX          "code="
#define WEBLOGIN_FACEBOOK_USERNAMEPOSTFIX       "@facebook.com"
#define WEBLOGIN_FACEBOOK_PROFILE_URL           "https://facebook.com/"
#define WEBLOGIN_FACEBOOK_LOGOUT_URL            "https://www.facebook.com/logout.php?"
#define WEBLOGIN_FACEBOOK_LOGOUT_REDIRECT       WEBLOGIN_FACEBOOK_REDIRECT

#define WEBLOGIN_URL                            "https://www.bearware.dk/teamtalk/weblogin.php?client=" APPNAME_SHORT \
                                                "&version=" APPVERSION_SHORT "&dllversion=" TEAMTALK_VERSION "&os=" OSTYPE
#define WEBLOGIN_BEARWARE_URLAUTH(uid, passwd)  QString(WEBLOGIN_URL "&service=bearware&action=auth&username=%1&password=%2").arg(uid).arg(passwd)
#define WEBLOGIN_BEARWARE_URLTOKEN(uid, token, accesstoken)  QString(WEBLOGIN_URL "&service=bearware&action=clientauth&username=%1&token=%2&accesstoken=%3").arg(uid).arg(token).arg(accesstoken)
#define WEBLOGIN_BEARWARE_USERNAME              "bearware"
#define WEBLOGIN_BEARWARE_USERNAMEPOSTFIX       "@bearware.dk"

#if QT_VERSION >= 0x050000
#if defined(Q_OS_WIN32)
#define QT_DEFAULT_DIALOG_HINTS (Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
#else
#define QT_DEFAULT_DIALOG_HINTS (Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
#endif
#else
#define QT_DEFAULT_DIALOG_HINTS (Qt::Dialog | Qt::WindowTitleHint)
#endif //QT_VERSION

#endif

