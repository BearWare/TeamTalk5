/*
 * Copyright (C) 2023, Bjørn D. Rasmussen, BearWare.dk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef APPINFO_H
#define APPINFO_H

#include "utiltt.h"

#include <QString>
#include <QIcon>
#include <QApplication>
#include <QDialog>

#define COMPANYNAME         "BearWare.dk"
#define APPVERSION_SHORT    "5.15.2"
#define APPVERSION_POSTFIX  " - Unreleased"
#define APPVERSION          TEAMTALK_VERSION APPVERSION_POSTFIX

#define APPICON             QString::fromUtf8(":/images/images/teamtalk.png")
#define APPTRAYICON         QString::fromUtf8(":/images/images/teamtalky.png")
#define APPTRAYICON_CON     QString::fromUtf8(":/images/images/teamtalk.png")
#define APPTRAYICON_ACTIVE  QString::fromUtf8(":/images/images/teamtalkg.png")
#define APPWEBSITE          "http://www.bearware.dk"

#define APPTITLE            "TeamTalk v. " APPVERSION
#define APPNAME_SHORT       "TeamTalk5"

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

#define URL_FREESERVER(official, pub, unofficial) QString("http://www.bearware.dk/teamtalk/tt5servers.php?client=" APPNAME_SHORT "&version=" APPVERSION_SHORT "&dllversion=" TEAMTALK_VERSION "&os=" OSTYPE "&official=%1&unofficial=%2").arg(official ? "1" : "0").arg(unofficial ? "1" : "0")
#define URL_PUBLISHSERVER(uid, token) QString("https://www.bearware.dk/teamtalk/tt5servers.php?client=" APPNAME_SHORT "&version=" APPVERSION_SHORT "&dllversion=" TEAMTALK_VERSION "&os=" OSTYPE "&action=publish&username=%1&token=%2").arg(uid).arg(token)
#define URL_APPUPDATE(beta)       QString("http://www.bearware.dk/teamtalk/tt5update.php?client=" APPNAME_SHORT "&version=" APPVERSION_SHORT "&dllversion=" TEAMTALK_VERSION "&os=" OSTYPE "&beta=%1").arg(beta ? "1" : "0")

#define TTFILE_EXT          ".tt"
#define TTFILE_ROOT         "teamtalk"
#define TTFILE_VERSION      "5.0"
#define TTLINK_PREFIX       "tt:"

#define WEBLOGIN_URL                            "https://www.bearware.dk/teamtalk/weblogin.php?client=" APPNAME_SHORT \
                                                "&version=" APPVERSION_SHORT "&dllversion=" TEAMTALK_VERSION "&os=" OSTYPE
#define WEBLOGIN_BEARWARE_URLAUTH(uid, passwd)  QString(WEBLOGIN_URL "&service=bearware&action=auth&username=%1&password=%2").arg(uid).arg(passwd)
#define WEBLOGIN_BEARWARE_URLTOKEN(uid, token, accesstoken)  QString(WEBLOGIN_URL "&service=bearware&action=clientauth&username=%1&token=%2&accesstoken=%3").arg(uid).arg(token).arg(accesstoken)
#define WEBLOGIN_BEARWARE_USERNAME              "bearware"
#define WEBLOGIN_BEARWARE_USERNAMEPOSTFIX       "@bearware.dk"

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#if defined(Q_OS_WIN32)
#define QT_DEFAULT_DIALOG_HINTS (Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
#else
#define QT_DEFAULT_DIALOG_HINTS (Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
#endif
#else
#define QT_DEFAULT_DIALOG_HINTS (Qt::Dialog | Qt::WindowTitleHint)
#endif //QT_VERSION

#endif

