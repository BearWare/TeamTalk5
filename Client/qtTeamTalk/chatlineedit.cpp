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

#include "chatlineedit.h"
#include <QKeyEvent>
#include <QDebug>

ChatLineEdit::ChatLineEdit(QWidget * parent/* = 0*/ )
: QLineEdit(parent)
{
    m_history_pos = m_history.begin();
}

void ChatLineEdit::kPress ( QKeyEvent * event )
{
    keyPressEvent(event);
}

void ChatLineEdit::keyPressEvent ( QKeyEvent * event )
{
    switch(event->key())
    {
    case Qt::Key_Up :
        if(text().size() && 
           m_history_pos == m_history.end())
        {
            m_history.push_back(text());
            m_history_pos = m_history.end();
        }
        if(m_history_pos != m_history.begin())
        {
            m_history_pos--;
            setText(*m_history_pos);
        }
        break;
    case Qt::Key_Down :
        if(m_history_pos != m_history.end())
        {
            m_history_pos++;
            if(m_history_pos != m_history.end())
                setText(*m_history_pos);
            else
                clear();
        }
        break;
    }
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
        (event->modifiers() & Qt::ShiftModifier) != 0)
    {
        insert("\n");
    }
    else if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
        (event->modifiers() & Qt::ShiftModifier) == 0 && text().size() != 0)
    {
        m_history.push_back(text());
        while(m_history.size() > 50)
            m_history.erase(m_history.begin());
        m_history_pos = m_history.end();
        emit sendTextMessage();
    }
    QLineEdit::keyPressEvent( event );
}
