#include <string.h>
#define WM_C
#include "WM_Intern.h"
#include "WM_Intern_ConfDep.h"
#include "GUI_Protected.h"
#include "SCROLLBAR.h"
#include "GUIDebug.h"

#define ASSIGN_IF_LESS(v0,v1) if (v1<v0) v0=v1

typedef struct {
	GUI_RECT ClientRect;
	GUI_RECT CurRect;
	int Cnt;
	int EntranceCnt;
} WM_IVR_CONTEXT;

uint8_t                     WM_IsActive;
uint16_t                    WM__CreateFlags;
WM_HWIN                WM__hCapture;
WM_HWIN                WM__hWinFocus;
char                   WM__CaptureReleaseAuto;
WM_tfPollPID *WM_pfPollPID;
uint8_t                     WM__PaintCallbackCnt;      /* Public for assertions only */
GUI_PID_STATE          WM_PID__StateLast;

#if WM_SUPPORT_TRANSPARENCY
int                    WM__TransWindowCnt;
WM_HWIN                WM__hATransWindow;
#endif

void (*WM__pfShowInvalid)(WM_HWIN hWin);

static WM_HWIN        NextDrawWin;
static WM_IVR_CONTEXT _ClipContext;
static char           _IsInited;
/*********************************************************************
*
*       _CheckCriticalHandles
*
* Purpose:
*   Checks the critical handles and resets the matching one
*/
static void _CheckCriticalHandles(WM_HWIN hWin) {
	WM_CRITICAL_HANDLE *pCH;
	for (pCH = WM__pFirstCriticalHandle; pCH; pCH = pCH->pNext) {
		if (pCH->hWin == hWin) {
			pCH->hWin = 0;
		}
	}
}

/*********************************************************************
*
*       _DesktopHandle2Index
*
* Function:
*   Convert the given desktop window into the display index.
*
* Return value:
*   Desktop index if window handle is valid.
*   else: -1
*/
static int _DesktopHandle2Index(WM_HWIN hDesktop) {
	if (hDesktop == WM__ahDesktopWin) {
		return 0;
	}
	return -1;
}

/*********************************************************************
*
*       _Invalidate1Abs
*
*  Invalidate given window, using absolute coordinates
*/
static void _Invalidate1Abs(WM_HWIN hWin, const GUI_RECT *pRect) {
	GUI_RECT r;
	WM_Obj *pWin;
	int Status;
	pWin = WM_H2P(hWin);
	Status = pWin->Status;
	if ((Status & WM_SF_ISVIS) == 0) {
		return;   /* Window is not visible... we are done */
	}
	if ((Status & (WM_SF_HASTRANS | WM_SF_CONST_OUTLINE)) == WM_SF_HASTRANS) {
		return;   /* Window is transparent; transparency may change... we are done, since background will be invalidated */
	}
	if (WM__RectIsNZ(pRect) == 0) {
		return;   /* Nothing to do ... */
	}
	/* Calc affected area */
	GUI__IntersectRects(&r, pRect, &pWin->Rect);
	if (WM__RectIsNZ(&r)) {
#if WM_SUPPORT_NOTIFY_VIS_CHANGED
		WM__SendMsgNoData(hWin, WM_NOTIFY_VIS_CHANGED);             /* Notify window that visibility may have changed */
#endif

		if (pWin->Status & WM_SF_INVALID) {
			GUI_MergeRect(&pWin->InvalidRect, &pWin->InvalidRect, &r);
		}
		else {
			pWin->InvalidRect = r;
			pWin->Status |= WM_SF_INVALID;
			WM__NumInvalidWindows++;
		}
		/* Debug code: shows invalid areas */
		if (WM__pfShowInvalid) {
			(WM__pfShowInvalid)(hWin);
		}
	}
}

/*********************************************************************
*
*       ResetNextDrawWin

  When drawing, we have to start at the bottom window !
*/
static void ResetNextDrawWin(void) {
	NextDrawWin = WM_HWIN_NULL;
}

/*********************************************************************
*
*       _GethDrawWin
*
* Return Window being drawn.
* Normally same as pAWin, except if overlaying transparent window is drawn
*
*/
static WM_HWIN _GethDrawWin(void) {
	WM_HWIN h;
#if WM_SUPPORT_TRANSPARENCY
	if (WM__hATransWindow) {
		h = WM__hATransWindow;
	}
	else
#endif
	{
		h = GUI_Context.hAWin;
	}
	return h;
}

static void _SetClipRectUserIntersect(const GUI_RECT *prSrc) {
	if (GUI_Context.WM__pUserClipRect == NULL) {
		LCD_SetClipRectEx(prSrc);
	}
	else {
		GUI_RECT r;
		r = *GUI_Context.WM__pUserClipRect;
		WM__Client2Screen(WM_H2P(_GethDrawWin()), &r);     /* Convert User ClipRect into screen coordinates */
		/* Set intersection as clip rect */
		GUI__IntersectRect(&r, prSrc);
		LCD_SetClipRectEx(&r);
	}
}

/*********************************************************************
*
*       WM__ClipAtParentBorders
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
*   hWin    Obvious
*   pRect   Pointer to the rectangle to be clipped. May not be NULL.
*           The parameter is IN/OUT.
*           Note that the rectangle is clipped only if the return
*           value indicates a valid rectangle remains.
*
* Return value:
*   1: Something is or may be visible.
*   0: Nothing is visible (outside of ancestors, no desktop, hidden)
*/
int WM__ClipAtParentBorders(GUI_RECT *pRect, WM_HWIN hWin) {
	WM_Obj *pWin;

	/* Iterate up the window hierarchy.
	   If the window is invisible, we are done.
	   Clip at parent boarders.
	   We are done with iterating if hWin has no parent.
	*/
	do {
		pWin = WM_H2P(hWin);
		if ((pWin->Status & WM_SF_ISVIS) == 0) {
			return 0;                     /* Invisible */
		}
		GUI__IntersectRect(pRect, &pWin->Rect);  /* And clip on borders */
		if (pWin->hParent == 0) {
			break;   /* hWin is now the top level window which has no parent */
		}
		hWin = pWin->hParent;                    /* Go one level up (parent)*/
	} while (1);                               /* Only way out is in the loop. Required for efficiency, no bug, even though some compilers may complain. */

	/* Now check if the top level window is a desktop window. If it is not,
	  then the window is not visible.
	*/
	if (_DesktopHandle2Index(hWin) < 0) {
		return 0;           /* No desktop - (unattached) - Nothing to draw */
	}
	return 1;               /* Something may be visible */
}

