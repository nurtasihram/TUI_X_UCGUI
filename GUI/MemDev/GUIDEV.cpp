#include "GUI_Private.h"
#include "GUIDebug.h"
#include "WM.h"

#if GUI_SUPPORT_MEMDEV

static constexpr int BYTES_PER_PIXEL_24BIT = 4;
static constexpr double MEMDEV_RESERVE_RATIO = 0.75;

void GUI_MEMDEV__GetRect(RECT *pRect) {
	auto pDev = GUI.pDevData;

	pRect->x0 = pDev->x0;
	pRect->y0 = pDev->y0;
	pRect->x1 = pDev->x0 + pDev->XSize - 1;
	pRect->y1 = pDev->y0 + pDev->YSize - 1;
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
	const unsigned int BitsPerPixel = pMemDevAPI->BitsPerPixel;

	unsigned int BytesPerLine;
	if (BitsPerPixel == 24)
		BytesPerLine = xsize * BYTES_PER_PIXEL_24BIT;
	else
		BytesPerLine = (xsize * BitsPerPixel + 7) >> 3;

	size_t MemSize = GUI_ALLOC_GetMaxSize();
	if (!(Flags & GUI_MEMDEV_NOTRANS))
		MemSize = static_cast<size_t>(MemSize * MEMDEV_RESERVE_RATIO);

	if (ysize <= 0) {
		int MaxLines = static_cast<int>((MemSize - sizeof(GUI_MEMDEV)) / BytesPerLine);
		ysize = (MaxLines > -ysize) ? -ysize : MaxLines;
	}

	if (ysize <= 0) {
		GUI_DEBUG_WARN("GUI_MEMDEV_Create: Too little memory");
		return nullptr;
	}

	MemSize = ysize * BytesPerLine + sizeof(GUI_MEMDEV);
	auto pDevData = static_cast<GUI_MEMDEV*>(GUI_ALLOC_AllocZero(MemSize));

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
	auto pDeviceAPI = LCD_aAPI[0];
	return GUI_MEMDEV__CreateFixed(x0, y0, xSize, ySize, Flags, pDeviceAPI->pMemDevAPI);
}

GUI_MEMDEV *GUI_MEMDEV_Create(int x0, int y0, int xsize, int ysize) {
	return GUI_MEMDEV_CreateEx(x0, y0, xsize, ysize, GUI_MEMDEV_HASTRANS);
}

GUI_MEMDEV *GUI_MEMDEV_Select(GUI_MEMDEV *pDev) {
	GUI_MEMDEV *pPrevDevice = GUI.pDevData;

	if (pDev == nullptr) {
		GUI_SelectLCD();
	} else {
		WObj::Deactivate();

		if (GUI.pDevData == nullptr)
			GUI.ClipRectPrev = GUI.ClipRect;

		GUI.pDevData = pDev;
		GUI.pDeviceAPI = pDev->pAPIList;
		LCD_SetClipRectMax();
	}

	return pPrevDevice;
}

void GUI_MEMDEV__WriteToActiveAt(GUI_MEMDEV *pDev, int x, int y) {
	const int XSize = pDev->XSize;
	const int YSize = pDev->YSize;
	const unsigned int BytesPerLine = pDev->BytesPerLine;
	const unsigned int BitsPerPixel = pDev->BitsPerPixel;
	auto pData = reinterpret_cast<uint8_t*>(pDev + 1);
	LCD_DrawBitmap(x, y, XSize, YSize, BitsPerPixel, BytesPerLine, pData, nullptr);
}

void GUI_MEMDEV_CopyToLCDAt(GUI_MEMDEV *pDev, int x, int y) {
	if (!pDev)
		return;

	GUI_MEMDEV *pMemPrev = GUI.pDevData;

	GUI_SelectLCD();

	if (x == GUI_POS_AUTO) {
		x = pDev->x0;
		y = pDev->y0;
	}

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
	GUI_MEMDEV_CopyToLCDAt(pDev, GUI_POS_AUTO, GUI_POS_AUTO);
}

int GUI_MEMDEV_GetXSize(GUI_MEMDEV *pDev) {
	if (!pDev)
		pDev = GUI.pDevData;
	return pDev ? pDev->XSize : 0;
}

int GUI_MEMDEV_GetYSize(GUI_MEMDEV *pDev) {
	if (!pDev)
		pDev = GUI.pDevData;
	return pDev ? pDev->YSize : 0;
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
	LCD_SetClipRectMax();
}

static int _Min(int v0, int v1) {
	return (v0 <= v1) ? v0 : v1;
}
int GUI_MEMDEV_Draw(RECT *pRect, GUI_CALLBACK_VOID_P *pfDraw, void *pData, int NumLines, int Flags) {
	int x0, y0, xsize, ysize;

	if (pRect) {
		x0 = (pRect->x0 < 0) ? 0 : pRect->x0;
		y0 = (pRect->y0 < 0) ? 0 : pRect->y0;
		int x1 = _Min(pRect->x1, LCD_GetXSize() - 1);
		int y1 = _Min(pRect->y1, LCD_GetYSize() - 1);
		xsize = x1 - x0 + 1;
		ysize = y1 - y0 + 1;
	} else {
		x0 = 0;
		y0 = 0;
		xsize = LCD_GetXSize();
		ysize = LCD_GetYSize();
	}

	if (NumLines == 0)
		NumLines = -ysize;

	if (xsize <= 0 || ysize <= 0)
		return 0;

	GUI_MEMDEV *pMD = GUI_MEMDEV_CreateEx(x0, y0, xsize, NumLines, Flags);
	if (!pMD) {
		pfDraw(pData);
		return 1;
	}

	NumLines = GUI_MEMDEV_GetYSize(pMD);
	GUI_MEMDEV_Select(pMD);

	for (int i = 0; i < ysize; i += NumLines) {
		int RemLines = ysize - i;
		if (RemLines < NumLines)
			GUI_MEMDEV_ReduceYSize(pMD, RemLines);

		if (i > 0)
			GUI_MEMDEV_SetOrg(pMD, x0, y0 + i);

		pfDraw(pData);
		GUI_MEMDEV_CopyToLCD(pMD);
	}

	GUI_MEMDEV_Delete(pMD);
	GUI_MEMDEV_Select(nullptr);
	return 0;
}

#endif
