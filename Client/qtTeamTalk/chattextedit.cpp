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
#include "utilui.h"

#include <QDesktopServices>
#include <QMenu>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSyntaxHighlighter>
#include <QTextCursor>
#include <QUrl>
#include <QMessageBox>

extern TTInstance* ttInst;
extern NonDefaultSettings* ttSettings;

QString urlFound(const QString& text, int& index, int& length)
{
    QRegularExpression httpExpression("(http[s]?://\\S+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch httpMatch;
    int i = text.indexOf(httpExpression, index, &httpMatch);
    if (i >= 0)
    {
        index = i;
        length = httpMatch.capturedLength();
        return httpMatch.captured(1);
    }

    QRegularExpression tturlExpression(QString("(%1//\\S+)").arg(TTLINK_PREFIX), QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch tturlMatch;
    i = text.indexOf(tturlExpression, index, &tturlMatch);
    if (i >= 0)
    {
        index = i;
        length = tturlMatch.capturedLength();
        return tturlMatch.captured(1);
    }

    return QString();
}

class UrlSyntaxHighlighter : public QSyntaxHighlighter
{
public:
    UrlSyntaxHighlighter(QTextDocument* parent)
        : QSyntaxHighlighter(parent)
    {
    }

protected:
    void highlightBlock(const QString& text)
    {
        int index = 0, length = 0;
        while(urlFound(text, index, length).size())
        {
            QTextCharFormat myClassFormat = format(index);
            myClassFormat.setFontUnderline(true);
            myClassFormat.setForeground(Qt::blue);
            setFormat(index, length, myClassFormat);
            index += length;
        }

        //QString pattern = "(http://[^ ]+)";
        //QRegExp expression(pattern);
        //int index = text.indexOf(expression);
        //while (index >= 0) {
        //    int length = expression.matchedLength();
        //    QTextCharFormat myClassFormat = format(index);
        //    myClassFormat.setFontUnderline(true);
        //    myClassFormat.setForeground(Qt::blue);
        //    myClassFormat.setAnchor(true);
        //    myClassFormat.setAnchorHref(expression.cap(1));
        //    myClassFormat.setAnchorName(expression.cap(1));
        //    setFormat(index, length, myClassFormat);
        //    index = text.indexOf(expression, index + length);
        //}
    }
};

QString getTextMessagePrefix(const TextMessage& msg, const User& user)
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
            return QString("<%1->%2>").arg(getDisplayName(user))
                           .arg(_Q(chpath));
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

ChatTextEdit::ChatTextEdit(QWidget * parent/* = 0*/)
: QPlainTextEdit(parent)
{
    new UrlSyntaxHighlighter(document());
    viewport()->setMouseTracking(true);
    setTabChangesFocus(true);
    setUndoRedoEnabled(false);
    setReadOnly(true);
    setTextInteractionFlags(Qt::TextInteractionFlag::LinksAccessibleByKeyboard |
                            Qt::TextInteractionFlag::LinksAccessibleByMouse |
                            Qt::TextInteractionFlag::TextBrowserInteraction |
                            Qt::TextInteractionFlag::TextSelectableByKeyboard |
                            Qt::TextInteractionFlag::TextSelectableByMouse);
}
   
QString ChatTextEdit::getTimeStamp(const QDateTime& tm)
{
    QString dt = getFormattedDateTime(tm.toString("yyyy-MM-dd HH:mm:ss"), "yyyy-MM-dd HH:mm:ss");
    return dt;
}

void ChatTextEdit::updateServer(const ServerProperties& srvprop)
{
    appendPlainText("");

    QString dt = getTimeStamp(QDateTime::currentDateTime());
    
    QTextCharFormat format = textCursor().charFormat();
    QTextCharFormat original = format;
    QTextCursor cursor = textCursor();
    
    //show 'joined new channel' in bold
    QFont font = format.font();
    font.setBold(true);
    format.setFont(font);
    cursor.setCharFormat(format);
    QString line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_SRVNAME, {{"{date}", dt}, {"{server}", _Q(srvprop.szServerName)}});
    setTextCursor(cursor);
    appendPlainText(line);
    if (_Q(srvprop.szMOTD).size() > 0)
    {
        line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_MOTD, {{"{date}", dt}, {"{MOTD}", _Q(srvprop.szMOTD)}});
        format.setForeground(QBrush(Qt::darkCyan));
        cursor.setCharFormat(format);
        setTextCursor(cursor);
        appendPlainText(line);
    }

    //revert bold
    font.setBold(false);
    format.setFont(font);

    //revert to original
    cursor.setCharFormat(original);
    setTextCursor(cursor);
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

    appendPlainText("");

    QString dt = getTimeStamp(QDateTime::currentDateTime());
    
    QTextCharFormat format = textCursor().charFormat();
    QTextCharFormat original = format;
    QTextCursor cursor = textCursor();
    
    //show channel name in green
    QFont font = format.font();
    font.setBold(true);
    format.setFont(font);
    format.setForeground(QBrush(Qt::darkGreen));
    cursor.setCharFormat(format);
    setTextCursor(cursor);
    QString line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_JOINCHAN, {{"{date}", dt}, {"{channelpath}", _Q(buff)}, {"{channelname}", _Q(chan.szName)}, {"{channeltopic}", _Q(chan.szTopic)}, {"{quota}", getFormattedSize(chan.nDiskQuota)}});
    appendPlainText(line);
    //revert bold
    font.setBold(false);
    format.setFont(font);
    //show topic in blue
    line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_CHANTOPIC, {{"{date}", dt}, {"{channelpath}", _Q(buff)}, {"{channelname}", _Q(chan.szName)}, {"{channeltopic}", _Q(chan.szTopic)}, {"{quota}", getFormattedSize(chan.nDiskQuota)}});
    format.setForeground(QBrush(Qt::darkYellow));
    cursor.setCharFormat(format);
    setTextCursor(cursor);
    appendPlainText(line);

    //show disk quota in red
    line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_DISKQUOTA, {{"{date}", dt}, {"{channelpath}", _Q(buff)}, {"{channelname}", _Q(chan.szName)}, {"{channeltopic}", _Q(chan.szTopic)}, {"{quota}", getFormattedSize(chan.nDiskQuota)}});
    format.setForeground(QBrush(Qt::darkRed));
    cursor.setCharFormat(format);
    setTextCursor(cursor);
    appendPlainText(line);

    //revert to original
    cursor.setCharFormat(original);
    setTextCursor(cursor);
    limitText();
}

