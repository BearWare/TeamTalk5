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
