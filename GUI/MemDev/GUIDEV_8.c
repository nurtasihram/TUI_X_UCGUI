#include <string.h>

#include "GUI_Private.h"
#include "GUIDebug.h"
#if GUI_WINSUPPORT
#include "WM.h"
#endif

/* Memory device capabilities are compiled only if support for them is enabled.*/
#if GUI_SUPPORT_MEMDEV

#ifndef PIXELINDEX
#define PIXELINDEX                      uint8_t
#define BITSPERPIXEL                     8
#define API_LIST      GUI_MEMDEV__APIList8
#endif

/*********************************************************************
*
*       ID translation table
*
* This table serves as translation table for DDBs
*/
static const RGB_COLOR aID[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

static PIXELINDEX* _XY2PTR(int x, int y) {
  GUI_MEMDEV* pDev = (GUI_Context.hDevData);
  uint8_t *pData = (uint8_t*)(pDev + 1);
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
    if ((x >= pDev->x0+pDev->XSize) | (x<pDev->x0) | (y >= pDev->y0+pDev->YSize) | (y<pDev->y0)) {
    }
#endif
  pData += (GUI_ALLOC_DATATYPE_U)(y - pDev->y0) * (GUI_ALLOC_DATATYPE_U)pDev->BytesPerLine;
  return ((PIXELINDEX*)pData) + x - pDev->x0;
}

static void _DrawBitLine1BPP(GUI_USAGE* pUsage, int x, int y, const uint8_t GUI_UNI_PTR * p, int Diff, unsigned int xsize,
                             const RGB_COLOR* pTrans, GUI_MEMDEV* pDev, PIXELINDEX* pDest)
{
  PIXELINDEX Index1;
  unsigned pixels;
  unsigned PixelCnt;
  PixelCnt = 8 - Diff;
  pixels = LCD_aMirror[*p] >> Diff;

  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:    /* Write mode */
    do {
      /* Prepare loop */
      if (PixelCnt > xsize) {
        PixelCnt = xsize;
      }
      xsize -= PixelCnt;
      /* Write as many pixels as we are allowed to and have loaded in this inner loop */
      do {
        *pDest++ = *(pTrans + (pixels & 1));
        pixels >>= 1;
      } while (--PixelCnt);
      /* Check if an other Source byte needs to be loaded */
      if (xsize == 0) {
        return;
      }
      PixelCnt = 8;
      pixels = LCD_aMirror[*++p];
    } while (1);
  case LCD_DRAWMODE_TRANS:
    Index1 = *(pTrans + 1);
    do {
      /* Prepare loop */
      if (PixelCnt > xsize) {
        PixelCnt = xsize;
      }
      xsize -= PixelCnt;
      if (pUsage) {
        do {
          if (pixels == 0) {      /* Early out optimization; not required */
            pDest += PixelCnt;
            x     += PixelCnt;
            break;
          }
          if ((pixels & 1)) {
            GUI_USAGE_AddPixel(pUsage, x, y);
            *pDest = Index1;
          }
          x++;
          pDest++;
          if (--PixelCnt == 0) {
            break;
          }
          pixels >>= 1;
        } while (1);
      } else {
        do {
          if (pixels == 0) {      /* Early out optimization; not required */
            pDest += PixelCnt;
            break;
          }
          if ((pixels & 1)) {
            *pDest = Index1;
          }
          pDest++;
          if (--PixelCnt == 0) {
            break;
          }
          pixels >>= 1;
        } while (1);
      }
      /* Check if an other Source byte needs to be loaded */
      if (xsize == 0) {
        return;
      }
      PixelCnt = 8;
      pixels = LCD_aMirror[*(++p)];
    } while (1);
  case LCD_DRAWMODE_XOR:
    do {
      /* Prepare loop */
      if (PixelCnt > xsize) {
        PixelCnt = xsize;
      }
      xsize -= PixelCnt;
      /* Write as many pixels as we are allowed to and have loaded in this inner loop */
      do {
        if ((pixels & 1)) {
          *pDest ^= ~0;
        }
        *pDest++;
        pixels >>= 1;
      } while (--PixelCnt);
      /* Check if an other Source byte needs to be loaded */
      if (xsize == 0) {
        return;
      }
      PixelCnt = 8;
      pixels = LCD_aMirror[*(++p)];
    } while (1);
  }
}

