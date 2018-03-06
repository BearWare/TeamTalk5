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

#ifndef USERVIDEODLG_H
#define USERVIDEODLG_H

#include "common.h"
#include "ui_uservideo.h"
#include "uservideowidget.h"

class UserVideoDlg : public QDialog
{
    Q_OBJECT

public:
    UserVideoDlg(int userid_masked, const User& user, const QSize& size, 
                 QWidget * parent = 0);
    UserVideoDlg(int userid_masked, const User& user, QWidget * parent = 0);
    ~UserVideoDlg();

    UserVideoWidget *uservideoWidget;

public slots:
    void slotUserUpdate(const User& user);
    void slotNewVideoFrame(int userid, int streamid);

protected:
    void initCommon(int userid);

private:
    Ui::UserVideoDlg ui;
    bool m_autosize;

signals:
    void userVideoDlgClosing(int userid);
};

#endif
