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

#include "chatlineedit.h"
#include <QKeyEvent>
#include <QDebug>

ChatLineEdit::ChatLineEdit(QWidget * parent/* = 0*/ )
: QLineEdit(parent)
{
    m_history_pos = m_history.begin();
}

void ChatLineEdit::keyPressEvent ( QKeyEvent * event )
{
    switch(event->key())
    {
    case Qt::Key_Enter :
    case Qt::Key_Return :
        if(text().size() == 0)
            break;

        m_history.push_back(text());
        while(m_history.size() > 50)
            m_history.erase(m_history.begin());
        m_history_pos = m_history.end();
        break;
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
    QLineEdit::keyPressEvent( event );
}
