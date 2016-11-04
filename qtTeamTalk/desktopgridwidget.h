/*
 * Copyright (c) 2005-2016, BearWare.dk
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

public slots:
    void slotAddUser(int userid);
    void slotRemoveUser(int channelid, const User& user);
    void slotDesktopWidgetFocused(int userid, bool focused);
};

#endif
