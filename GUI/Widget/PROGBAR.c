
#include <stdlib.h>
#include <string.h>

#include "GUI_Protected.h"

#include "WIDGET.h"
#include "PROGBAR.h"

#define PROGBAR_DEFAULT_FONT GUI_DEFAULT_FONT
#define PROGBAR_DEFAULT_BARCOLOR0 RGB_GRAYL(0x55)
#define PROGBAR_DEFAULT_BARCOLOR1 RGB_GRAYL(0xAA)
#define PROGBAR_DEFAULT_TEXTCOLOR0 RGB_WHITE
#define PROGBAR_DEFAULT_TEXTCOLOR1 RGB_BLACK
typedef struct {
	WIDGET Widget;
	int v;
	const GUI_FONT GUI_UNI_PTR *pFont;
	RGB_COLOR BarColor[2];
	RGB_COLOR TextColor[2];
	WM_HMEM hpText;
	int16_t XOff, YOff;
	int16_t TextAlign;
	int Min, Max;
	/*  int16_t Options; */
} PROGBAR_Obj;
#define Invalidate(h) WM_Invalidate(h)
static void _FreeText(PROGBAR_Handle hObj) {
	PROGBAR_Obj *pObj = (hObj);
	GUI_ALLOC_FreePtr(&pObj->hpText);
}
static int _Value2X(const PROGBAR_Obj *pObj, int v) {
	int EffectSize = pObj->Widget.pEffect->EffectSize;
	int xSize = pObj->Widget.Win.Rect.x1 - pObj->Widget.Win.Rect.x0 + 1;
	int Min = pObj->Min;
	int Max = pObj->Max;
	if (v < Min) {
		v = Min;
	}
	if (v > Max) {
		v = Max;
	}
	return EffectSize + ((xSize - 2 * EffectSize) * (int32_t)(v - Min)) / (Max - Min);
}
static void _DrawPart(const PROGBAR_Obj *pObj, int Index,
					  int xText, int yText, const char *pText) {
	GUI_SetBkColor(pObj->BarColor[Index]);
	GUI_SetColor(pObj->TextColor[Index]);
	GUI_Clear();
	GUI_GotoXY(xText, yText);
	GUI_DispString(pText);
}
static const char *_GetText(const PROGBAR_Obj *pObj, char *pBuffer) {
	char *pText;
	uint8_t value;
	if (pObj->hpText) {
		pText = (char *)(pObj->hpText);
	}
	else {
		pText = pBuffer;
		value = 100 * (pObj->v - pObj->Min) / (pObj->Max - pObj->Min);
		if (value == 100) {
			*pBuffer++ = '1';
			*pBuffer++ = '0';
			*pBuffer++ = '0';
		} else {
			if (value >= 10) {
				*pBuffer++ = '0' + value / 10;
				value %= 10;
			}
			*pBuffer++ = '0' + value;
		}
		*pBuffer++ = '%';
		*pBuffer = 0;
	}
	return (const char *)pText;
}
static void _GetTextRect(const PROGBAR_Obj *pObj, GUI_RECT *pRect, const char *pText) {
	int xSize = pObj->Widget.Win.Rect.x1 - pObj->Widget.Win.Rect.x0 + 1;
	int ySize = pObj->Widget.Win.Rect.y1 - pObj->Widget.Win.Rect.y0 + 1;
	int TextWidth = GUI_GetStringDistX(pText);
	int TextHeight = GUI_GetFontSizeY();
	int EffectSize = pObj->Widget.pEffect->EffectSize;
	switch (pObj->TextAlign & GUI_TA_HORIZONTAL) {
		case GUI_TA_CENTER:
			pRect->x0 = (xSize - TextWidth) / 2;
			break;
		case GUI_TA_RIGHT:
			pRect->x0 = xSize - TextWidth - 1 - EffectSize;
			break;
		default:
			pRect->x0 = EffectSize;
	}
	pRect->y0 = (ySize - TextHeight) / 2;
	pRect->x0 += pObj->XOff;
	pRect->y0 += pObj->YOff;
	pRect->x1 = pRect->x0 + TextWidth - 1;
	pRect->y1 = pRect->y0 + TextHeight - 1;
}
static void _Paint(PROGBAR_Handle hObj) {
	PROGBAR_Obj *pObj;
	GUI_RECT r, rInside, rClient, rText;
	const char *pText;
	char ac[5];
	int tm, xPos;
	pObj = (hObj);
	WM_GetClientRect(&rClient);
	GUI__ReduceRect(&rInside, &rClient, pObj->Widget.pEffect->EffectSize);
	xPos = _Value2X(pObj, pObj->v);
	pText = _GetText(pObj, ac);
	GUI_SetFont(pObj->pFont);
	_GetTextRect(pObj, &rText, pText);
	tm = GUI_SetTextMode(GUI_TM_TRANS);
	/* Draw left bar */
	r = rInside;
	r.x1 = xPos - 1;
	WM_SetUserClipRect(&r);
	_DrawPart(pObj, 0, rText.x0, rText.y0, pText);
	/* Draw right bar */
	r = rInside;
	r.x0 = xPos;
	WM_SetUserClipRect(&r);
	_DrawPart(pObj, 1, rText.x0, rText.y0, pText);
	WM_SetUserClipRect(NULL);
	GUI_SetTextMode(tm);
	WIDGET__EFFECT_DrawDownRect(&pObj->Widget, &rClient);
}
static void _Delete(PROGBAR_Handle hObj) {
	_FreeText(hObj);
}
static void _PROGBAR_Callback(WM_MESSAGE *pMsg) {
	PROGBAR_Handle hObj = (PROGBAR_Handle)pMsg->hWin;
	/* Let widget handle the standard messages */
	if (WIDGET_HandleActive(hObj, pMsg) == 0) {
		return;
	}
	switch (pMsg->MsgId) {
		case WM_PAINT:
			_Paint(hObj);
			return;
		case WM_DELETE:
			_Delete(hObj);
			break;
	}
	WM_DefaultProc(pMsg);
}
PROGBAR_Handle PROGBAR_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
								int WinFlags, int ExFlags, int Id) {
	PROGBAR_Handle hObj;
	GUI_USE_PARA(ExFlags);
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, _PROGBAR_Callback,
								  sizeof(PROGBAR_Obj) - sizeof(WM_Obj));
	if (hObj) {
		PROGBAR_Obj *pObj;

		pObj = (PROGBAR_Obj *)(hObj);
		/* init widget specific variables */
		WIDGET__Init(&pObj->Widget, Id, 0);
		WIDGET_SetEffect(hObj, &WIDGET_Effect_None); /* Standard effect for progbar: None */
		/* init member variables */
		pObj->pFont = GUI_DEFAULT_FONT;
		pObj->BarColor[0] = PROGBAR_DEFAULT_BARCOLOR0;
		pObj->BarColor[1] = PROGBAR_DEFAULT_BARCOLOR1;
		pObj->TextColor[0] = PROGBAR_DEFAULT_TEXTCOLOR0;
		pObj->TextColor[1] = PROGBAR_DEFAULT_TEXTCOLOR1;
		pObj->TextAlign = GUI_TA_CENTER;
		pObj->Max = 100;
		pObj->Min = 0;

	}
	return hObj;
}
void PROGBAR_SetValue(PROGBAR_Handle hObj, int v) {
	if (hObj) {
		PROGBAR_Obj *pObj;

		pObj = (hObj);
		/* Put v into legal range */
		if (v < pObj->Min) {
			v = pObj->Min;
		}
		if (v > pObj->Max) {
			v = pObj->Max;
		}
		if (pObj->v != v) {
			GUI_RECT r;
			/* Get x values */
			if (v < pObj->v) {
				r.x0 = _Value2X(pObj, v);
				r.x1 = _Value2X(pObj, pObj->v);
			}
			else {
				r.x0 = _Value2X(pObj, pObj->v);
				r.x1 = _Value2X(pObj, v);
			}
			r.y0 = 0;
			r.y1 = 4095;
			if (pObj->hpText == 0) {
				const GUI_FONT GUI_UNI_PTR *pOldFont;
				char acBuffer[5];
				GUI_RECT rText;
				pOldFont = GUI_SetFont(pObj->pFont);
				_GetTextRect(pObj, &rText, _GetText(pObj, acBuffer));
				GUI_MergeRect(&r, &r, &rText);
				pObj->v = v;
				_GetTextRect(pObj, &rText, _GetText(pObj, acBuffer));
				GUI_MergeRect(&r, &r, &rText);
				GUI_SetFont(pOldFont);
			}
			else {
				pObj->v = v;
			}
			WM_InvalidateRect(hObj, &r);
		}

	}
}
void PROGBAR_SetFont(PROGBAR_Handle hObj, const GUI_FONT GUI_UNI_PTR *pfont) {
	PROGBAR_Obj *pObj;
	if (hObj) {

		pObj = (hObj);
		pObj->pFont = pfont;
		WM_Invalidate(hObj);

	}
}
void PROGBAR_SetBarColor(PROGBAR_Handle hObj, unsigned int Index, RGB_COLOR color) {
	PROGBAR_Obj *pObj;
	if (hObj) {

		pObj = (hObj);
		if (Index < GUI_COUNTOF(pObj->BarColor)) {
			pObj->BarColor[Index] = color;
			WM_Invalidate(hObj);
		}

	}
}
void PROGBAR_SetTextColor(PROGBAR_Handle hObj, unsigned int Index, RGB_COLOR color) {
	PROGBAR_Obj *pObj;
	if (hObj) {

		pObj = (hObj);
		if (Index < GUI_COUNTOF(pObj->TextColor)) {
			pObj->TextColor[Index] = color;
			WM_Invalidate(hObj);
		}

	}
}
void PROGBAR_SetText(PROGBAR_Handle hObj, const char *s) {
	if (hObj) {
		PROGBAR_Obj *pObj;
		const GUI_FONT GUI_UNI_PTR *pOldFont;
		GUI_RECT r1;
		char acBuffer[5];

		pObj = (hObj);
		pOldFont = GUI_SetFont(pObj->pFont);
		_GetTextRect(pObj, &r1, _GetText(pObj, acBuffer));
		if (GUI__SetText(&pObj->hpText, s)) {
			GUI_RECT r2;
			_GetTextRect(pObj, &r2, _GetText(pObj, acBuffer));
			GUI_MergeRect(&r1, &r1, &r2);
			WM_InvalidateRect(hObj, &r1);
		}
		GUI_SetFont(pOldFont);

	}
}
void PROGBAR_SetTextAlign(PROGBAR_Handle hObj, int Align) {
	PROGBAR_Obj *pObj;
	if (hObj) {

		pObj = (hObj);
		pObj->TextAlign = Align;
		WM_Invalidate(hObj);

	}
}
void PROGBAR_SetTextPos(PROGBAR_Handle hObj, int XOff, int YOff) {
	PROGBAR_Obj *pObj;
	if (hObj) {

		pObj = (hObj);
		pObj->XOff = XOff;
		pObj->YOff = YOff;
		WM_Invalidate(hObj);

	}
}
void PROGBAR_SetMinMax(PROGBAR_Handle hObj, int Min, int Max) {
	PROGBAR_Obj *pObj;
	if (hObj) {

		pObj = (hObj);
		if (Max > Min) {
			if ((Max != pObj->Max) || (Min != pObj->Min)) {
				pObj->Min = Min;
				pObj->Max = Max;
				WM_Invalidate(hObj);
			}
		}

	}
}

PROGBAR_Handle PROGBAR_Create(int x0, int y0, int xsize, int ysize, int Flags) {
	return PROGBAR_CreateEx(x0, y0, xsize, ysize, 0, Flags, 0, 0);
}
PROGBAR_Handle PROGBAR_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags) {
	return PROGBAR_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, Id);
}

PROGBAR_Handle  PROGBAR_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	PROGBAR_Handle  hThis;
	GUI_USE_PARA(cb);
	hThis = PROGBAR_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							 hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
	return hThis;
}

