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

public slots:
    void slotUpdateUser(const User& user);
    void slotNewMessage(const TextMessage& textmsg);
    void slotUserLogout(const User& user);

signals:
    void newMyselfTextMessage(const TextMessage& textmsg);
    void closedTextMessage(int userid);

protected:
    void timerEvent(QTimerEvent *event);

private:
    void init(const User& user);

    Ui::TextMessageDlg ui;
    int m_userid;
    void newMsg(const TextMessage& msg, bool store);
    bool m_textchanged;
    int  m_local_typing_id, m_remote_typing_id;
    QFile m_logFile;

private slots:
    void slotCancel();
    void slotSendMsg();
    void slotSendMsg(const QString& txt_msg);
    void slotTextChanged();
};

#endif
