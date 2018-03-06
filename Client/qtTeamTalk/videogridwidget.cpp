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

#include "videogridwidget.h"
#include <math.h>
#include <QDebug>

VideoGridWidget::VideoGridWidget(QWidget * parent/* = 0*/)
: GridWidget(parent)
{
}

QString VideoGridWidget::getEmptyGridText() const
{
    return tr("No active video sessions");
}

int VideoGridWidget::getUserStreamID(int userid) const
{
    UserVideoWidget* widget = dynamic_cast<UserVideoWidget*>(getUserWidget(userid));
    if(widget)
        return widget->getStreamID();
    return 0;
}

void VideoGridWidget::removeUser(int userid)
{
    removeUserWidget(userid);

    emit(videoCountChanged(getActiveUsersCount()));
}

void VideoGridWidget::slotAddUser(int userid)
{
    Q_ASSERT(!userExists(userid));
    if(!userExists(userid))
    {
        UserVideoWidget* newvideo = new UserVideoWidget(this, userid,
                                                        BORDER_WIDTH);
        connect(newvideo, SIGNAL(userWidgetFocused(int, bool)), 
                this, SLOT(slotVideoWidgetFocused(int, bool)));
        connect(newvideo, SIGNAL(userVideoEnded(int)),
                SIGNAL(userVideoEnded(int)));
        connect(this, SIGNAL(preferencesModified()),
                newvideo, SLOT(slotUpdateVideoTextBox()));
        newvideo->setFocusPolicy(Qt::StrongFocus);
        newvideo->setAttribute(Qt::WA_DeleteOnClose);
        addUserWidget(userid, newvideo);

        emit(videoCountChanged(getActiveUsersCount()));
    }
}

void VideoGridWidget::slotRemoveUser(int channelid, const User& user)
{
    Q_UNUSED(channelid);
    removeUser(user.nUserID);
}

void VideoGridWidget::slotNewVideoFrame(int userid, int stream_id)
{
    UserVideoWidget* widget = dynamic_cast<UserVideoWidget*>(getUserWidget(userid));
    if(widget)
        widget->slotNewVideoFrame(userid, stream_id);
}

void VideoGridWidget::slotVideoWidgetFocused(int /*userid*/, bool focused)
{
    emit(userVideoSelected(focused));
}
