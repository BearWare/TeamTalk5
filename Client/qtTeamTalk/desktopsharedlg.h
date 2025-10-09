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

#ifndef DESKTOPSHAREDLG_H
#define DESKTOPSHAREDLG_H

#include "ui_desktopshare.h"

#include "common.h"
#include "utilos.h"

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
public:
    void accept();

private:
    Ui::DesktopShareDlg ui;
#if defined(Q_OS_LINUX)
    Display* m_display;
#endif
};

#endif
