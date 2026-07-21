#include "GUI_Protected.h"

#include "WIDGET.h"
#include "PROGBAR.h"

struct PROGBAR_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &GUI_Font13_1 };
		RGBC aBkColor[2]{
			/* Active */	RGB_DARKBLUE,
			/* Inactive */	RGB_GRAYL(0x55)
		};
		RGBC aTextColor[2]{
			/* Active */	RGB_WHITE,
			/* Inactive */	RGB_BLACK
		};
		TEXTALIGN Align{ TEXTALIGN_HCENTER };
	} static DefaultProps;
	Properties Props;
	char *pText;
	int16_t XOff, YOff;
	int16_t v, Min, Max;
};

PROGBAR_Obj::Properties PROGBAR_Obj::DefaultProps;

static void _FreeText(PROGBAR_Obj *pObj) {
	GUI_ALLOC_FreePtr((void **)&pObj->pText);
}
static int _Value2X(const PROGBAR_Obj *pObj, int v) {
	int EffectSize = pObj->pEffect->EffectSize;
	int xSize = pObj->Rect.x1 - pObj->Rect.x0 + 1;
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
	GUI_SetBkColor(pObj->Props.aBkColor[Index]);
	GUI_SetColor(pObj->Props.aTextColor[Index]);
	GUI_Clear();
	GUI_GotoXY(xText, yText);
	GUI_DispString(pText);
}
static const char *_GetText(const PROGBAR_Obj *pObj, char *pBuffer) {
	char *pText;
	uint8_t value;
	if (pObj->pText) {
		pText = pObj->pText;
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
	int xSize = pObj->Rect.x1 - pObj->Rect.x0 + 1;
	int ySize = pObj->Rect.y1 - pObj->Rect.y0 + 1;
	int TextWidth = GUI_GetStringDistX(pText);
	int TextHeight = GUI_GetFontSizeY();
	int EffectSize = pObj->pEffect->EffectSize;
	switch (pObj->Props.Align & TEXTALIGN_HORIZONTAL) {
		case TEXTALIGN_CENTER:
			pRect->x0 = (xSize - TextWidth) / 2;
			break;
		case TEXTALIGN_RIGHT:
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
static void _OnPaint(PROGBAR_Obj *pObj) {
	GUI_RECT r, rInside, rText;
	const char *pText;
	char ac[5];
	int xPos;
	auto rClient = WM_GetClientRect();
	rInside = rClient - pObj->pEffect->EffectSize;
	xPos = _Value2X(pObj, pObj->v);
	pText = _GetText(pObj, ac);
	GUI_SetFont(pObj->Props.pFont);
	_GetTextRect(pObj, &rText, pText);
	GUI_SetTextMode(DRAWMODE_TRANS);
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
	WM_SetUserClipRect(nullptr);
	WIDGET__EFFECT_DrawDownRect(pObj, rClient);
}
static void _Delete(PROGBAR_Obj *pObj) {
	_FreeText(pObj);
}
static WM_PARAM _PROGBAR_Callback(WM_Obj * hWin, int MsgId, WM_PARAM Data) {
	auto pObj = (PROGBAR_Obj *)hWin;
	/* Let widget handle the standard messages */
	if (!WIDGET_HandleActive(pObj, MsgId, &Data))
		return Data;
	switch (MsgId) {
		case WM_PAINT:
			_OnPaint(pObj);
			return 0;
		case WM_DELETE:
			_Delete(pObj);
			return 0;
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}
PROGBAR_Handle PROGBAR_CreateEx(int x0, int y0, int xsize, int ysize, WM_Obj * hParent,
								int WinFlags, int ExFlags, int Id) {
	PROGBAR_Handle hObj;
	GUI_USE_PARA(ExFlags);
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, _PROGBAR_Callback,
								  sizeof(PROGBAR_Obj) - sizeof(WM_Obj));
	if (hObj) {
		auto pObj = (PROGBAR_Obj *)hObj;

		pObj = (PROGBAR_Obj *)(hObj);
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, 0);
		WIDGET_SetEffect(hObj, WIDGET_Effect_None); /* Standard effect for progbar: None */
		/* init member variables */
		pObj->Props = PROGBAR_Obj::DefaultProps;
		pObj->Max = 100;
		pObj->Min = 0;

	}
	return hObj;
}
void PROGBAR_SetValue(PROGBAR_Handle hObj, int v) {
	if (hObj) {
		auto pObj = (PROGBAR_Obj *)hObj;
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
			if (pObj->pText) {
				PCFONT pOldFont;
				char acBuffer[5];
				GUI_RECT rText;
				pOldFont = GUI_SetFont(pObj->Props.pFont);
				_GetTextRect(pObj, &rText, _GetText(pObj, acBuffer));
				r |= rText;
				pObj->v = v;
				_GetTextRect(pObj, &rText, _GetText(pObj, acBuffer));
				r |= rText;
				GUI_SetFont(pOldFont);
			}
			else {
				pObj->v = v;
			}
			WM_InvalidateRect(hObj, &r);
		}

	}
}
void PROGBAR_SetFont(PROGBAR_Handle hObj, PCFONT pfont) {
	auto pObj = (PROGBAR_Obj *)hObj;
	if (hObj) {

		pObj->Props.pFont = pfont;
		WM_Invalidate(hObj);

	}
}
void PROGBAR_SetBarColor(PROGBAR_Handle hObj, unsigned int Index, RGBC color) {
	auto pObj = (PROGBAR_Obj *)hObj;
	if (hObj) {
		if (Index < GUI_COUNTOF(pObj->Props.aBkColor)) {
			pObj->Props.aBkColor[Index] = color;
			WM_Invalidate(hObj);
		}
	}
}
void PROGBAR_SetTextColor(PROGBAR_Handle hObj, unsigned int Index, RGBC color) {
	auto pObj = (PROGBAR_Obj *)hObj;
	if (hObj) {
		if (Index < GUI_COUNTOF(pObj->Props.aTextColor)) {
			pObj->Props.aTextColor[Index] = color;
			WM_Invalidate(hObj);
		}
	}
}
void PROGBAR_SetText(PROGBAR_Handle hObj, const char *s) {
	if (hObj) {
		auto pObj = (PROGBAR_Obj *)hObj;
		PCFONT pOldFont;
		GUI_RECT r1;
		char acBuffer[5];

		pOldFont = GUI_SetFont(pObj->Props.pFont);
		_GetTextRect(pObj, &r1, _GetText(pObj, acBuffer));
		if (GUI__SetText(&pObj->pText, s)) {
			GUI_RECT r2;
			_GetTextRect(pObj, &r2, _GetText(pObj, acBuffer));
			r1 |= r2;
			WM_InvalidateRect(hObj, &r1);
		}
		GUI_SetFont(pOldFont);
	}
}
void PROGBAR_SetTextAlign(PROGBAR_Handle hObj, int Align) {
	auto pObj = (PROGBAR_Obj *)hObj;
	if (hObj) {
		pObj->Props.Align = Align;
		WM_Invalidate(hObj);
	}
}
void PROGBAR_SetTextPos(PROGBAR_Handle hObj, int XOff, int YOff) {
	auto pObj = (PROGBAR_Obj *)hObj;
	if (hObj) {
		pObj->XOff = XOff;
		pObj->YOff = YOff;
		WM_Invalidate(hObj);
	}
}
void PROGBAR_SetMinMax(PROGBAR_Handle hObj, int Min, int Max) {
	auto pObj = (PROGBAR_Obj *)hObj;
	if (hObj) {
		if (Max > Min) {
			if (Max != pObj->Max || Min != pObj->Min) {
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
PROGBAR_Handle PROGBAR_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_Obj * hParent, int Id, int Flags) {
	return PROGBAR_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, Id);
}

PROGBAR_Handle  PROGBAR_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	PROGBAR_Handle  hThis;
	GUI_USE_PARA(cb);
	hThis = PROGBAR_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							 hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
	return hThis;
}
