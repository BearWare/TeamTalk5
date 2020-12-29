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

#include "channelstree.h"
#include <QDebug>
#include <QStack>
#include <QHeaderView>
#include <QBitmap>
#include <QPaintEngine>
#include <QTimerEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QUrl>
#include <QDir>
#include <QDrag>
#include <QMimeData>

#include "appinfo.h"
#include "settings.h"

//lower 16 bits is channel id
#define ID_MASK 0xFFFF
#define CHANNEL_TYPE 0x10000
#define USER_TYPE 0x20000
#define MESSAGED_TYPE 0x40000

enum
{
    COLUMN_ITEM,
    COLUMN_CHANMSG,
    COLUMN_VOICE,
    COLUMN_VIDEO,
    COLUMN_DESKTOP,
    COLUMN_MEDIAFILE,
};

extern TTInstance* ttInst;
extern QSettings* ttSettings;
       
#define COLOR_TALK      QColor(133,229,141)
#define COLOR_LASTTALK  QColor(255,232,61)
#define COLOR_LOSSY     QColor(201,2,40)

#define COLOR_TX_VOICE  QColor(130,209,255) // QColor(133,229,141)
#define COLOR_TX_VIDEO  QColor(130,209,255)

const char userMimeType[] = "application/user";

bool userCanTx(int userid, StreamTypes stream_type, const Channel& chan)
{
    int i=0;
    while(i<TT_TRANSMITUSERS_MAX && chan.transmitUsers[i][TT_TRANSMITUSERS_USERID_INDEX])
    {
        if(chan.transmitUsers[i][TT_TRANSMITUSERS_USERID_INDEX] == userid && (chan.transmitUsers[i][TT_TRANSMITUSERS_STREAMTYPE_INDEX] & stream_type))
            return (chan.uChannelType & CHANNEL_CLASSROOM) == CHANNEL_CLASSROOM;
        else i++;
    }
    return (chan.uChannelType & CHANNEL_CLASSROOM) == CHANNEL_DEFAULT;
}

bool userCanChanMessage(int userid, const Channel& chan)
{
    return userCanTx(userid, STREAMTYPE_CHANNELMSG, chan);
}

bool userCanVoiceTx(int userid, const Channel& chan)
{
    return userCanTx(userid, STREAMTYPE_VOICE, chan);
}

bool userCanVideoTx(int userid, const Channel& chan)
{
    return userCanTx(userid, STREAMTYPE_VIDEOCAPTURE, chan);
}

bool userCanDesktopTx(int userid, const Channel& chan)
{
    return userCanTx(userid, STREAMTYPE_DESKTOP, chan);
}

bool userCanMediaFileTx(int userid, const Channel& chan)
{
    return userCanTx(userid, STREAMTYPE_MEDIAFILE, chan);
}

channels_t getSubChannels(int channelid, const channels_t& channels, bool recursive /*= false*/)
{
    channels_t subchannels;
    for(auto ite = channels.begin(); ite != channels.end(); ++ite)
    {
        if(ite.value().nParentID == channelid)
        {
            subchannels[ite.key()] = ite.value();
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
            if(recursive)
                subchannels.unite(getSubChannels(ite.value().nChannelID, channels, recursive));
#else
                subchannels.insert(getSubChannels(ite.value().nChannelID, channels, recursive));
#endif
        }
    }
    return subchannels;
}

channels_t getParentChannels(int channelid, const channels_t& channels)
{
    channels_t parents;
    while(channels[channelid].nParentID>0)
    {
        parents[channels[channelid].nParentID] = channels[channelid];
        channelid = channels[channelid].nParentID;
    }
    return parents;
}

users_t getChannelUsers(int channelid, const users_t& users)
{
    users_t result;
    for(auto ite=users.begin();ite!=users.end();ite++)
    {
        if(ite.value().nChannelID == channelid)
            result.insert(ite.key(), ite.value());
    }
    return result;
}

bool isFreeForAll(StreamTypes stream_type, const int transmitUsers[][2],
                  int max_userids = TT_TRANSMITUSERS_MAX)
{
    int i=0;
    while(i<max_userids && transmitUsers[i][TT_TRANSMITUSERS_USERID_INDEX] != 0)
    {
        if(transmitUsers[i][TT_TRANSMITUSERS_USERID_INDEX] == TT_CLASSROOM_FREEFORALL &&
           (transmitUsers[i][TT_TRANSMITUSERS_STREAMTYPE_INDEX] & stream_type))
            return true;
        i++;
    }
    return false;
}

ChannelsTree::ChannelsTree(QWidget* parent)
: QTreeWidget(parent)
, m_showusercount(SETTINGS_DISPLAY_USERSCOUNT_DEFAULT)
, m_showlasttalk(SETTINGS_DISPLAY_LASTTALK_DEFAULT)
, m_last_talker_id(0)
, m_strlen(SETTINGS_DISPLAY_MAX_STRING_DEFAULT)
, m_desktopaccesTimerId(0)
, m_ignore_item_changes(false)
{
#if QT_VERSION >= 0x050000
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    header()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    setAcceptDrops(true);

    //connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
    //        SLOT(slotItemDoubleClicked(QTreeWidgetItem*, int)));
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            SLOT(slotItemChanged(QTreeWidgetItem*, int)));
    connect(this, SIGNAL(itemActivated(QTreeWidgetItem*, int)),
            SLOT(slotItemDoubleClicked(QTreeWidgetItem*, int)));
    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)),
            SLOT(slotUpdateTreeWidgetItem(QTreeWidgetItem*)));
    connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem*)),
            SLOT(slotUpdateTreeWidgetItem(QTreeWidgetItem*)));

    m_statTimerId = startTimer(500);
    m_questionTimerId = startTimer(1000);
}

int ChannelsTree::selectedChannel(bool include_user/* = false*/) const
{
    QTreeWidgetItem* item = currentItem();
    if(!item)
        return 0;

    if(item->type() & CHANNEL_TYPE)
        return (item->data(COLUMN_ITEM, Qt::UserRole).toInt() & ID_MASK);

    if(include_user && (item->type() & USER_TYPE))
    {
        users_t::const_iterator ite = m_users.find((item->data(COLUMN_ITEM, Qt::UserRole).toInt() & ID_MASK));
        if(ite != m_users.end())
            return ite->nChannelID;
    }
    return 0;
}

bool ChannelsTree::getSelectedChannel(Channel& chan) const
{
    int chanid = selectedChannel();
    if(chanid)
        return getChannel(chanid, chan);
    return false;
}

QVector<int> ChannelsTree::selectedChannels() const
{
    QVector<int> channels;
    QList<QTreeWidgetItem*> items = selectedItems();
    for(int i=0;i<items.size();i++)
        if(items[i]->type() & CHANNEL_TYPE)
            channels.push_back((items[i]->data(COLUMN_ITEM, Qt::UserRole).toInt() & ID_MASK));
    return channels;
}

