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

#include "DesktopSession.h"
#include "ttassert.h"

using namespace std;
using namespace teamtalk;

DesktopSession::DesktopSession(const DesktopWindow& wnd)
: m_wnd(wnd)
, m_padding(0)
, m_bytes_per_line(0)
{
    Init();

    if(GetBitmapSize())
        m_padding = ((GetWidth() * m_pixel_size + 3) & ~3) - GetWidth() * m_pixel_size;
    TTASSERT((GetWidth() * m_pixel_size + m_padding) % 4 == 0);
}

DesktopSession::DesktopSession(const DesktopWindow& wnd, int bytes_per_line)
: m_wnd(wnd)
, m_bytes_per_line(bytes_per_line)
{
    Init();
    m_padding = 0;
}

void DesktopSession::Init()
{
    switch(m_wnd.rgb_mode)
    {
    case BMP_RGB8_PALETTE :
        m_pixel_size = 1;
        m_block_width = RGB8_BLOCK_PIXEL_W;
        m_block_height = RGB8_BLOCK_PIXEL_H;
        break;
    case BMP_RGB16_555 :
        m_pixel_size = 2;
        m_block_width = RGB16_BLOCK_PIXEL_W;
        m_block_height = RGB16_BLOCK_PIXEL_H;
        break;
    case BMP_RGB24 :
        m_pixel_size = 3;
        m_block_width = RGB24_BLOCK_PIXEL_W;
        m_block_height = RGB24_BLOCK_PIXEL_H;
        break;
    case BMP_RGB32 :
        m_pixel_size = 4;
        m_block_width = RGB32_BLOCK_PIXEL_W;
        m_block_height = RGB32_BLOCK_PIXEL_H;
        break;
    default :
        m_wnd.height = 0;
        m_wnd.width = 0;
        m_pixel_size = 0;
        m_padding = 0;
        m_w_blocks = 0;
        m_h_blocks = 0;
        m_block_width = 0;
        m_block_height = 0;
        m_bytes_per_line = 0;
        break;
    }
    assert(m_block_width * m_block_height * m_pixel_size <= BLOCK_MAX_BYTESIZE);

    if(m_block_width && m_block_height)
    {
        m_w_blocks = m_wnd.width / m_block_width;
        m_h_blocks = m_wnd.height / m_block_height;

        if(m_wnd.width % m_block_width)
            m_w_blocks++;
        if(m_wnd.height % m_block_height)
            m_h_blocks++;
    }
}

RGBMode DesktopSession::GetRGBMode() const
{
    switch(m_pixel_size)
    {
    case 1 :
        return BMP_RGB8_PALETTE;
    case 2 :
        return BMP_RGB16_555;
    case 3 :
        return BMP_RGB24;
    case 4 :
        return BMP_RGB32;
    default :
        return BMP_NONE;
    }
}

int DesktopSession::GetBitmapSize() const
{
    if(m_bytes_per_line)
        return m_bytes_per_line * GetHeight();
    return GetHeight() * GetWidth() * m_pixel_size + (GetHeight() * m_padding);
}

int DesktopSession::GetBytesPerLine() const
{
    if(m_bytes_per_line)
        return m_bytes_per_line;
    return GetWidth() * m_pixel_size + m_padding;
}

DesktopSession teamtalk::MakeDesktopSession(int width, int height, 
                                            RGBMode rgb_mode,
                                            int bytes_per_line/* = 0*/)
{
    if(bytes_per_line)
        return DesktopSession(DesktopWindow(0, width, height, rgb_mode,
                              DESKTOPPROTOCOL_NONE), bytes_per_line);
    return DesktopSession(DesktopWindow(0, width, height, rgb_mode, 
                          DESKTOPPROTOCOL_NONE));
}

