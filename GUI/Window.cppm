export module TUX.Window;

#include "GUIConf.h"

#define WM_ASSERT_NOT_IN_PAINT()

export import TUX;
export import TUX.Types;
export import TUX.WindowTypes;

export {

PID_STATE WM_PID__StateLast{ 0 };
PID_STATE WM_PID__GetPrevState() {
	return WM_PID__StateLast;
}

class WObj;

struct NOTIFY_INFO {
	int Notification;
	WObj *pWinSrc;
};

struct NOTIFY_CHILD_HAS_FOCUS_INFO {
	WObj *pOld, *pNew;
};

typedef WM_PARAM WM_CALLBACK(WObj *pWin, int MsgId, WM_PARAM Data);

class WObj {
	RECT rWin, rInvalid;
	WObj *pNextLin = nullptr, *pNext = nullptr,
		*pParent = nullptr, *pFirstChild = nullptr;
	WM_CALLBACK *cb = nullptr; /* ptr to notification callback */
protected:
	uint16_t Status = 0; /* Some status flags */

#pragma region Window list
	static uint16_t NumWindows;
	static WObj *pDesktop;
	void _RemoveFromLinList() {
		for (auto pCur = pDesktop; pCur; ) {
			auto pNext = pCur->pNextLin;
			if (pNext == this) {
				pCur->pNextLin = pNextLin;
				break;
			}
			pCur = pNext;
		}
	}
	void _AddToLinList() {
		if (!pDesktop) {
			pDesktop = this;
			return;
		}
		auto pFirst = pDesktop;
		pNextLin = pFirst->pNextLin;
		pFirst->pNextLin = this;
	}
public:
	bool IsWindow() const {
		for (auto i = pDesktop; i; i = i->pNextLin)
			if (i == this)
				return true;
		return false;
	}
	static auto GetNumWindows() { return NumWindows; }
	static bool IsWindow(WObj *pWin) { return pWin ? pWin->IsWindow() : false; }
#pragma endregion

#pragma region Parent list
private:
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
protected:
	void _Detach() {
		_RemoveWindowFromList();
		/* Clear area used by this window */
		InvalidateArea(rWin);
	}
public:
	void Detach() {
		POINT org;
		if (pParent)
			org = -pParent->rWin.LeftTop();
		_Detach();
		Move(org); /* Convert screen coordinates -> parent coordinates */
		/* ToDo: Invalidate. If Parent window is located at (0,0). */
		pParent = nullptr;
	}
	void Attach(WObj *pParent, POINT Pos = {}) {
		Detach();
		if (pParent && pParent != this) {
			_InsertWindowIntoList(pParent);
			MoveTo(pParent->rWin.LeftTop() + Pos); /* Convert parent coordinates -> screen coordinates */
		}
	}
#pragma endregion

#pragma region Z-order
private:
	void _InvalidateWindowAndDescs() {
		Invalidate();
		for (auto pChild = FirstChild(); pChild; pChild = pChild->NextSibling()) {
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
			InvalidateArea(rWin);
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
private:
	static RGBC BkColorDesktop;
	static WM_PARAM cbBackWin(WObj *pWin, int MsgId, WM_PARAM Data) {
		switch (MsgId) {
			case WM_KEY: {
				return 0;
			}
			case WM_PAINT:
				if (BkColorDesktop != RGB_INVALID) {
					GUI.BkColor(BkColorDesktop);
					GUI.Clear();
				}
				return 0;
			default:
				return DefaultProc(pWin, MsgId, Data);
		}
		return 0;
	}
public:
	static WObj *GetDesktopWindow() { return pDesktop; }
	static void DesktopColor(RGBC Color) {
		BkColorDesktop = Color;
		if (pDesktop)
			pDesktop->Invalidate();
	}
	static WObj *CreateDesktopWindow() {
		if (!pDesktop) {
			pDesktop = new WObj(GUI_X_GetLCD()->Rect(), WC_VISIBLE, cbBackWin);
			pDesktop->Invalidate();
			pDesktop->Select();
		}
		return pDesktop;
	}
#pragma endregion

private:
	static WObj *pWinActive;
public:
	static auto ActiveWindow() { return pWinActive; }
	void Select() {
		WM_ASSERT_NOT_IN_PAINT();
		pWinActive = this;
		GUI.ClipRectMax();
		GUI.Off = rWin.LeftTop();
	}

#pragma region Invalidation
private:
	static uint16_t NumInvalidWindows;
	bool _ClipAtParentBorders(RECT &r) const {
		for (auto pWin = this; pWin->Status & WC_VISIBLE; pWin = pWin->pParent) {
			r &= pWin->rWin;
			if (!pWin->pParent)
				return pWin == pDesktop;
		}
		return false;
	}
	void _Invalidate1Abs(RECT r) {
		if (!(Status & WC_VISIBLE))
			return; /* Window is not visible... we are done */
		/* Calc affected area */
		if (r &= rWin) {
			if (Status & WC_ACTIVATE)
				rInvalid |= r;
			else {
				rInvalid = r;
				Status |= WC_ACTIVATE;
				NumInvalidWindows++;
			}
		}
	}
public:
	static uint16_t GetNumInvalidWindows() { return NumInvalidWindows; }
	const RECT &GetInvalidRect() const { return rInvalid; }
	static void InvalidateArea(const RECT &r) {
		for (auto pWin = pDesktop; pWin; pWin = pWin->pNextLin)
			pWin->_Invalidate1Abs(r);
	}
	void Invalidate(const RECT *pRect = nullptr) {
		if (!(Status & WC_VISIBLE))
			return;
		auto r = rWin;
		if (pRect)
			r &= *pRect + LeftTop();
		/* Optimization that saves invalidation if window area is not visible ... Not required */
		if (_ClipAtParentBorders(r))
			_Invalidate1Abs(r);
	}
	void InvalidateDescs() {
		Invalidate();    /* Invalidate window itself */
		for (auto pChild = FirstChild(); pChild; pChild = pChild->pNext)
			pChild->InvalidateDescs();
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
	class IVR {
		RECT rClient, CurRect;
		const RECT *prUserClip = nullptr;
		int Cnt = -1;
		
		void _ActivateClipRect() const {
			/* Take UserClipRect into account */
			RECT rSrc = CurRect;
			if (prUserClip) {
				auto r = *prUserClip;
				if (pWinActive)
					r += pWinActive->LeftTop(); /* Convert User rClip into screen coordinates */
				/* Set intersection as clip rect */
				rSrc &= r;
			}
			GUI.ClipRect(rSrc);
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
		bool _FindNext() {
			auto r = CurRect;  /* temps  so we do not have to work with pointers too much */
			/*
			   STEP 1:
				 Set the next position which could be part of the next IVR
				 This will be the first unhandle pixel in reading order, i.e. next one to the right
				 or next one down if we are at the right border.
			*/
			if (!Cnt) /* First IVR starts in upper left */
				r.LeftTop(rClient.LeftTop());
			else {
				r.x0 = CurRect.x1 + 1;
				r.y0 = CurRect.y0;
				if (r.x0 > rClient.x1) {
				NextStripe: /* go down to next stripe */
					r.x0 = rClient.x0;
					r.y0 = CurRect.y1 + 1;
				}
			}
			/*
			   STEP 2:
				 Check if we are done completely.
			*/
			if (r.y0 > rClient.y1)
				return false;
			/* STEP 3:
				 Find out the max. height (r.y1) if we are at the left border.
				 Since we are using the same height for all IVRs at the same y0,
				 we do this only for the leftmost one.
			*/
			static auto _Findy1 = [](const WObj *pWin, RECT &r) {
				for (; pWin; pWin = pWin->NextSibling())
					if (pWin->Status & WC_VISIBLE)
						if (auto rWin = pWin->rWin; rWin <= r) {
							if (rWin.y0 > r.y0) {
								if (r.y1 > rWin.y0 - 1) /* Check upper border of window */
									r.y1 = rWin.y0 - 1;
							}
							else if (r.y1 > rWin.y1) /* Check lower border of window */
								r.y1 = rWin.y1;
						}
			};
			if (r.x0 == rClient.x0) {
				r.RightBottom(rClient.RightBottom());
				/* Iterate over all windows which are above */
				/* Check all siblings above (Iterate over Parents and top siblings (hNext) */
				for (auto pParent = pWinActive; pParent; pParent = pParent->pParent)
					_Findy1(pParent->NextSibling(), r);
				/* Check all children */
				_Findy1(pWinActive->FirstChild(), r);
			}
			/*
			  STEP 4
				Find out x0 for the given y0, y1 by iterating over windows above.
				if we find one that intersects, adjust x0 to the right.
			*/
			static auto _Findx0 = [](const WObj * pWin, RECT & r) {
				for (; pWin; pWin = pWin->NextSibling())
					if (pWin->Status & WC_VISIBLE)
						if (auto rWin = pWin->rWin; rWin <= r) {
							r.x0 = rWin.x1 + 1;
							return true;
						}
				return false;
			};
		Find_x0:
			r.x1 = r.x0;
			/* Iterate over all windows which are above */
			/* Check all siblings above (siblings of window, siblings of parents, etc ...) */
			for (auto pParent = pWinActive; pParent; pParent = pParent->pParent)
				if (_Findx0(pParent->NextSibling(), r))
					goto Find_x0;
			/* Check all children */
			if (_Findx0(pWinActive->FirstChild(), r))
				goto Find_x0;
			/*
			 STEP 5:
			   If r.x0 out of right border, this stripe is done. Set next stripe and goto STEP 2
			   Find out x1 for the given x0, y0, y1
			*/
			r.x1 = rClient.x1;
			if (r.x1 < r.x0) { /* horizontal border reached ? */
				CurRect = r;
				goto NextStripe;
			}
			/*
			 STEP 6:
			   Find r.x1. We have to Iterate over all windows which are above
			*/
			static auto _Findx1 = [](const WObj *pWin, RECT &r) {
				for (; pWin; pWin = pWin->NextSibling())
					if (pWin->Status & WC_VISIBLE)
						if (auto rWin = pWin->rWin; rWin <= r)
							r.x1 = rWin.x0 - 1;
			};
			/* Check all siblings above (Iterate over Parents and top siblings (hNext) */
			for (auto pParent = pWinActive; pParent; pParent = pParent->pParent)
				_Findx1(pParent->NextSibling(), r);
			/* Check all children */
			_Findx1(pWinActive->FirstChild(), r);
			/* We are done. Return the rectangle we found in the  */
			if (Cnt > 200)
				return false;  /* error !!! This should not happen !*/
			CurRect = r;
			return true;  /* IVR is valid ! */
		}
	public:
		bool GetNext() {
	#if GUI_SUPPORT_CURSOR
			static char _CursorHidden = false;
			if (_CursorHidden) {
				_CursorHidden = 0;
				GUI_CURSOR__TempShow();
			}
	#endif
			++Cnt;
			/* Find next rectangle and use it as rClip */
			if (!_FindNext())
				return false;        /* Could not find an other one ! */
			_ActivateClipRect();
			/* Hide cursor if necessary */
	#if GUI_SUPPORT_CURSOR
			_CursorHidden = GUI_CURSOR__TempHide(CurRect);
	#endif
			return true;
		}
		bool InitSearch(RECT rcMax) {
			Cnt = -1;
			/* When using callback mechanism, it is legal to reduce drawing
			   area to the invalid area ! */
			RECT r = pWinActive->rWin;
			/* If the drawing routine has specified a rectangle, use it to reduce the rectangle */
			r &= rcMax;
			/* If user has reduced the cliprect size, reduce the rectangle */
			if (prUserClip)
				r &= *(prUserClip) + pWinActive->LeftTop();
			/* Store the rectangle and find the first rectangle of the area */
			rClient = r;
			return GetNext();
		}
		const RECT *UserClip(const RECT *pRect) {
			auto pRectReturn = prUserClip;
			prUserClip = pRect;
			_ActivateClipRect();
			return pRectReturn;
		}
	} static _ClipContext;

public:
	static const RECT *UserClip(const RECT *pRect) {
		return _ClipContext.UserClip(pRect);
	}
	static inline void Iterate(RECT r, auto fn) {

		if (_ClipContext.InitSearch(r))
			do { fn(); } while (_ClipContext.GetNext());

	}
#pragma endregion

#pragma region Paint & Draw
	bool _Paint(GUI_CONTEXT &ctx) {
		if (!(Status & WC_ACTIVATE))
			return false;
		bool Ret = false;
		if (cb && IsVisible() && _ClipAtParentBorders(rInvalid)) {
			Select();
			if (Status & WC_MEMDEV) {
				MemDev.Alloc(rInvalid);
				ctx.pDevice = &MemDev;
				ctx.ClipRectMax();
				Require(WM_PAINT, (WM_PARAM)&ctx);
				ctx.pDevice = GUI_X_GetLCD();
				Iterate(MemDev.Rect(), [&] {
					LCD_DrawBitmap(BITVIEW{
						MemDev.Rect(),
						MemDev.BytesPerLine,
						MemDev.BitsPerPixel(),
						MemDev.pData,
						nullptr });
				});
			}
			else
				Iterate(rInvalid, [&] {
					Require(WM_PAINT, (WM_PARAM)&ctx);
				});
			Ret = true;    /* Something has been done */
		}
		/* We purposly clear the invalid flag after painting so we can still query the invalid rectangle while painting */
		Status &= ~WC_ACTIVATE; /* Clear invalid flag */
		--NumInvalidWindows;
		return Ret;
	}
private:
	static WObj *pwDraw;
public:
	static bool DrawOnce(GUI_CONTEXT &ctx) {
		if (!NumInvalidWindows)
			return false;
		ctx.Init();
		if (!pwDraw) pwDraw = pDesktop;
		for (; pwDraw; pwDraw = pwDraw->pNextLin)
			if (pwDraw->_Paint(ctx))
				break;
		return true;
	}
#pragma endregion

#pragma region CriticalHandles
private:
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
					(pLast ? pLast->pNext : pFirst) = pCH->pNext;
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
		return GUI_MEM_Alloc(size);
	}
	void operator delete(void *p) {
		GUI_MEM_Free(p);
	}

public:
	WObj(RECT r, WM_CF Style, WM_CALLBACK *cb, WObj *pParent = nullptr) :
		rWin(r), cb(cb), Status(Style & WM_CF_MASK) {
		WM_ASSERT_NOT_IN_PAINT();
		if (!pParent)
			pParent = pDesktop;
		if (pParent) {
			rWin += pParent->rWin.LeftTop();
			if (!r.XSize())
				rWin.x1 = pParent->rWin.x1;
			if (!r.YSize())
				rWin.y1 = pParent->rWin.y1;
		}
		NumWindows++;
		_AddToLinList();
		_InsertWindowIntoList(pParent);
		/* Activate window if WC_ACTIVATE is specified */
		if (Style & WC_ACTIVATE)
			Select();  /* This is not needed if callbacks are being used, but it does not cost a lot and makes life easier ... */
		if (Style & WC_VISIBLE)
			Invalidate();
		Require(WM_CREATE);
	}
	~WObj() {
		WM_ASSERT_NOT_IN_PAINT();
		if (pDesktop == this)
			pDesktop = nullptr;
		if (pwDraw == this)
			pwDraw = nullptr;
		if (!IsWindow(this))
			return;
		if (pWinFocus == this) {
			Require(WM_SET_FOCUS, 0);
			pWinFocus = nullptr;
		}
		if (pWinCapture == this)
			ReleaseCapture();
		CriticalHandle::Check(this);
		_RemoveFromLinList();
		for (auto pChild = FirstChild(); pChild; ) {
			auto pNext = pChild->pNext;
			delete pChild;
			pChild = pNext;
		}
		Require(WM_DELETE);
		_RemoveWindowFromList();
		NotifyParent(WM_NOTIFICATION_CHILD_DELETED);
		pParent = nullptr;
		if (Status & WC_ACTIVATE)
			NumInvalidWindows--;
		InvalidateArea(rWin);
		NumWindows--;
		pDesktop->Select();
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
	auto FirstSibling() { return pParent ? pParent->FirstChild() : nullptr; }
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

	/*********************************************************************
	*
	*       DefaultProc
	*
	* Purpose
	*   Default callback for windows
	*   Any window should call this routine in the "default" part of the
	*   its callback function for messages it does not handle itself.
	*
	*/
	static WM_PARAM DefaultProc(WObj *pWin, int MsgId, WM_PARAM Data) {
		/* Exec message */
		switch (MsgId) {
		case WM_GET_INSIDE_RECT: /* return client window in absolute (screen) coordinates */
			*(RECT *)Data = pWin->ClientRect();
			return 0;
		case WM_GET_CLIENT_WINDOW: /* return handle to client window. For most windows, there is no seperate client window, so it is the same handle */
			return (WM_PARAM)pWin;
		case WM_KEY:
			pWin->Parent()->Require(WM_KEY, Data);
			return 0;
		case WM_GET_BKCOLOR:
			return RGB_INVALID;
		case WM_NOTIFY_ENABLE:
			pWin->Invalidate();
			return 0;
		}
		/* Message not handled. If it queries something, we return 0 to be on the safe side. */
		return 0;
	}

#pragma region Coordinate
private:
	static void _MoveDescendents(WObj *pWin, POINT d) {
		for (; pWin; pWin = pWin->pNext) {
			pWin->rWin += d;
			pWin->rInvalid += d;
			_MoveDescendents(pWin->FirstChild(), d);  /* Children need to be moved along ...*/
			pWin->Require(WM_MOVE);
		}
	}
public:
	auto Rect() const { return rWin; }
	auto LeftTop() const { return rWin.LeftTop(); }

	void Anchor(uint16_t AnchorFlags) {
		Status &= ~WC_ANCHOR_ALL;
		Status |= AnchorFlags & WC_ANCHOR_ALL;
	}

	void Move(POINT d) {
		if (!d) return;
		auto r = rWin;
		rWin += d;
		rInvalid += d;
		_MoveDescendents(FirstChild(), d);  /* Children need to be moved along ...*/
		Require(WM_MOVE); /* Notify window it has been moved */
		/* Invalidate old and new area ... */
		if (Status & WC_VISIBLE) {
			InvalidateArea(rWin);     /* Invalidate new area */
			InvalidateArea(r);        /* Invalidate old area */
		}
	}
	void MoveTo(POINT Pos) {
		Move(Pos - rWin.LeftTop());
	}
	void MoveChildTo(POINT Pos) {
		if (pParent)
			Move(Pos - rWin.LeftTop() + pParent->rWin.LeftTop());
	}

	void _UpdateChildPositions(RECT d) {
		for (auto pChild = FirstChild(); pChild; pChild = pChild->NextSibling()) {
			/* Compute size of new rectangle */
			auto rOld = pChild->rWin, rNew = rOld;
			switch (pChild->Status & WC_ANCHOR_VERTICAL) {
			case WC_ANCHOR_RIGHT:
				rNew.x0 += d.x1;
				rNew.x1 += d.x1;
				break;
			case WC_ANCHOR_VERTICAL:
				rNew.x0 += d.x0;
				rNew.x1 += d.x1;
				break;
			default:
				rNew.x0 += d.x0;
				rNew.x1 += d.x0;
				break;
			}
			switch (pChild->Status & WC_ANCHOR_HORIZONTAL) {
			case WC_ANCHOR_BOTTOM:
				rNew.y0 += d.y1;
				rNew.y1 += d.y1;
				break;
			case WC_ANCHOR_HORIZONTAL:
				rNew.y0 += d.y0;
				rNew.y1 += d.y1;
				break;
			default:
				rNew.y0 += d.y0;
				rNew.y1 += d.y0;
				break;
			}
			/* Set new window position using Move and Resize as required */
			pChild->Move(rNew.LeftTop() - rOld.LeftTop());
			pChild->Resize(rNew.Size() - rOld.Size());
		}
	}
	void Resize(POINT d) {
		if (!d) return;
		auto rOld = rWin, rNew = rOld;
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
		rWin = rNew;
		InvalidateArea(rOld | rNew);
		_UpdateChildPositions(rNew - rOld);
		rInvalid &= rWin; /* Make sure invalid area is not bigger than window itself */
		Require(WM_SIZE); /* Send size message to the window */
	}

	auto Size() const { return rWin.Size(); }
	auto SizeX() const { return rWin.XSize(); }
	auto SizeY() const { return rWin.YSize(); }
	void Size(POINT s) {
		Resize(s - Size());
	}

	RECT ClientRect() const { return{ 0, rWin.Dist() }; }
	RECT InsideRect() const {
		RECT r;
		Require(WM_GET_INSIDE_RECT, (WM_PARAM)&r);
		return r;
	}

	WObj *Screen2Win(POINT Pos, WObj *pStop = nullptr) {
		/* First check if the  coordinates are in the given window. If not, return 0 */
		if (!(rWin <= Pos))
			return nullptr;
		/* If the coordinates are in a child, search deeper ... */
		auto pWin = this;
		for (auto pChild = pWin->FirstChild(); pChild && (pChild != pStop); ) {
			auto pNextChild = pChild->NextSibling();
			if (auto pHit = pChild->Screen2Win(Pos, pStop))
				pWin = pHit; /* Found a window */
			pChild = pNextChild;
		}
		return pWin; /* No Child affected ... The parent is the right one */
	}
	static WObj *WM_Screen2Win(POINT Pos, WObj *pStop = nullptr) {
		return pDesktop->Screen2Win(Pos, pStop);
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
		auto rWin = Rect() + d,
			 rParent = Parent()->Rect() / MinVisibility;
		if (rParent <= rWin)
			Move(d);
	}
#pragma endregion 

#pragma region Scroll
	WObj *GetScrollbarV() { return GetItem(GUI_ID_VSCROLL); }
	const WObj *GetScrollbarV() const { return GetItem(GUI_ID_VSCROLL); }
	void  SetScrollbarV(bool);

	WObj *GetScrollbarH() { return GetItem(GUI_ID_HSCROLL); }
	const WObj *GetScrollbarH() const { return GetItem(GUI_ID_HSCROLL); }
	void  SetScrollbarH(bool);

	RECT InsideRectEx() const;

	void ScrollState(const SCROLL_STATE &State)
	{ Require(WM_SET_SCROLL_STATE, (WM_PARAM)&State); }
	SCROLL_STATE ScrollState() {
		SCROLL_STATE ScrollState;
		Require(WM_GET_SCROLL_STATE, (WM_PARAM)&ScrollState);
		return ScrollState;
	}
#pragma endregion

#pragma region ID
	uint16_t GetID() const { return (uint16_t)Require(WM_GET_ID); }
	void SetID(uint16_t Id) { Require(WM_SET_ID, (WM_PARAM)Id); }

	WObj *GetItem(uint16_t Id) {
		for (auto i = FirstChild(); i; i = i->NextSibling())
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

	RGBC BkColor() const { return (RGBC)(uint32_t)Require(WM_GET_BKCOLOR); }

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
			pObj = pChild->NextSibling();
		if (!pObj)
			pObj = pParent->FirstChild();
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
			_Invalidate1Abs(rWin);
		}
	}
#pragma endregion

	bool IsEnabled() const { return !(Status & WC_DISABLED); }
	
	void EnableMemdev() { Status |= WC_MEMDEV; }
	void DisableMemdev() { Status &= ~(WC_MEMDEV); }

public:
	static bool OnKey(KEY_STATE State) {
		if (pWinFocus)
			return pWinFocus->Require(WM_KEY, (WM_PARAM)&State);
		return false;
	}

#pragma region Mouse/Touch
private:
	void _SendMessageIfEnabled(uint16_t MsgId, WM_PARAM Data) {
		if (IsEnabled())
			Require(MsgId, Data);
	}
	void _SendTouchMessage(uint16_t MsgId, PID_STATE *pState) {
		if (pState)
			*pState -= rWin.LeftTop();
		_SendMessageIfEnabled(MsgId, (WM_PARAM)pState);
		/* Send notification to all ancestors.
		   We need to check if the window which has received the last message still exists,
		   since it may have deleted itself and its parent as result of the message.
		*/
		for (auto pWin = Parent(); IsWindow(pWin); pWin = pWin->Parent())
			pWin->_SendMessageIfEnabled(WM_TOUCH_CHILD, (WM_PARAM)pState); /* Send message to the ancestors */
	}
public:
	/*********************************************************************
	*
	*       HandlePID
	*
	* Polls the touch screen. If something has changed,
	* sends a message to the concerned window.
	*
	* Return value:
	*   0 if nothing has been done
	*   1 if touch message has been sent
	*/
	static bool HandlePID() {
		auto StateNew = GUI_PID_Get();
		if (WM_PID__StateLast == StateNew) return false;
		bool r = false;
#if GUI_SUPPORT_CURSOR
		GUI_CURSOR_SetPosition(StateNew.x, StateNew.y);
#endif
		CriticalHandle CHWin = pWinCapture ? pWinCapture : WM_Screen2Win(StateNew);
		CHWin.Add();
		/* Send WM_PID_STATE_CHANGED message if state has changed (just pressed or just released) */
		if (WM_PID__StateLast.Pressed != StateNew.Pressed && CHWin.pWin) {
			PID_CHANGED_INFO Info{ StateNew - CHWin.pWin->rWin.LeftTop(),
				StateNew.Pressed, WM_PID__StateLast.Pressed };
			CHWin.pWin->_SendMessageIfEnabled(WM_PID_STATE_CHANGED, (WM_PARAM)&Info);
		}
		/* Send WM_TOUCH message(s) Note that we may have to send 2 touch messages. */
		if (WM_PID__StateLast.Pressed | StateNew.Pressed) { /* Only if pressed or just released */
			r = true;
			/* Tell window if it is no longer pressed
			* This happens for 2 possible reasons:
			* a) PID is released
			* b) PID is moved out
			*/
			if (CHWinLast.pWin != CHWin.pWin && CHWinLast.pWin) {
				PID_STATE *pState = StateNew.Pressed ? nullptr : &WM_PID__StateLast;
				CHWinLast.pWin->_SendTouchMessage(WM_TOUCH, pState);
				CHWinLast.pWin = nullptr;
			}
			/* Sending WM_TOUCH to current window */
			if (CHWin.pWin) {
				/* Remember window */
				if (StateNew.Pressed)
					CHWinLast.pWin = CHWin.pWin;
				else {
					/* Handle automatic capture release */
					if (WM__CaptureReleaseAuto)
						ReleaseCapture();
					CHWinLast.pWin = nullptr;
				}
				CHWin.pWin->_SendTouchMessage(WM_TOUCH, &StateNew);
			}
		}
#if GUI_SUPPORT_MOUSE
	/* Send WM_MOUSEOVER Message */
		else if (CHWin.pWin)
			/* Do not send messages to disabled windows */
			if (CHWin.pWin->IsEnabled())
				CHWin.pWin->_SendTouchMessage(WM_MOUSEOVER, &StateNew);
#endif
		CHWin.Remove();
		/* Store the new state */
		WM_PID__StateLast = GUI_PID_Get();
		return r;
	}
#pragma endregion

	static bool Exec1(void) {
		/* Poll PID if necessary */
		if (HandlePID())
			return true; /* We have done something ... */
		if (GUI_PollKeyMsg())
			return true; /* We have done something ... */
		if (DrawOnce(GUI))
			return true; /* We have done something ... */
		return false; /* There was nothing to do ... */
	}
	static bool Exec(void) {
		bool r = false;
		while (Exec1())
			r = true; /* We have done something */
		return r;
	}

	static void Init(void) {
		if (pDesktop)
			return;
		/* Register the critical handles ... Note: This could be moved into the module setting the Window handle */
		CHWinLast.Add();
		CreateDesktopWindow();
	}
};

}

uint16_t WObj::NumWindows = 0;
WObj* WObj::pDesktop = nullptr;
WObj* WObj::pWinActive = nullptr;

uint16_t WObj::NumInvalidWindows = 0;
WObj *WObj::pwDraw = nullptr;

WObj::IVR WObj::_ClipContext;

WObj::CriticalHandle *WObj::CriticalHandle::pFirst = nullptr;
WObj::CriticalHandle WObj::CHWinLast;

RGBC WObj::BkColorDesktop = RGB_GRAY;

WObj* WObj::pWinCapture = nullptr;
bool  WObj::WM__CaptureReleaseAuto = false;
POINT WObj::WM__CapturePoint = { 0, 0 };

WObj* WObj::pWinFocus = nullptr;
