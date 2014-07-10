/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 4 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
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

class UserAccountsDlg : public QDialog
{
    Q_OBJECT

public:
    UserAccountsDlg(QWidget * parent = 0);

public slots:
    void slotCmdSuccess(int cmdid);
    void slotCmdError(int error, int cmdid);

private:
    Ui::UserAccountsDlg ui;
    UserAccountsModel* m_model;
    int m_add_cmdid, m_del_cmdid;

    UserAccount m_add_user;
    QString m_del_username;

    void lockUI(bool locked);
    void updateUserRights(const UserAccount& useraccount);

private slots:
    void slotClearUser();
    void slotAddUser();
    void slotDelUser();
    void slotUserSelected(const QModelIndex & index );
    void slotEdited(const QString&);
    void slotUserTypeChanged();

    void slotAddOpChannel();
    void slotRemoveOpChannel();
};

#endif
