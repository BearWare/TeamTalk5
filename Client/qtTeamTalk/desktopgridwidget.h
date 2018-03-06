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
