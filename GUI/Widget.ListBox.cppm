export module TUX.Widget.ListBox;

#include "GUIConf.h"

import TUX.Widget;
import TUX.Widget.ScrollBar;

import TUX.Array;

#define LISTBOX_ITEM_SELECTED (1 << 0)
#define LISTBOX_ITEM_DISABLED (1 << 1)

export {

constexpr int LISTBOX_ALL_ITEMS  = -1;

constexpr auto LISTBOX_NOTIFICATION_LOST_FOCUS = WM_NOTIFICATION_WIDGET<0>;

constexpr uint16_t
	LISTBOX_CF_AUTOSCROLLBAR_H      = WIDGET_STATE_USER<0>,
	LISTBOX_CF_AUTOSCROLLBAR_V      = WIDGET_STATE_USER<1>,
	LISTBOX_CF_MULTISEL             = WIDGET_STATE_USER<2>;

enum LISTBOX_CI {
	 LISTBOX_CI_UNSEL = 0,
	 LISTBOX_CI_SEL_UNFOCUS,
	 LISTBOX_CI_SEL_FOCUSSED,
	 LISTBOX_CI_DISABLED
};

class ListBox : public Widget {
	friend class DropDown;

public:
	struct Properties {
		PCFONT pFont{ GUI_DEFAULT_FONT };
		BRUSH aBrush[4]{
			/* Index                | Background      | Text      */
			/* Unselect          */ { RGB_WHITE       , RGB_BLACK },
			/* Selected          */ { RGB_GRAY        , RGB_WHITE },
			/* Selected focussed */	{ RGB_DARKBLUE    , RGB_WHITE },
			/* Disabled          */ { RGBC::Gray(0xC0), RGB_GRAY  }
		};
		uint8_t ScrollStepH{ 10 };
	} static DefaultProps;

private:
	Properties Props = DefaultProps;

	struct Item {
		mutable uint16_t xSize, ySize;
		uint8_t Status;
		char *pText;
	};

	ARRAY<Item> items;
	WIDGET_DRAW_ITEM_FUNC *pfDrawItem = nullptr;
	SCROLL_STATE ScrollStateV, ScrollStateH;
	WObj *pOwner = nullptr;
	int16_t Sel = 0; /* current selection */
	uint16_t ScrollbarWidth = 0;
	uint16_t ItemSpacing = 0;

	~ListBox() {
		for (uint16_t i = 0, n = items.NumItems(); i < n; i++)
			GUI_MEM_Free(items[i].pText);
		items.Delete();
	}

	static int _Tolower(int Key) {
		if (Key >= 0x41 && Key <= 0x5a)
			Key += 0x20;
		return Key;
	}
	static bool _IsAlphaNum(int Key) {
		Key = _Tolower(Key);
		if (Key >= 'a' && Key <= 'z')
			return true;
		if (Key >= '0' && Key <= '9')
			return true;
		return false;
	}

	void _NotifyOwner(int Notification) {
		auto pOwner = this->pOwner ? this->pOwner : Parent();
		NOTIFY_INFO Info;
		Info.Notification = Notification;
		Info.pWinSrc = this;
		pOwner->Require(WM_NOTIFY_PARENT, (WM_PARAM)&Info);
	}

	auto _CallOwnerDraw(int Cmd, int ItemIndex, POINT Pos) const {
		if (pfDrawItem)
			return pfDrawItem(this, Cmd, ItemIndex, Pos);
		return OwnerDraw(this, Cmd, ItemIndex, Pos);
	}
	auto _GetYSize() const { return InsideRectEx().YSize(); }
	auto _GetItemSizeX(uint16_t Index) const {
		auto &item = items[Index];
		int xSize = item.xSize;
		if (xSize == 0) {
			PCFONT pOldFont = GUI.Font(Props.pFont);
			xSize = _CallOwnerDraw(WIDGET_ITEM_GET_XSIZE, Index, {});
			GUI.Font(pOldFont);
		}
		return item.xSize = xSize;
	}
	auto _GetItemSizeY(uint16_t Index) const {
		auto &item = items[Index];
		int ySize = item.ySize;
		if (!ySize) {
			PCFONT pOldFont = GUI.Font(Props.pFont);
			ySize = _CallOwnerDraw(WIDGET_ITEM_GET_YSIZE, Index, {});
			GUI.Font(pOldFont);
		}
		return item.ySize = ySize;
	}
	int _GetContentsSizeX() {
		int Result = 0;
		auto NumItems = GetNumItems();
		for (auto i = 0; i < NumItems; i++) {
			auto SizeX = _GetItemSizeX(i);
			if (Result < SizeX)
				Result = SizeX;
		}
		return Result;
	}
	int _GetItemPosY(uint16_t Index) {
		if (Index < GetNumItems()) {
			if (Index >= ScrollStateV.v) {
				uint16_t i;
				int PosY = 0;
				for (i = ScrollStateV.v; i < Index; i++)
					PosY += _GetItemSizeY(i);
				return PosY;
			}
		}
		return -1;
	}
	bool _IsPartiallyVis() {
		if (Sel < GetNumItems())
			if (Sel >= ScrollStateV.v)
				if (_GetItemPosY(Sel) + _GetItemSizeY(Sel) > _GetYSize())
					return true;
		return false;
	}
	uint16_t _GetNumVisItems() {
		int r = 1;
		auto NumItems = GetNumItems();
		if (NumItems > 1) {
			int i, ySize, DistY = 0;
			ySize = _GetYSize();
			for (i = NumItems - 1; i >= 0; i--) {
				DistY += _GetItemSizeY(i);
				if (DistY > ySize)
					break;
			}
			r = NumItems - i - 1;
			if (r < 1)
				return 1;
		}
		return r;
	}
	int _UpdateScrollPos() {
		auto PrevScrollStateV = ScrollStateV.v;
		if (Sel >= 0) {
			/* Check upper limit */
			if (_IsPartiallyVis())
				ScrollStateV.v = Sel - ScrollStateV.PageSize + 1;
			/* Check lower limit */
			if (ScrollStateV.v > Sel)
				ScrollStateV.v = Sel;
		}
		ScrollStateV.Bounds();
		ScrollStateH.Bounds();
		ScrollState(ScrollStateV, ScrollStateH);
		return ScrollStateV.v - PrevScrollStateV;
	}
	void _InvalidateItemSize(uint16_t Index) {
		auto &item = items[Index];
		item.xSize = item.ySize = 0;
	}
	void _InvalidateInsideArea() {
		auto r = InsideRectEx();
		Invalidate(&r);
	}
	void _InvalidateItem(int Sel) {
		if (Sel < 0)
			return;
		auto ItemPosY = _GetItemPosY(Sel);
		if (ItemPosY < 0)
			return;
		auto r = InsideRectEx();
		r.y0 += ItemPosY;
		r.y1 = r.y0 + _GetItemSizeY(Sel) - 1;
		Invalidate(&r);
	}
	void _InvalidateItemAndBelow(int Sel) {
		if (Sel < 0)
			return;
		auto ItemPosY = _GetItemPosY(Sel);
		if (ItemPosY < 0)
			return;
		auto r = InsideRectEx();
		r.y0 += ItemPosY;
		Invalidate(&r);
	}
	void _SetScrollbarWidth() {
		if (auto pScroll = (ScrollBar *)GetScrollbarH())
			pScroll->SetWidth(ScrollbarWidth);
		if (auto pScroll = (ScrollBar *)GetScrollbarV())
			pScroll->SetWidth(ScrollbarWidth);
	}
	int _CalcScrollParas() {
		/* Calc vertical scroll parameters */
		ScrollStateV.NumItems = GetNumItems();
		ScrollStateV.PageSize = _GetNumVisItems();
		/* Calc horizontal scroll parameters */
		ScrollStateH.NumItems = _GetContentsSizeX();
		ScrollStateH.PageSize = InsideRectEx().XSize();
		return _UpdateScrollPos();
	}
	void _ManageAutoScroll() {
		if (States & LISTBOX_CF_AUTOSCROLLBAR_V)
			SetScrollbarV(_GetNumVisItems() < GetNumItems());
		if (States & LISTBOX_CF_AUTOSCROLLBAR_H)
			SetScrollbarH(_GetContentsSizeX() > InsideRectEx().XSize());
		if (ScrollbarWidth)
			_SetScrollbarWidth();
	}
	void _SelectByKey(int Key) {
		Key = _Tolower(Key);
		for (uint16_t i = 0; i < GetNumItems(); i++) {
			auto s = items[i].pText;
			if (_Tolower(*s) == Key) {
				SetSel(i);
				break;
			}
		}
	}
	void _OnPaint() {
		GUI.Font(Props.pFont);
		/* Calculate clipping rectangle */
		auto rClip = GetInvalidRect() - LeftTop();
		auto rInside = InsideRectEx();
		rClip &= rInside;
		RECT rItem{ rClip.x0, 0, rClip.x1, 0 };
		/* Fill item info structure */
		POINT ItemPos{
			rInside.x0 - ScrollStateH.v,
			rInside.y0
		};
		/* Do the drawing */
		for (int i = ScrollStateV.v, NumItems = GetNumItems(); i < NumItems; i++) {
			rItem.y0 = ItemPos.y;
			/* Break when all other rows are outside the drawing area */
			if (rItem.y0 > rClip.y1)
				break;
			auto ItemDistY = _GetItemSizeY(i);
			rItem.y1 = rItem.y0 + ItemDistY - 1;
			/* Make sure that we draw only when row is in drawing area */
			if (rItem.y1 >= rClip.y0) {
				/* Set user clip rect */
				UserClip(&rItem);
				/* Draw item */
				_CallOwnerDraw(WIDGET_ITEM_DRAW, i, POINT{ ItemPos.x, ItemPos.y });
			}
			ItemPos.y += ItemDistY;
		}
		UserClip(nullptr);
		/* Calculate & clear 'data free' area */
		rItem.y0 = ItemPos.y;
		rItem.y1 = rInside.y1;
		GUI.BkColor(Props.aBrush[0].BkColor);
		GUI.Clear(rItem);
		/* Draw the 3D effect (if configured) */
		DrawDown();
	}
	void _ToggleMultiSel(int Sel) {
		if (States & LISTBOX_CF_MULTISEL) {
			auto &item = items[Sel];
			if (!(item.Status & LISTBOX_ITEM_DISABLED)) {
				item.Status ^= LISTBOX_ITEM_SELECTED;
				_NotifyOwner(WM_NOTIFICATION_SEL_CHANGED);
				_InvalidateItem(Sel);
			}
		}
	}
	int _GetItemFromPos(POINT Pos) {
		auto r = InsideRectEx();
		if (!(r <= Pos))
			return -1;
		auto Sel = -1;
		auto y0 = r.y0;
		for (int i = ScrollStateV.v, NumItems = GetNumItems(); i < NumItems; i++) {
			if (Pos.y >= y0)
				Sel = i;
			y0 += _GetItemSizeY(i);
		}
		return Sel;
	}
	void _OnTouch(const PID_STATE *pState) {
		if (pState) { /* Something happened in our area (pressed or released) */
			if (pOwner)
				if (!(ClientRect() <= *pState)) {
					if (pState->Pressed)
						_NotifyOwner(LISTBOX_NOTIFICATION_LOST_FOCUS);
					return;
				}
			if (pState->Pressed == 0)
				_NotifyOwner(WM_NOTIFICATION_RELEASED);
		}
		else /* Mouse moved out */
			_NotifyOwner(WM_NOTIFICATION_MOVED_OUT);
	}
#if GUI_SUPPORT_MOUSE
	void _OnMouseOver(const PID_STATE *pState) {
		if (this->pOwner) {
			if (pState) {  /* Something happened in our area (pressed or released) */
				int Sel = _GetItemFromPos(*pState);
				if (Sel >= 0)
					if (Sel < (int)(ScrollStateV.v + _GetNumVisItems()))
						SetSel(Sel);
			}
		}
	}
#endif
	bool _OnKey(const KEY_STATE *pInfo) {
		if (pInfo->PressedCnt > 0) {
			switch (auto Key = pInfo->Key) {
			case ' ':
				_ToggleMultiSel(Sel);
				return true; /* Key has been consumed */
			case GUI_KEY_RIGHT:
				if (ScrollStateH.SetValue(ScrollStateH.v + Props.ScrollStepH)) {
					UpdateScrollers();
					_InvalidateInsideArea();
				}
				return true; /* Key has been consumed */
			case GUI_KEY_LEFT:
				if (ScrollStateH.SetValue(ScrollStateH.v - Props.ScrollStepH)) {
					UpdateScrollers();
					_InvalidateInsideArea();
				}
				return true; /* Key has been consumed */
			case GUI_KEY_DOWN:
				IncSel();
				return true; /* Key has been consumed */
			case GUI_KEY_UP:
				DecSel();
				return true; /* Key has been consumed */
			default:
				if (_IsAlphaNum(Key)) {
					_SelectByKey(Key);
					return true; /* Key has been consumed */
				}
			}
		}
		return false; /* Key has not been consumed */
	}
	void _MoveSel(int Dir) {
		int NewSel = -1;
		auto NumItems = GetNumItems();
		do {
			Sel += Dir;
			if (Sel < 0 || Sel >= NumItems)
				break;
			if (!(items[Sel].Status & LISTBOX_ITEM_DISABLED))
				NewSel = Sel;
		} while (NewSel < 0);
		if (NewSel >= 0)
			SetSel(NewSel);
	}

	static WM_PARAM _Callback(WObj *pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (ListBox *)pWin;
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_NOTIFY_PARENT: {
				auto pInfo = (const NOTIFY_INFO *)Data;
				auto pWinSrc = pInfo->pWinSrc;
				switch (pInfo->Notification) {
					case WM_NOTIFICATION_VALUE_CHANGED:
						if (pWinSrc == pObj->GetScrollbarV())
							pObj->ScrollStateV.v = pWinSrc->ScrollState().v;
						else if (pWinSrc == pObj->GetScrollbarH())
							pObj->ScrollStateH.v = pWinSrc->ScrollState().v;
						else break;
						pObj->_InvalidateInsideArea();
						pObj->_NotifyOwner(WM_NOTIFICATION_SCROLL_CHANGED);
						break;
					case WM_NOTIFICATION_SCROLLBAR_ADDED:
						pObj->UpdateScrollers();
						break;
				}
				return 0;
			}
			case WM_PID_STATE_CHANGED: {
				auto pInfo = (const PID_CHANGED_INFO *)Data;
				if (pInfo->Pressed) {
					auto Sel = pObj->_GetItemFromPos(*pInfo);
					if (Sel >= 0) {
						pObj->_ToggleMultiSel(Sel);
						pObj->SetSel(Sel);
					}
					pObj->_NotifyOwner(WM_NOTIFICATION_CLICKED);
				}
				break;
			}
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
				return 0;
#if GUI_SUPPORT_MOUSE
			case WM_MOUSEOVER:
				pObj->_OnMouseOver((const PID_STATE *)Data);
				return 0;
#endif
			case WM_KEY:
				if (pObj->_OnKey((const KEY_STATE *)Data))
					return 0;
				break;
			case WM_SIZE:
				pObj->UpdateScrollers();
				pObj->Invalidate();
				return 0;
			case WM_DELETE:
				pObj->~ListBox();
				return 0;
		}
		return pObj->WidgetProc(MsgId, Data);
	}

public:
	ListBox(RECT r, WM_CF Style, WObj *pParent, uint16_t Id) : 
		Widget(r, Style, _Callback, pParent, Id, WIDGET_STATE_FOCUSSABLE) {
		UpdateScrollers();
	}
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *pWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new ListBox(
			RECT(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0,
				 pCreateInfo->x0 + x0 + pCreateInfo->xSize - 1,
				 pCreateInfo->y0 + y0 + pCreateInfo->ySize - 1),
			pCreateInfo->Flags, pWinParent, pCreateInfo->Id);
	}

