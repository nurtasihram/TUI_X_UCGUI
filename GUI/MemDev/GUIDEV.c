#include <string.h>

#include "GUI_Private.h"
#include "GUIDebug.h"
#if GUI_WINSUPPORT
  #include "WM.h"
#endif

/* Memory device capabilities are compiled only if support for them is enabled.*/
#if GUI_SUPPORT_MEMDEV

void GUI_MEMDEV__GetRect(GUI_RECT* pRect) {
  GUI_MEMDEV* pDev = GUI_MEMDEV_H2P(GUI_Context.hDevData);
  pRect->x0 = pDev->x0;
  pRect->y0 = pDev->y0;
  pRect->x1 = pDev->x0 + pDev->XSize-1;
  pRect->y1 = pDev->y0 + pDev->YSize-1;
}

void GUI_MEMDEV_Delete(GUI_MEMDEV_Handle hMemDev) {
/* Make sure memory device is not used */

  if (hMemDev) {
    GUI_MEMDEV* pDev;
    if (GUI_Context.hDevData == hMemDev) {
	    GUI_SelectLCD();
    }
    pDev = GUI_MEMDEV_H2P(hMemDev);
    /* Delete the associated usage device */
    if (pDev->hUsage)
      GUI_USAGE_DecUseCnt(pDev->hUsage);
    GUI_ALLOC_Free(hMemDev);
  }

}

GUI_MEMDEV_Handle GUI_MEMDEV__CreateFixed(int x0, int y0, int xsize, int ysize, int Flags
                                        ,const tLCDDEV_APIList * pMemDevAPI) {
  I32 MemSize;
  GUI_USAGE_Handle hUsage = 0;
  unsigned int BitsPerPixel, BytesPerLine;
  GUI_MEMDEV_Handle hMemDev;
  BitsPerPixel = pMemDevAPI->BitsPerPixel;
  BytesPerLine = (xsize * BitsPerPixel + 7) >> 3;     /* Note: This code works with 8 and 16 bit memory devices. If other BPPs are introduced for MemDevs, it needs to be changed */
  /* Calc available MemSize */
  MemSize = GUI_ALLOC_GetMaxSize();
  if (!(Flags & GUI_MEMDEV_NOTRANS)) {
    MemSize = (MemSize / 4) * 3;   /* We need to reserve some memory for usage object ... TBD: This can be optimized as we do not use memory perfectly. */
  }
  if (ysize<=0) {
    int MaxLines = (MemSize - sizeof(GUI_MEMDEV)) / BytesPerLine;
    ysize = (MaxLines > -ysize) ? -ysize : MaxLines;
  }
  if (!(Flags & GUI_MEMDEV_NOTRANS)) {
    /* Create the usage map */
    hUsage = GUI_USAGE_BM_Create(x0, y0, xsize, ysize, 0);
  }
  /* Check if we can alloc sufficient memory */
  if (ysize <= 0) {
    GUI_DEBUG_WARN("GUI_MEMDEV_Create: Too little memory");

    return 0;
  }
  MemSize = ysize * BytesPerLine + sizeof(GUI_MEMDEV);
  if (Flags & GUI_MEMDEV_NOTRANS) {
    hMemDev = GUI_ALLOC_AllocNoInit(MemSize);
  } else {
    hMemDev = GUI_ALLOC_AllocZero(MemSize);
  }
  if (hMemDev) {
    GUI_MEMDEV* pDevData;
    pDevData = GUI_MEMDEV_H2P(hMemDev);
    pDevData->x0            = x0;
    pDevData->y0            = y0;
    pDevData->XSize         = xsize;
    pDevData->YSize         = ysize;
    pDevData->NumColors     = LCD_GET_NUMCOLORS();
    pDevData->BytesPerLine  = BytesPerLine;
    pDevData->hUsage        = hUsage;

    pDevData->pAPIList      = pMemDevAPI;
    pDevData->BitsPerPixel  = BitsPerPixel;
  } else {
    if (hUsage) {
      GUI_ALLOC_Free(hUsage);
    }
    GUI_DEBUG_WARN("GUI_MEMDEV_Create: Alloc failed");
  }
  return hMemDev;
}

GUI_MEMDEV_Handle GUI_MEMDEV_CreateEx(int x0, int y0, int xSize, int ySize, int Flags) {
  GUI_MEMDEV_Handle hMemDev;
  const tLCDDEV_APIList * pDeviceAPI;

  pDeviceAPI = LCD_aAPI[0];
  if (GUI_Context.hDevData == 0) {
  } else {
    /* If a memory device is already selected, we create a compatible one by copying its data */
    GUI_MEMDEV* pDevSel;
    pDevSel = GUI_MEMDEV_H2P(GUI_Context.hDevData);
  }
  hMemDev = GUI_MEMDEV__CreateFixed(x0, y0, xSize, ySize, Flags, pDeviceAPI->pMemDevAPI);

  return hMemDev;
}

