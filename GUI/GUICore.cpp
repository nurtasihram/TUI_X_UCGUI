#include "GUIConf.h"

import TUX;
import TUX.X;
import TUX.Window;
import TUX.Timer;

bool GUI_Exec1(void) {
	bool r = false;
#if GUI_SUPPORT_TIMER
	r |= Timer::Exec();
#endif
	r |= WObj::Exec();
	return r;
}
bool GUI_Exec() {
	bool r = false;
	while (GUI_Exec1())
		r = true;
	return r;
}

#pragma region Draw
void LCD_SetPixel(int x, int y, RGBC ColorIndex) {
	if (y < GUI.rClip.y0) return;
	if (y > GUI.rClip.y1) return;
	if (x < GUI.rClip.x0) return;
	if (x > GUI.rClip.x1) return;
	GUI.pDevice->SetPixel(x, y, ColorIndex);
}
void LCD_FillRect(RECT r) {
	if (r &= GUI.rClip)
		GUI.pDevice->rFill(r, GUI.Color());
}
void LCD_DrawBitmap(BITVIEW b) {
	if (b &= GUI.rClip)
		GUI.pDevice->SetBitmap(b, nullptr, GUI.BkColor() == RGB_INVALID);
}

void GUI_DrawRect(RECT r) {
	r += GUI.Off;
	LCD_FillRect({ r.x0, r.y0, r.x1, r.y0 });
	LCD_FillRect({ r.x0, r.y1, r.x1, r.y1 });
	LCD_FillRect({ r.x0, r.y0 + 1, r.x0, r.y1 - 1 });
	LCD_FillRect({ r.x1, r.y0 + 1, r.x1, r.y1 - 1 });
}
void GUI_DrawFocusRect(RECT r, int Dist) {
	r /= Dist;
	r += GUI.Off;
	auto color = GUI.Color();
	for (int i = r.x0; i <= r.x1; i += 2) {
		LCD_SetPixel(i, r.y0, color);
		LCD_SetPixel(i, r.y1, color);
	}
	for (int i = r.y0; i <= r.y1; i += 2) {
		LCD_SetPixel(r.x0, i, color);
		LCD_SetPixel(r.x1, i, color);
	}
}
void GUI_DrawBitmap(CBITMAP &bm, POINT Pos) {
	auto bmView = bm.At(Pos);
	CLOGPALETTE aPal{ GUI.BkColor(), GUI.Color() };
	if (bmView &= GUI.rClip)
		GUI.pDevice->SetBitmap(bmView, bm.pPalEntries ? bm.pPalEntries : bm.BitsPerPixel == BPP_1 ? aPal : nullptr, bm.IsTrans());
}
#pragma endregion

#pragma region Font&String
int GUI__GetLineSizeX(const char *s, int MaxNumChars) {
	int Dist = 0;
	UCFONT Font = GUI.Font();
	if (s) {
		while (--MaxNumChars >= 0) {
			auto Char = *s++;
			Dist += Font.CharWidth(Char);
		}
	}
	return Dist;
}
#pragma endregion

#pragma region GUI_Font
void FONT_MONO::DispChar(uint16_t c) const {
	TRANSINFO::LIST lst;
	if (FirstChar <= c && c <= LastChar)
		lst.c0 = c - FirstChar;
	else if (pTrans)
		if (pTrans->FirstChar <= c && c <= pTrans->LastChar)
			lst = pTrans->pList[c - pTrans->FirstChar];
	LOGPALETTE aPal{ GUI.BkColor(), GUI.Color() };
	/* Draw first character if it is valid */
	if (lst.c0 >= 0) {
		uint16_t BytesPerLine = (XSize + 7) >> 3;
		auto BytesPerChar = YSize * BytesPerLine;
		GUI_DrawBitmap(BITMAP{
			{ XSize, YSize },
			BytesPerLine, BPP_1,
			(const uint8_t *)pData + lst.c0 * BytesPerChar, aPal },
			GUI.DispPos);
		if (lst.c1 >= 0) {
			aPal[0] = RGB_INVALID;
			GUI_DrawBitmap(BITMAP{
				{ XSize, YSize },
				BytesPerLine, BPP_1,
				(const uint8_t *)pData + lst.c1 * BytesPerChar, aPal },
				GUI.DispPos);
		}
	}
	GUI.DispPos.x += XSize;
}
void FONT_PROP::DispChar(uint16_t c) const {
	auto pProp = FindChar(c);
	if (!pProp) return;
	auto &ci = pProp->paCharInfo[c - pProp->First];
	CLOGPALETTE aPal{ GUI.BkColor(), GUI.Color() };
	GUI_DrawBitmap(BITMAP{
		{ ci.XSize, YSize },
		ci.BytesPerLine, BPP_1,
		ci.pData, aPal },
		GUI.DispPos);
	GUI.DispPos.x += ci.XSize;
}
#pragma endregion

