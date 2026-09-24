module;

#include "GUIConf.h"

export module TUX;

export import TUX.Types;
export import TUX.Resources;
export import TUX.String;
export import TUX.LCD;

export {

using TEXTALIGN = uint8_t; // 4bits
constexpr TEXTALIGN
/* Text alignment flags, horizontal */
	TEXTALIGN_LEFT        = 0 << 0,
	TEXTALIGN_RIGHT       = 1 << 0,
	TEXTALIGN_HCENTER     = 2 << 0,
	TEXTALIGN_HORIZONTAL  = 3 << 0,
/* Text alignment flags, vertical */
	TEXTALIGN_TOP         = 0 << 2,
	TEXTALIGN_BOTTOM      = 1 << 2,
	TEXTALIGN_VCENTER     = 2 << 2,
	TEXTALIGN_VERTICAL    = 3 << 2,
	
	TEXTALIGN_CENTER      = TEXTALIGN_VCENTER | TEXTALIGN_HCENTER;

template<class T, size_t N>
constexpr uint16_t GUI_COUNTOF(T (&)[N]) { return N; }

#pragma region Text rendering and wrapping
typedef enum { WRAPMODE_NONE, WRAPMODE_WORD, WRAPMODE_CHAR } WRAPMODE;

int GUI__WrapGetNumCharsDisp(const char *pText, int xSize, WRAPMODE WrapMode);
int GUI__WrapGetNumCharsToNextLine(const char *pText, int xSize, WRAPMODE WrapMode);
int GUI__WrapGetNumBytesToNextLine(const char *pText, int xSize, WRAPMODE WrapMode);
#pragma endregion

int  GUI__DivideRound(int a, int b);

void GUI_DrawFocusRect(RECT r, int Dist);
void GUI_DrawRect(RECT r);
void GUI_DrawBitmap(CBITMAP &bm, POINT Pos);

#if GUI_SUPPORT_CURSOR
void     GUI_CURSOR_Activate(void);
void     GUI_CURSOR_Deactivate(void);
void     GUI_CURSOR_SetPosition(int x, int y);
PCCURSOR GUI_CURSOR_Select(PCCURSOR pCursor);
void     GUI_CURSOR_Show(void);
void     GUI_CURSOR_Hide(void);
void     GUI_CURSOR__TempShow(void);
bool     GUI_CURSOR__TempHide(RECT);
#endif

void GUI_DispStringInRect(const char *s, const RECT &r, int Flags);
void GUI_DispStringInRectMax(const char *s, RECT r, int TextAlign, int MaxLen); /* Not to be doc. */

bool GUI_Exec();
bool GUI_Exec1();

void GUI_KEY_Store(const KEY_STATE &State);
bool GUI_PollKeyMsg(void);

void GUI_PID_Store(const PID_STATE &State);
PID_STATE GUI_PID_Get(void);

struct BITVIEW_MEM : BITVIEW {
	BITVIEW_MEM(RECT r = {}, BPP_MODE BitsPerPixel = BPP_DEFAULT) :
		BITVIEW(r, (r.XSize() * BPP_Bits[BitsPerPixel] + 7) >> 3, BitsPerPixel, nullptr) {
		if (BytesPerLine)
			pData = GUI_MEM_Alloc(BytesPerLine * r.YSize());
	}
	~BITVIEW_MEM() { Free(); }
	void Alloc(const RECT &rect, BPP_MODE BitsPerPixel) {
		RECT::operator=(rect);
		BITVIEW::BitsPerPixel = BitsPerPixel;
		if ((BytesPerLine = (XSize() * BPP_Bits[BitsPerPixel] + 7) >> 3))
			pData = GUI_MEM_Realloc(pData, BytesPerLine * YSize());
	}
	void Free() {
		GUI_MEM_Free(pData);
		pData = nullptr;
	}
	uint32_t Dot(POINT Pos) const {
		uint8_t BitsPerPixel = BPP_Bits[this->BitsPerPixel];
		auto pLine = (const uint8_t *)pData + BytesPerLine * Pos.y;
		if (BitsPerPixel < 8) {
			uint8_t Mask = (1u << BitsPerPixel) - 1;
			auto xBits = Pos.x * BitsPerPixel + BitsXOff;
			return pLine[xBits >> 3] >> (xBits & 7) & Mask;
		} else {
			uint8_t BytesPerPixel = BitsPerPixel / 8;
			auto pPixel = pLine + Pos.x * BytesPerPixel;
			if (BytesPerPixel == 1)
				return *pPixel;
			else if (BytesPerPixel == 2)
				return *(const uint16_t *)pPixel;
			else if (BytesPerPixel == 3)
				return *(const RGB24b *)pPixel;
			else if (BytesPerPixel == 4)
				return *(const RGB32b *)pPixel;
		}
		return 0;
	}
	void Dot(POINT Pos, uint32_t color) {
		uint8_t BitsPerPixel = BPP_Bits[this->BitsPerPixel];
		auto pLine = (uint8_t *)pData + BytesPerLine * Pos.y;
		if (BitsPerPixel < 8) {
			uint8_t Mask = (1u << BitsPerPixel) - 1;
			auto xBits = Pos.x * BitsPerPixel + BitsXOff;
			auto xBitsMask = xBits & 7;
			auto pPixel = pLine + (xBits >> 3);
			*pPixel &= ~(Mask << xBitsMask);
			*pPixel |= (color & Mask) << xBitsMask;
		} else {
			uint8_t BytesPerPixel = BitsPerPixel / 8;
			auto pPixel = pLine + Pos.x * BytesPerPixel;
			if (BytesPerPixel == 1)
				*pPixel = color;
			else if (BytesPerPixel == 2)
				*(uint16_t *)pPixel = color;
			else if (BytesPerPixel == 3)
				*(RGB24b *)pPixel = color;
			else if (BytesPerPixel == 4)
				*(RGB32b *)pPixel = color;
		}
	}
};

struct MEMDEV : LCDDEV, BITVIEW_MEM {
public:
	MEMDEV() {}
	MEMDEV(const MEMDEV &) = delete;
	MEMDEV &operator=(const MEMDEV &) = delete;
public:
	RECT Rect() const override { return *this; }
	BPP_MODE BitsPerPixel() const override
	{ return BITVIEW_MEM::BitsPerPixel; }

	RGBC GetPixel(int16_t x, int16_t y) override {
		POINT Pos{ x, y };
		if (!(*this <= Pos))
			return RGB_INVALID;
		Pos -= LeftTop();
		return Dot(Pos);
	}
	void SetPixel(int16_t x, int16_t y, RGBC color) override {
		POINT Pos{ x, y };
		if (!(*this <= Pos))
			return;
		Pos -= LeftTop();
		Dot(Pos, color);
	}
} MemDev;

struct GUI_CONTEXT {
	LCDDEV *pDevice = nullptr;
	PCFONT pFont;
	BRUSH brush{ RGB_BLACK, RGB_WHITE };
	RECT rClip;
	POINT DispPos, Off;

public:
	void Init() {
		pDevice = GUI_X_GetLCD();
		if (pDevice)
			rClip = pDevice->Rect();
		Font(GUI_DEFAULT_FONT);
		BkColor(RGB_BLACK);
		Color(RGB_WHITE);
		ClipRectMax();
	}

public:
	auto BkColor() const { return brush.BkColor; }
	void BkColor(RGBC color) { brush.BkColor = color; }
	auto Color() const { return brush.Color; }
	void Color(RGBC color) { brush.Color = color; }
	auto Brush() const { return brush; }
	void Brush(BRUSH colors) { brush = colors; }

	UCFONT Font() const { return *pFont; } /// 
	PCFONT Font(PCFONT pNewFont) {
		PCFONT pOldFont = pFont;
		if (pNewFont)
			pFont = pNewFont;
		return pOldFont;
	}

	void ClipRect(const RECT &r)
	{ rClip = r & pDevice->Rect(); }
	void ClipRectMax()
	{ rClip = pDevice->Rect(); }

public:
	//void SetBitmap(BITVIEW b) {
	//	if (b &= GUI.rClip)
	//		GUI.pDevice->SetBitmap(b);
	//}
	//void SetBitmap(CBITMAP &bm, POINT Pos) {
	//	Pos += GUI.Off;
	//	auto bmView = bm.At(Pos);
	//	if (bmView &= GUI.rClip) {
	//		CLOGPALETTE aPal{ brush.BkColor, brush.Color };
	//		GUI.pDevice->SetBitmap(
	//			bmView,
	//			bm.pPalEntries ? bm.pPalEntries : bm.BitsPerPixel == BPP_1 ? aPal : nullptr,
	//			bm.IsTrans());
	//	}
	//}

	void rFill(RECT r) {
		r += Off;
		if (r &= rClip)
			pDevice->rFill(r, Color());
	}
	void Clear(RECT r) {
		r += Off;
		if (r &= rClip)
			pDevice->rFill(r, BkColor());
	}
	void Clear() {
		DispPos = 0;
		if (auto r = pDevice->Rect(); r &= rClip)
			pDevice->rFill(r, BkColor());
	}

	void DrawRect(RECT r);
	void DrawFocusRect(RECT r, int16_t Dist);
	void DrawVLine(int16_t x0, int16_t y0, int16_t y1)
	{ rFill({ x0, y0, x0, y1 }); }
	void DrawHLine(int16_t y0, int16_t x0, int16_t x1)
	{ rFill({ x0, y0, x1, y0 }); }

} GUI;

}
