
#include <stdlib.h>
#include <string.h>
#include "WIDGET.h"
#include "GUIDebug.h"
#include "GUI.h"
#include "GUI_Protected.h"
#include "WM_Intern.h"
const WIDGET_EFFECT *_pEffectDefault = &WIDGET_Effect_3D;
static void _UpdateChildPostions(WM_HWIN hObj, int Diff) {
	WM_Obj *pObj;

	pObj = (WM_Obj *)(hObj);
	WM__UpdateChildPositions(pObj, -Diff, -Diff, Diff, Diff);

}
/*********************************************************************
*
*       _EffectRequiresRedraw
*
* Purpose
*   Check if the effect to draw is inside the invalid rectangle.
* Returns:
*   0 if nothing need to be done.
*   1 if the effect needs to be drawn
*/
static int _EffectRequiresRedraw(const WIDGET *pWidget, const GUI_RECT *pRect) {
	int EffectSize = pWidget->pEffect->EffectSize;
	GUI_RECT InvalidRect;
	InvalidRect = pWidget->Win.InvalidRect;
	WM__Client2Screen(&pWidget->Win, &InvalidRect);
	/* Check if there a part of the effect is inside the invalid rectangle */
	if ((pRect->x0 + EffectSize) > InvalidRect.x0) {
		return 1;               /* Overlap ... Drawing required */
	}
	if ((pRect->x1 - EffectSize) < InvalidRect.x1) {
		return 1;               /* Overlap ... Drawing required */
	}
	if ((pRect->y0 + EffectSize) > InvalidRect.y0) {
		return 1;               /* Overlap ... Drawing required */
	}
	if ((pRect->y1 - EffectSize) < InvalidRect.y1) {
		return 1;               /* Overlap ... Drawing required */
	}
	return 0;                 /* No overlap ! */
}
void WIDGET__RotateRect90(WIDGET *pWidget, GUI_RECT *pDest, const GUI_RECT *pRect) {
	int x0, x1, XSize;
	x0 = pRect->x0;
	x1 = pRect->x1;
	XSize = pWidget->Win.Rect.x1 - pWidget->Win.Rect.x0;
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
void WIDGET__GetClientRect(WIDGET *pWidget, GUI_RECT *pRect) {
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		GUI_RECT Rect;
		WM_GetClientRect(&Rect);
		pRect->x0 = Rect.y0;
		pRect->x1 = Rect.y1;
		pRect->y0 = Rect.x0;
		pRect->y1 = Rect.x1;
	}
	else {
		WM_GetClientRect(pRect);
	}
}
RGB_COLOR WIDGET__GetBkColor(WM_HWIN hObj) {
	RGB_COLOR BkColor = WM_GetBkColor(WM_GetParent(hObj));
	if (BkColor == GUI_INVALID_COLOR) {
		BkColor = DIALOG_GetBkColor();
	}
	return BkColor;
}
void WIDGET__GetInsideRect(WIDGET *pWidget, GUI_RECT *pRect) {
	WM__GetClientRectWin(&pWidget->Win, pRect);
	GUI__ReduceRect(pRect, pRect, pWidget->pEffect->EffectSize);
}
int WIDGET__GetXSize(const WIDGET *pWidget) {
	int r;
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		r = pWidget->Win.Rect.y1 - pWidget->Win.Rect.y0;
	}
	else {
		r = pWidget->Win.Rect.x1 - pWidget->Win.Rect.x0;
	}
	return r + 1;
}
int WIDGET__GetYSize(const WIDGET *pWidget) {
	int r;
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		r = pWidget->Win.Rect.x1 - pWidget->Win.Rect.x0;
	}
	else {
		r = pWidget->Win.Rect.y1 - pWidget->Win.Rect.y0;
	}
	return r + 1;
}
/*******************************************************************
*
*       WIDGET__GetWindowSizeX
  Return width (or height in case of rotation) of window in pixels
*/
int WIDGET__GetWindowSizeX(WM_HWIN hWin) {
	WIDGET *pWidget = (hWin);
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		return WM_GetWindowSizeY(hWin);
	}
	else {
		return WM_GetWindowSizeX(hWin);
	}
}
void WIDGET_SetState(WM_HWIN hObj, int State) {
	WIDGET *pWidget;

	pWidget = (hObj);
	if (State != pWidget->State) {
		pWidget->State = State;
		WM_Invalidate(hObj);
	}

}
int WIDGET_GetState(WM_HWIN hObj) {
	int Ret = 0;
	WIDGET *pWidget;
	if (hObj) {

		pWidget = (hObj);
		Ret = pWidget->State;

	}
	return Ret;
}
void WIDGET_OrState(WM_HWIN hObj, int State) {
	if (hObj) {
		WIDGET *pWidget;

		pWidget = (hObj);
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
	uint16_t StateNew;
	if (hObj) {
		WIDGET *pWidget;

		pWidget = (hObj);
		StateNew = pWidget->State & (~Mask);
		if (pWidget->State != StateNew) {
			pWidget->State = StateNew;
			WM_Invalidate(hObj);
		}

	}
}
void WIDGET__Init(WIDGET *pWidget, int Id, uint16_t State) {
	pWidget->pEffect = _pEffectDefault;
	pWidget->State = State;
	pWidget->Id = Id;
}
int WIDGET_HandleActive(WM_HWIN hObj, WM_MESSAGE *pMsg) {
	int Diff, Notification;
	WIDGET *pWidget = (hObj);
	switch (pMsg->MsgId) {
		case WM_WIDGET_SET_EFFECT:
			Diff = pWidget->pEffect->EffectSize;
			pWidget->pEffect = (const WIDGET_EFFECT *)pMsg->Data;
			Diff -= pWidget->pEffect->EffectSize;
			_UpdateChildPostions(hObj, Diff);
			WM_Invalidate(hObj);
			return 0;                        /* Message handled -> Return */
		case WM_GET_ID:
			pMsg->Data = pWidget->Id;
			return 0;                        /* Message handled -> Return */
		case WM_PID_STATE_CHANGED:
			if (pWidget->State & WIDGET_STATE_FOCUSSABLE) {
				const WM_PID_STATE_CHANGED_INFO *pInfo = (const WM_PID_STATE_CHANGED_INFO *)pMsg->Data;
				if (pInfo->State) {
					WM_SetFocus(hObj);
				}
			}
			break;
		case WM_TOUCH_CHILD:
			/* A descendent (child) has been touched or released.
			   If it has been touched, we need to get to top.
			 */
		{
			const WM_MESSAGE *pMsgOrg;
			const GUI_PID_STATE *pState;
			pMsgOrg = (const WM_MESSAGE *)pMsg->Data;      /* The original touch message */
			pState = (const GUI_PID_STATE *)pMsgOrg->Data;
			if (pState) {          /* Message may not have a valid pointer (moved out) ! */
				if (pState->Pressed) {
					WM_BringToTop(hObj);
					return 0;                    /* Message handled -> Return */
				}
			}
		}
		break;
		case WM_SET_ID:
			pWidget->Id = pMsg->Data;
			return 0;                        /* Message handled -> Return */
		case WM_SET_FOCUS:
			if (pMsg->Data == 1) {
				WIDGET_SetState(hObj, pWidget->State | WIDGET_STATE_FOCUS);
				Notification = WM_NOTIFICATION_GOT_FOCUS;
			}
			else {
				WIDGET_SetState(hObj, pWidget->State & ~WIDGET_STATE_FOCUS);
				Notification = WM_NOTIFICATION_LOST_FOCUS;
			}
			WM_NotifyParent(hObj, Notification);
			pMsg->Data = 0;   /* Focus change accepted */
			return 0;
		case WM_GET_ACCEPT_FOCUS:
			pMsg->Data = (pWidget->State & WIDGET_STATE_FOCUSSABLE) ? 1 : 0;               /* Can handle focus */
			return 0;                         /* Message handled */
		case WM_GET_INSIDE_RECT:
			WIDGET__GetInsideRect(pWidget, (GUI_RECT *)pMsg->Data);
			return 0;                         /* Message handled */
	}
	return 1;                           /* Message NOT handled */
}
void WIDGET__SetScrollState(WM_HWIN hWin, const WM_SCROLL_STATE *pVState, const WM_SCROLL_STATE *pHState) {
	WM_HWIN hScroll;
	/* vertical scrollbar */
	hScroll = WM_GetDialogItem(hWin, GUI_ID_VSCROLL);
	WM_SetScrollState(hScroll, pVState);
	/* horizontal scrollbar */
	hScroll = WM_GetDialogItem(hWin, GUI_ID_HSCROLL);
	WM_SetScrollState(hScroll, pHState);
}
void WIDGET__DrawFocusRect(WIDGET *pWidget, const GUI_RECT *pRect, int Dist) {
	GUI_RECT Rect;
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		WIDGET__RotateRect90(pWidget, &Rect, pRect);
		pRect = &Rect;
	}
	GUI_DrawFocusRect(pRect, Dist);
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
void WIDGET__FillRectEx(WIDGET *pWidget, const GUI_RECT *pRect) {
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		GUI_RECT r;
		WIDGET__RotateRect90(pWidget, &r, pRect);
		pRect = &r;
	}
	GUI_FillRectEx(pRect);
}
void WIDGET__EFFECT_DrawDownRect(WIDGET *pWidget, GUI_RECT *pRect) {
	GUI_RECT Rect;
	if (pRect == NULL) {
		WM_GetClientRect(&Rect);
		pRect = &Rect;
	}
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		WIDGET__RotateRect90(pWidget, &Rect, pRect);
		pRect = &Rect;
	}
	if (_EffectRequiresRedraw(pWidget, pRect)) {
		pWidget->pEffect->pfDrawDownRect(pRect);
	}
}
void WIDGET__EFFECT_DrawDown(WIDGET *pWidget) {
	WIDGET__EFFECT_DrawDownRect(pWidget, NULL);
}
void WIDGET__EFFECT_DrawUpRect(WIDGET *pWidget, GUI_RECT *pRect) {
	GUI_RECT Rect;
	if (pWidget->State & WIDGET_STATE_VERTICAL) {
		WIDGET__RotateRect90(pWidget, &Rect, pRect);
		pRect = &Rect;
	}
	if (_EffectRequiresRedraw(pWidget, pRect)) {
		pWidget->pEffect->pfDrawUpRect(pRect);
	}
}
const WIDGET_EFFECT *WIDGET_SetDefaultEffect(const WIDGET_EFFECT *pEffect) {
	const WIDGET_EFFECT *r;
	r = _pEffectDefault;
	_pEffectDefault = pEffect;
	return r;
}
const WIDGET_EFFECT *WIDGET_GetDefaultEffect(void) {
	return _pEffectDefault;
}

