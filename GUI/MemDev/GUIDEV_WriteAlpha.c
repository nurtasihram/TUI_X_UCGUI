/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2004  SEGGER Microcontroller Systeme GmbH        *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

***** emWin - Graphical user interface for embedded applications *****
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIDEV_WriteAlpha.C
Purpose     : Implementation of memory devices
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include "GUI_Private.h"
#include "GUIDebug.h"

/* Memory device capabilities are compiled only if support for them is enabled.*/
#if GUI_SUPPORT_MEMDEV

static void _WriteAlphaToActiveAt(GUI_MEMDEV_Handle hMem, int Intens, int x, int y) {
  /* Make sure the memory handle is valid */
  if (hMem) {
    GUI_MEMDEV * pDev = (hMem);
    GUI_USAGE_h hUsage = pDev->hUsage;
    GUI_USAGE*  pUsage;
    int YSize = pDev->YSize;
    int yi;
    if (hUsage) {
      pUsage = (hUsage);
      for (yi = 0; yi < YSize; yi++) {
        int xOff = 0;
        int XSize;
        XSize = GUI_USAGE_GetNextDirty(pUsage, &xOff, yi);
        /* Draw the partial line which needs to be drawn */
        for (; XSize; ) {
          uint8_t* pData;
          pData = (uint8_t*)GUI_MEMDEV__XY2PTREx(pDev, xOff, yi);
          do {
            RGB_COLOR Color, BkColor;
            int xPos, yPos, Index;
            if (pDev->BitsPerPixel == 8) {
              Index = *pData++;
            } else {
              Index = *(uint16_t*)pData;
              pData += 2;
            }
            Color   = Index;
            xPos    = xOff + x;
            yPos    = yi +y;
            BkColor = LCD_GetPixelColor(xPos, yPos);
            Color   = LCD_MixColors256(Color, BkColor, Intens);
            Index   = Color;
            LCD_SetPixelIndex(xPos, yPos, Index);
            xOff++;
          } while (--XSize);
          XSize = GUI_USAGE_GetNextDirty(pUsage, &xOff, yi);
        }
      }
    }
  }
}

void GUI_MEMDEV_WriteAlphaAt(GUI_MEMDEV_Handle hMem, int Alpha, int x, int y) {
  if (hMem) {
    GUI_MEMDEV* pDevData;
#if (GUI_WINSUPPORT)
      GUI_RECT r;
#endif

    pDevData = (GUI_MEMDEV*) (hMem);  /* Convert to pointer */
    if (x == GUI_POS_AUTO) {
      x = pDevData->x0;
      y = pDevData->y0;
    }
#if (GUI_WINSUPPORT)
      r.x1 = (r.x0 = x) + pDevData->XSize-1;
      r.y1 = (r.y0 = y) + pDevData->YSize-1;;
      WM_ITERATE_START(&r) {
      _WriteAlphaToActiveAt(hMem, Alpha, x,y);
      } WM_ITERATE_END();
#else
      _WriteAlphaToActiveAt(hMem, Alpha, x,y);
#endif

  }
}

void GUI_MEMDEV_WriteAlpha(GUI_MEMDEV_Handle hMem, int Alpha) {
  GUI_MEMDEV_WriteAlphaAt(hMem, Alpha, GUI_POS_AUTO, GUI_POS_AUTO);
}

#else


#endif /* GUI_SUPPORT_MEMDEV */

