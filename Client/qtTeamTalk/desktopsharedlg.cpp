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

#include "desktopsharedlg.h"
#include "appinfo.h"
#include "settings.h"

#include <QSortFilterProxyModel>
#include <QPushButton>

#if defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#endif

extern NonDefaultSettings* ttSettings;

#if defined(Q_OS_LINUX)
DesktopShareDlg::DesktopShareDlg(Display* display, QWidget* parent)
#else
DesktopShareDlg::DesktopShareDlg(QWidget* parent)
#endif
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
#if defined(Q_OS_WIN32)
, m_hShareWnd(nullptr)
#elif defined(Q_OS_DARWIN)
, m_nCGShareWnd(0)
#elif defined(Q_OS_LINUX)
, m_nWindowShareWnd(0)
, m_display(display)
#endif
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

#if defined(Q_OS_WIN32)
    HWND hWnd = nullptr;
    int i = 0;
    while(TT_Windows_GetDesktopWindowHWND(i, &hWnd))
    {
        ShareWindow wnd;
        if(TT_Windows_GetWindow(hWnd, &wnd))
        {
            //int width = 0, height = 0;
            //TT_Windows_GetWindowDimensionsHWND(hWnd, &width, &height);
            if(_Q(wnd.szWindowTitle).size())
            {
                ui.windowComboBox->addItem(
                    QString(_Q(wnd.szWindowTitle) + " - (%1x%2)")
                    .arg(wnd.nWidth).arg(wnd.nHeight), (qulonglong)hWnd);
            }
        }
        i++;
    }
#elif defined(Q_OS_DARWIN)
    ShareWindow wnd = {};
    int i=0;
    while(TT_MacOS_GetWindow(i++, &wnd))
    {
        if(_Q(wnd.szWindowTitle).size())
            ui.windowComboBox->addItem(_Q(wnd.szWindowTitle) +
                                       QString(" - (%1x%2)").arg(wnd.nWidth)
                                       .arg(wnd.nHeight), (qint64)wnd.nWindowID);
    }
#elif defined(Q_OS_LINUX)
    //TODO: X11, share specific window
    // if(m_display)
    // {
    //     Window display_window = XRootWindow(m_display, 0);

    //     Window root = 0, parent_window = 0, *children = nullptr;
    //     unsigned int n_children = 0;

    //     Status s = XQueryTree(m_display, display_window, &root, &parent_window,
    //                           &children, &n_children);

    //     for(unsigned int i=0;i<n_children;i++)
    //     {
    //         char* str = 0;
    //         Status s_name = XFetchName(m_display, children[i], &str);
    //         if(str)
    //         {
    //             XWindowAttributes attr;
    //             ZERO_STRUCT(attr);
    //             Status s_attr = XGetWindowAttributes(m_display, children[i], &attr);
    //             ui.windowComboBox->addItem(QString("%1 - (%2x%3)")
    //                                        .arg(str).arg(attr.width).arg(attr.height),
    //                                        (qint64)children[i]);
    //             XFree(str);
    //         }
    //     }
    //     if(children)
    //         XFree(children);
    // }
#endif

    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(ui.windowComboBox);
    proxy->setSourceModel(ui.windowComboBox->model());
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui.windowComboBox->model()->setParent(proxy);
    ui.windowComboBox->setModel(proxy);
    ui.windowComboBox->model()->sort(0);

#if defined(Q_OS_DARWIN)
    DesktopShareMode mode = (DesktopShareMode)
        ttSettings->value(SETTINGS_DESKTOPSHARE_MODE,
                          DESKTOPSHARE_SPECIFIC_WINDOW).toInt();
    //TODO: MacOS, support desktop and active window
    ui.desktopRadioBtn->hide();
    ui.actwndRadioButton->hide();
#elif defined(Q_OS_LINUX)
    DesktopShareMode mode = (DesktopShareMode)
        ttSettings->value(SETTINGS_DESKTOPSHARE_MODE,
                          DESKTOPSHARE_DESKTOP).toInt();
    ui.specificwndRadioButton->hide();
    ui.windowComboBox->hide();
#else
    DesktopShareMode mode = (DesktopShareMode)
        ttSettings->value(SETTINGS_DESKTOPSHARE_MODE,
                          DESKTOPSHARE_ACTIVE_WINDOW).toInt();
