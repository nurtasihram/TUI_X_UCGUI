#include "GUI_Protected.h"

#include "WM_Intern.h"

#include "SCROLLBAR.h"
#include "SCROLLBAR_Private.h"

SCROLLBAR_Obj::Properties SCROLLBAR_Obj::DefaultProps;

static int _GetArrowSize(SCROLLBAR_Obj *pObj) {
	auto xSize = WIDGET__GetXSize(pObj);
	auto ySize = WIDGET__GetYSize(pObj);
	auto r = ySize / 2 + 5;
	if (r > xSize - 5)
		r = xSize - 5;
	return r;
}
/*********************************************************************
*
*       _WIDGET__RECT2VRECT
*
* Purpose:
*   Convert rectangle in real coordinates into virtual coordinates
*
* Add. info:
*   This function could eventualy be made none-static and move into
*   a module of its own.
*/
static void _WIDGET__RECT2VRECT(const WIDGET *pWidget, GUI_RECT *pRect) {
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		int xSize = pWidget->Rect.x1 - pWidget->Rect.x0 + 1;
		int x0, x1;
		x0 = pRect->x0;
		x1 = pRect->x1;
		pRect->x0 = pRect->y0;
		pRect->x1 = pRect->y1;
		pRect->y1 = xSize - 1 - x0;
		pRect->y0 = xSize - 1 - x1;
	}
}
/*********************************************************************
*
*       _CalcPositions
*
*  Calculates all positions required for drawing or for mouse / touch
*  evaluation.
*/
static void _CalcPositions(SCROLLBAR_Obj *pObj, SCROLLBAR_POSITIONS *pPos) {
	int xSizeArrow, xSize, xSizeMoveable, ThumbSize, NumItems, xSizeThumbArea;
	WM_HWIN hWin;
	GUI_RECT r, rSub;
	int x0, y0;
	r = pObj->Rect;
	x0 = r.x0;
	y0 = r.y0;
	pPos->x1 = (pObj->State & WIDGET_STATE_VERTICAL) ? r.y1 : r.x1;
	/* Subtract the rectangle of the other scrollbar (if existing and visible) */
	if (pObj->Id == GUI_ID_HSCROLL) {
		hWin = WM_GetScrollbarV(pObj->hParent);
		if (hWin) {
			rSub = WM_GetWindowRect(hWin);
			if (r.x1 == rSub.x1) {
				r.x1 = rSub.x0 - 1;
			}
		}
	}
	if (pObj->Id == GUI_ID_VSCROLL) {
		hWin = WM_GetScrollbarH(pObj->hParent);
		if (hWin) {
			rSub = WM_GetWindowRect(hWin);
			if (r.y1 == rSub.y1) {
				r.y1 = rSub.y0 - 1;
			}
		}
	}
	/* Convert coordinates of this window */
	r += GUI_POINT{-x0, -y0};
	/* Convert real into virtual coordinates */
	_WIDGET__RECT2VRECT(pObj, &r);
	NumItems = pObj->ScrollState.NumItems;
	xSize = r.x1 - r.x0 + 1;
	xSizeArrow = _GetArrowSize(pObj);
	xSizeThumbArea = xSize - 2 * xSizeArrow;     /* Number of pixels available for thumb and movement */
	ThumbSize = GUI__DivideRound(xSizeThumbArea * pObj->ScrollState.PageSize, NumItems);
	if (ThumbSize < 4) {
		ThumbSize = 4;
	}
	if (ThumbSize > xSizeThumbArea) {
		ThumbSize = xSizeThumbArea;
	}
	xSizeMoveable = xSizeThumbArea - ThumbSize;
	pPos->x0_LeftArrow = r.x0;
	pPos->x1_LeftArrow = xSizeArrow - 1;
	pPos->x1_RightArrow = xSize - 1;
	pPos->x0_RightArrow = xSize - xSizeArrow;
	pPos->x0_Thumb = pPos->x1_LeftArrow + 1 + GUI__DivideRound(xSizeMoveable * pObj->ScrollState.v, NumItems - pObj->ScrollState.PageSize);
	pPos->x1_Thumb = pPos->x0_Thumb + ThumbSize - 1;
	pPos->xSizeMoveable = xSizeMoveable;
	pPos->ThumbSize = ThumbSize;
}
static void _DrawTriangle(WIDGET *pWidget, int x, int y, int Size, int Inc) {
	if (pWidget->State & WIDGET_STATE_VERTICAL)
		for (; Size >= 0; Size--, x += Inc)
			GUI_DrawHLine(x, y - Size, y + Size);
	else 
		for (; Size >= 0; Size--, x += Inc)
			GUI_DrawVLine(x, y - Size, y + Size);
}
static void _OnPaint(SCROLLBAR_Obj *pObj) {
	int ArrowSize, ArrowOff;
	SCROLLBAR_POSITIONS Pos;
	GUI_RECT r, rClient;
	/*
	  Get / calc position info
	*/
	_CalcPositions(pObj, &Pos);
	rClient = WIDGET__GetClientRect(pObj);
	r = rClient;
	ArrowSize = ((r.y1 - r.y0) / 3) - 1;
	ArrowOff = 3 + ArrowSize + ArrowSize / 3;
	/*
	  Draw left Arrow
	*/
	GUI_SetColor(pObj->Props.Color);
	r = rClient;
	r.x0 = Pos.x0_LeftArrow;
	r.x1 = Pos.x1_LeftArrow;
	WIDGET__FillRect(pObj, r);
	GUI_SetColor(pObj->Props.aBkColor[1]);
	_DrawTriangle(pObj, r.x0 + ArrowOff, (r.y1 - r.y0) >> 1, ArrowSize, -1);
	WIDGET__EFFECT_DrawUpRect(pObj, r);
	/*
	  Draw the thumb area which is not covered by the thumb
	*/
	GUI_SetColor(pObj->Props.aBkColor[0]);
	r.x0 = Pos.x1_LeftArrow + 1;
	r.x1 = Pos.x0_Thumb - 1;
	WIDGET__FillRect(pObj, r);
	r = rClient;
	r.x0 = Pos.x1_Thumb + 1;
	r.x1 = Pos.x0_RightArrow - 1;
	WIDGET__FillRect(pObj, r);
	/*
	  Draw Thumb
	*/
	r = rClient;
	r.x0 = Pos.x0_Thumb;
	r.x1 = Pos.x1_Thumb;
	GUI_SetColor(pObj->Props.Color);
	WIDGET__FillRect(pObj, r);
	WIDGET__EFFECT_DrawUpRect(pObj, r);
	/*
	  Draw right Arrow
	*/
	GUI_SetColor(pObj->Props.Color);
	r.x0 = Pos.x0_RightArrow;
	r.x1 = Pos.x1_RightArrow;
	WIDGET__FillRect(pObj, r);
	GUI_SetColor(pObj->Props.aBkColor[1]);
	_DrawTriangle(pObj, r.x1 - ArrowOff, (r.y1 - r.y0) >> 1, ArrowSize, 1);
	WIDGET__EFFECT_DrawUpRect(pObj, r);
	/*
	  Draw overlap area (if any ...)
	*/
	if (Pos.x1_RightArrow != Pos.x1) {
		r.x0 = Pos.x1_RightArrow + 1;
		r.x1 = Pos.x1;
		GUI_SetColor(pObj->Props.Color);
		WIDGET__FillRect(pObj, r);
	}
}
static void _ScrollbarPressed(SCROLLBAR_Obj *pObj) {
	WIDGET_OrState(pObj, SCROLLBAR_STATE_PRESSED);
	if (pObj->Status & WM_SF_ISVIS) {
		WM_NotifyParent(pObj, WM_NOTIFICATION_CLICKED);
	}
}
static void _ScrollbarReleased(SCROLLBAR_Obj *pObj) {
	WIDGET_AndState(pObj, SCROLLBAR_STATE_PRESSED);
	if (pObj->Status & WM_SF_ISVIS) {
		WM_NotifyParent(pObj, WM_NOTIFICATION_RELEASED);
	}
}
static void _OnTouch(SCROLLBAR_Obj *pObj, const GUI_PID_STATE *pState) {
	SCROLLBAR_POSITIONS Pos;
	if (pState) { /* Something happened in our area (pressed or released) */
		if (pState->Pressed) {
			int Range;
			int x;
			int Sel = pObj->ScrollState.v;
			_CalcPositions(pObj, &Pos);
			Range = pObj->ScrollState.NumItems - pObj->ScrollState.PageSize;
			/* Swap mouse coordinates if necessary */
			if (pObj->State & WIDGET_STATE_VERTICAL)
				x = pState->y;
			else
				x = pState->x;
			if (x <= Pos.x1_LeftArrow) /* left arrow (line left) */
				Sel--;
			else if (x < Pos.x0_Thumb) /* left area  (page left) */
				Sel -= pObj->ScrollState.PageSize;
			else if (x <= Pos.x1_Thumb) {      /* Thumb area */
				if (Pos.xSizeMoveable > 0) {
					x = x - Pos.ThumbSize / 2 - Pos.x1_LeftArrow - 1;
					Sel = GUI__DivideRound(Range * x, Pos.xSizeMoveable);
				}
			}
			else if (x < Pos.x0_RightArrow) /* right area (page right) */
				Sel += pObj->ScrollState.PageSize;
			else  if (x <= Pos.x1_RightArrow)
				Sel++;
			/* WM_SetFocus(hObj); */
			WM_SetCapture(pObj, 1);
			SCROLLBAR_SetValue(pObj, Sel);
			if (!(pObj->State & SCROLLBAR_STATE_PRESSED))
				_ScrollbarPressed(pObj);
		}
		/* React only if button was pressed before ... avoid problems with moving / hiding windows above (such as dropdown) */
		else if (pObj->State & SCROLLBAR_STATE_PRESSED)
			_ScrollbarReleased(pObj);
	}
}
static char _OnKey(SCROLLBAR_Obj *pObj, const WM_KEY_INFO *pInfo) {
	if (pInfo->PressedCnt > 0) {
		switch (pInfo->Key) {
			case GUI_KEY_RIGHT:
			case GUI_KEY_DOWN:
				SCROLLBAR_Inc(pObj);
				return 1; /* Send to parent by not doing anything */
			case GUI_KEY_LEFT:
			case GUI_KEY_UP:
				SCROLLBAR_Dec(pObj);
				return 1; /* Send to parent by not doing anything */
		}
	}
	return 0;
}
static void _OnSetScrollState(SCROLLBAR_Obj *pObj, const WM_SCROLL_STATE *pState) {
	if (pObj->ScrollState != *pState) {
		pObj->ScrollState = *pState;
		WM_Invalidate(pObj);
	}
}
void SCROLLBAR__InvalidatePartner(SCROLLBAR_Handle hObj) {     /* Invalidate the partner, since it is also affected */
	WM_Invalidate(WM_GetScrollPartner(hObj));
	WM_SendMessageNoPara(WM_GetParent(hObj), WM_NOTIFY_CLIENTCHANGE);   /* Client area may have changed */
}
static WM_PARAM _SCROLLBAR_Callback(WM_HWIN hWin, int MsgId, WM_PARAM Data) {
	auto pObj = (SCROLLBAR_Obj *)hWin;
	/* Let widget handle the standard messages */
	if (!WIDGET_HandleActive(pObj, MsgId, &Data))
		return Data;
	switch (MsgId) {
		case WM_PAINT:
			_OnPaint(pObj);
			return 0;
		case WM_DELETE:
			SCROLLBAR__InvalidatePartner(pObj);
			return 0;
		case WM_TOUCH:
			_OnTouch(pObj, (const GUI_PID_STATE *)Data);
			return 0;
		case WM_KEY:
			if (_OnKey(pObj, (const WM_KEY_INFO *)Data))
				return 0; /* Send to parent by not doing anything */
			break;
		case WM_SET_SCROLL_STATE:
			_OnSetScrollState(pObj, (const WM_SCROLL_STATE *)Data);
			return 0;
		case WM_GET_SCROLL_STATE: 
			*(WM_SCROLL_STATE *)Data = pObj->ScrollState;
			break;
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}
/* Note: the parameters to a create function may vary.
		 Some widgets may have multiple create functions */
SCROLLBAR_Handle SCROLLBAR_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
									int WinFlags, int ExFlags, int Id) {
	SCROLLBAR_Handle hObj;

	/* Set defaults if necessary */
	if ((xsize == 0) && (ysize == 0)) {
		GUI_RECT Rect = WM_GetInsideRect(hParent);
		if (ExFlags & SCROLLBAR_CF_VERTICAL) {
			xsize = SCROLLBAR_Obj::DefaultWidth;
			x0 = Rect.x1 + 1 - xsize;
			y0 = Rect.y0;
			ysize = Rect.y1 - Rect.y0 + 1;
		}
		else {
			ysize = SCROLLBAR_Obj::DefaultWidth;
			y0 = Rect.y1 + 1 - ysize;
			x0 = Rect.x0;
			xsize = Rect.x1 - Rect.x0 + 1;
		}
	}
	/* Create the window */
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, _SCROLLBAR_Callback,
								  sizeof(SCROLLBAR_Obj) - sizeof(WM_Obj));
	if (hObj) {
		auto pObj = (SCROLLBAR_Obj *)hObj;
		uint16_t InitState;
		/* Handle SpecialFlags */
		InitState = 0;
		if (ExFlags & SCROLLBAR_CF_VERTICAL) {
			InitState |= WIDGET_CF_VERTICAL;
		}
		if (ExFlags & SCROLLBAR_CF_FOCUSSABLE) {
			InitState |= WIDGET_STATE_FOCUSSABLE;
		}
		if ((Id != GUI_ID_HSCROLL) && (Id != GUI_ID_VSCROLL)) {
			InitState |= WIDGET_STATE_FOCUSSABLE;
		}
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, InitState);
		/* init member variables */
		pObj->Props = SCROLLBAR_Obj::DefaultProps;
		pObj->ScrollState.NumItems = 100;
		pObj->ScrollState.PageSize = 10;
		pObj->ScrollState.v = 0;
		SCROLLBAR__InvalidatePartner(hObj);
	}
	else {
	}

	return hObj;
}
void SCROLLBAR_Dec(SCROLLBAR_Handle hObj) {
	SCROLLBAR_AddValue(hObj, -1);
}
void SCROLLBAR_Inc(SCROLLBAR_Handle hObj) {
	SCROLLBAR_AddValue(hObj, 1);
}
void SCROLLBAR_AddValue(SCROLLBAR_Handle hObj, int Add) {
	auto pObj = (SCROLLBAR_Obj *)hObj;
	if (hObj) {

		SCROLLBAR_SetValue(pObj, pObj->ScrollState.v + Add);

	}
}
void SCROLLBAR_SetValue(SCROLLBAR_Handle hObj, int v) {
	auto pObj = (SCROLLBAR_Obj *)hObj;
	int Max;
	if (hObj) {

		Max = pObj->ScrollState.NumItems - pObj->ScrollState.PageSize;
		if (Max < 0)
			Max = 0;
		/* Put in min/max range */
		if (v < 0) {
			v = 0;
		}
		if (v > Max) {
			v = Max;
		}
		if (pObj->ScrollState.v != v) {
			pObj->ScrollState.v = v;
			WM_Invalidate(hObj);
			WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
		}

	}
}
void SCROLLBAR_SetNumItems(SCROLLBAR_Handle hObj, int NumItems) {
	auto pObj = (SCROLLBAR_Obj *)hObj;
	if (hObj) {

		if (pObj->ScrollState.NumItems != NumItems) {
			pObj->ScrollState.NumItems = NumItems;
			WM_Invalidate(hObj);
		}

	}
}
void SCROLLBAR_SetPageSize(SCROLLBAR_Handle hObj, int PageSize) {
	auto pObj = (SCROLLBAR_Obj *)hObj;
	if (hObj) {
		if (pObj->ScrollState.PageSize != PageSize) {
			pObj->ScrollState.PageSize = PageSize;
			WM_Invalidate(hObj);
		}
	}
}
void  SCROLLBAR_SetState(SCROLLBAR_Handle hObj, const WM_SCROLL_STATE *pState) {
	if (hObj) {
		SCROLLBAR_SetPageSize(hObj, pState->PageSize);
		SCROLLBAR_SetNumItems(hObj, pState->NumItems);
		SCROLLBAR_SetValue(hObj, pState->v);
	}
}


