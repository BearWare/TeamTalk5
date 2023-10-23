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
        status = ((user.nStatusMode & STATUSMODE_FEMALE)?tr("Available", "For female"):tr("Available", "For male and neutral")); break;
    case STATUSMODE_AWAY :
        status = ((user.nStatusMode & STATUSMODE_FEMALE)?tr("Away", "For female"):tr("Away", "For male and neutral")); break;
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

    if(ui.clientname->text() != _Q(user.szClientName)+" "+getVersion(user))
        ui.clientname->setText(_Q(user.szClientName)+" "+getVersion(user));

    switch(user.uUserType)
    {
    case USERTYPE_ADMIN :
        status = ((user.nStatusMode & STATUSMODE_FEMALE)?tr("Administrator", "For female"):tr("Administrator", "For male and neutral"));break;
    case USERTYPE_DEFAULT :
        status = tr("Default"); break;
    default:
        status = tr("Unknown"); break;
    }

    if(ui.usertype->text() != status)
        ui.usertype->setText(status);

    if(ui.ipaddr->text() != _Q(user.szIPAddress))
        ui.ipaddr->setText(_Q(user.szIPAddress));

    UserStatistics stats;
    if(!TT_GetUserStatistics(ttInst, m_userid, &stats))
        return;

    ui.voicepacketloss->setText(QString("%1/%2").arg(stats.nVoicePacketsLost).arg(stats.nVoicePacketsRecv+stats.nVoicePacketsLost));
    ui.vidpacketloss->setText(QString("%1/%2").arg(stats.nVideoCaptureFramesLost).arg(stats.nVideoCaptureFramesRecv+stats.nVideoCaptureFramesLost));
    ui.mediaaudpacketloss->setText(QString("%1/%2").arg(stats.nMediaFileAudioPacketsLost).arg(stats.nMediaFileAudioPacketsRecv+stats.nMediaFileAudioPacketsLost));
    ui.mediavidpacketloss->setText(QString("%1/%2").arg(stats.nMediaFileVideoFramesLost).arg(stats.nMediaFileVideoFramesRecv+stats.nMediaFileVideoFramesLost));
}
