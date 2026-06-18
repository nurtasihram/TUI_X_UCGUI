#define DLL_IMPORTS 1
#define AYXANDAR
#include "SimDisp.h"
#include "GUI.h"

namespace SimDisp {
	Ayxandar Ayx;
}

extern "C" {
void LCD_L0_SetPixelIndex(int x, int y, int PixelIndex);
unsigned int LCD_L0_GetPixelIndex(int x, int y);
int LCD_L0_Init(void);
}

extern void LCD_L0_SetPixelIndex(int x, int y, int PixelIndex) {
	SimDisp::Ayx.Dot({x, y}, PixelIndex);
}
extern unsigned int LCD_L0_GetPixelIndex(int x, int y) {
	return SimDisp::Ayx.Dot({x, y});
}

extern int  LCD_L0_Init(void) {
	SimDisp::LoadDll(_T("SimClient.dll"));
	//SimDisp::LoadDll(_T("SimDisp.dll"));
	assert(SimDisp::Open(L"TUI By Nurtas Ihram", 320, 240));
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
	SimDisp::Ayx.Init();
	return 0;
}
