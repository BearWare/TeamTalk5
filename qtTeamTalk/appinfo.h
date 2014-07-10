/*
 * Copyright (c) 2005-2013, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Bedelundvej 79
 * DK-9830 Taars
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 4 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#ifndef APPINFO_H
#define APPINFO_H

#define COMPANYNAME         "BearWare.dk"
#define APPVERSION          "5.0 - TEST VERSION " TEAMTALK_VERSION
#define APPVERSION_SHORT    "5.0"
#define APPICON             QString::fromUtf8(":/images/images/teamtalk.png")
#define APPTRAYICON         QString::fromUtf8(":/images/images/teamtalky.png")
#define APPTRAYICON_CON     QString::fromUtf8(":/images/images/teamtalk.png")
#define APPTRAYICON_ACTIVE  QString::fromUtf8(":/images/images/teamtalkg.png")
#define APPWEBSITE          "http://www.bearware.dk"

#ifdef ENABLE_ENCRYPTION
#define APPTITLE            "TeamTalk Professional v. " APPVERSION
#define APPNAME_SHORT       "TeamTalk5Pro"
#else
#define APPTITLE            "TeamTalk v. " APPVERSION
#define APPNAME_SHORT       "TeamTalk5"
#endif

#define APPINIFILE          (QApplication::applicationName() + ".ini")

#if defined(Q_OS_DARWIN)
#define APPMANUAL           (QApplication::applicationDirPath() + "/../Resources/Help/toc.html")
#define TRANSLATE_FOLDER    (QApplication::applicationDirPath() + "/../Resources/Languages")
#else
#define APPMANUAL           (QApplication::applicationDirPath() + "/TeamTalk5.chm")
#define TRANSLATE_FOLDER    (QApplication::applicationDirPath() + "/" "languages")
#endif

#if defined(Q_OS_WIN32)
#define MP3ENCDLL_FILENAME "lame_enc.dll" 
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
#define TTLINK_PREFIX      "tt://"

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

