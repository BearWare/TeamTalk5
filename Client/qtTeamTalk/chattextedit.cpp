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

#include "chattextedit.h"
#include "settings.h"
#include "appinfo.h"

#include <QDateTime>
#include <QTextCursor>
#include <QSyntaxHighlighter>
#include <QDesktopServices>
#include <QUrl>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

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
    case MSGTYPE_CUSTOM : break;
    }
    return QString();
}

quint32 generateKey(const TextMessage& msg)
{
    return (msg.nMsgType << 16) | msg.nFromUserID;
}

ChatTextEdit::ChatTextEdit(QWidget * parent/* = 0*/)
: QPlainTextEdit(parent)
{
    new UrlSyntaxHighlighter(document());
    viewport()->setMouseTracking(true);
}
   
QString ChatTextEdit::getTimeStamp(const QDateTime& tm, bool force_ts)
{
    QString dt;
    if(ttSettings->value(SETTINGS_DISPLAY_MSGTIMESTAMP, false).toBool() || force_ts)
        dt = tm.toString(tr("yyyy-MM-dd HH:mm:ss")) + QString(" ");
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
    QString line = dt + tr("Server Name: %1").arg(_Q(srvprop.szServerName));;
    setTextCursor(cursor);
    appendPlainText(line);
    if (_Q(srvprop.szMOTD).size() > 0)
    {
        line = dt + tr("Message of the Day: %1").arg(_Q(srvprop.szMOTD)) + "\r\n";
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
    
    //show 'joined new channel' in bold
    QFont font = format.font();
    font.setBold(true);
    format.setFont(font);
    cursor.setCharFormat(format);
    QString line = dt + tr("Joined new channel");
    setTextCursor(cursor);
    appendPlainText(line);
    //revert bold
    font.setBold(false);
    format.setFont(font);
    
    //show channel name in green
    line = tr("Channel: %1").arg(_Q(buff));
    format.setForeground(QBrush(Qt::darkGreen));
    cursor.setCharFormat(format);
    setTextCursor(cursor);
    appendPlainText(line);

    //show topic in blue
    line = tr("Topic: %1").arg(_Q(chan.szTopic));
    format.setForeground(QBrush(Qt::darkYellow));
    cursor.setCharFormat(format);
    setTextCursor(cursor);
    appendPlainText(line);

    //show disk quota in red
    line = tr("Disk quota: %1 KBytes").arg(chan.nDiskQuota/1024);
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
    QString content;

    if (!mergeMessages(msg, content))
        return QString();

    line += QString("%1\r\n%2").arg(getTextMessagePrefix(msg, user)).arg(content);

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
    QString line = QString("%1 * %2").arg(getTimeStamp(QDateTime::currentDateTime())).arg(msg);
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

bool ChatTextEdit::mergeMessages(const MyTextMessage& msg, QString& content)
{
    m_mergemessages[generateKey(msg)].push_back(msg);

    // prevent out-of-memory.
    if (m_mergemessages[generateKey(msg)].size() > 1000)
        m_mergemessages.remove(generateKey(msg));

    if (!msg.bMore)
    {
        for (auto& m : m_mergemessages[generateKey(msg)])
        {
            content += _Q(m.szMessage);
        }
        m_mergemessages.remove(generateKey(msg));
    }

    return !msg.bMore;
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
