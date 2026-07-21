#include "WIDGET.h"
#include "MENU.h"
#include "BUTTON.h"

#include "FRAMEWIN.h"
#include "FRAMEWIN_Private.h"

FRAMEWIN_Obj::Properties FRAMEWIN_Obj::DefaultProps;

static int16_t FRAMEWIN__MinVisibility = 5;

static void _OnTouch(FRAMEWIN_Obj *pObj, const GUI_PID_STATE *pState) {
	if (pState) {  /* Something happened in our area (pressed or released) */
		if (pState->Pressed) {
			if (!(pObj->Flags & FRAMEWIN_CF_ACTIVE))
				WM_SetFocus(pObj);
			WM_BringToTop(pObj);
			if (pObj->Flags & FRAMEWIN_CF_MOVEABLE)
				WM_SetCaptureMove(pObj, pState, FRAMEWIN__MinVisibility);
		}
	}
}
static int _OnKey(FRAMEWIN_Obj *pObj, const WM_KEY_INFO *pInfo) {
	if (pInfo->PressedCnt > 0) { /* Key pressed? */
		int Key = pInfo->Key;
		switch (Key) {
			case GUI_KEY_TAB:
				pObj->hFocussedChild = WM_SetFocusOnNextChild(pObj);
				return 1;
		}
	}
	return 0;
}
/*********************************************************************
*
*       _OnPaint  (Frame)
*
*/
static void _OnPaint(FRAMEWIN_Obj *pObj) {
	WM_HWIN hWin = WM_GetActiveWindow();
	const char *pText = nullptr;
	auto xsize = WM_GetWindowSizeX(hWin);
	auto ysize = WM_GetWindowSizeY(hWin);
	auto BorderSize = pObj->Props.BorderSize;
	POSITIONS Pos;
	FRAMEWIN__CalcPositions(pObj, &Pos);
	GUI_RECT r{
		Pos.rClient.x0,
		Pos.rTitleText.y0,
		Pos.rClient.x1,
		Pos.rTitleText.y1
	};
	/* Perform computations */
	auto Index = (pObj->Flags & FRAMEWIN_CF_ACTIVE) ? 1 : 0;
	if (pObj->pText)
		pText = pObj->pText;
	Pos.rTitleText.y0++;
	Pos.rTitleText.x0++;
	Pos.rTitleText.x1--;
	GUI_SetFont(pObj->Props.pFont);
	GUI_RECT rText;
	GUI__CalcTextRect(pText, &Pos.rTitleText, &rText, pObj->Props.Align);
	auto y0 = Pos.TitleHeight + BorderSize;
	/* Draw Title */
	GUI_SetBkColor(pObj->Props.aBarColor[Index]);
	GUI_SetColor(pObj->Props.aTextColor[Index]);
	WIDGET__FillStringInRect(pText, r, Pos.rTitleText, rText);
	/* Draw Frame */
	GUI_SetColor(pObj->Props.FrameColor);
	GUI_FillRect({ 0, 0, xsize - 1, BorderSize - 1 });
	GUI_FillRect({ 0, 0, Pos.rClient.x0 - 1, ysize - 1 });
	GUI_FillRect({ Pos.rClient.x1 + 1, 0, xsize - 1, ysize - 1 });
	GUI_FillRect({ 0, Pos.rClient.y1 + 1, xsize - 1, ysize - 1 });
	GUI_FillRect({ 0, y0, xsize - 1, y0 + pObj->Props.IBorderSize - 1});
	/* Draw the 3D effect (if configured) */
	if (pObj->Props.BorderSize >= 2)
		WIDGET__EFFECT_DrawUpRect(pObj, WM_GetClientRect()); /* pObj->pEffect->pfDrawUp(); */
}
/*********************************************************************
*
*       _OnChildHasFocus
*
* Function:
*   A child has received or lost the focus.
*   The basic idea is to make sure the framewindow is active if a
*   descendant has the focus.
*   If the focus travels from one desc. to an other, there is no need
*   to make the framewindow inactive and active again.
*   Avoiding this complicates the code a litlle, but avoids flicker
*   and waste of CPU load.
*
*/
static void _OnChildHasFocus(FRAMEWIN_Obj *pObj, const WM_NOTIFY_CHILD_HAS_FOCUS_INFO *pInfo) {
	if (pInfo) {
		if (WM__IsAncestorOrSelf(pInfo->hNew, pObj)) /* A child has received the focus, Framewindow needs to be activated */
			FRAMEWIN_SetActive(pObj, 1);
		else { /* A child has lost the focus, we need to deactivate */
			FRAMEWIN_SetActive(pObj, 0);
			/* Remember the child which had the focus so we can reactive this child */
			if (WM__IsAncestor(pInfo->hOld, pObj))
				pObj->hFocussedChild = pInfo->hOld;
		}
	}
}
static int _HandleResizeable(WM_HWIN hWin, int MsgId, WM_PARAM Data);
static WM_PARAM _FRAMEWIN_Callback(WM_HWIN hWin, int MsgId, WM_PARAM Data) {
	auto pObj = (FRAMEWIN_Obj *)hWin;
	if (pObj->Flags & FRAMEWIN_CF_RESIZEABLE) 
		if (_HandleResizeable(hWin, MsgId, Data))
			return 0;
	switch (MsgId) {
		case WM_HANDLE_DIALOG_STATUS:
			if (Data) /* set pointer to Dialog status */
				pObj->pDialogStatus = (WM_DIALOG_STATUS *)Data;
			return (WM_PARAM)pObj->pDialogStatus;
		case WM_PAINT:
			_OnPaint(pObj);
			return 0;
		case WM_TOUCH:
			_OnTouch(pObj, (const GUI_PID_STATE *)Data);
			return 0;
		case WM_KEY:
			if (_OnKey(pObj, (const WM_KEY_INFO *)Data))
				return 0;
			break;
		case WM_GET_INSIDE_RECT: {
			POSITIONS Pos;
			FRAMEWIN__CalcPositions(pObj, &Pos);
			*(GUI_RECT *)Data = Pos.rClient;
			return 0;
		}
		case WM_GET_CLIENT_WINDOW: /* return handle to client window. For most windows, there is no seperate client window, so it is the same handle */
			return (WM_PARAM)pObj->hClient;
		case WM_NOTIFY_PARENT: {
			const WM_NOTIFY_INFO *pInfo = (const WM_NOTIFY_INFO *)Data;
			if (pInfo->Notification == WM_NOTIFICATION_RELEASED) {
				int Id = WM_GetId(pInfo->hWinSrc);
				switch (Id) {
					case GUI_ID_CLOSE:
						WM_DeleteWindow(pObj);
						break;
					case GUI_ID_MAXIMIZE:
						if (pObj->Flags & FRAMEWIN_CF_MAXIMIZED)
							FRAMEWIN_Restore(pObj);
						else
							FRAMEWIN_Maximize(pObj);
						break;
					case GUI_ID_MINIMIZE:
						if (pObj->Flags & FRAMEWIN_CF_MINIMIZED)
							FRAMEWIN_Restore(pObj);
						else
							FRAMEWIN_Minimize(pObj);
						break;
				}
			}
			return 0;
		}
		case WM_SET_FOCUS: /* We have received or lost focus */
			if (Data) {
				if (WM_IsWindow(pObj->hFocussedChild))
					WM_SetFocus(pObj->hFocussedChild);
				else
					pObj->hFocussedChild = WM_SetFocusOnNextChild(pObj->hClient);
				FRAMEWIN_SetActive(pObj, 1);
				return 0; /* Focus could be accepted */
			}
			else
				FRAMEWIN_SetActive(pObj, 0);
			return 0;
		case WM_TOUCH_CHILD:
			/* If a child of this framewindow has been touched and the frame window was not active,
			   the framewindow will receive the focus.
			 */
			if (!(pObj->Flags & FRAMEWIN_CF_ACTIVE)) {
				const GUI_PID_STATE *pState = (const GUI_PID_STATE *)Data;
				if (pState) /* Message may not have a valid pointer (moved out) ! */
					if (pState->Pressed)
						WM_SetFocus(pObj);
			}
			break;
		case WM_NOTIFY_CHILD_HAS_FOCUS:
			_OnChildHasFocus(pObj, (const WM_NOTIFY_CHILD_HAS_FOCUS_INFO *)Data);
			break;
		case WM_DELETE:
			GUI_DEBUG_LOG("FRAMEWIN: _FRAMEWIN_Callback(WM_DELETE)\n");
			GUI_ALLOC_Free(pObj->pText);
			pObj->pText = nullptr;
			break;
	}
	/* Let widget handle the standard messages */
	if (!WIDGET_HandleActive(pObj, MsgId, &Data))
		return Data;
	return WM_DefaultProc(hWin, MsgId, Data);
}
static WM_PARAM FRAMEWIN__cbClient(WM_HWIN hWin, int MsgId, WM_PARAM Data) {
	auto pParent = (FRAMEWIN_Obj *)WM_GetParent(hWin);
	WM_CALLBACK *cb = pParent->cb;
	switch (MsgId) {
		case WM_PAINT:
			if (pParent->Props.ClientColor != RGB_INVALID_COLOR) {
				GUI_SetBkColor(pParent->Props.ClientColor);
				GUI_Clear();
			}
			/* Give the user callback  a chance to draw.
			 * Note that we can not run into the bottom part, as this passes the parents handle
			  */
			 if (cb)
				 cb(hWin, MsgId, Data);
			return 0;
		case WM_SET_FOCUS:
			if (Data) { /* Focus received */
				if (pParent->hFocussedChild && pParent->hFocussedChild != hWin)
					WM_SetFocus(pParent->hFocussedChild);
				else
					pParent->hFocussedChild = WM_SetFocusOnNextChild(hWin);
				return 0; /* Focus change accepted */
			}
			return 0;
		case WM_GET_ACCEPT_FOCUS:
			WIDGET_HandleActive(pParent, MsgId, &Data);
			return Data;
		case WM_GET_BKCOLOR:
			return pParent->Props.ClientColor;
		case WM_GET_INSIDE_RECT:        /* This should not be passed to parent ... (We do not want parents coordinates)*/
		case WM_GET_ID:                 /* This should not be passed to parent ... (Possible recursion problem)*/
		case WM_GET_CLIENT_WINDOW:      /* return handle to client window. For most windows, there is no seperate client window, so it is the same handle */
			return WM_DefaultProc(hWin, MsgId, Data);
	}
	/* Call user callback. Note that the user callback gets the handle of the Framewindow itself, NOT the Client. */
	if (cb)
		return cb(pParent, MsgId, Data);
	return WM_DefaultProc(hWin, MsgId, Data);
}
int FRAMEWIN__CalcTitleHeight(FRAMEWIN_Obj *pObj) {
	int r = 0;
	if (pObj->State & FRAMEWIN_CF_TITLEVIS) {
		r = pObj->Props.TitleHeight;
		if (r == 0) {
			r = 2 + GUI_GetYSizeOfFont(pObj->Props.pFont);
		}
	}
	return r;
}
void FRAMEWIN__CalcPositions(FRAMEWIN_Obj *pObj, POSITIONS *pPos) {
	WM_Obj *pChild;
	int BorderSize = pObj->Props.BorderSize;
	int xsize = WM__GetWindowSizeX(pObj);
	int ysize = WM__GetWindowSizeY(pObj);
	int IBorderSize = 0;
	if (pObj->State & FRAMEWIN_CF_TITLEVIS)
		IBorderSize = pObj->Props.IBorderSize;
	int TitleHeight = FRAMEWIN__CalcTitleHeight(pObj);
	int MenuHeight = 0;
	if (pObj->hMenu)
		MenuHeight = WM_GetWindowSizeY(pObj->hMenu);
	pPos->TitleHeight = TitleHeight;
	pPos->MenuHeight = MenuHeight;
	/* Set object properties accordingly */
	pPos->rClient.x0 = BorderSize;
	pPos->rClient.x1 = xsize - BorderSize - 1;
	pPos->rClient.y0 = BorderSize + IBorderSize + TitleHeight + MenuHeight;
	pPos->rClient.y1 = ysize - BorderSize - 1;
	/* Calculate title rect */
	pPos->rTitleText.x0 = BorderSize;
	pPos->rTitleText.x1 = xsize - BorderSize - 1;
	pPos->rTitleText.y0 = BorderSize;
	pPos->rTitleText.y1 = BorderSize + TitleHeight - 1;
	/* Iterate over all children */
	for (pChild = (WM_Obj *)pObj->hFirstChild; pChild; pChild = (WM_Obj *)pChild->hNext) {
		int x0 = pChild->Rect.x0 - pObj->Rect.x0;
		int x1 = pChild->Rect.x1 - pObj->Rect.x0;
		int y0 = pChild->Rect.y0 - pObj->Rect.y0;
		if (y0 == BorderSize) {
			if (pChild->Status & WM_SF_ANCHOR_RIGHT) {
				if (x0 <= pPos->rTitleText.x1)
					pPos->rTitleText.x1 = x0 - 1;
			}
			else if (x1 >= pPos->rTitleText.x0)
				pPos->rTitleText.x0 = x1 + 1;
		}
	}
}
void FRAMEWIN__UpdatePositions(FRAMEWIN_Obj *pObj) {
	/* Move client window accordingly */
	if (pObj->hClient || pObj->hMenu) {
		POSITIONS Pos;
		FRAMEWIN__CalcPositions(pObj, &Pos);
		if (pObj->hClient) {
			WM_MoveChildTo(pObj->hClient, Pos.rClient.x0, Pos.rClient.y0);
			WM_SetSize(pObj->hClient,
					   Pos.rClient.x1 - Pos.rClient.x0 + 1,
					   Pos.rClient.y1 - Pos.rClient.y0 + 1);
		}
		if (pObj->hMenu) 
			WM_MoveChildTo(pObj->hMenu, Pos.rClient.x0, Pos.rClient.y0 - Pos.MenuHeight);
	}
}
FRAMEWIN_Handle FRAMEWIN_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
								  int WinFlags, int ExFlags, int Id, const char *pTitle, WM_CALLBACK *cb) {
	FRAMEWIN_Handle hObj;
	/* Create the window */
	WinFlags |= WM_CF_LATE_CLIP;    /* Always use late clipping since widget is optimized for it. */
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, _FRAMEWIN_Callback,
								  sizeof(FRAMEWIN_Obj) - sizeof(WM_Obj));
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		POSITIONS Pos;
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE | FRAMEWIN_CF_TITLEVIS);
		/* init member variables */
		pObj->Props = FRAMEWIN_Obj::DefaultProps;
		pObj->cb = cb;
		pObj->Flags = ExFlags;
		pObj->hFocussedChild = 0;
		pObj->hMenu = 0;
		FRAMEWIN__CalcPositions(pObj, &Pos);
		pObj->hClient = WM_CreateWindowAsChild(Pos.rClient.x0, Pos.rClient.y0,
											   Pos.rClient.x1 - Pos.rClient.x0 + 1,
											   Pos.rClient.y1 - Pos.rClient.y0 + 1,
											   hObj,
											   WM_CF_ANCHOR_RIGHT | WM_CF_ANCHOR_LEFT | WM_CF_ANCHOR_TOP | WM_CF_ANCHOR_BOTTOM | WM_CF_SHOW | WM_CF_LATE_CLIP,
											   FRAMEWIN__cbClient, 0);
		/* Normally we disable memory devices for the frame window:
		 * The frame window does not flicker, and not using memory devices is usually faster.
		 * You can still use memory by explicitly specifying the flag
		 */
		if ((WinFlags & (WM_CF_MEMDEV | (WM_CF_MEMDEV_ON_REDRAW))) == 0) {
			WM_DisableMemdev(hObj);
		}
		FRAMEWIN_SetText(hObj, pTitle);
	}
	return hObj;
}
void FRAMEWIN_SetText(FRAMEWIN_Handle hObj, const char *s) {
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		if (GUI__SetText(&pObj->pText, s))
			WM_Invalidate(hObj);
	}
}
void FRAMEWIN_SetTextAlign(FRAMEWIN_Handle hObj, int Align) {
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		if (pObj->Props.Align != Align) {
			pObj->Props.Align = Align;
			WM_Invalidate(hObj);
		}
	}
}
void FRAMEWIN_SetMoveable(FRAMEWIN_Handle hObj, int State) {
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		if (State)
			pObj->Flags |= FRAMEWIN_CF_MOVEABLE;
		else
			pObj->Flags &= ~FRAMEWIN_CF_MOVEABLE;
	}
}
void FRAMEWIN_SetActive(FRAMEWIN_Handle hObj, int State) {
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		if (State && !(pObj->Flags & FRAMEWIN_CF_ACTIVE)) {
			pObj->Flags |= FRAMEWIN_CF_ACTIVE;
			WM_Invalidate(hObj);
		}
		else if (!State && (pObj->Flags & FRAMEWIN_CF_ACTIVE)) {
			pObj->Flags &= ~FRAMEWIN_CF_ACTIVE;
			WM_Invalidate(hObj);
		}
	}
}

