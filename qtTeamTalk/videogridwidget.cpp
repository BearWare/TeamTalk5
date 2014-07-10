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
 * This source code is part of the TeamTalk 4 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
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
