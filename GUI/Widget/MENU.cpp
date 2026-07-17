#include <string.h>

#include "GUIDebug.h"
#include "GUI_Protected.h"

#include "MENU.h"
#include "MENU_Private.h"

/* Define default font */
#define MENU_FONT_DEFAULT         &GUI_Font13_1
/* Define default effect */
#define MENU_EFFECT_DEFAULT       &WIDGET_Effect_3D1L
/* Define colors, index 0, enabled, not selected */
#define MENU_TEXTCOLOR0_DEFAULT   RGB_BLACK
#define MENU_BKCOLOR0_DEFAULT     RGB_LIGHTGRAY
/* Define colors, index 1, enabled, selected */
#define MENU_TEXTCOLOR1_DEFAULT   RGB_WHITE
#define MENU_BKCOLOR1_DEFAULT     RGB_BLUEL(0x98)
/* Define colors, index 2, disabled, not selected */
#define MENU_TEXTCOLOR2_DEFAULT   RGB_GRAYL(0x7C)
#define MENU_BKCOLOR2_DEFAULT     RGB_LIGHTGRAY
/* Define colors, index 3, disabled, selected */
#define MENU_TEXTCOLOR3_DEFAULT   RGB_LIGHTGRAY
#define MENU_BKCOLOR3_DEFAULT     RGB_BLUEL(0x98)
/* Define colors, index 4, active submenu */
#define MENU_TEXTCOLOR4_DEFAULT   RGB_WHITE
#define MENU_BKCOLOR4_DEFAULT     RGB_GRAYL(0x7C)
/* Define borders */
#define MENU_BORDER_LEFT_DEFAULT    4
#define MENU_BORDER_RIGHT_DEFAULT   4
#define MENU_BORDER_TOP_DEFAULT     2
#define MENU_BORDER_BOTTOM_DEFAULT  2
MENU_PROPS MENU__DefaultProps = {
  MENU_TEXTCOLOR0_DEFAULT,
  MENU_TEXTCOLOR1_DEFAULT,
  MENU_TEXTCOLOR2_DEFAULT,
  MENU_TEXTCOLOR3_DEFAULT,
  MENU_TEXTCOLOR4_DEFAULT,
  MENU_BKCOLOR0_DEFAULT,
  MENU_BKCOLOR1_DEFAULT,
  MENU_BKCOLOR2_DEFAULT,
  MENU_BKCOLOR3_DEFAULT,
  MENU_BKCOLOR4_DEFAULT,
  MENU_BORDER_LEFT_DEFAULT,
  MENU_BORDER_RIGHT_DEFAULT,
  MENU_BORDER_TOP_DEFAULT,
  MENU_BORDER_BOTTOM_DEFAULT,
  MENU_FONT_DEFAULT
};
const WIDGET_EFFECT *MENU__pDefaultEffect = MENU_EFFECT_DEFAULT;

