

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Private.h"

#if GUI_SUPPORT_CURSOR

static GUI_HMEM          _hBuffer;
static GUI_RECT          _Rect;
static char              _CursorIsVis;        /* Currently visible ? */
static char              _CursorOn;
static const GUI_CURSOR  * _pCursor;
static uint8_t                _CursorDeActCnt;
static int               _AllocSize;
static int               _x, _y;              /* Position of hot spot */
static GUI_RECT          _ClipRect;
static RGB_COLOR    _ColorIndex[4];      /* Color-Cache */

/*********************************************************************
*
*       _SetPixelIndex
*
* Purpose
*   Sets the pixel index for the Cursor.
*   Note the following:
*   - We do the clipping in this routine
*   - We do NOT call the driver directly, but thru its API table.
*     This allows others (e.g. the VNC server) to be in the loop-
*/
static void _SetPixelIndex(int x, int y, int Index) {
  if ((y >= _ClipRect.y0) && (y <= _ClipRect.y1)) {
    if ((x >= _ClipRect.x0) && (x <= _ClipRect.x1)) {
      LCD_aAPI[0]->pfSetPixelIndex(x, y, Index);
    }
  }
}

/*********************************************************************
*
*       _GetPixelIndex
*
* Purpose
*   Gets a pixel index for the Cursor.
*/
static int _GetPixelIndex(int x, int y) {
  if ((y >= _ClipRect.y0) && (y <= _ClipRect.y1)) {
    if ((x >= _ClipRect.x0) && (x <= _ClipRect.x1)) {
      return LCD_L0_GetPixel(x, y);
    }
  }
  return 0;
}

/*********************************************************************
*
*       _Undraw
*
* Purpose
*   Remove the cursors
*/
static void _Undraw(void) {
  int x, y, xSize, ySize;
  RGB_COLOR* pData;
  /* Save bitmap data */

  if (_hBuffer) {
    pData = (RGB_COLOR*)_hBuffer;
    xSize = _Rect.x1 - _Rect.x0 + 1;
    ySize = _Rect.y1 - _Rect.y0 + 1;
    for (y = 0; y < ySize; y++) {
      for (x = 0; x < xSize; x++) {
        _SetPixelIndex(x + _Rect.x0, y + _Rect.y0, *(pData + x));
      }
      pData += _pCursor->pBitmap->XSize;
    }
  }

}

static int _Log2Phys(int Index) {
  if (Index < 4) {
    return _ColorIndex[Index];
  } else {
    RGB_COLOR Color = *(_pCursor->pBitmap->pPal->pPalEntries + Index);
    return Color;
  }
}

static void _Draw(void) {
  int x, y, xSize, ySize;
  RGB_COLOR* pData;
  const GUI_BITMAP  * pBM;

  if (_hBuffer) {
    /* Save bitmap data */
    pBM = _pCursor->pBitmap;
    pData = (RGB_COLOR*)_hBuffer;
    xSize = _Rect.x1 - _Rect.x0 + 1;
    ySize = _Rect.y1 - _Rect.y0 + 1;
    for (y = 0; y < ySize; y++) {
      for (x = 0; x < xSize; x++) {
        int BitmapPixel;
        *(pData + x) = _GetPixelIndex(_Rect.x0 + x, _Rect.y0 + y);
        BitmapPixel = GUI_GetBitmapPixelIndex(pBM, x, y);
        if (BitmapPixel) {
          _SetPixelIndex(_Rect.x0 + x, _Rect.y0 + y, _Log2Phys(BitmapPixel));
        }
      }
      pData += pBM->XSize;
    }
  }

}

static void _CalcRect(void) {
  if (_pCursor) {
    _Rect.x0 = _x - _pCursor->xHot;
    _Rect.y0 = _y - _pCursor->yHot;
    _Rect.x1 = _Rect.x0 + _pCursor->pBitmap->XSize - 1;
    _Rect.y1 = _Rect.y0 + _pCursor->pBitmap->YSize - 1;
  }
}

static void _Hide(void) {
  if (_CursorIsVis) {
    _Undraw();
    _CursorIsVis = 0;
  }
}

static void _Show(void) {
  if (_CursorOn && (_CursorDeActCnt==0)) {
    _CursorIsVis = 1;
    _Draw();
  }
}

/*********************************************************************
*
*       _TempHide
*
* Purpose:
*   Hide cursor if a part of the given rectangle is located in the
*   rectangle used for the cursor. This routine is called automatically
*   by the window manager. This way the window manager can
*   automatically make sure that the cursor is always displayed
*   correctly.
*
* Params:
*   pRect   Rectangle under consideration
*
* Return value:
*   0:      No action taken
*           Cursor was not visible or not affected because rectangles
*           did not overlap
*   1:      Cursor hidden -> WM needs to restore cursor after
*           drawing operation
*/
static char _TempHide(const GUI_RECT* pRect) {
  if (!_CursorIsVis) {
    return 0;             /* Cursor not visible -> nothing to do */
  }
  if ((pRect == NULL) || GUI_RectsIntersect(pRect, &_Rect)) {
    _Hide();              /* Cursor needs to be hidden */
    return 1;
  }
  return 0;               /* Cursor not affected -> nothing to do */
}

static void _TempUnhide(void) {
  _Show();
}

void GUI_CURSOR_Activate(void) {

  if ((--_CursorDeActCnt) ==0) {
    _Show();
  }

}

void GUI_CURSOR_Deactivate(void) {

  if (_CursorDeActCnt++ ==0)
    _Hide();

}

