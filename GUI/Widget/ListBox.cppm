module;

#include "DIALOG_Intern.h"

export module TUX.Widget.ListBox;

import TUX.Widget;
import TUX.Widget.ScrollBar;

import TUX.Array;

#define LISTBOX_ITEM_SELECTED (1 << 0)
#define LISTBOX_ITEM_DISABLED (1 << 1)

export {
constexpr int LISTBOX_ALL_ITEMS  = -1;

enum LISTBOX_CI {
	 LISTBOX_CI_UNSEL = 0,
	 LISTBOX_CI_SEL,
	 LISTBOX_CI_SELFOCUS,
	 LISTBOX_CI_DISABLED
};

typedef WM_Obj * LISTBOX_Handle;

constexpr uint16_t LISTBOX_NOTIFICATION_LOST_FOCUS = (WM_NOTIFICATION_WIDGET + 0);
constexpr uint16_t LISTBOX_CF_AUTOSCROLLBAR_H      = (1<<0);
constexpr uint16_t LISTBOX_CF_AUTOSCROLLBAR_V      = (1<<1);
constexpr uint16_t LISTBOX_CF_MULTISEL             = (1<<2);
constexpr uint16_t LISTBOX_SF_AUTOSCROLLBAR_H      = LISTBOX_CF_AUTOSCROLLBAR_H;
constexpr uint16_t LISTBOX_SF_AUTOSCROLLBAR_V      = LISTBOX_CF_AUTOSCROLLBAR_V;
constexpr uint16_t LISTBOX_SF_MULTISEL             = LISTBOX_CF_MULTISEL;

struct LISTBOX_ITEM {
	uint16_t xSize, ySize;
	uint8_t Status;
	char acText[1];
};

struct LISTBOX_Obj : public WIDGET {
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
	Properties Props;
	GUI_ARRAY ItemArray;
	WIDGET_DRAW_ITEM_FUNC *pfDrawItem;
	WM_SCROLL_STATE ScrollStateV, ScrollStateH;
	WM_Obj *hOwner;
	int16_t Sel; /* current selection */
	uint8_t Flags;
	uint16_t  ScrollbarWidth;
	uint16_t ItemSpacing;

	void _NotifyOwner(int Notification) {
		auto hOwner = this->hOwner ? this->hOwner : WM_GetParent(this);
		WM_NOTIFY_INFO Info;
		Info.Notification = Notification;
		Info.pWinSrc = this;
		WM_SendMessage(hOwner, WM_NOTIFY_PARENT, (WM_PARAM)&Info);
	}

	int _CallOwnerDraw(int Cmd, int ItemIndex) {
		WIDGET_ITEM_DRAW_INFO ItemInfo;
		int r;
		ItemInfo.Cmd = Cmd;
		ItemInfo.hWin = this;
		ItemInfo.ItemIndex = ItemIndex;
		if (this->pfDrawItem) {
			r = this->pfDrawItem(&ItemInfo);
		}
		else {
			r = OwnerDraw(&ItemInfo);
		}
		return r;
	}
	uint16_t _GetNumItems() {
		return GUI_ARRAY_GetNumItems(&ItemArray);
	}
	const char *_GetpString(int Index) {
		const char *s = nullptr;
		auto pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItem(&ItemArray, Index);
		if (pItem) {
			s = pItem->acText;
		}
		return s;
	}
	int _GetYSize() {
		GUI_RECT Rect;
		WM_GetInsideRectExScrollbar(this, &Rect);
		return (Rect.y1 - Rect.y0 + 1);
	}
	int _GetItemSizeX(uint16_t Index) {
		LISTBOX_ITEM *pItem;
		int xSize = 0;
		pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItem(&ItemArray, Index);
		if (pItem) {
			xSize = pItem->xSize;
		}
		if (xSize == 0) {
			PCFONT pOldFont;
			pOldFont = GUI_SetFont(Props.pFont);
			xSize = _CallOwnerDraw(WIDGET_ITEM_GET_XSIZE, Index);
			GUI_SetFont(pOldFont);
		}
		if (pItem) {
			pItem->xSize = xSize;
		}
		return xSize;
	}
	int _GetItemSizeY(uint16_t Index) {
		LISTBOX_ITEM *pItem;
		int ySize = 0;
		pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItem(&ItemArray, Index);
		if (pItem) {
			ySize = pItem->ySize;
		}
		if (ySize == 0) {
			PCFONT pOldFont;
			pOldFont = GUI_SetFont(Props.pFont);
			ySize = _CallOwnerDraw(WIDGET_ITEM_GET_YSIZE, Index);
			GUI_SetFont(pOldFont);
		}
		if (pItem) {
			pItem->ySize = ySize;
		}
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
		WM_CheckScrollBounds(&ScrollStateV);
		WM_CheckScrollBounds(&ScrollStateH);
		WIDGET__SetScrollState(this, &ScrollStateV, &ScrollStateH);
		return ScrollStateV.v - PrevScrollStateV;
	}
	void _InvalidateItemSize(uint16_t Index) {
		LISTBOX_ITEM *pItem;
		pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItem(&ItemArray, Index);
		if (pItem) {
			pItem->xSize = 0;
			pItem->ySize = 0;
		}
	}
	void _InvalidateInsideArea() {
		GUI_RECT Rect;
		WM_GetInsideRectExScrollbar(this, &Rect);
		WM_InvalidateRect(this, &Rect);
	}
	void _InvalidateItem(int Sel) {
		if (Sel >= 0) {
			int ItemPosY;
			ItemPosY = _GetItemPosY(Sel);
			if (ItemPosY >= 0) {
				GUI_RECT Rect;
				int ItemDistY;
				ItemDistY = _GetItemSizeY(Sel);
				WM_GetInsideRectExScrollbar(this, &Rect);
				Rect.y0 += ItemPosY;
				Rect.y1 = Rect.y0 + ItemDistY - 1;
				WM_InvalidateRect(this, &Rect);
			}
		}
	}
	void _InvalidateItemAndBelow(int Sel) {
		if (Sel >= 0) {
			int ItemPosY;
			ItemPosY = _GetItemPosY(Sel);
			if (ItemPosY >= 0) {
				GUI_RECT Rect;
				WM_GetInsideRectExScrollbar(this, &Rect);
				Rect.y0 += ItemPosY;
				WM_InvalidateRect(this, &Rect);
			}
		}
	}
	void _SetScrollbarWidth() {
		int Width = this->ScrollbarWidth;
		//	if (Width == 0)
		//		Width = SCROLLBAR_GetDefaultWidth();	////////////// FIX //////////////
		if (auto pScroll = (SCROLLBAR_Obj *)WM_GetDialogItem(this, GUI_ID_HSCROLL))
			pScroll->SetWidth(Width);
		if (auto pScroll = (SCROLLBAR_Obj *)WM_GetDialogItem(this, GUI_ID_VSCROLL))
			pScroll->SetWidth(Width);
	}
	int _CalcScrollParas() {
		/* Calc vertical scroll parameters */
		ScrollStateV.NumItems = _GetNumItems();
		ScrollStateV.PageSize = _GetNumVisItems();
		/* Calc horizontal scroll parameters */
		GUI_RECT Rect;
		WM_GetInsideRectExScrollbar(this, &Rect);
		ScrollStateH.NumItems = _GetContentsSizeX();
		ScrollStateH.PageSize = Rect.x1 - Rect.x0 + 1;
		return _UpdateScrollPos();
	}
	void _ManageAutoScroll() {
		char IsRequired;
		if (this->Flags & LISTBOX_SF_AUTOSCROLLBAR_V) {
			IsRequired = (_GetNumVisItems() < _GetNumItems());
			WM_SetScrollbarV(this, IsRequired);
		}
		if (this->Flags & LISTBOX_SF_AUTOSCROLLBAR_H) {
			GUI_RECT Rect;
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
		GUI_ARRAY_Delete(&ItemArray);
	}
	void _OnPaint(const GUI_RECT *pClipRect) {
		WIDGET_ITEM_DRAW_INFO ItemInfo;
		GUI_RECT RectInside, RectItem, ClipRect;
		int ItemDistY, NumItems, i;
		NumItems = _GetNumItems();
		GUI_SetFont(Props.pFont);
		/* Calculate clipping rectangle */
		ClipRect = *pClipRect - this->Rect.LeftTop();
		WM_GetInsideRectExScrollbar(this, &RectInside);
		ClipRect &= RectInside;
		RectItem.x0 = ClipRect.x0;
		RectItem.x1 = ClipRect.x1;
		/* Fill item info structure */
		ItemInfo.Cmd = WIDGET_ITEM_DRAW;
		ItemInfo.hWin = this;
		ItemInfo.x0 = RectInside.x0 - ScrollStateH.v;
		ItemInfo.y0 = RectInside.y0;
		/* Do the drawing */
		for (i = ScrollStateV.v; i < NumItems; i++) {
			RectItem.y0 = ItemInfo.y0;
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
				/* Fill item info structure */
				ItemInfo.ItemIndex = i;
				/* Draw item */
				if (this->pfDrawItem) {
					this->pfDrawItem(&ItemInfo);
				}
				else {
					OwnerDraw(&ItemInfo);
				}
			}
			ItemInfo.y0 += ItemDistY;
		}
		WM_SetUserClipRect(nullptr);
		/* Calculate & clear 'data free' area */
		RectItem.y0 = ItemInfo.y0;
		RectItem.y1 = RectInside.y1;
		GUI_SetBkColor(Props.aBkColor[0]);
		GUI_ClearRect(RectItem);
		/* Draw the 3D effect (if configured) */
		WIDGET__EFFECT_DrawDown(this);
	}
	void _ToggleMultiSel(int Sel) {
		if (this->Flags & LISTBOX_SF_MULTISEL) {
			WM_HMEM hItem = GUI_ARRAY_GethItem(&ItemArray, Sel);
			if (hItem) {
				auto pItem = (LISTBOX_ITEM *)(hItem);
				if (!(pItem->Status & LISTBOX_ITEM_DISABLED)) {
					pItem->Status ^= LISTBOX_ITEM_SELECTED;
					_NotifyOwner(WM_NOTIFICATION_SEL_CHANGED);
					_InvalidateItem(Sel);
				}
			}
		}
	}
	int _GetItemFromPos(int x, int y) {
		int Sel = -1;
		GUI_RECT Rect;
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
	void _OnTouch(const GUI_PID_STATE *pState) {
		if (pState) { /* Something happened in our area (pressed or released) */
			if (pState->Pressed == 0)
				_NotifyOwner(WM_NOTIFICATION_RELEASED);
		}
		else /* Mouse moved out */
			_NotifyOwner(WM_NOTIFICATION_MOVED_OUT);
	}
#if GUI_SUPPORT_MOUSE
	void _OnMouseOver(const GUI_PID_STATE *pState) {
		if (this->hOwner) {
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
		int Index, NewSel = -1, NumItems;
		Index = GetSel();
		NumItems = _GetNumItems();
		do {
			WM_HMEM hItem;
			Index += Dir;
			if ((Index < 0) || (Index >= NumItems)) {
				break;
			}
			hItem = GUI_ARRAY_GethItem(&ItemArray, Index);
			if (hItem) {
				auto pItem = (LISTBOX_ITEM *)(hItem);
				if (!(pItem->Status & LISTBOX_ITEM_DISABLED)) {
					NewSel = Index;
				}
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
				if (WM_SetScrollValue(&ScrollStateH, ScrollStateH.v + Props.ScrollStepH)) {
					UpdateScrollers();
					_InvalidateInsideArea();
				}
				return 1;               /* Key has been consumed */
			case GUI_KEY_LEFT:
				if (WM_SetScrollValue(&ScrollStateH, ScrollStateH.v - Props.ScrollStepH)) {
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

	static WM_PARAM _Callback(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (LISTBOX_Obj *)hWin;
		/* In popup mode (hOwner set), bypass WIDGET_HandleActive for WM_PID_STATE_CHANGED.
		 * WIDGET_HandleActive internally calls WM_SetFocus on press, which would steal
		 * focus from the dropdown and cause its parent window to flicker. */
		if (!(pObj->hOwner && MsgId == WM_PID_STATE_CHANGED)) {
			/* Let widget handle the standard messages */
			if (!WIDGET_HandleActive(pObj, MsgId, &Data))
				return Data;
		}
		switch (MsgId) {
			case WM_NOTIFY_PARENT: {
				auto pInfo = (const WM_NOTIFY_INFO *)Data;
				auto pWinSrc = pInfo->pWinSrc;
				switch (pInfo->Notification) {
					case WM_NOTIFICATION_VALUE_CHANGED: {
						WM_SCROLL_STATE ScrollState;
						if (pWinSrc == WM_GetScrollbarV(pObj)) {
							WM_GetScrollState(pWinSrc, &ScrollState);
							pObj->ScrollStateV.v = ScrollState.v;
							pObj->_InvalidateInsideArea();
							pObj->_NotifyOwner(WM_NOTIFICATION_SCROLL_CHANGED);
						}
						else if (pWinSrc == WM_GetScrollbarH(pObj)) {
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
				pObj->_OnPaint((const GUI_RECT *)Data);
				return 0;
			case WM_PID_STATE_CHANGED: {
				auto pInfo = (const WM_PID_STATE_CHANGED_INFO *)Data;
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
				auto pState = (const GUI_PID_STATE *)Data;
				if (pObj->hOwner && pState) {
					GUI_RECT r = WM_GetClientRect(pObj);
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
				pObj->_OnMouseOver((const GUI_PID_STATE *)Data);
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
				WM_Invalidate(pObj);
				return 0;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:

	int UpdateScrollers() {
		this->_ManageAutoScroll();
		return this->_CalcScrollParas();
	}
	static int OwnerDraw(const WIDGET_ITEM_DRAW_INFO *pDrawItemInfo) {
		auto pObj = (LISTBOX_Obj *)(pDrawItemInfo->hWin);
		switch (pDrawItemInfo->Cmd) {
			case WIDGET_ITEM_GET_XSIZE: {
				PCFONT pOldFont;
				const char *s;
				int DistX;
				pOldFont = GUI_SetFont(pObj->Props.pFont);
				s = pObj->_GetpString(pDrawItemInfo->ItemIndex);
				DistX = GUI_GetStringDistX(s);
				GUI_SetFont(pOldFont);
				return DistX;
			}
			case WIDGET_ITEM_GET_YSIZE: {
				pObj = (LISTBOX_Obj *)(pDrawItemInfo->hWin);
				return GUI_GetYDistOfFont(pObj->Props.pFont) + pObj->ItemSpacing;
			}
			case WIDGET_ITEM_DRAW: {
				LISTBOX_ITEM *pItem;
				WM_HMEM hItem;
				int FontDistY;
				int ItemIndex = pDrawItemInfo->ItemIndex;
				const char *s;
				int ColorIndex;
				char IsDisabled;
				char IsSelected;
				pObj = (LISTBOX_Obj *)(pDrawItemInfo->hWin);
				hItem = GUI_ARRAY_GethItem(&pObj->ItemArray, ItemIndex);
				pItem = (LISTBOX_ITEM *)(hItem);
				auto r = WM_GetInsideRect();
				FontDistY = GUI_GetFontDistY();
				/* Calculate color index */
				IsDisabled = (pItem->Status & LISTBOX_ITEM_DISABLED) ? 1 : 0;
				IsSelected = (pItem->Status & LISTBOX_ITEM_SELECTED) ? 1 : 0;
				if (pObj->Flags & LISTBOX_SF_MULTISEL) {
					if (IsDisabled) {
						ColorIndex = 3;
					}
					else {
						ColorIndex = (IsSelected) ? 2 : 0;
					}
				}
				else {
					if (IsDisabled) {
						ColorIndex = 3;
					}
					else {
						if (ItemIndex == pObj->Sel) {
							ColorIndex = (pObj->State & WIDGET_STATE_FOCUS || pObj->hOwner) ? 2 : 1;
						}
						else {
							ColorIndex = 0;
						}
					}
				}
				/* Display item */
				GUI_SetBkColor(pObj->Props.aBkColor[ColorIndex]);
				GUI_SetColor(pObj->Props.aTextColor[ColorIndex]);
				s = pObj->_GetpString(ItemIndex);
				GUI_SetTextMode(DRAWMODE_TRANS);
				GUI_Clear();
				GUI_DispStringAt(s, pDrawItemInfo->x0 + 1, pDrawItemInfo->y0);
				/* Display focus rectangle */
				if ((pObj->Flags & LISTBOX_SF_MULTISEL) && (ItemIndex == pObj->Sel)) {
					GUI_RECT rFocus;
					rFocus.x0 = pDrawItemInfo->x0;
					rFocus.y0 = pDrawItemInfo->y0;
					rFocus.x1 = r.x1;
					rFocus.y1 = pDrawItemInfo->y0 + FontDistY - 1;
					GUI_SetColor(RGB_WHITE - pObj->Props.aBkColor[ColorIndex]);
					GUI_DrawFocusRect(rFocus, 0);
				}
				return 0;
			}
		}
		return 0;
	}

	void InvalidateItem(int Index) {
		int NumItems;
		NumItems = this->_GetNumItems();
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
			LISTBOX_ITEM Item = { 0, 0 };

			if (GUI_ARRAY_AddItem(&this->ItemArray, &Item, sizeof(LISTBOX_ITEM) + GUI__strlen(s)) == 0) {
				uint16_t ItemIndex = GUI_ARRAY_GetNumItems(&this->ItemArray) - 1;
				auto pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, ItemIndex);
				GUI__strcpy(pItem->acText, s);
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
	void SetSel(int NewSel) {
		int MaxSel;
		MaxSel = this->_GetNumItems();
		MaxSel = MaxSel ? MaxSel - 1 : 0;
		if (NewSel > MaxSel) {
			NewSel = MaxSel;
		}
		if (NewSel < 0) {
			NewSel = -1;
		}
		else {
			WM_HMEM hItem = GUI_ARRAY_GethItem(&this->ItemArray, NewSel);
			if (hItem) {
				auto pItem = (LISTBOX_ITEM *)(hItem);
				if (pItem->Status & LISTBOX_ITEM_DISABLED) {
					NewSel = -1;
				}
			}
		}
		if (NewSel != this->Sel) {
			int OldSel;
			OldSel = this->Sel;
			this->Sel = NewSel;
			if (this->_UpdateScrollPos()) {
				this->_InvalidateInsideArea();
			}
			else {
				this->_InvalidateItem(OldSel);
				this->_InvalidateItem(NewSel);
			}
			this->_NotifyOwner(WM_NOTIFICATION_SEL_CHANGED);
		}
	}
	int  GetSel() {
		int r = -1;
		r = this->Sel;
		return r;
	}
	void IncSel() {
		this->_MoveSel(1);
	}
	void DecSel() {
		this->_MoveSel(-1);
	}

	void DeleteItem(uint16_t Index) {
		int Sel;
		uint16_t NumItems;
		NumItems = this->_GetNumItems();
		if (Index < NumItems) {
			GUI_ARRAY_DeleteItem(&this->ItemArray, Index);
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
	void SetFont(PCFONT pFont) {
		this->Props.pFont = pFont;
		InvalidateItem(LISTBOX_ALL_ITEMS);
	}
	PCFONT GetFont() {
		PCFONT pFont = nullptr;
		pFont = this->Props.pFont;

		return pFont;
	}
	void GetItemText(uint16_t Index, char *pBuffer, int MaxSize) {
		uint16_t NumItems;
		NumItems = this->_GetNumItems();
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
		return this->_GetNumItems();
	}
	void InsertString(const char *s, uint16_t Index) {
		if (s) {
			uint16_t NumItems;

			NumItems = this->_GetNumItems();
			if (Index < NumItems) {
				WM_HMEM hItem;
				hItem = GUI_ARRAY_InsertItem(&this->ItemArray, Index, sizeof(LISTBOX_ITEM) + GUI__strlen(s));
				if (hItem) {
					auto pItem = (LISTBOX_ITEM *)(hItem);
					pItem->Status = 0;
					GUI__strcpy(pItem->acText, s);
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
		NumItems = this->_GetNumItems();
		if (Index < NumItems) {
			WM_HMEM hItem = GUI_ARRAY_GethItem(&this->ItemArray, Index);
			if (hItem) {
				auto pItem = (LISTBOX_ITEM *)(hItem);
				if (pItem->Status & LISTBOX_ITEM_DISABLED) {
					Ret = 1;
				}
			}
		}

		return Ret;
	}
	void SetItemDisabled(uint16_t Index, int OnOff) {
		uint16_t NumItems;
		NumItems = this->_GetNumItems();
		if (Index < NumItems) {
			WM_HMEM hItem = GUI_ARRAY_GethItem(&this->ItemArray, Index);
			if (hItem) {
				auto pItem = (LISTBOX_ITEM *)(hItem);
				if (OnOff) {
					if (!(pItem->Status & LISTBOX_ITEM_DISABLED)) {
						pItem->Status |= LISTBOX_ITEM_DISABLED;
						this->_InvalidateItem(Index);
					}
				}
				else {
					if (pItem->Status & LISTBOX_ITEM_DISABLED) {
						pItem->Status &= ~LISTBOX_ITEM_DISABLED;
						this->_InvalidateItem(Index);
					}
				}
			}
		}
	}
	void SetItemSpacing(uint16_t Value) {
		this->ItemSpacing = Value;
		InvalidateItem(LISTBOX_ALL_ITEMS);
	}
	uint16_t GetItemSpacing() {
		uint16_t Value = 0;
		Value = this->ItemSpacing;

		return Value;
	}
	void SetMulti(int Mode) {
		if (Mode) {
			if (!(this->Flags & LISTBOX_SF_MULTISEL)) {
				this->Flags |= LISTBOX_SF_MULTISEL;
				this->_InvalidateInsideArea();
			}
		}
		else {
			if (this->Flags & LISTBOX_SF_MULTISEL) {
				this->Flags &= ~LISTBOX_SF_MULTISEL;
				this->_InvalidateInsideArea();
			}
		}
	}
	int GetMulti() {
		int Multi = 0;
		if (!(this->Flags & LISTBOX_SF_MULTISEL)) {
			Multi = 0;
		}
		else {
			Multi = 1;
		}

		return Multi;
	}
	int GetItemSel(uint16_t Index) {
		int Ret = 0;
		uint16_t NumItems;
		NumItems = this->_GetNumItems();
		if ((Index < NumItems) && (this->Flags & LISTBOX_SF_MULTISEL)) {
			WM_HMEM hItem = GUI_ARRAY_GethItem(&this->ItemArray, Index);
			if (hItem) {
				auto pItem = (LISTBOX_ITEM *)(hItem);
				if (pItem->Status & LISTBOX_ITEM_SELECTED) {
					Ret = 1;
				}
			}
		}

		return Ret;
	}
	void SetItemSel(uint16_t Index, int OnOff) {
		uint16_t NumItems;
		NumItems = this->_GetNumItems();
		if ((Index < NumItems) && (this->Flags & LISTBOX_SF_MULTISEL)) {
			WM_HMEM hItem = GUI_ARRAY_GethItem(&this->ItemArray, Index);
			if (hItem) {
				auto pItem = (LISTBOX_ITEM *)(hItem);
				if (OnOff) {
					if (!(pItem->Status & LISTBOX_ITEM_SELECTED)) {
						pItem->Status |= LISTBOX_ITEM_SELECTED;
						this->_InvalidateItem(Index);
					}
				}
				else {
					if (pItem->Status & LISTBOX_ITEM_SELECTED) {
						pItem->Status &= ~LISTBOX_ITEM_SELECTED;
						this->_InvalidateItem(Index);
					}
				}
			}
		}
	}
	void SetScrollStepH(int Value) {
		this->Props.ScrollStepH = Value;
	}
	int GetScrollStepH() {
		int Value = 0;
		Value = this->Props.ScrollStepH;

		return Value;
	}
	void SetAutoScrollH(int State) {
		char Flags;

		Flags = this->Flags & (~LISTBOX_SF_AUTOSCROLLBAR_H);
		if (State) {
			Flags |= LISTBOX_SF_AUTOSCROLLBAR_H;
		}
		if (this->Flags != Flags) {
			this->Flags = Flags;
			UpdateScrollers();
		}
	}
	void SetAutoScrollV(int State) {
		char Flags;

		Flags = this->Flags & (~LISTBOX_SF_AUTOSCROLLBAR_V);
		if (State) {
			Flags |= LISTBOX_SF_AUTOSCROLLBAR_V;
		}
		if (this->Flags != Flags) {
			this->Flags = Flags;
			UpdateScrollers();
		}
	}
	void SetBkColor(uint16_t Index, RGBC color) {
		if ((uint16_t)Index < GUI_COUNTOF(this->Props.aBkColor)) {
			this->Props.aBkColor[Index] = color;
			this->_InvalidateInsideArea();
		}
	}
	void SetOwner(WM_Obj *hOwner) {
		this->hOwner = hOwner;
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
			WM_Invalidate(this);
		}
	}
	void SetString(const char *s, uint16_t Index) {
		if (Index < (uint16_t)this->_GetNumItems()) {
			auto pItem = (LISTBOX_ITEM *)GUI_ARRAY_ResizeItem(&this->ItemArray, Index, sizeof(LISTBOX_ITEM) + GUI__strlen(s));
			if (pItem) {
				GUI__strcpy(pItem->acText, s);
				this->_InvalidateItemSize(Index);
				UpdateScrollers();
				this->_InvalidateItem(Index);
			}
		}
	}
	RGBC SetTextColor(uint16_t Index, RGBC Color) {
		RGBC r = RGB_INVALID_COLOR;
		if (Index < GUI_COUNTOF(this->Props.aBkColor)) {
			this->Props.aTextColor[Index] = Color;
			r = this->Props.aTextColor[Index];
			this->_InvalidateInsideArea();
		}

		return r;
	}

};

LISTBOX_Obj::Properties LISTBOX_Obj::DefaultProps;

LISTBOX_Obj *LISTBOX_CreateEx(int x0, int y0, int xsize, int ysize, WM_Obj *hParent,
								int WinFlags, int ExFlags, int Id, const char **ppText) {
	auto pObj = (LISTBOX_Obj *)WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, LISTBOX_Obj::_Callback,
								  sizeof(LISTBOX_Obj) - sizeof(WM_Obj));
	if (pObj) {
		/* Init sub-classes */
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE);
		pObj->Props = LISTBOX_Obj::DefaultProps;
		if (ppText) {
			/* init member variables */
			/* Set non-zero attributes */
			pObj->SetText(ppText);
		}
		pObj->UpdateScrollers();
	}
	return pObj;
}
LISTBOX_Obj *LISTBOX_Create(const char **ppText, int x0, int y0, int xsize, int ysize, int Flags) {
	return LISTBOX_CreateEx(x0, y0, xsize, ysize, nullptr, Flags, 0, 0, ppText);
}
LISTBOX_Obj *LISTBOX_CreateAsChild(const char **ppText, WM_Obj *hWinParent,
									 int x0, int y0, int xsize, int ysize, int Flags) {
	return LISTBOX_CreateEx(x0, y0, xsize, ysize, hWinParent, Flags, 0, 0, ppText);
}
WM_Obj *LISTBOX_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	LISTBOX_Handle hObj;
	GUI_USE_PARA(cb);
	hObj = LISTBOX_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id, 0);
	return hObj;
}

}
