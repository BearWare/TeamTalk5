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

#include "chattextlist.h"
#include "settings.h"
#include "appinfo.h"
#include "utilui.h"

#include <QDesktopServices>
#include <QMenu>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSyntaxHighlighter>
#include <QTextCursor>
#include <QUrl>
#include <QMessageBox>
#include <QClipboard>
#include <QLabel>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

ChatTextList::ChatTextList(QWidget * parent/* = 0*/)
: QListWidget(parent)
{
    setMouseTracking(true);
    setWordWrap(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
}

QString ChatTextList::getTimeStamp(const QDateTime& tm)
{
    QString dt = getFormattedDateTime(tm.toString("yyyy-MM-dd HH:mm:ss"), "yyyy-MM-dd HH:mm:ss");
    return dt;
}

QString ChatTextList::getTextMessagePrefix(const TextMessage& msg, const User& user)
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

void ChatTextList::updateServer(const ServerProperties& srvprop)
{
    QString dt = getTimeStamp(QDateTime::currentDateTime());

    QListWidgetItem* item = new QListWidgetItem(dt + tr("Server Name: %1").arg(_Q(srvprop.szServerName)));
    item->setFont(QFont("Arial", -1, QFont::Bold));

    item->setData(Qt::UserRole + 1, dt);
    item->setData(Qt::UserRole + 2, tr("Server"));
    item->setData(Qt::UserRole + 3, tr("Server Name: %1").arg(_Q(srvprop.szServerName)));

    addItem("");
    addItem(item);

    if (_Q(srvprop.szMOTD).size() > 0 && !ttSettings->value(SETTINGS_DISPLAY_MOTD_DLG, SETTINGS_DISPLAY_MOTD_DLG_DEFAULT).toBool())
    {
        QListWidgetItem* motdItem = new QListWidgetItem(dt + tr("Message of the Day: %1").arg(_Q(srvprop.szMOTD)));
        motdItem->setForeground(Qt::darkCyan);

        motdItem->setData(Qt::UserRole + 1, dt);
        motdItem->setData(Qt::UserRole + 2, tr("Server"));
        motdItem->setData(Qt::UserRole + 3, tr("Message of the Day: %1").arg(_Q(srvprop.szMOTD)));

        addItem(motdItem);
    }

    limitText();
}

void ChatTextList::joinedChannel(int channelid)
{
    TTCHAR buff[TT_STRLEN]; Channel chan;
    if(!TT_GetChannel(ttInst, channelid, &chan) || !TT_GetChannelPath(ttInst, channelid, buff)) return;

    QString dt = getTimeStamp(QDateTime::currentDateTime());

    QListWidgetItem* item = new QListWidgetItem(dt + tr("Joined channel %1").arg(_Q(buff)));
    item->setFont(QFont("Arial", -1, QFont::Bold));
    item->setForeground(Qt::darkGreen);

    item->setData(Qt::UserRole + 1, dt);
    item->setData(Qt::UserRole + 2, tr("Channel"));
    item->setData(Qt::UserRole + 3, tr("Joined channel %1").arg(_Q(buff)));

    addItem(""); addItem(item);

    QListWidgetItem* topicItem = new QListWidgetItem(tr("Topic: %1").arg(_Q(chan.szTopic)));
    topicItem->setForeground(Qt::darkYellow);
    topicItem->setData(Qt::UserRole + 1, dt);
    topicItem->setData(Qt::UserRole + 2, tr("Channel"));
    topicItem->setData(Qt::UserRole + 3, tr("Topic: %1").arg(_Q(chan.szTopic)));
    addItem(topicItem);

    QListWidgetItem* quotaItem = new QListWidgetItem(tr("Disk quota: %1").arg(getFormattedSize(chan.nDiskQuota)));
    quotaItem->setForeground(Qt::darkRed);
    quotaItem->setData(Qt::UserRole + 1, dt);
    quotaItem->setData(Qt::UserRole + 2, tr("Channel"));
    quotaItem->setData(Qt::UserRole + 3, tr("Disk quota: %1").arg(getFormattedSize(chan.nDiskQuota)));
    addItem(quotaItem);

    limitText();
}

QString ChatTextList::addTextMessage(const MyTextMessage& msg)
{
    User user;
    if (!TT_GetUser(ttInst, msg.nFromUserID, &user))
        return QString();

    QString dt = getTimeStamp(msg.receiveTime);
    QString sender = getTextMessagePrefix(msg, user);
    QString content = msg.moreMessage;

    QString line;
    switch (msg.nMsgType)
    {
    case MSGTYPE_CHANNEL :
        line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_CHANNELMSG, {{"{date}", dt}, {"{user}", getDisplayName(user)}, {"{content}", content}});
    case MSGTYPE_BROADCAST :
        line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_BROADMSG, {{"{date}", dt}, {"{user}", getDisplayName(user)}, {"{content}", content}});
    case MSGTYPE_USER :
        line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_PRIVMSG, {{"{date}", dt}, {"{user}", getDisplayName(user)}, {"{content}", content}});
    case MSGTYPE_CUSTOM :
    case MSGTYPE_NONE : break;
    }

    QListWidgetItem* item = new QListWidgetItem(line);

    item->setData(Qt::UserRole + 1, dt);
    item->setData(Qt::UserRole + 2, sender);
    item->setData(Qt::UserRole + 3, content);

    if (TT_GetMyUserID(ttInst) == msg.nFromUserID)
        item->setForeground(Qt::darkGray);

    addItem(item);
    limitText();

    return line;
}

