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

#include "uservideowidget.h"
#include "settings.h"

#include <QPaintEngine>
#include <QDebug>
#include <QTimerEvent>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

#define USE_VIDEO_SHAREDMEM_ZEROCOPY 1

UserVideoWidget::UserVideoWidget(QWidget * parent, int userid,
                                 int border_width)
  : UserImageWidget(parent, userid, true, border_width)
  , m_novideo_count(0)
  , m_showvidtext(false)
{
    Q_ASSERT(userid & VIDEOTYPE_MASK);

    slotUpdateVideoTextBox();
    startTimer(1000);

    User user;
    TT_GetUser(ttInst, m_userid & VIDEOTYPE_USERMASK, &user);

    if(userid & VIDEOTYPE_CAPTURE)
    {
        if((userid & VIDEOTYPE_USERMASK) == 0)
            m_textmsg = tr("Waiting for local video");
        else
            m_textmsg = tr("Waiting for video from %1").arg(_Q(user.szNickname));
    }

    if(userid & VIDEOTYPE_MEDIAFILE)
        m_textmsg = tr("Waiting for media file from %1").arg(_Q(user.szNickname));

    m_video_frame = acquireVideo();

#if !defined(USE_TT_PAINT) && !USE_VIDEO_SHAREDMEM_ZEROCOPY
    if(m_video_frame)
    {
        m_image = QImage(m_video_frame->nWidth, m_video_frame->nHeight,
                         QImage::Format_RGB32);
        if(m_mirrored)
            m_image = m_image.mirrored(true, true);
    }
#endif
}

UserVideoWidget::~UserVideoWidget()
{
    releaseVideo(m_video_frame);
}

QSize UserVideoWidget::imageSize() const
{
#ifndef USE_TT_PAINT
    if(!m_image.isNull())
        return m_image.size();
#endif
    if(m_video_frame && m_video_frame->nWidth && m_video_frame->nHeight)
        return QSize(m_video_frame->nWidth, m_video_frame->nHeight);
    return QSize();
}

#ifdef USE_TT_PAINT
void UserVideoWidget::runTTPaint(QPainter& painter)
{
    HDC hdc = painter.paintEngine()->getDC();
    QPoint p0 = this->mapTo(nativeParentWidget(), QPoint(0,0));
    //drain frames (we don't want any queued frames)

    VideoFrame* vid_frame = NULL;
    while(vid_frame = acquireVideo())
    {
        releaseVideo(m_video_frame);
        m_video_frame = vid_frame;
    }
    if(m_video_frame)
        TT_PaintVideoFrame(hdc, p0.x(), p0.y(), width(), height(), m_video_frame);
    painter.paintEngine()->releaseDC(hdc);
}
#endif

void UserVideoWidget::slotNewVideoFrame(int userid, int /*stream_id*/)
{
    Q_ASSERT(userid & VIDEOTYPE_MASK);

    if(m_userid != userid)
        return;

    m_novideo_count = 0;

#ifndef USE_TT_PAINT
    bool modified = false;
    //drain frames (we don't want any queued frames)
    while(true)
    {
#if USE_VIDEO_SHAREDMEM_ZEROCOPY
        //Zero-Copy (shared mem).
        //Use memory allocated by TeamTalk DLL as raw input to QImage

        VideoFrame* vid_frame = acquireVideo();
        if(vid_frame)
        {
            releaseVideo(m_video_frame);
            m_video_frame = vid_frame;
            m_image = QImage(static_cast<uchar*>(m_video_frame->frameBuffer),
                             m_video_frame->nWidth, m_video_frame->nHeight,
                             QImage::Format_RGB32);
            if(m_mirrored)
                m_image = m_image.mirrored(true, true);
        }
        else break;
#else
        if(m_video_frame)
            releaseVideo(m_video_frame);
        m_video_frame = NULL;

        VideoFrame* vid_frame = acquireVideo();
        if(vid_frame)
        {
            m_image = QImage(vid_frame->nWidth, vid_frame->nHeight,
                             QImage::Format_RGB32);
            Q_ASSERT(vid_frame->nFrameBufferSize == m_image.byteCount());
            memcpy(m_image.bits(), vid_frame->frameBuffer,
                   vid_frame->nFrameBufferSize);
            releaseVideo(vid_frame);
            if(m_mirrored)
                m_image = m_image.mirrored(true, true);
        }
        else break;
#endif
        modified = true;
    }

    if(modified)
        update();
#else
    Q_UNUSED(frame_count);
    update();
#endif
}