SCROLLBAR_Handle SCROLLBAR_Create(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int WinFlags, int SpecialFlags) {
	return SCROLLBAR_CreateEx(x0, y0, xsize, ysize, hParent, WinFlags, SpecialFlags, Id);
}
SCROLLBAR_Handle SCROLLBAR_CreateAttached(WM_HWIN hParent, int SpecialFlags) {
	SCROLLBAR_Handle  hThis;
	int Id;
	int WinFlags;
	if (SpecialFlags & SCROLLBAR_CF_VERTICAL) {
		Id = GUI_ID_VSCROLL;
		WinFlags = WM_CF_SHOW | WM_CF_STAYONTOP | WM_CF_ANCHOR_RIGHT | WM_CF_ANCHOR_TOP | WM_CF_ANCHOR_BOTTOM;
	}
	else {
		Id = GUI_ID_HSCROLL;
		WinFlags = WM_CF_SHOW | WM_CF_STAYONTOP | WM_CF_ANCHOR_BOTTOM | WM_CF_ANCHOR_LEFT | WM_CF_ANCHOR_RIGHT;
	}
	hThis = SCROLLBAR_CreateEx(0, 0, 0, 0, hParent, WinFlags, SpecialFlags, Id);
	WM_NotifyParent(hThis, WM_NOTIFICATION_SCROLLBAR_ADDED);
	return hThis;
}
SCROLLBAR_Handle SCROLLBAR_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	SCROLLBAR_Handle  hThis;
	GUI_USE_PARA(cb);
	hThis = SCROLLBAR_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							   hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
	return hThis;
}

int SCROLLBAR_GetValue(SCROLLBAR_Handle hObj) {
	int r = 0;
	auto pObj = (SCROLLBAR_Obj *)hObj;
	if (hObj) {
		r = pObj->ScrollState.v;
	}
	return r;
}

int SCROLLBAR_SetWidth(SCROLLBAR_Handle hObj, int Width) {
	int r = 0;
	if (hObj) {
		r = WIDGET_SetWidth(hObj, Width);
		SCROLLBAR__InvalidatePartner(hObj);     /* Invalidate the partner, since it is also affected */
	}
	return r;
}
