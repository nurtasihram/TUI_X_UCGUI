module;

#include "GUI.h"

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

void WM_Activate(void);
void WM_Deactivate(void);

void WM_Init(void);
int  WM_Exec(void);  /* Execute all jobs ... Return 0 if nothing was done. */
int  WM_Exec1(void); /* Execute one job  ... Return 0 if nothing was done. */

void    WM_DeleteWindow(WObj *pWin);
void  WM_SetHasTrans(WObj *pWin);
void  WM_SetTransState(WObj *pWin, unsigned State);
void  WM_SetStayOnTop(WObj *pWin, int OnOff);
int   WM_GetStayOnTop(WObj *pWin);
void  WM_SetAnchor(WObj *pWin, uint16_t AnchorFlags);

/* Move/resize windows */
int  WM_CreateTimer(WObj *pWin, int UserID, int Period, int Mode); /* not to be documented (may change in future version) */
void WM_DeleteTimer(WObj *pWin, int UserId); /* not to be documented (may change in future version) */

/* Set (new) callback function */
WM_CALLBACK *WM_SetCallback(WObj *Win, WM_CALLBACK *cb);

/* Get size/origin of a window */
RECT WM_GetClientRect();
RECT WM_GetInsideRect();

/* Select window used for drawing operations */
WObj *WM_SelectWindow(WObj *pWin);
WObj *WM_GetActiveWindow(void);
void    WM_Paint(WObj *pObj);

/* Reduce clipping area of a window */
const RECT *WM_SetUserClipRect(const RECT *pRect);

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

#pragma region IVR
bool WM__InitIVRSearch(RECT rcMax);
bool WM__GetNextIVR(void);
inline void WM_Iterate(RECT &r, auto fn) {
	if (WM__InitIVRSearch(r))
		do { fn(); } while (WM__GetNextIVR());

}
#pragma endregion

struct WObj {
	RECT Rect, InvalidRect;
	WM_CALLBACK *cb = nullptr; /* ptr to notification callback */
	WObj *pNextLin = nullptr, *pNext = nullptr,
		*pParent = nullptr, *pFirstChild = nullptr;
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
		if (!pParent)
			return;
		_RemoveWindowFromList();
		/* Clear area used by this window */
		InvalidateArea(Rect);
		pParent = nullptr;
	}
public:
	void Detach() {
		_Detach();
		if (pParent)
			Move(-pParent->Rect.LeftTop()); /* Convert screen coordinates -> parent coordinates */
		/* ToDo: Invalidate. If Parent window is located at (0,0). */
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

public:
	void *operator new(size_t size) {
		return GUI_ALLOC_AllocNoInit(size);
	}
	void operator delete(void *p) {
		GUI_ALLOC_Free(p);
	}

public:
	WObj(RECT r, WM_CF Style, WM_CALLBACK *cb, WObj *pParent = nullptr) :
		Rect(r), cb(cb), Status(Style &(WC_VISIBLE |
										WC_MEMDEV |
										WC_MEMDEV_ON_REDRAW |
										WC_STAYONTOP |
										WC_CONST_OUTLINE |
										WC_ANCHOR_RIGHT |
										WC_ANCHOR_BOTTOM |
										WC_ANCHOR_LEFT |
										WC_ANCHOR_TOP |
										WC_LATE_CLIP)) {
		//WM_ASSERT_NOT_IN_PAINT();
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
			WM_SelectWindow(this);  /* This is not needed if callbacks are being used, but it does not cost a lot and makes life easier ... */
		/* Handle the Style flags, one at a time */
		if (Style & WC_BGND)
			BringToBottom();
		if (Style & WC_VISIBLE)
			Invalidate();    /* Mark content as invalid */
		Require(WM_CREATE);
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

#pragma region Coordinate
	auto GetRect() const { return Rect; }

	auto GetOrg() const { return Rect.LeftTop(); }

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
		WM_DeleteWindow(this);
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
		NOTIFY_CHILD_HAS_FOCUS_INFO Info;
		Info.pOld = pWinFocus;
		Info.pNew = this;
		/* Send a "no more focus" message to window losing focus */
		if (pWinFocus)
			pWinFocus->Require(WM_SET_FOCUS, 0);
		/* Send "You have the focus now" message to the window */
		pWinFocus = this;
		if (Require(WM_SET_FOCUS, 1))
			return true;
		/* Set message to ancestors of window getting the focus */
		WObj *pWin = this;
		while ((pWin = pWin->Parent()))
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

uint16_t WObj::NumInvalidWindows = 0;

WObj* WObj::pWinDesktop = nullptr;
RGBC WObj::BkColorDesktop = RGB_GRAY;

WObj* WObj::pWinCapture = nullptr;
bool  WObj::WM__CaptureReleaseAuto = false;
POINT WObj::WM__CapturePoint = { 0, 0 };

WObj* WObj::pWinFocus = nullptr;