static void _DrawBitLine2BPP(GUI_USAGE* pUsage, int x, int y, const uint8_t GUI_UNI_PTR * p, int Diff, int xsize,
                             const RGB_COLOR* pTrans, PIXELINDEX* pDest)
{
  uint8_t pixels;
  uint8_t  PixelCnt;
  PixelCnt = 4 - Diff;
  pixels = (*p) << (Diff << 1);
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:    /* Write mode */
  PixelLoopWrite:
    if (PixelCnt > xsize) {
      PixelCnt = xsize;
    }
    xsize -= PixelCnt;
    do {
      *pDest++ = *(pTrans + (pixels >> 6));
      pixels <<= 2;
    } while (--PixelCnt);
    if (xsize) {
      PixelCnt = 4;
      pixels = *(++p);
      goto PixelLoopWrite;
    }
    break;
  case LCD_DRAWMODE_TRANS:
  PixelLoopTrans:
    if (PixelCnt > xsize)
      PixelCnt = xsize;
    xsize -= PixelCnt;
    do {
      if (pixels & 0xc0) {
        *pDest = *(pTrans + (pixels >> 6));
        if (pUsage) {
          GUI_USAGE_AddPixel(pUsage, x, y);
        }
      }
      pDest++;
      x++;
      pixels <<= 2;
    } while (--PixelCnt);
    if (xsize) {
      PixelCnt = 4;
      pixels = *(++p);
      goto PixelLoopTrans;
    }
    break;
  case LCD_DRAWMODE_XOR:;
  PixelLoopXor:
    if (PixelCnt > xsize)
      PixelCnt = xsize;
    xsize -= PixelCnt;
    do {
      if ((pixels & 0xc0))
        *pDest ^= 255;
      pDest++;
      pixels <<= 2;
    } while (--PixelCnt);
    if (xsize) {
      PixelCnt = 4;
      pixels = *(++p);
      goto PixelLoopXor;
    }
    break;
  }
}

static void _DrawBitLine4BPP(GUI_USAGE* pUsage, int x, int y, const uint8_t GUI_UNI_PTR * p, int Diff, int xsize,
                             const RGB_COLOR* pTrans, PIXELINDEX* pDest)
{
  uint8_t pixels;
  uint8_t PixelCnt;
  PixelCnt = 2 - Diff;
  pixels = (*p) << (Diff << 2);
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
/*
          * Write mode *
*/
  case 0:
    /* Draw incomplete bytes to the left of center area */
    if (Diff) {
      *pDest = *(pTrans + (pixels >> 4));
      pDest++;
      xsize--;
      pixels = *++p;
    }
    /* Draw center area (2 pixels in one byte) */
    if (xsize >= 2) {
      int i = xsize >> 1;
      xsize &= 1;
      do {
        *pDest     = *(pTrans + (pixels >> 4));   /* Draw 1. (left) pixel */
        *(pDest+1) = *(pTrans + (pixels & 15));   /* Draw 2. (right) pixel */
        pDest += 2;
        pixels = *++p;
      } while (--i);
    }
    /* Draw incomplete bytes to the right of center area */
    if (xsize) {
      *pDest = * (pTrans + (pixels >> 4));
    }
    break;
/*
          * Transparent draw mode *
*/
  case LCD_DRAWMODE_TRANS:
    /* Draw incomplete bytes to the left of center area */
    if (Diff) {
      if (pixels & 0xF0) {
        *pDest = *(pTrans + (pixels >> 4));
        if (pUsage) {
          GUI_USAGE_AddPixel(pUsage, x, y);
        }
      }
      pDest++;
      x++;
      xsize--;
      pixels = *++p;
    }
    /* Draw center area (2 pixels in one byte) */
    while (xsize >= 2) {
      /* Draw 1. (left) pixel */
      if (pixels & 0xF0) {
        *pDest = *(pTrans + (pixels >> 4));
        if (pUsage) {
          GUI_USAGE_AddPixel(pUsage, x, y);
        }
      }
      /* Draw 2. (right) pixel */
      if (pixels &= 15) {
        *(pDest + 1) = *(pTrans + pixels);
        if (pUsage) {
          GUI_USAGE_AddPixel(pUsage, x + 1, y);
        }
      }
      pDest += 2;
      x += 2;
      xsize -= 2;
      pixels = *++p;
    }
    /* Draw incomplete bytes to the right of center area */
    if (xsize) {
      if (pixels >>= 4) {
        *pDest = *(pTrans + pixels);
        if (pUsage) {
          GUI_USAGE_AddPixel(pUsage, x, y);
        }
      }
    }
    break;
  case LCD_DRAWMODE_XOR:;
  PixelLoopXor:
    if (PixelCnt > xsize)
      PixelCnt = xsize;
    xsize -= PixelCnt;
    do {
      if ((pixels & 0xc0)) {
        *pDest ^= 255;
      }
      pDest++;
      pixels <<= 4;
    } while (--PixelCnt);
    if (xsize) {
      PixelCnt = 2;
      pixels = *(++p);
      goto PixelLoopXor;
    }
    break;
  }
}

