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

#include "mytabwidget.h"

MyTabWidget::MyTabWidget(QWidget* parent/* = nullptr*/) : QTabWidget(parent)
{
}

void MyTabWidget::keyPressEvent(QKeyEvent* e)
{
    if (this->hasFocus())
    {
        if (e->key() == Qt::Key_Home && this->currentIndex() != 0)
            this->setCurrentIndex(0);
        else if (e->key() == Qt::Key_End && this->currentIndex() != this->count())
            this->setCurrentIndex(this->count()-1);
        else
            QTabWidget::keyPressEvent(e);
    }
    else
        QTabWidget::keyPressEvent(e);
}
