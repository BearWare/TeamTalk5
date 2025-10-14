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

#include "sendtextedit.h"
#include "common.h"


SendTextEdit::SendTextEdit(QWidget * parent/* = 0*/)
    : QPlainTextEdit(parent)
{
}
    
void SendTextEdit::kPress ( QKeyEvent * event )
{
    keyPressEvent(event);
}

void SendTextEdit::keyPressEvent(QKeyEvent* e)
{
    QString msg = toPlainText();
    if ((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) &&
        (e->modifiers() & Qt::ShiftModifier) == 0)
    {
        emit sendTextMessage(msg);
    }
    else
        QPlainTextEdit::keyPressEvent(e);
}

void SendTextEdit::keyReleaseEvent(QKeyEvent* e)
{
    QPlainTextEdit::keyReleaseEvent(e);
}