QString ChatTextEdit::addTextMessage(const MyTextMessage& msg)
{
    User user;
    if (!TT_GetUser(ttInst, msg.nFromUserID, &user))
        return QString();

    QString dt = getTimeStamp(msg.receiveTime);
    QString line = dt;

    line += QString("%1\r\n%2").arg(getTextMessagePrefix(msg, user)).arg(msg.moreMessage);

    switch (msg.nMsgType)
    {
    case MSGTYPE_CHANNEL :
        line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_CHANNELMSG, {{"{date}", dt}, {"{user}", getDisplayName(user)}, {"{content}", msg.moreMessage}});
        break;
    case MSGTYPE_BROADCAST :
        line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_BROADMSG, {{"{date}", dt}, {"{user}", getDisplayName(user)}, {"{content}", msg.moreMessage}});
        break;
    case MSGTYPE_USER :
        line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_PRIVMSG, {{"{date}", dt}, {"{user}", getDisplayName(user)}, {"{content}", msg.moreMessage}});
        break;
    case MSGTYPE_CUSTOM :
    case MSGTYPE_NONE : break;
    }

    if (TT_GetMyUserID(ttInst) == msg.nFromUserID)
    {
        QTextCharFormat format = textCursor().charFormat();
        QTextCharFormat original = format;
        format.setForeground(QBrush(Qt::darkGray));
        QTextCursor cursor = textCursor();
        cursor.setCharFormat(format);
        setTextCursor(cursor);
        appendPlainText(line);
        cursor.setCharFormat(original);
        setTextCursor(cursor);
    }
    else
    {
        appendPlainText(line);
    }

    limitText();

    return line;
}