void ChatTextList::addLogMessage(const QString& msg)
{
    QString dt = getTimeStamp(QDateTime::currentDateTime());
    QString sender = tr("System");
    QString content = msg;

    QString line = QString("%1 * %2").arg(dt).arg(msg);
    QListWidgetItem* item = new QListWidgetItem(line);
    item->setForeground(Qt::gray);

    item->setData(Qt::UserRole + 1, dt);
    item->setData(Qt::UserRole + 2, sender);
    item->setData(Qt::UserRole + 3, content);

    addItem(item);
    limitText();
}

void ChatTextList::updateTranslation()
{
    setAccessibleName(tr("History"));
}

void ChatTextList::clearHistory()
{
    clear();
}

void ChatTextList::copyAllHistory()
{
    QString allText;
    for(int i = 0; i < count(); ++i)
    {
        QListWidgetItem* item = this->item(i);
        if(item)
        {
            allText += item->text() + "\n";
        }
    }
    QApplication::clipboard()->setText(allText);
}

void ChatTextList::limitText()
{
    while(count() > 1000)
    {
        delete takeItem(0);
    }
}

QStringList ChatTextList::allUrls(const QString &text) const
{
    QStringList urls;
    QRegularExpression urlPattern("(http[s]?://\\S+)");
    QRegularExpressionMatchIterator it = urlPattern.globalMatch(text);
    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        urls << match.captured(0);
    }
    return urls;
}

QString ChatTextList::currentUrl(const QListWidgetItem* item) const
{
    QString text = item->text();
    QRegularExpression urlPattern("(http[s]?://\\S+)");
    QRegularExpressionMatch match = urlPattern.match(text);
    if (match.hasMatch())
    {
        return match.captured(0);
    }
    return QString();
}

void ChatTextList::mouseMoveEvent(QMouseEvent *e)
{
    QListWidgetItem* item = itemAt(e->pos());
    if (item && currentUrl(item).size())
    {
        setCursor(QCursor(Qt::PointingHandCursor));
    }
    else
    {
        setCursor(QCursor(Qt::IBeamCursor));
    }
    QListWidget::mouseMoveEvent(e);
}

