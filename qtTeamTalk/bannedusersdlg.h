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

#ifndef BANNEDUSERSDLG_H
#define BANNEDUSERSDLG_H

#include <QAbstractItemModel>
#include <QVector>

#include "ui_bannedusers.h"

#include "common.h"

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

private:
    Ui::BannedUsersDlg ui;
    BannedUsersModel* m_bannedmodel, *m_unbannedmodel;
    QString m_chanpath;

private slots:
    void slotClose();
    void slotUnbanUser();
    void slotBanUser();
    void slotNewBan();
};

#endif
