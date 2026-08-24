#include "WM_Intern.h"
#include "GUI_Protected.h"
#include "GUIDebug.h"

#define ASSIGN_IF_LESS(v0,v1) if (v1<v0) v0=v1

static bool IsActive;

#pragma region Focus
static WObj *_GetNextChild(WObj * pParent, WObj * pChild) {
	WObj *pObj = nullptr;
	if (pChild)
		pObj = pChild->pNext;
	if (!pObj)
		pObj = pParent->pFirstChild;
	if (pObj != pChild)
		return pObj;
	return nullptr;
}
WObj *WM_SetFocusOnNextChild(WObj * pParent) {
	if (auto pChild = WM__GetFocussedChild(pParent)) {
		do {
			if (!(pChild = _GetNextChild(pParent, pChild)))
				return nullptr;
		} while (pChild->IsFocussable());
		if (!pChild->SetFocus())
			return pChild;
	}
	return nullptr;
}
static WObj *_GetPrevChild(WObj * pChild) {
	WObj *pObj = nullptr;
	if (pChild)
		pObj = pChild->PrevSibling();
	if (!pObj)
		pObj = pChild->LastSibling();
	if (pObj != pChild)
		return pObj;
	return nullptr;
}
WObj *WM_SetFocusOnPrevChild(WObj * pParent) {
	WObj *pChild;
	WObj *pWin;
	pChild = WM__GetFocussedChild(pParent);
	pChild = _GetPrevChild(pChild);
	pWin = pChild;
	while (!pWin->IsFocussable() && pWin) {
		pWin = _GetPrevChild(pWin);
		if (pWin == pChild)
			break;
	}
	if (!pWin->SetFocus())
		return pWin;
	return 0;
}
#pragma endregion

uint8_t WM__PaintCallbackCnt;      /* Public for assertions only */
static WObj *pWinNextDraw;

PID_STATE WM_PID__StateLast{ 0 };

#pragma region CriticalHandles
struct CriticalHandle {
	static CriticalHandle *pFirst;
	CriticalHandle *pNext = nullptr;
	WObj *pWin = nullptr;
	static void Check(WObj * pWin) {
		for (auto pCH = pFirst; pCH; pCH = pCH->pNext)
			if (pCH->pWin == pWin)
				pCH->pWin = nullptr;
	}
	void Add() {
		pNext = pFirst;
		pFirst = this;
	}
	void Remove() {
		CriticalHandle *pLast = nullptr;
		for (auto pCH = pFirst; pCH; pCH = pCH->pNext) {
			if (pCH == this) {
				if (pLast)
					pLast->pNext = pCH->pNext;
				pFirst = pCH->pNext;
				break;
			}
			pLast = pCH;
		}
	}
};
CriticalHandle *CriticalHandle::pFirst = nullptr;
#pragma endregion

static void _SetClipRectUserIntersect(const RECT *prSrc) {
	if (GUI.WM__pUserClipRect == nullptr) {
		LCD_SetClipRectEx(prSrc);
	}
	else {
		auto r = *GUI.WM__pUserClipRect;
		if (WObj::pWinActive)
			r += WObj::pWinActive->GetOrg(); /* Convert User ClipRect into screen coordinates */
		/* Set intersection as clip rect */
		r &= *prSrc;
		LCD_SetClipRectEx(&r);
	}
}
static void _DeleteAllChildren(WObj * pFirstChild) {
	for (auto pChild = pFirstChild; pChild; ) {
		auto pNext = pChild->pNext;
		WM_DeleteWindow(pChild);
		pChild = pNext;
	}
}

void WM_DeleteWindow(WObj * pWin) {
	if (!pWin)
		return;
	WM_ASSERT_NOT_IN_PAINT();
	if (!WObj::IsWindow(pWin))
		return;
	pWinNextDraw = nullptr; /* Make sure the window will no longer receive drawing messages */
	/* Make sure that focus is set to an existing window */
	if (WObj::pWinFocus == pWin)
		WObj::pWinFocus = nullptr;
	WObj::ReleaseCapture(); /* Make sure the window does not have capture */
	/* check if critical handles are affected. If so, reset the window handle to 0 */
	CriticalHandle::Check(pWin);
	/* Inform parent */
	pWin->NotifyParent(WM_NOTIFICATION_CHILD_DELETED);
	/* Delete all children */
	_DeleteAllChildren(pWin->pFirstChild);
	/* Send WM_DELETE message to window in order to inform window itself */
	pWin->Require(WM_DELETE);     /* tell window about it */
	pWin->_Detach();
	/* Remove window from window stack */
	pWin->_RemoveFromLinList();
	/* Make sure window is no longer counted as invalid */
	if (pWin->Status & WC_ACTIVATE)
		WObj::NumInvalidWindows--;
	/* Free window memory */
	WObj::NumWindows--;
	GUI_ALLOC_Free(pWin);
	/* Select a valid window */
	WM_SelectWindow(WObj::pWinFirst);
}
WObj * WM_SelectWindow(WObj * pWin) {
	auto pWinPrev = WObj::pWinActive;
	WM_ASSERT_NOT_IN_PAINT();
	if (pWin == 0) {
		pWin = WObj::pWinFirst;
	}
	/* Select new window */
	WObj::pWinActive = pWin;
	LCD_SetClipRectMax();             /* Drawing operations will clip ... If WM is deactivated, allow all */
	GUI.Off = pWin->Rect.LeftTop();
	return pWinPrev;
}
WObj * WM_GetActiveWindow(void) {
	return WObj::pWinActive;
}

