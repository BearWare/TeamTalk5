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

#include "chattextedit.h"
#include "settings.h"
#include "appinfo.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QMenu>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSyntaxHighlighter>
#include <QTextCursor>
#include <QUrl>
#include <QMessageBox>
#include <QClipboard>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

ChatTextEdit::ChatTextEdit(QWidget * parent/* = 0*/)
: QListWidget(parent)
{
    setMouseTracking(true);
}

QString ChatTextEdit::getTimeStamp(const QDateTime& tm, bool force_ts)
{
    QString dt;
    if(ttSettings->value(SETTINGS_DISPLAY_MSGTIMESTAMP, false).toBool() || force_ts)
        dt = tm.toString(tr("yyyy-MM-dd HH:mm:ss")) + QString(" ");
    return dt;
}

QString ChatTextEdit::getTextMessagePrefix(const TextMessage& msg, const User& user)
{
    switch (msg.nMsgType)
    {
    case MSGTYPE_USER :
        return QString("<%1>").arg(getDisplayName(user));
    case MSGTYPE_CHANNEL :
        if (msg.nChannelID != TT_GetMyChannelID(ttInst))
        {
            TTCHAR chpath[TT_STRLEN] = {};
            TT_GetChannelPath(ttInst, msg.nChannelID, chpath);
            return QString("<%1->%2>").arg(getDisplayName(user)).arg(_Q(chpath));
        }
        else
            return QString("<%1>").arg(getDisplayName(user));
    case MSGTYPE_BROADCAST :
        return QString("<%1->BROADCAST>").arg(getDisplayName(user));
    case MSGTYPE_CUSTOM :
    case MSGTYPE_NONE : break;
    }
    return QString();
}

void ChatTextEdit::updateServer(const ServerProperties& srvprop)
{
    addItem("");

    QString dt = getTimeStamp(QDateTime::currentDateTime());
    
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(dt + tr("Server Name: %1").arg(_Q(srvprop.szServerName)));
    item->setFont(QFont("Arial", -1, QFont::Bold));
    addItem(item);
    
    if (_Q(srvprop.szMOTD).size() > 0)
    {
        if (ttSettings->value(SETTINGS_DISPLAY_MOTD_DLG, SETTINGS_DISPLAY_MOTD_DLG_DEFAULT).toBool() == true)
        {
            QMessageBox::information(this, tr("Welcome"), QString(tr("Welcome to %1.\r\nMessage of the day: %2")).arg(_Q(srvprop.szServerName)).arg(_Q(srvprop.szMOTD)));
        }
        else
        {
            item = new QListWidgetItem();
            item->setText(dt + tr("Message of the Day: %1").arg(_Q(srvprop.szMOTD)) + "\r\n");
            item->setForeground(Qt::darkCyan);
            addItem(item);
        }
    }
    
    limitText();
}

void ChatTextEdit::joinedChannel(int channelid)
{
    TTCHAR buff[TT_STRLEN];
    Channel chan;
    if(!TT_GetChannel(ttInst, channelid, &chan))
        return;
    if(!TT_GetChannelPath(ttInst, channelid, buff))
        return;

    addItem("");

    QString dt = getTimeStamp(QDateTime::currentDateTime());
    
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(dt + tr("Joined channel %1").arg(_Q(buff)));
    item->setFont(QFont("Arial", -1, QFont::Bold));
    item->setForeground(Qt::darkGreen);
    addItem(item);

    item = new QListWidgetItem();
    item->setText(tr("Topic: %1").arg(_Q(chan.szTopic)));
    item->setForeground(Qt::darkYellow);
    addItem(item);

    item = new QListWidgetItem();
    item->setText(tr("Disk quota: %1 KBytes").arg(chan.nDiskQuota/1024));
    item->setForeground(Qt::darkRed);
    addItem(item);

    limitText();
}

QString ChatTextEdit::addTextMessage(const MyTextMessage& msg)
{
    User user;
    if (!TT_GetUser(ttInst, msg.nFromUserID, &user))
        return QString();

    QString dt = getTimeStamp(msg.receiveTime);
    QString line = dt + QString("%1\r\n%2").arg(getTextMessagePrefix(msg, user)).arg(msg.moreMessage);

    QListWidgetItem* item = new QListWidgetItem();
    item->setText(line);

    if (TT_GetMyUserID(ttInst) == msg.nFromUserID)
    {
        item->setForeground(Qt::darkGray);
    }
    
    addItem(item);
    limitText();

    return line;
}

void ChatTextEdit::addLogMessage(const QString& msg)
{
    QString line = QString("%1 * %2").arg(getTimeStamp(QDateTime::currentDateTime())).arg(msg);
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(line);
    item->setForeground(Qt::gray);
    addItem(item);
    limitText();
}

void ChatTextEdit::clearHistory()
{
    clear();
}

void ChatTextEdit::limitText()
{
    while(count() > 1000)
    {
        delete takeItem(0);
    }
}

QString ChatTextEdit::currentUrl(const QListWidgetItem* item) const
{
    QString text = item->text();
    QRegularExpression urlPattern("(http[s]?://\\S+)");
    QRegularExpressionMatch match = urlPattern.match(text);
    if (match.hasMatch()) {
        return match.captured(0);
    }
    return QString();
}

void ChatTextEdit::mouseMoveEvent(QMouseEvent *e)
{
    QListWidgetItem* item = itemAt(e->pos());
    if (item && currentUrl(item).size()) {
        setCursor(QCursor(Qt::PointingHandCursor));
    } else {
        setCursor(QCursor(Qt::IBeamCursor));
    }
    QListWidget::mouseMoveEvent(e);
}

void ChatTextEdit::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::RightButton)
        return;

    QListWidgetItem* item = itemAt(e->pos());
    if (item) {
        QString url = currentUrl(item);
        if (url.size()) {
            QDesktopServices::openUrl(QUrl(url));
        }
    }
    QListWidget::mouseReleaseEvent(e);
}

void ChatTextEdit::keyPressEvent(QKeyEvent* e)
{
    QListWidget::keyPressEvent(e);

    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    {
        QListWidgetItem* item = currentItem();
        if (item) {
            QString url = currentUrl(item);
            if (url.size()) {
                QDesktopServices::openUrl(QUrl(url));
            }
        }
    }
}

void ChatTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    menu->addAction(tr("Copy"), [this]() {
        QListWidgetItem* item = currentItem();
        if (item) {
            QApplication::clipboard()->setText(item->text());
        }
    });
    menu->addAction(tr("&Clear"), this, &ChatTextEdit::clear);
    QAction* chosen = menu->exec(event->globalPos());
    if (chosen && chosen->text() == tr("&Clear")) {
        clearHistory();
    }
    delete menu;
}
