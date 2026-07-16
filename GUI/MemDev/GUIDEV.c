
#include "GUI_Private.h"
#include "GUIDebug.h"

#include "WM.h"

/* Memory device capabilities are compiled only if support for them is enabled.*/
#if GUI_SUPPORT_MEMDEV

void GUI_MEMDEV__GetRect(GUI_RECT *pRect) {
	GUI_MEMDEV *pDev = (GUI_Context.hDevData);
	pRect->x0 = pDev->x0;
	pRect->y0 = pDev->y0;
	pRect->x1 = pDev->x0 + pDev->XSize - 1;
	pRect->y1 = pDev->y0 + pDev->YSize - 1;
}

void GUI_MEMDEV_Delete(GUI_MEMDEV_Handle hMemDev) {
	/* Make sure memory device is not used */
	if (hMemDev) {
		GUI_MEMDEV *pDev = (GUI_MEMDEV *)hMemDev;
		if (GUI_Context.hDevData == hMemDev)
			GUI_SelectLCD();
		/* Delete the associated usage device */
		GUI_ALLOC_Free(hMemDev);
	}
}

GUI_MEMDEV_Handle GUI_MEMDEV__CreateFixed(int x0, int y0, int xsize, int ysize, int Flags,
										  const tLCDDEV_APIList *pMemDevAPI) {
	size_t MemSize;
	unsigned int BitsPerPixel, BytesPerLine;
	GUI_MEMDEV_Handle hMemDev;
	BitsPerPixel = pMemDevAPI->BitsPerPixel;
	/* Calculate BytesPerLine based on BitsPerPixel */
	if (BitsPerPixel == 24) {
		/* For 24-bit, use 4 bytes (32-bit) per pixel for alignment */
		BytesPerLine = xsize * 4;
	}
	else {
		/* For 8 and 16 bit memory devices */
		BytesPerLine = (xsize * BitsPerPixel + 7) >> 3;
	}
	/* Calc available MemSize */
	MemSize = GUI_ALLOC_GetMaxSize();
	if (!(Flags & GUI_MEMDEV_NOTRANS)) {
		MemSize = (MemSize / 4) * 3;   /* We need to reserve some memory for usage object ... TBD: This can be optimized as we do not use memory perfectly. */
	}
	if (ysize <= 0) {
		int MaxLines = (int)((MemSize - sizeof(GUI_MEMDEV)) / BytesPerLine);
		ysize = (MaxLines > -ysize) ? -ysize : MaxLines;
	}
	/* Check if we can alloc sufficient memory */
	if (ysize <= 0) {
		GUI_DEBUG_WARN("GUI_MEMDEV_Create: Too little memory");

		return 0;
	}
	MemSize = ysize * BytesPerLine + sizeof(GUI_MEMDEV);
	if (Flags & GUI_MEMDEV_NOTRANS) {
		hMemDev = GUI_ALLOC_AllocNoInit(MemSize);
	}
	else {
		hMemDev = GUI_ALLOC_AllocZero(MemSize);
	}
	if (hMemDev) {
		GUI_MEMDEV *pDevData;
		pDevData = (hMemDev);
		pDevData->x0 = x0;
		pDevData->y0 = y0;
		pDevData->XSize = xsize;
		pDevData->YSize = ysize;
		pDevData->BytesPerLine = BytesPerLine;

		pDevData->pAPIList = pMemDevAPI;
		pDevData->BitsPerPixel = BitsPerPixel;
	}
	else {
		GUI_DEBUG_WARN("GUI_MEMDEV_Create: Alloc failed");
	}
	return hMemDev;
}

GUI_MEMDEV_Handle GUI_MEMDEV_CreateEx(int x0, int y0, int xSize, int ySize, int Flags) {
	const tLCDDEV_APIList *pDeviceAPI = LCD_aAPI[0];
	return GUI_MEMDEV__CreateFixed(x0, y0, xSize, ySize, Flags, pDeviceAPI->pMemDevAPI);
}

GUI_MEMDEV_Handle GUI_MEMDEV_Create(int x0, int y0, int xsize, int ysize) {
	return GUI_MEMDEV_CreateEx(x0, y0, xsize, ysize, GUI_MEMDEV_HASTRANS);
}

GUI_MEMDEV_Handle GUI_MEMDEV_Select(GUI_MEMDEV_Handle hMem) {
	GUI_MEMDEV_Handle r = GUI_Context.hDevData;
	if (hMem == 0)
		GUI_SelectLCD();
	else {
		GUI_MEMDEV *pDev = (hMem);
		WM_Deactivate();
		/* If LCD was selected Save cliprect */
		if (GUI_Context.hDevData == 0)
			GUI_Context.ClipRectPrev = GUI_Context.ClipRect;
		GUI_Context.hDevData = hMem;
		GUI_Context.pDeviceAPI = pDev->pAPIList;
		LCD_SetClipRectMax();
	}
	return r;
}

void GUI_MEMDEV__WriteToActiveAt(GUI_MEMDEV_Handle hMem, int x, int y) {
	GUI_MEMDEV *pDev = (hMem);
	int YSize = pDev->YSize;
	unsigned int BytesPerLine = pDev->BytesPerLine;
	unsigned int BitsPerPixel = pDev->BitsPerPixel;
	int BytesPerPixel;
	/* Calculate BytesPerPixel correctly for 24-bit (uses 4 bytes per pixel) */
	if (BitsPerPixel == 24) {
		BytesPerPixel = 4;
	}
	else {
		BytesPerPixel = BitsPerPixel >> 3;
	}
	uint8_t *pData = (uint8_t *)(pDev + 1);
	LCD_DrawBitmap(x, y, pDev->XSize, YSize, BitsPerPixel, BytesPerLine, pData, NULL);
}

