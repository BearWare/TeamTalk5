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

public:
    void slotAddUser(int userid);
    void slotRemoveUser(int channelid, const User& user);
    void slotNewVideoFrame(int userid, int stream_id);

    void slotVideoWidgetFocused(int userid, bool focused);
};

#endif /* VIDEOGRIDWIDGET_H */