void FRAMEWIN_AddMenu(FRAMEWIN_Handle hObj, WM_HWIN hMenu) {
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		if (pObj) {
			int TitleHeight, BorderSize, IBorderSize = 0;
			int x0, y0, xSize;
			TitleHeight = FRAMEWIN__CalcTitleHeight(pObj);
			BorderSize = pObj->Props.BorderSize;
			if (pObj->State & FRAMEWIN_CF_TITLEVIS) {
				IBorderSize = pObj->Props.IBorderSize;
			}
			x0 = BorderSize;
			y0 = BorderSize + TitleHeight + IBorderSize;
			xSize = WM__GetWindowSizeX(pObj);
			xSize -= BorderSize * 2;
			pObj->hMenu = hMenu;
			if (pObj->cb) {
				MENU_SetOwner(hMenu, pObj->hClient);
			}
			MENU_Attach(hMenu, hObj, x0, y0, xSize, 0, 0);
			WM_SetAnchor(hMenu, WM_CF_ANCHOR_LEFT | WM_CF_ANCHOR_RIGHT);
			FRAMEWIN__UpdatePositions(pObj);
			WM_Invalidate(hObj);
		}
	}
}


FRAMEWIN_Handle FRAMEWIN_Create(const char *pText, WM_CALLBACK *cb, int Flags,
								int x0, int y0, int xsize, int ysize) {
	return FRAMEWIN_CreateEx(x0, y0, xsize, ysize, nullptr, Flags, 0, 0, pText, cb);
}
FRAMEWIN_Handle FRAMEWIN_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
									   const char *pText, WM_CALLBACK *cb, int Flags) {
	return FRAMEWIN_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, 0, pText, cb);
}
FRAMEWIN_Handle FRAMEWIN_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent,
										int x0, int y0, WM_CALLBACK *cb) {
	FRAMEWIN_Handle hObj;
	hObj = FRAMEWIN_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							 hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id, pCreateInfo->pName, cb);
	return hObj;
}

