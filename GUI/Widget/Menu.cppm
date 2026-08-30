module;

#include <string.h>

#include "WM_Intern.h"
#include "GUI_Protected.h"

export module TUX.Widget.Menu;

import TUX.Widget;

import TUX.Array;

/* Define default effect */
#define MENU_EFFECT_DEFAULT WIDGET_Effect_3D1L

export {

constexpr uint16_t
	MENU_CF_HORIZONTAL               = 0,
	MENU_CF_VERTICAL                 = WIDGET_STATE_USER<0>,
	MENU_CF_OPEN_ON_POINTEROVER      = WIDGET_STATE_USER<1>,
	MENU_CF_CLOSE_ON_SECOND_CLICK    = WIDGET_STATE_USER<2>,
	MENU_CF_HIDE_DISABLED_SEL        = WIDGET_STATE_USER<3>,  /* Hides the selection when a disabled item is selected */
	MENU_SF_ACTIVE                   = WIDGET_STATE_USER<4>,
	MENU_SF_POPUP                    = WIDGET_STATE_USER<5>;

constexpr uint16_t
	MENU_IF_DISABLED           = 1 << 0,
	MENU_IF_SEPARATOR          = 1 << 1;

constexpr uint16_t
	MENU_ON_ITEMSELECT        = 0,   /* Send to owner when selecting a menu item */
	MENU_ON_INITMENU          = 1,   /* Send to owner when for the first time selecting a submenu */
	MENU_ON_INITSUBMENU       = 2,   /* Send to owner when selecting a submenu */
	MENU_ON_OPEN              = 3,   /* Internal message of menu widget (send to submenus) */
	MENU_ON_CLOSE             = 4,   /* Internal message of menu widget (send to submenus) */
	MENU_IS_MENU              = 5;   /* Internal message of menu widget. Owner must call   */

enum MENU_BI { 
	 MENU_BI_LEFT = 0,
	 MENU_BI_RIGHT,
	 MENU_BI_TOP,
	 MENU_BI_BOTTOM
};
enum MENU_CI {
	 MENU_CI_ENABLED = 0,
	 MENU_CI_SELECTED,
	 MENU_CI_DISABLED,
	 MENU_CI_DISABLED_SEL,
	 MENU_CI_ACTIVE_SUBMENU
};


PCWIDGET_EFFECT MENU__pDefaultEffect = MENU_EFFECT_DEFAULT;

class Menu : public Widget {
	
public:
	struct ItemData {
		const char *pText;
		uint16_t    Id;
		uint16_t    Flags;
		Menu *pSubmenu;
	};

public:
	struct Properties {
		PCFONT pFont{ GUI_DEFAULT_FONT };
		RGBC aTextColor[5]{
			RGB_BLACK,          /* enabled, not selected */
			RGB_WHITE,          /* enabled, selected */
			RGB_GRAYL(0x7C),    /* disabled, not selected */
			RGB_LIGHTGRAY,      /* disabled, selected */
			RGB_WHITE           /* active submenu */
		};
		RGBC aBkColor[5]{
			RGB_LIGHTGRAY,
			RGB_BLUEL(0x98),
			RGB_LIGHTGRAY,
			RGB_BLUEL(0x98),
			RGB_GRAYL(0x7C)
		};
		uint8_t aBorder[4]{ 4, 4, 2, 2 }; /* Left, Right, Top, Bottom */
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;

	struct Item {
		Menu *pSubmenu;
		uint16_t Id;
		uint16_t Flags;
		uint16_t TextWidth;
		char *pText;
	};
	ARRAY<Item> ItemArray;
	WObj *pOwner = nullptr;
	char IsSubmenuActive = 0;
	uint16_t Width = 0, Height = 0;
	uint16_t Sel = -1;

	struct MsgPack {
		uint32_t MsgType : 16;
		uint32_t ItemId : 16;
	};

	static WM_PARAM _SendMenuMessage(WObj *pSrcWin, WObj *pDestWin, uint16_t MsgType, uint16_t ItemId) {
		if (!pDestWin)
			pDestWin = pSrcWin->Parent();
		if (pDestWin) {
			MsgPack MsgData;
			MsgData.MsgType = MsgType;
			MsgData.ItemId = ItemId;
			return pDestWin->Require(WM_MENU, *(WM_PARAM *)&MsgData);
		}
		return 0;
	}

	void _InvalidateItem(unsigned Index) {
		Invalidate();  /* Can be optimized, no need to invalidate all items */
	}

	bool _IsTopLevelMenu() { return !_SendMenuMessage(this, pOwner, MENU_IS_MENU, 0); }
	bool _HasEffect() { return (States & MENU_SF_POPUP) || !_IsTopLevelMenu(); }
	int _GetEffectSize() { return _HasEffect() ? EffectSize() : 0; }

	int _CalcTextWidth(const char *sText) {
		if (!sText)
			return 0;
		auto pOldFont = GUI.Font(Props.pFont);
		auto TextWidth = GUI_GetStringSizeX(sText);
		GUI.Font(pOldFont);
		return TextWidth;
	}
	int _GetItemWidth(unsigned Index) {
		if (Width && (States & MENU_CF_VERTICAL))
			return Width - (_GetEffectSize() << 1);
		auto &pItem = ItemArray[Index];
		auto ItemWidth = ((States & MENU_CF_VERTICAL) || !(pItem.Flags & MENU_IF_SEPARATOR))
			? pItem.TextWidth : 3;
		return ItemWidth + Props.aBorder[MENU_BI_LEFT] + Props.aBorder[MENU_BI_RIGHT];
	}
	int _GetItemHeight(unsigned Index) {
		if (Height && !(States & MENU_CF_VERTICAL))
			return Height - (_GetEffectSize() << 1);
		auto ItemHeight = Props.pFont->YSize;
		if ((States & MENU_CF_VERTICAL) && (ItemArray[Index].Flags & MENU_IF_SEPARATOR))
			ItemHeight = 3;
		return ItemHeight + Props.aBorder[MENU_BI_TOP] + Props.aBorder[MENU_BI_BOTTOM];
	}
	auto _GetNumItems() { return ItemArray.NumItems(); }
	int _CalcMenuSizeX() {
		auto NumItems = _GetNumItems();
		int xSize = 0;
		if (States & MENU_CF_VERTICAL) {
			for (unsigned i = 0; i < NumItems; i++) {
				auto ItemWidth = _GetItemWidth(i);
				if (ItemWidth > xSize)
					xSize = ItemWidth;
			}
		}
		else {
			for (unsigned i = 0; i < NumItems; i++)
				xSize += _GetItemWidth(i);
		}
		return xSize + (_GetEffectSize() << 1);
	}
	int _CalcMenuSizeY() {
		auto NumItems = _GetNumItems();
		int ySize = 0;
		if (States & MENU_CF_VERTICAL) {
			for (unsigned i = 0; i < NumItems; i++)
				ySize += _GetItemHeight(i);
		}
		else {
			for (unsigned i = 0; i < NumItems; i++) {
				auto ItemHeight = _GetItemHeight(i);
				if (ItemHeight > ySize)
					ySize = ItemHeight;
			}
		}
		return ySize + (_GetEffectSize() << 1);
	}

	int _CalcWindowSizeX() { return Width ? Width : _CalcMenuSizeX(); }
	int _CalcWindowSizeY() { return Height ? Height : _CalcMenuSizeY(); }

	int _GetItemFromPos(int x, int y) {
		auto EffectSize = _GetEffectSize();
		auto ySize = _CalcMenuSizeY();
		if (Height && Height < ySize)
			ySize = Height;
		auto xSize = _CalcMenuSizeX();
		if (Width && Width < xSize)
			xSize = Width;
		x -= EffectSize;
		y -= EffectSize;
		xSize -= (EffectSize << 1);
		ySize -= (EffectSize << 1);
		if ((x < 0) || (y < 0) || (x >= xSize) || (y >= ySize))
			return -1;
		auto NumItems = _GetNumItems();
		if (States & MENU_CF_VERTICAL) {
			int yPos = 0;
			for (unsigned i = 0; i < NumItems; i++) {
				yPos += _GetItemHeight(i);
				if (y < yPos)
					return i;
			}
		}
		else {
			int xPos = 0;
			for (unsigned i = 0; i < NumItems; i++) {
				xPos += _GetItemWidth(i);
				if (x < xPos)
					return i;
			}
		}
		return -1;
	}
	POINT _GetItemPos(uint16_t Index) {
		POINT Pos = _GetEffectSize();
		if (States & MENU_CF_VERTICAL)
			for (int i = 0; i < (int)Index; i++)
				Pos.y += _GetItemHeight(i);
		else
			for (int i = 0; i < Index; i++)
				Pos.x += _GetItemWidth(i);
		return Pos;
	}
	void _SetCapture() {
		if (!IsSubmenuActive && !HasCaptured())
			SetCapture(0);
	}
	void _ReleaseCapture() {
		if (HasCaptured() && _IsTopLevelMenu() && !(States & MENU_SF_POPUP))
			ReleaseCapture();
	}
	void _CloseSubmenu() {
		if (!(States & MENU_SF_ACTIVE))
			return;
		if (!IsSubmenuActive)
			return;
		auto &pItem = ItemArray[Sel];
		/* Inform submenu about its deactivation and detach it */
		_SendMenuMessage(this, pItem.pSubmenu, MENU_ON_CLOSE, 0);
		pItem.pSubmenu->Detach();
		IsSubmenuActive = 0;
		/*
		 * Keep capture in menu widget. The capture may only released
		 * by clicking outside the menu or when mouse moved out.
		 * And it may only released from a top level menu.
		 */
		_SetCapture();
		/* Invalidate menu item. This is needed because the appearance may have changed */
		_InvalidateItem(Sel);
	}
	void _OpenSubmenu(uint16_t Index) {
		if (!(States & MENU_SF_ACTIVE))
			return;
		bool PrevActiveSubmenu = IsSubmenuActive;
		/* Close previous submenu (if needed) */
		_CloseSubmenu();
		auto &pItem = ItemArray[Index];
		if (!pItem.pSubmenu)
			return;
		if (pItem.Flags & MENU_IF_DISABLED)
			return;
		/* Calculate position of submenu */
		auto EffectSize = _GetEffectSize();
		auto Pos = _GetItemPos(Index);
		if (States & MENU_CF_VERTICAL) {
			Pos.x += _CalcMenuSizeX() - (_GetEffectSize() << 1);
			Pos.y -= EffectSize;
		}
		else {
			Pos.y += _CalcMenuSizeY() - (_GetEffectSize() << 1);
			Pos.x -= EffectSize;
		}
		Pos += GetOrg();
		/*
			* Notify owner window when for the first time open a menu (when no
			* other submenu was open), so it can initialize the menu items.
			*/
		if (!PrevActiveSubmenu)
			if (_IsTopLevelMenu())
				_SendMenuMessage(this, pOwner, MENU_ON_INITMENU, 0);
		/* Notify owner window when a submenu opens, so it can initialize the menu items. */
		_SendMenuMessage(this, pOwner, MENU_ON_INITSUBMENU, pItem.Id);
		/* Set active menu as owner of submenu. */
		pItem.pSubmenu->SetOwner(this);
		/* Attach submenu and inform it about its activation. */
		pItem.pSubmenu->Attach(WObj::GetDesktopWindow(), Pos);
		_SendMenuMessage(this, pItem.pSubmenu, MENU_ON_OPEN, 0);
		IsSubmenuActive = true;
		/* Invalidate menu item. This is needed because the appearance may have changed. */
		_InvalidateItem(Index);
	}
	void _ClosePopup() {
		if (!(States & MENU_SF_POPUP))
			return;
		States &= ~(MENU_SF_POPUP);
		Detach();
		ReleaseCapture();
	}
	void _SetSelection(int Index) {
		if (Index != Sel) {
			_InvalidateItem(Sel); /* Invalidate previous selection */
			_InvalidateItem(Index);     /* Invalidate new selection */
			Sel = Index;
		}
	}
	void _SelectItem(uint16_t Index) {
		if (Sel != (int)Index) {
			_SetCapture();
			_OpenSubmenu(Index);
			_SetSelection(Index);
		}
	}
	void _DeselectItem() {
		if (!IsSubmenuActive) {
			_SetSelection(-1);
			_ReleaseCapture();
		}
	}
	void _ActivateItem(unsigned Index) {
		auto &pItem = ItemArray[Index];
		if (pItem.pSubmenu)
			return;
		if (pItem.Flags & (MENU_IF_DISABLED | MENU_IF_SEPARATOR))
			return;
		_ClosePopup();
		/* Send item select message to owner. */
		_SendMenuMessage(this, pOwner, MENU_ON_ITEMSELECT, pItem.Id);
	}
	void _ActivateMenu(unsigned Index) {
		if (!(States & MENU_CF_OPEN_ON_POINTEROVER)) {
			auto &pItem = ItemArray[Index];
			if (pItem.pSubmenu) {
				if ((pItem.Flags & MENU_IF_DISABLED) == 0) {
					if (!(States & MENU_SF_ACTIVE)) {
						States |= MENU_SF_ACTIVE;
						_OpenSubmenu(Index);
						_SetSelection(Index);
					}
					else if (States & MENU_CF_CLOSE_ON_SECOND_CLICK) {
						if ((int)Index == Sel) {
							_CloseSubmenu();
							States &= ~MENU_SF_ACTIVE;
						}
					}
				}
			}
		}
	}
	void _DeactivateMenu() {
		_CloseSubmenu();
		if (!(States & MENU_CF_OPEN_ON_POINTEROVER))
			States &= ~MENU_SF_ACTIVE;
	}
	int _ForwardMouseOverMsg(POINT Pos) {
#if (GUI_SUPPORT_MOUSE)
		if (!IsSubmenuActive && !(States & MENU_SF_POPUP)) {
			if (_IsTopLevelMenu()) {
				Pos += GetOrg();
				if (auto pBelow = WM_Screen2Win(Pos); pBelow && (pBelow != this)) {
					PID_STATE State = Pos - pBelow->GetOrg();
					pBelow->Require(WM_MOUSEOVER, (WM_PARAM)&State);
					return true;
				}
			}
		}
#endif
		return false;
	}
	bool _HandlePID(PID_STATE State) {
		auto PrevState = WM_PID__GetPrevState();
		/* Check if coordinates are inside the widget */
		bool XYInWidget = (State.x >= 0) && (State.y >= 0);
		if (XYInWidget) {
			auto r = GetClientRect();
			XYInWidget = (State.x <= r.x1) && (State.y <= r.y1);
		}
		if (!XYInWidget) {
			/* Handle PID when coordinates are outside the widget */
			if (State.Pressed == 1 && PrevState.Pressed == 0) {
				/* User has clicked outside the menu. Close the active submenu.
				 * The widget itself must be closed (if needed) by the owner. */
				_DeactivateMenu();
				_ClosePopup();
			}
			_DeselectItem();
			_ForwardMouseOverMsg(State);
			return true;   /* Coordinates are not in widget, we need to forward PID message to owner */
		}
		auto ItemIndex = _GetItemFromPos(State.x, State.y);
		/* Handle PID when coordinates are inside the widget */
		if (ItemIndex >= 0) {
			/* Coordinates are inside the menu */
			if (State.Pressed == 1) {
				if (PrevState.Pressed == 0) /* Clicked */
					_ActivateMenu(ItemIndex);
				_SelectItem(ItemIndex);
			}
			else if (State.Pressed == 0 && PrevState.Pressed == 1) /* Released */
				_ActivateItem(ItemIndex);
			else if (State.Pressed < 0) {  /* Mouse moved */
				if (!_ForwardMouseOverMsg(State))
					_SelectItem(ItemIndex);
				else
					_DeselectItem();
			}
		}
		/* Coordinates are outside the menu but inside the widget */
		else if (State.Pressed == 1) {
			if (PrevState.Pressed == 0) /* Clicked */
				/* User has clicked outside the menu. Close the active submenu.
					* The widget itself must be closed (if needed) by the owner. */
				_DeactivateMenu();
			_DeselectItem();
		}
		else if (State.Pressed < 0) /* Moved out or mouse moved */
			_DeselectItem();
		return false;
	}
	void _ForwardPIDMsgToOwner(int MsgId, const PID_STATE *pState) {
		if (!_IsTopLevelMenu()) {
			if (auto pOwner = this->pOwner ? this->pOwner : Parent()) {
				PID_STATE State{ 0 };
				if (pState) {
					State = *pState;
					State += GetOrg() - pOwner->GetOrg();
					pState = &State;
				}
				pOwner->Require(MsgId, (WM_PARAM)pState);
			}
		}
	}
	void _ResizeMenu() {
		SetSize({ _CalcWindowSizeX(), _CalcWindowSizeY() });
		Invalidate();
	}
	WM_PARAM _OnMenu(WM_PARAM Data) {
		auto msg = *(MsgPack *)&Data;
		switch (msg.MsgType) {
		case MENU_ON_ITEMSELECT:
			_DeactivateMenu();
			_DeselectItem();
			_ClosePopup();
			/* No break here. We need to forward message to owner. */
		case MENU_ON_INITMENU:
		case MENU_ON_INITSUBMENU:
			/* Forward message to owner. */
			if (auto pOwner = this->pOwner ? this->pOwner : Parent())
				pOwner->Require(WM_MENU, Data);
			break;
		case MENU_ON_OPEN:
			Sel = -1;
			IsSubmenuActive = 0;
			States |= MENU_SF_ACTIVE | MENU_CF_OPEN_ON_POINTEROVER;
			_SetCapture();
			_ResizeMenu();
			break;
		case MENU_ON_CLOSE:
			_CloseSubmenu();
			break;
		case MENU_IS_MENU:
			return 1;
		}
		return 0;
	}
	char _OnTouch(const PID_STATE *pState) {
		return pState ? _HandlePID(*pState) : _HandlePID({ -1, -1 });
	}
#if (GUI_SUPPORT_MOUSE)
	char _OnMouseOver(const PID_STATE *pState) {
		return pState ? _HandlePID({ *pState, -1 }) : 0;
	}
#endif
	void _SetPaintColors(const Item &pItem, int ItemIndex) {
		bool Selected = (ItemIndex == Sel);
		unsigned ColorIndex;
		if (IsSubmenuActive && Selected)
			ColorIndex = MENU_CI_ACTIVE_SUBMENU;
		else if (pItem.Flags & MENU_IF_SEPARATOR)
			ColorIndex = MENU_CI_ENABLED;
		else {
			ColorIndex = Selected ? MENU_CI_SELECTED : MENU_CI_ENABLED;
			if (pItem.Flags & MENU_IF_DISABLED)
				ColorIndex = (States & MENU_CF_HIDE_DISABLED_SEL) ? MENU_CI_DISABLED : ColorIndex + MENU_CI_DISABLED;
		}
		GUI.BkColor(Props.aBkColor[ColorIndex]);
		GUI.Color(Props.aTextColor[ColorIndex]);
	}
	void _OnPaint() {
		auto NumItems = _GetNumItems();
		auto BorderLeft = Props.aBorder[MENU_BI_LEFT];
		auto BorderTop = Props.aBorder[MENU_BI_TOP];
		auto FontHeight = Props.pFont->YSize;
		auto EffectSize = _GetEffectSize();
		auto FillRect = GetClientRect();
		RECT TextRect;
		FillRect -= EffectSize;
		GUI.Font(Props.pFont);
		if (States & MENU_CF_VERTICAL) {
			auto xSize = _CalcMenuSizeX();
			FillRect.x1 = xSize - EffectSize - 1;
			TextRect.x0 = FillRect.x0 + BorderLeft;
			for (unsigned i = 0; i < NumItems; i++) {
					auto &pItem = ItemArray[i];
					auto ItemHeight = _GetItemHeight(i);
					_SetPaintColors(pItem, i);
					FillRect.y1 = FillRect.y0 + ItemHeight - 1;
					if (pItem.Flags & MENU_IF_SEPARATOR) {
						GUI_ClearRect(FillRect);
						GUI.Color(RGB_GRAYL(0x7C));
						GUI_DrawHLine(FillRect.y0 + BorderTop + 1, FillRect.x0 + 2, FillRect.x1 - 2);
					}
					else {
						auto TextWidth = pItem.TextWidth;
						TextRect.x1 = TextRect.x0 + TextWidth - 1;
						TextRect.y0 = FillRect.y0 + BorderTop;
						TextRect.y1 = TextRect.y0 + FontHeight - 1;
						WIDGET__FillStringInRect(pItem.pText, FillRect, TextRect, TextRect);
					}
					FillRect.y0 += ItemHeight;
				}
		}
		else {
			auto ySize = _CalcMenuSizeY();
			FillRect.y1 = ySize - EffectSize - 1;
			TextRect.y0 = FillRect.y0 + BorderTop;
			TextRect.y1 = TextRect.y0 + FontHeight - 1;
			for (unsigned i = 0; i < NumItems; i++) {
					auto &pItem = ItemArray[i];
					auto ItemWidth = _GetItemWidth(i);
					_SetPaintColors(pItem, i);
					FillRect.x1 = FillRect.x0 + ItemWidth - 1;
					if (pItem.Flags & MENU_IF_SEPARATOR) {
						GUI_ClearRect(FillRect);
						GUI.Color(RGB_GRAYL(0x7C));
						GUI_DrawVLine(FillRect.x0 + BorderLeft + 1, FillRect.y0 + 2, FillRect.y1 - 2);
					}
					else {
						auto TextWidth = pItem.TextWidth;
						TextRect.x0 = FillRect.x0 + BorderLeft;
						TextRect.x1 = TextRect.x0 + TextWidth - 1;
						WIDGET__FillStringInRect(pItem.pText, FillRect, TextRect, TextRect);
					}
					FillRect.x0 += ItemWidth;
				}
		}
		if (Width || Height) {
			auto r = GetClientRect();
			r -= EffectSize;
			GUI.BkColor(Props.aBkColor[MENU_CI_ENABLED]);
			GUI_ClearRect({ FillRect.x1 + 1, EffectSize, r.x1, FillRect.y1 });
			GUI_ClearRect({ EffectSize, FillRect.y1 + 1, r.x1, r.y1 });
		}
		/* Draw 3D effect (if configured) */
		if (_HasEffect())
			DrawUp();
	}

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (Menu *)hWin;
		if (MsgId != WM_PID_STATE_CHANGED)
			/* Let widget handle the standard messages */
			if (!pObj->HandleActive(MsgId, &Data))
				return Data;
		switch (MsgId) {
			case WM_MENU:
				return pObj->_OnMenu(Data);
			case WM_TOUCH:
				if (pObj->_OnTouch((const PID_STATE *)Data))
					pObj->_ForwardPIDMsgToOwner(WM_TOUCH, (const PID_STATE *)Data);
				break;
#if (GUI_SUPPORT_MOUSE)
			case WM_MOUSEOVER:
				if (pObj->_OnMouseOver((const PID_STATE *)Data))
					pObj->_ForwardPIDMsgToOwner(WM_MOUSEOVER, (const PID_STATE *)Data);
				break;
#endif
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_DELETE: {
				unsigned _n = pObj->ItemArray.NumItems();
				for (unsigned _i = 0; _i < _n; _i++) {
					GUI_ALLOC_FreePtr((void **)&pObj->ItemArray[_i].pText);
				}
				pObj->ItemArray.Delete();
				break;
			}
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:
	Menu(uint16_t ExFlags, int Id) :
		Widget({}, WC_VISIBLE | WC_STAYONTOP, _Callback, nullptr, Id, ExFlags | WIDGET_STATE_FOCUSSABLE) {
		Detach();
		if (ExFlags & MENU_CF_OPEN_ON_POINTEROVER)
			States |= MENU_SF_ACTIVE;
		else
			States &= ~(MENU_SF_ACTIVE);
		Sel = -1;
		SetEffect(MENU__pDefaultEffect);
	}

private:

	void _RecalcTextWidthOfItems() {
		auto NumItems = _GetNumItems();
		auto pOldFont = GUI.Font(Props.pFont);
		for (unsigned i = 0; i < NumItems; i++) {
			auto &pItem = ItemArray[i];
			pItem.TextWidth = GUI_GetStringSizeX(pItem.pText);
		}
		GUI.Font(pOldFont);
	}
	char _SetItem(unsigned Index, const ItemData *pItemData) {
		auto pText = pItemData->pText ? pItemData->pText : "";
		Item item{
			.pSubmenu = (pItemData->Flags & MENU_IF_SEPARATOR) ? nullptr : pItemData->pSubmenu,
			.Id = pItemData->Id,
			.Flags = pItemData->Flags,
			.TextWidth = (uint16_t)_CalcTextWidth(pText)
		};
		if (Index >= ItemArray.NumItems())
			return 0;
		auto &pItem = ItemArray[Index];
		GUI__SetText(&pItem.pText, pText);
		pItem.Id       = item.Id;
		pItem.Flags    = item.Flags;
		pItem.pSubmenu = item.pSubmenu;
		pItem.TextWidth= item.TextWidth;
		if (item.pSubmenu)
			pItem.pSubmenu->SetOwner(this);
		return 1;
	}
	void _SetItemFlags(unsigned Index, uint16_t Mask, uint16_t Flags) {
		auto &pItem = ItemArray[Index];
		pItem.Flags &= ~Mask;
		pItem.Flags |= Flags;
	}
	int _FindItem(uint16_t ItemId, Menu **pMenu) {
		auto NumItems = _GetNumItems();
		for (unsigned i = 0; i < NumItems; i++) {
			auto &pItem = ItemArray[i];
			if (pItem.Id == ItemId) {
				*pMenu = this;
				return i;
			}
			if (pItem.pSubmenu) {
				if (auto ItemIndex = pItem.pSubmenu->_FindItem(ItemId, pMenu); ItemIndex >= 0)
					return ItemIndex;
			}
		}
		return -1;
	}

public:

	void AddItem(const ItemData *pItemData) {
		if (!pItemData)
			return;
		if (ItemArray.AddItem() != 0)
			return;
		auto Index = _GetNumItems() - 1;
		if (_SetItem(Index, pItemData))
			_ResizeMenu();
		else
			ItemArray.DeleteItem(Index);
	}
	void SetOwner(WObj *pOwner) {
		this->pOwner = pOwner;
	}
	void AttachMenu(WObj *pDestWin, POINT Pos, int xSize, int ySize) {
		Width = xSize > 0 ? xSize : 0;
		Height = ySize > 0 ? ySize : 0;
		Attach(pDestWin, Pos);
		_ResizeMenu();
		ShowWindow();
	}

	void DeleteItem(uint16_t ItemId) {
		Menu *pMenu;
		auto Index = _FindItem(ItemId, &pMenu);
		if (Index < 0)
			return;
		GUI_ALLOC_FreePtr((void **)&pMenu->ItemArray[Index].pText);
		ItemArray.DeleteItem(Index);
		_ResizeMenu();
	}
	void DisableItem(uint16_t ItemId) {
		Menu *pMenu;
		auto Index = _FindItem(ItemId, &pMenu);
		if (Index < 0)
			return;
		_SetItemFlags(Index, MENU_IF_DISABLED, MENU_IF_DISABLED);
		_InvalidateItem(Index);
	}
	void EnableItem(uint16_t ItemId) {
		Menu *pMenu;
		auto Index = _FindItem(ItemId, &pMenu);
		if (Index < 0)
			return;
		_SetItemFlags(Index, MENU_IF_DISABLED, 0);
		_InvalidateItem(Index);
	}
	void GetItem(uint16_t ItemId, ItemData *pItemData) {
		if (!pItemData)
			return;
		Menu *pMenu;
		auto Index = _FindItem(ItemId, &pMenu);
		if (Index < 0)
			return;
		auto &pItem = ItemArray[Index];
		pItemData->Flags = pItem.Flags;
		pItemData->Id = pItem.Id;
		pItemData->pSubmenu = pItem.pSubmenu;
		pItemData->pText = nullptr;
	}
	void GetItemText(uint16_t ItemId, char *pBuffer, unsigned BufferSize) {
		if (!pBuffer)
			return;
		Menu *pMenu;
		auto Index = _FindItem(ItemId, &pMenu);
		if (Index < 0)
			return;
		auto &pItem = ItemArray[Index];
		strncpy(pBuffer, pItem.pText, BufferSize);
		pBuffer[BufferSize - 1] = 0;
	}
	auto GetNumItems() { return _GetNumItems(); }
	void InsertItem(uint16_t ItemId, const ItemData *pItemData) {
		if (!pItemData)
			return;
		Menu *pMenu;
		auto Index = _FindItem(ItemId, &pMenu);
		if (Index < 0)
			return;
		if (ItemArray.InsertBlankItem(Index) != 0)
			return;
		if (_SetItem(Index, pItemData))
			_ResizeMenu();
		else
			ItemArray.DeleteItem(Index);
	}
	void Popup(WObj *pDestWin, int x, int y, int xSize, int ySize) {
		if (!pDestWin)
			return;
		States |= MENU_SF_POPUP;
		Width = xSize > 0 ? xSize : 0;
		Height = ySize > 0 ? ySize : 0;
		auto Pos = pDestWin->GetOrg();
		SetOwner(pDestWin);
		Attach(WObj::GetDesktopWindow(), Pos);
		_SendMenuMessage(pDestWin, this, MENU_ON_OPEN, 0);
	}
	void BkColor(unsigned ColorIndex, RGBC Color) {
		if (ColorIndex >= GUI_COUNTOF(Props.aBkColor))
			return;
		if (Color == Props.aBkColor[ColorIndex])
			return;
		Props.aBkColor[ColorIndex] = Color;
		Invalidate();
	}
	void SetBorderSize(unsigned BorderIndex, uint8_t BorderSize) {
		if (BorderIndex >= GUI_COUNTOF(Props.aBorder))
			return;
		if (BorderSize == Props.aBorder[BorderIndex])
			return;
		Props.aBorder[BorderIndex] = BorderSize;
		_ResizeMenu();
	}
	void Font(PCFONT pFont) {
		if (pFont == Props.pFont)
			return;
		Props.pFont = pFont;
		_RecalcTextWidthOfItems();
		_ResizeMenu();
	}
	void SetItem(uint16_t ItemId, const ItemData *pItemData) {
		if (!pItemData)
			return;
		Menu *pMenu;
		auto Index = _FindItem(ItemId, &pMenu);
		if (Index < 0)
			return;
		if (_SetItem(Index, pItemData))
			_ResizeMenu();
	}
	void TextColor(unsigned ColorIndex, RGBC Color) {
		if (ColorIndex >= GUI_COUNTOF(Props.aTextColor))
			return;
		if (Color == Props.aTextColor[ColorIndex])
			return;
		Props.aTextColor[ColorIndex] = Color;
		Invalidate();
	}
};

Menu::Properties Menu::DefaultProps;

}
