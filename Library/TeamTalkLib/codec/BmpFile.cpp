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

#include "BmpFile.h"

#include "myace/MyACE.h"

#include <cassert>
#include <cstdint>
#include <vector>

#if defined(ACE_WIN32)
typedef BITMAPFILEHEADER BitmapFileHeader;
#else
struct BitmapFileHeader
{
    ACE_UINT16 bfType;
    ACE_UINT32 bfSize;
    ACE_UINT16 bfReserved1;
    ACE_UINT16 bfReserved2;
    ACE_UINT32 bfOffBits;
} __attribute__((packed));
#endif

#if defined(WIN32)
typedef BITMAPINFOHEADER BitmapInfoHeader;
#else
struct BitmapInfoHeader
{
    ACE_UINT32 biSize;
    ACE_INT32  biWidth;
    ACE_INT32  biHeight;
    ACE_UINT16 biPlanes;
    ACE_UINT16 biBitCount;
    ACE_UINT32 biCompression;
    ACE_UINT32 biSizeImage;
    ACE_INT32  biXPelsPerMeter;
    ACE_INT32  biYPelsPerMeter;
    ACE_UINT32 biClrUsed;
    ACE_UINT32 biClrImportant;
};
#endif

const int BMPHDR_SIZE = 14;
const int BMIHEADER_SIZE = 40;

bool WriteBitmap(const ACE_TString& filename, const media::VideoFormat fmt,
                 const char* data, int size)
{
    BitmapFileHeader bmphdr = {};

    assert(BMPHDR_SIZE == sizeof(bmphdr));

    bmphdr.bfType        = 0x4d42;   // 'BM' WINDOWS_BITMAP_SIGNATURE
    bmphdr.bfSize        = (((3 * fmt.width + 3) & ~3) * fmt.height)
                           + BMPHDR_SIZE + BMIHEADER_SIZE;
    bmphdr.bfReserved1    = bmphdr.bfReserved2 = 0;
    bmphdr.bfOffBits      = BMPHDR_SIZE + BMIHEADER_SIZE;

    BitmapInfoHeader bmiHeader = {};
    assert(BMIHEADER_SIZE == sizeof(bmiHeader));

    bmiHeader.biSize = BMIHEADER_SIZE;
    bmiHeader.biWidth = fmt.width;
    bmiHeader.biHeight = fmt.height;
    bmiHeader.biPlanes = 1;
    switch (fmt.fourcc)
    {
    case media::FOURCC_RGB24 :
        bmiHeader.biBitCount = 3 * 8;
        break;
    case media::FOURCC_RGB32 :
        bmiHeader.biBitCount = 4 * 8;
        break;
    default :
        return false;
    }
    bmiHeader.biCompression = 0; //BI_RGB;
    bmiHeader.biSizeImage = (ACE_UINT32)size;

    MyFile bmpfile;
    if (!bmpfile.NewFile(filename))
        return false;

    bmpfile.Write(reinterpret_cast<const char*>(&bmphdr), BMPHDR_SIZE);
    bmpfile.Write(reinterpret_cast<const char*>(&bmiHeader), BMIHEADER_SIZE);

    ssize_t const written = bmpfile.Write(data, size);
    return written == size;
}

std::vector<char> LoadRawBitmap(const ACE_TString& filename, media::VideoFormat& fmt)
{
    std::vector<char> buff;
    BitmapFileHeader bmphdr = {};
    BitmapInfoHeader bmiHeader = {};

    assert(BMPHDR_SIZE == sizeof(bmphdr));
    assert(BMIHEADER_SIZE == sizeof(bmiHeader));

    MyFile bmpfile;
    if (!bmpfile.Open(filename))
        return buff;

    if (bmpfile.Read(reinterpret_cast<char*>(&bmphdr), sizeof(BitmapFileHeader)) != sizeof(BitmapFileHeader))
        return buff;

    if (bmpfile.Read(reinterpret_cast<char*>(&bmiHeader), sizeof(bmiHeader)) != sizeof(bmiHeader))
        return buff;

    if(bmiHeader.biCompression != 0 /* BI_RGB */)
        return buff;

    switch (bmiHeader.biBitCount)
    {
    case 24 :
        fmt = media::VideoFormat(bmiHeader.biWidth, bmiHeader.biHeight, media::FOURCC_RGB24);
        break;
    case 32 :
        fmt = media::VideoFormat(bmiHeader.biWidth, bmiHeader.biHeight, media::FOURCC_RGB32);
        break;
    default :
        return buff;
    }

    if (!bmpfile.Seek(BMPHDR_SIZE + bmiHeader.biSize, std::ios_base::beg))
        return buff;

    auto startpos = bmpfile.Tell();
    if (bmiHeader.biSizeImage == 0)
    {
        if (bmpfile.Seek(0, std::ios_base::end)) {
            bmiHeader.biSizeImage = uint32_t(bmpfile.Tell());
            bmiHeader.biSizeImage -= uint32_t(startpos);
        }
    }
    bmpfile.Seek(startpos, std::ios_base::beg);

    buff.resize(bmiHeader.biSizeImage);
    bmpfile.Read(buff.data(), bmiHeader.biSizeImage);

    return buff;
}