PCFONT FRAMEWIN_GetFont(FRAMEWIN_Handle hObj) {
	PCFONT r = nullptr;
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		r = pObj->Props.pFont;
		WM_Invalidate(hObj);
	}
	return r;
}
int FRAMEWIN_GetTitleHeight(FRAMEWIN_Handle hObj) {
	int r = 0;
	POSITIONS Pos;
	/* Move client window accordingly */
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		FRAMEWIN__CalcPositions(pObj, &Pos);
		r = pObj->Props.TitleHeight;
		if (r == 0) {
			r = Pos.TitleHeight;
		}
	}
	return r;
}
int FRAMEWIN_GetBorderSize(FRAMEWIN_Handle hObj) {
	int r = 0;
	/* Move client window accordingly */
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		r = pObj->Props.BorderSize;
	}
	return r;
}

int FRAMEWIN_IsMinimized(FRAMEWIN_Handle hObj) {
	int r = 0;
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		r = (pObj->Flags & FRAMEWIN_CF_MINIMIZED) ? 1 : 0;
	}
	return r;
}
int FRAMEWIN_IsMaximized(FRAMEWIN_Handle hObj) {
	int r = 0;
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		r = (pObj->Flags & FRAMEWIN_CF_MAXIMIZED) ? 1 : 0;
	}
	return r;
}

