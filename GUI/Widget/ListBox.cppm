module;

#include "GUI_Protected.h"

export module TUX.Widget.ListBox;

import TUX.Widget;
import TUX.Widget.ScrollBar;

import TUX.Array;

#define LISTBOX_ITEM_SELECTED (1 << 0)
#define LISTBOX_ITEM_DISABLED (1 << 1)

export {

constexpr int LISTBOX_ALL_ITEMS  = -1;

constexpr auto LISTBOX_NOTIFICATION_LOST_FOCUS = WM_NOTIFICATION_WIDGET<0>;

constexpr uint16_t
	LISTBOX_CF_AUTOSCROLLBAR_H      = (1<<0),
	LISTBOX_CF_AUTOSCROLLBAR_V      = (1<<1),
	LISTBOX_CF_MULTISEL             = (1<<2);

enum LISTBOX_CI {
	 LISTBOX_CI_UNSEL = 0,
	 LISTBOX_CI_SEL,
	 LISTBOX_CI_SELFOCUS,
	 LISTBOX_CI_DISABLED
};

class ListBox : public Widget {
	friend class DropDown;

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
	WM_SCROLL_STATE ScrollStateV, ScrollStateH;
	WObj *pOwner = nullptr;
	int16_t Sel = 0; /* current selection */
	uint8_t Flags = 0;
	uint16_t ScrollbarWidth = 0;
	uint16_t ItemSpacing = 0;

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
	uint16_t _GetNumItems() {
		return ItemArray.NumItems();
	}
	const char *_GetpString(int Index) {
		return ItemArray[Index].pText;
	}
	int _GetYSize() {
		RECT Rect;
		WM_GetInsideRectExScrollbar(this, &Rect);
		return (Rect.y1 - Rect.y0 + 1);
	}
	int _GetItemSizeX(uint16_t Index) {
		auto &pItem = ItemArray[Index];
		int xSize = pItem.xSize;
		if (xSize == 0) {
			PCFONT pOldFont = GUI.SetFont(Props.pFont);
			xSize = _CallOwnerDraw(WIDGET_ITEM_GET_XSIZE, Index, {});
			GUI.SetFont(pOldFont);
		}
		pItem.xSize = xSize;
		return xSize;
	}
	int _GetItemSizeY(uint16_t Index) {
		auto &pItem = ItemArray[Index];
		int ySize = pItem.ySize;
		if (ySize == 0) {
			PCFONT pOldFont = GUI.SetFont(Props.pFont);
			ySize = _CallOwnerDraw(WIDGET_ITEM_GET_YSIZE, Index, {});
			GUI.SetFont(pOldFont);
		}
		pItem.ySize = ySize;
		return ySize;
	}
	int _GetContentsSizeX() {
		int i, NumItems, SizeX;
		int Result = 0;
		NumItems = _GetNumItems();
		for (i = 0; i < NumItems; i++) {
			SizeX = _GetItemSizeX(i);
			if (Result < SizeX) {
				Result = SizeX;
			}
		}
		return Result;
	}
	int _GetItemPosY(uint16_t Index) {
		if (Index < _GetNumItems()) {
			if ((int)Index >= ScrollStateV.v) {
				uint16_t i;
				int PosY = 0;
				for (i = ScrollStateV.v; i < Index; i++) {
					PosY += _GetItemSizeY(i);
				}
				return PosY;
			}
		}
		return -1;
	}
	int _IsPartiallyVis() {
		int Index;
		Index = this->Sel;
		if (Index < (int)_GetNumItems()) {
			if (Index >= ScrollStateV.v) {
				int y;
				y = _GetItemPosY(Index);
				y += _GetItemSizeY(Index);
				if (y > _GetYSize()) {
					return 1;
				}
			}
		}
		return 0;
	}
	uint16_t _GetNumVisItems() {
		int NumItems, r = 1;
		NumItems = _GetNumItems();
		if (NumItems > 1) {
			int i, ySize, DistY = 0;
			ySize = _GetYSize();
			for (i = NumItems - 1; i >= 0; i--) {
				DistY += _GetItemSizeY(i);
				if (DistY > ySize) {
					break;
				}
			}
			r = NumItems - i - 1;
			if (r < 1) {
				return 1;
			}
		}
		return r;
	}
	int _UpdateScrollPos() {
		int PrevScrollStateV;
		PrevScrollStateV = ScrollStateV.v;
		if (this->Sel >= 0) {
			/* Check upper limit */
			if (_IsPartiallyVis()) {
				ScrollStateV.v = this->Sel - (ScrollStateV.PageSize - 1);
			}
			/* Check lower limit */
			if (this->Sel < ScrollStateV.v) {
				ScrollStateV.v = this->Sel;
			}
		}
		ScrollStateV.Bounds();
		ScrollStateH.Bounds();
		SetScrollState(ScrollStateV, ScrollStateH);
		return ScrollStateV.v - PrevScrollStateV;
	}
	void _InvalidateItemSize(uint16_t Index) {
		auto &pItem = ItemArray[Index];
		pItem.xSize = 0;
		pItem.ySize = 0;
	}
	void _InvalidateInsideArea() {
		RECT Rect;
		WM_GetInsideRectExScrollbar(this, &Rect);
		Invalidate(&Rect);
	}
	void _InvalidateItem(int Sel) {
		if (Sel >= 0) {
			int ItemPosY;
			ItemPosY = _GetItemPosY(Sel);
			if (ItemPosY >= 0) {
				RECT Rect;
				int ItemDistY;
				ItemDistY = _GetItemSizeY(Sel);
				WM_GetInsideRectExScrollbar(this, &Rect);
				Rect.y0 += ItemPosY;
				Rect.y1 = Rect.y0 + ItemDistY - 1;
				Invalidate(&Rect);
			}
		}
	}
	void _InvalidateItemAndBelow(int Sel) {
		if (Sel >= 0) {
			int ItemPosY;
			ItemPosY = _GetItemPosY(Sel);
			if (ItemPosY >= 0) {
				RECT Rect;
				WM_GetInsideRectExScrollbar(this, &Rect);
				Rect.y0 += ItemPosY;
				Invalidate(&Rect);
			}
		}
	}
	void _SetScrollbarWidth() {
		int Width = this->ScrollbarWidth;
		//	if (Width == 0)
		//		Width = SCROLLBAR_GetDefaultWidth();	////////////// FIX //////////////
		if (auto pScroll = (ScrollBar *)GetScrollbarH())
			pScroll->SetWidth(Width);
		if (auto pScroll = (ScrollBar *)GetScrollbarV())
			pScroll->SetWidth(Width);
	}
	int _CalcScrollParas() {
		/* Calc vertical scroll parameters */
		ScrollStateV.NumItems = _GetNumItems();
		ScrollStateV.PageSize = _GetNumVisItems();
		/* Calc horizontal scroll parameters */
		RECT Rect;
		WM_GetInsideRectExScrollbar(this, &Rect);
		ScrollStateH.NumItems = _GetContentsSizeX();
		ScrollStateH.PageSize = Rect.x1 - Rect.x0 + 1;
		return _UpdateScrollPos();
	}
	void _ManageAutoScroll() {
		char IsRequired;
		if (this->Flags & LISTBOX_CF_AUTOSCROLLBAR_V) {
			IsRequired = (_GetNumVisItems() < _GetNumItems());
			WM_SetScrollbarV(this, IsRequired);
		}
		if (this->Flags & LISTBOX_CF_AUTOSCROLLBAR_H) {
			RECT Rect;
			int xSize, xSizeContents;
			xSizeContents = _GetContentsSizeX();
			WM_GetInsideRectExScrollbar(this, &Rect);
			xSize = Rect.x1 - Rect.x0 + 1;
			IsRequired = (xSizeContents > xSize);
			WM_SetScrollbarH(this, IsRequired);
		}
		if (this->ScrollbarWidth) {
			_SetScrollbarWidth();
		}
	}
	int _Tolower(int Key) {
		if ((Key >= 0x41) && (Key <= 0x5a)) {
			Key += 0x20;
		}
		return Key;
	}
	int _IsAlphaNum(int Key) {
		Key = _Tolower(Key);
		if (Key >= 'a' && Key <= 'z') {
			return 1;
		}
		if (Key >= '0' && Key <= '9') {
			return 1;
		}
		return 0;
	}
	void _SelectByKey(int Key) {
		 Key = _Tolower(Key);
		for (uint16_t i = 0; i < _GetNumItems(); i++) {
			auto s = _GetpString(i);
			if (_Tolower(*s) == Key) {
				SetSel(i);
				break;
			}
		}
	}
	void _FreeAttached() {
		for (unsigned _i = 0, _n = ItemArray.NumItems(); _i < _n; _i++) {
			GUI_ALLOC_FreePtr((void **)&ItemArray[_i].pText);
		}
		ItemArray.Delete();
	}
	void _OnPaint(const RECT *pClipRect) {
		RECT RectInside, RectItem, ClipRect;
		int ItemDistY;
		GUI.SetFont(Props.pFont);
		/* Calculate clipping rectangle */
		ClipRect = *pClipRect - this->Rect.LeftTop();
		WM_GetInsideRectExScrollbar(this, &RectInside);
		ClipRect &= RectInside;
		RectItem.x0 = ClipRect.x0;
		RectItem.x1 = ClipRect.x1;
		/* Fill item info structure */
		POINT ItemPos{
			RectInside.x0 - ScrollStateH.v,
			RectInside.y0
		};
		/* Do the drawing */
		for (int i = ScrollStateV.v, NumItems = _GetNumItems(); i < NumItems; i++) {
			RectItem.y0 = ItemPos.y;
			/* Break when all other rows are outside the drawing area */
			if (RectItem.y0 > ClipRect.y1) {
				break;
			}
			ItemDistY = _GetItemSizeY(i);
			RectItem.y1 = RectItem.y0 + ItemDistY - 1;
			/* Make sure that we draw only when row is in drawing area */
			if (RectItem.y1 >= ClipRect.y0) {
				/* Set user clip rect */
				WM_SetUserClipRect(&RectItem);
				/* Draw item */
				_CallOwnerDraw(WIDGET_ITEM_DRAW, i, POINT{ ItemPos.x, ItemPos.y });
			}
			ItemPos.y += ItemDistY;
		}
		WM_SetUserClipRect(nullptr);
		/* Calculate & clear 'data free' area */
		RectItem.y0 = ItemPos.y;
		RectItem.y1 = RectInside.y1;
		GUI.SetBkColor(Props.aBkColor[0]);
		GUI_ClearRect(RectItem);
		/* Draw the 3D effect (if configured) */
		DrawDown();
	}
	void _ToggleMultiSel(int Sel) {
		if (this->Flags & LISTBOX_CF_MULTISEL) {
			auto &item = ItemArray[Sel];
			if (!(item.Status & LISTBOX_ITEM_DISABLED)) {
				item.Status ^= LISTBOX_ITEM_SELECTED;
				_NotifyOwner(WM_NOTIFICATION_SEL_CHANGED);
				_InvalidateItem(Sel);
			}
		}
	}
	int _GetItemFromPos(int x, int y) {
		int Sel = -1;
		RECT Rect;
		WM_GetInsideRectExScrollbar(this, &Rect);
		if ((x >= Rect.x0) && (y >= Rect.y0)) {
			if ((x <= Rect.x1) && (y <= Rect.y1)) {
				int NumItems = _GetNumItems();
				int i, y0 = Rect.y0;
				for (i = ScrollStateV.v; i < NumItems; i++) {
					if (y >= y0) {
						Sel = i;
					}
					y0 += _GetItemSizeY(i);
				}
			}
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
				int Sel = _GetItemFromPos(pState->x, pState->y);
				if (Sel >= 0)
					if (Sel < (int)(ScrollStateV.v + _GetNumVisItems()))
						SetSel(Sel);
			}
		}
	}
#endif
	int _OnKey(const WM_KEY_INFO *pInfo) {
		if (pInfo->PressedCnt > 0) {
			int Key = pInfo->Key;
			if (AddKey(Key))
				return 1; /* Key has been consumed */
		}
		return 0; /* Key has not been consumed */
	}
	void _MoveSel(int Dir) {
		int NewSel = -1;
		auto Index = GetSel();
		auto NumItems = _GetNumItems();
		do {
			Index += Dir;
			if ((Index < 0) || (Index >= NumItems)) {
				break;
			}
			auto &pItem = ItemArray[Index];
			if (!(pItem.Status & LISTBOX_ITEM_DISABLED)) {
				NewSel = Index;
			}
		} while (NewSel < 0);
		if (NewSel >= 0) {
			SetSel(NewSel);
		}
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

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (ListBox *)hWin;
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
					case WM_NOTIFICATION_VALUE_CHANGED: {
						WM_SCROLL_STATE ScrollState;
						if (pWinSrc == pObj->GetScrollbarV()) {
							WM_GetScrollState(pWinSrc, &ScrollState);
							pObj->ScrollStateV.v = ScrollState.v;
							pObj->_InvalidateInsideArea();
							pObj->_NotifyOwner(WM_NOTIFICATION_SCROLL_CHANGED);
						}
						else if (pWinSrc == pObj->GetScrollbarH()) {
							WM_GetScrollState(pWinSrc, &ScrollState);
							pObj->ScrollStateH.v = ScrollState.v;
							pObj->_InvalidateInsideArea();
							pObj->_NotifyOwner(WM_NOTIFICATION_SCROLL_CHANGED);
						}
						break;
					}
					case WM_NOTIFICATION_SCROLLBAR_ADDED:
						pObj->UpdateScrollers();
						break;
				}
				return 0;
			}
			case WM_PAINT:
				pObj->_OnPaint((const RECT *)Data);
				return 0;
			case WM_PID_STATE_CHANGED: {
				auto pInfo = (const PID_CHANGED_INFO *)Data;
				if (pInfo->State) {
					int Sel;
					Sel = pObj->_GetItemFromPos(pInfo->x, pInfo->y);
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
					auto r = pObj->GetClientRect();
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
				if (pObj->_OnKey((const WM_KEY_INFO *)Data))
					return 0;
				break;
			case WM_SIZE:
				pObj->UpdateScrollers();
				pObj->Invalidate();
				return 0;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:
	ListBox(RECT r, WM_CF Style, WObj *pParent, uint16_t Id) : 
		Widget(r, Style, _Callback, pParent, Id, WIDGET_STATE_FOCUSSABLE) {
		UpdateScrollers();
	}
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new ListBox(
			RECT(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0,
				 pCreateInfo->x0 + x0 + pCreateInfo->xSize - 1,
				 pCreateInfo->y0 + y0 + pCreateInfo->ySize - 1),
			pCreateInfo->Flags, hWinParent, pCreateInfo->Id);
	}

public:

#pragma region Properties

	PCFONT GetFont() { return Props.pFont; }
	void SetFont(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		InvalidateItem(LISTBOX_ALL_ITEMS);
	}

	void SetTextColor(LISTBOX_CI Index, RGBC Color) {
		if (Index >= GUI_COUNTOF(Props.aBkColor))
			return;
		Props.aTextColor[Index] = Color;
		_InvalidateInsideArea();
	}

	void SetBkColor(LISTBOX_CI Index, RGBC color) {
		if (Index >= GUI_COUNTOF(Props.aBkColor))
			return;
		Props.aBkColor[Index] = color;
		_InvalidateInsideArea();
	}

#pragma endregion

	int UpdateScrollers() {
		this->_ManageAutoScroll();
		return this->_CalcScrollParas();
	}
	static int OwnerDraw(WObj *pWin, int Cmd, int ItemIndex, POINT Pos) {
		auto pObj = (ListBox *)pWin;
		switch (Cmd) {
			case WIDGET_ITEM_GET_XSIZE: {
				auto pOldFont = GUI.SetFont(pObj->Props.pFont);
				auto s = pObj->_GetpString(ItemIndex);
				auto DistX = GUI_GetStringDistX(s);
				GUI.SetFont(pOldFont);
				return DistX;
			}
			case WIDGET_ITEM_GET_YSIZE:
				return pObj->Props.pFont->DistY() + pObj->ItemSpacing;
			case WIDGET_ITEM_DRAW: {
				auto &pItem = pObj->ItemArray[ItemIndex];
				auto r = pObj->GetInsideRect();
				auto FontDistY = pObj->Props.pFont->DistY();
				/* Calculate color index */
				bool IsDisabled = pItem.Status & LISTBOX_ITEM_DISABLED;
				bool IsSelected = pItem.Status & LISTBOX_ITEM_SELECTED;
				int ColorIndex;
				if (pObj->Flags & LISTBOX_CF_MULTISEL) {
					ColorIndex = IsDisabled ? 3 : IsSelected ? 2 : 0;
				}
				else {
					ColorIndex = IsDisabled ? 3 : ItemIndex != pObj->Sel ? 0	:
						pObj->GetStates() & WIDGET_STATE_FOCUS || pObj->pOwner ? 2 : 1;
				}
				/* Display item */
				GUI.SetBkColor(pObj->Props.aBkColor[ColorIndex]);
				GUI.SetColor(pObj->Props.aTextColor[ColorIndex]);
				auto s = pObj->_GetpString(ItemIndex);
				GUI.SetTextMode(DRAWMODE_TRANS);
				GUI_Clear();
				GUI_DispStringAt(s, Pos.x + 1, Pos.y);
				/* Display focus rectangle */
				if ((pObj->Flags & LISTBOX_CF_MULTISEL) && (ItemIndex == pObj->Sel)) {
					RECT rFocus;
					rFocus.LeftTop(Pos);
					rFocus.x1 = r.x1;
					rFocus.y1 = Pos.y + FontDistY - 1;
					GUI.SetColor(RGB_WHITE - pObj->Props.aBkColor[ColorIndex]);
					GUI_DrawFocusRect(rFocus, 0);
				}
				return 0;
			}
		}
		return 0;
	}

	void InvalidateItem(int Index) {
		int NumItems;
		NumItems = _GetNumItems();
		if (Index < NumItems) {
			if (Index < 0) {
				int i;
				for (i = 0; i < NumItems; i++) {
					this->_InvalidateItemSize(i);
				}
				UpdateScrollers();
				this->_InvalidateInsideArea();
			}
			else {
				this->_InvalidateItemSize(Index);
				UpdateScrollers();
				this->_InvalidateItemAndBelow(Index);
			}
		}
	}

	int AddKey(int Key) {
		int r = 0;
		r = this->_AddKey(Key);
		return r;
	}
	void AddString(const char *s) {
		if (s) {
			Item item = { 0, 0 };
			if (this->ItemArray.AddItem(&item) == 0) {
				uint16_t ItemIndex = ItemArray.NumItems() - 1;
				GUI__SetText(&ItemArray[ItemIndex].pText, s);
				this->_InvalidateItemSize(ItemIndex);
				UpdateScrollers();
				this->_InvalidateItem(ItemIndex);
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

	auto GetSel() { return Sel; }
	void SetSel(int NewSel) {
		auto MaxSel = _GetNumItems();
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

	void DeleteItem(uint16_t Index) {
		int Sel;
		uint16_t NumItems;
		NumItems = _GetNumItems();
		if (Index < NumItems) {
			GUI_ALLOC_FreePtr((void **)&this->ItemArray[Index].pText);
			this->ItemArray.DeleteItem(Index);
			/*
			 * Update selection
			 */
			Sel = this->Sel;
			if (Sel >= 0) {                     /* Valid selction ? */
				if ((int)Index == Sel) {          /* Deleting selected item ? */
					this->Sel = -1;                 /* Invalidate selection */
				}
				else if ((int)Index < Sel) {    /* Deleting item above selection ? */
					this->Sel--;
				}
			}
			if (UpdateScrollers()) {
				this->_InvalidateInsideArea();
			}
			else {
				this->_InvalidateItemAndBelow(Index);
			}
		}
	}

	void GetItemText(uint16_t Index, char *pBuffer, int MaxSize) {
		uint16_t NumItems;
		NumItems = _GetNumItems();
		if (Index < NumItems) {
			const char *pString;
			int CopyLen;
			pString = this->_GetpString(Index);
			CopyLen = GUI__strlen(pString);
			if (CopyLen > (MaxSize - 1)) {
				CopyLen = MaxSize - 1;
			}
			GUI__memcpy(pBuffer, pString, CopyLen);
			pBuffer[CopyLen] = 0;
		}
	}
	uint16_t GetNumItems() {
		return _GetNumItems();
	}
	void InsertString(const char *s, uint16_t Index) {
		if (s) {
			uint16_t NumItems;

			NumItems = _GetNumItems();
			if (Index < NumItems) {
				if (this->ItemArray.InsertItem(Index)) {
					auto &pItem = this->ItemArray[Index];
					pItem.Status = 0;
					GUI__SetText(&pItem.pText, s);
					InvalidateItem(Index);
				}
			}
			else {
				AddString(s);
			}
		}
	}
	int GetItemDisabled(uint16_t Index) {
		int Ret = 0;
		uint16_t NumItems;
		NumItems = _GetNumItems();
		if (Index < NumItems) {
			if (ItemArray[Index].Status & LISTBOX_ITEM_DISABLED) {
				Ret = 1;
			}
		}

		return Ret;
	}
	void SetItemDisabled(uint16_t Index, int OnOff) {
		uint16_t NumItems;
		NumItems = _GetNumItems();
		if (Index < NumItems) {
			auto &pItem = this->ItemArray[Index];
			if (OnOff) {
				if (!(pItem.Status & LISTBOX_ITEM_DISABLED)) {
					pItem.Status |= LISTBOX_ITEM_DISABLED;
					this->_InvalidateItem(Index);
				}
			}
			else {
				if (pItem.Status & LISTBOX_ITEM_DISABLED) {
					pItem.Status &= ~LISTBOX_ITEM_DISABLED;
					this->_InvalidateItem(Index);
				}
			}
		}
	}
	void SetItemSpacing(uint16_t Value) {
		this->ItemSpacing = Value;
		InvalidateItem(LISTBOX_ALL_ITEMS);
	}
	uint16_t GetItemSpacing() { return ItemSpacing; }
	void SetMulti(int Mode) {
		if (Mode) {
			if (!(this->Flags & LISTBOX_CF_MULTISEL)) {
				this->Flags |= LISTBOX_CF_MULTISEL;
				this->_InvalidateInsideArea();
			}
		}
		else {
			if (this->Flags & LISTBOX_CF_MULTISEL) {
				this->Flags &= ~LISTBOX_CF_MULTISEL;
				this->_InvalidateInsideArea();
			}
		}
	}
	bool GetMulti() const {
		return Flags & LISTBOX_CF_MULTISEL;
	}
	int GetItemSel(uint16_t Index) {
		int Ret = 0;
		uint16_t NumItems;
		NumItems = _GetNumItems();
		if ((Index < NumItems) && (this->Flags & LISTBOX_CF_MULTISEL)) {
			auto &pItem = this->ItemArray[Index];
			if (pItem.Status & LISTBOX_ITEM_SELECTED) {
				Ret = 1;
			}
		}

		return Ret;
	}
	void SetItemSel(uint16_t Index, int OnOff) {
		uint16_t NumItems;
		NumItems = _GetNumItems();
		if ((Index < NumItems) && (this->Flags & LISTBOX_CF_MULTISEL)) {
			auto &pItem = this->ItemArray[Index];
			if (OnOff) {
				if (!(pItem.Status & LISTBOX_ITEM_SELECTED)) {
					pItem.Status |= LISTBOX_ITEM_SELECTED;
					this->_InvalidateItem(Index);
				}
			}
			else {
				if (pItem.Status & LISTBOX_ITEM_SELECTED) {
					pItem.Status &= ~LISTBOX_ITEM_SELECTED;
					this->_InvalidateItem(Index);
				}
			}
		}
	}
	void SetScrollStepH(int Value) {
		Props.ScrollStepH = Value;
	}
	int GetScrollStepH() { return Props.ScrollStepH; }
	void SetAutoScrollH(int State) {
		char Flags;

		Flags = this->Flags & (~LISTBOX_CF_AUTOSCROLLBAR_H);
		if (State) {
			Flags |= LISTBOX_CF_AUTOSCROLLBAR_H;
		}
		if (this->Flags != Flags) {
			this->Flags = Flags;
			UpdateScrollers();
		}
	}
	void SetAutoScrollV(int State) {
		char Flags;

		Flags = this->Flags & (~LISTBOX_CF_AUTOSCROLLBAR_V);
		if (State) {
			Flags |= LISTBOX_CF_AUTOSCROLLBAR_V;
		}
		if (this->Flags != Flags) {
			this->Flags = Flags;
			UpdateScrollers();
		}
	}
	void SetOwner(WObj *pOwner) {
		this->pOwner = pOwner;
		this->_InvalidateInsideArea();
	}
	void SetOwnerDraw(WIDGET_DRAW_ITEM_FUNC *pfDrawItem) {
		this->pfDrawItem = pfDrawItem;
		InvalidateItem(LISTBOX_ALL_ITEMS);
	}
	void SetScrollbarWidth(uint16_t Width) {
		if (Width != (uint16_t)this->ScrollbarWidth) {
			this->ScrollbarWidth = Width;
			this->_SetScrollbarWidth();
			Invalidate();
		}
	}
	void SetString(const char *s, uint16_t Index) {
		if (Index < (uint16_t)_GetNumItems()) {
			if (GUI__SetText(&ItemArray[Index].pText, s)) {
				this->_InvalidateItemSize(Index);
				UpdateScrollers();
				this->_InvalidateItem(Index);
			}
		}
	}

};

ListBox::Properties ListBox::DefaultProps;

}
