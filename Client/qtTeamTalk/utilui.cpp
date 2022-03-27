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

#include "utilui.h"
#include "settings.h"
#include "bearwarelogindlg.h"

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QDesktopWidget>
#include <QApplication>
#else
#include <QScreen>
#include <QGuiApplication>
#endif

extern TTInstance* ttInst;
extern QSettings* ttSettings;

void setVideoTextBox(const QRect& rect, const QColor& bgcolor,
                     const QColor& fgcolor, const QString& text,
                     quint32 text_pos, int w_percent, int h_percent,
                     QPainter& painter)
{
    int w = w_percent / 100. * rect.width();
    int h = h_percent / 100. * rect.height();

    int x, y;
    switch(text_pos & VIDTEXT_POSITION_MASK)
    {
    case VIDTEXT_POSITION_TOPLEFT :
        x = 0; y = 0;
        break;
    case VIDTEXT_POSITION_TOPRIGHT :
        x = rect.width() - w;
        y = 0;
        break;
    case VIDTEXT_POSITION_BOTTOMLEFT :
        x = 0;
        y = rect.height() - h;
        break;
    case VIDTEXT_POSITION_BOTTOMRIGHT :
    default :
        x = rect.width() - w;
        y = rect.height() - h;
        break;
    }

    if(h>0 && w>0)
    {
        const QFont font = painter.font();
        if(font.pixelSize() != h)
        {
            QFont newFont(font);
            newFont.setPixelSize(h);
            painter.setFont(newFont);
        }
        painter.fillRect(x, y, w, h, bgcolor);
        painter.setPen(fgcolor);
        painter.drawText(x, y, w, h, Qt::AlignHCenter | Qt::AlignCenter, text);

        if(font.pixelSize() != h)
            painter.setFont(font);
    }
}

#if defined(Q_OS_DARWIN)
void setMacResizeMargins(QDialog* dlg, QLayout* layout)
{
    QSize size = dlg->size();
    QMargins margins = layout->contentsMargins();
    margins.setBottom(margins.bottom()+12);
    layout->setContentsMargins(margins);
    size += QSize(0, 12);
    dlg->resize(size);
}
#endif /* Q_OS_DARWIN */

void setCurrentItemData(QComboBox* cbox, const QVariant& itemdata)
{
    int index = cbox->findData(itemdata);
    if(index>=0)
        cbox->setCurrentIndex(index);
}

QVariant getCurrentItemData(QComboBox* cbox, const QVariant& not_found/* = QVariant()*/)
{
    if(cbox->currentIndex()>=0)
        return cbox->itemData(cbox->currentIndex());
    return not_found;
}

QString getBearWareWebLogin(QWidget* parent)
{
    QString username = ttSettings->value(SETTINGS_GENERAL_BEARWARE_USERNAME).toString();
    if (username.isEmpty())
    {
        BearWareLoginDlg dlg(parent);
        if (dlg.exec())
        {
            username = ttSettings->value(SETTINGS_GENERAL_BEARWARE_USERNAME).toString();
        }
    }
    return username;
}

textmessages_t buildTextMessages(const TextMessage& msg, const QString& content)
{
    Q_ASSERT(msg.szMessage[0] == '\0');

    textmessages_t result;
    MyTextMessage newmsg(msg);
    QString remain = content;

    if (remain.toUtf8().size() <= TT_STRLEN - 1)
    {
        COPY_TTSTR(newmsg.szMessage, remain);
        newmsg.bMore = FALSE;
        result.append(newmsg);
        return result;
    }

    newmsg.bMore = TRUE;

    int curlen = remain.size();
    while (remain.left(curlen).toUtf8().size() > TT_STRLEN - 1)
        curlen /= 2;

    int half = TT_STRLEN / 2;
    while (half > 0)
    {
        auto utf8str = remain.left(curlen + half).toUtf8();
        if (utf8str.size() <= TT_STRLEN - 1)
            curlen += half;
        if (utf8str.size() == TT_STRLEN - 1)
            break;
        half /= 2;
    }

    COPY_TTSTR(newmsg.szMessage, remain.left(curlen));
    result.append(newmsg);
    newmsg.szMessage[0] = {'\0'};
    result.append(buildTextMessages(newmsg, remain.mid(curlen)));
    return result;
}

textmessages_t sendTextMessage(const TextMessage& msg, const QString& content)
{
    bool sent = true;
    auto messages = buildTextMessages(msg, content);
    for (const auto& m : messages)
    {
        sent = sent && TT_DoTextMessage(ttInst, &m) > 0;
    }
    return sent ? messages : textmessages_t();
}

RestoreIndex::RestoreIndex(QAbstractItemView* view)
    : m_view(view)
{
    m_parent = view->currentIndex().parent();
    m_row = view->currentIndex().row();
    m_column = view->currentIndex().column();
}

RestoreIndex::~RestoreIndex()
{
    if (m_view->model()->rowCount() == 0 || m_view->model()->columnCount() == 0)
        return;

    m_row = std::min(m_row, m_view->model()->rowCount() - 1);
    m_column = std::min(m_column, m_view->model()->columnCount() - 1);
    m_view->setCurrentIndex(m_view->model()->index(m_row, m_column, m_parent));
}

void saveWindowPosition(const QString& setting, QWidget* widget)
{
    if (widget->windowState() == Qt::WindowNoState)
    {
        QRect r = widget->geometry();
        QVariantList windowpos;
        windowpos.push_back(r.x());
        windowpos.push_back(r.y());
        windowpos.push_back(r.width());
        windowpos.push_back(r.height());
        ttSettings->setValue(setting, windowpos);
    }
}

bool restoreWindowPosition(const QString& setting, QWidget* widget)
{
    bool success = false;
    QVariantList windowpos = ttSettings->value(setting).toList();
    if (windowpos.size() == 4)
    {
        int x = windowpos[0].toInt();
        int y = windowpos[1].toInt();
        int w = windowpos[2].toInt();
        int h = windowpos[3].toInt();

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        int desktopW = QApplication::desktop()->width();
        int desktopH = QApplication::desktop()->height();
        if(x <= desktopW && y <= desktopH)
        {
            widget->setGeometry(x, y, w, h);
            success = true;
        }
#else
        // check that we are within bounds
        QScreen* screen = QGuiApplication::screenAt(QPoint(x, y));
        if (screen)
        {
            widget->setGeometry(x, y, w, h);
            success = true;
        }
#endif
    }
    return success;
}
