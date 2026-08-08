module;

#include "DIALOG_Intern.h"

export module TUX.Widget.DropDown;

import TUX.Widget;
import TUX.Widget.ListBox;

import TUX.Array;

#define DROPDOWN_SF_AUTOSCROLLBAR DROPDOWN_CF_AUTOSCROLLBAR

export {
  
constexpr uint16_t DROPDOWN_CF_AUTOSCROLLBAR    = 1 << 0;
constexpr uint16_t DROPDOWN_CF_UP               = 1 << 1;
constexpr uint16_t DROPDOWN_CI_UNSEL     = 0;
constexpr uint16_t DROPDOWN_CI_SEL       = 1;
constexpr uint16_t DROPDOWN_CI_SELFOCUS  = 2;

struct DROPDOWN_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &FontProp13_1 };
		RGBC aBkColor[4]{
			/* Unselect */			RGB_WHITE,
			/* Selected */			RGB_GRAY,
			/* Selected focussed */	RGB_DARKBLUE,
			/* Disabled */			RGB_GRAYL(0xC0)
		};
		RGBC aTextColor[4]{
			/* Unselect */			RGB_BLACK,
			/* Selected */			RGB_WHITE,
			/* Selected focussed */	RGB_WHITE,
			/* Disabled */			RGB_GRAY
		};
		int16_t TextBorderSize{ 2 };
		int16_t Align{ TEXTALIGN_LEFT };
	} static DefaultProps;
	Properties Props;
	int16_t    Sel;      /* current selection */
	int16_t    ySizeEx;  /* Drop down size */
	int16_t    TextHeight;
	GUI_ARRAY Handles;
	WM_SCROLL_STATE ScrollState;
	LISTBOX_Obj *pListWin;
	uint8_t  Flags;
	uint16_t ItemSpacing;
	uint8_t  ScrollbarWidth;
	char  IsPressed;

	static int _Tolower(int Key) {
		if ((Key >= 0x41) && (Key <= 0x5a)) {
			Key += 0x20;
		}
		return Key;
	}
	
	int _GetNumItems() {
		return Handles.NumItems;
	}
	WM_HMEM _GethItem(int Index) {
		return GUI_ARRAY_GethItem(&Handles, Index);
	}
	void _DrawTriangleDown(int x, int y, int Size) {
		for (; Size >= 0; Size--, y++) {
			GUI_DrawHLine(y, x - Size, x + Size);
		}
	}
	const char *_GetpItem(int Index) {
		const char *s = nullptr;
		WM_HMEM h = _GethItem(Index);
		if (h) {
			s = (const char *)(h);
		}
		return s;
	}
	void _SelectByKey(int Key) {
		int i;
		Key = _Tolower(Key);
		for (i = 0; i < _GetNumItems(); i++) {
			char c = _Tolower(*_GetpItem(i));
			if (c == Key) {
				SetSel(i);
				break;
			}
		}
	}
	void _FreeAttached() {
		GUI_ARRAY_Delete(&Handles);
		WM_DeleteWindow(this->pListWin);
		this->pListWin = nullptr;
	}
	void _OnPaint() {
		int Border;
		const char *s;
		int InnerSize, ColorIndex;
		int TextBorderSize;
		/* Do some initial calculations */
		Border = this->EffectSize();
		TextBorderSize = this->Props.TextBorderSize;
		GUI_SetFont(this->Props.pFont);
		ColorIndex = (this->State & WIDGET_STATE_FOCUS) ? 2 : 1;
		s = _GetpItem(Sel);
		auto r = WM_GetClientRect();
		r -= Border;
		InnerSize = r.y1 - r.y0 + 1;
		/* Draw the 3D effect (if configured) */
		DrawDown();
		/* Draw the outer text frames */
		r.x1 -= InnerSize;     /* Spare square area to the right */
		GUI.SetColor(this->Props.aBkColor[ColorIndex]);
		/* Draw the text */
		GUI.SetBkColor(this->Props.aBkColor[ColorIndex]);
		GUI_FillRect(r);
		r.x0 += TextBorderSize;
		r.x1 -= TextBorderSize;
		GUI.SetColor(this->Props.aTextColor[ColorIndex]);
		GUI_DispStringInRect(s, &r, this->Props.Align);/**/
		/* Draw arrow */
		r = WM_GetClientRect();
		r -= Border;
		r.x0 = r.x1 + 1 - InnerSize;
		GUI.SetColor(RGB_GRAYL(0xc0));
		GUI_FillRect(r);
		GUI.SetColor(RGB_BLACK);
		_DrawTriangleDown((r.x1 + r.x0) / 2, r.y0 + 5, (r.y1 - r.y0 - 8) / 2);
		DrawUp(r);
	}
	void _OnTouch(const PID_STATE *pState) {
		if (pState) { /* Something happened in our area (pressed or released) */
			if (pState->Pressed) {
				Expand();
				WM_NotifyParent(this, WM_NOTIFICATION_CLICKED);
			}
			else {
				WM_NotifyParent(this, WM_NOTIFICATION_RELEASED);
			}
		}
		else { /* Mouse moved out */
			WM_NotifyParent(this, WM_NOTIFICATION_MOVED_OUT);
		}
	}
	int _OnKey(const WM_KEY_INFO *pInfo) {
		if (pInfo->PressedCnt > 0) {
			int Key = pInfo->Key;
			switch (Key) {
				case GUI_KEY_TAB:
					break; /* Send to parent by not doing anything */
				default:
					AddKey(Key);
					return 1; /* Message handled */
			}
		}
		return 0;
	}
	void _AdjustHeight() {
		int Height;
		Height = this->TextHeight;
		if (!Height) {
			Height = Props.pFont->DistY();
		}
		Height += this->EffectSize() + 2 * this->Props.TextBorderSize;
		WM_SetSize(this, GetSizeX(), Height);
	}

	static WM_PARAM _Callback(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (DROPDOWN_Obj *)hWin;
		bool IsExpandedBeforeMsg = pObj->pListWin ? pObj->pListWin->IsVisible() : false;
		/* Let widget handle the standard messages */
		if (!WIDGET_HandleActive(pObj, MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_NOTIFY_PARENT: {
				auto pInfo = (const NOTIFY_INFO *)Data;
				switch (pInfo->Notification) {
					case WM_NOTIFICATION_SCROLL_CHANGED:
						WM_NotifyParent(pObj, WM_NOTIFICATION_SCROLL_CHANGED);
						break;
					case WM_NOTIFICATION_CLICKED: {
						auto pListWin = (LISTBOX_Obj *)pInfo->pWinSrc;
						int Sel = pListWin->GetSel();
						pObj->SetSel(Sel);
						break;
					}
					case WM_NOTIFICATION_RELEASED:
						pObj->Collapse();
						WM_SetFocus(pObj);
						break;
					case LISTBOX_NOTIFICATION_LOST_FOCUS:
						pObj->Collapse();
						break;
				}
				return 0;
			}
			case WM_PID_STATE_CHANGED:
				if (!IsExpandedBeforeMsg) {    /* Make sure we do not react a second time */
					auto pInfo = (const PID_CHANGED_INFO *)Data;
					if (pInfo->State)
						pObj->Expand();
				}
				return 0;
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
				return 0;
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_DELETE:
				pObj->_FreeAttached();
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const WM_KEY_INFO *)Data))
					return 0;
				break;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:
	void Collapse() {
		if (this->pListWin) {
			auto pListWin = this->pListWin;
			this->pListWin = nullptr;
			WM_ReleaseCapture();
			WM_DeleteWindow(pListWin);
		}
	}
	void Expand() {
		int xSize, ySize, i, NumItems;
		RECT r;
		xSize = GetSizeX();
		ySize = ySizeEx;
		NumItems = _GetNumItems();
		r = GetRect();
		if (Flags & DROPDOWN_CF_UP) {
			r.y0 -= ySize;
		}
		else {
			r.y0 = r.y1 + 1;
		}
		auto pLst = this->pListWin;
		if (pLst == 0) {
			pLst = LISTBOX_CreateAsChild(nullptr, WM_GetDesktopWindow(), r.x0, r.y0, xSize, ySize, WC_VISIBLE | WC_STAYONTOP | WC_ACTIVATE);
			pLst->SetEffect(WIDGET_Effect_3D1L);
			if (pLst) {
				if (this->Flags & DROPDOWN_SF_AUTOSCROLLBAR) {
					pLst->SetScrollbarWidth(this->ScrollbarWidth);
					pLst->SetAutoScrollV(1);
				}
				this->pListWin = pLst;
				pLst->SetOwner(this);
			}
		}
		else {
			WM_MoveTo(pLst, r.x0, r.y0);
			pLst->ShowWindow();
		}
		if (pLst) {
			while (pLst->GetNumItems() > 0)
				pLst->DeleteItem(0);
			for (i = 0; i < NumItems; i++)
				pLst->AddString(_GetpItem(i));
			for (i = 0; i < GUI_COUNTOF(this->Props.aBkColor); i++)
				pLst->SetBkColor(i, this->Props.aBkColor[i]);
			for (i = 0; i < GUI_COUNTOF(this->Props.aTextColor); i++)
				pLst->SetTextColor(i, this->Props.aTextColor[i]);
			pLst->SetItemSpacing(this->ItemSpacing);
			pLst->SetFont(this->Props.pFont);
			pLst->SetSel(this->Sel);
			WM_NotifyParent(this, WM_NOTIFICATION_CLICKED);
			WM_SetCapture(pLst, 0);
		}
	}
	void AddKey(int Key) {
		switch (Key) {
			case GUI_KEY_DOWN:
				IncSel();
				break;
			case GUI_KEY_UP:
				DecSel();
				break;
			default:
				_SelectByKey(Key);
				break;
		}
	}
	void AddString(const char *s) {
		if (s) {
			GUI_ARRAY_AddItem(&this->Handles, s, GUI__strlen(s) + 1);
			WM_Invalidate(this);
		}
	}
	int  GetNumItems() {
		int r = 0;
		r = _GetNumItems();

		return r;
	}
	void SetFont(PCFONT pfont) {
		int OldHeight;
		OldHeight = Props.pFont->DistY();
		this->Props.pFont = pfont;
		_AdjustHeight();
		WM_Invalidate(this);
		if (this->pListWin) {
			if (OldHeight != Props.pFont->DistY()) {
				Collapse();
				Expand();
			}
			this->pListWin->SetFont(pfont);
		}
	}
	void SetBkColor(unsigned int Index, RGBC color) {
		if (Index < GUI_COUNTOF(this->Props.aBkColor)) {
			this->Props.aBkColor[Index] = color;
			WM_Invalidate(this);
			if (this->pListWin) {
				this->pListWin->SetBkColor(Index, color);
			}
		}
	}
	void SetTextColor(unsigned int Index, RGBC color) {
		if (Index < GUI_COUNTOF(this->Props.aBkColor)) {
			this->Props.aTextColor[Index] = color;
			WM_Invalidate(this);
			if (this->pListWin) {
				this->pListWin->SetTextColor(Index, color);
			}
		}
	}
	void SetSel(int Sel) {
		int NumItems, MaxSel;
		NumItems = _GetNumItems();
		MaxSel = NumItems ? NumItems - 1 : 0;
		if (Sel > MaxSel) {
			Sel = MaxSel;
		}
		if (Sel != this->Sel) {
			this->Sel = Sel;
			WM_Invalidate(this);
			WM_NotifyParent(this, WM_NOTIFICATION_SEL_CHANGED);
		}
	}
	void IncSel() {
		int Sel = GetSel();
		SetSel(Sel + 1);
	}
	void DecSel() {
		int Sel = GetSel();
		if (Sel)
			Sel--;
		SetSel(Sel);
	}
	int  GetSel() {
		int r = 0;
		r = this->Sel;

		return r;
	}
	void SetScrollbarWidth(unsigned Width) {
		if (Width != (unsigned)this->ScrollbarWidth) {
			this->ScrollbarWidth = Width;
			if (this->pListWin) {
				this->pListWin->SetScrollbarWidth(Width);
			}
		}
	}

	void DeleteItem(unsigned int Index) {
		unsigned int NumItems;
		NumItems = GetNumItems();
		if (Index < NumItems) {
			GUI_ARRAY_DeleteItem(&this->Handles, Index);
			WM_Invalidate(this);
			if (this->pListWin) {
				this->pListWin->DeleteItem(Index);
			}
		}
	}
	void InsertString(const char *s, unsigned int Index) {
		if (s) {
			unsigned int NumItems;

			NumItems = GetNumItems();
			if (Index < NumItems) {
				WM_HMEM hItem;
				hItem = GUI_ARRAY_InsertItem(&this->Handles, Index, GUI__strlen(s) + 1);
				if (hItem) {
					auto pBuffer = (char *)(hItem);
					GUI__strcpy(pBuffer, s);
				}
				WM_Invalidate(this);
				if (this->pListWin) {
					this->pListWin->InsertString(s, Index);
				}
			}
			else {
				AddString(s);
				if (this->pListWin) {
					this->pListWin->AddString(s);
				}
			}
		}
	}
	void SetItemSpacing(unsigned Value) {
		this->ItemSpacing = Value;
		if (this->pListWin) {
			this->pListWin->SetItemSpacing(Value);
		}
	}
	uint16_t GetItemSpacing() {
		return this->ItemSpacing;
	}
	void SetAutoScroll(int OnOff) {
		char Flags = this->Flags & (~DROPDOWN_SF_AUTOSCROLLBAR);
		if (OnOff) {
			Flags |= DROPDOWN_SF_AUTOSCROLLBAR;
		}
		if (this->Flags != Flags) {
			this->Flags = Flags;
			if (this->pListWin) {
				this->pListWin->SetAutoScrollV((Flags & DROPDOWN_SF_AUTOSCROLLBAR) ? 1 : 0);
			}
		}
	}
	void SetTextAlign(int Align) {
		this->Props.Align = Align;
		WM_Invalidate(this);
	}
	void SetTextHeight(unsigned TextHeight) {
		this->TextHeight = TextHeight;
		_AdjustHeight();
		WM_Invalidate(this);
	}
};