static char _IsTopLevelMenu(MENU_Obj *pObj) {
	if (MENU__SendMenuMessage(pObj, pObj->hOwner, MENU_IS_MENU, 0) == 0) {
		return 1;
	}
	return 0;
}
static int _HasEffect(MENU_Obj *pObj) {
	if (!(pObj->Flags & MENU_SF_POPUP)) {
		if (_IsTopLevelMenu(pObj)) {
			return 0;
		}
	}
	return 1;
}
static int _GetEffectSize(MENU_Obj *pObj) {
	int r = 0;
	if (_HasEffect(pObj)) {
		r = pObj->Widget.pEffect->EffectSize;
	}
	return r;
}
static int _CalcTextWidth(MENU_Obj *pObj, const char  *sText) {
	int TextWidth = 0;
	if (sText) {
		const GUI_FONT  *pOldFont;
		pOldFont = GUI_SetFont(pObj->Props.pFont);
		TextWidth = GUI_GetStringDistX(sText);
		GUI_SetFont(pOldFont);
	}
	return TextWidth;
}
static int _GetItemWidth(MENU_Obj *pObj, unsigned Index) {
	int ItemWidth;
	if (pObj->Width && (pObj->Flags & MENU_SF_VERTICAL)) {
		ItemWidth = pObj->Width - (_GetEffectSize(pObj) << 1);
	}
	else {
		MENU_ITEM *pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&pObj->ItemArray, Index);
		if ((pObj->Flags & MENU_SF_VERTICAL) || !(pItem->Flags & MENU_IF_SEPARATOR)) {
			ItemWidth = pItem->TextWidth;
		}
		else {
			ItemWidth = 3;
		}
		ItemWidth += pObj->Props.aBorder[MENU_BI_LEFT] + pObj->Props.aBorder[MENU_BI_RIGHT];
	}
	return ItemWidth;
}
static int _GetItemHeight(MENU_Obj *pObj, unsigned Index) {
	int ItemHeight;
	if (pObj->Height && !(pObj->Flags & MENU_SF_VERTICAL)) {
		ItemHeight = pObj->Height - (_GetEffectSize(pObj) << 1);
	}
	else {
		ItemHeight = GUI_GetYDistOfFont(pObj->Props.pFont);
		if (pObj->Flags & MENU_SF_VERTICAL) {
			MENU_ITEM *pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&pObj->ItemArray, Index);
			if (pItem->Flags & MENU_IF_SEPARATOR) {
				ItemHeight = 3;
			}
		}
		ItemHeight += pObj->Props.aBorder[MENU_BI_TOP] + pObj->Props.aBorder[MENU_BI_BOTTOM];
	}
	return ItemHeight;
}
static int _CalcMenuSizeX(MENU_Obj *pObj) {
	unsigned i, NumItems = MENU__GetNumItems(pObj);
	int xSize = 0;
	if (pObj->Flags & MENU_SF_VERTICAL) {
		int ItemWidth;
		for (i = 0; i < NumItems; i++) {
			ItemWidth = _GetItemWidth(pObj, i);
			if (ItemWidth > xSize) {
				xSize = ItemWidth;
			}
		}
	}
	else {
		for (i = 0; i < NumItems; i++) {
			xSize += _GetItemWidth(pObj, i);
		}
	}
	xSize += (_GetEffectSize(pObj) << 1);
	return xSize;
}
static int _CalcMenuSizeY(MENU_Obj *pObj) {
	unsigned i, NumItems = MENU__GetNumItems(pObj);
	int ySize = 0;
	if (pObj->Flags & MENU_SF_VERTICAL) {
		for (i = 0; i < NumItems; i++) {
			ySize += _GetItemHeight(pObj, i);
		}
	}
	else {
		int ItemHeight;
		for (i = 0; i < NumItems; i++) {
			ItemHeight = _GetItemHeight(pObj, i);
			if (ItemHeight > ySize) {
				ySize = ItemHeight;
			}
		}
	}
	ySize += (_GetEffectSize(pObj) << 1);
	return ySize;
}
static int _CalcWindowSizeX(MENU_Obj *pObj) {
	int xSize = pObj->Width;
	if (xSize == 0) {
		xSize = _CalcMenuSizeX(pObj);
	}
	return xSize;
}
static int _CalcWindowSizeY(MENU_Obj *pObj) {
	int ySize = pObj->Height;
	if (ySize == 0) {
		ySize = _CalcMenuSizeY(pObj);
	}
	return ySize;
}
/*********************************************************************
*
*       _GetItemFromPos
*
* Return value:
*   Zero based index of item at given position or -1.
*
* NOTE:
*   This function has to ensure that index is always less than the
*   maximum number of items.
*/
static int _GetItemFromPos(MENU_Obj *pObj, int x, int y) {
	int xSize, ySize, EffectSize, r = -1;
	ySize = _CalcMenuSizeY(pObj);
	if ((pObj->Height) && (pObj->Height < ySize)) {
		ySize = pObj->Height;
	}
	xSize = _CalcMenuSizeX(pObj);
	if ((pObj->Width) && (pObj->Width < xSize)) {
		xSize = pObj->Width;
	}
	EffectSize = _GetEffectSize(pObj);
	x -= EffectSize;
	y -= EffectSize;
	xSize -= (EffectSize << 1);
	ySize -= (EffectSize << 1);
	if ((x >= 0) && (y >= 0) && (x < xSize) && (y < ySize)) {
		unsigned i, NumItems = MENU__GetNumItems(pObj);
		if (pObj->Flags & MENU_SF_VERTICAL) {
			int yPos = 0;
			for (i = 0; i < NumItems; i++) {
				yPos += _GetItemHeight(pObj, i);
				if (y < yPos) {
					r = i;
					break;
				}
			}
		}
		else {
			int xPos = 0;
			for (i = 0; i < NumItems; i++) {
				xPos += _GetItemWidth(pObj, i);
				if (x < xPos) {
					r = i;
					break;
				}
			}
		}
	}
	return r;
}
static void _GetItemPos(MENU_Obj *pObj, unsigned Index, int *px, int *py) {
	int i, EffectSize;
	EffectSize = _GetEffectSize(pObj);
	if (pObj->Flags & MENU_SF_VERTICAL) {
		int yPos = 0;
		for (i = 0; i < (int)Index; i++) {
			yPos += _GetItemHeight(pObj, i);
		}
		*px = EffectSize;
		*py = EffectSize + yPos;
	}
	else {
		int xPos = 0;
		for (i = 0; i < (int)Index; i++) {
			xPos += _GetItemWidth(pObj, i);
		}
		*px = EffectSize + xPos;
		*py = EffectSize;
	}
}
static void _SetCapture(MENU_Obj *pObj) {
	if (pObj->IsSubmenuActive == 0) {
		if (WM_HasCaptured(pObj) == 0) {
			WM_SetCapture(pObj, 0);
		}
	}
}
static void _ReleaseCapture(MENU_Obj *pObj) {
	if (WM_HasCaptured(pObj)) {
		if (_IsTopLevelMenu(pObj) && !(pObj->Flags & MENU_SF_POPUP)) {
			WM_ReleaseCapture();
		}
	}
}
static void _CloseSubmenu(MENU_Obj *pObj) {
	if (pObj->Flags & MENU_SF_ACTIVE) {
		if (pObj->IsSubmenuActive) {
			MENU_ITEM *pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&pObj->ItemArray, pObj->Sel);
			/* Inform submenu about its deactivation and detach it */
			MENU__SendMenuMessage(pObj, pItem->hSubmenu, MENU_ON_CLOSE, 0);
			WM_DetachWindow(pItem->hSubmenu);
			pObj->IsSubmenuActive = 0;
			/*
			 * Keep capture in menu widget. The capture may only released
			 * by clicking outside the menu or when mouse moved out.
			 * And it may only released from a top level menu.
			 */
			_SetCapture(pObj);
			/* Invalidate menu item. This is needed because the appearance may have changed */
			MENU__InvalidateItem(pObj, pObj->Sel);
		}
	}
}
static void _OpenSubmenu(MENU_Obj *pObj, unsigned Index) {
	if (pObj->Flags & MENU_SF_ACTIVE) {
		MENU_ITEM *pItem;
		char PrevActiveSubmenu;
		PrevActiveSubmenu = pObj->IsSubmenuActive;
		/* Close previous submenu (if needed) */
		_CloseSubmenu(pObj);
		pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&pObj->ItemArray, Index);
		if (pItem->hSubmenu) {
			if ((pItem->Flags & MENU_IF_DISABLED) == 0) {
				int x, y, EffectSize;
				/* Calculate position of submenu */
				EffectSize = _GetEffectSize(pObj);
				_GetItemPos(pObj, Index, &x, &y);
				if (pObj->Flags & MENU_SF_VERTICAL) {
					x += _CalcMenuSizeX(pObj) - (_GetEffectSize(pObj) << 1);
					y -= EffectSize;
				}
				else {
					y += _CalcMenuSizeY(pObj) - (_GetEffectSize(pObj) << 1);
					x -= EffectSize;
				}
				x += WM_GetWindowOrgX(pObj);
				y += WM_GetWindowOrgY(pObj);
				/*
				 * Notify owner window when for the first time open a menu (when no
				 * other submenu was open), so it can initialize the menu items.
				 */
				if (PrevActiveSubmenu == 0) {
					if (_IsTopLevelMenu(pObj)) {
						MENU__SendMenuMessage(pObj, pObj->hOwner, MENU_ON_INITMENU, 0);
					}
				}
				/* Notify owner window when a submenu opens, so it can initialize the menu items. */
				MENU__SendMenuMessage(pObj, pObj->hOwner, MENU_ON_INITSUBMENU, pItem->Id);
				/* Set active menu as owner of submenu. */
				MENU_SetOwner(pItem->hSubmenu, pObj);
				/* Attach submenu and inform it about its activation. */
				WM_AttachWindowAt(pItem->hSubmenu, WM_GetDesktopWindow(), x, y);
				MENU__SendMenuMessage(pObj, pItem->hSubmenu, MENU_ON_OPEN, 0);
				pObj->IsSubmenuActive = 1;
				/* Invalidate menu item. This is needed because the appearance may have changed. */
				MENU__InvalidateItem(pObj, Index);
			}
		}
	}
}
static void _ClosePopup(MENU_Obj *pObj) {
	if (pObj->Flags & MENU_SF_POPUP) {
		pObj->Flags &= ~(MENU_SF_POPUP);
		WM_DetachWindow(pObj);
		WM_ReleaseCapture();
	}
}
static void _SetSelection(MENU_Obj *pObj, int Index) {
	if (Index != pObj->Sel) {
		MENU__InvalidateItem(pObj, pObj->Sel); /* Invalidate previous selection */
		MENU__InvalidateItem(pObj, Index);     /* Invalidate new selection */
		pObj->Sel = Index;
	}
}
static void _SelectItem(MENU_Obj *pObj, unsigned Index) {
	if (pObj->Sel != (int)Index) {
		_SetCapture(pObj);
		_OpenSubmenu(pObj, Index);
		_SetSelection(pObj, Index);
	}
}
static void _DeselectItem(MENU_Obj *pObj) {
	if (pObj->IsSubmenuActive == 0) {
		_SetSelection(pObj, -1);
		_ReleaseCapture(pObj);
	}
}
static void _ActivateItem(MENU_Obj *pObj, unsigned Index) {
	MENU_ITEM *pItem;
	pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&pObj->ItemArray, Index);
	if (pItem->hSubmenu == 0) {
		if ((pItem->Flags & (MENU_IF_DISABLED | MENU_IF_SEPARATOR)) == 0) {
			_ClosePopup(pObj);
			/* Send item select message to owner. */
			MENU__SendMenuMessage(pObj, pObj->hOwner, MENU_ON_ITEMSELECT, pItem->Id);
		}
	}
}
static void _ActivateMenu(MENU_Obj *pObj, unsigned Index) {
	if ((pObj->Flags & MENU_SF_OPEN_ON_POINTEROVER) == 0) {
		MENU_ITEM *pItem;
		pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&pObj->ItemArray, Index);
		if (pItem->hSubmenu) {
			if ((pItem->Flags & MENU_IF_DISABLED) == 0) {
				if ((pObj->Flags & MENU_SF_ACTIVE) == 0) {
					pObj->Flags |= MENU_SF_ACTIVE;
					_OpenSubmenu(pObj, Index);
					_SetSelection(pObj, Index);
				}
				else if (pObj->Flags & MENU_SF_CLOSE_ON_SECOND_CLICK) {
					if ((int)Index == pObj->Sel) {
						_CloseSubmenu(pObj);
						pObj->Flags &= ~MENU_SF_ACTIVE;
					}
				}
			}
		}
	}
}
static void _DeactivateMenu(MENU_Obj *pObj) {
	_CloseSubmenu(pObj);
	if ((pObj->Flags & MENU_SF_OPEN_ON_POINTEROVER) == 0) {
		pObj->Flags &= ~MENU_SF_ACTIVE;
	}
}
static int _ForwardMouseOverMsg(MENU_Obj *pObj, int x, int y) {
#if (GUI_SUPPORT_MOUSE)
	if ((pObj->IsSubmenuActive == 0) && !(pObj->Flags & MENU_SF_POPUP)) {
		if (_IsTopLevelMenu(pObj)) {
			WM_HWIN hBelow;
			x += WM_GetWindowOrgX(pObj);
			y += WM_GetWindowOrgY(pObj);
			hBelow = WM_Screen2hWin(x, y);
			if (hBelow && (hBelow != pObj)) {
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
/*********************************************************************
*
*       _HandlePID
*
* Return values:
*   1 = We need to forward PID message to owner.
*   0 = We do not need to inform owner.
*/
static char _HandlePID(MENU_Obj *pObj, int x, int y, int Pressed) {
	GUI_PID_STATE PrevState;
	char XYInWidget = 0;
	WM_PID__GetPrevState(&PrevState);
	/*
	 * Check if coordinates are inside the widget.
	 */
	if ((x >= 0) && (y >= 0)) {
		GUI_RECT r;
		WM_GetClientRectEx(&pObj->Widget.Win, &r);
		if (x <= r.x1 && y <= r.y1)
			XYInWidget = 1;
	}
	if (XYInWidget) {
		int ItemIndex = _GetItemFromPos(pObj, x, y);
		/*
		 * Handle PID when coordinates are inside the widget.
		 */
		if (ItemIndex >= 0) {
			/*
			 * Coordinates are inside the menu.
			 */
			if (Pressed == 1) {
				if (PrevState.Pressed == 0) /* Clicked */
					_ActivateMenu(pObj, ItemIndex);
				_SelectItem(pObj, ItemIndex);
			}
			else if (Pressed == 0 && PrevState.Pressed == 1) /* Released */
				_ActivateItem(pObj, ItemIndex);
			else if (Pressed < 0) {  /* Mouse moved */
				if (_ForwardMouseOverMsg(pObj, x, y) == 0)
					_SelectItem(pObj, ItemIndex);
				else
					_DeselectItem(pObj);
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
					_DeactivateMenu(pObj);
				_DeselectItem(pObj);
			}
			else if (Pressed < 0) /* Moved out or mouse moved */
				_DeselectItem(pObj);
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
			_DeactivateMenu(pObj);
			_ClosePopup(pObj);
		}
		_DeselectItem(pObj);
		_ForwardMouseOverMsg(pObj, x, y);
	}
	return 1;   /* Coordinates are not in widget, we need to forward PID message to owner */
}
static void _ForwardPIDMsgToOwner(MENU_Obj *pObj, int MsgId, const GUI_PID_STATE *pState) {
	if (!_IsTopLevelMenu(pObj)) {
		WM_HWIN hOwner = pObj->hOwner ? pObj->hOwner : WM_GetParent(pObj);
		if (hOwner) {
			GUI_PID_STATE State;
			if (pState) {
				State = *pState;
				State.x += WM_GetWindowOrgX(pObj) - WM_GetWindowOrgX(hOwner);
				State.y += WM_GetWindowOrgY(pObj) - WM_GetWindowOrgY(hOwner);
				pState = &State;
			}
			WM__SendMessage(hOwner, MsgId, (WM_PARAM)pState);
		}
	}
}
static WM_PARAM _OnMenu(MENU_Obj *pObj, WM_PARAM Data) {
	const MENU_MSG_DATA *pData = (const MENU_MSG_DATA *)Data;
	if (!pData) 
		return 0;
	switch (pData->MsgType) {
		case MENU_ON_ITEMSELECT:
			_DeactivateMenu(pObj);
			_DeselectItem(pObj);
			_ClosePopup(pObj);
			/* No break here. We need to forward message to owner. */
		case MENU_ON_INITMENU:
		case MENU_ON_INITSUBMENU: {
			/* Forward message to owner. */
			WM_HWIN hOwner = pObj->hOwner ? pObj->hOwner : WM_GetParent(pObj);
			if (hOwner)
				WM__SendMessage(hOwner, WM_MENU, Data);
			break;
		}
		case MENU_ON_OPEN:
			pObj->Sel = -1;
			pObj->IsSubmenuActive = 0;
			pObj->Flags |= MENU_SF_ACTIVE | MENU_SF_OPEN_ON_POINTEROVER;
			_SetCapture(pObj);
			MENU__ResizeMenu(pObj);
			break;
		case MENU_ON_CLOSE:
			_CloseSubmenu(pObj);
			break;
		case MENU_IS_MENU:
			return 1;
	}
	return 0;
}
static char _OnTouch(MENU_Obj *pObj, const GUI_PID_STATE *pState) {
	if (pState) /* Something happened in our area (pressed or released) */
		return _HandlePID(pObj, pState->x, pState->y, pState->Pressed);
	return _HandlePID(pObj, -1, -1, -1); /* Moved out */
}
#if (GUI_SUPPORT_MOUSE)
static char _OnMouseOver(MENU_Obj *pObj, const GUI_PID_STATE *pState) {
	if (pState)
		return _HandlePID(pObj, pState->x, pState->y, -1);
	return 0;
}
#endif
static void _SetPaintColors(const MENU_Obj *pObj, const MENU_ITEM *pItem, int ItemIndex) {
	char Selected;
	unsigned ColorIndex;
	Selected = (ItemIndex == pObj->Sel) ? 1 : 0;
	if (pObj->IsSubmenuActive && Selected) {
		ColorIndex = MENU_CI_ACTIVE_SUBMENU;
	}
	else if (pItem->Flags & MENU_IF_SEPARATOR) {
		ColorIndex = MENU_CI_ENABLED;
	}
	else {
		ColorIndex = (Selected) ? MENU_CI_SELECTED : MENU_CI_ENABLED;
		if (pItem->Flags & MENU_IF_DISABLED) {
			if (pObj->Flags & MENU_CF_HIDE_DISABLED_SEL) {
				ColorIndex = MENU_CI_DISABLED;
			}
			else {
				ColorIndex += MENU_CI_DISABLED;
			}
		}
	}
	GUI_SetBkColor(pObj->Props.aBkColor[ColorIndex]);
	GUI_SetColor(pObj->Props.aTextColor[ColorIndex]);
}
static void _OnPaint(MENU_Obj *pObj) {
	GUI_RECT FillRect, TextRect;
	MENU_ITEM *pItem;
	unsigned TextWidth, NumItems, i;
	uint8_t BorderLeft = pObj->Props.aBorder[MENU_BI_LEFT];
	uint8_t BorderTop = pObj->Props.aBorder[MENU_BI_TOP];
	int FontHeight = GUI_GetYDistOfFont(pObj->Props.pFont);
	int EffectSize = _GetEffectSize(pObj);
	NumItems = MENU__GetNumItems(pObj);
	WM_GetClientRectEx(&pObj->Widget.Win, &FillRect);
	GUI__ReduceRect(&FillRect, &FillRect, EffectSize);
	GUI_SetFont(pObj->Props.pFont);
	if (pObj->Flags & MENU_SF_VERTICAL) {
		int ItemHeight, xSize;
		xSize = _CalcMenuSizeX(pObj);
		FillRect.x1 = xSize - EffectSize - 1;
		TextRect.x0 = FillRect.x0 + BorderLeft;
		for (i = 0; i < NumItems; i++) {
			pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&pObj->ItemArray, i);
			ItemHeight = _GetItemHeight(pObj, i);
			_SetPaintColors(pObj, pItem, i);
			FillRect.y1 = FillRect.y0 + ItemHeight - 1;
			if (pItem->Flags & MENU_IF_SEPARATOR) {
				GUI_ClearRectEx(&FillRect);
				GUI_SetColor(RGB_GRAYL(0x7C));
				GUI_DrawHLine(FillRect.y0 + BorderTop + 1, FillRect.x0 + 2, FillRect.x1 - 2);
			}
			else {
				TextWidth = pItem->TextWidth;
				TextRect.x1 = TextRect.x0 + TextWidth - 1;
				TextRect.y0 = FillRect.y0 + BorderTop;
				TextRect.y1 = TextRect.y0 + FontHeight - 1;
				WIDGET__FillStringInRect(pItem->acText, &FillRect, &TextRect, &TextRect);
			}
			FillRect.y0 += ItemHeight;
		}
	}
	else {
		int ItemWidth, ySize;
		ySize = _CalcMenuSizeY(pObj);
		FillRect.y1 = ySize - EffectSize - 1;
		TextRect.y0 = FillRect.y0 + BorderTop;
		TextRect.y1 = TextRect.y0 + FontHeight - 1;
		for (i = 0; i < NumItems; i++) {
			pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&pObj->ItemArray, i);
			ItemWidth = _GetItemWidth(pObj, i);
			_SetPaintColors(pObj, pItem, i);
			FillRect.x1 = FillRect.x0 + ItemWidth - 1;
			if (pItem->Flags & MENU_IF_SEPARATOR) {
				GUI_ClearRectEx(&FillRect);
				GUI_SetColor(RGB_GRAYL(0x7C));
				GUI_DrawVLine(FillRect.x0 + BorderLeft + 1, FillRect.y0 + 2, FillRect.y1 - 2);
			}
			else {
				TextWidth = pItem->TextWidth;
				TextRect.x0 = FillRect.x0 + BorderLeft;
				TextRect.x1 = TextRect.x0 + TextWidth - 1;
				WIDGET__FillStringInRect(pItem->acText, &FillRect, &TextRect, &TextRect);
			}
			FillRect.x0 += ItemWidth;
		}
	}
	if (pObj->Width || pObj->Height) {
		GUI_RECT r;
		WM_GetClientRectEx(&pObj->Widget.Win, &r);
		GUI__ReduceRect(&r, &r, EffectSize);
		GUI_SetBkColor(pObj->Props.aBkColor[MENU_CI_ENABLED]);
		GUI_ClearRect(FillRect.x1 + 1, EffectSize, r.x1, FillRect.y1);
		GUI_ClearRect(EffectSize, FillRect.y1 + 1, r.x1, r.y1);
	}
	/* Draw 3D effect (if configured) */
	if (_HasEffect(pObj)) {
		pObj->Widget.pEffect->pfDrawUp();
	}
}
static WM_PARAM _MENU_Callback(WM_HWIN hWin, int MsgId, WM_PARAM Data) {
	MENU_Obj *pObj = (MENU_Obj *)hWin;
	if (MsgId != WM_PID_STATE_CHANGED)
		/* Let widget handle the standard messages */
		if (!WIDGET_HandleActive(pObj, MsgId, &Data))
			return Data;
	switch (MsgId) {
		case WM_MENU:
			return _OnMenu(pObj, Data);
		case WM_TOUCH:
			if (_OnTouch(pObj, (const GUI_PID_STATE *)Data))
				_ForwardPIDMsgToOwner(pObj, WM_TOUCH, (const GUI_PID_STATE *)Data);
			break;
#if (GUI_SUPPORT_MOUSE)
		case WM_MOUSEOVER:
			if (_OnMouseOver(pObj, (const GUI_PID_STATE *)Data))
				_ForwardPIDMsgToOwner(pObj, WM_MOUSEOVER, (const GUI_PID_STATE *)Data);
			break;
#endif
		case WM_PAINT:
			_OnPaint(pObj);
			return 0;
		case WM_DELETE:
			GUI_ARRAY_Delete(&pObj->ItemArray);
			break; /* No return here ... WM_DefaultProc needs to be called */
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}
MENU_Handle MENU_CreateEx(int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id) {
	MENU_Handle hObj;
	/* Create the window */
	hObj = WM_CreateWindowAsChild(x0, y0, xSize, ySize, hParent, WM_CF_SHOW | WM_CF_STAYONTOP | WinFlags, &_MENU_Callback,
								  sizeof(MENU_Obj) - sizeof(WM_Obj));
	if (hObj) {
		MENU_Obj *pObj;

		pObj = (MENU_Obj *)(hObj);
		/* Init sub-classes */
		GUI_ARRAY_CREATE(&pObj->ItemArray);
		/* init widget specific variables */
		WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
		/* init member variables */
		if (ExFlags & MENU_SF_OPEN_ON_POINTEROVER) {
			ExFlags |= MENU_SF_ACTIVE;
		}
		else {
			ExFlags &= ~(MENU_SF_ACTIVE);
		}
		pObj->Props = MENU__DefaultProps;
		pObj->Flags = ExFlags;
		pObj->Width = ((xSize > 0) ? xSize : 0);
		pObj->Height = ((ySize > 0) ? ySize : 0);
		pObj->Sel = -1;
		pObj->hOwner = 0;
		pObj->IsSubmenuActive = 0;
		WIDGET_SetEffect(hObj, MENU__pDefaultEffect);

	}
	else {
	}
	return hObj;
}
unsigned MENU__GetNumItems(MENU_Obj *pObj) {
	return GUI_ARRAY_GetNumItems(&pObj->ItemArray);
}
void MENU__InvalidateItem(MENU_Obj *pObj, unsigned Index) {
	GUI_USE_PARA(pObj);
	GUI_USE_PARA(Index);
	WM_Invalidate(pObj);  /* Can be optimized, no need to invalidate all items */
}
void MENU__RecalcTextWidthOfItems(MENU_Obj *pObj) {
	const GUI_FONT  *pOldFont;
	MENU_ITEM *pItem;
	unsigned i, NumItems;
	NumItems = MENU__GetNumItems(pObj);
	pOldFont = GUI_SetFont(pObj->Props.pFont);
	for (i = 0; i < NumItems; i++) {
		pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&pObj->ItemArray, i);
		pItem->TextWidth = GUI_GetStringDistX(pItem->acText);
	}
	GUI_SetFont(pOldFont);
}
void MENU__ResizeMenu(MENU_Obj *pObj) {
	int xSize, ySize;
	xSize = _CalcWindowSizeX(pObj);
	ySize = _CalcWindowSizeY(pObj);
	WM_SetSize(pObj, xSize, ySize);
	WM_Invalidate(pObj);
}
char MENU__SetItem(MENU_Obj *pObj, unsigned Index, const MENU_ITEM_DATA *pItemData) {
	MENU_ITEM Item = { 0 };
	const char *pText;
	pText = pItemData->pText;
	if (!pText) {
		pText = "";
	}
	Item.Id = pItemData->Id;
	Item.Flags = pItemData->Flags;
	Item.hSubmenu = pItemData->hSubmenu;
	Item.TextWidth = _CalcTextWidth(pObj, pText);
	if (Item.Flags & MENU_IF_SEPARATOR) {
		Item.hSubmenu = 0;   /* Ensures that no separator is a submenu */
	}
	if (GUI_ARRAY_SetItem(&pObj->ItemArray, Index, &Item, sizeof(MENU_ITEM) + GUI__strlen(pText)) != 0) {
		MENU_ITEM *pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&pObj->ItemArray, Index);
		GUI__strcpy(pItem->acText, pText);
		MENU_SetOwner(Item.hSubmenu, pObj);
		return 1;
	}
	return 0;
}
void MENU__SetItemFlags(MENU_Obj *pObj, unsigned Index, uint16_t Mask, uint16_t Flags) {
	MENU_ITEM *pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&pObj->ItemArray, Index);
	pItem->Flags &= ~Mask;
	pItem->Flags |= Flags;
}
int MENU__SendMenuMessage(MENU_Handle hObj, WM_HWIN hDestWin, uint16_t MsgType, uint16_t ItemId) {
	if (!hDestWin)
		hDestWin = WM_GetParent(hObj);
	if (hDestWin) {
		MENU_MSG_DATA MsgData;
		MsgData.MsgType = MsgType;
		MsgData.ItemId = ItemId;
		return (int)WM__SendMessage(hDestWin, WM_MENU, (WM_PARAM)&MsgData);
	}
	return 0;
}
void MENU_AddItem(MENU_Handle hObj, const MENU_ITEM_DATA *pItemData) {
	if (hObj && pItemData) {
		MENU_Obj *pObj = (MENU_Obj *)hObj;
		if (pObj) {
			if (GUI_ARRAY_AddItem(&pObj->ItemArray, NULL, 0) == 0) {
				unsigned Index;
				Index = MENU__GetNumItems(pObj) - 1;
				if (MENU__SetItem(pObj, Index, pItemData) == 0) {
					GUI_ARRAY_DeleteItem(&pObj->ItemArray, Index);
				}
				else {
					MENU__ResizeMenu(pObj);
				}
			}
		}

	}
}
void MENU_SetOwner(MENU_Handle hObj, WM_HWIN hOwner) {
	if (hObj) {
		MENU_Obj *pObj = (MENU_Obj *)hObj;
		if (pObj) {
			pObj->hOwner = hOwner;
		}

	}
}

