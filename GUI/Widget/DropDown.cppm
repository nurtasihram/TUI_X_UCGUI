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

class DropDown : public Widget {

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
	Properties Props = DefaultProps;

	int16_t    Sel = 0;      /* current selection */
	int16_t    ySizeEx;  /* Drop down size */
	int16_t    TextHeight = 0;
	ARRAY<char *> Handles;
	WM_SCROLL_STATE ScrollState;
	ListBox *pListWin = nullptr;
	uint8_t  Flags;
	uint16_t ItemSpacing = 0;
	uint8_t  ScrollbarWidth = 0;
	char  IsPressed;

	static int _Tolower(int Key) {
		if ((Key >= 0x41) && (Key <= 0x5a)) {
			Key += 0x20;
		}
		return Key;
	}
	
	const char *_GetpItem(int Index) {
		if (Index < 0 || Index >= GetNumItems())
			return nullptr;
		return Handles[Index];
	}
	void _DrawTriangleDown(int x, int y, int Size) {
		while (Size--)
			GUI_DrawHLine(y++, x - Size, x + Size);
	}
	void _SelectByKey(int Key) {
		Key = _Tolower(Key);
		for (int i = 0; i < GetNumItems(); i++) {
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
		/* Do some initial calculations */
		auto Border = this->EffectSize();
		auto TextBorderSize = Props.TextBorderSize;
		GUI.SetFont(Props.pFont);
		auto ColorIndex = (GetStates() & WIDGET_STATE_FOCUS) ? 2 : 1;
		auto s = _GetpItem(Sel);
		auto r = WM_GetClientRect() / Border;
		auto InnerSize = r.YSize();
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
		r = WM_GetClientRect() / Border;
		r.x0 = r.x1 + 1 - InnerSize;
		GUI.SetColor(RGB_GRAYL(0xc0));
		GUI_FillRect(r);
		GUI.SetColor(RGB_BLACK);
		_DrawTriangleDown((r.x1 + r.x0) / 2, r.y0 + 5, (r.YSize()) / 3);
		DrawUp(r);
	}
	void _OnTouch(const PID_STATE *pState) {
		if (pState) { /* Something happened in our area (pressed or released) */
			if (pState->Pressed) {
				Expand();
				NotifyParent(WM_NOTIFICATION_CLICKED);
			}
			else
				NotifyParent(WM_NOTIFICATION_RELEASED);
		}
		else /* Mouse moved out */
			NotifyParent(WM_NOTIFICATION_MOVED_OUT);
	}
	bool _OnKey(const WM_KEY_INFO *pInfo) {
		if (pInfo->PressedCnt > 0) {
			int Key = pInfo->Key;
			switch (Key) {
				case GUI_KEY_TAB:
					break; /* Send to parent by not doing anything */
				default:
					AddKey(Key);
					return true; /* Message handled */
			}
		}
		return false;
	}
	void _AdjustHeight() {
		auto Height = TextHeight;
		if (!Height)
			Height = Props.pFont->DistY();
		Height += EffectSize() + 2 * Props.TextBorderSize;
		SetSize({ GetSizeX(), Height });
	}

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (DropDown *)hWin;
		/* Let widget handle the standard messages */
		if (!pObj->HandleActive(MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_NOTIFY_PARENT: {
				auto pInfo = (const NOTIFY_INFO *)Data;
				switch (pInfo->Notification) {
					case WM_NOTIFICATION_SCROLL_CHANGED:
						pObj->NotifyParent(WM_NOTIFICATION_SCROLL_CHANGED);
						break;
					case WM_NOTIFICATION_CLICKED: {
						auto pListWin = (ListBox *)pInfo->pWinSrc;
						int Sel = pListWin->GetSel();
						pObj->SetSel(Sel);
						break;
					}
					case WM_NOTIFICATION_RELEASED:
						pObj->Collapse();
						pObj->SetFocus();
						break;
					case LISTBOX_NOTIFICATION_LOST_FOCUS:
						pObj->Collapse();
						break;
				}
				return 0;
			}
			case WM_PID_STATE_CHANGED:
				if (auto pInfo = (const PID_CHANGED_INFO *)Data)
					if (pInfo->State)
						pObj->Expand();
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
	DropDown(RECT r, WM_CF Style, WObj* pParent, uint16_t Id,
			 uint8_t ExFlags) :
		Widget({ r.x0, r.y0, r.x1, r.y0 - 1 }, Style, _Callback, pParent, Id, WIDGET_STATE_FOCUSSABLE),
		ySizeEx(r.YSize()), Flags(ExFlags) {
		_AdjustHeight();
	}
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new DropDown(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			0, hWinParent, pCreateInfo->Id,
			(uint8_t)pCreateInfo->Flags
		);
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
		auto NumItems = GetNumItems();
		auto r = GetRect();
		if (Flags & DROPDOWN_CF_UP)
			r.y0 -= ySizeEx;
		else
			r.y0 = r.y1 + 1;
		r.y1 = r.y0 + ySizeEx;
		if (!pListWin) {
			pListWin = new ListBox(r, WC_VISIBLE | WC_STAYONTOP | WC_ACTIVATE, nullptr, 0);
			pListWin->SetEffect(WIDGET_Effect_3D1L);
			if (pListWin) {
				if (Flags & DROPDOWN_SF_AUTOSCROLLBAR) {
					pListWin->SetScrollbarWidth(this->ScrollbarWidth);
					pListWin->SetAutoScrollV(1);
				}
				pListWin->SetOwner(this);
			}
		}
		else {
			pListWin->MoveTo(r.LeftTop());
			pListWin->ShowWindow();
		}
		if (pListWin) {
			while (pListWin->GetNumItems() > 0)
				pListWin->DeleteItem(0);
			for (int i = 0; i < NumItems; i++)
				pListWin->AddString(_GetpItem(i));
			pListWin->Props.pFont = Props.pFont;
			pListWin->Props.aBkColor[0] = Props.aBkColor[0];
			pListWin->Props.aBkColor[1] = Props.aBkColor[1];
			pListWin->Props.aBkColor[2] = Props.aBkColor[2];
			pListWin->Props.aBkColor[3] = Props.aBkColor[3];
			pListWin->Props.aTextColor[0] = Props.aTextColor[0];
			pListWin->Props.aTextColor[1] = Props.aTextColor[1];
			pListWin->Props.aTextColor[2] = Props.aTextColor[2];
			pListWin->Props.aTextColor[3] = Props.aTextColor[3];

			pListWin->SetItemSpacing(this->ItemSpacing);
			pListWin->SetSel(this->Sel);
			NotifyParent(WM_NOTIFICATION_CLICKED);
			pListWin->SetCapture(0);
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
			Invalidate();
		}
	}

	auto GetNumItems() { return Handles.NumItems(); }
	void SetFont(PCFONT pFont) {
		auto OldHeight = Props.pFont->DistY();
		Props.pFont = pFont;
		_AdjustHeight();
		Invalidate();
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
			Invalidate();
			if (this->pListWin) {
				this->pListWin->SetBkColor(Index, color);
			}
		}
	}
	void SetTextColor(DROPDOWN_CI Index, RGBC color) {
		if (Index < GUI_COUNTOF(Props.aTextColor)) {
			Props.aTextColor[Index] = color;
			Invalidate();
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
			Invalidate();
			NotifyParent(WM_NOTIFICATION_SEL_CHANGED);
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
			Invalidate();
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
				Invalidate();
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
			Invalidate();
		}
	}
	void SetTextHeight(unsigned TextHeight) {
		this->TextHeight = TextHeight;
		_AdjustHeight();
		Invalidate();
	}
};

DropDown::Properties DropDown::DefaultProps;

}
