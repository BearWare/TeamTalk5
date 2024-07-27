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

#include "streammediafiledlg.h"
#include "appinfo.h"
#include "settings.h"
#include "audiopreprocessordlg.h"
#include "utilui.h"
#include "utilmedia.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QComboBox>
#include <QKeyEvent>

extern QSettings* ttSettings;
extern TTInstance* ttInst;

#define MAX_MEDIAFILES 10

StreamMediaFileDlg::StreamMediaFileDlg(QWidget* parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    this->setAccessibleDescription(tr("Streaming to channel"));
    setWindowIcon(QIcon(APPICON));

    connect(this, &QDialog::accepted, this, &StreamMediaFileDlg::slotAccepted);
    connect(ui.toolButton, &QAbstractButton::clicked, this, &StreamMediaFileDlg::slotSelectFile);
    connect(ui.deleteButton, &QAbstractButton::clicked, this, &StreamMediaFileDlg::slotDeleteFromHistory);
    connect(ui.clearButton, &QAbstractButton::clicked, this, &StreamMediaFileDlg::slotClearHistory);
    connect(ui.refreshBtn, &QAbstractButton::clicked, this, &StreamMediaFileDlg::showMediaFormatInfo);
    //connect(ui.vidcodecBox, &QComboBox::currentIndexChanged, ui.vidcodecStackedWidget, &QStackedWidget::setCurrentIndex);
    connect(ui.stopToolButton, &QAbstractButton::clicked, this, &StreamMediaFileDlg::slotStopMediaFile);
    connect(ui.startToolButton, &QAbstractButton::clicked, this, &StreamMediaFileDlg::slotPlayMediaFile);
    connect(ui.mediafileComboBox->lineEdit(), &QLineEdit::editingFinished, this, &StreamMediaFileDlg::showMediaFormatInfo);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(ui.mediafileComboBox, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, &StreamMediaFileDlg::slotSelectionFile);
#else
    connect(ui.mediafileComboBox, &QComboBox::currentTextChanged, this, &StreamMediaFileDlg::slotSelectionFile);
#endif
    connect(ui.preprocessorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StreamMediaFileDlg::slotChangePreprocessor);
    connect(ui.preprocessButton, &QAbstractButton::clicked, this, &StreamMediaFileDlg::slotSetupPreprocessor);
    connect(ui.playbackOffsetSlider, &QSlider::sliderMoved, this, &StreamMediaFileDlg::slotChangePlayOffset);
    //connect(ui.playbackOffsetSlider, &QSlider::valueChanged, this, &StreamMediaFileDlg::slotChangePlayOffset);

    // audio preprocessor
    ui.preprocessorComboBox->addItem(tr("No Audio Preprocessor"), NO_AUDIOPREPROCESSOR);
    ui.preprocessorComboBox->addItem(tr("TeamTalk Audio Preprocessor"), TEAMTALK_AUDIOPREPROCESSOR);
    ui.preprocessorComboBox->addItem(tr("Speex DSP Audio Preprocessor"), SPEEXDSP_AUDIOPREPROCESSOR);
    AudioPreprocessorType apt = AudioPreprocessorType(ttSettings->value(SETTINGS_STREAMMEDIA_AUDIOPREPROCESSOR,
                                                      SETTINGS_STREAMMEDIA_AUDIOPREPROCESSOR_DEFAULT).toInt());
    setCurrentItemData(ui.preprocessorComboBox, apt);

    ui.playbackOffsetSlider->setMaximum(MEDIAFILE_SLIDER_MAXIMUM);

    m_videocodec.nCodec = Codec(ttSettings->value(SETTINGS_STREAMMEDIA_CODEC, DEFAULT_VIDEO_CODEC).toInt());
    switch(m_videocodec.nCodec)
    {
    case WEBM_VP8_CODEC:
        m_videocodec.webm_vp8.nRcTargetBitrate = ttSettings->value(SETTINGS_STREAMMEDIA_WEBMVP8_BITRATE, DEFAULT_WEBMVP8_BITRATE).toInt();
        m_videocodec.webm_vp8.nEncodeDeadline = DEFAULT_WEBMVP8_DEADLINE;
        break;
    default:
        break;
    }

    ui.vidcodecBox->addItem(tr("No video"), NO_CODEC);
    ui.vidcodecBox->addItem("WebM VP8", WEBM_VP8_CODEC);
    ui.vp8bitrateSpinBox->setValue(m_videocodec.webm_vp8.nRcTargetBitrate);
    setCurrentItemData(ui.vidcodecBox, m_videocodec.nCodec);

    int i = 0;
    QString item;
    while ((item = ttSettings->value(QString(SETTINGS_STREAMMEDIA_FILENAME).arg(i++)).toString()).size())
    {
        ui.mediafileComboBox->addItem(item);
    }
    ui.mediafileComboBox->setCurrentIndex(0); // generates showMediaFormatInfo()

    updateControls();
    ui.mediafileComboBox->installEventFilter(this);
    ui.mediafileComboBox->lineEdit()->setCursorPosition(0);
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

    ttSettings->setValue(SETTINGS_STREAMMEDIA_AUDIOPREPROCESSOR, getCurrentItemData(ui.preprocessorComboBox).toInt());
    ttSettings->setValue(SETTINGS_STREAMMEDIA_OFFSET, m_mfp.uOffsetMSec);
    ttSettings->setValue(SETTINGS_STREAMMEDIA_LOOP, ui.loopChkBox->isChecked());
    ttSettings->setValue(SETTINGS_STREAMMEDIA_CODEC, getCurrentItemData(ui.vidcodecBox).toInt());
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
        ui.mediafileComboBox->setCurrentIndex(0); // generates showMediaFormatInfo()
    }
}

