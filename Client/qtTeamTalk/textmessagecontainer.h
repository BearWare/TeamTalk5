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

#if !defined(TEXTMESSAGECONTAINER_H)
#define TEXTMESSAGECONTAINER_H

#include "common.h"

#include <QList>


class TextMessageContainer
{
public:
    TextMessageContainer();
    MyTextMessage addTextMessage(const TextMessage& msg);
    const textmessages_t* getUserTextMessages(int userid) const;
    void clearUserTextMessages(int userid);
    void clear();

private:
    void addCompleteTextMessage(const MyTextMessage& msg);
    bool mergeMessages(const TextMessage& msg, QString& content);
    typedef QMap<quint32, QList<TextMessage> > messages_t;
    messages_t m_mergemessages;

    typedef QMap<int, textmessages_t> usermessages_t;
    usermessages_t m_usermessages;
};

#endif // TEXTMESSAGECONTAINER_H
