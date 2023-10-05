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

#include "textmessagecontainer.h"

extern TTInstance* ttInst;

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
        Q_ASSERT(msg.nFromUserID);
        if (msg.nFromUserID == TT_GetMyUserID(ttInst))
        {
            // text message from 'myself' to this user
            if (msg.nToUserID)
                m_usermessages[msg.nToUserID].append(msg);
            else //text message from 'myself' to 'myself'
                m_usermessages[msg.nFromUserID].append(msg);
        }
        else
        {
            m_usermessages[msg.nFromUserID].append(msg);
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
