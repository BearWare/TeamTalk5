/*
 * Copyright (c) 2005-2016, BearWare.dk
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

#include "channelsmodel.h"

#include <QSet>
#include <QStack>

#define CHANNEL_ITEM 0x10000
#define USER_ITEM 0x20000
#define ITEM_TYPE 0xFFFF0000
#define ITEM_DATA 0xFFFF

extern TTInstance* ttInst;


int ChannelsModel::columnCount ( const QModelIndex & /*parent = QModelIndex()*/ ) const
{
    return 1;
}

int ChannelsModel::rowCount ( const QModelIndex & parent/* = QModelIndex()*/ ) const
{
    if(parent.isValid())
    {
        if( (parent.internalId() & CHANNEL_ITEM) == 0 )
            return 0;
        int parentid = (parent.internalId() & ITEM_DATA);
        channels_t::const_iterator chan_ite = m_channels.find(parentid);
        Q_ASSERT(chan_ite != m_channels.end());
        users_t::const_iterator user_ite = m_users.find(parentid);
        Q_ASSERT(user_ite != m_users.end());
        return chan_ite.value().size() + user_ite.value().size();
    }
    if(m_rootchannelid != 0)
        return 1;
    return 0;
}

QVariant ChannelsModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(index.isValid());
    switch(role)
    {
    case Qt::DisplayRole :
        if(index.internalId() & CHANNEL_ITEM)
        {
            int channelid = index.internalId() & ITEM_DATA;

            if(m_rootchannelid == channelid)
            {
                //make server servername appear as the root channel name
                ServerProperties prop = {0};
                TT_GetServerProperties(ttInst, &prop);
                return _Q(prop.szServerName);
            }

            mchannels_t::const_iterator p_ite = m_channelparent.find(channelid);
            Q_ASSERT(p_ite != m_channelparent.end());
            channels_t::const_iterator c_ite = m_channels.find(p_ite.value());
            Q_ASSERT(c_ite != m_channels.end());
            const subchannels_t& subs = c_ite.value();
            subchannels_t::const_iterator ite = subs.begin();
            while(ite != subs.end())
            {
                if(ite->nChannelID == channelid)
                    return _Q(ite->szName);
                ite++;
            }
            Q_ASSERT(ite != subs.end());
            return QVariant();
        }
        else if(index.internalId() & USER_ITEM)
        {
            int userid = (index.internalId() & ITEM_DATA);
            musers_t::const_iterator p_ite = m_userparent.find(userid);
            Q_ASSERT(p_ite != m_userparent.end());
            int channelid = p_ite.value();
            users_t::const_iterator u_ite = m_users.find(channelid);
            Q_ASSERT(u_ite != m_users.end());
            const chanusers_t& users = u_ite.value();
            chanusers_t::const_iterator ite = users.begin();
            while(ite != users.end())
            {
                if(ite->nUserID == userid)
                    return _Q(ite->szNickname);
            }
            return QVariant();
        }
        else Q_ASSERT(0);
        break;
    case Qt::DecorationRole:
        /*
        if(index.internalId() & CHANNEL_ITEM)
            ;//return channel QPixmap
        else if(index.internalId() & USER_ITEM)
            ;//return user QPixmap
            */
        break;
    case Qt::UserRole :
    case Qt::FontRole :
        break;
    }
    return QVariant();
}

QModelIndex ChannelsModel::index ( int row, int column, const QModelIndex & parent/* = QModelIndex() */) const
{
    if(m_rootchannelid == 0)
        return QModelIndex();

    if(!parent.isValid())
        return createIndex(0, 0, (m_rootchannelid | CHANNEL_ITEM));
    Q_ASSERT(parent.internalId() & CHANNEL_ITEM);
    int parentid = (parent.internalId() & ITEM_DATA);
    channels_t::const_iterator chan_ite = m_channels.find(parentid);
    if(row < chan_ite.value().size())
        return createIndex(row, column, chan_ite.value()[row].nChannelID | CHANNEL_ITEM);

    int subs = chan_ite.value().size();
    users_t::const_iterator user_ite = m_users.find(parentid);
    Q_ASSERT(row - subs >= 0);
    if(row - subs < user_ite.value().size())
        return createIndex(row, column, user_ite.value()[row-subs].nUserID | USER_ITEM);

    Q_ASSERT(0);
    return QModelIndex();
}

QModelIndex ChannelsModel::parent ( const QModelIndex & index ) const
{
    Q_ASSERT(index.isValid());
    if(index.internalId() & CHANNEL_ITEM)
    {
        int channelid = (index.internalId() & ITEM_DATA);
        mchannels_t::const_iterator p_ite = m_channelparent.find(channelid);
        Q_ASSERT(p_ite != m_channelparent.end());
        int parentid = p_ite.value();
        channels_t::const_iterator chan_ite = m_channels.find(parentid);
        if(chan_ite == m_channels.end() || parentid == 0)
            return QModelIndex();
        int row = getRowNumber(parentid);
        Q_ASSERT(row>=0);
        return createIndex(row, 0, parentid | CHANNEL_ITEM);
    }

    if(index.internalId() & USER_ITEM)
    {
        int userid = (index.internalId() & ITEM_DATA);
        musers_t::const_iterator p_ite = m_userparent.find(userid);
        Q_ASSERT(p_ite != m_userparent.end());
        int parentid = p_ite.value();
        int row = getRowNumber(parentid);
        Q_ASSERT(row>=0);
        return createIndex(row, 0, parentid | CHANNEL_ITEM);
    }
    return QModelIndex();
}

