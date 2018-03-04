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

#if !defined(THEORA_ENCODE_H)
#define THEORA_ENCODE_H

#include "myace/MyACE.h"
#include <ace/FILE_IO.h>

#include <theora/codec.h>
#include <theora/theoraenc.h>

typedef unsigned char uchar_t;

class TheoraEncode
{
public:
    TheoraEncode();
    ~TheoraEncode();

    bool OpenEncoder(int width, int height, int fps_numerator, 
                     int fps_denominator, int quality, int bitrate);
    void CloseEncoder();
    
    // @return -1 error, 0 done, 1 more headers
    int ProcessHeader(ogg_packet& oggpkt);

    bool EncodeFrame(const uchar_t* frame_buff, bool top_down_image);
    // @return -1 error, 0 done, 1 more packets
    int FlushEncoder(ogg_packet& oggpkt, bool final = false);

private:
    th_enc_ctx* enc_ctx_;
    th_ycbcr_buffer ycbcr_;
    th_info info_;
    th_comment comment_;
    uchar_t* tmp_img_buff_;
};

class TheoraEncFile
{
public:
    TheoraEncFile();
    ~TheoraEncFile();

    bool OpenEncoder(int width, int height, int fps_numerator, 
                     int fps_denominator, int quality, int bitrate, 
                     const ACE_TString& filename);
    void CloseEncoder();

    bool EncodeFrame(const uchar_t* frame_buff, bool top_down_image);
    // @return -1 error, 0 done, 1 more packets
    int FlushEncoder(bool final = false);

private:
    TheoraEncode encoder_;

    // For writing files
    ogg_stream_state ogg_state_;
    ACE_FILE_IO file_;
    ogg_packet oggpkt_;
    ogg_page oggpg_;
};

#endif
