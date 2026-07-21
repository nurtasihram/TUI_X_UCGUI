#define  GL_CORE_C

#include "GUI_Private.h"
#include "GUIDebug.h"

#include "WM_GUI.h"
#include "WM.h"

void GUI_Init(void) {
	GUI_X_Init();
	/* Init context */
	/* memset(..,0,..) is not required, as this function is called only at startup of the GUI when data is 0 */
#if GUI_SUPPORT_DEVICES
	GUI_Context.pDeviceAPI = LCD_aAPI[0]; /* &LCD_L0_APIList; */
#endif
	LCD_L0_GetRect(&GUI_Context.ClipRect);
	GUI_Context.pAFont = GUI_DEFAULT_FONT;
	GUI_Context.hAWin = WM_GetDesktopWindow();
	GUI_SetBkColor(GUI_DEFAULT_BKCOLOR);
	GUI_SetColor(GUI_DEFAULT_COLOR);
	GUI_Context.pUC_API = &GUI__API_TableNone;
	LCD_SetClipRectMax();
	LCD_L0_Init();
	GUI_SetDrawMode(DRAWMODE_REV);
	LCD_FillRect(0, 0, GUI_XMAX, GUI_YMAX);
	GUI_SetDrawMode(0);
	WM_Init();
}
void GUI_SelectLCD(void) {
#if GUI_SUPPORT_DEVICES
	GUI_Context.pDeviceAPI = LCD_aAPI[0];
	GUI_Context.hDevData = 0;
#endif
	LCD_SetClipRectMax();
	WM_Activate();
}
int GUI_Exec1(void) {
	int r = 0;
	/* Execute background jobs */
	if (GUI_pfTimerExec) {
		if ((*GUI_pfTimerExec)()) {
			r = 1; /* We have done something */
		}
	}
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

void GUI_GotoXY(int x, int y) {
	GUI_Context.DispPos.x = x;
	GUI_Context.DispPos.y = y;
}

#pragma region Set/Get Properties
void GUI_SetDefault(void) {
	GUI_SetBkColor(GUI_DEFAULT_BKCOLOR);
	GUI_SetColor(GUI_DEFAULT_COLOR);
	GUI_SetTextAlign(0);
	GUI_SetTextMode(0);
	GUI_SetDrawMode(0);
	GUI_SetFont(GUI_DEFAULT_FONT);
}

void GUI_SaveContext(GUI_CONTEXT *pContext) {
	*pContext = GUI_Context;
}
void GUI_RestoreContext(const GUI_CONTEXT *pContext) {
	GUI_Context = *pContext;
}

GUI_DRAWMODE GUI_SetDrawMode(GUI_DRAWMODE dm) {
	GUI_DRAWMODE OldDM = GUI_Context.DrawMode;
	if ((GUI_Context.DrawMode ^ dm) & DRAWMODE_REV) {
		RGBC temp = LCD_BKCOLORINDEX;
		LCD_BKCOLORINDEX = LCD_COLORINDEX;
		LCD_COLORINDEX = temp;
	}
	GUI_Context.DrawMode = dm;
	return OldDM;
}

void GUI_SetTextAlign(int Align) {
	GUI_Context.TextAlign = Align;
}
void GUI_SetTextMode(int Mode) {
	GUI_Context.TextMode = Mode;
}

int GUI_GetTextAlign(void) {
	return GUI_Context.TextAlign;
}
int GUI_GetTextMode(void) {
	return GUI_Context.TextMode;
}

void GUI_SetBkColor(RGBC color) {
	LCD_ACOLORINDEX[(GUI_Context.DrawMode & DRAWMODE_REV) ? 1 : 0] = color;
}
void GUI_SetColor(RGBC color) {
	LCD_ACOLORINDEX[(GUI_Context.DrawMode & DRAWMODE_REV) ? 0 : 1] = color;
}

RGBC GUI_GetBkColor(void) {
	return LCD_BKCOLORINDEX;
}
RGBC GUI_GetColor(void) {
	return LCD_COLORINDEX;
}
#pragma endregion

#pragma region Draw
void GUI_ClearRect(GUI_RECT r) {
	GUI_DRAWMODE PrevDraw = GUI_SetDrawMode(DRAWMODE_REV);
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	WM_ITERATE_START(&r) {
		LCD_FillRect(r.x0, r.y0, r.x1, r.y1);
	} WM_ITERATE_END();
	GUI_SetDrawMode(PrevDraw);
}
void GUI_Clear(void) {
	GUI_GotoXY(0, 0);     /* Reset text cursor to upper left */
	GUI_RECT r = {GUI_XMIN, GUI_YMIN, GUI_XMAX, GUI_YMAX};
	GUI_ClearRect(r);
}
void GUI_FillRect(GUI_RECT r) {
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	WM_ITERATE_START(&r) {
		LCD_FillRect(r.x0, r.y0, r.x1, r.y1);
	} WM_ITERATE_END();
}
void GUI_DrawRect(GUI_RECT r) {
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	WM_ITERATE_START(&r) {
		LCD_DrawHLine(r.x0, r.y0, r.x1);
		LCD_DrawHLine(r.x0, r.y1, r.x1);
		LCD_DrawVLine(r.x0, r.y0 + 1, r.y1 - 1);
		LCD_DrawVLine(r.x1, r.y0 + 1, r.y1 - 1);
	} WM_ITERATE_END();
}
void GUI_DrawFocusRect(GUI_RECT r, int Dist) {
	r -= Dist;
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	WM_ITERATE_START(&r) {
		for (int i = r.x0; i <= r.x1; i += 2) {
			LCD_DrawPixel(i, r.y0);
			LCD_DrawPixel(i, r.y1);
		}
		for (int i = r.y0; i <= r.y1; i += 2) {
			LCD_DrawPixel(r.x0, i);
			LCD_DrawPixel(r.x1, i);
		}
	} WM_ITERATE_END();
}
void GUI_DrawVLine(int x0, int y0, int y1) {
	GUI_RECT r;
	WM_ADDORG(x0, y0);
	WM_ADDORGY(y1);
	r.x1 = r.x0 = x0;
	r.y0 = y0;
	r.y1 = y1;
	WM_ITERATE_START(&r) {
		LCD_DrawVLine(x0, y0, y1);
	} WM_ITERATE_END();

}
void GUI_DrawHLine(int y0, int x0, int x1) {
	GUI_RECT r;
	WM_ADDORG(x0, y0);
	WM_ADDORGX(x1);
	r.x0 = x0;
	r.x1 = x1;
	r.y1 = r.y0 = y0;
	WM_ITERATE_START(&r) {
		LCD_DrawHLine(x0, y0, x1);
	} WM_ITERATE_END();
}

void GUI_DrawBitmap(PCBITMAP pBitmap, int x0, int y0) {
	const GUI_LOGPALETTE *pPal = pBitmap->pPal;
	GUI_DRAWMODE PrevDraw = GUI_SetDrawMode(0);  /* No Get... at this point */
	GUI_SetDrawMode((pPal && pPal->HasTrans) ? (PrevDraw | DRAWMODE_TRANS) : PrevDraw & (~DRAWMODE_TRANS));
	const RGBC *pTrans = pBitmap->pPal ? pBitmap->pPal->pPalEntries : nullptr;
	if (!pTrans) 
		pTrans = (pBitmap->BitsPerPixel != 1) ? nullptr : &LCD_BKCOLORINDEX;
	GUI_RECT r;
	WM_ADDORG(x0, y0);
	r.x1 = (r.x0 = x0) + pBitmap->XSize - 1;
	r.y1 = (r.y0 = y0) + pBitmap->YSize - 1;
	WM_ITERATE_START(&r) {
	LCD_DrawBitmap(x0, y0
					, pBitmap->XSize, pBitmap->YSize
					, pBitmap->BitsPerPixel
					, pBitmap->BytesPerLine
					, pBitmap->pData
					, pTrans);
	} WM_ITERATE_END();
	GUI_SetDrawMode(PrevDraw);
}
#pragma endregion

#pragma region Font&String
PCFONT GUI_GetFont(void) {
	return GUI_Context.pAFont;
}
PCFONT GUI_SetFont(PCFONT pNewFont) {
	PCFONT pOldFont = GUI_Context.pAFont;
	if (pNewFont)
		GUI_Context.pAFont = pNewFont;
	return pOldFont;
}

void GUI_GetTextExtend(GUI_RECT *pRect, const char *s, int MaxNumChars) {
	int xMax = 0;
	int NumLines = 0;
	int LineSizeX = 0;
	uint16_t Char;
	pRect->x0 = GUI_Context.DispPos.x;
	pRect->y0 = GUI_Context.DispPos.y;
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
	pRect->y1 = pRect->y0 + GUI_GetFontSizeY() * NumLines - 1;
}

void GUI__CalcTextRect(const char *pText, const GUI_RECT *pTextRectIn, GUI_RECT *pTextRectOut, int TextAlign) {
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

		/* Calculate Y-pos of text*/
		TextHeight = GUI_GetFontDistY();
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
		if (GUI_Context.pAFont->pafEncode) {
			return GUI_Context.pAFont->pafEncode->pfGetLineDistX(s, MaxNumChars);
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
	switch (GUI_Context.TextAlign & TEXTALIGN_VERTICAL) {
		case TEXTALIGN_BOTTOM:
			r = GUI_Context.pAFont->YSize - 1;
			break;
		case TEXTALIGN_VCENTER:
			r = GUI_Context.pAFont->YSize / 2;
			break;
		case TEXTALIGN_BASELINE:
			r = GUI_Context.pAFont->YSize / 2;
	}
	return r;
}

int GUI_GetFontDistY(void) {
	return GUI_Context.pAFont->YDist;
}
int GUI_GetCharDistX(uint16_t c) {
	return GUI_Context.pAFont->pfGetCharDistX(c);
}
int GUI_GetStringDistX(const char *s) {
	return GUI__GetLineDistX(s, GUI__strlen(s));
}
int GUI_GetYSizeOfFont(PCFONT pFont) {
	return pFont->YSize;
}
int GUI_GetYDistOfFont(PCFONT pFont) {
	return pFont->YDist;
}
int GUI_GetFontSizeY(void) {
	return GUI_Context.pAFont->YSize;
}

void GUI_GetFontInfo(PCFONT pFont, GUI_FONTINFO *pFontInfo) {
	if (pFont == nullptr) {
		pFont = GUI_Context.pAFont;
	}
	pFontInfo->Baseline = pFont->Baseline;
	pFontInfo->CHeight = pFont->CHeight;
	pFontInfo->LHeight = pFont->LHeight;
	pFont->pfGetFontInfo(pFont, pFontInfo);
}

char GUI_IsInFont(PCFONT pFont, uint16_t c) {
	if (pFont == nullptr)
		pFont = GUI_Context.pAFont;
	return pFont->pfIsInFont(pFont, c);
}
#pragma endregion

#pragma region GUI_Font
#pragma region Font MONO
void GUIMONO_DispChar(uint16_t c) {
	int c0, c1;
	const uint8_t *pd;
	int x = GUI_Context.DispPos.x;
	int y = GUI_Context.DispPos.y;
	/* do some checking if drawing is actually necessary ... */
	auto pMono = GUI_Context.pAFont->p.pMono;
	unsigned int FirstChar = pMono->FirstChar;
	/* translate character into 2 characters to display : c0,c1 */
	/* Check if regular character first. */
	if ((c >= (uint16_t)FirstChar) && (c <= (uint16_t)pMono->LastChar)) {
		pd = pMono->pData;
		c0 = ((int)c) - FirstChar;
		c1 = -1;
	}
	else {
		/* Check if character is in translation table */
		FONT_TRANSINFO const *pti = pMono->pTrans;
		pd = pMono->pTransData;
		if (pti) {
			FirstChar = pti->FirstChar;
			if ((c >= (uint16_t)FirstChar) && (c <= (uint16_t)pti->LastChar)) {
				FONT_TRANSLIST const *ptl;
				c -= pti->FirstChar;
				ptl = pti->pList;
				ptl += c;
				c0 = ptl->c0;
				c1 = ptl->c1;
			}
			else {
				c0 = c1 = -1;
			}
		}
		else {
			c0 = c1 = -1;
		}
	}
	/* Draw first character if it is valid */
	if (c0 != -1) {
		int BytesPerChar = GUI_Context.pAFont->YSize * pMono->BytesPerLine;
		GUI_DRAWMODE DrawMode;
		int XSize = pMono->XSize;
		int YSize = GUI_Context.pAFont->YSize;
		/* Select the right drawing mode */
		DrawMode = GUI_Context.TextMode;
		/* call drawing routine */
		{
			uint8_t OldMode = GUI_SetDrawMode(DrawMode);
			LCD_DrawBitmap(x, y,
						   XSize, YSize,
						   1,     /* Bits per Pixel */
						   pMono->BytesPerLine,
						   pd + c0 * BytesPerChar,
						   &LCD_BKCOLORINDEX
			);
			if (c1 != -1) {
				GUI_SetDrawMode(DrawMode | DRAWMODE_TRANS);
				LCD_DrawBitmap(x, y,
							   XSize, YSize,
							   1,     /* Bits per Pixel */
							   pMono->BytesPerLine,
							   pd + c1 * BytesPerChar,
							   &LCD_BKCOLORINDEX
				);
			}
			/* Fill empty pixel lines */
			if (GUI_Context.pAFont->YDist > GUI_Context.pAFont->YSize) {
				if (DrawMode != DRAWMODE_TRANS) {
					GUI_SetDrawMode(DrawMode ^ DRAWMODE_REV);  /* Reverse so we can fill with BkColor */
					LCD_FillRect(x,
								 y + GUI_Context.pAFont->YSize * GUI_Context.pAFont->YDist,
								 x + XSize,
								 y + GUI_Context.pAFont->YDist);
				}
			}
			GUI_SetDrawMode(OldMode);
		}
	}
	GUI_Context.DispPos.x += pMono->XDist;

}

int GUIMONO_GetCharDistX(uint16_t c) {
	auto pMono = GUI_Context.pAFont->p.pMono;
	GUI_USE_PARA(c);
	return pMono->XDist;

}

void GUIMONO_GetFontInfo(PCFONT pFont, GUI_FONTINFO *pfi) {
	GUI_USE_PARA(pFont);
	pfi->Flags = GUI_FONTINFO_FLAG_MONO;
}

char GUIMONO_IsInFont(PCFONT pFont, uint16_t c) {
	auto pMono = pFont->p.pMono;
	unsigned int FirstChar = pMono->FirstChar;
	/* Check if regular character first. */
	if ((c >= (uint16_t)FirstChar) && (c <= (uint16_t)pMono->LastChar)) {
		return 1;  /* Yes, we have it ! */
	}
	else {
		/* Check if character is in translation table */
		FONT_TRANSINFO const *pti = pMono->pTrans;
		if (pti) {
			if ((c >= pti->FirstChar) && (c <= pti->LastChar)) {
				return 1;  /* Yes, we have it ! */
			}
		}
	}
	return 0;  /* No, we can not display this character */
}
#pragma endregion

#pragma region Font PROP
static const FONT_PROP *GUIPROP_FindChar(const FONT_PROP *pProp, uint16_t c) {
	for (; pProp; pProp = pProp->pNext) {
		if ((c >= pProp->First) && (c <= pProp->Last))
			break;
	}
	return pProp;
}

void GUIPROP_DispChar(uint16_t c) {
	int BytesPerLine;

	GUI_DRAWMODE DrawMode = GUI_Context.TextMode;
	auto pProp = GUIPROP_FindChar(GUI_Context.pAFont->p.pProp, c);
	if (pProp) {
		GUI_DRAWMODE OldDrawMode;
		const GUI_CHARINFO *pCharInfo = pProp->paCharInfo + (c - pProp->First);
		BytesPerLine = pCharInfo->BytesPerLine;
		OldDrawMode = GUI_SetDrawMode(DrawMode);
		LCD_DrawBitmap(GUI_Context.DispPos.x, GUI_Context.DispPos.y,
					   pCharInfo->XSize, GUI_Context.pAFont->YSize,
					   1,     /* Bits per Pixel */
					   BytesPerLine,
					   pCharInfo->pData,
					   &LCD_BKCOLORINDEX
		);
		/* Fill empty pixel lines */
		if (GUI_Context.pAFont->YDist > GUI_Context.pAFont->YSize) {
			int YDist = GUI_Context.pAFont->YDist;
			int YSize = GUI_Context.pAFont->YSize;
			if (DrawMode != DRAWMODE_TRANS) {
				RGBC OldColor = GUI_GetColor();
				GUI_SetColor(GUI_GetBkColor());
				LCD_FillRect(GUI_Context.DispPos.x,
							 GUI_Context.DispPos.y + YSize,
							 GUI_Context.DispPos.x + pCharInfo->XSize,
							 GUI_Context.DispPos.y + YDist);
				GUI_SetColor(OldColor);
			}
		}
		GUI_SetDrawMode(OldDrawMode); /* Restore draw mode */
		GUI_Context.DispPos.x += pCharInfo->XDist;
	}
}

int GUIPROP_GetCharDistX(uint16_t c) {
	auto pProp = GUIPROP_FindChar(GUI_Context.pAFont->p.pProp, c);
	return (pProp) ? (pProp->paCharInfo + (c - pProp->First))->XSize : 0;
}

void GUIPROP_GetFontInfo(PCFONT pFont, GUI_FONTINFO *pfi) {
	GUI_USE_PARA(pFont);
	pfi->Flags = GUI_FONTINFO_FLAG_PROP;
}

char GUIPROP_IsInFont(PCFONT pFont, uint16_t c) {
	auto pProp = GUIPROP_FindChar(pFont->p.pProp, c);
	return (pProp == nullptr) ? 0 : 1;
}
#pragma endregion
#pragma endregion

#pragma region Display String

static void _DispLine(const char *s, int MaxNumChars, const GUI_RECT *pRect) {
	/* Check if we have anything to do at all ... */
	if (!GUI_RectsIntersect(&GUI_Context.ClipRect, pRect))
		return;
	if (GUI_Context.pAFont->pafEncode)
		GUI_Context.pAFont->pafEncode->pfDispLine(s, MaxNumChars);
	else while (MaxNumChars--) 
		GUI_Context.pAFont->pfDispChar(GUI_UC__GetCharCodeInc(&s));
}
void GUI__DispLine(const char *s, int MaxNumChars, const GUI_RECT *pr) {
	GUI_RECT r;
	r = *pr;
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	WM_ITERATE_START(&r) {
		GUI_Context.DispPos.x = r.x0;
		GUI_Context.DispPos.y = r.y0;
		/* Do the actual drawing via routine call. */
		_DispLine(s, MaxNumChars, &r);
	} WM_ITERATE_END();
	WM_SUBORG(GUI_Context.DispPos.x, GUI_Context.DispPos.y);
}

void GUI_DispString(const char *s) {
	int xAdjust, yAdjust, xOrg;
	int FontSizeY;
	if (!s)
		return;

	FontSizeY = GUI_GetFontDistY();
	xOrg = GUI_Context.DispPos.x;
	/* Adjust vertical position */
	yAdjust = GUI_GetYAdjust();
	GUI_Context.DispPos.y -= yAdjust;
	for (; *s; s++) {
		GUI_RECT r;
		int LineNumChars = GUI__GetLineNumChars(s, 0x7fff);
		int xLineSize = GUI__GetLineDistX(s, LineNumChars);
		/* Check if x-position needs to be changed due to h-alignment */
		switch (GUI_Context.TextAlign & TEXTALIGN_HORIZONTAL) {
			case TEXTALIGN_CENTER: xAdjust = xLineSize / 2; break;
			case TEXTALIGN_RIGHT:  xAdjust = xLineSize; break;
			default:            xAdjust = 0;
		}
		r.x0 = GUI_Context.DispPos.x -= xAdjust;
		r.x1 = r.x0 + xLineSize - 1;
		r.y0 = GUI_Context.DispPos.y;
		r.y1 = r.y0 + FontSizeY - 1;
		GUI__DispLine(s, LineNumChars, &r);
		GUI_Context.DispPos.y = r.y0;
		s += GUI_UC__NumChars2NumBytes(s, LineNumChars);
		if ((*s == '\n') || (*s == '\r')) {
			switch (GUI_Context.TextAlign & TEXTALIGN_HORIZONTAL) {
				case TEXTALIGN_CENTER:
				case TEXTALIGN_RIGHT:
					GUI_Context.DispPos.x = xOrg;
					break;
				default:
					GUI_Context.DispPos.x = 0;
					break;
			}
			if (*s == '\n')
				GUI_Context.DispPos.y += FontSizeY;
		}
		else {
			GUI_Context.DispPos.x = r.x0 + xLineSize;
		}
		if (*s == 0)    /* end of string (last line) reached ? */
			break;
	}
	GUI_Context.DispPos.y += yAdjust;
	GUI_Context.TextAlign &= ~TEXTALIGN_HORIZONTAL;

}
void GUI_DispStringAt(const char *s, int x, int y) {
	GUI_Context.DispPos.x = x;
	GUI_Context.DispPos.y = y;
	GUI_DispString(s);
}
void GUI__DispStringInRect(const char *s, GUI_RECT *pRect, int TextAlign, int MaxNumChars) {
	GUI_RECT r;
	GUI_RECT rLine;
	int y = 0;
	const char *sOrg = s;
	int FontYSize;
	int xLine = 0;
	int LineLen;
	int NumCharsRem;           /* Number of remaining characters */
	FontYSize = GUI_GetFontSizeY();
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
		rLine.x0 = GUI_Context.DispPos.x = xLine;
		rLine.x1 = rLine.x0 + xLineSize - 1;
		rLine.y0 = GUI_Context.DispPos.y = y;
		rLine.y1 = y + FontYSize - 1;
		GUI__DispLine(s, LineLen, &rLine);
		s += GUI_UC__NumChars2NumBytes(s, LineLen);
		y += GUI_GetFontDistY();
		if (GUI__HandleEOLine(&s))
			break;
	}
}

void GUI_DispStringInRectMax(const char *s, GUI_RECT *pRect, int TextAlign, int MaxLen) {
	if (s) {
		const GUI_RECT *pOldClipRect = nullptr;
		GUI_RECT r;

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

void GUI_DispStringInRect(const char *s, GUI_RECT *pRect, int TextAlign) {
	GUI_DispStringInRectMax(s, pRect, TextAlign, 0x7fff);
}
#pragma endregion

#pragma region Display Char
void GUI_DispChar(uint16_t c) {
	GUI_RECT r;
	WM_ADDORG(GUI_Context.DispPos.x, GUI_Context.DispPos.y);
	r.x1 = (r.x0 = GUI_Context.DispPos.x) + GUI_GetCharDistX(c) - 1;
	r.y1 = (r.y0 = GUI_Context.DispPos.y) + GUI_GetFontSizeY() - 1;
	WM_ITERATE_START(&r) {
		GL_DispChar(c);
	} WM_ITERATE_END();
	if (c != '\n') {
		GUI_Context.DispPos.x = r.x1 + 1;
	}
	WM_SUBORG(GUI_Context.DispPos.x, GUI_Context.DispPos.y);
}

void GUI_DispCharAt(uint16_t c, int16_t x, int16_t y) {
	GUI_Context.DispPos.x = x;
	GUI_Context.DispPos.y = y;
	GUI_DispChar(c);
}
void GUI_DispChars(uint16_t c, int NumChars) {
	while (--NumChars >= 0)
		GUI_DispChar(c);
}

void GUI_DispNextLine(void) {
	GUI_Context.DispPos.y += GUI_GetFontDistY();
	GUI_Context.DispPos.x = 0;
}
void GL_DispChar(uint16_t c) {
	/* check for control characters */
	if (c == '\n')
		GUI_DispNextLine();
	else if (c != '\r')
		GUI_Context.pAFont->pfDispChar(c);
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
	char *s = sDest;
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
	const char *s = *ps;
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
		if (GUI_Context.pAFont->pafEncode) {
			return GUI_Context.pAFont->pafEncode->pfGetLineLen(s, MaxNumChars);
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

#pragma region Rect operators
int GUI_RectsIntersect(const GUI_RECT *pr0, const GUI_RECT *pr1) {
	if (pr0->y0 <= pr1->y1) {
		if (pr1->y0 <= pr0->y1) {
			if (pr0->x0 <= pr1->x1) {
				if (pr1->x0 <= pr0->x1) {
					return 1;
				}
			}
		}
	}
	return 0;
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
	int r = 0;
	if (b) {
		r = ((a + b / 2) / b);
	}
	return r;
}