void StreamMediaFileDlg::slotDeleteFromHistory()
{
    ui.mediafileComboBox->removeItem(ui.mediafileComboBox->currentIndex());
    for (int i = 0; i<ttSettings->value(QString(SETTINGS_STREAMMEDIA_FILENAME).arg(i)).toString().size(); i++)
    {
        ttSettings->remove(QString(SETTINGS_STREAMMEDIA_FILENAME).arg(i));
    }
    for (int i = 0; i<ui.mediafileComboBox->count(); i++)
    {
        ttSettings->remove(QString(SETTINGS_STREAMMEDIA_FILENAME).arg(i));
    }
    QVector<QString> files;
    for (int j = 0; j<ui.mediafileComboBox->count(); j++)
    {
        files.push_back(ui.mediafileComboBox->itemText(j));
    }
    QString filename = ui.mediafileComboBox->lineEdit()->text();
    files.removeAll(filename);
    files.push_front(filename);
    if (files.size() > MAX_MEDIAFILES)
        files.resize(MAX_MEDIAFILES);
    for (int k = 0; k < files.size(); k++)
    {
        ttSettings->setValue(QString(SETTINGS_STREAMMEDIA_FILENAME).arg(k), files[k]);
    }
}

void StreamMediaFileDlg::slotClearHistory()
{
    QMessageBox answer;
    answer.setText(tr("Are you sure you want to clear stream history?"));
    QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
    QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
    Q_UNUSED(NoButton);
    answer.setIcon(QMessageBox::Question);
    answer.setWindowTitle(tr("Clear history"));
    answer.exec();
    if(answer.clickedButton() == YesButton)
    {
        for (int i = 0; i<ui.mediafileComboBox->count(); i++)
        {
            ttSettings->remove(QString(SETTINGS_STREAMMEDIA_FILENAME).arg(i));
        }
        ui.mediafileComboBox->clear();
    }
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
        audio = getMediaAudioDescription(m_mediaFile.audioFmt);
        video = getMediaVideoDescription(m_mediaFile.videoFmt);
        duration = durationToString(m_mediaFile.uDurationMSec, false);
    }
    else
    {
        m_mediaFile = {};
    }

    ui.audioLabel->setText(audio);
    ui.audioLabel->setAccessibleName(QString("%1 %2").arg(ui.label_2->text()).arg(audio));
    ui.videoLabel->setText(video);
    ui.videoLabel->setAccessibleName(QString("%1 %2").arg(ui.label_3->text()).arg(video));
    ui.durationLabel->setText(duration);
    ui.durationLabel->setAccessibleName(QString("%1 %2").arg(ui.label_4->text()).arg(duration));

    if (m_mediaFile.videoFmt.picFourCC == FOURCC_NONE)
    {
        setCurrentItemData(ui.vidcodecBox, NO_CODEC);
    }
    else
    {
        setCurrentItemData(ui.vidcodecBox, WEBM_VP8_CODEC);
    }
    ui.vidcodecBox->setEnabled(m_mediaFile.videoFmt.picFourCC != FOURCC_NONE);
}