static void _InvalidateButton(FRAMEWIN_Obj *pObj, int Id) {
	WM_Obj *pChild;
	for (pChild = (WM_Obj *)pObj->hFirstChild; pChild; pChild = (WM_Obj *)pChild->hNext)
		if (WM_GetId(pChild) == Id)
			WM_Invalidate(pChild);
}
static void _RestoreMinimized(FRAMEWIN_Obj *pObj) {
	/* When window was minimized, restore it */
	if (pObj->Flags & FRAMEWIN_CF_MINIMIZED) {
		int OldHeight = 1 + pObj->Rect.y1 - pObj->Rect.y0;
		int NewHeight = 1 + pObj->rRestore.y1 - pObj->rRestore.y0;
		WM_ResizeWindow(pObj, 0, NewHeight - OldHeight);
		WM_ShowWindow(pObj->hClient);
		WM_ShowWindow(pObj->hMenu);
		FRAMEWIN__UpdatePositions(pObj);
		pObj->Flags &= ~FRAMEWIN_CF_MINIMIZED;
		_InvalidateButton(pObj, GUI_ID_MINIMIZE);
	}
}
static void _RestoreMaximized(FRAMEWIN_Obj *pObj) {
	/* When window was maximized, restore it */
	if (pObj->Flags & FRAMEWIN_CF_MAXIMIZED) {
		GUI_RECT r = pObj->rRestore;
		WM_MoveTo(pObj, r.x0, r.y0);
		WM_SetSize(pObj, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1);
		FRAMEWIN__UpdatePositions(pObj);
		pObj->Flags &= ~FRAMEWIN_CF_MAXIMIZED;
		_InvalidateButton(pObj, GUI_ID_MAXIMIZE);
	}
}
static void _MinimizeFramewin(FRAMEWIN_Obj *pObj) {
	_RestoreMaximized(pObj);
	/* When window is not minimized, minimize it */
	if ((pObj->Flags & FRAMEWIN_CF_MINIMIZED) == 0) {
		int OldHeight = pObj->Rect.y1 - pObj->Rect.y0 + 1;
		int NewHeight = FRAMEWIN__CalcTitleHeight(pObj) + pObj->pEffect->EffectSize * 2 + 2;
		pObj->rRestore = pObj->Rect;
		WM_HideWindow(pObj->hClient);
		WM_HideWindow(pObj->hMenu);
		WM_ResizeWindow(pObj, 0, NewHeight - OldHeight);
		FRAMEWIN__UpdatePositions(pObj);
		pObj->Flags |= FRAMEWIN_CF_MINIMIZED;
		_InvalidateButton(pObj, GUI_ID_MINIMIZE);
	}
}
static void _MaximizeFramewin(FRAMEWIN_Obj *pObj) {
	_RestoreMinimized(pObj);
	/* When window is not maximized, maximize it */
	if (!(pObj->Flags & FRAMEWIN_CF_MAXIMIZED)) {
		auto pParent = (WM_Obj *)pObj->hParent;
		GUI_RECT r = pParent->Rect;
		if (!pParent->hParent) {
			r.x1 = LCD_GetXSize();
			r.y1 = LCD_GetYSize();
		}
		pObj->rRestore = pObj->Rect;
		WM_MoveTo(pObj, r.x0, r.y0);
		WM_SetSize(pObj, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1);
		FRAMEWIN__UpdatePositions(pObj);
		pObj->Flags |= FRAMEWIN_CF_MAXIMIZED;
		_InvalidateButton(pObj, GUI_ID_MAXIMIZE);
	}
}
void FRAMEWIN_Minimize(FRAMEWIN_Handle hObj) {
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		_MinimizeFramewin(pObj);
	}
}
void FRAMEWIN_Maximize(FRAMEWIN_Handle hObj) {
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		_MaximizeFramewin(pObj);
	}
}
void FRAMEWIN_Restore(FRAMEWIN_Handle hObj) {
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		_RestoreMinimized(pObj);
		_RestoreMaximized(pObj);
	}
}

void FRAMEWIN_SetBorderSize(FRAMEWIN_Handle hObj, unsigned Size) {
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		int OldHeight = FRAMEWIN__CalcTitleHeight(pObj);
		int OldSize = pObj->Props.BorderSize;
		int Diff = Size - OldSize;
		for (auto pChild = (WM_Obj *)pObj->hFirstChild; pChild; pChild = (WM_Obj *)pChild->hNext) {
			GUI_RECT r = pChild->Rect + GUI_POINT{-pObj->Rect.x0, -pObj->Rect.y0};
			if (r.y0 == pObj->Props.BorderSize  && r.y1 - r.y0 + 1 == OldHeight) {
				if (pChild->Status & WM_SF_ANCHOR_RIGHT)
					WM_MoveWindow(pChild, -Diff, Diff);
				else
					WM_MoveWindow(pChild, Diff, Diff);
			}
		}
		pObj->Props.BorderSize = Size;
		FRAMEWIN__UpdatePositions(pObj);
		WM_Invalidate(hObj);
	}
}