#pragma region Display String
static void _DispLine(const char *s, int MaxNumChars, RECT r) {
	r += GUI.Off;
	GUI.DispPos = r.LeftTop();
	if (!(r <= GUI.rClip))
		return;
	else while (MaxNumChars--)
		GUI.pFont->DispChar(*s++);
}
void GUI_DispStringInRectMax(const char *pText, RECT r, int TextAlign, int MaxNumChars) {
	if (!pText) return;
	RECT rLine;
	int y = 0;
	auto FontYSize = GUI.pFont->YSize;
	/* handle vertical alignment */
	if ((TextAlign & TEXTALIGN_VERTICAL) == TEXTALIGN_TOP) {
		y = r.y0;
	} else {
		int NumLines = 1;
		auto s = pText;
		/* Count the number of lines */
		for (auto NumCharsRem = MaxNumChars; NumCharsRem; NumLines++) {
			auto LineLen = GUI__GetLineNumChars(s, NumCharsRem);
			NumCharsRem -= LineLen;
			s += LineLen;
			if (GUI__HandleEOLine(&s))
				break;
		}
		/* Do the vertical alignment */
		switch (TextAlign & TEXTALIGN_VERTICAL) {
		case TEXTALIGN_BOTTOM:
			y = r.y1 - NumLines * FontYSize + 1;
			break;
		case TEXTALIGN_VCENTER:
			y = r.y0 + (r.y1 - r.y0 + 1 - NumLines * FontYSize) / 2;
			break;
		}
	}
	/* Output string */
	auto s = pText;
	for (auto NumCharsRem = MaxNumChars; NumCharsRem;) {
		auto LineLen = GUI__GetLineNumChars(s, NumCharsRem);
		NumCharsRem -= LineLen;
		auto xLineSize = GUI__GetLineSizeX(s, LineLen);
		int xLine = 0;
		switch (TextAlign & TEXTALIGN_HORIZONTAL) {
		case TEXTALIGN_HCENTER:
			xLine = r.x0 + (r.x1 - r.x0 - xLineSize) / 2; break;
		case TEXTALIGN_LEFT:
			xLine = r.x0; break;
		case TEXTALIGN_RIGHT:
			xLine = r.x1 - xLineSize + 1;
		}
		rLine.x0 = GUI.DispPos.x = xLine;
		rLine.x1 = rLine.x0 + xLineSize - 1;
		rLine.y0 = GUI.DispPos.y = y;
		rLine.y1 = y + FontYSize - 1;
		_DispLine(s, LineLen, rLine);
		s += LineLen;
		y += GUI.pFont->YSize;
		if (GUI__HandleEOLine(&s))
			break;
	}
}
void GUI_DispStringInRect(const char *s, const RECT &r, int TextAlign) {
	GUI_DispStringInRectMax(s, r, TextAlign, 0x7fff);
}
#pragma endregion

#pragma region Text Wrapping
static int _IsLineEnd(uint16_t Char) {
	if (!Char || (Char == '\n')) {
		return 1;
	}
	return 0;
}
static int _GetWordWrap(const char *s, int xSize) {
	int xDist = 0, NumChars = 0, WordWrap = 0;
	uint16_t Char, PrevChar = 0;
	UCFONT Font = GUI.Font();
	while (1) {
		Char = *s++;   /* Similar to:  *s++ */
		/* Let's first check if the line end is reached. In this case we are done. */
		if (_IsLineEnd(Char)) {
			WordWrap = NumChars;
			break;
		}
		/* If current character is a space, we found a wrap position */
		if (Char == ' ' && Char != PrevChar)
			WordWrap = NumChars;
		PrevChar = Char;
		xDist += Font.CharWidth(Char);
		if (xDist <= xSize || NumChars == 0)
			NumChars++;
		else
			break;
	}
	if (!WordWrap)
		WordWrap = NumChars;
	return WordWrap;
}
static int _GetCharWrap(const char *s, int xSize) {
	int xDist = 0, NumChars = 0;
	UCFONT Font = GUI.Font();
	while (auto Char = *s++) {
		xDist += Font.CharWidth(Char);
		if ((NumChars && xDist > xSize) || Char == '\n') 
			break;
		NumChars++;
	}
	return NumChars;
}
static int _GetNoWrap(const char *s) {
	return GUI__GetLineNumChars(s, 0x7FFF);
}
/*********************************************************************
*
*       GUI__WrapGetNumCharsDisp
*
* Returns:
*  Number of characters to display in the line.
*  Trailing spaces and line end character are
*  not counted
*/
int GUI__WrapGetNumCharsDisp(const char *pText, int xSize, WRAPMODE WrapMode) {
	switch (WrapMode) {
		case WRAPMODE_WORD:
			return _GetWordWrap(pText, xSize);
		case WRAPMODE_CHAR:
			return _GetCharWrap(pText, xSize);
	}
	return _GetNoWrap(pText);
}
int GUI__WrapGetNumCharsToNextLine(const char *pText, int xSize, WRAPMODE WrapMode) {
	auto NumChars = GUI__WrapGetNumCharsDisp(pText, xSize, WrapMode);
	pText += NumChars;
	auto Char = *pText++;
	if (Char == '\n')
		NumChars++;
	else if (WrapMode == WRAPMODE_WORD)
		while (Char == ' ') {
			NumChars++;
			Char = *pText++;
		}
	return NumChars;
}
int GUI__WrapGetNumBytesToNextLine(const char *pText, int xSize, WRAPMODE WrapMode) {
	return GUI__WrapGetNumCharsToNextLine(pText, xSize, WrapMode);
}
#pragma endregion

/*********************************************************************
*
*       GUI__DivideRound
*
* This routine is used internally for computations. Primary goal is
* to minimize the effects of rounding which occur if we simply
* divide.
*/
int GUI__DivideRound(int a, int b) {
	return b ? ((a + b / 2) / b) : 0;
}


#pragma region Key Message Handling
static KEY_STATE _KeyState{ 0 };
uint8_t _KeyStateCnt = 0;
void GUI_KEY_Store(const KEY_STATE &State) {
	_KeyState = State;
	_KeyStateCnt = 1;
}
bool GUI_PollKeyMsg(void) {
	if (!_KeyStateCnt) return false;
	_KeyStateCnt--;
	WObj::OnKey(_KeyState);
	return true;
}
#pragma endregion

#pragma region Touch Input Handling
static PID_STATE _PidState{ 0 };
PID_STATE GUI_PID_Get(void) {
	return _PidState;
}
void GUI_PID_Store(const PID_STATE &State) {
	_PidState = State;
}
#pragma endregion