void StreamMediaFileDlg::updateControls()
{
    ui.startToolButton->setEnabled(!isMyselfStreaming());
    ui.stopToolButton->setEnabled(!isMyselfStreaming());
    ui.preprocessorComboBox->setEnabled(m_playbackid <= 0 || !isMyselfStreaming());
    ui.vidcodecBox->setEnabled(!isMyselfStreaming());
}

void StreamMediaFileDlg::updateProgress(quint32 elapsed, bool setvalue)
{
    ui.playbackTimeLabel->setText(durationToString(elapsed));

    if (m_mediaFile.uDurationMSec && setvalue)
    {
        double percent = elapsed / (double)m_mediaFile.uDurationMSec;
        int value = int(percent * double(ui.playbackOffsetSlider->maximum()));
        ui.playbackOffsetSlider->setValue(value);
    }
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
    ui.startToolButton->setText(tr("&Play"));
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
    ui.startToolButton->setText(tr("&Pause"));
    }

    ui.preprocessorComboBox->setEnabled(m_playbackid <= 0);
}

void StreamMediaFileDlg::slotStopMediaFile()
{
    if (m_playbackid > 0)
        TT_StopLocalPlayback(ttInst, m_playbackid);
    m_playbackid = 0;
    ui.startToolButton->setText(tr("&Play"));
    updateControls();
    ui.playbackOffsetSlider->setValue(0);
    updateProgress(0, true);
}

void StreamMediaFileDlg::slotChangePlayOffset(int value)
{
    double percent = value / double(ui.playbackOffsetSlider->maximum());
    quint32 remain = m_mediaFile.uDurationMSec;

    remain = quint32(remain * percent);

    updateProgress(remain, false);

    m_mfp.uOffsetMSec = UINT32(m_mediaFile.uDurationMSec * percent);

    if (m_progressupdate)
        return;

    if (m_playbackid > 0)
    {
        if(!TT_UpdateLocalPlayback(ttInst, m_playbackid, &m_mfp))
        {
            QMessageBox::critical(this, tr("Play"), tr("Failed to play media file"));
            slotStopMediaFile();
        }
    }
    else if (isMyselfStreaming())
    {
        if (!TT_UpdateStreamingMediaFileToChannel(ttInst, &m_mfp, &m_videocodec))
        {
            QMessageBox::critical(this, tr("Stream"), tr("Failed to stream media file"));
        }
    }
}

void StreamMediaFileDlg::slotChangePreprocessor(int /*index*/)
{
    auto apt = AudioPreprocessorType(ui.preprocessorComboBox->currentData().toInt());
    m_mfp.audioPreprocessor = loadAudioPreprocessor(apt);
}

