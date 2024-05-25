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

#ifndef USERACCOUNTSMODEL_H
#define USERACCOUNTSMODEL_H

#include "utiltt.h"

#include <QAbstractTableModel>
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

class UserAccountsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    UserAccountsModel(QObject* parent);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex & index) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;

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
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex & index) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    void setUserRights(UserTypes usertypes, UserRights rights);
    UserRights getUserRights() const;

private:
    void insertUserRights();
    void itemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    QVector<UserRight> m_userrights;
    UserRights m_activeUserRights = USERRIGHT_DEFAULT;
};

#endif // USERACCOUNTSMODEL_H
