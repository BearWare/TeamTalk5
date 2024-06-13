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

#ifndef USERACCOUNTDLG_H
#define USERACCOUNTDLG_H

#include "utiltt.h"

#include <QDialog>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QDialogButtonBox>

namespace Ui {
class UserAccountDlg;
}

class UserAccountDlg : public QDialog
{
    Q_OBJECT

public:
    enum UserAccountDlgType
    {
        USER_CREATE,
        USER_UPDATE,
        USER_READONLY
    };
    explicit UserAccountDlg(UserAccountDlgType type, const UserAccount& useraccount, QWidget *parent = nullptr);
    ~UserAccountDlg();

    UserAccount getUserAccount() const;

protected:
    void accept() override;

private:
    Ui::UserAccountDlg *ui;
    QWidget* m_userRightsTab;
    class UserRightsModel* m_userrightsModel;
    UserAccount m_useraccount;
    UserAccountDlgType m_type;
    void updateUserRights(const UserAccount& useraccount);
    void toggleUserRights(const QModelIndex &index);
    UserTypes getUserType() const;
    void slotUserTypeChanged();
    void slotCustomCmdLimit(int index);
    void slotAddOpChannel();
    void slotRemoveOpChannel();
    void slotUsernameChanged();
    void showUserAccount(const UserAccount& useraccount);
    void updateCustomLimitText(int nCommandsLimit, int nCommandsIntervalMSec);
};

class CustomCmdLimitDialog : public QDialog
{
    Q_OBJECT

public:
    CustomCmdLimitDialog(int currentLimit, int currentIntervalSec, QWidget *parent = nullptr);

    int getCommandLimit() const;
    int getIntervalSec() const;

private:
    QSpinBox *m_cmdLimitSpinBox;
    QSpinBox *m_intervalSpinBox;
};

#endif // USERACCOUNTDLG_H
