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
 * This source code is part of the TeamTalk 4 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#ifndef SERVERSTATISTICS_H
#define SERVERSTATISTICS_H

#include "common.h"

#include "ui_serverstats.h"

class ServerStatisticsDlg : public QDialog
{
    Q_OBJECT

public:
    ServerStatisticsDlg(QWidget * parent = 0);

private slots:
    void slotUpdateCmd();
    void slotCmdSuccess(int cmdid);
    void slotUpdateStats(const ServerStatistics& stats);

private:
    Ui::ServerStatsDlg ui;
    int m_cmdid;
    ServerStatistics m_lastStats;
};

#endif