void MENU_Attach(MENU_Handle hObj, WM_HWIN hDestWin, int x, int y, int xSize, int ySize, int Flags) {
	GUI_USE_PARA(Flags);
	if (hObj) {
		MENU_Obj *pObj = (MENU_Obj *)hObj;
		if (pObj) {
			pObj->Width = ((xSize > 0) ? xSize : 0);
			pObj->Height = ((ySize > 0) ? ySize : 0);
			WM_AttachWindowAt(hObj, hDestWin, x, y);
			MENU__ResizeMenu(pObj);
		}

	}
}

MENU_Handle MENU_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	MENU_Handle hMenu;
	GUI_USE_PARA(cb);
	hMenu = MENU_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
						  hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
	return hMenu;
}

void MENU_SetDefaultTextColor(unsigned ColorIndex, RGB_COLOR Color) {
	if (ColorIndex <= GUI_COUNTOF(MENU__DefaultProps.aTextColor)) {
		MENU__DefaultProps.aTextColor[ColorIndex] = Color;
	}
}
void MENU_SetDefaultBkColor(unsigned ColorIndex, RGB_COLOR Color) {
	if (ColorIndex <= GUI_COUNTOF(MENU__DefaultProps.aBkColor)) {
		MENU__DefaultProps.aBkColor[ColorIndex] = Color;
	}
}
void MENU_SetDefaultBorderSize(unsigned BorderIndex, uint8_t BorderSize) {
	if (BorderIndex <= GUI_COUNTOF(MENU__DefaultProps.aBorder)) {
		MENU__DefaultProps.aBorder[BorderIndex] = BorderSize;
	}
}
void MENU_SetDefaultEffect(const WIDGET_EFFECT *pEffect) {
	MENU__pDefaultEffect = pEffect;
}
void MENU_SetDefaultFont(const GUI_FONT  *pFont) {
	MENU__DefaultProps.pFont = pFont;
}
RGB_COLOR MENU_GetDefaultTextColor(unsigned ColorIndex) {
	RGB_COLOR Color = GUI_INVALID_COLOR;
	if (ColorIndex <= GUI_COUNTOF(MENU__DefaultProps.aTextColor)) {
		Color = MENU__DefaultProps.aTextColor[ColorIndex];
	}
	return Color;
}
RGB_COLOR MENU_GetDefaultBkColor(unsigned ColorIndex) {
	RGB_COLOR Color = GUI_INVALID_COLOR;
	if (ColorIndex <= GUI_COUNTOF(MENU__DefaultProps.aBkColor)) {
		Color = MENU__DefaultProps.aBkColor[ColorIndex];
	}
	return Color;
}
uint8_t MENU_GetDefaultBorderSize(unsigned BorderIndex) {
	uint8_t BorderSize = 0;
	if (BorderIndex <= GUI_COUNTOF(MENU__DefaultProps.aBorder)) {
		BorderSize = MENU__DefaultProps.aBorder[BorderIndex];
	}
	return BorderSize;
}
const WIDGET_EFFECT *MENU_GetDefaultEffect(void) {
	return MENU__pDefaultEffect;
}
const GUI_FONT  *MENU_GetDefaultFont(void) {
	return MENU__DefaultProps.pFont;
}

