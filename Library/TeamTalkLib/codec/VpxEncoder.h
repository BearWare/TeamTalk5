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

#ifndef VPXENCODER_H
#define VPXENCODER_H

#ifndef VPX_CODEC_DISABLE_COMPAT
#define VPX_CODEC_DISABLE_COMPAT 1
#endif

#include <vpx/vpx_encoder.h>

class VpxEncoder
{
public:
    VpxEncoder();
    ~VpxEncoder();

    bool Open(int width, int height, int target_bitrate, int fps);
    void Close();

    vpx_codec_err_t Encode(const char* imgbuf, vpx_img_fmt fmt, int stride,
                           bool bottom_up, unsigned long tm, int enc_deadline);

    vpx_codec_err_t EncodeRGB32(const char* imgbuf, int imglen, bool bottom_up_bmp,
                                unsigned long tm, int enc_deadline);

    const char* GetEncodedData(int& len);

private:
    vpx_codec_ctx_t m_codec;
    vpx_codec_enc_cfg_t m_cfg;
    vpx_codec_iter_t m_iter;
    vpx_codec_pts_t m_frame_index;
};
#endif
