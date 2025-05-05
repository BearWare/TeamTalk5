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

#define CHATTEXTITEM_TIMESTAMP_ROLE  (Qt::UserRole + 1)
#define CHATTEXTITEM_SENDER_ROLE     (Qt::UserRole + 2)
#define CHATTEXTITEM_CONTENT_ROLE    (Qt::UserRole + 3)

ChatTextList::ChatTextList(QWidget *parent)
: QListWidget(parent)
{
    setMouseTracking(true);
    setWordWrap(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setContextMenuPolicy(Qt::DefaultContextMenu);
    m_copy = new QShortcut(QKeySequence::Copy, this);
    m_copy->setContext(Qt::WidgetShortcut);
    connect(m_copy, &QShortcut::activated, this, [this]
    {
        menuAction(COPY);
    });
    m_details = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), this);
    m_details->setContext(Qt::WidgetShortcut);
    connect(m_details, &QShortcut::activated, this, [this]
    {
        menuAction(VIEWDETAILS);
    });
    m_copyAll = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C), this);
    m_copyAll->setContext(Qt::WidgetShortcut);
    connect(m_copyAll, &QShortcut::activated, this, [this]
    {
        menuAction(COPYALL);
    });
    m_clear = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Delete), this);
    m_clear->setContext(Qt::WidgetShortcut);
    connect(m_clear, &QShortcut::activated, this, [this]
    {
        menuAction(CLEAR);
    });
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

    QListWidgetItem* item = new QListWidgetItem(UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_SRVNAME, {{"{date}", dt}, {"{server}", _Q(srvprop.szServerName)}}));
    item->setFont(QFont("Arial", -1, QFont::Bold));

    item->setData(CHATTEXTITEM_TIMESTAMP_ROLE, dt);
    item->setData(CHATTEXTITEM_SENDER_ROLE, tr("Server"));
    item->setData(CHATTEXTITEM_CONTENT_ROLE, tr("Server Name: %1").arg(_Q(srvprop.szServerName)));

    addItem("");
    addItem(item);

    if (_Q(srvprop.szMOTD).size() > 0)
    {
        QListWidgetItem* motdItem = new QListWidgetItem(UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_MOTD, {{"{date}", dt}, {"{MOTD}", _Q(srvprop.szMOTD)}}));
        motdItem->setForeground(Qt::darkCyan);
        motdItem->setData(CHATTEXTITEM_TIMESTAMP_ROLE, dt);
        motdItem->setData(CHATTEXTITEM_SENDER_ROLE, tr("Server"));
        motdItem->setData(CHATTEXTITEM_CONTENT_ROLE, tr("Message of the Day: %1").arg(_Q(srvprop.szMOTD)));
        addItem(motdItem);
    }

    limitText();
}

