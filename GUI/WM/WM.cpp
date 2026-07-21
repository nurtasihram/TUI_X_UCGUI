#include "WM_Intern.h"
#include "GUI_Protected.h"
#include "SCROLLBAR.h"
#include "GUIDebug.h"

#define ASSIGN_IF_LESS(v0,v1) if (v1<v0) v0=v1

static uint16_t NumWindows, NumInvalidWindows;

static WM_Obj *pWinFirst;
static WM_CRITICAL_HANDLE *pFirstCriticalHandle;

static WM_Obj *pWinDesktop;
static RGBC BkColorDesktop;

static WM_Obj *pWinActive = nullptr;

static bool IsActive;
uint16_t WM__CreateFlags;
static WM_Obj *pWinFocus;

uint8_t WM__PaintCallbackCnt;      /* Public for assertions only */
static WM_Obj *pWinNextDraw;

static WM_Obj *pWinCapture;
bool WM__CaptureReleaseAuto;

GUI_PID_STATE          WM_PID__StateLast;

#if WM_SUPPORT_TRANSPARENCY
int                    WM__TransWindowCnt;
WM_Obj *WM__hATransWindow;
#endif

/*********************************************************************
*
*       _CheckCriticalHandles
*
* Purpose:
*   Checks the critical handles and resets the matching one
*/
static void _CheckCriticalHandles(WM_Obj * pWin) {
	WM_CRITICAL_HANDLE *pCH;
	for (pCH = pFirstCriticalHandle; pCH; pCH = pCH->pNext)
		if (pCH->pWin == pWin)
			pCH->pWin = 0;
}
/*********************************************************************
*
*       _Invalidate1Abs
*
*  Invalidate given window, using absolute coordinates
*/
static void _Invalidate1Abs(WM_Obj * pWin, const GUI_RECT *pRect) {
	GUI_RECT r;
	int Status;
	Status = pWin->Status;
	if (!(Status & WM_SF_ISVIS))
		return; /* Window is not visible... we are done */
	if ((Status & (WM_SF_HASTRANS | WM_SF_CONST_OUTLINE)) == WM_SF_HASTRANS)
		return; /* Window is transparent; transparency may change... we are done, since background will be invalidated */
	if (!*pRect)
		return; /* Nothing to do ... */
	/* Calc affected area */
	r = *pRect & pWin->Rect;
	if (r) {
#if WM_SUPPORT_NOTIFY_VIS_CHANGED
		WM__SendMsgNoData(pWin, WM_NOTIFY_VIS_CHANGED);             /* Notify window that visibility may have changed */
#endif
		if (pWin->Status & WM_SF_INVALID)
			pWin->InvalidRect |= r;
		else {
			pWin->InvalidRect = r;
			pWin->Status |= WM_SF_INVALID;
			NumInvalidWindows++;
		}
	}
}
/*********************************************************************
*
*       ResetNextDrawWin
  When drawing, we have to start at the bottom window !
*/
static void ResetNextDrawWin(void) {
	pWinNextDraw = nullptr;
}
/*********************************************************************
*
*       _GethDrawWin
*
* Return Window being drawn.
* Normally same as pAWin, except if overlaying transparent window is drawn
*
*/
static WM_Obj * _GethDrawWin(void) {
#if WM_SUPPORT_TRANSPARENCY
	if (WM__hATransWindow) 
		return WM__hATransWindow;
#endif
	return pWinActive;
}
static void _SetClipRectUserIntersect(const GUI_RECT *prSrc) {
	if (GUI_Context.WM__pUserClipRect == nullptr) {
		LCD_SetClipRectEx(prSrc);
	}
	else {
		GUI_RECT r = *GUI_Context.WM__pUserClipRect;
		WM__Client2Screen(_GethDrawWin(), &r);     /* Convert User ClipRect into screen coordinates */
		/* Set intersection as clip rect */
		r &= *prSrc;
		LCD_SetClipRectEx(&r);
	}
}
/*********************************************************************
*
*       _ClipAtParentBorders
*
* Function:
*   Iterates over the window itself and all its ancestors.
*   Intersects all rectangles to
*   find out which part is actually visible.
*   Reduces the rectangle to the visible area.
*   This routines takes into account both the rectangles of the
*   ancestors as well as the WM_SF_ISVIS flag.
*
* Parameters
*   pWin    Obvious
*   pRect   Pointer to the rectangle to be clipped. May not be nullptr.
*           The parameter is IN/OUT.
*           Note that the rectangle is clipped only if the return
*           value indicates a valid rectangle remains.
*
* Return value:
*   1: Something is or may be visible.
*   0: Nothing is visible (outside of ancestors, no desktop, hidden)
*/
static bool _ClipAtParentBorders(GUI_RECT &r, WM_Obj * pWin) {
	/* Iterate up the window hierarchy.
	   If the window is invisible, we are done.
	   Clip at parent boarders.
	   We are done with iterating if pWin has no parent.
	*/
	for (;; pWin = pWin->pParent) {
		if (!(pWin->Status & WM_SF_ISVIS))
			return false; /* Invisible */
		r &= pWin->Rect;  /* And clip on borders */
		if (!pWin->pParent)
			break; /* pWin is now the top level window which has no parent */
		/* Go one level up (parent)*/
	} /* Only way out is in the loop. Required for efficiency, no bug, even though some compilers may complain. */
	/* Now check if the top level window is a desktop window. If it is not,
	  then the window is not visible.
	*/
	if (pWin != pWinDesktop)
		return false; /* No desktop - (unattached) - Nothing to draw */
	return true; /* Something may be visible */
}
/*********************************************************************
*
*       WM__InsertWindowIntoList
*
* Routine describtion
*   This routine inserts the window in the list of child windows for
*   a particular parent window.
*   The window is placed on top of all siblings with the same level.
*/
void WM__InsertWindowIntoList(WM_Obj * pWin, WM_Obj * pParent) {
	int OnTop;
	WM_Obj *pi;
	WM_Obj *pFirstChild;
	if (pParent) {
		pWin->pNext = 0;
		pWin->pParent = pParent;
		OnTop = pWin->Status & WM_CF_STAYONTOP;
		pFirstChild = pParent->pFirstChild;
		/* Put it at beginning of the list if there is no child */
		if (pFirstChild == nullptr) {   /* No child yet ... Makes things easy ! */
			pParent->pFirstChild = pWin;
			return;                         /* Early out ... We are done */
		}
		/* Put it at beginning of the list if first child is a TOP window and new one is not */
		pi = pFirstChild;
		if (!OnTop) {
			if (pi->Status & WM_SF_STAYONTOP) {
				pWin->pNext = pFirstChild;
				pParent->pFirstChild = pWin;
				return;                         /* Early out ... We are done */
			}
		}
		/* Put it at the end of the list or before the last non "STAY-ON-TOP" child */
		do {
			WM_Obj *pNext;
			if ((pNext = pi->pNext) == nullptr) {   /* End of sibling list ? */
				pi->pNext = pWin;             /* Then modify this last element to point to new one and we are done */
				break;
			}
			if (!OnTop) {
				if (pNext->Status & WM_SF_STAYONTOP) {
					pi->pNext = pWin;
					pWin->pNext = pNext;
					break;
				}
			}
			pi = pNext;
		} while (1);
#if WM_SUPPORT_NOTIFY_VIS_CHANGED
		WM__NotifyVisChanged(pWin, &pWin->Rect);
#endif
	}
}
void WM__RemoveWindowFromList(WM_Obj * pWin) {
	WM_Obj *pParent, *pi;
	WM_Obj *pChild;
	pParent = pWin->pParent;
	if (pParent) {
		pChild = pParent->pFirstChild;
		if (pChild == pWin) {
			pParent->pFirstChild = pChild->pNext;
		}
		else {
			while (pChild) {
				pi = pChild;
				if (pi->pNext == pWin) {
					pi->pNext = pWin->pNext;
					break;
				}
				pChild = pi->pNext;
			}
		}
	}
}
/*********************************************************************
*
*       WM__DetachWindow
*
* Detaches the given window. The window still exists, it keeps all
* children, but it is no longer visible since it is taken out of
* the tree of the desktop window.
*/
void WM__DetachWindow(WM_Obj * pWin) {
	if (pWin->pParent) {
		WM__RemoveWindowFromList(pWin);
		/* Clear area used by this window */
		WM_InvalidateArea(&pWin->Rect);
		pWin->pParent = 0;
	}
}
static void _DeleteAllChildren(WM_Obj * pFirstChild) {
	for (auto pChild = pFirstChild; pChild; ) {
		auto pNext = pChild->pNext;
		WM_DeleteWindow(pChild);
		pChild = pNext;
	}
}
void WM__Client2Screen(const WM_Obj *pWin, GUI_RECT *pRect) {
	*pRect += GUI_POINT{pWin->Rect.x0, pWin->Rect.y0};
}
bool WM_IsWindow(WM_Obj * pWin) {
	for (auto pCur = pWinFirst; pCur; pCur = pCur->pNextLin)
		if (pCur == pWin)
			return true;
	return false;
}
/*********************************************************************
*
*         WM__InvalidateAreaBelow
  Params: pRect  Rectangle in Absolute coordinates
*/
void WM__InvalidateAreaBelow(const GUI_RECT *pRect, WM_Obj * StopWin) {
	GUI_USE_PARA(StopWin);
	WM_InvalidateArea(pRect);      /* Can be optimized to spare windows above */
}
void WM__RemoveFromLinList(WM_Obj * pWin) {
	for (auto pCur = pWinFirst; pCur; ) {
		auto pNext = pCur->pNextLin;
		if (pNext == pWin) {
			pCur->pNextLin = pWin->pNextLin;
			break;
		}
		pCur = pNext;
	}
}
static void _AddToLinList(WM_Obj * pNew) {
	WM_Obj *pFirst;
	if (pWinFirst) {
		pFirst = pWinFirst;
		pNew->pNextLin = pFirst->pNextLin;
		pFirst->pNextLin = pNew;
	}
	else
		pWinFirst = pNew;
}
static void _Findy1(WM_Obj *pWin, GUI_RECT *pRect, GUI_RECT *pParentRect) {
	for (; pWin; pWin = pWin->pNext) {
		int Status = pWin->Status;
		/* Check if this window affects us at all */
		if (!(Status & WM_SF_ISVIS))
			continue;
		auto rWinClipped = pWin->Rect; /* Window rect, clipped to part inside of ancestors */
		if (pParentRect)
			rWinClipped &= *pParentRect;
		/* Check if this window affects us at all */
		if (!GUI_RectsIntersect(pRect, &rWinClipped))
			continue;
		if ((Status & WM_SF_HASTRANS) == 0) {
			if (pWin->Rect.y0 > pRect->y0) {
				ASSIGN_IF_LESS(pRect->y1, rWinClipped.y0 - 1);      /* Check upper border of window */
			}
			else {
				ASSIGN_IF_LESS(pRect->y1, rWinClipped.y1);        /* Check lower border of window */
			}
		}
		else /* Check all children */
			for (auto pChild = pWin->pFirstChild; pChild; pChild = pChild->pNext)
				_Findy1(pChild, pRect, &rWinClipped);
	}
}
static bool _Findx0(WM_Obj *pWin, GUI_RECT *pRect, GUI_RECT *pParentRect) {
	for (; pWin; pWin = pWin->pNext) {
		int Status = pWin->Status;
		if (!(Status & WM_SF_ISVIS))
			continue;
		/* If window is not visible, it can be safely ignored */
		auto rWinClipped = pWin->Rect;               /* Window rect, clipped to part inside of ancestors */
		if (pParentRect)
			rWinClipped &= *pParentRect;
		/* Check if this window affects us at all */
		if (!GUI_RectsIntersect(pRect, &rWinClipped))
			continue;
		if (!(Status & WM_SF_HASTRANS)) {
			pRect->x0 = rWinClipped.x1 + 1;
			return true;
		}
		/* Check all children */
		for (auto pChild = pWin->pFirstChild; pChild; pChild = pChild->pNext)
			if (_Findx0(pChild, pRect, &rWinClipped))
				return true;
	}
	return false;
}
static void _Findx1(WM_Obj *pWin, GUI_RECT *pRect, GUI_RECT *pParentRect) {
	for (; pWin; pWin = pWin->pNext) {
		int Status = pWin->Status;
		if (!(Status & WM_SF_ISVIS))
			continue;
		/* If window is not visible, it can be safely ignored */
		GUI_RECT rWinClipped = pWin->Rect; /* Window rect, clipped to part inside of ancestors */
		if (pParentRect)
			rWinClipped &= *pParentRect;
		/* Check if this window affects us at all */
		if (!GUI_RectsIntersect(pRect, &rWinClipped))
			continue;
		if (!(Status & WM_SF_HASTRANS))
			pRect->x1 = rWinClipped.x0 - 1;
		else /* Check all children */
			for (auto pChild = pWin->pFirstChild; pChild; pChild = pChild->pNext)
				_Findx1(pChild, pRect, &rWinClipped);
	}
}