void ChatTextEdit::addLogMessage(const QString& msg)
{
    QString line = UtilUI::getChatTemplate(SETTINGS_CHATTEMPLATES_LOGMSG, {{"{date}", getTimeStamp(QDateTime::currentDateTime())}, {"{content}", msg}});
    QTextCharFormat format = textCursor().charFormat();
    QTextCharFormat original = format;
    format.setForeground(QBrush(Qt::gray));
    QTextCursor cursor = textCursor();
    cursor.setCharFormat(format);
    setTextCursor(cursor);
    appendPlainText(line);
    cursor.setCharFormat(original);
    setTextCursor(cursor);
    limitText();
}

void ChatTextEdit::updateTranslation()
{
    setAccessibleName(tr("History"));
}

void ChatTextEdit::limitText()
{
    QTextCursor cursor = this->textCursor();
    
    cursor.movePosition(QTextCursor::End);

    while(cursor.position() > 0x20000)
    {
        cursor.movePosition(QTextCursor::Start);
        cursor.select(QTextCursor::LineUnderCursor);
        if(cursor.hasSelection())
            cursor.removeSelectedText();
        else
            cursor.deleteChar();
        cursor.movePosition(QTextCursor::End);
    }
}

QString ChatTextEdit::currentUrl(const QTextCursor& cursor) const
{
    QTextDocument* doc = document();
    int cursor_pos = cursor.position();
    QTextBlock block = doc->findBlock(cursor_pos);
    int block_pos = block.position();

    QString text = block.text();

    QVector<int> url_index, url_length;
    QStringList urls;

    int index = 0;
    QString url;
    do
    {
        int length = 0;
        url = urlFound(text, index, length);
        if(url.size())
        {
            url_index.push_back(index);
            url_length.push_back(length);
            urls.push_back(url);
        }
        index += length;
    }
    while(url.size());

    url.clear();

    for(int i=0;i<url_index.size();i++)
    {
        if(cursor_pos >= block_pos+url_index[i] &&
           cursor_pos < block_pos+url_index[i]+url_length[i])
        {
           url = urls[i];
           break;
        }
    }

    return url;
}

void ChatTextEdit::mouseMoveEvent(QMouseEvent *e)
{
    QPlainTextEdit::mouseMoveEvent(e);
    if (currentUrl(cursorForPosition(e->pos())).size())
        viewport()->setCursor(QCursor(Qt::PointingHandCursor));
    else
        viewport()->setCursor(QCursor(Qt::IBeamCursor));
}

void ChatTextEdit::mouseReleaseEvent(QMouseEvent *e)
{
    QPlainTextEdit::mouseReleaseEvent(e);
    
    if(e->button() == Qt::RightButton)
        return;

    QString url = currentUrl(cursorForPosition(e->pos()));
    if(url.size())
       QDesktopServices::openUrl(QUrl(url));
}

void ChatTextEdit::keyPressEvent(QKeyEvent* e)
{
    QPlainTextEdit::keyPressEvent(e);

    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    {
        QString url = currentUrl(textCursor());
        if (url.size())
           QDesktopServices::openUrl(QUrl(url));
    }
}

void ChatTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    menu->addSeparator();
    auto clearMenu = menu->addAction(tr("&Clear"), this, &QPlainTextEdit::clear);
    QAction* chosen = menu->exec(event->globalPos());
    if (clearMenu == chosen)
    {
        emit clearHistory();
    }
    delete menu;
}
