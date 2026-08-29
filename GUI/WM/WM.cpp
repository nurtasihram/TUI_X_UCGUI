#include "WM_Intern.h"
#include "GUI_Protected.h"
#include "GUIDebug.h"

static bool IsActive;

static WObj *pWinNextDraw;

#pragma region Mouse/Touch
static void _SendMessageIfEnabled(WObj *pWin, int MsgId, WM_PARAM Data) {
	if (pWin->IsEnabled())
		WM__SendMessage(pWin, MsgId, Data);
}

static void _SendTouchMessage(WObj *pWin, int MsgId, PID_STATE *pState) {
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

PID_STATE WM_PID__StateLast{ 0 };
PID_STATE WM_PID__GetPrevState() {
	return WM_PID__StateLast;
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
	auto StateNew = GUI_PID_GetState();
	if (WM_PID__StateLast == StateNew) return false;
	bool r = false;
#if GUI_SUPPORT_CURSOR
	GUI_CURSOR_SetPosition(StateNew.x, StateNew.y);
#endif
	WObj::CriticalHandle CHWin = WObj::pWinCapture ? WObj::pWinCapture : WM_Screen2Win(StateNew);
	CHWin.Add();
	/* Send WM_PID_STATE_CHANGED message if state has changed (just pressed or just released) */
	if (WM_PID__StateLast.Pressed != StateNew.Pressed && CHWin.pWin) {
		PID_CHANGED_INFO Info;
		auto pWin = CHWin.pWin;
		Info.State = StateNew.Pressed;
		Info.StatePrev = WM_PID__StateLast.Pressed;
		Info.x = StateNew.x - pWin->Rect.x0;
		Info.y = StateNew.y - pWin->Rect.y0;
		_SendMessageIfEnabled((WObj *)CHWin.pWin, WM_PID_STATE_CHANGED, (WM_PARAM)&Info);
	}
	/* Send WM_TOUCH message(s) Note that we may have to send 2 touch messages. */
	if (WM_PID__StateLast.Pressed | StateNew.Pressed) { /* Only if pressed or just released */
		r = 1;
		/* Tell window if it is no longer pressed
		* This happens for 2 possible reasons:
		* a) PID is released
		* b) PID is moved out
		*/
		if (WObj::CHWinLast.pWin != CHWin.pWin) {
			if (WObj::CHWinLast.pWin) {
				GUI_DEBUG_LOG("\nSending WM_TOUCH to LastWindow %d (out of area)", WObj::CHWinLast.pWin);
				PID_STATE *pState = StateNew.Pressed ? nullptr : &WM_PID__StateLast;
				_SendTouchMessage((WObj *)WObj::CHWinLast.pWin, WM_TOUCH, pState);
				WObj::CHWinLast.pWin = nullptr;
			}
		}
		/* Sending WM_TOUCH to current window */
		if (CHWin.pWin) {
			/* Remember window */
			if (StateNew.Pressed)
				WObj::CHWinLast.pWin = CHWin.pWin;
			else {
				/* Handle automatic capture release */
				if (WObj::WM__CaptureReleaseAuto)
					WObj::ReleaseCapture();
				WObj::CHWinLast.pWin = nullptr;
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
	CHWin.Remove();
	/* Store the new state */
	WM_PID__StateLast = GUI_PID_GetState();
	return r;
}
#pragma endregion

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
	if (WObj::pWinCapture == pWin)
		WObj::ReleaseCapture(); /* Make sure the window does not have capture */
	/* check if critical handles are affected. If so, reset the window handle to 0 */
	WObj::CriticalHandle::Check(pWin);
	pWin->_RemoveFromLinList();
	/* Delete all children */
	_DeleteAllChildren(pWin->pFirstChild);
	/* Send WM_DELETE message to window in order to inform window itself */
	pWin->Require(WM_DELETE);     /* tell window about it */
	/* Remove window from window stack */
	pWin->_RemoveWindowFromList();
	pWin->NotifyParent(WM_NOTIFICATION_CHILD_DELETED);
	pWin->pParent = nullptr;
	/* Make sure window is no longer counted as invalid */
	if (pWin->Status & WC_ACTIVATE)
		WObj::NumInvalidWindows--;
	WObj::InvalidateArea(pWin->Rect);
	/* Free window memory */
	WObj::NumWindows--;
	GUI_ALLOC_Free(pWin);
	/* Select a valid window */
	WObj::pWinFirst->Select();
}

#pragma region IVR
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
	auto pAWin = WObj::pWinActive;
	if (r.x0 == _ClipContext.ClientRect.x0) {
		r.y1 = _ClipContext.ClientRect.y1;
		r.x1 = _ClipContext.ClientRect.x1;
		/* Iterate over all windows which are above */
		/* Check all siblings above (Iterate over Parents and top siblings (hNext) */
		for (auto pParent = WObj::pWinActive; pParent; pParent = pParent->pParent)
			pParent->pNext->_Findy1(r);
		/* Check all children */
		pAWin->pFirstChild->_Findy1(r);
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
	for (auto pParent = WObj::pWinActive; pParent; pParent = pParent->pParent)
		if (pParent->pNext->_Findx0(r))
			goto Find_x0;
	/* Check all children */
	if (pAWin->pFirstChild->_Findx0(r))
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
	for (auto pParent = WObj::pWinActive; pParent; pParent = pParent->pParent)
		pParent->pNext->_Findx1(r);
	/* Check all children */
	pAWin->pFirstChild->_Findx1(r);
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
	if (WObj::_PaintCallbackCnt)
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
void WM__ActivateClipRect(void) {
	if (IsActive)
		_SetClipRectUserIntersect(&_ClipContext.CurRect);
	else {
		/* Window manager disabled, typically because meory device is active */
		/* Take UserClipRect into account */
		_SetClipRectUserIntersect(&WObj::pWinActive->Rect);
	}
}
#pragma endregion

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
	auto pWin = WObj::pWinActive;
	auto Rect = pWin->InvalidRect;
	pWin->InvalidRect = GUI.ClipRect;
	pWin->_Paint1();
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
			pWin->Select();
#if GUI_SUPPORT_MEMDEV
			if (pWin->Status & WC_MEMDEV) {
				auto r = pWin->InvalidRect;
				auto Flags = GUI_MEMDEV_NOTRANS;
				/*
					* Currently we treat a desktop window as transparent, because per default it does not repaint itself.
					*/
				if (!pWin->pParent)
					Flags = GUI_MEMDEV_HASTRANS;
				GUI_MEMDEV_Draw(&r, _cbPaintMemDev, pWin, 0, Flags);
			}
			else
#endif
			pWin->_Paint1();
			Ret = true;    /* Something has been done */
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
	GUI_CONTEXT ContextOld;
	GUI_SaveContext(&ContextOld);
	auto iWin = pWinNextDraw ? pWinNextDraw : WObj::pWinFirst; 
	/* Make sure the next window to redraw is valid */
	for (; iWin; iWin = iWin->pNextLin)
		if (_Paint(iWin))
			break;
	pWinNextDraw = iWin;   /* Remember the window */
	GUI_RestoreContext(&ContextOld);
}

bool WM_Exec1(void) {
	/* Poll PID if necessary */
	if (WM_HandlePID())
		return true; /* We have done something ... */
	if (GUI_PollKeyMsg())
		return true; /* We have done something ... */
	if (IsActive && WObj::NumInvalidWindows) {
		_DrawNext();
		return true; /* We have done something ... */
	}
	return false; /* There was nothing to do ... */
}
bool WM_Exec(void) {
	bool r = false;
	while (WM_Exec1())
		r = true; /* We have done something */
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

RECT WM_GetClientRect() {
	return WObj::pWinActive->GetClientRect();
}
RECT WM_GetInsideRect() {
	return WObj::pWinActive->GetInsideRect();
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

const RECT *WM_SetUserClipRect(const RECT *pRect) {
	auto pRectReturn = GUI.WM__pUserClipRect;
	GUI.WM__pUserClipRect = pRect;
	/* Activate it ... */
	WM__ActivateClipRect();
	return pRectReturn;
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
	WObj::CHWinLast.Add();
	WObj::pWinDesktop->Select();
	WM_Activate();
	_IsInited = true;
}
