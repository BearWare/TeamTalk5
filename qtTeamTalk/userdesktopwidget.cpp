/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#include "userdesktopwidget.h"
#include <QPaintEngine>
#include <QMouseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>

#include "common.h" //need to include Carbon.h

extern TTInstance* ttInst;

UserDesktopWidget::UserDesktopWidget(QWidget* parent, int userid, int border_width)
    : UserImageWidget(parent, userid, false, border_width)
    , m_desktop_updated(true)
    , m_access_requested(false)
    , m_mousedown(false)
    , m_desktop_window(NULL)
    , m_user()
{
    m_refresh_timerid = startTimer(100);
    m_sendinput_timerid = startTimer(50);

    setMouseTracking(true);

    setUserID(userid);
}

UserDesktopWidget::~UserDesktopWidget()
{
    TT_ReleaseUserDesktopWindow(ttInst, m_desktop_window);
}

QSize UserDesktopWidget::imageSize() const
{
#ifdef USE_TT_PAINT
    m_desktop_window = TT_AcquireUserDesktopWindow(ttInst, m_userid);
    if(!m_desktop_window)
        return QSize();
    return QSize(m_desktop_window->nWidth, m_desktop_window->nHeight);
#else
    return m_image.size();
#endif
}

void UserDesktopWidget::paintEvent(QPaintEvent *p)
{
    UserImageWidget::paintEvent(p);

    if(!m_cursorpos.isNull())
    {
        QPainter painter(this);
        QPoint topleft(m_cursorpos.x() - m_img_offset.x() + m_paint_offset.x() - 3,
                       m_cursorpos.y() - m_img_offset.y() + m_paint_offset.y() - 3);
        QRect r(topleft, QSize(5, 5));
        painter.drawRect(r);
        topleft.setX(topleft.x() + 1);
        topleft.setY(topleft.y() + 1);
        r = QRect(topleft, QSize(4, 4));
        painter.fillRect(r, QBrush(QColor(128,255,86)));
    }
}

void UserDesktopWidget::setUserID(int userid)
{
    UserImageWidget::setUserID(userid);
    TT_GetUser(ttInst, userid, &m_user);
    m_nickname = getDisplayName(m_user);
}

void UserDesktopWidget::timerEvent(QTimerEvent* e)
{
    if(e->timerId() == m_refresh_timerid)
        refreshTimeout();
    else if(e->timerId() == m_sendinput_timerid)
        sendDesktopInputTimeout();
}

void UserDesktopWidget::refreshTimeout()
{
    if(!m_desktop_updated)
        return;

#ifndef USE_TT_PAINT
    DesktopWindow* wnd = TT_AcquireUserDesktopWindow(ttInst, m_userid);
    if(wnd)
        TT_ReleaseUserDesktopWindow(ttInst, m_desktop_window);
    m_desktop_window = wnd;
    if(!m_desktop_window)
    {
        emit(userDesktopWindowEnded(m_userid));
        return;
    }

    QImage::Format fmt;
    switch(m_desktop_window->bmpFormat)
    {
    case BMP_RGB8_PALETTE :
        fmt = QImage::Format_Indexed8;
        break;
    case BMP_RGB16_555 :
        //fmt = QImage::Format_RGB16;
        fmt = QImage::Format_RGB555;
        break;
    case BMP_RGB24 :
        fmt = QImage::Format_RGB888;
        break;
    case BMP_RGB32 :
    default :
        fmt = QImage::Format_RGB32;
        break;
    }

    m_image = QImage(static_cast<uchar*>(m_desktop_window->frameBuffer),
                        m_desktop_window->nWidth, m_desktop_window->nHeight, fmt);
    Q_ASSERT(!m_image.isNull());
    if(m_desktop_window->bmpFormat == BMP_RGB8_PALETTE)
    {
        QVector<QRgb> colors(256);
        for(int i=0;i<256;i++)
        {
            unsigned char* rgb = TT_Palette_GetColorTable(BMP_RGB8_PALETTE, i);
            //Qt swaps red and blue??
            colors[i] = qRgb( rgb[2], rgb[1], rgb[0]);
        }
        m_image.setColorTable(colors);
    }
    m_img_offset = QPoint(0,0);

    //TODO: Qt swaps red and blue. Very ineffective way of getting around this.
    if(fmt == QImage::Format_RGB888)
        m_image = m_image.rgbSwapped();
#endif

    m_desktop_updated = false;

    update();
}

void UserDesktopWidget::sendDesktopInputTimeout()
{
    while(m_sendinput_queue.size())
    {
        int n_send = qMin(m_sendinput_queue.size(), TT_DESKTOPINPUT_MAX);
        if(TT_SendDesktopInput(ttInst, m_userid, &m_sendinput_queue[0], n_send))
            m_sendinput_queue.remove(0, n_send);
        else break;
    }
}

