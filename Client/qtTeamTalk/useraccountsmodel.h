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

#ifndef USERACCOUNTSMODEL_H
#define USERACCOUNTSMODEL_H

#include "common.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

enum
{
    COLUMN_INDEX_USERNAME,
    COLUMN_INDEX_PASSWORD,
    COLUMN_INDEX_USERTYPE,
    COLUMN_INDEX_NOTE,
    COLUMN_INDEX_CHANNEL,
    COLUMN_INDEX_MODIFIED,
    COLUMN_COUNT_USERACCOUNTS,
};

class UserAccountsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    UserAccountsModel(QObject* parent);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex & index) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    void addRegUser(const UserAccount& user, bool do_reset);
    void delRegUser(int index);
    void delRegUser(const QString& username);
    const useraccounts_t& getUsers() const { return m_users; }
private:
    useraccounts_t m_users;
};

class UserRightsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    UserRightsModel(QObject* parent);
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex & index) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    void setUserRights(UserTypes usertypes, UserRights rights);
    UserRights getUserRights() const;

private:
    void insertUserRights();
    void itemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    QVector<UserRight> m_userrights;
    UserRights m_activeUserRights = USERRIGHT_DEFAULT;
};

#endif // USERACCOUNTSMODEL_H
