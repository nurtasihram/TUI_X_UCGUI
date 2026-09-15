module;

#include "GUIConf.h"

export module TUX;

export import TUX.Types;
export import TUX.Resources;
export import TUX.String;
export import TUX.LCD;

export {

struct PID_STATE : POINT {
	int8_t Pressed;
	PID_STATE(POINT Pos, int8_t Pressed = 0) : POINT(Pos), Pressed(Pressed) {}
	auto operator=(const POINT &p) { x = p.x, y = p.y; }
	bool operator==(const PID_STATE &p) const
	{ return x == p.x && y == p.y && Pressed == p.Pressed; }
	bool operator!=(const PID_STATE &p) const
	{ return !(*this == p); }
};

using DRAWMODE = uint8_t; // 2bits
constexpr DRAWMODE DRAWMODE_NORMAL = 0,
				   DRAWMODE_TRANS  = 1 << 1;

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
	TEXTALIGN_VERTICAL    = 3 << 2;

typedef void GUI_CALLBACK_VOID_P(void *p);
/* Create a memory device which is compatible to the selected LCD */
void GUI_MEMDEV_Draw(RECT r, GUI_CALLBACK_VOID_P *pfDraw, void *pData);

#pragma region Text rendering and wrapping
typedef enum { WRAPMODE_NONE, WRAPMODE_WORD, WRAPMODE_CHAR } WRAPMODE;

int GUI__WrapGetNumCharsDisp(const char *pText, int xSize, WRAPMODE WrapMode);
int GUI__WrapGetNumCharsToNextLine(const char *pText, int xSize, WRAPMODE WrapMode);
int GUI__WrapGetNumBytesToNextLine(const char *pText, int xSize, WRAPMODE WrapMode);

void GUI__DispLine(const char *s, int Len, const RECT *pr);
#pragma endregion

struct GUI_CONTEXT {
	/* Variables in LCD module */
	BRUSH brush{ GUI_DEFAULT_BKCOLOR, GUI_DEFAULT_COLOR };
	RECT rClip;
	/* Variables in GUICHAR module */
	PCFONT pAFont;
	POINT DispPos;
	/* Variables in WM module */
	const RECT *WM__pUserClipRect = nullptr;
	POINT Off;
	/* Variables in MEMDEV module (with memory devices only) */
	LCDDEV *pDeviceAPI;

public:
	auto BkColor() const { return brush.BkColor; }
	void BkColor(RGBC color) { brush.BkColor = color; }
	auto Color() const { return brush.Color; }
	void Color(RGBC color) { brush.Color = color; }
	auto Brush() const { return brush; }
	void Brush(BRUSH colors) { brush = colors; }

	UCFONT Font() const { return *pAFont; } /// 
	PCFONT Font(PCFONT pNewFont) {
		PCFONT pOldFont = pAFont;
		if (pNewFont)
			pAFont = pNewFont;
		return pOldFont;
	}

	void ClipRect(const RECT &r)
	{ rClip = r & pDeviceAPI->GetRect(); }
	void ClipRectMax()
	{ rClip = pDeviceAPI->GetRect(); }

public:
	void DrawRect(RECT r);
	void DrawFocusRect(RECT r, int16_t Dist);
	void ClearRect(RECT r);
	void Clear();
	void FillRect(RECT r);
	void DrawVLine(int16_t x, int16_t y0, int16_t y1);
	void DrawHLine(int16_t y, int16_t x0, int16_t x1);

} GUI;

}
