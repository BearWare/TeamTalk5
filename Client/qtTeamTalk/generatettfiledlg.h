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

#ifndef GENERATETTFILEDLG_H
#define GENERATETTFILEDLG_H

#include "ui_generatettfile.h"

#include "common.h"

class GenerateTTFileDlg : public QDialog
{
    Q_OBJECT

public:
    GenerateTTFileDlg(const HostEntry& entry, QWidget * parent = 0);

private slots:
    void slotSetupHotkey();
    void slotVideoCodecChange(int);
    void slotSaveTTFile();

private:
    void loadVideoFormats();
    Ui::GenTTFileDlg ui;
    QVector<VideoFormat> m_vidcap_fmts;
    HostEntry m_hostentry;
};

#endif
