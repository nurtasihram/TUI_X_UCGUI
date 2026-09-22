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
		uint16_t xSize, ySize;
		uint8_t Status;
		char *pText;
	};

	ARRAY<Item> ItemArray;
	WIDGET_DRAW_ITEM_FUNC *pfDrawItem = nullptr;
	SCROLL_STATE ScrollStateV, ScrollStateH;
	WObj *pOwner = nullptr;
	int16_t Sel = 0; /* current selection */
	uint16_t ScrollbarWidth = 0;
	uint16_t ItemSpacing = 0;

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

	int _CallOwnerDraw(int Cmd, int ItemIndex, POINT Pos) {
		if (pfDrawItem)
			return pfDrawItem(this, Cmd, ItemIndex, Pos);
		return OwnerDraw(this, Cmd, ItemIndex, Pos);
	}
	auto _GetYSize() const { return InsideRectEx().YSize(); }
	auto _GetItemSizeX(uint16_t Index) {
		auto &pItem = ItemArray[Index];
		int xSize = pItem.xSize;
		if (xSize == 0) {
			PCFONT pOldFont = GUI.Font(Props.pFont);
			xSize = _CallOwnerDraw(WIDGET_ITEM_GET_XSIZE, Index, {});
			GUI.Font(pOldFont);
		}
		return pItem.xSize = xSize;
	}
	auto _GetItemSizeY(uint16_t Index) {
		auto &pItem = ItemArray[Index];
		int ySize = pItem.ySize;
		if (ySize == 0) {
			PCFONT pOldFont = GUI.Font(Props.pFont);
			ySize = _CallOwnerDraw(WIDGET_ITEM_GET_YSIZE, Index, {});
			GUI.Font(pOldFont);
		}
		return pItem.ySize = ySize;
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
		auto &pItem = ItemArray[Index];
		pItem.xSize = pItem.ySize = 0;
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
			auto s = ItemArray[i].pText;
			if (_Tolower(*s) == Key) {
				SetSel(i);
				break;
			}
		}
	}
	void _FreeAttached() {
		for (unsigned _i = 0, _n = ItemArray.NumItems(); _i < _n; _i++)
			GUI_MEM_FreePtr((void **)&ItemArray[_i].pText);
		ItemArray.Delete();
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
			auto &item = ItemArray[Sel];
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
		if (pInfo->PressedCnt > 0)
			if (AddKey(pInfo->Key))
				return true; /* Key has been consumed */
		return false; /* Key has not been consumed */
	}
	void _MoveSel(int Dir) {
		int NewSel = -1;
		auto Sel = GetSel();
		auto NumItems = GetNumItems();
		do {
			Sel += Dir;
			if (Sel < 0 || Sel >= NumItems)
				break;
			if (!(ItemArray[Sel].Status & LISTBOX_ITEM_DISABLED))
				NewSel = Sel;
		} while (NewSel < 0);
		if (NewSel >= 0)
			SetSel(NewSel);
	}
	int _AddKey(int Key) {
		switch (Key) {
			case ' ':
				_ToggleMultiSel(this->Sel);
				return 1;               /* Key has been consumed */
			case GUI_KEY_RIGHT:
				if (ScrollStateH.SetValue(ScrollStateH.v + Props.ScrollStepH)) {
					UpdateScrollers();
					_InvalidateInsideArea();
				}
				return 1;               /* Key has been consumed */
			case GUI_KEY_LEFT:
				if (ScrollStateH.SetValue(ScrollStateH.v - Props.ScrollStepH)) {
					UpdateScrollers();
					_InvalidateInsideArea();
				}
				return 1;               /* Key has been consumed */
			case GUI_KEY_DOWN:
				IncSel();
				return 1;               /* Key has been consumed */
			case GUI_KEY_UP:
				DecSel();
				return 1;               /* Key has been consumed */
			default:
				if (_IsAlphaNum(Key)) {
					_SelectByKey(Key);
					return 1;               /* Key has been consumed */
				}
		}
		return 0;
	}

	static WM_PARAM _Callback(WObj *pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (ListBox *)pWin;
		/* In popup mode (pOwner set), bypass WIDGET_HandleActive for WM_PID_STATE_CHANGED.
		 * WIDGET_HandleActive internally calls SetFocus on press, which would steal
		 * focus from the dropdown and cause its parent window to flicker. */
		if (!(pObj->pOwner && MsgId == WM_PID_STATE_CHANGED)) {
			/* Let widget handle the standard messages */
			if (!pObj->HandleActive(MsgId, &Data))
				return Data;
		}
		switch (MsgId) {
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
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
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
				return 0;
			}
			case WM_TOUCH: {
				auto pState = (const PID_STATE *)Data;
				if (pObj->pOwner && pState) {
					auto r = pObj->ClientRect();
					if (pState->x < 0 || pState->y < 0 || pState->x > r.x1 || pState->y > r.y1) {
						if (pState->Pressed)
							pObj->_NotifyOwner(LISTBOX_NOTIFICATION_LOST_FOCUS);
						return 0;
					}
				}
				pObj->_OnTouch(pState);
				return 0;
			}
#if GUI_SUPPORT_MOUSE
			case WM_MOUSEOVER:
				pObj->_OnMouseOver((const PID_STATE *)Data);
				return 0;
#endif
			case WM_DELETE:
				pObj->_FreeAttached();
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const KEY_STATE *)Data))
					return 0;
				break;
			case WM_SIZE:
				pObj->UpdateScrollers();
				pObj->Invalidate();
				return 0;
		}
		return DefaultProc(pWin, MsgId, Data);
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
	void _PaintItem(int ItemIndex, POINT Pos) const {
		auto &pItem = ItemArray[ItemIndex];
		auto r = InsideRect();
		/* Calculate color index */
		auto ColorIndex =
			pItem.Status & LISTBOX_ITEM_DISABLED ? LISTBOX_CI_DISABLED :
			States & LISTBOX_CF_MULTISEL ?
			pItem.Status & LISTBOX_ITEM_SELECTED ? LISTBOX_CI_SEL_FOCUSSED : LISTBOX_CI_UNSEL :
			ItemIndex != Sel ? LISTBOX_CI_UNSEL :
			States & WIDGET_STATE_FOCUS || pOwner ? LISTBOX_CI_SEL_FOCUSSED : LISTBOX_CI_SEL_UNFOCUS;
		/* Display item */
		GUI.Brush(Props.aBrush[ColorIndex]);
		auto s = ItemArray[ItemIndex].pText;
		GUI.Clear();
		GUI_DispStringAt(s, Pos.x + 1, Pos.y);
		/* Display focus rectangle */
		if ((States & LISTBOX_CF_MULTISEL) && ItemIndex == Sel) {
			RECT rFocus;
			rFocus.LeftTop(Pos);
			rFocus.x1 = r.x1;
			rFocus.y1 = Pos.y + Props.pFont->TextBound(s).x - 1;
			GUI.Color(RGB_WHITE - Props.aBrush[ColorIndex].BkColor);
			GUI_DrawFocusRect(rFocus, 0);
		}
	}
