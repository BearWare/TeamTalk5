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

#include "TheoraEncode.h"

#include <assert.h>
#include <ace/FILE_Connector.h> 

void encodeRGB32toYV12(const th_ycbcr_buffer& ycbcr, 
                       const th_info& info,
                       bool imageFlipped,
                       const uchar_t* inBuf, 
                       uchar_t* outBuf, 
                       long inNumBytes);
void encodeAYUVtoYV12(const th_ycbcr_buffer& ycbcr, 
                      const th_info& info,
                      uchar_t* inBuf, 
                      long inNumBytes);

TheoraEncode::TheoraEncode()
: enc_ctx_(NULL)
, tmp_img_buff_(NULL)
{
    memset(ycbcr_, 0, sizeof(ycbcr_));
}

TheoraEncode::~TheoraEncode()
{
    CloseEncoder();
}

bool TheoraEncode::OpenEncoder(int width, int height, int fps_numerator, 
                               int fps_denominator, int quality, int bitrate)
{
    assert(enc_ctx_ == NULL);
    if(enc_ctx_)
        return false;

    th_info_init(&info_);

    info_.frame_width = width;
    info_.frame_height = height;
    info_.pic_width = width;
    info_.pic_height = height;
    info_.fps_numerator = fps_numerator;
    info_.fps_denominator = fps_denominator;
    info_.quality = quality;
    info_.target_bitrate = bitrate;

    info_.frame_height -= info_.frame_height%16;
    info_.frame_width -= info_.frame_width%16;
    info_.pic_width -= info_.pic_width%16;
    info_.pic_height -= info_.pic_height%16;

    enc_ctx_ = th_encode_alloc(&info_);
    if(!enc_ctx_)
        return false;

    th_comment_init(&comment_);

    //Y
    ycbcr_[0].width = info_.frame_width;
    ycbcr_[0].height = info_.frame_height;
    ycbcr_[0].stride = info_.frame_width;
    //Cb
    ycbcr_[1].width = ycbcr_[0].width / 2;
    ycbcr_[1].height = info_.frame_height / 2;
    ycbcr_[1].stride = ycbcr_[0].width / 2;
    //Cr
    ycbcr_[2] = ycbcr_[1];

    int locYBuffSize = ycbcr_[0].height * ycbcr_[0].stride;
    ycbcr_[0].data = new uchar_t[locYBuffSize];
    ycbcr_[1].data = new uchar_t[locYBuffSize/4];
    ycbcr_[2].data = new uchar_t[locYBuffSize/4];

    tmp_img_buff_ = new uchar_t[info_.pic_width*info_.pic_height*4];

    if(ycbcr_[0].data && ycbcr_[1].data && ycbcr_[2].data && tmp_img_buff_)
        return true;

    CloseEncoder();
    return false;
}

void TheoraEncode::CloseEncoder()
{
    delete [] ycbcr_[0].data;
    delete [] ycbcr_[1].data;
    delete [] ycbcr_[2].data;
    memset(ycbcr_, 0, sizeof(ycbcr_));
    delete [] tmp_img_buff_;
    tmp_img_buff_ = NULL;

    if(enc_ctx_)
        th_encode_free(enc_ctx_);
    enc_ctx_ = NULL;

    th_comment_clear(&comment_);
    th_info_clear(&info_);
}

int TheoraEncode::ProcessHeader(ogg_packet& oggpkt)
{
    return th_encode_flushheader(enc_ctx_, &comment_, &oggpkt);
}

bool TheoraEncode::EncodeFrame(const uchar_t* frame_buff, bool top_down_image)
{
    assert(enc_ctx_);
    if(!enc_ctx_)
        return false;

    encodeRGB32toYV12(ycbcr_, info_, top_down_image, frame_buff,
                      tmp_img_buff_, info_.pic_width * info_.pic_height * 4);

    if(th_encode_ycbcr_in(enc_ctx_, ycbcr_) != 0)
        return false;

    return true;
}

int TheoraEncode::FlushEncoder(ogg_packet& oggpkt, bool final/* = false*/)
{
    return th_encode_packetout(enc_ctx_, (int)final, &oggpkt);
}


TheoraEncFile::TheoraEncFile()
{
}

TheoraEncFile::~TheoraEncFile()
{
}

