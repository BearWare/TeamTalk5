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
    User getUser(int userid) const;
    bool getSelectedUser(User& user) const;
    QVector<User> getSelectedUsers() const;

    QVector<int> getUsersInChannel(int channelid) const;
    QVector<int> getUsers() const;
    QVector<int> getChannels() const;
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
    bool m_ignore_item_changes;

    QTreeWidgetItem* getChannelItem(int channelid) const;
    QTreeWidgetItem* getUserItem(int userid) const;
    QPixmap getChannelIcon(const Channel& chan, const QTreeWidgetItem* item) const;
    QPixmap getUserIcon(const User& user, const Channel& chan, const QTreeWidgetItem* item) const;
    void setChannelTransmitUsers(const Channel& chan, QTreeWidgetItem* item);
    void setUserTransmitUser(const User& user, const Channel& chan, QTreeWidgetItem* item);
    /* return the "should be" index. Not the current index */
    int getUserIndex(const QTreeWidgetItem* parent, const QString& name) const;
    /* return the "should be" index. Not the current index */
    int getChannelIndex(const QTreeWidgetItem* item) const;
    void updateChannelItem(int channelid);
    void updateChannelItem(QTreeWidgetItem* item);
    void updateUserItem(QTreeWidgetItem* item);
    void updateUserStatistics();
    void updateUserDesktopAccess();
    void updateUserQuestionMode();

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