bool ChannelsTree::getChannel(int channelid, Channel& chan) const
{
    channels_t::const_iterator ite = m_channels.find(channelid);
    if(ite != m_channels.end())
    {
        chan = *ite;
        return true;
    }
    return false;
}

int ChannelsTree::selectedUser() const
{
    QTreeWidgetItem* item = currentItem();
    if(!item || (item->type() & CHANNEL_TYPE) )
        return 0;
    return item->data(COLUMN_ITEM, Qt::UserRole).toInt() & ID_MASK;
}

QVector<int> ChannelsTree::selectedUsers() const
{
    QVector<int> users;
    QList<QTreeWidgetItem*> items = selectedItems();
    for(int i=0;i<items.size();i++)
        if(items[i]->type() & USER_TYPE)
            users.push_back((items[i]->data(COLUMN_ITEM, Qt::UserRole).toInt()) & ID_MASK);
    return users;
}

QVector<User> ChannelsTree::getSelectedUsers() const
{
    QVector<User> users;
    foreach(int userid, selectedUsers())
    {
        User u;
        if(getUser(userid, u))
            users.push_back(u);
    }
    return users;
}

bool ChannelsTree::getUser(int userid, User& user) const
{
    users_t::const_iterator ite = m_users.find(userid);
    if(ite != m_users.end())
    {
        user = *ite;
        return true;
    }
    return false;
}

bool ChannelsTree::getSelectedUser(User& user) const
{
    int userid = selectedUser();
    if(userid)
        return getUser(userid, user);
    return false;
}

QVector<int> ChannelsTree::getUsersInChannel(int channelid) const
{
    QVector<int> result;
    users_t::const_iterator ite;
    for(ite = m_users.begin();ite!=m_users.end();ite++)
        if(ite->nChannelID == channelid)
            result.push_back(ite->nUserID);
    return result;
}

QVector<int> ChannelsTree::getUsers() const
{
    QVector<int> result;
    users_t::const_iterator ite;
    for(ite = m_users.begin();ite!=m_users.end();ite++)
        result.push_back(ite->nUserID);
    return result;
}

users_t ChannelsTree::getUsers(int channelid) const
{
    users_t users;
    for(auto i=m_users.begin();i!=m_users.end();++i)
    {
        if(channelid == 0 || i.value().nChannelID == channelid)
            users[i.key()] = i.value();
    }
    return users;
}

void ChannelsTree::getTransmitUsers(int channelid,
                                     QMap<int, StreamTypes>& transmitUsers)
{
    QTreeWidgetItem* parent = getChannelItem(channelid);
    if(!parent)
        return;
    Q_ASSERT(parent->data(COLUMN_ITEM, Qt::UserRole).toInt() == channelid);

    transmitUsers[TT_CLASSROOM_FREEFORALL] = STREAMTYPE_NONE;
    if (parent->checkState(COLUMN_CHANMSG) == Qt::Checked)
        transmitUsers[TT_CLASSROOM_FREEFORALL] |= STREAMTYPE_CHANNELMSG;
    if (parent->checkState(COLUMN_VOICE) == Qt::Checked)
        transmitUsers[TT_CLASSROOM_FREEFORALL] |= STREAMTYPE_VOICE;
    if (parent->checkState(COLUMN_VIDEO) == Qt::Checked)
        transmitUsers[TT_CLASSROOM_FREEFORALL] |= STREAMTYPE_VIDEOCAPTURE;
    if (parent->checkState(COLUMN_DESKTOP) == Qt::Checked)
        transmitUsers[TT_CLASSROOM_FREEFORALL] |= STREAMTYPE_DESKTOP;
    if (parent->checkState(COLUMN_MEDIAFILE) == Qt::Checked)
        transmitUsers[TT_CLASSROOM_FREEFORALL] |= STREAMTYPE_MEDIAFILE;
    
    int childCount = parent->childCount();
    QTreeWidgetItem* child;
    for(int i=0;i<childCount;i++)
    {
        child = parent->child(i);
        Q_ASSERT(child);
        if(child->type() != USER_TYPE)
            continue;
        if ((bool)child->checkState(COLUMN_CHANMSG))
            transmitUsers[child->data(COLUMN_ITEM, Qt::UserRole).toInt()] |= STREAMTYPE_CHANNELMSG;
        if ((bool)child->checkState(COLUMN_VOICE))
            transmitUsers[child->data(COLUMN_ITEM, Qt::UserRole).toInt()] |= STREAMTYPE_VOICE;
        if ((bool)child->checkState(COLUMN_VIDEO))
            transmitUsers[child->data(COLUMN_ITEM, Qt::UserRole).toInt()] |= STREAMTYPE_VIDEOCAPTURE;
        if ((bool)child->checkState(COLUMN_DESKTOP))
            transmitUsers[child->data(COLUMN_ITEM, Qt::UserRole).toInt()] |= STREAMTYPE_DESKTOP;
        if ((bool)child->checkState(COLUMN_MEDIAFILE))
            transmitUsers[child->data(COLUMN_ITEM, Qt::UserRole).toInt()] |= STREAMTYPE_MEDIAFILE;
    }
}

void ChannelsTree::reset()
{
    delete topLevelItem(0);
    m_channels.clear();
    m_users.clear();   
    m_videousers.clear();
    m_desktopaccess_users.clear();
    m_blinkhand_users.clear();
    m_blinkchalk_users.clear();
    m_stats.clear();
    if(m_desktopaccesTimerId)
    {
        killTimer(m_desktopaccesTimerId);
        m_desktopaccesTimerId = 0;
    }
}

void ChannelsTree::setChannelMessaged(int /*channelid*/, bool /*messaged*/)
{
}

void ChannelsTree::setUserMessaged(int userid, bool messaged)
{
    QTreeWidgetItem* item = getUserItem(userid);
    if(item)
    {
        int data = item->data(COLUMN_ITEM, Qt::UserRole).toInt();
        item->setData(COLUMN_ITEM, Qt::UserRole, messaged?data|MESSAGED_TYPE:data&~MESSAGED_TYPE);
        data = item->data(COLUMN_ITEM, Qt::UserRole).toInt();
        slotUpdateTreeWidgetItem(item);
    }
}

void ChannelsTree::setUserDesktopAccess(int userid, bool enable)
{
    if(enable)
        m_desktopaccess_users.insert(userid);
    else
    {
        m_desktopaccess_users.remove(userid);
        m_blinkchalk_users.remove(userid);
    }
    if(m_desktopaccess_users.size() && m_desktopaccesTimerId == 0)
        m_desktopaccesTimerId = startTimer(1000);
    else if(m_desktopaccess_users.empty() && m_desktopaccesTimerId)
    {
        killTimer(m_desktopaccesTimerId);
        m_desktopaccesTimerId = 0;
        
        QTreeWidgetItem* userItem = getUserItem(userid);
        if(userItem)
            slotUpdateTreeWidgetItem(userItem);
    }
}

void ChannelsTree::setShowUserCount(bool show)
{
    m_showusercount = show;
    channels_t::const_iterator ite = m_channels.begin();
    while(ite != m_channels.end())
    {
        updateChannelItem(ite.key());
        ite++;
    }
}