#pragma region IVR
static void _Findy1(WObj *pWin, RECT *pRect, RECT *pParentRect) {
	for (; pWin; pWin = pWin->pNext) {
		int Status = pWin->Status;
		/* Check if this window affects us at all */
		if (!(Status & WC_VISIBLE))
			continue;
		auto rWinClipped = pWin->Rect; /* Window rect, clipped to part inside of ancestors */
		if (pParentRect)
			rWinClipped &= *pParentRect;
		/* Check if this window affects us at all */
		if (!(rWinClipped <= *pRect))
			continue;
		if (pWin->Rect.y0 > pRect->y0) {
			ASSIGN_IF_LESS(pRect->y1, rWinClipped.y0 - 1);      /* Check upper border of window */
		}
		else {
			ASSIGN_IF_LESS(pRect->y1, rWinClipped.y1);        /* Check lower border of window */
		}
	}
}
static bool _Findx0(WObj *pWin, RECT *pRect, RECT *pParentRect) {
	for (; pWin; pWin = pWin->pNext) {
		int Status = pWin->Status;
		if (!(Status & WC_VISIBLE))
			continue;
		/* If window is not visible, it can be safely ignored */
		auto rWinClipped = pWin->Rect;               /* Window rect, clipped to part inside of ancestors */
		if (pParentRect)
			rWinClipped &= *pParentRect;
		/* Check if this window affects us at all */
		if (rWinClipped <= *pRect) {
			pRect->x0 = rWinClipped.x1 + 1;
			return true;
		}
	}
	return false;
}
static void _Findx1(WObj *pWin, RECT *pRect, RECT *pParentRect) {
	for (; pWin; pWin = pWin->pNext) {
		int Status = pWin->Status;
		if (!(Status & WC_VISIBLE))
			continue;
		/* If window is not visible, it can be safely ignored */
		RECT rWinClipped = pWin->Rect; /* Window rect, clipped to part inside of ancestors */
		if (pParentRect)
			rWinClipped &= *pParentRect;
		/* Check if this window affects us at all */
		if (!(rWinClipped <= *pRect))
			continue;
		pRect->x1 = rWinClipped.x0 - 1;
	}
}

struct {
	RECT ClientRect;
	RECT CurRect;
	int Cnt;
	int EntranceCnt;
} static _ClipContext;