/*********************************************************************
*
*       WM_InvalidateRect
*
*  Invalidate a section of the window. The optional rectangle
*  contains client coordinates, which are independent of the
*  position of the window on the logical desktop area.
*/
void WM_InvalidateRect(WM_Obj * pWin, const GUI_RECT *pRect) {
	GUI_RECT r;
	int Status;
	if (pWin) {
		Status = pWin->Status;
		if (Status & WM_SF_ISVIS) {
			r = pWin->Rect;
			if (pRect) {
				GUI_RECT rPara;
				rPara = *pRect;
				WM__Client2Screen(pWin, &rPara);
				r &= rPara;
			}
			if (_ClipAtParentBorders(r, pWin)) {      /* Optimization that saves invalidation if window area is not visible ... Not required */
				if ((Status & (WM_SF_HASTRANS | WM_SF_CONST_OUTLINE)) == WM_SF_HASTRANS)
					WM__InvalidateAreaBelow(&r, pWin);        /* Can be optimized to spare windows above */
				else
					_Invalidate1Abs(pWin, &r);
			}
		}
	}
}
/*********************************************************************
*
*        WM_Invalidate
*
  Invalidates an entire window.
*/
void WM_Invalidate(WM_Obj * pWin) {
	WM_InvalidateRect(pWin, nullptr);
}
/*********************************************************************
*
*        WM_InvalidateArea
  Invalidate a certain section of the display. One main reason for this is
  that the top window has been moved or destroyed.
  The coordinates given are absolute coordinates (desktop coordinates)
*/
void WM_InvalidateArea(const GUI_RECT *pRect) {
	/* Iterate over all windows */
	for (auto pWin = pWinFirst; pWin; pWin = pWin->pNextLin) {
		_Invalidate1Abs(pWin, pRect);
	}
}
WM_Obj * WM_CreateWindowAsChild(int x0, int y0, int width, int height
							   , WM_Obj * pParent, uint16_t Style, WM_CALLBACK *cb
							   , int NumExtraBytes) {
	WM_Obj *pWin;
	WM_ASSERT_NOT_IN_PAINT();
	Style |= WM__CreateFlags;
	/* Default parent is Desktop 0 */
	if (!pParent) {
		if (NumWindows)
			pParent = pWinDesktop;
	}
	if (pParent == WM_UNATTACHED)
		pParent = nullptr;
	if (pParent) {
		x0 += pParent->Rect.x0;
		y0 += pParent->Rect.y0;
		if (width == 0)
			width = pParent->Rect.x1 - pParent->Rect.x0 + 1;
		if (height == 0)
			height = pParent->Rect.y1 - pParent->Rect.y0 + 1;
	}
	if ((pWin = (WM_Obj *)GUI_ALLOC_AllocZero(NumExtraBytes + sizeof(WM_Obj))) == 0) {
	}
	else {
		NumWindows++;
		pWin->Rect.x0 = x0;
		pWin->Rect.y0 = y0;
		pWin->Rect.x1 = x0 + width - 1;
		pWin->Rect.y1 = y0 + height - 1;
		pWin->cb = cb;
		/* Copy the flags which can simply be accepted */
		pWin->Status |= (Style & (WM_CF_SHOW |
								  WM_SF_MEMDEV |
								  WM_CF_MEMDEV_ON_REDRAW |
								  WM_SF_STAYONTOP |
								  WM_SF_CONST_OUTLINE |
								  WM_SF_HASTRANS |
								  WM_CF_ANCHOR_RIGHT |
								  WM_CF_ANCHOR_BOTTOM |
								  WM_CF_ANCHOR_LEFT |
								  WM_CF_ANCHOR_TOP |
								  WM_CF_LATE_CLIP));
		/* Add to linked lists */
		_AddToLinList(pWin);
		WM__InsertWindowIntoList(pWin, pParent);
		/* Activate window if WM_CF_ACTIVATE is specified */
		if (Style & WM_CF_ACTIVATE)
			WM_SelectWindow(pWin);  /* This is not needed if callbacks are being used, but it does not cost a lot and makes life easier ... */
		/* Handle the Style flags, one at a time */
#if WM_SUPPORT_TRANSPARENCY
		if (Style & WM_SF_HASTRANS)
			WM__TransWindowCnt++; /* Increment counter for transparency windows */
#endif
		if (Style & WM_CF_BGND)
			WM_BringToBottom(pWin);
		if (Style & WM_CF_SHOW) {
			pWin->Status |= WM_SF_ISVIS;  /* Set Visibility flag */
			WM_Invalidate(pWin);    /* Mark content as invalid */
		}
		WM__SendMsgNoData(pWin, WM_CREATE);
	}
	return pWin;
}
WM_Obj * WM_CreateWindow(int x0, int y0, int width, int height, uint16_t Style, WM_CALLBACK *cb, int NumExtraBytes) {
	return WM_CreateWindowAsChild(x0, y0, width, height, 0 /* No parent */, Style, cb, NumExtraBytes);
}
void WM_DeleteWindow(WM_Obj * pWin) {
	if (!pWin)
		return;
	WM_ASSERT_NOT_IN_PAINT();
	if (WM_IsWindow(pWin)) {
		ResetNextDrawWin(); /* Make sure the window will no longer receive drawing messages */
		/* Make sure that focus is set to an existing window */
		if (pWinFocus == pWin)
			pWinFocus = nullptr;
		if (pWinCapture == pWin)
			pWinCapture = nullptr;
		/* check if critical handles are affected. If so, reset the window handle to 0 */
		_CheckCriticalHandles(pWin);
		/* Inform parent */
		WM_NotifyParent(pWin, WM_NOTIFICATION_CHILD_DELETED);
		/* Delete all children */
		_DeleteAllChildren(pWin->pFirstChild);
#if WM_SUPPORT_NOTIFY_VIS_CHANGED
		WM__SendMsgNoData(pWin, WM_NOTIFY_VIS_CHANGED);             /* Notify window that visibility may have changed */
#endif
		/* Send WM_DELETE message to window in order to inform window itself */
		WM__SendMsgNoData(pWin, WM_DELETE);     /* tell window about it */
		WM__DetachWindow(pWin);
		/* Remove window from window stack */
		WM__RemoveFromLinList(pWin);
		/* Handle transparency counter if necessary */
#if WM_SUPPORT_TRANSPARENCY
		if (pWin->Status & WM_SF_HASTRANS)
			WM__TransWindowCnt--;
#endif
		/* Make sure window is no longer counted as invalid */
		if (pWin->Status & WM_SF_INVALID)
			NumInvalidWindows--;
		/* Free window memory */
		NumWindows--;
		GUI_ALLOC_Free(pWin);
		/* Select a valid window */
		WM_SelectWindow(pWinFirst);
	}
}
/*********************************************************************
*
*       WM_SelectWindow
*
*  Sets the active Window. The active Window is the one that is used for all
*  drawing (and text) operations.
*/
WM_Obj * WM_SelectWindow(WM_Obj * pWin) {
	auto pWinPrev = pWinActive;
	WM_ASSERT_NOT_IN_PAINT();
	if (pWin == 0) {
		pWin = pWinFirst;
	}
	/* Select new window */
	pWinActive = pWin;
	LCD_SetClipRectMax();             /* Drawing operations will clip ... If WM is deactivated, allow all */
	GUI_Context.xOff = pWin->Rect.x0;
	GUI_Context.yOff = pWin->Rect.y0;
	return pWinPrev;
}
WM_Obj * WM_GetActiveWindow(void) {
	return pWinActive;
}

