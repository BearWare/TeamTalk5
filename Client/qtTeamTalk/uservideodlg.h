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

#ifndef USERVIDEODLG_H
#define USERVIDEODLG_H

#include "common.h"
#include "ui_uservideo.h"
#include "uservideowidget.h"

class UserVideoDlg : public QDialog
{
    Q_OBJECT

public:
    UserVideoDlg(int userid_masked, const User& user, const QSize& size, 
                 QWidget * parent = 0);
    UserVideoDlg(int userid_masked, const User& user, QWidget * parent = 0);
    ~UserVideoDlg();

    UserVideoWidget *uservideoWidget;

public:
    void slotUserUpdate(const User& user);
    void slotNewVideoFrame(int userid, int streamid);

protected:
    void initCommon(int userid);

private:
    Ui::UserVideoDlg ui;
    bool m_autosize;

signals:
    void userVideoDlgClosing(int userid);
};

#endif
