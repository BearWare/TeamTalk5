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

#if !defined(TEXTMESSAGECONTAINER_H)
#define TEXTMESSAGECONTAINER_H

#include "common.h"

#include <QList>
#include <QObject>

class TextMessageContainer : public QObject
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
