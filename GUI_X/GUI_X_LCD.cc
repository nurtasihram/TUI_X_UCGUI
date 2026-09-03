#define DLL_IMPORTS 1
#define AYXANDAR
#include "SimDisp.h"
#include "GUI.h"

#define LCD_XSIZE 320 * 2
#define LCD_YSIZE 240 * 2

Ayxandar Ayx;

static RECT _GetRect() {
	return{ 0, 0, LCD_XSIZE - 1, LCD_YSIZE - 1 };
}

static void _SetPixel(int x, int y, RGBC Color) {
	Ayx.Dot({ x, y }, Color);
}
static RGBC _GetPixel(int x, int y) {
	return Ayx.Dot({ x, y });
}

static void _FillRect(RECT r) {
	Ayx.Fill(GUI.Color(), { r.x0, r.y0, r.x1, r.y1 });
}

#pragma region LCD API List
static void  _DrawBitLine1BPP(int x, int y, uint8_t const *p, int Diff, int xsize, PCLOGPALETTE pTrans) {
	RGBC Index0 = pTrans[0], Index1 = pTrans[1];
	if (GUI.DrawMode & DRAWMODE_TRANS)
		do {
			if (*p & (1 << Diff))
				_SetPixel(x, y, Index1);
			x++;
			if (++Diff == 8) {
				Diff = 0;
				p++;
			}
		} while (--xsize);
	else do {
		_SetPixel(x++, y, (*p & (1 << Diff)) ? Index1 : Index0);
		if (++Diff == 8) {
			Diff = 0;
			p++;
		}
	} while (--xsize);
}
static void  _DrawBitLine2BPP(int x, int y, uint8_t const *p, int Diff, int xsize, PCLOGPALETTE pTrans) {
	RGBC Pixels = *p;
	int CurrentPixel = Diff;
	if (GUI.DrawMode & DRAWMODE_TRANS) {
		if (pTrans)
			do {
				int Shift = (3 - CurrentPixel) << 1;
				int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
				if (Index) {
					RGBC PixelIndex = *(pTrans + Index);
					_SetPixel(x, y, PixelIndex);
				}
				x++;
				if (++CurrentPixel == 4) {
					CurrentPixel = 0;
					Pixels = *(++p);
				}
			} while (--xsize);
		else do {
			int Shift = (3 - CurrentPixel) << 1;
			int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
			if (Index) {
				_SetPixel(x, y, Index);
			}
			x++;
			if (++CurrentPixel == 4) {
				CurrentPixel = 0;
				Pixels = *(++p);
			}
		} while (--xsize);
	}
	else if (pTrans)
		do {
			int Shift = (3 - CurrentPixel) << 1;
			int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
			RGBC PixelIndex = *(pTrans + Index);
			_SetPixel(x++, y, PixelIndex);
			if (++CurrentPixel == 4) {
				CurrentPixel = 0;
				Pixels = *(++p);
			}
		} while (--xsize);
	else do {
		int Shift = (3 - CurrentPixel) << 1;
		int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
		_SetPixel(x++, y, Index);
		if (++CurrentPixel == 4) {
			CurrentPixel = 0;
			Pixels = *(++p);
		}
	} while (--xsize);
}
static void  _DrawBitLine4BPP(int x, int y, uint8_t const *p, int Diff, int xsize, PCLOGPALETTE pTrans) {
	RGBC Pixels = *p;
	int CurrentPixel = Diff;
	if (GUI.DrawMode & DRAWMODE_TRANS)
		if (pTrans)
			do {
				int Shift = (1 - CurrentPixel) << 2;
				int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
				if (Index) {
					RGBC PixelIndex = *(pTrans + Index);
					_SetPixel(x, y, PixelIndex);
				}
				x++;
				if (++CurrentPixel == 2) {
					CurrentPixel = 0;
					Pixels = *(++p);
				}
			} while (--xsize);
		else do {
			int Shift = (1 - CurrentPixel) << 2;
			int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
			if (Index)
				_SetPixel(x, y, Index);
			x++;
			if (++CurrentPixel == 2) {
				CurrentPixel = 0;
				Pixels = *(++p);
			}
		} while (--xsize);
	else if (pTrans)
		do {
			int Shift = (1 - CurrentPixel) << 2;
			int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
			RGBC PixelIndex = *(pTrans + Index);
			_SetPixel(x++, y, PixelIndex);
			if (++CurrentPixel == 2) {
				CurrentPixel = 0;
				Pixels = *(++p);
			}
		} while (--xsize);
	else do {
		int Shift = (1 - CurrentPixel) << 2;
		int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
		_SetPixel(x++, y, Index);
		if (++CurrentPixel == 2) {
			CurrentPixel = 0;
			Pixels = *(++p);
		}
	} while (--xsize);

}