/*********************************************************************
*
*       IVR calculation
*
**********************************************************************
IVRs are invalid rectangles. When redrawing, only the portion of the
window which is
  a) within the window-rectangle
  b) not covered by an other window
  c) marked as invalid
  is actually redrawn. Unfortunately, this section is not always
  rectangular. If the window is partially covered by an other window,
  it consists of the sum of multiple rectangles. In all drawing
  operations, we have to iterate over every one of these rectangles in
  order to make sure the window is drawn completly.
Function works as follows:
  STEP 1: - Set upper left coordinates to next pixel. If end of line (right border), goto next line -> (r.x0, r.y0)
  STEP 2: - Check if we are done, return if we are.
  STEP 3: - If we are at the left border, find max. heigtht (r.y1) by iterating over windows above
  STEP 4: - Find x0 for the given y0, y1 by iterating over windows above
  STEP 5: - If r.x0 out of right border, this stripe is done. Set next stripe and goto STEP 2
  STEP 6: - Find r.x1. We have to Iterate over all windows which are above
*/
#if WM_SUPPORT_OBSTRUCT
static bool _FindNext_IVR(void) {
	WObj *pParent;
	WObj *pAWin;
	auto r = _ClipContext.CurRect;  /* temps  so we do not have to work with pointers too much */
	/*
	   STEP 1:
		 Set the next position which could be part of the next IVR
		 This will be the first unhandle pixel in reading order, i.e. next one to the right
		 or next one down if we are at the right border.
	*/
	if (!_ClipContext.Cnt) {       /* First IVR starts in upper left */
		r.x0 = _ClipContext.ClientRect.x0;
		r.y0 = _ClipContext.ClientRect.y0;
	}
	else {
		r.x0 = _ClipContext.CurRect.x1 + 1;
		r.y0 = _ClipContext.CurRect.y0;
		if (r.x0 > _ClipContext.ClientRect.x1) {
		NextStripe:  /* go down to next stripe */
			r.x0 = _ClipContext.ClientRect.x0;
			r.y0 = _ClipContext.CurRect.y1 + 1;
		}
	}
	/*
	   STEP 2:
		 Check if we are done completely.
	*/
	if (r.y0 > _ClipContext.ClientRect.y1)
		return false;
	/* STEP 3:
		 Find out the max. height (r.y1) if we are at the left border.
		 Since we are using the same height for all IVRs at the same y0,
		 we do this only for the leftmost one.
	*/
	pAWin = WObj::pWinActive;
	if (r.x0 == _ClipContext.ClientRect.x0) {
		r.y1 = _ClipContext.ClientRect.y1;
		r.x1 = _ClipContext.ClientRect.x1;
		/* Iterate over all windows which are above */
		/* Check all siblings above (Iterate over Parents and top siblings (hNext) */
		for (pParent = WObj::pWinActive; pParent; pParent = pParent->pParent) {
			_Findy1(pParent->pNext, &r, nullptr);
		}
		/* Check all children */
		_Findy1(pAWin->pFirstChild, &r, nullptr);
	}
	/*
	  STEP 4
		Find out x0 for the given y0, y1 by iterating over windows above.
		if we find one that intersects, adjust x0 to the right.
	*/
Find_x0:
	r.x1 = r.x0;
	/* Iterate over all windows which are above */
	/* Check all siblings above (siblings of window, siblings of parents, etc ...) */
#if 0   /* This is a planned, but not yet released optimization */
	if (Status & WC_DONT_CLIP_SIBLINGS)
		pParent = pAWin->pParent;
	else
#endif
		pParent = WObj::pWinActive;
		for (; pParent; pParent = pParent->pParent) {
			if (_Findx0(pParent->pNext, &r, nullptr))
				goto Find_x0;
		}
	/* Check all children */
	if (_Findx0(pAWin->pFirstChild, &r, nullptr))
		goto Find_x0;
	/*
	 STEP 5:
	   If r.x0 out of right border, this stripe is done. Set next stripe and goto STEP 2
	   Find out x1 for the given x0, y0, y1
	*/
	r.x1 = _ClipContext.ClientRect.x1;
	if (r.x1 < r.x0) {/* horizontal border reached ? */
		_ClipContext.CurRect = r;
		goto NextStripe;
	}
	/*
	 STEP 6:
	   Find r.x1. We have to Iterate over all windows which are above
	*/
	/* Check all siblings above (Iterate over Parents and top siblings (hNext) */
#if 0   /* This is a planned, but not yet released optimization */
	if (Status & WC_DONT_CLIP_SIBLINGS)
		pParent = pAWin->pParent;
	else
#endif
		pParent = WObj::pWinActive;
		for (; pParent; pParent = pParent->pParent) {
			_Findx1(pParent->pNext, &r, nullptr);
		}
	/* Check all children */
	_Findx1(pAWin->pFirstChild, &r, nullptr);
	/* We are done. Return the rectangle we found in the _ClipContext. */
	if (_ClipContext.Cnt > 200)
		return false;  /* error !!! This should not happen !*/
	_ClipContext.CurRect = r;
	return true;  /* IVR is valid ! */
}
#else
static bool _FindNext_IVR(void) {
	if (!_ClipContext.Cnt) {
		_ClipContext.CurRect = WObj::pWinActive->Rect;
		return true;  /* IVR is valid ! */
	}
	return false;  /* Nothing left to draw */
}
#endif
bool WM__GetNextIVR(void) {
#if GUI_SUPPORT_CURSOR
	static char _CursorHidden;
#endif
	/* If WM is not active, we have no rectangles to return */
	if (!IsActive)
		return false;
	if (_ClipContext.EntranceCnt > 1) {
		_ClipContext.EntranceCnt--;
		return false;
	}
#if GUI_SUPPORT_CURSOR
	if (_CursorHidden) {
		_CursorHidden = 0;
		GUI_CURSOR__TempShow();
	}
#endif
	++_ClipContext.Cnt;
	/* Find next rectangle and use it as ClipRect */
	if (!_FindNext_IVR()) {
		_ClipContext.EntranceCnt--;  /* This search is over ! */
		return false;        /* Could not find an other one ! */
	}
	WM__ActivateClipRect();
	/* Hide cursor if necessary */
#if GUI_SUPPORT_CURSOR
	_CursorHidden = GUI_CURSOR__TempHide(_ClipContext.CurRect);
#endif
	return true;
}
bool WM__InitIVRSearch(RECT rcMax) {
	/* If WM is not active -> nothing to do, leave cliprect alone */
	if (!IsActive) {
		WM__ActivateClipRect();
		return true;
	}
	/* If we entered multiple times, leave Cliprect alone */
	if (++_ClipContext.EntranceCnt > 1)
		return true;
	auto pAWin = WObj::pWinActive;
	_ClipContext.Cnt = -1;
	/* When using callback mechanism, it is legal to reduce drawing
	   area to the invalid area ! */
	RECT r;
	if (WM__PaintCallbackCnt)
		r = pAWin->InvalidRect;
	else if (pAWin->Status & WC_VISIBLE) /* Not using callback mechanism, therefor allow entire rectangle */
		r = pAWin->Rect;
	else {
		--_ClipContext.EntranceCnt;
		return false;  /* window is not even visible ! */
	}
	/* If the drawing routine has specified a rectangle, use it to reduce the rectangle */
	r &= rcMax;
	/* If user has reduced the cliprect size, reduce the rectangle */
	if (GUI.WM__pUserClipRect) {
		auto pWin = pAWin;
		auto rUser = *(GUI.WM__pUserClipRect);
		rUser += pWin->GetOrg();
		r &= rUser;
	}
	/* Iterate over all ancestors and clip at their borders. If there is no visible part, we are done */
	if (!WObj::pWinActive->_ClipAtParentBorders(r)) {
		--_ClipContext.EntranceCnt;
		return false;           /* Nothing to draw */
	}
	/* Store the rectangle and find the first rectangle of the area */
	_ClipContext.ClientRect = r;
	return WM__GetNextIVR();
}
void WM__ActivateClipRect(void) {
	if (IsActive)
		_SetClipRectUserIntersect(&_ClipContext.CurRect);
	else {    /* Window manager disabled, typically because meory device is active */
		RECT r;
		WObj *pAWin;
		pAWin = WObj::pWinActive;
		r = pAWin->Rect;
		/* Take UserClipRect into account */
		_SetClipRectUserIntersect(&r);
	}
}
#pragma endregion

