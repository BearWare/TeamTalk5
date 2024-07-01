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

#ifndef KEYCOMPDLG_H
#define KEYCOMPDLG_H

#include <QList>
#include <QSet>

#include "utilhotkey.h"
#include "ui_keycomp.h"

class KeyCompDlg : public QDialog
{
    Q_OBJECT

public:
    KeyCompDlg(HotKeyID hkID, QWidget * parent = 0);
    ~KeyCompDlg();

    hotkey_t m_hotkey;

protected:
#if defined(Q_OS_WIN32)
    
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    bool nativeEvent(const QByteArray& eventType, void* message,
                     long* result) override;
#else
    bool nativeEvent(const QByteArray& eventType, void* message,
                     qintptr* result) override;
#endif /* QT_VERSION */
    
#elif defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
#endif

private:
    Ui::KeyCompDlg ui;
    QSet<INT32> m_activekeys;
};

#endif