size_t teamtalk::ConvertBitmap(const std::vector<char>& src_bitmap,
                               const DesktopSession& src_ses,
                               std::vector<char>& dst_bitmap, 
                               const DesktopSession& dst_ses)
{
    size_t rgbsrc_pos = 0, rgbdest_pos = 0;
    size_t rgbsrc_bytes_per_line = src_ses.GetBytesPerLine();
    size_t rgbdest_bytes_per_line = dst_ses.GetBytesPerLine();
    for(size_t h=0;h<(size_t)src_ses.GetHeight();h++)
    {
        rgbsrc_pos = rgbsrc_bytes_per_line * h;
        size_t rgbsrc_end = rgbsrc_pos + src_ses.GetWidthSize();
        rgbdest_pos = rgbdest_bytes_per_line * h;
        size_t rgbdest_end = rgbdest_pos + rgbdest_bytes_per_line;
        TTASSERT(rgbdest_end <= dst_bitmap.size());
        switch(src_ses.GetRGBMode())
        {
        case BMP_RGB8_PALETTE :
            switch(dst_ses.GetRGBMode())
            {
            case BMP_RGB8_PALETTE : //BMP_RGB8_PALETTE -> BMP_RGB8_PALETTE
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=1)
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i];
            break;
            case BMP_RGB16_555 : //BMP_RGB8_PALETTE -> BMP_RGB16_555
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=1)
                {
                    ACE_UINT8 rgb8 = src_bitmap[i];
                    unsigned char r = BMPPalette::Instance()->m_rgb8_palette[rgb8][0];
                    unsigned char g = BMPPalette::Instance()->m_rgb8_palette[rgb8][1];
                    unsigned char b = BMPPalette::Instance()->m_rgb8_palette[rgb8][2];

                    unsigned short rgb16;
                    rgb16  = (r / 8);
                    rgb16 |= (g / 8) << 5;
                    rgb16 |= (b / 8) << 10;

                    dst_bitmap[rgbdest_pos++] = rgb16 & 0xFF;
                    dst_bitmap[rgbdest_pos++] = (rgb16 >> 8);
                }
            break;
            case BMP_RGB24 : //BMP_RGB8_PALETTE -> BMP_RGB24
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=1)
                {
                    ACE_UINT8 rgb8 = src_bitmap[i];
                    dst_bitmap[rgbdest_pos++] = BMPPalette::Instance()->m_rgb8_palette[rgb8][0];
                    dst_bitmap[rgbdest_pos++] = BMPPalette::Instance()->m_rgb8_palette[rgb8][1];
                    dst_bitmap[rgbdest_pos++] = BMPPalette::Instance()->m_rgb8_palette[rgb8][2];
                }
            break;
            case BMP_RGB32 : //BMP_RGB8_PALETTE -> BMP_RGB32
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=1)
                {
                    ACE_UINT8 rgb8 = src_bitmap[i];
                    dst_bitmap[rgbdest_pos++] = BMPPalette::Instance()->m_rgb8_palette[rgb8][0];
                    dst_bitmap[rgbdest_pos++] = BMPPalette::Instance()->m_rgb8_palette[rgb8][1];
                    dst_bitmap[rgbdest_pos++] = BMPPalette::Instance()->m_rgb8_palette[rgb8][2];
                    dst_bitmap[rgbdest_pos++] = BMPPalette::Instance()->m_rgb8_palette[rgb8][3];
                }
            break;
            default :
                TTASSERT(0);
                return 0;
            }
        break; /* BMP_RGB8_PALETTE */
        case BMP_RGB16_555 :
            switch(dst_ses.GetRGBMode())
            {
            case BMP_RGB8_PALETTE : //RGB16 -> BMP_RGB8_PALETTE
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=2)
                {
                    unsigned short rgb = *reinterpret_cast<const unsigned short*>(&src_bitmap[i]);
                    dst_bitmap[rgbdest_pos++] = RGB8Palette((rgb & 0x1F) * 8,
                                                            ((rgb >> 5) & 0x1F) * 8,
                                                            ((rgb >> 10) & 0x1F) * 8);
                }
            break;
            case BMP_RGB16_555 : //RGB16 -> RGB16
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=2)
                {
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i];
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i+1];
                }
            break;
            case BMP_RGB24 : //RGB16 -> RGB24
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=2)
                {
                    unsigned short rgb = *reinterpret_cast<const unsigned short*>(&src_bitmap[i]);
                    dst_bitmap[rgbdest_pos++] = (rgb & 0x1F) * 8;
                    dst_bitmap[rgbdest_pos++] = ((rgb >> 5) & 0x1F) * 8;
                    dst_bitmap[rgbdest_pos++] = ((rgb >> 10) & 0x1F) * 8;
                }
            break;
            case BMP_RGB32 : //RGB16 -> RGB32
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=2)
                {
                    unsigned short rgb = *reinterpret_cast<const unsigned short*>(&src_bitmap[i]);
                    dst_bitmap[rgbdest_pos++] = (rgb & 0x1F) * 8;
                    dst_bitmap[rgbdest_pos++] = ((rgb >> 5) & 0x1F) * 8;
                    dst_bitmap[rgbdest_pos++] = ((rgb >> 10) & 0x1F) * 8;
                    dst_bitmap[rgbdest_pos++] = -1;
                }
            break;
            default :
                TTASSERT(0);
                return 0;
            }
        break; /* BMP_RGB16_555 */
        case BMP_RGB24 :
            switch(dst_ses.GetRGBMode())
            {
            case BMP_RGB8_PALETTE : //RGB24 -> BMP_RGB8_PALETTE
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=3)
                {
                    dst_bitmap[rgbdest_pos++] = RGB8Palette(src_bitmap[i],
                                                            src_bitmap[i+1],
                                                            src_bitmap[i+2]);
                }
            break;
            case BMP_RGB16_555 : //RGB24 -> RGB16
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=3)
                {
                    unsigned char r = src_bitmap[i];
                    unsigned char g = src_bitmap[i+1];
                    unsigned char b = src_bitmap[i+2];

                    unsigned short rgb16;
                    rgb16  = (r / 8);
                    rgb16 |= (g / 8) << 5;
                    rgb16 |= (b / 8) << 10;

                    dst_bitmap[rgbdest_pos++] = rgb16 & 0xFF;
                    dst_bitmap[rgbdest_pos++] = (rgb16 >> 8);
                }
            break;
            case BMP_RGB24 : //RGB24 -> RGB24
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=3)
                {
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i];
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i+1];
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i+2];
                }
            break;
            case BMP_RGB32 : //RGB24 -> RGB32
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=3)
                {
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i];
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i+1];
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i+2];
                    dst_bitmap[rgbdest_pos++] = -1;
                }
            break;
            default :
                TTASSERT(0);
                return 0;
            }
        break; /* BMP_RGB24 */
        case BMP_RGB32 :
            switch(dst_ses.GetRGBMode())
            {
            case BMP_RGB8_PALETTE : //RGB32 -> BMP_RGB8_PALETTE
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=4)
                {
                    dst_bitmap[rgbdest_pos++] = RGB8Palette(src_bitmap[i], 
                                                            src_bitmap[i+1],
                                                            src_bitmap[i+2]);
                }
            break;
            case BMP_RGB16_555 : //RGB32 -> RGB16
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=4)
                {
                    unsigned char r = src_bitmap[i];
                    unsigned char g = src_bitmap[i+1];
                    unsigned char b = src_bitmap[i+2];

                    unsigned short rgb16;
                    rgb16  = (r / 8);
                    rgb16 |= (g / 8) << 5;
                    rgb16 |= (b / 8) << 10;

                    dst_bitmap[rgbdest_pos++] = rgb16 & 0xFF;
                    dst_bitmap[rgbdest_pos++] = (rgb16 >> 8);
                }
            break;
            case BMP_RGB24 : //RGB32 -> RGB24
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=4)
                {
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i];
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i+1];
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i+2];
                }
            break;
            case BMP_RGB32 : //RGB32 -> RGB32
                for(size_t i=rgbsrc_pos;i<rgbsrc_end;i+=4)
                {
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i];
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i+1];
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i+2];
                    dst_bitmap[rgbdest_pos++] = src_bitmap[i+3];
                }
            break;
            default :
                TTASSERT(0);
                return 0;
            }
        break; /* BMP_RGB32 */
        }
        while(rgbdest_pos<rgbdest_end)
            dst_bitmap[rgbdest_pos++] = 0;
    }

    return rgbdest_pos;
}

