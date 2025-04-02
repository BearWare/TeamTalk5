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

#include <QPlainTextEdit>

class ChatTextHistory
{
public:
    virtual ~ChatTextHistory() = default;

    virtual void updateServer(const ServerProperties& srvprop) = 0;
    virtual void joinedChannel(int channelid) = 0;

    virtual QString addTextMessage(const MyTextMessage& msg) = 0;
    virtual void addLogMessage(const QString& msg) = 0;

    virtual bool hasFocus() const = 0;
    virtual void setFocus() = 0;
};

class ChatTextEdit : public QPlainTextEdit, public ChatTextHistory
{
    Q_OBJECT

public:
    ChatTextEdit(QWidget * parent = 0);

    void updateServer(const ServerProperties& srvprop) override;

    void joinedChannel(int channelid) override;

    QString addTextMessage(const MyTextMessage& msg) override;
    void addLogMessage(const QString& msg) override;
    bool hasFocus() const override { return QPlainTextEdit::hasFocus(); }
    void setFocus() override { QPlainTextEdit::setFocus(); }
signals:
    void clearHistory();
private:
    static QString getTimeStamp(const QDateTime& tm, bool force_ts = false);
    void limitText();
    QString currentUrl(const QTextCursor& cursor) const;

protected:
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
};

#endif
