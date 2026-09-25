export module TUX.Widget.Menu;

#include "GUIConf.h"

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
	MENU_SF_POPUP                    = WIDGET_STATE_USER<5>,
	MENU_SF_SUBMENU_ACTIVE           = WIDGET_STATE_USER<6>;

constexpr uint16_t
	MENU_IF_DISABLED           = 1 << 0,
	MENU_IF_SEPARATOR          = 1 << 1;

enum MENU_MSG : uint16_t {
	MENU_ON_INITMENU = 0, /* Send to owner when for the first time selecting a submenu */
	MENU_ON_ITEMSELECT  , /* Send to owner when selecting a menu item */
	MENU_ON_INITSUBMENU , /* Send to owner when selecting a submenu */
	MENU_ON_OPEN        , /* Internal message of menu widget (send to submenus) */
	MENU_ON_CLOSE       , /* Internal message of menu widget (send to submenus) */
	MENU_IS_MENU          /* Internal message of menu widget. Owner must call   */
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
		uint16_t Id;
		uint16_t Flags;
		Menu *pSubmenu;
	};

public:
	struct Properties {
		PCFONT pFont{ GUI_DEFAULT_FONT };
		BRUSH aBrush[5]{
			/* Index                  | Background      | Text              */
			/* enabled  unselected */ { RGB_LIGHTGRAY   , RGB_BLACK         },
			/* enabled    selected */ { RGBC::Blue(0x98), RGB_WHITE         },
			/* disabled unselected */ { RGB_LIGHTGRAY   , RGBC::Gray(0x7C)  },
			/* disabled   selected */ { RGBC::Blue(0x98), RGB_LIGHTGRAY     },
			/* active submenu      */ { RGBC::Gray(0x7C), RGB_WHITE         }
		};
		RECT Border{ 4, 2, 4, 2 };
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
	ARRAY<Item> items;
	WObj *pOwner = nullptr;
	uint16_t Width = 0, Height = 0;
	uint16_t Sel = -1;

	~Menu() {
		for (uint16_t i = 0, n = items.NumItems(); i < n; i++)
			GUI_MEM_Free(items[i].pText);
		items.Delete();
	}

	struct MsgPack {
		MENU_MSG MsgType;
		uint16_t ItemId;
	};

	static WM_PARAM _SendMenuMessage(WObj *pSrcWin, WObj *pDestWin, MENU_MSG MsgType, uint16_t ItemId) {
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

	void _InvalidateItem(uint16_t Index) {
		Invalidate();  /* Can be optimized, no need to invalidate all items */
	}

	bool _IsTopLevelMenu() const { return !_SendMenuMessage(const_cast<Menu *>(this), pOwner, MENU_IS_MENU, 0); }
	bool _HasEffect() const { return (States & MENU_SF_POPUP) || !_IsTopLevelMenu(); }
	uint16_t _GetEffectSize() const { return _HasEffect() ? EffectSize() : 0; }

	uint16_t _GetItemWidth(uint16_t Index) const {
		if (Width && (States & MENU_CF_VERTICAL))
			return Width - (_GetEffectSize() << 1);
		auto &item = items[Index];
		uint16_t ItemWidth = (States & MENU_CF_VERTICAL) || !(item.Flags & MENU_IF_SEPARATOR)
			? item.TextWidth : 3;
		return ItemWidth + Props.Border.x0 + Props.Border.x1;
	}
	uint16_t _GetItemHeight(uint16_t Index) const {
		if (Height && !(States & MENU_CF_VERTICAL))
			return Height - (_GetEffectSize() << 1);
		uint16_t ItemHeight = Props.pFont->YSize;
		if ((States & MENU_CF_VERTICAL) && (items[Index].Flags & MENU_IF_SEPARATOR))
			ItemHeight = 3;
		return ItemHeight + Props.Border.y0 + Props.Border.y1;
	}

	uint16_t _CalcMenuSizeX() const {
		auto NumItems = GetNumItems();
		uint16_t xSize = 0;
		if (States & MENU_CF_VERTICAL)
			for (uint16_t i = 0; i < NumItems; i++) {
				if (auto ItemWidth = _GetItemWidth(i); ItemWidth > xSize)
					xSize = ItemWidth;
			}
		else
			for (uint16_t i = 0; i < NumItems; i++)
				xSize += _GetItemWidth(i);
		return xSize + (_GetEffectSize() << 1);
	}
	uint16_t _CalcMenuSizeY() const {
		auto NumItems = GetNumItems();
		uint16_t ySize = 0;
		if (States & MENU_CF_VERTICAL)
			for (uint16_t i = 0; i < NumItems; i++)
				ySize += _GetItemHeight(i);
		else
			for (uint16_t i = 0; i < NumItems; i++)
				if (auto ItemHeight = _GetItemHeight(i); ItemHeight > ySize)
					ySize = ItemHeight;
		return ySize + (_GetEffectSize() << 1);
	}

	uint16_t _CalcWindowSizeX() const { return Width ? Width : _CalcMenuSizeX(); }
	uint16_t _CalcWindowSizeY() const { return Height ? Height : _CalcMenuSizeY(); }

	int _GetItemFromPos(POINT Pos) const {
		if (!(RECT{ _GetEffectSize(), { _CalcMenuSizeX(), _CalcMenuSizeY() } } <= Pos))
			return -1;
		auto NumItems = GetNumItems();
		if (States & MENU_CF_VERTICAL) {
			int yPos = 0;
			for (uint16_t i = 0; i < NumItems; i++) {
				yPos += _GetItemHeight(i);
				if (Pos.y < yPos)
					return i;
			}
		}
		else {
			int xPos = 0;
			for (uint16_t i = 0; i < NumItems; i++) {
				xPos += _GetItemWidth(i);
				if (Pos.x < xPos)
					return i;
			}
		}
		return -1;
	}

	POINT _GetItemPos(uint16_t Index) const {
		POINT Pos = _GetEffectSize();
		if (States & MENU_CF_VERTICAL)
			for (uint16_t i = 0; i < Index; i++)
				Pos.y += _GetItemHeight(i);
		else
			for (uint16_t i = 0; i < Index; i++)
				Pos.x += _GetItemWidth(i);
		return Pos;
	}

	void _SetCapture() {
		if (!(States & MENU_SF_SUBMENU_ACTIVE) && !HasCaptured())
			SetCapture(0);
	}
	void _ReleaseCapture() {
		if (HasCaptured() && _IsTopLevelMenu() && !(States & MENU_SF_POPUP))
			ReleaseCapture();
	}
	void _CloseSubmenu() {
		if (!(States & MENU_SF_ACTIVE))
			return;
		if (!(States & MENU_SF_SUBMENU_ACTIVE))
			return;
		auto &item = items[Sel];
		/* Inform submenu about its deactivation and detach it */
		_SendMenuMessage(this, item.pSubmenu, MENU_ON_CLOSE, 0);
		item.pSubmenu->Detach();
		States &= ~MENU_SF_SUBMENU_ACTIVE;
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
		bool PrevActiveSubmenu = States & MENU_SF_SUBMENU_ACTIVE;
		/* Close previous submenu (if needed) */
		_CloseSubmenu();
		auto &item = items[Index];
		if (!item.pSubmenu)
			return;
		if (item.Flags & MENU_IF_DISABLED)
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
		Pos += LeftTop();
		/*
			* Notify owner window when for the first time open a menu (when no
			* other submenu was open), so it can initialize the menu items.
			*/
		if (!PrevActiveSubmenu)
			if (_IsTopLevelMenu())
				_SendMenuMessage(this, pOwner, MENU_ON_INITMENU, 0);
		/* Notify owner window when a submenu opens, so it can initialize the menu items. */
		_SendMenuMessage(this, pOwner, MENU_ON_INITSUBMENU, item.Id);
		/* Set active menu as owner of submenu. */
		item.pSubmenu->SetOwner(this);
		/* Attach submenu and inform it about its activation. */
		item.pSubmenu->Attach(WObj::GetDesktopWindow(), Pos);
		_SendMenuMessage(this, item.pSubmenu, MENU_ON_OPEN, 0);
		States |= MENU_SF_SUBMENU_ACTIVE;
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
	void _SetSelection(int16_t Index) {
		if (Index != Sel) {
			_InvalidateItem(Sel); /* Invalidate previous selection */
			_InvalidateItem(Index);     /* Invalidate new selection */
			Sel = Index;
		}
	}
	void _SelectItem(uint16_t Index) {
		if (Sel != Index) {
			_SetCapture();
			_OpenSubmenu(Index);
			_SetSelection(Index);
		}
	}
	void _DeselectItem() {
		if (!(States & MENU_SF_SUBMENU_ACTIVE)) {
			_SetSelection(-1);
			_ReleaseCapture();
		}
	}
	void _ActivateItem(uint16_t Index) {
		auto &item = items[Index];
		if (item.pSubmenu)
			return;
		if (item.Flags & (MENU_IF_DISABLED | MENU_IF_SEPARATOR))
			return;
		_ClosePopup();
		/* Send item select message to owner. */
		_SendMenuMessage(this, pOwner, MENU_ON_ITEMSELECT, item.Id);
	}
	void _ActivateMenu(uint16_t Index) {
		if (!(States & MENU_CF_OPEN_ON_POINTEROVER)) {
			auto &item = items[Index];
			if (item.pSubmenu) {
				if ((item.Flags & MENU_IF_DISABLED) == 0) {
					if (!(States & MENU_SF_ACTIVE)) {
						States |= MENU_SF_ACTIVE;
						_OpenSubmenu(Index);
						_SetSelection(Index);
					}
					else if (States & MENU_CF_CLOSE_ON_SECOND_CLICK) {
						if (Index == Sel) {
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
	bool _ForwardMouseOverMsg(POINT Pos) {
#if (GUI_SUPPORT_MOUSE)
		if (!(States & MENU_SF_SUBMENU_ACTIVE) && !(States & MENU_SF_POPUP)) {
			if (_IsTopLevelMenu()) {
				Pos += LeftTop();
				if (auto pBelow = WM_Screen2Win(Pos); pBelow && (pBelow != this)) {
					PID_STATE State = Pos - pBelow->LeftTop();
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
		if (!(ClientRect() <= State)) {
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
		auto ItemIndex = _GetItemFromPos(State);
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
					State += LeftTop() - pOwner->LeftTop();
					pState = &State;
				}
				pOwner->Require(MsgId, (WM_PARAM)pState);
			}
		}
	}
	void _ResizeMenu() {
		Size({ _CalcWindowSizeX(), _CalcWindowSizeY() });
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
			States &= ~MENU_SF_SUBMENU_ACTIVE;
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
	void _SetPaintColors(const Item &item, int ItemIndex) const {
		bool Selected = ItemIndex == Sel;
		auto ColorIndex = 
			(States & MENU_SF_SUBMENU_ACTIVE) && Selected ? MENU_CI_ACTIVE_SUBMENU :
			item.Flags & MENU_IF_SEPARATOR ? MENU_CI_ENABLED :
			item.Flags & MENU_IF_DISABLED ?
				!(States & MENU_CF_HIDE_DISABLED_SEL) && Selected ?
					MENU_CI_DISABLED_SEL : MENU_CI_DISABLED :
				Selected ? MENU_CI_SELECTED : MENU_CI_ENABLED;
		GUI.Brush(Props.aBrush[ColorIndex]);
	}
	void _OnPaint() const {
		auto NumItems = GetNumItems();
		auto FontHeight = Props.pFont->YSize;
		auto EffectSize = _GetEffectSize();
		auto rFill = ClientRect() / EffectSize;
		RECT rText;
		GUI.Font(Props.pFont);

		if (States & MENU_CF_VERTICAL) {
			auto xSize = _CalcMenuSizeX();
			rFill.x1 = xSize - EffectSize - 1;
			rText.x0 = rFill.x0 + Props.Border.x0;
			for (uint16_t i = 0; i < NumItems; i++) {
				auto &item = items[i];
				auto ItemHeight = _GetItemHeight(i);
				_SetPaintColors(item, i);
				rFill.y1 = rFill.y0 + ItemHeight - 1;
				if (item.Flags & MENU_IF_SEPARATOR) {
					GUI.Clear(rFill);
					GUI.Color(RGBC::Gray(0x7C));
					GUI.DrawHLine(rFill.y0 + Props.Border.y0 + 1, rFill.x0 + 2, rFill.x1 - 2);
				}
				else {
					auto TextWidth = item.TextWidth;
					rText.x1 = rText.x0 + TextWidth - 1;
					rText.y0 = rFill.y0 + Props.Border.y0;
					rText.y1 = rText.y0 + FontHeight - 1;
					GUI.Clear(rFill);
					GUI_DispStringInRect(item.pText, rText, TEXTALIGN_LEFT | TEXTALIGN_VCENTER);
				}
				rFill.y0 += ItemHeight;
			}
		}
		else {
			auto ySize = _CalcMenuSizeY();
			rFill.y1 = ySize - EffectSize - 1;
			rText.y0 = rFill.y0 + Props.Border.y0;
			rText.y1 = rText.y0 + FontHeight - 1;
			for (uint16_t i = 0; i < NumItems; i++) {
				auto &item = items[i];
				auto ItemWidth = _GetItemWidth(i);
				_SetPaintColors(item, i);
				rFill.x1 = rFill.x0 + ItemWidth - 1;
				if (item.Flags & MENU_IF_SEPARATOR) {
					GUI.Clear(rFill);
					GUI.Color(RGBC::Gray(0x7C));
					GUI.DrawVLine(rFill.x0 + Props.Border.x0 + 1, rFill.y0 + 2, rFill.y1 - 2);
				}
				else {
					auto TextWidth = item.TextWidth;
					rText.x0 = rFill.x0 + Props.Border.x0;
					rText.x1 = rText.x0 + TextWidth - 1;
					GUI.Clear(rFill);
					GUI_DispStringInRect(item.pText, rText, TEXTALIGN_LEFT | TEXTALIGN_VCENTER);
				}
				rFill.x0 += ItemWidth;
			}
		}

		if (Width || Height) {
			auto r = ClientRect() / EffectSize;
			GUI.BkColor(Props.aBrush[MENU_CI_ENABLED].BkColor);
			GUI.Clear({ rFill.x1 + 1, EffectSize, r.x1, rFill.y1 });
			GUI.Clear({ EffectSize, rFill.y1 + 1, r.x1, r.y1 });
		}
		/* Draw 3D effect (if configured) */
		if (_HasEffect())
			DrawUp();
	}

	static WM_PARAM _Callback(WObj *pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (Menu *)pWin;
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_MENU:
				return pObj->_OnMenu(Data);
			case WM_TOUCH:
				if (auto pState = (const PID_STATE *)Data) {
					if (pObj->_HandlePID(*pState))
						pObj->_ForwardPIDMsgToOwner(WM_TOUCH, (const PID_STATE *)Data);
				} else pObj->_HandlePID({ -1, -1 });
				return 0;
#if (GUI_SUPPORT_MOUSE)
			case WM_MOUSEOVER:
				if (auto pState = (const PID_STATE *)Data)
					if (pObj->_HandlePID({ *pState, -1 }))
						pObj->_ForwardPIDMsgToOwner(WM_MOUSEOVER, (const PID_STATE *)Data);
				return 0;
#endif
			case WM_DELETE:
				pObj->~Menu();
				return 0;
		}
		return pObj->WidgetProc(MsgId, Data);
	}

public:
	Menu(uint16_t ExFlags, int Id) :
		Widget({}, WC_VISIBLE | WC_STAYONTOP, _Callback, nullptr, Id, ExFlags) {
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
		for (uint16_t i = 0, NumItems = GetNumItems(); i < NumItems; i++) {
			auto &item = items[i];
			item.TextWidth = Props.pFont->TextBound(item.pText).x;
		}
	}
	bool _SetItem(uint16_t Index, const ItemData *pItemData) {
		if (Index >= items.NumItems())
			return false;
		auto &item = items[Index];
		item.Id        = pItemData->Id;
		item.Flags     = pItemData->Flags;
		item.pSubmenu  = pItemData->Flags & MENU_IF_SEPARATOR ? nullptr : pItemData->pSubmenu;
		GUI__SetText(item.pText, pItemData->pText);
		item.TextWidth = Props.pFont->TextBound(item.pText).x;
		if (item.pSubmenu)
			item.pSubmenu->SetOwner(this);
		return true;
	}
	void _SetItemFlags(uint16_t Index, uint16_t Mask, uint16_t Flags) {
		auto &item = items[Index];
		item.Flags &= ~Mask;
		item.Flags |= Flags;
	}
	int _FindItem(uint16_t ItemId, Menu **pMenu) {
		auto NumItems = GetNumItems();
		for (uint16_t i = 0; i < NumItems; i++) {
			auto &item = items[i];
			if (item.Id == ItemId) {
				*pMenu = this;
				return i;
			}
			if (item.pSubmenu) {
				if (auto ItemIndex = item.pSubmenu->_FindItem(ItemId, pMenu); ItemIndex >= 0)
					return ItemIndex;
			}
		}
		return -1;
	}

public:
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
	void Popup(WObj *pDestWin, int x, int y, int xSize, int ySize) {
		if (!pDestWin)
			return;
		States |= MENU_SF_POPUP;
		Width = xSize > 0 ? xSize : 0;
		Height = ySize > 0 ? ySize : 0;
		auto Pos = pDestWin->LeftTop();
		SetOwner(pDestWin);
		Attach(WObj::GetDesktopWindow(), Pos);
		_SendMenuMessage(pDestWin, this, MENU_ON_OPEN, 0);
	}

public:

#pragma region Properties
	auto Font() const { return Props.pFont; }
	void Font(PCFONT pFont) {
		if (pFont == Props.pFont)
			return;
		Props.pFont = pFont;
		_RecalcTextWidthOfItems();
		_ResizeMenu();
	}

	auto Brush(MENU_CI ColorIndex) const { return Props.aBrush[ColorIndex]; }
	void Brush(MENU_CI ColorIndex, BRUSH brush) {
		if (Props.aBrush[ColorIndex] == brush)
			return;
		Props.aBrush[ColorIndex] = brush;
		Invalidate();
	}

	auto Border() const { return Props.Border; }
	void Border(RECT Border) {
		if (Props.Border == Border)
			return;
		Props.Border = Border;
		_ResizeMenu();
	}
#pragma endregion

	auto GetNumItems() const { return items.NumItems(); }
	void AddItem(const ItemData *pItemData) {
		if (!pItemData)
			return;
		if (items.AddItem() != 0)
			return;
		auto Index = GetNumItems() - 1;
		if (_SetItem(Index, pItemData))
			_ResizeMenu();
		else
			items.Delete(Index);
	}
	void DeleteItem(uint16_t ItemId) {
		Menu *pMenu;
		auto Index = _FindItem(ItemId, &pMenu);
		if (Index < 0)
			return;
		GUI_MEM_FreePtr((void **)&pMenu->items[Index].pText);
		items.Delete(Index);
		_ResizeMenu();
	}
	void InsertItem(uint16_t ItemId, const ItemData *pItemData) {
		if (!pItemData)
			return;
		Menu *pMenu;
		auto Index = _FindItem(ItemId, &pMenu);
		if (Index < 0)
			return;
		items.Insert(Index);
		if (_SetItem(Index, pItemData))
			_ResizeMenu();
		else
			items.Delete(Index);
	}
	void GetItem(uint16_t ItemId, ItemData *pItemData) {
		if (!pItemData)
			return;
		Menu *pMenu;
		auto Index = _FindItem(ItemId, &pMenu);
		if (Index < 0)
			return;
		auto &item = items[Index];
		pItemData->Flags = item.Flags;
		pItemData->Id = item.Id;
		pItemData->pSubmenu = item.pSubmenu;
		pItemData->pText = nullptr;
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
	void EnableItem(uint16_t ItemId, bool bEnabled) {
		Menu *pMenu;
		auto Index = _FindItem(ItemId, &pMenu);
		if (Index < 0)
			return;
		_SetItemFlags(Index, MENU_IF_DISABLED, bEnabled ? 0 : MENU_IF_DISABLED);
		_InvalidateItem(Index);
	}
};

Menu::Properties Menu::DefaultProps;

}
