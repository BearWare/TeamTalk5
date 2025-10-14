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

#ifndef DESKTOPGRIDWIDGET_H
#define DESKTOPGRIDWIDGET_H

#include "gridwidget.h"

#include "common.h"

class DesktopGridWidget : public GridWidget
{
    Q_OBJECT

public:
    DesktopGridWidget(QWidget * parent);

    void removeUser(int userid);

signals:
    void userDesktopWindowUpdate(int,int);
    void userDesktopWindowEnded(int);
    void userDesktopSelected(bool selected);
    void userDesktopCursorUpdate(int, const DesktopInput&);
    void userUpdated(const User&);

    void desktopCountChanged(int);

protected:
    QString getEmptyGridText() const;

public:
    void slotAddUser(int userid);
    void slotRemoveUser(int channelid, const User& user);
    void slotDesktopWidgetFocused(int userid, bool focused);
};

#endif
