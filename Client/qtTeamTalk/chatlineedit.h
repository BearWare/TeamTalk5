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

#ifndef CHATLINEEDIT_H
#define CHATLINEEDIT_H

#include <QLineEdit>
#include <QList>

class ChatLineEdit : public QLineEdit
{
public:
    ChatLineEdit(QWidget * parent = 0);

protected:
    void keyPressEvent ( QKeyEvent * event );

private:
    QList<QString> m_history;
    QList<QString>::const_iterator m_history_pos;
};

#endif

