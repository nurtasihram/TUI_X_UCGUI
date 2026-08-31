#include "GUI_Private.h"
#include "GUIDebug.h"

#include "WM.h"

#if GUI_SUPPORT_TIMER
import TUX.Core.Timer;
#endif

void GUI_Init(void) {
	GUI_X_Init();
	/* Init context */
	/* memset(..,0,..) is not required, as this function is called only at startup of the GUI when data is 0 */
#if GUI_SUPPORT_DEVICES
	GUI.pDeviceAPI = LCD_aAPI[0]; /* &LCD_L0_APIList; */
#endif
	LCD_L0_GetRect(&GUI.ClipRect);
	GUI.Font(GUI_DEFAULT_FONT);
	GUI.BkColor(GUI_DEFAULT_BKCOLOR);
	GUI.Color(GUI_DEFAULT_COLOR);
	GUI.pUC_API = &GUI__API_TableNone;
	LCD_SetClipRectMax();
	LCD_L0_Init();
	WM_Init();
}
void GUI_SelectLCD(void) {
#if GUI_SUPPORT_DEVICES
	GUI.pDeviceAPI = LCD_aAPI[0];
	GUI.pDevData = nullptr;
#endif
	LCD_SetClipRectMax();
	WObj::Activate();
}
int GUI_Exec1(void) {
	int r = 0;
	/* Execute background jobs */
#if GUI_SUPPORT_TIMER
	if (Timer::Exec())
		r = 1; /* We have done something */
#endif
		if (WM_Exec())
		r = 1;
	return r;
}
int GUI_Exec(void) {
	int r = 0;
	while (GUI_Exec1()) {
		r = 1; /* We have done something */
	}
	return r;
}

int GUI_GetTime(void) {
	return GUI_X_GetTime();
}
void GUI_Delay(int Period) {
	int EndTime = GUI_GetTime() + Period;
	int tRem; /* remaining Time */
	while (tRem = EndTime - GUI_GetTime(), tRem > 0) {
		GUI_Exec();
		GUI_X_Delay((tRem > 5) ? 5 : tRem);
	}
}

#pragma region Set/Get Properties
void GUI_SaveContext(GUI_CONTEXT *pContext) {
	*pContext = GUI;
}
void GUI_RestoreContext(const GUI_CONTEXT *pContext) {
	GUI = *pContext;
}
#pragma endregion

#pragma region Draw
void GUI_ClearRect(RECT r) {
	auto color = GUI.Color();
	GUI.Color(GUI.BkColor());
	r += GUI.Off;
	WObj::Iterate(r, [&] {
		LCD_FillRect(r.x0, r.y0, r.x1, r.y1);
	});
	GUI.Color(color);
}
void GUI_Clear(void) {
	GUI.DispPos = 0;
	GUI_ClearRect({ GUI_XMIN, GUI_YMIN, GUI_XMAX, GUI_YMAX });
}
void GUI_FillRect(RECT r) {
	r += GUI.Off;
	WObj::Iterate(r, [&] {
		LCD_FillRect(r.x0, r.y0, r.x1, r.y1);
	});
}
void GUI_DrawRect(RECT r) {
	r += GUI.Off;
	WObj::Iterate(r, [&] {
		LCD_DrawHLine(r.x0, r.y0, r.x1);
		LCD_DrawHLine(r.x0, r.y1, r.x1);
		LCD_DrawVLine(r.x0, r.y0 + 1, r.y1 - 1);
		LCD_DrawVLine(r.x1, r.y0 + 1, r.y1 - 1);
	});
}
void GUI_DrawFocusRect(RECT r, int Dist) {
	r /= Dist;
	r += GUI.Off;
	WObj::Iterate(r, [&] {
		for (int i = r.x0; i <= r.x1; i += 2) {
			LCD_DrawPixel(i, r.y0);
			LCD_DrawPixel(i, r.y1);
		}
		for (int i = r.y0; i <= r.y1; i += 2) {
			LCD_DrawPixel(r.x0, i);
			LCD_DrawPixel(r.x1, i);
		}
	});
}
void GUI_DrawVLine(int x0, int y0, int y1) {
	GUI_FillRect({ x0, y0, x0, y1 });
}
void GUI_DrawHLine(int y0, int x0, int x1) {
	GUI_FillRect({ x0, y0, x1, y0 });	
}

