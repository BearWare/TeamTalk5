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

#include "VpxDecoder.h"
#include <assert.h>
#include "vpx/vp8dx.h"
#define dec_interface vpx_codec_vp8_dx()

void I420toRGB32(vpx_image_t* img, uint8_t* outbuf, int outlen);

VpxDecoder::VpxDecoder()
: m_codec()
, m_cfg()
, m_iter(NULL)
{
}

VpxDecoder::~VpxDecoder()
{
}

bool VpxDecoder::Open(int width, int height)
{
    int flags = 0;
    vpx_codec_err_t ret;

    if(m_codec.iface)
        return false;
    
    m_cfg.threads = 0;
    m_cfg.w = width;
    m_cfg.h = height;
    ret = vpx_codec_dec_init(&m_codec, dec_interface, &m_cfg, flags);
    assert(ret == VPX_CODEC_OK);
    return ret == VPX_CODEC_OK;
}

void VpxDecoder::Close()
{
    if(m_codec.iface)
        vpx_codec_destroy(&m_codec);
    memset(&m_codec, 0, sizeof(m_codec));
    m_iter = NULL;
}

int VpxDecoder::PushDecoder(const char* frame_data, int frame_len)
{
    vpx_codec_err_t ret;

    assert(m_codec.iface);
    ret = vpx_codec_decode(&m_codec, 
                           reinterpret_cast<const uint8_t*>(frame_data),
                           frame_len, NULL, 0);
    //assert(ret == VPX_CODEC_OK);
    return ret;
}

vpx_image_t* VpxDecoder::GetVpxImage()
{
    vpx_image_t* img;
    assert(m_codec.iface);
    if((img = vpx_codec_get_frame(&m_codec, &m_iter)))
    {
        return img;
    }
    else
    {
        m_iter = NULL;
    }

    return NULL;
}

bool VpxDecoder::GetRGB32Image(char* outbuf, int buflen)
{
    assert(RGB32_BYTES(m_cfg.w, m_cfg.h) == buflen);
    vpx_image_t* img = GetVpxImage();
    if(img)
    {
        I420toRGB32(img, reinterpret_cast<uint8_t*>(outbuf), buflen);
    }

    return img != nullptr;
}

media::VideoFrame VpxDecoder::GetImage()
{
    vpx_image_t* img = GetVpxImage();

    if (img)
    {
        /*
        vpx_image.c:

        const uint64_t alloc_size = (fmt & VPX_IMG_FMT_PLANAR)
        ? (uint64_t)h * s * bps / 8
        : (uint64_t)h * s;

        */

        assert(img->fmt == VPX_IMG_FMT_I420);

        return media::VideoFrame(reinterpret_cast<char*>(img->img_data),
            img->d_w * img->d_h * img->bps / 8,
            img->d_w, img->d_h, media::FOURCC_I420, false);
    }

    return media::VideoFrame();
}


inline uint8_t CLAMP(short v)    
{
    if (v > 255)
        return 255;
    else if (v < 0)
        return 0;
    return (uint8_t)v;
}

void I420toRGB32(vpx_image_t* img, uint8_t* outbuf, int outlen)
{
    int plane_size = img->d_w * img->d_h;
    uint8_t * ptry = img->planes[VPX_PLANE_Y];
    uint8_t * ptru = img->planes[VPX_PLANE_U];
    uint8_t * ptrv = img->planes[VPX_PLANE_V];
    uint8_t * ptro = outbuf;

    for (unsigned int i = 0; i < img->d_h; i++) 
    {
        uint8_t* ptro2 = ptro;
        for (unsigned int j = 0; j < img->d_w; j += 2) 
        {
            short pr, pg, pb, y;
            short r, g, b;

            pr = (-56992 + ptrv[j / 2] * 409) >> 8;
            pg = (34784 - ptru[j / 2] * 100 - ptrv[j / 2] * 208) >> 8;
            pb = (short)((-70688 + ptru[j / 2] * 516) >> 8);

            y = 298*ptry[j] >> 8;
            r = y + pr;
            g = y + pg;
            b = y + pb;

            *ptro2++ = CLAMP(b);
            *ptro2++ = CLAMP(g);
            *ptro2++ = CLAMP(r);
            assert(ptro2 < outbuf + outlen);
            *ptro2++ = 255;
            y = 298*ptry[j + 1] >> 8;
            r = y + pr;
            g = y + pg;
            b = y + pb;

            *ptro2++ = CLAMP(b);
            *ptro2++ = CLAMP(g);
            *ptro2++ = CLAMP(r);
            assert(ptro2 < outbuf + outlen);
            *ptro2++ = 255;
        }
        ptry += img->stride[VPX_PLANE_Y];
        if (i & 1) 
        {
            ptru += img->stride[VPX_PLANE_U];
            ptrv += img->stride[VPX_PLANE_V];
        }
        ptro += img->d_w * 4;
    }
}
