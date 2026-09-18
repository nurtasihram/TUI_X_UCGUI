#define DLL_IMPORTS 1
#define AYXANDAR
#include "SimDisp.h"
#include "GUI.h"

#define LCD_XSIZE 320 * 2
#define LCD_YSIZE 240 * 2

Ayxandar Ayx;

struct LCD_API : public LCDDEV {
	LCD_API() : LCDDEV(BPP_32) {}
	RECT Rect() override {
		return{ 0, 0, LCD_XSIZE - 1, LCD_YSIZE - 1 };
	}
	RGBC GetPixel(int16_t x, int16_t y) override {
		return Ayx.Dot({ x, y });
	}
	void SetPixel(int16_t x, int16_t y, RGBC Color) override {
		Ayx.Dot({ x, y }, Color);
	}
	void FillRect(RECT r, RGBC color) override {
		Ayx.Fill(color, { r.x0, r.y0, r.x1, r.y1 });
	}
} LCD_API;

LCDDEV *GUI_X_GetLCD() {
	static LCDDEV *pLCD_API = nullptr;
	if (pLCD_API)
		return pLCD_API;
	SimDisp::LoadDll(_T("SimClient.dll"));
	//SimDisp::LoadDll(_T("SimDisp.dll"));
	assert(SimDisp::Open(L"TUI By Nurtas Ihram", LCD_XSIZE, LCD_YSIZE));
	SimDisp::SetOnDestroy([] {
		ExitProcess(0);
	});
	SimDisp::SetOnMouse([](int16_t xPos, int16_t yPos, int16_t zPos,
						   tSimDisp_MouseKey mk) {
		if (xPos < 0 || yPos < 0)
			return;
		GUI_PID_Store({ { xPos, yPos }, mk.Left });
	});
	SimDisp::SetOnKey([](uint16_t Key, uint8_t Pressed) {
		GUI_KEY_Store({ Key, Pressed });
	});
	SimDisp::SetOnResize([](uint16_t nSizeX, uint16_t nSizeY) -> BOOL {
		//xSizeDisp = nSizeX;
		//ySizeDisp = nSizeY;
		return TRUE;
	});
	//SimDisp::GetSize(&xSizeDisp, &ySizeDisp);
	SimDisp::HideCursor(true);
	SimDisp::AutoFlush(true);
	SimDisp::Show(true);
	Ayx.Init();
	return pLCD_API = &LCD_API;
}
