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

#ifndef COMMON_H
#define COMMON_H

#include "utiltt.h"

#include <QList>
#include <QFile>
#include <QDateTime>
#include <QMap>
#include <QVector>

//For TT_DoChangeStatus
enum StatusMode
{
    STATUSMODE_AVAILABLE        = 0x00000000,
    STATUSMODE_AWAY             = 0x00000001, 
    STATUSMODE_QUESTION         = 0x00000002,
    STATUSMODE_MODE             = 0x000000FF,

    STATUSMODE_FLAGS            = 0xFFFFFF00,
    STATUSMODE_MALE             = 0x00000000,
    STATUSMODE_FEMALE           = 0x00000100,
    STATUSMODE_NEUTRAL          = 0x00001000,
    STATUSMODE_VIDEOTX          = 0x00000200,
    STATUSMODE_DESKTOP          = 0x00000400,
    STATUSMODE_STREAM_MEDIAFILE = 0x00000800,
    STATUSMODE_STREAM_MEDIAFILE_PAUSED = 0x00002000,
    STATUSMODE_GENDER_MASK = (STATUSMODE_MALE | STATUSMODE_FEMALE | STATUSMODE_NEUTRAL),
};

enum Gender
{
    GENDER_NONE               = 0,
    GENDER_MALE               = 1,
    GENDER_FEMALE             = 2,
    GENDER_NEUTRAL            = 3
};

typedef std::vector<INT32> hotkey_t;

enum AudioStorageMode
{
    AUDIOSTORAGE_NONE               = 0x0,
    AUDIOSTORAGE_SINGLEFILE         = 0x1,
    AUDIOSTORAGE_SEPARATEFILES      = 0x2,

    AUDIOSTORAGE_BOTH               = AUDIOSTORAGE_SINGLEFILE | AUDIOSTORAGE_SEPARATEFILES,
};

enum DesktopShareMode
{
    DESKTOPSHARE_NONE,
    DESKTOPSHARE_DESKTOP,
    DESKTOPSHARE_ACTIVE_WINDOW,
    DESKTOPSHARE_SPECIFIC_WINDOW,
};

struct HostEncryption
{
    QString cacertdata;
    QString certdata;
    QString privkeydata;
    bool verifypeer = false;
};

struct HostEntry
{
    QString name;
    bool latesthost = false;
    QString ipaddr;
    int tcpport = DEFAULT_TCPPORT;
    int udpport = DEFAULT_UDPPORT;
    bool encrypted = false;
    HostEncryption encryption;

    QString username;
    QString password;
    QString nickname;
    bool lastChan = false;
    QString channel;
    QString chanpasswd;
    //tt-file specific
    Gender gender = GENDER_NONE;
    hotkey_t hotkey;
    int voiceact = -1;
    VideoFormat capformat = {};
    VideoCodec vidcodec = {};

    // doesn't include 'name'
    bool sameHost(const HostEntry& host, bool nickcheck = true) const;
    // same as sameHost() but also host.name == name
    bool sameHostEntry(const HostEntry& host) const;
    QString generateEntryName() const;
};

struct DesktopAccessEntry
{
    QString ipaddr;
    int tcpport;
    QStringList channels;
    QStringList usernames;
    DesktopAccessEntry()
    : tcpport(0) { }
};

struct UserCached
{
    bool valid = false;
    Subscriptions subscriptions = SUBSCRIBE_NONE;
    bool voiceMute = false, mediaMute = false;
    int voiceVolume = SOUND_VOLUME_DEFAULT, mediaVolume = SOUND_VOLUME_DEFAULT;
    bool voiceLeftSpeaker = true, voiceRightSpeaker = true,
        mediaLeftSpeaker = true, mediaRightSpeaker = true;
    UserCached() {}
    UserCached(const User& user);
    void sync(TTInstance* ttInst, const User& user);
};

//internal TeamTalk 5 commands. Use with TT_DoTextMessage() and 
//message type MSGTYPE_CUSTOM
#define TT_INTCMD_TYPING_TEXT         "typing"
#define TT_INTCMD_DESKTOP_ACCESS      "desktopaccess"

struct MyTextMessage : TextMessage
{
    QDateTime receiveTime;
    QString moreMessage;
    MyTextMessage() : TextMessage()
    {
        this->szFromUsername[0] = '\0';
        this->szMessage[0] = '\0';
        receiveTime = QDateTime::currentDateTime();
    }

    explicit MyTextMessage(const TextMessage& msg)
        : MyTextMessage()
    {
        this->nChannelID = msg.nChannelID;
        this->nFromUserID = msg.nFromUserID;
        this->nMsgType = msg.nMsgType;
        this->nToUserID = msg.nToUserID;
        this->bMore = msg.bMore;
#if defined(Q_OS_WIN32)
        wcsncpy(this->szFromUsername, msg.szFromUsername, TT_STRLEN);
        wcsncpy(this->szMessage, msg.szMessage, TT_STRLEN);
#else
        strncpy(this->szFromUsername, msg.szFromUsername, TT_STRLEN);
        strncpy(this->szMessage, msg.szMessage, TT_STRLEN);
#endif
        moreMessage = _Q(this->szMessage);
    }
};

typedef QList<MyTextMessage> textmessages_t;

QString makeCustomCommand(const QString& cmd, const QString& value);
QStringList getCustomCommand(const MyTextMessage& msg);

bool setupEncryption(const HostEntry& host);

void addLatestHost(const HostEntry& host);

//server entries in settings file
void addServerEntry(const HostEntry& host);
void setServerEntry(int index, const HostEntry& host);
bool getServerEntry(int index, HostEntry& host, bool latesthost);
void deleteServerEntry(const HostEntry& host);

//get desktop access list
void addDesktopAccessEntry(const DesktopAccessEntry& entry);
void getDesktopAccessList(QVector<DesktopAccessEntry>& entries);
void getDesktopAccessList(QVector<DesktopAccessEntry>& entries,
                          const QString& ipaddr, int tcpport);
bool hasDesktopAccess(const QVector<DesktopAccessEntry>& entries,
                      const User& user);
void deleteDesktopAccessEntries();

AudioPreprocessor loadAudioPreprocessor(AudioPreprocessorType preprocessortype);
void saveAudioPreprocessor(const AudioPreprocessor& preprocess);

bool isWebLogin(const QString& username, bool includeParentLoginName);
QString userCacheID(const User& user);

bool versionSameOrLater(const QString& check, const QString& against);
QString getVersion(const User& user);

QString generateAudioStorageFilename(AudioFileFormat aff);

QString getDateTimeStamp();
QString generateLogFileName(const QString& name);
bool openLogFile(QFile& file, const QString& folder, const QString& name);
bool writeLogEntry(QFile& file, const QString& line);

#endif
