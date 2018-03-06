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

#ifndef VIDEOGRIDWIDGET_H
#define VIDEOGRIDWIDGET_H

#include "gridwidget.h"

#include "uservideowidget.h"

class VideoGridWidget : public GridWidget
{
    Q_OBJECT

public:
    VideoGridWidget(QWidget * parent);

    int getUserStreamID(int userid) const;

    void removeUser(int userid);

signals:
    void userVideoSelected(bool selected);
    void userVideoEnded(int userid);

    void videoCountChanged(int);
    void preferencesModified();

protected:
    QString getEmptyGridText() const;

public slots:
    void slotAddUser(int userid);
    void slotRemoveUser(int channelid, const User& user);
    void slotNewVideoFrame(int userid, int stream_id);

    void slotVideoWidgetFocused(int userid, bool focused);
};

#endif /* VIDEOGRIDWIDGET_H */