void ChannelsTree::setShowUsername()
{
    users_t::const_iterator i = m_users.begin();
    while(i != m_users.end())
    {
        QTreeWidgetItem* item = getUserItem(i.key());
        if(item)
            slotUpdateTreeWidgetItem(item);
        ++i;
    }
}

void ChannelsTree::setShowLastToTalk(bool show)
{
    m_showlasttalk = show;
    QTreeWidgetItem* user_item = getUserItem(m_last_talker_id);
    if(user_item)
        slotUpdateTreeWidgetItem(user_item);
}

void ChannelsTree::updateItemTextLength(int new_length)
{
    if(m_strlen == new_length)
        return;
    m_strlen = new_length;

    QTreeWidgetItem* item;
    channels_t::const_iterator ic = m_channels.begin();
    while(ic != m_channels.end())
    {
        item = getChannelItem(ic.key());
        if(item)
            slotUpdateTreeWidgetItem(item);
        ic++;
    }
    users_t::iterator iu = m_users.begin();
    while(iu != m_users.end())
    {
        item = getUserItem(iu.key());
        if(item)
            slotUpdateTreeWidgetItem(item);
        iu++;
    }
}

void ChannelsTree::timerEvent(QTimerEvent* event)
{
    QTreeWidget::timerEvent(event);

    if(event->timerId() == m_statTimerId)
    {
        UserStatistics stats;
        QTreeWidgetItem* userItem;
        statistics_t::iterator ii = m_stats.begin();
        for(;ii != m_stats.end();ii++)
        {
            int userid = ii.key();
            if(!TT_GetUserStatistics(ttInst, userid, &stats))
                continue;
            const UserStatistics& tmp = ii.value();
            int audlost = stats.nVoicePacketsLost - tmp.nVoicePacketsLost;
            int audrecv = stats.nVoicePacketsRecv - tmp.nVoicePacketsRecv;
            int vidlost = stats.nVideoCaptureFramesLost - tmp.nVideoCaptureFramesLost;
            int vidrecv = stats.nVideoCaptureFramesRecv - tmp.nVideoCaptureFramesRecv;
            *ii = stats;

            float audloss_pct = 0.0f;
            if(audrecv)
                audloss_pct = (float)audlost / (float)audrecv;
            float vidloss_pct = 0.0f;
            if(vidrecv)
                vidloss_pct = (float)vidlost / (float)vidrecv;

            userItem = getUserItem(userid);
            if(userItem)
            {
                bool update_item = false;
                if(audloss_pct >= .1f || vidloss_pct >= .1f)
                {
                    if(userItem->background(COLUMN_ITEM) != COLOR_LOSSY)
                        userItem->setBackground(COLUMN_ITEM, COLOR_LOSSY);
                    else
                        update_item = true;
                }
                else if(userItem->background(COLUMN_ITEM) == COLOR_LOSSY)
                    update_item = true;

                if(vidrecv)
                {
                    if(m_videousers.find(userid) == m_videousers.end())
                    {
                        m_videousers.insert(userid);
                        update_item = true;
                    }
                }
                else if(m_videousers.find(userid) != m_videousers.end())
                {
                    m_videousers.remove(userid);
                    update_item = true;
                }
                if(update_item)
                    slotUpdateTreeWidgetItem(userItem);
            }
        }
    }
    else if(event->timerId() == m_desktopaccesTimerId)
    {
        //find users in desktop access mode so they can blink
        QTreeWidgetItem* userItem;
        QSet<int>::const_iterator ite = m_desktopaccess_users.begin();
        while(ite != m_desktopaccess_users.end())
        {
            if(m_blinkchalk_users.find(*ite) == m_blinkchalk_users.end())
                m_blinkchalk_users.insert(*ite);
            else
                m_blinkchalk_users.remove(*ite);

            userItem = getUserItem(*ite);
            if(userItem)
                slotUpdateTreeWidgetItem(userItem);

            ite++;
        }
    }
    else if(event->timerId() == m_questionTimerId)
    {
        //find users in question mode so they can blink
        QTreeWidgetItem* userItem;
        users_t::const_iterator ite = m_users.begin();
        while(ite != m_users.end())
        {
            const User& user = ite.value();
            if((user.nStatusMode & STATUSMODE_MODE) == STATUSMODE_QUESTION)
            {
                if(m_blinkhand_users.find(user.nUserID) == m_blinkhand_users.end())
                    m_blinkhand_users.insert(user.nUserID);
                else
                    m_blinkhand_users.remove(user.nUserID);

                userItem = getUserItem(user.nUserID);
                if(userItem) //may not be in a channel
                    slotUpdateTreeWidgetItem(userItem);
            }
            else if(m_blinkhand_users.find(user.nUserID) != m_blinkhand_users.end())
            {
                m_blinkhand_users.remove(user.nUserID);
                userItem = getUserItem(user.nUserID);
                if(userItem) //may not be in a channel
                    slotUpdateTreeWidgetItem(userItem);
            }
            ite++;
        }
    }
}

void ChannelsTree::mousePressEvent(QMouseEvent *event)
{
    QTreeWidget::mousePressEvent(event);
    m_dragStartPos = event->pos();
}

void ChannelsTree::mouseMoveEvent(QMouseEvent *event)
{
    QTreeWidget::mouseMoveEvent(event);

    if (!(event->buttons() & Qt::LeftButton))
        return;

    QTreeWidgetItem* item = itemAt(m_dragStartPos);
    if(!item || item->type() != USER_TYPE)
        return;

    if ((event->pos() - m_dragStartPos).manhattanLength()
        < QApplication::startDragDistance())
        return;

     QDrag *drag = new QDrag(this);
     QMimeData *mimeData = new QMimeData;
     mimeData->setText(item->text(COLUMN_ITEM));
     mimeData->setData(userMimeType, QByteArray());
     drag->setMimeData(mimeData);
     QPixmap img = item->data(COLUMN_ITEM, Qt::DecorationRole).value<QPixmap>();
     drag->setPixmap(img);
     drag->exec(Qt::MoveAction);
}

void ChannelsTree::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->urls().size() && 
       event->mimeData()->urls()[0].toLocalFile().right(QString(TTFILE_EXT).size()) == TTFILE_EXT)
    {
        QString filepath = QDir::toNativeSeparators(event->mimeData()->urls()[0].toLocalFile());
        emit(fileDropped(filepath));
        return;
    }

    QTreeWidgetItem* item = itemAt(event->pos());
    if(!item || item->type() != CHANNEL_TYPE)
        return;
    int chanid = item->data(COLUMN_ITEM, Qt::UserRole).toInt();
    QList<QTreeWidgetItem *> items = selectedItems();
    for(int i=0;i<items.size();i++)
    {
        if(items[i]->type() == USER_TYPE)
            TT_DoMoveUser(ttInst, items[i]->data(COLUMN_ITEM, Qt::UserRole).toInt(), 
                          chanid);
    }
}

