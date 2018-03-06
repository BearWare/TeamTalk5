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

#ifndef CDIBITMAP_H
#define CDIBITMAP_H

class CDIBitmap
{
    friend class CBmpPalette;

    BITMAPINFO *    m_pInfo;
    BYTE *            m_pPixels;
    CBmpPalette *    m_pPal;
    BOOL            m_bIsPadded;

public:        //constructors
    CDIBitmap();
    virtual ~CDIBitmap();

private:
    CDIBitmap( const CDIBitmap& dbmp );

public:        // attributes
    BITMAPINFO *    GetHeaderPtr() const;
    BYTE *            GetPixelPtr() const;
    RGBQUAD *        GetColorTablePtr() const;
    int                GetWidth() const;
    int                GetHeight() const;
    CBmpPalette *    GetPalette() { return m_pPal; }

public:        // operations
    BOOL            CreatePalette();    // auto. made by "Load()" and "CreateFromBitmap()"
    void            ClearPalette();        // destroy the palette associated with this image
    BOOL            CreateFromBitmap( CDC *, CBitmap * );
    BOOL            LoadResource(LPCTSTR ID);
    BOOL            LoadResource(UINT ID) { return LoadResource(MAKEINTRESOURCE(ID)); }
    BOOL            LoadBitmap(UINT ID) { return LoadResource(ID); }
    BOOL            LoadBitmap(LPCTSTR ID) { return LoadResource(ID); }
    void            DestroyBitmap();
    BOOL            DeleteObject() { DestroyBitmap(); return TRUE; }

    BOOL            IsEmpty(void) const { return (m_pInfo == NULL); }

public:        // overridables
    // draw the bitmap at the specified location
    virtual void    DrawDIB( CDC * pDC, int x=0, int y=0 );

    // draw the bitmap and stretch/compress it to the desired size
    virtual void    DrawDIB( CDC * pDC, int x, int y, int width, int height );

    // draw parts of the dib into a given area of the DC
    virtual int        DrawDIB( CDC * pDC, CRect & rectDC, CRect & rectDIB );

    // load a bitmap from disk
    virtual BOOL    Load( CFile * pFile );
    virtual BOOL    Load( const CString & );

    // save the bitmap to disk
    virtual BOOL    Save( CFile * pFile );
    virtual BOOL    Save( const CString & );

protected:
    int                GetPalEntries() const;
    int                GetPalEntries( BITMAPINFOHEADER& infoHeader ) const;
    DWORD            GetBitsPerPixel() const;
    DWORD            LastByte( DWORD BitsPerPixel, DWORD PixelCount ) const;
    DWORD            GetBytesPerLine( DWORD BitsPerPixel, DWORD Width ) const;
    BOOL            PadBits();
    BOOL            UnPadBits();
    WORD            GetColorCount() const;
};


#endif
