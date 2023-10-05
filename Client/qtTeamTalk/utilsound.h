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

QStringList initSelectedSoundDevices(SoundDevice& indev, SoundDevice& outdev);
QStringList initDefaultSoundDevices(SoundDevice& indev, SoundDevice& outdev);

void incVolume(int userid, StreamType stream_type);
void decVolume(int userid, StreamType stream_type);
int refVolume(double percent);
int refVolumeToPercent(int volume);
int refGain(double percent);

enum SoundEvent
{
    SOUNDEVENT_NEWUSER,
    SOUNDEVENT_REMOVEUSER,
    SOUNDEVENT_SERVERLOST,
    SOUNDEVENT_USERMSG,
    SOUNDEVENT_USERMSGSENT,
    SOUNDEVENT_CHANNELMSG,
    SOUNDEVENT_CHANNELMSGSENT,
    SOUNDEVENT_BROADCASTMSG,
    SOUNDEVENT_HOTKEY,
    SOUNDEVENT_SILENCE,
    SOUNDEVENT_NEWVIDEO,
    SOUNDEVENT_NEWDESKTOP,
    SOUNDEVENT_FILESUPD,
    SOUNDEVENT_FILETXDONE,
    SOUNDEVENT_QUESTIONMODE,
    SOUNDEVENT_DESKTOPACCESS,
    SOUNDEVENT_USERLOGGEDIN,
    SOUNDEVENT_USERLOGGEDOUT,
    SOUNDEVENT_VOICEACTON,
    SOUNDEVENT_VOICEACTOFF,
    SOUNDEVENT_MUTEALLON,
    SOUNDEVENT_MUTEALLOFF,
    SOUNDEVENT_TRANSMITQUEUE_HEAD,
    SOUNDEVENT_TRANSMITQUEUE_STOP,
    SOUNDEVENT_VOICEACTTRIG,
    SOUNDEVENT_VOICEACTSTOP,
    SOUNDEVENT_VOICEACTMEON,
    SOUNDEVENT_VOICEACTMEOFF,
    SOUNDEVENT_INTERCEPT,
    SOUNDEVENT_INTERCEPTEND,
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

#endif
