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

#ifndef CHANNELSMODEL_H
#define CHANNELSMODEL_H

#include <QAbstractItemModel>
#include <QMap>
#include <QVector>
#include <QList>

#include "mainwindow.h"

class ChannelsModel : public QAbstractItemModel
{
	Q_OBJECT

public:
    ChannelsModel(QObject* parent) : QAbstractItemModel(parent), m_rootchannelid(0){}
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    QModelIndex parent ( const QModelIndex & index ) const;

public:
    void slotAddChannel(int channelid);
    void slotUpdateChannel(int channelid);
    void slotRemoveChannel(int channelid);
    void slotAddUser(int userid, int channelid);
    void slotUpdateUser(int userid, int channelid);
    void slotRemoveUser(int userid, int channelid);

private:
    int getRowNumber(int find_channelid) const;
        
    int m_rootchannelid;

    typedef QList<Channel> subchannels_t;
    //channelid -> sub-channels
    typedef QMap<int, subchannels_t> channels_t;
    channels_t m_channels;

    typedef QList<User> chanusers_t;
    //channelid -> users
    typedef QMap<int, chanusers_t> users_t;
    users_t m_users;

    //userid -> parent channelid (for fast access)
    typedef QMap<int,int> musers_t;
    musers_t m_userparent;
    //channelid -> parent channelid (for fast access)
    typedef QMap<int,int> mchannels_t;
    mchannels_t m_channelparent;
};

#endif