DROPDOWN_Obj::Properties DROPDOWN_Obj::DefaultProps;

DROPDOWN_Obj *DROPDOWN_CreateEx(int x0, int y0, int xsize, int ysize, WM_Obj *hParent,
								int WinFlags, int ExFlags, int Id) {
	auto pObj = (DROPDOWN_Obj *)WM_CreateWindowAsChild(
		x0, y0, xsize, -1,
		hParent, WinFlags, DROPDOWN_Obj::_Callback,
		sizeof(DROPDOWN_Obj) - sizeof(WM_Obj));
	if (pObj) {
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE);
		pObj->Props = DROPDOWN_Obj::DefaultProps;
		pObj->Flags = ExFlags;
		pObj->ScrollbarWidth = 0;
		pObj->ySizeEx = ysize;
		pObj->_AdjustHeight();
	}
	return pObj;
}
DROPDOWN_Obj *DROPDOWN_Create(WM_Obj *hWinParent, int x0, int y0, int xsize, int ysize, int Flags) {
	return DROPDOWN_CreateEx(x0, y0, xsize, ysize, hWinParent, Flags, 0, 0);
}
WM_Obj *DROPDOWN_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo,
								WM_Obj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	return DROPDOWN_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0,
							 pCreateInfo->xSize, pCreateInfo->ySize,
							 hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
}

}
