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

#include "desktopsharedlg.h"
#include "appinfo.h"
#include "settings.h"

#include <QSortFilterProxyModel>

#if defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#endif

extern QSettings* ttSettings;

#if defined(Q_OS_LINUX)
DesktopShareDlg::DesktopShareDlg(Display* display, QWidget* parent)
#else
DesktopShareDlg::DesktopShareDlg(QWidget* parent)
#endif
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
#if defined(Q_OS_WIN32)
, m_hShareWnd(NULL)
#elif defined(Q_OS_DARWIN)
, m_nCGShareWnd(0)
#elif defined(Q_OS_LINUX)
, m_nWindowShareWnd(0)
, m_display(display)
#endif
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

#if defined(Q_OS_WIN32)
    HWND hWnd = NULL;
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
    ShareWindow wnd;
    ZERO_STRUCT(wnd);
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

    //     Window root = 0, parent_window = 0, *children = NULL;
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
