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

#ifndef CHATTEXTEDIT_H
#define CHATTEXTEDIT_H

#include "common.h"

#include <QListWidget>

class ChatTextEdit : public QListWidget
{
    Q_OBJECT

public:
    ChatTextEdit(QWidget * parent = 0);

    void updateServer(const ServerProperties& srvprop);
    void joinedChannel(int channelid);

    QString addTextMessage(const MyTextMessage& msg);
    void addLogMessage(const QString& msg);
    void clearHistory();
    void copyAllHistory();

private:
    static QString getTimeStamp(const QDateTime& tm, bool force_ts = false);
    QString getTextMessagePrefix(const TextMessage& msg, const User& user);
    QStringList allUrls(const QString &text) const;
    QString currentUrl(const QListWidgetItem* item) const;
    void limitText();

protected:
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
};

#endif // CHATTEXTEDIT_H
