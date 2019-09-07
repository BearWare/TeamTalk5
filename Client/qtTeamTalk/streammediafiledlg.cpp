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

#include "streammediafiledlg.h"
#include "appinfo.h"
#include "settings.h"
#include "audiopreprocessordlg.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QComboBox>

extern QSettings* ttSettings;
extern TTInstance* ttInst;

#define MAX_MEDIAFILES 10

StreamMediaFileDlg::StreamMediaFileDlg(QWidget* parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    connect(this, &QDialog::accepted, this, &StreamMediaFileDlg::slotAccepted);
    connect(ui.toolButton, &QAbstractButton::clicked, this, &StreamMediaFileDlg::slotSelectFile);
    connect(ui.refreshBtn, &QAbstractButton::clicked, this, &StreamMediaFileDlg::showMediaFormatInfo);
    //connect(ui.vidcodecBox, &QComboBox::currentIndexChanged, ui.vidcodecStackedWidget, &QStackedWidget::setCurrentIndex);
    connect(ui.stopToolButton, &QAbstractButton::clicked, this, &StreamMediaFileDlg::slotStopMediaFile);
    connect(ui.startToolButton, &QAbstractButton::clicked, this, &StreamMediaFileDlg::slotPlayMediaFile);
    connect(ui.mediafileComboBox->lineEdit(), &QLineEdit::editingFinished, this, &StreamMediaFileDlg::showMediaFormatInfo);
    connect(ui.mediafileComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(slotSelectionFile(const QString&)));
    connect(ui.preprocessorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChangePreprocessor(int)));
    connect(ui.preprocessButton, &QAbstractButton::clicked, this, &StreamMediaFileDlg::slotSetupPreprocessor);
    connect(ui.playbackOffsetSlider, &QSlider::sliderMoved, this, &StreamMediaFileDlg::slotChangePlayOffset);
    connect(ui.playbackOffsetSlider, &QSlider::valueChanged, this, &StreamMediaFileDlg::slotChangePlayOffset);

    int i = 0;
    QString item;
    while ((item = ttSettings->value(QString(SETTINGS_STREAMMEDIA_FILENAME).arg(i++)).toString()).size())
    {
        ui.mediafileComboBox->addItem(item);
    }
    ui.mediafileComboBox->setCurrentIndex(0);
    ui.preprocessorComboBox->addItem("No Audio Preprocessor", NO_AUDIOPREPROCESSOR);
    ui.preprocessorComboBox->addItem("TeamTalk Audio Preprocessor", TEAMTALK_AUDIOPREPROCESSOR);
    ui.preprocessorComboBox->addItem("Speex DSP Audio Preprocessor", SPEEXDSP_AUDIOPREPROCESSOR);

    ui.playbackOffsetSlider->setMaximum(10000);

    int vidcodec = ttSettings->value(SETTINGS_STREAMMEDIA_CODEC, DEFAULT_VIDEO_CODEC).toInt();
    ui.vidcodecBox->addItem("WebM VP8", WEBM_VP8_CODEC);
    ui.vp8bitrateSpinBox->setValue(ttSettings->value(SETTINGS_STREAMMEDIA_WEBMVP8_BITRATE,
                                                     DEFAULT_WEBMVP8_BITRATE).toInt());
    int vidindex = ui.vidcodecBox->findData(vidcodec);
    ui.vidcodecBox->setCurrentIndex(vidindex);

    showMediaFormatInfo();
}

StreamMediaFileDlg::~StreamMediaFileDlg()
{
    TT_StopLocalPlayback(ttInst, m_playbackid);
}

void StreamMediaFileDlg::slotAccepted()
{
    QVector<QString> files;
    for (int i=0;i<ui.mediafileComboBox->count();i++)
    {
        files.push_back(ui.mediafileComboBox->itemText(i));
    }
    QString filename = ui.mediafileComboBox->lineEdit()->text();
    files.removeAll(filename);
    files.push_front(filename);
    if (files.size() > MAX_MEDIAFILES)
        files.resize(MAX_MEDIAFILES);

    for (int i = 0; i < files.size(); i++)
    {
        ttSettings->setValue(QString(SETTINGS_STREAMMEDIA_FILENAME).arg(i), files[i]);
    }

    ttSettings->setValue(SETTINGS_STREAMMEDIA_LOOP,
                         ui.loopChkBox->isChecked());
    int codec_index = ui.vidcodecBox->currentIndex();
    ttSettings->setValue(SETTINGS_STREAMMEDIA_CODEC, ui.vidcodecBox->itemData(codec_index).toInt());
    ttSettings->setValue(SETTINGS_STREAMMEDIA_WEBMVP8_BITRATE, ui.vp8bitrateSpinBox->value());
}

void StreamMediaFileDlg::slotSelectFile()
{
    QFileInfo fileinfo(QDir::fromNativeSeparators(ui.mediafileComboBox->lineEdit()->text()));

    QString fileName = QFileDialog::getOpenFileName(this,
                        tr("Open Media File"),
                        QDir::toNativeSeparators(fileinfo.dir().absolutePath()),
                        tr("Media files %1").arg("(*.*)"));
    if(fileName.size())
    {
        fileName = QDir::toNativeSeparators(fileName);
        ui.mediafileComboBox->insertItem(0, fileName);
        ui.mediafileComboBox->setCurrentIndex(0);
    }
    showMediaFormatInfo();
}

void StreamMediaFileDlg::slotSelectionFile(const QString&)
{
    showMediaFormatInfo();
}

