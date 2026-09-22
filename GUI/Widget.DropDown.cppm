export module TUX.Widget.DropDown;

#include "GUIConf.h"

import TUX.Widget;
import TUX.Widget.ListBox;

import TUX.Array;

export {
  
constexpr uint16_t
	DROPDOWN_CF_AUTOSCROLLBAR = WIDGET_STATE_USER<0>,
	DROPDOWN_CF_UP            = WIDGET_STATE_USER<1>;

using DROPDOWN_CI = LISTBOX_CI;

class DropDown : public Widget {

public:
	struct Properties {
		PCFONT pFont{ GUI_DEFAULT_FONT };
		BRUSH aBrush[4] {
			/* Index                | Background      | Text         */
			/* Unselect          */ { RGB_WHITE       , RGB_BLACK    },
			/* Selected          */ { RGB_GRAY        , RGB_WHITE    },
			/* Selected focussed */ { RGB_DARKBLUE    , RGB_WHITE    },
			/* Disabled          */	{ RGBC::Gray(0xC0), RGB_GRAY     }
		};
		int16_t TextBorderSize{ 2 };
		TEXTALIGN Align{ TEXTALIGN_LEFT };
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;

	int16_t Sel = 0; /* current selection */
	int16_t ySizeEx; /* Drop down size */
	int16_t TextHeight = 0;
	ARRAY<char *> Handles;
	ListBox *pListWin = nullptr;
	uint16_t ItemSpacing = 0;
	uint8_t  ScrollbarWidth = 0;

	static int _Tolower(int Key) {
		if (Key >= 0x41 && Key <= 0x5a)
			Key += 0x20;
		return Key;
	}
	
	const char *_GetpItem(int Index) const {
		if (Index < 0 || Index >= GetNumItems())
			return nullptr;
		return Handles[Index];
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
		for (int i = 0; i < Handles.NumItems(); i++)
			GUI__SetText(Handles[i], nullptr);
		Handles.Delete();
		delete pListWin;
		this->pListWin = nullptr;
	}
	void _DrawTriangleDown(int x, int y, int Size) const {
		while (Size--)
			GUI.DrawHLine(y++, x - Size, x + Size);
	}
	void _OnPaint() const {
		/* Do some initial calculations */
		auto Border = this->EffectSize();
		auto TextBorderSize = Props.TextBorderSize;
		GUI.Font(Props.pFont);
		auto ColorIndex = (States & WIDGET_STATE_FOCUS) ? 2 : 1;
		auto s = _GetpItem(Sel);
		auto r = ClientRect() / Border;
		auto InnerSize = r.YSize();
		/* Draw the 3D effect (if configured) */
		DrawDown();
		/* Draw the outer text frames */
		r.x1 -= InnerSize; /* Spare square area to the right */
		GUI.Brush(Props.aBrush[ColorIndex]);
		/* Draw the text */
		GUI.FillRect(r);
		r.x0 += TextBorderSize;
		r.x1 -= TextBorderSize;
		GUI_DispStringInRect(s, r, Props.Align);
		/* Draw arrow */
		r = ClientRect() / Border;
		r.x0 = r.x1 + 1 - InnerSize;
		GUI.Color(RGBC::Gray(0xc0));
		GUI.FillRect(r);
		GUI.Color(RGB_BLACK);
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
	bool _OnKey(const KEY_STATE *pInfo) {
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
			Height = Props.pFont->YSize;
		Height += EffectSize() + 2 * Props.TextBorderSize;
		Size({ SizeX(), Height });
	}

	static WM_PARAM _Callback(WObj *pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (DropDown *)pWin;
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
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
					if (pInfo->Pressed)
						pObj->Expand();
				return 0;
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
				return 0;
			case WM_DELETE:
				pObj->_FreeAttached();
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const KEY_STATE *)Data))
					return 0;
				break;
		}
		return pObj->WidgetProc(MsgId, Data);
	}

public:
	DropDown(RECT r, WM_CF Style, WObj *pParent, uint16_t Id) :
		Widget({ r.x0, r.y0, r.x1, r.y0 - 1 }, Style, _Callback, pParent, Id, WIDGET_STATE_FOCUSSABLE),
		ySizeEx(r.YSize())
	{ _AdjustHeight(); }
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *pWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new DropDown(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			0, pWinParent, pCreateInfo->Id
		);
	}

public:

