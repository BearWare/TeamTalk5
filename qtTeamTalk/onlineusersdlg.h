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

#ifndef ONLINEUSERSDLG_H
#define ONLINEUSERSDLG_H

#include "ui_onlineusers.h"
#include "onlineusersmodel.h"
#include <QSortFilterProxyModel>

class OnlineUsersDlg : public QDialog
{
    Q_OBJECT

public:
    OnlineUsersDlg(QWidget* parent = 0);

    void updateTitle();

public slots:
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
    void kickbanUser(int userid, int chanid);
    void streamfileToUser(int userid);

private slots:
    void slotTreeContextMenu(const QPoint&);

private:
    Ui::OnlineUsersDlg ui;
    OnlineUsersModel* m_model;
    QSortFilterProxyModel* m_proxyModel;
};

#endif
