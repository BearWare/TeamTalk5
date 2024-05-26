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

#ifndef USERACCOUNTSDLG_H
#define USERACCOUNTSDLG_H

#include "useraccountsmodel.h"

#include "ui_useraccounts.h"

class UserAccountsDlg : public QDialog
{
    Q_OBJECT

public:
    UserAccountsDlg(const useraccounts_t& useraccounts, QWidget* parent = 0);
    ~UserAccountsDlg();

public:
    void slotCmdSuccess(int cmdid);
    void slotCmdError(int error, int cmdid);

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    Ui::UserAccountsDlg ui;
    UserAccountsModel* m_useraccountsModel;
    QSortFilterProxyModel* m_proxyModel;
    int m_add_cmdid, m_del_cmdid;
    UserAccount m_user;

    QString m_del_username;

private:
    void slotAddUser();
    void slotDelUser();
    void slotEditUser();
    void slotTreeContextMenu(const QPoint&);
};

#endif
