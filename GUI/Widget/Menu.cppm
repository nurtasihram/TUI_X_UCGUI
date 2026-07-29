module;

#include <string.h>

#include "DIALOG_Intern.h"

export module TUX.Widget.Menu;

import TUX.Widget;

import TUX.Array;

#define MENU_SF_HORIZONTAL              MENU_CF_HORIZONTAL
#define MENU_SF_VERTICAL                MENU_CF_VERTICAL
#define MENU_SF_OPEN_ON_POINTEROVER     MENU_CF_OPEN_ON_POINTEROVER
#define MENU_SF_CLOSE_ON_SECOND_CLICK   MENU_CF_CLOSE_ON_SECOND_CLICK
#define MENU_SF_HIDE_DISABLED_SEL       MENU_CF_HIDE_DISABLED_SEL
#define MENU_SF_ACTIVE            (1<<6)  /* Internal flag only */
#define MENU_SF_POPUP             (1<<7)  /* Internal flag only */

/* Define default effect */
#define MENU_EFFECT_DEFAULT WIDGET_Effect_3D1L

export {
constexpr uint16_t MENU_CF_HORIZONTAL               = 0 << 0;
constexpr uint16_t MENU_CF_VERTICAL                 = 1 << 0;
constexpr uint16_t MENU_CF_OPEN_ON_POINTEROVER      = 1 << 1;
constexpr uint16_t MENU_CF_CLOSE_ON_SECOND_CLICK    = 1 << 2;
constexpr uint16_t MENU_CF_HIDE_DISABLED_SEL        = 1 << 3;  /* Hides the selection when a disabled item is selected */

constexpr uint16_t MENU_IF_DISABLED           = 1 << 0;
constexpr uint16_t MENU_IF_SEPARATOR          = 1 << 1;

enum MENU_CI {
	 MENU_CI_ENABLED           = 0,
	 MENU_CI_SELECTED          = 1,
	 MENU_CI_DISABLED          = 2,
	 MENU_CI_DISABLED_SEL      = 3,
	 MENU_CI_ACTIVE_SUBMENU    = 4
};

constexpr uint16_t MENU_BI_LEFT              = 0;
constexpr uint16_t MENU_BI_RIGHT             = 1;
constexpr uint16_t MENU_BI_TOP               = 2;
constexpr uint16_t MENU_BI_BOTTOM            = 3;

constexpr uint16_t MENU_ON_ITEMSELECT        = 0;   /* Send to owner when selecting a menu item */
constexpr uint16_t MENU_ON_INITMENU          = 1;   /* Send to owner when for the first time selecting a submenu */
constexpr uint16_t MENU_ON_INITSUBMENU       = 2;   /* Send to owner when selecting a submenu */
constexpr uint16_t MENU_ON_OPEN              = 3;   /* Internal message of menu widget (send to submenus) */
constexpr uint16_t MENU_ON_CLOSE             = 4;   /* Internal message of menu widget (send to submenus) */
constexpr uint16_t MENU_IS_MENU              = 5;   /* Internal message of menu widget. Owner must call   */
									  /* WM_DefaultProc() when not handle the message.      */

struct MENU_MSG_DATA {
	uint16_t MsgType;
	uint16_t ItemId;
};

struct MENU_ITEM_DATA {
	const char *pText;
	uint16_t    Id;
	uint16_t    Flags;
	struct MENU_Obj *pSubmenu;
};

struct MENU_ITEM {
	struct MENU_Obj *pSubmenu;
	uint16_t Id;
	uint16_t Flags;
	uint16_t TextWidth;
	char acText[1];
};

PCWIDGET_EFFECT MENU__pDefaultEffect = MENU_EFFECT_DEFAULT;

struct MENU_Obj : public WIDGET {
	struct Properties {
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
		PCFONT pFont{ &FontProp13_1 };
	} static DefaultProps;
	Properties Props;
	GUI_ARRAY   ItemArray;
	WM_Obj *hOwner;
	uint16_t Flags;
	char IsSubmenuActive;
	uint16_t Width;
	uint16_t Height;
	uint16_t Sel;

	static int _SendMenuMessage(WM_Obj *pSrcWin, WM_Obj *pDestWin, uint16_t MsgType, uint16_t ItemId) {
		if (!pDestWin)
			pDestWin = WM_GetParent(pSrcWin);
		if (pDestWin) {
			MENU_MSG_DATA MsgData;
			MsgData.MsgType = MsgType;
			MsgData.ItemId = ItemId;
			return (int)WM__SendMessage(pDestWin, WM_MENU, (WM_PARAM)&MsgData);
		}
		return 0;
	}
	void _InvalidateItem(unsigned Index) {
		WM_Invalidate(this);  /* Can be optimized, no need to invalidate all items */
	}
	char _IsTopLevelMenu() {
		if (_SendMenuMessage(this, this->hOwner, MENU_IS_MENU, 0) == 0) {
			return 1;
		}
		return 0;
	}
	int _HasEffect() {
		if (!(this->Flags & MENU_SF_POPUP)) {
			if (this->_IsTopLevelMenu()) {
				return 0;
			}
		}
		return 1;
	}
	int _GetEffectSize() {
		int r = 0;
		if (this->_HasEffect()) {
			r = this->pEffect->EffectSize;
		}
		return r;
	}
	int _CalcTextWidth(const char *sText) {
		int TextWidth = 0;
		if (sText) {
			PCFONT pOldFont;
			pOldFont = GUI_SetFont(this->Props.pFont);
			TextWidth = GUI_GetStringDistX(sText);
			GUI_SetFont(pOldFont);
		}
		return TextWidth;
	}
	int _GetItemWidth(unsigned Index) {
		int ItemWidth;
		if (this->Width && (this->Flags & MENU_SF_VERTICAL)) {
			ItemWidth = this->Width - (this->_GetEffectSize() << 1);
		}
		else {
			auto pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, Index);
			if ((this->Flags & MENU_SF_VERTICAL) || !(pItem->Flags & MENU_IF_SEPARATOR)) {
				ItemWidth = pItem->TextWidth;
			}
			else {
				ItemWidth = 3;
			}
			ItemWidth += this->Props.aBorder[MENU_BI_LEFT] + this->Props.aBorder[MENU_BI_RIGHT];
		}
		return ItemWidth;
	}
	int _GetItemHeight(unsigned Index) {
		int ItemHeight;
		if (this->Height && !(this->Flags & MENU_SF_VERTICAL)) {
			ItemHeight = this->Height - (this->_GetEffectSize() << 1);
		}
		else {
			ItemHeight = GUI_GetYDistOfFont(this->Props.pFont);
			if (this->Flags & MENU_SF_VERTICAL) {
				auto pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, Index);
				if (pItem->Flags & MENU_IF_SEPARATOR) {
					ItemHeight = 3;
				}
			}
			ItemHeight += this->Props.aBorder[MENU_BI_TOP] + this->Props.aBorder[MENU_BI_BOTTOM];
		}
		return ItemHeight;
	}
	unsigned _GetNumItems() {
		return GUI_ARRAY_GetNumItems(&this->ItemArray);
	}
	int _CalcMenuSizeX() {
		unsigned i, NumItems = this->_GetNumItems();
		int xSize = 0;
		if (this->Flags & MENU_SF_VERTICAL) {
			int ItemWidth;
			for (i = 0; i < NumItems; i++) {
				ItemWidth = this->_GetItemWidth(i);
				if (ItemWidth > xSize) {
					xSize = ItemWidth;
				}
			}
		}
		else {
			for (i = 0; i < NumItems; i++) {
				xSize += this->_GetItemWidth(i);
			}
		}
		xSize += (this->_GetEffectSize() << 1);
		return xSize;
	}
	int _CalcMenuSizeY() {
		unsigned i, NumItems = this->_GetNumItems();
		int ySize = 0;
		if (this->Flags & MENU_SF_VERTICAL) {
			for (i = 0; i < NumItems; i++) {
				ySize += this->_GetItemHeight(i);
			}
		}
		else {
			int ItemHeight;
			for (i = 0; i < NumItems; i++) {
				ItemHeight = this->_GetItemHeight(i);
				if (ItemHeight > ySize) {
					ySize = ItemHeight;
				}
			}
		}
		ySize += (this->_GetEffectSize() << 1);
		return ySize;
	}
	int _CalcWindowSizeX() {
		int xSize = this->Width;
		if (xSize == 0) {
			xSize = this->_CalcMenuSizeX();
		}
		return xSize;
	}
	int _CalcWindowSizeY() {
		int ySize = this->Height;
		if (ySize == 0) {
			ySize = this->_CalcMenuSizeY();
		}
		return ySize;
	}
	int _GetItemFromPos(int x, int y) {
		int xSize, ySize, EffectSize, r = -1;
		ySize = this->_CalcMenuSizeY();
		if ((this->Height) && (this->Height < ySize)) {
			ySize = this->Height;
		}
		xSize = this->_CalcMenuSizeX();
		if ((this->Width) && (this->Width < xSize)) {
			xSize = this->Width;
		}
		EffectSize = this->_GetEffectSize();
		x -= EffectSize;
		y -= EffectSize;
		xSize -= (EffectSize << 1);
		ySize -= (EffectSize << 1);
		if ((x >= 0) && (y >= 0) && (x < xSize) && (y < ySize)) {
			unsigned i, NumItems = this->_GetNumItems();
			if (this->Flags & MENU_SF_VERTICAL) {
				int yPos = 0;
				for (i = 0; i < NumItems; i++) {
					yPos += this->_GetItemHeight(i);
					if (y < yPos) {
						r = i;
						break;
					}
				}
			}
			else {
				int xPos = 0;
				for (i = 0; i < NumItems; i++) {
					xPos += this->_GetItemWidth(i);
					if (x < xPos) {
						r = i;
						break;
					}
				}
			}
		}
		return r;
	}
	void _GetItemPos(unsigned Index, int *px, int *py) {
		int i, EffectSize;
		EffectSize = this->_GetEffectSize();
		if (this->Flags & MENU_SF_VERTICAL) {
			int yPos = 0;
			for (i = 0; i < (int)Index; i++) {
				yPos += this->_GetItemHeight(i);
			}
			*px = EffectSize;
			*py = EffectSize + yPos;
		}
		else {
			int xPos = 0;
			for (i = 0; i < (int)Index; i++) {
				xPos += this->_GetItemWidth(i);
			}
			*px = EffectSize + xPos;
			*py = EffectSize;
		}
	}
	void _SetCapture() {
		if (this->IsSubmenuActive == 0) {
			if (WM_HasCaptured(this) == 0) {
				WM_SetCapture(this, 0);
			}
		}
	}
	void _ReleaseCapture() {
		if (WM_HasCaptured(this)) {
			if (this->_IsTopLevelMenu() && !(this->Flags & MENU_SF_POPUP)) {
				WM_ReleaseCapture();
			}
		}
	}
	void _CloseSubmenu() {
		if (this->Flags & MENU_SF_ACTIVE) {
			if (this->IsSubmenuActive) {
				auto pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, this->Sel);
				/* Inform submenu about its deactivation and detach it */
				_SendMenuMessage(this, pItem->pSubmenu, MENU_ON_CLOSE, 0);
				WM_DetachWindow(pItem->pSubmenu);
				this->IsSubmenuActive = 0;
				/*
				 * Keep capture in menu widget. The capture may only released
				 * by clicking outside the menu or when mouse moved out.
				 * And it may only released from a top level menu.
				 */
				this->_SetCapture();
				/* Invalidate menu item. This is needed because the appearance may have changed */
				this->_InvalidateItem(this->Sel);
			}
		}
	}
	void _OpenSubmenu(unsigned Index) {
		if (this->Flags & MENU_SF_ACTIVE) {
			MENU_ITEM *pItem;
			char PrevActiveSubmenu;
			PrevActiveSubmenu = this->IsSubmenuActive;
			/* Close previous submenu (if needed) */
			this->_CloseSubmenu();
			pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, Index);
			if (pItem->pSubmenu) {
				if ((pItem->Flags & MENU_IF_DISABLED) == 0) {
					int x, y, EffectSize;
					/* Calculate position of submenu */
					EffectSize = this->_GetEffectSize();
					this->_GetItemPos(Index, &x, &y);
					if (this->Flags & MENU_SF_VERTICAL) {
						x += this->_CalcMenuSizeX() - (this->_GetEffectSize() << 1);
						y -= EffectSize;
					}
					else {
						y += this->_CalcMenuSizeY() - (this->_GetEffectSize() << 1);
						x -= EffectSize;
					}
					x += WM_GetWindowOrgX(this);
					y += WM_GetWindowOrgY(this);
					/*
					 * Notify owner window when for the first time open a menu (when no
					 * other submenu was open), so it can initialize the menu items.
					 */
					if (PrevActiveSubmenu == 0) {
						if (this->_IsTopLevelMenu()) {
							_SendMenuMessage(this, this->hOwner, MENU_ON_INITMENU, 0);
						}
					}
					/* Notify owner window when a submenu opens, so it can initialize the menu items. */
					_SendMenuMessage(this, this->hOwner, MENU_ON_INITSUBMENU, pItem->Id);
					/* Set active menu as owner of submenu. */
					pItem->pSubmenu->SetOwner(this);
					/* Attach submenu and inform it about its activation. */
					WM_AttachWindowAt(pItem->pSubmenu, WM_GetDesktopWindow(), x, y);
					_SendMenuMessage(this, pItem->pSubmenu, MENU_ON_OPEN, 0);
					this->IsSubmenuActive = 1;
					/* Invalidate menu item. This is needed because the appearance may have changed. */
					this->_InvalidateItem(Index);
				}
			}
		}
	}
	void _ClosePopup() {
		if (this->Flags & MENU_SF_POPUP) {
			this->Flags &= ~(MENU_SF_POPUP);
			WM_DetachWindow(this);
			WM_ReleaseCapture();
		}
	}
	void _SetSelection(int Index) {
		if (Index != this->Sel) {
			this->_InvalidateItem(this->Sel); /* Invalidate previous selection */
			this->_InvalidateItem(Index);     /* Invalidate new selection */
			this->Sel = Index;
		}
	}
	void _SelectItem(unsigned Index) {
		if (this->Sel != (int)Index) {
			this->_SetCapture();
			this->_OpenSubmenu(Index);
			this->_SetSelection(Index);
		}
	}
	void _DeselectItem() {
		if (this->IsSubmenuActive == 0) {
			this->_SetSelection(-1);
			this->_ReleaseCapture();
		}
	}
	void _ActivateItem(unsigned Index) {
		MENU_ITEM *pItem;
		pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, Index);
		if (!pItem->pSubmenu) {
			if ((pItem->Flags & (MENU_IF_DISABLED | MENU_IF_SEPARATOR)) == 0) {
				this->_ClosePopup();
				/* Send item select message to owner. */
				_SendMenuMessage(this, this->hOwner, MENU_ON_ITEMSELECT, pItem->Id);
			}
		}
	}
	void _ActivateMenu(unsigned Index) {
		if ((this->Flags & MENU_SF_OPEN_ON_POINTEROVER) == 0) {
			auto pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, Index);
			if (pItem->pSubmenu) {
				if ((pItem->Flags & MENU_IF_DISABLED) == 0) {
					if ((this->Flags & MENU_SF_ACTIVE) == 0) {
						this->Flags |= MENU_SF_ACTIVE;
						this->_OpenSubmenu(Index);
						this->_SetSelection(Index);
					}
					else if (this->Flags & MENU_SF_CLOSE_ON_SECOND_CLICK) {
						if ((int)Index == this->Sel) {
							this->_CloseSubmenu();
							this->Flags &= ~MENU_SF_ACTIVE;
						}
					}
				}
			}
		}
	}
	void _DeactivateMenu() {
		this->_CloseSubmenu();
		if ((this->Flags & MENU_SF_OPEN_ON_POINTEROVER) == 0) {
			this->Flags &= ~MENU_SF_ACTIVE;
		}
	}
	int _ForwardMouseOverMsg(int x, int y) {
#if (GUI_SUPPORT_MOUSE)
		if ((this->IsSubmenuActive == 0) && !(this->Flags & MENU_SF_POPUP)) {
			if (this->_IsTopLevelMenu()) {
				WM_Obj *hBelow;
				x += WM_GetWindowOrgX(this);
				y += WM_GetWindowOrgY(this);
				hBelow = WM_Screen2hWin(x, y);
				if (hBelow && (hBelow != this)) {
					GUI_PID_STATE State;
					x -= WM_GetWindowOrgX(hBelow);
					y -= WM_GetWindowOrgY(hBelow);
					State.Pressed = 0;
					State.x = x;
					State.y = y;
					WM__SendMessage(hBelow, WM_MOUSEOVER, (WM_PARAM)&State);
					return 1;
				}
			}
		}
#endif
		return 0;
	}
	char _HandlePID(int x, int y, int Pressed) {
		GUI_PID_STATE PrevState;
		char XYInWidget = 0;
		WM_PID__GetPrevState(&PrevState);
		/*
		 * Check if coordinates are inside the widget.
		 */
		if ((x >= 0) && (y >= 0)) {
			GUI_RECT r = WM_GetClientRect(this);
			if (x <= r.x1 && y <= r.y1)
				XYInWidget = 1;
		}
		if (XYInWidget) {
			int ItemIndex = this->_GetItemFromPos(x, y);
			/*
			 * Handle PID when coordinates are inside the widget.
			 */
			if (ItemIndex >= 0) {
				/*
				 * Coordinates are inside the menu.
				 */
				if (Pressed == 1) {
					if (PrevState.Pressed == 0) /* Clicked */
						this->_ActivateMenu(ItemIndex);
					this->_SelectItem(ItemIndex);
				}
				else if (Pressed == 0 && PrevState.Pressed == 1) /* Released */
					this->_ActivateItem(ItemIndex);
				else if (Pressed < 0) {  /* Mouse moved */
					if (this->_ForwardMouseOverMsg(x, y) == 0)
						this->_SelectItem(ItemIndex);
					else
						this->_DeselectItem();
				}
			}
			else {
				/*
				 * Coordinates are outside the menu but inside the widget.
				 */
				if (Pressed == 1) {
					if (PrevState.Pressed == 0) /* Clicked */
						/*
						 * User has clicked outside the menu. Close the active submenu.
						 * The widget itself must be closed (if needed) by the owner.
						 */
						this->_DeactivateMenu();
					this->_DeselectItem();
				}
				else if (Pressed < 0) /* Moved out or mouse moved */
					this->_DeselectItem();
			}
			return 0;
		}
		else {
			/*
			 * Handle PID when coordinates are outside the widget.
			 */
			if (Pressed == 1 && PrevState.Pressed == 0) {
				/*
				 * User has clicked outside the menu. Close the active submenu.
				 * The widget itself must be closed (if needed) by the owner.
				 */
				this->_DeactivateMenu();
				this->_ClosePopup();
			}
			this->_DeselectItem();
			this->_ForwardMouseOverMsg(x, y);
		}
		return 1;   /* Coordinates are not in widget, we need to forward PID message to owner */
	}
	void _ForwardPIDMsgToOwner(int MsgId, const GUI_PID_STATE *pState) {
		if (!this->_IsTopLevelMenu()) {
			auto hOwner = this->hOwner ? this->hOwner : WM_GetParent(this);
			if (hOwner) {
				GUI_PID_STATE State;
				if (pState) {
					State = *pState;
					State.x += WM_GetWindowOrgX(this) - WM_GetWindowOrgX(hOwner);
					State.y += WM_GetWindowOrgY(this) - WM_GetWindowOrgY(hOwner);
					pState = &State;
				}
				WM__SendMessage(hOwner, MsgId, (WM_PARAM)pState);
			}
		}
	}
	void _ResizeMenu() {
		int xSize, ySize;
		xSize = this->_CalcWindowSizeX();
		ySize = this->_CalcWindowSizeY();
		WM_SetSize(this, xSize, ySize);
		WM_Invalidate(this);
	}
	WM_PARAM _OnMenu(WM_PARAM Data) {
		auto pData = (const MENU_MSG_DATA *)Data;
		if (!pData)
			return 0;
		switch (pData->MsgType) {
			case MENU_ON_ITEMSELECT:
				this->_DeactivateMenu();
				this->_DeselectItem();
				this->_ClosePopup();
				/* No break here. We need to forward message to owner. */
			case MENU_ON_INITMENU:
			case MENU_ON_INITSUBMENU: {
				/* Forward message to owner. */
				auto hOwner = this->hOwner ? this->hOwner : WM_GetParent(this);
				if (hOwner)
					WM__SendMessage(hOwner, WM_MENU, Data);
				break;
			}
			case MENU_ON_OPEN:
				this->Sel = -1;
				this->IsSubmenuActive = 0;
				this->Flags |= MENU_SF_ACTIVE | MENU_SF_OPEN_ON_POINTEROVER;
				this->_SetCapture();
				this->_ResizeMenu();
				break;
			case MENU_ON_CLOSE:
				this->_CloseSubmenu();
				break;
			case MENU_IS_MENU:
				return 1;
		}
		return 0;
	}
	char _OnTouch(const GUI_PID_STATE *pState) {
		if (pState) /* Something happened in our area (pressed or released) */
			return this->_HandlePID(pState->x, pState->y, pState->Pressed);
		return this->_HandlePID(-1, -1, -1); /* Moved out */
	}