void StreamMediaFileDlg::slotSetupPreprocessor(bool)
{
    m_mfp.audioPreprocessor.nPreprocessor = AudioPreprocessorType(ui.preprocessorComboBox->currentData().toInt());
    AudioPreprocessorDlg dlg(m_mfp.audioPreprocessor, this);
    if (dlg.exec())
    {
        m_mfp.audioPreprocessor = dlg.m_preprocess;

        ttSettings->setValue(SETTINGS_STREAMMEDIA_AUDIOPREPROCESSOR, m_mfp.audioPreprocessor.nPreprocessor);
        switch(m_mfp.audioPreprocessor.nPreprocessor)
        {
        case NO_AUDIOPREPROCESSOR :
            break;
        case SPEEXDSP_AUDIOPREPROCESSOR:
             ttSettings->setValue(SETTINGS_STREAMMEDIA_SPX_AGC_ENABLE, m_mfp.audioPreprocessor.speexdsp.bEnableAGC);
             ttSettings->setValue(SETTINGS_STREAMMEDIA_SPX_AGC_GAINLEVEL, m_mfp.audioPreprocessor.speexdsp.nGainLevel);
             ttSettings->setValue(SETTINGS_STREAMMEDIA_SPX_AGC_INC_MAXDB, m_mfp.audioPreprocessor.speexdsp.nMaxIncDBSec);
             ttSettings->setValue(SETTINGS_STREAMMEDIA_SPX_AGC_DEC_MAXDB, m_mfp.audioPreprocessor.speexdsp.nMaxDecDBSec);
             ttSettings->setValue(SETTINGS_STREAMMEDIA_SPX_AGC_GAINMAXDB, m_mfp.audioPreprocessor.speexdsp.nMaxGainDB);
             ttSettings->setValue(SETTINGS_STREAMMEDIA_SPX_DENOISE_ENABLE, m_mfp.audioPreprocessor.speexdsp.bEnableDenoise);
             ttSettings->setValue(SETTINGS_STREAMMEDIA_SPX_DENOISE_SUPPRESS, m_mfp.audioPreprocessor.speexdsp.nMaxNoiseSuppressDB);
            break;
        case TEAMTALK_AUDIOPREPROCESSOR:
             ttSettings->setValue(SETTINGS_STREAMMEDIA_TTAP_MUTELEFT, m_mfp.audioPreprocessor.ttpreprocessor.bMuteLeftSpeaker);
             ttSettings->setValue(SETTINGS_STREAMMEDIA_TTAP_MUTERIGHT, m_mfp.audioPreprocessor.ttpreprocessor.bMuteRightSpeaker);
             ttSettings->setValue(SETTINGS_STREAMMEDIA_TTAP_GAINLEVEL, m_mfp.audioPreprocessor.ttpreprocessor.nGainLevel);
            break;
        case WEBRTC_AUDIOPREPROCESSOR :
            // TODO: no dialog for WebRTC so far.
            break;
        }
        
        m_mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
        if (m_playbackid > 0)
        {
            if (!TT_UpdateLocalPlayback(ttInst, m_playbackid, &m_mfp))
                QMessageBox::critical(this, tr("Audio Preprocessor"), tr("Failed to activate audio preprocessor"));
        }
        else if (isMyselfStreaming())
        {
            if (!TT_UpdateStreamingMediaFileToChannel(ttInst, &m_mfp, &m_videocodec))
                QMessageBox::critical(this, tr("Audio Preprocessor"), tr("Failed to activate audio preprocessor"));
        }
    }
}

void StreamMediaFileDlg::slotMediaStreamProgress(const MediaFileInfo& mfi)
{
    m_progressupdate = true;

    m_mediaFile = mfi;
    updateProgress(mfi.uElapsedMSec, true);
    switch (mfi.nStatus)
    {
    case MFS_CLOSED :
        break;
    case MFS_STARTED :
        ui.startToolButton->setText(tr("&Pause"));
        break;
    case MFS_PLAYING :
        break;
    case MFS_PAUSED :
        ui.startToolButton->setText(tr("&Play"));
        break;
    case MFS_ABORTED :
        break;
    case MFS_ERROR :
    case MFS_FINISHED :
        updateControls();
        break;
    }
    m_progressupdate = false;
}

void StreamMediaFileDlg::slotMediaPlaybackProgress(int sessionid, const MediaFileInfo& mfi)
{
    if (m_playbackid != sessionid)
        return;

    m_progressupdate = true;
    m_mediaFile = mfi;
    
    if (!ui.playbackOffsetSlider->isSliderDown())
        updateProgress(mfi.uElapsedMSec, true);

    switch (mfi.nStatus)
    {
    case MFS_CLOSED :
        break;
    case MFS_STARTED :
        ui.startToolButton->setText(tr("&Pause"));
        break;
    case MFS_PLAYING :
        break;
    case MFS_PAUSED :
        ui.startToolButton->setText(tr("&Play"));
        break;
    case MFS_ABORTED :
        break;
    case MFS_ERROR :
    case MFS_FINISHED :
        m_playbackid = 0; // automatically deleted. Don't call TT_StopLocalPlayback().
        slotStopMediaFile();
        updateControls();
        break;
    }

    m_progressupdate = false;
}

bool StreamMediaFileDlg::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui.mediafileComboBox && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->matches(QKeySequence::Paste) && ui.mediafileComboBox->currentIndex() == 0 && ui.mediafileComboBox->lineEdit()->cursorPosition() == 0)
        {
            ui.mediafileComboBox->lineEdit()->clear();
            ui.mediafileComboBox->lineEdit()->paste();
            return true;
        }
        else
            return false;
    }
    return false;
}
