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

#ifndef STREAMMEDIAFILEDLG_H
#define STREAMMEDIAFILEDLG_H

#include "ui_streammediafile.h"

#include "common.h"

class StreamMediaFileDlg : public QDialog
{
    Q_OBJECT

public:
    StreamMediaFileDlg(QWidget* parent = nullptr);
    ~StreamMediaFileDlg();
    
public:
    void slotMediaStreamProgress(const MediaFileInfo& mfi);
    void slotMediaPlaybackProgress(int sessionid, const MediaFileInfo& mfi);

private slots:
    void slotAccepted();
    void slotSelectFile();
    void slotDeleteItem();
    void slotSelectionFile(const QString&);
    void showMediaFormatInfo();
    void updateControls();
    void updateProgress(quint32 elapsed, bool setvalue);

    void slotPlayMediaFile();
    void slotStopMediaFile();
    void slotChangePlayOffset(int);
    void slotChangePreprocessor(int);
    void slotSetupPreprocessor(bool);

private:
    Ui::StreamMediaFileDlg ui;
    MediaFileInfo m_mediaFile = {};
    MediaFilePlayback m_mfp = {};
    VideoCodec m_videocodec = {};
    int m_playbackid = 0;
    bool m_progressupdate = false;
};

#endif