#if (GUI_SUPPORT_MOUSE)
	char _OnMouseOver(const GUI_PID_STATE *pState) {
		if (pState)
			return this->_HandlePID(pState->x, pState->y, -1);
		return 0;
	}
#endif
	void _SetPaintColors(const MENU_ITEM *pItem, int ItemIndex) {
		char Selected;
		unsigned ColorIndex;
		Selected = (ItemIndex == this->Sel) ? 1 : 0;
		if (this->IsSubmenuActive && Selected) {
			ColorIndex = MENU_CI_ACTIVE_SUBMENU;
		}
		else if (pItem->Flags & MENU_IF_SEPARATOR) {
			ColorIndex = MENU_CI_ENABLED;
		}
		else {
			ColorIndex = (Selected) ? MENU_CI_SELECTED : MENU_CI_ENABLED;
			if (pItem->Flags & MENU_IF_DISABLED) {
				if (this->Flags & MENU_CF_HIDE_DISABLED_SEL) {
					ColorIndex = MENU_CI_DISABLED;
				}
				else {
					ColorIndex += MENU_CI_DISABLED;
				}
			}
		}
		GUI_SetBkColor(this->Props.aBkColor[ColorIndex]);
		GUI_SetColor(this->Props.aTextColor[ColorIndex]);
	}
	void _OnPaint() {
		GUI_RECT FillRect, TextRect;
		MENU_ITEM *pItem;
		unsigned TextWidth, NumItems, i;
		uint8_t BorderLeft = this->Props.aBorder[MENU_BI_LEFT];
		uint8_t BorderTop = this->Props.aBorder[MENU_BI_TOP];
		int FontHeight = GUI_GetYDistOfFont(this->Props.pFont);
		int EffectSize = this->_GetEffectSize();
		NumItems = this->_GetNumItems();
		FillRect = WM_GetClientRect(this);
		FillRect -= EffectSize;
		GUI_SetFont(this->Props.pFont);
		if (this->Flags & MENU_SF_VERTICAL) {
			int ItemHeight, xSize;
			xSize = this->_CalcMenuSizeX();
			FillRect.x1 = xSize - EffectSize - 1;
			TextRect.x0 = FillRect.x0 + BorderLeft;
			for (i = 0; i < NumItems; i++) {
				pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, i);
				ItemHeight = this->_GetItemHeight(i);
				this->_SetPaintColors(pItem, i);
				FillRect.y1 = FillRect.y0 + ItemHeight - 1;
				if (pItem->Flags & MENU_IF_SEPARATOR) {
					GUI_ClearRect(FillRect);
					GUI_SetColor(RGB_GRAYL(0x7C));
					GUI_DrawHLine(FillRect.y0 + BorderTop + 1, FillRect.x0 + 2, FillRect.x1 - 2);
				}
				else {
					TextWidth = pItem->TextWidth;
					TextRect.x1 = TextRect.x0 + TextWidth - 1;
					TextRect.y0 = FillRect.y0 + BorderTop;
					TextRect.y1 = TextRect.y0 + FontHeight - 1;
					WIDGET__FillStringInRect(pItem->acText, FillRect, TextRect, TextRect);
				}
				FillRect.y0 += ItemHeight;
			}
		}
		else {
			int ItemWidth, ySize;
			ySize = this->_CalcMenuSizeY();
			FillRect.y1 = ySize - EffectSize - 1;
			TextRect.y0 = FillRect.y0 + BorderTop;
			TextRect.y1 = TextRect.y0 + FontHeight - 1;
			for (i = 0; i < NumItems; i++) {
				pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, i);
				ItemWidth = this->_GetItemWidth(i);
				this->_SetPaintColors(pItem, i);
				FillRect.x1 = FillRect.x0 + ItemWidth - 1;
				if (pItem->Flags & MENU_IF_SEPARATOR) {
					GUI_ClearRect(FillRect);
					GUI_SetColor(RGB_GRAYL(0x7C));
					GUI_DrawVLine(FillRect.x0 + BorderLeft + 1, FillRect.y0 + 2, FillRect.y1 - 2);
				}
				else {
					TextWidth = pItem->TextWidth;
					TextRect.x0 = FillRect.x0 + BorderLeft;
					TextRect.x1 = TextRect.x0 + TextWidth - 1;
					WIDGET__FillStringInRect(pItem->acText, FillRect, TextRect, TextRect);
				}
				FillRect.x0 += ItemWidth;
			}
		}
		if (this->Width || this->Height) {
			GUI_RECT r = WM_GetClientRect(this);
			r -= EffectSize;
			GUI_SetBkColor(this->Props.aBkColor[MENU_CI_ENABLED]);
			GUI_ClearRect({ FillRect.x1 + 1, EffectSize, r.x1, FillRect.y1 });
			GUI_ClearRect({ EffectSize, FillRect.y1 + 1, r.x1, r.y1 });
		}
		/* Draw 3D effect (if configured) */
		if (this->_HasEffect())
			this->pEffect->DrawUp();
	}

	static WM_PARAM _Callback(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (MENU_Obj *)hWin;
		if (MsgId != WM_PID_STATE_CHANGED)
			/* Let widget handle the standard messages */
			if (!WIDGET_HandleActive(pObj, MsgId, &Data))
				return Data;
		switch (MsgId) {
			case WM_MENU:
				return pObj->_OnMenu(Data);
			case WM_TOUCH:
				if (pObj->_OnTouch((const GUI_PID_STATE *)Data))
					pObj->_ForwardPIDMsgToOwner(WM_TOUCH, (const GUI_PID_STATE *)Data);
				break;
#if (GUI_SUPPORT_MOUSE)
			case WM_MOUSEOVER:
				if (pObj->_OnMouseOver((const GUI_PID_STATE *)Data))
					pObj->_ForwardPIDMsgToOwner(WM_MOUSEOVER, (const GUI_PID_STATE *)Data);
				break;
#endif
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_DELETE:
				GUI_ARRAY_Delete(&pObj->ItemArray);
				break; /* No return here ... WM_DefaultProc needs to be called */
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:

	void _RecalcTextWidthOfItems() {
		PCFONT pOldFont;
		MENU_ITEM *pItem;
		unsigned i, NumItems;
		NumItems = this->_GetNumItems();
		pOldFont = GUI_SetFont(this->Props.pFont);
		for (i = 0; i < NumItems; i++) {
			pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, i);
			pItem->TextWidth = GUI_GetStringDistX(pItem->acText);
		}
		GUI_SetFont(pOldFont);
	}
	char _SetItem(unsigned Index, const MENU_ITEM_DATA *pItemData) {
		MENU_ITEM Item = { 0 };
		const char *pText;
		pText = pItemData->pText;
		if (!pText) {
			pText = "";
		}
		Item.Id = pItemData->Id;
		Item.Flags = pItemData->Flags;
		Item.pSubmenu = pItemData->pSubmenu;
		Item.TextWidth = this->_CalcTextWidth(pText);
		if (Item.Flags & MENU_IF_SEPARATOR) {
			Item.pSubmenu = nullptr;   /* Ensures that no separator is a submenu */
		}
		if (GUI_ARRAY_SetItem(&this->ItemArray, Index, &Item, sizeof(MENU_ITEM) + GUI__strlen(pText)) != 0) {
			auto pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, Index);
			GUI__strcpy(pItem->acText, pText);
			if (Item.pSubmenu)
				pItem->pSubmenu->SetOwner(this);
			return 1;
		}
		return 0;
	}
	void _SetItemFlags(unsigned Index, uint16_t Mask, uint16_t Flags) {
		auto pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, Index);
		pItem->Flags &= ~Mask;
		pItem->Flags |= Flags;
	}
	int _FindItem(uint16_t ItemId, MENU_Obj **pMenu) {
		int ItemIndex = -1;
		MENU_ITEM *pItem;
		unsigned NumItems, i;
		NumItems = this->_GetNumItems();
		for (i = 0; (i < NumItems) && (ItemIndex < 0); i++) {
			pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, i);
			if (pItem->Id == ItemId) {
				*pMenu = this;
				ItemIndex = i;
			}
			else if (pItem->pSubmenu) {
				ItemIndex = pItem->pSubmenu->_FindItem(ItemId, pMenu);
			}
		}

		return ItemIndex;
	}