void FRAMEWIN_SetBarColor(FRAMEWIN_Handle hObj, unsigned Index, RGBC Color) {
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		if (Index < GUI_COUNTOF(pObj->Props.aBarColor)) {
			pObj->Props.aBarColor[Index] = Color;
			WM_Invalidate(hObj);
		}
	}
}
void FRAMEWIN_SetTextColor(FRAMEWIN_Handle hObj, RGBC Color) {
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		for (int i = 0; i < GUI_COUNTOF(pObj->Props.aTextColor); i++)
			pObj->Props.aTextColor[i] = Color;
		WM_Invalidate(hObj);
	}
}
void FRAMEWIN_SetTextColorEx(FRAMEWIN_Handle hObj, unsigned Index, RGBC Color) {
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		if (Index < GUI_COUNTOF(pObj->Props.aTextColor)) {
			pObj->Props.aTextColor[Index] = Color;
			WM_Invalidate(hObj);
		}
	}
}
void FRAMEWIN_SetClientColor(FRAMEWIN_Handle hObj, RGBC Color) {
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		if (pObj->Props.ClientColor != Color) {
			pObj->Props.ClientColor = Color;
			WM_Invalidate(pObj->hClient);
		}
	}
}

void FRAMEWIN_SetFont(FRAMEWIN_Handle hObj, PCFONT pFont) {
	if (hObj) {
		auto pObj = (FRAMEWIN_Obj *)hObj;
		int OldHeight = FRAMEWIN__CalcTitleHeight(pObj);
		pObj->Props.pFont = pFont;
		FRAMEWIN__UpdatePositions(pObj);
		FRAMEWIN__UpdateButtons(pObj, OldHeight);
		WM_Invalidate(hObj);
	}
}