void UserVideoWidget::slotUpdateVideoTextBox()
{
    m_showvidtext = ttSettings->value(SETTINGS_DISPLAY_VIDEOTEXT_SHOW, false).toBool();

    if(m_showvidtext)
        m_nickname.clear();
    else
    {
        User user;
        if(TT_GetUser(ttInst, m_userid & VIDEOTYPE_USERMASK, &user))
            m_nickname = _Q(user.szNickname);
    }

    m_vidtext = ttSettings->value(SETTINGS_DISPLAY_VIDEOTEXT,
                                  SETTINGS_DISPLAY_VIDEOTEXT_DEFAULT).toUInt();
        
    m_vidtext_w_pct = ttSettings->value(SETTINGS_DISPLAY_VIDEOTEXT_WIDTH,
                                        SETTINGS_DISPLAY_VIDEOTEXT_WIDTH_DEFAULT).toInt();
    m_vidtext_h_pct = ttSettings->value(SETTINGS_DISPLAY_VIDEOTEXT_HEIGHT,
                                        SETTINGS_DISPLAY_VIDEOTEXT_HEIGHT_DEFAULT).toInt();

    m_vidtext_bg = ttSettings->value(SETTINGS_DISPLAY_VIDEOTEXT_BGCOLOR,
                                     SETTINGS_DISPLAY_VIDEOTEXT_BGCOLOR_DEFAULT)
                                     .value<QColor>();
    m_vidtext_fg = ttSettings->value(SETTINGS_DISPLAY_VIDEOTEXT_FGCOLOR,
                                     SETTINGS_DISPLAY_VIDEOTEXT_FGCOLOR_DEFAULT)
                                     .value<QColor>();
    update();
}

void UserVideoWidget::timerEvent(QTimerEvent *e)
{
    if(m_novideo_count++ >= VIDEOSESSION_TIMEOUT)
    {
        killTimer(e->timerId());
        emit(userVideoEnded(m_userid));
    }
}

VideoFrame* UserVideoWidget::acquireVideo()
{
    switch(m_userid & VIDEOTYPE_MASK)
    {
    case VIDEOTYPE_CAPTURE :
        return TT_AcquireUserVideoCaptureFrame(ttInst,
                                               m_userid & VIDEOTYPE_USERMASK);
    break;
    case VIDEOTYPE_MEDIAFILE :
        return TT_AcquireUserMediaVideoFrame(ttInst,
                                             m_userid & VIDEOTYPE_USERMASK);
    }
    return NULL;
}

bool UserVideoWidget::releaseVideo(VideoFrame* vid_frame)
{
    switch(m_userid & VIDEOTYPE_MASK)
    {
    case VIDEOTYPE_CAPTURE :
        return TT_ReleaseUserVideoCaptureFrame(ttInst, vid_frame);
    case VIDEOTYPE_MEDIAFILE :
        return TT_ReleaseUserMediaVideoFrame(ttInst, vid_frame);
    }
    return false;
}

#if !defined(USE_TT_PAINT)
void UserVideoWidget::completePaint(const QRect& r, QPainter& painter)
{
    if(m_showvidtext && m_vidtext != VIDEOTEXT_NONE)
    {
        User user;
        if(TT_GetUser(ttInst, m_userid & VIDEOTYPE_USERMASK, &user))
        {
            QString text = QString("User #%1").arg(m_userid & VIDEOTYPE_USERMASK);
            switch(m_vidtext & VIDTEXT_SHOW_MASK)
            {
            case VIDTEXT_SHOW_NICKNAME :
                text = _Q(user.szNickname);
                break;
            case VIDTEXT_SHOW_USERNAME :
                text = _Q(user.szUsername);
                break;
            case VIDTEXT_SHOW_STATUSTEXT :
                text = _Q(user.szStatusMsg);
                break;
            }
            setVideoTextBox(r, m_vidtext_bg, m_vidtext_fg, text, m_vidtext, 
                            m_vidtext_w_pct, m_vidtext_h_pct, painter);
        }
    }
}
#endif
