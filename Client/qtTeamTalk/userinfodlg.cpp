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

#include "userinfodlg.h"
#include "appinfo.h"
#include "common.h"

#include <QUrl>
#include <QDesktopServices>

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
    if(!TT_GetUser(ttInst, m_userid, &user))
        return;

    this->setAccessibleDescription(tr("Information of %1").arg(_Q(user.szNickname)));

    if(ui.userid->text() != QString::number(user.nUserID))
        ui.userid->setText(QString::number(user.nUserID));

    if(ui.nickname->text() != _Q(user.szNickname))
        ui.nickname->setText(_Q(user.szNickname));

    QString status;
    switch(user.nStatusMode & STATUSMODE_MODE)
    {
    case STATUSMODE_AVAILABLE :
        status = tr("Available"); break;
    case STATUSMODE_AWAY :
        status = tr("Away"); break;
    case STATUSMODE_QUESTION :
        status = tr("Question"); break;
    default :
        status = tr("Unknown"); break;
    }
    
    if(ui.statusmode->text() != status)
        ui.statusmode->setText(status);

    if(ui.statusmsg->text() != _Q(user.szStatusMsg))
        ui.statusmsg->setText(_Q(user.szStatusMsg));
    if(ui.username->text() != _Q(user.szUsername))
        ui.username->setText(_Q(user.szUsername));

    if(ui.clientname->text() != _Q(user.szClientName))
        ui.clientname->setText(_Q(user.szClientName));

    switch(user.uUserType)
    {
    case USERTYPE_ADMIN :
        status = tr("Administrator");break;
    case USERTYPE_DEFAULT :
        status = tr("Default"); break;
    default:
        status = tr("Unknown"); break;
    }

    if(ui.usertype->text() != status)
        ui.usertype->setText(status);

    if(ui.ipaddr->text() != _Q(user.szIPAddress))
        ui.ipaddr->setText(_Q(user.szIPAddress));

    if(ui.version->text() != getVersion(user))
        ui.version->setText(getVersion(user));

    UserStatistics stats;
    if(!TT_GetUserStatistics(ttInst, m_userid, &stats))
        return;

    ui.voicepacketloss->setText(QString("%1/%2").arg(stats.nVoicePacketsLost).arg(stats.nVoicePacketsRecv+stats.nVoicePacketsLost));
    ui.vidpacketloss->setText(QString("%1/%2").arg(stats.nVideoCaptureFramesLost).arg(stats.nVideoCaptureFramesRecv+stats.nVideoCaptureFramesLost));
    ui.mediaaudpacketloss->setText(QString("%1/%2").arg(stats.nMediaFileAudioPacketsLost).arg(stats.nMediaFileAudioPacketsRecv+stats.nMediaFileAudioPacketsLost));
    ui.mediavidpacketloss->setText(QString("%1/%2").arg(stats.nMediaFileVideoFramesLost).arg(stats.nMediaFileVideoFramesRecv+stats.nMediaFileVideoFramesLost));
}
