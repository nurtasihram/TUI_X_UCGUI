module;

#include "DIALOG_Intern.h"
#include <memory>

export module TUX.Widget;

export {
constexpr uint16_t
	WIDGET_STATE_FOCUS       = 1 << 0,
	WIDGET_STATE_FOCUSSABLE  = 1 << 1,
	WIDGET_STATE_USER_START  = 1 << 2;
template<uint8_t bits>
constexpr uint16_t
	WIDGET_STATE_USER = 1 << (WIDGET_STATE_USER_START + bits);

constexpr uint16_t WM_WIDGET_SET_EFFECT     = WM_WIDGET + 0;

constexpr uint8_t
	WIDGET_ITEM_DRAW          = 0,
	WIDGET_ITEM_GET_XSIZE     = 1,
	WIDGET_ITEM_GET_YSIZE     = 2;
typedef int WIDGET_DRAW_ITEM_FUNC(WM_Obj *pWin, int Cmd, int ItemIndex, GUI_POINT ItemPos);

#pragma region Widget Effect
struct WIDGET_EFFECT {
	void DrawUp(void) const { DrawUp(WM_GetClientRect()); }
	void DrawDown(void) const { DrawDown(WM_GetClientRect()); }
	GUI_RECT GetRect() const { return WM_GetClientRect() - EffectSize; }

	virtual void DrawUp(GUI_RECT r) const {}
	virtual void DrawDown(GUI_RECT r) const {}

	int EffectSize;

	WIDGET_EFFECT(int EffectSize = 0) : EffectSize(EffectSize) {}
};
using CWIDGET_EFFECT = const WIDGET_EFFECT;
using PCWIDGET_EFFECT = const WIDGET_EFFECT *;

extern PCWIDGET_EFFECT
	WIDGET_Effect_None,
	WIDGET_Effect_Simple,
	WIDGET_Effect_3D,
	WIDGET_Effect_3D1L,
	WIDGET_Effect_3D2L;
#pragma endregion

#pragma region Draw
typedef void GUI_DRAW_SELF_CB(void);
struct GUI_DRAW {
	virtual void Draw(int x, int y) const = 0;
	virtual int GetXSize() const { return 0; }
	virtual int GetYSize() const { return 0; }
	GUI_POINT Off;
	GUI_DRAW(GUI_POINT Off = { 0, 0 }) : Off(Off) {}
};

GUI_DRAW *GUI_DRAW_BITMAP_Create(PCBITMAP pBitmap, int x, int y) {
	struct _GUI_DRAW : public GUI_DRAW {
		PCBITMAP pBitmap;
		_GUI_DRAW(PCBITMAP pBitmap, GUI_POINT Off = { 0, 0 }) : GUI_DRAW(Off), pBitmap(pBitmap) {}
		void Draw(int x, int y) const override {
			GUI_DrawBitmap(pBitmap, x + Off.x, y + Off.y);
		}
		int GetXSize() const override {
			return pBitmap ? pBitmap->XSize : 0;
		}
		int GetYSize() const override {
			return pBitmap ? pBitmap->YSize : 0;
		}
	};
	auto pObj = (GUI_DRAW *)GUI_ALLOC_AllocZero(sizeof(_GUI_DRAW));
	if (pObj)
		new (pObj) _GUI_DRAW{ pBitmap, { x, y } };
	return pObj;
}

GUI_DRAW *GUI_DRAW_SELF_Create(GUI_DRAW_SELF_CB *pfDraw, int x, int y) {
	struct _GUI_DRAW : public GUI_DRAW {
		GUI_DRAW_SELF_CB *pfDraw;
		_GUI_DRAW(GUI_DRAW_SELF_CB *pfDraw, GUI_POINT Off = { 0, 0 }) : GUI_DRAW(Off), pfDraw(pfDraw) {}
		void Draw(int x, int y) const override {
			if (pfDraw)
				pfDraw();
		}
	};
	auto pObj = (GUI_DRAW *)GUI_ALLOC_AllocZero(sizeof(_GUI_DRAW));
	if (pObj)
		new (pObj) _GUI_DRAW{ pfDraw, { x, y } };
	return pObj;
}
#pragma endregion

struct WIDGET : public WM_Obj {
	static PCWIDGET_EFFECT DefaultEffect;
	PCWIDGET_EFFECT pEffect = DefaultEffect;
	uint16_t Id, State;
	void SetBkColorPrefer(RGBC BkColor) {
		while (BkColor == RGB_INVALID_COLOR) {
			auto pParent = WM_GetParent(this);
			if (pParent)
				BkColor = WM_GetBkColor(pParent);
			else
				break;
		}
		GUI.SetBkColor(BkColor);
	}

	auto EffectSize() const {
		return pEffect ? pEffect->EffectSize : 0;
	}
	void DrawUp() const {
		if (pEffect)
			pEffect->DrawUp();
	}
	void DrawDown() const {
		if (pEffect)
			pEffect->DrawDown();
	}
	void DrawUp(const GUI_RECT &r) const {
		if (pEffect)
			pEffect->DrawUp(r);
	}
	void DrawDown(const GUI_RECT &r) const {
		if (pEffect)
			pEffect->DrawDown(r);
	}
};

GUI_RECT  WIDGET__GetInsideRect(WIDGET *pWidget);

void      WIDGET__Init(WIDGET *pWidget, int Id, uint16_t State);
void      WIDGET__RotateRect90(WIDGET *pWidget, GUI_RECT *pDest, const GUI_RECT *pRect);
void      WIDGET__SetScrollState(WM_Obj *hWin, const WM_SCROLL_STATE *pVState, const WM_SCROLL_STATE *pState);
void      WIDGET__FillStringInRect(const char *pText, GUI_RECT FillRect, GUI_RECT TextRectMax, GUI_RECT TextRectAct);

void  WIDGET_SetState(WM_Obj *hObj, int State);
void  WIDGET_AndState(WM_Obj *hObj, int State);
void  WIDGET_OrState(WM_Obj *hObj, int State);
int   WIDGET_GetState(WM_Obj *hObj);

void  WIDGET_SetEffect(WM_Obj *hObj, const WIDGET_EFFECT *pEffect);

bool  WIDGET_HandleActive(WM_Obj *hObj, int MsgId, WM_PARAM *Data);
}