void GUI_DrawBitmap(PCBITMAP pBitmap, int x0, int y0) {
	POINT Pos{ x0, y0 };
	Pos += GUI.Off;
	auto pPal = pBitmap->pPal;
	DRAWMODE PrevDraw = GUI.SetDrawMode(0);  /* No Get... at this point */
	GUI.SetDrawMode((pPal && pPal->HasTrans) ? (PrevDraw | DRAWMODE_TRANS) : PrevDraw & (~DRAWMODE_TRANS));
	auto pTrans = pBitmap->pPal ? pBitmap->pPal->pPalEntries : nullptr;
	if (!pTrans) 
		pTrans = (pBitmap->BitsPerPixel != 1) ? nullptr : &LCD_BKCOLORINDEX;
	RECT r = RECT::LeftTop(Pos, pBitmap->Size);
	WObj::Iterate(r, [&] {
		LCD_DrawBitmap(Pos.x, Pos.y
						, pBitmap->Size.x, pBitmap->Size.y
						, pBitmap->BitsPerPixel
						, pBitmap->BytesPerLine
						, pBitmap->pData
						, pTrans);
	});
	GUI.SetDrawMode(PrevDraw);
}
#pragma endregion



#pragma region Font&String
void GUI_GetTextExtend(RECT *pRect, const char *s, int MaxNumChars) {
	int xMax = 0;
	int NumLines = 0;
	int LineSizeX = 0;
	pRect->x0 = GUI.DispPos.x;
	pRect->y0 = GUI.DispPos.y;
	UCFONT Font = GUI.Font();
	while (MaxNumChars--) {
		auto Char = GUI_UC__GetCharCodeInc(&s);
		if ((Char == '\n') || (Char == 0)) {
			if (LineSizeX > xMax) {
				xMax = LineSizeX;
			}
			LineSizeX = 0;
			NumLines++;
			if (!Char) {
				break;
			}
		}
		else {
			LineSizeX += Font.GetCharSizeX(Char);
		}
	}
	if (LineSizeX > xMax) {
		xMax = LineSizeX;
	}
	if (!NumLines) {
		NumLines = 1;
	}
	pRect->x1 = pRect->x0 + xMax - 1;
	pRect->y1 = pRect->y0 + Font.YSize * NumLines - 1;
}

