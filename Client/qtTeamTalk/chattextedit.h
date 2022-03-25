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

#ifndef CHATTEXTEDIT_H
#define CHATTEXTEDIT_H

#include "common.h"

#include <QPlainTextEdit>

class ChatTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    ChatTextEdit(QWidget * parent = 0);

    void updateServer(const ServerProperties& srvprop);

    void joinedChannel(int channelid);

    QString addTextMessage(const MyTextMessage& msg);
    void addLogMessage(const QString& msg);

private:
    static QString getTimeStamp(const QDateTime& tm, bool force_ts = false);
    void limitText();
    QString currentUrl(const QTextCursor& cursor) const;
    bool mergeMessages(const MyTextMessage& msg, QString& content);
    typedef QMap<quint32, textmessages_t> messages_t;
    messages_t m_mergemessages;

protected:
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent* e) override;
};

#endif
