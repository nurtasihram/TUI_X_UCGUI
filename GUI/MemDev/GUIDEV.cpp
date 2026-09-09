#include "GUI.h"

#include "WM.h"

void GUI_MEMDEV_Select(GUI_MEMDEV *pDev) {
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
}

void GUI_MEMDEV_CopyToLCD(GUI_MEMDEV *pDev) {
	if (!pDev)
		return;
	GUI_MEMDEV *pMemPrev = GUI.pDevData;
	GUI_SelectLCD();
	WObj::Activate();
	WObj::Iterate(pDev->rect, [&] {
	LCD_DrawBitmap(BITVIEW{
		pDev->rect,
		(uint16_t)pDev->BytesPerLine,
		(uint8_t)pDev->BitsPerPixel,
		(const uint8_t*)pDev->pData });
	});
	GUI_MEMDEV_Select(pMemPrev);
}

int GUI_MEMDEV_Draw(RECT r, GUI_CALLBACK_VOID_P *pfDraw, void *pData) {
	if (!(r &= pLCD_API->GetRect()))
		return 0;
	auto pDev = new GUI_MEMDEV(r, pLCD_API->pMemDevAPI);
	if (!pDev) {
		pfDraw(pData);
		return 1;
	}
	GUI_MEMDEV_Select(pDev);
	for (int i = 0; i < r.YSize(); i += pDev->GetSizeY()) {
		int RemLines = r.YSize() - i;
		if (RemLines < pDev->GetSizeY())
			pDev->ReduceYSize(RemLines);
		if (i > 0)
			pDev->Org({ r.x0, r.y0 + i });
		pfDraw(pData);
		GUI_MEMDEV_CopyToLCD(pDev);
	}
	delete pDev;
	GUI_MEMDEV_Select(nullptr);
	return 0;
}

#define PIXELINDEX RGBC

struct MemDev_APIList24 : LCDDEV_API {
	MemDev_APIList24() : LCDDEV_API(nullptr, 24) {}

	RECT GetRect() override {
		return GUI.pDevData->Rect();
	}

	static PIXELINDEX *_XY2PTR(int x, int y) {
		auto pDev = GUI.pDevData;
		auto pData = (uint8_t *)pDev->pData;
		pData += (y - pDev->rect.y0) * pDev->BytesPerLine;
		return ((PIXELINDEX *)pData) + x - pDev->rect.x0;
	}
	RGBC GetPixel(int16_t x, int16_t y) override {
		return *_XY2PTR(x, y);
	}
	void SetPixel(int16_t x, int16_t y, RGBC color) override {
		*_XY2PTR(x, y) = color;
	}
	//void FillRect(RECT r, RGBC color) override {
	//	unsigned BytesPerLine;
	//	int Len;
	//	auto pDev = GUI.pDevData;
	//	auto pData = _XY2PTR(r.x0, r.y0);
	//	BytesPerLine = pDev->BytesPerLine;
	//	Len = r.x1 - r.x0 + 1;
	//	for (; r.y0 <= r.y1; r.y0++) {
	//		for (int i = 0; i < Len; i++)
	//			pData[i] = color;
	//		pData = (PIXELINDEX *)((uint8_t *)pData + BytesPerLine);
	//	}
	//}
} GUI_MEMDEV__APIList24;

LCDDEV_API *pMEMDEV__APIList24 = &GUI_MEMDEV__APIList24;
