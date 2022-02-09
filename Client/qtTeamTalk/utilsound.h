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

#ifndef UTILSOUND_H
#define UTILSOUND_H

#include "common.h"

#include <QVector>
#include <QString>
#include <QStringList>

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
};

void playSoundEvent(SoundEvent event);
void resetDefaultSoundsPack();

#endif