void WIDGET_SetEffect(WM_HWIN hObj, const WIDGET_EFFECT *pEffect) {
	WM_MESSAGE Msg;
	Msg.hWinSrc = 0;
	Msg.MsgId = WM_WIDGET_SET_EFFECT;
	Msg.Data = (WM_PARAM)pEffect;
	WM_SendMessage(hObj, &Msg);
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
	WIDGET *pWidget;
	int r = 0;
	if (hObj) {

		pWidget = (hObj);
		if (pWidget->State & WIDGET_STATE_VERTICAL) {
			r = WM_SetXSize(hObj, Width);
		}
		else {
			r = WM_SetYSize(hObj, Width);
		}

	}
	return r;
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
void WIDGET__FillStringInRect(const char  *pText, const GUI_RECT *pFillRect, const GUI_RECT *pTextRectMax, const GUI_RECT *pTextRectAct) {
	/* Check if we have anything to do at all ... */
	if (GUI_Context.pClipRect_HL) {
		GUI_RECT r;
		r = *pFillRect;
		WM_ADDORG(r.x0, r.y0);
		WM_ADDORG(r.x1, r.y1);
		if (GUI_RectsIntersect(GUI_Context.pClipRect_HL, &r) == 0)
			return;
	}
	if (pText) {
		if (*pText) {             /* Speed optimization, not required */
			const GUI_RECT *pOldClipRect;
			/* Fill border */
#if WIDGET_FILL_TEXT_USES_TRANS
			GUI_ClearRectEx(pFillRect);
#else
			{
				GUI_RECT rText;
				GUI__IntersectRects(&rText, pTextRectMax, pTextRectAct);
				GUI_ClearRect(pFillRect->x0, pFillRect->y0, pFillRect->x1, rText.y0 - 1); /* Top */
				GUI_ClearRect(pFillRect->x0, rText.y0, rText.x0 - 1, rText.y1);     /* Left */
				GUI_ClearRect(rText.x1 + 1, rText.y0, pFillRect->x1, rText.y1);     /* Right */
				GUI_ClearRect(pFillRect->x0, rText.y1 + 1, pFillRect->x1, pFillRect->y1);/* Bottom */
			}
#endif
			/* Set clipping rectangle */
			pOldClipRect = WM_SetUserClipRect(pTextRectMax);
			/* Display text */
#if WIDGET_FILL_TEXT_USES_TRANS
			GUI_SetTextMode(DRAWMODE_TRANS);
#else
			GUI_SetTextMode(DRAWMODE_NORMAL);
#endif
			GUI_DispStringAt(pText, pTextRectAct->x0, pTextRectAct->y0);
			/* Restore clipping rectangle */
			WM_SetUserClipRect(pOldClipRect);
			return;
		}
	}
	GUI_ClearRectEx(pFillRect);
}
