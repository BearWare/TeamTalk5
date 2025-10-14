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

#include "mainwindow.h"
#include "license.h"
#include "appinfo.h"

#include <QAbstractNativeEventFilter>
#include <QApplication>
#include <QColor>
#include <QFileOpenEvent>
#include <QPalette>
#include <QSettings>
#include <QStyleFactory>
#include <QUrl>
#include <QtPlugin>

#include <iostream>
#include <string>

#if defined(Q_OS_WIN32)
#include <windows.h>
#endif


TTInstance* ttInst = nullptr;

#if defined(Q_OS_WIN32)

class MyQApplication
    : public QApplication
{
public:
    MyQApplication(int& argc, char **argv)
        : QApplication(argc, argv), m_mainwindow(nullptr)
    {
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
        if (settings.value("AppsUseLightTheme", -1).toUInt() == 0)
        {
            setStyle(QStyleFactory::create("Fusion"));
            QPalette darkPalette;
            QColor darkColor = QColor(45, 45, 45);
            QColor disabledColor = QColor(127, 127, 127);
            darkPalette.setColor(QPalette::Window, darkColor);
            darkPalette.setColor(QPalette::WindowText, Qt::white);
            darkPalette.setColor(QPalette::Base, QColor(18, 18, 18));
            darkPalette.setColor(QPalette::AlternateBase, darkColor);
            darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
            darkPalette.setColor(QPalette::ToolTipText, Qt::white);
            darkPalette.setColor(QPalette::Text, Qt::white);
            darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
            darkPalette.setColor(QPalette::Button, darkColor);
            darkPalette.setColor(QPalette::ButtonText, Qt::white);
            darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
            darkPalette.setColor(QPalette::BrightText, Qt::red);
            darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

            darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
            darkPalette.setColor(QPalette::HighlightedText, Qt::black);
            darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);

            setPalette(darkPalette);

            setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
        }
    }

    MainWindow* m_mainwindow;
};

#elif defined(Q_OS_LINUX)

#include <X11/Xlib.h>
#include <xcb/xcb.h> // used by Qt5

struct x_auto_repeat_data
{
    KeyCode keycode;
    Time timestamp;
    bool release;
    bool error;
};

static Bool qt_keypress_scanner(Display *, XEvent *event, XPointer arg)
{
    if (event->type != KeyPress && event->type != KeyRelease)
        return false;

    x_auto_repeat_data *data = (x_auto_repeat_data *) arg;
    if (data->error)
        return false;

    if (event->xkey.keycode != data->keycode)
    {
        data->error = true;
        return false;
    }

    if (event->type == KeyPress)
    {
        data->error = (! data->release || event->xkey.time - data->timestamp > 10);
        return (! data->error);
    }

    // must be XKeyRelease event
    if (data->release)
    {
        // found a second release
        data->error = true;
        return false;
    }

    // found a single release
    data->release = true;
    data->timestamp = event->xkey.time;

    return false;
}

class MyQApplication : public QApplication
                     , public QAbstractNativeEventFilter
{
public:
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result)
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
#endif
    {
        Q_UNUSED(result);

        if(eventType == "xcb_generic_event_t")
        {
            xcb_generic_event_t *ev = static_cast<xcb_generic_event_t *>(message);
            xcb_key_press_event_t *keyevent = 0;
            unsigned int keycode = 0, mods = 0, type = (ev->response_type & ~0x80);
            switch(type)
            {
            case XCB_KEY_PRESS :
            case XCB_KEY_RELEASE :
            {
                keyevent = static_cast<xcb_key_press_event_t *>(message);
                keycode = keyevent->detail;
                if(keyevent->state & XCB_MOD_MASK_1)
                    mods |= Mod1Mask;
                if(keyevent->state & XCB_MOD_MASK_CONTROL)
                    mods |= ControlMask;
                if(keyevent->state & XCB_MOD_MASK_4)
                    mods |= Mod4Mask;
                if(keyevent->state & XCB_MOD_MASK_SHIFT)
                    mods |= ShiftMask;

                if(m_mainwindow)
                    m_mainwindow->keysActive(keycode, mods, type == XCB_KEY_PRESS);
                break;
            }
            }
        }
        return false;
    }

    MyQApplication(int& argc, char **argv)
        : QApplication(argc, argv), m_mainwindow(nullptr)
    {
        installNativeEventFilter(this);
    }

    MainWindow* m_mainwindow;
};

#elif defined(Q_OS_DARWIN)

OSStatus mac_callback(EventHandlerCallRef nextHandler, EventRef event, void*);

