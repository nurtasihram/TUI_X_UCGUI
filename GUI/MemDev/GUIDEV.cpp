#include "GUI_Private.h"
#include "WM.h"

#if GUI_SUPPORT_MEMDEV

RECT GUI_MEMDEV__GetRect() {
	auto pDev = GUI.pDevData;
	RECT r;
	r.x0 = pDev->x0;
	r.y0 = pDev->y0;
	r.x1 = pDev->x0 + pDev->XSize - 1;
	r.y1 = pDev->y0 + pDev->YSize - 1;
	return r;
}

void GUI_MEMDEV_Delete(GUI_MEMDEV *pDev) {
	if (!pDev)
		return;
	if (GUI.pDevData == pDev)
		GUI_SelectLCD();
	GUI_ALLOC_Free(pDev);
}

GUI_MEMDEV *GUI_MEMDEV__CreateFixed(int x0, int y0, int xsize, int ysize, int Flags,
									const tLCDDEV_APIList *pMemDevAPI) {
	if (xsize <= 0 || ysize <= 0) {
		GUI_DEBUG_WARN("GUI_MEMDEV_Create: Too little memory");
		return nullptr;
	}
	auto BitsPerPixel = pMemDevAPI->BitsPerPixel;
	unsigned int BytesPerLine;
	if (BitsPerPixel == 24)
		BytesPerLine = xsize * 4;
	else
		BytesPerLine = (xsize * BitsPerPixel + 7) >> 3;
	auto MemSize = ysize * BytesPerLine + sizeof(GUI_MEMDEV);
	auto pDevData = static_cast<GUI_MEMDEV*>(GUI_ALLOC_Alloc(MemSize));
	if (pDevData) {
		pDevData->x0 = x0;
		pDevData->y0 = y0;
		pDevData->XSize = xsize;
		pDevData->YSize = ysize;
		pDevData->BytesPerLine = BytesPerLine;
		pDevData->pAPIList = pMemDevAPI;
		pDevData->BitsPerPixel = BitsPerPixel;
	} else
		GUI_DEBUG_WARN("GUI_MEMDEV_Create: Alloc failed");
	return pDevData;
}
GUI_MEMDEV *GUI_MEMDEV_CreateEx(int x0, int y0, int xSize, int ySize, int Flags) {
	return GUI_MEMDEV__CreateFixed(x0, y0, xSize, ySize, Flags, LCD_API.pMemDevAPI);
}
GUI_MEMDEV *GUI_MEMDEV_Create(int x0, int y0, int xsize, int ysize) {
	return GUI_MEMDEV_CreateEx(x0, y0, xsize, ysize, GUI_MEMDEV_HASTRANS);
}

GUI_MEMDEV *GUI_MEMDEV_Select(GUI_MEMDEV *pDev) {
	auto pPrevDevice = GUI.pDevData;
	if (!pDev) {
		GUI_SelectLCD();
	} else {
		WObj::Deactivate();
		if (!GUI.pDevData)
			GUI.ClipRectPrev = GUI.rClip;
		GUI.pDevData = pDev;
		GUI.pDeviceAPI = pDev->pAPIList;
		GUI.ClipRectMax();
	}
	return pPrevDevice;
}

void GUI_MEMDEV__WriteToActiveAt(GUI_MEMDEV *pDev, int x, int y) {
	LCD_DrawBitmap(BITVIEW{
		RECT::LeftTop({ x, y }, { pDev->XSize, pDev->YSize }),
		(uint16_t)pDev->BytesPerLine, (uint8_t)pDev->BitsPerPixel,
		reinterpret_cast<uint8_t*>(pDev + 1) });
}

void GUI_MEMDEV_CopyToLCDAt(GUI_MEMDEV *pDev, int x, int y) {
	if (!pDev)
		return;
	GUI_MEMDEV *pMemPrev = GUI.pDevData;
	GUI_SelectLCD();
	RECT r;
	r.x0 = x;
	r.y0 = y;
	r.x1 = x + pDev->XSize - 1;
	r.y1 = y + pDev->YSize - 1;
	WObj::Activate();
	WObj::Iterate(r, [&] {
		GUI_MEMDEV__WriteToActiveAt(pDev, x, y);
	});
	GUI_MEMDEV_Select(pMemPrev);
}

void GUI_MEMDEV_CopyToLCD(GUI_MEMDEV *pDev) {
	GUI_MEMDEV_CopyToLCDAt(pDev, pDev->x0, pDev->y0);
}

void GUI_MEMDEV_ReduceYSize(GUI_MEMDEV *pDev, int YSize) {
	if (!pDev)
		pDev = GUI.pDevData;
	if (!pDev)
		return;
	if (pDev->YSize > YSize)
		pDev->YSize = YSize;
}

void GUI_MEMDEV_SetOrg(GUI_MEMDEV *pDev, int x0, int y0) {
	if (!pDev) {
		pDev = GUI.pDevData;
		if (!pDev)
			return;
	}
	pDev->x0 = x0;
	pDev->y0 = y0;
	GUI.ClipRectMax();
}

static int _Min(int v0, int v1) {
	return (v0 <= v1) ? v0 : v1;
}
int GUI_MEMDEV_Draw(RECT *pRect, GUI_CALLBACK_VOID_P *pfDraw, void *pData, int NumLines, int Flags) {
	auto rc = pRect ? *pRect & LCD_API.pfGetRect() : LCD_API.pfGetRect();
	if (NumLines == 0)
		NumLines = rc.YSize();
	if (rc.XSize() <= 0 || rc.YSize() <= 0)
		return 0;
	auto pMD = GUI_MEMDEV_CreateEx(rc.x0, rc.y0, rc.XSize(), NumLines, Flags);
	if (!pMD) {
		pfDraw(pData);
		return 1;
	}
	NumLines = pMD->YSize;
	GUI_MEMDEV_Select(pMD);
	for (int i = 0; i < rc.YSize(); i += NumLines) {
		int RemLines = rc.YSize() - i;
		if (RemLines < NumLines)
			GUI_MEMDEV_ReduceYSize(pMD, RemLines);
		if (i > 0)
			GUI_MEMDEV_SetOrg(pMD, rc.x0, rc.y0 + i);
		pfDraw(pData);
		GUI_MEMDEV_CopyToLCD(pMD);
	}
	GUI_MEMDEV_Delete(pMD);
	GUI_MEMDEV_Select(nullptr);
	return 0;
}

#endif