static void _Paint1(WObj *pWin) {
	int Status = pWin->Status;
	/* Send WM_PAINT if window is visible and a callback is defined */
	if ((pWin->cb != nullptr) && (Status & WC_VISIBLE)) {
		WM__PaintCallbackCnt++;
		if (Status & WC_LATE_CLIP) {
			pWin->Require(WM_PAINT, (WM_PARAM)&pWin->InvalidRect);
		}
		else {
			WM_Iterate(pWin->InvalidRect, [&] {
				pWin->Require(WM_PAINT, (WM_PARAM)&pWin->InvalidRect);
			});
		}
		WM__PaintCallbackCnt--;
	}
}

/*********************************************************************
*
*       Callback for Paint message
*
/*********************************************************************
*
*       WM__PaintWinAndOverlays
*
* Purpose
*   Paint the given window and all overlaying windows
*   (transparent children and transparent top siblings)
*/
void WM__PaintWinAndOverlays(WObj *pWin) {
	_Paint1(pWin); /* Draw the window itself */
}
/*********************************************************************
*
*       _cbPaintMemDev
*
* Purpose:
*   This is the routine called by the banding memory device. It calls
*   the same _cbPaint Routine which is also used when drawing directly;
*   the only add. work done is adjustment of the invalid rectangle.
*   This way the invalid rectangle visible by the window callback function
*   is limited to the current band, allowing the callback to optimize
*   better.
*/
#if GUI_SUPPORT_MEMDEV
static void _cbPaintMemDev(void *p) {
	RECT Rect;
	auto pWin = WObj::pWinActive;
	Rect = pWin->InvalidRect;
	pWin->InvalidRect = GUI.ClipRect;
	WM__PaintWinAndOverlays((WObj *)p);
	pWin->InvalidRect = Rect;
}
#endif
/*********************************************************************
*
*       _Paint
  Returns:
	1: a window has been redrawn
	0: No window has been drawn
*/
static int _Paint(WObj *pWin) {
	if (!(pWin->Status & WC_ACTIVATE))
		return 0;
	int Ret = 0;
	if (pWin->cb) {
		if (pWin->_ClipAtParentBorders(pWin->InvalidRect)) {
			WM_SelectWindow(pWin);
#if GUI_SUPPORT_MEMDEV
			if (pWin->Status & WC_MEMDEV) {
				int Flags;
				RECT r = pWin->InvalidRect;
				Flags = GUI_MEMDEV_NOTRANS;
				/*
					* Currently we treat a desktop window as transparent, because per default it does not repaint itself.
					*/
				if (pWin->pParent == 0) {
					Flags = GUI_MEMDEV_HASTRANS;
				}
				GUI_MEMDEV_Draw(&r, _cbPaintMemDev, pWin, 0, Flags);
			}
			else
#endif
			WM__PaintWinAndOverlays(pWin);
			Ret = 1;    /* Something has been done */
		}
	}
	/* We purposly clear the invalid flag after painting so we can still query the invalid rectangle while painting */
	pWin->Status &= ~WC_ACTIVATE; /* Clear invalid flag */
	if (pWin->Status & WC_MEMDEV_ON_REDRAW)
		pWin->Status |= WC_MEMDEV;
	WObj::NumInvalidWindows--;
	return Ret;      /* Nothing done */
}
static void _DrawNext(void) {
	int UpdateRem = 1;
	auto iWin = pWinNextDraw ? pWinNextDraw : WObj::pWinFirst;
	GUI_CONTEXT ContextOld;
	GUI_SaveContext(&ContextOld);
	/* Make sure the next window to redraw is valid */
	for (; iWin && UpdateRem; iWin = iWin->pNextLin)
		if (_Paint(iWin))
			UpdateRem--;  /* Only the given number of windows at a time ... */
	pWinNextDraw = iWin;   /* Remember the window */
	GUI_RestoreContext(&ContextOld);
}
int WM_Exec1(void) {
	/* Poll PID if necessary */
	if (WM_HandlePID())
		return 1; /* We have done something ... */
	if (GUI_PollKeyMsg())
		return 1; /* We have done something ... */
	if (IsActive && WObj::NumInvalidWindows) {
		_DrawNext();
		return 1; /* We have done something ... */
	}
	return 0; /* There was nothing to do ... */
}
int WM_Exec(void) {
	int r = 0;
	while (WM_Exec1())
		r = 1; /* We have done something */
	return r;
}

