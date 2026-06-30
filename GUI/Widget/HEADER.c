#include "SCROLLBAR.h"

#include "HEADER.h"
#include "HEADER_Private.h"

#define HEADER_SUPPORT_DRAG 1
#define HEADER_BORDER_V_DEFAULT 0
#define HEADER_BORDER_H_DEFAULT 2
/* Define default fonts */
#define HEADER_FONT_DEFAULT &GUI_Font13_1
/* Define colors */
#define HEADER_BKCOLOR_DEFAULT RGB_GRAYL(0xAA)
#define HEADER_TEXTCOLOR_DEFAULT RGB_BLACK
/* Define cursors */
#define HEADER_CURSOR_DEFAULT &GUI_CursorHeaderM
/* Remember the old cursor */
static const GUI_CURSOR  *_pOldCursor;
/* Default values */
static const GUI_CURSOR  *_pDefaultCursor = HEADER_CURSOR_DEFAULT;
static RGB_COLOR          _DefaultBkColor = HEADER_BKCOLOR_DEFAULT;
static RGB_COLOR          _DefaultTextColor = HEADER_TEXTCOLOR_DEFAULT;
static int                _DefaultBorderH = HEADER_BORDER_H_DEFAULT;
static int                _DefaultBorderV = HEADER_BORDER_V_DEFAULT;
static const GUI_FONT  *_pDefaultFont = HEADER_FONT_DEFAULT;
static void _OnPaint(HEADER_Obj *pObj) {
	GUI_RECT Rect;
	int i, xPos = -pObj->ScrollPos;
	int NumItems = GUI_ARRAY_GetNumItems(&pObj->Columns);
	int EffectSize = pObj->Widget.pEffect->EffectSize;
	GUI_SetBkColor(pObj->BkColor);
	GUI_SetFont(pObj->pFont);
	GUI_Clear();
	for (i = 0; i < NumItems; i++) {
		HEADER_COLUMN *pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItem(&pObj->Columns, i);
		WM_GetClientRect(&Rect);
		Rect.x0 = xPos;
		Rect.x1 = Rect.x0 + pColumn->Width;
		if (pColumn->hDrawObj) {
			int xOff = 0, yOff = 0;
			switch (pColumn->Align & GUI_TA_HORIZONTAL) {
				case GUI_TA_RIGHT:
					xOff = (pColumn->Width - GUI_DRAW__GetXSize(pColumn->hDrawObj));
					break;
				case GUI_TA_HCENTER:
					xOff = (pColumn->Width - GUI_DRAW__GetXSize(pColumn->hDrawObj)) / 2;
					break;
			}
			switch (pColumn->Align & GUI_TA_VERTICAL) {
				case GUI_TA_BOTTOM:
					yOff = ((Rect.y1 - Rect.y0 + 1) - GUI_DRAW__GetYSize(pColumn->hDrawObj));
					break;
				case GUI_TA_VCENTER:
					yOff = ((Rect.y1 - Rect.y0 + 1) - GUI_DRAW__GetYSize(pColumn->hDrawObj)) / 2;
					break;
			}
			WM_SetUserClipRect(&Rect);
			GUI_DRAW__Draw(pColumn->hDrawObj, xPos + xOff, yOff);
			WM_SetUserClipRect(NULL);
		}
		WIDGET__EFFECT_DrawUpRect(&pObj->Widget, &Rect);
		xPos += Rect.x1 - Rect.x0;
		Rect.x0 += EffectSize + _DefaultBorderH;
		Rect.x1 -= EffectSize + _DefaultBorderH;
		Rect.y0 += EffectSize + _DefaultBorderV;
		Rect.y1 -= EffectSize + _DefaultBorderV;
		GUI_SetColor(pObj->TextColor);
		GUI_DispStringInRect(pColumn->acText, &Rect, pColumn->Align);
	}
	WM_GetClientRect(&Rect);
	Rect.x0 = xPos;
	Rect.x1 = 0xfff;
	WIDGET__EFFECT_DrawUpRect(&pObj->Widget, &Rect);
}
static void _RestoreOldCursor(void) {
	if (_pOldCursor) {
#if GUI_SUPPORT_CURSOR
		GUI_CURSOR_Select(_pOldCursor);
#endif
		_pOldCursor = 0;
	}
}
/*********************************************************************
*
*       _FreeAttached
*
* Delete attached objects (if any)
*/
static void _FreeAttached(HEADER_Obj *pObj) {
	int i, NumItems;
	NumItems = GUI_ARRAY_GetNumItems(&pObj->Columns);
	for (i = 0; i < NumItems; i++) {
		HEADER_COLUMN *pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItem(&pObj->Columns, i);
		if (pColumn->hDrawObj) {
			GUI_ALLOC_Free(pColumn->hDrawObj);
		}
	}
	/* Delete attached objects (if any) */
	GUI_ARRAY_Delete(&pObj->Columns);
	_RestoreOldCursor();
}
#if (HEADER_SUPPORT_DRAG)
static int _GetItemIndex(HEADER_Obj *pObj, int x, int y) {
	int Item = -1;
	if ((y >= 0) && (y < WM_GetWindowSizeY(pObj))) {
		if (pObj) {
			int Index, xPos = 0, NumColumns;
			NumColumns = GUI_ARRAY_GetNumItems(&pObj->Columns);
			for (Index = 0; Index < NumColumns; Index++) {
				HEADER_COLUMN *pColumn;
				pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItem(&pObj->Columns, Index);
				xPos += pColumn->Width;
				if ((xPos >= (x - 4)) && (xPos <= (x + 4))) {
					Item = Index;
					if ((Index < (NumColumns - 1)) && (x < xPos)) {
						pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItem(&pObj->Columns, Index + 1);
						if (pColumn->Width == 0) {
							break;
						}
					}
				}
			}
		}
	}
	return Item;
}
#endif
#if (HEADER_SUPPORT_DRAG)
static void _HandlePID(HEADER_Obj *pObj, int x, int y, int Pressed) {
	int Hit = _GetItemIndex(pObj, x, y);
	/* set capture position () */
	if ((Pressed == 1) && (Hit >= 0) && (pObj->CapturePosX == -1)) {
		pObj->CapturePosX = x;
		pObj->CaptureItem = Hit;
	}
	/* set mouse cursor and capture () */
	if (Hit >= 0) {
		WM_SetCapture(pObj, 1);
#if GUI_SUPPORT_CURSOR
		if (!_pOldCursor) {
			_pOldCursor = GUI_CURSOR_Select(_pDefaultCursor);
		}
#endif
	}
	/* modify header */
	if ((pObj->CapturePosX >= 0) && (x != pObj->CapturePosX) && (Pressed == 1)) {
		int NewSize = HEADER_GetItemWidth(pObj, pObj->CaptureItem) + x - pObj->CapturePosX;
		if (NewSize >= 0) {
			HEADER_SetItemWidth(pObj, pObj->CaptureItem, NewSize);
			pObj->CapturePosX = x;
		}
	}
	/* release capture & restore cursor */
	if (Pressed <= 0) {
#if (GUI_SUPPORT_MOUSE)
		if (Hit == -1)
#endif
		{
			_RestoreOldCursor();
			pObj->CapturePosX = -1;
			WM_ReleaseCapture();
		}
	}
}
#endif
#if (HEADER_SUPPORT_DRAG & GUI_SUPPORT_MOUSE)
static void _OnMouseOver(HEADER_Obj *pObj, const GUI_PID_STATE *pState) {
	if (pState)
		_HandlePID(pObj, pState->x + pObj->ScrollPos, pState->y, -1);
}
#endif
#if (HEADER_SUPPORT_DRAG)
static void _OnTouch(HEADER_Obj *pObj, const GUI_PID_STATE *pState) {
	int Notification;
	if (pState) {  /* Something happened in our area (pressed or released) */
		_HandlePID(pObj, pState->x + pObj->ScrollPos, pState->y, pState->Pressed);
		Notification = pState->Pressed ?
			WM_NOTIFICATION_CLICKED : WM_NOTIFICATION_RELEASED;
	}
	else
		Notification = WM_NOTIFICATION_MOVED_OUT;
	WM_NotifyParent(pObj, Notification);
}
#endif
static void _HEADER_Callback(WM_MESSAGE *pMsg) {
	HEADER_Obj *pObj = pMsg->hWin;
	/* Let widget handle the standard messages */
	if (WIDGET_HandleActive(pObj, pMsg) == 0) {
		return;
	}
	switch (pMsg->MsgId) {
		case WM_PAINT:
			_OnPaint(pObj);
			break;
#if (HEADER_SUPPORT_DRAG)
		case WM_TOUCH:
			_OnTouch(pObj, (const GUI_PID_STATE *)pMsg->Data);
			break;
#endif
#if (HEADER_SUPPORT_DRAG & GUI_SUPPORT_MOUSE)
		case WM_MOUSEOVER:
			_OnMouseOver(pObj, (const GUI_PID_STATE *)pMsg->Data);
			break;
#endif
		case WM_DELETE:
			_FreeAttached(pObj); /* No return here ... WM_DefaultProc needs to be called */
		default:
			WM_DefaultProc(pMsg);
	}

}
HEADER_Handle HEADER_Create(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, int ExFlags) {
	return HEADER_CreateEx(x0, y0, xsize, ysize, hParent, Flags, ExFlags, Id);
}
HEADER_Handle HEADER_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
							  int WinFlags, int ExFlags, int Id) {
	HEADER_Handle hObj;
	GUI_USE_PARA(ExFlags);
	/* Create the window */

	if ((xsize == 0) && (x0 == 0) && (y0 == 0)) {
		GUI_RECT Rect;
		WM_GetInsideRectEx(hParent, &Rect);
		xsize = Rect.x1 - Rect.x0 + 1;
		x0 = Rect.x0;
		y0 = Rect.y0;
	}
	if (ysize == 0) {
		const WIDGET_EFFECT *pEffect = WIDGET_GetDefaultEffect();
		ysize = GUI_GetYDistOfFont(_pDefaultFont);
		ysize += 2 * _DefaultBorderV;
		ysize += 2 * (unsigned)pEffect->EffectSize;
	}
	WinFlags |= WM_CF_ANCHOR_LEFT | WM_CF_ANCHOR_RIGHT;
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, &_HEADER_Callback,
								  sizeof(HEADER_Obj) - sizeof(WM_Obj));
	if (hObj) {
		HEADER_Obj *pObj = (hObj);
		/* Init sub-classes */
		GUI_ARRAY_CREATE(&pObj->Columns);
		/* init widget specific variables */
		WIDGET__Init(&pObj->Widget, Id, 0);
		/* init member variables */
		pObj->BkColor = _DefaultBkColor;
		pObj->TextColor = _DefaultTextColor;
		pObj->pFont = _pDefaultFont;
		pObj->CapturePosX = -1;
		pObj->CaptureItem = -1;
		pObj->ScrollPos = 0;
	}
	else {
	}

	return hObj;
}
const GUI_CURSOR  *HEADER_SetDefaultCursor(const GUI_CURSOR *pCursor) {
	const GUI_CURSOR  *pOldCursor = _pDefaultCursor;
	_pDefaultCursor = pCursor;
	return pOldCursor;
}
RGB_COLOR HEADER_SetDefaultBkColor(RGB_COLOR Color) {
	RGB_COLOR OldColor = _DefaultBkColor;
	_DefaultBkColor = Color;
	return OldColor;
}
RGB_COLOR HEADER_SetDefaultTextColor(RGB_COLOR Color) {
	RGB_COLOR OldColor = _DefaultTextColor;
	_DefaultTextColor = Color;
	return OldColor;
}
int HEADER_SetDefaultBorderH(int Spacing) {
	int OldSpacing = _DefaultBorderH;
	_DefaultBorderH = Spacing;
	return OldSpacing;
}
int HEADER_SetDefaultBorderV(int Spacing) {
	int OldSpacing = _DefaultBorderV;
	_DefaultBorderV = Spacing;
	return OldSpacing;
}
const GUI_FONT  *HEADER_SetDefaultFont(const GUI_FONT  *pFont) {
	const GUI_FONT  *pOldFont = _pDefaultFont;
	_pDefaultFont = pFont;
	return pOldFont;
}
const GUI_CURSOR  *HEADER_GetDefaultCursor(void) { return _pDefaultCursor; }
RGB_COLOR          HEADER_GetDefaultBkColor(void) { return _DefaultBkColor; }
RGB_COLOR          HEADER_GetDefaultTextColor(void) { return _DefaultTextColor; }
int                HEADER_GetDefaultBorderH(void) { return _DefaultBorderH; }
int                HEADER_GetDefaultBorderV(void) { return _DefaultBorderV; }
const GUI_FONT  *HEADER_GetDefaultFont(void) { return _pDefaultFont; }
void HEADER_SetFont(HEADER_Handle hObj, const GUI_FONT  *pFont) {
	if (hObj) {
		HEADER_Obj *pObj;

		pObj = (hObj);
		pObj->pFont = pFont;
		WM_Invalidate(hObj);

	}
}
void HEADER_SetHeight(HEADER_Handle hObj, int Height) {
	if (hObj) {
		GUI_RECT Rect;
		WM_GetClientRectEx(hObj, &Rect);
		WM_SetSize(hObj, Rect.x1 - Rect.x0 + 1, Height);
		WM_Invalidate(WM_GetParent(hObj));
	}
}
void HEADER_SetTextColor(HEADER_Handle hObj, RGB_COLOR Color) {
	if (hObj) {
		HEADER_Obj *pObj;

		pObj = (hObj);
		pObj->TextColor = Color;
		WM_Invalidate(hObj);

	}
}
void HEADER_SetBkColor(HEADER_Handle hObj, RGB_COLOR Color) {
	if (hObj) {
		HEADER_Obj *pObj;

		pObj = (hObj);
		pObj->BkColor = Color;
		WM_Invalidate(hObj);

	}
}
void HEADER_SetTextAlign(HEADER_Handle hObj, unsigned int Index, int Align) {
	if (hObj) {
		HEADER_Obj *pObj;

		pObj = (hObj);
		if (Index <= GUI_ARRAY_GetNumItems(&pObj->Columns)) {
			HEADER_COLUMN *pColumn;
			pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItem(&pObj->Columns, Index);
			pColumn->Align = Align;
			WM_Invalidate(hObj);
		}

	}
}
void HEADER_SetScrollPos(HEADER_Handle hObj, int ScrollPos) {
	if (hObj && (ScrollPos >= 0)) {
		HEADER_Obj *pObj;

		pObj = (hObj);
		if (ScrollPos != pObj->ScrollPos) {
			pObj->ScrollPos = ScrollPos;
			WM_Invalidate(hObj);
			WM_Invalidate(WM_GetParent(hObj));
		}

	}
}
void HEADER_AddItem(HEADER_Handle hObj, int Width, const char *s, int Align) {
	if (hObj) {
		HEADER_Obj *pObj;
		HEADER_COLUMN Column;
		int Index;

		pObj = (hObj);
		if (!Width) {
			const GUI_FONT  *pFont = GUI_SetFont(pObj->pFont);
			Width = GUI_GetStringDistX(s) + 2 * (pObj->Widget.pEffect->EffectSize + _DefaultBorderH);
			GUI_SetFont(pFont);
		}
		Column.Width = Width;
		Column.Align = Align;
		Column.hDrawObj = 0;
		Index = GUI_ARRAY_GetNumItems(&pObj->Columns);
		if (GUI_ARRAY_AddItem(&pObj->Columns, &Column, sizeof(HEADER_COLUMN) + GUI__strlen(s) + 1) == 0) {
			HEADER_COLUMN *pColumn;
			pObj = (hObj);
			pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItem(&pObj->Columns, Index);
			GUI__strcpy(pColumn->acText, s);
			WM_Invalidate(hObj);
			WM_Invalidate(WM_GetParent(hObj));
		}

	}
}
void HEADER_DeleteItem(HEADER_Handle hObj, unsigned Index) {
	if (hObj) {
		HEADER_Obj *pObj;

		pObj = (hObj);
		if (Index < GUI_ARRAY_GetNumItems(&pObj->Columns)) {
			GUI_ARRAY_DeleteItem(&pObj->Columns, Index);
			WM_Invalidate(hObj);
			WM_Invalidate(WM_GetParent(hObj));
		}

	}
}
void HEADER_SetItemText(HEADER_Handle hObj, unsigned int Index, const char *s) {
	if (hObj) {
		HEADER_Obj *pObj;

		pObj = (hObj);
		if (Index < GUI_ARRAY_GetNumItems(&pObj->Columns)) {
			HEADER_COLUMN *pColumn;
			pColumn = (HEADER_COLUMN *)GUI_ARRAY_ResizeItem(&pObj->Columns, Index, sizeof(HEADER_COLUMN) + GUI__strlen(s));
			if (pColumn) {
				GUI__strcpy(pColumn->acText, s);
			}
		}

	}
}
void HEADER_SetItemWidth(HEADER_Handle hObj, unsigned int Index, int Width) {
	if (hObj && (Width >= 0)) {
		HEADER_Obj *pObj;

		pObj = (hObj);
		if (Index <= GUI_ARRAY_GetNumItems(&pObj->Columns)) {
			HEADER_COLUMN *pColumn;
			pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItem(&pObj->Columns, Index);
			if (pColumn) {
				pColumn->Width = Width;
				WM_Invalidate(hObj);
				WM__SendMsgNoData(WM_GetParent(hObj), WM_NOTIFY_CLIENTCHANGE);
				WM_Invalidate(WM_GetParent(hObj));
			}
		}

	}
}
int HEADER_GetHeight(HEADER_Handle hObj) {
	int Height = 0;
	if (hObj) {
		GUI_RECT Rect;
		WM_GetClientRectEx(hObj, &Rect);
		GUI_MoveRect(&Rect, -Rect.x0, -Rect.y0);
		Height = Rect.y1 - Rect.y0 + 1;
	}
	return Height;
}
int HEADER_GetItemWidth(HEADER_Handle hObj, unsigned int Index) {
	int Width = 0;
	if (hObj) {
		HEADER_Obj *pObj;

		pObj = (hObj);
		if (Index <= GUI_ARRAY_GetNumItems(&pObj->Columns)) {
			HEADER_COLUMN *pColumn;
			pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItem(&pObj->Columns, Index);
			Width = pColumn->Width;
		}

	}
	return Width;
}
int  HEADER_GetNumItems(HEADER_Handle hObj) {
	int NumCols = 0;
	if (hObj) {
		HEADER_Obj *pObj;

		pObj = (hObj);
		NumCols = GUI_ARRAY_GetNumItems(&pObj->Columns);

	}
	return NumCols;
}

