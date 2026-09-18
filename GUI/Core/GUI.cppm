module;

#include "GUIConf.h"
#include "GUI_X.h"

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
	TEXTALIGN_VERTICAL    = 3 << 2;

constexpr int16_t
	GUI_XMIN = -4095,
	GUI_XMAX =  4095,
	GUI_YMIN = -4095,
	GUI_YMAX =  4095;

typedef void GUI_CALLBACK_VOID_P(void *p);
/* Create a memory device which is compatible to the selected LCD */
void GUI_MEMDEV_Draw(RECT r, GUI_CALLBACK_VOID_P *pfDraw, void *pData);

#pragma region Text rendering and wrapping
typedef enum { WRAPMODE_NONE, WRAPMODE_WORD, WRAPMODE_CHAR } WRAPMODE;

int GUI__WrapGetNumCharsDisp(const char *pText, int xSize, WRAPMODE WrapMode);
int GUI__WrapGetNumCharsToNextLine(const char *pText, int xSize, WRAPMODE WrapMode);
int GUI__WrapGetNumBytesToNextLine(const char *pText, int xSize, WRAPMODE WrapMode);
#pragma endregion

int  GUI__DivideRound(int a, int b);

void GUI_DrawFocusRect(RECT r, int Dist);
void GUI_DrawRect(RECT r);
void GUI_DrawBitmap(PCBITMAP pBM, POINT Pos);

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

void GUI_DispChar(uint16_t c);
void GUI_DispString(const char *s);
void GUI_DispStringAt(const char *s, int x, int y);
void GUI_DispStringInRect(const char *s, const RECT &r, int Flags);
void GUI_DispStringInRectMax(const char *s, RECT r, int TextAlign, int MaxLen); /* Not to be doc. */
void GUI_DispNextLine(void);

void GUI_SelectLCD(void);

bool GUI_Exec();
bool GUI_Exec1();

void GUI_KEY_Store(const KEY_STATE &State);
bool GUI_PollKeyMsg(void);

void GUI_PID_Store(const PID_STATE &State);
PID_STATE GUI_PID_Get(void);

struct GUI_CONTEXT {
	LCDDEV *pDevice = nullptr;
	PCFONT pFont;
	BRUSH brush{ GUI_DEFAULT_BKCOLOR, GUI_DEFAULT_COLOR };
	RECT rClip;
	POINT DispPos, Off;

public:
	void Init() {
		pDevice = GUI_X_GetLCD();
		if (pDevice)
			rClip = pDevice->Rect();
		Font(GUI_DEFAULT_FONT);
		BkColor(GUI_DEFAULT_BKCOLOR);
		Color(GUI_DEFAULT_COLOR);
		ClipRectMax();
	}
	void Select(LCDDEV *pDevice) {
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
	void FillRect(RECT r) {
		r += Off;
		if (r &= rClip)
			pDevice->FillRect(r, Color());
	}
	void Clear(RECT r) {
		r += Off;
		if (r &= rClip)
			pDevice->FillRect(r, BkColor());
	}
	void Clear() {
		DispPos = 0;
		if (auto r = pDevice->Rect(); r &= rClip)
			pDevice->FillRect(r, BkColor());
	}

	void DrawRect(RECT r);
	void DrawFocusRect(RECT r, int16_t Dist);
	void DrawVLine(int16_t x0, int16_t y0, int16_t y1)
	{ FillRect({ x0, y0, x0, y1 }); }
	void DrawHLine(int16_t y0, int16_t x0, int16_t x1)
	{ FillRect({ x0, y0, x1, y0 }); }

} GUI;

void GUI_SaveContext(GUI_CONTEXT *pContext);
void GUI_RestoreContext(const GUI_CONTEXT *pContext);

}
