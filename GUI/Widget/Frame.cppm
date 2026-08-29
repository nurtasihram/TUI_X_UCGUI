module;

#include "WM_Intern.h"
#include "GUI_Protected.h"

export module TUX.Widget.Frame;

import TUX.Widget;
import TUX.Widget.Button;
import TUX.Widget.Menu;
import TUX.Widget.Window;

static int16_t FRAMEWIN__MinVisibility = 5;

static int16_t FRAMEWIN_REACT_BORDER = 3;

static int16_t
	FRAMEWIN_MINSIZE_X = 20,
	FRAMEWIN_MINSIZE_Y = 20;

static int16_t
	FRAMEWIN_RESIZE_X     = 1 << 0,
	FRAMEWIN_RESIZE_Y     = 1 << 1,
	FRAMEWIN_REPOS_X      = 1 << 2,
	FRAMEWIN_REPOS_Y      = 1 << 3,
	FRAMEWIN_MOUSEOVER    = 1 << 4,
	FRAMEWIN_RESIZE       = FRAMEWIN_RESIZE_X | FRAMEWIN_RESIZE_Y | FRAMEWIN_REPOS_X | FRAMEWIN_REPOS_Y;

export {

constexpr uint16_t
	FRAMEWIN_CF_ACTIVE     = 1 << 3,
	FRAMEWIN_CF_MOVEABLE   = 1 << 4,
	FRAMEWIN_CF_RESIZEABLE = 1 << 5,
	FRAMEWIN_CF_TITLEVIS   = 1 << 6,
	FRAMEWIN_CF_MINIMIZED  = 1 << 7,
	FRAMEWIN_CF_MAXIMIZED  = 1 << 8;
constexpr uint16_t
	FRAMEWIN_BUTTON_RIGHT   = 1 << 0,
	FRAMEWIN_BUTTON_LEFT    = 1 << 1;

class Frame : public Widget {

public:
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
	
private:
	Properties Props = DefaultProps;
	
	Menu *pMenu = nullptr;
	Window *pClient = nullptr;
	char *pText = nullptr;
	RECT rRestore;
	uint16_t Flags;

	struct POSITIONS {
		int16_t TitleHeight;
		int16_t MenuHeight;
		RECT rClient;
		RECT rTitleText;
	};

	int _IBorderSize() const
	{ return GetStates() & FRAMEWIN_CF_TITLEVIS ? Props.IBorderSize : 0; }

	int _CalcTitleHeight() const {
		return GetStates() & FRAMEWIN_CF_TITLEVIS ? 
			Props.TitleHeight ?
				Props.TitleHeight :
				2 + Props.pFont->SizeY() :
			0;
	}

	POSITIONS _CalcPositions() const {
		auto BorderSize = Props.BorderSize;
		int IBorderSize = _IBorderSize();
		POSITIONS Pos;
		Pos.TitleHeight = _CalcTitleHeight();
		Pos.MenuHeight = pMenu ? pMenu->GetSizeY() : 0;
		/* Set object properties accordingly */
		Pos.rClient = GetClientRect() / BorderSize;
		Pos.rClient.y0 += IBorderSize + Pos.TitleHeight + Pos.MenuHeight;
		/* Calculate title rect */
		Pos.rTitleText.x0 = BorderSize;
		Pos.rTitleText.x1 = Pos.rClient.x1 - BorderSize - 1;
		Pos.rTitleText.y0 = BorderSize;
		Pos.rTitleText.y1 = BorderSize + Pos.TitleHeight - 1;
		/* Iterate over all children */
		for (auto pChild = FirstChild(); pChild; pChild = pChild->pNext) {
			int x0 = pChild->Rect.x0 - this->Rect.x0;
			int x1 = pChild->Rect.x1 - this->Rect.x0;
			int y0 = pChild->Rect.y0 - this->Rect.y0;
			if (y0 == BorderSize) {
				if (pChild->Status & WC_ANCHOR_RIGHT) {
					if (x0 <= Pos.rTitleText.x1)
						Pos.rTitleText.x1 = x0 - 1;
				}
				else if (x1 >= Pos.rTitleText.x0)
					Pos.rTitleText.x0 = x1 + 1;
			}
		}
		return Pos;
	}
	void _UpdatePositions() {
		/* Move client window accordingly */
		if (pClient || pMenu) {
			auto &&Pos = _CalcPositions();
			if (pClient) {
				pClient->MoveChildTo(Pos.rClient.LeftTop());
				pClient->SetSize(Pos.rClient.Size());
			}
			if (pMenu)
				pMenu->MoveChildTo({ Pos.rClient.x0, Pos.rClient.y0 - Pos.MenuHeight });
		}
	}
	void _UpdateButtons(int OldHeight) {
		int TitleHeight = _CalcTitleHeight();
		int Diff = TitleHeight - OldHeight;
		if (Diff) {
			WObj *pLeft, *pRight, *pChild;
			int xLeft, xRight, n = 0;
			do {
				pLeft = pRight = nullptr;
				xLeft = GUI_XMAX;
				xRight = GUI_XMIN;
				for (pChild = this->pFirstChild; pChild; pChild = pChild->pNext) {
					auto r = pChild->Rect - this->Rect.LeftTop();
					if ((r.y0 == Props.BorderSize) && ((r.y1 - r.y0 + 1) == OldHeight)) {
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
					pLeft->Resize(Diff);
					pLeft->Move({ n * Diff, 0 });
				}
				if (pRight) {
					pRight->Resize(Diff);
					pRight->Move({ -(n * Diff), 0 });
				}
				n++;
			} while (pLeft || pRight);
		}
	}
	void _OnTouch(const PID_STATE *pState) {
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (pState->Pressed) {
				if (!(this->Flags & FRAMEWIN_CF_ACTIVE))
					SetFocus();
				BringToTop();
				if (this->Flags & FRAMEWIN_CF_MOVEABLE)
					SetCaptureMove(*pState, FRAMEWIN__MinVisibility);
			}
		}
	}
	void _OnPaint() {
		auto size = GetSize();
		auto BorderSize = Props.BorderSize;
		auto &&Pos = _CalcPositions();
		RECT r{
			Pos.rClient.x0,
			Pos.rTitleText.y0,
			Pos.rClient.x1,
			Pos.rTitleText.y1
		};
		/* Perform computations */
		Pos.rTitleText.y0++;
		Pos.rTitleText.x0++;
		Pos.rTitleText.x1--;
		GUI.SetFont(Props.pFont);
		RECT rText;
		GUI__CalcTextRect(pText, &Pos.rTitleText, &rText, Props.Align);
		auto y0 = Pos.TitleHeight + BorderSize;
		/* Draw Title */
		auto Index = (Flags & FRAMEWIN_CF_ACTIVE) ? 1 : 0;
		GUI.SetBkColor(Props.aBarColor[Index]);
		GUI.SetColor(Props.aTextColor[Index]);
		WIDGET__FillStringInRect(pText, r, Pos.rTitleText, rText);
		/* Draw Frame */
		GUI.SetColor(Props.FrameColor);
		GUI_FillRect({ 0, 0, size.x - 1, BorderSize - 1 });
		GUI_FillRect({ 0, 0, Pos.rClient.x0 - 1, size.y - 1 });
		GUI_FillRect({ Pos.rClient.x1 + 1, 0, size.x - 1, size.y - 1 });
		GUI_FillRect({ 0, Pos.rClient.y1 + 1, size.x - 1, size.y - 1 });
		GUI_FillRect({ 0, y0, size.x - 1, y0 + Props.IBorderSize - 1 });
		/* Draw the 3D effect (if configured) */
		if (Props.BorderSize >= 2)
			DrawUp();
	}
	void _OnChildHasFocus(const NOTIFY_CHILD_HAS_FOCUS_INFO *pInfo) {
		if (pInfo) 
			SetActive(IsAncestorOrSelf(pInfo->pNew));
	}

#pragma region Resize Control
#if GUI_SUPPORT_CURSOR
	static void _SetResizeCursor(int Mode) {
		static PCCURSOR _pOldCursor;
		static const uint8_t _acResizeCursorH[]{
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
			/* Size */ 17, 17,
			/* BPL, BPP */ 5, 2,
			_acResizeCursorH, &GUI_CursorPal
		};
		static CCURSOR _ResizeCursorH{ &_bmResizeCursorH, 8, 8 };
		static const uint8_t _acResizeCursorV[]{
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
		static CBITMAP _bmResizeCursorV{
			/* Size */ 17, 17,
			/* BPL, BPP */ 5, 2,
			_acResizeCursorV, &GUI_CursorPal
		};
		static CCURSOR _ResizeCursorV{ &_bmResizeCursorV, 8, 8 };
		static const uint8_t _acResizeCursorDD[]{
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
		static CBITMAP _bmResizeCursorDD{
			/* Size */ 17, 17,
			/* BPL, BPP */ 5, 2,
			_acResizeCursorDD, &GUI_CursorPal
		};
		static CCURSOR _ResizeCursorDD{ &_bmResizeCursorDD, 8, 8 };
		static const uint8_t _acResizeCursorDU[]{
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
		static CBITMAP _bmResizeCursorDU{
			/* Size */ 17, 17,
			/* BPL, BPP */ 5, 2,
			_acResizeCursorDU, &GUI_CursorPal
		};
		static CCURSOR _ResizeCursorDU{ &_bmResizeCursorDU, 8, 8 };
		PCCURSOR pNewCursor = nullptr;
		if (Mode) {
			auto Direction = Mode & (FRAMEWIN_RESIZE_X | FRAMEWIN_RESIZE_Y);
			if (Direction == FRAMEWIN_RESIZE_X)
				pNewCursor = &_ResizeCursorH;
			else if (Direction == FRAMEWIN_RESIZE_Y)
				pNewCursor = &_ResizeCursorV;
			else {
				Direction = Mode & (FRAMEWIN_REPOS_X | FRAMEWIN_REPOS_Y);
				if ((Direction == (FRAMEWIN_REPOS_X | FRAMEWIN_REPOS_Y)) || !Direction)
					pNewCursor = &_ResizeCursorDD;
				else
					pNewCursor = &_ResizeCursorDU;
			}
		}
		if (pNewCursor) {
			auto pOldCursor = GUI_CURSOR_Select(pNewCursor);
			if (_pOldCursor == nullptr)
				_pOldCursor = pOldCursor;
		}
		else if (_pOldCursor) {
			GUI_CURSOR_Select(_pOldCursor);
			_pOldCursor = nullptr;
		}
	}
#endif
	static POINT _Capture;
	static int _CaptureFlags;
	void _SetCapture(POINT Pos, int Mode) {
		if (!(_CaptureFlags & FRAMEWIN_REPOS_X))
			_Capture.x = Pos.x;
		if (!(_CaptureFlags & FRAMEWIN_REPOS_Y))
			_Capture.y = Pos.y;
		if (!Mode)
			return;
		if (!HasCaptured())
			SetCapture(0);
#if GUI_SUPPORT_CURSOR
		_SetResizeCursor(Mode);
#endif
		if (Mode & FRAMEWIN_MOUSEOVER)
			Mode = 0;
		_CaptureFlags = Mode | FRAMEWIN_MOUSEOVER;
	}
	void _ChangeWindowPosSize(POINT p) {
		/* Calculate new size of window */
		POINT d{
			_CaptureFlags & FRAMEWIN_RESIZE_X ?
				_CaptureFlags & FRAMEWIN_REPOS_X ? _Capture.x - p.x : p.x - _Capture.x : 0,
			_CaptureFlags & FRAMEWIN_RESIZE_Y ?
				_CaptureFlags & FRAMEWIN_REPOS_Y ? _Capture.y - p.y : p.y - _Capture.y : 0
		};
		auto Rect = GetClientRect();
		/* Check the minimal size of window */
		if (auto xMin = FRAMEWIN_MINSIZE_X - Rect.x1 - 1; d.x < xMin) {
			d.x = xMin;
			p.x = _Capture.x + xMin;
		}
		if (auto yMin = FRAMEWIN_MINSIZE_Y - Rect.y1 - 1; d.y < yMin) {
			d.y = yMin;
			p.y = _Capture.y + yMin;
		}
		/* Set new window position */
		Move({
			(_CaptureFlags & FRAMEWIN_REPOS_X) ? -d.x : 0,
			(_CaptureFlags & FRAMEWIN_REPOS_Y) ? -d.y : 0
		});
		/* Set new window size */
		Resize(d);
	}
	static int16_t _CheckBorderX(int x, int x1, int Border) {
		return
			x > x1 - Border ? FRAMEWIN_RESIZE_X :
			x < Border ? FRAMEWIN_RESIZE_X | FRAMEWIN_REPOS_X :
			0;
	}
	static int16_t _CheckBorderY(int y, int y1, int Border) {
		return
			y > y1 - Border ? FRAMEWIN_RESIZE_Y :
			y < Border ? FRAMEWIN_RESIZE_Y | FRAMEWIN_REPOS_Y :
			0;
	}
	int _CheckReactBorder(POINT Pos) {
		auto r = GetClientRect();
		if (!(r <= Pos)) return 0;
		if (auto Mode = _CheckBorderX(Pos.x, r.x1, FRAMEWIN_REACT_BORDER))
			return Mode | _CheckBorderY(Pos.y, r.y1, 4 * FRAMEWIN_REACT_BORDER);
		if (auto Mode = _CheckBorderY(Pos.y, r.y1, FRAMEWIN_REACT_BORDER))
			return Mode | _CheckBorderX(Pos.x, r.x1, 4 * FRAMEWIN_REACT_BORDER);
		return 0;
	}
	bool _OnTouchResize(const PID_STATE *pState) {
		if (pState) {  /* Something happened in our area (pressed or released) */
			POINT Pos = *pState;
			auto Mode = _CheckReactBorder(Pos);
			if (pState->Pressed == 1) {
				if (_CaptureFlags & FRAMEWIN_RESIZE) {
					_ChangeWindowPosSize(Pos);
					_SetCapture(Pos, 0);
					return true;
				}
				else if (Mode) {
					SetFocus();
					BringToTop();
					_SetCapture(Pos, Mode);
					return true;
				}
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
				else if (_CaptureFlags) {
					ReleaseCapture();
					return true;
				}
#endif
			}
			else if (HasCaptured()) {
				_CaptureFlags &= ~(FRAMEWIN_RESIZE);
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
				if (!Mode)
#endif
					ReleaseCapture();
				return true;
			}
		}
		return false;
	}
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
	bool _ForwardMouseOverMsg(POINT Pos) {
		PID_STATE StateBelow = Pos + GetOrg();
		if (auto pBelow = WM_Screen2Win(StateBelow); pBelow != this) {
			StateBelow -= pBelow->GetOrg();
			WM__SendMessage(pBelow, WM_MOUSEOVER, (WM_PARAM)&StateBelow);
			return true;
		}
		return false;
	}
	bool _OnMouseOver(POINT Pos) {
		if (auto Mode = _CheckReactBorder(Pos)) {
			if (!_ForwardMouseOverMsg(Pos))
				_SetCapture(Pos, Mode | FRAMEWIN_MOUSEOVER);
			return true;
		}
		if (HasCaptured()) {
			if (!(_CaptureFlags & FRAMEWIN_RESIZE)) {
				ReleaseCapture();
				_ForwardMouseOverMsg(Pos);
			}
			return true;
		}
		return false;
	}
#endif
	bool _HandleResizeable(int MsgId, WM_PARAM Data) {
		if (HasCaptured() && !_CaptureFlags)
			return false;
		if (IsMinimized() || IsMaximized())
			return false;
		switch (MsgId) {
		case WM_TOUCH:
			return _OnTouchResize((const PID_STATE *)Data);
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
		case WM_MOUSEOVER:
			if (auto pState = (const PID_STATE *)Data)
				return _OnMouseOver(*pState);
#endif
		case WM_CAPTURE_RELEASED:
#if GUI_SUPPORT_CURSOR
			_SetResizeCursor(0);
#endif
			_CaptureFlags = 0;
			return true;
		}
		return false;
	}
#pragma endregion

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (Frame *)hWin;
		if (pObj->Flags & FRAMEWIN_CF_RESIZEABLE)
			if (pObj->_HandleResizeable(MsgId, Data))
				return 0;
		switch (MsgId) {
		case WM_PAINT:
			pObj->_OnPaint();
			return 0;
		case WM_TOUCH:
			pObj->_OnTouch((const PID_STATE *)Data);
			return 0;
		case WM_GET_INSIDE_RECT: {
			*(RECT *)Data = pObj->_CalcPositions().rClient;
			return 0;
		}
		case WM_HANDLE_DIALOG_STATUS:
			return pObj->pClient->Require(WM_HANDLE_DIALOG_STATUS, Data);
		case WM_GET_CLIENT_WINDOW: /* return handle to client window. For most windows, there is no seperate client window, so it is the same handle */
			return (WM_PARAM)pObj->pClient;
		case WM_NOTIFY_PARENT: {
			auto pInfo = (const NOTIFY_INFO *)Data;
			auto pWinSrc = pInfo->pWinSrc;
			switch (pInfo->Notification) {
			case WM_NOTIFICATION_CHILD_DELETED:
				if (pWinSrc == pObj->pClient) {
					pObj->pClient = nullptr;
					WM_DeleteWindow(pObj);
				}
				break;
			case WM_NOTIFICATION_RELEASED:
				switch (pWinSrc->GetID()) {
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
				break;
			}
			return 0;
		}
		case WM_SET_FOCUS: /* We have received or lost focus */
			if (Data) {
				pObj->pClient->SetFocus();
			}
			pObj->SetActive(Data);
			return 0;
		case WM_NOTIFY_CHILD_HAS_FOCUS:
			pObj->_OnChildHasFocus((const NOTIFY_CHILD_HAS_FOCUS_INFO *)Data);
			break;
		case WM_DELETE:
			GUI_DEBUG_LOG("FRAMEWIN: _FRAMEWIN_Callback(WM_DELETE)\n");
			GUI_ALLOC_Free(pObj->pText);
			pObj->pText = nullptr;
			break;
		}
		/* Let widget handle the standard messages */
		if (!pObj->HandleActive(MsgId, &Data))
			return Data;
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:
	Frame(RECT r, WM_CF Style, WObj *pParent, uint16_t Id,
		  uint16_t ExFlags, const char *pTitle, WM_CALLBACK *cb) :
		Widget(r, Style | WC_LATE_CLIP, _Callback, pParent, Id, WIDGET_STATE_FOCUSSABLE | FRAMEWIN_CF_TITLEVIS),
		pClient(new Window(
			_CalcPositions().rClient,
			WC_ANCHOR_ALL | WC_VISIBLE | WC_LATE_CLIP, this, 0, cb)),
		Flags(ExFlags) {
		if (!(Style & (WC_MEMDEV | WC_MEMDEV_ON_REDRAW)))
			WM_DisableMemdev(this);
		SetText(pTitle);
	}
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *hWinParent,
								  int x0, int y0, WM_CALLBACK *cb) {
		return new Frame(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			0, hWinParent, pCreateInfo->Id,
			pCreateInfo->Flags, pCreateInfo->pName, cb);
	}

public:

#pragma region Properties

	PCFONT GetFont() const { return Props.pFont; }
	void SetFont(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		int OldHeight = _CalcTitleHeight();
		Props.pFont = pFont;
		_UpdatePositions();
		_UpdateButtons(OldHeight);
		Invalidate();
	}

	int GetBorderSize() const { return Props.BorderSize; }

	void SetTextAlign(TEXTALIGN Align) {
		if (Props.Align == Align)
			return;
		Props.Align = Align;
		Invalidate();
	}

	void SetBarColor(unsigned Index, RGBC Color) {
		if (Index >= GUI_COUNTOF(Props.aBarColor))
			return;
		if (Props.aBarColor[Index] == Color)
			return;
		Props.aBarColor[Index] = Color;
		Invalidate();
	}

	void SetTextColor(unsigned Index, RGBC Color) {
		if (Index >= GUI_COUNTOF(Props.aTextColor))
			return;
		if (Props.aTextColor[Index] == Color)
			return;
		Props.aTextColor[Index] = Color;
		Invalidate();
	}

	void SetClientColor(RGBC Color) {
		if (Props.ClientColor == Color)
			return;
		Props.ClientColor = Color;
		pClient->Invalidate();
	}
	
#pragma endregion

	void SetText(const char *s) {
		if (GUI__SetText(&this->pText, s))
			Invalidate();
	}

	void SetMoveable(bool bMoveable) {
		Flags = bMoveable ?
			Flags | FRAMEWIN_CF_MOVEABLE :
			Flags & ~FRAMEWIN_CF_MOVEABLE;
	}

	void SetActive(bool bActive) {
		auto State = bActive ?
			Flags | FRAMEWIN_CF_ACTIVE :
			Flags & ~FRAMEWIN_CF_ACTIVE;
		if (Flags!= State) {
			Flags = State;
			Invalidate();
		}
	}

	void AddMenu(Menu *pMenu) {
		auto TitleHeight = _CalcTitleHeight();
		uint16_t BorderSize = Props.BorderSize, IBorderSize = _IBorderSize();
		auto xSize = GetSizeX() - BorderSize * 2;
		this->pMenu = pMenu;
		if (pClient)
			pMenu->SetOwner(pClient);
		pMenu->AttachMenu(this, { BorderSize, BorderSize + TitleHeight + IBorderSize }, xSize, 0);
		WM_SetAnchor(pMenu, WC_ANCHOR_LEFT | WC_ANCHOR_RIGHT);
		_UpdatePositions();
		Invalidate();
	}

#pragma region Minimize / Maximizes
private:
	void _InvalidateButton(int Id) {
		for (auto pChild = this->pFirstChild; pChild; pChild = pChild->pNext)
			if (pChild->GetID() == Id)
				pChild->Invalidate();
	}

	void _RestoreMinimized() {
		if (!IsMinimized())
			return;
		Resize({ 0, rRestore.YSize() - Rect.YSize() });
		pClient->ShowWindow();
		pMenu->ShowWindow();
		_UpdatePositions();
		Flags &= ~FRAMEWIN_CF_MINIMIZED;
		_InvalidateButton(GUI_ID_MINIMIZE);
	}
	void _RestoreMaximized() {
		if (!IsMaximized())
			return;
		MoveTo(rRestore.LeftTop());
		SetSize(rRestore.Size());
		_UpdatePositions();
		Flags &= ~FRAMEWIN_CF_MAXIMIZED;
		_InvalidateButton(GUI_ID_MAXIMIZE);
	}
public:
	bool IsMinimized() { return Flags & FRAMEWIN_CF_MINIMIZED; }
	void Minimize() {
		_RestoreMaximized();
		/* When window is not minimized, minimize it */
		if (IsMinimized())
			return;
		int OldHeight = Rect.y1 - Rect.y0 + 1;
		int NewHeight = _CalcTitleHeight() + EffectSize() * 2 + 2;
		rRestore = Rect;
		pClient->HideWindow();
		pMenu->HideWindow();
		Resize({ 0, NewHeight - OldHeight });
		_UpdatePositions();
		Flags |= FRAMEWIN_CF_MINIMIZED;
		_InvalidateButton(GUI_ID_MINIMIZE);
	}

	bool IsMaximized() { return Flags & FRAMEWIN_CF_MAXIMIZED; }
	void Maximize() {
		_RestoreMinimized();
		/* When window is not maximized, maximize it */
		if (IsMaximized())
			return;
		auto r = pParent->Rect;
		if (!pParent->pParent) {
			r.x1 = LCD_GetXSize();
			r.y1 = LCD_GetYSize();
		}
		rRestore = Rect;
		MoveTo(r.LeftTop());
		SetSize(r.Size());
		_UpdatePositions();
		Flags |= FRAMEWIN_CF_MAXIMIZED;
		_InvalidateButton(GUI_ID_MAXIMIZE);
	}

	void Restore() {
		_RestoreMinimized();
		_RestoreMaximized();
	}
#pragma endregion

	void SetBorderSize(unsigned Size) {
		int OldHeight = _CalcTitleHeight();
		int OldSize = Props.BorderSize;
		int Diff = Size - OldSize;
		for (auto pChild = FirstChild(); pChild; pChild = pChild->pNext) {
			auto r = pChild->Rect - Rect.LeftTop();
			if (r.y0 == Props.BorderSize && r.YSize() == OldHeight) {
				if (pChild->Status & WC_ANCHOR_RIGHT)
					pChild->Move({ -Diff, Diff });
				else
					pChild->Move(Diff);
			}
		}
		Props.BorderSize = Size;
		_UpdatePositions();
		Invalidate();
	}

	void SetResizeable(int State) {
		Flags = State ?
			Flags | FRAMEWIN_CF_RESIZEABLE :
			Flags & ~FRAMEWIN_CF_RESIZEABLE;
	}

	void SetTitleHeight(int Height) {
		if (Props.TitleHeight == Height)
			return;
		auto OldHeight = _CalcTitleHeight();
		Props.TitleHeight = Height;
		_UpdatePositions();
		_UpdateButtons(OldHeight);
		Invalidate();
	}

	void SetTitleVis(bool bShow) {
		auto State = bShow ?
			Flags | FRAMEWIN_CF_TITLEVIS :
			Flags & ~FRAMEWIN_CF_TITLEVIS;
		if (Flags == State)
			return;
		Flags = State;
		_UpdatePositions();
		for (auto pChild = pFirstChild; pChild; pChild = pChild->pNext)
			if (pChild->Rect.y0 - Rect.y0 == Props.BorderSize && pChild != pClient) {
				if (GetStates() & FRAMEWIN_CF_TITLEVIS)
					pChild->ShowWindow();
				else
					pChild->HideWindow();
			}
		if (Flags & FRAMEWIN_CF_MINIMIZED) {
			if (bShow)
				ShowWindow();
			else
				HideWindow();
		}
		Invalidate();
	}

#pragma region Buttons
	Button *AddButton(int Flags, int Off, int Id) {
		auto Size = _CalcTitleHeight();
		auto BorderSize = GetBorderSize();
		int WinFlags, x;
		auto &&Pos = _CalcPositions();
		if (Flags & FRAMEWIN_BUTTON_RIGHT) {
			x = Pos.rTitleText.x1 - (Size - 1) - Off;
			WinFlags = WC_VISIBLE | WC_ANCHOR_RIGHT;
		}
		else {
			x = Pos.rTitleText.x0 + Off;
			WinFlags = WC_VISIBLE;
		}
		auto r = new Button(RECT::LeftTop({ x, BorderSize }, { Size, Size }), WinFlags, this, Id);
		r->SetFocussable(0);
		return r;
	}
	Button *AddCloseButton(int Flags = FRAMEWIN_BUTTON_RIGHT, int Off = 1) {
		auto pButton = AddButton(Flags, Off, GUI_ID_CLOSE);
		pButton->SetSelfDraw(BUTTON_BI_UNPRESSED, [](RECT &r) {
			int Size = r.x1 - r.x0 - 2;
			for (int i = 2; i < Size; i++) {
				LCD_DrawHLine(r.x0 + i, r.y0 + i, r.x0 + i + 1);
				LCD_DrawHLine(r.x1 - i - 1, r.y0 + i, r.x1 - i);
			}
		});
		return pButton;
	}
	Button *AddMaxButton(int Flags = FRAMEWIN_BUTTON_RIGHT, int Off = 1) {
		auto pButton = AddButton(Flags, Off, GUI_ID_MAXIMIZE);
		pButton->SetSelfDraw(BUTTON_BI_UNPRESSED, [](RECT &r) {
			auto pObj = (Frame*)ActiveWindow()->Parent();
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
		});
		return pButton;
	}
	Button *AddMinButton(int Flags = FRAMEWIN_BUTTON_RIGHT, int Off = 1) {
		auto pButton = AddButton(Flags, Off, GUI_ID_MINIMIZE);
		pButton->SetSelfDraw(BUTTON_BI_UNPRESSED, [](RECT &r) {
			auto pObj = (Frame *)ActiveWindow()->Parent();
			int Size = (r.x1 - r.x0 + 1) >> 1;
			if (pObj->Flags & FRAMEWIN_CF_MINIMIZED)
				for (int i = 1; i < Size; i++)
					LCD_DrawHLine(r.x0 + i, r.y0 + i + (Size >> 1), r.x1 - i);
			else
				for (int i = 1; i < Size; i++)
					LCD_DrawHLine(r.x0 + i, r.y1 - i - (Size >> 1), r.x1 - i);
		});
		return pButton;
	}
#pragma endregion

};

Frame::Properties Frame::DefaultProps;

POINT Frame::_Capture{};
int Frame::_CaptureFlags = 0;

}