void MENU_DeleteItem(MENU_Handle hObj, uint16_t ItemId) {
	if (hObj) {
		int Index;

		Index = MENU__FindItem(hObj, ItemId, &hObj);
		if (Index >= 0) {
			MENU_Obj *pObj = (MENU_Obj *)hObj;
			GUI_ARRAY_DeleteItem(&pObj->ItemArray, Index);
			MENU__ResizeMenu(pObj);
		}

	}
}

void MENU_DisableItem(MENU_Handle hObj, uint16_t ItemId) {
	if (hObj) {
		int Index;

		Index = MENU__FindItem(hObj, ItemId, &hObj);
		if (Index >= 0) {
			MENU_Obj *pObj = (MENU_Obj *)hObj;
			MENU__SetItemFlags(pObj, Index, MENU_IF_DISABLED, MENU_IF_DISABLED);
			MENU__InvalidateItem(pObj, Index);
		}

	}
}

void MENU_EnableItem(MENU_Handle hObj, uint16_t ItemId) {
	if (hObj) {
		int Index;

		Index = MENU__FindItem(hObj, ItemId, &hObj);
		if (Index >= 0) {
			MENU_Obj *pObj = (MENU_Obj *)hObj;
			MENU__SetItemFlags(pObj, Index, MENU_IF_DISABLED, 0);
			MENU__InvalidateItem(pObj, Index);
		}

	}
}

