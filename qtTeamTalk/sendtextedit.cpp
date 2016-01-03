/*
 * Copyright (c) 2005-2016, BearWare.dk
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

#include "sendtextedit.h"
#include "common.h"


SendTextEdit::SendTextEdit(QWidget * parent/* = 0*/)
    : QPlainTextEdit(parent)
{
}
    
void SendTextEdit::keyPressEvent(QKeyEvent* e)
{
    QString msg = toPlainText();
    QPlainTextEdit::keyPressEvent(e);
    if((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) &&
       (e->modifiers() & Qt::ShiftModifier) == 0)
        emit(sendTextMessage(msg));
}

void SendTextEdit::keyReleaseEvent(QKeyEvent* e)
{
    QPlainTextEdit::keyReleaseEvent(e);
}