public:

	void AddItem(const MENU_ITEM_DATA *pItemData) {
		if (pItemData) {
			if (GUI_ARRAY_AddItem(&this->ItemArray, nullptr, 0) == 0) {
				unsigned Index;
				Index = this->_GetNumItems() - 1;
				if (this->_SetItem(Index, pItemData) == 0) {
					GUI_ARRAY_DeleteItem(&this->ItemArray, Index);
				}
				else {
					this->_ResizeMenu();
				}
			}
		}
	}
	void SetOwner(WM_Obj *hOwner) {
		this->hOwner = hOwner;
	}
	void Attach(WM_Obj *hDestWin, int x, int y, int xSize, int ySize, int Flags) {
		GUI_USE_PARA(Flags);
		this->Width = ((xSize > 0) ? xSize : 0);
		this->Height = ((ySize > 0) ? ySize : 0);
		WM_AttachWindowAt(this, hDestWin, x, y);
		this->_ResizeMenu();
	}

	void DeleteItem(uint16_t ItemId) {
		MENU_Obj *pMenu;
		int Index = _FindItem(ItemId, &pMenu);
		if (Index >= 0) {
			GUI_ARRAY_DeleteItem(&this->ItemArray, Index);
			this->_ResizeMenu();
		}
	}
	void DisableItem(uint16_t ItemId) {
		MENU_Obj *pMenu;
		int Index = _FindItem(ItemId, &pMenu);
		if (Index >= 0) {
			this->_SetItemFlags(Index, MENU_IF_DISABLED, MENU_IF_DISABLED);
			this->_InvalidateItem(Index);
		}
	}
	void EnableItem(uint16_t ItemId) {
		MENU_Obj *pMenu;
		int Index = _FindItem(ItemId, &pMenu);
		if (Index >= 0) {
			this->_SetItemFlags(Index, MENU_IF_DISABLED, 0);
			this->_InvalidateItem(Index);
		}
	}
	void GetItem(uint16_t ItemId, MENU_ITEM_DATA *pItemData) {
		if (pItemData) {
			MENU_Obj *pMenu;
			int Index = _FindItem(ItemId, &pMenu);
			if (Index >= 0) {
				auto pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, Index);
				pItemData->Flags = pItem->Flags;
				pItemData->Id = pItem->Id;
				pItemData->pSubmenu = pItem->pSubmenu;
				pItemData->pText = nullptr;
			}
		}
	}
	void GetItemText(uint16_t ItemId, char *pBuffer, unsigned BufferSize) {
		if (pBuffer) {
			MENU_Obj *pMenu;
			int Index = _FindItem(ItemId, &pMenu);
			if (Index >= 0) {
				auto pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&this->ItemArray, Index);
				strncpy(pBuffer, pItem->acText, BufferSize);
				pBuffer[BufferSize - 1] = 0;
			}
		}
	}
	auto GetNumItems() {
		unsigned r = 0;
		r = this->_GetNumItems();
		return r;
	}
	void InsertItem(uint16_t ItemId, const MENU_ITEM_DATA *pItemData) {
		if (pItemData) {
			MENU_Obj *pMenu;
			int Index = _FindItem(ItemId, &pMenu);
			if (Index >= 0) {
				if (GUI_ARRAY_InsertBlankItem(&this->ItemArray, Index) != 0) {
					if (this->_SetItem(Index, pItemData) == 0) {
						GUI_ARRAY_DeleteItem(&this->ItemArray, Index);
					}
					else {
						this->_ResizeMenu();
					}
				}
			}
		}
	}
	void Popup(WM_Obj *hDestWin, int x, int y, int xSize, int ySize, int Flags) {
		GUI_USE_PARA(Flags);
		if (hDestWin) {
			this->Flags |= MENU_SF_POPUP;
			this->Width = ((xSize > 0) ? xSize : 0);
			this->Height = ((ySize > 0) ? ySize : 0);
			x += WM_GetWindowOrgX(hDestWin);
			y += WM_GetWindowOrgY(hDestWin);
			SetOwner(hDestWin);
			WM_AttachWindowAt(this, WM_GetDesktopWindow(), x, y);
			_SendMenuMessage(hDestWin, this, MENU_ON_OPEN, 0);
		}
	}
	void SetBkColor(unsigned ColorIndex, RGBC Color) {
		if (ColorIndex < GUI_COUNTOF(this->Props.aBkColor)) {
			if (Color != this->Props.aBkColor[ColorIndex]) {
				this->Props.aBkColor[ColorIndex] = Color;
				WM_Invalidate(this);
			}
		}
	}
	void SetBorderSize(unsigned BorderIndex, uint8_t BorderSize) {
		if (BorderIndex < GUI_COUNTOF(this->Props.aBorder)) {
			if (BorderSize != this->Props.aBorder[BorderIndex]) {
				this->Props.aBorder[BorderIndex] = BorderSize;
				this->_ResizeMenu();
			}
		}
	}
	void SetFont(PCFONT pFont) {
		if (pFont != this->Props.pFont) {
			this->Props.pFont = pFont;
			this->_RecalcTextWidthOfItems();
			this->_ResizeMenu();
		}
	}
	void SetItem(uint16_t ItemId, const MENU_ITEM_DATA *pItemData) {
		if (pItemData) {
			MENU_Obj *pMenu;
			int Index = _FindItem(ItemId, &pMenu);
			if (Index >= 0) {
				if (this->_SetItem(Index, pItemData) != 0) {
					this->_ResizeMenu();
				}
			}
		}
	}
	void SetTextColor(unsigned ColorIndex, RGBC Color) {
		if (ColorIndex < GUI_COUNTOF(this->Props.aTextColor)) {
			if (Color != this->Props.aTextColor[ColorIndex]) {
				this->Props.aTextColor[ColorIndex] = Color;
				WM_Invalidate(this);
			}
		}
	}
};

