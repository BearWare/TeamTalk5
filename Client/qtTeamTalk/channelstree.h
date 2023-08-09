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

#ifndef CHANNELSTREE_H
#define CHANNELSTREE_H

#include "common.h"

#include <QSet>
#include <QTreeWidget>

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
    QVector<User> getSelectedUsers() const;

    QVector<int> getUsersInChannel(int channelid) const;
    QVector<int> getUsers() const;
    users_t getUsers(int channelid) const;

    void getTransmitUsers(int channelid, QMap<int, StreamTypes>& transmitUsers) const;

    void resetChannels();

    void setChannelMessaged(int channelid, bool messaged);
    void setUserMessaged(int userid, bool messaged);
    void setUserDesktopAccess(int userid, bool enable);
    void updateAllItems();

signals:
    void userDoubleClicked(int);
    void channelDoubleClicked(int);
    void fileDropped(const QString&);

    //chanid, voiceUsers, videoUsers, desktopUsers
    void transmitusersChanged(int, const QMap<int, StreamTypes>&);

protected:
    void timerEvent(QTimerEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent * event) override;
    void keyPressEvent(QKeyEvent* e) override;

private:
    typedef QMap<int, UserStatistics> statistics_t;
    typedef QSet<int> uservideo_t;
    channels_t m_channels;
    users_t m_users; //contains all users (also those not in channels)
    int m_last_talker_id;
    statistics_t m_stats;
    uservideo_t m_videousers;
    QSet<int> m_desktopaccess_users;

    //users who are blinking with hand
    QSet<int> m_blinkhand_users, m_blinkchalk_users;
    int m_statTimerId, m_questionTimerId, m_desktopaccesTimerId;

    QPoint m_dragStartPos;

    QTreeWidgetItem* getChannelItem(int channelid) const;
    QTreeWidgetItem* getUserItem(int userid) const;
    QPixmap getChannelIcon(const Channel& chan, const QTreeWidgetItem* item) const;
    /* return the "should be" index. Not the current index */
    int getUserIndex(const QTreeWidgetItem* parent, const QString& name) const;
    /* return the "should be" index. Not the current index */
    int getChannelIndex(const QTreeWidgetItem* item) const;
    void updateChannelItem(int channelid);
    void updateChannelItem(QTreeWidgetItem* item);
    void updateUserItem(QTreeWidgetItem* item);

    bool m_ignore_item_changes;

private:
    void slotItemDoubleClicked(QTreeWidgetItem* item, int column);
    void slotItemChanged(QTreeWidgetItem* item, int column);
    void slotUpdateTreeWidgetItem(QTreeWidgetItem* item);

public:
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
    QString getItemText() const;
};

#endif
