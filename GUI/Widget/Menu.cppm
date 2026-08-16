module;

#include <string.h>

#include "WM_Intern.h"
#include "GUI_Protected.h"

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

constexpr uint16_t
	MENU_CF_HORIZONTAL               = 0 << 0,
	MENU_CF_VERTICAL                 = 1 << 0,
	MENU_CF_OPEN_ON_POINTEROVER      = 1 << 1,
	MENU_CF_CLOSE_ON_SECOND_CLICK    = 1 << 2,
	MENU_CF_HIDE_DISABLED_SEL        = 1 << 3;  /* Hides the selection when a disabled item is selected */

constexpr uint16_t
	MENU_IF_DISABLED           = 1 << 0,
	MENU_IF_SEPARATOR          = 1 << 1;

constexpr uint16_t MENU_ON_ITEMSELECT        = 0;   /* Send to owner when selecting a menu item */
constexpr uint16_t MENU_ON_INITMENU          = 1;   /* Send to owner when for the first time selecting a submenu */
constexpr uint16_t MENU_ON_INITSUBMENU       = 2;   /* Send to owner when selecting a submenu */
constexpr uint16_t MENU_ON_OPEN              = 3;   /* Internal message of menu widget (send to submenus) */
constexpr uint16_t MENU_ON_CLOSE             = 4;   /* Internal message of menu widget (send to submenus) */
constexpr uint16_t MENU_IS_MENU              = 5;   /* Internal message of menu widget. Owner must call   */

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

class Menu : public WIDGET {
	
public:
	struct ItemData {
		const char *pText;
		uint16_t    Id;
		uint16_t    Flags;
		Menu *pSubmenu;
	};

public:
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
	
private:
	Properties Props;

	struct Item {
		Menu *pSubmenu;
		uint16_t Id;
		uint16_t Flags;
		uint16_t TextWidth;
		char *pText;
	};
	ARRAY<Item> ItemArray;
	WObj *pOwner;
	uint16_t Flags;
	char IsSubmenuActive;
	uint16_t Width;
	uint16_t Height;
	uint16_t Sel;

	struct MsgPack {
		uint32_t MsgType : 16;
		uint32_t ItemId : 16;
	};