void WM_Activate(void) {
	IsActive = true;       /* Running */
}
void WM_Deactivate(void) {
	IsActive = false;       /* No clipping performed by WM */
	LCD_SetClipRectMax();
}

WM_PARAM WM__SendMessage(WObj * pWin, int MsgId, WM_PARAM Data) {
	if (pWin->cb)
		return pWin->cb(pWin, MsgId, Data);
	return WM_DefaultProc(pWin, MsgId, Data);
}

/*********************************************************************
*
*       WM_DefaultProc
*
* Purpose
*   Default callback for windows
*   Any window should call this routine in the "default" part of the
*   its callback function for messages it does not handle itself.
*
*/
WM_PARAM WM_DefaultProc(WObj * pWin, int MsgId, WM_PARAM Data) {
	/* Exec message */
	switch (MsgId) {
		case WM_GET_INSIDE_RECT: /* return client window in absolute (screen) coordinates */
			*(RECT *)Data = pWin->GetClientRect();
			return 0;
		case WM_GET_CLIENT_WINDOW: /* return handle to client window. For most windows, there is no seperate client window, so it is the same handle */
			return (WM_PARAM)pWin;
		case WM_KEY: 
			pWin->Parent()->Require(WM_KEY, Data);
			return 0;	
		case WM_GET_BKCOLOR:
			return RGB_INVALID_COLOR;
		case WM_NOTIFY_ENABLE:
			pWin->Invalidate();
			return 0;
	}
	/* Message not handled. If it queries something, we return 0 to be on the safe side. */
	return 0;
}
void WM__ForEachDesc(WObj * pWin, WM_tfForEach *pcb, void *pData) {
	WObj *pChild;
	for (pChild = pWin->pFirstChild; pChild; pChild = pChild->pNext) {
		pcb(pChild, pData);
		WM_ForEachDesc(pChild, pcb, pData);
	}
}

WObj * WM__GetFocussedChild(WObj * pWin) {
	WObj * r = 0;
	if (WM__IsChild(WObj::pWinFocus, pWin)) {
		r = WObj::pWinFocus;
	}
	return r;
}

/*********************************************************************
*
*       WM__IsAncestor
*
* Return value:
*   if hChild is indeed a descendent (Child or child of child etc.) : true
*   Else: false
*/
bool WM__IsAncestor(WObj *pChild, WObj *pParent) {
	if (pChild && pParent) {
		while (pChild) {
			if (pChild->pParent == pParent)
				return true;
			pChild = pChild->pParent;
		}
	}
	return false;
}
/*********************************************************************
*
*       WM__IsAncestor
*
* Return value:
*   if hChild is indeed a descendent (Child or child of child etc.) : true
*   Else: false
*
*
*/
bool WM__IsAncestorOrSelf(WObj * pChild, WObj * pParent) {
	if (pChild == pParent)
		return true;
	return WM__IsAncestor(pChild, pParent);
}
bool WM__IsChild(WObj *pWin, WObj *pParent) {
	if (pWin) {
		if (pWin->pParent == pParent)
			return true;
	}
	return false;
}
void WM__Screen2Client(const WObj *pWin, RECT *pRect) {
	*pRect -= pWin->Rect.LeftTop();
}
#define WM_DEBUG_LEVEL 1