void MENU_GetItem(MENU_Handle hObj, uint16_t ItemId, MENU_ITEM_DATA *pItemData) {
	if (hObj && pItemData) {
		int Index;

		Index = MENU__FindItem(hObj, ItemId, &hObj);
		if (Index >= 0) {
			MENU_Obj *pObj = (MENU_Obj *)hObj;
			MENU_ITEM *pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&pObj->ItemArray, Index);
			pItemData->Flags = pItem->Flags;
			pItemData->Id = pItem->Id;
			pItemData->hSubmenu = pItem->hSubmenu;
			pItemData->pText = 0;
		}

	}
}

void MENU_GetItemText(MENU_Handle hObj, uint16_t ItemId, char *pBuffer, unsigned BufferSize) {
	if (hObj && pBuffer) {
		int Index = MENU__FindItem(hObj, ItemId, &hObj);
		if (Index >= 0) {
			MENU_Obj *pObj = (MENU_Obj *)hObj;
			MENU_ITEM *pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&pObj->ItemArray, Index);
			strncpy(pBuffer, pItem->acText, BufferSize);
			pBuffer[BufferSize - 1] = 0;
		}
	}
}

unsigned MENU_GetNumItems(MENU_Handle hObj) {
	unsigned r = 0;
	if (hObj) {
		MENU_Obj *pObj = (MENU_Obj *)hObj;
		if (pObj) {
			r = MENU__GetNumItems(pObj);
		}

	}
	return r;
}

