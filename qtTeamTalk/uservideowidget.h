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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
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

public slots:
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