	static int _SendMenuMessage(WObj *pSrcWin, WObj *pDestWin, uint16_t MsgType, uint16_t ItemId) {
		if (!pDestWin)
			pDestWin = pSrcWin->Parent();
		if (pDestWin) {
			MsgPack MsgData;
			MsgData.MsgType = MsgType;
			MsgData.ItemId = ItemId;
			return (int)WM__SendMessage(pDestWin, WM_MENU, *(WM_PARAM *)&MsgData);
		}
		return 0;
	}
	void _InvalidateItem(unsigned Index) {
		Invalidate();  /* Can be optimized, no need to invalidate all items */
	}
	char _IsTopLevelMenu() {
		if (_SendMenuMessage(this, this->pOwner, MENU_IS_MENU, 0) == 0) {
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
			r = this->EffectSize();
		}
		return r;
	}
	int _CalcTextWidth(const char *sText) {
		int TextWidth = 0;
		if (sText) {
			PCFONT pOldFont;
			pOldFont = GUI.SetFont(Props.pFont);
			TextWidth = GUI_GetStringDistX(sText);
			GUI.SetFont(pOldFont);
		}
		return TextWidth;
	}
	int _GetItemWidth(unsigned Index) {
		int ItemWidth;
		if (this->Width && (this->Flags & MENU_SF_VERTICAL)) {
			ItemWidth = this->Width - (this->_GetEffectSize() << 1);
		}
		else {
			auto &pItem = this->ItemArray[Index];
				if ((this->Flags & MENU_SF_VERTICAL) || !(pItem.Flags & MENU_IF_SEPARATOR)) {
					ItemWidth = pItem.TextWidth;
			}
			else {
				ItemWidth = 3;
			}
			ItemWidth += Props.aBorder[MENU_BI_LEFT] + Props.aBorder[MENU_BI_RIGHT];
		}
		return ItemWidth;
	}
	int _GetItemHeight(unsigned Index) {
		int ItemHeight;
		if (this->Height && !(this->Flags & MENU_SF_VERTICAL)) {
			ItemHeight = this->Height - (this->_GetEffectSize() << 1);
		}
		else {
			ItemHeight = Props.pFont->DistY();
			if (this->Flags & MENU_SF_VERTICAL) {
				auto &pItem = this->ItemArray[Index];
					if (pItem.Flags & MENU_IF_SEPARATOR) {
					ItemHeight = 3;
				}
			}
			ItemHeight += Props.aBorder[MENU_BI_TOP] + Props.aBorder[MENU_BI_BOTTOM];
		}
		return ItemHeight;
	}
	unsigned _GetNumItems() {
		return ItemArray.NumItems();
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
			if (!HasCaptured()) {
				SetCapture(0);
			}
		}
	}
	void _ReleaseCapture() {
		if (HasCaptured()) {
			if (this->_IsTopLevelMenu() && !(this->Flags & MENU_SF_POPUP)) {
				ReleaseCapture();
			}
		}
	}
	void _CloseSubmenu() {
		if (this->Flags & MENU_SF_ACTIVE) {
			if (this->IsSubmenuActive) {
				auto &pItem = this->ItemArray[this->Sel];
					/* Inform submenu about its deactivation and detach it */
					_SendMenuMessage(this, pItem.pSubmenu, MENU_ON_CLOSE, 0);
					WM_DetachWindow(pItem.pSubmenu);
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
			char PrevActiveSubmenu;
			PrevActiveSubmenu = this->IsSubmenuActive;
			/* Close previous submenu (if needed) */
			this->_CloseSubmenu();
			auto &pItem = this->ItemArray[Index];
			if (pItem.pSubmenu) {
				if ((pItem.Flags & MENU_IF_DISABLED) == 0) {
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
					x += GetOrgX();
					y += GetOrgY();
					/*
					 * Notify owner window when for the first time open a menu (when no
					 * other submenu was open), so it can initialize the menu items.
					 */
					if (PrevActiveSubmenu == 0) {
						if (this->_IsTopLevelMenu()) {
							_SendMenuMessage(this, this->pOwner, MENU_ON_INITMENU, 0);
						}
					}
					/* Notify owner window when a submenu opens, so it can initialize the menu items. */
					_SendMenuMessage(this, this->pOwner, MENU_ON_INITSUBMENU, pItem.Id);
					/* Set active menu as owner of submenu. */
					pItem.pSubmenu->SetOwner(this);
					/* Attach submenu and inform it about its activation. */
					WM_AttachWindowAt(pItem.pSubmenu, WObj::GetDesktopWindow(), x, y);
					_SendMenuMessage(this, pItem.pSubmenu, MENU_ON_OPEN, 0);
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
			ReleaseCapture();
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
		auto &pItem = this->ItemArray[Index];
		if (!pItem.pSubmenu) {
			if ((pItem.Flags & (MENU_IF_DISABLED | MENU_IF_SEPARATOR)) == 0) {
				this->_ClosePopup();
				/* Send item select message to owner. */
				_SendMenuMessage(this, this->pOwner, MENU_ON_ITEMSELECT, pItem.Id);
			}
		}
	}
	void _ActivateMenu(unsigned Index) {
		if ((this->Flags & MENU_SF_OPEN_ON_POINTEROVER) == 0) {
			auto &pItem = this->ItemArray[Index];
				if (pItem.pSubmenu) {
					if ((pItem.Flags & MENU_IF_DISABLED) == 0) {
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
	int _ForwardMouseOverMsg(POINT Pos) {
#if (GUI_SUPPORT_MOUSE)
		if (!IsSubmenuActive && !(Flags & MENU_SF_POPUP)) {
			if (_IsTopLevelMenu()) {
				Pos += GetOrg();
				if (auto pBelow = WM_Screen2Win(Pos); pBelow && (pBelow != this)) {
					PID_STATE State;
					State = Pos - pBelow->GetOrg();
					State.Pressed = 0;
					WM__SendMessage(pBelow, WM_MOUSEOVER, (WM_PARAM)&State);
					return true;
				}
			}
		}
#endif
		return false;
	}
	char _HandlePID(int x, int y, int Pressed) {
		PID_STATE PrevState;
		char XYInWidget = 0;
		WM_PID__GetPrevState(&PrevState);
		/*
		 * Check if coordinates are inside the widget.
		 */
		if ((x >= 0) && (y >= 0)) {
			RECT r = GetClientRect();
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
						_ActivateMenu(ItemIndex);
					_SelectItem(ItemIndex);
				}
				else if (Pressed == 0 && PrevState.Pressed == 1) /* Released */
					_ActivateItem(ItemIndex);
				else if (Pressed < 0) {  /* Mouse moved */
					if (!_ForwardMouseOverMsg({x, y}))
						_SelectItem(ItemIndex);
					else
						_DeselectItem();
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
						_DeactivateMenu();
					_DeselectItem();
				}
				else if (Pressed < 0) /* Moved out or mouse moved */
					_DeselectItem();
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
				_DeactivateMenu();
				_ClosePopup();
			}
			_DeselectItem();
			_ForwardMouseOverMsg({ x, y });
		}
		return 1;   /* Coordinates are not in widget, we need to forward PID message to owner */
	}
	void _ForwardPIDMsgToOwner(int MsgId, const PID_STATE *pState) {
		if (!this->_IsTopLevelMenu()) {
			auto pOwner = this->pOwner ? this->pOwner : Parent();
			if (pOwner) {
				PID_STATE State;
				if (pState) {
					State = *pState;
					State += GetOrg() - pOwner->GetOrg();
					pState = &State;
				}
				WM__SendMessage(pOwner, MsgId, (WM_PARAM)pState);
			}
		}
	}
	void _ResizeMenu() {
		int xSize, ySize;
		xSize = this->_CalcWindowSizeX();
		ySize = this->_CalcWindowSizeY();
		WM_SetSize(this, xSize, ySize);
		Invalidate();
	}
	WM_PARAM _OnMenu(WM_PARAM Data) {
		auto msg = *(MsgPack *)&Data;
		switch (msg.MsgType) {
			case MENU_ON_ITEMSELECT:
				this->_DeactivateMenu();
				this->_DeselectItem();
				this->_ClosePopup();
				/* No break here. We need to forward message to owner. */
			case MENU_ON_INITMENU:
			case MENU_ON_INITSUBMENU: {
				/* Forward message to owner. */
				auto pOwner = this->pOwner ? this->pOwner : Parent();
				if (pOwner)
					WM__SendMessage(pOwner, WM_MENU, Data);
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
	char _OnTouch(const PID_STATE *pState) {
		if (pState) /* Something happened in our area (pressed or released) */
			return this->_HandlePID(pState->x, pState->y, pState->Pressed);
		return this->_HandlePID(-1, -1, -1); /* Moved out */
	}
#if (GUI_SUPPORT_MOUSE)
	char _OnMouseOver(const PID_STATE *pState) {
		if (pState)
			return this->_HandlePID(pState->x, pState->y, -1);
		return 0;
	}
#endif
	void _SetPaintColors(const Item &pItem, int ItemIndex) {
		char Selected;
		unsigned ColorIndex;
		Selected = (ItemIndex == this->Sel) ? 1 : 0;
		if (this->IsSubmenuActive && Selected) {
			ColorIndex = MENU_CI_ACTIVE_SUBMENU;
		}
		else if (pItem.Flags & MENU_IF_SEPARATOR) {
			ColorIndex = MENU_CI_ENABLED;
		}
		else {
			ColorIndex = (Selected) ? MENU_CI_SELECTED : MENU_CI_ENABLED;
			if (pItem.Flags & MENU_IF_DISABLED) {
				if (this->Flags & MENU_CF_HIDE_DISABLED_SEL) {
					ColorIndex = MENU_CI_DISABLED;
				}
				else {
					ColorIndex += MENU_CI_DISABLED;
				}
			}
		}
		GUI.SetBkColor(Props.aBkColor[ColorIndex]);
		GUI.SetColor(Props.aTextColor[ColorIndex]);
	}
	void _OnPaint() {
		RECT FillRect, TextRect;
		unsigned TextWidth, NumItems, i;
		uint8_t BorderLeft = Props.aBorder[MENU_BI_LEFT];
		uint8_t BorderTop = Props.aBorder[MENU_BI_TOP];
		int FontHeight = Props.pFont->DistY();
		int EffectSize = this->_GetEffectSize();
		NumItems = this->_GetNumItems();
		FillRect = GetClientRect();
		FillRect -= EffectSize;
		GUI.SetFont(Props.pFont);
		if (this->Flags & MENU_SF_VERTICAL) {
			int ItemHeight, xSize;
			xSize = this->_CalcMenuSizeX();
			FillRect.x1 = xSize - EffectSize - 1;
			TextRect.x0 = FillRect.x0 + BorderLeft;
			for (i = 0; i < NumItems; i++) {
					auto &pItem = this->ItemArray[i];
					ItemHeight = this->_GetItemHeight(i);
					this->_SetPaintColors(pItem, i);
					FillRect.y1 = FillRect.y0 + ItemHeight - 1;
					if (pItem.Flags & MENU_IF_SEPARATOR) {
						GUI_ClearRect(FillRect);
						GUI.SetColor(RGB_GRAYL(0x7C));
						GUI_DrawHLine(FillRect.y0 + BorderTop + 1, FillRect.x0 + 2, FillRect.x1 - 2);
					}
					else {
						TextWidth = pItem.TextWidth;
						TextRect.x1 = TextRect.x0 + TextWidth - 1;
						TextRect.y0 = FillRect.y0 + BorderTop;
						TextRect.y1 = TextRect.y0 + FontHeight - 1;
						WIDGET__FillStringInRect(pItem.pText, FillRect, TextRect, TextRect);
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
					auto &pItem = this->ItemArray[i];
					ItemWidth = this->_GetItemWidth(i);
					this->_SetPaintColors(pItem, i);
					FillRect.x1 = FillRect.x0 + ItemWidth - 1;
					if (pItem.Flags & MENU_IF_SEPARATOR) {
						GUI_ClearRect(FillRect);
						GUI.SetColor(RGB_GRAYL(0x7C));
						GUI_DrawVLine(FillRect.x0 + BorderLeft + 1, FillRect.y0 + 2, FillRect.y1 - 2);
					}
					else {
						TextWidth = pItem.TextWidth;
						TextRect.x0 = FillRect.x0 + BorderLeft;
						TextRect.x1 = TextRect.x0 + TextWidth - 1;
						WIDGET__FillStringInRect(pItem.pText, FillRect, TextRect, TextRect);
					}
					FillRect.x0 += ItemWidth;
				}
		}
		if (this->Width || this->Height) {
			RECT r = GetClientRect();
			r -= EffectSize;
			GUI.SetBkColor(Props.aBkColor[MENU_CI_ENABLED]);
			GUI_ClearRect({ FillRect.x1 + 1, EffectSize, r.x1, FillRect.y1 });
			GUI_ClearRect({ EffectSize, FillRect.y1 + 1, r.x1, r.y1 });
		}
		/* Draw 3D effect (if configured) */
		if (this->_HasEffect())
			this->pEffect->DrawUp();
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

	static Menu *Create(int x0, int y0, int xSize, int ySize, WObj *hParent, int WinFlags, int ExFlags, int Id) {
		/* Create the window */
		auto pObj = (Menu *)WM_CreateWindowAsChild(x0, y0, xSize, ySize, hParent,
												   WC_VISIBLE | WC_STAYONTOP | WinFlags,
												   Menu::_Callback, sizeof(Menu) - sizeof(WObj));
		if (!pObj) {
			GUI_DEBUG_ERROROUT_IF(pObj == 0, "Menu create failed");
			return nullptr;
		}
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE);
		/* init member variables */
		if (ExFlags & MENU_SF_OPEN_ON_POINTEROVER)
			ExFlags |= MENU_SF_ACTIVE;
		else
			ExFlags &= ~(MENU_SF_ACTIVE);
		pObj->Props = Menu::DefaultProps;
		pObj->Flags = ExFlags;
		pObj->Width = ((xSize > 0) ? xSize : 0);
		pObj->Height = ((ySize > 0) ? ySize : 0);
		pObj->Sel = -1;
		pObj->pOwner = 0;
		pObj->IsSubmenuActive = 0;
		pObj->SetEffect(MENU__pDefaultEffect);
		return pObj;
	}
	WIDGET *CreateIndirect(const WIDGET_CREATE_INFO *pCreateInfo, WObj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return Create(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							 hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
	}


private:

	void _RecalcTextWidthOfItems() {
		PCFONT pOldFont;
		unsigned i, NumItems;
		NumItems = this->_GetNumItems();
		pOldFont = GUI.SetFont(Props.pFont);
		for (i = 0; i < NumItems; i++) {
			auto &pItem = this->ItemArray[i];
			pItem.TextWidth = GUI_GetStringDistX(pItem.pText);
		}
		GUI.SetFont(pOldFont);
	}
	char _SetItem(unsigned Index, const ItemData *pItemData) {
		Item item = { 0 };
		const char *pText;
		pText = pItemData->pText;
		if (!pText) {
			pText = "";
		}
		item.Id = pItemData->Id;
		item.Flags = pItemData->Flags;
		item.pSubmenu = pItemData->pSubmenu;
		item.TextWidth = this->_CalcTextWidth(pText);
		if (item.Flags & MENU_IF_SEPARATOR) {
			item.pSubmenu = nullptr;   /* Ensures that no separator is a submenu */
		}
		if (Index < this->ItemArray.NumItems()) {
			auto &pItem = this->ItemArray[Index];
			GUI__SetText(&pItem.pText, pText);
			pItem.Id       = item.Id;
			pItem.Flags    = item.Flags;
			pItem.pSubmenu = item.pSubmenu;
			pItem.TextWidth= item.TextWidth;
			if (item.pSubmenu)
				pItem.pSubmenu->SetOwner(this);
			return 1;
		}
		return 0;
	}
	void _SetItemFlags(unsigned Index, uint16_t Mask, uint16_t Flags) {
		auto &pItem = this->ItemArray[Index];
		pItem.Flags &= ~Mask;
		pItem.Flags |= Flags;
	}
	int _FindItem(uint16_t ItemId, Menu **pMenu) {
		int ItemIndex = -1;
		unsigned NumItems, i;
		NumItems = this->_GetNumItems();
		for (i = 0; (i < NumItems) && (ItemIndex < 0); i++) {
			auto &pItem = this->ItemArray[i];
			if (pItem.Id == ItemId) {
				*pMenu = this;
				ItemIndex = i;
			}
			else if (pItem.pSubmenu) {
				ItemIndex = pItem.pSubmenu->_FindItem(ItemId, pMenu);
			}
		}

		return ItemIndex;
	}

public:

	void AddItem(const ItemData *pItemData) {
		if (pItemData) {
			if (this->ItemArray.AddItem() == 0) {
				unsigned Index;
				Index = this->_GetNumItems() - 1;
				if (this->_SetItem(Index, pItemData) == 0) {
					this->ItemArray.DeleteItem(Index);
				}
				else {
					this->_ResizeMenu();
				}
			}
		}
	}
	void SetOwner(WObj *pOwner) {
		this->pOwner = pOwner;
	}
	void Attach(WObj *pDestWin, int x, int y, int xSize, int ySize, int Flags) {
		GUI_USE_PARA(Flags);
		this->Width = ((xSize > 0) ? xSize : 0);
		this->Height = ((ySize > 0) ? ySize : 0);
		WM_AttachWindowAt(this, pDestWin, x, y);
		this->_ResizeMenu();
	}

	void DeleteItem(uint16_t ItemId) {
		Menu *pMenu;
		int Index = _FindItem(ItemId, &pMenu);
		if (Index >= 0) {
			GUI_ALLOC_FreePtr((void **)&pMenu->ItemArray[Index].pText);
			this->ItemArray.DeleteItem(Index);
			this->_ResizeMenu();
		}
	}
	void DisableItem(uint16_t ItemId) {
		Menu *pMenu;
		int Index = _FindItem(ItemId, &pMenu);
		if (Index >= 0) {
			this->_SetItemFlags(Index, MENU_IF_DISABLED, MENU_IF_DISABLED);
			this->_InvalidateItem(Index);
		}
	}
	void EnableItem(uint16_t ItemId) {
		Menu *pMenu;
		int Index = _FindItem(ItemId, &pMenu);
		if (Index >= 0) {
			this->_SetItemFlags(Index, MENU_IF_DISABLED, 0);
			this->_InvalidateItem(Index);
		}
	}
	void GetItem(uint16_t ItemId, ItemData *pItemData) {
		if (pItemData) {
			Menu *pMenu;
			int Index = _FindItem(ItemId, &pMenu);
			if (Index >= 0) {
				auto &pItem = this->ItemArray[Index];
				pItemData->Flags = pItem.Flags;
				pItemData->Id = pItem.Id;
				pItemData->pSubmenu = pItem.pSubmenu;
				pItemData->pText = nullptr;
			}
		}
	}
	void GetItemText(uint16_t ItemId, char *pBuffer, unsigned BufferSize) {
		if (pBuffer) {
			Menu *pMenu;
			int Index = _FindItem(ItemId, &pMenu);
			if (Index >= 0) {
				auto &pItem = this->ItemArray[Index];
				strncpy(pBuffer, pItem.pText, BufferSize);
				pBuffer[BufferSize - 1] = 0;
			}
		}
	}
	auto GetNumItems() {
		unsigned r = 0;
		r = this->_GetNumItems();
		return r;
	}
	void InsertItem(uint16_t ItemId, const ItemData *pItemData) {
		if (pItemData) {
			Menu *pMenu;
			int Index = _FindItem(ItemId, &pMenu);
			if (Index >= 0) {
				if (this->ItemArray.InsertBlankItem(Index) != 0) {
					if (this->_SetItem(Index, pItemData) == 0) {
						this->ItemArray.DeleteItem(Index);
					}
					else {
						this->_ResizeMenu();
					}
				}
			}
		}
	}
	void Popup(WObj *pDestWin, int x, int y, int xSize, int ySize, int Flags) {
		if (pDestWin) {
			this->Flags |= MENU_SF_POPUP;
			this->Width = ((xSize > 0) ? xSize : 0);
			this->Height = ((ySize > 0) ? ySize : 0);
			auto Pos = pDestWin->GetOrg();
			SetOwner(pDestWin);
			WM_AttachWindowAt(this, WObj::GetDesktopWindow(), Pos.x, Pos.y);
			_SendMenuMessage(pDestWin, this, MENU_ON_OPEN, 0);
		}
	}
	void SetBkColor(unsigned ColorIndex, RGBC Color) {
		if (ColorIndex < GUI_COUNTOF(Props.aBkColor)) {
			if (Color != Props.aBkColor[ColorIndex]) {
				Props.aBkColor[ColorIndex] = Color;
				Invalidate();
			}
		}
	}
	void SetBorderSize(unsigned BorderIndex, uint8_t BorderSize) {
		if (BorderIndex < GUI_COUNTOF(Props.aBorder)) {
			if (BorderSize != Props.aBorder[BorderIndex]) {
				Props.aBorder[BorderIndex] = BorderSize;
				this->_ResizeMenu();
			}
		}
	}
	void SetFont(PCFONT pFont) {
		if (pFont != Props.pFont) {
			Props.pFont = pFont;
			this->_RecalcTextWidthOfItems();
			this->_ResizeMenu();
		}
	}
	void SetItem(uint16_t ItemId, const ItemData *pItemData) {
		if (pItemData) {
			Menu *pMenu;
			int Index = _FindItem(ItemId, &pMenu);
			if (Index >= 0) {
				if (this->_SetItem(Index, pItemData) != 0) {
					this->_ResizeMenu();
				}
			}
		}
	}
	void SetTextColor(unsigned ColorIndex, RGBC Color) {
		if (ColorIndex < GUI_COUNTOF(Props.aTextColor)) {
			if (Color != Props.aTextColor[ColorIndex]) {
				Props.aTextColor[ColorIndex] = Color;
				Invalidate();
			}
		}
	}
};

Menu::Properties Menu::DefaultProps;

}
