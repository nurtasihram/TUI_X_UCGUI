
#include "WIDGET.h"
#include "GUIDebug.h"
#include "GUI.h"
#include "GUI_Protected.h"
#include "WM_Intern.h"

const WIDGET_EFFECT *WIDGET::DefaultEffect = &WIDGET_Effect_3D2L;

static void _UpdateChildPostions(WM_HWIN hObj, int Diff) {
	WM_Obj *pObj;
	pObj = (WM_Obj *)(hObj);
	WM__UpdateChildPositions(pObj, -Diff, -Diff, Diff, Diff);
}

void WIDGET__RotateRect90(WIDGET *pWidget, GUI_RECT *pDest, const GUI_RECT *pRect) {
	int x0 = pRect->x0;
	int x1 = pRect->x1;
	int XSize = pWidget->Rect.x1 - pWidget->Rect.x0;
	pDest->x0 = XSize - pRect->y1;
	pDest->x1 = XSize - pRect->y0;
	pDest->y0 = x0;
	pDest->y1 = x1;
}

/*********************************************************************
*
*       WIDGET__GetClientRect
  Returns the logical client rectangle, which means the normal
  client rectangle for widgets with their standard orientation
  and the rotated one for rotated widgets.
*/
GUI_RECT WIDGET__GetClientRect(WIDGET *pWidget) {
	if (pWidget->State & WIDGET_STATE_VERTICAL)
		return ~WM_GetClientRect();
	return WM_GetClientRect();
}
RGBC WIDGET__GetBkColor(WM_HWIN hObj) {
	RGBC BkColor = WM_GetBkColor(WM_GetParent(hObj));
	if (BkColor == RGB_INVALID_COLOR) {
		BkColor = DIALOG_GetBkColor();
	}
	return BkColor;
}
GUI_RECT WIDGET__GetInsideRect(WIDGET *pWidget) {
	return WM_GetClientRect(pWidget)- pWidget->pEffect->EffectSize;
}
int WIDGET__GetXSize(const WIDGET *pWidget) {
	int r;
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		r = pWidget->Rect.y1 - pWidget->Rect.y0;
	}
	else {
		r = pWidget->Rect.x1 - pWidget->Rect.x0;
	}
	return r + 1;
}
int WIDGET__GetYSize(const WIDGET *pWidget) {
	int r;
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		r = pWidget->Rect.x1 - pWidget->Rect.x0;
	}
	else {
		r = pWidget->Rect.y1 - pWidget->Rect.y0;
	}
	return r + 1;
}
/*******************************************************************
*
*       WIDGET__GetWindowSizeX
  Return width (or height in case of rotation) of window in pixels
*/
int WIDGET__GetWindowSizeX(WM_HWIN hWin) {
	auto pWidget = (WIDGET *)hWin;
	if (pWidget->State & WIDGET_STATE_VERTICAL)
		return WM_GetWindowSizeY(hWin);
	return WM_GetWindowSizeX(hWin);
}
void WIDGET_SetState(WM_HWIN hObj, int State) {
	auto pWidget = (WIDGET *)hObj;
	if (State != pWidget->State) {
		pWidget->State = State;
		WM_Invalidate(hObj);
	}
}
int WIDGET_GetState(WM_HWIN hObj) {
	auto pWidget = (WIDGET *)hObj;
	if (hObj)
		return pWidget->State;
	return 0;
}
void WIDGET_OrState(WM_HWIN hObj, int State) {
	auto pWidget = (WIDGET *)hObj;
	if (hObj) {
		if (State != (pWidget->State & State)) {
			pWidget->State |= State;
			WM_Invalidate(hObj);
		}
	}
}
/*********************************************************************
*
*       WIDGET_AndState
  Purpose:
	Clear flags in the State element of the widget.
	The bits to be cleared are set.
  Example:
	...(..., 3);   // Clears bit 0, 1 int the state member
*/
void WIDGET_AndState(WM_HWIN hObj, int Mask) {
	if (hObj) {
		auto pWidget = (WIDGET *)hObj;
		auto StateNew = pWidget->State & (~Mask);
		if (pWidget->State != StateNew) {
			pWidget->State = StateNew;
			WM_Invalidate(hObj);
		}
	}
}
void WIDGET__Init(WIDGET *pWidget, int Id, uint16_t State) {
	pWidget->pEffect = WIDGET::DefaultEffect;
	pWidget->State = State;
	pWidget->Id = Id;
}
int WIDGET_HandleActive(WM_HWIN hObj, int MsgId, WM_PARAM *Data) {
	int Diff, Notification;
	auto pWidget = (WIDGET *)hObj;
	switch (MsgId) {
		case WM_WIDGET_SET_EFFECT:
			Diff = pWidget->pEffect->EffectSize;
			pWidget->pEffect = (const WIDGET_EFFECT *)*Data;
			Diff -= pWidget->pEffect->EffectSize;
			_UpdateChildPostions(hObj, Diff);
			WM_Invalidate(hObj);
			return 0; /* Message handled -> Return */
		case WM_GET_ID:
			*Data = pWidget->Id;
			return 0; /* Message handled -> Return */
		case WM_PID_STATE_CHANGED:
			if (pWidget->State & WIDGET_STATE_FOCUSSABLE) {
				const WM_PID_STATE_CHANGED_INFO *pInfo = (const WM_PID_STATE_CHANGED_INFO *)*Data;
				if (pInfo->State)
					WM_SetFocus(hObj);
			}
			break;
		case WM_TOUCH_CHILD: {
			/* A descendent (child) has been touched or released.
			   If it has been touched, we need to get to top.
			 */
			const GUI_PID_STATE *pState = (const GUI_PID_STATE *)*Data;
			if (pState) { /* Message may not have a valid pointer (moved out) ! */
				if (pState->Pressed) {
					WM_BringToTop(hObj);
					return 0; /* Message handled -> Return */
				}
			}
			break;
		}
		case WM_SET_ID:
			pWidget->Id = (int16_t)*Data;
			return 0; /* Message handled -> Return */
		case WM_SET_FOCUS:
			if (*Data) {
				WIDGET_SetState(hObj, pWidget->State | WIDGET_STATE_FOCUS);
				Notification = WM_NOTIFICATION_GOT_FOCUS;
			}
			else {
				WIDGET_SetState(hObj, pWidget->State & ~WIDGET_STATE_FOCUS);
				Notification = WM_NOTIFICATION_LOST_FOCUS;
			}
			WM_NotifyParent(hObj, Notification);
			*Data = 0;   /* Focus change accepted */
			return 0;
		case WM_GET_ACCEPT_FOCUS:
			*Data = (pWidget->State & WIDGET_STATE_FOCUSSABLE) ? 1 : 0; /* Can handle focus */
			return 0; /* Message handled */
		case WM_GET_INSIDE_RECT:
			*(GUI_RECT *)*Data = WIDGET__GetInsideRect(pWidget);
			return 0; /* Message handled */
	}
	return 1; /* Message NOT handled */
}
void WIDGET__SetScrollState(WM_HWIN hWin, const WM_SCROLL_STATE *pVState, const WM_SCROLL_STATE *pHState) {
	WM_SetScrollState(WM_GetDialogItem(hWin, GUI_ID_VSCROLL), pVState);
	WM_SetScrollState(WM_GetDialogItem(hWin, GUI_ID_HSCROLL), pHState);
}
void WIDGET__DrawFocusRect(WIDGET *pWidget, GUI_RECT r, int Dist) {
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		GUI_RECT Rect;
		WIDGET__RotateRect90(pWidget, &Rect, &r);
		r = Rect;
	}
	GUI_DrawFocusRect(r, Dist);
}
void WIDGET__DrawVLine(WIDGET *pWidget, int x, int y0, int y1) {
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		GUI_RECT r0, r1;
		r0.x0 = x;
		r0.x1 = x;
		r0.y0 = y0;
		r0.y1 = y1;
		WIDGET__RotateRect90(pWidget, &r1, &r0);
		GUI_DrawHLine(r1.y0, r1.x0, r1.x1);
	}
	else {
		GUI_DrawVLine(x, y0, y1);
	}
}
void WIDGET__FillRect(WIDGET *pWidget, GUI_RECT r) {
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		GUI_RECT Rect;
		WIDGET__RotateRect90(pWidget, &Rect, &r);
		r = Rect;
	}
	GUI_FillRect(r);
}

