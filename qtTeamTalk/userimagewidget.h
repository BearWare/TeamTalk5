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

#ifndef USERIMAGEWIDGET_H
#define USERIMAGEWIDGET_H


#include <QWidget>
#include <QImage>
#include <QPainter>

//#define USE_TT_PAINT 1

//size of border if used in grid
#define BORDER_WIDTH 1

class UserImageWidget : public QWidget
{
    Q_OBJECT

public:
    UserImageWidget(QWidget * parent, int userid, bool scaled, int border_width);
    ~UserImageWidget();

    virtual void paintEvent(QPaintEvent *p);

    virtual void setUserID(int userid) { m_userid = userid; }
    int getUserID() const { return m_userid; }

    virtual QSize imageSize() const = 0;

    int getBorderSize() const { return m_border_width; }

signals:
    void userWidgetFocused(int userid, bool focused);

protected slots:
    virtual void slotContextMenu(const QPoint& p);

protected:
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
 
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);

    int m_userid;

#ifdef USE_TT_PAINT
    virtual void runTTPaint(QPainter& painter) = 0;
#else
    virtual void completePaint(const QRect& r, QPainter& painter) { Q_UNUSED(r);Q_UNUSED(painter); }
    QImage m_image;
#endif
    bool m_mouseover;
    QString m_nickname;
    QString m_textmsg;
    bool m_scaled;
    int m_border_width;
    QPoint m_img_offset; //top-left corner in 'm_image' where painting should start
    QPoint m_paint_offset; //top-left corner where 'm_image' is painted in the widget
    bool m_mirrored;
};

#endif
