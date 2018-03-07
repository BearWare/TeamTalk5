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

#ifndef USERINFODLG_H
#define USERINFODLG_H

#include "ui_userinfo.h"

class UserInfoDlg : public QDialog
{
    Q_OBJECT

public:
    UserInfoDlg(int userid, QWidget * parent = 0);

protected:
    void timerEvent(QTimerEvent *event);

private:
    int m_userid;
     Ui::UserInfoDlg ui;
     void updateUser();

private slots:
     void slotProfile(bool checked = false);
};


#endif