//void rgb_to_hsv(unsigned r, unsigned g, unsigned b, double* h, double* s, double* v);
//void hsv_to_rgb(double hue, double s, double v, unsigned* r, unsigned* g, unsigned* b);

BMPPalette::BMPPalette()
{
    memset(m_rgb8_palette, 0xFF, sizeof(m_rgb8_palette));
    InitPalette();
}

BMPPalette* BMPPalette::Instance()
{
    static BMPPalette r;
    return &r;
}
//
//void BMPPalette::InitPalette0()
//{
//    unsigned r, g, b;
//    register double hue, sat, val;
//    int i, j;
//
//    m_rgb8_palette0[0][0] = 0;      /* program 16 gray shades */
//    m_rgb8_palette0[0][1] = 0;
//    m_rgb8_palette0[0][2] = 0;
//    m_rgb8_palette0[0][3] = -1;
//
//    for (i = 1; i <= 15; i++)
//    {
//        m_rgb8_palette0[i][0] = (ACE_UINT8)(64.0 / 15.0 * (double) i - 1);
//        m_rgb8_palette0[i][1] = (ACE_UINT8)(64.0 / 15.0 * (double) i - 1);
//        m_rgb8_palette0[i][2] = (ACE_UINT8)(64.0 / 15.0 * (double) i - 1);
//        m_rgb8_palette0[i][3] = -1;
//    } 
//    for (i = 1; i <= 15; i++)                /* program 248 color hues */
//    {
//        hue = 360.0 * ((double)(i)) / 15.0;   /* normalize to 360 */
//        sat = 1.0 ;
//        val = 1.0 ;
//        hsv_to_rgb (hue, sat, val, &r, &g, &b);
//        for (j = 15; j >= 0; j--) 
//        { 
//            val = (1.0 / 16.0) * (j + 1) ;
//            m_rgb8_palette0[i * 16 + j][0] = (ACE_UINT8)(r * val);
//            m_rgb8_palette0[i * 16 + j][1] = (ACE_UINT8)(g * val);
//            m_rgb8_palette0[i * 16 + j][2] = (ACE_UINT8)(b * val);
//            m_rgb8_palette0[i * 16 + j][3] = -1;
//        } 
//    } 
//}

