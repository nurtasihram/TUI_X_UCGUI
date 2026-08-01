#include "WM/WM.h"

import TUX.Window;
import TUX.Widget.ScrollBar;

static bool _SetScrollbar(WM_Obj *pWin, int OnOff, int Id, int Flags) {
	auto pBar = pWin->GetItem<SCROLLBAR_Obj>(Id);
	if (OnOff) {
		if (!pBar)
			SCROLLBAR_CreateAttached(pWin, Flags);
	}
	else {
		if (pBar)
			WM_DeleteWindow(pBar);
	}
	return pBar;
}
bool WM_SetScrollbarV(WM_Obj *pWin, int OnOff) {
	return _SetScrollbar(pWin, OnOff, GUI_ID_VSCROLL, SCROLLBAR_CF_VERTICAL);
}
bool WM_SetScrollbarH(WM_Obj *pWin, int OnOff) {
	return _SetScrollbar(pWin, OnOff, GUI_ID_HSCROLL, 0);
}

WM_Obj *WM_GetScrollPartner(WM_Obj *pScroll) {
	int Id = pScroll->GetID();
	if (Id == GUI_ID_HSCROLL) {
		Id = GUI_ID_VSCROLL;
	}
	else if (Id == GUI_ID_VSCROLL) {
		Id = GUI_ID_HSCROLL;
	}
	return WM_GetParent(pScroll)->GetItem<SCROLLBAR_Obj>(Id);
}
void WM_GetScrollState(WM_Obj *pObj, WM_SCROLL_STATE *pScrollState) {
	pObj->SendMessage(WM_GET_SCROLL_STATE, (WM_PARAM)pScrollState);
}
/*********************************************************************
*
*       WM_GetInsideRectExScrollbar
*
  Purpose:
	Return the inside rectangle in client coordinates.
	The inside rectangle is the client rectangle minus the effect,
	which typically reduces the rectangle by 0 - 3 pixels on either side
	(2 for the standard 3D effect).
*/
void WM_GetInsideRectExScrollbar(WM_Obj *pWin, GUI_RECT *pRect) {
	if (pWin) {
		if (pRect) {
			auto rWin = pWin->GetRect();     /* The entire window in screen coordinates */
			auto rInside = WM_GetInsideRect(pWin);
			if (auto pBarV = pWin->GetScrollbarV()) {
				auto rScrollbar = pBarV->GetRect() - rWin.LeftTop();
				auto WinFlags = pBarV->GetFlags();
				if ((WinFlags & WC_ANCHOR_RIGHT) && (WinFlags & WC_VISIBLE)) {
					rInside.x1 = rScrollbar.x0 - 1;
				}
			}
			if (auto pBarH = pWin->GetScrollbarH()) {
				auto rScrollbar = pBarH->GetRect() - rWin.LeftTop();
				auto WinFlags = pBarH->GetFlags();
				if ((WinFlags & WC_ANCHOR_BOTTOM) && (WinFlags & WC_VISIBLE)) {
					rInside.y1 = rScrollbar.y0 - 1;
				}
			}
			*pRect = rInside;
		}
	}
}

void WIDGET::SetScrollState(const WM_SCROLL_STATE &VState, const WM_SCROLL_STATE &HState) {
	if (auto pScroll = GetScrollbarV())
		pScroll->SetScrollState(VState);
	if (auto pScroll = GetScrollbarH())
		pScroll->SetScrollState(HState);
}
