/*
 * Copyright (c) 2005-2017, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
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
};


#endif