#pragma region Properties
	void Font(PCFONT pFont) {
		auto OldHeight = Props.pFont->YSize;
		Props.pFont = pFont;
		_AdjustHeight();
		Invalidate();
		if (pListWin) {
			if (OldHeight != Props.pFont->YSize) {
				Collapse();
				Expand();
			}
			pListWin->Font(pFont);
		}
	}

	void TextAlign(TEXTALIGN Align) {
		if (Props.Align == Align)
			return;
		Props.Align = Align;
		Invalidate();
	}

	void Brush(DROPDOWN_CI Index, BRUSH aBrush) {
		if (Props.aBrush[Index] == aBrush)
			return;
		Props.aBrush[Index] = aBrush;
		Invalidate();
		if (pListWin)
			pListWin->Brush(Index, aBrush);
	}
#pragma endregion

	void Collapse() {
		if (pListWin) {
			ReleaseCapture();
			delete pListWin;
			pListWin = nullptr;
		}
	}
	void Expand() {
		auto NumItems = GetNumItems();
		auto r = Rect();
		if (States & DROPDOWN_CF_UP)
			r.y0 -= ySizeEx;
		else
			r.y0 = r.y1 + 1;
		r.y1 = r.y0 + ySizeEx;
		if (!pListWin) {
			pListWin = new ListBox(r, WC_VISIBLE | WC_STAYONTOP | WC_ACTIVATE, nullptr, 0);
			pListWin->SetEffect(WIDGET_Effect_3D1L);
			if (pListWin) {
				if (States & DROPDOWN_CF_AUTOSCROLLBAR) {
					pListWin->SetScrollbarWidth(this->ScrollbarWidth);
					pListWin->SetAutoScrollV(true);
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
			pListWin->Props.aBrush[0] = Props.aBrush[0];
			pListWin->Props.aBrush[1] = Props.aBrush[1];
			pListWin->Props.aBrush[2] = Props.aBrush[2];
			pListWin->Props.aBrush[3] = Props.aBrush[3];

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

	void SetSel(int Sel) {
		auto NumItems = GetNumItems();
		auto MaxSel = NumItems ? NumItems - 1 : 0;
		if (Sel > MaxSel)
			Sel = MaxSel;
		if (Sel != this->Sel) {
			this->Sel = Sel;
			Invalidate();
			NotifyParent(WM_NOTIFICATION_SEL_CHANGED);
		}
	}
	void IncSel() { SetSel(Sel + 1); }
	void DecSel() {
		int Sel = GetSel();
		if (Sel)
			Sel--;
		SetSel(Sel);
	}
	auto GetSel() const { return Sel; }

	auto GetNumItems() const { return Handles.NumItems(); }
	void AddString(const char *s) {
		if (!s) return;
		auto idx = Handles.NumItems();
		if (Handles.AddItem() == 0)
			GUI__SetText(Handles[idx], s);
		Invalidate();
	}
	void DeleteItem(uint16_t Index) {
		if (Index >= GetNumItems()) 
			return;
		GUI__SetText(Handles[Index], nullptr);
		Handles.DeleteItem(Index);
		Invalidate();
		if (pListWin)
			pListWin->DeleteItem(Index);
	}
	void InsertString(const char *s, unsigned int Index) {
		if (!s) return;
		auto NumItems = GetNumItems();
		if (Index < NumItems) {
			auto pp = Handles.InsertItem(Index);
			if (pp)
				GUI__SetText(*pp, s);
			Invalidate();
			if (pListWin)
				pListWin->InsertString(Index, s);
		}
		else {
			AddString(s);
			if (pListWin)
				pListWin->AddString(s);
		}
	}
	
	void SetScrollbarWidth(uint8_t Width) {
		if (ScrollbarWidth == Width)
			return;
		ScrollbarWidth = Width;
		if (pListWin)
			pListWin->SetScrollbarWidth(Width);
	}
	void SetItemSpacing(uint16_t Value) {
		ItemSpacing = Value;
		if (pListWin)
			pListWin->SetItemSpacing(Value);
	}
	auto GetItemSpacing() const { return ItemSpacing; }

	void SetAutoScroll(bool OnOff) {
		char Flags = this->States & (~DROPDOWN_CF_AUTOSCROLLBAR);
		if (OnOff)
			Flags |= DROPDOWN_CF_AUTOSCROLLBAR;
		if (this->States != Flags) {
			this->States = Flags;
			if (pListWin)
				pListWin->SetAutoScrollV((Flags & DROPDOWN_CF_AUTOSCROLLBAR) ? 1 : 0);
		}
	}
	void SetTextHeight(uint16_t TextHeight) {
		if (this->TextHeight == TextHeight)
			return;
		this->TextHeight = TextHeight;
		_AdjustHeight();
		Invalidate();
	}
};

DropDown::Properties DropDown::DefaultProps;

}