void GUI__CalcTextRect(const char *pText, const RECT *pTextRectIn, RECT *pTextRectOut, int TextAlign) {
	if (pText) {
		int xPos, yPos, TextWidth, TextHeight;
		/* Calculate X-pos of text */
		TextWidth = GUI_GetStringSizeX(pText);
		switch (TextAlign & TEXTALIGN_HORIZONTAL) {
			case TEXTALIGN_HCENTER:
				xPos = pTextRectIn->x0 + ((pTextRectIn->x1 - pTextRectIn->x0 + 1) - TextWidth) / 2;
				break;
			case TEXTALIGN_RIGHT:
				xPos = pTextRectIn->x1 - TextWidth + 1;
				break;
			default:
				xPos = pTextRectIn->x0;
		}

		/* Calculate Y-pos of text */
		TextHeight = GUI.Font().YSize;
		switch (TextAlign & TEXTALIGN_VERTICAL) {
			case TEXTALIGN_VCENTER:
				yPos = pTextRectIn->y0 + ((pTextRectIn->y1 - pTextRectIn->y0 + 1) - TextHeight) / 2;
				break;
			case TEXTALIGN_BOTTOM:
				yPos = pTextRectIn->y1 - TextHeight + 1;
				break;
			default:
				yPos = pTextRectIn->y0;
		}

		/* Return text rectangle */
		pTextRectOut->x0 = xPos;
		pTextRectOut->y0 = yPos;
		pTextRectOut->x1 = xPos + TextWidth - 1;
		pTextRectOut->y1 = yPos + TextHeight - 1;
	}
	else {
		*pTextRectOut = *pTextRectIn;
	}
}
/*********************************************************************
*
*       GUI_GetLineDistX
*
*  This routine is used to calculate the length of a line in pixels.
*/
int GUI__GetLineSizeX(const char *s, int MaxNumChars) {
	int Dist = 0;
	UCFONT Font = GUI.Font();
	if (s) {
		while (--MaxNumChars >= 0) {
			auto Char = GUI_UC__GetCharCodeInc(&s);
			Dist += Font.GetCharSizeX(Char);
		}
	}
	return Dist;
}
int GUI_GetStringSizeX(const char *s) {
	return GUI__GetLineSizeX(s, GUI__strlen(s));
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
	/* Draw first character if it is valid */
	if (lst.c0 >= 0) {
		auto BytesPerLine = (XSize + 7) >> 3;
		auto BytesPerChar = YSize * BytesPerLine;
		auto DrawMode = GUI.TextMode;
		/* call drawing routine */
		auto OldMode = GUI.SetDrawMode(DrawMode);
		LCD_DrawBitmap(GUI.DispPos.x, GUI.DispPos.y,
						XSize, YSize,
						1, BytesPerLine,
						(const uint8_t *)pData + lst.c0 * BytesPerChar,
						&LCD_BKCOLORINDEX);
		if (lst.c1 >= 0) {
			GUI.SetDrawMode(DrawMode | DRAWMODE_TRANS);
			LCD_DrawBitmap(GUI.DispPos.x, GUI.DispPos.y,
							XSize, YSize,
							1, BytesPerLine,
							(const uint8_t *)pData + lst.c1 * BytesPerChar,
							&LCD_BKCOLORINDEX);
		}
		GUI.SetDrawMode(OldMode);
	}
	GUI.DispPos.x += XSize;
}
void FONT_PROP::DispChar(uint16_t c) const {
	auto pProp = FindChar(c);
	if (!pProp) return;
	auto pCharInfo = pProp->paCharInfo + (c - pProp->First);
	auto OldDrawMode = GUI.SetDrawMode(GUI.TextMode);
	LCD_DrawBitmap(GUI.DispPos.x, GUI.DispPos.y,
				   pCharInfo->XSize, YSize,
				   1, pCharInfo->BytesPerLine,
				   pCharInfo->pData,
				   &LCD_BKCOLORINDEX);
	GUI.SetDrawMode(OldDrawMode); /* Restore draw mode */
	GUI.DispPos.x += pCharInfo->XSize;
}
#pragma endregion

