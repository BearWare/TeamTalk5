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

#ifndef USERDLG_H
#define USERDLG_H

#include "common.h"
#include "useraccountsmodel.h"

#include <QDialog>

namespace Ui {
class UserDlg;
}

class UserDlg : public QDialog
{
    Q_OBJECT

public:
    enum UserDlgType
    {
        USER_CREATE,
        USER_UPDATE,
        USER_READONLY
    };
    explicit UserDlg(UserDlgType type, const UserAccount& user, QWidget *parent = nullptr);
    ~UserDlg();

    UserAccount getUser() const;

protected:
    void accept() override;
    void keyPressEvent(QKeyEvent* e) override;

private:
    Ui::UserDlg *ui;
    UserRightsModel* m_userrightsModel;
    UserAccount m_user, newUser;
    UserDlgType m_type;
    AbusePrevention m_abuse;
    void updateUserRights(const UserAccount& useraccount);
    void toggleUserRights(const QModelIndex &index);
    UserTypes getUserType() const;
    void slotUserTypeChanged();
    void slotCustomCmdLimit(int index);
    void slotAddOpChannel();
    void slotRemoveOpChannel();
    void slotUsernameChanged(const QString& text);
    void showUserAccount(const UserAccount& useraccount);
};

#endif // USERDLG_H