void WM_BringToBottom(WObj * pWin) {
	if (pWin) {
		if (auto pPrev = pWin->PrevSibling()) { /* If there is no previous one, there is nothing to do ! */
			auto pParent = pWin->Parent();
			/* unlink pWin */
			pPrev->pNext = pWin->pNext;
			/* Link from parent (making it the first child) */
			pWin->pNext = pParent->pFirstChild;
			pParent->pFirstChild = pWin;
			/* Send message in order to make sure top window will be drawn */
			WObj::InvalidateArea(pWin->Rect);
		}
	}
}
static void _cbInvalidateOne(WObj * pWin, void *p) {
	GUI_USE_PARA(p);
	pWin->Invalidate();
}
static void _InvalidateWindowAndDescs(WObj * pWin) {
	pWin->Invalidate();
	WM_ForEachDesc(pWin, _cbInvalidateOne, 0);
}

void WM_BringToTop(WObj * pWin) {
	if (pWin) {
		auto pNext = pWin->pNext;
		/* Is window alread on top ? If so, we are done. (Not required, just an optimization) */
		if (pNext == 0) {
			return;
		}
		/* For non-top windows, it is good enough if the next one is a stay-on-top-window (Not required, just an optimization) */
		if ((pWin->Status & WC_STAYONTOP) == 0) {
			if (pNext->Status & WC_STAYONTOP) {
				return;
			}
		}
		auto pParent = pWin->pParent;
		pWin->_RemoveWindowFromList();
		pWin->_InsertWindowIntoList(pParent);
		_InvalidateWindowAndDescs(pWin);
	}
}

/*********************************************************************
*
*       _ShowInvalid
*
* Function:
*   Debug code: shows invalid areas
*/
static void _ShowInvalid(WObj * pWin) {
	auto Context = GUI;
	auto rClient = pWin->InvalidRect - pWin->Rect.LeftTop();
	WM_SelectWindow(pWin);
	GUI.SetColor(RGB_GREEN);
	GUI.SetBkColor(RGB_GREEN);
	GUI_FillRect(rClient);
	GUI = Context;
}
void WM_ForEachDesc(WObj * pWin, WM_tfForEach *pcb, void *pData) {
	WM__ForEachDesc(pWin, pcb, pData);
}

RECT WM_GetClientRect() {
	return WObj::pWinActive->GetClientRect();
}
RECT WM_GetInsideRect() {
	return WObj::pWinActive->GetInsideRect();
}

static char _WindowSiblingsOverlapRect(WObj * iWin, RECT *pRect) {
	WObj *pWin;
	for (; iWin; iWin = pWin->pNext) {
		int Status = (pWin = iWin)->Status;
		/* Check if this window affects us at all */
		if (Status & WC_VISIBLE) {
			/* Check if this window affects us at all */
			if (pWin->Rect <= *pRect) {
				return 1;
			}
		}
	}
	return 0;
}
static int _HasOverlap(WObj *pWin, RECT *pRect) {
	WObj *pParent;
	/* Step 1:
	 Check if there are any visible children. If this is so, then the
	 window has an overlap.
	 */
	 /* Check all children */
	if (_WindowSiblingsOverlapRect(pWin->pFirstChild, pRect)) {
		return 1;
	}
	/* STEP 2:
		 Find out the max. height (r.y1) if we are at the left border.
		 Since we are using the same height for all IVRs at the same y0,
		 we do this only for the leftmost one.
	*/
	/* Iterate over all windows which are above */
	/* Check all siblings above (Iterate over Parents and top siblings (hNext) */
	for (pParent = pWin->pParent; pParent; pParent = pParent->pParent) {
		if (_WindowSiblingsOverlapRect(pParent->pNext, pRect)) {
			return 1;
		}
	}
	return 0;
}

void WM_Paint(WObj * pWin) {
	GUI_CONTEXT Context;
	WM_ASSERT_NOT_IN_PAINT();
	if (pWin) {
		GUI_SaveContext(&Context);
		WM_SelectWindow(pWin);
		pWin->Invalidate();  /* Important ... Window procedure is informed about invalid rect and may optimize */
		/* Paint the window and its overlaying transparent windows */
		WM__PaintWinAndOverlays(pWin);
		pWin->Validate();
		GUI_RestoreContext(&Context);
	}
}
PID_STATE WM_PID__GetPrevState() {
	return WM_PID__StateLast;
}

