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

#ifndef VPXDECODER_H
#define VPXDECODER_H

#ifndef VPX_CODEC_DISABLE_COMPAT
#define VPX_CODEC_DISABLE_COMPAT 1
#endif

#include <vpx/vpx_decoder.h>

class VpxDecoder
{
public:
    VpxDecoder();
    ~VpxDecoder();

    bool Open(int width, int height);
    void Close();

    int PushDecoder(const char* frame_data, int frame_len);
    bool GetRGB32Image(char* outbuf, int buflen);
    const vpx_codec_dec_cfg_t& GetConfig() const { return m_cfg; }
private:
    vpx_codec_ctx_t m_codec;
    vpx_codec_dec_cfg_t m_cfg;
    vpx_codec_iter_t m_iter;
};
#endif