/*********************************************************************
*
*       _EffectRequiresRedraw
*
* Purpose
*   Check if the effect to draw is inside the invalid rectangle.
* Returns:
*   false if nothing need to be done.
*   true if the effect needs to be drawn
*/
static bool _EffectRequiresRedraw(const WIDGET *pWidget, GUI_RECT r) {
	int EffectSize = pWidget->pEffect->EffectSize;
	GUI_RECT InvalidRect = pWidget->InvalidRect;
	WM__Client2Screen(pWidget, &InvalidRect);
	/* Check if there a part of the effect is inside the invalid rectangle */
	if (r.x0 + EffectSize > InvalidRect.x0)
		return true; /* Overlap ... Drawing required */
	if (r.y0 + EffectSize > InvalidRect.y0)
		return true; /* Overlap ... Drawing required */
	if (r.x1 - EffectSize < InvalidRect.x1)
		return true; /* Overlap ... Drawing required */
	if (r.y1 - EffectSize < InvalidRect.y1)
		return true; /* Overlap ... Drawing required */
	return false; /* No overlap ! */
}
void WIDGET__EFFECT_DrawDownRect(WIDGET *pWidget, GUI_RECT r) {
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		GUI_RECT Rect;
		WIDGET__RotateRect90(pWidget, &Rect, &r);
		r = Rect;
	}
	if (_EffectRequiresRedraw(pWidget, r))
		pWidget->pEffect->pfDrawDownRect(r);
}
void WIDGET__EFFECT_DrawDown(WIDGET *pWidget) {
	WIDGET__EFFECT_DrawDownRect(pWidget, WM_GetClientRect());
}
void WIDGET__EFFECT_DrawUpRect(WIDGET *pWidget, GUI_RECT r) {
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		GUI_RECT Rect;
		WIDGET__RotateRect90(pWidget, &Rect, &r);
		r = Rect;
	}
	if (_EffectRequiresRedraw(pWidget, r))
		pWidget->pEffect->pfDrawUpRect(r);
}

