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
 * This source code is part of the TeamTalk 4 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
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