void StreamMediaFileDlg::showMediaFormatInfo()
{
    QString filename = ui.mediafileComboBox->lineEdit()->text();
    QString audio, video, duration;

    if (_Q(m_mediaFile.szFileName) == filename)
        return;

    if (TT_GetMediaFileInfo(_W(filename), &m_mediaFile))
    {
        if(m_mediaFile.audioFmt.nAudioFmt != AFF_NONE)
        {
            QString channels;
            if(m_mediaFile.audioFmt.nChannels == 2)
                channels = tr("Stereo");
            else if(m_mediaFile.audioFmt.nChannels == 1)
                channels = tr("Mono");
            else
                channels = tr("%1 audio channels").arg(m_mediaFile.audioFmt.nChannels);

            audio = tr("%1 Hz, %2").arg(m_mediaFile.audioFmt.nSampleRate).arg(channels);
        }
        else
            audio = tr("Unknown format");

        double fps = double(m_mediaFile.videoFmt.nFPS_Numerator) / double(m_mediaFile.videoFmt.nFPS_Denominator);
        if(m_mediaFile.videoFmt.picFourCC != FOURCC_NONE)
            video = tr("%1x%2 %3 FPS").arg(m_mediaFile.videoFmt.nWidth).arg(m_mediaFile.videoFmt.nHeight).arg(fps, 0, 'f', 1);
        else
            video = tr("Unknown format");

        int duration_sec = m_mediaFile.uDurationMSec / 1000;
        duration = QString("%1:%2:%3")
                .arg(duration_sec / 3600, 2 , 10, QChar('0'))
                .arg((duration_sec % 3600) / 60, 2 , 10, QChar('0'))
                .arg(duration_sec % 60, 2 , 10, QChar('0'));
    }
    else
    {
        m_mediaFile = {};
    }

    ui.audioLabel->setText(audio);
    ui.videoLabel->setText(video);
    ui.durationLabel->setText(duration);
    ui.vidcodecBox->setEnabled(m_mediaFile.videoFmt.picFourCC != FOURCC_NONE);
}

void StreamMediaFileDlg::slotPlayMediaFile()
{
    if (m_playbackid) // pause
    {
        m_mfp.bPaused = !m_mfp.bPaused;
        m_mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;

        if (!TT_UpdateLocalPlayback(ttInst, m_playbackid, &m_mfp))
        {
            QMessageBox::critical(this, tr("Play"), tr("Failed to play media file"));
            slotStopMediaFile();
        }
    }
    else // play
    {
        m_mfp.bPaused = false;
        m_mfp.uOffsetMSec = 0;
        double percent = ui.playbackOffsetSlider->value() / double(ui.playbackOffsetSlider->maximum());
        m_mfp.uOffsetMSec =  UINT32(m_mediaFile.uDurationMSec * percent);
        m_playbackid = TT_InitLocalPlayback(ttInst, _W(ui.mediafileComboBox->lineEdit()->text()), &m_mfp);
        if (m_playbackid <= 0)
        {
            QMessageBox::critical(this, tr("Play"), tr("Failed to play media file"));
            slotStopMediaFile();
        }
    }

    ui.preprocessorComboBox->setEnabled(m_playbackid <= 0);
}

void StreamMediaFileDlg::slotStopMediaFile()
{
    TT_StopLocalPlayback(ttInst, m_playbackid);
    m_playbackid = 0;

    ui.preprocessButton->setEnabled(m_playbackid <= 0);
    ui.preprocessorComboBox->setEnabled(m_playbackid <= 0);
}

void StreamMediaFileDlg::slotChangePlayOffset(int value)
{
    if (m_playbackid <= 0)
        return;

    double percent = value / double(ui.playbackOffsetSlider->maximum());
    quint32 remain = m_mediaFile.uDurationMSec;

    remain *= percent;

    quint32 hours = remain / (60 * 60 * 1000);
    remain -= 60 * 60 * 1000 * hours;
    quint32 minutes = remain / (60 * 1000);
    remain -= 60 * 1000 * minutes;
    quint32 seconds = remain / 1000;
    quint32 msec = remain % 1000;

    ui.playbackTimeLabel->setText(QString("%1:%2:%3.%4").arg(hours)
                                  .arg(minutes, 2, 10, QChar('0'))
                                  .arg(seconds, 2, 10, QChar('0'))
                                  .arg(msec, 3, 10, QChar('0')));

    m_mfp.uOffsetMSec =  UINT32(m_mediaFile.uDurationMSec * percent);

    if (!TT_UpdateLocalPlayback(ttInst, m_playbackid, &m_mfp))
    {
        QMessageBox::critical(this, tr("Play"), tr("Failed to play media file"));
        slotStopMediaFile();
    }
}

void StreamMediaFileDlg::slotChangePreprocessor(int)
{
    m_mfp.audioPreprocessor.nPreprocessor = AudioPreprocessorType(ui.preprocessorComboBox->currentData().toInt());
    initDefaultAudioPreprocessor(m_mfp.audioPreprocessor);
}

void StreamMediaFileDlg::slotSetupPreprocessor(bool)
{
    m_mfp.audioPreprocessor.nPreprocessor = AudioPreprocessorType(ui.preprocessorComboBox->currentData().toInt());
    AudioPreprocessorDlg dlg(m_mfp.audioPreprocessor, this);
    if (dlg.exec())
    {
        m_mfp.audioPreprocessor = dlg.m_preprocess;
        m_mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
        if (!TT_UpdateLocalPlayback(ttInst, m_playbackid, &m_mfp))
        {
            QMessageBox::critical(this, tr("Audio Preprocessor"), tr("Failed to activate audio preprocessor"));
        }
    }
}