#ifdef USE_TT_PAINT
void UserDesktopWidget::runTTPaint(QPainter& painter)
{
    HDC hdc = painter.paintEngine()->getDC();
    QPoint p0 = this->mapTo(nativeParentWidget(), QPoint(0,0));

    //if(!TT_PaintDesktopWindow(ttInst, m_userid, hdc, p0.x(), p0.y(), width(), height()))
    //    emit(userDesktopWindowEnded(m_userid));

    QSize size = imageSize();
    int w = qMin(size.width(), width());
    int h = qMin(size.height(), height());

    if(!TT_PaintDesktopWindowEx(ttInst, m_userid, hdc, p0.x(), p0.y(), width(), height(),
                                0, 0, w, h))
        emit(userDesktopWindowEnded(m_userid));

    painter.paintEngine()->releaseDC(hdc);
}
#endif


void UserDesktopWidget::slotDesktopUpdate(int userid, int sessionid)
{
    Q_ASSERT(m_userid > 0);

    Q_UNUSED(sessionid);
    if(userid != m_userid)
        return;

    //Don't update display whenever the desktop window is updated.
    //It will take too much CPU.
    //Better to start a timer which detects changes
    m_desktop_updated = true;
}

void UserDesktopWidget::slotDesktopCursorUpdate(int src_userid,
                                                const DesktopInput& input)
{
    if(src_userid != m_userid)
        return;

    m_desktop_updated = true;

    if(input.uMousePosX < 0 || input.uMousePosY < 0)
        m_cursorpos = QPoint();
    else
        m_cursorpos = QPoint(input.uMousePosX, input.uMousePosY);

    update();
}

void UserDesktopWidget::slotUserUpdated(const User& user)
{
    if(user.nUserID == m_userid)
        m_nickname = getDisplayName(m_user);
}

void UserDesktopWidget::slotContextMenu(const QPoint& p)
{
    Q_UNUSED(p);

    //don't show context menu if we're currently using desktop access
    if(m_user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT)
        return;

    QMenu menu(this);
#ifndef USE_TT_PAINT
    QAction* save = menu.addAction(tr("&Save to Image File"));
#endif
    menu.addSeparator();

    QAction* access = NULL;
    if(m_access_requested)
        access = menu.addAction(tr("Retract &Desktop Access"));
    else
        access = menu.addAction(tr("Request &Desktop Access"));
    access->setIcon(QIcon(QString::fromUtf8(":/images/images/chalkstick.png")));
    QAction* result = menu.exec(QCursor::pos());
#ifndef USE_TT_PAINT
    if(result == save)
    {
        QString name = QFileDialog::getSaveFileName(this, tr("Save File"), 
                                                    "", tr("PNG files (*.png)"));
        if(name.size() && !m_image.save(name, "PNG"))
            QMessageBox::critical(this, tr("&Save to Image File"), tr("Failed to save file."));
    }
    else
#endif
    if(result == access)
    {
        TextMessage msg;
        ZERO_STRUCT(msg);
        msg.nFromUserID = TT_GetMyUserID(ttInst);
        msg.nMsgType = MSGTYPE_CUSTOM;
        msg.nToUserID = m_userid;
        m_access_requested = !m_access_requested;
        QString cmd = makeCustomCommand(TT_INTCMD_DESKTOP_ACCESS, 
                                        QString::number(m_access_requested));
        COPY_TTSTR(msg.szMessage, cmd);
        TT_DoTextMessage(ttInst, &msg);
    }
}

void UserDesktopWidget::mousePressEvent(QMouseEvent* event)
{
    UserImageWidget::mousePressEvent(event);
    m_mousedown = true;
    m_old_pos = event->pos();

    quint32 key_code = TT_DESKTOPINPUT_KEYCODE_IGNORE;
    switch(event->button())
    {
    case Qt::LeftButton :
        key_code = TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN;
        break;
    case Qt::MidButton :
        key_code = TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN;
        break;
    case Qt::RightButton :
        key_code = TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN;
        break;
    default:
        break;
    }
    sendMouseEvent(event->pos(), key_code, DESKTOPKEYSTATE_DOWN);
}

void UserDesktopWidget::mouseReleaseEvent(QMouseEvent* event)
{
    UserImageWidget::mouseReleaseEvent(event);
    m_mousedown = false;

    quint32 key_code = TT_DESKTOPINPUT_KEYCODE_IGNORE;
    switch(event->button())
    {
    case Qt::LeftButton :
        key_code = TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN;
        break;
    case Qt::MidButton :
        key_code = TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN;
        break;
    case Qt::RightButton :
        key_code = TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN;
        break;
    default:
        break;
    }
    sendMouseEvent(event->pos(), key_code, DESKTOPKEYSTATE_UP);
}

