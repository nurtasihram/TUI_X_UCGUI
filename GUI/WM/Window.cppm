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

uint16_t WM_SetCreateFlags(uint16_t Flags);

void    WM_AttachWindow(WObj *pWin, WObj *pParent);
void    WM_AttachWindowAt(WObj *pWin, WObj *pParent, int x, int y);
void    WM_ClrHasTrans(WObj *pWin);
WObj *WM_CreateWindow(int x0, int y0, int xSize, int ySize, uint16_t Style, WM_CALLBACK *cb, int NumExtraBytes);
WObj *WM_CreateWindowAsChild(int x0, int y0, int xSize, int ySize, WObj *pWinParent, uint16_t Style, WM_CALLBACK *cb, int NumExtraBytes);
void    WM_DeleteWindow(WObj *pWin);
void    WM_DetachWindow(WObj *pWin);
int     WM_GetHasTrans(WObj *pWin);
void  WM_InvalidateArea(const RECT *pRect);
void  WM_Invalidate(WObj *pWin, const RECT *pRect = nullptr);
void  WM_InvalidateDescs(WObj *pWin);    /* not to be documented (may change in future version) */
void  WM_SetHasTrans(WObj *pWin);
void  WM_SetTransState(WObj *pWin, unsigned State);
void  WM_ValidateWindow(WObj *pWin);
int   WM_GetInvalidRect(WObj *pWin, RECT *pRect);
void  WM_SetStayOnTop(WObj *pWin, int OnOff);
int   WM_GetStayOnTop(WObj *pWin);
void  WM_SetAnchor(WObj *pWin, uint16_t AnchorFlags);

/* Move/resize windows */
void WM_MoveWindow(WObj *pWin, int dx, int dy);
void WM_ResizeWindow(WObj *pWin, int dx, int dy);
void WM_MoveTo(WObj *pWin, int x, int y);
void WM_MoveChildTo(WObj *pWin, int x, int y);
void WM_SetSize(WObj *pWin, int XSize, int YSize);
int  WM_SetXSize(WObj *pWin, int xSize);
int  WM_SetYSize(WObj *pWin, int ySize);
int  WM_CreateTimer(WObj *pWin, int UserID, int Period, int Mode); /* not to be documented (may change in future version) */
void WM_DeleteTimer(WObj *pWin, int UserId); /* not to be documented (may change in future version) */

/* Diagnostics */
int WM_GetNumWindows(void);
int WM_GetNumInvalidWindows(void);

/* Set (new) callback function */
WM_CALLBACK *WM_SetCallback(WObj *Win, WM_CALLBACK *cb);

/* Get size/origin of a window */
RECT WM_GetClientRect();
RECT WM_GetInsideRect();
RECT WM_GetInsideRect(WObj *pWin);

WObj *WM_GetPrevSibling(WObj *pWin);

WObj *WM_GetClientWindow(WObj *pObj);

/* Change Z-Order of windows */
void WM_BringToBottom(WObj *pWin);
void WM_BringToTop(WObj *pWin);

/* Desktop */
void WM_SetDesktopColor(RGBC Color);
WObj *WM_GetDesktopWindow(void);

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

void      WM_NotifyParent(WObj *pWin, int Notification);

WM_PARAM  WM_DefaultProc(WObj *pWin, int MsgId, WM_PARAM Data);

WObj *WM_SetFocusOnNextChild(WObj *pParent);     /* Set the focus to the next child */
WObj *WM_SetFocusOnPrevChild(WObj *pParent);     /* Set the focus to the previous child */

/* Scroll functions */
void WM_GetInsideRectExScrollbar(WObj *pWin, RECT *pRect); /* not to be documented (may change in future version) */
WObj *WM_GetScrollPartner(WObj *pWin);
bool WM_SetScrollbarH(WObj *pWin, int OnOff); /* not to be documented (may change in future version) */
bool WM_SetScrollbarV(WObj *pWin, int OnOff); /* not to be documented (may change in future version) */
void WM_GetScrollState(WObj *pObj, WM_SCROLL_STATE *pScrollState);

bool WM_HandlePID(void);
void WM_ForEachDesc(WObj *pWin, WM_tfForEach *pcb, void *pData);

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
	WM_CALLBACK *cb; /* ptr to notification callback */
	WObj *pNextLin, *pNext,
		 *pParent, *pFirstChild;
	uint16_t Status; /* Some status flags */

#pragma region Window list
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
	static bool IsWindow(WObj *pWin) { return pWin ? pWin->IsWindow() : false; }
#pragma endregion

public:

	auto GetFlags() const { return Status; }

	WM_PARAM Require(uint16_t MsgId, WM_PARAM Data = 0)
	{ return cb ? cb(this, MsgId, Data) : (WM_PARAM)0; }
	WM_PARAM Require(uint16_t MsgId, WM_PARAM Data = 0) const
	{ return const_cast<WObj *>(this)->Require(MsgId, Data); }

	auto FirstChild() { return pFirstChild; }
	auto FirstChild() const { return pFirstChild; }
	auto Parent() { return pParent; }
	auto Parent() const { return pParent; }
	auto NextSibling() { return pNext; }
	auto NextSibling() const { return pNext; }

#pragma region Coordinate

	auto GetRect() const { return Rect; }
	
	auto GetOrg() const { return Rect.LeftTop(); }
	auto GetOrgX() const { return Rect.x0; }
	auto GetOrgY() const { return Rect.y0; }

	auto GetSize() const { return Rect.Size(); }
	auto GetSizeX() const { return Rect.XSize(); }
	auto GetSizeY() const { return Rect.YSize(); }

	RECT GetClientRect() const { return{ 0, Rect.Dist() }; }

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
			WM_MoveWindow(this, d.x, d.y);
			return;
		}
		/* make sure at least a part of the windows stays inside of its parent */
		auto Rect = GetRect() + d,
			 RectParent = Parent()->GetRect() - MinVisibility;
		if (RectParent <= Rect)
			WM_MoveWindow(this, d.x, d.y);
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
#pragma endregion

#pragma region Visibility

	bool IsVisible() const { return Status & WC_VISIBLE; }
	void ShowWindow();
	void HideWindow();

#pragma endregion

	bool IsEnabled() const { return !(Status & WC_DISABLED); }

};

WObj *WM_Screen2Win(POINT Pos, WObj *pStop = nullptr) {
	return WObj::pWinFirst->Screen2Win(Pos, pStop);
}

}

WObj *WObj::pWinFirst = nullptr;

WObj *WObj::pWinCapture = nullptr;
bool  WObj::WM__CaptureReleaseAuto = false;
POINT WObj::WM__CapturePoint = { 0, 0 };
WObj *WObj::pWinFocus = nullptr;
