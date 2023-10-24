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

#ifndef BANNEDUSERSDLG_H
#define BANNEDUSERSDLG_H

#include "ui_bannedusers.h"

#include "common.h"

#include <QAbstractItemModel>
#include <QVector>
#include <QSortFilterProxyModel>

typedef QVector<BannedUser> bannedusers_t;

class BannedUsersModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    BannedUsersModel(QObject* parent);
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    QModelIndex parent ( const QModelIndex & index ) const;
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

    void addBannedUser(const BannedUser& user, bool do_reset);
    void delBannedUser(int index);
    const bannedusers_t& getUsers() const { return m_users; }
private:
    bannedusers_t m_users;
};

class BannedUsersDlg : public QDialog
{
    Q_OBJECT

public:
    BannedUsersDlg(const bannedusers_t& bannedusers, const QString& chanpath, QWidget * parent = 0);
    ~BannedUsersDlg();

private:
    Ui::BannedUsersDlg ui;
    BannedUsersModel* m_bannedmodel, *m_unbannedmodel;
    QSortFilterProxyModel* m_bannedproxy, *m_unbannedproxy;
    QString m_chanpath;

private:
    void slotClose();
    void slotUnbanUser();
    void slotBanUser();
    void slotNewBan();
};

#endif
