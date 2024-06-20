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

#ifndef UTILSOUND_H
#define UTILSOUND_H

#include "common.h"

#include <QQueue>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>
#if defined(QT_MULTIMEDIA_LIB) && QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#include <QAudioDevice>
#endif

QVector<SoundDevice> getSoundDevices();
bool getSoundDevice(int deviceid, const QVector<SoundDevice>& devs, SoundDevice& dev);
bool getSoundDevice(const QString& devid, bool input, const QVector<SoundDevice>& devs, SoundDevice& dev);
int getSoundDuplexSampleRate(const SoundDevice& indev, const SoundDevice& outdev);
bool isSoundDeviceEchoCapable(const SoundDevice& indev, const SoundDevice& outdev);

int getDefaultSndInputDevice();
int getDefaultSndOutputDevice();

int getSoundInputFromUID(int inputid, const QString& uid);
int getSoundOutputFromUID(int outputid, const QString& uid);

int getSelectedSndInputDevice();
int getSelectedSndOutputDevice();

#if defined(QT_MULTIMEDIA_LIB) && QT_VERSION >= QT_VERSION_CHECK(6,0,0)
QAudioDevice getSelectedOutputAudioDevice();
#endif

QStringList initSelectedSoundDevices(SoundDevice& indev, SoundDevice& outdev);
QStringList initDefaultSoundDevices(SoundDevice& indev, SoundDevice& outdev);

void incVolume(int userid, StreamType stream_type);
void decVolume(int userid, StreamType stream_type);
int refVolume(double percent);
int refVolumeToPercent(int volume);
int refGain(double percent);

enum SoundEvent : qulonglong
{
    SOUNDEVENT_NONE = 0x0,
    SOUNDEVENT_NEWUSER = qulonglong(1) << 0,
    SOUNDEVENT_REMOVEUSER = qulonglong(1) << 1,
    SOUNDEVENT_SERVERLOST = qulonglong(1) << 2,
    SOUNDEVENT_USERMSG = qulonglong(1) << 3,
    SOUNDEVENT_USERMSGSENT = qulonglong(1) << 4,
    SOUNDEVENT_CHANNELMSG = qulonglong(1) << 5,
    SOUNDEVENT_CHANNELMSGSENT = qulonglong(1) << 6,
    SOUNDEVENT_BROADCASTMSG = qulonglong(1) << 7,
    SOUNDEVENT_HOTKEY = qulonglong(1) << 8,
    SOUNDEVENT_SILENCE = qulonglong(1) << 9,
    SOUNDEVENT_NEWVIDEO = qulonglong(1) << 10,
    SOUNDEVENT_NEWDESKTOP = qulonglong(1) << 11,
    SOUNDEVENT_FILESUPD = qulonglong(1) << 12,
    SOUNDEVENT_FILETXDONE = qulonglong(1) << 13,
    SOUNDEVENT_QUESTIONMODE = qulonglong(1) << 14,
    SOUNDEVENT_DESKTOPACCESS = qulonglong(1) << 15,
    SOUNDEVENT_USERLOGGEDIN = qulonglong(1) << 16,
    SOUNDEVENT_USERLOGGEDOUT = qulonglong(1) << 17,
    SOUNDEVENT_VOICEACTON = qulonglong(1) << 18,
    SOUNDEVENT_VOICEACTOFF = qulonglong(1) << 19,
    SOUNDEVENT_MUTEALLON = qulonglong(1) << 20,
    SOUNDEVENT_MUTEALLOFF = qulonglong(1) << 21,
    SOUNDEVENT_TRANSMITQUEUE_HEAD = qulonglong(1) << 22,
    SOUNDEVENT_TRANSMITQUEUE_STOP = qulonglong(1) << 23,
    SOUNDEVENT_VOICEACTTRIG = qulonglong(1) << 24,
    SOUNDEVENT_VOICEACTSTOP = qulonglong(1) << 25,
    SOUNDEVENT_VOICEACTMEON = qulonglong(1) << 26,
    SOUNDEVENT_VOICEACTMEOFF = qulonglong(1) << 27,
    SOUNDEVENT_INTERCEPT = qulonglong(1) << 28,
    SOUNDEVENT_INTERCEPTEND = qulonglong(1) << 29,
    SOUNDEVENT_TYPING = qulonglong(1) << 30,
    SOUNDEVENT_NEXT_UNUSED = qulonglong(1) << 31,
};

typedef qulonglong SoundEvents;

struct SoundEventInfo
{
    QString settingKey;
    QString defaultFileName;
};

enum PlaybackMode
{
    PLAYBACKMODE_NONE           = 0x0,
    PLAYBACKMODE_DEFAULT        = 0x1,

    PLAYBACKMODE_TEAMTALK       = 0x80000,
    PLAYBACKMODE_ONEBYONE       = PLAYBACKMODE_TEAMTALK | 0x1,
    PLAYBACKMODE_OVERLAPPING    = PLAYBACKMODE_TEAMTALK | 0x2,
};

class PlaySoundEvent : public QObject
{
public:
    PlaySoundEvent(QObject* parent);
    void queueSoundEvent(SoundEvent event);
    void playbackUpdate(int playbackid, const MediaFileInfo& mfi);
private:
    void playSoundEvent();
    bool playTeamTalkSoundEvent(const QString& filename);
    void playDefaultSoundEvent(const QString& filename);
    QQueue<SoundEvent> m_playbackQueue;
    QSet<int> m_activeSessions;
};

void playSoundEvent(SoundEvent event);
void resetDefaultSoundsPack();
QString getSoundEventFilename(SoundEvent event);

class UtilSound : public QObject
{
    Q_OBJECT

public:
    static QHash<SoundEvents, SoundEventInfo> eventToSettingMap();
    static QString getDefaultFile(const QString& paramKey);
    static QString getFile(const QString& paramKey);
};

#endif
