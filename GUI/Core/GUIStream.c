

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI.h"

void GUI_DrawStreamedBitmap(const GUI_BITMAP_STREAM *pBitmapHeader, int x, int y) {
  GUI_BITMAP Bitmap = {0};
  GUI_LOGPALETTE Palette;
  Bitmap.BitsPerPixel = pBitmapHeader->BitsPerPixel;
  Bitmap.BytesPerLine = pBitmapHeader->BytesPerLine;
  Bitmap.pData        = ((const U8*)pBitmapHeader)+16+4*pBitmapHeader->NumColors;
  Bitmap.pPal         = &Palette;
  Bitmap.XSize        = pBitmapHeader->XSize;
  Bitmap.YSize        = pBitmapHeader->YSize;
  Palette.HasTrans    = pBitmapHeader->HasTrans;
  Palette.NumEntries  = pBitmapHeader->NumColors;
  Palette.pPalEntries = (const LCD_COLOR*)((const U8*)pBitmapHeader+16);
  GUI_DrawBitmap(&Bitmap, x, y);
}

/*************************** End of file ****************************/
