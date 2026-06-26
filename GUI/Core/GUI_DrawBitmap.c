

#include "GUI_Private.h"

/*********************************************************************
*
*       GL_DrawBitmap
*
* Purpose:
*  Translates the external bitmap format into an internal
*  format. This turned out to be necessary as the internal
*  format is easier to create and more flexible for routines
*  that draw text-bitmaps.
*/
void GL_DrawBitmap(const GUI_BITMAP GUI_UNI_PTR * pBitmap, int x0, int y0) {
  GUI_DRAWMODE PrevDraw;
  const GUI_LOGPALETTE GUI_UNI_PTR * pPal;
  pPal = pBitmap->pPal;
  PrevDraw = GUI_SetDrawMode(0);  /* No Get... at this point */
  GUI_SetDrawMode((pPal && pPal->HasTrans) ? (PrevDraw|GUI_DRAWMODE_TRANS) : PrevDraw &(~GUI_DRAWMODE_TRANS));
  if (pBitmap->pMethods) {
    pBitmap->pMethods->pfDraw(x0, y0, pBitmap->XSize ,pBitmap->YSize, (uint8_t const *)pBitmap->pData, pBitmap->pPal, 1, 1);
  } else {
    const RGB_COLOR* pTrans = pBitmap->pPal ? pBitmap->pPal->pPalEntries : NULL;
    if (!pTrans) {
      pTrans = (pBitmap->BitsPerPixel != 1) ? NULL : &LCD_BKCOLORINDEX;
    }
    LCD_DrawBitmap( x0,y0
                    ,pBitmap->XSize ,pBitmap->YSize
                    ,pBitmap->BitsPerPixel
                    ,pBitmap->BytesPerLine
                    ,pBitmap->pData
                    ,pTrans);
  }
  GUI_SetDrawMode(PrevDraw);
}

void GUI_DrawBitmap(const GUI_BITMAP GUI_UNI_PTR * pBitmap, int x0, int y0) {
#if (GUI_WINSUPPORT)
    GUI_RECT r;
#endif

#if (GUI_WINSUPPORT)
    WM_ADDORG(x0,y0);
    r.x1 = (r.x0 = x0) + pBitmap->XSize-1;
    r.y1 = (r.y0 = y0) + pBitmap->YSize-1;
    WM_ITERATE_START(&r) {
#endif
  GL_DrawBitmap(pBitmap, x0, y0);
#if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
#endif

}