void HEADER_SetBitmapEx(HEADER_Handle hObj, unsigned Index, const GUI_BITMAP *pBitmap, int x, int y) {
	HEADER__SetDrawObj(hObj, Index, GUI_DRAW_BITMAP_Create(pBitmap, x, y));
	WM_Invalidate(hObj);
}
void HEADER_SetBitmap(HEADER_Handle hObj, unsigned Index, const GUI_BITMAP *pBitmap) {
	HEADER_SetBitmapEx(hObj, Index, pBitmap, 0, 0);
}


HEADER_Handle HEADER_CreateAttached(WM_HWIN hParent, int Id, int SpecialFlags) {
	return HEADER_CreateEx(0, 0, 0, 0, hParent, WM_CF_SHOW, SpecialFlags, Id);
}

HEADER_Handle HEADER_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	HEADER_Handle  hThis;
	GUI_USE_PARA(cb);
	hThis = HEADER_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							hWinParent, pCreateInfo->Flags, 0, pCreateInfo->Id);
	return hThis;
}

void HEADER__SetDrawObj(HEADER_Handle hObj, unsigned Index, GUI_DRAW_HANDLE hDrawObj) {
	if (hObj) {
		HEADER_Obj *pObj;

		pObj = (hObj);
		if (Index <= GUI_ARRAY_GetNumItems(&pObj->Columns)) {
			HEADER_COLUMN *pColumn;
			pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItem(&pObj->Columns, Index);
			if (pColumn) {
				GUI_ALLOC_FreePtr(&pColumn->hDrawObj);
				pColumn->hDrawObj = hDrawObj;
			}
		}

	}
}
