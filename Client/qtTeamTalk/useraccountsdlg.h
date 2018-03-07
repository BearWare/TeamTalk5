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

#ifndef USERACCOUNTSDLG_H
#define USERACCOUNTSDLG_H

#include "common.h"

#include <QAbstractItemModel>
#include <QVector>
#include <QSet>
#include "ui_useraccounts.h"


typedef QVector<UserAccount> useraccounts_t;


class UserAccountsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    UserAccountsModel(QObject* parent);
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    QModelIndex parent ( const QModelIndex & index ) const;
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

    void addRegUser(const UserAccount& user, bool do_reset);
    void delRegUser(int index);
    void delRegUser(const QString& username);
    const useraccounts_t& getUsers() const { return m_users; }
private:
    useraccounts_t m_users;
};

enum UserAccountsDisplay
{
    UAD_READWRITE,
    UAD_READONLY,
};

class UserAccountsDlg : public QDialog
{
    Q_OBJECT

public:
    UserAccountsDlg(const useraccounts_t& useraccounts, UserAccountsDisplay uad, QWidget* parent = 0);

public slots:
    void slotCmdSuccess(int cmdid);
    void slotCmdError(int error, int cmdid);

private:
    Ui::UserAccountsDlg ui;
    UserAccountsModel* m_model;
    int m_add_cmdid, m_del_cmdid;

    UserAccount m_add_user;
    QString m_del_username;

    AbusePrevention m_abuse;
    void lockUI(bool locked);
    void showUserAccount(const UserAccount& useraccount);
    void updateUserRights(const UserAccount& useraccount);
    UserAccountsDisplay m_uad;

private slots:
    void slotClearUser();
    void slotAddUser();
    void slotDelUser();
    void slotUserSelected(const QModelIndex & index );
    void slotEdited(const QString&);
    void slotUserTypeChanged();
    void slotCustomCmdLimit(int index);

    void slotAddOpChannel();
    void slotRemoveOpChannel();

    void slotUsernameChanged(const QString& text);
};

#endif