void ChannelsTree::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->urls().size() && 
       event->mimeData()->urls()[0].toLocalFile().right(QString(TTFILE_EXT).size()) == TTFILE_EXT)
    {
        event->acceptProposedAction();
        return;
    }

    if(!event->mimeData()->hasFormat(userMimeType))
        return;
    event->acceptProposedAction();
}

void ChannelsTree::dragMoveEvent(QDragMoveEvent * event)
{
    if(event->mimeData()->urls().size() && 
       event->mimeData()->urls()[0].toLocalFile().right(QString(TTFILE_EXT).size()) == TTFILE_EXT)
    {
        event->acceptProposedAction();
        return;
    }

    if(!event->mimeData()->hasFormat(userMimeType))
    {
        event->ignore();
        return;
    }
    QTreeWidgetItem* item = itemAt(event->pos());
    if(!item || item->type() != CHANNEL_TYPE)
    {
        event->ignore();
        return;
    }
    event->acceptProposedAction();
}

QTreeWidgetItem* ChannelsTree::getChannelItem(int channelid)
{
    QTreeWidgetItem* item = topLevelItem(0);
    if(!item)
        return nullptr;
    QStack<QTreeWidgetItem*> channels;
    channels.push(item);
    while(channels.size())
    {
        item = channels.pop();
        if((item->data(COLUMN_ITEM, Qt::UserRole).toInt() & ID_MASK) == channelid)
            return item;
        int count = item->childCount();
        for(int i=0;i<count;i++)
        {
            if(item->child(i)->type() & CHANNEL_TYPE)
                channels.push(item->child(i));
        }
    }
    return nullptr;
}

QTreeWidgetItem* ChannelsTree::getUserItem(int userid)
{
    QTreeWidgetItem* item = topLevelItem(0);
    if(!item)
        return nullptr;
    QStack<QTreeWidgetItem*> channels;
    channels.push(item);
    while(channels.size())
    {
        item = channels.pop();
        int count = item->childCount();
        for(int i=0;i<count;i++)
        {
            QTreeWidgetItem* child = item->child(i);
            if(child->type() & CHANNEL_TYPE)
                channels.push(child);
            else if( (child->type() & USER_TYPE) &&
                (child->data(COLUMN_ITEM, Qt::UserRole).toInt() & ID_MASK) == userid)
                return child;
        }
    }
    return nullptr;
}

int ChannelsTree::getUserIndex(const QTreeWidgetItem* parent, const QString& nick)
{
    Q_ASSERT(parent);
    int count = parent->childCount();
    int i;
    for(i=0;i<count;i++)
    {
        QTreeWidgetItem* child = parent->child(i);
        QString childName = child->data(COLUMN_ITEM, Qt::DisplayRole).toString();
        if((child->type() & CHANNEL_TYPE) || ((child->type() & USER_TYPE) &&
            nick.compare(childName, Qt::CaseInsensitive) < 0))
            break;
    }
    return i;
}

void ChannelsTree::updateChannelItem(int channelid)
{
    QTreeWidgetItem* item = getChannelItem(channelid);
    if(item)
        slotUpdateTreeWidgetItem(item);
}

