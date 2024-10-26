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

#include "audiopreprocessordlg.h"
#include <QPushButton>

AudioPreprocessorDlg::AudioPreprocessorDlg(AudioPreprocessor preprocess, QWidget* parent)
    : QDialog(parent)
    , m_preprocess(preprocess)
{
    ui.setupUi(this);
    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    connect(this, &QDialog::accepted, this, &AudioPreprocessorDlg::slotAccepted);
    connect(ui.ttdefaultButton, &QAbstractButton::clicked, this, &AudioPreprocessorDlg::slotDefaultTTPreprocessor);
    connect(ui.spxDefaultButton, &QAbstractButton::clicked, this, &AudioPreprocessorDlg::slotDefaultSpeexDSP);

    // TeamTalk audio preprocessor
    ui.gainlevelSlider->setRange(SOUND_GAIN_MIN, SOUND_GAIN_MAX);

    // SpeexDSP audio preprocessor
    ui.gainlevelSpinBox->setRange(SPEEXDSP_AGC_GAINLEVEL_MIN, SPEEXDSP_AGC_GAINLEVEL_MAX);
    ui.maxdecSpinBox->setRange(-100, 0);
    ui.maxdenoiseSpinBox->setRange(-100, 0);

    showSettings();
}

void AudioPreprocessorDlg::showSettings()
{
    switch(m_preprocess.nPreprocessor)
    {
    case NO_AUDIOPREPROCESSOR :
        ui.stackedWidget->setCurrentIndex(0);
        setWindowTitle(tr("No Audio Preprocessor"));
        break;
    case SPEEXDSP_AUDIOPREPROCESSOR :
        ui.stackedWidget->setCurrentIndex(1);
        ui.agcCheckBox->setChecked(m_preprocess.speexdsp.bEnableAGC);
        ui.gainlevelSpinBox->setValue(m_preprocess.speexdsp.nGainLevel);
        ui.maxgainSpinBox->setValue(m_preprocess.speexdsp.nMaxGainDB);
        ui.maxincSpinBox->setValue(m_preprocess.speexdsp.nMaxIncDBSec);
        ui.maxdecSpinBox->setValue(m_preprocess.speexdsp.nMaxDecDBSec);
        ui.denoiseCheckBox->setChecked(m_preprocess.speexdsp.bEnableDenoise);
        ui.maxdenoiseSpinBox->setValue(m_preprocess.speexdsp.nMaxNoiseSuppressDB);
        setWindowTitle(tr("Speex DSP Audio Preprocessor"));
        break;
    case TEAMTALK_AUDIOPREPROCESSOR :
        ui.stackedWidget->setCurrentIndex(2);
        ui.gainlevelSlider->setValue(m_preprocess.ttpreprocessor.nGainLevel);
        ui.muteleftCheckBox->setChecked(m_preprocess.ttpreprocessor.bMuteLeftSpeaker);
        ui.muteRightCheckBox->setChecked(m_preprocess.ttpreprocessor.bMuteRightSpeaker);
        setWindowTitle(tr("TeamTalk Audio Preprocessor"));
        break;
    case WEBRTC_AUDIOPREPROCESSOR :
        break;
    }
}

void AudioPreprocessorDlg::slotDefaultTTPreprocessor(bool)
{
    m_preprocess = initDefaultAudioPreprocessor(TEAMTALK_AUDIOPREPROCESSOR);
    showSettings();
}

void AudioPreprocessorDlg::slotDefaultSpeexDSP(bool)
{
    m_preprocess = initDefaultAudioPreprocessor(SPEEXDSP_AUDIOPREPROCESSOR);
    showSettings();
}

void AudioPreprocessorDlg::slotAccepted()
{
    switch(m_preprocess.nPreprocessor)
    {
    case NO_AUDIOPREPROCESSOR :
        break;
    case SPEEXDSP_AUDIOPREPROCESSOR :
        m_preprocess.speexdsp.bEnableAGC = ui.agcCheckBox->isChecked();
        m_preprocess.speexdsp.nGainLevel = ui.gainlevelSpinBox->value();
        m_preprocess.speexdsp.nMaxGainDB = ui.maxgainSpinBox->value();
        m_preprocess.speexdsp.nMaxIncDBSec = ui.maxincSpinBox->value();
        m_preprocess.speexdsp.nMaxDecDBSec = ui.maxdecSpinBox->value();
        m_preprocess.speexdsp.bEnableDenoise = ui.denoiseCheckBox->isChecked();
        m_preprocess.speexdsp.nMaxNoiseSuppressDB = ui.maxdenoiseSpinBox->value();
        break;
    case TEAMTALK_AUDIOPREPROCESSOR :
        m_preprocess.ttpreprocessor.nGainLevel = ui.gainlevelSlider->value();
        m_preprocess.ttpreprocessor.bMuteLeftSpeaker = ui.muteleftCheckBox->isChecked();
        m_preprocess.ttpreprocessor.bMuteRightSpeaker = ui.muteRightCheckBox->isChecked();
        break;
    case WEBRTC_AUDIOPREPROCESSOR :
        break;
    }
}
