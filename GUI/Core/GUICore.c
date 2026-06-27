#include <stddef.h>           /* needed for definition of NULL */

#define  GL_CORE_C

#include "GUI_Private.h"
#include "GUIDebug.h"

#include "WM_GUI.h"
#include "WM.h"

int GUI_Init(void) {
	int r;
	GUI_X_Init();
	/* Init context */
	/* memset(..,0,..) is not required, as this function is called only at startup of the GUI when data is 0 */
#if GUI_SUPPORT_DEVICES
	GUI_Context.pDeviceAPI = LCD_aAPI[0]; /* &LCD_L0_APIList; */
#endif
	GUI_Context.pClipRect_HL = &GUI_Context.ClipRect;
	LCD_L0_GetRect(&GUI_Context.ClipRect);
	GUI_Context.pAFont = GUI_DEFAULT_FONT;
	GUI_Context.pClipRect_HL = &GUI_Context.ClipRect;
	GUI_Context.hAWin = WM_GetDesktopWindow();
	GUI_Context.Color = GUI_INVALID_COLOR;
	GUI_Context.BkColor = GUI_INVALID_COLOR;
	GUI_SetBkColor(GUI_DEFAULT_BKCOLOR);
	GUI_SetColor(GUI_DEFAULT_COLOR);
	GUI_Context.pUC_API = &GUI__API_TableNone;
	r = LCD_Init();
	WM_Init();
	return r;
}
void GUI_SelectLCD(void) {
#if GUI_SUPPORT_DEVICES
	GUI_Context.pDeviceAPI = LCD_aAPI[0];
	GUI_Context.hDevData = 0;
#endif
	GUI_Context.pClipRect_HL = &GUI_Context.ClipRect;
	LCD_SetClipRectMax();
	GUI_SetColor(GUI_Context.Color);
	GUI_SetBkColor(GUI_Context.BkColor);
	WM_Activate();
}
int GUI_Exec1(void) {
	int r = 0;
	/* Execute background jobs */
	if (GUI_pfTimerExec) {
		if ((*GUI_pfTimerExec)()) {
			r = 1;                  /* We have done something */
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
	GUI_Context.DispPosX = x;
	GUI_Context.DispPosY = y;
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

int GUI_SetLBorder(int x) {
	int r;
	r = GUI_Context.LBorder;
	GUI_Context.LBorder = x;
	return r;
}

GUI_DRAWMODE GUI_SetDrawMode(GUI_DRAWMODE dm) {
	GUI_DRAWMODE OldMode;
	OldMode = LCD_SetDrawMode(dm);
	return OldMode;
}

int GUI_SetTextAlign(int Align) {
	int r;
	r = GUI_Context.TextAlign;
	GUI_Context.TextAlign = Align;
	return r;
}
int GUI_GetTextAlign(void) {
	return GUI_Context.TextAlign;
}

int GUI_SetTextMode(int Mode) {
	int r;
	r = GUI_Context.TextMode;
	GUI_Context.TextMode = Mode;
	return r;
}
int GUI_GetTextMode(void) {
	return GUI_Context.TextMode;
}

static int _GetColorIndex(int i)  /* i is 0 or 1 */ {
	return  (GUI_Context.DrawMode & DRAWMODE_REV) ? i - 1 : i;
}
void GUI_SetBkColor(RGB_COLOR color) {
	LCD_ACOLORINDEX[_GetColorIndex(0)] = color;
}
void GUI_SetColor(RGB_COLOR color) {
	LCD_ACOLORINDEX[_GetColorIndex(1)] = color;
}

RGB_COLOR GUI_GetBkColor(void) {
	return LCD_BKCOLORINDEX;
}
RGB_COLOR GUI_GetColor(void) {
	return LCD_COLORINDEX;
}
#pragma endregion

#pragma region Draw
void GUI_ClearRect(int x0, int y0, int x1, int y1) {
	GUI_DRAWMODE PrevDraw;
	GUI_RECT r;
	PrevDraw = LCD_SetDrawMode(DRAWMODE_REV);
	WM_ADDORG(x0, y0);
	WM_ADDORG(x1, y1);
	r.x0 = x0;
	r.x1 = x1;
	r.y0 = y0;
	r.y1 = y1;
	WM_ITERATE_START(&r) {
		LCD_FillRect(x0, y0, x1, y1);
	} WM_ITERATE_END();
	LCD_SetDrawMode(PrevDraw);
}
void GUI_Clear(void) {
	GUI_GotoXY(0, 0);     /* Reset text cursor to upper left */
	GUI_ClearRect(GUI_XMIN, GUI_YMIN, GUI_XMAX, GUI_YMAX);
}
void GUI_ClearRectEx(const GUI_RECT *pRect) {
	GUI_ClearRect(pRect->x0, pRect->y0, pRect->x1, pRect->y1);
}

void GUI_FillRect(int x0, int y0, int x1, int y1) {
	GUI_RECT r;
	WM_ADDORG(x0, y0);
	WM_ADDORG(x1, y1);
	r.x0 = x0; r.x1 = x1;
	r.y0 = y0; r.y1 = y1;
	WM_ITERATE_START(&r); {
		LCD_FillRect(x0, y0, x1, y1);
	} WM_ITERATE_END();
}
void GUI_FillRectEx(const GUI_RECT *pRect) {
	GUI_FillRect(pRect->x0, pRect->y0, pRect->x1, pRect->y1);
}

void GUI_DrawRect(int x0, int y0, int x1, int y1) {
	GUI_RECT r;
	WM_ADDORG(x0, y0);
	WM_ADDORG(x1, y1);
	r.x0 = x0;
	r.x1 = x1;
	r.y0 = y0;
	r.y1 = y1;
	WM_ITERATE_START(&r); {
		LCD_DrawHLine(x0, y0, x1);
		LCD_DrawHLine(x0, y1, x1);
		LCD_DrawVLine(x0, y0 + 1, y1 - 1);
		LCD_DrawVLine(x1, y0 + 1, y1 - 1);
	} WM_ITERATE_END();
}
void GUI_DrawRectEx(const GUI_RECT *pRect) {
	GUI_DrawRect(pRect->x0, pRect->y0, pRect->x1, pRect->y1);
}

void GUI_DrawFocusRect(const GUI_RECT *pRect, int Dist) {
	GUI_RECT r;
	GUI__ReduceRect(&r, pRect, Dist);
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	WM_ITERATE_START(&r); {
		int i;
		for (i = r.x0; i <= r.x1; i += 2) {
			LCD_DrawPixel(i, r.y0);
			LCD_DrawPixel(i, r.y1);
		}
		for (i = r.y0; i <= r.y1; i += 2) {
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
	WM_ITERATE_START(&r); {
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



#include "GUI_Private.h"

/*********************************************************************
*
*       GL_DrawBitmap
*
* Purpose:
*  Translates the external bitmap format into an internal
*  format. This turned out to be necessary as the internal
*  format is easier to create and more flexible for routines
*  that draw text-bitmaps.
*/
void GL_DrawBitmap(const GUI_BITMAP *pBitmap, int x0, int y0) {
	GUI_DRAWMODE PrevDraw;
	const GUI_LOGPALETTE *pPal;
	pPal = pBitmap->pPal;
	PrevDraw = GUI_SetDrawMode(0);  /* No Get... at this point */
	GUI_SetDrawMode((pPal && pPal->HasTrans) ? (PrevDraw | DRAWMODE_TRANS) : PrevDraw & (~DRAWMODE_TRANS));
	if (pBitmap->pMethods) {
		pBitmap->pMethods->pfDraw(x0, y0, pBitmap->XSize, pBitmap->YSize, (uint8_t const *)pBitmap->pData, pBitmap->pPal, 1, 1);
	}
	else {
		const RGB_COLOR *pTrans = pBitmap->pPal ? pBitmap->pPal->pPalEntries : NULL;
		if (!pTrans) {
			pTrans = (pBitmap->BitsPerPixel != 1) ? NULL : &LCD_BKCOLORINDEX;
		}
		LCD_DrawBitmap(x0, y0
					   , pBitmap->XSize, pBitmap->YSize
					   , pBitmap->BitsPerPixel
					   , pBitmap->BytesPerLine
					   , pBitmap->pData
					   , pTrans);
	}
	GUI_SetDrawMode(PrevDraw);
}

void GUI_DrawBitmap(const GUI_BITMAP *pBitmap, int x0, int y0) {
#if (GUI_WINSUPPORT)
	GUI_RECT r;
#endif

#if (GUI_WINSUPPORT)
	WM_ADDORG(x0, y0);
	r.x1 = (r.x0 = x0) + pBitmap->XSize - 1;
	r.y1 = (r.y0 = y0) + pBitmap->YSize - 1;
	WM_ITERATE_START(&r) {
#endif
		GL_DrawBitmap(pBitmap, x0, y0);
#if (GUI_WINSUPPORT)
	} WM_ITERATE_END();
#endif

}
#pragma endregion

#pragma region Font&String
const GUI_FONT *GUI_GetFont(void) {
	return GUI_Context.pAFont;
}
const GUI_FONT *GUI_SetFont(const GUI_FONT *pNewFont) {
	const GUI_FONT *pOldFont = GUI_Context.pAFont;
	if (pNewFont)
		GUI_Context.pAFont = pNewFont;
	return pOldFont;
}

void GUI_GetTextExtend(GUI_RECT *pRect, const char *s, int MaxNumChars) {
	int xMax = 0;
	int NumLines = 0;
	int LineSizeX = 0;
	uint16_t Char;
	pRect->x0 = GUI_Context.DispPosX;
	pRect->y0 = GUI_Context.DispPosY;
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
		switch (TextAlign & GUI_TA_HORIZONTAL) {
			case GUI_TA_HCENTER:
				xPos = pTextRectIn->x0 + ((pTextRectIn->x1 - pTextRectIn->x0 + 1) - TextWidth) / 2;
				break;
			case GUI_TA_RIGHT:
				xPos = pTextRectIn->x1 - TextWidth + 1;
				break;
			default:
				xPos = pTextRectIn->x0;
		}

		/* Calculate Y-pos of text*/
		TextHeight = GUI_GetFontDistY();
		switch (TextAlign & GUI_TA_VERTICAL) {
			case GUI_TA_VCENTER:
				yPos = pTextRectIn->y0 + ((pTextRectIn->y1 - pTextRectIn->y0 + 1) - TextHeight) / 2;
				break;
			case GUI_TA_BOTTOM:
				yPos = pTextRectIn->y1 - TextHeight + 1;
				break;
			case GUI_TA_BASELINE:
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
	switch (GUI_Context.TextAlign & GUI_TA_VERTICAL) {
		case GUI_TA_BOTTOM:
			r = GUI_Context.pAFont->YSize - 1;
			break;
		case GUI_TA_VCENTER:
			r = GUI_Context.pAFont->YSize / 2;
			break;
		case GUI_TA_BASELINE:
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
int GUI_GetYSizeOfFont(const GUI_FONT *pFont) {
	return pFont->YSize;
}
int GUI_GetYDistOfFont(const GUI_FONT *pFont) {
	return pFont->YDist;
}
int GUI_GetFontSizeY(void) {
	return GUI_Context.pAFont->YSize;
}

void GUI_GetFontInfo(const GUI_FONT *pFont, GUI_FONTINFO *pFontInfo) {
	if (pFont == NULL) {
		pFont = GUI_Context.pAFont;
	}
	pFontInfo->Baseline = pFont->Baseline;
	pFontInfo->CHeight = pFont->CHeight;
	pFontInfo->LHeight = pFont->LHeight;
	pFont->pfGetFontInfo(pFont, pFontInfo);
}

char GUI_IsInFont(const GUI_FONT *pFont, uint16_t c) {
	if (pFont == NULL)
		pFont = GUI_Context.pAFont;
	return pFont->pfIsInFont(pFont, c);
}
#pragma endregion

#pragma region GUI_Font
#pragma region Font MONO
void GUIMONO_DispChar(uint16_t c) {
	int c0, c1;
	const uint8_t *pd;
	int x = GUI_Context.DispPosX;
	int y = GUI_Context.DispPosY;
	/* do some checking if drawing is actually necessary ... */
	const GUI_FONT_MONO *pMono = GUI_Context.pAFont->p.pMono;
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
		GUI_FONT_TRANSINFO const *pti = pMono->pTrans;
		pd = pMono->pTransData;
		if (pti) {
			FirstChar = pti->FirstChar;
			if ((c >= (uint16_t)FirstChar) && (c <= (uint16_t)pti->LastChar)) {
				GUI_FONT_TRANSLIST const *ptl;
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
			uint8_t OldMode = LCD_SetDrawMode(DrawMode);
			LCD_DrawBitmap(x, y,
						   XSize, YSize,
						   1,     /* Bits per Pixel */
						   pMono->BytesPerLine,
						   pd + c0 * BytesPerChar,
						   &LCD_BKCOLORINDEX
			);
			if (c1 != -1) {
				LCD_SetDrawMode(DrawMode | DRAWMODE_TRANS);
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
					LCD_SetDrawMode(DrawMode ^ DRAWMODE_REV);  /* Reverse so we can fill with BkColor */
					LCD_FillRect(x,
								 y + GUI_Context.pAFont->YSize * GUI_Context.pAFont->YDist,
								 x + XSize,
								 y + GUI_Context.pAFont->YDist);
				}
			}
			LCD_SetDrawMode(OldMode);
		}
	}
	GUI_Context.DispPosX += pMono->XDist;

}

int GUIMONO_GetCharDistX(uint16_t c) {
	const GUI_FONT_MONO *pMono = GUI_Context.pAFont->p.pMono;
	GUI_USE_PARA(c);
	return pMono->XDist;

}

void GUIMONO_GetFontInfo(const GUI_FONT *pFont, GUI_FONTINFO *pfi) {
	GUI_USE_PARA(pFont);
	pfi->Flags = GUI_FONTINFO_FLAG_MONO;
}

char GUIMONO_IsInFont(const GUI_FONT *pFont, uint16_t c) {
	const GUI_FONT_MONO *pMono = pFont->p.pMono;
	unsigned int FirstChar = pMono->FirstChar;
	/* Check if regular character first. */
	if ((c >= (uint16_t)FirstChar) && (c <= (uint16_t)pMono->LastChar)) {
		return 1;  /* Yes, we have it ! */
	}
	else {
		/* Check if character is in translation table */
		GUI_FONT_TRANSINFO const *pti;
		pti = pMono->pTrans;
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
static const GUI_FONT_PROP *GUIPROP_FindChar(const GUI_FONT_PROP *pProp, uint16_t c) {
	for (; pProp; pProp = pProp->pNext) {
		if ((c >= pProp->First) && (c <= pProp->Last))
			break;
	}
	return pProp;
}

void GUIPROP_DispChar(uint16_t c) {
	int BytesPerLine;

	GUI_DRAWMODE DrawMode = GUI_Context.TextMode;
	const GUI_FONT_PROP *pProp = GUIPROP_FindChar(GUI_Context.pAFont->p.pProp, c);
	if (pProp) {
		GUI_DRAWMODE OldDrawMode;
		const GUI_CHARINFO *pCharInfo = pProp->paCharInfo + (c - pProp->First);
		BytesPerLine = pCharInfo->BytesPerLine;
		OldDrawMode = LCD_SetDrawMode(DrawMode);
		LCD_DrawBitmap(GUI_Context.DispPosX, GUI_Context.DispPosY,
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
				RGB_COLOR OldColor = GUI_GetColor();
				GUI_SetColor(GUI_GetBkColor());
				LCD_FillRect(GUI_Context.DispPosX,
							 GUI_Context.DispPosY + YSize,
							 GUI_Context.DispPosX + pCharInfo->XSize,
							 GUI_Context.DispPosY + YDist);
				GUI_SetColor(OldColor);
			}
		}
		LCD_SetDrawMode(OldDrawMode); /* Restore draw mode */
		GUI_Context.DispPosX += pCharInfo->XDist;
	}
}

int GUIPROP_GetCharDistX(uint16_t c) {
	const GUI_FONT_PROP *pProp = GUIPROP_FindChar(GUI_Context.pAFont->p.pProp, c);
	return (pProp) ? (pProp->paCharInfo + (c - pProp->First))->XSize : 0;
}

void GUIPROP_GetFontInfo(const GUI_FONT *pFont, GUI_FONTINFO *pfi) {
	GUI_USE_PARA(pFont);
	pfi->Flags = GUI_FONTINFO_FLAG_PROP;
}

char GUIPROP_IsInFont(const GUI_FONT *pFont, uint16_t c) {
	const GUI_FONT_PROP *pProp = GUIPROP_FindChar(pFont->p.pProp, c);
	return (pProp == NULL) ? 0 : 1;
}
#pragma endregion
#pragma endregion

#pragma region Display String

static void _DispLine(const char *s, int MaxNumChars, const GUI_RECT *pRect) {
	/* Check if we have anything to do at all ... */
	if (GUI_Context.pClipRect_HL) {
		if (GUI_RectsIntersect(GUI_Context.pClipRect_HL, pRect) == 0)
			return;
	}
	if (GUI_Context.pAFont->pafEncode) {
		GUI_Context.pAFont->pafEncode->pfDispLine(s, MaxNumChars);
	}
	else {
		uint16_t Char;
		while (--MaxNumChars >= 0) {
			Char = GUI_UC__GetCharCodeInc(&s);
			GUI_Context.pAFont->pfDispChar(Char);
		}
	}
}
void GUI__DispLine(const char *s, int MaxNumChars, const GUI_RECT *pr) {
	GUI_RECT r;
	{
		r = *pr;
#if GUI_WINSUPPORT
		WM_ADDORG(r.x0, r.y0);
		WM_ADDORG(r.x1, r.y1);
		WM_ITERATE_START(&r) {
#endif
			GUI_Context.DispPosX = r.x0;
			GUI_Context.DispPosY = r.y0;
			/* Do the actual drawing via routine call. */
			_DispLine(s, MaxNumChars, &r);
#if GUI_WINSUPPORT
		} WM_ITERATE_END();
		WM_SUBORG(GUI_Context.DispPosX, GUI_Context.DispPosY);
#endif
	}
}

void GUI_DispString(const char *s) {
	int xAdjust, yAdjust, xOrg;
	int FontSizeY;
	if (!s)
		return;

	FontSizeY = GUI_GetFontDistY();
	xOrg = GUI_Context.DispPosX;
	/* Adjust vertical position */
	yAdjust = GUI_GetYAdjust();
	GUI_Context.DispPosY -= yAdjust;
	for (; *s; s++) {
		GUI_RECT r;
		int LineNumChars = GUI__GetLineNumChars(s, 0x7fff);
		int xLineSize = GUI__GetLineDistX(s, LineNumChars);
		/* Check if x-position needs to be changed due to h-alignment */
		switch (GUI_Context.TextAlign & GUI_TA_HORIZONTAL) {
			case GUI_TA_CENTER: xAdjust = xLineSize / 2; break;
			case GUI_TA_RIGHT:  xAdjust = xLineSize; break;
			default:            xAdjust = 0;
		}
		r.x0 = GUI_Context.DispPosX -= xAdjust;
		r.x1 = r.x0 + xLineSize - 1;
		r.y0 = GUI_Context.DispPosY;
		r.y1 = r.y0 + FontSizeY - 1;
		GUI__DispLine(s, LineNumChars, &r);
		GUI_Context.DispPosY = r.y0;
		s += GUI_UC__NumChars2NumBytes(s, LineNumChars);
		if ((*s == '\n') || (*s == '\r')) {
			switch (GUI_Context.TextAlign & GUI_TA_HORIZONTAL) {
				case GUI_TA_CENTER:
				case GUI_TA_RIGHT:
					GUI_Context.DispPosX = xOrg;
					break;
				default:
					GUI_Context.DispPosX = GUI_Context.LBorder;
					break;
			}
			if (*s == '\n')
				GUI_Context.DispPosY += FontSizeY;
		}
		else {
			GUI_Context.DispPosX = r.x0 + xLineSize;
		}
		if (*s == 0)    /* end of string (last line) reached ? */
			break;
	}
	GUI_Context.DispPosY += yAdjust;
	GUI_Context.TextAlign &= ~GUI_TA_HORIZONTAL;

}
void GUI_DispStringAt(const char *s, int x, int y) {
	GUI_Context.DispPosX = x;
	GUI_Context.DispPosY = y;
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
		WM_GetClientRect(&r);
	}
	/* handle vertical alignment */
	if ((TextAlign & GUI_TA_VERTICAL) == GUI_TA_TOP) {
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
		switch (TextAlign & GUI_TA_VERTICAL) {
			case GUI_TA_BASELINE:
			case GUI_TA_BOTTOM:
				y = r.y1 - NumLines * FontYSize + 1;
				break;
			case GUI_TA_VCENTER:
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
		switch (TextAlign & GUI_TA_HORIZONTAL) {
			case GUI_TA_HCENTER:
				xLine = r.x0 + (r.x1 - r.x0 - xLineSize) / 2; break;
			case GUI_TA_LEFT:
				xLine = r.x0; break;
			case GUI_TA_RIGHT:
				xLine = r.x1 - xLineSize + 1;
		}
		rLine.x0 = GUI_Context.DispPosX = xLine;
		rLine.x1 = rLine.x0 + xLineSize - 1;
		rLine.y0 = GUI_Context.DispPosY = y;
		rLine.y1 = y + FontYSize - 1;
		GUI__DispLine(s, LineLen, &rLine);
		s += GUI_UC__NumChars2NumBytes(s, LineLen);
		y += GUI_GetFontDistY();
		if (GUI__HandleEOLine(&s))
			break;
	}
}

#if (GUI_WINSUPPORT)
void GUI_DispStringInRectMax(const char *s, GUI_RECT *pRect, int TextAlign, int MaxLen) {
	if (s) {
		const GUI_RECT *pOldClipRect = NULL;
		GUI_RECT r;

		if (pRect) {
			pOldClipRect = WM_SetUserClipRect(pRect);
			if (pOldClipRect) {
				GUI__IntersectRects(&r, pRect, pOldClipRect);
				WM_SetUserClipRect(&r);
			}
		}
		GUI__DispStringInRect(s, pRect, TextAlign, MaxLen);
		WM_SetUserClipRect(pOldClipRect);

	}
}
#else
void GUI_DispStringInRectMax(const char *s, GUI_RECT *pRect, int TextAlign, int MaxLen) {
	GUI_RECT Rect_Old, r;
	if (s && pRect) {
		Rect_Old = GUI_Context.ClipRect;
		GUI__IntersectRects(&r, pRect, &Rect_Old);
		LCD_SetClipRectEx(&r);
		GUI__DispStringInRect(s, pRect, TextAlign, MaxLen);
		LCD_SetClipRectEx(&Rect_Old);
	}
}
#endif
void GUI_DispStringInRect(const char *s, GUI_RECT *pRect, int TextAlign) {
	GUI_DispStringInRectMax(s, pRect, TextAlign, 0x7fff);
}
#pragma endregion

#pragma region Display Char
#if (GUI_WINSUPPORT)
static void CL_DispChar(uint16_t c) {
	GUI_RECT r;
	WM_ADDORG(GUI_Context.DispPosX, GUI_Context.DispPosY);
	r.x1 = (r.x0 = GUI_Context.DispPosX) + GUI_GetCharDistX(c) - 1;
	r.y1 = (r.y0 = GUI_Context.DispPosY) + GUI_GetFontSizeY() - 1;
	WM_ITERATE_START(&r) {
		GL_DispChar(c);
	} WM_ITERATE_END();
	if (c != '\n') {
		GUI_Context.DispPosX = r.x1 + 1;
	}
	WM_SUBORG(GUI_Context.DispPosX, GUI_Context.DispPosY);
}
#endif

void GUI_DispChar(uint16_t c) {

#if (GUI_WINSUPPORT)
	CL_DispChar(c);
#else
	GL_DispChar(c);
#endif

}
void GUI_DispCharAt(uint16_t c, int16_t x, int16_t y) {

	GUI_Context.DispPosX = x;
	GUI_Context.DispPosY = y;
#if (GUI_WINSUPPORT)
	CL_DispChar(c);
#else
	GL_DispChar(c);
#endif

}
void GUI_DispChars(uint16_t c, int NumChars) {
	while (--NumChars >= 0) {
		GUI_DispChar(c);
	}
}

void GUI_DispNextLine(void) {
	GUI_Context.DispPosY += GUI_GetFontDistY();
	GUI_Context.DispPosX = GUI_Context.LBorder;
}
void GL_DispChar(uint16_t c) {
	/* check for control characters */
	if (c == '\n')
		GUI_DispNextLine();
	else if (c != '\r') {
		GUI_Context.pAFont->pfDispChar(c);
	}
}
#pragma endregion

#pragma region Text operators
int GUI__SetText(GUI_HMEM *phText, const char *s) {
	int r = 0;
	if (GUI__strcmp(*phText, s) != 0) { /* Make sure we have a quick out if nothing changes */
		GUI_HMEM hMem;
		hMem = GUI_ALLOC_AllocNoInit(GUI__strlen(s) + 1);
		if (hMem) {
			char *pMem;
			pMem = (char *)(hMem);
			strcpy(pMem, s);
			GUI_ALLOC_FreePtr(phText);
			*phText = hMem;
			r = 1;
		}
	}
	return r;
}
int GUI__strcmp(const char *s0, const char *s1) {
	if (s0 == NULL) {
		s0 = "";
	}
	if (s1 == NULL) {
		s1 = "";
	}
	do {
		if (*s0 != *s1) {
			return 1;
		}
		s1++;
	} while (*++s0);
	if (*s1) {
		return 1;    /* Not equal, since s1 is longer than s0 */
	}
	return 0;      /* Equal ! */
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
#define MIN(v0,v1) ((v0>v1) ? v1 : v0)
#define MAX(v0,v1) ((v0>v1) ? v0 : v1)

int GUI__IntersectRects(GUI_RECT *pDest, const GUI_RECT *pr0, const GUI_RECT *pr1) {
	pDest->x0 = MAX(pr0->x0, pr1->x0);
	pDest->y0 = MAX(pr0->y0, pr1->y0);
	pDest->x1 = MIN(pr0->x1, pr1->x1);
	pDest->y1 = MIN(pr0->y1, pr1->y1);
	if (pDest->x1 < pDest->x0) {
		return 0;
	}
	if (pDest->y1 < pDest->y0) {
		return 0;
	}
	return 1;
}

void GUI_MergeRect(GUI_RECT *pDest, const GUI_RECT *pr0, const GUI_RECT *pr1) {
	if (pDest) {
		if (pr0 && pr1) {
			pDest->x0 = MIN(pr0->x0, pr1->x0);
			pDest->y0 = MIN(pr0->y0, pr1->y0);
			pDest->x1 = MAX(pr0->x1, pr1->x1);
			pDest->y1 = MAX(pr0->y1, pr1->y1);
			return;
		}
		*pDest = *(pr0 ? pr0 : pr1);
	}
}
void GUI_MoveRect(GUI_RECT *pRect, int dx, int dy) {
	if (pRect) {
		pRect->x0 += dx;
		pRect->x1 += dx;
		pRect->y0 += dy;
		pRect->y1 += dy;
	}
}
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
void GUI__IntersectRect(GUI_RECT *pDest, const GUI_RECT *pr0) {
	if (pDest->x0 < pr0->x0) {
		pDest->x0 = pr0->x0;
	}
	if (pDest->y0 < pr0->y0) {
		pDest->y0 = pr0->y0;
	}
	if (pDest->x1 > pr0->x1) {
		pDest->x1 = pr0->x1;
	}
	if (pDest->y1 > pr0->y1) {
		pDest->y1 = pr0->y1;
	}
}
void GUI__ReduceRect(GUI_RECT *pDest, const GUI_RECT *pRect, int Dist) {
	pDest->x0 = pRect->x0 + Dist;
	pDest->x1 = pRect->x1 - Dist;
	pDest->y0 = pRect->y0 + Dist;
	pDest->y1 = pRect->y1 - Dist;
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
/*********************************************************************
*
*       GUI__DivideRound32
*
* This routine is used internally for computations. Primary goal is
* to minimize the effects of rounding which occur if we simply
* divide.
*/
int32_t GUI__DivideRound32(int32_t a, int32_t b) {
	if (b) {
		if (a < 0) {
			return (a - (b >> 1)) / b;
		}
		else {
			return (a + (b >> 1)) / b;
		}
	}
	return 0;
}
