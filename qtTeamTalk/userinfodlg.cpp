/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#include "userinfodlg.h"
#include "appinfo.h"
#include "common.h"

extern TTInstance* ttInst;

UserInfoDlg::UserInfoDlg(int userid, QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
, m_userid(userid)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    startTimer(250);
    updateUser();
}

void UserInfoDlg::timerEvent(QTimerEvent *)
{
    updateUser();
}

void UserInfoDlg::updateUser()
{
    User user;
    UserStatistics stats;
    if(!TT_GetUser(ttInst, m_userid, &user))
        return;

    if(!TT_GetUserStatistics(ttInst, m_userid, &stats))
        return;

    ui.userid->setText(QString::number(user.nUserID));
    if(ui.nickname->text() != _Q(user.szNickname))
        ui.nickname->setText(_Q(user.szNickname));
    switch(user.nStatusMode & STATUSMODE_MODE)
    {
    case STATUSMODE_AVAILABLE :
        ui.statusmode->setText(tr("Available")); break;
    case STATUSMODE_AWAY :
        ui.statusmode->setText(tr("Away")); break;
    case STATUSMODE_QUESTION :
        ui.statusmode->setText(tr("Question")); break;
    default :
        ui.statusmode->setText(tr("Unknown")); break;
    }

    if(ui.statusmsg->text() != _Q(user.szStatusMsg))
        ui.statusmsg->setText(_Q(user.szStatusMsg));
    if(ui.username->text() != _Q(user.szUsername))
        ui.username->setText(_Q(user.szUsername));
    if(ui.clientname->text() != _Q(user.szClientName))
        ui.clientname->setText(_Q(user.szClientName));
    switch(user.uUserType)
    {
    case USERTYPE_ADMIN :
        ui.usertype->setText(tr("Administrator"));break;
    case USERTYPE_DEFAULT :
        ui.usertype->setText(tr("Default")); break;
    default:
        ui.usertype->setText(tr("Unknown")); break;
    }

    if(TT_GetMyUserType(ttInst) & USERTYPE_ADMIN)
    {
        if(ui.ipaddr->text() != _Q(user.szIPAddress))
            ui.ipaddr->setText(_Q(user.szIPAddress));
    }
    else
        ui.ipaddr->setText(QString());
    if(ui.version->text() != getVersion(user))
        ui.version->setText(getVersion(user));

    ui.voicepacketloss->setText(QString("%1/%2").arg(stats.nVoicePacketsLost).arg(stats.nVoicePacketsRecv+stats.nVoicePacketsLost));
    ui.vidpacketloss->setText(QString("%1/%2").arg(stats.nVideoCaptureFramesLost).arg(stats.nVideoCaptureFramesRecv+stats.nVideoCaptureFramesLost));
    ui.mediaaudpacketloss->setText(QString("%1/%2").arg(stats.nMediaFileAudioPacketsLost).arg(stats.nMediaFileAudioPacketsRecv+stats.nMediaFileAudioPacketsLost));
    ui.mediavidpacketloss->setText(QString("%1/%2").arg(stats.nMediaFileVideoFramesLost).arg(stats.nMediaFileVideoFramesRecv+stats.nMediaFileVideoFramesLost));
}
