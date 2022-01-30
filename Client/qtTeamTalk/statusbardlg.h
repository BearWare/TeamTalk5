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

#ifndef STATUSBARDLG_H
#define STATUSBARDLG_H

#include "ui_statusbardlg.h"
#include "utilui.h"

class StatusBarDlg : public QDialog
{
    Q_OBJECT

public:
    StatusBarDlg(QWidget* parent, StatusBarEvents events);

private:
    void slotAccept();

private:
    Ui::StatusBarDlg ui;
    class StatusBarEventsModel* m_statusbarmodel = nullptr;
    void slotStatusBarEventToggled(const QModelIndex &index);
    void slotStatusBarEnableAll(bool checked);
    void slotStatusBarClearAll(bool checked);
    void slotStatusBarRevert(bool checked);
    StatusBarEvents m_events;
};

#endif