public:

#pragma region Properties
	UCFONT Font() const { return *Props.pFont; }
	void Font(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		InvalidateItem(LISTBOX_ALL_ITEMS);
	}

	void Brush(LISTBOX_CI Index, BRUSH aBrush) {
		if (Props.aBrush[Index] == aBrush)
			return;
		Props.aBrush[Index] = aBrush;
		_InvalidateInsideArea();
	}
#pragma endregion

	auto UpdateScrollers() {
		_ManageAutoScroll();
		return _CalcScrollParas();
	}

#pragma region OwnerDraw
private:
	void _PaintItem(uint16_t ItemIndex, POINT Pos) const {
		auto &item = items[ItemIndex];
		auto rInside = InsideRect();
		/* Calculate color index */
		auto ColorIndex =
			item.Status & LISTBOX_ITEM_DISABLED ? LISTBOX_CI_DISABLED :
			States & LISTBOX_CF_MULTISEL ?
			item.Status & LISTBOX_ITEM_SELECTED ? LISTBOX_CI_SEL_FOCUSSED : LISTBOX_CI_UNSEL :
			ItemIndex != Sel ? LISTBOX_CI_UNSEL :
			States & WIDGET_STATE_FOCUS || pOwner ? LISTBOX_CI_SEL_FOCUSSED : LISTBOX_CI_SEL_UNFOCUS;
		/* Display item */
		GUI.Brush(Props.aBrush[ColorIndex]);
		GUI.Clear();
		RECT rText{ Pos, { rInside, Pos.y + _GetItemSizeY(ItemIndex) - 1 } };
		GUI_DispStringInRect(items[ItemIndex].pText, rText, TEXTALIGN_LEFT | TEXTALIGN_VCENTER);
		/* Display focus rectangle */
		if ((States & LISTBOX_CF_MULTISEL) && ItemIndex == Sel) {
			GUI.Color(RGB_WHITE - Props.aBrush[ColorIndex].BkColor);
			GUI_DrawFocusRect(rText, 0);
		}
	}
