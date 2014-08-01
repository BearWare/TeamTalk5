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

#include "chattextedit.h"
#include "settings.h"
#include <QDateTime>
#include <QTextCursor>
#include <QSyntaxHighlighter>
#include <QDesktopServices>
#include <QUrl>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

QString urlFound(const QString& text, int& index, int& length)
{
    static QRegExp expression("(http[s]?://\\S+)");
    expression.setCaseSensitivity(Qt::CaseInsensitive);
    int i = text.indexOf(expression, index);
    if(i>=0)
    {
        index = i;
        length = expression.matchedLength();
        return expression.cap(1);
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

ChatTextEdit::ChatTextEdit(QWidget * parent/* = 0*/)
: QPlainTextEdit(parent)
{
    new UrlSyntaxHighlighter(document());
    viewport()->setMouseTracking(true);
}
   
QString ChatTextEdit::getTimeStamp(bool force_ts)
{
    QString dt;
    if(ttSettings->value(SETTINGS_DISPLAY_MSGTIMESTAMP, false).toBool() || force_ts)
        dt = QDateTime::currentDateTime().toString(Qt::ISODate) + QString(" ");
    return dt;
}

void ChatTextEdit::updateServer(const ServerProperties& srvprop)
{
    appendPlainText("");

    QString dt = getTimeStamp();
    
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

    line = dt + tr("Message of the Day: %1").arg(_Q(srvprop.szMOTD)) + "\r\n";
    format.setForeground(QBrush(Qt::darkCyan));
    cursor.setCharFormat(format);
    setTextCursor(cursor);
    appendPlainText(line);

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

    QString dt = getTimeStamp();
    
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
    format.setForeground(QBrush(Qt::darkBlue));
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

void ChatTextEdit::addTextMessage(const TextMessage& msg)
{
    User user;
    if(!TT_GetUser(ttInst, msg.nFromUserID, &user))
        return;

    QString dt = getTimeStamp();
    QString line = dt;

    switch(msg.nMsgType)
    {
    case MSGTYPE_USER :
        line += QString("<%1> %2").arg(_Q(user.szNickname)).arg(_Q(msg.szMessage));
        break;
    case MSGTYPE_CHANNEL :
        if(msg.nChannelID != TT_GetMyChannelID(ttInst))
        {
            TTCHAR chpath[TT_STRLEN] = {0};
            TT_GetChannelPath(ttInst, msg.nChannelID, chpath);
            line += QString("<%1->%2> %3").arg(_Q(user.szNickname))
                           .arg(_Q(chpath)).arg(_Q(msg.szMessage));
        }
        else
            line += QString("<%1> %2").arg(_Q(user.szNickname))
                           .arg(_Q(msg.szMessage));
        break;
    case MSGTYPE_BROADCAST :
        line += QString("<%1->BROADCAST> %2").arg(_Q(user.szNickname))
                       .arg(_Q(msg.szMessage));
        break;
    }

    if(TT_GetMyUserID(ttInst) == msg.nFromUserID)
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
        appendPlainText(line);
    limitText();
}

void ChatTextEdit::addLogMessage(const QString& msg)
{
    QString line = QString("%1 * %2").arg(getTimeStamp()).arg(msg);
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

void ChatTextEdit::mouseMoveEvent(QMouseEvent *e)
{
    QPlainTextEdit::mouseMoveEvent(e);
    if(currentUrl(e).size())
        viewport()->setCursor(QCursor(Qt::PointingHandCursor));
    else
        viewport()->setCursor(QCursor(Qt::IBeamCursor));
}

void ChatTextEdit::mouseReleaseEvent(QMouseEvent *e)
{
    QPlainTextEdit::mouseReleaseEvent(e);
    
    if(e->button() == Qt::RightButton)
        return;

    QString url = currentUrl(e);
    if(url.size())
       QDesktopServices::openUrl(QUrl(url));
}

QString ChatTextEdit::currentUrl(QMouseEvent* e) const
{
    QTextDocument* doc = document();

    int cursor_pos = cursorForPosition(e->pos()).position();
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