void GUI_MEMDEV_CopyToLCDAt(GUI_MEMDEV_Handle hMem, int x, int y) {
	if (hMem) {
		GUI_RECT r;
		GUI_MEMDEV_Handle hMemPrev = GUI_Context.hDevData;
		GUI_MEMDEV *pDevData = (GUI_MEMDEV *)(hMem);  /* Convert to pointer */
		/* Make sure LCD is selected as device */
		GUI_SelectLCD();  /* Activate LCD */
		if (x == GUI_POS_AUTO) {
			x = pDevData->x0;
			y = pDevData->y0;
		}
		/* Calculate rectangle */
		r.x1 = (r.x0 = x) + pDevData->XSize - 1;
		r.y1 = (r.y0 = y) + pDevData->YSize - 1;;
		/* Do the drawing. Window manager has to be on */
		WM_Activate();
		WM_ITERATE_START(&r) {
			GUI_MEMDEV__WriteToActiveAt(hMem, x, y);
		} WM_ITERATE_END();
		/* Reactivate previously used device */
		GUI_MEMDEV_Select(hMemPrev);
	}
}

void GUI_MEMDEV_CopyToLCD(GUI_MEMDEV_Handle hMem) {
	GUI_MEMDEV_CopyToLCDAt(hMem, GUI_POS_AUTO, GUI_POS_AUTO);
}

int GUI_MEMDEV_GetXSize(GUI_MEMDEV_Handle hMem) {
	if (!hMem)
		hMem = GUI_Context.hDevData;
	if (hMem)
		return ((GUI_MEMDEV *)hMem)->XSize;
	return 0;
}
int GUI_MEMDEV_GetYSize(GUI_MEMDEV_Handle hMem) {
	if (!hMem)
		hMem = GUI_Context.hDevData;
	if (hMem)
		return ((GUI_MEMDEV *)hMem)->YSize;
	return 0;
}
void GUI_MEMDEV_ReduceYSize(GUI_MEMDEV_Handle hMem, int YSize) {
	/* Make sure memory handle is valid */
	if (!hMem)
		hMem = GUI_Context.hDevData;
	if (!hMem)
		return;
	GUI_MEMDEV *pDevData = (GUI_MEMDEV *)(hMem);  /* Convert to pointer */
	if (pDevData->YSize > YSize)
		pDevData->YSize = YSize;
}

void GUI_MEMDEV_SetOrg(GUI_MEMDEV_Handle hMem, int x0, int y0) {
	/* Make sure memory handle is valid */
	if (!hMem) {
		if ((hMem = GUI_Context.hDevData) == 0) {
			return;
		}
	}

	{
		GUI_MEMDEV *pDev = (hMem);  /* Convert to pointer */
		pDev->y0 = y0;
		pDev->x0 = x0;
		LCD_SetClipRectMax();
	}

}

static int _Min(int v0, int v1) {
	if (v0 <= v1) {
		return v0;
	}
	return v1;
}

/*********************************************************************
*
*       GUI_MEMDEV_Draw
*
*  This routine uses a banding memory device to draw the
*  given area flicker free. It not only draws, but also
*  automatically calculates the size of, creates, moves
*  and then destroys the memory device.
*/
int GUI_MEMDEV_Draw(GUI_RECT *pRect, GUI_CALLBACK_VOID_P *pfDraw, void *pData, int NumLines, int Flags) {
	int x0, y0, x1, y1, xsize, ysize;
	GUI_MEMDEV_Handle hMD;
	if (pRect) {
		x0 = (pRect->x0 < 0) ? 0 : pRect->x0;
		y0 = (pRect->y0 < 0) ? 0 : pRect->y0;
		x1 = _Min(pRect->x1, LCD_GetXSize() - 1);
		y1 = _Min(pRect->y1, LCD_GetYSize() - 1);
		xsize = x1 - x0 + 1;
		ysize = y1 - y0 + 1;
	}
	else {
		x0 = 0;
		y0 = 0;
		xsize = LCD_GetXSize();
		ysize = LCD_GetYSize();
	}
	if (NumLines == 0) {
		NumLines = -ysize;   /* Request <ysize> lines ... Less is o.k. */
	}
	if ((xsize <= 0) || (ysize <= 0))
		return 0;           /* Nothing to do ... */
	/* Create memory device */
	hMD = GUI_MEMDEV_CreateEx(x0, y0, xsize, NumLines, Flags);
	if (!hMD) {
		pfDraw(pData);
		return 1;
	}
	NumLines = GUI_MEMDEV_GetYSize(hMD);
	GUI_MEMDEV_Select(hMD);
	/* Start drawing ... */
	{
		int i;
		for (i = 0; i < ysize; i += NumLines) {
			int RemLines = ysize - i;
			if (RemLines < NumLines) {
				GUI_MEMDEV_ReduceYSize(hMD, RemLines);
			}
			if (i) {
				GUI_MEMDEV_SetOrg(hMD, x0, y0 + i);
			}
			pfDraw(pData);
			GUI_MEMDEV_CopyToLCD(hMD);
		}
	}
	GUI_MEMDEV_Delete(hMD);
	GUI_MEMDEV_Select(0);
	return 0;             /* Success ! */
}

#endif /* GUI_SUPPORT_MEMDEV */