static unsigned char COLOR_MATRIX[6] = {0x00, 0x33, 0x66, 0x99, 0xcc, 0xff};

void BMPPalette::InitPalette()
{
  unsigned int palette_idx, r_idx, g_idx, b_idx;

  unsigned char r,g,b;

  r = 0;
  g = 0;
  b = 0;

  palette_idx = 0;
  r_idx = 0;
  while(r_idx<6)
  {
      r = COLOR_MATRIX[r_idx];
      g_idx = 0;

      while(g_idx<6)
      {
          g=COLOR_MATRIX[g_idx];

          b_idx=0;
          while(b_idx<6)
          {
              b=COLOR_MATRIX[b_idx];

              m_rgb8_palette[palette_idx][0] = r;
              m_rgb8_palette[palette_idx][1] = g;
              m_rgb8_palette[palette_idx][2] = b;
              b_idx++;
              palette_idx++;

          }
          g_idx++;
      }
      r_idx++;

  }
  r=0xff;
  g=0xff;
  b=0xff;

  //palette_idx--;
  //fill out the rest with 0xff;
  while(palette_idx<256)
  {
      m_rgb8_palette[palette_idx][0] = r;
      m_rgb8_palette[palette_idx][1] = g;
      m_rgb8_palette[palette_idx][2] = b;

      palette_idx++;
  }
}


inline unsigned char cube_idx(unsigned char color);

inline int teamtalk::RGB8Palette(unsigned char r, unsigned char g, unsigned char b)
{
  return cube_idx(r)*36+cube_idx(g)*6+cube_idx(b);
}

inline unsigned char cube_idx(unsigned char color)
{
    int i = 0;
    unsigned char j = 0;
    int result;
    while(i<6){

        result=color-COLOR_MATRIX[i];
        if(result<0){

            result=-result;
        }
        if(result<=25){

            j=i;
            i=10;
        }
        i++;

    }
    return j;
}

