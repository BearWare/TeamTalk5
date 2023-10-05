/*
 * Copyright (C) 2023, Bjørn D. Rasmussen, BearWare.dk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "userdesktopdlg.h"
#include "appinfo.h"
#include "utilui.h"

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
    connect(this, &UserDesktopDlg::userDesktopWindowUpdate,
            ui.userdesktopWidget, &UserDesktopWidget::slotDesktopUpdate);
    connect(ui.userdesktopWidget, &UserDesktopWidget::userDesktopWindowEnded,
            this, &UserDesktopDlg::userDesktopWindowEnded);
    connect(this, &UserDesktopDlg::userDesktopCursorUpdate,
            ui.userdesktopWidget, &UserDesktopWidget::slotDesktopCursorUpdate);
    connect(this, &UserDesktopDlg::userUpdated, ui.userdesktopWidget,
            &UserDesktopWidget::slotUserUpdated);
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
