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

#include "userdesktopdlg.h"
#include "appinfo.h"

extern TTInstance* ttInst;

UserDesktopDlg::UserDesktopDlg(const User& user, const QSize& size, 
                               QWidget * parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
, m_autosize(false)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    this->resize(size);
    ui.userdesktopWidget->setUserID(user.nUserID);
    connect(this, SIGNAL(userDesktopWindowUpdate(int,int)),
            ui.userdesktopWidget, SLOT(slotDesktopUpdate(int,int)));
    connect(ui.userdesktopWidget, SIGNAL(userDesktopWindowEnded(int)),
            SIGNAL(userDesktopWindowEnded(int)));
    connect(this, SIGNAL(userDesktopCursorUpdate(int,const DesktopInput&)),
            ui.userdesktopWidget, SLOT(slotDesktopCursorUpdate(int,const DesktopInput&)));
    connect(this, SIGNAL(userUpdated(const User&)), ui.userdesktopWidget,
            SLOT(slotUserUpdated(const User&)));
    ui.userdesktopWidget->slotDesktopUpdate(user.nUserID, -1);
    slotUserUpdate(user);

    initBackground();

#if defined(Q_OS_DARWIN)
    setMacResizeMargins(this, ui.horizontalLayout);
#endif
}

UserDesktopDlg::~UserDesktopDlg()
{
    emit(userDesktopDlgClosing(ui.userdesktopWidget->getUserID()));
}

void UserDesktopDlg::initBackground()
{
    //set black as background color
    ui.userdesktopWidget->setAutoFillBackground(true);
    QPalette p = ui.userdesktopWidget->palette();
    p.setBrush(QPalette::Window, QBrush(Qt::black));
    ui.userdesktopWidget->setPalette(p);
}

void UserDesktopDlg::slotUserUpdate(const User& user)
{
    if(user.nUserID != ui.userdesktopWidget->getUserID())
        return;

    if(user.nUserID == 0)
        setWindowTitle(tr("Desktop") + QString(" - ") + tr("Myself"));
    else
        setWindowTitle(tr("Desktop") + QString(" - ") + getDisplayName(user));
}
