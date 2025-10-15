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

#include "VpxEncoder.h"

#include "MediaUtil.h"

#include <vpx/vp8cx.h>

#include <cassert>
#include <cstddef>
#include <cstring>

#define enc_interface vpx_codec_vp8_cx()

static void RGB32toYUV420P(const unsigned char * rgb,
                    unsigned char * yuv,
                    unsigned rgbIncrement,
                    unsigned char flip,
                    int srcFrameWidth, int srcFrameHeight);

VpxEncoder::VpxEncoder()
: m_codec()
, m_cfg()
, m_iter(nullptr)
, m_frame_index(0)
{
}

VpxEncoder::~VpxEncoder()
{
    Close();
}

bool VpxEncoder::Open(int width, int height, int target_bitrate, int fps)
{
    if(m_codec.iface != nullptr)
        return false;

    vpx_codec_err_t ret;
    ret = vpx_codec_enc_config_default(enc_interface, &m_cfg, 0);
    assert(ret == VPX_CODEC_OK);
    if(ret != VPX_CODEC_OK)
        return false;

    m_cfg.g_w = width;
    m_cfg.g_h = height;
    m_cfg.g_error_resilient = VPX_ERROR_RESILIENT_DEFAULT;
    m_cfg.g_threads = 4;
    if(target_bitrate != 0)
        m_cfg.rc_target_bitrate = target_bitrate;
    m_cfg.g_timebase.num = 1;
    m_cfg.g_timebase.den = fps;

    ret = vpx_codec_enc_init(&m_codec, enc_interface, &m_cfg, 0);
    assert(ret == VPX_CODEC_OK);
    return ret == VPX_CODEC_OK;
}

void VpxEncoder::Close()
{
    if(m_codec.iface != nullptr)
        vpx_codec_destroy(&m_codec);
    memset(&m_codec, 0, sizeof(m_codec));
    m_iter = nullptr;
    m_frame_index = 0;
}

bool VpxEncoder::Update(int  /*target_bitrate*/)
{
    if (m_codec.iface == nullptr)
        return false;

    return vpx_codec_enc_config_set(&m_codec, &m_cfg) == VPX_CODEC_OK;
}

vpx_codec_err_t VpxEncoder::Encode(const char* imgbuf, vpx_img_fmt fmt, int stride,
                                   bool bottom_up, unsigned long  /*tm*/, int enc_deadline)
{
    vpx_codec_err_t ret;
    vpx_image_t* img = nullptr;

    assert(m_codec.iface);

    /* VPX supported formats 
    VPX_IMG_FMT_YV12
    VPX_IMG_FMT_I420
    VPX_IMG_FMT_VPXI420
    VPX_IMG_FMT_VPXYV12
    */

    img = vpx_img_wrap(nullptr, fmt, m_cfg.g_w, m_cfg.g_h, stride, reinterpret_cast<unsigned char*>(const_cast<char*>(imgbuf)));

    if (!bottom_up && (img != nullptr))
    {
        vpx_img_flip(img);
    }

    ret = vpx_codec_encode(&m_codec, img, m_frame_index++, 1 /*duration*/,
        0, enc_deadline);
    assert(ret == VPX_CODEC_OK);
    vpx_img_free(img);

    return ret;
}

vpx_codec_err_t VpxEncoder::EncodeRGB32(const char* imgbuf, int imglen, bool bottom_up_bmp,
                                        unsigned long /* tm */, int enc_deadline)
{
    vpx_codec_err_t ret;
    vpx_image_t* img = nullptr;

    assert(m_codec.iface);
    img = vpx_img_alloc(nullptr, VPX_IMG_FMT_YV12, m_cfg.g_w, m_cfg.g_h, 1);
    assert(img);
    assert(imglen == RGB32_BYTES(m_cfg.g_w, m_cfg.g_h));
    RGB32toYUV420P(reinterpret_cast<const unsigned char *>(imgbuf), 
                   img->img_data, 4, static_cast<unsigned char>(bottom_up_bmp), m_cfg.g_w, m_cfg.g_h);

    ret = vpx_codec_encode(&m_codec, img, m_frame_index++, 1 /*duration*/, 
                           0, enc_deadline);
    assert(ret == VPX_CODEC_OK);
    vpx_img_free(img);

    return ret;
}

const char* VpxEncoder::GetEncodedData(int& len)
{
    const vpx_codec_cx_pkt_t *pkt = nullptr;
    assert(m_codec.iface);

    if((pkt = vpx_codec_get_cx_data(&m_codec, &m_iter)) != nullptr)
    {
        switch(pkt->kind)
        {
        case VPX_CODEC_CX_FRAME_PKT :
            len = int(pkt->data.frame.sz);
            return reinterpret_cast<const char*>(pkt->data.frame.buf);
        break;
        case VPX_CODEC_STATS_PKT :
            break;
        case VPX_CODEC_PSNR_PKT :
            break;
        default :
            break;
        }
    }
    else
    {
        m_iter = nullptr;
    }

    return nullptr;
}


#define rgbtoy(b, g, r, y) \
y=(unsigned char)(((int)(30*(r)) + (int)(59*(g)) + (int)(11*(b)))/100)

#define rgbtoyuv(b, g, r, y, u, v) \
rgbtoy(b, g, r, y); \
u=(unsigned char)(((int)(-17*(r)) - (int)(33*(g)) + (int)(50*(b))+12800)/100); \
v=(unsigned char)(((int)(50*(r)) - (int)(42*(g)) - (int)(8*(b))+12800)/100)


void RGB32toYUV420P(const unsigned char * rgb,
                    unsigned char * yuv,
                    unsigned rgbIncrement,
                    unsigned char flip,
                    int srcFrameWidth, int srcFrameHeight)
{
    unsigned int planeSize = 0;
    unsigned int halfWidth = 0;

    unsigned char * yplane = nullptr;
    unsigned char * uplane = nullptr;
    unsigned char * vplane = nullptr;
    const unsigned char * rgbIndex = nullptr;

    int x;
    int y;
    unsigned char * yline = nullptr;
    unsigned char * uline = nullptr;
    unsigned char * vline = nullptr;

    planeSize = srcFrameWidth * srcFrameHeight;
    halfWidth = srcFrameWidth >> 1;

    // get pointers to the data
    yplane = yuv;
    uplane = yuv + planeSize;
    vplane = yuv + planeSize + (planeSize >> 2);
    rgbIndex = rgb;

    for (y = 0; y < srcFrameHeight; y++)
    {
        yline = yplane + (y * srcFrameWidth);
        uline = uplane + ((y >> 1) * halfWidth);
        vline = vplane + ((y >> 1) * halfWidth);

        if (flip != 0u)
            rgbIndex = rgb + (srcFrameWidth*(srcFrameHeight-1-y)*rgbIncrement);

        for (x = 0; x < srcFrameWidth; x+=2)
        {
            rgbtoyuv(rgbIndex[2], rgbIndex[1], rgbIndex[0], *yline, *uline, *vline);
            rgbIndex += rgbIncrement;
            yline++;
            rgbtoyuv(rgbIndex[2], rgbIndex[1], rgbIndex[0], *yline, *uline, *vline);
            rgbIndex += rgbIncrement;
            yline++;
            uline++;
            vline++;
        }
    }
}
