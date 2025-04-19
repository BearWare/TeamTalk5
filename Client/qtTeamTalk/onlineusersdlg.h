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

#ifndef ONLINEUSERSDLG_H
#define ONLINEUSERSDLG_H

#include "ui_onlineusers.h"
#include "onlineusersmodel.h"
#include <QSortFilterProxyModel>
#include <QShortcut>

class OnlineUsersDlg : public QDialog
{
    Q_OBJECT

public:
    OnlineUsersDlg(QWidget* parent = 0);
    ~OnlineUsersDlg();

    void updateTitle();

public:
    void slotUserLoggedIn(const User& user);
    void slotUserLoggedOut(const User& user);
    void slotUserUpdate(const User& user);
    void slotUserJoin(int channelid, const User& user);
    void slotUserLeft(int channelid, const User& user);

signals:
    void viewUserInformation(int userid);
    void sendUserMessage(int userid);
    void muteUser(int userid, bool mute);
    void changeUserVolume(int userid);
    void opUser(int userid, int chanid);
    void kickUser(int userid, int chanid);
    void kickbanUser(const User& user);
    void streamfileToUser(int userid);
    void moveUser(int userid);

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    void slotTreeContextMenu(const QPoint&);
    QShortcut *m_viewInfoAct   = nullptr;
    QShortcut *m_msgAct   = nullptr;
    QShortcut *m_opAct   = nullptr;
    QShortcut *m_kickAct   = nullptr;
    QShortcut *m_kickServAct   = nullptr;
    QShortcut *m_kickBanAct   = nullptr;
    QShortcut *m_kickBanServAct   = nullptr;
    QShortcut *m_selMoveAct   = nullptr;
    enum MenuAction
    {
        VIEW_USERINFORMATION,
        SEND_TEXTMESSAGE,
        OP,
        KICK_FROM_CHANNEL,
        KICK_FROM_SERVER,
        BAN_FROM_CHANNEL,
        BAN_FROM_SERVER,
        MOVE
    };

    void menuAction(MenuAction ma);
    void slotUpdateSettings();

private:
    Ui::OnlineUsersDlg ui;
    OnlineUsersModel* m_model;
    QSortFilterProxyModel* m_proxyModel;
};

#endif
