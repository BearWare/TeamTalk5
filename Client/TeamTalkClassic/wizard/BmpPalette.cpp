#include "stdafx.h"
#include "DIBitmap.h"
#include "BmpPalette.h"

CBmpPalette::CBmpPalette( CDIBitmap* pBmp )
{
    ASSERT( pBmp );
    int cPaletteEntries = pBmp->GetPalEntries();
    int cPalette = sizeof(LOGPALETTE) +
        sizeof(PALETTEENTRY) * cPaletteEntries;
    // Since the LOGPALETTE structure is open-ended, you
    // must dynamically allocate it, rather than using one
    // off the stack.
    LOGPALETTE* pPal = (LOGPALETTE*)new BYTE[cPalette];
    RGBQUAD*    pColorTab = pBmp->GetColorTablePtr();
    pPal->palVersion = 0x300;
    pPal->palNumEntries = (unsigned short)cPaletteEntries;
    // Roll through the color table, and add each color to
    // the logical palette.
    for( int ndx = 0; ndx < cPaletteEntries; ndx++ )
    {
        pPal->palPalEntry[ndx].peRed   = pColorTab[ndx].rgbRed;
        pPal->palPalEntry[ndx].peGreen = pColorTab[ndx].rgbGreen;
        pPal->palPalEntry[ndx].peBlue  = pColorTab[ndx].rgbBlue;
        pPal->palPalEntry[ndx].peFlags = NULL;
    }
    VERIFY( CreatePalette( pPal ) );
    delete [] (BYTE*)pPal;
}