bool WM__IsInWindow(WObj *pWin, int x, int y) {
	if ((pWin->Status & WC_VISIBLE)
		&& (x >= pWin->Rect.x0)
		&& (x <= pWin->Rect.x1)
		&& (y >= pWin->Rect.y0)
		&& (y <= pWin->Rect.y1)) {
		return true;
	}
	return false;
}
void WM_SetAnchor(WObj * pWin, uint16_t AnchorFlags) {
	if (pWin) {
		uint16_t Mask;
		Mask = (WC_ANCHOR_LEFT | WC_ANCHOR_RIGHT | WC_ANCHOR_TOP | WC_ANCHOR_BOTTOM);
		AnchorFlags &= Mask;
		pWin->Status &= ~(Mask);
		pWin->Status |= AnchorFlags;
	}
}
WM_CALLBACK *WM_SetCallback(WObj * pWin, WM_CALLBACK *cb) {
	WM_CALLBACK *r = nullptr;
	if (pWin) {
		r = pWin->cb;
		pWin->cb = cb;
		pWin->Invalidate();
	}
	return r;
}

void WObj::ShowWindow() {
	if (!(Status & WC_VISIBLE)) {
		Status |= WC_VISIBLE;
		WM_InvalidateDescs(this);
	}
}


const RECT *WM_SetUserClipRect(const RECT *pRect) {
	auto pRectReturn = GUI.WM__pUserClipRect;
	GUI.WM__pUserClipRect = pRect;
	/* Activate it ... */
	WM__ActivateClipRect();
	return pRectReturn;
}

void WM_InvalidateDescs(WObj * pWin) {
	if (pWin) {
		pWin->Invalidate();    /* Invalidate window itself */
		for (auto pChild = pWin->FirstChild(); pChild;) {
			auto pNextChild = pChild->pNext;
			WM_InvalidateDescs(pChild);
			pChild = pNextChild;
		}
	}
}

void WM_SetStayOnTop(WObj * pWin, int OnOff) {
	if (pWin) {
		uint16_t OldStatus;
		OldStatus = pWin->Status;
		if (OnOff) {
			if (!(pWin->Status & WC_STAYONTOP)) /* First check if this is necessary at all */
				pWin->Status |= WC_STAYONTOP;
		}
		else if (pWin->Status & WC_STAYONTOP) /* First check if this is necessary at all */
			pWin->Status &= ~WC_STAYONTOP;
		if (pWin->Status != OldStatus)
			pWin->Attach(pWin->Parent());
	}
}
int WM_GetStayOnTop(WObj * pWin) {
	int Result = 0;
	if (pWin) {
		if (pWin->Status & WC_STAYONTOP)
			Result = 1;
	}
	return Result;
}
/*********************************************************************
*
*       WM__SubRect
  The result is the smallest rectangle which includes the entire
  remaining area.
  *pDest = *pr0- *pr1;
*/
static void _SubRect(RECT *pDest, const RECT *pr0, const RECT *pr1) {
	if ((pDest == nullptr) || (pr0 == nullptr))
		return;
	*pDest = *pr0;
	if (pr1 == nullptr)
		return;
	/* Check left/right sides */
	if ((pr1->y0 <= pr0->y0)
		&& (pr1->y1 >= pr0->y1)) {
		pDest->x0 = Max(pr0->x0, pr1->x1);
		pDest->x1 = Min(pr0->x1, pr1->x0);
	}
	/* Check top/bottom sides */
	if ((pr1->x0 <= pr0->x0)
		&& (pr1->x1 >= pr0->x1)) {
		pDest->y0 = Max(pr0->y0, pr1->y1);
		pDest->y1 = Min(pr0->y1, pr1->y0);
	}
}
int WM_OnKey(int Key, int Pressed) {
	if (WObj::pWinFocus) {
		WM_KEY_INFO Info;
		Info.Key = Key;
		Info.PressedCnt = Pressed;
		WM__SendMessage(WObj::pWinFocus, WM_KEY, (WM_PARAM)&Info);
		return 1;
	}
	return 0;
}

#pragma region Mouse/Touch
static CriticalHandle WM__CHWinModal, WM__CHWinLast;
static bool _IsInModalArea(WObj * pWin) {
	return (!WM__CHWinModal.pWin ||
		WM__IsAncestor(pWin, (WObj *)WM__CHWinModal.pWin) ||
		WM__CHWinModal.pWin == pWin) ? true : false;
}

static void _SendMessageIfEnabled(WObj * pWin, int MsgId, WM_PARAM Data) {
	if (pWin->IsEnabled())
		WM__SendMessage(pWin, MsgId, Data);
}

static void _SendTouchMessage(WObj * pWin, int MsgId, PID_STATE *pState) {
	if (pState) {
		pState->x -= pWin->Rect.x0;
		pState->y -= pWin->Rect.y0;
	}
	_SendMessageIfEnabled(pWin, MsgId, (WM_PARAM)pState);
	/* Send notification to all ancestors.
	   We need to check if the window which has received the last message still exists,
	   since it may have deleted itself and its parent as result of the message.
	*/
	while (WObj::IsWindow(pWin)) {
		pWin = pWin->Parent();
		if (pWin)
			_SendMessageIfEnabled(pWin, WM_TOUCH_CHILD, (WM_PARAM)pState); /* Send message to the ancestors */
	}
}

