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

    connect(ui.applyPositionButton, &QPushButton::clicked, this, &UserVolumeDlg::slotApplyPosition);

    User user;
    if(TT_GetUser(ttInst, m_userid, &user))
    {
        setWindowTitle(windowTitle() + QString(" - ") + getDisplayName(user));
        this->setAccessibleDescription(tr("Volume for %1").arg(getDisplayName(user)));
    }
    ui.voicevolSlider->setValue(refVolumeToPercent(user.nVolumeVoice));
    ui.mfvolSlider->setValue(refVolumeToPercent(user.nVolumeMediaFile));

    ui.voiceleftChkBox->setChecked(!user.stereoPlaybackVoice[0]);
    ui.voicerightChkBox->setChecked(!user.stereoPlaybackVoice[1]);
    ui.mfleftChkBox->setChecked(!user.stereoPlaybackMediaFile[0]);
    ui.mfrightChkBox->setChecked(!user.stereoPlaybackMediaFile[1]);

    bool supports3DPositioning = isOutputDevice3DSupported();

    ui.groupBox_5->setVisible(supports3DPositioning);

    if (supports3DPositioning)
    {
        ui.xCoordSpinBox->setValue(user.soundPositionVoice[0]);
        ui.yCoordSpinBox->setValue(user.soundPositionVoice[1]);
    }
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

    if (isOutputDevice3DSupported())
    {
        ui.xCoordSpinBox->setValue(0.0);
        ui.yCoordSpinBox->setValue(0.0);
        slotApplyPosition();
    }
}

void UserVolumeDlg::slotApplyPosition()
{
    if (!isOutputDevice3DSupported())
        return;

    float x = ui.xCoordSpinBox->value();
    float y = ui.yCoordSpinBox->value();

    TTBOOL result = TT_SetUserPosition(ttInst, m_userid, STREAMTYPE_VOICE, x, y, 0.0);
    if (!result)
    {
        QMessageBox::critical(this, tr("Position"), 
        tr("Failed to set user's 3D position"));
    }
}
