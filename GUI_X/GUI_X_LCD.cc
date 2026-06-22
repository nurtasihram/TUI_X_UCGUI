#define DLL_IMPORTS 1
#define AYXANDAR
#include "SimDisp.h"
#include "GUI.h"
#include "GUI_Private.h"

#define LCD_XSIZE 320
#define LCD_YSIZE 240

Ayxandar Ayx;

int LCD_GetXSize(void) { return LCD_XSIZE; }
int LCD_GetYSize(void) { return LCD_YSIZE; }

extern void LCD_L0_SetPixelIndex(int x, int y, int PixelIndex) {
	Ayx.Dot({x, y}, PixelIndex);
}
extern unsigned int LCD_L0_GetPixelIndex(int x, int y) {
	return Ayx.Dot({x, y});
}
void LCD_L0_XorPixel(int x, int y) {
	LCD_L0_SetPixelIndex(x, y, ~LCD_L0_GetPixelIndex(x, y));
}

void LCD_L0_DrawHLine(int x0, int y, int x1) {
	if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
		for (; x0 <= x1; x0++) {
			LCD_L0_XorPixel(x0, y);
		}
	}
	else {
		for (; x0 <= x1; x0++) {
			LCD_L0_SetPixelIndex(x0, y, LCD_COLORINDEX);
		}
	}
}

void LCD_L0_DrawVLine(int x, int y0, int y1) {
	if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
		for (; y0 <= y1; y0++) {
			LCD_L0_XorPixel(x, y0);
		}
	}
	else {
		for (; y0 <= y1; y0++) {
			LCD_L0_SetPixelIndex(x, y0, LCD_COLORINDEX);
		}
	}
}

void LCD_L0_FillRect(int x0, int y0, int x1, int y1) {
	for (; y0 <= y1; y0++) {
		LCD_L0_DrawHLine(x0, y0, x1);
	}
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
		if (mk.Left) _State.Pressed = 1;
		else _State.Pressed = 0;
		_State.x = xPos;
		_State.y = yPos;
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
