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

#include "textmessagedlg.h"
#include "appinfo.h"
#include "settings.h"
#include <QDebug>

#define LOCAL_TYPING_DELAY  5000
#define REMOTE_TYPING_DELAY 10000

extern TTInstance* ttInst;
extern QSettings* ttSettings;

TextMessageDlg::TextMessageDlg(const User& user, QWidget * parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
, m_userid(user.nUserID)
, m_textchanged(false)
, m_remote_typing_id(0)
{
    init(user);
}

TextMessageDlg::TextMessageDlg(const User& user, const textmessages_t& msgs,
                               QWidget * parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
, m_userid(user.nUserID)
, m_textchanged(false)
, m_remote_typing_id(0)
{
    init(user);
    
    textmessages_t::const_iterator ii = msgs.begin();
    for(;ii!=msgs.end();ii++)
        newMsg(*ii, false);
}

void TextMessageDlg::init(const User& user)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    connect(ui.cancelButton, SIGNAL(clicked()), SLOT(slotCancel()));
    connect(ui.sendButton, SIGNAL(clicked()), SLOT(slotSendMsg()));
    connect(ui.newmsgTextEdit, SIGNAL(textChanged()), SLOT(slotTextChanged()));
    connect(ui.newmsgTextEdit, SIGNAL(sendTextMessage(const QString&)),
            SLOT(slotSendMsg(const QString&)));
    slotUpdateUser(user);
    slotTextChanged();

    m_local_typing_id = startTimer(LOCAL_TYPING_DELAY);
}

TextMessageDlg::~TextMessageDlg()
{
    emit(closedTextMessage(m_userid));
}

void TextMessageDlg::slotUpdateUser(const User& user)
{
    if(user.nUserID == m_userid)
        this->setWindowTitle(tr("Messages") + " - " + getDisplayName(user));
}

void TextMessageDlg::slotNewMessage(const TextMessage& textmsg)
{
    if(textmsg.nFromUserID == m_userid)
    {
        newMsg(textmsg, true);
    }
}

void TextMessageDlg::slotUserLogout(const User& user)
{
    if(m_userid == user.nUserID)
        ui.newmsgTextEdit->setDisabled(true);
}

void TextMessageDlg::timerEvent(QTimerEvent *event)
{
    if(m_local_typing_id == event->timerId())
    {
        if(m_textchanged)
        {
            ServerProperties srvprop;
            if(TT_GetServerProperties(ttInst, &srvprop))
            {
                TextMessage msg;
                ZERO_STRUCT(msg);
                msg.nFromUserID = TT_GetMyUserID(ttInst);
                msg.nMsgType = MSGTYPE_CUSTOM;
                msg.nToUserID = m_userid;
                QString cmd = makeCustomCommand(TT_INTCMD_TYPING_TEXT,
                                                QString::number((int)!ui.newmsgTextEdit->toPlainText().isEmpty()));
                COPY_TTSTR(msg.szMessage, cmd);
                if(TT_DoTextMessage(ttInst, &msg)>0)
                    emit(newMyselfTextMessage(msg));
            }
            m_textchanged = false;
        }
    }

    if(m_remote_typing_id == event->timerId())
    {
        ui.newmsgLabel->setText(tr("New message"));
        killTimer(m_remote_typing_id);
        m_remote_typing_id = 0;
    }
}

void TextMessageDlg::slotCancel()
{
    this->close();
}

void TextMessageDlg::slotSendMsg()
{
    slotSendMsg(ui.newmsgTextEdit->toPlainText());
}

void TextMessageDlg::slotSendMsg(const QString& txt_msg)
{
    if(txt_msg.isEmpty())
        return;

    TextMessage msg;
    ZERO_STRUCT(msg);
    msg.nFromUserID = TT_GetMyUserID(ttInst);
    msg.nChannelID = 0;
    msg.nMsgType = MSGTYPE_USER;
    msg.nToUserID = m_userid;
    COPY_TTSTR(msg.szMessage, txt_msg);
    if(TT_DoTextMessage(ttInst, &msg)>0)
    {
        ui.newmsgTextEdit->setPlainText("");
        newMsg(msg, true);
        emit(newMyselfTextMessage(msg));
        m_textchanged = false;
    }
}

void TextMessageDlg::slotTextChanged()
{
    ui.sendButton->setEnabled(ui.newmsgTextEdit->toPlainText().size()>0);
    m_textchanged = true;
}

void TextMessageDlg::newMsg(const TextMessage& msg, bool store)
{
    switch(msg.nMsgType)
    {
    case MSGTYPE_USER :
    {
        QString line = ui.historyTextEdit->addTextMessage(msg);
        ui.newmsgLabel->setText(tr("New message"));

        QString folder = ttSettings->value(SETTINGS_MEDIASTORAGE_USERLOGFOLDER).toString();
        if(store && folder.size())
        {
            User user;
            if(TT_GetUser(ttInst, m_userid, &user))
            {
                if(!m_logFile.isOpen())
                    openLogFile(m_logFile, folder, getDisplayName(user) + ".ulog");
            }
            writeLogEntry(m_logFile, line);
        }
    }
    break;
    case MSGTYPE_CUSTOM :
    {
        QStringList cmd_msg = getCustomCommand(msg);
        if(cmd_msg.size() >= 2 && cmd_msg[0] == TT_INTCMD_TYPING_TEXT)
        {
            if(cmd_msg[1] == "1")
            {
                ui.newmsgLabel->setText(tr("New message - remote user typing."));
                if(m_remote_typing_id)
                    killTimer(m_remote_typing_id);
                m_remote_typing_id = startTimer(REMOTE_TYPING_DELAY);
            }
            else
            {
                if(m_remote_typing_id)
                    killTimer(m_remote_typing_id);
                m_remote_typing_id = 0;
                ui.newmsgLabel->setText(tr("New message"));
            }
        }
        break;
    }
    default :
        break;
    }
}
