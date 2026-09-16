#include "WM/WM.h"

import TUX.Window;
import TUX.Widget.ScrollBar;

static bool _SetScrollbar(WObj *pWin, int OnOff, int Id, int Flags) {
	auto pBar = pWin->GetItem<ScrollBar>(Id);
	if (OnOff) {
		if (!pBar)
			new ScrollBar(pWin, Flags);
	}
	else if (pBar)
		delete pBar;
	return pBar;
}
bool WM_SetScrollbarV(WObj *pWin, int OnOff) {
	return _SetScrollbar(pWin, OnOff, GUI_ID_VSCROLL, SCROLLBAR_CF_VERTICAL);
}
bool WM_SetScrollbarH(WObj *pWin, int OnOff) {
	return _SetScrollbar(pWin, OnOff, GUI_ID_HSCROLL, 0);
}

WObj *WM_GetScrollPartner(WObj *pScroll) {
	int Id = pScroll->GetID();
	if (Id == GUI_ID_HSCROLL)
		Id = GUI_ID_VSCROLL;
	else if (Id == GUI_ID_VSCROLL)
		Id = GUI_ID_HSCROLL;
	return pScroll->Parent()->GetItem<ScrollBar>(Id);
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
void WM_GetInsideRectExScrollbar(WObj *pWin, RECT *pRect) {
	if (pWin) {
		if (pRect) {
			auto rWin = pWin->Rect();     /* The entire window in screen coordinates */
			auto rInside = pWin->InsideRect();
			if (auto pBarV = pWin->GetScrollbarV()) {
				auto rScrollbar = pBarV->Rect() - rWin.LeftTop();
				auto WinFlags = pBarV->GetFlags();
				if ((WinFlags & WC_ANCHOR_RIGHT) && (WinFlags & WC_VISIBLE)) {
					rInside.x1 = rScrollbar.x0 - 1;
				}
			}
			if (auto pBarH = pWin->GetScrollbarH()) {
				auto rScrollbar = pBarH->Rect() - rWin.LeftTop();
				auto WinFlags = pBarH->GetFlags();
				if ((WinFlags & WC_ANCHOR_BOTTOM) && (WinFlags & WC_VISIBLE)) {
					rInside.y1 = rScrollbar.y0 - 1;
				}
			}
			*pRect = rInside;
		}
	}
}

void Widget::ScrollState(const SCROLL_STATE &VState, const SCROLL_STATE &HState) {
	if (auto pScroll = GetScrollbarV())
		pScroll->ScrollState(VState);
	if (auto pScroll = GetScrollbarH())
		pScroll->ScrollState(HState);
}