const GUI_CURSOR  * GUI_CURSOR_Select(const GUI_CURSOR  * pCursor) {
  int AllocSize;
  const GUI_BITMAP  * pBM;
  const GUI_CURSOR  * pOldCursor;

  pOldCursor = _pCursor;
  if (pCursor != _pCursor) {
    int i;
    pBM = pCursor->pBitmap;
    i = pBM->pPal->NumEntries > 4 ? 4 : pBM->pPal->NumEntries;
    while (i--) {
      RGB_COLOR Color = *(pBM->pPal->pPalEntries + i);
      _ColorIndex[i] = Color;
    }
    _Hide();
    AllocSize = pBM->XSize * pBM->YSize * sizeof(RGB_COLOR);
    if (AllocSize != _AllocSize) {
      GUI_ALLOC_Free(_hBuffer);
      _hBuffer = 0;
    }
    _hBuffer = GUI_ALLOC_AllocZero(AllocSize);
    _CursorOn = 1;
    _pCursor = pCursor;
    _CalcRect();
    _Show();
  }

  return pOldCursor;
}

void GUI_CURSOR_Hide(void) {

  _Hide();
  _CursorOn = 0;
  /* Set function pointer which window manager can use */
  GUI_CURSOR_pfTempHide   = NULL;
  GUI_CURSOR_pfTempUnhide = NULL;

}

void GUI_CURSOR_Show(void) {

  LCDDEV_L0_GetRect(&_ClipRect);
  _Hide();
  _CursorOn = 1;
  /* Set function pointer which window manager can use */
  GUI_CURSOR_pfTempHide   = _TempHide;
  GUI_CURSOR_pfTempUnhide = _TempUnhide;
  if (!_pCursor) {
    GUI_CURSOR_Select(GUI_DEFAULT_CURSOR);
  } else {
    _Show();
  }

}

void GUI_CURSOR_SetPosition(int xNewPos, int yNewPos) {
  int x, xStart, xStep, xEnd, xOff, xOverlapMin, xOverlapMax;
  int y, yStart, yStep, yEnd, yOff, yOverlapMin, yOverlapMax;
  int xSize;
  RGB_COLOR* pData;

  if (_hBuffer) {
    if ((_x != xNewPos) | (_y != yNewPos)) {
      if (_CursorOn) {
        const GUI_BITMAP  * pBM = _pCursor->pBitmap;
        /* Save & set clip rect */
        /* Compute helper variables */
        pData = (RGB_COLOR*)_hBuffer;
        xSize = _pCursor->pBitmap->XSize;
        xOff = xNewPos - _x;
        if (xOff > 0) {
          xStep  = 1;
          xStart = 0;
          xEnd   = _pCursor->pBitmap->XSize;
          xOverlapMax = xEnd -1;
          xOverlapMin = xOff;
        } else {
          xStep  = -1;
          xStart = xSize - 1;
          xEnd   = -1;
          xOverlapMin = 0;
          xOverlapMax = xStart + xOff;
        }
        yOff = yNewPos - _y;
        if (yOff > 0) {
          yStep  = 1;
          yStart = 0;
          yEnd   = _pCursor->pBitmap->YSize;
          yOverlapMax = yEnd -1;
          yOverlapMin = yOff;
        } else {
          yStep  = -1;
          yStart = _pCursor->pBitmap->YSize - 1;
          yEnd   = -1;
          yOverlapMin = 0;
          yOverlapMax = yStart + yOff;
        }
        /* Restore & Draw */
        for (y = yStart; y != yEnd; y += yStep) {
          char yOverlaps;
          char yNewOverlaps;
          int yNew = y + yOff;
          yOverlaps    = (y >= yOverlapMin) && (y <= yOverlapMax);
          yNewOverlaps = (yNew >= yOverlapMin) && (yNew <= yOverlapMax);
          for (x= xStart; x != xEnd; x += xStep) {
            char xyOverlaps, xyNewOverlaps;
            int BitmapPixel;
            RGB_COLOR Pixel;
            RGB_COLOR* pSave = pData + x + y * xSize;
            int xNew = x + xOff;
            BitmapPixel = GUI_GetBitmapPixelIndex(pBM, x, y);
            xyOverlaps    = (x    >= xOverlapMin) && (x    <= xOverlapMax) && yOverlaps;
            xyNewOverlaps = (xNew >= xOverlapMin) && (xNew <= xOverlapMax) && yNewOverlaps;
            /* Restore old pixel if it was not transparent */
            if (BitmapPixel) {
              if (!xyOverlaps || (GUI_GetBitmapPixelIndex(pBM, x - xOff, y - yOff) == 0)) {
                _SetPixelIndex(x + _Rect.x0, y + _Rect.y0, *(pSave));
              }
            }
            /* Save */
            if (xyNewOverlaps) {
              Pixel = *(pData + xNew + yNew * xSize);
            } else {
              Pixel = _GetPixelIndex(_Rect.x0 + xNew, _Rect.y0 + yNew);
            }
            *pSave = Pixel;
            /* Write new  ... We could write pixel by pixel here */
            if (BitmapPixel) {
              RGB_COLOR NewPixel = _Log2Phys(BitmapPixel);
              _SetPixelIndex(_Rect.x0 + xNew, _Rect.y0 + yNew, NewPixel);
            }
          }
        }
      }
      _x = xNewPos;
      _y = yNewPos;
      _CalcRect();
    }
  }

}

#else


#endif   /* GUI_SUPPORT_CURSOR */