static void _DrawBitLine8BPP(GUI_USAGE* pUsage, int x, int y, const uint8_t GUI_UNI_PTR * pSrc, int xsize,
                             const RGB_COLOR* pTrans, PIXELINDEX* pDest) {
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:    /* Write mode */
    do {
      *pDest = *(pTrans + *pSrc);
      pDest++;
      pSrc++;
    } while (--xsize);
    break;
  case LCD_DRAWMODE_TRANS:
    do {
      if (*pSrc) {
        *pDest = *(pTrans + *pSrc);
        if (pUsage) {
          GUI_USAGE_AddPixel(pUsage, x, y);
        }
      }
      x++;
      pDest++;
      pSrc++;
    } while (--xsize);
    break;
  }
}

#if GUI_UNI_PTR_USED

static void _Memcopy(PIXELINDEX * pDest, const uint8_t GUI_UNI_PTR * pSrc, int NumBytes) {
  while(NumBytes--) {
    *pDest++ = *pSrc++;
  }
}
#endif

static void _DrawBitLine8BPP_DDB(GUI_USAGE* pUsage, int x, int y, const uint8_t GUI_UNI_PTR * pSrc, int xsize, PIXELINDEX* pDest) {
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:    /* Write mode */
#if GUI_UNI_PTR_USED
      _Memcopy(pDest, pSrc, xsize);
#else
      memcpy(pDest, pSrc, xsize);
#endif
    break;
  case LCD_DRAWMODE_TRANS:
    do {
      if (*pSrc) {
        *pDest = *pSrc;
        if (pUsage) {
          GUI_USAGE_AddPixel(pUsage, x, y);
        }
      }
      x++;
      pDest++;
      pSrc++;
    } while (--xsize);
    break;
  }
}

static void _DrawBitmap(int x0, int y0, int xsize, int ysize,
                        int BitsPerPixel, int BytesPerLine,
                        const uint8_t GUI_UNI_PTR * pData, int Diff, const RGB_COLOR* pTrans)
{
  int i;
  GUI_MEMDEV* pDev   = (GUI_Context.hDevData);
  GUI_USAGE*  pUsage = (pDev->hUsage) ? (pDev->hUsage) : 0;
  unsigned    BytesPerLineDest;
  PIXELINDEX* pDest;
  BytesPerLineDest = pDev->BytesPerLine;
  x0 += Diff;
  /* Mark all affected pixels dirty unless transparency is set */
  if (pUsage) {
    if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS) == 0) {
      GUI_USAGE_AddRect(pUsage, x0, y0 , xsize, ysize);
    }
  }
  pDest = _XY2PTR(x0, y0);
#if BITSPERPIXEL == 16
  /* handle 16 bpp bitmaps in high color modes, but only without palette */
  if (BitsPerPixel == 16) {
    for (i = 0; i < ysize; i++) {
      _DrawBitLine16BPP_DDB(pUsage, x0, i + y0, (const uint16_t*)pData, xsize, pDest);
      pData += BytesPerLine;
      pDest = (PIXELINDEX*)((uint8_t*)pDest + BytesPerLineDest);
    }
    return;
  }
#endif
  /* Handle 8 bpp bitmaps seperately as we have different routine bitmaps with or without palette */
  if (BitsPerPixel == 8) {
    for (i = 0; i < ysize; i++) {
      if (pTrans) {
        _DrawBitLine8BPP(pUsage, x0, i + y0, pData, xsize, pTrans, pDest);
      } else {
        _DrawBitLine8BPP_DDB(pUsage, x0, i + y0, pData, xsize, pDest);
      }
      pData += BytesPerLine;
      pDest = (PIXELINDEX*)((uint8_t*)pDest + BytesPerLineDest);
    }
    return;
  }
  /* Use aID for bitmaps without palette */
  if (!pTrans) {
    pTrans = aID;
  }
  for (i = 0; i < ysize; i++) {
    switch (BitsPerPixel) {
    case 1:
      _DrawBitLine1BPP(pUsage, x0, i + y0, pData, Diff, xsize, pTrans, pDev, pDest);
      break;
    case 2:
      _DrawBitLine2BPP(pUsage, x0, i + y0, pData, Diff, xsize, pTrans, pDest);
      break;
    case 4:
      _DrawBitLine4BPP(pUsage, x0, i + y0, pData, Diff, xsize, pTrans, pDest);
      break;
    }
    pData += BytesPerLine;
    pDest = (PIXELINDEX*)((uint8_t*)pDest + BytesPerLineDest);
  }
}

