/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#include "streammediafiledlg.h"
#include "appinfo.h"
#include "settings.h"
#include "common.h"

#include <QFileDialog>

extern QSettings* ttSettings;
extern TTInstance* ttInst;

StreamMediaFileDlg::StreamMediaFileDlg(QWidget* parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    connect(this, SIGNAL(accepted()), SLOT(slotAccepted()));
    connect(ui.toolButton, SIGNAL(clicked()), SLOT(slotSelectFile()));
    connect(ui.vidcodecBox, SIGNAL(currentIndexChanged(int)),
            ui.vidcodecStackedWidget, SLOT(setCurrentIndex(int)));

    ui.mediafileEdit->setText(ttSettings->value(SETTINGS_STREAMMEDIA_FILENAME).toString());

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
    ttSettings->setValue(SETTINGS_STREAMMEDIA_FILENAME,
                         ui.mediafileEdit->text());
    ttSettings->setValue(SETTINGS_STREAMMEDIA_LOOP,
                         ui.loopChkBox->isChecked());
    int codec_index = ui.vidcodecBox->currentIndex();
    ttSettings->setValue(SETTINGS_STREAMMEDIA_CODEC, ui.vidcodecBox->itemData(codec_index).toInt());
    ttSettings->setValue(SETTINGS_STREAMMEDIA_WEBMVP8_BITRATE, ui.vp8bitrateSpinBox->value());
}

void StreamMediaFileDlg::slotSelectFile()
{
    QFileInfo fileinfo(QDir::fromNativeSeparators(ui.mediafileEdit->text()));

    QString fileName = QFileDialog::getOpenFileName(this,
                        tr("Open Media File"),
                        QDir::toNativeSeparators(fileinfo.dir().absolutePath()),
                        tr("Media files (*.*)"));

    fileName = QDir::toNativeSeparators(fileName);
    ui.mediafileEdit->setText(fileName);

    showMediaFormatInfo();
}

void StreamMediaFileDlg::showMediaFormatInfo()
{
    QString filename = ui.mediafileEdit->text();
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

    ui.okBtn->setEnabled(mediaFile.audioFmt.nAudioFmt != AFF_NONE ||
                         mediaFile.videoFmt.picFourCC != FOURCC_NONE);
}