void ChannelsTree::slotUpdateTreeWidgetItem(QTreeWidgetItem* item)
{
    m_ignore_item_changes = true;

    int mychanid = TT_GetMyChannelID(ttInst);
    bool emoji = ttSettings->value(SETTINGS_DISPLAY_EMOJI, SETTINGS_DISPLAY_EMOJI_DEFAULT).toBool();

    if(item->type() & CHANNEL_TYPE)
    {
        int channelid = (item->data(COLUMN_ITEM, Qt::UserRole).toInt() & ID_MASK);
        channels_t::const_iterator ite = m_channels.find(channelid);
        Q_ASSERT(ite != m_channels.end());
        if(ite == m_channels.end())
            return;

        const char* img_name = "";
        QString channame;
        if(channelid == TT_GetRootChannelID(ttInst))
        {
            //make server servername appear as the root channel name
            ServerProperties prop = {};
            TT_GetServerProperties(ttInst, &prop);
            channame += _Q(prop.szServerName);
            if(item->isExpanded())
                img_name = ":/images/images/root_open.png";
            else
                img_name = ":/images/images/root.png";
        }
        else
        {
            channame += _Q(ite->szName);
            item->setData(COLUMN_ITEM, Qt::DisplayRole, channame);
            if(item->isExpanded())
                img_name = ":/images/images/channel_open.png";
            else
                img_name = ":/images/images/channel.png";
        }

        if(channame.size()>m_strlen)
        {
            channame.resize(m_strlen);
            channame += "...";
        }

        if(m_showusercount)
        {
            int count = getChannelUsers(channelid, m_users).size();
            if(!item->isExpanded())
            {
                channels_t subs = getSubChannels(channelid, m_channels, true);
                for(auto i=subs.begin();i!=subs.end();++i)
                    count += getChannelUsers(i.key(), m_users).size();
            }
            channame = QString("%1 (%2)").arg(channame).arg(count);
        }
        if (emoji && ite->bPassword)
            channame += " - 🔒";
        item->setData(COLUMN_ITEM, Qt::DisplayRole, channame);
        QPixmap img(QString::fromUtf8(img_name));
        //img.setMask(img.createHeuristicMask());
        if(ite->bPassword)
        {
            QPixmap lock(QString::fromUtf8(":/images/images/lock.png"));
            //lock.setMask(lock.createMaskFromColor(QColor(255,255,255)));
            //lock.setMask(lock.createHeuristicMask());
            QRect r_lock = lock.rect();
            QRect r_img = img.rect();
            r_img.setLeft(r_img.width() - r_lock.width());
            r_img.setTop(r_img.height() - r_lock.height());
            QPainter p(&img);
            p.drawPixmap(r_img, lock, r_lock);
        }
        item->setData(COLUMN_ITEM, Qt::DecorationRole, img);

        //set speaker or webcam icon
        if(ite->nChannelID == mychanid)
        {
            item->setIcon(COLUMN_CHANMSG, QIcon(QString::fromUtf8(":/images/images/message_blue.png")));
            item->setIcon(COLUMN_VOICE, QIcon(QString::fromUtf8(":/images/images/speaker.png")));
            item->setIcon(COLUMN_VIDEO, QIcon(QString::fromUtf8(":/images/images/webcam.png")));
            item->setIcon(COLUMN_DESKTOP, QIcon(QString::fromUtf8(":/images/images/desktoptx.png")));
            item->setIcon(COLUMN_MEDIAFILE, QIcon(QString::fromUtf8(":/images/images/streammedia.png")));

            bool opadmin = TT_IsChannelOperator(ttInst, 
                                                TT_GetMyUserID(ttInst), 
                                                channelid);
            opadmin |= (bool)(TT_GetMyUserType(ttInst) & USERTYPE_ADMIN);

            if(opadmin && (ite->uChannelType & CHANNEL_CLASSROOM)) // free for all in non-classroom
            {
                item->setCheckState(COLUMN_CHANMSG,
                                    isFreeForAll(STREAMTYPE_CHANNELMSG, ite->transmitUsers)?
                                    Qt::Checked : Qt::Unchecked);
                item->setCheckState(COLUMN_VOICE, 
                                    isFreeForAll(STREAMTYPE_VOICE, ite->transmitUsers)?
                                    Qt::Checked : Qt::Unchecked);
                item->setCheckState(COLUMN_VIDEO, 
                                    isFreeForAll(STREAMTYPE_VIDEOCAPTURE, ite->transmitUsers)?
                                    Qt::Checked : Qt::Unchecked);
                item->setCheckState(COLUMN_DESKTOP, 
                                    isFreeForAll(STREAMTYPE_DESKTOP, ite->transmitUsers)?
                                    Qt::Checked : Qt::Unchecked);
                item->setCheckState(COLUMN_MEDIAFILE, 
                                    isFreeForAll(STREAMTYPE_MEDIAFILE, ite->transmitUsers)?
                                    Qt::Checked : Qt::Unchecked);
            }
            else
            {
                if (!item->data(COLUMN_CHANMSG, Qt::CheckStateRole).isNull())
                    item->setData(COLUMN_CHANMSG, Qt::CheckStateRole, QVariant());
                if (!item->data(COLUMN_VOICE, Qt::CheckStateRole).isNull())
                    item->setData(COLUMN_VOICE, Qt::CheckStateRole, QVariant());
                if (!item->data(COLUMN_VIDEO, Qt::CheckStateRole).isNull())
                    item->setData(COLUMN_VIDEO, Qt::CheckStateRole, QVariant());
                if (!item->data(COLUMN_DESKTOP, Qt::CheckStateRole).isNull())
                    item->setData(COLUMN_DESKTOP, Qt::CheckStateRole, QVariant());
                if (!item->data(COLUMN_MEDIAFILE, Qt::CheckStateRole).isNull())
                    item->setData(COLUMN_MEDIAFILE, Qt::CheckStateRole, QVariant());
            }            
        }
        else
        {
            if (!item->icon(COLUMN_CHANMSG).isNull())
                item->setIcon(COLUMN_CHANMSG, QIcon());
            if (!item->icon(COLUMN_VOICE).isNull())
                item->setIcon(COLUMN_VOICE, QIcon());
            if (!item->icon(COLUMN_VIDEO).isNull())
                item->setIcon(COLUMN_VIDEO, QIcon());
            if (!item->icon(COLUMN_DESKTOP).isNull())
                item->setIcon(COLUMN_DESKTOP, QIcon());
            if (!item->icon(COLUMN_MEDIAFILE).isNull())
                item->setIcon(COLUMN_MEDIAFILE, QIcon());
            if (!item->data(COLUMN_CHANMSG, Qt::CheckStateRole).isNull())
                item->setData(COLUMN_CHANMSG, Qt::CheckStateRole, QVariant());
            if (!item->data(COLUMN_VOICE, Qt::CheckStateRole).isNull())
                item->setData(COLUMN_VOICE, Qt::CheckStateRole, QVariant());
            if (!item->data(COLUMN_VIDEO, Qt::CheckStateRole).isNull())
                item->setData(COLUMN_VIDEO, Qt::CheckStateRole, QVariant());
            if (!item->data(COLUMN_MEDIAFILE, Qt::CheckStateRole).isNull())
                item->setData(COLUMN_MEDIAFILE, Qt::CheckStateRole, QVariant());
        }
    }
    else if(item->type() & USER_TYPE)
    {
        int userid = (item->data(COLUMN_ITEM, Qt::UserRole).toInt() & ID_MASK);
        users_t::const_iterator ite = m_users.find(userid);
        Q_ASSERT(ite != m_users.end());
        if(ite == m_users.end())
            return;
        const User& user = *ite;

        QTreeWidgetItem* parentItem = item->parent();
        Q_ASSERT(parentItem);
        Q_ASSERT(parentItem->type() & CHANNEL_TYPE);
        int chanid = (parentItem->data(COLUMN_ITEM, Qt::UserRole).toInt() & ID_MASK);
        channels_t::const_iterator chanIte = m_channels.find(chanid);
        Q_ASSERT(chanIte != m_channels.end());
        const Channel& chan = *chanIte;

        bool talking = false;
        if(userid != TT_GetMyUserID(ttInst))
            talking = user.uUserState & USERSTATE_VOICE;
        else
            talking = isMyselfTalking();

        bool video_active = m_videousers.find(userid) != m_videousers.end();
        video_active |= (bool)(user.nStatusMode & STATUSMODE_VIDEOTX);

        const char* user_rc;
        if(m_blinkchalk_users.find(userid) != m_blinkchalk_users.end())
            user_rc = ":/images/images/chalkstick.png";
        else if(m_blinkhand_users.find(userid) != m_blinkhand_users.end())
            user_rc = ":/images/images/hand.png";
        else if(user.nStatusMode & STATUSMODE_FEMALE)
            user_rc = ":/images/images/user_female.png";
        else
            user_rc = ":/images/images/user.png";

        QPixmap user_img(QString::fromUtf8(user_rc));
        QRect r_user = user_img.rect();

        QPixmap img(user_img.width(), user_img.height());
        img.fill(QColor(0,0,0,0)); //make transparent bg

        QPainter p(&img);

        if(video_active)
        {
            //make video frame background
            QPixmap video_img(QString::fromUtf8(":/images/images/tvframe.png"));
            p.drawPixmap(0, 0, video_img);
        }

        if(user.nStatusMode & STATUSMODE_DESKTOP)
        {
            //draw desktop in top right corner
            QPixmap dtx(QString::fromUtf8(":/images/images/desktopbg.png"));
            QRect r_dtx = dtx.rect();
            QRect r_img = img.rect();
            r_img.setLeft(r_img.width() - r_dtx.width()/* - 1*/);
            r_img.setWidth(r_dtx.width());
            //r_img.setTop(1);
            r_img.setBottom(r_dtx.height());
            p.drawPixmap(r_img, dtx, r_dtx);
        }

        if(user.uLocalSubscriptions & SUBSCRIBE_DESKTOPINPUT)
        {
            QPixmap di(QString::fromUtf8(":/images/images/chalkstickbg.png"));

            QRect r_di = di.rect();
            QRect r_img = img.rect();
            r_img.setLeft(r_img.width() - r_di.width());
            r_img.setTop(r_img.height() - r_di.height());
            p.drawPixmap(r_img, di, r_di);
        }

        if(video_active)
        {
            //move user image inside video frame
            QRect r_img = img.rect();
            r_img.setLeft(r_img.left()+1);
            r_img.setTop(r_img.top()+1);
            r_img.setBottom(r_img.bottom()-1);

            r_user.setLeft(r_user.left()+1);
            r_user.setTop(r_user.top()+1);
            r_user.setBottom(r_user.bottom()-1);
            p.drawPixmap(r_img, user_img, r_user);
        }
        else
            p.drawPixmap(img.rect(), user_img);

        if((user.nStatusMode & STATUSMODE_MODE) == STATUSMODE_AWAY)
        {
            p.setPen(QPen(QBrush(Qt::red), 2));
            p.drawLine(0, 0, img.width(), img.height());
            p.drawLine(img.width(), 0, 0, img.height());
        }

        QString itemtext;
        QString name = getDisplayName(user);
        if(emoji && item->data(COLUMN_ITEM, Qt::UserRole).toInt() & MESSAGED_TYPE)
            itemtext += "✉ ";
        itemtext += name;
        if (emoji)
        {
            switch (user.nStatusMode & STATUSMODE_MODE)
            {
            case STATUSMODE_AWAY :
                itemtext += tr(", Away");
                break;
            case STATUSMODE_QUESTION :
                itemtext += tr(", Question");
                break;
            }
            if(user.uUserState & USERSTATE_VOICE)
                itemtext += "🎤";
            if (user.nStatusMode & STATUSMODE_STREAM_MEDIAFILE)
                itemtext += tr(", Streaming media file");

            if (user.nStatusMode & STATUSMODE_VIDEOTX)
                itemtext += tr(", Webcam");
        }

        if(_Q(user.szStatusMsg).size())
            itemtext += QString(" - ") + _Q(user.szStatusMsg);
        if (emoji && (user.nStatusMode & STATUSMODE_FEMALE))
            itemtext += " 👩";
        if (emoji)
        {
            if(user.uUserType & USERTYPE_ADMIN)
                itemtext += tr(" (Administrator)");

            if(TT_IsChannelOperator(ttInst, userid, ite->nChannelID))
                itemtext += tr(" (Channel operator)");
        }

        if (itemtext.size() > m_strlen)
        {
            itemtext.resize(m_strlen);
            itemtext += "...";
        }
        item->setData(COLUMN_ITEM, Qt::DisplayRole, itemtext);

        if(user.nStatusMode & STATUSMODE_STREAM_MEDIAFILE)
        {
            QPixmap strm(QString::fromUtf8(":/images/images/stream.png"));
            QRect r_strm = strm.rect();
            QRect r_img = img.rect();
            //r_img.setRight(r_img.width() - );
            r_img.setTop(r_img.height() / 2 - r_strm.height() / 2);
            r_img.setLeft(r_img.right() - r_strm.width());
            r_img.setBottom(r_img.top() + r_strm.height());
            p.drawPixmap(r_img, strm, r_strm);
        }

        if(TT_IsChannelOperator(ttInst, userid, ite->nChannelID))
        {
            QPixmap op(QString::fromUtf8(":/images/images/op.png"));
            //op.setMask(op.createMaskFromColor(QColor(255,255,255)));
            //lock.setMask(lock.createHeuristicMask());
            QRect r_op = op.rect();
            QRect r_img = img.rect();
            r_img.setLeft(r_img.width() - r_op.width());
            r_img.setTop(r_img.height() - r_op.height());
            p.drawPixmap(r_img, op, r_op);
        }

        if(user.uUserType & USERTYPE_ADMIN)
        {
            QPixmap admin(QString::fromUtf8(":/images/images/admin.png"));
            //admin.setMask(admin.createMaskFromColor(QColor(255,255,255)));
            //lock.setMask(lock.createHeuristicMask());
            QRect r_admin = admin.rect();
            QRect r_img = img.rect();
            r_img.setLeft(r_img.width() - r_admin.width());
            r_img.setTop(r_img.height() - r_admin.height());
            p.drawPixmap(r_img, admin, r_admin);
        }

        if(item->data(COLUMN_ITEM, Qt::UserRole).toInt() & MESSAGED_TYPE)
        {
            QPixmap msg(QString::fromUtf8(":/images/images/msg.png"));
            //msg.setMask(msg.createMaskFromColor(QColor(255,255,255)));
            //lock.setMask(lock.createHeuristicMask());
            QRect r_msg = msg.rect();
            QRect r_img = img.rect();
            r_img.setLeft(r_img.width() - r_msg.width());
            r_img.setTop(2);
            r_img.setBottom(r_img.top()+msg.height());
            p.drawPixmap(r_img, msg, r_msg);
        }
        item->setData(COLUMN_ITEM, Qt::DecorationRole, img);

        //set checkboxes if it's a CHANNEL_CLASSROOM
        if(chan.nChannelID == mychanid)
        {
            bool modifychan = TT_IsChannelOperator(ttInst, TT_GetMyUserID(ttInst), user.nChannelID);
            modifychan |= ((TT_GetMyUserRights(ttInst) & USERRIGHT_MODIFY_CHANNELS) != USERRIGHT_NONE);

            bool txchanmsg = userCanChanMessage(userid, chan);
            bool txvoice = userCanVoiceTx(userid, chan);
            bool txvideo = userCanVideoTx(userid, chan);
            bool txdesktop = userCanDesktopTx(userid, chan);
            bool txmediafile = userCanMediaFileTx(userid, chan);
            if (modifychan)
            {
                if (!item->icon(COLUMN_CHANMSG).isNull())
                    item->setIcon(COLUMN_CHANMSG, QIcon());
                if (!item->icon(COLUMN_VOICE).isNull())
                    item->setIcon(COLUMN_VOICE, QIcon());
                if (!item->icon(COLUMN_VIDEO).isNull())
                    item->setIcon(COLUMN_VIDEO, QIcon());
                if (!item->icon(COLUMN_DESKTOP).isNull())
                    item->setIcon(COLUMN_DESKTOP, QIcon());
                if (!item->icon(COLUMN_MEDIAFILE).isNull())
                    item->setIcon(COLUMN_MEDIAFILE, QIcon());

                item->setCheckState(COLUMN_CHANMSG,
                                    txchanmsg ? Qt::Checked : Qt::Unchecked);
                item->setCheckState(COLUMN_VOICE, 
                                    txvoice ? Qt::Checked : Qt::Unchecked);
                item->setCheckState(COLUMN_VIDEO, 
                                    txvideo ? Qt::Checked : Qt::Unchecked);
                item->setCheckState(COLUMN_DESKTOP, 
                                    txdesktop ? Qt::Checked : Qt::Unchecked);
                item->setCheckState(COLUMN_MEDIAFILE, 
                                    txmediafile ? Qt::Checked : Qt::Unchecked);
            }
            else
            {
                if (!item->data(COLUMN_CHANMSG, Qt::CheckStateRole).isNull())
                    item->setData(COLUMN_CHANMSG, Qt::CheckStateRole, QVariant());
                if (!item->data(COLUMN_VOICE, Qt::CheckStateRole).isNull())
                    item->setData(COLUMN_VOICE, Qt::CheckStateRole, QVariant());
                if (!item->data(COLUMN_VIDEO, Qt::CheckStateRole).isNull())
                    item->setData(COLUMN_VIDEO, Qt::CheckStateRole, QVariant());
                if (!item->data(COLUMN_DESKTOP, Qt::CheckStateRole).isNull())
                    item->setData(COLUMN_DESKTOP, Qt::CheckStateRole, QVariant());
                if (!item->data(COLUMN_MEDIAFILE, Qt::CheckStateRole).isNull())
                    item->setData(COLUMN_MEDIAFILE, Qt::CheckStateRole, QVariant());

                if (txchanmsg || isFreeForAll(STREAMTYPE_CHANNELMSG, chan.transmitUsers))
                    item->setIcon(COLUMN_CHANMSG,
                    QIcon(QString::fromUtf8(":/images/images/oksign.png")));
                else
                    item->setIcon(COLUMN_CHANMSG,
                    QIcon(QString::fromUtf8(":/images/images/stopsign.png")));

                if (txvoice || isFreeForAll(STREAMTYPE_VOICE, chan.transmitUsers))
                    item->setIcon(COLUMN_VOICE,
                    QIcon(QString::fromUtf8(":/images/images/oksign.png")));
                else
                    item->setIcon(COLUMN_VOICE,
                    QIcon(QString::fromUtf8(":/images/images/stopsign.png")));

                if (txvideo || isFreeForAll(STREAMTYPE_VIDEOCAPTURE, chan.transmitUsers))
                    item->setIcon(COLUMN_VIDEO, 
                    QIcon(QString::fromUtf8(":/images/images/oksign.png")));
                else
                    item->setIcon(COLUMN_VIDEO, 
                    QIcon(QString::fromUtf8(":/images/images/stopsign.png")));

                if (txdesktop || isFreeForAll(STREAMTYPE_DESKTOP, chan.transmitUsers))
                    item->setIcon(COLUMN_DESKTOP,
                    QIcon(QString::fromUtf8(":/images/images/oksign.png")));
                else
                    item->setIcon(COLUMN_DESKTOP,
                    QIcon(QString::fromUtf8(":/images/images/stopsign.png")));

                if (txmediafile || isFreeForAll(STREAMTYPE_MEDIAFILE, chan.transmitUsers))
                    item->setIcon(COLUMN_MEDIAFILE,
                    QIcon(QString::fromUtf8(":/images/images/oksign.png")));
                else
                    item->setIcon(COLUMN_MEDIAFILE,
                    QIcon(QString::fromUtf8(":/images/images/stopsign.png")));
            }
        }
        else //make sure columns COLUMN_VOICE and COLUMN_VIDEO are empty
        {
            if (!item->icon(COLUMN_CHANMSG).isNull())
                item->setIcon(COLUMN_CHANMSG, QIcon());
            if (!item->icon(COLUMN_VOICE).isNull())
                item->setIcon(COLUMN_VOICE, QIcon());
            if (!item->icon(COLUMN_VIDEO).isNull())
                item->setIcon(COLUMN_VIDEO, QIcon());
            if (!item->icon(COLUMN_DESKTOP).isNull())
                item->setIcon(COLUMN_DESKTOP, QIcon());
            if (!item->icon(COLUMN_MEDIAFILE).isNull())
                item->setIcon(COLUMN_MEDIAFILE, QIcon());

            if (!item->data(COLUMN_CHANMSG, Qt::CheckStateRole).isNull())
                item->setData(COLUMN_CHANMSG, Qt::CheckStateRole, QVariant());
            if (!item->data(COLUMN_VOICE, Qt::CheckStateRole).isNull())
                item->setData(COLUMN_VOICE, Qt::CheckStateRole, QVariant());
            if (!item->data(COLUMN_VIDEO, Qt::CheckStateRole).isNull())
                item->setData(COLUMN_VIDEO, Qt::CheckStateRole, QVariant());
            if (!item->data(COLUMN_DESKTOP, Qt::CheckStateRole).isNull())
                item->setData(COLUMN_DESKTOP, Qt::CheckStateRole, QVariant());
            if (!item->data(COLUMN_MEDIAFILE, Qt::CheckStateRole).isNull())
                item->setData(COLUMN_MEDIAFILE, Qt::CheckStateRole, QVariant());
        }

        QBrush bgColor = talking ? QBrush(COLOR_TALK) : QPalette().brush(QPalette::Base);
        if(!talking && m_showlasttalk && userid == m_last_talker_id)
            bgColor = QBrush(COLOR_LASTTALK);
        item->setBackground(COLUMN_ITEM, bgColor);
    }

    m_ignore_item_changes = false;
}

