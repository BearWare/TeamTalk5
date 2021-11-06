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


#include <QApplication>
#include <QtPlugin>
#include <QFileOpenEvent>
#include <QUrl>
#include <QSettings>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QAbstractNativeEventFilter>
#endif

#include "mainwindow.h"
#include "license.h"

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

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    //TeamTalk event handling for Win32
    bool winEventFilter ( MSG * msg, long * result )
    {
        if(msg->message == WM_TEAMALK_CLIENTEVENT)
        {
            TTMessage ttmsg;
            INT32 wait_ms = 0;
            if(TT_GetMessage(ttInst, &ttmsg, &wait_ms) && m_mainwindow)
                m_mainwindow->processTTMessage(ttmsg);
        }
        return QApplication::winEventFilter(msg, result);
    }
#endif
    MainWindow* m_mainwindow;
};

#elif defined(Q_OS_LINUX)

//For hotkeys on X11
#include <QX11Info>
#include <X11/Xlib.h>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <xcb/xcb.h> // used by Qt5
#endif

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

    if (event->xkey.keycode != data->keycode) {
        data->error = true;
        return false;
    }

    if (event->type == KeyPress) {
        data->error = (! data->release || event->xkey.time - data->timestamp > 10);
        return (! data->error);
    }

    // must be XKeyRelease event
    if (data->release) {
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
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
                     , public QAbstractNativeEventFilter
#endif
{
public:
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result)
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
#endif

    MyQApplication(int& argc, char **argv)
        : QApplication(argc, argv), m_mainwindow(nullptr)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        installNativeEventFilter(this);
#endif
    }

    bool x11EventFilter ( XEvent * event )
    {
        if(event->type == KeyPress || event->type == KeyRelease)
        {
            XKeyEvent* key = reinterpret_cast<XKeyEvent*> (event);
            
            bool autor = false;
            static uint curr_autorep = 0;
            if (event->type == KeyPress) 
            {
                if (curr_autorep == event->xkey.keycode) 
                {
                    autor = true;
                    curr_autorep = 0;
                }
            }
            else
            {
                // look ahead for auto-repeat
                XEvent nextpress;

                Display* dpy = QX11Info::display();

                // was this the last auto-repeater?
                x_auto_repeat_data auto_repeat_data;
                auto_repeat_data.keycode = event->xkey.keycode;
                auto_repeat_data.timestamp = event->xkey.time;

                auto_repeat_data.release = true;
                auto_repeat_data.error = false;
                if (XCheckIfEvent(dpy, &nextpress, &qt_keypress_scanner,
                                  (XPointer) &auto_repeat_data)) {
                    autor = true;
                    XPutBackEvent(dpy,&nextpress);
                }

                curr_autorep = autor ? event->xkey.keycode : 0;
            }

            if(!autor)
                m_mainwindow->keysActive(key->keycode, key->state, event->type == KeyPress);
        }
        
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        return true; //x11EventFilter is not supported in Qt5, so just return true
#else
        return QApplication::x11EventFilter(event);
#endif
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

#if QT_VERSION >= QT_VERSION_CHECK(5,4,0)
        QApplication::setQuitOnLastWindowClosed(false);
#endif
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

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        return true; //Just return what ever...
#else
        return QApplication::macEventFilter(caller, event);
#endif
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
#if QT_VERSION >= QT_VERSION_CHECK(5,4,0)
        // This handles press in Dock on macOS
        if (e->type() == QEvent::ApplicationActivated)
        {
            if(m_mainwindow->isHidden())
                m_mainwindow->show();
        }
#endif
        return QApplication::event(e);
    }
};

OSStatus mac_callback(EventHandlerCallRef nextHandler, EventRef event, void*)
{
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    MyQApplication* myApp = dynamic_cast<MyQApplication*>(qApp);
    Q_ASSERT(myApp);
    myApp->macEventFilter(nextHandler, event);
#else
    qApp->macEventFilter(nextHandler, event);
#endif
    return noErr;
}

#endif

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN32) || defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
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

#if defined(Q_OS_WIN32) && USE_POLL == 0
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