class MyQApplication : public QApplication
{
public:
    MyQApplication(int& argc, char **argv) 
        : QApplication(argc, argv), m_mainwindow(nullptr)
    {
        EventTypeSpec hkEvents[2];
        hkEvents[0].eventClass = kEventClassKeyboard;
        hkEvents[0].eventKind = kEventHotKeyPressed;
        hkEvents[1].eventClass = kEventClassKeyboard;
        hkEvents[1].eventKind = kEventHotKeyReleased;

        OSStatus oss = InstallApplicationEventHandler(&mac_callback, 2, hkEvents, nullptr, nullptr);
        Q_ASSERT(oss == 0);

        QApplication::setQuitOnLastWindowClosed(false);
    }

    //TeamTalk event handling for macOS (Carbon). In QT 4 this is an
    //inherited method from QCoreApplication. In QT 5 this is a
    //standalone method.
    bool macEventFilter(EventHandlerCallRef caller, EventRef event)
    {
        Q_UNUSED(caller);
        if(GetEventClass(event) == kEventClassKeyboard)
        {
            UInt32 kind = GetEventKind(event);
            if(kind == kEventHotKeyPressed ||
               kind == kEventHotKeyReleased)
            {
                EventHotKeyID keyID;
                GetEventParameter(event, kEventParamDirectObject, 
                                  typeEventHotKeyID, 
                                  nullptr, sizeof(keyID), nullptr, &keyID);
                m_mainwindow->hotkeyToggle((HotKeyID)keyID.id, kind == kEventHotKeyPressed);
            }
        }

        return true; //Just return what ever...
    }

    MainWindow* m_mainwindow;

protected:
    bool event(QEvent* e)
    {
        //Opened through Launch Service
        if(e->type() == QEvent::FileOpen)
        {
            QString tturi = static_cast<QFileOpenEvent*>(e)->file();
            if(tturi.isEmpty())
                tturi = static_cast<QFileOpenEvent*>(e)->url().toString();
            if(m_mainwindow && tturi.size())
                m_mainwindow->parseArgs(QStringList() << "abc" << tturi);
        }

        // This handles press in Dock on macOS
        if (e->type() == QEvent::ApplicationActivated)
        {
            if(m_mainwindow->isHidden())
                m_mainwindow->show();
        }
        return QApplication::event(e);
    }
};

OSStatus mac_callback(EventHandlerCallRef nextHandler, EventRef event, void*)
{
    MyQApplication* myApp = dynamic_cast<MyQApplication*>(qApp);
    Q_ASSERT(myApp);
    myApp->macEventFilter(nextHandler, event);

    return noErr;
}

#endif

static bool showVersionOnly(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "-v" ||
            std::string(argv[i]) == "--version")
        {
#if defined(Q_OS_WIN32)
            if (!GetConsoleWindow())
            {
                AttachConsole(ATTACH_PARENT_PROCESS);
                freopen("CONOUT$", "w", stdout);
                freopen("CONOUT$", "w", stderr);
            }
#endif
            std::cout << APPTITLE << std::endl;
            return true;
        }
    }
    return false;
}

int main(int argc, char* argv[])
{
    if (showVersionOnly(argc, argv)) return 0;
#if defined(Q_OS_WIN32)
    // Use QWindowsIntegration plugin to distinguish Alt+Gr from Ctrl+Alt on Windows
    // https://qthub.com/static/doc/qt5/qtdoc/qpa.html
    std::vector< const char* > argv_;
    for (int i = 0; i < argc; ++i)
        argv_.push_back(argv[i]);
    argv_.push_back("-platform");
    argv_.push_back("windows:altgr");
    argc = int(argv_.size());
    MyQApplication app(argc, const_cast<char**>(&argv_[0]));
#elif defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
    MyQApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif

    QString cfgfile;
    int idx = QApplication::arguments().indexOf("-cfg");
    if(idx >= 0 && ++idx < QApplication::arguments().size())
        cfgfile = QApplication::arguments()[idx];

    MainWindow window(cfgfile);

    /* Set license information before creating the first client instance */
    TT_SetLicenseInformation(_W(QString(REGISTRATION_NAME)), _W(QString(REGISTRATION_KEY)));

#if defined(Q_OS_WIN32)
    HWND hWnd = reinterpret_cast<HWND>(window.winId());
    app.m_mainwindow = &window;
    ttInst = TT_InitTeamTalk(hWnd, WM_TEAMALK_CLIENTEVENT);
#elif defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
    app.m_mainwindow = &window;
    ttInst = TT_InitTeamTalkPoll();
#else
    ttInst = TT_InitTeamTalkPoll();
#endif
    
    window.loadSettings(); //load settings now that we have ttInst

    window.show();
    int ret = app.exec();
    TT_CloseTeamTalk(ttInst);
    return ret;
}
