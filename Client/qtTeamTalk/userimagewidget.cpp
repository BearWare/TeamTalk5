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

#include "userimagewidget.h"
#include "common.h"

#include <QPaintEngine>
#include <QFocusEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QMenu>

UserImageWidget::UserImageWidget(QWidget * parent/* = 0 */, 
                                 int userid, bool scaled,
                                 int border_width)
: QWidget(parent)
, m_userid(userid)
, m_mouseover(false)
, m_scaled(scaled)
, m_border_width(border_width)
, m_img_offset(QPoint(0,0))
, m_paint_offset(QPoint(0,0))
, m_mirrored(false)
{
    //setAttribute(Qt::WA_OpaquePaintEvent);
#if defined(Q_OS_WIN32)
    setAttribute(Qt::WA_PaintOnScreen);
#endif

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QWidget::customContextMenuRequested,
            this, &UserImageWidget::slotContextMenu);
}

UserImageWidget::~UserImageWidget()
{
}

void UserImageWidget::paintEvent(QPaintEvent* p)
{
    Q_UNUSED(p);
    QPainter painter(this);

    //widget rect
    QRect r = rect();
    //image rect
    QRect src_rect(0, 0, imageSize().width(), imageSize().height());

    QSize img_size = imageSize();

    if(m_scaled)
    {
        QSize scaled = imageSize();
        scaled.scale(qMax(0, r.width() - 2 * m_border_width), 
                     qMax(0, r.height() - 2 * m_border_width), Qt::KeepAspectRatio);
        r.setWidth(scaled.width());
        r.setHeight(scaled.height());
    }
    else if(img_size.isValid())
    {
        int bw2 = m_border_width * 2;
        if(r.width() > img_size.width() + bw2)
            r.setWidth(img_size.width() + bw2);
        else
            src_rect.setWidth(r.width() - bw2);

        if(r.height() > img_size.height() + bw2)
            r.setHeight(img_size.height() + bw2);
        else
            src_rect.setHeight(r.height() - bw2);

        Q_ASSERT(src_rect.width() == r.width() - bw2);
        Q_ASSERT(src_rect.height() == r.height() - bw2);

        int x = m_img_offset.x();
        if(x + r.width() - bw2 > imageSize().width())
            x = imageSize().width() - r.width() - bw2;
        x = qMax(0, x);

        Q_ASSERT(x + r.width() - bw2 <= imageSize().width());

        src_rect.setLeft(x);
        src_rect.setWidth(r.width() - bw2);

        int y = m_img_offset.y();
        if(y + r.height() - bw2 > imageSize().height())
            y = imageSize().height() - r.height() - bw2;
        y = qMax(0, y);

        Q_ASSERT(y + r.height() - bw2 <= imageSize().height());

        src_rect.setTop(y);
        src_rect.setHeight(r.height() - bw2);

        Q_ASSERT(src_rect.width() == r.width() - bw2);
        Q_ASSERT(src_rect.height() == r.height() - bw2);
    }

    QPen org_pen = painter.pen();

    if(m_border_width)
    {
        QColor color;
        if(hasFocus())
            color = QPalette().color(QPalette::Highlight);
        else
            color = QColor(Qt::gray);

        painter.fillRect(r, color);

        r.setTop(r.top() + m_border_width);
        r.setLeft(r.left() + m_border_width);
        r.setRight(r.right() - m_border_width);
        r.setBottom(r.bottom() - m_border_width);
    }
    else
    {
        //keep centered
        QRect wnd_rect = rect();
        if(r.width() < wnd_rect.width())
        {
            int h_diff = (wnd_rect.width()-r.width())/2;
            r.setLeft(r.left() + h_diff);
            r.setRight(r.right() + h_diff);
        }
        if(r.height() < wnd_rect.height())
        {
            int v_diff = (wnd_rect.height() - r.height())/2;
            r.setTop(r.top() + v_diff);
            r.setBottom(r.bottom() + v_diff);
        }
    }

    m_paint_offset = QPoint(r.left(), r.top());

    painter.setPen(org_pen);

#ifdef USE_TT_PAINT
    runTTPaint(painter);
#else
    if(m_image.isNull())
    {
        //QWidget::paintEvent(p);
        painter.drawText(r, Qt::AlignCenter, m_textmsg);
    }
    else
    {
        if(!m_scaled)
        {
            Q_ASSERT(r.width() == src_rect.width());
            Q_ASSERT(r.height() == src_rect.height());
            Q_ASSERT(src_rect.width() <= imageSize().width());
            Q_ASSERT(src_rect.height() <= imageSize().height());
            painter.drawImage(r, m_image, src_rect);
        }
        else
            painter.drawImage(r, m_image);
    }

    completePaint(r, painter);

    //print name of user if mouse over
    if(m_mouseover && m_userid && m_nickname.size())
    {
        QPen old = painter.pen();
        painter.setPen(QPen(QColor(Qt::green)));
        painter.drawText(r, Qt::AlignHCenter | Qt::AlignTop, 
                         m_nickname);
        painter.setPen(old);
    }
#endif    
}

void UserImageWidget::slotContextMenu(const QPoint& /*p*/)
{
#ifndef USE_TT_PAINT
    QMenu menu(this);
    QAction* save = menu.addAction(tr("&Save to Image File"));
    QAction* mirror = menu.addAction(tr("&Flip image"));
    mirror->setCheckable(true);
    mirror->setChecked(m_mirrored);
    QAction* result = menu.exec(QCursor::pos());
    if(result == save)
    {
        QString name = QFileDialog::getSaveFileName(this, tr("Save File"), 
                                                    "", tr("PNG files (*.png)"));
        if(name.size() && !m_image.save(name, "PNG"))
            QMessageBox::critical(this, tr("&Save to Image File"), tr("Failed to save file."));
    }
    else if(result == mirror)
    {
        m_mirrored = !m_mirrored;
        m_image = m_image.mirrored(m_mirrored, m_mirrored);
        update();
    }
#endif
}

void UserImageWidget::focusInEvent(QFocusEvent* event)
{
    QWidget::focusInEvent(event);
    if(focusPolicy() != Qt::NoFocus)
        emit userWidgetFocused(m_userid, event->gotFocus());
}

void UserImageWidget::focusOutEvent(QFocusEvent* event)
{
    QWidget::focusOutEvent(event);
    if(focusPolicy() != Qt::NoFocus)
        emit userWidgetFocused(m_userid, event->gotFocus());
}

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
void UserImageWidget::enterEvent(QEnterEvent* event)
#else
void UserImageWidget::enterEvent(QEvent* event)
#endif
{
    QWidget::enterEvent(event);

    //so the name can be painted
    if(!m_mouseover)
        update();

    m_mouseover = true;
}

void UserImageWidget::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);

    //so the name can be removed
    if(m_mouseover)
        update();

    m_mouseover = false;
}


