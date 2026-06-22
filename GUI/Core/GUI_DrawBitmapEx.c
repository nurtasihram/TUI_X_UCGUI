

#include "GUI_Private.h"
#include "stdlib.h"

static void GL_DrawBitmapEx(const GUI_BITMAP GUI_UNI_PTR * pBitmap, int x0, int y0,
                            int xCenter, int yCenter, int xMag, int yMag) {
  RGB_COLOR Index, IndexPrev = 0;
  RGB_COLOR Color;
  int x, y, xi, yi, xSize, ySize, xAct, xStart, xMagAbs, xiMag, yMin, yMax, yEnd, yPrev, yStep;
  char Cached, HasTrans = 0;
  /* Use clipping rect to reduce calculation */
  yMin = GUI_Context.ClipRect.y0;
  yMax = GUI_Context.ClipRect.y1;
  /* Init some values */
  xSize    = pBitmap->XSize;
  ySize    = pBitmap->YSize;
  xMagAbs  = ((xMag < 0) ? -xMag : xMag);
  x0      -= (int32_t)((xMag < 0) ? xSize - xCenter - 1 : xCenter) * (int32_t)(xMagAbs) / (int32_t)(1000);
  yEnd     = y0 + GUI__DivideRound32(((int32_t)(-yCenter) * (int32_t)(yMag)), 1000);
  yPrev    = yEnd + 1;
  yStep = (yMag < 0) ? -1 : 1;
  if (pBitmap->pPal) {
    if (pBitmap->pPal->HasTrans) {
      HasTrans = 1;
    }
  }
  for (yi = 0; yi < ySize; yi++) {
    y = yEnd;
    yEnd = y0 + GUI__DivideRound32(((int32_t)(yi + 1 - yCenter) * (int32_t)(yMag)), 1000);
    if (y != yPrev) {
      yPrev = y;
      do {
        if ((y >= yMin) && (y <= yMax)) {
          xStart = -1;
          x      =  0;
          xiMag  =  0;
          Cached =  0;
          for (xi = 0; xi < xSize; xi++) {
            xiMag += xMagAbs;
            if (xiMag >= 1000) {
              xAct  = (xMag > 0) ? xi : xSize - xi - 1;
              Index = GUI_GetBitmapPixelIndex(pBitmap, xAct, yi);
              if (Index != IndexPrev || xStart == -1) {
                if ((Index == 0) && HasTrans) {
                  /* Transparent color ... clear cache */
                  if (Cached) {
                    LCD_DrawHLine(x0 + xStart, y, x0 + x - 1);
                    Cached = 0;
                  }
                } else {
                  /* Another color ... draw contents of cache */
                  if (Cached && xStart != -1) {
                    LCD_DrawHLine(x0 + xStart, y, x0 + x - 1);
                  }
                  xStart    = x;
                  Cached    = 1;
                  if (pBitmap->pMethods) {
                    Color = Index;
                  } else {
                    Color = pBitmap->pPal->pPalEntries[Index];
                  }
                  LCD_SetColorIndex(Color);
                }
                IndexPrev = Index;
              }
              do {
                x++;
                xiMag -= 1000;
              } while (xiMag >= 1000);
            }
          }
          /* Clear cache */
          if (Cached) {
            LCD_DrawHLine(x0 + xStart, y, x0 + x - 1);
          }
        }
        y += yStep;
      } while ((yMag < 0) ? (y > yEnd) : (y < yEnd));
    }
  }
}

void GUI_DrawBitmapEx(const GUI_BITMAP GUI_UNI_PTR * pBitmap, int x0, int y0,
                      int xCenter, int yCenter, int xMag, int yMag) {
  RGB_COLOR OldColor;
#if (GUI_WINSUPPORT)
    GUI_RECT r;
#endif

  OldColor = GUI_GetColor();
#if (GUI_WINSUPPORT)
    WM_ADDORG(x0, y0);
    if (xMag >= 0) {
      r.x0 = x0 + GUI__DivideRound32(((int32_t)(-xCenter) * (int32_t)(xMag)), 1000);
      r.x1 = x0 + GUI__DivideRound32(((int32_t)(pBitmap->XSize - xCenter - 1) * (int32_t)(xMag)), 1000);
    } else {
      r.x1 = x0 + GUI__DivideRound32(((int32_t)(-xCenter) * (int32_t)(xMag)), 1000);
      r.x0 = x0 + GUI__DivideRound32(((int32_t)(pBitmap->XSize - xCenter - 1) * (int32_t)(xMag)), 1000);
    }
    if (yMag >= 0) {
      r.y0 = y0 + GUI__DivideRound32(((int32_t)(-yCenter) * (int32_t)(yMag)), 1000);
      r.y1 = y0 + GUI__DivideRound32(((int32_t)(pBitmap->YSize - yCenter - 1) * (int32_t)(yMag)), 1000);
    } else {
      r.y1 = y0 + GUI__DivideRound32(((int32_t)(-yCenter) * (int32_t)(yMag)), 1000);
      r.y0 = y0 + GUI__DivideRound32(((int32_t)(pBitmap->YSize - yCenter - 1) * (int32_t)(yMag)), 1000);
    }
    WM_ITERATE_START(&r) {
#endif
  GL_DrawBitmapEx(pBitmap, x0, y0, xCenter, yCenter, xMag, yMag);
#if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
#endif
  GUI_SetColor(OldColor);

}