bool TheoraEncFile::OpenEncoder(int width, int height, 
                                int fps_numerator, int fps_denominator, 
                                int quality, int bitrate, 
                                const ACE_TString& filename)
{
    int ret;
    ACE_FILE_Connector con;

    if(!encoder_.OpenEncoder(width, height, fps_numerator, fps_denominator,
                             quality, bitrate))
        return false;

    if(file_.get_handle() != ACE_INVALID_HANDLE)
        goto cleanup;

#if !defined(UNDER_CE)
    if(con.connect(file_, ACE_FILE_Addr(filename.c_str()),
        0, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT | O_TRUNC) < 0)
        goto cleanup;
#else
    if(con.connect(file_, ACE_FILE_Addr(filename.c_str()),
        0, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT, FILE_SHARE_READ | FILE_SHARE_WRITE)<0)
        goto cleanup;
#endif
    ogg_stream_init(&ogg_state_, 3);

    if(encoder_.ProcessHeader(oggpkt_)<=0)
        goto cleanup;

    ret = ogg_stream_packetin(&ogg_state_, &oggpkt_);
    ret = ogg_stream_pageout(&ogg_state_, &oggpg_);
    assert(ret>0);
    file_.send(oggpg_.header, oggpg_.header_len);
    file_.send(oggpg_.body, oggpg_.body_len);
        
    while((ret = encoder_.ProcessHeader(oggpkt_)) > 0)
        ogg_stream_packetin(&ogg_state_, &oggpkt_);

    if(ret<0)
        goto cleanup;

    /* ret == 0 */
    while(ogg_stream_flush(&ogg_state_, &oggpg_) > 0)
    {
        file_.send(oggpg_.header, oggpg_.header_len);
        file_.send(oggpg_.body, oggpg_.body_len);
    }

    return true;

cleanup:
    CloseEncoder();
    return false;
}

void TheoraEncFile::CloseEncoder()
{
    encoder_.CloseEncoder();

    ogg_stream_clear(&ogg_state_);

    if(file_.get_handle() != ACE_INVALID_HANDLE)
        file_.close();
}

bool TheoraEncFile::EncodeFrame(const uchar_t* frame_buff, bool top_down_image)
{
    return encoder_.EncodeFrame(frame_buff, top_down_image);
}

int TheoraEncFile::FlushEncoder(bool final/* = false*/)
{
    int ret = 0;
    assert(file_.get_handle() != ACE_INVALID_HANDLE);
    if(file_.get_handle() == ACE_INVALID_HANDLE)
        return -1;

    while(encoder_.FlushEncoder(oggpkt_, final)>0)
    {
        ret = ogg_stream_packetin(&ogg_state_, &oggpkt_);
        assert(ret >= 0);
    }

    while(ogg_stream_pageout(&ogg_state_, &oggpg_)>0)
    {
        file_.send(oggpg_.header, oggpg_.header_len);
        file_.send(oggpg_.body, oggpg_.body_len);
    }

    if(final)
    {
        if(ogg_stream_flush(&ogg_state_,&oggpg_)>0)
        {
            if(ogg_stream_pageout(&ogg_state_, &oggpg_)>0)
            {
                file_.send(oggpg_.header, oggpg_.header_len);
                file_.send(oggpg_.body, oggpg_.body_len);
            }
        }
        assert(ogg_stream_eos(&ogg_state_));

        return 0;
    }

    return 0;
}







#define CLIP3(x,y,z) ((z < x) ? x : ((z > y) ? y : z))

void encodeRGB32toYV12(const th_ycbcr_buffer& ycbcr, 
                       const th_info& info,
                       bool imageFlipped,
                       const uchar_t* inBuf, 
                       uchar_t* outBuf, 
                       long inNumBytes) 
{
    //Scaled by factor of 65536 to integerise.
    const int KR = 19596;
    const int KB = 7472;
    
    const int ROUNDER = 32768;

    const int G_FACTOR = 38470;
    const int U_FACTOR = 12716213;
    const int V_FACTOR = 10061022;

    int locL = 0;
    int locB = 0;
    int locR = 0;

    //uchar_t* locSourcePtr = inBuf;
    uchar_t* pDest = outBuf;

    //SOURCE: Blue Green Red Blue Green Red.
    //DEST: v u y a

    const uchar_t* pSourceEnds = inBuf + inNumBytes;

    const uchar_t* pSource = 0;
    const uchar_t* pEnd = 0;

    long stride = 0;

    if (imageFlipped)
    {
        stride = info.frame_width * 4;
        pSource = inBuf;
        pEnd = pSourceEnds;
    }
    else
    {
        // Negative stride
        stride = 0 - info.frame_width * 4;
        pSource = pSourceEnds - std::abs(stride);
        pEnd = inBuf - std::abs(stride);
    }

    for (; pSource != pEnd; pSource += stride) 
    {
        const uchar_t* pColSource = pSource;
        const uchar_t* pColEnd = pColSource + std::abs(stride);

        while (pColSource < pColEnd) 
        {
            locB = pColSource[0];                    //Blue
            locL = KB * (locB);                        //Blue

            locL += G_FACTOR * (pColSource[1]);        //Green

            locR = pColSource[2];                    //Red
            locL += KR * (locR);                    //Red

            *(pDest++) = CLIP3(0, 255, ((112 * ( (locR<<16) - locL)) / V_FACTOR) + 128);    //V for Victor
            *(pDest++) = CLIP3(0, 255, ((112 * ( (locB<<16) - locL)) / U_FACTOR) + 128);    //U for ugly
            *(pDest++) = CLIP3(0, 255, locL >> 16);                                            //Y for yellow
            *(pDest++) = pColSource[3];                                                        //A for alpha

            pColSource += 4;
        }
    }
    
    encodeAYUVtoYV12(ycbcr, info, outBuf, inNumBytes);
}