#define FRAMEWIN_REACT_BORDER 3
#define FRAMEWIN_MINSIZE_X    20
#define FRAMEWIN_MINSIZE_Y    20
#define FRAMEWIN_RESIZE_X     (1<<0)
#define FRAMEWIN_RESIZE_Y     (1<<1)
#define FRAMEWIN_REPOS_X      (1<<2)
#define FRAMEWIN_REPOS_Y      (1<<3)
#define FRAMEWIN_MOUSEOVER    (1<<4)
#define FRAMEWIN_RESIZE       (FRAMEWIN_RESIZE_X | FRAMEWIN_RESIZE_Y | FRAMEWIN_REPOS_X | FRAMEWIN_REPOS_Y)
static int _CaptureX, _CaptureY;
static int _CaptureFlags;
#if GUI_SUPPORT_CURSOR
static PCCURSOR _pOldCursor;
#endif
#if GUI_SUPPORT_CURSOR
static const RGBC _ColorsCursor[] = {
	RGB_RED,RGB_BLACK,RGB_WHITE
};
static const GUI_LOGPALETTE _PalCursor = {
  3,	/* number of entries */
  1, 	/* Has transparency */
  &_ColorsCursor[0]
};
static const uint8_t _acResizeCursorH[] = {
________,________,________,________,________,
________,________,________,________,________,
________,________,________,________,________,
________,________,________,________,________,
________,XX______,________,XX______,________,
______XX,XX______,________,XXXX____,________,
____XXoo,XX______,________,XXooXX__,________,
__XXoooo,XXXXXXXX,XXXXXXXX,XXooooXX,________,
XXoooooo,oooooooo,oooooooo,oooooooo,XX______,
__XXoooo,XXXXXXXX,XXXXXXXX,XXooooXX,________,
____XXoo,XX______,________,XXooXX__,________,
______XX,XX______,________,XXXX____,________,
________,XX______,________,XX______,________,
________,________,________,________,________,
________,________,________,________,________,
________,________,________,________,________,
________,________,________,________,________,
};
static CBITMAP _bmResizeCursorH = {
 17,  /* XSize */
 17,  /* YSize */
 5,   /* BytesPerLine */
 2,   /* BitsPerPixel */
 _acResizeCursorH,    /* Pointer to picture data (indices) */
 &_PalCursor          /* Pointer to palette */
};
static CCURSOR _ResizeCursorH = {
  &_bmResizeCursorH, 8, 8
};
static const uint8_t _acResizeCursorV[] = {
________,________,XX______,________,________,
________,______XX,ooXX____,________,________,
________,____XXoo,ooooXX__,________,________,
________,__XXoooo,ooooooXX,________,________,
________,XXXXXXXX,ooXXXXXX,XX______,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
________,XXXXXXXX,ooXXXXXX,XX______,________,
________,__XXoooo,ooooooXX,________,________,
________,____XXoo,ooooXX__,________,________,
________,______XX,ooXX____,________,________,
________,________,XX______,________,________,
};
static CBITMAP _bmResizeCursorV = {
 17,  /* XSize */
 17,  /* YSize */
 5,   /* BytesPerLine */
 2,   /* BitsPerPixel */
 _acResizeCursorV,    /* Pointer to picture data (indices) */
 &_PalCursor          /* Pointer to palette */
};
static CCURSOR _ResizeCursorV = {
  &_bmResizeCursorV, 8, 8
};
static const uint8_t _acResizeCursorDD[] = {
________,________,________,________,________,
________,________,________,________,________,
____XXXX,XXXXXXXX,________,________,________,
____XXoo,ooooXX__,________,________,________,
____XXoo,ooXX____,________,________,________,
____XXoo,XXooXX__,________,________,________,
____XXXX,__XXooXX,________,________,________,
____XX__,____XXoo,XX______,________,________,
________,______XX,ooXX____,________,________,
________,________,XXooXX__,____XX__,________,
________,________,__XXooXX,__XXXX__,________,
________,________,____XXoo,XXooXX__,________,
________,________,______XX,ooooXX__,________,
________,________,____XXoo,ooooXX__,________,
________,________,__XXXXXX,XXXXXX__,________,
________,________,________,________,________,
________,________,________,________,________,
};
static CBITMAP _bmResizeCursorDD = {
 17,  /* XSize */
 17,  /* YSize */
 5,   /* BytesPerLine */
 2,   /* BitsPerPixel */
 _acResizeCursorDD,   /* Pointer to picture data (indices) */
 &_PalCursor          /* Pointer to palette */
};
static CCURSOR _ResizeCursorDD = {
  &_bmResizeCursorDD, 8, 8
};
static const uint8_t _acResizeCursorDU[] = {
________,________,________,________,________,
________,________,________,________,________,
________,________,__XXXXXX,XXXXXX__,________,
________,________,____XXoo,ooooXX__,________,
________,________,______XX,ooooXX__,________,
________,________,____XXoo,XXooXX__,________,
________,________,__XXooXX,__XXXX__,________,
________,________,XXooXX__,____XX__,________,
________,______XX,ooXX____,________,________,
____XX__,____XXoo,XX______,________,________,
____XXXX,__XXooXX,________,________,________,
____XXoo,XXooXX__,________,________,________,
____XXoo,ooXX____,________,________,________,
____XXoo,ooooXX__,________,________,________,
____XXXX,XXXXXXXX,________,________,________,
________,________,________,________,________,
________,________,________,________,________,
};
static CBITMAP _bmResizeCursorDU = {
 17,  /* XSize */
 17,  /* YSize */
 5,   /* BytesPerLine */
 2,   /* BitsPerPixel */
 _acResizeCursorDU,   /* Pointer to picture data (indices) */
 &_PalCursor          /* Pointer to palette */
};
static CCURSOR _ResizeCursorDU = {
  &_bmResizeCursorDU, 8, 8
};
#endif
#if GUI_SUPPORT_CURSOR
static void _SetResizeCursor(int Mode) {
	PCCURSOR pNewCursor = nullptr;
	if (Mode) {
		int Direction;
		Direction = Mode & (FRAMEWIN_RESIZE_X | FRAMEWIN_RESIZE_Y);
		if (Direction == FRAMEWIN_RESIZE_X) {
			pNewCursor = &_ResizeCursorH;
		}
		else if (Direction == FRAMEWIN_RESIZE_Y) {
			pNewCursor = &_ResizeCursorV;
		}
		else {
			Direction = Mode & (FRAMEWIN_REPOS_X | FRAMEWIN_REPOS_Y);
			if ((Direction == (FRAMEWIN_REPOS_X | FRAMEWIN_REPOS_Y)) || !Direction) {
				pNewCursor = &_ResizeCursorDD;
			}
			else {
				pNewCursor = &_ResizeCursorDU;
			}
		}
	}
	if (pNewCursor) {
		PCCURSOR pOldCursor;
		pOldCursor = GUI_CURSOR_Select(pNewCursor);
		if (_pOldCursor == nullptr) {
			_pOldCursor = pOldCursor;
		}
	}
	else if (_pOldCursor) {
		GUI_CURSOR_Select(_pOldCursor);
		_pOldCursor = nullptr;
	}
}
#endif
static void _SetCapture(FRAMEWIN_Handle hWin, int x, int y, int Mode) {
	if ((_CaptureFlags & FRAMEWIN_REPOS_X) == 0) {
		_CaptureX = x;
	}
	if ((_CaptureFlags & FRAMEWIN_REPOS_Y) == 0) {
		_CaptureY = y;
	}
	if (Mode) {
		if (!WM_HasCaptured(hWin))
			WM_SetCapture(hWin, 0);
#if GUI_SUPPORT_CURSOR
		_SetResizeCursor(Mode);
#endif
		if (Mode & FRAMEWIN_MOUSEOVER)
			Mode = 0;
		_CaptureFlags = Mode | FRAMEWIN_MOUSEOVER;
	}
}
static void _ChangeWindowPosSize(FRAMEWIN_Handle hWin, int *px, int *py) {
	int dx = 0, dy = 0;
	GUI_RECT Rect = WM_GetClientRect(hWin);
	/* Calculate new size of window */
	if (_CaptureFlags & FRAMEWIN_RESIZE_X)
		dx = (_CaptureFlags & FRAMEWIN_REPOS_X) ? _CaptureX - *px : *px - _CaptureX;
	if (_CaptureFlags & FRAMEWIN_RESIZE_Y)
		dy = (_CaptureFlags & FRAMEWIN_REPOS_Y) ? _CaptureY - *py : *py - _CaptureY;
	/* Check the minimal size of window */
	if ((Rect.x1 + dx + 1) < FRAMEWIN_MINSIZE_X) {
		dx = FRAMEWIN_MINSIZE_X - Rect.x1 - 1;
		*px = _CaptureX + dx;
	}
	if ((Rect.y1 + dy + 1) < FRAMEWIN_MINSIZE_Y) {
		dy = FRAMEWIN_MINSIZE_Y - Rect.y1 - 1;
		*py = _CaptureY + dy;
	}
	/* Set new window position */
	if (_CaptureFlags & FRAMEWIN_REPOS_X) {
		WM_MoveWindow(hWin, -dx, 0);
	}
	if (_CaptureFlags & FRAMEWIN_REPOS_Y) {
		WM_MoveWindow(hWin, 0, -dy);
	}
	/* Set new window size */
	WM_ResizeWindow(hWin, dx, dy);
}
static int _CheckBorderX(int x, int x1, int Border) {
	int Mode = 0;
	if (x > (x1 - Border)) {
		Mode = FRAMEWIN_RESIZE_X;
	}
	else if (x < (Border)) {
		Mode = FRAMEWIN_RESIZE_X | FRAMEWIN_REPOS_X;
	}
	return Mode;
}
static int _CheckBorderY(int y, int y1, int Border) {
	int Mode = 0;
	if (y > (y1 - Border)) {
		Mode = FRAMEWIN_RESIZE_Y;
	}
	else if (y < (Border)) {
		Mode = FRAMEWIN_RESIZE_Y | FRAMEWIN_REPOS_Y;
	}
	return Mode;
}
static int _CheckReactBorder(FRAMEWIN_Handle hWin, int x, int y) {
	int Mode = 0;
	GUI_RECT r = WM_GetClientRect(hWin);
	if ((x >= 0) && (y >= 0) && (x <= r.x1) && (y <= r.y1)) {
		Mode |= _CheckBorderX(x, r.x1, FRAMEWIN_REACT_BORDER);
		if (Mode) {
			Mode |= _CheckBorderY(y, r.y1, 4 * FRAMEWIN_REACT_BORDER);
		}
		else {
			Mode |= _CheckBorderY(y, r.y1, FRAMEWIN_REACT_BORDER);
			if (Mode) {
				Mode |= _CheckBorderX(x, r.x1, 4 * FRAMEWIN_REACT_BORDER);
			}
		}
	}
	return Mode;
}
static int _OnTouchResize(FRAMEWIN_Handle hWin, const GUI_PID_STATE *pState) {
	if (pState) {  /* Something happened in our area (pressed or released) */
		int x = pState->x, y = pState->y;
		int Mode = _CheckReactBorder(hWin, x, y);
		if (pState->Pressed == 1) {
			if (_CaptureFlags & FRAMEWIN_RESIZE) {
				_ChangeWindowPosSize(hWin, &x, &y);
				_SetCapture(hWin, x, y, 0);
				return 1;
			}
			else if (Mode) {
				WM_SetFocus(hWin);
				WM_BringToTop(hWin);
				_SetCapture(hWin, x, y, Mode);
				return 1;
			}
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
			else if (_CaptureFlags) {
				WM_ReleaseCapture();
				return 1;
			}
#endif
		}
		else if (WM_HasCaptured(hWin)) {
			_CaptureFlags &= ~(FRAMEWIN_RESIZE);
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
			if (!Mode)
#endif
			{
				WM_ReleaseCapture();
			}
			return 1;
		}
	}
	return 0;
}
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
static int _ForwardMouseOverMsg(FRAMEWIN_Handle hWin, const GUI_PID_STATE *pState) {
	WM_HWIN hBelow;
	GUI_PID_STATE StateBelow;
	StateBelow.x = pState->x + WM_GetWindowOrgX(hWin);
	StateBelow.y = pState->y + WM_GetWindowOrgY(hWin);
	hBelow = WM_Screen2hWin(StateBelow.x, StateBelow.y);
	if (hBelow && (hBelow != hWin)) {
		StateBelow.x -= WM_GetWindowOrgX(hBelow);
		StateBelow.y -= WM_GetWindowOrgY(hBelow);
		WM__SendMessage(hBelow, WM_MOUSEOVER, (WM_PARAM)&StateBelow);
		return 1;
	}
	return 0;
}
#endif
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
static int _OnMouseOver(FRAMEWIN_Handle hWin, const GUI_PID_STATE *pState) {
	if (pState) {
		int x = pState->x, y = pState->y;
		int Mode = _CheckReactBorder(hWin, x, y);
		if (Mode) {
			if (_ForwardMouseOverMsg(hWin, pState) == 0)
				_SetCapture(hWin, x, y, Mode | FRAMEWIN_MOUSEOVER);
			return 1;
		}
		else if (WM_HasCaptured(hWin)) {
			if ((_CaptureFlags & FRAMEWIN_RESIZE) == 0) {
				WM_ReleaseCapture();
				_ForwardMouseOverMsg(hWin, pState);
			}
			return 1;
		}
	}
	return 0;
}
#endif
static int _HandleResizeable(WM_HWIN hWin, int MsgId, WM_PARAM Data) {
	if (WM_HasCaptured(hWin) && _CaptureFlags == 0)
		return 0;
	if (FRAMEWIN_IsMinimized(hWin) || FRAMEWIN_IsMaximized(hWin))
		return 0;
	switch (MsgId) {
		case WM_TOUCH:
			return _OnTouchResize(hWin, (const GUI_PID_STATE *)Data);
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
		case WM_MOUSEOVER:
			return _OnMouseOver(hWin, (const GUI_PID_STATE *)Data);
#endif
		case WM_CAPTURE_RELEASED:
#if GUI_SUPPORT_CURSOR
			_SetResizeCursor(0);
#endif
			_CaptureFlags = 0;
			return 1;
	}
	return 0;
}
void FRAMEWIN_SetResizeable(FRAMEWIN_Handle hObj, int State) {
	auto pObj = (FRAMEWIN_Obj *)hObj;
	if (pObj) {
		if (State)
			pObj->Flags |= FRAMEWIN_CF_RESIZEABLE;
		else
			pObj->Flags &= ~FRAMEWIN_CF_RESIZEABLE;
	}
}