void  WM__ActivateClipRect(void) {
	if (WM_IsActive) {
		_SetClipRectUserIntersect(&_ClipContext.CurRect);
	}
	else {    /* Window manager disabled, typically because meory device is active */
		GUI_RECT r;
		WM_Obj *pAWin;
		pAWin = WM_H2P(GUI_Context.hAWin);
		r = pAWin->Rect;
#if WM_SUPPORT_TRANSPARENCY
		if (WM__hATransWindow) {
			WM__ClipAtParentBorders(&r, WM__hATransWindow);
		}
#endif
		/* Take UserClipRect into account */
		_SetClipRectUserIntersect(&r);
	}
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
void WM__InsertWindowIntoList(WM_HWIN hWin, WM_HWIN hParent) {
	int OnTop;
	WM_HWIN hi;
	WM_Obj *pWin;
	WM_Obj *pParent;
	WM_Obj *pi;

	if (hParent) {
		pWin = WM_H2P(hWin);
		pWin->hNext = 0;
		pWin->hParent = hParent;
		pParent = WM_H2P(hParent);
		OnTop = pWin->Status & WM_CF_STAYONTOP;
		hi = pParent->hFirstChild;
		/* Put it at beginning of the list if there is no child */
		if (hi == 0) {   /* No child yet ... Makes things easy ! */
			pParent->hFirstChild = hWin;
			return;                         /* Early out ... We are done */
		}
		/* Put it at beginning of the list if first child is a TOP window and new one is not */
		pi = WM_H2P(hi);
		if (!OnTop) {
			if (pi->Status & WM_SF_STAYONTOP) {
				pWin->hNext = hi;
				pParent->hFirstChild = hWin;
				return;                         /* Early out ... We are done */
			}
		}
		/* Put it at the end of the list or before the last non "STAY-ON-TOP" child */
		do {
			WM_Obj *pNext;
			WM_HWIN hNext;
			if ((hNext = pi->hNext) == 0) {   /* End of sibling list ? */
				pi->hNext = hWin;             /* Then modify this last element to point to new one and we are done */
				break;
			}
			pNext = WM_H2P(hNext);
			if (!OnTop) {
				if (pNext->Status & WM_SF_STAYONTOP) {
					pi->hNext = hWin;
					pWin->hNext = hNext;
					break;
				}
			}
			pi = pNext;
		} while (1);
#if WM_SUPPORT_NOTIFY_VIS_CHANGED
		WM__NotifyVisChanged(hWin, &pWin->Rect);
#endif
	}
}

void WM__RemoveWindowFromList(WM_HWIN hWin) {
	WM_HWIN hi, hParent;
	WM_Obj *pWin, *pParent, *pi;

	pWin = WM_H2P(hWin);
	hParent = pWin->hParent;
	if (hParent) {
		pParent = WM_H2P(hParent);
		hi = pParent->hFirstChild;
		if (hi == hWin) {
			pi = WM_H2P(hi);
			pParent->hFirstChild = pi->hNext;
		}
		else {
			while (hi) {
				pi = WM_H2P(hi);
				if (pi->hNext == hWin) {
					pi->hNext = pWin->hNext;
					break;
				}
				hi = pi->hNext;
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
void WM__DetachWindow(WM_HWIN hWin) {
	WM_Obj *pWin;
	WM_HWIN hParent;
	pWin = WM_H2P(hWin);
	hParent = pWin->hParent;
	if (hParent) {
		WM__RemoveWindowFromList(hWin);
		/* Clear area used by this window */
		WM_InvalidateArea(&pWin->Rect);
		pWin->hParent = 0;
	}
}

static void _DeleteAllChildren(WM_HWIN hChild) {
	while (hChild) {
		WM_Obj *pChild = WM_H2P(hChild);
		WM_HWIN hNext = pChild->hNext;
		WM_DeleteWindow(hChild);
		hChild = hNext;
	}
}

void WM__Client2Screen(const WM_Obj *pWin, GUI_RECT *pRect) {
	GUI_MoveRect(pRect, pWin->Rect.x0, pWin->Rect.y0);
}

int WM__IsWindow(WM_HWIN hWin) {
	WM_HWIN iWin;
	for (iWin = WM__FirstWin; iWin; iWin = WM_H2P(iWin)->hNextLin) {
		if (iWin == hWin) {
			return 1;
		}
	}
	return 0;
}

/*********************************************************************
*
*         WM__InvalidateAreaBelow

  Params: pRect  Rectangle in Absolute coordinates
*/
void WM__InvalidateAreaBelow(const GUI_RECT *pRect, WM_HWIN StopWin) {
	GUI_USE_PARA(StopWin);
	WM_InvalidateArea(pRect);      /* Can be optimized to spare windows above */
}

void WM__RemoveFromLinList(WM_HWIN hWin) {
	WM_Obj *piWin;
	WM_HWIN hiWin;
	WM_HWIN hNext;
	for (hiWin = WM__FirstWin; hiWin; ) {
		piWin = WM_H2P(hiWin);
		hNext = piWin->hNextLin;
		if (hNext == hWin) {
			piWin->hNextLin = WM_H2P(hWin)->hNextLin;
			break;
		}
		hiWin = hNext;
	}
}

static void _AddToLinList(WM_HWIN hNew) {
	WM_Obj *pFirst;
	WM_Obj *pNew;
	if (WM__FirstWin) {
		pFirst = WM_H2P(WM__FirstWin);
		pNew = WM_H2P(hNew);
		pNew->hNextLin = pFirst->hNextLin;
		pFirst->hNextLin = hNew;
	}
	else {
		WM__FirstWin = hNew;
	}
}

/*********************************************************************
*
*       WM__RectIsNZ
*
   Check if the rectangle has some content (is non-zero)
   Returns 0 if the Rectangle has no content, else 1.
*/
int WM__RectIsNZ(const GUI_RECT *pr) {
	if (pr->x0 > pr->x1)
		return 0;
	if (pr->y0 > pr->y1)
		return 0;
	return 1;
}

static void _Findy1(WM_HWIN iWin, GUI_RECT *pRect, GUI_RECT *pParentRect) {
	WM_Obj *pWin;
	for (; iWin; iWin = pWin->hNext) {
		int Status = (pWin = WM_H2P(iWin))->Status;
		/* Check if this window affects us at all */
		if (Status & WM_SF_ISVIS) {
			GUI_RECT rWinClipped;               /* Window rect, clipped to part inside of ancestors */
			if (pParentRect) {
				GUI__IntersectRects(&rWinClipped, &pWin->Rect, pParentRect);
			}
			else {
				rWinClipped = pWin->Rect;
			}
			/* Check if this window affects us at all */
			if (GUI_RectsIntersect(pRect, &rWinClipped)) {
				if ((Status & WM_SF_HASTRANS) == 0) {
					if (pWin->Rect.y0 > pRect->y0) {
						ASSIGN_IF_LESS(pRect->y1, rWinClipped.y0 - 1);      /* Check upper border of window */
					}
					else {
						ASSIGN_IF_LESS(pRect->y1, rWinClipped.y1);        /* Check lower border of window */
					}
				}
				else {
					/* Check all children*/
					WM_HWIN hChild;
					WM_Obj *pChild;
					for (hChild = pWin->hFirstChild; hChild; hChild = pChild->hNext) {
						pChild = WM_H2P(hChild);
						_Findy1(hChild, pRect, &rWinClipped);
					}
				}
			}
		}
	}
}

static int _Findx0(WM_HWIN hWin, GUI_RECT *pRect, GUI_RECT *pParentRect) {
	WM_Obj *pWin;
	int r = 0;
	for (; hWin; hWin = pWin->hNext) {
		int Status = (pWin = WM_H2P(hWin))->Status;
		if (Status & WM_SF_ISVIS) {           /* If window is not visible, it can be safely ignored */
			GUI_RECT rWinClipped;               /* Window rect, clipped to part inside of ancestors */
			if (pParentRect) {
				GUI__IntersectRects(&rWinClipped, &pWin->Rect, pParentRect);
			}
			else {
				rWinClipped = pWin->Rect;
			}
			/* Check if this window affects us at all */
			if (GUI_RectsIntersect(pRect, &rWinClipped)) {
				if ((Status & WM_SF_HASTRANS) == 0) {
					pRect->x0 = rWinClipped.x1 + 1;
					r = 1;
				}
				else {
					/* Check all children */
					WM_HWIN hChild;
					WM_Obj *pChild;
					for (hChild = pWin->hFirstChild; hChild; hChild = pChild->hNext) {
						pChild = WM_H2P(hChild);
						if (_Findx0(hChild, pRect, &rWinClipped)) {
							r = 1;
						}
					}
				}
			}
		}
	}
	return r;
}

static void _Findx1(WM_HWIN hWin, GUI_RECT *pRect, GUI_RECT *pParentRect) {
	WM_Obj *pWin;
	for (; hWin; hWin = pWin->hNext) {
		int Status = (pWin = WM_H2P(hWin))->Status;
		if (Status & WM_SF_ISVIS) {           /* If window is not visible, it can be safely ignored */
			GUI_RECT rWinClipped;               /* Window rect, clipped to part inside of ancestors */
			if (pParentRect) {
				GUI__IntersectRects(&rWinClipped, &pWin->Rect, pParentRect);
			}
			else {
				rWinClipped = pWin->Rect;
			}
			/* Check if this window affects us at all */
			if (GUI_RectsIntersect(pRect, &rWinClipped)) {
				if ((Status & WM_SF_HASTRANS) == 0) {
					pRect->x1 = rWinClipped.x0 - 1;
				}
				else {
					/* Check all children */
					WM_HWIN hChild;
					WM_Obj *pChild;
					for (hChild = pWin->hFirstChild; hChild; hChild = pChild->hNext) {
						pChild = WM_H2P(hChild);
						_Findx1(hChild, pRect, &rWinClipped);
					}
				}
			}
		}
	}
}

void WM_SendMessage(WM_HWIN hWin, WM_MESSAGE *pMsg) {
	if (hWin) {
		WM_Obj *pWin;

		pWin = WM_H2P(hWin);
		if (pWin->cb != NULL) {
			pMsg->hWin = hWin;
			(*pWin->cb)(pMsg);
		}

	}
}

void WM__SendMsgNoData(WM_HWIN hWin, uint8_t MsgId) {
	WM_MESSAGE Msg;
	Msg.hWin = hWin;
	Msg.MsgId = MsgId;
	WM_SendMessage(hWin, &Msg);
}

/*********************************************************************
*
*       WM__GetClientRectWin
*
  Get client rectangle in windows coordinates. This means that the
  upper left corner is always at (0,0).
*/
void WM__GetClientRectWin(const WM_Obj *pWin, GUI_RECT *pRect) {
	pRect->x0 = pRect->y0 = 0;
	pRect->x1 = pWin->Rect.x1 - pWin->Rect.x0;
	pRect->y1 = pWin->Rect.y1 - pWin->Rect.y0;
}

static void WM__GetInvalidRectAbs(WM_Obj *pWin, GUI_RECT *pRect) {
	*pRect = pWin->InvalidRect;
}
/*********************************************************************
*
*       WM_InvalidateRect
*
*  Invalidate a section of the window. The optional rectangle
*  contains client coordinates, which are independent of the
*  position of the window on the logical desktop area.
*/
void WM_InvalidateRect(WM_HWIN hWin, const GUI_RECT *pRect) {
	GUI_RECT r;
	WM_Obj *pWin;
	int Status;
	if (hWin) {

		pWin = WM_H2P(hWin);
		Status = pWin->Status;
		if (Status & WM_SF_ISVIS) {
			r = pWin->Rect;
			if (pRect) {
				GUI_RECT rPara;
				rPara = *pRect;
				WM__Client2Screen(pWin, &rPara);
				GUI__IntersectRect(&r, &rPara);
			}
			if (WM__ClipAtParentBorders(&r, hWin)) {      /* Optimization that saves invalidation if window area is not visible ... Not required */
				if ((Status & (WM_SF_HASTRANS | WM_SF_CONST_OUTLINE)) == WM_SF_HASTRANS) {
					WM__InvalidateAreaBelow(&r, hWin);        /* Can be optimized to spare windows above */
				}
				else {
					_Invalidate1Abs(hWin, &r);
				}
			}
		}

	}
}

/*********************************************************************
*
*        WM_InvalidateWindow
*
  Invalidates an entire window.
*/
void WM_InvalidateWindow(WM_HWIN hWin) {
	WM_InvalidateRect(hWin, NULL);
}

/*********************************************************************
*
*        WM_InvalidateArea

  Invalidate a certain section of the display. One main reason for this is
  that the top window has been moved or destroyed.
  The coordinates given are absolute coordinates (desktop coordinates)
*/
void WM_InvalidateArea(const GUI_RECT *pRect) {
	WM_HWIN   hWin;

	/* Iterate over all windows */
	for (hWin = WM__FirstWin; hWin; hWin = WM_H2P(hWin)->hNextLin) {
		_Invalidate1Abs(hWin, pRect);
	}

}

WM_HWIN WM_CreateWindowAsChild(int x0, int y0, int width, int height
							   , WM_HWIN hParent, uint16_t Style, WM_CALLBACK *cb
							   , int NumExtraBytes) {
	WM_Obj *pWin;
	WM_HWIN hWin;
	WM_ASSERT_NOT_IN_PAINT();

	Style |= WM__CreateFlags;
	/* Default parent is Desktop 0 */
	if (!hParent) {
		if (WM__NumWindows) {
			hParent = WM__ahDesktopWin;
		}
	}
	if (hParent == WM_UNATTACHED) {
		hParent = WM_HWIN_NULL;
	}
	if (hParent) {
		WM_Obj *pParent = WM_H2P(hParent);
		x0 += pParent->Rect.x0;
		y0 += pParent->Rect.y0;
		if (width == 0) {
			width = pParent->Rect.x1 - pParent->Rect.x0 + 1;
		}
		if (height == 0) {
			height = pParent->Rect.y1 - pParent->Rect.y0 + 1;
		}
	}
	if ((hWin = (WM_HWIN)GUI_ALLOC_AllocZero(NumExtraBytes + sizeof(WM_Obj))) == 0) {
	}
	else {
		WM__NumWindows++;
		pWin = WM_H2P(hWin);
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
		_AddToLinList(hWin);
		WM__InsertWindowIntoList(hWin, hParent);
		/* Activate window if WM_CF_ACTIVATE is specified */
		if (Style & WM_CF_ACTIVATE) {
			WM_SelectWindow(hWin);  /* This is not needed if callbacks are being used, but it does not cost a lot and makes life easier ... */
		}
		/* Handle the Style flags, one at a time */
#if WM_SUPPORT_TRANSPARENCY
		if (Style & WM_SF_HASTRANS) {
			WM__TransWindowCnt++;          /* Increment counter for transparency windows */
		}
#endif
		if (Style & WM_CF_BGND) {
			WM_BringToBottom(hWin);
		}
		if (Style & WM_CF_SHOW) {
			pWin->Status |= WM_SF_ISVIS;  /* Set Visibility flag */
			WM_InvalidateWindow(hWin);    /* Mark content as invalid */
		}
		WM__SendMsgNoData(hWin, WM_CREATE);
	}

	return hWin;
}

WM_HWIN WM_CreateWindow(int x0, int y0, int width, int height, uint16_t Style, WM_CALLBACK *cb, int NumExtraBytes) {
	return WM_CreateWindowAsChild(x0, y0, width, height, 0 /* No parent */, Style, cb, NumExtraBytes);
}

void WM_DeleteWindow(WM_HWIN hWin) {
	WM_Obj *pWin;
	if (!hWin) {
		return;
	}
	WM_ASSERT_NOT_IN_PAINT();

	if (WM__IsWindow(hWin)) {
		pWin = WM_H2P(hWin);
		ResetNextDrawWin();              /* Make sure the window will no longer receive drawing messages */
		/* Make sure that focus is set to an existing window */
		if (WM__hWinFocus == hWin) {
			WM__hWinFocus = 0;
		}
		if (WM__hCapture == hWin) {
			WM__hCapture = 0;
		}
		/* check if critical handles are affected. If so, reset the window handle to 0 */
		_CheckCriticalHandles(hWin);
		/* Inform parent */
		WM_NotifyParent(hWin, WM_NOTIFICATION_CHILD_DELETED);
		/* Delete all children */
		_DeleteAllChildren(pWin->hFirstChild);
#if WM_SUPPORT_NOTIFY_VIS_CHANGED
		WM__SendMsgNoData(hWin, WM_NOTIFY_VIS_CHANGED);             /* Notify window that visibility may have changed */
#endif
		/* Send WM_DELETE message to window in order to inform window itself */
		WM__SendMsgNoData(hWin, WM_DELETE);     /* tell window about it */
		WM__DetachWindow(hWin);
		/* Remove window from window stack */
		WM__RemoveFromLinList(hWin);
		/* Handle transparency counter if necessary */
#if WM_SUPPORT_TRANSPARENCY
		if (pWin->Status & WM_SF_HASTRANS) {
			WM__TransWindowCnt--;
		}
#endif
		/* Make sure window is no longer counted as invalid */
		if (pWin->Status & WM_SF_INVALID) {
			WM__NumInvalidWindows--;
		}
		/* Free window memory */
		WM__NumWindows--;
		GUI_ALLOC_Free(hWin);
		/* Select a valid window */
		WM_SelectWindow(WM__FirstWin);
	}
}

/*********************************************************************
*
*       WM_SelectWindow
*
*  Sets the active Window. The active Window is the one that is used for all
*  drawing (and text) operations.
*/
WM_HWIN WM_SelectWindow(WM_HWIN  hWin) {
	WM_HWIN hWinPrev;
	WM_Obj *pObj;

	WM_ASSERT_NOT_IN_PAINT();

	hWinPrev = GUI_Context.hAWin;
	if (hWin == 0) {
		hWin = WM__FirstWin;
	}
	/* Select new window */
	GUI_Context.hAWin = hWin;
	pObj = WM_H2P(hWin);
	LCD_SetClipRectMax();             /* Drawing operations will clip ... If WM is deactivated, allow all */
	GUI_Context.xOff = pObj->Rect.x0;
	GUI_Context.yOff = pObj->Rect.y0;

	return hWinPrev;
}

WM_HWIN WM_GetActiveWindow(void) {
	return GUI_Context.hAWin;
}

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
	WM_HMEM hParent;
	GUI_RECT r;
	WM_Obj *pAWin;
	WM_Obj *pParent;
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
	if (r.y0 > _ClipContext.ClientRect.y1) {
		return 0;
	}
	/* STEP 3:
		 Find out the max. height (r.y1) if we are at the left border.
		 Since we are using the same height for all IVRs at the same y0,
		 we do this only for the leftmost one.
	*/
	pAWin = WM_H2P(GUI_Context.hAWin);
	if (r.x0 == _ClipContext.ClientRect.x0) {
		r.y1 = _ClipContext.ClientRect.y1;
		r.x1 = _ClipContext.ClientRect.x1;
		/* Iterate over all windows which are above */
		/* Check all siblings above (Iterate over Parents and top siblings (hNext) */
		for (hParent = GUI_Context.hAWin; hParent; hParent = pParent->hParent) {
			pParent = WM_H2P(hParent);
			_Findy1(pParent->hNext, &r, NULL);
		}
		/* Check all children */
		_Findy1(pAWin->hFirstChild, &r, NULL);
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
	if (Status & WM_SF_DONT_CLIP_SIBLINGS) {
		hParent = pAWin->hParent;
	}
	else
#endif
	{
		hParent = GUI_Context.hAWin;
	}
	for (; hParent; hParent = pParent->hParent) {
		pParent = WM_H2P(hParent);
		if (_Findx0(pParent->hNext, &r, NULL)) {
			goto Find_x0;
		}
	}
	/* Check all children */
	if (_Findx0(pAWin->hFirstChild, &r, NULL)) {
		goto Find_x0;
	}
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
	if (Status & WM_SF_DONT_CLIP_SIBLINGS) {
		hParent = pAWin->hParent;
	}
	else
#endif
	{
		hParent = GUI_Context.hAWin;
	}
	for (; hParent; hParent = pParent->hParent) {
		pParent = WM_H2P(hParent);
		_Findx1(pParent->hNext, &r, NULL);
	}
	/* Check all children */
	_Findx1(pAWin->hFirstChild, &r, NULL);
	/* We are done. Return the rectangle we found in the _ClipContext. */
	if (_ClipContext.Cnt > 200) {
		return 0;  /* error !!! This should not happen !*/
	}
	_ClipContext.CurRect = r;
	return 1;  /* IVR is valid ! */
}

#else

static int _FindNext_IVR(void) {
	if (_ClipContext.Cnt == 0) {
		_ClipContext.CurRect = GUI_Context.pAWin->Rect;
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
int  WM__GetNextIVR(void) {
#if GUI_SUPPORT_CURSOR
	static char _CursorHidden;
#endif
	/* If WM is not active, we have no rectangles to return */
	if (WM_IsActive == 0) {
		return 0;
	}
	if (_ClipContext.EntranceCnt > 1) {
		_ClipContext.EntranceCnt--;
		return 0;
	}
#if GUI_SUPPORT_CURSOR
	if (_CursorHidden) {
		_CursorHidden = 0;
		(*GUI_CURSOR_pfTempUnhide) ();
	}
#endif
	++_ClipContext.Cnt;
	/* Find next rectangle and use it as ClipRect */
	if (!_FindNext_IVR()) {
		_ClipContext.EntranceCnt--;  /* This search is over ! */
		return 0;        /* Could not find an other one ! */
	}
	WM__ActivateClipRect();
	/* Hide cursor if necessary */
#if GUI_SUPPORT_CURSOR
	if (GUI_CURSOR_pfTempHide) {
		_CursorHidden = (*GUI_CURSOR_pfTempHide) (&_ClipContext.CurRect);
	}
#endif
	return 1;
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
int WM__InitIVRSearch(const GUI_RECT *pMaxRect) {
	GUI_RECT r;
	WM_Obj *pAWin;
	/* If WM is not active -> nothing to do, leave cliprect alone */
	if (WM_IsActive == 0) {
		WM__ActivateClipRect();
		return 1;
	}
	/* If we entered multiple times, leave Cliprect alone */
	if (++_ClipContext.EntranceCnt > 1)
		return 1;
	pAWin = WM_H2P(GUI_Context.hAWin);
	_ClipContext.Cnt = -1;
	/* When using callback mechanism, it is legal to reduce drawing
	   area to the invalid area ! */
	if (WM__PaintCallbackCnt) {
		WM__GetInvalidRectAbs(pAWin, &r);
	}
	else {  /* Not using callback mechanism, therefor allow entire rectangle */
		if (pAWin->Status & WM_SF_ISVIS) {
			r = pAWin->Rect;
		}
		else {
			--_ClipContext.EntranceCnt;
			return 0;  /* window is not even visible ! */
		}
	}
	/* If the drawing routine has specified a rectangle, use it to reduce the rectangle */
	if (pMaxRect) {
		GUI__IntersectRect(&r, pMaxRect);
	}
	/* If user has reduced the cliprect size, reduce the rectangle */
	if (GUI_Context.WM__pUserClipRect) {
		WM_Obj *pWin = pAWin;
		GUI_RECT rUser = *(GUI_Context.WM__pUserClipRect);
#if WM_SUPPORT_TRANSPARENCY
		if (WM__hATransWindow) {
			pWin = WM_H2P(WM__hATransWindow);
		}
#endif
		WM__Client2Screen(pWin, &rUser);
		GUI__IntersectRect(&r, &rUser);
	}
	/* For transparent windows, we need to further reduce the rectangle */
#if WM_SUPPORT_TRANSPARENCY
	if (WM__hATransWindow) {
		if (WM__ClipAtParentBorders(&r, WM__hATransWindow) == 0) {
			--_ClipContext.EntranceCnt;
			return 0;           /* Nothing to draw */
		}
	}
#endif
	/* Iterate over all ancestors and clip at their borders. If there is no visible part, we are done */
	if (WM__ClipAtParentBorders(&r, GUI_Context.hAWin) == 0) {
		--_ClipContext.EntranceCnt;
		return 0;           /* Nothing to draw */
	}
	/* Store the rectangle and find the first rectangle of the area */
	_ClipContext.ClientRect = r;
	return WM__GetNextIVR();
}

void WM_SetDefault(void) {
	GUI_SetDefault();
	GUI_Context.WM__pUserClipRect = NULL;   /* No add. clipping */
}

static void _Paint1(WM_HWIN hWin, WM_Obj *pWin) {
	int Status = pWin->Status;
	/* Send WM_PAINT if window is visible and a callback is defined */
	if ((pWin->cb != NULL) && (Status & WM_SF_ISVIS)) {
		WM_MESSAGE Msg;
		WM__PaintCallbackCnt++;
		if (Status & WM_SF_LATE_CLIP) {
			Msg.hWin = hWin;
			Msg.MsgId = WM_PAINT;
			Msg.Data.p = (GUI_RECT *)&pWin->InvalidRect;
			WM_SetDefault();
			WM_SendMessage(hWin, &Msg);
		}
		else {
			WM_ITERATE_START(&pWin->InvalidRect) {
				Msg.hWin = hWin;
				Msg.MsgId = WM_PAINT;
				Msg.Data.p = (GUI_RECT *)&pWin->InvalidRect;
				WM_SetDefault();
				WM_SendMessage(hWin, &Msg);
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
static int _Paint1Trans(WM_HWIN hWin, WM_Obj *pWin) {
	int xPrev, yPrev;
	WM_Obj *pAWin = WM_H2P(GUI_Context.hAWin);
	/* Check if we need to do any drawing */
	if (GUI_RectsIntersect(&pAWin->InvalidRect, &pWin->Rect)) {
		/* Save old values */
		xPrev = GUI_Context.xOff;
		yPrev = GUI_Context.yOff;
		/* Set values for the current (transparent) window, rather than the one below */
		GUI__IntersectRects(&pWin->InvalidRect, &pWin->Rect, &pAWin->InvalidRect);
		WM__hATransWindow = hWin;
		GUI_Context.xOff = pWin->Rect.x0;
		GUI_Context.yOff = pWin->Rect.y0;
		/* Do the actual drawing ... */
		_Paint1(hWin, pWin);
		/* Restore settings */
		WM__hATransWindow = 0;
		GUI_Context.xOff = xPrev;
		GUI_Context.yOff = yPrev;
		return 1;                       /* Some drawing took place */
	}
	return 0;                         /* No invalid area, so nothing was drawn */
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
	WM_HWIN hChild;
	WM_Obj *pChild;
	if (pWin->Status & WM_SF_ISVIS) {
		for (hChild = pWin->hFirstChild; hChild; hChild = pChild->hNext) {
			pChild = WM_H2P(hChild);
			if ((pChild->Status & (WM_SF_HASTRANS | WM_SF_ISVIS))   /* Transparent & visible ? */
				== (WM_SF_HASTRANS | WM_SF_ISVIS)) {
				/* Set invalid area of the window to draw */
				if (GUI_RectsIntersect(&pChild->Rect, &pWin->InvalidRect)) {
					GUI_RECT InvalidRectPrev;
					InvalidRectPrev = pWin->InvalidRect;
					if (_Paint1Trans(hChild, pChild)) {
						_PaintTransChildren(pChild);
					}
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
static void _PaintTransTopSiblings(WM_HWIN hWin, WM_Obj *pWin) {
	WM_HWIN hParent;
	WM_Obj *pParent;
	hParent = pWin->hParent;
	hWin = pWin->hNext;
	while (hParent) { /* Go hierarchy up to desktop window */
		for (; hWin; hWin = pWin->hNext) {
			pWin = WM_H2P(hWin);
			/* paint window if it is transparent & visible */
			if ((pWin->Status & (WM_SF_HASTRANS | WM_SF_ISVIS)) == (WM_SF_HASTRANS | WM_SF_ISVIS)) {
				_Paint1Trans(hWin, pWin);
			}
			/* paint transparent & visible children */
			_PaintTransChildren(pWin);
		}
		pParent = WM_H2P(hParent);
		hWin = pParent->hNext;
		hParent = pParent->hParent;
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
void WM__PaintWinAndOverlays(WM_PAINTINFO *pInfo) {
	WM_HWIN hWin;
	WM_Obj *pWin;
	hWin = pInfo->hWin;
	pWin = pInfo->pWin;
#if WM_SUPPORT_TRANSPARENCY
	/* Transparent windows without const outline are drawn as part of the background and can be skipped. */
	if ((pWin->Status & (WM_SF_HASTRANS | WM_SF_CONST_OUTLINE)) != WM_SF_HASTRANS) {
#endif
		_Paint1(hWin, pWin);    /* Draw the window itself */
#if WM_SUPPORT_TRANSPARENCY
	}
	if (WM__TransWindowCnt != 0) {
		_PaintTransChildren(pWin);       /* Draw all transparent children */
		_PaintTransTopSiblings(hWin, pWin);    /* Draw all transparent top level siblings */
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
	WM_Obj *pWin = WM_H2P(GUI_Context.hAWin);
	Rect = pWin->InvalidRect;
	pWin->InvalidRect = GUI_Context.ClipRect;
	WM__PaintWinAndOverlays((WM_PAINTINFO *)p);
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
static int _Paint(WM_HWIN hWin, WM_Obj *pWin) {
	int Ret = 0;
	if (pWin->Status & WM_SF_INVALID) {
		if (pWin->cb) {
			if (WM__ClipAtParentBorders(&pWin->InvalidRect, hWin)) {
				WM_PAINTINFO Info;
				Info.hWin = hWin;
				Info.pWin = pWin;
				WM_SelectWindow(hWin);
#if GUI_SUPPORT_MEMDEV
				if (pWin->Status & WM_SF_MEMDEV) {
					int Flags;
					GUI_RECT r = pWin->InvalidRect;
					Flags = (pWin->Status & WM_SF_HASTRANS) ? GUI_MEMDEV_HASTRANS : GUI_MEMDEV_NOTRANS;
					/*
					 * Currently we treat a desktop window as transparent, because per default it does not repaint itself.
					 */
					if (pWin->hParent == 0) {
						Flags = GUI_MEMDEV_HASTRANS;
					}
					GUI_MEMDEV_Draw(&r, _cbPaintMemDev, &Info, 0, Flags);
				}
				else
#endif
				{
					WM__PaintWinAndOverlays(&Info);
					Ret = 1;    /* Something has been done */
				}
			}
		}
		/* We purposly clear the invalid flag after painting so we can still query the invalid rectangle while painting */
		pWin->Status &= ~WM_SF_INVALID; /* Clear invalid flag */
		if (pWin->Status & WM_CF_MEMDEV_ON_REDRAW) {
			pWin->Status |= WM_CF_MEMDEV;
		}
		WM__NumInvalidWindows--;
	}
	return Ret;      /* Nothing done */
}

static void _DrawNext(void) {
	int UpdateRem = 1;
	WM_HWIN iWin = (NextDrawWin == WM_HWIN_NULL) ? WM__FirstWin : NextDrawWin;
	GUI_CONTEXT ContextOld;
	GUI_SaveContext(&ContextOld);
	/* Make sure the next window to redraw is valid */
	for (; iWin && UpdateRem; ) {
		WM_Obj *pWin = WM_H2P(iWin);
		if (_Paint(iWin, pWin)) {
			UpdateRem--;  /* Only the given number of windows at a time ... */
		}
		iWin = pWin->hNextLin;
	}
	NextDrawWin = iWin;   /* Remember the window */
	GUI_RestoreContext(&ContextOld);
}

int WM_Exec1(void) {
	/* Poll PID if necessary */
	if (WM_pfPollPID) {
		WM_pfPollPID();
	}
	if (WM_pfHandlePID) {
		if (WM_pfHandlePID())
			return 1;               /* We have done something ... */
	}
	if (GUI_PollKeyMsg()) {
		return 1;               /* We have done something ... */
	}
	if (WM_IsActive && WM__NumInvalidWindows) {

		_DrawNext();

		return 1;               /* We have done something ... */
	}
	return 0;                  /* There was nothing to do ... */
}

int WM_Exec(void) {
	int r = 0;
	while (WM_Exec1()) {
		r = 1;                  /* We have done something */
	}
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
static void cbBackWin(WM_MESSAGE *pMsg) {
	const WM_KEY_INFO *pKeyInfo;
	switch (pMsg->MsgId) {
		case WM_KEY:
			pKeyInfo = (const WM_KEY_INFO *)pMsg->Data.p;
			if (pKeyInfo->PressedCnt == 1) {
				GUI_StoreKey(pKeyInfo->Key);
			}
			break;
		case WM_PAINT:
		{
			if (WM__aBkColor != GUI_INVALID_COLOR) {
				GUI_SetBkColor(WM__aBkColor);
				GUI_Clear();
			}
		}
		default:
			WM_DefaultProc(pMsg);
	}
}

void WM_Activate(void) {
	WM_IsActive = 1;       /* Running */
}

void WM_Deactivate(void) {
	WM_IsActive = 0;       /* No clipping performed by WM */

	LCD_SetClipRectMax();

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
void WM_DefaultProc(WM_MESSAGE *pMsg) {
	WM_HWIN hWin = pMsg->hWin;
	const void *p = pMsg->Data.p;
	WM_Obj *pWin = WM_H2P(hWin);
	/* Exec message */
	switch (pMsg->MsgId) {
		case WM_GET_INSIDE_RECT:      /* return client window in absolute (screen) coordinates */
			WM__GetClientRectWin(pWin, (GUI_RECT *)p);
			break;
		case WM_GET_CLIENT_WINDOW:      /* return handle to client window. For most windows, there is no seperate client window, so it is the same handle */
			pMsg->Data.p = hWin;
			return;                       /* Message handled */
		case WM_KEY:
			WM_SendToParent(hWin, pMsg);
			return;                       /* Message handled */
		case WM_GET_BKCOLOR:
			pMsg->Data.Color = GUI_INVALID_COLOR;
			return;                       /* Message handled */
		case WM_NOTIFY_ENABLE:
			WM_InvalidateWindow(hWin);
			return;                       /* Message handled */
	}
	/* Message not handled. If it queries something, we return 0 to be on the safe side. */
	pMsg->Data.v = 0;
	pMsg->Data.p = 0;
}

void WM_Init(void) {
	if (!_IsInited) {
		NextDrawWin = WM__FirstWin = WM_HWIN_NULL;
		GUI_Context.WM__pUserClipRect = NULL;
		WM__NumWindows = WM__NumInvalidWindows = 0;
		/* Make sure we have at least one window. This greatly simplifies the
			drawing routines as they do not have to check if the window is valid.
		*/
		WM__ahDesktopWin = WM_CreateWindow(0, 0, GUI_XMAX, GUI_YMAX, WM_CF_SHOW, cbBackWin, 0);
		WM__aBkColor = GUI_INVALID_COLOR;
		WM_InvalidateWindow(WM__ahDesktopWin); /* Required because a desktop window has no parent. */
		/* Register the critical handles ... Note: This could be moved into the module setting the Window handle */
		WM__AddCriticalHandle(&WM__CHWinModal);
		WM__AddCriticalHandle(&WM__CHWinLast);

		WM_SelectWindow(WM__ahDesktopWin);
		WM_Activate();
		_IsInited = 1;
	}
}

void WM__ForEachDesc(WM_HWIN hWin, WM_tfForEach *pcb, void *pData) {
	WM_HWIN hChild;
	WM_Obj *pChild;
	WM_Obj *pWin;
	pWin = WM_H2P(hWin);
	for (hChild = pWin->hFirstChild; hChild; hChild = pChild->hNext) {
		pChild = WM_H2P(hChild);
		pcb(hChild, pData);
		WM_ForEachDesc(hChild, pcb, pData);
	}
}

/*********************************************************************
*
*       WM__GetFirstSibling

  Return value: Handle of parent, 0 if none
*/
WM_HWIN WM__GetFirstSibling(WM_HWIN hWin) {
	hWin = WM_GetParent(hWin);
	return (hWin) ? WM_HANDLE2PTR(hWin)->hFirstChild : 0;
}

WM_HWIN WM__GetFocussedChild(WM_HWIN hWin) {
	WM_HWIN r = 0;
	if (WM__IsChild(WM__hWinFocus, hWin)) {
		r = WM__hWinFocus;
	}
	return r;
}

/*********************************************************************
*
*       WM__GetLastSibling

  Return value: Handle of last sibling
*/
WM_HWIN WM__GetLastSibling(WM_HWIN hWin) {
	WM_Obj *pWin;
	for (; hWin; hWin = pWin->hNext) {
		pWin = WM_H2P(hWin);
		if (pWin->hNext == 0) {
			break;
		}
	}
	return hWin;
}

/*********************************************************************
*
*       WM__GetPrevSibling

  Return value: Handle of previous sibling (if any), otherwise 0
*/
WM_HWIN WM__GetPrevSibling(WM_HWIN hWin) {
	WM_HWIN hi;
	WM_Obj *pi;
	for (hi = WM__GetFirstSibling(hWin); hi; hi = pi->hNext) {
		if (hi == hWin) {
			hi = 0;                 /* There is no previous sibling. Return 0 */
			break;
		}
		pi = WM_H2P(hi);
		if (pi->hNext == hWin) {
			break;                  /* We found the previous one ! */
		}
	}
	return hi;
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
int WM__IsAncestor(WM_HWIN hChild, WM_HWIN hParent) {
	int r = 0;
	if (hChild && hParent) {
		while (hChild) {
			WM_Obj *pChild = WM_H2P(hChild);
			if (pChild->hParent == hParent) {
				r = 1;
				break;
			}
			hChild = pChild->hParent;
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
int WM__IsAncestorOrSelf(WM_HWIN hChild, WM_HWIN hParent) {
	if (hChild == hParent) {
		return 1;
	}
	return WM__IsAncestor(hChild, hParent);
}

int WM__IsChild(WM_HWIN hWin, WM_HWIN hParent) {
	int r = 0;
	if (hWin) {
		WM_Obj *pObj = WM_H2P(hWin);
		if (pObj) {
			if (pObj->hParent == hParent) {
				r = 1;
			}
		}
	}
	return r;
}

int WM__IsEnabled(WM_HWIN hWin) {
	int r = 1;
	if ((WM_H2P(hWin)->Status) & WM_SF_DISABLED) {
		r = 0;
	}
	return r;
}

/*********************************************************************
*
*       _NotifyVisChanged
*
* Description
*   Notify all descendents
*/
static void _NotifyVisChanged(WM_HWIN hWin, GUI_RECT *pRect) {
	WM_Obj *pWin;

	for (hWin = WM_GetFirstChild(hWin); hWin; hWin = pWin->hNext) {
		pWin = WM_H2P(hWin);
		if (pWin->Status & WM_SF_ISVIS) {
			if (GUI_RectsIntersect(&pWin->Rect, pRect)) {
				WM__SendMsgNoData(hWin, WM_NOTIFY_VIS_CHANGED);             /* Notify window that visibility may have changed */
				_NotifyVisChanged(hWin, pRect);
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
void WM__NotifyVisChanged(WM_HWIN hWin, GUI_RECT *pRect) {
	WM_Obj *pWin;
	WM_HWIN hParent;
	pWin = WM_H2P(hWin);
	hParent = pWin->hParent;
	if (hParent) {
		_NotifyVisChanged(hParent, pRect);
	}
}

void WM__Screen2Client(const WM_Obj *pWin, GUI_RECT *pRect) {
	GUI_MoveRect(pRect, -pWin->Rect.x0, -pWin->Rect.y0);
}

void WM__SendMessage(WM_HWIN hWin, WM_MESSAGE *pMsg) {
	WM_Obj *pWin = WM_HANDLE2PTR(hWin);
	pMsg->hWin = hWin;
	if (pWin->cb != NULL) {
		(*pWin->cb)(pMsg);
	}
	else {
		WM_DefaultProc(pMsg);
	}
}

void WM__SendMessageIfEnabled(WM_HWIN hWin, WM_MESSAGE *pMsg) {
	if (WM__IsEnabled(hWin)) {
		WM__SendMessage(hWin, pMsg);
	}
}

void WM__SendMessageNoPara(WM_HWIN hWin, int MsgId) {
	WM_MESSAGE Msg = { 0 };
	WM_Obj *pWin = WM_HANDLE2PTR(hWin);
	if (pWin->cb != NULL) {
		Msg.hWin = hWin;
		Msg.MsgId = MsgId;
		(*pWin->cb)(&Msg);
	}
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
	WM_HWIN hChild;
	WM_Obj *pChild;
	int dx, dy, dw, dh;
	for (hChild = pObj->hFirstChild; hChild; hChild = pChild->hNext) {
		int Status;
		GUI_RECT rOld, rNew;
		pChild = WM_H2P(hChild);
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
			WM_MoveWindow(hChild, dx, dy);
		}
		dw = (rNew.x1 - rNew.x0) - (rOld.x1 - rOld.x0);
		dh = (rNew.y1 - rNew.y0) - (rOld.y1 - rOld.y0);
		if (dw || dh) {
			WM_ResizeWindow(hChild, dw, dh);
		}
	}
}

void WM_DetachWindow(WM_HWIN hWin) {
	if (hWin) {
		WM_HWIN hParent;
		WM_Obj *pWin;

		pWin = WM_H2P(hWin);
		hParent = pWin->hParent;
		if (hParent) {
			WM_Obj *pParent;
			WM__DetachWindow(hWin);
			pParent = WM_H2P(hParent);
			WM_MoveWindow(hWin, -pParent->Rect.x0, -pParent->Rect.y0);   /* Convert screen coordinates -> parent coordinates */
			/* ToDo: Invalidate. If Parent window is located at (0,0). */
		}

	}
}

void WM_AttachWindow(WM_HWIN hWin, WM_HWIN hParent) {

	if (hParent && (hParent != hWin)) {
		WM_Obj *pWin = WM_H2P(hWin);
		WM_Obj *pParent = WM_H2P(hParent);
		if (pWin->hParent != hParent) {
			WM_DetachWindow(hWin);
			WM__InsertWindowIntoList(hWin, hParent);
			WM_MoveWindow(hWin, pParent->Rect.x0, pParent->Rect.y0);    /* Convert parent coordinates -> screen coordinates */
		}
	}

}

void WM_AttachWindowAt(WM_HWIN hWin, WM_HWIN hParent, int x, int y) {
	WM_DetachWindow(hWin);
	WM_MoveTo(hWin, x, y);
	WM_AttachWindow(hWin, hParent);
}

void WM_BringToBottom(WM_HWIN hWin) {
	WM_HWIN hParent;
	WM_HWIN hPrev;
	WM_Obj *pWin;
	WM_Obj *pPrev;
	WM_Obj *pParent;

	if (hWin) {
		pWin = WM_H2P(hWin);
		hPrev = WM__GetPrevSibling(hWin);
		if (hPrev) {                   /* If there is no previous one, there is nothing to do ! */
			hParent = WM_GetParent(hWin);
			pParent = WM_H2P(hParent);
			/* unlink hWin */
			pPrev = WM_H2P(hPrev);
			pPrev->hNext = pWin->hNext;
			/* Link from parent (making it the first child) */
			pWin->hNext = pParent->hFirstChild;
			pParent->hFirstChild = hWin;
			/* Send message in order to make sure top window will be drawn */
			WM_InvalidateArea(&pWin->Rect);
		}
	}

}

typedef struct {
	GUI_RECT Rect;
} VCDATA;

static void _cbInvalidateOne(WM_HWIN hWin, void *p) {
	GUI_USE_PARA(p);
	WM_InvalidateWindow(hWin);
}

static void _InvalidateWindowAndDescs(WM_HWIN hWin) {
	WM_InvalidateWindow(hWin);
	WM_ForEachDesc(hWin, _cbInvalidateOne, 0);
}

static void _BringToTop(WM_HWIN hWin) {
	WM_HWIN hNext, hParent;
	WM_Obj *pWin, *pNext;
	if (hWin) {
		pWin = WM_H2P(hWin);
		hNext = pWin->hNext;
		/* Is window alread on top ? If so, we are done. (Not required, just an optimization) */
		if (hNext == 0) {
			return;
		}
		/* For non-top windows, it is good enough if the next one is a stay-on-top-window (Not required, just an optimization) */
		if ((pWin->Status & WM_SF_STAYONTOP) == 0) {
			pNext = WM_H2P(hNext);
			if (pNext->Status & WM_SF_STAYONTOP) {
				return;
			}
		}
		hParent = pWin->hParent;
		WM__RemoveWindowFromList(hWin);
		WM__InsertWindowIntoList(hWin, hParent);
		_InvalidateWindowAndDescs(hWin);
	}
}

void WM_BringToTop(WM_HWIN hWin) {

	_BringToTop(hWin);

}

int WM_BroadcastMessage(WM_MESSAGE *pMsg) {
	WM_HWIN hWin;

	for (hWin = WM__FirstWin; hWin; ) {
		WM_SendMessage(hWin, pMsg);
		hWin = WM_H2P(hWin)->hNextLin;
	}

	return 0;
}

void WM_CheckScrollBounds(WM_SCROLL_STATE *pScrollState) {
	int Max;
	Max = pScrollState->NumItems - pScrollState->PageSize;
	if (Max < 0) {
		Max = 0;
	}
	/* Make sure scroll pos is in bounds */
	if (pScrollState->v < 0) {
		pScrollState->v = 0;
	}
	if (pScrollState->v > Max) {
		pScrollState->v = Max;
	}
}

/*********************************************************************
*
*       WM_CheckScrollPos
*
* Return value: Difference between old an new Scroll value.
*               Therefor 0 if settings have not changed.
*/
int  WM_CheckScrollPos(WM_SCROLL_STATE *pScrollState, int Pos, int LowerDist, int UpperDist) {
	int vOld;
	vOld = pScrollState->v;
	/* Check upper limit */
	if (Pos > pScrollState->v + pScrollState->PageSize - 1) {
		pScrollState->v = Pos - (pScrollState->PageSize - 1) + UpperDist;
	}
	/* Check lower limit */
	if (Pos < pScrollState->v) {
		pScrollState->v = Pos - LowerDist;
	}
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
	int vOld;
	vOld = pScrollState->v;
	pScrollState->v = v;
	WM_CheckScrollBounds(pScrollState);
	return pScrollState->v - vOld;
}

void WM__AddCriticalHandle(WM_CRITICAL_HANDLE *pCriticalHandle) {
	pCriticalHandle->pNext = WM__pFirstCriticalHandle;
	WM__pFirstCriticalHandle = pCriticalHandle;
}

void WM__RemoveCriticalHandle(WM_CRITICAL_HANDLE *pCriticalHandle) {
	if (WM__pFirstCriticalHandle) {
		WM_CRITICAL_HANDLE *pCH, *pLast = 0;
		for (pCH = WM__pFirstCriticalHandle; pCH; pCH = pCH->pNext) {
			if (pCH == pCriticalHandle) {
				if (pLast) {
					pLast->pNext = pCH->pNext;
				}
				else if (pCH->pNext) {
					WM__pFirstCriticalHandle = pCH->pNext;
				}
				else {
					WM__pFirstCriticalHandle = 0;
				}
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
static void _ShowInvalid(WM_HWIN hWin) {
	GUI_CONTEXT Context = GUI_Context;
	GUI_RECT rClient;
	WM_Obj *pWin;
	pWin = WM_H2P(hWin);
	rClient = pWin->InvalidRect;
	GUI_MoveRect(&rClient, -pWin->Rect.x0, -pWin->Rect.y0);
	WM_SelectWindow(hWin);
	GUI_SetColor(GUI_GREEN);
	GUI_SetBkColor(GUI_GREEN);
	GUI_FillRect(rClient.x0, rClient.y0, rClient.x1, rClient.y1);
	GUI_Context = Context;
}

void WM_DIAG_EnableInvalidationColoring(int OnOff) {
	if (OnOff) {
		WM__pfShowInvalid = _ShowInvalid;
	}
	else {
		WM__pfShowInvalid = NULL;
	}
}

void WM_SetEnableState(WM_HWIN hWin, int State) {
	WM_Obj *pWin;
	uint16_t Status;

	pWin = WM_H2P(hWin);
	Status = pWin->Status;
	if (State) {
		Status &= ~WM_SF_DISABLED;
	}
	else {
		Status |= WM_SF_DISABLED;
	}
	if (pWin->Status != Status) {
		WM_MESSAGE Msg;
		pWin->Status = Status;
		Msg.MsgId = WM_NOTIFY_ENABLE;
		Msg.Data.v = State;
		WM_SendMessage(hWin, &Msg);
	}

}

void WM_EnableWindow(WM_HWIN hWin) {
	WM_SetEnableState(hWin, 1);
}

void WM_DisableWindow(WM_HWIN hWin) {
	WM_SetEnableState(hWin, 0);
}

void WM_ForEachDesc(WM_HWIN hWin, WM_tfForEach *pcb, void *pData) {

	WM__ForEachDesc(hWin, pcb, pData);

}

/*********************************************************************
*
*       WM_GetBkColor
*
  Purpose:
	Return the clients background color.
	If a window does not define a background color, the default
	procedure returns GUI_INVALID_COLOR
*/
RGB_COLOR WM_GetBkColor(WM_HWIN hObj) {
	if (hObj) {
		WM_MESSAGE Msg;
		Msg.MsgId = WM_GET_BKCOLOR;
		WM_SendMessage(hObj, &Msg);
		return Msg.Data.Color;
	}
	return GUI_INVALID_COLOR;
}

/*********************************************************************
*
*       WM_GetClientRectEx
*
  Purpose:
	Return the client rectangle in client coordinates.
	This means for all windows that
	x0 = y0 = 0
	x1 = width - 1
	y1 = height - 1
*/
void WM_GetClientRectEx(WM_HWIN hWin, GUI_RECT *pRect) {
	WM_Obj *pWin;

	if (hWin) {
		if (pRect) {
			pWin = WM_H2P(hWin);
			WM__GetClientRectWin(pWin, pRect);
		}
	}

}

void WM_GetClientRect(GUI_RECT *pRect) {
	WM_HWIN hWin;

#if WM_SUPPORT_TRANSPARENCY
	hWin = WM__hATransWindow ? WM__hATransWindow : GUI_Context.hAWin;
#else
	hWin = GUI_Context.hAWin;
#endif
	WM_GetClientRectEx(hWin, pRect);

}

WM_HWIN WM_GetClientWindow(WM_HWIN hObj) {
	WM_MESSAGE Msg;
	Msg.Data.v = 0;
	Msg.MsgId = WM_GET_CLIENT_WINDOW;
	WM_SendMessage(hObj, &Msg);
	return (WM_HWIN)Msg.Data.v;

}

WM_HWIN WM_GetDesktopWindow(void) {
	return WM__ahDesktopWin;
}

int WM_GetNumWindows(void) {
	return WM__NumWindows;
}

int WM_GetNumInvalidWindows(void) {
	return WM__NumInvalidWindows;
}

/*******************************************************************
*
*       WM__GetDialogItem

  Recursively scan window and its child windows until dialog item is
  found or all children have been scanned.
*/
static WM_HWIN _GetDialogItem(WM_HWIN hWin, int Id) {
	WM_HWIN hi;
	WM_HWIN r = 0;
	WM_Obj *pWin = WM_H2P(hWin);
	hi = pWin->hFirstChild;
	while (hi) {
		/* This windows Id matching ? */
		if (WM_GetId(hi) == Id) {
			return hi;
		}
		/* Any child windows Id matching ? */
		if ((r = _GetDialogItem(hi, Id)) != 0) {
			break;
		}
		hi = WM_HANDLE2PTR(hi)->hNext;
	}
	return r;
}

WM_HWIN WM_GetDialogItem(WM_HWIN hWin, int Id) {
	WM_HWIN r = 0;
	if (hWin) {

		r = _GetDialogItem(hWin, Id);

	}
	return r;
}

WM_HWIN WM_GetFirstChild(WM_HWIN hWin) {
	if (hWin) {

		hWin = WM_H2P(hWin)->hFirstChild;

	}
	return hWin;
}

uint16_t WM_GetFlags(WM_HWIN hWin) {
	uint16_t r = 0;
	if (hWin) {

		r = WM_H2P(hWin)->Status;

	}
	return r;
}

WM_HWIN WM_GetFocussedWindow(void) {
	WM_HWIN r;

	r = WM__hWinFocus;

	return r;
}

int WM_GetId(WM_HWIN hObj) {
	WM_MESSAGE Msg;
	Msg.MsgId = WM_GET_ID;
	WM_SendMessage(hObj, &Msg);
	return Msg.Data.v;
}

/*********************************************************************
*
*       WM_GetInsideRectEx
*
  Purpose:
	Return the inside rectangle in client coordinates.
	The inside rectangle is the client rectangle minus the effect,
	which typically reduces the rectangle by 0 - 3 pixels on either side
	(2 for the standard 3D effect).
*/
void WM_GetInsideRectEx(WM_HWIN hWin, GUI_RECT *pRect) {
	WM_MESSAGE Msg;
	Msg.Data.p = pRect;
	Msg.MsgId = WM_GET_INSIDE_RECT;
	WM_SendMessage(hWin, &Msg);
}

void WM_GetInsideRect(GUI_RECT *pRect) {
	WM_GetInsideRectEx(GUI_Context.hAWin, pRect);
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
void WM_GetInsideRectExScrollbar(WM_HWIN hWin, GUI_RECT *pRect) {
	GUI_RECT rWin, rInside, rScrollbar;
	WM_HWIN hBarV, hBarH;
	uint16_t WinFlags;
	if (hWin) {
		if (pRect) {
			hBarH = WM_GetDialogItem(hWin, GUI_ID_HSCROLL);
			hBarV = WM_GetDialogItem(hWin, GUI_ID_VSCROLL);
			WM_GetWindowRectEx(hWin, &rWin);     /* The entire window in screen coordinates */
			WM_GetInsideRectEx(hWin, &rInside);
			if (hBarV) {
				WM_GetWindowRectEx(hBarV, &rScrollbar);
				GUI_MoveRect(&rScrollbar, -rWin.x0, -rWin.y0);
				WinFlags = WM_GetFlags(hBarV);
				if ((WinFlags & WM_SF_ANCHOR_RIGHT) && (WinFlags & WM_SF_ISVIS)) {
					rInside.x1 = rScrollbar.x0 - 1;
				}
			}
			if (hBarH) {
				WM_GetWindowRectEx(hBarH, &rScrollbar);
				GUI_MoveRect(&rScrollbar, -rWin.x0, -rWin.y0);
				WinFlags = WM_GetFlags(hBarH);
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
int WM_GetInvalidRect(WM_HWIN hWin, GUI_RECT *pRect) {
	int IsInvalid = 0;
	if (hWin) {
		WM_Obj *pWin;

		pWin = WM_HANDLE2PTR(hWin);
		if (pWin->Status & WM_SF_INVALID) {
			IsInvalid = 1;
			*pRect = pWin->InvalidRect;
		}

	}
	return IsInvalid;
}

WM_HWIN WM_GetNextSibling(WM_HWIN hWin) {

	if (hWin) {
		hWin = WM_H2P(hWin)->hNext;
	}

	return hWin;
}

int WM_GetWindowOrgX(WM_HWIN hWin) {
	int r = 0;
	if (hWin) {

		r = WM_HANDLE2PTR(hWin)->Rect.x0;

	}
	return r;
}

int WM_GetWindowOrgY(WM_HWIN hWin) {
	int r = 0;
	if (hWin) {

		r = WM_HANDLE2PTR(hWin)->Rect.y0;

	}
	return r;
}

int WM_GetOrgX(void) {
	return WM_GetWindowOrgX(GUI_Context.hAWin);
}

int WM_GetOrgY(void) {
	return WM_GetWindowOrgY(GUI_Context.hAWin);
}

WM_HWIN WM_GetParent(WM_HWIN hWin) {
	if (hWin) {

		hWin = WM_H2P(hWin)->hParent;

	}
	return hWin;
}

WM_HWIN WM_GetPrevSibling(WM_HWIN hWin) {
	WM_HWIN hPrev = 0;

	if (hWin) {
		hPrev = WM__GetPrevSibling(hWin);
	}

	return hPrev;
}

WM_HWIN WM_GetScrollbarH(WM_HWIN hWin) {
	return WM_GetDialogItem(hWin, GUI_ID_HSCROLL);
}

WM_HWIN WM_GetScrollbarV(WM_HWIN hWin) {
	return WM_GetDialogItem(hWin, GUI_ID_VSCROLL);
}

WM_HWIN WM_GetScrollPartner(WM_HWIN hScroll) {
	int Id = WM_GetId(hScroll);
	if (Id == GUI_ID_HSCROLL) {
		Id = GUI_ID_VSCROLL;
	}
	else if (Id == GUI_ID_VSCROLL) {
		Id = GUI_ID_HSCROLL;
	}
	return WM_GetDialogItem(WM_GetParent(hScroll), Id);

}

void WM_GetScrollState(WM_HWIN hObj, WM_SCROLL_STATE *pScrollState) {
	WM_MESSAGE Msg;
	Msg.MsgId = WM_GET_SCROLL_STATE;
	Msg.Data.p = pScrollState;
	WM_SendMessage(hObj, &Msg);
}

#define WM_DEBUG_LEVEL 1
/*********************************************************************
*
*       WM_GetWindowRect
*
* Returns the window rect in screen (desktop) coordinates.
*/
void WM_GetWindowRect(GUI_RECT *pRect) {
	WM_HWIN hWin;

	if (pRect) {
		WM_Obj *pWin;
#if WM_SUPPORT_TRANSPARENCY
		hWin = WM__hATransWindow ? WM__hATransWindow : GUI_Context.hAWin;
#else
		hWin = GUI_Context.hAWin;
#endif
		pWin = WM_HANDLE2PTR(hWin);
		*pRect = pWin->Rect;
	}

}

void WM_GetWindowRectEx(WM_HWIN hWin, GUI_RECT *pRect) {
	if (hWin && pRect) {
		WM_Obj *pWin;

		pWin = WM_HANDLE2PTR(hWin);
		if (pWin) {
			*pRect = pWin->Rect;
		}

	}
}

/*********************************************************************
*
*       _GetDefaultWin

  When drawing, we have to start at the bottom window !
*/
static WM_HWIN _GetDefaultWin(WM_HWIN hWin) {
	if (!hWin)
		hWin = WM_GetActiveWindow();
	return hWin;
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
int WM_GetWindowSizeX(WM_HWIN hWin) {
	int r;
	WM_Obj *pWin;

	hWin = _GetDefaultWin(hWin);
	pWin = WM_H2P(hWin);
	r = WM__GetWindowSizeX(pWin);

	return r;
}

/*********************************************************************
*
*       WM_GetWindowSizeY

  Return height of window in pixels
*/
int WM_GetWindowSizeY(WM_HWIN hWin) {
	int r;
	WM_Obj *pWin;

	hWin = _GetDefaultWin(hWin);
	pWin = WM_H2P(hWin);
	r = WM__GetWindowSizeY(pWin);

	return r;
}

int WM_HasCaptured(WM_HWIN hWin) {
	return hWin == WM__hCapture;
}

int WM_HasFocus(WM_HWIN hWin) {
	return hWin == WM__hWinFocus;
}

#define WM_DEBUG_LEVEL 1

void WM_HideWindow(WM_HWIN hWin) {
	if (hWin) {
		WM_Obj *pWin;

		pWin = WM_HANDLE2PTR(hWin);
		/* First check if this is necessary at all */
		if (pWin->Status & WM_SF_ISVIS) {
			/* Clear Visibility flag */
			pWin->Status &= ~WM_SF_ISVIS;
			/* Mark content as invalid */
			WM__InvalidateAreaBelow(&WM_HANDLE2PTR(hWin)->Rect, hWin);
#if WM_SUPPORT_NOTIFY_VIS_CHANGED
			WM__SendMsgNoData(hWin, WM_NOTIFY_VIS_CHANGED);             /* Notify window that visibility may have changed */
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

static char _WindowSiblingsOverlapRect(WM_HWIN iWin, GUI_RECT *pRect) {
	WM_Obj *pWin;
	for (; iWin; iWin = pWin->hNext) {
		int Status = (pWin = WM_H2P(iWin))->Status;
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
	WM_HMEM hParent;
	/* Step 1:
	 Check if there are any visible children. If this is so, then the
	 window has an overlap.
	 */
	 /* Check all children */
	if (_WindowSiblingsOverlapRect(pWin->hFirstChild, pRect)) {
		return 1;
	}

	/* STEP 2:
		 Find out the max. height (r.y1) if we are at the left border.
		 Since we are using the same height for all IVRs at the same y0,
		 we do this only for the leftmost one.
	*/

	/* Iterate over all windows which are above */
	/* Check all siblings above (Iterate over Parents and top siblings (hNext) */
	for (hParent = pWin->hParent; hParent; hParent = pParent->hParent) {
		pParent = WM_H2P(hParent);
		if (_WindowSiblingsOverlapRect(pParent->hNext, pRect)) {
			return 1;
		}
	}
	return 0;
}

static char _IsCompletelyVisible(WM_HWIN hWin) {
	WM_Obj *pWin;
	GUI_RECT Rect;

	pWin = WM_H2P(hWin);
	Rect = pWin->Rect;
	if (WM__ClipAtParentBorders(&Rect, hWin) == 0) {
		return 0;                 /* Nothing is left */
	}
	/* Check if the window is still the original one */
	if (_CompareRect(&Rect, &pWin->Rect)) {
		return 0;                 /* Not completely visible */
	}
	/* Now the difficult part ...
	   Find the rectangles.
	*/
	if (_HasOverlap(pWin, &Rect)) {
		return 0;
	}
	return 1;                   /* Is completely visible */
}

char WM_IsCompletelyVisible(WM_HWIN hWin) {
	int r = 0;
	if (hWin) {

		r = _IsCompletelyVisible(hWin);

	}
	return r;
}

int WM_IsEnabled(WM_HWIN hObj) {
	int r = 0;
	if (hObj) {

		r = WM__IsEnabled(hObj);

	}
	return r;
}

int WM_IsFocussable(WM_HWIN hWin) {
	int r = 0;
	if (hWin) {
		WM_MESSAGE Msg;
		Msg.Data.v = 0;
		Msg.MsgId = WM_GET_ACCEPT_FOCUS;
		WM_SendMessage(hWin, &Msg);
		r = Msg.Data.v;
	}
	return r;
}

int WM_IsVisible(WM_HWIN hWin) {
	int r = 0;
	if (hWin) {
		WM_Obj *pWin;

		pWin = WM_H2P(hWin);
		if (pWin->Status & WM_SF_ISVIS) {
			r = 1;
		}

	}
	return r;
}

int WM_IsWindow(WM_HWIN hWin) {
	int r;

	r = WM__IsWindow(hWin);

	return r;
}

/*********************************************************************
*
*       WM_MakeModal
*
* Purpose:
*   Makes the window modal.
*   We also need to send a message to the window which has received
*   the last "pressed" message
*
* Return value:
*/
void WM_MakeModal(WM_HWIN hWin) {

	WM__CHWinModal.hWin = hWin;
	/* Send a message to the window that it is no longer pressed (WM_TOUCH(0))
	   if it is outside the modal area, because otherwise it will not receive this message any more.
	*/
	if (WM__CHWinLast.hWin) {
		if (!WM__IsInModalArea(WM__CHWinLast.hWin)) {
			WM_MESSAGE Msg = { 0 };
			Msg.MsgId = WM_TOUCH;
			WM__SendPIDMessage(WM__CHWinLast.hWin, &Msg);
			WM__CHWinLast.hWin = 0;
		}
	}

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
static void _MoveDescendents(WM_HWIN hWin, int dx, int dy) {
	WM_Obj *pWin;

	for (; hWin; hWin = pWin->hNext) {
		pWin = WM_HANDLE2PTR(hWin);
		GUI_MoveRect(&pWin->Rect, dx, dy);
		GUI_MoveRect(&pWin->InvalidRect, dx, dy);
		_MoveDescendents(pWin->hFirstChild, dx, dy);  /* Children need to be moved along ...*/
		WM__SendMsgNoData(hWin, WM_MOVE);
	}
}

void WM__MoveWindow(WM_HWIN hWin, int dx, int dy) {
	GUI_RECT r;
	WM_Obj *pWin;
	if (hWin) {
		pWin = WM_HANDLE2PTR(hWin);
		r = pWin->Rect;
		GUI_MoveRect(&pWin->Rect, dx, dy);
		GUI_MoveRect(&pWin->InvalidRect, dx, dy);
		_MoveDescendents(pWin->hFirstChild, dx, dy);  /* Children need to be moved along ...*/
		/* Invalidate old and new area ... */
		if (pWin->Status & WM_SF_ISVIS) {
			WM_InvalidateArea(&pWin->Rect);     /* Invalidate new area */
			WM_InvalidateArea(&r);     /* Invalidate old area */
		}
		WM__SendMsgNoData(hWin, WM_MOVE);             /* Notify window it has been moved */
	}
}

void WM__MoveTo(WM_HWIN hWin, int x, int y) {
	if (hWin) {
		WM_Obj *pWin = WM_HANDLE2PTR(hWin);
		x -= pWin->Rect.x0;
		y -= pWin->Rect.y0;
		WM__MoveWindow(hWin, x, y);
	}
}

void WM_MoveWindow(WM_HWIN hWin, int dx, int dy) {
	{
		WM__MoveWindow(hWin, dx, dy);
	}
}

void WM_MoveTo(WM_HWIN hWin, int x, int y) {
	{
		WM__MoveTo(hWin, x, y);
	}
}

void WM_MoveChildTo(WM_HWIN hWin, int x, int y) {
	if (hWin) {
		WM_HWIN hParent;

		hParent = WM_GetParent(hWin);
		if (hParent) {
			WM_Obj *pParent, *pWin;
			pParent = WM_HANDLE2PTR(hParent);
			pWin = WM_HANDLE2PTR(hWin);
			x -= pWin->Rect.x0 - pParent->Rect.x0;
			y -= pWin->Rect.y0 - pParent->Rect.y0;
			WM__MoveWindow(hWin, x, y);
		}

	}
}

void WM_NotifyParent(WM_HWIN hWin, int Notification) {
	WM_MESSAGE Msg;
	Msg.MsgId = WM_NOTIFY_PARENT;
	Msg.Data.v = Notification;
	WM_SendToParent(hWin, &Msg);
}

void WM_Paint(WM_HWIN hWin) {
	GUI_CONTEXT Context;
	WM_PAINTINFO PaintInfo;
	WM_Obj *pWin;

	WM_ASSERT_NOT_IN_PAINT();
	if (hWin) {

		GUI_SaveContext(&Context);
		pWin = WM_H2P(hWin);
		WM_SelectWindow(hWin);
		WM_SetDefault();
		WM_InvalidateWindow(hWin);  /* Important ... Window procedure is informed about invalid rect and may optimize */
		/* Paint the window and its overlaying transparent windows */
		PaintInfo.hWin = hWin;
		PaintInfo.pWin = pWin;
		WM__PaintWinAndOverlays(&PaintInfo);
		WM_ValidateWindow(hWin);
		GUI_RestoreContext(&Context);

	}
}

void WM_PID__GetPrevState(GUI_PID_STATE *pPrevState) {
	*pPrevState = WM_PID__StateLast;
}

#define WM_DEBUG_LEVEL 1

void WM_ResizeWindow(WM_HWIN hWin, int dx, int dy) {
	GUI_RECT rOld, rNew, rMerge;
	WM_Obj *pWin;
	if (((dx | dy) == 0) || (hWin == 0)) { /* Early out if there is nothing to do */
		return;
	}

	pWin = WM_HANDLE2PTR(hWin);
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
	GUI_MergeRect(&rMerge, &rOld, &rNew);
	pWin->Rect = rNew;
	WM_InvalidateArea(&rMerge);
	WM__UpdateChildPositions(pWin, rNew.x0 - rOld.x0, rNew.y0 - rOld.y0, rNew.x1 - rOld.x1, rNew.y1 - rOld.y1);
	GUI__IntersectRect(&pWin->InvalidRect, &pWin->Rect); /* Make sure invalid area is not bigger than window itself */
	WM__SendMsgNoData(hWin, WM_SIZE);                    /* Send size message to the window */

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
static WM_HWIN _Screen2hWin(WM_HWIN hWin, WM_HWIN hStop, int x, int y) {
	WM_Obj *pWin = WM_HANDLE2PTR(hWin);
	WM_HWIN hChild;
	WM_HWIN hHit;
	/* First check if the  coordinates are in the given window. If not, return 0 */
	if (WM__IsInWindow(pWin, x, y) == 0) {
		return 0;
	}
	/* If the coordinates are in a child, search deeper ... */
	for (hChild = pWin->hFirstChild; hChild && (hChild != hStop); ) {
		WM_Obj *pChild = WM_HANDLE2PTR(hChild);
		if ((hHit = _Screen2hWin(hChild, hStop, x, y)) != 0) {
			hWin = hHit;        /* Found a window */
		}
		hChild = pChild->hNext;
	}
	return hWin;            /* No Child affected ... The parent is the right one */
}

int WM__IsInWindow(WM_Obj *pWin, int x, int y) {
	if ((pWin->Status & WM_SF_ISVIS)
		&& (x >= pWin->Rect.x0)
		&& (x <= pWin->Rect.x1)
		&& (y >= pWin->Rect.y0)
		&& (y <= pWin->Rect.y1)) {
		return 1;
	}
	return 0;
}

WM_HWIN WM_Screen2hWin(int x, int y) {
	WM_HWIN r;

	r = _Screen2hWin(WM__FirstWin, 0, x, y);

	return r;
}

WM_HWIN WM_Screen2hWinEx(WM_HWIN hStop, int x, int y) {
	WM_HWIN r;

	r = _Screen2hWin(WM__FirstWin, hStop, x, y);

	return r;
}

void WM_SendMessageNoPara(WM_HWIN hWin, int MsgId) {

	WM__SendMessageNoPara(hWin, MsgId);

}

void WM_SendToParent(WM_HWIN hChild, WM_MESSAGE *pMsg) {
	if (pMsg) {
		WM_HWIN hParent;

		hParent = WM_GetParent(hChild);
		if (hParent) {
			pMsg->hWinSrc = hChild;
			WM_SendMessage(hParent, pMsg);
		}

	}
}

void WM_SetAnchor(WM_HWIN hWin, uint16_t AnchorFlags) {
	if (hWin) {
		WM_Obj *pWin;
		uint16_t Mask;

		pWin = WM_H2P(hWin);
		Mask = (WM_SF_ANCHOR_LEFT | WM_SF_ANCHOR_RIGHT | WM_SF_ANCHOR_TOP | WM_SF_ANCHOR_BOTTOM);

		AnchorFlags &= Mask;

		pWin->Status &= ~(Mask);
		pWin->Status |= AnchorFlags;

	}
}

WM_CALLBACK *WM_SetCallback(WM_HWIN hWin, WM_CALLBACK *cb) {
	WM_CALLBACK *r = NULL;
	if (hWin) {
		WM_Obj *pWin;

		pWin = WM_H2P(hWin);
		r = pWin->cb;
		pWin->cb = cb;
		WM_InvalidateWindow(hWin);

	}
	return r;
}

static void WM__ReleaseCapture(void) {
	if (WM__hCapture) {
		WM_MESSAGE Msg;
		Msg.MsgId = WM_CAPTURE_RELEASED;
		WM_SendMessage(WM__hCapture, &Msg);
		WM__hCapture = 0;
	}
}

void WM_SetCapture(WM_HWIN hObj, int AutoRelease) {

	if (WM__hCapture != hObj) {
		WM__ReleaseCapture();
	}
	WM__hCapture = hObj;
	WM__CaptureReleaseAuto = AutoRelease;

}

void WM_ReleaseCapture(void) {

	WM__ReleaseCapture();

}

static GUI_POINT WM__CapturePoint;

void WM_SetCaptureMove(WM_HWIN hWin, const GUI_PID_STATE *pState, int MinVisibility) {
	if (!WM_HasCaptured(hWin)) {
		WM_SetCapture(hWin, 1);        /* Set capture with auto release */
		WM__CapturePoint.x = pState->x;
		WM__CapturePoint.y = pState->y;
	}
	else {                         /* Moving ... let the window move ! */
		int dx, dy;
		dx = pState->x - WM__CapturePoint.x;
		dy = pState->y - WM__CapturePoint.y;
		/* make sure at least a part of the windows stays inside of its parent */
		if (MinVisibility == 0) {
			WM_MoveWindow(hWin, dx, dy);
		}
		else {
			GUI_RECT Rect, RectParent;
			/* make sure at least a part of the windows stays inside of its parent */
			WM_GetWindowRectEx(hWin, &Rect);
			WM_GetWindowRectEx(WM_GetParent(hWin), &RectParent);
			GUI_MoveRect(&Rect, dx, dy);
			GUI__ReduceRect(&RectParent, &RectParent, MinVisibility);
			if (GUI_RectsIntersect(&Rect, &RectParent)) {
				WM_MoveWindow(hWin, dx, dy);
			}
		}
	}
}

uint16_t WM_SetCreateFlags(uint16_t Flags) {
	uint16_t r = WM__CreateFlags;
	WM__CreateFlags = Flags;
	return r;
}

RGB_COLOR WM_SetDesktopColor(RGB_COLOR Color) {
	RGB_COLOR r;
	r = WM__aBkColor;
	WM__aBkColor = Color;
	WM_InvalidateWindow(WM__ahDesktopWin);
	return r;
}

void WM_SetDesktopColors(RGB_COLOR Color) {
	WM_SetDesktopColor(Color);
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
int WM_SetFocus(WM_HWIN hWin) {
	int r;
	WM_MESSAGE Msg = { 0 };

	if ((hWin) && (hWin != WM__hWinFocus)) {
		WM_NOTIFY_CHILD_HAS_FOCUS_INFO Info;
		Info.hOld = WM__hWinFocus;
		Info.hNew = hWin;
		Msg.MsgId = WM_SET_FOCUS;
		/* Send a "no more focus" message to window losing focus */
		Msg.Data.v = 0;
		if (WM__hWinFocus) {
			WM_SendMessage(WM__hWinFocus, &Msg);
		}
		/* Send "You have the focus now" message to the window */
		Msg.Data.v = 1;
		WM_SendMessage(WM__hWinFocus = hWin, &Msg);
		if ((r = Msg.Data.v) == 0) { /* On success only */
			/* Set message to ancestors of window getting the focus */
			while ((hWin = WM_GetParent(hWin)) != 0) {
				Msg.MsgId = WM_NOTIFY_CHILD_HAS_FOCUS;
				Msg.Data.p = &Info;
				WM_SendMessage(hWin, &Msg);
			}
			/* Set message to ancestors of window loosing the focus */
			hWin = Info.hOld;
			if (WM_IsWindow(hWin)) {    /* Make sure window has not been deleted in the mean time. Can be optimized: _DeleteWindow could clear the handle to avoid this check (RS) */
				while ((hWin = WM_GetParent(hWin)) != 0) {
					Msg.MsgId = WM_NOTIFY_CHILD_HAS_FOCUS;
					Msg.Data.p = &Info;
					WM_SendMessage(hWin, &Msg);
				}
			}
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
static WM_HWIN _GetNextChild(WM_HWIN hParent, WM_HWIN hChild) {
	WM_HWIN hObj = 0;
	WM_Obj *pObj;
	if (hChild) {
		pObj = WM_HANDLE2PTR(hChild);
		hObj = pObj->hNext;
	}
	if (!hObj) {
		pObj = WM_HANDLE2PTR(hParent);
		hObj = pObj->hFirstChild;
	}
	if (hObj != hChild) {
		return hObj;
	}
	return 0;
}

/*********************************************************************
*
*       _SetFocusOnNextChild
*
* Purpose:
*   Sets the focus on next focussable child of a window.
*
* Return value:
*   Handle of focussed child, if we found an other focussable child
*   as the current. Otherwise the return value is zero.
*/
static WM_HWIN _SetFocusOnNextChild(WM_HWIN hParent) {
	WM_HWIN hChild, hWin;
	hChild = WM__GetFocussedChild(hParent);
	hChild = _GetNextChild(hParent, hChild);
	hWin = hChild;
	while ((WM_IsFocussable(hWin) == 0) && hWin) {
		hWin = _GetNextChild(hParent, hWin);
		if (hWin == hChild) {
			break;
		}
	}
	if (WM_SetFocus(hWin) == 0) {
		return hWin;
	}
	return 0;
}

WM_HWIN WM_SetFocusOnNextChild(WM_HWIN hParent) {
	WM_HWIN r = 0;
	if (hParent) {

		r = _SetFocusOnNextChild(hParent);

	}
	return r;
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
static WM_HWIN _GetPrevChild(WM_HWIN hChild) {
	WM_HWIN hObj = 0;
	if (hChild) {
		hObj = WM__GetPrevSibling(hChild);
	}
	if (!hObj) {
		hObj = WM__GetLastSibling(hChild);
	}
	if (hObj != hChild) {
		return hObj;
	}
	return 0;
}

/*********************************************************************
*
*       _SetFocusOnPrevChild
*
* Purpose:
*   Sets the focus on previous focussable child of a window.
*
* Return value:
*   Handle of focussed child, if we found an other focussable child
*   as the current. Otherwise the return value is zero.
*/
static WM_HWIN _SetFocusOnPrevChild(WM_HWIN hParent) {
	WM_HWIN hChild, hWin;
	hChild = WM__GetFocussedChild(hParent);
	hChild = _GetPrevChild(hChild);
	hWin = hChild;
	while ((WM_IsFocussable(hWin) == 0) && hWin) {
		hWin = _GetPrevChild(hWin);
		if (hWin == hChild) {
			break;
		}
	}
	if (WM_SetFocus(hWin) == 0) {
		return hWin;
	}
	return 0;
}

WM_HWIN WM_SetFocusOnPrevChild(WM_HWIN hParent) {
	WM_HWIN r = 0;
	if (hParent) {

		r = _SetFocusOnPrevChild(hParent);

	}
	return r;
}

void WM_SetId(WM_HWIN hObj, int Id) {
	WM_MESSAGE Msg;
	Msg.MsgId = WM_SET_ID;
	Msg.Data.v = Id;
	WM_SendMessage(hObj, &Msg);
}

WM_tfPollPID *WM_SetpfPollPID(WM_tfPollPID *pf) {
	WM_tfPollPID *r = WM_pfPollPID;
	WM_pfPollPID = pf;
	return r;
}

/*********************************************************************
*
*       _SetScrollbar
*
* Return value: 1 if scrollbar was visible, 0 if not
*/
static int _SetScrollbar(WM_HWIN hWin, int OnOff, int Id, int Flags) {
	WM_HWIN hBar;
	hBar = WM_GetDialogItem(hWin, Id);
	if (OnOff) {
		if (!hBar) {
			SCROLLBAR_CreateAttached(hWin, Flags);
		}
	}
	else {
		WM_HideWindow(hBar);
		if (hBar) {
			WM_DeleteWindow(hBar);
		}
	}
	return (hBar ? 1 : 0);
}

int WM__SetScrollbarV(WM_HWIN hWin, int OnOff) {
	return _SetScrollbar(hWin, OnOff, GUI_ID_VSCROLL, SCROLLBAR_CF_VERTICAL);
}

int WM__SetScrollbarH(WM_HWIN hWin, int OnOff) {
	return _SetScrollbar(hWin, OnOff, GUI_ID_HSCROLL, 0);
}

int WM_SetScrollbarH(WM_HWIN hWin, int OnOff) {
	int r;

	r = WM__SetScrollbarH(hWin, OnOff);

	return r;
}

int WM_SetScrollbarV(WM_HWIN hWin, int OnOff) {
	int r;

	r = WM__SetScrollbarV(hWin, OnOff);

	return r;
}

void WM_SetScrollState(WM_HWIN hWin, const WM_SCROLL_STATE *pState) {
	if (hWin && pState) {
		WM_MESSAGE Msg;
		Msg.MsgId = WM_SET_SCROLL_STATE;
		Msg.Data.p = (const void *)pState;
		WM_SendMessage(hWin, &Msg);
	}
}

#define WM_DEBUG_LEVEL 1

void WM_SetSize(WM_HWIN hWin, int xSize, int ySize) {
	WM_Obj *pWin;
	int dx, dy;
	if (hWin) {

		pWin = WM_H2P(hWin);
		dx = xSize - (pWin->Rect.x1 - pWin->Rect.x0 + 1);
		dy = ySize - (pWin->Rect.y1 - pWin->Rect.y0 + 1);
		WM_ResizeWindow(hWin, dx, dy);

	}
}

#if WM_SUPPORT_TRANSPARENCY   /* If 0, WM will not generate any code */

void WM_SetHasTrans(WM_HWIN hWin) {
	WM_Obj *pWin;

	if (hWin) {
		pWin = WM_H2P(hWin);
		/* First check if this is necessary at all */
		if ((pWin->Status & WM_SF_HASTRANS) == 0) {
			pWin->Status |= WM_SF_HASTRANS; /* Set Transparency flag */
			WM__TransWindowCnt++;          /* Increment counter for transparency windows */
			WM_InvalidateWindow(hWin);      /* Mark content as invalid */
		}
	}

}

void WM_ClrHasTrans(WM_HWIN hWin) {
	WM_Obj *pWin;

	if (hWin) {
		pWin = WM_H2P(hWin);
		/* First check if this is necessary at all */
		if (pWin->Status & WM_SF_HASTRANS) {
			pWin->Status &= ~WM_SF_HASTRANS;
			WM__TransWindowCnt--;            /* Decrement counter for transparency windows */
			WM_InvalidateWindow(hWin);        /* Mark content as invalid */
		}
	}

}

int WM_GetHasTrans(WM_HWIN hWin) {
	int r = 0;
	WM_Obj *pWin;

	if (hWin) {
		pWin = WM_H2P(hWin);
		r = pWin->Status & WM_SF_HASTRANS;
	}

	return r;
}

#endif /*WM_SUPPORT_TRANSPARENCY*/

#if WM_SUPPORT_TRANSPARENCY

void WM_SetTransState(WM_HWIN hWin, unsigned State) {
	WM_Obj *pWin;

	if (hWin) {
		pWin = WM_H2P(hWin);
		if (State & WM_CF_HASTRANS) {
			WM_SetHasTrans(hWin);
		}
		else {
			WM_ClrHasTrans(hWin);
		}
		if (State & WM_CF_CONST_OUTLINE) {
			if (!(pWin->Status & WM_CF_CONST_OUTLINE)) {
				pWin->Status |= WM_CF_CONST_OUTLINE;
				WM_InvalidateWindow(hWin);
			}
		}
		else {
			if (pWin->Status & WM_CF_CONST_OUTLINE) {
				pWin->Status &= ~WM_CF_CONST_OUTLINE;
				WM_InvalidateWindow(hWin);
			}
		}
	}

}

#else
void WM_SetTransState_c(void);
void WM_SetTransState_c(void) {} /* avoid empty object files */
#endif /* WM_SUPPORT_TRANSPARENCY */

const GUI_RECT *WM_SetUserClipRect(const GUI_RECT *pRect) {
	const GUI_RECT *pRectReturn;

	pRectReturn = GUI_Context.WM__pUserClipRect;
	GUI_Context.WM__pUserClipRect = pRect;
	/* Activate it ... */
	WM__ActivateClipRect();

	return pRectReturn;
}

int WM_SetXSize(WM_HWIN hWin, int XSize) {
	WM_Obj *pWin;
	int dx;
	int r = 0;
	if (hWin) {

		pWin = WM_H2P(hWin);
		dx = XSize - (pWin->Rect.x1 - pWin->Rect.x0 + 1);
		WM_ResizeWindow(hWin, dx, 0);
		r = pWin->Rect.x1 - pWin->Rect.x0 + 1;

	}
	return r;
}

int WM_SetYSize(WM_HWIN hWin, int YSize) {
	WM_Obj *pWin;
	int dy;
	int r = 0;
	if (hWin) {

		pWin = WM_H2P(hWin);
		dy = YSize - (pWin->Rect.y1 - pWin->Rect.y0 + 1);
		WM_ResizeWindow(hWin, 0, dy);
		r = pWin->Rect.y1 - pWin->Rect.y0 + 1;

	}
	return r;
}

#define WM_DEBUG_LEVEL 1
/*********************************************************************
*
*       WM_InvalidateWindowDescs

  Invalidate window and all descendents (children and grandchildren and ...
*/
void WM_InvalidateWindowDescs(WM_HWIN hWin) {
	WM_HWIN hChild;
	if (hWin) {
		WM_InvalidateWindow(hWin);    /* Invalidate window itself */
		for (hChild = WM_GetFirstChild(hWin); hChild;) {
			WM_Obj *pChild = WM_H2P(hChild);
			WM_InvalidateWindowDescs(hChild);
			hChild = pChild->hNext;
		}
	}
}

void WM_ShowWindow(WM_HWIN hWin) {
	if (hWin) {
		WM_Obj *pWin;

		pWin = WM_H2P(hWin);
		if ((pWin->Status & WM_SF_ISVIS) == 0) {  /* First check if this is necessary at all */
			pWin->Status |= WM_SF_ISVIS;  /* Set Visibility flag */
			WM_InvalidateWindowDescs(hWin);    /* Mark content as invalid */
#if WM_SUPPORT_NOTIFY_VIS_CHANGED
			WM__NotifyVisChanged(hWin, &pWin->Rect);
#endif
		}

	}
}

void WM_SetStayOnTop(WM_HWIN hWin, int OnOff) {
	WM_Obj *pWin;
	if (hWin) {
		uint16_t OldStatus;

		pWin = WM_H2P(hWin);
		OldStatus = pWin->Status;
		if (OnOff) {
			if ((pWin->Status & WM_SF_STAYONTOP) == 0) {  /* First check if this is necessary at all */
				pWin->Status |= WM_SF_STAYONTOP;
			}
		}
		else {
			if ((pWin->Status & WM_SF_STAYONTOP) != 0) {  /* First check if this is necessary at all */
				pWin->Status &= ~WM_SF_STAYONTOP;
			}
		}
		if (pWin->Status != OldStatus) {
			WM_AttachWindow(hWin, WM_GetParent(hWin));
		}

	}
}

int WM_GetStayOnTop(WM_HWIN hWin) {
	int Result = 0;
	WM_Obj *pWin;
	if (hWin) {

		pWin = WM_H2P(hWin);
		if ((pWin->Status & WM_SF_STAYONTOP) != 0) {
			Result = 1;
		}

	}
	return Result;
}

static int Min(int v0, int v1) {
	return (v0 < v1) ? v0 : v1;
}
static int Max(int v0, int v1) {
	return (v0 > v1) ? v0 : v1;
}

/*********************************************************************
*
*       WM__SubRect

  The result is the smallest rectangle which includes the entire
  remaining area.

  *pDest = *pr0- *pr1;
*/
static void _SubRect(GUI_RECT *pDest, const GUI_RECT *pr0, const GUI_RECT *pr1) {
	if ((pDest == NULL) || (pr0 == NULL))
		return;
	*pDest = *pr0;
	if (pr1 == NULL)
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
void WM_ValidateRect(WM_HWIN hWin, const GUI_RECT *pRect) {
	WM_Obj *pWin;
	if (hWin) {

		pWin = WM_HANDLE2PTR(hWin);
		if (pWin->Status & WM_SF_INVALID) {
			if (pRect) {
				_SubRect(&pWin->InvalidRect, &pWin->InvalidRect, pRect);
				if (WM__RectIsNZ(&pWin->InvalidRect))
					return;
			}
			pWin->Status &= ~WM_SF_INVALID;
			WM__NumInvalidWindows--;
		}

	}
}

void WM_ValidateWindow(WM_HWIN hWin) {
	WM_Obj *pWin;

	if (hWin) {
		pWin = WM_HANDLE2PTR(hWin);
		if (pWin->Status & WM_SF_INVALID) {
			pWin->Status &= ~WM_SF_INVALID;
			WM__NumInvalidWindows--;
		}
	}

}

int WM_OnKey(int Key, int Pressed) {
	int r = 0;
	WM_MESSAGE Msg;
	if (WM__hWinFocus != 0) {
		WM_KEY_INFO Info;
		Info.Key = Key;
		Info.PressedCnt = Pressed;
		Msg.MsgId = WM_KEY;
		Msg.Data.p = &Info;
		WM__SendMessage(WM__hWinFocus, &Msg);
		r = 1;
	}
	return r;
}