public:
	static int OwnerDraw(const Widget *pWidget, int Cmd, int ItemIndex, POINT Pos) {
		auto pObj = (const ListBox *)pWidget;
		switch (Cmd) {
			case WIDGET_ITEM_GET_XSIZE: {
				auto s = pObj->items[ItemIndex].pText;
				return pObj->Props.pFont->TextBound(s).x;
			}
			case WIDGET_ITEM_GET_YSIZE: {
				auto s = pObj->items[ItemIndex].pText;
				return pObj->Props.pFont->TextBound(s).y + pObj->ItemSpacing;
			}
			case WIDGET_ITEM_DRAW: 
				pObj->_PaintItem(ItemIndex, Pos);
				break;
		}
		return 0;
	}
#pragma endregion

	void InvalidateItem(int Index) {
		auto NumItems = GetNumItems();
		if (Index >= NumItems)
			return;
		if (Index < 0) {
			for (int i = 0; i < NumItems; i++)
				_InvalidateItemSize(i);
			UpdateScrollers();
			_InvalidateInsideArea();
		}
		else {
			_InvalidateItemSize(Index);
			UpdateScrollers();
			_InvalidateItemAndBelow(Index);
		}
	}

	void AddString(const char *s) {
		if (!s) return;
		Item item = { 0, 0 };
		if (items.AddItem(&item) == 0) {
			uint16_t ItemIndex = items.NumItems() - 1;
			GUI__SetText(items[ItemIndex].pText, s);
			_InvalidateItemSize(ItemIndex);
			UpdateScrollers();
			_InvalidateItem(ItemIndex);
		}
	}
	void SetText(const char **ppText) {
		if (!ppText) return;
		while (auto s = *(ppText++))
			AddString(s);
		InvalidateItem(LISTBOX_ALL_ITEMS);
	}

	auto GetSel() const { return Sel; }
	void SetSel(int NewSel) {
		auto MaxSel = GetNumItems();
		MaxSel = MaxSel ? MaxSel - 1 : 0;
		if (NewSel > MaxSel)
			NewSel = MaxSel;
		if (NewSel < 0)
			NewSel = -1;
		else if (items[NewSel].Status & LISTBOX_ITEM_DISABLED)
			NewSel = -1;
		if (NewSel != Sel) {
			auto OldSel = Sel;
			Sel = NewSel;
			if (_UpdateScrollPos())
				_InvalidateInsideArea();
			else {
				_InvalidateItem(OldSel);
				_InvalidateItem(NewSel);
			}
			_NotifyOwner(WM_NOTIFICATION_SEL_CHANGED);
		}
	}
	void IncSel() { _MoveSel(1); }
	void DecSel() { _MoveSel(-1); }

	bool GetMulti() const { return States & LISTBOX_CF_MULTISEL; }
	void SetMulti(bool Mode) {
		if (CtlStates(LISTBOX_CF_MULTISEL, Mode))
			_InvalidateInsideArea();
	}

	auto GetNumItems() const { return items.NumItems(); }
	void DeleteItem(uint16_t Index) {
		auto NumItems = GetNumItems();
		if (Index < NumItems) {
			GUI_MEM_FreePtr((void **)&items[Index].pText);
			items.Delete(Index);
			/*
			 * Update selection
			 */
			if (Sel >= 0) {              /* Valid selction ? */
				if (Index == Sel)        /* Deleting selected item ? */
					Sel = -1;            /* Invalidate selection */
				else if (Index < Sel)    /* Deleting item above selection ? */
					Sel--;
			}
			if (UpdateScrollers())
				_InvalidateInsideArea();
			else
				_InvalidateItemAndBelow(Index);
		}
	}
	void InsertItem(uint16_t Index, const char *s) {
		if (!s) return;
		if (Index >= GetNumItems()) {
			AddString(s);
			return;
		}
		auto &item = items.Insert(Index);
		GUI__SetText(item.pText, s);
		InvalidateItem(Index);
	}
	int GetItemDisabled(uint16_t Index) const {
		if (Index >= GetNumItems()) 
			return true;
		return items[Index].Status & LISTBOX_ITEM_DISABLED;
	}
	void SetItemDisabled(uint16_t Index, bool OnOff) {
		if (Index >= GetNumItems())
			return;
		auto &item = items[Index];
		if (OnOff) {
			if (!(item.Status & LISTBOX_ITEM_DISABLED)) {
				item.Status |= LISTBOX_ITEM_DISABLED;
				_InvalidateItem(Index);
			}
		}
		else if (item.Status & LISTBOX_ITEM_DISABLED) {
			item.Status &= ~LISTBOX_ITEM_DISABLED;
			_InvalidateItem(Index);
		}
	}

	void GetItemText(uint16_t Index, char *pBuffer, int MaxSize) const {
		auto NumItems = GetNumItems();
		if (Index >= NumItems || !pBuffer || MaxSize <= 0)
			return;
		const char *pString;
		int CopyLen;
		pString = items[Index].pText;
		CopyLen = GUI__strlen(pString);
		if (CopyLen > MaxSize - 1)
			CopyLen = MaxSize - 1;
		GUI__memcpy(pBuffer, pString, CopyLen);
		pBuffer[CopyLen] = 0;
	}
	void SetString(uint16_t Index, const char *s) {
		if (Index < GetNumItems()) {
			if (GUI__SetText(items[Index].pText, s)) {
				_InvalidateItemSize(Index);
				UpdateScrollers();
				_InvalidateItem(Index);
			}
		}
	}

	bool GetItemSel(uint16_t Index) const {
		if (Index >= GetNumItems() || !(States & LISTBOX_CF_MULTISEL))
			return false;
		return items[Index].Status & LISTBOX_ITEM_SELECTED;
	}
	void SetItemSel(uint16_t Index, bool OnOff) {
		if (Index >= GetNumItems() || !(States & LISTBOX_CF_MULTISEL))
			return;
		auto &item = items[Index];
		if (OnOff) {
			if (!(item.Status & LISTBOX_ITEM_SELECTED)) {
				item.Status |= LISTBOX_ITEM_SELECTED;
				_InvalidateItem(Index);
			}
		}
		else if (item.Status & LISTBOX_ITEM_SELECTED) {
			item.Status &= ~LISTBOX_ITEM_SELECTED;
			_InvalidateItem(Index);
		}
	}

	auto GetItemSpacing() const { return ItemSpacing; }
	void SetItemSpacing(uint16_t Value) {
		if (ItemSpacing == Value)
			return;
		this->ItemSpacing = Value;
		InvalidateItem(LISTBOX_ALL_ITEMS);
	}

	auto GetScrollStepH() const { return Props.ScrollStepH; }
	void SetScrollStepH(int Value) { Props.ScrollStepH = Value; }

	auto GetScrollbarWidth() const { return ScrollbarWidth; }
	void SetScrollbarWidth(uint16_t Width) {
		if (ScrollbarWidth == Width)
			return;
		ScrollbarWidth = Width;
		_SetScrollbarWidth();
		Invalidate();
	}

	void SetAutoScrollH(bool State) {
		auto Flags = States & (~LISTBOX_CF_AUTOSCROLLBAR_H);
		if (State)
			Flags |= LISTBOX_CF_AUTOSCROLLBAR_H;
		if (States != Flags) {
			States = Flags;
			UpdateScrollers();
		}
	}
	void SetAutoScrollV(bool State) {
		auto Flags = States & (~LISTBOX_CF_AUTOSCROLLBAR_V);
		if (State)
			Flags |= LISTBOX_CF_AUTOSCROLLBAR_V;
		if (States != Flags) {
			States = Flags;
			UpdateScrollers();
		}
	}

	void SetOwner(WObj *pOwner) {
		this->pOwner = pOwner;
		_InvalidateInsideArea();
	}
	void SetOwnerDraw(WIDGET_DRAW_ITEM_FUNC *pfDrawItem) {
		this->pfDrawItem = pfDrawItem;
		InvalidateItem(LISTBOX_ALL_ITEMS);
	}
};

ListBox::Properties ListBox::DefaultProps;

}
