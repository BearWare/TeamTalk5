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
    if (current.isValid() && ttSettings->value(SETTINGS_TTS_SPEAKLISTS, SETTINGS_TTS_SPEAKLISTS_DEFAULT).toBool() == true)
        addTextToSpeechMessage(current.data(Qt::AccessibleTextRole).toString());
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
