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

protected:
    virtual void slotContextMenu(const QPoint& p);

protected:
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    void enterEvent(QEnterEvent* event);
#else
    void enterEvent(QEvent* event);
#endif
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
