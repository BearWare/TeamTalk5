/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#ifndef DESKTOPSHAREDLG_H
#define DESKTOPSHAREDLG_H

#include "ui_desktopshare.h"

#include "common.h"

#ifdef Q_OS_LINUX
#include <QX11Info>
#endif

class DesktopShareDlg : public QDialog
{
    Q_OBJECT

public:
#if defined(Q_OS_LINUX)
    DesktopShareDlg(Display* display, QWidget* parent);
#else
    DesktopShareDlg(QWidget* parent);
#endif
#if defined(Q_OS_WIN32)
    HWND m_hShareWnd;
#elif defined(Q_OS_DARWIN)
    INT64 m_nCGShareWnd;
#elif defined(Q_OS_LINUX)
    INT64 m_nWindowShareWnd;
#endif
public slots:
    void accept();

private:
    Ui::DesktopShareDlg ui;
#if defined(Q_OS_LINUX)
    Display* m_display;
#endif
}; 

#endif

