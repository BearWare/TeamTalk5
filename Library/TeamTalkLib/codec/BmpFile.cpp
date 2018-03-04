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

#include <ace/FILE_IO.h>
#include <ace/FILE_Connector.h>
#include <ace/ACE.h>
#include <assert.h>

struct BitmapFileHeader
{
    ACE_UINT16 bfType;
    ACE_UINT32 bfSize;
    ACE_UINT16 bfReserved1;
    ACE_UINT16 bfReserved2;
    ACE_UINT32 bfOffBits;
#ifdef ACE_WIN32
};
#else
} __attribute__((packed));
#endif

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

void WriteBitmap(const ACE_TString& filename, int w, int h, int pxl_size,
                 const char* data, int size)
{
    BitmapFileHeader bmphdr = {0};
    const int BMPHDR_SIZE = 14;
    const int BMIHEADER_SIZE = 40;

    assert(BMPHDR_SIZE == sizeof(bmphdr));

    bmphdr.bfType        = 0x4d42;   // 'BM' WINDOWS_BITMAP_SIGNATURE
    bmphdr.bfSize        = (((3 * w + 3) & ~3) * h)  
                           + BMPHDR_SIZE + BMIHEADER_SIZE;
    bmphdr.bfReserved1    = bmphdr.bfReserved2 = 0;
    bmphdr.bfOffBits      = BMPHDR_SIZE + BMIHEADER_SIZE;

    BitmapInfoHeader bmiHeader = {0};
    assert(BMIHEADER_SIZE == sizeof(bmiHeader));

    bmiHeader.biSize = BMIHEADER_SIZE;
    bmiHeader.biWidth = w;
    bmiHeader.biHeight = h;
    bmiHeader.biPlanes = 1;
    bmiHeader.biBitCount = pxl_size * 8;
    bmiHeader.biCompression = 0; //BI_RGB;
    bmiHeader.biSizeImage = (ACE_UINT32)size;

    ACE_FILE_Connector con;
    ACE_FILE_IO bmpfile;
    int ret = con.connect(bmpfile, ACE_FILE_Addr(filename.c_str()),
                          0, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT | O_TRUNC);

    bmpfile.send(&bmphdr, BMPHDR_SIZE);
    bmpfile.send(&bmiHeader, BMIHEADER_SIZE);

    bmpfile.send(data, size);
    bmpfile.close();
}