int FRAMEWIN_SetTitleHeight(FRAMEWIN_Handle hObj, int Height) {
	int r = 0;
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		int OldHeight;
		pObj = (FRAMEWIN_Obj *)hObj;
		r = pObj->Props.TitleHeight;
		if (Height != r) {
			OldHeight = FRAMEWIN__CalcTitleHeight(pObj);
			pObj->Props.TitleHeight = Height;
			FRAMEWIN__UpdatePositions(pObj);
			FRAMEWIN__UpdateButtons(pObj, OldHeight);
			WM_Invalidate(hObj);
		}
	}
	return r;
}

static void _ShowHideButtons(FRAMEWIN_Obj *pObj) {
	WM_Obj *pChild;
	int y0;
	for (pChild = (WM_Obj *)pObj->hFirstChild; pChild; pChild = (WM_Obj *)pChild->hNext) {
		y0 = pChild->Rect.y0 - pObj->Rect.y0;
		if ((y0 == pObj->Props.BorderSize) && (pChild != pObj->hClient)) {
			if (pObj->State & FRAMEWIN_CF_TITLEVIS) {
				WM_ShowWindow(pChild);
			}
			else {
				WM_HideWindow(pChild);
			}
		}
	}
}
void FRAMEWIN_SetTitleVis(FRAMEWIN_Handle hObj, int Show) {
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		int State;
		pObj = (FRAMEWIN_Obj *)hObj;
		State = pObj->State;
		if (Show) {
			State |= FRAMEWIN_CF_TITLEVIS;
		}
		else {
			State &= ~FRAMEWIN_CF_TITLEVIS;
		}
		if (pObj->State != State) {
			pObj->State = State;
			FRAMEWIN__UpdatePositions(pObj);
			_ShowHideButtons(pObj);
			if (pObj->Flags & FRAMEWIN_CF_MINIMIZED) {
				if (State & FRAMEWIN_CF_TITLEVIS) {
					WM_ShowWindow(hObj);
				}
				else {
					WM_HideWindow(hObj);
				}
			}
			WM_Invalidate(hObj);
		}
	}
}

/*********************************************************************
*
*       FRAMEWIN__UpdateButtons
*
* Purpose:
*   Adjust button positions & size.
*   This is required after resizing the title bar
*/
void FRAMEWIN__UpdateButtons(FRAMEWIN_Obj *pObj, int OldHeight) {
	int TitleHeight = FRAMEWIN__CalcTitleHeight(pObj);
	int Diff = TitleHeight - OldHeight;
	if (Diff) {
		WM_Obj *pLeft, *pRight, *pChild;
		GUI_RECT r;
		int xLeft, xRight, n;
		n = 0;
		do {
			pLeft = pRight = nullptr;
			xLeft = GUI_XMAX;
			xRight = GUI_XMIN;
			for (pChild = (WM_Obj *)pObj->hFirstChild; pChild; pChild = (WM_Obj *)pChild->hNext) {
				r = pChild->Rect;
				r += GUI_POINT{-pObj->Rect.x0, -pObj->Rect.y0};
				if ((r.y0 == pObj->Props.BorderSize) && ((r.y1 - r.y0 + 1) == OldHeight)) {
					if (pChild->Status & WM_SF_ANCHOR_RIGHT) {
						if (r.x1 > xRight) {
							pRight = pChild;
							xRight = r.x0;
						}
					}
					else {
						if (r.x0 < xLeft) {
							pLeft = pChild;
							xLeft = r.x0;
						}
					}
				}
			}
			if (pLeft) {
				WM_ResizeWindow(pLeft, Diff, Diff);
				WM_MoveWindow(pLeft, n * Diff, 0);
			}
			if (pRight) {
				WM_ResizeWindow(pRight, Diff, Diff);
				WM_MoveWindow(pRight, -(n * Diff), 0);
			}
			n++;
		} while (pLeft || pRight);
	}
}