void MENU_InsertItem(MENU_Handle hObj, uint16_t ItemId, const MENU_ITEM_DATA *pItemData) {
	if (hObj && pItemData) {
		int Index;

		Index = MENU__FindItem(hObj, ItemId, &hObj);
		if (Index >= 0) {
			MENU_Obj *pObj = (MENU_Obj *)hObj;
			if (GUI_ARRAY_InsertBlankItem(&pObj->ItemArray, Index) != 0) {
				if (MENU__SetItem(pObj, Index, pItemData) == 0) {
					GUI_ARRAY_DeleteItem(&pObj->ItemArray, Index);
				}
				else {
					MENU__ResizeMenu(pObj);
				}
			}
		}

	}
}

void MENU_Popup(MENU_Handle hObj, WM_HWIN hDestWin, int x, int y, int xSize, int ySize, int Flags) {
	GUI_USE_PARA(Flags);
	if (hObj && hDestWin) {
		MENU_Obj *pObj = (MENU_Obj *)hObj;
		if (pObj) {
			pObj->Flags |= MENU_SF_POPUP;
			pObj->Width = ((xSize > 0) ? xSize : 0);
			pObj->Height = ((ySize > 0) ? ySize : 0);
			x += WM_GetWindowOrgX(hDestWin);
			y += WM_GetWindowOrgY(hDestWin);
			MENU_SetOwner(hObj, hDestWin);
			WM_AttachWindowAt(hObj, WM_GetDesktopWindow(), x, y);
			MENU__SendMenuMessage(hDestWin, hObj, MENU_ON_OPEN, 0);
		}
	}
}

