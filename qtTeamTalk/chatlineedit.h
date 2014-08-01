/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
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