#pragma region IVR
struct {
	GUI_RECT ClientRect;
	GUI_RECT CurRect;
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
static int _FindNext_IVR(void) {
	WM_Obj *pParent;
	GUI_RECT r;
	WM_Obj *pAWin;
	r = _ClipContext.CurRect;  /* temps  so we do not have to work with pointers too much */
	/*
	   STEP 1:
		 Set the next position which could be part of the next IVR
		 This will be the first unhandle pixel in reading order, i.e. next one to the right
		 or next one down if we are at the right border.
	*/
	if (_ClipContext.Cnt == 0) {       /* First IVR starts in upper left */
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
		return 0;
	/* STEP 3:
		 Find out the max. height (r.y1) if we are at the left border.
		 Since we are using the same height for all IVRs at the same y0,
		 we do this only for the leftmost one.
	*/
	pAWin = pWinActive;
	if (r.x0 == _ClipContext.ClientRect.x0) {
		r.y1 = _ClipContext.ClientRect.y1;
		r.x1 = _ClipContext.ClientRect.x1;
		/* Iterate over all windows which are above */
		/* Check all siblings above (Iterate over Parents and top siblings (hNext) */
		for (pParent = pWinActive; pParent; pParent = pParent->pParent) {
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
	if (Status & WM_SF_DONT_CLIP_SIBLINGS)
		pParent = pAWin->pParent;
	else
#endif
		pParent = pWinActive;
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
	if (Status & WM_SF_DONT_CLIP_SIBLINGS)
		pParent = pAWin->pParent;
	else
#endif
		pParent = pWinActive;
		for (; pParent; pParent = pParent->pParent) {
			_Findx1(pParent->pNext, &r, nullptr);
		}
	/* Check all children */
	_Findx1(pAWin->pFirstChild, &r, nullptr);
	/* We are done. Return the rectangle we found in the _ClipContext. */
	if (_ClipContext.Cnt > 200)
		return 0;  /* error !!! This should not happen !*/
	_ClipContext.CurRect = r;
	return 1;  /* IVR is valid ! */
}
#else
static int _FindNext_IVR(void) {
	if (_ClipContext.Cnt == 0) {
		_ClipContext.CurRect = pWinActive->Rect;
		return 1;  /* IVR is valid ! */
	}
	return 0;  /* Nothing left to draw */
}
#endif
/*********************************************************************
*
*       WM_GetNextIVR
  Sets the next clipping rectangle. If a valid one has
  been found (and set), 1 is returned in order to indicate
  that the drawing operation needs to be executed.
  Returning 0 signals that we have iterated over all
  rectangles.
  Returns: 0 if no valid rectangle is found
		   1 if rectangle has been found
*/
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
	_CursorHidden = GUI_CURSOR__TempHide(&_ClipContext.CurRect);
#endif
	return true;
}
/*********************************************************************
*
*       WM__InitIVRSearch
  This routine is called from the clipping level
  (the WM_ITERATE_START macro) when starting an iteration over the
  visible rectangles.
  Return value:
	0 : There is no valid rectangle (nothing to do ...)
	1 : There is a valid rectangle
*/
bool WM__InitIVRSearch(const GUI_RECT *pMaxRect) {
	GUI_RECT r;
	WM_Obj *pAWin;
	/* If WM is not active -> nothing to do, leave cliprect alone */
	if (!IsActive) {
		WM__ActivateClipRect();
		return true;
	}
	/* If we entered multiple times, leave Cliprect alone */
	if (++_ClipContext.EntranceCnt > 1)
		return true;
	pAWin = pWinActive;
	_ClipContext.Cnt = -1;
	/* When using callback mechanism, it is legal to reduce drawing
	   area to the invalid area ! */
	if (WM__PaintCallbackCnt)
		r = pAWin->InvalidRect;
	else if (pAWin->Status & WM_SF_ISVIS) /* Not using callback mechanism, therefor allow entire rectangle */
		r = pAWin->Rect;
	else {
		--_ClipContext.EntranceCnt;
		return false;  /* window is not even visible ! */
	}
	/* If the drawing routine has specified a rectangle, use it to reduce the rectangle */
	if (pMaxRect)
		r &= *pMaxRect;
	/* If user has reduced the cliprect size, reduce the rectangle */
	if (GUI_Context.WM__pUserClipRect) {
		WM_Obj *pWin = pAWin;
		GUI_RECT rUser = *(GUI_Context.WM__pUserClipRect);
#if WM_SUPPORT_TRANSPARENCY
		if (WM__hATransWindow)
			pWin = WM__hATransWindow;
#endif
		WM__Client2Screen(pWin, &rUser);
		r &= rUser;
	}
	/* For transparent windows, we need to further reduce the rectangle */
#if WM_SUPPORT_TRANSPARENCY
	if (WM__hATransWindow)
		if (_ClipAtParentBorders(r, WM__hATransWindow) == 0) {
			--_ClipContext.EntranceCnt;
			return false;           /* Nothing to draw */
		}
#endif
	/* Iterate over all ancestors and clip at their borders. If there is no visible part, we are done */
	if (_ClipAtParentBorders(r, pWinActive) == 0) {
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
		GUI_RECT r;
		WM_Obj *pAWin;
		pAWin = pWinActive;
		r = pAWin->Rect;
#if WM_SUPPORT_TRANSPARENCY
		if (WM__hATransWindow)
			_ClipAtParentBorders(r, WM__hATransWindow);
#endif
		/* Take UserClipRect into account */
		_SetClipRectUserIntersect(&r);
	}
}
#pragma endregion

void WM_SetDefault(void) {
	GUI_SetDefault();
	GUI_Context.WM__pUserClipRect = nullptr;   /* No add. clipping */
}
static void _Paint1(WM_Obj *pWin) {
	int Status = pWin->Status;
	/* Send WM_PAINT if window is visible and a callback is defined */
	if ((pWin->cb != nullptr) && (Status & WM_SF_ISVIS)) {
		WM__PaintCallbackCnt++;
		if (Status & WM_SF_LATE_CLIP) {
			WM_SetDefault();
			WM_SendMessage(pWin, WM_PAINT, (WM_PARAM)&pWin->InvalidRect);
		}
		else {
			WM_ITERATE_START(&pWin->InvalidRect) {
				WM_SetDefault();
				WM_SendMessage(pWin, WM_PAINT, (WM_PARAM)&pWin->InvalidRect);
			} WM_ITERATE_END();
		}
		WM__PaintCallbackCnt--;
	}
}
/*********************************************************************
*
*       _Paint1Trans
*
* Purpose:
*   Draw a transparent window as part of an other one (the active window: pAWin).
*   This is required because transparent windows are drawn as part of their
*   non-transparent parents.
* Return value:
*   0 if nothing was drawn (no invalid rect)
*   1 if something was drawn (invalid rect exists)
* Add. info:
*   It is important to restore the modified settings, especially the invalid rectangle
*   of the window. The invalid rectangle needs to be set, as it is passed as add. info
*   to the callback on WM_PAINT.
*   On traditional transparent windows, the transparent window is never drawn on its own,
*   so there is no need to restore the invalid rectangle.
*   However, with WM_SF_CONST_OUTLINE, the window itself may need to be redrawn because it
*   can be invalid. Modifying the invalid rectangle would lead to not updating the window
*   in the worst case.
*/
#if WM_SUPPORT_TRANSPARENCY
static int _Paint1Trans(WM_Obj *pWin) {
	int xPrev, yPrev;
	auto pAWin = pWinActive;
	/* Check if we need to do any drawing */
	if (GUI_RectsIntersect(&pAWin->InvalidRect, &pWin->Rect)) {
		/* Save old values */
		xPrev = GUI_Context.xOff;
		yPrev = GUI_Context.yOff;
		/* Set values for the current (transparent) window, rather than the one below */
		pWin->InvalidRect = pWin->Rect & pAWin->InvalidRect;
		WM__hATransWindow = pWin;
		GUI_Context.xOff = pWin->Rect.x0;
		GUI_Context.yOff = pWin->Rect.y0;
		/* Do the actual drawing ... */
		_Paint1(pWin);
		/* Restore settings */
		WM__hATransWindow = 0;
		GUI_Context.xOff = xPrev;
		GUI_Context.yOff = yPrev;
		return 1; /* Some drawing took place */
	}
	return 0; /* No invalid area, so nothing was drawn */
}
#endif
/*********************************************************************
*
*       _PaintTransChildren
*
* Purpose:
*   Paint transparent children. This function is obviously required
*   only if there are transparent windows.
* Function:  Obvious
* Parameter: Obvious
* Returns:   ---
*/
#if WM_SUPPORT_TRANSPARENCY
static void _PaintTransChildren(WM_Obj *pWin) {
	WM_Obj *pChild;
	if (pWin->Status & WM_SF_ISVIS) {
		for (pChild = pWin->pFirstChild; pChild; pChild = pChild->pNext) {
			if ((pChild->Status & (WM_SF_HASTRANS | WM_SF_ISVIS))   /* Transparent & visible ? */
				== (WM_SF_HASTRANS | WM_SF_ISVIS)) {
				/* Set invalid area of the window to draw */
				if (GUI_RectsIntersect(&pChild->Rect, &pWin->InvalidRect)) {
					GUI_RECT InvalidRectPrev = pWin->InvalidRect;
					if (_Paint1Trans(pChild))
						_PaintTransChildren(pChild);
					pWin->InvalidRect = InvalidRectPrev;
				}
			}
		}
	}
}
#endif
/*********************************************************************
*
*       _PaintTransTopSiblings
*
* Purpose:
*   Paint transparent top siblings. This function is obviously required
*   only if there are transparent windows.
* Function:  Obvious
* Parameter: Obvious
* Returns:   ---
*/
#if WM_SUPPORT_TRANSPARENCY
static void _PaintTransTopSiblings(WM_Obj *pWin) {
	auto pParent = pWin->pParent;
	pWin = pWin->pNext;
	while (pParent) { /* Go hierarchy up to desktop window */
		for (; pWin; pWin = pWin->pNext) {
			/* paint window if it is transparent & visible */
			if ((pWin->Status & (WM_SF_HASTRANS | WM_SF_ISVIS)) == (WM_SF_HASTRANS | WM_SF_ISVIS))
				_Paint1Trans(pWin);
			/* paint transparent & visible children */
			_PaintTransChildren(pWin);
		}
		pWin = pParent->pNext;
		pParent = pParent->pParent;
	}
}
#endif
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
void WM__PaintWinAndOverlays(WM_Obj *pWin) {
#if WM_SUPPORT_TRANSPARENCY
	/* Transparent windows without const outline are drawn as part of the background and can be skipped. */
	if ((pWin->Status & (WM_SF_HASTRANS | WM_SF_CONST_OUTLINE)) != WM_SF_HASTRANS)
#endif
		_Paint1(pWin); /* Draw the window itself */
#if WM_SUPPORT_TRANSPARENCY
	if (WM__TransWindowCnt != 0) {
		_PaintTransChildren(pWin); /* Draw all transparent children */
		_PaintTransTopSiblings(pWin); /* Draw all transparent top level siblings */
	}
#endif
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
	GUI_RECT Rect;
	auto pWin = pWinActive;
	Rect = pWin->InvalidRect;
	pWin->InvalidRect = GUI_Context.ClipRect;
	WM__PaintWinAndOverlays((WM_Obj *)p);
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
static int _Paint(WM_Obj *pWin) {
	if (!(pWin->Status & WM_SF_INVALID))
		return 0;
	int Ret = 0;
	if (pWin->cb) {
		if (_ClipAtParentBorders(pWin->InvalidRect, pWin)) {
			WM_SelectWindow(pWin);
#if GUI_SUPPORT_MEMDEV
			if (pWin->Status & WM_SF_MEMDEV) {
				int Flags;
				GUI_RECT r = pWin->InvalidRect;
				Flags = (pWin->Status & WM_SF_HASTRANS) ? GUI_MEMDEV_HASTRANS : GUI_MEMDEV_NOTRANS;
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
	pWin->Status &= ~WM_SF_INVALID; /* Clear invalid flag */
	if (pWin->Status & WM_CF_MEMDEV_ON_REDRAW)
		pWin->Status |= WM_CF_MEMDEV;
	NumInvalidWindows--;
	return Ret;      /* Nothing done */
}
static void _DrawNext(void) {
	int UpdateRem = 1;
	WM_Obj *iWin = (pWinNextDraw == nullptr) ? pWinFirst : pWinNextDraw;
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
	if (IsActive && NumInvalidWindows) {
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
/*********************************************************************
*
*       cbBackWin
*
* Purpose
*   Callback for background window
*
*/
static WM_PARAM cbBackWin(WM_Obj * pWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_KEY: {
			const WM_KEY_INFO *pKeyInfo = (const WM_KEY_INFO *)Data;
			if (pKeyInfo->PressedCnt == 1)
				GUI_StoreKey(pKeyInfo->Key);
			return 0;
		}
		case WM_PAINT:
			if (BkColorDesktop != RGB_INVALID_COLOR) {
				GUI_SetBkColor(BkColorDesktop);
				GUI_Clear();
			}
			return 0;
		default:
			return WM_DefaultProc(pWin, MsgId, Data);
	}
	return 0;
}
void WM_Activate(void) {
	IsActive = true;       /* Running */
}
void WM_Deactivate(void) {
	IsActive = false;       /* No clipping performed by WM */
	LCD_SetClipRectMax();
}

void WM_NotifyParent(WM_Obj * pWin, int Notification) {
	auto pParent = WM_GetParent(pWin);
	if (pParent) {
		WM_NOTIFY_INFO NotifyInfo;
		NotifyInfo.Notification = Notification;
		NotifyInfo.pWinSrc = pWin;
		WM_SendMessage(pParent, WM_NOTIFY_PARENT, (WM_PARAM)&NotifyInfo);
	}
}
WM_PARAM WM_SendMessage(WM_Obj * pWin, int MsgId, WM_PARAM Data) {
	if (pWin) {
		if (pWin->cb)
			return pWin->cb(pWin, MsgId, Data);
	}
	return 0;
}
WM_PARAM WM__SendMessage(WM_Obj * pWin, int MsgId, WM_PARAM Data) {
	if (pWin->cb)
		return pWin->cb(pWin, MsgId, Data);
	return WM_DefaultProc(pWin, MsgId, Data);
}
void WM_SendMessageNoPara(WM_Obj * pWin, int MsgId) {
	if (pWin->cb)
		pWin->cb(pWin, MsgId, 0);
}
void WM__SendMsgNoData(WM_Obj * pWin, int MsgId) {
	WM_SendMessage(pWin, MsgId, 0);
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
WM_PARAM WM_DefaultProc(WM_Obj * pWin, int MsgId, WM_PARAM Data) {
	/* Exec message */
	switch (MsgId) {
		case WM_GET_INSIDE_RECT: /* return client window in absolute (screen) coordinates */
			*(GUI_RECT *)Data = WM_GetClientRect(pWin);
			return 0;
		case WM_GET_CLIENT_WINDOW: /* return handle to client window. For most windows, there is no seperate client window, so it is the same handle */
			return (WM_PARAM)pWin;
		case WM_KEY: 
			WM_SendMessage(WM_GetParent(pWin), WM_KEY, Data);
			return 0;	
		case WM_GET_BKCOLOR:
			return RGB_INVALID_COLOR;
		case WM_NOTIFY_ENABLE:
			WM_Invalidate(pWin);
			return 0;
	}
	/* Message not handled. If it queries something, we return 0 to be on the safe side. */
	return 0;
}
void WM__ForEachDesc(WM_Obj * pWin, WM_tfForEach *pcb, void *pData) {
	WM_Obj *pChild;
	for (pChild = pWin->pFirstChild; pChild; pChild = pChild->pNext) {
		pcb(pChild, pData);
		WM_ForEachDesc(pChild, pcb, pData);
	}
}
/*********************************************************************
*
*       WM__GetFirstSibling
  Return value: Handle of parent, 0 if none
*/
WM_Obj * WM__GetFirstSibling(WM_Obj * pWin) {
	pWin = WM_GetParent(pWin);
	if (pWin) {
		return pWin->pFirstChild;
	}
	return 0;
}
WM_Obj * WM__GetFocussedChild(WM_Obj * pWin) {
	WM_Obj * r = 0;
	if (WM__IsChild(pWinFocus, pWin)) {
		r = pWinFocus;
	}
	return r;
}
/*********************************************************************
*
*       WM__GetLastSibling
  Return value: Handle of last sibling
*/
WM_Obj * WM__GetLastSibling(WM_Obj * pWin) {
	for (; pWin; pWin = pWin->pNext) {
		if (pWin->pNext == 0)
			break;
	}
	return pWin;
}
/*********************************************************************
*
*       WM_GetPrevSibling
  Return value: Handle of previous sibling (if any), otherwise 0
*/
WM_Obj * WM_GetPrevSibling(WM_Obj * pWin) {
	WM_Obj * pIter;
	WM_Obj *pPrev;
	for (pIter = WM__GetFirstSibling(pWin); pIter; pIter = pPrev->pNext) {
		if (pIter == pWin) {
			pIter = 0; /* There is no previous sibling. Return 0 */
			break;
		}
		pPrev = pIter;
		if (pPrev->pNext == pWin)
			break; /* We found the previous one ! */
	}
	return pIter;
}
/*********************************************************************
*
*       WM__IsAncestor
*
* Return value:
*   if hChild is indeed a descendent (Child or child of child etc.) : 1
*   Else: 0
*
*
*/
bool WM__IsAncestor(WM_Obj * pChild, WM_Obj * pParent) {
	bool r = false;
	if (pChild && pParent) {
		while (pChild) {
			if (pChild->pParent == pParent) {
				r = true;
				break;
			}
			pChild = pChild->pParent;
		}
	}
	return r;
}
/*********************************************************************
*
*       WM__IsAncestor
*
* Return value:
*   if hChild is indeed a descendent (Child or child of child etc.) : 1
*   Else: 0
*
*
*/
bool WM__IsAncestorOrSelf(WM_Obj * pChild, WM_Obj * pParent) {
	if (pChild == pParent) {
		return true;
	}
	return WM__IsAncestor(pChild, pParent);
}
bool WM__IsChild(WM_Obj * pWin, WM_Obj * pParent) {
	bool r = false;
	if (pWin) {
		if (pWin->pParent == pParent)
			r = true;
	}
	return r;
}
bool WM_IsEnabled(WM_Obj * pWin) {
	bool r = true;
	if ((pWin)->Status & WM_SF_DISABLED)
		r = false;
	return r;
}
/*********************************************************************
*
*       _NotifyVisChanged
*
* Description
*   Notify all descendents
*/
static void _NotifyVisChanged(WM_Obj * pWin, GUI_RECT *pRect) {
	for (auto pChild = WM_GetFirstChild(pWin); pChild; pChild = pChild->pNext) {
		if (pChild->Status & WM_SF_ISVIS) {
			if (GUI_RectsIntersect(&pChild->Rect, pRect)) {
				WM__SendMsgNoData(pChild, WM_NOTIFY_VIS_CHANGED);             /* Notify window that visibility may have changed */
				_NotifyVisChanged(pChild, pRect);
			}
		}
	}
}
/*********************************************************************
*
*       WM__NotifyVisChanged
*
* Description
*   Notify all siblings & descendents
*/
void WM__NotifyVisChanged(WM_Obj * pWin, GUI_RECT *pRect) {
	auto pParent = pWin->pParent;
	if (pParent) {
		_NotifyVisChanged(pParent, pRect);
	}
}
void WM__Screen2Client(const WM_Obj *pWin, GUI_RECT *pRect) {
	*pRect += GUI_POINT{-pWin->Rect.x0, -pWin->Rect.y0};
}
#define WM_DEBUG_LEVEL 1
/*********************************************************************
*
*       WM__UpdateChildPositions
*
* Purpose:
*   Move and/or resize all children of a resized window.
*   What exactly happens to the window depends on how the edges are
*   anchored.
*/
void WM__UpdateChildPositions(WM_Obj *pObj, int dx0, int dy0, int dx1, int dy1) {
	WM_Obj *pChild;
	int dx, dy, dw, dh;
	for (pChild = pObj->pFirstChild; pChild; pChild = pChild->pNext) {
		int Status;
		GUI_RECT rOld, rNew;
		/* Compute size of new rectangle */
		rOld = pChild->Rect;
		rNew = rOld;
		Status = pChild->Status & (WM_SF_ANCHOR_RIGHT | WM_SF_ANCHOR_LEFT);
		switch (Status) {
			case WM_SF_ANCHOR_RIGHT:                      /* Right ANCHOR : Move window with right side */
				rNew.x0 += dx1;
				rNew.x1 += dx1;
				break;
			case WM_SF_ANCHOR_RIGHT | WM_SF_ANCHOR_LEFT:    /* Left & Right ANCHOR: Resize window */
				rNew.x0 += dx0;
				rNew.x1 += dx1;
				break;
			default:                                    /* Left ANCHOR: Move window with left side of parent */
				rNew.x0 += dx0;
				rNew.x1 += dx0;
		}
		Status = pChild->Status & (WM_SF_ANCHOR_TOP | WM_SF_ANCHOR_BOTTOM);
		switch (Status) {
			case WM_SF_ANCHOR_BOTTOM:                     /* Bottom ANCHOR */
				rNew.y0 += dy1;
				rNew.y1 += dy1;
				break;
			case WM_SF_ANCHOR_BOTTOM | WM_SF_ANCHOR_TOP:    /* resize window */
				rNew.y0 += dy0;
				rNew.y1 += dy1;
				break;
			default:                                    /* Top ANCHOR */
				rNew.y0 += dy0;
				rNew.y1 += dy0;
		}
		/* Set new window position using Move and Resize as required */
		dx = rNew.x0 - rOld.x0;
		dy = rNew.y0 - rOld.y0;
		if (dx || dy) {
			WM_MoveWindow(pChild, dx, dy);
		}
		dw = (rNew.x1 - rNew.x0) - (rOld.x1 - rOld.x0);
		dh = (rNew.y1 - rNew.y0) - (rOld.y1 - rOld.y0);
		if (dw || dh) {
			WM_ResizeWindow(pChild, dw, dh);
		}
	}
}
void WM_DetachWindow(WM_Obj * pWin) {
	if (pWin) {
		auto pParent = pWin->pParent;
		if (pParent) {
			WM__DetachWindow(pWin);
			WM_MoveWindow(pWin, -pParent->Rect.x0, -pParent->Rect.y0);   /* Convert screen coordinates -> parent coordinates */
			/* ToDo: Invalidate. If Parent window is located at (0,0). */
		}
	}
}
void WM_AttachWindow(WM_Obj * pWin, WM_Obj * pParent) {
	if (pParent && (pParent != pWin)) {
		if (pWin->pParent != pParent) {
			WM_DetachWindow(pWin);
			WM__InsertWindowIntoList(pWin, pParent);
			WM_MoveWindow(pWin, pParent->Rect.x0, pParent->Rect.y0);    /* Convert parent coordinates -> screen coordinates */
		}
	}
}
void WM_AttachWindowAt(WM_Obj * pWin, WM_Obj * pParent, int x, int y) {
	WM_DetachWindow(pWin);
	WM_MoveTo(pWin, x, y);
	WM_AttachWindow(pWin, pParent);
}
void WM_BringToBottom(WM_Obj * pWin) {
	if (pWin) {
		auto pPrev = WM_GetPrevSibling(pWin);
		if (pPrev) {                   /* If there is no previous one, there is nothing to do ! */
			auto pParent = WM_GetParent(pWin);
			/* unlink pWin */
			pPrev->pNext = pWin->pNext;
			/* Link from parent (making it the first child) */
			pWin->pNext = pParent->pFirstChild;
			pParent->pFirstChild = pWin;
			/* Send message in order to make sure top window will be drawn */
			WM_InvalidateArea(&pWin->Rect);
		}
	}
}
static void _cbInvalidateOne(WM_Obj * pWin, void *p) {
	GUI_USE_PARA(p);
	WM_Invalidate(pWin);
}
static void _InvalidateWindowAndDescs(WM_Obj * pWin) {
	WM_Invalidate(pWin);
	WM_ForEachDesc(pWin, _cbInvalidateOne, 0);
}
void WM_BringToTop(WM_Obj * pWin) {
	if (pWin) {
		auto pNext = pWin->pNext;
		/* Is window alread on top ? If so, we are done. (Not required, just an optimization) */
		if (pNext == 0) {
			return;
		}
		/* For non-top windows, it is good enough if the next one is a stay-on-top-window (Not required, just an optimization) */
		if ((pWin->Status & WM_SF_STAYONTOP) == 0) {
			if (pNext->Status & WM_SF_STAYONTOP) {
				return;
			}
		}
		auto pParent = pWin->pParent;
		WM__RemoveWindowFromList(pWin);
		WM__InsertWindowIntoList(pWin, pParent);
		_InvalidateWindowAndDescs(pWin);
	}
}
void WM_CheckScrollBounds(WM_SCROLL_STATE *pScrollState) {
	int Max = pScrollState->NumItems - pScrollState->PageSize;
	if (Max < 0)
		Max = 0;
	/* Make sure scroll pos is in bounds */
	if (pScrollState->v < 0)
		pScrollState->v = 0;
	if (pScrollState->v > Max)
		pScrollState->v = Max;
}
/*********************************************************************
*
*       WM_CheckScrollPos
*
* Return value: Difference between old an new Scroll value.
*               Therefor 0 if settings have not changed.
*/
int  WM_CheckScrollPos(WM_SCROLL_STATE *pScrollState, int Pos, int LowerDist, int UpperDist) {
	int vOld = pScrollState->v;
	/* Check upper limit */
	if (Pos > pScrollState->v + pScrollState->PageSize - 1)
		pScrollState->v = Pos - (pScrollState->PageSize - 1) + UpperDist;
	/* Check lower limit */
	if (Pos < pScrollState->v)
		pScrollState->v = Pos - LowerDist;
	WM_CheckScrollBounds(pScrollState);
	return pScrollState->v - vOld;
}
/*********************************************************************
*
*       WM_SetScrollValue
*
* Return value: Difference between old an new Scroll value.
*               Therefor 0 if settings have not changed.
*/
int  WM_SetScrollValue(WM_SCROLL_STATE *pScrollState, int v) {
	int vOld = pScrollState->v;
	pScrollState->v = v;
	WM_CheckScrollBounds(pScrollState);
	return pScrollState->v - vOld;
}
void WM__AddCriticalHandle(WM_CRITICAL_HANDLE *pCriticalHandle) {
	pCriticalHandle->pNext = pFirstCriticalHandle;
	pFirstCriticalHandle = pCriticalHandle;
}
void WM__RemoveCriticalHandle(WM_CRITICAL_HANDLE *pCriticalHandle) {
	if (pFirstCriticalHandle) {
		WM_CRITICAL_HANDLE *pCH, *pLast = 0;
		for (pCH = pFirstCriticalHandle; pCH; pCH = pCH->pNext) {
			if (pCH == pCriticalHandle) {
				if (pLast)
					pLast->pNext = pCH->pNext;
				else if (pCH->pNext)
					pFirstCriticalHandle = pCH->pNext;
				else
					pFirstCriticalHandle = 0;
				break;
			}
			pLast = pCH;
		}
	}
}
/*********************************************************************
*
*       _ShowInvalid
*
* Function:
*   Debug code: shows invalid areas
*/
static void _ShowInvalid(WM_Obj * pWin) {
	GUI_CONTEXT Context = GUI_Context;
	GUI_RECT rClient;
	rClient = pWin->InvalidRect + GUI_POINT{-pWin->Rect.x0, -pWin->Rect.y0};
	WM_SelectWindow(pWin);
	GUI_SetColor(RGB_GREEN);
	GUI_SetBkColor(RGB_GREEN);
	GUI_FillRect(rClient);
	GUI_Context = Context;
}
void WM_SetEnableState(WM_Obj * pWin, int State) {
	uint16_t Status;
	Status = pWin->Status;
	if (State)
		Status &= ~WM_SF_DISABLED;
	else 
		Status |= WM_SF_DISABLED;
	if (pWin->Status != Status) {
		pWin->Status = Status;
		WM_SendMessage(pWin, WM_NOTIFY_ENABLE, (WM_PARAM)State);
	}
}
void WM_EnableWindow(WM_Obj * pWin) {
	WM_SetEnableState(pWin, 1);
}
void WM_DisableWindow(WM_Obj * pWin) {
	WM_SetEnableState(pWin, 0);
}
void WM_ForEachDesc(WM_Obj * pWin, WM_tfForEach *pcb, void *pData) {
	WM__ForEachDesc(pWin, pcb, pData);
}
/*********************************************************************
*
*       WM_GetBkColor
*
  Purpose:
	Return the clients background color.
	If a window does not define a background color, the default
	procedure returns RGB_INVALID_COLOR
*/
RGBC WM_GetBkColor(WM_Obj * pObj) {
	if (pObj) 
		return (RGBC)WM_SendMessage(pObj, WM_GET_BKCOLOR, 0);
	return RGB_INVALID_COLOR;
}
GUI_RECT WM_GetClientRect() {
	WM_Obj * pWin;
#if WM_SUPPORT_TRANSPARENCY
	pWin = WM__hATransWindow ? WM__hATransWindow : pWinActive;
#else
	pWin = pWinActive;
#endif
	return WM_GetClientRect(pWin);
}
/*********************************************************************
*
*       WM_GetClientRect (overloaded with WM_Obj * parameter)
*
  Get client rectangle in windows coordinates. This means that the
  upper left corner is always at (0,0).
*/
GUI_RECT WM_GetClientRect(WM_Obj * pWin) {
	GUI_RECT Rect;
	Rect.x0 = Rect.y0 = 0;
	Rect.x1 = pWin->Rect.x1 - pWin->Rect.x0;
	Rect.y1 = pWin->Rect.y1 - pWin->Rect.y0;
	return Rect;
}
/*********************************************************************
*
*       WM_GetInsideRect (overloaded with WM_Obj * parameter)
*
  Purpose:
	Return the inside rectangle in client coordinates.
	The inside rectangle is the client rectangle minus the effect,
	which typically reduces the rectangle by 0 - 3 pixels on either side
	(2 for the standard 3D effect).
*/
GUI_RECT WM_GetInsideRect(WM_Obj * pWin) {
	GUI_RECT Rect;
	WM_SendMessage(pWin, WM_GET_INSIDE_RECT, (WM_PARAM)&Rect);
	return Rect;
}
GUI_RECT WM_GetInsideRect() {
	return WM_GetInsideRect(pWinActive);
}
WM_Obj * WM_GetClientWindow(WM_Obj * pObj) {
	return (WM_Obj *)WM_SendMessage(pObj, WM_GET_CLIENT_WINDOW, 0);
}
WM_Obj * WM_GetDesktopWindow(void) {
	return pWinDesktop;
}
int WM_GetNumWindows(void) {
	return NumWindows;
}
int WM_GetNumInvalidWindows(void) {
	return NumInvalidWindows;
}
/*******************************************************************
*
*       WM__GetDialogItem
  Recursively scan window and its child windows until dialog item is
  found or all children have been scanned.
*/
WM_Obj * WM_GetDialogItem(WM_Obj * pWin, int Id) {
	WM_Obj * pIter;
	WM_Obj * r = 0;
	pIter = pWin->pFirstChild;
	while (pIter) {
		/* This windows Id matching ? */
		if (WM_GetId(pIter) == Id)
			return pIter;
		/* Any child windows Id matching ? */
		if ((r = WM_GetDialogItem(pIter, Id)))
			break;
		pIter = pIter->pNext;
	}
	return r;
}
WM_Obj * WM_GetFirstChild(WM_Obj * pWin) {
	if (pWin) {
		pWin = pWin->pFirstChild;
	}
	return pWin;
}
uint16_t WM_GetFlags(WM_Obj * pWin) {
	uint16_t r = 0;
	if (pWin) {
		r = pWin->Status;
	}
	return r;
}
WM_Obj * WM_GetFocussedWindow(void) {
	return pWinFocus;
}
int WM_GetId(WM_Obj * pObj) {
	return (int)WM_SendMessage(pObj, WM_GET_ID, 0);
}
/*********************************************************************
*
*       WM_GetInsideRectExScrollbar
*
  Purpose:
	Return the inside rectangle in client coordinates.
	The inside rectangle is the client rectangle minus the effect,
	which typically reduces the rectangle by 0 - 3 pixels on either side
	(2 for the standard 3D effect).
*/
void WM_GetInsideRectExScrollbar(WM_Obj * pWin, GUI_RECT *pRect) {
	GUI_RECT rWin, rInside, rScrollbar;
	WM_Obj *pBarV;
	WM_Obj *pBarH;
	uint16_t WinFlags;
	if (pWin) {
		if (pRect) {
			pBarH = WM_GetDialogItem(pWin, GUI_ID_HSCROLL);
			pBarV = WM_GetDialogItem(pWin, GUI_ID_VSCROLL);
			rWin = WM_GetWindowRect(pWin);     /* The entire window in screen coordinates */
			rInside = WM_GetInsideRect(pWin);
			if (pBarV) {
				rScrollbar = WM_GetWindowRect(pBarV) + GUI_POINT{-rWin.x0, -rWin.y0};
				WinFlags = WM_GetFlags(pBarV);
				if ((WinFlags & WM_SF_ANCHOR_RIGHT) && (WinFlags & WM_SF_ISVIS)) {
					rInside.x1 = rScrollbar.x0 - 1;
				}
			}
			if (pBarH) {
				rScrollbar = WM_GetWindowRect(pBarH) + GUI_POINT{-rWin.x0, -rWin.y0};
				WinFlags = WM_GetFlags(pBarH);
				if ((WinFlags & WM_SF_ANCHOR_BOTTOM) && (WinFlags & WM_SF_ISVIS)) {
					rInside.y1 = rScrollbar.y0 - 1;
				}
			}
			*pRect = rInside;
		}
	}
}
/*********************************************************************
*
*       WM_GetInvalidRect
*
* Return value:
*   0 if window is valid (there is no invalid rectangle)
*   1 if there is an invalid rectangle
*
*/
int WM_GetInvalidRect(WM_Obj * pWin, GUI_RECT *pRect) {
	int IsInvalid = 0;
	if (pWin) {
		if (pWin->Status & WM_SF_INVALID) {
			IsInvalid = 1;
			*pRect = pWin->InvalidRect;
		}
	}
	return IsInvalid;
}
WM_Obj * WM_GetNextSibling(WM_Obj * pWin) {
	if (pWin) {
		pWin = pWin->pNext;
	}
	return pWin;
}
int WM_GetWindowOrgX(WM_Obj * pWin) {
	int r = 0;
	if (pWin) {
		r = pWin->Rect.x0;
	}
	return r;
}
int WM_GetWindowOrgY(WM_Obj * pWin) {
	int r = 0;
	if (pWin) {
		r = pWin->Rect.y0;
	}
	return r;
}
int WM_GetOrgX(void) {
	return WM_GetWindowOrgX(pWinActive);
}
int WM_GetOrgY(void) {
	return WM_GetWindowOrgY(pWinActive);
}
WM_Obj * WM_GetParent(WM_Obj * pWin) {
	if (pWin) {
		pWin = pWin->pParent;
	}
	return pWin;
}
WM_Obj * WM_GetScrollbarH(WM_Obj * pWin) {
	return WM_GetDialogItem(pWin, GUI_ID_HSCROLL);
}
WM_Obj * WM_GetScrollbarV(WM_Obj * pWin) {
	return WM_GetDialogItem(pWin, GUI_ID_VSCROLL);
}
WM_Obj * WM_GetScrollPartner(WM_Obj * pScroll) {
	int Id = WM_GetId(pScroll);
	if (Id == GUI_ID_HSCROLL) {
		Id = GUI_ID_VSCROLL;
	}
	else if (Id == GUI_ID_VSCROLL) {
		Id = GUI_ID_HSCROLL;
	}
	return WM_GetDialogItem(WM_GetParent(pScroll), Id);
}
void WM_GetScrollState(WM_Obj * pObj, WM_SCROLL_STATE *pScrollState) {
	WM_SendMessage(pObj, WM_GET_SCROLL_STATE, (WM_PARAM)pScrollState);
}
#define WM_DEBUG_LEVEL 1
/*********************************************************************
*
*       WM_GetWindowRect
*
* Returns the window rect in screen (desktop) coordinates.
*/
GUI_RECT WM_GetWindowRect() {
	WM_Obj * pWin;
#if WM_SUPPORT_TRANSPARENCY
	pWin = WM__hATransWindow ? WM__hATransWindow : pWinActive;
#else
	pWin = pWinActive;
#endif
	return pWin->Rect;
}
GUI_RECT WM_GetWindowRect(WM_Obj * pWin) {
	GUI_RECT Rect = {};
	if (pWin) {
		Rect = pWin->Rect;
	}
	return Rect;
}
/*********************************************************************
*
*       _GetDefaultWin
  When drawing, we have to start at the bottom window !
*/
static WM_Obj * _GetDefaultWin(WM_Obj * pWin) {
	if (!pWin)
		pWin = WM_GetActiveWindow();
	return pWin;
}
/*********************************************************************
*
*       WM__GetWindowSizeX
  Return width of window in pixels
*/
int WM__GetWindowSizeX(const WM_Obj *pWin) {
	return pWin->Rect.x1 - pWin->Rect.x0 + 1;
}
/*********************************************************************
*
*       WM__GetWindowSizeY
  Return height of window in pixels
*/
int WM__GetWindowSizeY(const WM_Obj *pWin) {
	return pWin->Rect.y1 - pWin->Rect.y0 + 1;
}
/*********************************************************************
*
*       WM_GetWindowSizeX
  Return width of window in pixels
*/
int WM_GetWindowSizeX(WM_Obj * pWin) {
	int r;
	pWin = _GetDefaultWin(pWin);
	r = WM__GetWindowSizeX(pWin);
	return r;
}
/*********************************************************************
*
*       WM_GetWindowSizeY
  Return height of window in pixels
*/
int WM_GetWindowSizeY(WM_Obj * pWin) {
	int r;
	pWin = _GetDefaultWin(pWin);
	r = WM__GetWindowSizeY(pWin);
	return r;
}
bool WM_HasCaptured(WM_Obj * pWin) {
	return (pWin == pWinCapture) ? true : false;
}
bool WM_HasFocus(WM_Obj * pWin) {
	return (pWin == pWinFocus) ? true : false;
}
#define WM_DEBUG_LEVEL 1
void WM_HideWindow(WM_Obj * pWin) {
	if (pWin) {
		/* First check if this is necessary at all */
		if (pWin->Status & WM_SF_ISVIS) {
			/* Clear Visibility flag */
			pWin->Status &= ~WM_SF_ISVIS;
			/* Mark content as invalid */
			WM__InvalidateAreaBelow(&pWin->Rect, pWin);
#if WM_SUPPORT_NOTIFY_VIS_CHANGED
			WM__SendMsgNoData(pWin, WM_NOTIFY_VIS_CHANGED);             /* Notify window that visibility may have changed */
#endif
		}
	}
}
static char _CompareRect(const GUI_RECT *pRect0, const GUI_RECT *pRect1) {
	if (pRect0->x0 != pRect1->x0) {
		return 1;                          /* Not equal */
	}
	if (pRect0->x1 != pRect1->x1) {
		return 1;                          /* Not equal */
	}
	if (pRect0->y0 != pRect1->y0) {
		return 1;                          /* Not equal */
	}
	if (pRect0->y1 != pRect1->y1) {
		return 1;                          /* Not equal */
	}
	return 0;                            /* Equal */
}
static char _WindowSiblingsOverlapRect(WM_Obj * iWin, GUI_RECT *pRect) {
	WM_Obj *pWin;
	for (; iWin; iWin = pWin->pNext) {
		int Status = (pWin = iWin)->Status;
		/* Check if this window affects us at all */
		if (Status & WM_SF_ISVIS) {
			/* Check if this window affects us at all */
			if (GUI_RectsIntersect(pRect, &pWin->Rect)) {
				return 1;
			}
		}
	}
	return 0;
}
static int _HasOverlap(WM_Obj *pWin, GUI_RECT *pRect) {
	WM_Obj *pParent;
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
bool WM_IsFocussable(WM_Obj * pWin) {
	if (pWin)
		return WM_SendMessage(pWin, WM_GET_ACCEPT_FOCUS, 0) ? true : false;
	return false;
}
bool WM_IsVisible(WM_Obj * pWin) {
	if (pWin) {
		if (pWin->Status & WM_SF_ISVIS) 
			return true;
	}
	return false;
}

/*********************************************************************
*
*       _MoveDescendents
*
* Description
*   Moves _MoveDescendents.
*   Since the parent has already been moved, there is no need to
*   take care of invalidation.
*
* Parameters
*   hWin    The first of all descendents to be moved (first child)
*/
static void _MoveDescendents(WM_Obj * pWin, int dx, int dy) {
	for (; pWin; pWin = pWin->pNext) {
		pWin->Rect += GUI_POINT{dx, dy};
		pWin->InvalidRect += GUI_POINT{dx, dy};
		_MoveDescendents(pWin->pFirstChild, dx, dy);  /* Children need to be moved along ...*/
		WM__SendMsgNoData(pWin, WM_MOVE);
	}
}
void WM_MoveWindow(WM_Obj * pWin, int dx, int dy) {
	GUI_RECT r;
	if (pWin) {
		r = pWin->Rect;
		pWin->Rect += GUI_POINT{dx, dy};
		pWin->InvalidRect += GUI_POINT{dx, dy};
		_MoveDescendents(pWin->pFirstChild, dx, dy);  /* Children need to be moved along ...*/
		/* Invalidate old and new area ... */
		if (pWin->Status & WM_SF_ISVIS) {
			WM_InvalidateArea(&pWin->Rect);     /* Invalidate new area */
			WM_InvalidateArea(&r);     /* Invalidate old area */
		}
		WM__SendMsgNoData(pWin, WM_MOVE);             /* Notify window it has been moved */
	}
}
void WM_MoveTo(WM_Obj * pWin, int x, int y) {
	if (pWin) {
		x -= pWin->Rect.x0;
		y -= pWin->Rect.y0;
		WM_MoveWindow(pWin, x, y);
	}
}
void WM_MoveChildTo(WM_Obj * pWin, int x, int y) {
	if (pWin) {
		auto pParent = WM_GetParent(pWin);
		if (pParent) {
			x -= pWin->Rect.x0 - pParent->Rect.x0;
			y -= pWin->Rect.y0 - pParent->Rect.y0;
			WM_MoveWindow(pWin, x, y);
		}
	}
}
void WM_Paint(WM_Obj * pWin) {
	GUI_CONTEXT Context;
	WM_ASSERT_NOT_IN_PAINT();
	if (pWin) {
		GUI_SaveContext(&Context);
		WM_SelectWindow(pWin);
		WM_SetDefault();
		WM_Invalidate(pWin);  /* Important ... Window procedure is informed about invalid rect and may optimize */
		/* Paint the window and its overlaying transparent windows */
		WM__PaintWinAndOverlays(pWin);
		WM_ValidateWindow(pWin);
		GUI_RestoreContext(&Context);
	}
}
void WM_PID__GetPrevState(GUI_PID_STATE *pPrevState) {
	*pPrevState = WM_PID__StateLast;
}
#define WM_DEBUG_LEVEL 1
void WM_ResizeWindow(WM_Obj * pWin, int dx, int dy) {
	GUI_RECT rOld, rNew, rMerge;
	if (((dx | dy) == 0) || (pWin == 0)) { /* Early out if there is nothing to do */
		return;
	}
	rOld = pWin->Rect;
	rNew = rOld;
	if (dx) {
		if ((pWin->Status & WM_SF_ANCHOR_RIGHT) && (!(pWin->Status & WM_SF_ANCHOR_LEFT))) {
			rNew.x0 -= dx;
		}
		else {
			rNew.x1 += dx;
		}
	}
	if (dy) {
		if ((pWin->Status & WM_SF_ANCHOR_BOTTOM) && (!(pWin->Status & WM_SF_ANCHOR_TOP))) {
			rNew.y0 -= dy;
		}
		else {
			rNew.y1 += dy;
		}
	}
	rMerge = rOld | rNew;
	pWin->Rect = rNew;
	WM_InvalidateArea(&rMerge);
	WM__UpdateChildPositions(pWin, rNew.x0 - rOld.x0, rNew.y0 - rOld.y0, rNew.x1 - rOld.x1, rNew.y1 - rOld.y1);
	pWin->InvalidRect &= pWin->Rect; /* Make sure invalid area is not bigger than window itself */
	WM__SendMsgNoData(pWin, WM_SIZE); /* Send size message to the window */
}
/*********************************************************************
*
*       _Screen2hWin
  This routine is recursive.
  It checks if the given coordinates are in the window or a decendant.
  Returns:
  0:   If coordinates are neither in the given window nor a decendent
  !=0  Handle of the topmost visible decendent in which the given
	   coordinate falls.
*/
static WM_Obj * _Screen2hWin(WM_Obj * pWin, WM_Obj * pStop, int x, int y) {
	WM_Obj * pChild;
	WM_Obj * pHit;
	/* First check if the  coordinates are in the given window. If not, return 0 */
	if (!WM__IsInWindow(pWin, x, y))
		return 0;
	/* If the coordinates are in a child, search deeper ... */
	for (pChild = pWin->pFirstChild; pChild && (pChild != pStop); ) {
		auto pNextChild = pChild->pNext;
		if ((pHit = _Screen2hWin(pChild, pStop, x, y)) != 0)
			pWin = pHit;        /* Found a window */
		pChild = pNextChild;
	}
	return pWin; /* No Child affected ... The parent is the right one */
}
WM_Obj * WM_Screen2hWin(int x, int y) {
	return _Screen2hWin(pWinFirst, 0, x, y);
}
WM_Obj * WM_Screen2hWinEx(WM_Obj * pStop, int x, int y) {
	return _Screen2hWin(pWinFirst, pStop, x, y);
}

bool WM__IsInWindow(WM_Obj *pWin, int x, int y) {
	if ((pWin->Status & WM_SF_ISVIS)
		&& (x >= pWin->Rect.x0)
		&& (x <= pWin->Rect.x1)
		&& (y >= pWin->Rect.y0)
		&& (y <= pWin->Rect.y1)) {
		return true;
	}
	return false;
}
void WM_SetAnchor(WM_Obj * pWin, uint16_t AnchorFlags) {
	if (pWin) {
		uint16_t Mask;
		Mask = (WM_SF_ANCHOR_LEFT | WM_SF_ANCHOR_RIGHT | WM_SF_ANCHOR_TOP | WM_SF_ANCHOR_BOTTOM);
		AnchorFlags &= Mask;
		pWin->Status &= ~(Mask);
		pWin->Status |= AnchorFlags;
	}
}
WM_CALLBACK *WM_SetCallback(WM_Obj * pWin, WM_CALLBACK *cb) {
	WM_CALLBACK *r = nullptr;
	if (pWin) {
		r = pWin->cb;
		pWin->cb = cb;
		WM_Invalidate(pWin);
	}
	return r;
}
void WM_ReleaseCapture(void) {
	if (pWinCapture) {
		WM_SendMessage(pWinCapture, WM_CAPTURE_RELEASED, 0);
		pWinCapture = nullptr;
	}
}
void WM_SetCapture(WM_Obj * pObj, int AutoRelease) {
	if (pWinCapture != pObj)
		WM_ReleaseCapture();
	pWinCapture = pObj;
	WM__CaptureReleaseAuto = AutoRelease;
}
static GUI_POINT WM__CapturePoint;
void WM_SetCaptureMove(WM_Obj * pWin, const GUI_PID_STATE *pState, int MinVisibility) {
	if (!WM_HasCaptured(pWin)) {
		WM_SetCapture(pWin, 1);        /* Set capture with auto release */
		WM__CapturePoint.x = pState->x;
		WM__CapturePoint.y = pState->y;
	}
	else {                         /* Moving ... let the window move ! */
		int dx, dy;
		dx = pState->x - WM__CapturePoint.x;
		dy = pState->y - WM__CapturePoint.y;
		/* make sure at least a part of the windows stays inside of its parent */
		if (MinVisibility == 0) {
			WM_MoveWindow(pWin, dx, dy);
		}
		else {
			GUI_RECT Rect, RectParent;
			/* make sure at least a part of the windows stays inside of its parent */
			Rect = WM_GetWindowRect(pWin) + GUI_POINT{dx, dy};
			RectParent = WM_GetWindowRect(WM_GetParent(pWin)) - MinVisibility;
			if (GUI_RectsIntersect(&Rect, &RectParent))
				WM_MoveWindow(pWin, dx, dy);
		}
	}
}
uint16_t WM_SetCreateFlags(uint16_t Flags) {
	uint16_t r = WM__CreateFlags;
	WM__CreateFlags = Flags;
	return r;
}
RGBC WM_SetDesktopColor(RGBC Color) {
	RGBC r;
	r = BkColorDesktop;
	BkColorDesktop = Color;
	WM_Invalidate(pWinDesktop);
	return r;
}
/*********************************************************************
*
*       WM_SetFocus
*
* Purpose:
*   Sets the focus to the specified child. It sends 2 messages:
*    WM_SET_FOCUS(1) to window to receive focus
*    WM_SET_FOCUS(0) to window to lose focus
*
* Return value:
*   0    on success (Focus could be set)
*   !=0  on failure (Windows could not take the focus)
*/
int WM_SetFocus(WM_Obj * pWin) {
	int r;
	if (pWin && pWin != pWinFocus) {
		WM_NOTIFY_CHILD_HAS_FOCUS_INFO Info;
		Info.pOld = pWinFocus;
		Info.pNew = pWin;
		/* Send a "no more focus" message to window losing focus */
		if (pWinFocus)
			WM_SendMessage(pWinFocus, WM_SET_FOCUS, 0);
		/* Send "You have the focus now" message to the window */
		r = (int)WM_SendMessage((pWinFocus = pWin), WM_SET_FOCUS, 1);
		if (!r) { /* On success only */
			/* Set message to ancestors of window getting the focus */
			while ((pWin = WM_GetParent(pWin)))
				WM_SendMessage(pWin, WM_NOTIFY_CHILD_HAS_FOCUS, (WM_PARAM)&Info);
			/* Set message to ancestors of window loosing the focus */
			pWin = Info.pOld;
			if (WM_IsWindow(pWin)) /* Make sure window has not been deleted in the mean time. Can be optimized: _DeleteWindow could clear the handle to avoid this check (RS) */
				while ((pWin = WM_GetParent(pWin)))
					WM_SendMessage(pWin, WM_NOTIFY_CHILD_HAS_FOCUS, (WM_PARAM)&Info);
		}
	}
	else {
		r = 1;
	}
	return r;
}
/*********************************************************************
*
*       _GetNextChild
*
* Purpose:
*   Returns a handle to the next child of a window.
*
* Parameters:
*   hParent:  handle of parent window.
*   hChild:   handle of child to begin our search to its next sibling.
*
* Return value:
*   Handle to next child if we found one.
*   0 if window has no other children.
*/
static WM_Obj * _GetNextChild(WM_Obj * pParent, WM_Obj * pChild) {
	WM_Obj * pObj = 0;
	if (pChild) {
		pObj = pChild->pNext;
	}
	if (!pObj) {
		pObj = pParent->pFirstChild;
	}
	if (pObj != pChild) {
		return pObj;
	}
	return 0;
}
/*********************************************************************
*
*       WM_SetFocusOnNextChild
*
* Purpose:
*   Sets the focus on next focussable child of a window.
*
* Return value:
*   Handle of focussed child, if we found an other focussable child
*   as the current. Otherwise the return value is zero.
*/
WM_Obj * WM_SetFocusOnNextChild(WM_Obj * pParent) {
	WM_Obj *pChild;
	WM_Obj *pWin;
	pChild = WM__GetFocussedChild(pParent);
	pChild = _GetNextChild(pParent, pChild);
	pWin = pChild;
	while ((WM_IsFocussable(pWin) == 0) && pWin) {
		pWin = _GetNextChild(pParent, pWin);
		if (pWin == pChild) {
			break;
		}
	}
	if (WM_SetFocus(pWin) == 0) {
		return pWin;
	}
	return 0;
}
/*********************************************************************
*
*       _GetPrevChild
*
* Purpose:
*   Returns a handle to the previous child of a window.
*
* Parameters:
*   hParent:  Handle of parent window.
*   hChild:   Handle of child to begin our search to its previous sibling.
*
* Return value:
*   Handle to previous child if we found one.
*   0 if window has no other children.
*/
static WM_Obj * _GetPrevChild(WM_Obj * pChild) {
	WM_Obj * pObj = 0;
	if (pChild)
		pObj = WM_GetPrevSibling(pChild);
	if (!pObj) 
		pObj = WM__GetLastSibling(pChild);
	if (pObj != pChild)
		return pObj;
	return 0;
}
/*********************************************************************
*
*       WM_SetFocusOnPrevChild
*
* Purpose:
*   Sets the focus on previous focussable child of a window.
*
* Return value:
*   Handle of focussed child, if we found an other focussable child
*   as the current. Otherwise the return value is zero.
*/
WM_Obj * WM_SetFocusOnPrevChild(WM_Obj * pParent) {
	WM_Obj *pChild;
	WM_Obj *pWin;
	pChild = WM__GetFocussedChild(pParent);
	pChild = _GetPrevChild(pChild);
	pWin = pChild;
	while (!WM_IsFocussable(pWin) && pWin) {
		pWin = _GetPrevChild(pWin);
		if (pWin == pChild)
			break;
	}
	if (!WM_SetFocus(pWin))
		return pWin;
	return 0;
}
void WM_SetId(WM_Obj * pObj, int Id) {
	WM_SendMessage(pObj, WM_SET_ID, (WM_PARAM)Id);
}
/*********************************************************************
*
*       _SetScrollbar
*
* Return value: 1 if scrollbar was visible, 0 if not
*/
static int _SetScrollbar(WM_Obj * pWin, int OnOff, int Id, int Flags) {
	WM_Obj * pBar = WM_GetDialogItem(pWin, Id);
	if (OnOff) {
		if (!pBar)
			SCROLLBAR_CreateAttached(pWin, Flags);
	}
	else {
		WM_HideWindow(pBar);
		if (pBar)
			WM_DeleteWindow(pBar);
	}
	return (pBar ? 1 : 0);
}
int WM_SetScrollbarV(WM_Obj * pWin, int OnOff) {
	return _SetScrollbar(pWin, OnOff, GUI_ID_VSCROLL, SCROLLBAR_CF_VERTICAL);
}
int WM_SetScrollbarH(WM_Obj * pWin, int OnOff) {
	return _SetScrollbar(pWin, OnOff, GUI_ID_HSCROLL, 0);
}
void WM_SetScrollState(WM_Obj * pWin, const WM_SCROLL_STATE *pState) {
	if (pWin && pState)
		WM_SendMessage(pWin, WM_SET_SCROLL_STATE, (WM_PARAM)pState);
}
#define WM_DEBUG_LEVEL 1
void WM_SetSize(WM_Obj * pWin, int xSize, int ySize) {
	int dx, dy;
	if (pWin) {
		dx = xSize - (pWin->Rect.x1 - pWin->Rect.x0 + 1);
		dy = ySize - (pWin->Rect.y1 - pWin->Rect.y0 + 1);
		WM_ResizeWindow(pWin, dx, dy);
	}
}
#if WM_SUPPORT_TRANSPARENCY   /* If 0, WM will not generate any code */
void WM_SetHasTrans(WM_Obj * pWin) {
	if (pWin) {
		/* First check if this is necessary at all */
		if (!(pWin->Status & WM_SF_HASTRANS)) {
			pWin->Status |= WM_SF_HASTRANS; /* Set Transparency flag */
			WM__TransWindowCnt++;          /* Increment counter for transparency windows */
			WM_Invalidate(pWin);      /* Mark content as invalid */
		}
	}
}
void WM_ClrHasTrans(WM_Obj * pWin) {
	if (pWin) {
		/* First check if this is necessary at all */
		if (pWin->Status & WM_SF_HASTRANS) {
			pWin->Status &= ~WM_SF_HASTRANS;
			WM__TransWindowCnt--;            /* Decrement counter for transparency windows */
			WM_Invalidate(pWin);        /* Mark content as invalid */
		}
	}
}
int WM_GetHasTrans(WM_Obj * pWin) {
	int r = 0;
	if (pWin) {
		r = pWin->Status & WM_SF_HASTRANS;
	}
	return r;
}
void WM_SetTransState(WM_Obj * pWin, unsigned State) {
	if (pWin) {
		if (State & WM_CF_HASTRANS)
			WM_SetHasTrans(pWin);
		else 
			WM_ClrHasTrans(pWin);
		if (State & WM_CF_CONST_OUTLINE) {
			if (!(pWin->Status & WM_CF_CONST_OUTLINE)) {
				pWin->Status |= WM_CF_CONST_OUTLINE;
				WM_Invalidate(pWin);
			}
		}
		else if (pWin->Status & WM_CF_CONST_OUTLINE) {
			pWin->Status &= ~WM_CF_CONST_OUTLINE;
			WM_Invalidate(pWin);
		}
	}
}
#endif /* WM_SUPPORT_TRANSPARENCY */

const GUI_RECT *WM_SetUserClipRect(const GUI_RECT *pRect) {
	const GUI_RECT *pRectReturn = GUI_Context.WM__pUserClipRect;
	GUI_Context.WM__pUserClipRect = pRect;
	/* Activate it ... */
	WM__ActivateClipRect();
	return pRectReturn;
}
int WM_SetXSize(WM_Obj * pWin, int XSize) {
	int dx;
	int r = 0;
	if (pWin) {
		dx = XSize - (pWin->Rect.x1 - pWin->Rect.x0 + 1);
		WM_ResizeWindow(pWin, dx, 0);
		r = pWin->Rect.x1 - pWin->Rect.x0 + 1;
	}
	return r;
}
int WM_SetYSize(WM_Obj * pWin, int YSize) {
	int dy;
	int r = 0;
	if (pWin) {
		dy = YSize - (pWin->Rect.y1 - pWin->Rect.y0 + 1);
		WM_ResizeWindow(pWin, 0, dy);
		r = pWin->Rect.y1 - pWin->Rect.y0 + 1;
	}
	return r;
}
#define WM_DEBUG_LEVEL 1
/*********************************************************************
*
*       WM_InvalidateDescs
  Invalidate window and all descendents (children and grandchildren and ...
*/
void WM_InvalidateDescs(WM_Obj * pWin) {
	WM_Obj * pChild;
	if (pWin) {
		WM_Invalidate(pWin);    /* Invalidate window itself */
		for (pChild = WM_GetFirstChild(pWin); pChild;) {
			auto pNextChild = pChild->pNext;
			WM_InvalidateDescs(pChild);
			pChild = pNextChild;
		}
	}
}
void WM_ShowWindow(WM_Obj * pWin) {
	if (pWin) {
		if ((pWin->Status & WM_SF_ISVIS) == 0) {  /* First check if this is necessary at all */
			pWin->Status |= WM_SF_ISVIS;  /* Set Visibility flag */
			WM_InvalidateDescs(pWin);    /* Mark content as invalid */
#if WM_SUPPORT_NOTIFY_VIS_CHANGED
			WM__NotifyVisChanged(pWin, &pWin->Rect);
#endif
		}
	}
}
void WM_SetStayOnTop(WM_Obj * pWin, int OnOff) {
	if (pWin) {
		uint16_t OldStatus;
		OldStatus = pWin->Status;
		if (OnOff) {
			if (!(pWin->Status & WM_SF_STAYONTOP)) /* First check if this is necessary at all */
				pWin->Status |= WM_SF_STAYONTOP;
		}
		else if (pWin->Status & WM_SF_STAYONTOP) /* First check if this is necessary at all */
			pWin->Status &= ~WM_SF_STAYONTOP;
		if (pWin->Status != OldStatus)
			WM_AttachWindow(pWin, WM_GetParent(pWin));
	}
}
int WM_GetStayOnTop(WM_Obj * pWin) {
	int Result = 0;
	if (pWin) {
		if (pWin->Status & WM_SF_STAYONTOP)
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
static void _SubRect(GUI_RECT *pDest, const GUI_RECT *pr0, const GUI_RECT *pr1) {
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
/*********************************************************************
*
*       WM_ValidateRect
*
  Use this function with great care ! It should under most circumstances not
  be necessary to use it, as validation is done automatically as soon as
  a window has been redrawn. If you validate a section of a window, this
  part will not be included in the paint-command and could therefor not
  be updated.
*/
void WM_ValidateRect(WM_Obj * pWin, const GUI_RECT *pRect) {
	if (pWin) {
		if (pWin->Status & WM_SF_INVALID) {
			if (pRect) {
				_SubRect(&pWin->InvalidRect, &pWin->InvalidRect, pRect);
				if (pWin->InvalidRect)
					return;
			}
			pWin->Status &= ~WM_SF_INVALID;
			NumInvalidWindows--;
		}
	}
}
void WM_ValidateWindow(WM_Obj * pWin) {
	if (pWin) {
		if (pWin->Status & WM_SF_INVALID) {
			pWin->Status &= ~WM_SF_INVALID;
			NumInvalidWindows--;
		}
	}
}
int WM_OnKey(int Key, int Pressed) {
	if (pWinFocus) {
		WM_KEY_INFO Info;
		Info.Key = Key;
		Info.PressedCnt = Pressed;
		WM__SendMessage(pWinFocus, WM_KEY, (WM_PARAM)&Info);
		return 1;
	}
	return 0;
}

#pragma region Mouse/Touch
static WM_CRITICAL_HANDLE WM__CHWinModal;
static WM_CRITICAL_HANDLE WM__CHWinLast;

static bool _IsInModalArea(WM_Obj * pWin) {
	return (!WM__CHWinModal.pWin ||
		WM__IsAncestor(pWin, (WM_Obj *)WM__CHWinModal.pWin) ||
		WM__CHWinModal.pWin == pWin) ? true : false;
}

static void _SendMessageIfEnabled(WM_Obj * pWin, int MsgId, WM_PARAM Data) {
	if (WM_IsEnabled(pWin))
		WM__SendMessage(pWin, MsgId, Data);
}

static void _SendTouchMessage(WM_Obj * pWin, int MsgId, GUI_PID_STATE *pState) {
	if (pState) {
		pState->x -= pWin->Rect.x0;
		pState->y -= pWin->Rect.y0;
	}
	_SendMessageIfEnabled(pWin, MsgId, (WM_PARAM)pState);
	/* Send notification to all ancestors.
	   We need to check if the window which has received the last message still exists,
	   since it may have deleted itself and its parent as result of the message.
	*/
	while (WM_IsWindow(pWin)) {
		pWin = WM_GetParent(pWin);
		if (pWin)
			_SendMessageIfEnabled(pWin, WM_TOUCH_CHILD, (WM_PARAM)pState); /* Send message to the ancestors */
	}
}

static WM_Obj * _Screen2Win(GUI_PID_STATE *pState) {
	if (pWinCapture)
		return pWinCapture;
	return WM_Screen2hWin(pState->x, pState->y);
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
int WM_HandlePID(void) {
	int r = 0;
	WM_CRITICAL_HANDLE CHWin;
	GUI_PID_STATE StateNew;
	GUI_PID_GetState(&StateNew);
	WM__AddCriticalHandle(&CHWin);
	if ((WM_PID__StateLast.x != StateNew.x) || (WM_PID__StateLast.y != StateNew.y) || (WM_PID__StateLast.Pressed != StateNew.Pressed)) {
#if GUI_SUPPORT_CURSOR
		GUI_CURSOR_SetPosition(StateNew.x, StateNew.y);
#endif
		CHWin.pWin = _Screen2Win(&StateNew);
		if (_IsInModalArea((WM_Obj *)CHWin.pWin)) {
			/*
			 * Send WM_PID_STATE_CHANGED message if state has changed (just pressed or just released)
			 */
			if ((WM_PID__StateLast.Pressed != StateNew.Pressed) && CHWin.pWin) {
				WM_PID_STATE_CHANGED_INFO Info;
				auto pWin = CHWin.pWin;
				Info.State = StateNew.Pressed;
				Info.StatePrev = WM_PID__StateLast.Pressed;
				Info.x = StateNew.x - pWin->Rect.x0;
				Info.y = StateNew.y - pWin->Rect.y0;
				_SendMessageIfEnabled((WM_Obj *)CHWin.pWin, WM_PID_STATE_CHANGED, (WM_PARAM)&Info);
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
						GUI_PID_STATE *pState = StateNew.Pressed ? nullptr : &WM_PID__StateLast;
						_SendTouchMessage((WM_Obj *)WM__CHWinLast.pWin, WM_TOUCH, pState);
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
						if (WM__CaptureReleaseAuto)
							WM_ReleaseCapture();
						WM__CHWinLast.pWin = nullptr;
					}
					_SendTouchMessage((WM_Obj *)CHWin.pWin, WM_TOUCH, &StateNew);
				}
			}
#if GUI_SUPPORT_MOUSE
			/* Send WM_MOUSEOVER Message */
			else if (CHWin.pWin)
				/* Do not send messages to disabled windows */
				if (WM_IsEnabled((WM_Obj *)CHWin.pWin))
					_SendTouchMessage((WM_Obj *)CHWin.pWin, WM_MOUSEOVER, &StateNew);
#endif
		}
		/* Store the new state */
		GUI_PID_GetState(&StateNew);
		WM_PID__StateLast = StateNew;
	}
	WM__RemoveCriticalHandle(&CHWin);
	return r;
}
#pragma endregion

void WM_Init(void) {
	static bool _IsInited = false;
	if (_IsInited)
		return;
	pWinNextDraw = pWinFirst = nullptr;
	GUI_Context.WM__pUserClipRect = nullptr;
	NumWindows = NumInvalidWindows = 0;
	/* Make sure we have at least one window. This greatly simplifies the
		drawing routines as they do not have to check if the window is valid.
	*/
	pWinDesktop = WM_CreateWindow(0, 0, GUI_XMAX, GUI_YMAX, WM_CF_SHOW, cbBackWin, 0);
	BkColorDesktop = RGB_INVALID_COLOR;
	WM_Invalidate(pWinDesktop); /* Required because a desktop window has no parent. */
	/* Register the critical handles ... Note: This could be moved into the module setting the Window handle */
	WM__AddCriticalHandle(&WM__CHWinModal);
	WM__AddCriticalHandle(&WM__CHWinLast);
	WM_SelectWindow(pWinDesktop);
	WM_Activate();
	_IsInited = true;
}
