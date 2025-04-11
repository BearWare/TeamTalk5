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

#ifndef TEXTMESSAGEDLG_H
#define TEXTMESSAGEDLG_H

#include "common.h"

#include <QDialog>

#include "ui_textmessage.h"

class TextMessageDlg : public QDialog
{
    Q_OBJECT

public:
    TextMessageDlg(const User& user, QWidget * parent = 0);
    TextMessageDlg(const User& user, const textmessages_t& msgs,
                   QWidget * parent = 0);
    ~TextMessageDlg();

public:
    void slotUpdateUser(const User& user);
    void slotNewMessage(const MyTextMessage& textmsg);
    void slotUserLogout(const User& user);

signals:
    void newMyselfTextMessage(const MyTextMessage& textmsg);
    void closedTextMessage(int userid);
    void clearUserTextMessages(int userid);

protected:
    void timerEvent(QTimerEvent *event) override;
    void keyPressEvent(QKeyEvent* e) override;

private:
    void init(const User& user);

    Ui::TextMessageDlg ui;
    ChatTextHistory* m_history;
    int m_userid;
    void newMsg(const MyTextMessage& msg, bool store);
    bool m_textchanged;
    int  m_local_typing_id, m_remote_typing_id;
    QFile m_logFile;

private:
    void slotCancel();
    void slotSendMsg();
    void slotSendTextMessage(const QString& txt_msg);
    void slotTextChanged();
};

#endif
