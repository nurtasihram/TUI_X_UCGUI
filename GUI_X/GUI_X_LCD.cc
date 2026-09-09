#define DLL_IMPORTS 1
#define AYXANDAR
#include "SimDisp.h"
#include "GUI.h"

#define LCD_XSIZE 320 * 2
#define LCD_YSIZE 240 * 2

Ayxandar Ayx;

static void _SetPixel(int x, int y, RGBC Color) {
	Ayx.Dot({ x, y }, Color);
}
static RGBC _GetPixel(int x, int y) {
	return Ayx.Dot({ x, y });
}

#pragma region LCD API List

template<bool HasTrans, uint8_t BitsPerPixel>
static void  _DrawBitmapBPP(
	uint16_t x0, uint16_t y0,
	uint16_t x1, uint16_t y1,
	const void *pData, uint16_t BytesPerLine,
	uint8_t diff, PCLOGPALETTE pTrans) {
	constexpr uint8_t PixelsPerByte = 8 / BitsPerPixel;
	constexpr uint8_t Mask = 0xFF >> (8 - BitsPerPixel);
	auto pPixels = (const uint8_t *)pData;
	for (auto y = y0; y <= y1; ++y) {
		auto pLine = pPixels;
		auto Pixels = *pLine;
		auto Diff = diff;
		for (auto x = x0; x <= x1; ++x) {
			auto Pixel = Pixels & Mask;
			if constexpr (HasTrans) {
				if (Pixel)
					_SetPixel(x, y, pTrans[Pixel]);
			}
			else
				_SetPixel(x, y, pTrans[Pixel]);
			if (++Diff < PixelsPerByte)
				Pixels >>= BitsPerPixel;
			else {
				Diff = 0;
				Pixels = *++pLine;
			}
		}
		pPixels += BytesPerLine;
	}
}
template<bool HasTrans, uint8_t BitsPerPixel>
void _DrawBitmapBPPP(
	uint16_t x0, uint16_t y0,
	uint16_t x1, uint16_t y1,
	const void *pData, uint16_t BytesPerLine,
	uint8_t, PCLOGPALETTE pTrans) {
	constexpr uint8_t BytesPerPixel = BitsPerPixel / 8;
	constexpr uint32_t Mask = 0xFFFFFFFF & (32 - BitsPerPixel);
	auto pPixel = (const uint8_t *)pData;
	for (auto y = y0; y <= y1; ++y) {
		auto pLine = pPixel;
		for (auto x = x0; x <= x1; ++x) {
			auto Pixel = *(const RGBC *)pLine & Mask;
			if constexpr (HasTrans) {
				if (Pixel)
					_SetPixel(x, y, pTrans[Pixel]);
			}
			else
				_SetPixel(x, y, pTrans[Pixel]);
			pLine += BytesPerPixel;
		}
		pPixel += BytesPerLine;
	}
}

static void _DrawBitmap(BITVIEW b) {
	switch (b.BitsPerPixel) {
	case 1:
		if (GUI.DrawMode & DRAWMODE_TRANS)
			_DrawBitmapBPP<true, 1>(b.x0, b.y0, b.x1, b.y1, b.pData, b.BytesPerLine, b.BitsXOff, b.pPalEntries);
		else
			_DrawBitmapBPP<false, 1>(b.x0, b.y0, b.x1, b.y1, b.pData, b.BytesPerLine, b.BitsXOff, b.pPalEntries);
		break;
	case 2:
		if (GUI.DrawMode & DRAWMODE_TRANS)
			_DrawBitmapBPP<true, 2>(b.x0, b.y0, b.x1, b.y1, b.pData, b.BytesPerLine, b.BitsXOff, b.pPalEntries);
		else
			_DrawBitmapBPP<false, 2>(b.x0, b.y0, b.x1, b.y1, b.pData, b.BytesPerLine, b.BitsXOff, b.pPalEntries);
		break;
	case 4:
		if (GUI.DrawMode & DRAWMODE_TRANS)
			_DrawBitmapBPP<true, 4>(b.x0, b.y0, b.x1, b.y1, b.pData, b.BytesPerLine, b.BitsXOff, b.pPalEntries);
		else
			_DrawBitmapBPP<false, 4>(b.x0, b.y0, b.x1, b.y1, b.pData, b.BytesPerLine, b.BitsXOff, b.pPalEntries);
		break;
	case 8:
		if (GUI.DrawMode & DRAWMODE_TRANS)
			_DrawBitmapBPPP<true, 8>(b.x0, b.y0, b.x1, b.y1, b.pData, b.BytesPerLine, b.BitsXOff, b.pPalEntries);
		else
			_DrawBitmapBPPP<false, 8>(b.x0, b.y0, b.x1, b.y1, b.pData, b.BytesPerLine, b.BitsXOff, b.pPalEntries);
		break;
	case 16:
		if (GUI.DrawMode & DRAWMODE_TRANS)
			_DrawBitmapBPPP<true, 16>(b.x0, b.y0, b.x1, b.y1, b.pData, b.BytesPerLine, b.BitsXOff, b.pPalEntries);
		else
			_DrawBitmapBPPP<false, 16>(b.x0, b.y0, b.x1, b.y1, b.pData, b.BytesPerLine, b.BitsXOff, b.pPalEntries);
		break;
	case 24:
		if (GUI.DrawMode & DRAWMODE_TRANS)
			_DrawBitmapBPPP<true, 24>(b.x0, b.y0, b.x1, b.y1, b.pData, b.BytesPerLine, b.BitsXOff, b.pPalEntries);
		else
			_DrawBitmapBPPP<false, 24>(b.x0, b.y0, b.x1, b.y1, b.pData, b.BytesPerLine, b.BitsXOff, b.pPalEntries);
		break;
	}
}
#pragma endregion

struct LCD_API : public tLCDDEV_APIList {
	LCD_API() : tLCDDEV_APIList(pMEMDEV__APIList24, 24) {}
	RECT GetRect() override {
		return{ 0, 0, LCD_XSIZE - 1, LCD_YSIZE - 1 };
	}
	RGBC GetPixel(int x, int y) override {
		return _GetPixel(x, y);
	}
	void SetPixel(int x, int y, RGBC Color) override {
		_SetPixel(x, y, Color);
	}
	void DrawBitmap(BITVIEW b) override {
		_DrawBitmap(b);
	}
	void FillRect(RECT r) override {
		Ayx.Fill(GUI.Color(), { r.x0, r.y0, r.x1, r.y1 });
	}
} LCD_API;

extern tLCDDEV_APIList *LCD_L0_Init(void) {
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
	return &LCD_API;
}
