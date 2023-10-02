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

#include "mytreeview.h"
#include "utiltts.h"
#include "settings.h"

extern QSettings* ttSettings;

MyTreeView::MyTreeView(QWidget* parent/* = nullptr*/) : QTreeView(parent)
{
}

void MyTreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
#if defined(Q_OS_DARWIN)
#if QT_VERSION < QT_VERSION_CHECK(6,4,0)
    if (current.isValid() && ttSettings->value(SETTINGS_TTS_SPEAKLISTS, SETTINGS_TTS_SPEAKLISTS_DEFAULT).toBool() == true)
        addTextToSpeechMessage(current.data(Qt::AccessibleTextRole).toString());
#endif
#endif
    QTreeView::currentChanged(current, previous);
}

void MyTreeView::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Space)
    {
        auto i = currentIndex();
        emit doubleClicked(i);
        setCurrentIndex(i);
    }

    QTreeView::keyPressEvent(e);
}