PCWIDGET_EFFECT WIDGET::DefaultEffect = WIDGET_Effect_3D2L;

void WIDGET__RotateRect90(WIDGET *pWidget, GUI_RECT *pDest, const GUI_RECT *pRect) {
	int x0 = pRect->x0;
	int x1 = pRect->x1;
	int XSize = pWidget->Rect.x1 - pWidget->Rect.x0;
	pDest->x0 = XSize - pRect->y1;
	pDest->y0 = x0;
	pDest->x1 = XSize - pRect->y0;
	pDest->y1 = x1;
}

GUI_RECT WIDGET__GetInsideRect(WIDGET *pWidget) {
	return WM_GetClientRect(pWidget) - pWidget->EffectSize();
}

void WIDGET_SetState(WM_Obj *hObj, int State) {
	auto pWidget = (WIDGET *)hObj;
	if (State != pWidget->State) {
		pWidget->State = State;
		WM_Invalidate(hObj);
	}
}
int WIDGET_GetState(WM_Obj *hObj) {
	auto pWidget = (WIDGET *)hObj;
	if (hObj)
		return pWidget->State;
	return 0;
}
void WIDGET_OrState(WM_Obj *hObj, int State) {
	auto pWidget = (WIDGET *)hObj;
		if (State != (pWidget->State & State)) {
			pWidget->State |= State;
			WM_Invalidate(hObj);
		}
}
void WIDGET_AndState(WM_Obj *hObj, int Mask) {
		auto pWidget = (WIDGET *)hObj;
		auto StateNew = pWidget->State & (~Mask);
		if (pWidget->State != StateNew) {
			pWidget->State = StateNew;
			WM_Invalidate(hObj);
		}
}

