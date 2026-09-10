#include "GUI.h"

#include "WM.h"

struct MEMDEV : LCDDEV {
	RECT rect;
	uint16_t BytesPerLine;
	void *pData;
public:
	MEMDEV(RECT r) : LCDDEV(GUI.pDeviceAPI->BitsPerPixel), rect(r) {
		BytesPerLine = (r.XSize() * BPP_Bits[BitsPerPixel] + 7) >> 3;
		pData = GUI_ALLOC_Alloc(r.YSize() * BytesPerLine);
	}
	~MEMDEV() {
		GUI_ALLOC_Free(pData);
		pData = nullptr;
	}
	MEMDEV(const MEMDEV &) = delete;
	MEMDEV &operator=(const MEMDEV &) = delete;
public:
	uint16_t GetSizeX() const { return rect.XSize(); }
	uint16_t GetSizeY() const { return rect.YSize(); }
	RECT Rect() const { return rect; }
public:
	RECT GetRect() override { return rect; }

	RGBC *_XY2PTR(int x, int y) {
		auto pData = (uint8_t *)this->pData;
		pData += (y - rect.y0) * BytesPerLine;
		return ((RGBC *)pData) + x - rect.x0;
	}
	RGBC GetPixel(int16_t x, int16_t y) override {
		return *_XY2PTR(x, y);
	}
	void SetPixel(int16_t x, int16_t y, RGBC color) override {
		*_XY2PTR(x, y) = color;
	}
};

void GUI_MEMDEV_Select(auto pDev) {
	if (!pDev) {
		GUI_SelectLCD();
	} else {
		WObj::Deactivate();
		GUI.pDeviceAPI = pDev;
		GUI.ClipRectMax();
	}
}

void GUI_MEMDEV_CopyToLCD(MEMDEV *pDev) {
	if (!pDev)
		return;
	auto pDevPrev = GUI.pDeviceAPI;
	GUI_SelectLCD();
	WObj::Activate();
	WObj::Iterate(pDev->rect, [&] {
		LCD_DrawBitmap(BITVIEW{
			pDev->rect,
			pDev->BytesPerLine,
			pDev->BitsPerPixel,
			pDev->pData,
			nullptr });
	});
	GUI_MEMDEV_Select(pDevPrev);
}

void GUI_MEMDEV_Draw(RECT r, GUI_CALLBACK_VOID_P *pfDraw, void *pData) {
	if (!(r &= pLCD_API->GetRect()))
		return;
	auto pDev = new MEMDEV(r);
	GUI_MEMDEV_Select(pDev);
	pfDraw(pData);
	GUI_MEMDEV_CopyToLCD(pDev);
	delete pDev;
	GUI_MEMDEV_Select(nullptr);
}