WM_HWIN FRAMEWIN_AddButton(FRAMEWIN_Handle hObj, int Flags, int Off, int Id) {
	WM_HWIN r = 0;
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		POSITIONS Pos;
		int Size = FRAMEWIN_GetTitleHeight(hObj);
		int BorderSize = FRAMEWIN_GetBorderSize(hObj);
		int WinFlags, x;

		pObj = (FRAMEWIN_Obj *)hObj;
		FRAMEWIN__CalcPositions(pObj, &Pos);
		if (Flags & FRAMEWIN_BUTTON_RIGHT) {
			x = Pos.rTitleText.x1 - (Size - 1) - Off;
			WinFlags = WM_CF_SHOW | WM_CF_ANCHOR_RIGHT;
		}
		else {
			x = Pos.rTitleText.x0 + Off;
			WinFlags = WM_CF_SHOW;
		}
		r = BUTTON_CreateAsChild(x, BorderSize, Size, Size, hObj, Id, WinFlags);
		BUTTON_SetFocussable(r, 0);

	}
	return r;
}

static void _DrawClose(void) {
	auto r = WM_GetInsideRect();
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	int Size = r.x1 - r.x0 - 2;
	WM_ITERATE_START(&r) {
		for (int i = 2; i < Size; i++) {
			LCD_DrawHLine(r.x0 + i, r.y0 + i, r.x0 + i + 1);
			LCD_DrawHLine(r.x1 - i - 1, r.y0 + i, r.x1 - i);
		}
	} WM_ITERATE_END();
}
WM_HWIN FRAMEWIN_AddCloseButton(FRAMEWIN_Handle hObj, int Flags, int Off) {
	WM_HWIN hButton = FRAMEWIN_AddButton(hObj, Flags, Off, GUI_ID_CLOSE);
	BUTTON_SetSelfDraw(hButton, 0, &_DrawClose);
	return hButton;
}

static void _PaintMax(void) {
	auto r = WM_GetInsideRect();
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	WM_ITERATE_START(&r) {
		LCD_DrawHLine(r.x0 + 1, r.y0 + 1, r.x1 - 1);
		LCD_DrawHLine(r.x0 + 1, r.y0 + 2, r.x1 - 1);
		LCD_DrawHLine(r.x0 + 1, r.y1 - 1, r.x1 - 1);
		LCD_DrawVLine(r.x0 + 1, r.y0 + 1, r.y1 - 1);
		LCD_DrawVLine(r.x1 - 1, r.y0 + 1, r.y1 - 1);
	} WM_ITERATE_END();
}
static void _DrawRestoreClose(void) {
	auto r = WM_GetInsideRect();
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	int Size = ((r.x1 - r.x0 + 1) << 1) / 3;
	WM_ITERATE_START(&r) {
		LCD_DrawHLine(r.x1 - Size, r.y0 + 1, r.x1 - 1);
		LCD_DrawHLine(r.x1 - Size, r.y0 + 2, r.x1 - 1);
		LCD_DrawHLine(r.x0 + Size, r.y0 + Size, r.x1 - 1);
		LCD_DrawVLine(r.x1 - Size, r.y0 + 1, r.y1 - Size);
		LCD_DrawVLine(r.x1 - 1, r.y0 + 1, r.y0 + Size);
		LCD_DrawHLine(r.x0 + 1, r.y1 - Size, r.x0 + Size);
		LCD_DrawHLine(r.x0 + 1, r.y1 - Size + 1, r.x0 + Size);
		LCD_DrawHLine(r.x0 + 1, r.y1 - 1, r.x0 + Size);
		LCD_DrawVLine(r.x0 + 1, r.y1 - Size, r.y1 - 1);
		LCD_DrawVLine(r.x0 + Size, r.y1 - Size, r.y1 - 1);
	} WM_ITERATE_END();
}
static void _DrawMax(void) {
	auto pObj = (FRAMEWIN_Obj *)WM_GetParent(WM_GetActiveWindow());
	if (pObj->Flags & FRAMEWIN_CF_MAXIMIZED)
		_DrawRestoreClose();
	else
		_PaintMax();
}
WM_HWIN FRAMEWIN_AddMaxButton(FRAMEWIN_Handle hObj, int Flags, int Off) {
	WM_HWIN hButton = FRAMEWIN_AddButton(hObj, Flags, Off, GUI_ID_MAXIMIZE);
	BUTTON_SetSelfDraw(hButton, 0, &_DrawMax);
	return hButton;
}

static void _PaintMin(void) {
	auto r = WM_GetInsideRect();
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	int Size = (r.x1 - r.x0 + 1) >> 1;
	WM_ITERATE_START(&r) {
		for (int i = 1; i < Size; i++)
			LCD_DrawHLine(r.x0 + i, r.y1 - i - (Size >> 1), r.x1 - i);
	} WM_ITERATE_END();
}
static void _DrawRestoreMin(void) {
	auto r = WM_GetInsideRect();
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	int Size = (r.x1 - r.x0 + 1) >> 1;
	WM_ITERATE_START(&r) {
		for (int i = 1; i < Size; i++)
			LCD_DrawHLine(r.x0 + i, r.y0 + i + (Size >> 1), r.x1 - i);
	} WM_ITERATE_END();
}
static void _DrawMin(void) {
	auto pObj = (FRAMEWIN_Obj *)WM_GetParent(WM_GetActiveWindow());
	if (pObj->Flags & FRAMEWIN_CF_MINIMIZED)
		_DrawRestoreMin();
	else
		_PaintMin();
}
WM_HWIN FRAMEWIN_AddMinButton(FRAMEWIN_Handle hObj, int Flags, int Off) {
	WM_HWIN hButton = FRAMEWIN_AddButton(hObj, Flags, Off, GUI_ID_MINIMIZE);
	BUTTON_SetSelfDraw(hButton, 0, &_DrawMin);
	return hButton;
}
