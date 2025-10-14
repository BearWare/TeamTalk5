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

    emit videoCountChanged(getActiveUsersCount());
}

void VideoGridWidget::slotAddUser(int userid)
{
    Q_ASSERT(!userExists(userid));
    if(!userExists(userid))
    {
        UserVideoWidget* newvideo = new UserVideoWidget(this, userid,
                                                        BORDER_WIDTH);
        connect(newvideo, &UserImageWidget::userWidgetFocused,
                this, &VideoGridWidget::slotVideoWidgetFocused);
        connect(newvideo, &UserVideoWidget::userVideoEnded,
                this, &VideoGridWidget::userVideoEnded);
        connect(this, &VideoGridWidget::preferencesModified,
                newvideo, &UserVideoWidget::slotUpdateVideoTextBox);
        newvideo->setFocusPolicy(Qt::StrongFocus);
        newvideo->setAttribute(Qt::WA_DeleteOnClose);
        addUserWidget(userid, newvideo);

        emit videoCountChanged(getActiveUsersCount());
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
    emit userVideoSelected(focused);
}