void ChannelsTree::slotItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    int id = item->data(COLUMN_ITEM, Qt::UserRole).toInt();
    Q_ASSERT(id);
    switch(item->type())
    {
    case USER_TYPE :
        if(column == COLUMN_ITEM)
            emit(userDoubleClicked(id));
        break;
    case CHANNEL_TYPE :
        if(column == COLUMN_ITEM)
            emit(channelDoubleClicked(id));
        break;
    }
}

void ChannelsTree::slotItemChanged(QTreeWidgetItem* item, int column)
{
    if(m_ignore_item_changes || (column == COLUMN_ITEM))
        return;

    int channelid;
    if(item->type() == USER_TYPE)
    {
        int id = item->data(COLUMN_ITEM, Qt::UserRole).toInt();
        Q_ASSERT(id);

        //Seems the only way to check whether it's a user who checked
        //an item or it was an updated which invoked it is by checking
        //the current state of a channel
        users_t::const_iterator uIte = m_users.find(id);
        Q_ASSERT(uIte != m_users.end());
        if(uIte == m_users.end())
            return;

        channels_t::const_iterator cIte = m_channels.find(uIte.value().nChannelID);
        Q_ASSERT(cIte != m_channels.end());
        if(cIte == m_channels.end())
            return;

        const Channel& chan = cIte.value();
        channelid = chan.nChannelID;
    }
    else if(item->type() == CHANNEL_TYPE)
    {
        channelid = item->data(COLUMN_ITEM, Qt::UserRole).toInt();
        Q_ASSERT(channelid);
    }
    else return;

    QMap<int, StreamTypes> transmitUsers;
    getTransmitUsers(channelid, transmitUsers);
    emit(transmitusersChanged(channelid, transmitUsers));
}

