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

#include "keycompdlg.h"
#include "common.h"
#include "appinfo.h"
#include <QDebug>
#include <QKeySequence>
#include <QVector>
#include <QMessageBox>
#include <QKeyEvent>

extern TTInstance* ttInst;

KeyCompDlg::KeyCompDlg(QWidget * parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

#ifdef Q_OS_WIN32
    HWND hWnd = reinterpret_cast<HWND>(this->winId());
    TT_HotKey_InstallTestHook(ttInst, hWnd, WM_TEAMTALK_HOTKEYEVENT);
#endif
}

KeyCompDlg::~KeyCompDlg()
{
#if defined(Q_OS_WIN32)
    TT_HotKey_RemoveTestHook(ttInst);
#elif defined(Q_OS_DARWIN)
    if(m_hotkey.empty())
        QMessageBox::warning(this, tr("Key Combination"),
                             tr("Modifiers (Option, Control, Command and Shift) must be used in combination with other keys."));
#endif
}

#if defined(Q_OS_WIN32) && QT_VERSION >= 0x050000

bool KeyCompDlg::nativeEvent(const QByteArray& eventType, void* message,
                             long* result)
{
    MSG* msg = reinterpret_cast<MSG*>(message);

    if(msg->message == WM_TEAMTALK_HOTKEYEVENT)
    {
        if(msg->lParam)
        {
            m_hotkey.push_back(msg->wParam);
            ui.keycompEdit->setText(getHotKeyText(m_hotkey));
            m_activekeys.insert(msg->wParam);
        }
        else
            m_activekeys.remove(msg->wParam);

        if(m_activekeys.size() == 0)
            this->accept();
    }

    return QDialog::nativeEvent(eventType, message, result);
}

#elif defined(Q_OS_WIN32)

bool KeyCompDlg::winEvent(MSG *message, long *result)
{
    if(message->message == WM_TEAMTALK_HOTKEYEVENT)
    {
        if(message->lParam)
        {
            m_hotkey.push_back(message->wParam);
            ui.keycompEdit->setText(getHotKeyText(m_hotkey));
            m_activekeys.insert(message->wParam);
        }
        else
            m_activekeys.remove(message->wParam);

        if(m_activekeys.size() == 0)
            this->accept();
    }
    return QDialog::winEvent(message, result);
}

#elif defined(Q_OS_LINUX)

void KeyCompDlg::keyPressEvent(QKeyEvent* event)
{
    m_hotkey.clear();

    Qt::KeyboardModifiers mods = event->modifiers();
    if(mods & Qt::CTRL)
        m_activekeys.insert(Qt::CTRL);
    if(mods & Qt::ALT)
        m_activekeys.insert(Qt::ALT);
    if(mods & Qt::SHIFT)
        m_activekeys.insert(Qt::SHIFT);
    if(mods & Qt::META)
        m_activekeys.insert(Qt::META);

    switch(event->key())
    {
    case Qt::Key_Control :
    case Qt::Key_Alt :
    case Qt::Key_Shift :
    case Qt::Key_Meta :
        break;
    default:
        m_activekeys.insert(event->key());
    }

    QSet<INT32>::const_iterator ite = m_activekeys.begin();
    for(;ite != m_activekeys.end();ite++)
        m_hotkey.push_back(*ite);

    ui.keycompEdit->setText(getHotKeyText(m_hotkey));
}

#elif defined(Q_OS_DARWIN)

void KeyCompDlg::keyPressEvent(QKeyEvent* event)
{
    qDebug() << "KeyCompDlg::keyPressEvent";

    qDebug() << "Native: " << QString("%1").arg(event->nativeModifiers(), 0, 16)
             << "modifiers: " << QString("%1").arg(event->modifiers(), 0, 16)
             << "key: " << QString("%1").arg(event->key(), 0, 16);

    if(m_hotkey.isEmpty())
    {
        for(int i=0;i<MAC_HOTKEY_SIZE;i++)
            m_hotkey.push_back(MAC_NO_KEY);
    }

    switch(event->key())
    {
    case 0 : break;
    case Qt::Key_Control :
        m_hotkey[0] = m_hotkey[0] == (INT32)MAC_NO_KEY? 0 : m_hotkey[0];
        m_hotkey[0] |= cmdKey;
        qDebug() << "Ctrl";
        break;
    case Qt::Key_Alt :
        m_hotkey[0] = m_hotkey[0] == (INT32)MAC_NO_KEY? 0 : m_hotkey[0];
        m_hotkey[0] |= optionKey;
        qDebug() << "Alt";
        break;
    case Qt::Key_Shift :
        m_hotkey[0] = m_hotkey[0] == (INT32)MAC_NO_KEY? 0 : m_hotkey[0];
        m_hotkey[0] |= shiftKey;
        qDebug() << "Shift";
        break;
    case Qt::Key_Meta :
        m_hotkey[0] = m_hotkey[0] == (INT32)MAC_NO_KEY? 0 : m_hotkey[0];
        m_hotkey[0] |= controlKey;
        qDebug() << "MEta";
        break;
    default:
        m_hotkey[1] = event->nativeVirtualKey();
        qDebug() << "VK " << event->nativeVirtualKey();
        break;
    }
    
    m_activekeys.insert(event->key());

    Q_ASSERT(m_hotkey.size() == 2);

    ui.keycompEdit->setText(getHotKeyText(m_hotkey));
}

void KeyCompDlg::keyReleaseEvent(QKeyEvent* event)
{
    qDebug() << "KeyCompDlg::keyReleaseEvent";

    qDebug() << "Native: " << QString("%1").arg(event->nativeModifiers(), 0, 16)
             << "modifiers: " << QString("%1").arg(event->modifiers(), 0, 16)
             << "key: " << QString("%1").arg(event->key(), 0, 16)
             << "active keys count " << m_activekeys.size();

    if(event->isAutoRepeat())
        return;

    m_activekeys.remove(event->key());

    bool close_dlg = false;

    if(m_activekeys.isEmpty())
        close_dlg = true;

    //when holding cmdKey and pressing another key hides the other
    //key-release event, so we just have to close the dialog
    switch(event->key())
    {
    case Qt::Key_Control :
        close_dlg = true;
        qDebug() << "Closing dialog due to cmdKey release";
        break;
    }

    if(close_dlg && m_hotkey[0] != (INT32)MAC_NO_KEY && m_hotkey[1] == (INT32)MAC_NO_KEY)
    {
        QMessageBox::information(this, tr("Invalid key combination"),
                                 tr("macOS does not support only modifier keys, "
                                    "i.e. Cmd, Option and Shift must be used in "
                                    "combination with other non-modifier keys."));

        m_hotkey[0] = m_hotkey[1] = MAC_NO_KEY;
    }
    if(close_dlg)
        this->accept();
}

#endif

#if defined(Q_OS_LINUX)

void KeyCompDlg::keyReleaseEvent(QKeyEvent* event)
{
    if(event->isAutoRepeat())
        return;
    if(event->modifiers() == 0)
        this->accept();
}

#endif
