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

#include "audiopreprocessordlg.h"

AudioPreprocessorDlg::AudioPreprocessorDlg(AudioPreprocessor preprocess, QWidget* parent)
    : QDialog(parent)
    , m_preprocess(preprocess)
{
    ui.setupUi(this);

    connect(this, &QDialog::accepted, this, &AudioPreprocessorDlg::slotAccepted);
    connect(ui.ttdefaultButton, &QAbstractButton::clicked, this, &AudioPreprocessorDlg::slotDefaultTTPreprocessor);
    connect(ui.spxDefaultButton, &QAbstractButton::clicked, this, &AudioPreprocessorDlg::slotDefaultSpeexDSP);

    // TeamTalk audio preprocessor
    ui.gainlevelSlider->setRange(SOUND_GAIN_MIN, SOUND_GAIN_MAX);

    // SpeexDSP audio preprocessor
    ui.gainlevelSpinBox->setRange(0, 0x7FFF);
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
    }
}

void AudioPreprocessorDlg::slotDefaultTTPreprocessor(bool)
{
    m_preprocess.nPreprocessor = TEAMTALK_AUDIOPREPROCESSOR;
    initDefaultAudioPreprocessor(m_preprocess);
    showSettings();
}

void AudioPreprocessorDlg::slotDefaultSpeexDSP(bool)
{
    m_preprocess.nPreprocessor = SPEEXDSP_AUDIOPREPROCESSOR;
    initDefaultAudioPreprocessor(m_preprocess);
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
    }
}
