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
	GUI.pAFont = GUI_DEFAULT_FONT;
	GUI.SetBkColor(GUI_DEFAULT_BKCOLOR);
	GUI.SetColor(GUI_DEFAULT_COLOR);
	GUI.pUC_API = &GUI__API_TableNone;
	LCD_SetClipRectMax();
	LCD_L0_Init();
	GUI.SetDrawMode(DRAWMODE_REV);
	LCD_FillRect(0, 0, GUI_XMAX, GUI_YMAX);
	GUI.SetDrawMode(0);
	WM_Init();
}
void GUI_SelectLCD(void) {
#if GUI_SUPPORT_DEVICES
	GUI.pDeviceAPI = LCD_aAPI[0];
	GUI.hDevData = 0;
#endif
	LCD_SetClipRectMax();
	WM_Activate();
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


void GUI_GotoXY(int x, int y) {
	GUI.DispPos.x = x;
	GUI.DispPos.y = y;
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
	DRAWMODE PrevDraw = GUI.SetDrawMode(DRAWMODE_REV);
	r += GUI.Off;
	WM_Iterate(r, [&] {
		LCD_FillRect(r.x0, r.y0, r.x1, r.y1);
	});
	GUI.SetDrawMode(PrevDraw);
}
void GUI_Clear(void) {
	GUI_GotoXY(0, 0); /* Reset text cursor to upper left */
	GUI_ClearRect({ GUI_XMIN, GUI_YMIN, GUI_XMAX, GUI_YMAX });
}
void GUI_FillRect(RECT r) {
	r += GUI.Off;
	WM_Iterate(r, [&] {
		LCD_FillRect(r.x0, r.y0, r.x1, r.y1);
	});
}
void GUI_DrawRect(RECT r) {
	r += GUI.Off;
	WM_Iterate(r, [&] {
		LCD_DrawHLine(r.x0, r.y0, r.x1);
		LCD_DrawHLine(r.x0, r.y1, r.x1);
		LCD_DrawVLine(r.x0, r.y0 + 1, r.y1 - 1);
		LCD_DrawVLine(r.x1, r.y0 + 1, r.y1 - 1);
	});
}
void GUI_DrawFocusRect(RECT r, int Dist) {
	r -= Dist;
	r += GUI.Off;
	WM_Iterate(r, [&] {
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
	auto pPal = pBitmap->pPal;
	DRAWMODE PrevDraw = GUI.SetDrawMode(0);  /* No Get... at this point */
	GUI.SetDrawMode((pPal && pPal->HasTrans) ? (PrevDraw | DRAWMODE_TRANS) : PrevDraw & (~DRAWMODE_TRANS));
	auto pTrans = pBitmap->pPal ? pBitmap->pPal->pPalEntries : nullptr;
	if (!pTrans) 
		pTrans = (pBitmap->BitsPerPixel != 1) ? nullptr : &LCD_BKCOLORINDEX;
	RECT r;
	x0 += GUI.Off.x;
	y0 += GUI.Off.y;
	r.x1 = (r.x0 = x0) + pBitmap->XSize - 1;
	r.y1 = (r.y0 = y0) + pBitmap->YSize - 1;
	WM_Iterate(r, [&] {
		LCD_DrawBitmap(x0, y0
						, pBitmap->XSize, pBitmap->YSize
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
	uint16_t Char;
	pRect->x0 = GUI.DispPos.x;
	pRect->y0 = GUI.DispPos.y;
	while (MaxNumChars--) {
		Char = GUI_UC__GetCharCodeInc(&s);
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
			LineSizeX += GUI_GetCharDistX(Char);
		}
	}
	if (LineSizeX > xMax) {
		xMax = LineSizeX;
	}
	if (!NumLines) {
		NumLines = 1;
	}
	pRect->x1 = pRect->x0 + xMax - 1;
	pRect->y1 = pRect->y0 + GUI.pAFont->SizeY() * NumLines - 1;
}

void GUI__CalcTextRect(const char *pText, const RECT *pTextRectIn, RECT *pTextRectOut, int TextAlign) {
	if (pText) {
		int xPos, yPos, TextWidth, TextHeight;

		/* Calculate X-pos of text */
		TextWidth = GUI_GetStringDistX(pText);
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
		TextHeight = GUI.pAFont->DistY();
		switch (TextAlign & TEXTALIGN_VERTICAL) {
			case TEXTALIGN_VCENTER:
				yPos = pTextRectIn->y0 + ((pTextRectIn->y1 - pTextRectIn->y0 + 1) - TextHeight) / 2;
				break;
			case TEXTALIGN_BOTTOM:
				yPos = pTextRectIn->y1 - TextHeight + 1;
				break;
			case TEXTALIGN_BASELINE:
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
int GUI__GetLineDistX(const char *s, int MaxNumChars) {
	int Dist = 0;
	if (s) {
		uint16_t Char;
		if (GUI.pAFont->pafEncode) {
			return GUI.pAFont->pafEncode->pfGetLineDistX(s, MaxNumChars);
		}
		while (--MaxNumChars >= 0) {
			Char = GUI_UC__GetCharCodeInc(&s);
			Dist += GUI_GetCharDistX(Char);
		}
	}
	return Dist;
}
/*********************************************************************
*
*       GUI_GetYAdjust
*
* Returns adjustment in vertical (Y) direction
*
* Note: The return value needs to be subtracted from
*       the y-position of the character.
*/
int GUI_GetYAdjust(void) {
	int r = 0;
	switch (GUI.TextAlign & TEXTALIGN_VERTICAL) {
		case TEXTALIGN_BOTTOM:
			r = GUI.pAFont->YSize - 1;
			break;
		case TEXTALIGN_VCENTER:
			r = GUI.pAFont->YSize / 2;
			break;
		case TEXTALIGN_BASELINE:
			r = GUI.pAFont->YSize / 2;
	}
	return r;
}

int GUI_GetCharDistX(uint16_t c) {
	return GUI.pAFont->GetCharDistX(c);
}
int GUI_GetStringDistX(const char *s) {
	return GUI__GetLineDistX(s, GUI__strlen(s));
}
#pragma endregion

#pragma region GUI_Font
#pragma region Font MONO
void FONT_MONO::DispChar(uint16_t c) const {
	int c0, c1;
	const void *pd;
	int x = GUI.DispPos.x, y = GUI.DispPos.y;
	if (FirstChar <= c && c <= LastChar) {
		pd = pData;
		c0 = c - FirstChar;
		c1 = -1;
	}
	else if (pTrans) {
		pd = pTransData;
		if (pTrans->FirstChar <= c && c <= pTrans->LastChar) {
			c -= pTrans->FirstChar;
			auto ptl = pTrans->pList;
			ptl += c;
			c0 = ptl->c0;
			c1 = ptl->c1;
		}
		else
			c0 = c1 = -1;
	}
	else
		c0 = c1 = -1;
	/* Draw first character if it is valid */
	if (c0 != -1) {
		auto BytesPerChar = YSize * BytesPerLine;
		auto DrawMode = GUI.TextMode;
		/* call drawing routine */
		{
			auto OldMode = GUI.SetDrawMode(DrawMode);
			LCD_DrawBitmap(x, y,
						   XSize, YSize,
						   1, BytesPerLine,
						   (const uint8_t *)pd + c0 * BytesPerChar,
						   &LCD_BKCOLORINDEX);
			if (c1 != -1) {
				GUI.SetDrawMode(DrawMode | DRAWMODE_TRANS);
				LCD_DrawBitmap(x, y,
							   XSize, YSize,
							   1, BytesPerLine,
							   (const uint8_t *)pd + c1 * BytesPerChar,
							   &LCD_BKCOLORINDEX);
			}
			GUI.SetDrawMode(OldMode);
		}
	}
	GUI.DispPos.x += XDist;

}
int FONT_MONO::GetCharDistX(uint16_t c) const {
	return XDist;
}
bool FONT_MONO::IsInFont(uint16_t c) const {
	if (FirstChar <= c && c <= LastChar)
		return true;
	else if (pTrans)
		if (pTrans->FirstChar <= c && c <= pTrans->LastChar)
			return true;
	return false;
}
#pragma endregion
#pragma region Font PROP
const FONT_PROP *FONT_PROP::FindChar(uint16_t c) const {
	for (auto i = this; i; i = i->pNext)
		if (i->First <= c && c <= i->Last)
			return i;
	return nullptr;
}
void FONT_PROP::DispChar(uint16_t c) const {
	auto pProp = FindChar(c);
	if (!pProp)
		return;
	auto pCharInfo = pProp->paCharInfo + (c - pProp->First);
	auto OldDrawMode = GUI.SetDrawMode(GUI.TextMode);
	LCD_DrawBitmap(GUI.DispPos.x, GUI.DispPos.y,
				   pCharInfo->XSize, YSize,
				   1, pCharInfo->BytesPerLine,
				   pCharInfo->pData,
				   &LCD_BKCOLORINDEX);
	GUI.SetDrawMode(OldDrawMode); /* Restore draw mode */
	GUI.DispPos.x += pCharInfo->XDist;
}
int FONT_PROP::GetCharDistX(uint16_t c) const {
	if (auto pProp = FindChar(c))
		return pProp->paCharInfo[c - pProp->First].XDist;
	return 0;
}
bool FONT_PROP::IsInFont(uint16_t c) const {
	return FindChar(c);
}
#pragma endregion
#pragma endregion

#pragma region Display String

static void _DispLine(const char *s, int MaxNumChars, const RECT *pRect) {
	/* Check if we have anything to do at all ... */
	if (!(*pRect <= GUI.ClipRect))
		return;
	if (GUI.pAFont->pafEncode)
		GUI.pAFont->pafEncode->pfDispLine(s, MaxNumChars);
	else while (MaxNumChars--) 
		GUI.pAFont->DispChar(GUI_UC__GetCharCodeInc(&s));
}
void GUI__DispLine(const char *s, int MaxNumChars, const RECT *pr) {
	auto r = *pr + GUI.Off;
	WM_Iterate(r, [&] {
		GUI.DispPos = r.LeftTop();
		_DispLine(s, MaxNumChars, &r);
	});
}

void GUI_DispString(const char *s) {
	int xAdjust, yAdjust, xOrg;
	int FontSizeY;
	if (!s)
		return;
	FontSizeY = GUI.pAFont->DistY();
	xOrg = GUI.DispPos.x;
	/* Adjust vertical position */
	yAdjust = GUI_GetYAdjust();
	GUI.DispPos.y -= yAdjust;
	for (; *s; s++) {
		RECT r;
		int LineNumChars = GUI__GetLineNumChars(s, 0x7fff);
		int xLineSize = GUI__GetLineDistX(s, LineNumChars);
		/* Check if x-position needs to be changed due to h-alignment */
		switch (GUI.TextAlign & TEXTALIGN_HORIZONTAL) {
			case TEXTALIGN_CENTER: xAdjust = xLineSize / 2; break;
			case TEXTALIGN_RIGHT:  xAdjust = xLineSize; break;
			default:            xAdjust = 0;
		}
		r.x0 = GUI.DispPos.x -= xAdjust;
		r.y0 = GUI.DispPos.y;
		r.x1 = r.x0 + xLineSize - 1;
		r.y1 = r.y0 + FontSizeY - 1;
		GUI__DispLine(s, LineNumChars, &r);
		GUI.DispPos.y = r.y0;
		s += GUI_UC__NumChars2NumBytes(s, LineNumChars);
		if ((*s == '\n') || (*s == '\r')) {
			switch (GUI.TextAlign & TEXTALIGN_HORIZONTAL) {
				case TEXTALIGN_CENTER:
				case TEXTALIGN_RIGHT:
					GUI.DispPos.x = xOrg;
					break;
				default:
					GUI.DispPos.x = 0;
					break;
			}
			if (*s == '\n')
				GUI.DispPos.y += FontSizeY;
		}
		else {
			GUI.DispPos.x = r.x0 + xLineSize;
		}
		if (*s == 0)    /* end of string (last line) reached ? */
			break;
	}
	GUI.DispPos.y += yAdjust;
	GUI.TextAlign &= ~TEXTALIGN_HORIZONTAL;

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
	FontYSize = GUI.pAFont->SizeY();
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
			case TEXTALIGN_BASELINE:
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
		xLineSize = GUI__GetLineDistX(s, LineLen);
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
		y += GUI.pAFont->DistY();
		if (GUI__HandleEOLine(&s))
			break;
	}
}

void GUI_DispStringInRectMax(const char *s, RECT *pRect, int TextAlign, int MaxLen) {
	if (s) {
		const RECT *pOldClipRect = nullptr;
		RECT r;

		if (pRect) {
			pOldClipRect = WM_SetUserClipRect(pRect);
			if (pOldClipRect) {
				r = *pRect;
				r &= *pOldClipRect;
				WM_SetUserClipRect(&r);
			}
		}
		GUI__DispStringInRect(s, pRect, TextAlign, MaxLen);
		WM_SetUserClipRect(pOldClipRect);
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
	r.x1 = (r.x0 = GUI.DispPos.x) + GUI_GetCharDistX(c) - 1;
	r.y1 = (r.y0 = GUI.DispPos.y) + GUI.pAFont->SizeY() - 1;
	WM_Iterate(r, [&] {
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
	GUI.DispPos.y += GUI.pAFont->DistY();
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
	if (!s) {
		if (*ppText) 
			GUI_ALLOC_Free(*ppText);
		*ppText = nullptr;
		return true;
	}
	if (!GUI__strcmp(*ppText, s)) 
		return false;	
	auto pNewText = (char *)GUI_ALLOC_AllocNoInit(GUI__strlen(s) + 1);
	if (!pNewText) 
		return false;
	GUI__strcpy(pNewText, s);
	GUI_ALLOC_Free(*ppText);
	*ppText = pNewText;
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
		if (GUI.pAFont->pafEncode) {
			return GUI.pAFont->pafEncode->pfGetLineLen(s, MaxNumChars);
		}
		for (; NumChars < MaxNumChars; NumChars++) {
			uint16_t Data = GUI_UC__GetCharCodeInc(&s);
			if ((Data == 0) || (Data == '\n')) {
				break;
			}
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
		xDist += GUI_GetCharDistX(Char);
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
	uint16_t Char;
	while ((Char = GUI_UC__GetCharCodeInc(&s)) != 0) {
		xDist += GUI_GetCharDistX(Char);
		if ((NumChars && (xDist > xSize)) || (Char == '\n')) {
			break;
		}
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
int GUI__WrapGetNumCharsDisp(const char *pText, int xSize, GUI_WRAPMODE WrapMode) {
	int r;
	switch (WrapMode) {
		case GUI_WRAPMODE_WORD:
			r = _GetWordWrap(pText, xSize);
			break;
		case GUI_WRAPMODE_CHAR:
			r = _GetCharWrap(pText, xSize);
			break;
		default:
			r = _GetNoWrap(pText);
	}
	return r;
}
int GUI__WrapGetNumCharsToNextLine(const char *pText, int xSize, GUI_WRAPMODE WrapMode) {
	int NumChars;
	uint16_t Char;
	NumChars = GUI__WrapGetNumCharsDisp(pText, xSize, WrapMode);
	pText += GUI_UC__NumChars2NumBytes(pText, NumChars);
	Char = GUI_UC__GetCharCodeInc(&pText);
	if (Char == '\n') {
		NumChars++;
	}
	else {
		if (WrapMode == GUI_WRAPMODE_WORD) {
			while (Char == ' ') {
				NumChars++;
				Char = GUI_UC__GetCharCodeInc(&pText);
			}
		}
	}
	return NumChars;
}
int GUI__WrapGetNumBytesToNextLine(const char *pText, int xSize, GUI_WRAPMODE WrapMode) {
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