#endif
    switch(mode)
    {
    case DESKTOPSHARE_DESKTOP :
        ui.desktopRadioBtn->setChecked(true);
        break;
    case DESKTOPSHARE_SPECIFIC_WINDOW :
        ui.specificwndRadioButton->setChecked(true);
        break;
    case DESKTOPSHARE_ACTIVE_WINDOW:
    default:
        ui.actwndRadioButton->setChecked(true);
        break;
    }

    QString prev_title = ttSettings->value(SETTINGS_DESKTOPSHARE_WINDOWTITLE).toString();
    int index = ui.windowComboBox->findText(prev_title);
    if(index >= 0)
        ui.windowComboBox->setCurrentIndex(index);

    ui.rgbComboBox->addItem(tr("Low (8-bit colors)"), BMP_RGB8_PALETTE);
    ui.rgbComboBox->addItem(tr("Medium (16-bit colors)"), BMP_RGB16_555);
    ui.rgbComboBox->addItem(tr("High (24-bit colors)"), BMP_RGB24);
    ui.rgbComboBox->addItem(tr("Maximum (32-bit colors)"), BMP_RGB32);

    BitmapFormat bmp_mode = (BitmapFormat)ttSettings->value(SETTINGS_DESKTOPSHARE_BMPMODE, BMP_RGB16_555).toInt();
    for(int i=0;i<ui.rgbComboBox->count();i++)
    {
        if(ui.rgbComboBox->itemData(i) == (int)bmp_mode)
            ui.rgbComboBox->setCurrentIndex(i);
    }
    int updinterval = ttSettings->value(SETTINGS_DESKTOPSHARE_INTERVAL, 1000).toInt();
    ui.updintervakChkBox->setChecked(updinterval>0);
    ui.intervalSpinBox->setEnabled(updinterval>0);
    if(updinterval>0)
        ui.intervalSpinBox->setValue(updinterval);

    ui.cursorChkBox->setChecked(ttSettings->value(SETTINGS_DESKTOPSHARE_CURSOR, false).toBool());
}

void DesktopShareDlg::accept()
{
    DesktopShareMode mode = DESKTOPSHARE_NONE;

    if(ui.desktopRadioBtn->isChecked())
    {
        mode = DESKTOPSHARE_DESKTOP;
#if defined(Q_OS_WIN32)
        m_hShareWnd = TT_Windows_GetDesktopHWND();
#elif defined(Q_OS_LINUX)
        if(m_display)
            m_nWindowShareWnd = XRootWindow(m_display, 0);
#endif
    }
    if(ui.actwndRadioButton->isChecked())
        mode = DESKTOPSHARE_ACTIVE_WINDOW;
    if(ui.specificwndRadioButton->isChecked())
    {
        mode = DESKTOPSHARE_SPECIFIC_WINDOW;
#if defined(Q_OS_WIN32)
        m_hShareWnd = (HWND)ui.windowComboBox->itemData(ui.windowComboBox->currentIndex()).toULongLong();
        //QMessageBox::critical(this, tr("Invalid Window"),
        //                      tr("The requested window cannot be found"));
#elif defined(Q_OS_DARWIN)
        m_nCGShareWnd = (INT64)ui.windowComboBox->itemData(ui.windowComboBox->currentIndex()).toLongLong();
#elif defined(Q_OS_LINUX)
        m_nWindowShareWnd  = (INT64)ui.windowComboBox->itemData(ui.windowComboBox->currentIndex()).toLongLong();
#endif
        ttSettings->setValue(SETTINGS_DESKTOPSHARE_WINDOWTITLE,
                             ui.windowComboBox->currentText());
    }

    ttSettings->setValue(SETTINGS_DESKTOPSHARE_MODE, mode);
    if(ui.updintervakChkBox->isChecked())
        ttSettings->setValue(SETTINGS_DESKTOPSHARE_INTERVAL,
                             ui.intervalSpinBox->value());
    else
        ttSettings->setValue(SETTINGS_DESKTOPSHARE_INTERVAL, 0);

    ttSettings->setValue(SETTINGS_DESKTOPSHARE_BMPMODE,
                         ui.rgbComboBox->itemData(ui.rgbComboBox->currentIndex()));

    ttSettings->setValue(SETTINGS_DESKTOPSHARE_CURSOR, ui.cursorChkBox->isChecked());
    QDialog::accept();
}
