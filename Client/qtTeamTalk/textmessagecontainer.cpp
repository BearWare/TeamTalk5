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

#include "textmessagecontainer.h"

quint32 generateKey(const TextMessage& msg)
{
    return (msg.nMsgType << 16) | msg.nFromUserID;
}


TextMessageContainer::TextMessageContainer()
{

}

MyTextMessage TextMessageContainer::addTextMessage(const TextMessage& msg)
{
    Q_ASSERT(msg.nMsgType != MSGTYPE_NONE);

    QString content;
    if (!mergeMessages(msg, content))
        return MyTextMessage();

    MyTextMessage mergemessage(msg);
    mergemessage.moreMessage = content;

    addCompleteTextMessage(mergemessage);
    return mergemessage;
}

void TextMessageContainer::addCompleteTextMessage(const MyTextMessage& msg)
{
    switch (msg.nMsgType)
    {
    case MSGTYPE_NONE :
    case MSGTYPE_BROADCAST :
    case MSGTYPE_CHANNEL :
    case MSGTYPE_CUSTOM :
        break;
    case MSGTYPE_USER :
        if (msg.nFromUserID)
        {
            m_usermessages[msg.nFromUserID].append(msg);
        }
        else
        {
            // text message from 'myself' to this user
            Q_ASSERT(msg.nToUserID);
            m_usermessages[msg.nToUserID].append(msg);
        }
    }
}

const textmessages_t* TextMessageContainer::getUserTextMessages(int userid) const
{
    auto i = m_usermessages.find(userid);
    if (i != m_usermessages.end())
        return &i.value();
    return nullptr;
}

void TextMessageContainer::clearUserTextMessages(int userid)
{
    m_usermessages.remove(userid);
}

void TextMessageContainer::clear()
{
    m_usermessages.clear();
    m_mergemessages.clear();
}

bool TextMessageContainer::mergeMessages(const TextMessage& msg, QString& content)
{
    m_mergemessages[generateKey(msg)].push_back(msg);

    // prevent out-of-memory.
    if (m_mergemessages[generateKey(msg)].size() > 1000)
        m_mergemessages.remove(generateKey(msg));

    if (!msg.bMore)
    {
        for (auto& m : m_mergemessages[generateKey(msg)])
        {
            content += _Q(m.szMessage);
        }
        m_mergemessages.remove(generateKey(msg));
    }

    return !msg.bMore;
}