void WIDGET__Init(WIDGET *pWidget, int Id, uint16_t State) {
	pWidget->pEffect = WIDGET::DefaultEffect;
	pWidget->State = State;
	pWidget->Id = Id;
}

bool WIDGET_HandleActive(WM_Obj *hObj, int MsgId, WM_PARAM *Data) {
	auto pWidget = (WIDGET *)hObj;
	switch (MsgId) {
		case WM_WIDGET_SET_EFFECT: {
			auto Diff = pWidget->EffectSize();
			pWidget->pEffect = (const WIDGET_EFFECT *)*Data;
			Diff -= pWidget->EffectSize();
			WM__UpdateChildPositions(hObj, -Diff, -Diff, Diff, Diff);
			WM_Invalidate(hObj);
			return false; /* Message handled -> Return */
		}
		case WM_GET_ID:
			*Data = pWidget->Id;
			return false; /* Message handled -> Return */
		case WM_PID_STATE_CHANGED:
			if (pWidget->State & WIDGET_STATE_FOCUSSABLE) {
				auto pInfo = (const WM_PID_STATE_CHANGED_INFO *)*Data;
				if (pInfo->State)
					WM_SetFocus(hObj);
			}
			break;
		case WM_TOUCH_CHILD: {
			/* A descendent (child) has been touched or released.
			   If it has been touched, we need to get to top.
			 */
			auto pState = (const GUI_PID_STATE *)*Data;
			if (pState) { /* Message may not have a valid pointer (moved out) ! */
				if (pState->Pressed) {
					WM_BringToTop(hObj);
					return false; /* Message handled -> Return */
				}
			}
			break;
		}
		case WM_SET_ID:
			pWidget->Id = (int16_t)*Data;
			return false; /* Message handled -> Return */
		case WM_SET_FOCUS: {
			int Notification;
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
			return false;
		}
		case WM_GET_ACCEPT_FOCUS:
			*(bool *)Data = pWidget->State & WIDGET_STATE_FOCUSSABLE; /* Can handle focus */
			return false; /* Message handled */
		case WM_GET_INSIDE_RECT:
			*(GUI_RECT *)*Data = WIDGET__GetInsideRect(pWidget);
			return false; /* Message handled */
	}
	return true; /* Message NOT handled */
}

void WIDGET__SetScrollState(WM_Obj *hWin, const WM_SCROLL_STATE *pVState, const WM_SCROLL_STATE *pHState) {
	WM_SetScrollState(WM_GetDialogItem(hWin, GUI_ID_VSCROLL), pVState);
	WM_SetScrollState(WM_GetDialogItem(hWin, GUI_ID_HSCROLL), pHState);
}

void WIDGET_SetEffect(WM_Obj *hObj, const WIDGET_EFFECT *pEffect) {
	WM_SendMessage(hObj, WM_WIDGET_SET_EFFECT, (WM_PARAM)pEffect);
}

void WIDGET__FillStringInRect(const char *pText, GUI_RECT FillRect, GUI_RECT TextRectMax, GUI_RECT TextRectAct) {
	/* Check if we have anything to do at all ... */
	auto r = FillRect + GUI.Off;
	if (!(r <= GUI.ClipRect))
		return;
	if (pText) {
		if (*pText) { /* Speed optimization, not required */
			/* Fill border */
			GUI_ClearRect(FillRect);
			/* Set clipping rectangle */
			auto pOldClipRect = WM_SetUserClipRect(&TextRectMax);
			/* Display text */
			GUI.SetTextMode(DRAWMODE_NORMAL);
			GUI_DispStringAt(pText, TextRectAct.x0, TextRectAct.y0);
			/* Restore clipping rectangle */
			WM_SetUserClipRect(pOldClipRect);
			return;
		}
	}
	GUI_ClearRect(FillRect);
}