static void  _DrawBitLine8BPP(int x, int y, uint8_t const *p, int xsize, PCLOGPALETTE pTrans) {
	RGBC Pixel;
	if (GUI.DrawMode & DRAWMODE_TRANS) {
		if (pTrans)
			for (; xsize--; x++, p++) {
				Pixel = *p;
				if (Pixel)
					_SetPixel(x, y, *(pTrans + Pixel));
			}
		else for (; xsize--; x++, p++) {
			Pixel = *p;
			if (Pixel)
				_SetPixel(x, y, Pixel);
		}
	}
	else if (pTrans)
		for (; xsize--; x++, p++)
			_SetPixel(x, y, pTrans[*p]);
	else for (; xsize--; x++, p++)
		_SetPixel(x, y, *p);
}
static void  DrawBitLine16BPP(int x, int y, uint16_t const *p, int xsize, PCLOGPALETTE pTrans) {
	RGBC pixel;
	if (GUI.DrawMode & DRAWMODE_TRANS)
		if (pTrans)
			for (; xsize--; x++, p++) {
				pixel = *p;
				if (pixel)
					_SetPixel(x, y, *(pTrans + pixel));
			}
		else for (; xsize--; x++, p++) {
			pixel = *p;
			if (pixel)
				_SetPixel(x, y, pixel);
		}
	else if (pTrans)
		for (; xsize--; x++, p++) {
			pixel = *p;
			_SetPixel(x, y, *(pTrans + pixel));
		}
	else for (; xsize--; x++, p++)
		_SetPixel(x, y, *p);
}

static void _DrawBitLine24BPP(int x, int y, RGBC const *p, int xsize, PCLOGPALETTE pTrans) {
	RGBC pixel;
	if (GUI.DrawMode & DRAWMODE_TRANS)
		for (; xsize--; x++, p++) {
			pixel = *p;
			if (pixel)
				_SetPixel(x, y, pixel);
		}
	else for (; xsize--; x++, p++)
		_SetPixel(x, y, *p);
}

static void _DrawBitmap(BITVIEW b) {
	
	auto x0 = b.x0, y0 = b.y0;
	auto xsize = b.XSize(), ysize = b.YSize();
	auto pData = (const uint8_t *)b.pData;
	auto pTrans = b.pPalEntries;
	auto Diff = b.BitsXOff;
	auto BytesPerLine = b.BytesPerLine;
	auto BitsPerPixel = b.BitsPerPixel;
	int y1 = y0 + ysize;

	switch (b.BitsPerPixel) {
	case 1:
		for (; y0 < y1; y0++, pData += BytesPerLine)
			_DrawBitLine1BPP(x0, y0, pData, Diff, xsize, pTrans);
		break;
	case 2:
		for (; y0 < y1; y0++, pData += BytesPerLine)
			_DrawBitLine2BPP(x0, y0, pData, Diff, xsize, pTrans);
		break;
	case 4:
		for (; y0 < y1; y0++, pData += BytesPerLine)
			_DrawBitLine4BPP(x0, y0, pData, Diff, xsize, pTrans);
		break;
	case 8:
		for (; y0 < y1; y0++, pData += BytesPerLine)
			_DrawBitLine8BPP(x0, y0, pData, xsize, pTrans);
		break;
	case 16:
		for (; y0 < y1; y0++, pData += BytesPerLine)
			DrawBitLine16BPP(x0, y0, (const uint16_t *)pData, xsize, pTrans);
		break;
	case 24:
		for (; y0 < y1; y0++, pData += BytesPerLine)
			_DrawBitLine24BPP(x0, y0, (PCLOGPALETTE)pData, xsize, pTrans);
		break;
	}
}

#pragma endregion

const tLCDDEV_APIList LCD_API{
	_DrawBitmap,
	_FillRect,
	_GetPixel,
	_SetPixel,
	_GetRect,
	24,
#if GUI_SUPPORT_MEMDEV
	& GUI_MEMDEV__APIList24
#endif
};

extern bool LCD_L0_Init(void) {
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
		GUI_PID_StoreState({ { xPos, yPos }, mk.Left });
	});
	SimDisp::SetOnKey([](uint16_t Key, uint8_t Pressed) {
		GUI_StoreKeyMsg(Key, Pressed);
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
	return true;
}
