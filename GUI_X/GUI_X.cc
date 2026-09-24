#define DLL_IMPORTS 1
#define AYXANDAR
#include "SimDisp.h"

import TUX;
import TUX.X;

void GUI_X_Init(void) {}

int GUI_TIME_Get(void) {
	return (int)GetTickCount();
}

#pragma region LCD Device
#define LCD_XSIZE 320 * 2
#define LCD_YSIZE 240 * 2

Ayxandar Ayx;

struct LCD_API : public LCDDEV {
	LCD_API() : LCDDEV() {}
	RECT Rect() const override {
		return{ 0, 0, LCD_XSIZE - 1, LCD_YSIZE - 1 };
	}
	BPP_MODE BitsPerPixel() const override
	{ return BPP_32; }
	RGBC GetPixel(int16_t x, int16_t y) override {
		return Ayx.Dot({ x, y });
	}
	void SetPixel(int16_t x, int16_t y, RGBC Color) override {
		Ayx.Dot({ x, y }, Color);
	}
	void rFill(RECT r, RGBC color) override {
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
#pragma endregion

#pragma region Memory Management
void *GUI_MEM_Alloc(size_t Size) {
	return malloc(Size);
}
void GUI_MEM_Free(void *ptr) {
	free(ptr);
}
void GUI_MEM_FreePtr(void **pptr) {
	GUI_MEM_Free(*pptr);
	*pptr = 0;
}
void *GUI_MEM_Realloc(void *ptr, size_t NewSize) {
	return realloc(ptr, NewSize);
}

void GUI__memcpy(void *pDest, const void *pSrc, size_t NumBytes) {
	memcpy(pDest, pSrc, NumBytes);
}
void GUI__memmove(void *pDest, const void *pSrc, size_t NumBytes) {
	memmove(pDest, pSrc, NumBytes);
}
#pragma endregion
