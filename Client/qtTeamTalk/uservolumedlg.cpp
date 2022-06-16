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

#include "uservolumedlg.h"
#include "appinfo.h"
#include "utilsound.h"
#include <settings.h>

#include <QMessageBox>
#include <QSettings>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

UserVolumeDlg::UserVolumeDlg(int userid, QWidget * parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
, m_userid(userid)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    ui.voicevolSlider->setRange(0, 100);
    ui.mfvolSlider->setRange(0, 100);

    connect(ui.voicevolSlider, &QAbstractSlider::valueChanged,
            this, &UserVolumeDlg::slotVolumeChanged);
    connect(ui.voiceleftChkBox, &QAbstractButton::clicked, this, &UserVolumeDlg::slotMuteChannel);
    connect(ui.voicerightChkBox, &QAbstractButton::clicked, this, &UserVolumeDlg::slotMuteChannel);

    connect(ui.mfvolSlider, &QAbstractSlider::valueChanged,
            this, &UserVolumeDlg::slotVolumeChanged);
    connect(ui.mfleftChkBox, &QAbstractButton::clicked, this, &UserVolumeDlg::slotMuteChannel);
    connect(ui.mfrightChkBox, &QAbstractButton::clicked, this, &UserVolumeDlg::slotMuteChannel);
    connect(ui.defaultsButton, &QAbstractButton::clicked, this, &UserVolumeDlg::slotDefaults);

    User user;
    if(TT_GetUser(ttInst, m_userid, &user))
        setWindowTitle(windowTitle() + QString(" - ") + getDisplayName(user));
    ui.voicevolSlider->setValue(refVolumeToPercent(user.nVolumeVoice));
    ui.mfvolSlider->setValue(refVolumeToPercent(user.nVolumeMediaFile));

    ui.voiceleftChkBox->setChecked(!user.stereoPlaybackVoice[0]);
    ui.voicerightChkBox->setChecked(!user.stereoPlaybackVoice[1]);
    ui.mfleftChkBox->setChecked(!user.stereoPlaybackMediaFile[0]);
    ui.mfrightChkBox->setChecked(!user.stereoPlaybackMediaFile[1]);
}

void UserVolumeDlg::slotVolumeChanged(int /*vol*/)
{
    TTBOOL b = TRUE;

    double percent = ui.voicevolSlider->value();

    b &= TT_SetUserVolume(ttInst, m_userid, STREAMTYPE_VOICE,
                          refVolume(percent));

    percent = ui.mfvolSlider->value();
    b &= TT_SetUserVolume(ttInst, m_userid, STREAMTYPE_MEDIAFILE_AUDIO,
                          refVolume(percent));

    if(!b)
        QMessageBox::critical(this, tr("Volume"), 
        tr("Failed to change user's volume"));
}

void UserVolumeDlg::slotMuteChannel()
{
    TT_SetUserStereo(ttInst, m_userid,
                     STREAMTYPE_VOICE,
                     !ui.voiceleftChkBox->isChecked(), 
                     !ui.voicerightChkBox->isChecked());
    TT_SetUserStereo(ttInst, m_userid,
                     STREAMTYPE_MEDIAFILE_AUDIO,
                     !ui.mfleftChkBox->isChecked(), 
                     !ui.mfrightChkBox->isChecked());
}

void UserVolumeDlg::slotDefaults()
{
    ui.voicevolSlider->setValue(refVolumeToPercent(SOUND_VOLUME_DEFAULT));
    ui.mfvolSlider->setValue(ttSettings->value(SETTINGS_SOUND_MEDIASTREAM_VOLUME, SETTINGS_SOUND_MEDIASTREAM_VOLUME_DEFAULT).toInt());
    slotVolumeChanged(refVolumeToPercent(SOUND_VOLUME_DEFAULT));
}

