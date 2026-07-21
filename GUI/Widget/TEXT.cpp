#include "GUIDebug.h"
#include "GUI_Protected.h"

#include "TEXT.h"

/* Define default fonts */
#define TEXT_FONT_DEFAULT &GUI_Font13_1
#define TEXT_DEFAULT_TEXT_COLOR RGB_BLACK
static PCFONT _pDefaultFont = TEXT_FONT_DEFAULT;
static RGBC        _DefaultTextColor = TEXT_DEFAULT_TEXT_COLOR;
static void _FreeAttached(TEXT_Obj *pObj) {
	GUI_ALLOC_FreePtr((void **)&pObj->pText);
}
static void _OnPaint(TEXT_Obj *pObj) {
	const char *s;
	GUI_USE_PARA(pObj);
	GUI_SetColor(pObj->TextColor);
	GUI_SetFont(pObj->pFont);
	/* Fill with parents background color */
#if !WM_SUPPORT_TRANSPARENCY   /* Not needed any more, since window is transparent*/
	if (pObj->BkColor == RGB_INVALID_COLOR) {
		GUI_SetBkColor(WIDGET__GetBkColor(hObj));
	}
	else {
		GUI_SetBkColor(pObj->BkColor);
	}
	GUI_Clear();
#else
	if (!WM_GetHasTrans(pObj)) {
		GUI_SetBkColor(pObj->BkColor);
		GUI_Clear();
	}
#endif
	/* Show the text */
	if (pObj->pText) {
		s = pObj->pText;
		GUI_SetTextMode(DRAWMODE_TRANS);
		auto r = WM_GetClientRect();
		GUI_DispStringInRect(s, &r, pObj->Align);
	}
}
static void _Delete(TEXT_Obj *pObj) {
	/* Delete attached objects (if any) */
	_FreeAttached(pObj);
}
static WM_PARAM _TEXT_Callback(WM_Obj * hWin, int MsgId, WM_PARAM Data) {
	auto pObj = (TEXT_Obj *)hWin;
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
/* Note: the parameters to a create function may vary.
		 Some widgets may have multiple create functions */
TEXT_Handle TEXT_CreateEx(int x0, int y0, int xsize, int ysize, WM_Obj * hParent,
						  int WinFlags, int ExFlags, int Id, const char *pText) {
	TEXT_Handle hObj;
	/* Create the window */
#if WM_SUPPORT_TRANSPARENCY
	WinFlags |= WM_CF_HASTRANS;
#endif
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, _TEXT_Callback,
								  sizeof(TEXT_Obj) - sizeof(WM_Obj));
	if (hObj) {
		auto pObj = (TEXT_Obj *)hObj;
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, 0);
		/* init member variables */
		if (pText) 
			GUI__SetText(&pObj->pText, pText);
		else
			pObj->pText = nullptr;
		pObj->Align = ExFlags;
		pObj->pFont = _pDefaultFont;
		pObj->BkColor = RGB_INVALID_COLOR;
		pObj->TextColor = _DefaultTextColor;
	}
	else {
	}
	return hObj;
}

TEXT_Handle TEXT_Create(int x0, int y0, int xsize, int ysize, int Id, int Flags, const char *s, int Align) {
	return TEXT_CreateEx(x0, y0, xsize, ysize, nullptr, Flags, Align, Id, s);
}
TEXT_Handle TEXT_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_Obj * hParent, int Id, int Flags, const char *s, int Align) {
	return TEXT_CreateEx(x0, y0, xsize, ysize, hParent, Flags, Align, Id, s);
}

TEXT_Handle TEXT_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	TEXT_Handle  hThis;
	GUI_USE_PARA(cb);
	hThis = TEXT_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
						  hWinParent, WM_CF_SHOW, pCreateInfo->Flags, pCreateInfo->Id, pCreateInfo->pName);
	return hThis;
}

void TEXT_SetBkColor(TEXT_Handle hObj, RGBC Color) {
	if (hObj) {
		auto pObj = (TEXT_Obj *)hObj;
		pObj->BkColor = Color;
#if WM_SUPPORT_TRANSPARENCY
		if (Color <= RGB_WHITE) {
			WM_ClrHasTrans(hObj);
		}
		else {
			WM_SetHasTrans(hObj);
		}
#endif
		WM_Invalidate(hObj);
	}
}

void TEXT_SetFont(TEXT_Handle hObj, PCFONT pFont) {
	if (hObj) {
		auto pObj = (TEXT_Obj *)hObj;
		pObj->pFont = pFont;
		WM_Invalidate(hObj);
	}
}


void TEXT_SetText(TEXT_Handle hObj, const char *s) {
	if (hObj) {
		auto pObj = (TEXT_Obj *)hObj;
		if (GUI__SetText(&pObj->pText, s))
			WM_Invalidate(hObj);
	}
}

void TEXT_SetTextAlign(TEXT_Handle hObj, int Align) {
	if (hObj) {
		auto pObj = (TEXT_Obj *)hObj;
		pObj->Align = Align;
		WM_Invalidate(hObj);

	}
}
void TEXT_SetTextColor(TEXT_Handle hObj, RGBC Color) {
	if (hObj) {
		auto pObj = (TEXT_Obj *)hObj;
		pObj->TextColor = Color;
		WM_Invalidate(hObj);

	}
}
