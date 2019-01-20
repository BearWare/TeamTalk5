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
#include "common.h"

#include <QFileDialog>
#include <QLineEdit>

extern QSettings* ttSettings;
extern TTInstance* ttInst;

#define MAX_MEDIAFILES 10

StreamMediaFileDlg::StreamMediaFileDlg(QWidget* parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    connect(this, SIGNAL(accepted()), SLOT(slotAccepted()));
    connect(ui.toolButton, SIGNAL(clicked()), SLOT(slotSelectFile()));
    connect(ui.refreshBtn, SIGNAL(clicked()), SLOT(showMediaFormatInfo()));
    connect(ui.vidcodecBox, SIGNAL(currentIndexChanged(int)),
            ui.vidcodecStackedWidget, SLOT(setCurrentIndex(int)));

    int i = 0;
    QString item;
    while ((item = ttSettings->value(QString(SETTINGS_STREAMMEDIA_FILENAME).arg(i++)).toString()).size())
    {
        ui.mediafileComboBox->addItem(item);

    }
    ui.mediafileComboBox->setCurrentIndex(0);

    int vidcodec = ttSettings->value(SETTINGS_STREAMMEDIA_CODEC, DEFAULT_VIDEO_CODEC).toInt();

    ui.vidcodecBox->addItem("WebM VP8", WEBM_VP8_CODEC);

    ui.vp8bitrateSpinBox->setValue(ttSettings->value(SETTINGS_STREAMMEDIA_WEBMVP8_BITRATE,
                                                     DEFAULT_WEBMVP8_BITRATE).toInt());
    int vidindex = ui.vidcodecBox->findData(vidcodec);
    ui.vidcodecBox->setCurrentIndex(vidindex);

    showMediaFormatInfo();
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

void StreamMediaFileDlg::showMediaFormatInfo()
{
    QString filename = ui.mediafileComboBox->lineEdit()->text();
    QString audio, video;
    MediaFileInfo mediaFile;
    ZERO_STRUCT(mediaFile);

    if(filename.size() &&
       TT_GetMediaFileInfo(_W(filename), &mediaFile))
    {
        if(mediaFile.audioFmt.nAudioFmt != AFF_NONE)
        {
            QString channels;
            if(mediaFile.audioFmt.nChannels == 2)
                channels = tr("Stereo");
            else if(mediaFile.audioFmt.nChannels == 1)
                channels = tr("Mono");
            else
                channels = tr("%1 audio channels").arg(mediaFile.audioFmt.nChannels);

            audio = tr("%1 Hz, %2").arg(mediaFile.audioFmt.nSampleRate).arg(channels);
        }
        else
            audio = tr("Unknown format");

        double fps = (double)mediaFile.videoFmt.nFPS_Numerator / (double)mediaFile.videoFmt.nFPS_Denominator;
        if(mediaFile.videoFmt.picFourCC != FOURCC_NONE)
            video = tr("%1x%2 %3 FPS").arg(mediaFile.videoFmt.nWidth).arg(mediaFile.videoFmt.nHeight).arg(fps, 0, 'f', 1);
        else
            video = tr("Unknown format");

        int duration_sec = mediaFile.uDurationMSec / 1000;
        ui.durationLabel->setText(QString("%1:%2:%3")
                                  .arg(duration_sec / 3600, 2 , 10, QChar('0'))
                                  .arg((duration_sec % 3600) / 60, 2 , 10, QChar('0'))
                                  .arg(duration_sec % 60, 2 , 10, QChar('0')));
        ui.vidcodecBox->setEnabled(mediaFile.videoFmt.picFourCC != FOURCC_NONE);
    }

    ui.audioLabel->setText(audio);
    ui.videoLabel->setText(video);
}