static void _FillRect(int x0, int y0, int x1, int y1) {
  unsigned BytesPerLine;
  int Len;
  GUI_MEMDEV* pDev   = (GUI_Context.hDevData);
  PIXELINDEX* pData  = _XY2PTR(x0, y0);
  BytesPerLine = pDev->BytesPerLine;
  Len = x1 - x0 + 1;
  /* Mark rectangle as modified */
  if (pDev->hUsage) {
    GUI_USAGE_AddRect((pDev->hUsage), x0, y0, Len, y1 - y0 + 1);
  }
  /* Do the drawing */
  for (; y0 <= y1; y0++) {
    if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
      int RemPixels;
      PIXELINDEX* pDest;
      RemPixels = Len;
      pDest  = pData;
      do {
        *pDest = *pDest ^ ~0;
        pDest++;
      } while (--RemPixels);
    } else {  /* Fill */
#if BITSPERPIXEL == 8
        GUI_MEMSET(pData, LCD_COLORINDEX, Len);
#elif BITSPERPIXEL == 16
        GUI__memset16(pData, LCD_COLORINDEX, Len);
#else
#error Unsupported
#endif
    }
    pData = (PIXELINDEX*)((uint8_t*)pData + BytesPerLine);
  }
}

static void _DrawHLine(int x0, int y, int x1) {
  _FillRect(x0, y, x1, y);
}

static void _DrawVLine(int x , int y0, int y1) {
  GUI_MEMDEV* pDev   = (GUI_Context.hDevData);
  GUI_USAGE_h hUsage = pDev->hUsage;
  GUI_USAGE*  pUsage = hUsage ? (hUsage) : NULL;
  PIXELINDEX* pData  = _XY2PTR(x, y0);
  unsigned BytesPerLine = pDev->BytesPerLine;

  if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
    do {
      if (hUsage) {
        GUI_USAGE_AddPixel(pUsage, x, y0);
      }
      *pData = *pData ^ ~0;
      pData = (PIXELINDEX*)((uint8_t*)pData + pDev->BytesPerLine); /* Same as "pData += pDev->BytesPerLine >> 1;", Just more efficient */
    } while (++y0 <= y1);
  } else {
    if (hUsage) {
      do {
        GUI_USAGE_AddPixel(pUsage, x, y0);
        *pData = LCD_COLORINDEX;
        pData = (PIXELINDEX*)((uint8_t*)pData + BytesPerLine); /* Same as "pData += pDev->BytesPerLine >> 1;", Just more efficient */
      } while (++y0 <= y1);
    } else {
      unsigned NumPixels;
      NumPixels = y1 - y0 + 1;
      do {
        *pData = LCD_COLORINDEX;
        pData = (PIXELINDEX*)((uint8_t*)pData + BytesPerLine); /* Same as "pData += pDev->BytesPerLine >> 1;", Just more efficient */
      } while (--NumPixels);
    }
  }
}

static void _SetPixelIndex(int x, int y, int Index) {
  GUI_MEMDEV* pDev = (GUI_Context.hDevData);
  GUI_USAGE_h hUsage = pDev->hUsage;
  PIXELINDEX* pData = _XY2PTR(x, y);
  *pData = Index;
  if (hUsage) {
    GUI_USAGE_AddPixel(((GUI_USAGE *)hUsage), x, y);
  }
}

static void _XorPixel(int x, int y) {
  GUI_MEMDEV* pDev = (GUI_Context.hDevData);
  GUI_USAGE_h hUsage = pDev->hUsage;
  PIXELINDEX* pData = _XY2PTR(x, y);
  *pData = *pData ^ ~0;
  if (hUsage) {
    GUI_USAGE_AddPixel(((GUI_USAGE *)hUsage), x, y);
  }
}

static unsigned int _GetPixelIndex(int x, int y) {
  PIXELINDEX* pData = _XY2PTR(x, y);
  return *pData;
}

const tLCDDEV_APIList API_LIST = {
  (tLCDDEV_DrawBitmap*)_DrawBitmap,
  _DrawHLine,
  _DrawVLine,
  _FillRect,
  _GetPixelIndex,
  GUI_MEMDEV__GetRect,
  _SetPixelIndex,
  _XorPixel,
  NULL,               /* pfFillPolygon   */
  NULL,               /* MemDevAPI       */
  BITSPERPIXEL        /* BitsPerPixel    */
};

#else


#endif /* GUI_SUPPORT_MEMDEV */

