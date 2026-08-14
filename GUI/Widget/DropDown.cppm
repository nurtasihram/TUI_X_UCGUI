module;

#include "GUI_Protected.h"

export module TUX.Widget.DropDown;

import TUX.Widget;
import TUX.Widget.ListBox;

import TUX.Array;

#define DROPDOWN_SF_AUTOSCROLLBAR DROPDOWN_CF_AUTOSCROLLBAR

export {
  
constexpr uint16_t
	DROPDOWN_CF_AUTOSCROLLBAR    = 1 << 0,
	DROPDOWN_CF_UP               = 1 << 1;

using DROPDOWN_CI = LISTBOX_CI;

class DropDown : public WIDGET {

public:
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
	
private:
	Properties Props;

	int16_t    Sel;      /* current selection */
	int16_t    ySizeEx;  /* Drop down size */
	int16_t    TextHeight;
	ARRAY<char *> Handles;
	WM_SCROLL_STATE ScrollState;
	ListBox *pListWin;
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
	
	const char *_GetpItem(int Index) {
		return Handles[Index];
	}
	void _DrawTriangleDown(int x, int y, int Size) {
		for (; Size >= 0; Size--, y++) {
			GUI_DrawHLine(y, x - Size, x + Size);
		}
	}
	void _SelectByKey(int Key) {
		int i;
		Key = _Tolower(Key);
		for (i = 0; i < GetNumItems(); i++) {
			char c = _Tolower(*_GetpItem(i));
			if (c == Key) {
				SetSel(i);
				break;
			}
		}
	}
	void _FreeAttached() {
		Handles.Delete();
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
		TextBorderSize = Props.TextBorderSize;
		GUI.SetFont(Props.pFont);
		ColorIndex = (this->State & WIDGET_STATE_FOCUS) ? 2 : 1;
		s = _GetpItem(Sel);
		auto r = WM_GetClientRect();
		r -= Border;
		InnerSize = r.y1 - r.y0 + 1;
		/* Draw the 3D effect (if configured) */
		DrawDown();
		/* Draw the outer text frames */
		r.x1 -= InnerSize;     /* Spare square area to the right */
		GUI.SetColor(Props.aBkColor[ColorIndex]);
		/* Draw the text */
		GUI.SetBkColor(Props.aBkColor[ColorIndex]);
		GUI_FillRect(r);
		r.x0 += TextBorderSize;
		r.x1 -= TextBorderSize;
		GUI.SetColor(Props.aTextColor[ColorIndex]);
		GUI_DispStringInRect(s, &r, Props.Align);/**/
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
		Height += this->EffectSize() + 2 * Props.TextBorderSize;
		WM_SetSize(this, GetSizeX(), Height);
	}

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (DropDown *)hWin;
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
						auto pListWin = (ListBox *)pInfo->pWinSrc;
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
				for (int i = 0; i < pObj->Handles.NumItems(); i++)
					GUI__SetText(&pObj->Handles[i], nullptr);
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

	static DropDown *Create(int x0, int y0, int xsize, int ysize,
					 WObj *hParent, int WinFlags, int ExFlags, int Id) {
		auto pObj = (DropDown *)WM_CreateWindowAsChild(
			x0, y0, xsize, -1,
			hParent, WinFlags, DropDown::_Callback,
			sizeof(DropDown) - sizeof(WObj));
		if (!pObj) {
			GUI_DEBUG_ERROROUT_IF(pObj == 0, "DropDown create failed");
			return nullptr;
		}
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE);
		pObj->Props = DropDown::DefaultProps;
		pObj->Flags = ExFlags;
		pObj->ScrollbarWidth = 0;
		pObj->ySizeEx = ysize;
		pObj->_AdjustHeight();
		return pObj;
	}
	static WIDGET *CreateIndirect(const WIDGET_CREATE_INFO *pCreateInfo,
								WObj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return Create(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0,
					  pCreateInfo->xSize, pCreateInfo->ySize,
					  hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
	}

public:
	void Collapse() {
		if (this->pListWin) {
			auto pListWin = this->pListWin;
			this->pListWin = nullptr;
			ReleaseCapture();
			WM_DeleteWindow(pListWin);
		}
	}
	void Expand() {
		int xSize, ySize, NumItems;
		RECT r;
		xSize = GetSizeX();
		ySize = ySizeEx;
		NumItems = GetNumItems();
		r = GetRect();
		if (Flags & DROPDOWN_CF_UP) {
			r.y0 -= ySize;
		}
		else {
			r.y0 = r.y1 + 1;
		}
		auto pLst = this->pListWin;
		if (pLst == 0) {
			pLst = ListBox::Create(nullptr, WM_GetDesktopWindow(), r.x0, r.y0, xSize, ySize, WC_VISIBLE | WC_STAYONTOP | WC_ACTIVATE);
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
			for (int i = 0; i < NumItems; i++)
				pLst->AddString(_GetpItem(i));

			pLst->Props.pFont = Props.pFont;
			pLst->Props.aBkColor[0] = Props.aBkColor[0];
			pLst->Props.aBkColor[1] = Props.aBkColor[1];
			pLst->Props.aBkColor[2] = Props.aBkColor[2];
			pLst->Props.aBkColor[3] = Props.aBkColor[3];
			pLst->Props.aTextColor[0] = Props.aTextColor[0];
			pLst->Props.aTextColor[1] = Props.aTextColor[1];
			pLst->Props.aTextColor[2] = Props.aTextColor[2];
			pLst->Props.aTextColor[3] = Props.aTextColor[3];

			pLst->SetItemSpacing(this->ItemSpacing);
			pLst->SetSel(this->Sel);
			WM_NotifyParent(this, WM_NOTIFICATION_CLICKED);
			pLst->SetCapture(0);
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
			auto idx = Handles.NumItems();
			if (Handles.AddItem() == 0)
					GUI__SetText(&Handles[idx], s);
			WM_Invalidate(this);
		}
	}

	auto GetNumItems() { return Handles.NumItems(); }
	void SetFont(PCFONT pFont) {
		auto OldHeight = Props.pFont->DistY();
		Props.pFont = pFont;
		_AdjustHeight();
		WM_Invalidate(this);
		if (this->pListWin) {
			if (OldHeight != Props.pFont->DistY()) {
				Collapse();
				Expand();
			}
			this->pListWin->SetFont(pFont);
		}
	}
	void SetBkColor(DROPDOWN_CI Index, RGBC color) {
		if (Index < GUI_COUNTOF(Props.aBkColor)) {
			Props.aBkColor[Index] = color;
			WM_Invalidate(this);
			if (this->pListWin) {
				this->pListWin->SetBkColor(Index, color);
			}
		}
	}
	void SetTextColor(DROPDOWN_CI Index, RGBC color) {
		if (Index < GUI_COUNTOF(Props.aTextColor)) {
			Props.aTextColor[Index] = color;
			WM_Invalidate(this);
			if (this->pListWin) {
				this->pListWin->SetTextColor(Index, color);
			}
		}
	}
	void SetSel(int Sel) {
		int NumItems, MaxSel;
		NumItems = GetNumItems();
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
			GUI__SetText(&Handles[Index], nullptr);
			Handles.DeleteItem(Index);
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
				auto pp = Handles.InsertItem(Index);
				if (pp)
					GUI__SetText(pp, s);
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
	void SetTextAlign(TEXTALIGN Align) {
		if (Props.Align != Align) {
			Props.Align = Align;
			WM_Invalidate(this);
		}
	}
	void SetTextHeight(unsigned TextHeight) {
		this->TextHeight = TextHeight;
		_AdjustHeight();
		WM_Invalidate(this);
	}
};

DropDown::Properties DropDown::DefaultProps;

}