MENU_Obj::Properties MENU_Obj::DefaultProps;

MENU_Obj *MENU_CreateEx(int x0, int y0, int xSize, int ySize, WM_Obj *hParent, int WinFlags, int ExFlags, int Id) {
	/* Create the window */
	auto pObj = (MENU_Obj *)WM_CreateWindowAsChild(x0, y0, xSize, ySize, hParent,
												   WM_CF_SHOW | WM_CF_STAYONTOP | WinFlags,
												   MENU_Obj::_Callback, sizeof(MENU_Obj) - sizeof(WM_Obj));
	if (pObj) {
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE);
		/* init member variables */
		if (ExFlags & MENU_SF_OPEN_ON_POINTEROVER) {
			ExFlags |= MENU_SF_ACTIVE;
		}
		else {
			ExFlags &= ~(MENU_SF_ACTIVE);
		}
		pObj->Props = MENU_Obj::DefaultProps;
		pObj->Flags = ExFlags;
		pObj->Width = ((xSize > 0) ? xSize : 0);
		pObj->Height = ((ySize > 0) ? ySize : 0);
		pObj->Sel = -1;
		pObj->hOwner = 0;
		pObj->IsSubmenuActive = 0;
		WIDGET_SetEffect(pObj, MENU__pDefaultEffect);
	}
	else {
	}
	return pObj;
}
WM_Obj *MENU_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	return MENU_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
						  hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
}

}
