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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#ifndef CHANNELSTREE_H
#define CHANNELSTREE_H

#include <QMap>
#include <QSet>
#include <QVector>
#include <QTreeWidget>

#include "common.h"

typedef QVector<int> userids_t;

bool userCanVoiceTx(int userid, const Channel& chan);
bool userCanVideoTx(int userid, const Channel& chan);
bool userCanDesktopTx(int userid, const Channel& chan);
bool userCanMediaFileTx(int userid, const Channel& chan);

class ChannelsTree : public QTreeWidget
{
    Q_OBJECT

public:
    ChannelsTree(QWidget* parent);

    int selectedChannel(bool include_user = false) const;
    QVector<int> selectedChannels() const;

    int selectedUser() const;
    QVector<int> selectedUsers() const;

    bool getChannel(int channelid, Channel& chan) const;
    bool getSelectedChannel(Channel& chan) const;
    bool getUser(int userid, User& user) const;
    bool getSelectedUser(User& user) const;

    QVector<int> getUsersInChannel(int channelid) const;
    QVector<int> getUsers() const;

    void getClassRoomUsers(int channelid, QMap<int, StreamTypes>& transmitUsers);

    void reset();

    void setChannelMessaged(int channelid, bool messaged);
    void setUserMessaged(int userid, bool messaged);
    void setUserDesktopAccess(int userid, bool enable);
    void setShowUserCount(bool show);
    void setShowLastToTalk(bool show);
    void updateItemTextLength(int new_length);

signals:
    void userDoubleClicked(int);
    void channelDoubleClicked(int);
    void fileDropped(const QString&);

    //chanid, voiceUsers, videoUsers, desktopUsers
    void classroomChanged(int, const QMap<int, StreamTypes>&);

protected:
    void timerEvent(QTimerEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent * event);

private:
    typedef QMap<int, Channel> channels_t;
    typedef QMap<int, User> users_t;
    typedef QMap<int, UserStatistics> statistics_t;
    typedef QSet<int> uservideo_t;
    channels_t m_channels;
    users_t m_users; //contains all users (also those not in channels)
    bool m_showusercount;
    bool m_showlasttalk;
    int m_last_talker_id;
    int m_strlen;
    statistics_t m_stats;
    uservideo_t m_videousers;
    QSet<int> m_desktopaccess_users;

    //users who are blinking with hand
    QSet<int> m_blinkhand_users, m_blinkchalk_users;
    int m_statTimerId, m_questionTimerId, m_desktopaccesTimerId;

    QPoint m_dragStartPos;

    QTreeWidgetItem* getChannelItem(int channelid);
    QTreeWidgetItem* getUserItem(int userid);
    int getUserIndex(const QTreeWidgetItem* parent, const QString& nick);
    void updateChannelItem(int channelid);

    bool m_ignore_item_changes;

private slots:
    void slotItemDoubleClicked(QTreeWidgetItem* item, int column);
    void slotItemChanged(QTreeWidgetItem* item, int column);
    void slotUpdateTreeWidgetItem(QTreeWidgetItem* item);

public slots:
    void slotServerUpdate(const ServerProperties& srvprop);
    void slotAddChannel(const Channel& chan);
    void slotUpdateChannel(const Channel& chan);
    void slotRemoveChannel(const Channel& chan);
    void slotUserLoggedIn(const User& user);
    void slotUserLoggedOut(const User& user);
    void slotUserUpdate(const User& user);
    void slotUserJoin(int channelid, const User& user);
    void slotUserLeft(int channelid, const User& user);
    void slotUserStateChange(const User& user);
    void slotUpdateMyself();
    void slotUserVideoFrame(int userid, int stream_id);
};

#endif
