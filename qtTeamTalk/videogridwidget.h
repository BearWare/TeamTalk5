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
