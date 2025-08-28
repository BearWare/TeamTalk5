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

#include "textmessagedlg.h"
#include "appinfo.h"
#include "settings.h"
#include "utilsound.h"
#include "utiltts.h"
#include "utilui.h"
#include "chattextlist.h"

#include <QDebug>
#include <QMessageBox>

#define LOCAL_TYPING_DELAY  5000
#define REMOTE_TYPING_DELAY 10000

extern TTInstance* ttInst;
extern NonDefaultSettings* ttSettings;

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
    bool listview = ttSettings->value(SETTINGS_DISPLAY_CHAT_HISTORY_LISTVIEW, SETTINGS_DISPLAY_CHAT_HISTORY_LISTVIEW_DEFAULT).toBool();
    if (listview)
    {
        ui.horizontalLayout_2->removeWidget(ui.historyTextEdit);

        auto chat = new ChatTextList(ui.groupBox);
        m_history = chat;
        ui.horizontalLayout_2->addWidget(chat);
        delete ui.historyTextEdit;
        ui.historyTextEdit = nullptr;
    }
    else
    {
        m_history = ui.historyTextEdit;
        connect(ui.historyTextEdit, &ChatTextEdit::clearHistory, [&]() {
            emit clearUserTextMessages(m_userid);
        });
    }

    setWindowIcon(QIcon(APPICON));
    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_TEXTMSGWINDOWPOS).toByteArray());
    ui.splitter->restoreState(ttSettings->value(SETTINGS_DISPLAY_TEXTMSGWINDOWPOS_SPLITTER).toByteArray());
    ui.newmsgTextEdit->setFocus();

    connect(ui.cancelButton, &QAbstractButton::clicked, this, &TextMessageDlg::slotCancel);
    connect(ui.sendButton, &QAbstractButton::clicked, this, &TextMessageDlg::slotSendMsg);
    connect(ui.newmsgTextEdit, &QPlainTextEdit::textChanged, this, &TextMessageDlg::slotTextChanged);
    connect(ui.newmsgTextEdit, &SendTextEdit::sendTextMessage,
            this, &TextMessageDlg::slotSendMsg);
    slotUpdateUser(user);
    slotTextChanged();

    m_local_typing_id = startTimer(LOCAL_TYPING_DELAY);
}

TextMessageDlg::~TextMessageDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_TEXTMSGWINDOWPOS, saveGeometry());
    ttSettings->setValue(SETTINGS_DISPLAY_TEXTMSGWINDOWPOS_SPLITTER, ui.splitter->saveState());

    emit closedTextMessage(m_userid);
}

void TextMessageDlg::slotUpdateUser(const User& user)
{
    if(user.nUserID == m_userid)
    {
        this->setWindowTitle(tr("Messages") + " - " + getDisplayName(user));
        this->setAccessibleDescription(tr("Private chat with %1").arg(getDisplayName(user)));
    }
}

void TextMessageDlg::slotNewMessage(const MyTextMessage& textmsg)
{
    if(textmsg.nFromUserID == m_userid)
    {
        newMsg(textmsg, true);
    }
}

void TextMessageDlg::slotUserLogout(const User& user)
{
    if(m_userid == user.nUserID)
    {
        ui.newmsgTextEdit->setDisabled(true);
        ui.sendButton->setDisabled(true);
    }
}

void TextMessageDlg::timerEvent(QTimerEvent *event)
{
    if(m_local_typing_id == event->timerId())
    {
        if(m_textchanged)
        {
            ServerProperties srvprop;
            if (TT_GetServerProperties(ttInst, &srvprop))
            {
                MyTextMessage msg;
                msg.nFromUserID = TT_GetMyUserID(ttInst);
                msg.nMsgType = MSGTYPE_CUSTOM;
                msg.nToUserID = m_userid;
                QString cmd = makeCustomCommand(TT_INTCMD_TYPING_TEXT,
                                                QString::number((int)!ui.newmsgTextEdit->toPlainText().isEmpty()));
                sendTextMessage(msg, cmd);
            }
            m_textchanged = false;
        }
    }

    if(m_remote_typing_id == event->timerId())
    {
        ui.newmsgGroupBox->setTitle(tr("New message"));
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
    slotSendTextMessage(ui.newmsgTextEdit->toPlainText());
}

void TextMessageDlg::slotSendTextMessage(const QString& txt_msg)
{
    if(txt_msg.isEmpty())
        return;

    MyTextMessage msg;
    msg.nFromUserID = TT_GetMyUserID(ttInst);
    msg.nChannelID = 0;
    msg.nMsgType = MSGTYPE_USER;
    msg.nToUserID = m_userid;
    msg.moreMessage = txt_msg;

    auto sentmessages = sendTextMessage(msg, txt_msg);
    if (sentmessages.size() > 0)
    {
        ui.newmsgTextEdit->setPlainText("");

        for (auto& m : sentmessages)
        {
            emit newMyselfTextMessage(m);
        }
        newMsg(msg, true);

        playSoundEvent(SOUNDEVENT_USERMSGSENT);
        addTextToSpeechMessage(TTS_USER_TEXTMSG_PRIVATE_SEND, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_PRIVATEMSGSEND, {{"{message}", txt_msg}}));
        m_textchanged = false;
    }
}

void TextMessageDlg::slotTextChanged()
{
    ui.sendButton->setEnabled(ui.newmsgTextEdit->toPlainText().size()>0);
    m_textchanged = true;
}

void TextMessageDlg::newMsg(const MyTextMessage& msg, bool store)
{
    switch(msg.nMsgType)
    {
    case MSGTYPE_USER :
    {
        QString line = m_history->addTextMessage(msg);
        ui.newmsgGroupBox->setTitle(tr("New message"));

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
                ui.newmsgGroupBox->setTitle(tr("New message - remote user typing."));
                User remoteuser;
                if (TT_GetUser(ttInst, m_userid, &remoteuser))
                {
                    if (!this->isActiveWindow())
                        addTextToSpeechMessage(TTS_USER_TEXTMSG_PRIVATE_TYPING_GLOBAL, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_TYPING, {{"{user}", getDisplayName(remoteuser)}}));
                    else
                    {
                        playSoundEvent(SOUNDEVENT_TYPING);
                        addTextToSpeechMessage(TTS_USER_TEXTMSG_PRIVATE_TYPING, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_TYPING, {{"{user}", getDisplayName(remoteuser)}}));
                    }
                }
                if(m_remote_typing_id)
                    killTimer(m_remote_typing_id);
                m_remote_typing_id = startTimer(REMOTE_TYPING_DELAY);
            }
            else
            {
                if(m_remote_typing_id)
                    killTimer(m_remote_typing_id);
                m_remote_typing_id = 0;
                ui.newmsgGroupBox->setTitle(tr("New message"));
            }
        }
        break;
    }
    default :
        break;
    }
}

void TextMessageDlg::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_F6)
    {
        if (ui.newmsgTextEdit->hasFocus())
            m_history->setFocus();
        else if (m_history->hasFocus())
            ui.newmsgTextEdit->setFocus();
    }
    if (m_history->hasFocus())
    {
        QString key = e->text();
        if (!key.isEmpty() && key.size() == 1)
        {
            QChar keyText = key.at(0);    
            if (keyText.isPrint())
            {
                ui.newmsgTextEdit->setFocus();
                ui.newmsgTextEdit->kPress(e);
            }
        }
    }
    QDialog::keyPressEvent(e);
}