/*********************************************************************
*
*       WM_HandlePID
*
* Polls the touch screen. If something has changed,
* sends a message to the concerned window.
*
* Return value:
*   0 if nothing has been done
*   1 if touch message has been sent
*/
bool WM_HandlePID(void) {
	bool r = false;
	CriticalHandle CHWin;
	CHWin.Add();
	auto StateNew = GUI_PID_GetState();
	if ((WM_PID__StateLast.x != StateNew.x) || (WM_PID__StateLast.y != StateNew.y) || (WM_PID__StateLast.Pressed != StateNew.Pressed)) {
#if GUI_SUPPORT_CURSOR
		GUI_CURSOR_SetPosition(StateNew.x, StateNew.y);
#endif
		CHWin.pWin = WObj::pWinCapture ? WObj::pWinCapture : WM_Screen2Win(StateNew);
		if (_IsInModalArea((WObj *)CHWin.pWin)) {
			/*
			 * Send WM_PID_STATE_CHANGED message if state has changed (just pressed or just released)
			 */
			if ((WM_PID__StateLast.Pressed != StateNew.Pressed) && CHWin.pWin) {
				PID_CHANGED_INFO Info;
				auto pWin = CHWin.pWin;
				Info.State = StateNew.Pressed;
				Info.StatePrev = WM_PID__StateLast.Pressed;
				Info.x = StateNew.x - pWin->Rect.x0;
				Info.y = StateNew.y - pWin->Rect.y0;
				_SendMessageIfEnabled((WObj *)CHWin.pWin, WM_PID_STATE_CHANGED, (WM_PARAM)&Info);
			}
			/*
			 * Send WM_TOUCH message(s)
			 * Note that we may have to send 2 touch messages.
			 */
			if (WM_PID__StateLast.Pressed | StateNew.Pressed) { /* Only if pressed or just released */
				r = 1;
				/*
				 * Tell window if it is no longer pressed
				 * This happens for 2 possible reasons:
				 * a) PID is released
				 * b) PID is moved out
				 */
				if (WM__CHWinLast.pWin != CHWin.pWin) {
					if (WM__CHWinLast.pWin) {
						GUI_DEBUG_LOG("\nSending WM_Touch to LastWindow %d (out of area)", WM__CHWinLast.pWin);
						PID_STATE *pState = StateNew.Pressed ? nullptr : &WM_PID__StateLast;
						_SendTouchMessage((WObj *)WM__CHWinLast.pWin, WM_TOUCH, pState);
						WM__CHWinLast.pWin = nullptr;
					}
				}
				/* Sending WM_Touch to current window */
				if (CHWin.pWin) {
					/* Remember window */
					if (StateNew.Pressed)
						WM__CHWinLast.pWin = CHWin.pWin;
					else {
						/* Handle automatic capture release */
						if (WObj::WM__CaptureReleaseAuto)
							WObj::ReleaseCapture();
						WM__CHWinLast.pWin = nullptr;
					}
					_SendTouchMessage((WObj *)CHWin.pWin, WM_TOUCH, &StateNew);
				}
			}
#if GUI_SUPPORT_MOUSE
			/* Send WM_MOUSEOVER Message */
			else if (CHWin.pWin)
				/* Do not send messages to disabled windows */
				if (CHWin.pWin->IsEnabled())
					_SendTouchMessage((WObj *)CHWin.pWin, WM_MOUSEOVER, &StateNew);
#endif
		}
		/* Store the new state */
		WM_PID__StateLast = GUI_PID_GetState();
	}
	CHWin.Remove();
	return r;
}
#pragma endregion

void WM_Init(void) {
	static bool _IsInited = false;
	if (_IsInited)
		return;
	pWinNextDraw = WObj::pWinFirst = nullptr;
	GUI.WM__pUserClipRect = nullptr;
	/* Make sure we have at least one window. This greatly simplifies the
		drawing routines as they do not have to check if the window is valid.
	*/
	WObj::pWinDesktop = new WObj({ 0, 0, GUI_XMAX, GUI_YMAX }, WC_VISIBLE, WObj::cbBackWin);
	WObj::pWinDesktop->Invalidate(); /* Required because a desktop window has no parent. */
	/* Register the critical handles ... Note: This could be moved into the module setting the Window handle */
	WM__CHWinModal.Add();
	WM__CHWinLast.Add();
	WM_SelectWindow(WObj::pWinDesktop);
	WM_Activate();
	_IsInited = true;
}
