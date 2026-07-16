#define DLL_IMPORTS 1
#define AYXANDAR
#include "SimDisp.h"
#include "GUI.h"
#include "GUI_Private.h"

#define LCD_XSIZE 320 * 2
#define LCD_YSIZE 240 * 2

Ayxandar Ayx;

int LCD_GetXSize(void) { return LCD_XSIZE; }
int LCD_GetYSize(void) { return LCD_YSIZE; }

extern void LCD_L0_SetPixel(int x, int y, RGB_COLOR PixelIndex) {
	Ayx.Dot({ x, y }, PixelIndex);
}
extern RGB_COLOR LCD_L0_GetPixel(int x, int y) {
	return Ayx.Dot({ x, y });
}

void LCD_L0_DrawHLine(int x0, int y, int x1) {
	Ayx.Fill(LCD_COLORINDEX, { x0, y, x1, y });
}

void LCD_L0_DrawVLine(int x, int y0, int y1) {
	Ayx.Fill(LCD_COLORINDEX, { x, y0, x, y1 });
}

void LCD_L0_FillRect(int x0, int y0, int x1, int y1) {
	Ayx.Fill(LCD_COLORINDEX, { x0, y0, x1, y1 });
}

extern int  LCD_L0_Init(void) {
	SimDisp::LoadDll(_T("SimClient.dll"));
	//SimDisp::LoadDll(_T("SimDisp.dll"));
	assert(SimDisp::Open(L"TUI By Nurtas Ihram", LCD_XSIZE, LCD_YSIZE));
	SimDisp::SetOnMouse([](int16_t xPos, int16_t yPos, int16_t zPos,
						   tSimDisp_MouseKey mk) {
		if (xPos < 0 || yPos < 0)
			return;
		static GUI_PID_STATE _State;
		_State.x = xPos;
		_State.y = yPos;
		_State.Pressed = mk.Left;
		GUI_PID_StoreState(&_State);
	});
	SimDisp::SetOnDestroy([] {
		ExitProcess(0);
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
	return 0;
}
