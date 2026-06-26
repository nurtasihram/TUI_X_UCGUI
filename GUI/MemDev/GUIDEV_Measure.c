#include <string.h>

#include "GUI_Private.h"
#include "GUIDebug.h"
#if GUI_WINSUPPORT
#include "WM.h"
#endif

/* Memory device capabilities are compiled only if support for them is enabled.*/
#if GUI_SUPPORT_MEMDEV

typedef struct {
  GUI_RECT rUsed;
} GUI_MEASDEV;


static void _MarkPixel(int x, int y) {
  GUI_MEASDEV* pDev = (GUI_MEASDEV*)((GUI_Context.hDevData));
  if (x < pDev->rUsed.x0)
    pDev->rUsed.x0 = x;
  if (x > pDev->rUsed.x1)
    pDev->rUsed.x1 = x;
  if (y < pDev->rUsed.y0)
    pDev->rUsed.y0 = y;
  if (y > pDev->rUsed.y1)
    pDev->rUsed.y1 = y;
}

static void _MarkRect(int x0, int y0, int x1, int y1) {
  GUI_MEASDEV* pDev = (GUI_MEASDEV*)((GUI_Context.hDevData));
  if (x0 < pDev->rUsed.x0)
    pDev->rUsed.x0 = x0;
  if (x1 > pDev->rUsed.x1)
    pDev->rUsed.x1 = x1;
  if (y1 < pDev->rUsed.y0)
    pDev->rUsed.y0 = y0;
  if (y1 > pDev->rUsed.y1)
    pDev->rUsed.y1 = y1;
}

static void _DrawBitmap(int x0, int y0, int xsize, int ysize,
                       int BitsPerPixel, int BytesPerLine,
                       const uint8_t  * pData, int Diff,
                       const RGB_COLOR* pTrans)
{
  GUI_USE_PARA(BitsPerPixel);
  GUI_USE_PARA(BytesPerLine);
  GUI_USE_PARA(pData);
  GUI_USE_PARA(Diff);
  GUI_USE_PARA(pTrans);
  _MarkRect(x0, y0, x0 + xsize - 1, y0 + ysize - 1);
}

static void _DrawHLine(int x0, int y,  int x1) {
  _MarkRect(x0, y, x1, y);
}

static void _DrawVLine(int x , int y0,  int y1) {
  _MarkRect(x, y0, x, y1);
}

static void _SetPixelIndex(int x, int y, int Index) {
  GUI_USE_PARA(Index);
  _MarkPixel(x, y);
}

static void _XorPixel(int x, int y) {
  _MarkPixel(x, y);
}

static unsigned int _GetPixelIndex(int x, int y) {
  GUI_USE_PARA(x);
  GUI_USE_PARA(y);
  return 0;
}

static void _FillRect(int x0, int y0, int x1, int y1) {
  _MarkRect(x0, y0, x1, y1);
}

static void _GetRect(GUI_RECT* pRect) {
  pRect->x0 = pRect->y0 = -4095;
  pRect->x1 = pRect->y1 =  4095;
}

static const tLCDDEV_APIList _APIList = {
  (tLCDDEV_DrawBitmap*)_DrawBitmap,
  _DrawHLine,
  _DrawVLine,
  _FillRect,
  _GetPixelIndex,
  _GetRect,
  _SetPixelIndex,
  _XorPixel,
  NULL,
};

void GUI_MEASDEV_Delete(GUI_MEASDEV_Handle hMemDev) {
  /* Make sure memory device is not used */
  if ((GUI_Context.hDevData = hMemDev) != 0) {
	  GUI_SelectLCD();
  }
  GUI_ALLOC_Free(hMemDev);
}

void GUI_MEASDEV_ClearRect(GUI_MEASDEV_Handle hMemDev) {
  if (hMemDev) {
    GUI_MEASDEV* pDevData;

    pDevData = (GUI_MEASDEV*)(hMemDev);
    pDevData->rUsed.x0 = GUI_XMAX;
    pDevData->rUsed.y0 = GUI_YMAX;
    pDevData->rUsed.x1 = GUI_XMIN;
    pDevData->rUsed.y1 = GUI_YMIN;

  }
}

GUI_MEASDEV_Handle GUI_MEASDEV_Create(void) {
  int MemSize;
  GUI_MEASDEV_Handle hMemDev;
  MemSize = sizeof(GUI_MEASDEV);
  hMemDev = GUI_ALLOC_AllocZero(MemSize);
  if (hMemDev) {
    GUI_MEASDEV* pDevData;

    pDevData = (GUI_MEASDEV*)(hMemDev);
    GUI_MEASDEV_ClearRect(hMemDev);

  } else {
  }
  return hMemDev;
}

void GUI_MEASDEV_Select(GUI_MEASDEV_Handle hMem) {
  if (hMem == 0) {
    GUI_SelectLCD();
  } else {
#if GUI_WINSUPPORT
      WM_Deactivate();
#endif
    GUI_Context.hDevData     = hMem;
    GUI_Context.pDeviceAPI   = &_APIList;
    GUI_Context.pClipRect_HL = NULL;
    LCD_SetClipRectMax();
  }
}

void GUI_MEASDEV_GetRect(GUI_MEASDEV_Handle hMem, GUI_RECT* pRect) {
  if (hMem) {
    GUI_MEASDEV* pDev;

    pDev = (GUI_MEASDEV*)(hMem);
    if (pRect) {
      pRect->x0 = pDev->rUsed.x0;
      pRect->y0 = pDev->rUsed.y0;
      pRect->x1 = pDev->rUsed.x1;
      pRect->y1 = pDev->rUsed.y1;
    }

  }
}

#else

void GUIDEV_Measure(void) {} /* avoid empty object files */

#endif /* GUI_MEMDEV_SUPPORT */

