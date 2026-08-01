module;
#include "DIALOG_Intern.h"

export module TUX.Widget.Frame;

import TUX.Widget;
import TUX.Widget.Button;
import TUX.Widget.Menu;

static int16_t FRAMEWIN__MinVisibility = 5;

#define FRAMEWIN_REACT_BORDER 3
#define FRAMEWIN_MINSIZE_X    20
#define FRAMEWIN_MINSIZE_Y    20
#define FRAMEWIN_RESIZE_X     (1<<0)
#define FRAMEWIN_RESIZE_Y     (1<<1)
#define FRAMEWIN_REPOS_X      (1<<2)
#define FRAMEWIN_REPOS_Y      (1<<3)
#define FRAMEWIN_MOUSEOVER    (1<<4)
#define FRAMEWIN_RESIZE       (FRAMEWIN_RESIZE_X | FRAMEWIN_RESIZE_Y | FRAMEWIN_REPOS_X | FRAMEWIN_REPOS_Y)

export {

constexpr uint16_t FRAMEWIN_CF_ACTIVE     = 1 << 3;
constexpr uint16_t FRAMEWIN_CF_MOVEABLE   = 1 << 4;
constexpr uint16_t FRAMEWIN_CF_RESIZEABLE = 1 << 5;
constexpr uint16_t FRAMEWIN_CF_TITLEVIS   = 1 << 6;
constexpr uint16_t FRAMEWIN_CF_MINIMIZED  = 1 << 7;
constexpr uint16_t FRAMEWIN_CF_MAXIMIZED  = 1 << 8;
constexpr uint16_t FRAMEWIN_BUTTON_RIGHT   = 1 << 0;
constexpr uint16_t FRAMEWIN_BUTTON_LEFT    = 1 << 1;

struct FRAMEWIN_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &FontProp13_1 };
		RGBC aTextColor[2]{
			/* Lose focused */	RGB_BLACK,
			/* Focused */		RGB_WHITE
		};
		RGBC aBarColor[2]{
			/* Lose focused */	RGB_GRAYL(0x80),
			/* Focused */		RGB_BLUEL(0x80)
		};
		RGBC ClientColor{ RGB_GRAYL(0xE4) };
		RGBC FrameColor{ RGB_GRAYL(0xAA) };
		uint16_t TitleHeight{ 20 };
		uint16_t BorderSize{ 2 };
		uint16_t IBorderSize{ 1 };
		TEXTALIGN Align{ TEXTALIGN_VCENTER };
		uint8_t Border{ 0 };
	} static DefaultProps;
	Properties Props;
	WM_CALLBACK *cb;
	WM_Obj *hClient;
	MENU_Obj *pMenu;
	char *pText;
	GUI_RECT rRestore;
	uint16_t Flags;
	WM_Obj *hFocussedChild; /* Handle to focussed child .. default none (0) */
	WM_DIALOG_STATUS *pDialogStatus;

	struct POSITIONS {
		int16_t TitleHeight;
		int16_t MenuHeight;
		GUI_RECT rClient;
		GUI_RECT rTitleText;
	};

	int _CalcTitleHeight() {
		return State & FRAMEWIN_CF_TITLEVIS ? 
			Props.TitleHeight ?
				Props.TitleHeight :
				2 + Props.pFont->SizeY() :
			0;
	}
	void _CalcPositions(POSITIONS *pPos) {
		WM_Obj *pChild;
		int BorderSize = this->Props.BorderSize;
		auto size = GetSize();
		int IBorderSize = 0;
		if (this->State & FRAMEWIN_CF_TITLEVIS)
			IBorderSize = this->Props.IBorderSize;
		int TitleHeight = _CalcTitleHeight();
		int MenuHeight = 0;
		if (pMenu)
			MenuHeight = pMenu->GetSizeY();
		pPos->TitleHeight = TitleHeight;
		pPos->MenuHeight = MenuHeight;
		/* Set object properties accordingly */
		pPos->rClient.x0 = BorderSize;
		pPos->rClient.x1 = size.x - BorderSize - 1;
		pPos->rClient.y0 = BorderSize + IBorderSize + TitleHeight + MenuHeight;
		pPos->rClient.y1 = size.y - BorderSize - 1;
		/* Calculate title rect */
		pPos->rTitleText.x0 = BorderSize;
		pPos->rTitleText.x1 = size.x - BorderSize - 1;
		pPos->rTitleText.y0 = BorderSize;
		pPos->rTitleText.y1 = BorderSize + TitleHeight - 1;
		/* Iterate over all children */
		for (pChild = this->pFirstChild; pChild; pChild = pChild->pNext) {
			int x0 = pChild->Rect.x0 - this->Rect.x0;
			int x1 = pChild->Rect.x1 - this->Rect.x0;
			int y0 = pChild->Rect.y0 - this->Rect.y0;
			if (y0 == BorderSize) {
				if (pChild->Status & WC_ANCHOR_RIGHT) {
					if (x0 <= pPos->rTitleText.x1)
						pPos->rTitleText.x1 = x0 - 1;
				}
				else if (x1 >= pPos->rTitleText.x0)
					pPos->rTitleText.x0 = x1 + 1;
			}
		}
	}
	void _UpdatePositions() {
		/* Move client window accordingly */
		if (this->hClient || this->pMenu) {
			POSITIONS Pos;
			_CalcPositions(&Pos);
			if (this->hClient) {
				WM_MoveChildTo(this->hClient, Pos.rClient.x0, Pos.rClient.y0);
				WM_SetSize(this->hClient,
						   Pos.rClient.x1 - Pos.rClient.x0 + 1,
						   Pos.rClient.y1 - Pos.rClient.y0 + 1);
			}
			if (this->pMenu)
				WM_MoveChildTo(this->pMenu, Pos.rClient.x0, Pos.rClient.y0 - Pos.MenuHeight);
		}
	}
	void _UpdateButtons(int OldHeight) {
		int TitleHeight = _CalcTitleHeight();
		int Diff = TitleHeight - OldHeight;
		if (Diff) {
			WM_Obj *pLeft, *pRight, *pChild;
			int xLeft, xRight, n = 0;
			do {
				pLeft = pRight = nullptr;
				xLeft = GUI_XMAX;
				xRight = GUI_XMIN;
				for (pChild = this->pFirstChild; pChild; pChild = pChild->pNext) {
					auto r = pChild->Rect - this->Rect.LeftTop();
					if ((r.y0 == this->Props.BorderSize) && ((r.y1 - r.y0 + 1) == OldHeight)) {
						if (pChild->Status & WC_ANCHOR_RIGHT) {
							if (r.x1 > xRight) {
								pRight = pChild;
								xRight = r.x0;
							}
						}
						else if (r.x0 < xLeft) {
							pLeft = pChild;
							xLeft = r.x0;
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
	void _OnTouch(const GUI_PID_STATE *pState) {
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (pState->Pressed) {
				if (!(this->Flags & FRAMEWIN_CF_ACTIVE))
					WM_SetFocus(this);
				WM_BringToTop(this);
				if (this->Flags & FRAMEWIN_CF_MOVEABLE)
					WM_SetCaptureMove(this, pState, FRAMEWIN__MinVisibility);
			}
		}
	}
	int _OnKey(const WM_KEY_INFO *pInfo) {
		if (pInfo->PressedCnt > 0) { /* Key pressed? */
			int Key = pInfo->Key;
			switch (Key) {
				case GUI_KEY_TAB:
					this->hFocussedChild = WM_SetFocusOnNextChild(this);
					return 1;
			}
		}
		return 0;
	}
	void _OnPaint() {
		const char *pText = nullptr;
		auto size = GetSize();
		auto BorderSize = this->Props.BorderSize;
		POSITIONS Pos;
		_CalcPositions(&Pos);
		GUI_RECT r{
			Pos.rClient.x0,
			Pos.rTitleText.y0,
			Pos.rClient.x1,
			Pos.rTitleText.y1
		};
		/* Perform computations */
		auto Index = (this->Flags & FRAMEWIN_CF_ACTIVE) ? 1 : 0;
		if (this->pText)
			pText = this->pText;
		Pos.rTitleText.y0++;
		Pos.rTitleText.x0++;
		Pos.rTitleText.x1--;
		GUI_SetFont(this->Props.pFont);
		GUI_RECT rText;
		GUI__CalcTextRect(pText, &Pos.rTitleText, &rText, this->Props.Align);
		auto y0 = Pos.TitleHeight + BorderSize;
		/* Draw Title */
		GUI.SetBkColor(this->Props.aBarColor[Index]);
		GUI.SetColor(this->Props.aTextColor[Index]);
		WIDGET__FillStringInRect(pText, r, Pos.rTitleText, rText);
		/* Draw Frame */
		GUI.SetColor(this->Props.FrameColor);
		GUI_FillRect({ 0, 0, size.x - 1, BorderSize - 1 });
		GUI_FillRect({ 0, 0, Pos.rClient.x0 - 1, size.y - 1 });
		GUI_FillRect({ Pos.rClient.x1 + 1, 0, size.x - 1, size.y - 1 });
		GUI_FillRect({ 0, Pos.rClient.y1 + 1, size.x - 1, size.y - 1 });
		GUI_FillRect({ 0, y0, size.x - 1, y0 + this->Props.IBorderSize - 1 });
		/* Draw the 3D effect (if configured) */
		if (this->Props.BorderSize >= 2)
			DrawUp();
	}
	void _OnChildHasFocus(const WM_NOTIFY_CHILD_HAS_FOCUS_INFO *pInfo) {
		if (pInfo) {
			if (WM__IsAncestorOrSelf(pInfo->pNew, this)) /* A child has received the focus, Framewindow needs to be activated */
				SetActive(1);
			else { /* A child has lost the focus, we need to deactivate */
				SetActive(0);
				/* Remember the child which had the focus so we can reactive this child */
				if (WM__IsAncestor(pInfo->pOld, this))
					this->hFocussedChild = pInfo->pOld;
			}
		}
	}

	static WM_PARAM _Callback(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (FRAMEWIN_Obj *)hWin;
		if (pObj->Flags & FRAMEWIN_CF_RESIZEABLE)
			if (pObj->_HandleResizeable(MsgId, Data))
				return 0;
		switch (MsgId) {
			case WM_HANDLE_DIALOG_STATUS:
				if (Data) /* set pointer to Dialog status */
					pObj->pDialogStatus = (WM_DIALOG_STATUS *)Data;
				return (WM_PARAM)pObj->pDialogStatus;
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_TOUCH:
				pObj->_OnTouch((const GUI_PID_STATE *)Data);
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const WM_KEY_INFO *)Data))
					return 0;
				break;
			case WM_GET_INSIDE_RECT: {
				POSITIONS Pos;
				pObj->_CalcPositions(&Pos);
				*(GUI_RECT *)Data = Pos.rClient;
				return 0;
			}
			case WM_GET_CLIENT_WINDOW: /* return handle to client window. For most windows, there is no seperate client window, so it is the same handle */
				return (WM_PARAM)pObj->hClient;
			case WM_NOTIFY_PARENT: {
				auto pInfo = (const WM_NOTIFY_INFO *)Data;
				auto pWinSrc = pInfo->pWinSrc;
				if (pInfo->Notification == WM_NOTIFICATION_RELEASED) {
					int Id = pWinSrc->GetID();
					switch (Id) {
						case GUI_ID_CLOSE:
							WM_DeleteWindow(pObj);
							break;
						case GUI_ID_MAXIMIZE:
							if (pObj->Flags & FRAMEWIN_CF_MAXIMIZED)
								pObj->Restore();
							else
								pObj->Maximize();
							break;
						case GUI_ID_MINIMIZE:
							if (pObj->Flags & FRAMEWIN_CF_MINIMIZED)
								pObj->Restore();
							else
								pObj->Minimize();
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
					pObj->SetActive(1);
					return 0; /* Focus could be accepted */
				}
				else
					pObj->SetActive(0);
				return 0;
			case WM_TOUCH_CHILD:
				/* If a child of this framewindow has been touched and the frame window was not active,
				   the framewindow will receive the focus.
				 */
				if (!(pObj->Flags & FRAMEWIN_CF_ACTIVE)) {
					auto pState = (const GUI_PID_STATE *)Data;
					if (pState) /* Message may not have a valid pointer (moved out) ! */
						if (pState->Pressed)
							WM_SetFocus(pObj);
				}
				break;
			case WM_NOTIFY_CHILD_HAS_FOCUS:
				pObj->_OnChildHasFocus((const WM_NOTIFY_CHILD_HAS_FOCUS_INFO *)Data);
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
	static WM_PARAM _cbClient(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
		auto pParent = (FRAMEWIN_Obj *)WM_GetParent(hWin);
		auto cb = pParent->cb;
		switch (MsgId) {
			case WM_PAINT:
				if (pParent->Props.ClientColor != RGB_INVALID_COLOR) {
					GUI.SetBkColor(pParent->Props.ClientColor);
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

public:
	void SetText(const char *s) {
		if (GUI__SetText(&this->pText, s))
			WM_Invalidate(this);
	}
	void SetTextAlign(int Align) {
		if (Props.Align != Align) {
			Props.Align = Align;
			WM_Invalidate(this);
		}
	}
	void SetMoveable(int State) {
		if (State)
			this->Flags |= FRAMEWIN_CF_MOVEABLE;
		else
			this->Flags &= ~FRAMEWIN_CF_MOVEABLE;
	}
	void SetActive(int State) {
		if (State && !(this->Flags & FRAMEWIN_CF_ACTIVE)) {
			this->Flags |= FRAMEWIN_CF_ACTIVE;
			WM_Invalidate(this);
		}
		else if (!State && (this->Flags & FRAMEWIN_CF_ACTIVE)) {
			this->Flags &= ~FRAMEWIN_CF_ACTIVE;
			WM_Invalidate(this);
		}
	}
	void AddMenu(MENU_Obj *pMenu) {
		int TitleHeight, BorderSize, IBorderSize = 0;
		int x0, y0, xSize;
		TitleHeight = _CalcTitleHeight();
		BorderSize = Props.BorderSize;
		if (this->State & FRAMEWIN_CF_TITLEVIS) {
			IBorderSize = Props.IBorderSize;
		}
		x0 = BorderSize;
		y0 = BorderSize + TitleHeight + IBorderSize;
		xSize = GetSizeX() - BorderSize * 2;
		this->pMenu = pMenu;
		if (this->cb) {
			pMenu->SetOwner(this->hClient);
		}
		pMenu->Attach(this, x0, y0, xSize, 0, 0);
		WM_SetAnchor(pMenu, WC_ANCHOR_LEFT | WC_ANCHOR_RIGHT);
		_UpdatePositions();
		WM_Invalidate(this);
	}

	PCFONT GetFont() {
		PCFONT r = nullptr;
		r = Props.pFont;
		WM_Invalidate(this);

		return r;
	}
	int GetTitleHeight() {
		int r = 0;
		POSITIONS Pos;
		/* Move client window accordingly */
		_CalcPositions(&Pos);
		r = Props.TitleHeight;
		if (r == 0) {
			r = Pos.TitleHeight;
		}

		return r;
	}
	int GetBorderSize() {
		int r = 0;
		/* Move client window accordingly */
		r = Props.BorderSize;

		return r;
	}

	bool IsMinimized() {
		return this->Flags & FRAMEWIN_CF_MINIMIZED;
	}
	bool IsMaximized() {
		return this->Flags & FRAMEWIN_CF_MAXIMIZED;
	}

	void _InvalidateButton(int Id) {
		WM_Obj *pChild;
		for (pChild = this->pFirstChild; pChild; pChild = pChild->pNext)
			if (pChild->GetID() == Id)
				WM_Invalidate(pChild);
	}
	void _RestoreMinimized() {
		/* When window was minimized, restore it */
		if (this->Flags & FRAMEWIN_CF_MINIMIZED) {
			int OldHeight = 1 + this->Rect.y1 - this->Rect.y0;
			int NewHeight = 1 + this->rRestore.y1 - this->rRestore.y0;
			WM_ResizeWindow(this, 0, NewHeight - OldHeight);
			hClient->ShowWindow();
			pMenu->ShowWindow();
			_UpdatePositions();
			this->Flags &= ~FRAMEWIN_CF_MINIMIZED;
			_InvalidateButton(GUI_ID_MINIMIZE);
		}
	}
	void _RestoreMaximized() {
		/* When window was maximized, restore it */
		if (this->Flags & FRAMEWIN_CF_MAXIMIZED) {
			GUI_RECT r = this->rRestore;
			WM_MoveTo(this, r.x0, r.y0);
			WM_SetSize(this, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1);
			_UpdatePositions();
			this->Flags &= ~FRAMEWIN_CF_MAXIMIZED;
			_InvalidateButton(GUI_ID_MAXIMIZE);
		}
	}
	void _MinimizeFramewin() {
		_RestoreMaximized();
		/* When window is not minimized, minimize it */
		if ((this->Flags & FRAMEWIN_CF_MINIMIZED) == 0) {
			int OldHeight = this->Rect.y1 - this->Rect.y0 + 1;
			int NewHeight = _CalcTitleHeight() + this->EffectSize() * 2 + 2;
			this->rRestore = this->Rect;
			hClient->HideWindow();
			pMenu->HideWindow();
			WM_ResizeWindow(this, 0, NewHeight - OldHeight);
			_UpdatePositions();
			this->Flags |= FRAMEWIN_CF_MINIMIZED;
			_InvalidateButton(GUI_ID_MINIMIZE);
		}
	}
	void _MaximizeFramewin() {
		_RestoreMinimized();
		/* When window is not maximized, maximize it */
		if (!(this->Flags & FRAMEWIN_CF_MAXIMIZED)) {
			auto pParent = this->pParent;
			GUI_RECT r = pParent->Rect;
			if (!pParent->pParent) {
				r.x1 = LCD_GetXSize();
				r.y1 = LCD_GetYSize();
			}
			this->rRestore = this->Rect;
			WM_MoveTo(this, r.x0, r.y0);
			WM_SetSize(this, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1);
			_UpdatePositions();
			this->Flags |= FRAMEWIN_CF_MAXIMIZED;
			_InvalidateButton(GUI_ID_MAXIMIZE);
		}
	}

	void Minimize() {
		_MinimizeFramewin();
	}
	void Maximize() {
		_MaximizeFramewin();
	}
	void Restore() {
		_RestoreMinimized();
		_RestoreMaximized();
	}

	void SetBorderSize(unsigned Size) {
		int OldHeight = _CalcTitleHeight();
		int OldSize = Props.BorderSize;
		int Diff = Size - OldSize;
		for (auto pChild = this->pFirstChild; pChild; pChild = pChild->pNext) {
			auto r = pChild->Rect - this->Rect.LeftTop();
			if (r.y0 == Props.BorderSize && r.y1 - r.y0 + 1 == OldHeight) {
				if (pChild->Status & WC_ANCHOR_RIGHT)
					WM_MoveWindow(pChild, -Diff, Diff);
				else
					WM_MoveWindow(pChild, Diff, Diff);
			}
		}
		Props.BorderSize = Size;
		_UpdatePositions();
		WM_Invalidate(this);
	}

	void SetBarColor(unsigned Index, RGBC Color) {
		if (Index < GUI_COUNTOF(Props.aBarColor)) {
			Props.aBarColor[Index] = Color;
			WM_Invalidate(this);
		}
	}
	void SetTextColor(RGBC Color) {
		for (int i = 0; i < GUI_COUNTOF(Props.aTextColor); i++)
			Props.aTextColor[i] = Color;
		WM_Invalidate(this);
	}
	void SetTextColorEx(unsigned Index, RGBC Color) {
		if (Index < GUI_COUNTOF(Props.aTextColor)) {
			Props.aTextColor[Index] = Color;
			WM_Invalidate(this);
		}
	}
	void SetClientColor(RGBC Color) {
		if (Props.ClientColor != Color) {
			Props.ClientColor = Color;
			WM_Invalidate(hClient);
		}
	}

	void SetFont(PCFONT pFont) {
		int OldHeight = _CalcTitleHeight();
		Props.pFont = pFont;
		_UpdatePositions();
		_UpdateButtons(OldHeight);
		WM_Invalidate(this);
	}

#if GUI_SUPPORT_CURSOR
	static void _SetResizeCursor(int Mode) {
		static PCCURSOR _pOldCursor;
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
		 &GUI_CursorPal       /* Pointer to palette */
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
		 &GUI_CursorPal       /* Pointer to palette */
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
		 &GUI_CursorPal       /* Pointer to palette */
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
		 &GUI_CursorPal       /* Pointer to palette */
		};
		static CCURSOR _ResizeCursorDU = {
		  &_bmResizeCursorDU, 8, 8
		};
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
	static int _CaptureX, _CaptureY;
	static int _CaptureFlags;
	void _SetCapture(int x, int y, int Mode) {
		if ((_CaptureFlags & FRAMEWIN_REPOS_X) == 0) {
			_CaptureX = x;
		}
		if ((_CaptureFlags & FRAMEWIN_REPOS_Y) == 0) {
			_CaptureY = y;
		}
		if (Mode) {
			if (!WM_HasCaptured(this))
				WM_SetCapture(this, 0);
#if GUI_SUPPORT_CURSOR
			_SetResizeCursor(Mode);
#endif
			if (Mode & FRAMEWIN_MOUSEOVER)
				Mode = 0;
			_CaptureFlags = Mode | FRAMEWIN_MOUSEOVER;
		}
	}
	void _ChangeWindowPosSize(int *px, int *py) {
		int dx = 0, dy = 0;
		GUI_RECT Rect = WM_GetClientRect(this);
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
			WM_MoveWindow(this, -dx, 0);
		}
		if (_CaptureFlags & FRAMEWIN_REPOS_Y) {
			WM_MoveWindow(this, 0, -dy);
		}
		/* Set new window size */
		WM_ResizeWindow(this, dx, dy);
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
	int _CheckReactBorder(int x, int y) {
		int Mode = 0;
		GUI_RECT r = WM_GetClientRect(this);
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
	int _OnTouchResize(const GUI_PID_STATE *pState) {
		if (pState) {  /* Something happened in our area (pressed or released) */
			int x = pState->x, y = pState->y;
			int Mode = _CheckReactBorder(x, y);
			if (pState->Pressed == 1) {
				if (_CaptureFlags & FRAMEWIN_RESIZE) {
					_ChangeWindowPosSize(&x, &y);
					_SetCapture(x, y, 0);
					return 1;
				}
				else if (Mode) {
					WM_SetFocus(this);
					WM_BringToTop(this);
					_SetCapture(x, y, Mode);
					return 1;
				}
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
				else if (_CaptureFlags) {
					WM_ReleaseCapture();
					return 1;
				}
#endif
			}
			else if (WM_HasCaptured(this)) {
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
	int _ForwardMouseOverMsg(const GUI_PID_STATE *pState) {
		GUI_PID_STATE StateBelow = *pState;
		StateBelow += GetOrg();
		auto pBelow = WM_Screen2hWin(StateBelow.x, StateBelow.y);
		if (pBelow && pBelow != this) {
			StateBelow -= pBelow->GetOrg();
			WM__SendMessage(pBelow, WM_MOUSEOVER, (WM_PARAM)&StateBelow);
			return true;
		}
		return false;
	}
	int _OnMouseOver(const GUI_PID_STATE *pState) {
		if (pState) {
			int x = pState->x, y = pState->y;
			int Mode = _CheckReactBorder(x, y);
			if (Mode) {
				if (_ForwardMouseOverMsg(pState) == 0)
					_SetCapture(x, y, Mode | FRAMEWIN_MOUSEOVER);
				return 1;
			}
			else if (WM_HasCaptured(this)) {
				if ((_CaptureFlags & FRAMEWIN_RESIZE) == 0) {
					WM_ReleaseCapture();
					_ForwardMouseOverMsg(pState);
				}
				return 1;
			}
		}
		return 0;
	}
#endif
	int _HandleResizeable(int MsgId, WM_PARAM Data) {
		if (WM_HasCaptured(this) && _CaptureFlags == 0)
			return 0;
		if (IsMinimized() || IsMaximized())
			return 0;
		switch (MsgId) {
			case WM_TOUCH:
				return _OnTouchResize((const GUI_PID_STATE *)Data);
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
			case WM_MOUSEOVER:
				return _OnMouseOver((const GUI_PID_STATE *)Data);
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
	void SetResizeable(int State) {
		if (State)
			Flags |= FRAMEWIN_CF_RESIZEABLE;
		else
			Flags &= ~FRAMEWIN_CF_RESIZEABLE;
	}

	int SetTitleHeight(int Height) {
		int r = 0;
		int OldHeight;
		r = Props.TitleHeight;
		if (Height != r) {
			OldHeight = _CalcTitleHeight();
			Props.TitleHeight = Height;
			_UpdatePositions();
			_UpdateButtons(OldHeight);
			WM_Invalidate(this);
		}

		return r;
	}

	void _ShowHideButtons() {
		WM_Obj *pChild;
		int y0;
		for (pChild = pFirstChild; pChild; pChild = pChild->pNext) {
			y0 = pChild->Rect.y0 - Rect.y0;
			if ((y0 == Props.BorderSize) && (pChild != hClient)) {
				if (State & FRAMEWIN_CF_TITLEVIS) {
					pChild->ShowWindow();
				}
				else {
					pChild->HideWindow();
				}
			}
		}
	}
	void SetTitleVis(int Show) {
		int State = this->State;
		if (Show) {
			State |= FRAMEWIN_CF_TITLEVIS;
		}
		else {
			State &= ~FRAMEWIN_CF_TITLEVIS;
		}
		if (this->State != State) {
			this->State = State;
			_UpdatePositions();
			_ShowHideButtons();
			if (this->Flags & FRAMEWIN_CF_MINIMIZED) {
				if (State & FRAMEWIN_CF_TITLEVIS) {
					ShowWindow();
				}
				else {
					HideWindow();
				}
			}
			WM_Invalidate(this);
		}
	}

	BUTTON_Obj *AddButton(int Flags, int Off, int Id) {
		POSITIONS Pos;
		int Size = GetTitleHeight();
		int BorderSize = GetBorderSize();
		int WinFlags, x;
		_CalcPositions(&Pos);
		if (Flags & FRAMEWIN_BUTTON_RIGHT) {
			x = Pos.rTitleText.x1 - (Size - 1) - Off;
			WinFlags = WC_VISIBLE | WC_ANCHOR_RIGHT;
		}
		else {
			x = Pos.rTitleText.x0 + Off;
			WinFlags = WC_VISIBLE;
		}
		auto r = BUTTON_CreateAsChild(x, BorderSize, Size, Size, this, Id, WinFlags);
		r->SetFocussable(0);
		return r;
	}

	BUTTON_Obj *AddCloseButton(int Flags = FRAMEWIN_BUTTON_RIGHT, int Off = 1) {
		auto pButton = AddButton(Flags, Off, GUI_ID_CLOSE);
		pButton->SetSelfDraw(0, []() {
			auto r = WM_GetInsideRect() + GUI.Off;
			int Size = r.x1 - r.x0 - 2;
			for (int i = 2; i < Size; i++) {
				LCD_DrawHLine(r.x0 + i, r.y0 + i, r.x0 + i + 1);
				LCD_DrawHLine(r.x1 - i - 1, r.y0 + i, r.x1 - i);
			}
		});
		return pButton;
	}

	static void _DrawMax(void) {
		auto pObj = (FRAMEWIN_Obj *)WM_GetParent(WM_GetActiveWindow());
		auto r = WM_GetInsideRect() + GUI.Off;
		if (pObj->Flags & FRAMEWIN_CF_MAXIMIZED) {
			int Size = ((r.x1 - r.x0 + 1) << 1) / 3;
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
		}
		else {
			LCD_DrawHLine(r.x0 + 1, r.y0 + 1, r.x1 - 1);
			LCD_DrawHLine(r.x0 + 1, r.y0 + 2, r.x1 - 1);
			LCD_DrawHLine(r.x0 + 1, r.y1 - 1, r.x1 - 1);
			LCD_DrawVLine(r.x0 + 1, r.y0 + 1, r.y1 - 1);
			LCD_DrawVLine(r.x1 - 1, r.y0 + 1, r.y1 - 1);
		}
	}
	BUTTON_Obj *AddMaxButton(int Flags = FRAMEWIN_BUTTON_RIGHT, int Off = 1) {
		auto pButton = AddButton(Flags, Off, GUI_ID_MAXIMIZE);
		pButton->SetSelfDraw(0, _DrawMax);
		return pButton;
	}

	BUTTON_Obj *AddMinButton(int Flags = FRAMEWIN_BUTTON_RIGHT, int Off = 1) {
		auto pButton = AddButton(Flags, Off, GUI_ID_MINIMIZE);
		pButton->SetSelfDraw(0, [] {
			auto pObj = (FRAMEWIN_Obj *)WM_GetParent(WM_GetActiveWindow());
			auto r = WM_GetInsideRect() + GUI.Off;
			int Size = (r.x1 - r.x0 + 1) >> 1;
			if (pObj->Flags & FRAMEWIN_CF_MINIMIZED) {
				for (int i = 1; i < Size; i++)
					LCD_DrawHLine(r.x0 + i, r.y0 + i + (Size >> 1), r.x1 - i);
			}
			else {
				for (int i = 1; i < Size; i++)
					LCD_DrawHLine(r.x0 + i, r.y1 - i - (Size >> 1), r.x1 - i);
			}
		});
		return pButton;
	}
};

FRAMEWIN_Obj::Properties FRAMEWIN_Obj::DefaultProps;

int FRAMEWIN_Obj::_CaptureX = 0, FRAMEWIN_Obj::_CaptureY = 0;
int FRAMEWIN_Obj::_CaptureFlags = 0;

FRAMEWIN_Obj *FRAMEWIN_CreateEx(int x0, int y0, int xsize, int ysize, WM_Obj *hParent,
								  int WinFlags, int ExFlags, int Id, const char *pTitle, WM_CALLBACK *cb) {
	/* Create the window */
	WinFlags |= WC_LATE_CLIP;    /* Always use late clipping since widget is optimized for it. */
	auto pObj = (FRAMEWIN_Obj *)WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, FRAMEWIN_Obj::_Callback,
								  sizeof(FRAMEWIN_Obj) - sizeof(WM_Obj));
	if (pObj) {
		FRAMEWIN_Obj::POSITIONS Pos;
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE | FRAMEWIN_CF_TITLEVIS);
		/* init member variables */
		pObj->Props = FRAMEWIN_Obj::DefaultProps;
		pObj->cb = cb;
		pObj->Flags = ExFlags;
		pObj->hFocussedChild = 0;
		pObj->pMenu = nullptr;
		pObj->_CalcPositions(&Pos);
		pObj->hClient = WM_CreateWindowAsChild(Pos.rClient.x0, Pos.rClient.y0,
											   Pos.rClient.x1 - Pos.rClient.x0 + 1,
											   Pos.rClient.y1 - Pos.rClient.y0 + 1,
											   pObj,
											   WC_ANCHOR_RIGHT | WC_ANCHOR_LEFT | WC_ANCHOR_TOP | WC_ANCHOR_BOTTOM | WC_VISIBLE | WC_LATE_CLIP,
											   FRAMEWIN_Obj::_cbClient, 0);
		/* Normally we disable memory devices for the frame window:
		 * The frame window does not flicker, and not using memory devices is usually faster.
		 * You can still use memory by explicitly specifying the flag
		 */
		if ((WinFlags & (WC_MEMDEV | (WC_MEMDEV_ON_REDRAW))) == 0) {
			WM_DisableMemdev(pObj);
		}
		pObj->SetText(pTitle);
	}
	return pObj;
}
FRAMEWIN_Obj *FRAMEWIN_Create(const char *pText, WM_CALLBACK *cb, int Flags,
								int x0, int y0, int xsize, int ysize) {
	return FRAMEWIN_CreateEx(x0, y0, xsize, ysize, nullptr, Flags, 0, 0, pText, cb);
}
FRAMEWIN_Obj *FRAMEWIN_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_Obj *hParent,
									   const char *pText, WM_CALLBACK *cb, int Flags) {
	return FRAMEWIN_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, 0, pText, cb);
}
WM_Obj *FRAMEWIN_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj *hWinParent,
										int x0, int y0, WM_CALLBACK *cb) {
	return FRAMEWIN_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							 hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id, pCreateInfo->pName, cb);
}

}
