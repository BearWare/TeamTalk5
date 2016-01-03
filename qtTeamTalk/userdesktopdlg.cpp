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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
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
    p.setBrush(QPalette::Background, QBrush(Qt::black));
    ui.userdesktopWidget->setPalette(p);
}

void UserDesktopDlg::slotUserUpdate(const User& user)
{
    if(user.nUserID != ui.userdesktopWidget->getUserID())
        return;

    if(user.nUserID == 0)
        setWindowTitle(tr("Desktop") + QString(" - ") + tr("Myself"));
    else
        setWindowTitle(tr("Desktop") + QString(" - ") + _Q(user.szNickname));
}
