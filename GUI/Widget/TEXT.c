#include "GUIDebug.h"
#include "GUI_Protected.h"

#include "TEXT.h"

/* Define default fonts */
#define TEXT_FONT_DEFAULT &GUI_Font13_1
#define TEXT_DEFAULT_TEXT_COLOR RGB_BLACK
static const GUI_FONT  *_pDefaultFont = TEXT_FONT_DEFAULT;
static RGB_COLOR        _DefaultTextColor = TEXT_DEFAULT_TEXT_COLOR;
static void _FreeAttached(TEXT_Obj *pObj) {
	GUI_ALLOC_FreePtr(&pObj->hpText);
}
static void _OnPaint(TEXT_Obj *pObj) {
	const char *s;
	GUI_RECT Rect;
	GUI_USE_PARA(pObj);
	GUI_SetColor(pObj->TextColor);
	GUI_SetFont(pObj->pFont);
	/* Fill with parents background color */
#if !WM_SUPPORT_TRANSPARENCY   /* Not needed any more, since window is transparent*/
	if (pObj->BkColor == GUI_INVALID_COLOR) {
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
	if (pObj->hpText) {
		s = (const char *)(pObj->hpText);
		GUI_SetTextMode(DRAWMODE_TRANS);
		WM_GetClientRect(&Rect);
		GUI_DispStringInRect(s, &Rect, pObj->Align);
	}
}
static void _Delete(TEXT_Obj *pObj) {
	/* Delete attached objects (if any) */
	_FreeAttached(pObj);
}
static void _TEXT_Callback(WM_MESSAGE *pMsg) {
	TEXT_Obj *pObj = pMsg->hWin;
	/* Let widget handle the standard messages */
	if (WIDGET_HandleActive(pObj, pMsg) == 0) {
		return;
	}
	switch (pMsg->MsgId) {
		case WM_PAINT:
			_OnPaint(pObj);
			return;
		case WM_DELETE:
			_Delete(pObj);
			break; /* No return here ... WM_DefaultProc needs to be called */
	}
	WM_DefaultProc(pMsg);
}
/* Note: the parameters to a create function may vary.
		 Some widgets may have multiple create functions */
TEXT_Handle TEXT_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
						  int WinFlags, int ExFlags, int Id, const char *pText) {
	TEXT_Handle hObj;
	/* Create the window */
#if WM_SUPPORT_TRANSPARENCY
	WinFlags |= WM_CF_HASTRANS;
#endif
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, _TEXT_Callback,
								  sizeof(TEXT_Obj) - sizeof(WM_Obj));
	if (hObj) {
		TEXT_Obj *pObj;
		WM_HMEM hMem = 0;

		pObj = (hObj);
		/* init widget specific variables */
		WIDGET__Init(&pObj->Widget, Id, 0);
		/* init member variables */
		if (pText) {
			hMem = GUI_ALLOC_AllocZero(GUI__strlen(pText) + 1);
			if (hMem) {
				GUI__strcpy((char *)(hMem), pText);
			}
		}
		pObj->hpText = hMem;
		pObj->Align = ExFlags;
		pObj->pFont = _pDefaultFont;
		pObj->BkColor = GUI_INVALID_COLOR;
		pObj->TextColor = _DefaultTextColor;

	}
	else {
	}
	return hObj;
}
void TEXT_SetDefaultFont(const GUI_FONT  *pFont) {
	_pDefaultFont = pFont;
}
void TEXT_SetDefaultTextColor(RGB_COLOR Color) {
	_DefaultTextColor = Color;
}
const GUI_FONT  *TEXT_GetDefaultFont(void) {
	return _pDefaultFont;
}

TEXT_Handle TEXT_Create(int x0, int y0, int xsize, int ysize, int Id, int Flags, const char *s, int Align) {
	return TEXT_CreateEx(x0, y0, xsize, ysize, WM_HMEM_NULL, Flags, Align, Id, s);
}
TEXT_Handle TEXT_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, const char *s, int Align) {
	return TEXT_CreateEx(x0, y0, xsize, ysize, hParent, Flags, Align, Id, s);
}

TEXT_Handle TEXT_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	TEXT_Handle  hThis;
	GUI_USE_PARA(cb);
	hThis = TEXT_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
						  hWinParent, WM_CF_SHOW, pCreateInfo->Flags, pCreateInfo->Id, pCreateInfo->pName);
	return hThis;
}

void TEXT_SetBkColor(TEXT_Handle hObj, RGB_COLOR Color) {
	if (hObj) {
		TEXT_Obj *pObj;

		pObj = (hObj);
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

void TEXT_SetFont(TEXT_Handle hObj, const GUI_FONT  *pFont) {
	if (hObj) {
		TEXT_Obj *pObj;
		pObj = (hObj);
		pObj->pFont = pFont;
		/*
		GUI_ALLOC_FreePtr(&pObj->hpText);
		if (s) {
		  hMem = GUI_ALLOC_AllocZero(GUI__strlen(s)+1);
		  if (hMem) {
			GUI__strcpy((char *) (hMem), s);
		  }
		  pObj->hpText = hMem;
		}
		*/
		WM_Invalidate(hObj);
	}
}


void TEXT_SetText(TEXT_Handle hObj, const char *s) {
	if (hObj) {
		TEXT_Obj *pObj;

		pObj = (hObj);
		if (GUI__SetText(&pObj->hpText, s)) {
			WM_Invalidate(hObj);
		}

	}
}

void TEXT_SetTextAlign(TEXT_Handle hObj, int Align) {
	if (hObj) {
		TEXT_Obj *pObj;

		pObj = (hObj);
		pObj->Align = Align;
		WM_Invalidate(hObj);

	}
}
void TEXT_SetTextColor(TEXT_Handle hObj, RGB_COLOR Color) {
	if (hObj) {
		TEXT_Obj *pObj;

		pObj = (hObj);
		pObj->TextColor = Color;
		WM_Invalidate(hObj);

	}
}
