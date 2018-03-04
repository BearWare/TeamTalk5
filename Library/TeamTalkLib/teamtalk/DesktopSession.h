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

#ifndef DESKTOPSESSION_H
#define DESKTOPSESSION_H

#include "Common.h"
#include "PacketLayout.h"
#include <vector>

#define RGB8_BLOCK_PIXEL_W 120
#define RGB8_BLOCK_PIXEL_H 34

#define RGB16_BLOCK_PIXEL_W 102
#define RGB16_BLOCK_PIXEL_H 20

#define RGB24_BLOCK_PIXEL_W 85
#define RGB24_BLOCK_PIXEL_H 16

#define RGB32_BLOCK_PIXEL_W 51
#define RGB32_BLOCK_PIXEL_H 20

#define BLOCK_MAX_BYTESIZE 0xFFF



#define DESKTOP_RTX_MIN_TIMEOUT         ACE_Time_Value(0, 10000) //minimum RTX timeout

#define DESKTOP_RTX_TIMER_INTERVAL      ACE_Time_Value(1, 0) //interval for checking whether to do RTX
#define DESKTOP_DEFAULT_RTX_TIMEOUT     ACE_Time_Value(4, 0) //consider a packet lost after this duration

namespace teamtalk {

    class DesktopSession
    {
    public:
        DesktopSession(const DesktopWindow& wnd);
        DesktopSession(const DesktopWindow& wnd, int bytes_per_line);

        int GetSessionID() const { return m_wnd.session_id; }
        RGBMode GetRGBMode() const;
        int GetWidth() const { return m_wnd.width; }
        int GetHeight() const { return m_wnd.height; }
        const DesktopWindow& GetDesktopWindow() const { return m_wnd; }
        int GetBlocksCount() const { return m_w_blocks * m_h_blocks; }
        int GetPadding() const { return m_padding; }
        int GetBitmapSize() const;
        int GetBytesPerLine() const;
        int GetWidthSize() const { return GetWidth() * m_pixel_size; }
        DesktopProtocol GetDesktopProtocol() const { return m_wnd.desktop_protocol; }

    protected:
        void Init();
        DesktopWindow m_wnd;
        int m_pixel_size;
        int m_padding;
        int m_w_blocks, m_h_blocks;
        int m_block_width, m_block_height;
        int m_bytes_per_line;
    };

    class BMPPalette
    {
    public:
        static BMPPalette* Instance();

        ACE_UINT8 m_rgb8_palette[256][4];

    private:
        BMPPalette();
        void InitPalette();
    };

    //index in 'm_rgb8_palette'
    inline int RGB8Palette(unsigned char r, unsigned char g, unsigned char b);

    DesktopSession MakeDesktopSession(int width, int height, RGBMode rgb_mode, 
                                      int bytes_per_line = 0);
    
    size_t ConvertBitmap(const std::vector<char>& src_bitmap,
                         const DesktopSession& src_ses,
                         std::vector<char>& dst_bitmap, 
                         const DesktopSession& dst_ses);
    
}

#endif
