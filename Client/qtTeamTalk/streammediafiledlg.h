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

private:
    void slotAccepted();
    void slotSelectFile();
    void slotDeleteFromHistory();
    void slotClearHistory();
    void slotSelectionFile(const QString&);
    void showMediaFormatInfo();
    void updateControls();
    void updateProgress(quint32 elapsed, bool setvalue);

    void slotPlayMediaFile();
    void slotStopMediaFile();
    void slotChangePlayOffset(int);
    void slotChangePreprocessor(int);
    void slotSetupPreprocessor(bool);

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    Ui::StreamMediaFileDlg ui;
    MediaFileInfo m_mediaFile = {};
    MediaFilePlayback m_mfp = {};
    VideoCodec m_videocodec = {};
    int m_playbackid = 0;
    bool m_progressupdate = false;
};

#endif
