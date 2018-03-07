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

#ifndef KEYCOMPDLG_H
#define KEYCOMPDLG_H

#include <QList>
#include <QSet>

#include "common.h"
#include "ui_keycomp.h"

class KeyCompDlg : public QDialog
{
    Q_OBJECT

public:
    KeyCompDlg(QWidget * parent = 0);
    ~KeyCompDlg();

    hotkey_t m_hotkey;

protected:
#if defined(Q_OS_WIN32) && QT_VERSION >= 0x050000
    bool nativeEvent(const QByteArray& eventType, void* message,
                     long* result);
#elif defined(Q_OS_WIN32)
    bool winEvent(MSG *message, long *result);
#elif defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
#endif

private:
    Ui::KeyCompDlg ui;
    QSet<INT32> m_activekeys;
};

#endif
