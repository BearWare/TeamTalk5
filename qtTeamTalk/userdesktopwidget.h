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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#ifndef USERDESKTOPWIDGET_H
#define USERDESKTOPWIDGET_H

#include "userimagewidget.h"
#include "common.h"

class UserDesktopWidget : public UserImageWidget
{
    Q_OBJECT

public:
    UserDesktopWidget(QWidget* parent, int userid = 0, int border_width = 0);
    ~UserDesktopWidget();

    QSize imageSize() const;

    void paintEvent(QPaintEvent *p);

    void setUserID(int userid);

public slots:
    void slotDesktopUpdate(int userid, int sessionid);
    void slotDesktopCursorUpdate(int src_userid, const DesktopInput& input);
    void slotUserUpdated(const User& user);

signals:
    void userDesktopWindowEnded(int userid);

protected slots:
    void slotContextMenu(const QPoint& p);

protected:
    /* Qt inherit */
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    
    //TT send mouse event as desktop input
    void sendMouseEvent(const QPoint& p, quint32 keycode,
                        DesktopKeyState keystate);

    /* Qt inherit */
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    //TT send key event as desktop input
    void sendKeyEvent(quint32 keycode, DesktopKeyState keystate);

    void translateSend(const DesktopInput& deskinput);

#ifdef USE_TT_PAINT
    void runTTPaint(QPainter& painter);
#endif
    void timerEvent(QTimerEvent *e);
    void refreshTimeout();
    void sendDesktopInputTimeout();

    int m_refresh_timerid;
    bool m_desktop_updated;
    QPoint m_cursorpos; //shared desktop cursor

    bool m_access_requested;

    bool m_mousedown;
    QPoint m_old_pos;
    //Desktop window to display
    DesktopWindow* m_desktop_window;
    //transmission queue for desktop input
    QVector<DesktopInput> m_sendinput_queue;
    int m_sendinput_timerid;
    //cache of user data
    User m_user;
};

#endif
