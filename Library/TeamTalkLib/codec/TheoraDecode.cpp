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

#include "TheoraDecode.h"
#include <assert.h>

void DecodeToRGB32(const th_ycbcr_buffer& ycbcr, const th_info& info, uchar_t* outBuff);

TheoraDecode::TheoraDecode()
: dec_ctx_(NULL)
, setup_(NULL)
{
    memset(ycbcr_, 0, sizeof(ycbcr_));
}

TheoraDecode::~TheoraDecode()
{
    assert(dec_ctx_ == NULL);
}

bool TheoraDecode::OpenDecoder()
{
    if(dec_ctx_)
        return false;

    th_info_init(&info_);
    th_comment_init(&comment_);

    return true;
}

void TheoraDecode::CloseDecoder()
{
    if(dec_ctx_)
        th_decode_free(dec_ctx_);
    dec_ctx_ = NULL;

    th_setup_free(setup_);
    setup_ = NULL;

    th_comment_clear(&comment_);
    th_info_clear(&info_);
}

bool TheoraDecode::GetFormat(th_info& info) const
{
    if(dec_ctx_)
    {
        info = info_;
        return true;
    }
    return false;
}

int TheoraDecode::ProcessHeader(ogg_packet& oggpkt)
{
    int ret;
    assert(dec_ctx_ == NULL);
    if(dec_ctx_)
        return -1;

    ret = th_decode_headerin(&info_, &comment_, &setup_, &oggpkt);
    if(ret == 0)
        dec_ctx_ = th_decode_alloc(&info_, setup_);

    return ret;
}

bool TheoraDecode::DecodePacket(ogg_packet& oggpkt)
{
    assert(dec_ctx_);
    if(!dec_ctx_)
        return false;

    int ret = th_decode_packetin(dec_ctx_, &oggpkt, NULL);
    return ret == 0;
}

bool TheoraDecode::OutputFrame(char* outBuff, int length)
{
    assert(dec_ctx_);
    if(!dec_ctx_)
        return false;

    if(th_decode_ycbcr_out(dec_ctx_, ycbcr_) == 0)
    {
        DecodeToRGB32(ycbcr_, info_, reinterpret_cast<uchar_t*>(outBuff));
        return true;
    }
    return false;
}

inline uchar_t CLAMP(short v)    
{
    if (v > 255)
        return 255;
    else if (v < 0)
        return 0;
    return (uchar_t)v;
}

void DecodeToRGB32(const th_ycbcr_buffer& ycbcr, const th_info& info, uchar_t* outBuff)
{
    uchar_t * ptry = ycbcr[0].data;
    uchar_t * ptru = ycbcr[1].data;
    uchar_t * ptrv = ycbcr[2].data;
    uchar_t * ptro = outBuff;

    for (int i = info.pic_y; i < ycbcr[0].height; i++) 
    {
        uchar_t* ptro2 = ptro;
        for (int j = info.pic_x; j < ycbcr[0].width; j += 2) 
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
            *ptro2++ = 255;

            y = 298*ptry[j + 1] >> 8;
            r = y + pr;
            g = y + pg;
            b = y + pb;

            *ptro2++ = CLAMP(b);
            *ptro2++ = CLAMP(g);
            *ptro2++ = CLAMP(r);
            *ptro2++ = 255;
        }
        ptry += ycbcr[0].stride;
        if (i & 1) 
        {
            ptru += ycbcr[1].stride;
            ptrv += ycbcr[1].stride;
        }
        ptro += info.frame_width * 4;
    }
}