void MENU_SetBkColor(MENU_Handle hObj, unsigned ColorIndex, RGB_COLOR Color) {
	if (hObj) {
		MENU_Obj *pObj = (MENU_Obj *)hObj;
		if (pObj) {
			if (ColorIndex < GUI_COUNTOF(pObj->Props.aBkColor)) {
				if (Color != pObj->Props.aBkColor[ColorIndex]) {
					pObj->Props.aBkColor[ColorIndex] = Color;
					WM_Invalidate(hObj);
				}
			}
		}

	}
}

void MENU_SetBorderSize(MENU_Handle hObj, unsigned BorderIndex, uint8_t BorderSize) {
	if (hObj) {
		MENU_Obj *pObj = (MENU_Obj *)hObj;
		if (pObj) {
			if (BorderIndex < GUI_COUNTOF(pObj->Props.aBorder)) {
				if (BorderSize != pObj->Props.aBorder[BorderIndex]) {
					pObj->Props.aBorder[BorderIndex] = BorderSize;
					MENU__ResizeMenu(pObj);
				}
			}
		}

	}
}

void MENU_SetFont(MENU_Handle hObj, const GUI_FONT  *pFont) {
	if (hObj) {
		MENU_Obj *pObj = (MENU_Obj *)hObj;
		if (pObj) {
			if (pFont != pObj->Props.pFont) {
				pObj->Props.pFont = pFont;
				MENU__RecalcTextWidthOfItems(pObj);
				MENU__ResizeMenu(pObj);
			}
		}

	}
}