void UserDesktopWidget::mouseMoveEvent(QMouseEvent* event)
{
    UserImageWidget::mouseMoveEvent(event);

    QPoint pos = event->pos();

    if(m_mousedown &&
       (m_user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT) == SUBSCRIBE_NONE)
    {
        int widget_width = this->width();
        int widget_height = this->height();

        int x_max = imageSize().width() - widget_width - 2 * m_border_width;
        int y_max = imageSize().height() - widget_height  - 2 * m_border_width;

        int x_diff = pos.x() - m_old_pos.x();
        int y_diff = pos.y() - m_old_pos.y();

        int new_x = m_img_offset.x() + x_diff * -1; 
        int new_y = m_img_offset.y() + y_diff * -1; 

        new_x = qMax(new_x, 0);
        new_x = qMin(new_x, x_max);
        new_y = qMax(new_y, 0);
        new_y = qMin(new_y, y_max);

        m_img_offset.setX(new_x);
        m_img_offset.setY(new_y);

        update();
    }

    if((m_user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT) == SUBSCRIBE_NONE)
        m_old_pos = pos;

    sendMouseEvent(event->pos(), TT_DESKTOPINPUT_KEYCODE_IGNORE,
                   DESKTOPKEYSTATE_NONE);
}

void UserDesktopWidget::sendMouseEvent(const QPoint& p, quint32 keycode,
                                       DesktopKeyState keystate)
{
    //only queue desktop input events if user subscribes
    if(m_user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT)
    {
        DesktopInput deskinput;
        ZERO_STRUCT(deskinput);
        //qDebug() << "Offset : " << m_mouseinput_offset.x() << "," << m_mouseinput_offset.y();
        deskinput.uMousePosX = p.x() + m_img_offset.x() - m_paint_offset.x();
        deskinput.uMousePosY = p.y() + m_img_offset.y() - m_paint_offset.y();
        deskinput.uKeyCode = keycode;
        deskinput.uKeyState = keystate;

        translateSend(deskinput);
    }
}

void UserDesktopWidget::keyPressEvent(QKeyEvent* event)
{
    if(event->key() != Qt::Key_Escape)
        UserImageWidget::keyPressEvent(event);

    quint32 keycode;
#if defined(Q_OS_WIN32)
    keycode = event->nativeScanCode();
#elif defined(Q_OS_DARWIN)
    switch(event->key())
    {
    case Qt::Key_Control :
        keycode = kVK_Command;
        break;
    case Qt::Key_Alt :
        keycode = kVK_Option;
        break;
    case Qt::Key_Shift :
        keycode = kVK_Shift;
        break;
    case Qt::Key_Meta :
        keycode = kVK_Control;
        break;
    default :
        keycode = event->nativeVirtualKey();
        break;
    }
#else
    keycode = event->nativeVirtualKey();
#endif
    sendKeyEvent(keycode, DESKTOPKEYSTATE_DOWN);
}

void UserDesktopWidget::keyReleaseEvent(QKeyEvent* event)
{
    if(event->key() != Qt::Key_Escape)
        UserImageWidget::keyReleaseEvent(event);

    quint32 keycode;
#if defined(Q_OS_WIN32)
    keycode = event->nativeScanCode();
#elif defined(Q_OS_DARWIN)
    switch(event->key())
    {
    case Qt::Key_Control :
        keycode = kVK_Command;
        break;
    case Qt::Key_Alt :
        keycode = kVK_Option;
        break;
    case Qt::Key_Shift :
        keycode = kVK_Shift;
        break;
    case Qt::Key_Meta :
        keycode = kVK_Control;
        break;
    default :
        keycode = event->nativeVirtualKey();
        break;
    }
#else
    keycode = event->nativeVirtualKey();
#endif
    sendKeyEvent(keycode, DESKTOPKEYSTATE_UP);
}

void UserDesktopWidget::sendKeyEvent(quint32 keycode,
                                     DesktopKeyState keystate)
{
    //only queue desktop input events if user subscribes
    if(m_user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT)
    {
        DesktopInput deskinput;
        ZERO_STRUCT(deskinput);
    
        //terminate mouse
        deskinput.uMousePosX = TT_DESKTOPINPUT_MOUSEPOS_IGNORE;
        deskinput.uMousePosY = TT_DESKTOPINPUT_MOUSEPOS_IGNORE;

        //insert key input
        deskinput.uKeyCode = keycode;
        deskinput.uKeyState = keystate;

        translateSend(deskinput);
    }
}

void UserDesktopWidget::translateSend(const DesktopInput& deskinput)
{
    DesktopInput desksend;
    ZERO_STRUCT(desksend);

    TTKeyTranslate key_trans = TTKEY_NO_TRANSLATE;
#if ENABLE_KEY_TRANSLATION
#if defined(Q_OS_WIN32)
    key_trans = TTKEY_WINKEYCODE_TO_TTKEYCODE;
#elif defined(Q_OS_DARWIN)
    key_trans = TTKEY_MACKEYCODE_TO_TTKEYCODE;
#elif defined(Q_OS_LINUX)
    //TODO: X11, key translate
    qDebug() << "No key translate for X11";
#endif
#endif /* ENABLE_KEY_TRANSLATION */

    if(TT_DesktopInput_KeyTranslate(key_trans, &deskinput,
                                    &desksend, 1))
        m_sendinput_queue.push_back(desksend);
    else
        qDebug() << "Failed to translate desktop input for transmission. KeyCode: 0x" 
                 << QString::number(deskinput.uKeyCode, 16);
 
}