#pragma region Display String
static void _DispLine(const char *s, int MaxNumChars, const RECT *pRect) {
	/* Check if we have anything to do at all ... */
	if (!(*pRect <= GUI.ClipRect))
		return;
	else while (MaxNumChars--) 
		GUI.pAFont->DispChar(GUI_UC__GetCharCodeInc(&s));
}
void GUI__DispLine(const char *s, int MaxNumChars, const RECT *pr) {
	auto r = *pr + GUI.Off;
	WObj::Iterate(r, [&] {
		GUI.DispPos = r.LeftTop();
		_DispLine(s, MaxNumChars, &r);
	});
}
void GUI_DispString(const char *s) {
	if (!s)
		return;
	auto FontSizeY = GUI.pAFont->YSize;
	auto xOrg = GUI.DispPos.x;
	for (; *s; s++) {
		RECT r;
		int LineNumChars = GUI__GetLineNumChars(s, 0x7fff);
		int xLineSize = GUI__GetLineSizeX(s, LineNumChars);
		r.x0 = GUI.DispPos.x;
		r.y0 = GUI.DispPos.y;
		r.x1 = r.x0 + xLineSize - 1;
		r.y1 = r.y0 + FontSizeY - 1;
		GUI__DispLine(s, LineNumChars, &r);
		GUI.DispPos.y = r.y0;
		s += GUI_UC__NumChars2NumBytes(s, LineNumChars);
		if (*s == '\n' || *s == '\r') {
			GUI.DispPos.x = 0;
			if (*s == '\n')
				GUI.DispPos.y += FontSizeY;
		}
		else {
			GUI.DispPos.x = r.x0 + xLineSize;
		}
		if (*s == 0)    /* end of string (last line) reached ? */
			break;
	}
}
void GUI_DispStringAt(const char *s, int x, int y) {
	GUI.DispPos.x = x;
	GUI.DispPos.y = y;
	GUI_DispString(s);
}
void GUI__DispStringInRect(const char *s, RECT *pRect, int TextAlign, int MaxNumChars) {
	RECT r;
	RECT rLine;
	int y = 0;
	auto sOrg = s;
	int FontYSize;
	int xLine = 0;
	int LineLen;
	int NumCharsRem;           /* Number of remaining characters */
	FontYSize = GUI.pAFont->YSize;
	if (pRect) {
		r = *pRect;
	}
	else {
		r = WM_GetClientRect();
	}
	/* handle vertical alignment */
	if ((TextAlign & TEXTALIGN_VERTICAL) == TEXTALIGN_TOP) {
		y = r.y0;
	}
	else {
		int NumLines;
		/* Count the number of lines */
		for (NumCharsRem = MaxNumChars, NumLines = 1; NumCharsRem; NumLines++) {
			LineLen = GUI__GetLineNumChars(s, NumCharsRem);
			NumCharsRem -= LineLen;
			s += GUI_UC__NumChars2NumBytes(s, LineLen);
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
	for (NumCharsRem = MaxNumChars, s = sOrg; NumCharsRem;) {
		int xLineSize;
		LineLen = GUI__GetLineNumChars(s, NumCharsRem);
		NumCharsRem -= LineLen;
		xLineSize = GUI__GetLineSizeX(s, LineLen);
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
		GUI__DispLine(s, LineLen, &rLine);
		s += GUI_UC__NumChars2NumBytes(s, LineLen);
		y += GUI.pAFont->YSize;
		if (GUI__HandleEOLine(&s))
			break;
	}
}

void GUI_DispStringInRectMax(const char *s, RECT *pRect, int TextAlign, int MaxLen) {
	if (s) {
		const RECT *pOldClipRect = nullptr;
		RECT r;

		if (pRect) {
			pOldClipRect = WObj::SetUserClipRect(pRect);
			if (pOldClipRect) {
				r = *pRect;
				r &= *pOldClipRect;
				WObj::SetUserClipRect(&r);
			}
		}
		GUI__DispStringInRect(s, pRect, TextAlign, MaxLen);
		WObj::SetUserClipRect(pOldClipRect);
	}
}

void GUI_DispStringInRect(const char *s, RECT *pRect, int TextAlign) {
	GUI_DispStringInRectMax(s, pRect, TextAlign, 0x7fff);
}
#pragma endregion

#pragma region Display Char
void GUI_DispChar(uint16_t c) {
	RECT r;
	GUI.DispPos += GUI.Off;
	r.x1 = (r.x0 = GUI.DispPos.x) + GUI.Font().GetCharSizeX(c) - 1;
	r.y1 = (r.y0 = GUI.DispPos.y) + GUI.pAFont->YSize - 1;
	WObj::Iterate(r, [&] {
		GL_DispChar(c);
	});
	if (c != '\n') {
		GUI.DispPos.x = r.x1 + 1;
	}
	GUI.DispPos -= GUI.Off;
}

void GUI_DispCharAt(uint16_t c, int16_t x, int16_t y) {
	GUI.DispPos.x = x;
	GUI.DispPos.y = y;
	GUI_DispChar(c);
}
void GUI_DispChars(uint16_t c, int NumChars) {
	while (--NumChars >= 0)
		GUI_DispChar(c);
}

void GUI_DispNextLine(void) {
	GUI.DispPos.y += GUI.pAFont->YSize;
	GUI.DispPos.x = 0;
}
void GL_DispChar(uint16_t c) {
	/* check for control characters */
	if (c == '\n')
		GUI_DispNextLine();
	else if (c != '\r')
		GUI.pAFont->DispChar(c);
}
#pragma endregion

#pragma region Text operators
bool GUI__SetText(char **ppText, const char *s) {
	if (!ppText)
		return false;
	auto size = GUI__strlen(s);
	if (!size) {
		if (*ppText) 
			GUI_ALLOC_Free(*ppText);
		*ppText = nullptr;
		return true;
	}
	auto pText = *ppText = (char *)GUI_ALLOC_Realloc(*ppText, ++size);
	if (!pText)
		return false;
	GUI__memcpy(pText, s, size);
	return true;
}
bool GUI__strcmp(const char *s0, const char *s1) {
	if (s0 == nullptr)
		s0 = "";
	if (s1 == nullptr)
		s1 = "";
	do {
		if (*s0 != *s1)
			return true;
		s1++;
	} while (*++s0);
	if (*s1)
		return true;    /* Not equal, since s1 is longer than s0 */
	return false;      /* Equal ! */
}
int GUI__strlen(const char *s) {
	int r = -1;
	if (s) {
		do {
			r++;
		} while (*s++);
	}
	return r;
}
int GUI__strcpy(char *sDest, const char *sSrc) {
	auto s = sDest;
	while ((*s++ = *sSrc++) != 0) {}
	return (int)(s - sDest - 1);
}
/*********************************************************************
*
*       GUI__HandleEOLine
*
* Is called when processing strings which may consist of
* multiple lines after a line has been processed. It will
* a) Swall the line feed character (if it is there)
* b) Return 1 if end of string, otherwise 0
*/
int GUI__HandleEOLine(const char **ps) {
	auto s = *ps;
	char c = *s++;
	if (c == 0) {
		return 1;
	}
	if (c == '\n') {
		*ps = s;
	}
	return 0;
}

int GUI__GetNumChars(const char *s) {
	int NumChars = 0;
	if (s) {
		while (GUI_UC__GetCharCodeInc(&s)) {
			NumChars++;
		}
	}
	return NumChars;
}
int GUI__GetLineNumChars(const char *s, int MaxNumChars) {
	int NumChars = 0;
	if (s) {
		for (; NumChars < MaxNumChars; NumChars++) {
			auto Data = GUI_UC__GetCharCodeInc(&s);
			if (Data == 0 || Data == '\n')
				break;
		}
	}
	return NumChars;
}

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
		Char = GUI_UC__GetCharCodeInc(&s);   /* Similar to:  *s++ */
		/* Let's first check if the line end is reached. In this case we are done. */
		if (_IsLineEnd(Char)) {
			WordWrap = NumChars;
			break;
		}
		/* If current character is a space, we found a wrap position */
		if ((Char == ' ') && (Char != PrevChar)) {
			WordWrap = NumChars;
		}
		PrevChar = Char;
		xDist += Font.GetCharSizeX(Char);
		if ((xDist <= xSize) || (NumChars == 0)) {
			NumChars++;
		}
		else {
			break;
		}
	}
	if (!WordWrap) {
		WordWrap = NumChars;
	}
	return WordWrap;
}
static int _GetCharWrap(const char *s, int xSize) {
	int xDist = 0, NumChars = 0;
	UCFONT Font = GUI.Font();
	while (uint16_t Char = GUI_UC__GetCharCodeInc(&s)) {
		xDist += Font.GetCharSizeX(Char);
		if ((NumChars && (xDist > xSize)) || (Char == '\n')) 
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
	int r;
	switch (WrapMode) {
		case WRAPMODE_WORD:
			r = _GetWordWrap(pText, xSize);
			break;
		case WRAPMODE_CHAR:
			r = _GetCharWrap(pText, xSize);
			break;
		default:
			r = _GetNoWrap(pText);
	}
	return r;
}
int GUI__WrapGetNumCharsToNextLine(const char *pText, int xSize, WRAPMODE WrapMode) {
	int NumChars;
	uint16_t Char;
	NumChars = GUI__WrapGetNumCharsDisp(pText, xSize, WrapMode);
	pText += GUI_UC__NumChars2NumBytes(pText, NumChars);
	Char = GUI_UC__GetCharCodeInc(&pText);
	if (Char == '\n') {
		NumChars++;
	}
	else {
		if (WrapMode == WRAPMODE_WORD) {
			while (Char == ' ') {
				NumChars++;
				Char = GUI_UC__GetCharCodeInc(&pText);
			}
		}
	}
	return NumChars;
}
int GUI__WrapGetNumBytesToNextLine(const char *pText, int xSize, WRAPMODE WrapMode) {
	int NumChars, NumBytes;
	NumChars = GUI__WrapGetNumCharsToNextLine(pText, xSize, WrapMode);
	NumBytes = GUI_UC__NumChars2NumBytes(pText, NumChars);
	return NumBytes;
}
#pragma endregion
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
	int r = 0;
	if (b) {
		r = ((a + b / 2) / b);
	}
	return r;
}
