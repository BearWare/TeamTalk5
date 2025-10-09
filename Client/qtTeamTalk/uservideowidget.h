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

#ifndef USERVIDEOWIDGET_H
#define USERVIDEOWIDGET_H

#include "userimagewidget.h"
#include "common.h"

#define VIDEOSESSION_TIMEOUT 5

enum
{
    VIDEOTYPE_CAPTURE       = 0x10000,
    VIDEOTYPE_MEDIAFILE     = 0x20000,

    VIDEOTYPE_MASK          = 0xF0000,
    VIDEOTYPE_USERMASK      = 0x0FFFF
};

class UserVideoWidget : public UserImageWidget
{
    Q_OBJECT

public:
    UserVideoWidget(QWidget * parent, int userid, 
                    int border_width);
    ~UserVideoWidget();

    int getStreamID() const { return m_video_frame? m_video_frame->nStreamID : 0; }

    QSize imageSize() const;

public:
    void slotNewVideoFrame(int userid, int stream_id);
    void slotUpdateVideoTextBox();

signals:
    void userVideoEnded(int userid);

protected:
#if defined(USE_TT_PAINT)
    void runTTPaint(QPainter& painter);
#else
    void completePaint(const QRect& r, QPainter& painter);
#endif
    void timerEvent(QTimerEvent *e);

    VideoFrame* acquireVideo();
    bool releaseVideo(VideoFrame* vid_frame);

    int m_novideo_count;

    VideoFrame* m_video_frame;

    //video text settings
    bool m_showvidtext;
    quint32 m_vidtext;
    QColor m_vidtext_fg, m_vidtext_bg;
    int m_vidtext_w_pct, m_vidtext_h_pct;
};

#endif