void ChatTextList::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::RightButton)
        return;

    QListWidgetItem* item = itemAt(e->pos());
    if (item)
    {
        QString url = currentUrl(item);
        if (url.size())
        {
            QDesktopServices::openUrl(QUrl(url));
        }
    }
    QListWidget::mouseReleaseEvent(e);
}

void ChatTextList::keyPressEvent(QKeyEvent* e)
{
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    {
        QListWidgetItem* item = currentItem();
        if(item)
        {
            QStringList urls = allUrls(item->text());
            if(urls.size() == 1)
            {
                QDesktopServices::openUrl(QUrl(urls.first()));
            }
            else if(urls.size() > 1)
            {
                QMenu menu(this);
                for(const QString& url : urls)
                {
                    QAction* action = menu.addAction(url);
                    connect(action, &QAction::triggered, [url]() {
                        QDesktopServices::openUrl(QUrl(url));
                    });
                }
                menu.exec(QCursor::pos());
            }
        }
        return;
    }
    QListWidget::keyPressEvent(e);
}

void ChatTextList::mouseDoubleClickEvent(QMouseEvent* e)
{
    QListWidgetItem* item = itemAt(e->pos());
    if(item)
    {
        QStringList urls = allUrls(item->text());
        if(urls.size() == 1)
        {
            QDesktopServices::openUrl(QUrl(urls.first()));
        }
        else if(urls.size() > 1)
        {
            QMenu menu(this);
            for(const QString& url : urls)
            {
                QAction* action = menu.addAction(url);
                connect(action, &QAction::triggered, [url]() {
                    QDesktopServices::openUrl(QUrl(url));
                });
            }
            menu.exec(e->globalPos());
        }
    }
    QListWidget::mouseDoubleClickEvent(e);
}

void ChatTextList::contextMenuEvent(QContextMenuEvent* event)
{
    QListWidgetItem* item = itemAt(event->pos());
    QMenu menu(this);

    if(item)
    {
        menu.addAction(tr("&Copy"), [item]() {
            QApplication::clipboard()->setText(item->text());
        });

        menu.addAction(tr("View &Details..."), [this, item]() {
            QString datetime = item->data(Qt::UserRole + 1).toString();
            QString sender = item->data(Qt::UserRole + 2).toString();
            QString content = item->data(Qt::UserRole + 3).toString();

            MessageDetailsDlg dlg(datetime, sender, content, this);
            dlg.exec();
        });
    }

    menu.addSeparator();
    menu.addAction(tr("Copy &All"), this, &ChatTextList::copyAllHistory);
    menu.addAction(tr("C&lear"), this, &ChatTextList::clearHistory);

    menu.exec(event->globalPos());
}

MessageDetailsDlg::MessageDetailsDlg(const QString& datetime, const QString& sender, const QString& content, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Message Details"));
    setAccessibleDescription(tr("Message Details"));

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* lblDateTime = new QLabel(tr("Sent: %1").arg(datetime));
    lblDateTime->setFocusPolicy(Qt::StrongFocus);
    QLabel* lblSender = new QLabel(tr("By: %1").arg(sender));
    lblSender->setFocusPolicy(Qt::StrongFocus);
    QPlainTextEdit* txtContent = new QPlainTextEdit(content);
    txtContent->setReadOnly(true);
    txtContent->setAccessibleName(tr("Content:"));
    txtContent->setTextInteractionFlags(Qt::TextInteractionFlag::LinksAccessibleByKeyboard|Qt::TextInteractionFlag::LinksAccessibleByMouse|Qt::TextInteractionFlag::TextBrowserInteraction|Qt::TextInteractionFlag::TextSelectableByKeyboard|Qt::TextInteractionFlag::TextSelectableByMouse);

    layout->addWidget(lblDateTime);
    layout->addWidget(lblSender);
    layout->addWidget(txtContent);

    QDialogButtonBox* btnBox = new QDialogButtonBox(this);
    QPushButton* closeButton = btnBox->addButton(tr("&Close"), QDialogButtonBox::RejectRole);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(btnBox);

    resize(500, 300);
}