int ChannelsModel::getRowNumber(int find_channelid) const
{
    if(m_rootchannelid == find_channelid)
        return 0;

    int row = 0;
    QStack<int> channels;
    channels.push(m_rootchannelid);

    while(channels.size())
    {
        int channelid = channels.pop();
        channels_t::const_iterator ite = m_channels.find(channelid);
        Q_ASSERT(ite != m_channels.end());
        const subchannels_t& subs = ite.value();
        subchannels_t::const_iterator sub_ite = subs.begin();
        while(sub_ite != subs.end())
        {
            row++;
            if(sub_ite->nChannelID == find_channelid)
                return row;
            channels.push(sub_ite->nChannelID);
            sub_ite++;
        }
        users_t::const_iterator p_ite = m_users.find(channelid);
        Q_ASSERT(p_ite != m_users.end());
        const chanusers_t& users = p_ite.value();
        row += users.size();
    }
    return -1;
}

void ChannelsModel::slotAddChannel(int channelid)
{
    Channel chan = {0};
    if(!TT_GetChannel(ttInst, channelid, &chan))
        return;

    if(channelid = TT_GetRootChannelID(ttInst))
    {
        m_rootchannelid = channelid;
        m_channels.insert(0, subchannels_t()); //virtual channel to simulate root
    }

    channels_t::iterator ite = m_channels.find(chan.nParentID);
    if(ite != m_channels.end())
    {
        subchannels_t& subs = ite.value();
        subchannels_t::iterator sub_ite = subs.begin();
        while(sub_ite != subs.end())
        {
            if(wcscmp(chan.szName, sub_ite->szName)<0)
                break;
            sub_ite++;
        }
        subs.insert(sub_ite, chan);
    }

    m_channels.insert(chan.nChannelID, subchannels_t());
    m_channelparent.insert(chan.nChannelID, chan.nParentID);
    m_users.insert(chan.nChannelID, chanusers_t());

    reset();
}

void ChannelsModel::slotUpdateChannel(int channelid)
{
    Channel chan = {0};
    if(!TT_GetChannel(ttInst, channelid, &chan))
        return;

    mchannels_t::const_iterator p_ite = m_channelparent.find(channelid);
    Q_ASSERT(p_ite != m_channelparent.end());
    channels_t::iterator c_ite = m_channels.find(p_ite.value());
    Q_ASSERT(c_ite != m_channels.end());
    subchannels_t& subs = c_ite.value();
    subchannels_t::iterator ite = subs.begin();
    while(ite != subs.end())
    {
        if(ite->nChannelID == channelid)
        {
            ite = subs.erase(ite);
            subs.insert(ite, chan);
            break;
        }
        ite++;
    }

    reset();
}

void ChannelsModel::slotRemoveChannel(int channelid)
{    
    //erase users
    users_t::const_iterator c_ite = m_users.find(channelid);
    const chanusers_t& users = c_ite.value();
    chanusers_t::const_iterator ite = users.begin();
    while(ite != users.end())
    {
        m_userparent.remove(ite->nUserID);
        ite++;
    }
    m_users.remove(channelid);

    //remove from subchannels
    mchannels_t::iterator p_ite = m_channelparent.find(channelid);
    Q_ASSERT(p_ite != m_channelparent.end());
    if(p_ite.value() != 0)
    {
        channels_t::iterator c_ite = m_channels.find(p_ite.value());
        subchannels_t& subs = c_ite.value();
        subchannels_t::iterator ite = subs.begin();
        while(ite != subs.end())
        {
            if(ite->nChannelID == channelid)
            {
                subs.erase(ite);
                break;
            }
            ite++;
        }
    }
    Q_ASSERT(m_channels.find(channelid) == m_channels.end());

    m_channelparent.erase(p_ite);
    if(channelid = m_rootchannelid)
        m_rootchannelid = 0;

    reset();
}

void ChannelsModel::slotAddUser(int userid, int channelid)
{
    User user = {0};
    if(!TT_GetUser(ttInst, userid, &user))
        return;

    users_t::iterator c_ite = m_users.find(channelid);
    Q_ASSERT(c_ite != m_users.end());
    chanusers_t& users = c_ite.value();
    chanusers_t::iterator ite = users.begin();

    while(ite != users.end())
    {
        if(wcscmp(user.szNickname, ite->szNickname)<0)
            break;
        ite++;
    }
    users.insert(ite, user);

    m_userparent.insert(userid, channelid);
    reset();
}

void ChannelsModel::slotUpdateUser(int userid, int channelid)
{
    User user = {0};
    if(!TT_GetUser(ttInst, userid, &user) || channelid == 0)
        return;

    users_t::iterator c_ite = m_users.find(channelid);
    Q_ASSERT(c_ite != m_users.end());
    chanusers_t& users = c_ite.value();
    chanusers_t::iterator u_ite = users.begin();
    while(u_ite != users.end())
    {
        if(u_ite->nUserID == userid)
        {
            u_ite = users.erase(u_ite);
            users.insert(u_ite, user);
            break;
        }
        u_ite++;
    }
    reset();
}

void ChannelsModel::slotRemoveUser(int userid, int channelid)
{
    users_t::iterator c_ite = m_users.find(channelid);
    Q_ASSERT(c_ite != m_users.end());
    chanusers_t& users = c_ite.value();
    chanusers_t::iterator u_ite = users.begin();
    while(u_ite != users.end())
    {
        if(u_ite->nUserID == userid)
        {
            users.erase(u_ite);
            break;
        }
        u_ite++;
    }
    m_userparent.remove(userid);
    reset();
}
