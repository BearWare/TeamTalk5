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

#if !defined(THEORADECODE_H)
#define THEORADECODE_H

#include <ace/FILE_IO.h>

#include <theora/codec.h>
#include <theora/theoradec.h>

typedef unsigned char uchar_t;

class TheoraDecode
{
public:
    TheoraDecode();
    ~TheoraDecode();

    bool OpenDecoder();
    void CloseDecoder();

    bool GetFormat(th_info& info) const;

    //@return -1 error, 0 done, 1 more packets needed
    int ProcessHeader(ogg_packet& oggpkt);

    //true means a new frame can be retrieved
    bool DecodePacket(ogg_packet& oggpkt);

    //true means new frame written to outBuff
    bool OutputFrame(char* outBuff, int length);
private:
    th_info info_;
    th_comment comment_;
    th_setup_info* setup_;
    th_ycbcr_buffer ycbcr_;
    th_dec_ctx* dec_ctx_;
};

#endif