GUI_MEMDEV_Handle GUI_MEMDEV_Create(int x0, int y0, int xsize, int ysize) {
  return GUI_MEMDEV_CreateEx(x0, y0, xsize, ysize, GUI_MEMDEV_HASTRANS);
}

GUI_MEMDEV_Handle GUI_MEMDEV_Select(GUI_MEMDEV_Handle hMem) {
  GUI_MEMDEV_Handle r;

  r = GUI_Context.hDevData;
  if (hMem == 0) {
    GUI_SelectLCD();
  } else {
    GUI_MEMDEV* pDev = GUI_MEMDEV_H2P(hMem);
    #if GUI_WINSUPPORT
      WM_Deactivate();
    #endif
    /* If LCD was selected Save cliprect */
    if (GUI_Context.hDevData == 0) {
      GUI_Context.ClipRectPrev = GUI_Context.ClipRect;
    }
    GUI_Context.hDevData = hMem;
    GUI_Context.pDeviceAPI = pDev->pAPIList;
    LCD_SetClipRectMax();
  }

  return r;
}

void GUI_MEMDEV__WriteToActiveAt(GUI_MEMDEV_Handle hMem,int x, int y) {
  GUI_MEMDEV* pDev = GUI_MEMDEV_H2P(hMem);
  GUI_USAGE_h hUsage = pDev->hUsage;
  GUI_USAGE*  pUsage;
  int YSize = pDev->YSize;
  int yi;
  unsigned int BytesPerLine = pDev->BytesPerLine;
  unsigned int BitsPerPixel = pDev->BitsPerPixel;
  int BytesPerPixel = BitsPerPixel >> 3;
  U8* pData = (U8*)(pDev+1);
  if (hUsage) {
    pUsage = GUI_USAGE_H2P(hUsage);
    for (yi = 0; yi < YSize; yi++) {
      int xOff = 0;
      int XSize;
      XSize = GUI_USAGE_GetNextDirty(pUsage, &xOff, yi);
      if (XSize == pDev->XSize) {
        /* If the entire line is affected, calculate the number of entire lines */
        int y0 = yi;
        while ((GUI_USAGE_GetNextDirty(pUsage, &xOff, yi + 1)) == XSize) {
          yi++;
        }
		    LCD_DrawBitmap(x, y + y0, pDev->XSize, yi - y0 + 1, 1, 1, BitsPerPixel, BytesPerLine, pData, NULL);
        pData += (yi - y0 + 1) * BytesPerLine;
      } else {
        /* Draw the partial line which needs to be drawn */
        for (; XSize; ) {
          LCD_DrawBitmap(x + xOff, y + yi, XSize, 1, 1, 1, BitsPerPixel, BytesPerLine, pData + xOff * BytesPerPixel, NULL);
          xOff += XSize;
          XSize = GUI_USAGE_GetNextDirty(pUsage, &xOff, yi);
        }
        pData += BytesPerLine;
      }
    }
  } else {
		LCD_DrawBitmap(x, y, pDev->XSize, YSize, 1, 1, BitsPerPixel, BytesPerLine, pData, NULL);
  }
}

void GUI_MEMDEV_CopyToLCDAt(GUI_MEMDEV_Handle hMem, int x, int y) {
  if (hMem) {
    GUI_MEMDEV_Handle hMemPrev;
    GUI_MEMDEV* pDevData;
  #if (GUI_WINSUPPORT)
    GUI_RECT r;
  #endif

    hMemPrev = GUI_Context.hDevData;
    pDevData = (GUI_MEMDEV*) GUI_ALLOC_h2p(hMem);  /* Convert to pointer */
    /* Make sure LCD is selected as device */
    GUI_SelectLCD();  /* Activate LCD */
    if (x == GUI_POS_AUTO) {
      x = pDevData->x0;
      y = pDevData->y0;
    }
  #if (GUI_WINSUPPORT)
    /* Calculate rectangle */
    r.x1 = (r.x0 = x) + pDevData->XSize-1;
    r.y1 = (r.y0 = y) + pDevData->YSize-1;;
    /* Do the drawing. Window manager has to be on */
    WM_Activate();
    WM_ITERATE_START(&r) {
  #endif
    GUI_MEMDEV__WriteToActiveAt(hMem, x, y);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
    /* Reactivate previously used device */
    GUI_MEMDEV_Select(hMemPrev);

  }
}

void GUI_MEMDEV_CopyToLCD(GUI_MEMDEV_Handle hMem) {
  GUI_MEMDEV_CopyToLCDAt(hMem, GUI_POS_AUTO, GUI_POS_AUTO);
}

#else


#endif /* GUI_SUPPORT_MEMDEV */

