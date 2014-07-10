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

#ifndef USERDESKTOPDLG_H
#define USERDESKTOPDLG_H

#include "common.h"

#include "ui_userdesktop.h"

class UserDesktopDlg : public QDialog
{
    Q_OBJECT

public:
    UserDesktopDlg(const User& user, const QSize& size, QWidget * parent = 0);
    ~UserDesktopDlg();

public slots:
    void slotUserUpdate(const User& user);

private:
    void initBackground();

    Ui::UserDesktopDlg ui;
    bool m_autosize;

signals:
    void userDesktopDlgClosing(int userid);
    void userDesktopWindowUpdate(int userid, int sessionid);
    void userDesktopWindowEnded(int);
    void userDesktopCursorUpdate(int, const DesktopInput&);
    void userUpdated(const User&);
};

#endif