void ChatTextList::joinedChannel(int channelid)
{
    TTCHAR buff[TT_STRLEN]; Channel chan;
    if(!TT_GetChannel(ttInst, channelid, &chan) || !TT_GetChannelPath(ttInst, channelid, buff)) return;

    QString dt = getTimeStamp(QDateTime::currentDateTime());

    QListWidgetItem* item = new QListWidgetItem(UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_JOINCHAN, {{"{date}", dt}, {"{channelpath}", _Q(buff)}, {"{channelname}", _Q(chan.szName)}, {"{channeltopic}", _Q(chan.szTopic)}, {"{quota}", getFormattedSize(chan.nDiskQuota)}}));
    item->setFont(QFont("Arial", -1, QFont::Bold));
    item->setForeground(Qt::darkGreen);

    item->setData(CHATTEXTITEM_TIMESTAMP_ROLE, dt);
    item->setData(CHATTEXTITEM_SENDER_ROLE, tr("Channel"));
    item->setData(CHATTEXTITEM_CONTENT_ROLE, tr("Joined channel %1").arg(_Q(buff)));

    addItem("");
    addItem(item);

    QListWidgetItem* topicItem = new QListWidgetItem(UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_CHANTOPIC, {{"{date}", dt}, {"{channelpath}", _Q(buff)}, {"{channelname}", _Q(chan.szName)}, {"{channeltopic}", _Q(chan.szTopic)}, {"{quota}", getFormattedSize(chan.nDiskQuota)}}));
    topicItem->setForeground(Qt::darkYellow);
    topicItem->setData(CHATTEXTITEM_TIMESTAMP_ROLE, dt);
    topicItem->setData(CHATTEXTITEM_SENDER_ROLE, tr("Channel"));
    topicItem->setData(CHATTEXTITEM_CONTENT_ROLE, tr("Topic: %1").arg(_Q(chan.szTopic)));
    addItem(topicItem);

    QListWidgetItem* quotaItem = new QListWidgetItem(UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_DISKQUOTA, {{"{date}", dt}, {"{channelpath}", _Q(buff)}, {"{channelname}", _Q(chan.szName)}, {"{channeltopic}", _Q(chan.szTopic)}, {"{quota}", getFormattedSize(chan.nDiskQuota)}}));
    quotaItem->setForeground(Qt::darkRed);
    quotaItem->setData(CHATTEXTITEM_TIMESTAMP_ROLE, dt);
    quotaItem->setData(CHATTEXTITEM_SENDER_ROLE, tr("Channel"));
    quotaItem->setData(CHATTEXTITEM_CONTENT_ROLE, tr("Disk quota: %1").arg(getFormattedSize(chan.nDiskQuota)));
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
        break;
    case MSGTYPE_BROADCAST :
        line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_BROADMSG, {{"{date}", dt}, {"{user}", getDisplayName(user)}, {"{content}", content}});
        break;
    case MSGTYPE_USER :
        line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_PRIVMSG, {{"{date}", dt}, {"{user}", getDisplayName(user)}, {"{content}", content}});
        break;
    case MSGTYPE_CUSTOM :
    case MSGTYPE_NONE : break;
    }

    QListWidgetItem* item = new QListWidgetItem(line);

    item->setData(CHATTEXTITEM_TIMESTAMP_ROLE, dt);
    item->setData(CHATTEXTITEM_SENDER_ROLE, (TT_GetMyUserID(ttInst) == msg.nFromUserID)?tr("You"):getDisplayName(user));
    item->setData(CHATTEXTITEM_CONTENT_ROLE, content);

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

    QString line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_LOGMSG, {{"{date}", dt}, {"{content}", msg}});
    QListWidgetItem* item = new QListWidgetItem(line);
    item->setForeground(Qt::gray);

    item->setData(CHATTEXTITEM_TIMESTAMP_ROLE, dt);
    item->setData(CHATTEXTITEM_SENDER_ROLE, sender);
    item->setData(CHATTEXTITEM_CONTENT_ROLE, content);

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
    QString text = item->data(CHATTEXTITEM_CONTENT_ROLE).toString();
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
            QStringList urls = allUrls(item->data(CHATTEXTITEM_CONTENT_ROLE).toString());
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
        QStringList urls = allUrls(item->data(CHATTEXTITEM_CONTENT_ROLE).toString());
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

void ChatTextList::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    QListWidgetItem *item = itemAt(event->pos());

    auto add = [&](const QString &txt, const QKeySequence &seq, MenuAction ma)
    {
        QAction *act = menu.addAction(txt, [this, ma]{ menuAction(ma); });
        act->setShortcut(seq);
    };

    if (item)
    {
        add(tr("&Copy"), QKeySequence::Copy, COPY);
        add(tr("View &Details..."), QKeySequence(Qt::CTRL | Qt::Key_Return), VIEWDETAILS);
    }

    menu.addSeparator();
    add(tr("Copy &All"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C), COPYALL);
    add(tr("C&lear"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Delete), CLEAR);

    menu.exec(event->globalPos());
}

void ChatTextList::menuAction(MenuAction ma)
{
    QListWidgetItem *item = currentItem();

    switch (ma)
    {

    case COPY :
        if (item)
            QApplication::clipboard()->setText(item->text());
        break;

    case VIEWDETAILS :
    {
        if (!item)
            break;

        QString datetime = item->data(CHATTEXTITEM_TIMESTAMP_ROLE).toString();
        QString sender   = item->data(CHATTEXTITEM_SENDER_ROLE).toString();
        QString content  = item->data(CHATTEXTITEM_CONTENT_ROLE).toString();

        MessageDetailsDlg dlg(datetime, sender, content, this);
        dlg.exec();
        break;
    }

    case COPYALL :
        copyAllHistory();
        break;

    case CLEAR :
        clearHistory();
        break;

    default :
        break;
    }
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
    txtContent->setFocus();

    QDialogButtonBox* btnBox = new QDialogButtonBox(this);
    QPushButton* closeButton = btnBox->addButton(tr("&Close"), QDialogButtonBox::RejectRole);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(btnBox);

    resize(500, 300);
}
