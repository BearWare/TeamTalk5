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

public slots:
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
