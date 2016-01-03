/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 4 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
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