//
//int teamtalk::RGB8Pal0(unsigned char r, unsigned char g, unsigned char b)
//{ 
//    int hue, val, color;
//    double h, s, v;
//    rgb_to_hsv((unsigned)r,(unsigned)g,(unsigned)b, &h, &s, &v);
//    hue = (int)floor(h / 24);
//    val = (int)ceil(v / 0.0625);
//    color = max(1, min(15, hue)) * 16 + max(0, min(15, val - 1));
//    /* map gray shades and ultra-brights highlights */ 
//    if (r == g && g == b || s < 0.15) 
//        color = max(0, min(15, val - 1));
//    return(color);
//} 
//
///**/ 
//void hsv_to_rgb(double hue, double s, double v, unsigned* r, unsigned* g, unsigned* b)
///* Conversion from Hue, Saturation, Value to Red, Green, and Blue */ 
//{ 
//    register double i, f, p1, p2, p3;
//    register double xh;
//    register double nr, ng, nb;  /* rgb values of 0.0 - 1.0      */ 
//    if (hue == 360.0) 
//        hue = 0.0;                    /* (THIS LOOKS BACKWARDS BUT OK) */ 
//    xh = hue / 60.0;                  /* convert hue to be in 0,6 */ 
//    i = floor(xh);                    /* i = greatest integer <= h */ 
//    f = xh - i;                       /* f = fractional part of h */ 
//    p1 = v * (1 - s);
//    p2 = v * (1 - (s * f));
//    p3 = v * (1 - (s * (1 - f)));
//    switch ((int) i) 
//    { 
//    case 0: 
//        nr = v;
//        ng = p3;
//        nb = p1;
//        break;
//    case 1: 
//        nr = p2;
//        ng = v;
//        nb = p1;
//        break;
//    case 2: 
//        nr = p1;
//        ng = v;
//        nb = p3;
//        break;
//    case 3: 
//        nr = p1;
//        ng = p2;
//        nb = v;
//        break;
//    case 4: 
//        nr = p3;
//        ng = p1;
//        nb = v;
//        break;
//    case 5: 
//        nr = v;
//        ng = p1;
//        nb = p2;
//        break;
//    } 
//    *r = (unsigned)(nr * 63.0);        /* Normalize the values to 63 */ 
//    *g = (unsigned)(ng * 63.0);
//    *b = (unsigned)(nb * 63.0);
//} 
//
///**/ 
//void rgb_to_hsv(unsigned r, unsigned g, unsigned b, double* h, double* s, double* v) 
///* Conversion from Red, Green, and Blue to Hue, Saturation, Value */ 
//{ 
//    register double m, r1, g1, b1;
//    register double nr, ng, nb;           /* rgb values of 0.0 - 1.0 */ 
//    register double nh, ns, nv;           /* hsv local values */ 
//    nr = (double) r / 255.0;
//    ng = (double) g / 255.0;
//    nb = (double) b / 255.0;
//    nv = max (nr, max (ng, nb));
//    m = min (nr, min (ng, nb));
//    if (nv != 0.0)                      /* if no value, it's black! */ 
//        ns = (nv - m) / nv;
//    else 
//        ns = 0.0;                      /* black = no color saturation */ 
//    if (ns == 0.0)                      /* hue undefined if no saturation */ 
//    { 
//        *h = 0.0;                      /* return black level (?) */ 
//        *s = 0.0;
//        *v = nv;
//        return;
//    } 
//    r1 = (nv - nr) / (nv - m);         /* distance of color from red   */ 
//    g1 = (nv - ng) / (nv - m);         /* distance of color from green */ 
//    b1 = (nv - nb) / (nv - m);         /* distance of color from blue */ 
//    if (nv == nr) 
//    { 
//        if (m == ng) 
//            nh = 5. + b1;
//        else 
//            nh = 1. - g1;
//    } 
//    if (nv == ng) 
//    { 
//        if (m == nb) 
//            nh = 1. + r1;
//        else 
//            nh = 3. - b1;
//    } 
//    if (nv == nb) 
//    { 
//        if (m == nr) 
//            nh = 3. + g1;
//        else 
//            nh = 5. - r1;
//    } 
//    *h = nh * 60.0;            /* return h converted to degrees */ 
//    *s = ns;
//    *v = nv;
//}
