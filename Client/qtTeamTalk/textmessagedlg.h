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

protected:
    void timerEvent(QTimerEvent *event) override;
    void keyPressEvent(QKeyEvent* e) override;

private:
    void init(const User& user);

    Ui::TextMessageDlg ui;
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
