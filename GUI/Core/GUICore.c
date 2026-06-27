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
#pragma endregion

#pragma region Font&String
#include "GUI.h"
#include "GUI_Protected.h"

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