void ChannelsTree::slotServerUpdate(const ServerProperties& /*srvprop*/)
{
    QTreeWidgetItem* root = getChannelItem(TT_GetRootChannelID(ttInst));
    if(root)
        slotUpdateTreeWidgetItem(root);
}

void ChannelsTree::slotAddChannel(const Channel& chan)
{
    m_channels.insert(chan.nChannelID, chan);

    QTreeWidgetItem* parent = getChannelItem(chan.nParentID);
    if(!parent)
    {
        parent = new QTreeWidgetItem(this, CHANNEL_TYPE);
        parent->setData(COLUMN_ITEM, Qt::UserRole, chan.nChannelID);
        slotUpdateTreeWidgetItem(parent);
    }
    else
    {
        int count = parent->childCount();
        QTreeWidgetItem* item;
        QString name = _Q(chan.szName);
        int i;
        for(i=0;i<count;i++)
        {
            item = parent->child(i);
            QString itemName = item->data(COLUMN_ITEM, Qt::DisplayRole).toString();
            if( (item->type() & CHANNEL_TYPE) &&
                name.compare(itemName, Qt::CaseInsensitive) < 0)
                break;
        }
        if(i==0)
            item = new QTreeWidgetItem(parent, parent, CHANNEL_TYPE);
        else
            item = new QTreeWidgetItem(parent, parent->child(i-1), CHANNEL_TYPE);
        item->setData(COLUMN_ITEM, Qt::UserRole, chan.nChannelID);
        slotUpdateTreeWidgetItem(item);
    }
}

