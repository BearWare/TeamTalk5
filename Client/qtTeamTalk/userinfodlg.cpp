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
#include <QClipboard>

extern TTInstance* ttInst;

UserInfoDlg::UserInfoDlg(int userid, QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
, m_userid(userid)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    connect(ui.copyButton, &QAbstractButton::clicked,
            this, &UserInfoDlg::copyInfoToClipboard);
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

    QString idLabelText = QString(tr("User ID") + ": %1").arg(QString::number(user.nUserID));
    if(ui.idLabel->text() != idLabelText)
        ui.idLabel->setText(idLabelText);

    QString nickLabelText = QString(tr("Nickname") + ": %1").arg(_Q(user.szNickname));
    if(ui.nicknameLabel->text() != nickLabelText)
        ui.nicknameLabel->setText(nickLabelText);

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
    QString statusLabelText = QString(tr("Status mode") + ": %1").arg(status);    
    if(ui.statusLabel->text() != statusLabelText)
        ui.statusLabel->setText(statusLabelText);

    if (_Q(user.szStatusMsg).size() > 0)
    {
        QString statusmsgLabelText = QString(tr("Status message") + ": %1").arg(_Q(user.szStatusMsg));
        if(ui.statusmsgLabel->text() != statusmsgLabelText)
            ui.statusmsgLabel->setText(statusmsgLabelText);
        ui.statusmsgLabel->show();
    }
    else
        ui.statusmsgLabel->hide();

    QString usernameLabelText = QString(tr("Username") + ": %1").arg(_Q(user.szUsername));
    if(ui.usernameLabel->text() != usernameLabelText)
        ui.usernameLabel->setText(usernameLabelText);

    QString clientLabelText = QString(tr("Client") + ": %1 %2").arg(_Q(user.szClientName)).arg(getVersion(user));
    if(ui.clientLabel->text() != clientLabelText)
        ui.clientLabel->setText(clientLabelText);

    switch(user.uUserType)
    {
    case USERTYPE_ADMIN :
        status = ((user.nStatusMode & STATUSMODE_FEMALE)?tr("Administrator", "For female"):tr("Administrator", "For male and neutral"));break;
    case USERTYPE_DEFAULT :
        status = tr("Default"); break;
    default:
        status = tr("Unknown"); break;
    }
    QString usertypeLabelText = QString(tr("User type") + ": %1").arg(status);
    if(ui.usertypeLabel->text() != usertypeLabelText)
        ui.usertypeLabel->setText(usertypeLabelText);

    if (_Q(user.szIPAddress).size() > 0)
    {
        QString ipLabelText = QString(tr("IP-address") + ": %1").arg(_Q(user.szIPAddress));
        if(ui.ipLabel->text() != ipLabelText)
            ui.ipLabel->setText(ipLabelText);
        ui.ipLabel->show();
    }
    else
        ui.ipLabel->hide();

    UserStatistics stats;
    if(!TT_GetUserStatistics(ttInst, m_userid, &stats))
        return;

    ui.vplLabel->setText(QString(tr("Voice packet loss") + ": %1/%2").arg(stats.nVoicePacketsLost).arg(stats.nVoicePacketsRecv+stats.nVoicePacketsLost));
    ui.vflLabel->setText(QString(tr("Video frame loss") + ": %1/%2").arg(stats.nVideoCaptureFramesLost).arg(stats.nVideoCaptureFramesRecv+stats.nVideoCaptureFramesLost));
    ui.afplLabel->setText(QString(tr("Audio file packets loss") + ": %1/%2").arg(stats.nMediaFileAudioPacketsLost).arg(stats.nMediaFileAudioPacketsRecv+stats.nMediaFileAudioPacketsLost));
    ui.vfflLabel->setText(QString(tr("Video file frame loss") + ": %1/%2").arg(stats.nMediaFileVideoFramesLost).arg(stats.nMediaFileVideoFramesRecv+stats.nMediaFileVideoFramesLost));
}

void UserInfoDlg::copyInfoToClipboard()
{
    QString cp = ui.idLabel->text() + "\n" +
                 ui.nicknameLabel->text() + "\n" +
                 ui.usernameLabel->text() + "\n" +
                 ui.clientLabel->text() + "\n" +
                 ui.statusLabel->text() + "\n";
    if (ui.statusmsgLabel->isVisible())
        cp += ui.statusmsgLabel->text() + "\n";
    cp += ui.usertypeLabel->text() + "\n";
    if (ui.ipLabel->isVisible())
        cp += ui.ipLabel->text() + "\n";
    cp += ui.vplLabel->text() + "\n" +
          ui.vflLabel->text() + "\n" +
          ui.afplLabel->text() + "\n" +
          ui.vfflLabel->text();

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(cp);
}