void WIDGET_SetEffect(WM_HWIN hObj, const WIDGET_EFFECT *pEffect) {
	WM_SendMessage(hObj, WM_WIDGET_SET_EFFECT, (WM_PARAM)pEffect);
}

/*********************************************************************
*
*       WIDGET_SetWidth
*
* Function:
*   Set width of the given widget. Width can be X-Size or Y-Size,
*   depending on if the widget is rotated.
*/
int WIDGET_SetWidth(WM_HWIN hObj, int Width) {
	auto pWidget = (WIDGET *)hObj;
	if (hObj) {
		if (pWidget->State & WIDGET_STATE_VERTICAL)
			return WM_SetXSize(hObj, Width);
		return WM_SetYSize(hObj, Width);
	}
	return 0;
}

#define WIDGET_FILL_TEXT_USES_TRANS 0

/*********************************************************************
*
*       WIDGET__FillStringInRect
*
* Purpose
*
* Parameters
*
* Notes
*/
void WIDGET__FillStringInRect(const char *pText, GUI_RECT FillRect, GUI_RECT TextRectMax, GUI_RECT TextRectAct) {
	/* Check if we have anything to do at all ... */
	GUI_RECT r = FillRect;
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	if (!GUI_RectsIntersect(&GUI_Context.ClipRect, &r))
		return;
	if (pText) {
		if (*pText) { /* Speed optimization, not required */
			/* Fill border */
			GUI_ClearRect(FillRect);
			/* Set clipping rectangle */
			auto pOldClipRect = WM_SetUserClipRect(&TextRectMax);
			/* Display text */
			GUI_SetTextMode(DRAWMODE_NORMAL);
			GUI_DispStringAt(pText, TextRectAct.x0, TextRectAct.y0);
			/* Restore clipping rectangle */
			WM_SetUserClipRect(pOldClipRect);
			return;
		}
	}
	GUI_ClearRect(FillRect);
}
