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
    connect(this, &DesktopGridWidget::userDesktopWindowUpdate,
            userdesktop, &UserDesktopWidget::slotDesktopUpdate);
    connect(userdesktop, &UserDesktopWidget::userDesktopWindowEnded,
            this, &DesktopGridWidget::userDesktopWindowEnded);
    connect(userdesktop, &UserImageWidget::userWidgetFocused,
            this, &DesktopGridWidget::slotDesktopWidgetFocused);
    connect(this, &DesktopGridWidget::userDesktopCursorUpdate,
            userdesktop, &UserDesktopWidget::slotDesktopCursorUpdate);
    connect(this, &DesktopGridWidget::userUpdated, userdesktop,
            &UserDesktopWidget::slotUserUpdated);
    addUserWidget(userid, userdesktop);

    emit desktopCountChanged(getActiveUsersCount());
}

void DesktopGridWidget::slotRemoveUser(int channelid, const User& user)
{
    Q_UNUSED(channelid);
    removeUser(user.nUserID);
}

void DesktopGridWidget::removeUser(int userid)
{
    removeUserWidget(userid);

    emit desktopCountChanged(getActiveUsersCount());
}

void DesktopGridWidget::slotDesktopWidgetFocused(int /*userid*/, bool focused)
{
    emit userDesktopSelected(focused);
}