void encodeAYUVtoYV12(const th_ycbcr_buffer& ycbcr, 
                      const th_info& info,
                      uchar_t* inBuf, 
                      long inNumBytes) {

    //TODO::: This doesn't appear to do offsets.

    //Victor Ugly Yellow Alpha --fonts are fuzzy late at night-- (Yellow is not colour yellow)

    //AYUV is VUYA VUYA VUYA VUYA 4:4:4 sampling
    // Twice the vertical and horizontal sampling of YV12 in chrominance

    //Strategy : Process two lines and 2 cols at a time averaging 4 U and V around the position where a
    // YV12 chroma sample will be... leave luminance samples... ignore alpha samples

    //debugLog<<"Encode AYUV To YV12 :"<<endl;
    
    const int PIXEL_BYTE_SIZE = 4;
    assert (info.frame_height % 2 == 0);
    assert (info.frame_width % 2 == 0);

    uchar_t* locSourceUptoPtr = inBuf;                        //View only... don't delete locUptoPtr
    uchar_t* locSourceNextLine = locSourceUptoPtr + (info.frame_width * PIXEL_BYTE_SIZE);    //View only don't delete
    
    uchar_t* locYUpto = ycbcr[0].data;
    uchar_t* locUUpto = ycbcr[1].data;
    uchar_t* locVUpto = ycbcr[2].data;
    //Pointer to the same pixel on next line
    uchar_t* locDestNextLine = locYUpto + (ycbcr[0].stride);                //View only... don't delete

    int temp = 0;

    //Process 2 lines at a time
    for (unsigned long line = 0; line < info.frame_height; line += 2) {
        //debugLog<<"Encode AYUV To YV12 : ++ Line = "<<line<<endl;
        
        //Ensures the current destination buffer skips a line ie points to line 2, 4, 6 etc
        locYUpto = (ycbcr[0].data + (line * ycbcr[0].stride));
        //Ensures the nextlinedest skips a
        locDestNextLine = locYUpto + (ycbcr[0].stride);

        //locUUpto = (ycbcr[1].data + ((line/2) * ycbcr[1].datav_stride));
        //locVUpto = (ycbcr[2].data + ((line/2) * ycbcr[1].datav_stride));

        assert (locSourceUptoPtr == (inBuf + (line * (info.frame_width * PIXEL_BYTE_SIZE))));
        assert (locSourceNextLine == locSourceUptoPtr + (info.frame_width * PIXEL_BYTE_SIZE));
        assert (locYUpto == (ycbcr[0].data + (line * ycbcr[0].stride)));
        assert (locDestNextLine == locYUpto + (ycbcr[0].stride));
        

        //Pad out the start of the line if needed
        if (info.pic_x != 0) {
            memset((void*)locYUpto, 0, info.pic_x);
            memset((void*)locDestNextLine, 0, info.pic_x);
            memset((void*)locUUpto, 0, info.pic_x/2);
            memset((void*)locVUpto, 0, info.pic_x/2);
            locYUpto += info.pic_x;
            locDestNextLine += info.pic_x;
            locUUpto += (info.pic_x/2);
            locVUpto += (info.pic_x/2);
        }
        //Columns also done 2 at a time
        for (unsigned long col = 0; col < info.frame_width; col += 2) {
            //debugLog<<"Encode AYUV To YV12 : ++++++ Col = "<<col<<endl;
            




            //                        v    u    y    a    v    u    y    a
            //SourceUpto            ^
            //NextUpto                ^

            //====================
            //V for Victor samples
            //====================
            temp =    *(locSourceUptoPtr++);            //Current pixel

            //                        v    u    y    a    v    u    y    a
            //SourceUpto                ^
            //NextUpto                ^

            //This is three as we already advanced one and are pointing to a u not a v
            temp += *(locSourceUptoPtr + 3);        //Pixel to right
            temp += *(locSourceNextLine++);            //Pixel below

            //                        v    u    y    a    v    u    y    a
            //SourceUpto                ^
            //NextUpto                    ^

            temp += *(locSourceNextLine + 3);            //Pixel below right
            temp >>= 2;                                //Divide by 4 to average.
            *(locVUpto++) = (uchar_t)temp;

            //                        v    u    y    a    v    u    y    a
            //SourceUpto                ^
            //NextUpto                    ^

            //====================
            //U for Ugly samples
            //====================
            temp =    *(locSourceUptoPtr++);            //Current pixel

            //                        v    u    y    a    v    u    y    a
            //SourceUpto                    ^
            //NextUpto                    ^

            temp += *(locSourceUptoPtr + 3);            //Pixel to right
            temp += *(locSourceNextLine++);            //Pixel below


            //                        v    u    y    a    v    u    y    a
            //SourceUpto                    ^
            //NextUpto                        ^

            temp += *(locSourceNextLine + 3);            //Pixel below right
            temp >>= 2;                                //Divide by 4 to average.
            *(locUUpto++) = (uchar_t)temp;

            //                        v    u    y    a    v    u    y    a
            //SourceUpto                    ^
            //NextUpto                        ^

            //====================
            //Y for Yellow samples.
            //====================

            *(locYUpto++) = *(locSourceUptoPtr++);
            *(locDestNextLine++) = *(locSourceNextLine++);

            //                        v    u    y    a    v    u    y    a
            //SourceUpto                        ^
            //NextUpto                            ^

            //Ignore the alpha channel
            //--
            //locSourceUptoPtr++;            //Optimised away... merged into increment below
            //locSourceNextLine++;            // "    "    "    "    "    "    "    "    "    "    "

            //                        v    u    y    a    v    u    y    a
            //SourceUpto                            ^
            //NextUpto                                ^


            //--
            //Source and next pointer have advanced four bytes so far.
            //Added 2 Y for yellows (one from each line) and one each of U and V, ignore an A on each line
            //--

            //Current line extra Y for yellows.
            locSourceUptoPtr += 3;                    //Skip the A and U and V samples
            *(locYUpto++) = *(locSourceUptoPtr);    //get the Y for yellow sample        
            locSourceUptoPtr += 2;                    //Advance 1 for the Y for yellow and Skip the A sample.

            //                        v    u    y    a    v    u    y    a
            //SourceUpto                                            ^
            //NextUpto                                ^


            //Next line extra Y for yellows.
            locSourceNextLine += 3;                            //Skip the A and U and V samples
            *(locDestNextLine++) = *(locSourceNextLine);    //get the Y for yellow sample        
            locSourceNextLine += 2;                            //Advance 1 for the Y for yellow and Skip the A sample.

            //                        v    u    y    a    v    u    y    a
            //SourceUpto                                            ^
            //NextUpto                                                ^

            //--
            //In total source and next pointer advanced 8 bytes on each line, and we got 4 Y for yellows (2 each line)
            // and one each U, V, ignored 4 A's (2 each line)
            //--


        }
        //Overall this loop will advance :
        //Sourceupto    by        = 8 bytes * (info.frame_width in pixels / 2 pixels at a time) * 2 lines
        //                        = 8 * info.frame_width
        //                        = 2 lines of 4 byte pixels of pixel width info.frame_width
        //and the same for sourcenextline
        //--
        //At the end of this loop sourceupto points to the end of the current line (start of next line)
        // and nextupto points to the end of the next line
        //
        //On the next iteration we want the sourceupto to point 2 lines down from where it did on the previous one
        // This is the same place that the sourcenextline points at the end of the iteration.
        //--

        //Ensures source will point to lines 2, 4, 6 etc.
        locSourceUptoPtr = locSourceNextLine;            
        //Ensures nextlinesource is one line ahead of the main source.
        locSourceNextLine += (info.frame_width * PIXEL_BYTE_SIZE);
    }
}
