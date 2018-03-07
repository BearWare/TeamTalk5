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

#ifndef USERVOLUMEDLG_H
#define USERVOLUMEDLG_H

#include "ui_uservolume.h"

class UserVolumeDlg : public QDialog
{
    Q_OBJECT

public:
    UserVolumeDlg(int userid, QWidget * parent = 0);

private slots:
    void slotVolumeChanged(int);
    void slotMuteChannel();
    void slotDefaults();

private:
    int m_userid;
    Ui::UserVolumeDlg ui;
};
#endif
