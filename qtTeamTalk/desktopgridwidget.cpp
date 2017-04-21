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

#include "desktopgridwidget.h"
#include "userdesktopwidget.h"

DesktopGridWidget::DesktopGridWidget(QWidget * parent)
    : GridWidget(parent)
{
}

QString DesktopGridWidget::getEmptyGridText() const
{
    return tr("No active desktop sessions");
}

void DesktopGridWidget::slotAddUser(int userid)
{
    Q_ASSERT(!userExists(userid));

    UserDesktopWidget* userdesktop = new UserDesktopWidget(this, userid, BORDER_WIDTH);
    userdesktop->setFocusPolicy(Qt::StrongFocus); //so we can get keyboard input
    userdesktop->setAttribute(Qt::WA_DeleteOnClose);
    connect(this, SIGNAL(userDesktopWindowUpdate(int,int)),
            userdesktop, SLOT(slotDesktopUpdate(int,int)));
    connect(userdesktop, SIGNAL(userDesktopWindowEnded(int)),
            SIGNAL(userDesktopWindowEnded(int)));
    connect(userdesktop, SIGNAL(userWidgetFocused(int, bool)), 
            this, SLOT(slotDesktopWidgetFocused(int, bool)));
    connect(this, SIGNAL(userDesktopCursorUpdate(int,const DesktopInput&)),
            userdesktop, SLOT(slotDesktopCursorUpdate(int,const DesktopInput&)));
    connect(this, SIGNAL(userUpdated(const User&)), userdesktop,
            SLOT(slotUserUpdated(const User&)));
    addUserWidget(userid, userdesktop);

    emit(desktopCountChanged(getActiveUsersCount()));
}

void DesktopGridWidget::slotRemoveUser(int channelid, const User& user)
{
    Q_UNUSED(channelid);
    removeUser(user.nUserID);
}

void DesktopGridWidget::removeUser(int userid)
{
    removeUserWidget(userid);

    emit(desktopCountChanged(getActiveUsersCount()));
}

void DesktopGridWidget::slotDesktopWidgetFocused(int /*userid*/, bool focused)
{
    emit(userDesktopSelected(focused));
}
