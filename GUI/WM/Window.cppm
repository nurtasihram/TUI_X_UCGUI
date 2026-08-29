module;

#include "GUI.h"
#include "GUI_Protected.h"
#include "GUIDebug.h"

#if GUI_DEBUG_LEVEL  >= GUI_DEBUG_LEVEL_LOG_WARNINGS
#define WM_ASSERT_NOT_IN_PAINT() { if (WObj::_PaintCallbackCnt) \
									   GUI_DEBUG_ERROROUT("Function may not be called from within a paint event"); \
								   }
#else
#define WM_ASSERT_NOT_IN_PAINT()
#endif

export module TUX.Window;

export import TUX;
export import TUX.Types;
export import TUX.WindowTypes;

export {
	
typedef WM_PARAM WM_CALLBACK(struct WObj *pWin, int MsgId, WM_PARAM Data);

using WM_HMEM = GUI_HMEM;

struct WObj;

struct NOTIFY_INFO {
	int Notification;
	WObj *pWinSrc;
};

struct NOTIFY_CHILD_HAS_FOCUS_INFO {
	WObj *pOld, *pNew;
};

typedef void WM_tfForEach(WObj *pWin, void *pData);

void WM_Init(void);
bool WM_Exec(void);  /* Execute all jobs ... Return 0 if nothing was done. */
bool WM_Exec1(void); /* Execute one job  ... Return 0 if nothing was done. */

/* Move/resize windows */
int  WM_CreateTimer(WObj *pWin, int UserID, int Period, int Mode); /* not to be documented (may change in future version) */
void WM_DeleteTimer(WObj *pWin, int UserId); /* not to be documented (may change in future version) */

/* Get size/origin of a window */
RECT WM_GetClientRect();
RECT WM_GetInsideRect();

/* Use of memory devices */
void WM_EnableMemdev(WObj *pWin);
void WM_DisableMemdev(WObj *pWin);

int WM_OnKey(int Key, int Pressed);

/******************************************************************
*
*           Message related funcions
*
*******************************************************************
	Please note that some of these functions do not yet show up in the
	documentation, as they should not be required by application program.
*/

WM_PARAM  WM_DefaultProc(WObj *pWin, int MsgId, WM_PARAM Data);

/* Scroll functions */
void WM_GetInsideRectExScrollbar(WObj *pWin, RECT *pRect); /* not to be documented (may change in future version) */
WObj *WM_GetScrollPartner(WObj *pWin);
bool WM_SetScrollbarH(WObj *pWin, int OnOff); /* not to be documented (may change in future version) */
bool WM_SetScrollbarV(WObj *pWin, int OnOff); /* not to be documented (may change in future version) */
void WM_GetScrollState(WObj *pObj, WM_SCROLL_STATE *pScrollState);

bool WM_HandlePID(void);
void WM_ForEachChild(WObj *pWin, WM_tfForEach *pcb, void *pData);

struct WObj {
	RECT Rect, InvalidRect;
	WObj *pNextLin = nullptr, *pNext = nullptr,
		*pParent = nullptr, *pFirstChild = nullptr;
	WM_CALLBACK *cb = nullptr; /* ptr to notification callback */
	uint16_t Status = 0; /* Some status flags */

#pragma region Window list
	static uint16_t NumWindows;
	static WObj *pWinFirst;
	void _RemoveFromLinList() {
		for (auto pCur = pWinFirst; pCur; ) {
			auto pNext = pCur->pNextLin;
			if (pNext == this) {
				pCur->pNextLin = pNextLin;
				break;
			}
			pCur = pNext;
		}
	}
	void _AddToLinList() {
		if (!pWinFirst) {
			pWinFirst = this;
			return;
		}
		auto pFirst = pWinFirst;
		pNextLin = pFirst->pNextLin;
		pFirst->pNextLin = this;
	}
public:
	bool IsWindow() const {
		for (auto i = pWinFirst; i; i = i->pNextLin)
			if (i == this)
				return true;
		return false;
	}
	static auto GetNumWindows() { return NumWindows; }
	static bool IsWindow(WObj *pWin) { return pWin ? pWin->IsWindow() : false; }
#pragma endregion

#pragma region Parent list
// private:
	void _InsertWindowIntoList(WObj *pNewParent) {
		if (!pNewParent)
			return;
		bool isStayOnTop = Status & WC_STAYONTOP;
		auto pCurrent = pNewParent->pFirstChild;
		pNext = nullptr;
		pParent = pNewParent;
		if (!pCurrent) {
			pNewParent->pFirstChild = this;
			return;
		}
		/* Insert before the first STAYONTOP child when the new window is not STAYONTOP. */
		if (!isStayOnTop && (pCurrent->Status & WC_STAYONTOP)) {
			pNext = pCurrent;
			pNewParent->pFirstChild = this;
			return;
		}
		/* Walk the sibling list until the correct insertion point is found. */
		for (;;) {
			auto pNext = pCurrent->pNext;
			if (!pNext) {
				pCurrent->pNext = this;
				return;
			}
			if (!isStayOnTop && (pNext->Status & WC_STAYONTOP)) {
				pCurrent->pNext = this;
				this->pNext = pNext;
				return;
			}
			pCurrent = pNext;
		}
	}
	void _RemoveWindowFromList() {
		if (!pParent)
			return;
		auto pChild = pParent->pFirstChild;
		if (pChild == this) {
			pParent->pFirstChild = pChild->pNext;
			return;
		}
		while (pChild) {
			auto pNext = pChild->pNext;
			if (pNext == this) {
				pChild->pNext = this->pNext;
				return;
			}
			pChild = pNext;
		}
	}

//protected:
	void _Detach() {
		_RemoveWindowFromList();
		/* Clear area used by this window */
		InvalidateArea(Rect);
	}
public:
	void Detach() {
		POINT org;
		if (pParent)
			org = -pParent->Rect.LeftTop();
		_Detach();
		Move(org); /* Convert screen coordinates -> parent coordinates */
		/* ToDo: Invalidate. If Parent window is located at (0,0). */
		pParent = nullptr;
	}
	void Attach(WObj *pParent, POINT Pos = {}) {
		Detach();
		if (pParent && pParent != this) {
			_InsertWindowIntoList(pParent);
			MoveTo(pParent->Rect.LeftTop() + Pos); /* Convert parent coordinates -> screen coordinates */
		}
	}

#pragma endregion
	static WObj *pWinActive;
	static bool IsActive;
public:
	static auto ActiveWindow() { return pWinActive; }
	void Select() {
		WM_ASSERT_NOT_IN_PAINT();
		WObj::pWinActive = this;
		LCD_SetClipRectMax();
		GUI.Off = Rect.LeftTop();
	}
	static void Activate() { IsActive = true; }
	static void Deactivate() {
		IsActive = false; /* No clipping performed by WM */
		LCD_SetClipRectMax();
	}

#pragma region Invalidation
	static uint16_t NumInvalidWindows;
//private:
	bool _ClipAtParentBorders(RECT &r) const {
		for (auto pWin = this; pWin->Status & WC_VISIBLE; pWin = pWin->pParent) {
			r &= pWin->Rect;
			if (!pWin->pParent)
				return pWin == pWinDesktop;
		}
		return false;
	}
	void _Invalidate1Abs(RECT r) {
		if (!(Status & WC_VISIBLE))
			return; /* Window is not visible... we are done */
		/* Calc affected area */
		if (r &= Rect) {
			if (Status & WC_ACTIVATE)
				InvalidRect |= r;
			else {
				InvalidRect = r;
				Status |= WC_ACTIVATE;
				NumInvalidWindows++;
			}
		}
	}
public:
	static uint16_t GetNumInvalidWindows() { return NumInvalidWindows; }
	static void InvalidateArea(const RECT &r) {
		for (auto pWin = pWinFirst; pWin; pWin = pWin->pNextLin)
			pWin->_Invalidate1Abs(r);
	}
	void Invalidate(const RECT *pRect = nullptr) {
		if (!(Status & WC_VISIBLE))
			return;
		auto r = Rect;
		if (pRect)
			r &= *pRect + GetOrg();
		/* Optimization that saves invalidation if window area is not visible ... Not required */
		if (!_ClipAtParentBorders(r))
			return;
		_Invalidate1Abs(r);
	}
	void InvalidateDescs() {
		Invalidate();    /* Invalidate window itself */
		for (auto pChild = FirstChild(); pChild;) {
			auto pNextChild = pChild->pNext;
			pChild->InvalidateDescs();
			pChild = pNextChild;
		}
	}
	void Validate() {
		if (Status & WC_ACTIVATE) {
			Status &= ~WC_ACTIVATE;
			NumInvalidWindows--;
		}
	}
#pragma endregion

#pragma region IVR
private:
	void _Findy1(RECT &r) const {
		for (auto pWin = this; pWin; pWin = pWin->pNext) {
			auto Status = pWin->Status;
			/* Check if this window affects us at all */
			if (!(Status & WC_VISIBLE))
				continue;
			auto rWinClipped = pWin->Rect; /* Window rect, clipped to part inside of ancestors */
			/* Check if this window affects us at all */
			if (!(rWinClipped <= r))
				continue;
			if (pWin->Rect.y0 > r.y0) {
				if (r.y1 > rWinClipped.y0 - 1) /* Check upper border of window */
					r.y1 = rWinClipped.y0 - 1;
			}
			else if (r.y1 > rWinClipped.y1) /* Check lower border of window */
				r.y1 = rWinClipped.y1;
		}
	}
	bool _Findx0(RECT &r) const {
		for (auto pWin = this; pWin; pWin = pWin->pNext) {
			auto Status = pWin->Status;
			if (!(Status & WC_VISIBLE))
				continue;
			/* If window is not visible, it can be safely ignored */
			auto rWinClipped = pWin->Rect; /* Window rect, clipped to part inside of ancestors */
			/* Check if this window affects us at all */
			if (rWinClipped <= r) {
				r.x0 = rWinClipped.x1 + 1;
				return true;
			}
		}
		return false;
	}
	void _Findx1(RECT &r) const {
		for (auto pWin = this; pWin; pWin = pWin->pNext) {
			auto Status = pWin->Status;
			if (!(Status & WC_VISIBLE))
				continue;
			/* If window is not visible, it can be safely ignored */
			auto rWinClipped = pWin->Rect; /* Window rect, clipped to part inside of ancestors */
			/* Check if this window affects us at all */
			if (!(rWinClipped <= r))
				continue;
			r.x1 = rWinClipped.x0 - 1;
		}
	}
private:

	struct {
		RECT ClientRect, CurRect;
		int Cnt = -1, EntranceCnt = 0;
	} static _ClipContext;

	static void _ActivateClipRect() {
		/* Window manager disabled, typically because memory device is active */
		/* Take UserClipRect into account */
		const RECT *prSrc = WObj::IsActive ? &_ClipContext.CurRect : &WObj::pWinActive->Rect;
		if (GUI.WM__pUserClipRect) {
			auto r = *GUI.WM__pUserClipRect;
			if (WObj::pWinActive)
				r += WObj::pWinActive->GetOrg(); /* Convert User ClipRect into screen coordinates */
			/* Set intersection as clip rect */
			r &= *prSrc;
			LCD_SetClipRectEx(&r);
		}
		else
			LCD_SetClipRectEx(prSrc);
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
	static bool _GetNextIVR(void) {
#if GUI_SUPPORT_CURSOR
		static char _CursorHidden;
#endif
	/* If WM is not active, we have no rectangles to return */
		if (!WObj::IsActive)
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
		_ActivateClipRect();
		/* Hide cursor if necessary */
#if GUI_SUPPORT_CURSOR
		_CursorHidden = GUI_CURSOR__TempHide(_ClipContext.CurRect);
#endif
		return true;
	}
	static bool _InitIVRSearch(RECT rcMax) {
		/* If WM is not active -> nothing to do, leave cliprect alone */
		if (!WObj::IsActive) {
			_ActivateClipRect();
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
		return _GetNextIVR();
	}
public:
	static const RECT *SetUserClipRect(const RECT *pRect) {
		auto pRectReturn = GUI.WM__pUserClipRect;
		GUI.WM__pUserClipRect = pRect;
		/* Activate it ... */
		_ActivateClipRect();
		return pRectReturn;
	}
	static inline void Iterate(RECT &r, auto fn) {
		if (_InitIVRSearch(r))
			do { fn(); } while (_GetNextIVR());

	}
#pragma endregion

#pragma region Paint & Draw
	static uint8_t _PaintCallbackCnt;      /* Public for assertions only */
	void _Paint1() /* const */ {
		/* Send WM_PAINT if window is visible and a callback is defined */
		if (cb && (Status & WC_VISIBLE)) {
			_PaintCallbackCnt++;
			if (Status & WC_LATE_CLIP)
				Require(WM_PAINT, (WM_PARAM)&InvalidRect);
			else
				Iterate(InvalidRect, [&] {
					Require(WM_PAINT, (WM_PARAM)&InvalidRect);
				});
			_PaintCallbackCnt--;
		}
	}
	bool _Paint() {
		if (!(Status & WC_ACTIVATE))
			return false;
		bool Ret = false;
		if (cb) {
			if (_ClipAtParentBorders(InvalidRect)) {
				Select();
#if GUI_SUPPORT_MEMDEV
				if (Status & WC_MEMDEV) {
					auto r = InvalidRect;
					auto Flags = GUI_MEMDEV_NOTRANS;
					/*
						* Currently we treat a desktop window as transparent, because per default it does not repaint itself.
						*/
					if (!pParent)
						Flags = GUI_MEMDEV_HASTRANS;
					GUI_MEMDEV_Draw(&r, [](void *p) {
						auto pWin = WObj::pWinActive;
						auto Rect = pWin->InvalidRect;
						pWin->InvalidRect = GUI.ClipRect;
						pWin->_Paint1();
						pWin->InvalidRect = Rect;
					}, this, 0, Flags);
				}
				else
#endif
					_Paint1();
				Ret = true;    /* Something has been done */
			}
		}
		/* We purposly clear the invalid flag after painting so we can still query the invalid rectangle while painting */
		Status &= ~WC_ACTIVATE; /* Clear invalid flag */
		if (Status & WC_MEMDEV_ON_REDRAW)
			Status |= WC_MEMDEV;
		--NumInvalidWindows;
		return Ret;
	}
public:
//private:
	static WObj *pWinNextDraw;
public:
	static bool DrawOnce() {
		if (!IsActive || !NumInvalidWindows)
			return false;
		GUI_CONTEXT ContextOld;
		GUI_SaveContext(&ContextOld);
		auto iWin = pWinNextDraw ? pWinNextDraw : WObj::pWinFirst;
		/* Make sure the next window to redraw is valid */
		for (; iWin; iWin = iWin->pNextLin)
			if (iWin->_Paint())
				break;
		pWinNextDraw = iWin;   /* Remember the window */
		GUI_RestoreContext(&ContextOld);
		return true;
	}

#pragma endregion

#pragma region CriticalHandles
//private:
	struct CriticalHandle {
		static CriticalHandle *pFirst;
		CriticalHandle *pNext = nullptr;
		WObj *pWin;
		CriticalHandle(WObj *pWin = nullptr) : pWin(pWin) {}
		static void Check(WObj *pWin) {
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
	static CriticalHandle CHWinLast;
#pragma endregion

public:
	void *operator new(size_t size) {
		return GUI_ALLOC_AllocNoInit(size);
	}
	void operator delete(void *p) {
		GUI_ALLOC_Free(p);
	}

public:
	WObj(RECT r, WM_CF Style, WM_CALLBACK *cb, WObj *pParent = nullptr) :
		Rect(r), cb(cb), Status(Style & WM_CF_MASK) {
		WM_ASSERT_NOT_IN_PAINT();
		/* Default parent is Desktop 0 */
		if (!pParent)
			if (NumWindows)
				pParent = pWinDesktop;
		if (pParent) {
			Rect += pParent->Rect.LeftTop();
			if (!r.XSize())
				Rect.x1 = pParent->Rect.x1;
			if (!r.YSize())
				Rect.y1 = pParent->Rect.y1;
		}
		NumWindows++;
		/* Add to linked lists */
		_AddToLinList();
		_InsertWindowIntoList(pParent);
		/* Activate window if WC_ACTIVATE is specified */
		if (Style & WC_ACTIVATE)
			Select();  /* This is not needed if callbacks are being used, but it does not cost a lot and makes life easier ... */
		/* Handle the Style flags, one at a time */
		if (Style & WC_BGND)
			BringToBottom();
		if (Style & WC_VISIBLE)
			Invalidate();    /* Mark content as invalid */
		Require(WM_CREATE);
	}
	~WObj() {
		WM_ASSERT_NOT_IN_PAINT();
		if (!IsWindow(this))
			return;
		pWinNextDraw = nullptr; /* Make sure the window will no longer receive drawing messages */
		/* Make sure that focus is set to an existing window */
		if (pWinFocus == this)
			pWinFocus = nullptr;
		if (pWinCapture == this)
			ReleaseCapture(); /* Make sure the window does not have capture */
		/* check if critical handles are affected. If so, reset the window handle to 0 */
		CriticalHandle::Check(this);
		_RemoveFromLinList();
		/* Delete all children */
		for (auto pChild = pFirstChild; pChild; ) {
			auto pNext = pChild->pNext;
			delete pChild;
			pChild = pNext;
		}
		/* Send WM_DELETE message to window in order to inform window itself */
		Require(WM_DELETE);     /* tell window about it */
		/* Remove window from window stack */
		_RemoveWindowFromList();
		NotifyParent(WM_NOTIFICATION_CHILD_DELETED);
		pParent = nullptr;
		/* Make sure window is no longer counted as invalid */
		if (Status & WC_ACTIVATE)
			NumInvalidWindows--;
		InvalidateArea(Rect);
		/* Free window memory */
		NumWindows--;
		/* Select a valid window */
		pWinFirst->Select();
//		GUI_ALLOC_Free(this);
	}

public:

	auto GetFlags() const { return Status; }

	WM_PARAM Require(uint16_t MsgId, WM_PARAM Data = 0)
	{ return cb ? cb(this, MsgId, Data) : (WM_PARAM)0; }
	WM_PARAM Require(uint16_t MsgId, WM_PARAM Data = 0) const
	{ return const_cast<WObj *>(this)->Require(MsgId, Data); }
	void NotifyParent(int Notification) {
		if (pParent) {
			NOTIFY_INFO NotifyInfo;
			NotifyInfo.Notification = Notification;
			NotifyInfo.pWinSrc = this;
			pParent->Require(WM_NOTIFY_PARENT, (WM_PARAM)&NotifyInfo);
		}
	}

	auto Client() { return (WObj *)Require(WM_GET_CLIENT_WINDOW); }

	auto FirstChild() { return pFirstChild; }
	auto FirstChild() const { return pFirstChild; }
	auto Parent() { return pParent; }
	auto Parent() const { return pParent; }

	auto NextSibling() { return pNext; }
	auto NextSibling() const { return pNext; }
	auto FirstSibling() { return pParent ? pParent->pFirstChild : nullptr; }
	auto LastSibling() {
		for (auto pWin = this; pWin; pWin = pWin->pNext)
			if (!pWin->pNext)
				return pWin;
		return this;
	}

	WObj *PrevSibling() {
		for (WObj *pWin = FirstSibling(), *pPrev = nullptr; pWin; pPrev = pWin, pWin = pWin->pNext)
			if (pWin == this)
				return pPrev;
		return nullptr;
	}

	bool IsAncestorOf(WObj *pChild) {
		for (; pChild; pChild = pChild->pParent)
			if (pChild->pParent == this)
				return true;
		return false;
	}
	bool IsAncestorOrSelf(WObj *pChild) {
		if (pChild == this)
			return true;
		return IsAncestorOf(pChild);
	}

#pragma region Coordinate
	auto GetRect() const { return Rect; }

	auto GetOrg() const { return Rect.LeftTop(); }

	void Anchor(uint16_t AnchorFlags) {
		Status &= ~WC_ANCHOR_ALL;
		Status |= AnchorFlags & WC_ANCHOR_ALL;
	}

	void _MoveDescendents(POINT d) {
		for (auto pWin = this; pWin; pWin = pWin->pNext) {
			pWin->Rect += d;
			pWin->InvalidRect += d;
			pWin->pFirstChild->_MoveDescendents(d);  /* Children need to be moved along ...*/
			pWin->Require(WM_MOVE);
		}
	}
	void Move(POINT d) {
		if (!d) return;
		auto r = Rect;
		Rect += d;
		InvalidRect += d;
		pFirstChild->_MoveDescendents(d);  /* Children need to be moved along ...*/
		Require(WM_MOVE); /* Notify window it has been moved */
		/* Invalidate old and new area ... */
		if (Status & WC_VISIBLE) {
			InvalidateArea(Rect);     /* Invalidate new area */
			InvalidateArea(r);        /* Invalidate old area */
		}
	}
	void MoveTo(POINT Pos) {
		Move(Pos - Rect.LeftTop());
	}
	void MoveChildTo(POINT Pos) {
		if (pParent)
			Move(Pos - Rect.LeftTop() + pParent->Rect.LeftTop());
	}

	void _UpdateChildPositions(RECT d) {
		for (auto pChild = pFirstChild; pChild; pChild = pChild->pNext) {
			/* Compute size of new rectangle */
			auto rOld = pChild->Rect, rNew = rOld;
			switch (pChild->Status & (WC_ANCHOR_RIGHT | WC_ANCHOR_LEFT)) {
			case WC_ANCHOR_RIGHT: /* Right ANCHOR : Move window with right side */
				rNew.x0 += d.x1;
				rNew.x1 += d.x1;
				break;
			case WC_ANCHOR_RIGHT | WC_ANCHOR_LEFT: /* Left & Right ANCHOR: Resize window */
				rNew.x0 += d.x0;
				rNew.x1 += d.x1;
				break;
			default: /* Left ANCHOR: Move window with left side of parent */
				rNew.x0 += d.x0;
				rNew.x1 += d.x0;
			}
			switch (pChild->Status & (WC_ANCHOR_TOP | WC_ANCHOR_BOTTOM)) {
			case WC_ANCHOR_BOTTOM: /* Bottom ANCHOR */
				rNew.y0 += d.y1;
				rNew.y1 += d.y1;
				break;
			case WC_ANCHOR_BOTTOM | WC_ANCHOR_TOP: /* resize window */
				rNew.y0 += d.y0;
				rNew.y1 += d.y1;
				break;
			default: /* Top ANCHOR */
				rNew.y0 += d.y0;
				rNew.y1 += d.y0;
			}
			/* Set new window position using Move and Resize as required */
			pChild->Move(rNew.LeftTop() - rOld.LeftTop());
			pChild->Resize(rNew.Size() - rOld.Size());
		}
	}
	void Resize(POINT d) {
		if (!d) return;
		auto rOld = Rect, rNew = rOld;
		if (d.x) {
			if ((Status & WC_ANCHOR_RIGHT) && !(Status & WC_ANCHOR_LEFT))
				rNew.x0 -= d.x;
			else
				rNew.x1 += d.x;
		}
		if (d.y) {
			if ((Status & WC_ANCHOR_BOTTOM) && !(Status & WC_ANCHOR_TOP))
				rNew.y0 -= d.y;
			else
				rNew.y1 += d.y;
		}
		Rect = rNew;
		InvalidateArea(rOld | rNew);
		_UpdateChildPositions(rNew - rOld);
		InvalidRect &= Rect; /* Make sure invalid area is not bigger than window itself */
		Require(WM_SIZE); /* Send size message to the window */
	}

	auto GetSize() const { return Rect.Size(); }
	auto GetSizeX() const { return Rect.XSize(); }
	auto GetSizeY() const { return Rect.YSize(); }
	void SetSize(POINT Size) {
		Resize(Size - GetSize());
	}

	RECT GetClientRect() const { return{ 0, Rect.Dist() }; }
	POINT GetClientSize() const { return GetClientRect().Size(); }
	RECT GetInsideRect() const {
		RECT r;
		Require(WM_GET_INSIDE_RECT, (WM_PARAM)&r);
		return r;
	}

	WObj *Screen2Win(POINT Pos, WObj *pStop = nullptr) {
		/* First check if the  coordinates are in the given window. If not, return 0 */
		if (!(Rect <= Pos))
			return nullptr;
		/* If the coordinates are in a child, search deeper ... */
		auto pWin = this;
		for (auto pChild = pWin->pFirstChild; pChild && (pChild != pStop); ) {
			auto pNextChild = pChild->pNext;
			if (auto pHit = pChild->Screen2Win(Pos, pStop))
				pWin = pHit; /* Found a window */
			pChild = pNextChild;
		}
		return pWin; /* No Child affected ... The parent is the right one */
	}
#pragma endregion

#pragma region Z-order
private:
	void _InvalidateWindowAndDescs() {
		Invalidate();
		for (auto pChild = pFirstChild; pChild; pChild = pChild->pNext) {
			pChild->Invalidate();
			pChild->_InvalidateWindowAndDescs();
		}
	}
public:
	void BringToTop() {
		/* Is window alread on top ? If so, we are done. (Not required, just an optimization) */
		if (!pNext)
			return;
		/* For non-top windows, it is good enough if the next one is a stay-on-top-window (Not required, just an optimization) */
		if (!(Status & WC_STAYONTOP))
			if (pNext->Status & WC_STAYONTOP)
				return;
		_RemoveWindowFromList();
		_InsertWindowIntoList(pParent);
		_InvalidateWindowAndDescs();
	}
	void BringToBottom() {
		if (auto pPrev = PrevSibling()) { /* If there is no previous one, there is nothing to do ! */
			auto pParent = Parent();
			/* unlink this */
			pPrev->pNext = pNext;
			/* Link from parent (making it the first child) */
			pNext = pParent->pFirstChild;
			pParent->pFirstChild = this;
			/* Send message in order to make sure top window will be drawn */
			InvalidateArea(Rect);
		}
	}
	void StayOnTop(bool bOnTop) {
		auto Status = bOnTop ? 
			this->Status | WC_STAYONTOP :
			this->Status & ~WC_STAYONTOP;
		if (this->Status != Status) {
			this->Status = Status;
			Attach(Parent());
		}
	}
	bool StayOnTop() const { return Status & WC_STAYONTOP; }
#pragma endregion

#pragma region Desktop
//private:
	static WObj *pWinDesktop;
	static RGBC BkColorDesktop;
	static WM_PARAM cbBackWin(WObj *pWin, int MsgId, WM_PARAM Data) {
		switch (MsgId) {
			case WM_KEY: {
				auto pKeyInfo = (const WM_KEY_INFO *)Data;
				if (pKeyInfo->PressedCnt == 1)
					GUI_StoreKey(pKeyInfo->Key);
				return 0;
			}
			case WM_PAINT:
				if (BkColorDesktop != RGB_INVALID_COLOR) {
					GUI.SetBkColor(BkColorDesktop);
					GUI_Clear();
				}
				return 0;
			default:
				return WM_DefaultProc(pWin, MsgId, Data);
		}
		return 0;
	}
public:
	static WObj *GetDesktopWindow() { return pWinDesktop; }
	static void SetDesktopColor(RGBC Color) {
		BkColorDesktop = Color;
		if (pWinDesktop)
			pWinDesktop->Invalidate();
	}
#pragma endregion

#pragma region Capture
	static WObj *pWinCapture;
	static bool WM__CaptureReleaseAuto;
	bool HasCaptured() const { return this == pWinCapture ? true : false; }
	static void ReleaseCapture(void) {
		if (pWinCapture) {
			pWinCapture->Require(WM_CAPTURE_RELEASED, 0);
			pWinCapture = nullptr;
		}
	}
	void SetCapture(int AutoRelease) {
		if (pWinCapture != this)
			ReleaseCapture();
		pWinCapture = this;
		WM__CaptureReleaseAuto = AutoRelease;
	}
	static POINT WM__CapturePoint;
	void SetCaptureMove(POINT Pos, int MinVisibility) {
		if (!HasCaptured()) {
			SetCapture(1); /* Set capture with auto release */
			WM__CapturePoint = Pos;
			return;
		}
		/* Moving ... let the window move ! */
		POINT d = Pos - WM__CapturePoint;
		/* make sure at least a part of the windows stays inside of its parent */
		if (!MinVisibility) {
			Move(d);
			return;
		}
		/* make sure at least a part of the windows stays inside of its parent */
		auto Rect = GetRect() + d,
			 RectParent = Parent()->GetRect() / MinVisibility;
		if (RectParent <= Rect)
			Move(d);
	}
#pragma endregion 

#pragma region Scroll
	WObj *GetScrollbarH() { return GetItem(GUI_ID_HSCROLL); }
	WObj *GetScrollbarV() { return GetItem(GUI_ID_VSCROLL); }

	void SetScrollState(const WM_SCROLL_STATE &State)
	{ Require(WM_SET_SCROLL_STATE, (WM_PARAM)&State); }
#pragma endregion

#pragma region ID
	uint16_t GetID() const { return (uint16_t)Require(WM_GET_ID); }
	void SetID(uint16_t Id) { Require(WM_SET_ID, (WM_PARAM)Id); }

	WObj *GetItem(uint16_t Id) {
		for (auto i = pFirstChild; i; i = i->pNext)
			if (i->GetID() == Id)
				return i;
			else if (auto pItem = i->GetItem(Id))
				return pItem;
		return nullptr;
	}
	const WObj *GetItem(uint16_t Id) const { return const_cast<WObj *>(this)->GetItem(Id); }
	template<class Ret>
	Ret *GetItem(uint16_t Id) { return (Ret *)GetItem(Id); }
	template<class Ret>
	const Ret *GetItem(uint16_t Id) const { return (const Ret *)GetItem(Id); }
#pragma endregion

#pragma region Dialog
	void DialogStatus(DIALOG_STATUS *Status) { Require(WM_HANDLE_DIALOG_STATUS, (WM_PARAM)Status); }
	auto DialogStatus() const { return (DIALOG_STATUS *)Require(WM_HANDLE_DIALOG_STATUS); }

	int DialogExec() {
		DIALOG_STATUS Status;
		DialogStatus(&Status);
		while (!Status.Done)
			GUI_Exec();
		return Status.ReturnValue;
	}

	void DialogEnd(uint16_t r) {
		if (auto pStatus = DialogStatus()) {
			pStatus->ReturnValue = r;
			pStatus->Done = 1;
		}
		delete Parent();
	}
#pragma endregion 

	RGBC GetBkColor() const { return (RGBC)Require(WM_GET_BKCOLOR); }

#pragma region Focus
	static WObj *pWinFocus;
	static auto GetFocussedWindow() { return pWinFocus; }
	bool HasFocus() const { return this == pWinFocus; }
	bool IsFocussable() const { return Require(WM_GET_ACCEPT_FOCUS); }
	bool SetFocus() {
		if (HasFocus())
			return true;
		NOTIFY_CHILD_HAS_FOCUS_INFO Info{ pWinFocus, this };
		/* Send a "no more focus" message to window losing focus */
		if (pWinFocus)
			pWinFocus->Require(WM_SET_FOCUS, 0);
		/* Send "You have the focus now" message to the window */
		pWinFocus = this;
		if (Require(WM_SET_FOCUS, 1))
			return true;
		/* Set message to ancestors of window getting the focus */
		WObj *pWin = this;
		for (pWin = this; pWin = pWin->Parent();)
			pWin->Require(WM_NOTIFY_CHILD_HAS_FOCUS, (WM_PARAM)&Info);
		/* Set message to ancestors of window loosing the focus */
		pWin = Info.pOld;
		if (IsWindow(pWin)) /* Make sure window has not been deleted in the mean time. Can be optimized: _DeleteWindow could clear the handle to avoid this check (RS) */
			while ((pWin = pWin->Parent()))
				pWin->Require(WM_NOTIFY_CHILD_HAS_FOCUS, (WM_PARAM)&Info);
		return false;
	}

	static WObj *_GetNextChild(WObj *pParent, WObj *pChild) {
		WObj *pObj = nullptr;
		if (pChild)
			pObj = pChild->pNext;
		if (!pObj)
			pObj = pParent->pFirstChild;
		if (pObj != pChild)
			return pObj;
		return nullptr;
	}
	WObj *_GetFocussedChild() {
		if (!pWinFocus) return nullptr;
		if (pWinFocus->pParent == this)
			return pWinFocus;
		return nullptr;
	}
	WObj *SetFocusOnNextChild() {
		if (auto pChild = _GetFocussedChild()) {
			do {
				if (!(pChild = _GetNextChild(this, pChild)))
					return nullptr;
			} while (!pChild->IsFocussable());
			if (!pChild->SetFocus())
				return pChild;
		}
		return nullptr;
	}

#pragma endregion

#pragma region Visibility
	bool IsVisible() const { return Status & WC_VISIBLE; }
	void ShowWindow() {
		if (!(Status & WC_VISIBLE)) {
			Status |= WC_VISIBLE;
			InvalidateDescs();
		}
	}
	void HideWindow() {
		if (Status & WC_VISIBLE) {
			Status &= ~WC_VISIBLE;
			_Invalidate1Abs(Rect);
		}
	}
#pragma endregion

	bool IsEnabled() const { return !(Status & WC_DISABLED); }

};

WObj *WM_Screen2Win(POINT Pos, WObj *pStop = nullptr) {
	return WObj::pWinFirst->Screen2Win(Pos, pStop);
}

}

uint16_t WObj::NumWindows = 0;
WObj* WObj::pWinFirst = nullptr;
WObj* WObj::pWinActive = nullptr;

bool WObj::IsActive = false;

uint16_t WObj::NumInvalidWindows = 0;
WObj *WObj::pWinNextDraw = nullptr;

decltype(WObj::_ClipContext) WObj::_ClipContext;

uint8_t WObj::_PaintCallbackCnt = 0;

WObj::CriticalHandle *WObj::CriticalHandle::pFirst = nullptr;
WObj::CriticalHandle WObj::CHWinLast;

WObj* WObj::pWinDesktop = nullptr;
RGBC WObj::BkColorDesktop = RGB_GRAY;

WObj* WObj::pWinCapture = nullptr;
bool  WObj::WM__CaptureReleaseAuto = false;
POINT WObj::WM__CapturePoint = { 0, 0 };

WObj* WObj::pWinFocus = nullptr;