void MENU_SetItem(MENU_Handle hObj, uint16_t ItemId, const MENU_ITEM_DATA *pItemData) {
	if (hObj && pItemData) {
		int Index;

		Index = MENU__FindItem(hObj, ItemId, &hObj);
		if (Index >= 0) {
			MENU_Obj *pObj = (MENU_Obj *)hObj;
			if (MENU__SetItem(pObj, Index, pItemData) != 0) {
				MENU__ResizeMenu(pObj);
			}
		}

	}
}

void MENU_SetTextColor(MENU_Handle hObj, unsigned ColorIndex, RGB_COLOR Color) {
	if (hObj) {
		MENU_Obj *pObj = (MENU_Obj *)hObj;
		if (pObj) {
			if (ColorIndex < GUI_COUNTOF(pObj->Props.aTextColor)) {
				if (Color != pObj->Props.aTextColor[ColorIndex]) {
					pObj->Props.aTextColor[ColorIndex] = Color;
					WM_Invalidate(hObj);
				}
			}
		}

	}
}

int MENU__FindItem(MENU_Handle hObj, uint16_t ItemId, MENU_Handle *phMenu) {
	int ItemIndex = -1;
	MENU_Obj *pObj = (MENU_Obj *)hObj;
	if (pObj) {
		MENU_ITEM *pItem;
		unsigned NumItems, i;
		NumItems = MENU__GetNumItems(pObj);
		for (i = 0; (i < NumItems) && (ItemIndex < 0); i++) {
			pItem = (MENU_ITEM *)GUI_ARRAY_GetpItem(&pObj->ItemArray, i);
			if (pItem->Id == ItemId) {
				*phMenu = hObj;
				ItemIndex = i;
			}
			else if (pItem->hSubmenu) {
				ItemIndex = MENU__FindItem(pItem->hSubmenu, ItemId, phMenu);
			}
		}
	}
	return ItemIndex;
}