void ChannelsTree::slotUpdateChannel(const Channel& chan)
{
    m_channels.insert(chan.nChannelID, chan);

    updateChannelItem(chan.nChannelID);

    //update users since there might be a new operator
    users_t::const_iterator ite;
    for(ite=m_users.begin();ite!=m_users.end();ite++)
    {
        QTreeWidgetItem* item = getUserItem(ite.key());
        if(item)
            slotUpdateTreeWidgetItem(item);
    }
}

void ChannelsTree::slotRemoveChannel(const Channel& chan)
{
    m_channels.remove(chan.nChannelID);
    delete getChannelItem(chan.nChannelID);
}

void ChannelsTree::slotUserLoggedIn(const User& user)
{
    m_users.insert(user.nUserID, user);
}

void ChannelsTree::slotUserLoggedOut(const User& user)
{
    m_stats.remove(user.nUserID);
    m_blinkhand_users.remove(user.nUserID);
    m_blinkchalk_users.remove(user.nUserID);
    m_desktopaccess_users.remove(user.nUserID);
    m_videousers.remove(user.nUserID);

    Q_ASSERT(m_users.find(user.nUserID) != m_users.end());
    m_users.remove(user.nUserID);
}

void ChannelsTree::slotUserUpdate(const User& user)
{
    Q_ASSERT(m_users.find(user.nUserID) != m_users.end());
    m_users.insert(user.nUserID, user);

    //ignore user if not in channel
    if(user.nChannelID == 0)
        return;

    QTreeWidgetItem* item = getUserItem(user.nUserID);
    if(item)
    {
        QString name = getDisplayName(user);
        //only update with nickname is changed
        if(item->data(COLUMN_ITEM, Qt::DisplayRole).toString() != name)
        {
            QTreeWidgetItem* parent = item->parent();
            bool selected = this->currentItem() == item;
            parent->removeChild(item);
            parent->insertChild(getUserIndex(parent, getDisplayName(user)), item);
            if (selected)
               this->setCurrentItem(item);
        }
        //clear blinking request user (if enabled)
        if(user.uLocalSubscriptions & SUBSCRIBE_DESKTOPINPUT)
            setUserDesktopAccess(user.nUserID, false);

        slotUpdateTreeWidgetItem(item);
    }
}

void ChannelsTree::slotUserJoin(int channelid, const User& user)
{
    m_users.insert(user.nUserID, user);

    QTreeWidgetItem* parent = getChannelItem(channelid), *item;
    Q_ASSERT(parent);

    int i = getUserIndex(parent, getDisplayName(user));
    if(i == 0)
        item = new QTreeWidgetItem(parent, parent, USER_TYPE);
    else
        item = new QTreeWidgetItem(parent, parent->child(i-1), USER_TYPE);
    item->setData(COLUMN_ITEM, Qt::UserRole, user.nUserID);
    if(user.nUserID == TT_GetMyUserID(ttInst))
    {
        QFont font = item->data(COLUMN_ITEM, Qt::FontRole).value<QFont>();
        font.setBold(true);
        item->setData(COLUMN_ITEM, Qt::FontRole, font);
    }
    slotUpdateTreeWidgetItem(item);

    if(user.nUserID == TT_GetMyUserID(ttInst))
        scrollToItem(item);

    // update user count on parent channels
    updateChannelItem(user.nChannelID);
    channels_t parents = getParentChannels(channelid, m_channels);
    for(auto i=parents.begin();i!=parents.end();++i)
        updateChannelItem(i.key());
}

void ChannelsTree::slotUserLeft(int channelid, const User& user)
{
    Q_ASSERT(m_users.find(user.nUserID) != m_users.end());
    m_stats.remove(user.nUserID);
    m_blinkhand_users.remove(user.nUserID);
    m_blinkchalk_users.remove(user.nUserID);
    m_desktopaccess_users.remove(user.nUserID);
    delete getUserItem(user.nUserID);
    m_users.insert(user.nUserID, user);

    // update user count on parent channels
    updateChannelItem(channelid);
    channels_t parents = getParentChannels(channelid, m_channels);
    for(auto i=parents.begin();i!=parents.end();++i)
        updateChannelItem(i.key());
}

void ChannelsTree::slotUserStateChange(const User& user)
{
    User oldUser;
    ZERO_STRUCT(oldUser);
    getUser(user.nUserID, oldUser);

    if((user.uUserState & USERSTATE_VOICE) && user.nUserID != m_last_talker_id)
    {
        QTreeWidgetItem* lasttalk_item = getUserItem(m_last_talker_id);
        m_last_talker_id = 0;
        if(lasttalk_item)
            slotUpdateTreeWidgetItem(lasttalk_item);
    }

    m_users.insert(user.nUserID, user);
    
    if((oldUser.uUserState & USERSTATE_VOICE) &&
       (user.uUserState & USERSTATE_VOICE) == 0)
    {
        QTreeWidgetItem* lasttalk_item = getUserItem(m_last_talker_id);
        m_last_talker_id = user.nUserID;
        if(lasttalk_item)
            slotUpdateTreeWidgetItem(lasttalk_item);
    }

    QTreeWidgetItem* user_item = getUserItem(user.nUserID);
    if(user_item)
        slotUpdateTreeWidgetItem(user_item);

    if((user.uUserState & USERSTATE_VOICE) ||
       (user.uUserState & USERSTATE_VIDEOCAPTURE) ||
       (user.uUserState & USERSTATE_MEDIAFILE_VIDEO) ||
       (user.uUserState & USERSTATE_MEDIAFILE_AUDIO))
    {
        //add user to statistics so we can show if there's packetloss
        UserStatistics stats;
        if(m_stats.find(user.nUserID) == m_stats.end() &&
           TT_GetUserStatistics(ttInst, user.nUserID, &stats))
            m_stats.insert(user.nUserID, stats);
    }
    else
        m_stats.remove(user.nUserID);
}

void ChannelsTree::slotUpdateMyself()
{
    QTreeWidgetItem* user_item = getUserItem(TT_GetMyUserID(ttInst));
    if(user_item)
        slotUpdateTreeWidgetItem(user_item);

    //update last talking
    if(isMyselfTalking())
    {
        user_item = getUserItem(m_last_talker_id);
        m_last_talker_id = 0;
        if(user_item)
            slotUpdateTreeWidgetItem(user_item);
    }
}

void ChannelsTree::slotUserVideoFrame(int userid, int stream_id)
{
    Q_UNUSED(userid);
    Q_UNUSED(stream_id);
}