public:
	static int OwnerDraw(WObj *pWin, int Cmd, int ItemIndex, POINT Pos) {
		auto pObj = (ListBox *)pWin;
		switch (Cmd) {
			case WIDGET_ITEM_GET_XSIZE: {
				auto s = pObj->ItemArray[ItemIndex].pText;
				return pObj->Props.pFont->TextBound(s).x;
			}
			case WIDGET_ITEM_GET_YSIZE: {
				auto s = pObj->ItemArray[ItemIndex].pText;
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
		int NumItems;
		NumItems = GetNumItems();
		if (Index < NumItems) {
			if (Index < 0) {
				int i;
				for (i = 0; i < NumItems; i++) {
					_InvalidateItemSize(i);
				}
				UpdateScrollers();
				_InvalidateInsideArea();
			}
			else {
				_InvalidateItemSize(Index);
				UpdateScrollers();
				_InvalidateItemAndBelow(Index);
			}
		}
	}

	int AddKey(int Key) {
		int r = 0;
		r = _AddKey(Key);
		return r;
	}
	void AddString(const char *s) {
		if (s) {
			Item item = { 0, 0 };
			if (ItemArray.AddItem(&item) == 0) {
				uint16_t ItemIndex = ItemArray.NumItems() - 1;
				GUI__SetText(ItemArray[ItemIndex].pText, s);
				_InvalidateItemSize(ItemIndex);
				UpdateScrollers();
				_InvalidateItem(ItemIndex);
			}
		}
	}
	void SetText(const char **ppText) {
		int i;
		const char *s;
		if (ppText) {
			for (i = 0; (s = *(ppText + i)) != 0; i++) {
				AddString(s);
			}
		}
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
		else if (ItemArray[NewSel].Status & LISTBOX_ITEM_DISABLED)
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
		if (Mode) {
			if (!(States & LISTBOX_CF_MULTISEL)) {
				States |= LISTBOX_CF_MULTISEL;
				_InvalidateInsideArea();
			}
		}
		else {
			if (States & LISTBOX_CF_MULTISEL) {
				States &= ~LISTBOX_CF_MULTISEL;
				_InvalidateInsideArea();
			}
		}
	}

	auto GetNumItems() const { return ItemArray.NumItems(); }
	void DeleteItem(uint16_t Index) {
		auto NumItems = GetNumItems();
		if (Index < NumItems) {
			GUI_MEM_FreePtr((void **)&ItemArray[Index].pText);
			ItemArray.DeleteItem(Index);
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
	void InsertString(uint16_t Index, const char *s) {
		if (!s) return;
		auto NumItems = GetNumItems();
		if (Index < NumItems) {
			if (ItemArray.InsertItem(Index)) {
				auto &pItem = ItemArray[Index];
				pItem.Status = 0;
				GUI__SetText(pItem.pText, s);
				InvalidateItem(Index);
			}
		}
		else
			AddString(s);
	}
	int GetItemDisabled(uint16_t Index) const {
		if (Index >= GetNumItems()) 
			return true;
		return ItemArray[Index].Status & LISTBOX_ITEM_DISABLED;
	}
	void SetItemDisabled(uint16_t Index, bool OnOff) {
		if (Index >= GetNumItems())
			return;
		auto &pItem = ItemArray[Index];
		if (OnOff) {
			if (!(pItem.Status & LISTBOX_ITEM_DISABLED)) {
				pItem.Status |= LISTBOX_ITEM_DISABLED;
				_InvalidateItem(Index);
			}
		}
		else if (pItem.Status & LISTBOX_ITEM_DISABLED) {
			pItem.Status &= ~LISTBOX_ITEM_DISABLED;
			_InvalidateItem(Index);
		}
	}

	void GetItemText(uint16_t Index, char *pBuffer, int MaxSize) {
		uint16_t NumItems;
		NumItems = GetNumItems();
		if (Index < NumItems) {
			const char *pString;
			int CopyLen;
			pString = ItemArray[Index].pText;
			CopyLen = GUI__strlen(pString);
			if (CopyLen > (MaxSize - 1)) {
				CopyLen = MaxSize - 1;
			}
			GUI__memcpy(pBuffer, pString, CopyLen);
			pBuffer[CopyLen] = 0;
		}
	}
	void SetString(uint16_t Index, const char *s) {
		if (Index < GetNumItems()) {
			if (GUI__SetText(ItemArray[Index].pText, s)) {
				_InvalidateItemSize(Index);
				UpdateScrollers();
				_InvalidateItem(Index);
			}
		}
	}

	bool GetItemSel(uint16_t Index) {
		if (Index >= GetNumItems() || !(States & LISTBOX_CF_MULTISEL))
			return false;
		return ItemArray[Index].Status & LISTBOX_ITEM_SELECTED;
	}
	void SetItemSel(uint16_t Index, bool OnOff) {
		if (Index >= GetNumItems() || !(States & LISTBOX_CF_MULTISEL))
			return;
		auto &pItem = ItemArray[Index];
		if (OnOff) {
			if (!(pItem.Status & LISTBOX_ITEM_SELECTED)) {
				pItem.Status |= LISTBOX_ITEM_SELECTED;
				_InvalidateItem(Index);
			}
		}
		else if (pItem.Status & LISTBOX_ITEM_SELECTED) {
			pItem.Status &= ~LISTBOX_ITEM_SELECTED;
			_InvalidateItem(Index);
		}
	}
	void SetItemSpacing(uint16_t Value) {
		this->ItemSpacing = Value;
		InvalidateItem(LISTBOX_ALL_ITEMS);
	}
	auto GetItemSpacing() const { return ItemSpacing; }

	auto GetScrollStepH() const { return Props.ScrollStepH; }
	void SetScrollStepH(int Value) { Props.ScrollStepH = Value; }

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
