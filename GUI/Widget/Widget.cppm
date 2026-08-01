module;

#include "DIALOG_Intern.h"
#include <memory>

export module TUX.Widget;

export {
constexpr uint16_t WIDGET_STATE_FOCUS       = 1 << 0;
constexpr uint16_t WIDGET_STATE_VERTICAL    = 1 << 3;
constexpr uint16_t WIDGET_STATE_FOCUSSABLE  = 1 << 4;
constexpr uint16_t WIDGET_STATE_USER0       = 1 << 8;    /*Freely available for derived widget */
constexpr uint16_t WIDGET_STATE_USER1       = 1 << 9;    /*Freely available for derived widget */
constexpr uint16_t WIDGET_STATE_USER2       = 1 << 10;   /*Freely available for derived widget */
constexpr uint16_t WIDGET_CF_VERTICAL       = WIDGET_STATE_VERTICAL;

constexpr uint8_t WIDGET_ITEM_DRAW          = 0;
constexpr uint8_t WIDGET_ITEM_GET_XSIZE     = 1;
constexpr uint8_t WIDGET_ITEM_GET_YSIZE     = 2;

constexpr uint16_t WM_WIDGET_SET_EFFECT      = WM_WIDGET + 0;

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
};

void      WIDGET__DrawFocusRect(WIDGET *pWidget, GUI_RECT r, int Dist);
void      WIDGET__DrawVLine(WIDGET *pWidget, int x, int y0, int y1);
void      WIDGET__FillRect(WIDGET *pWidget, GUI_RECT r);

void      WIDGET__EFFECT_DrawDownRect(WIDGET *pWidget, GUI_RECT r);
void      WIDGET__EFFECT_DrawDown(WIDGET *pWidget);
void      WIDGET__EFFECT_DrawUpRect(WIDGET *pWidget, GUI_RECT r);

RGBC WIDGET__GetBkColor(WM_Obj *hObj);

int       WIDGET__GetWindowSizeX(WM_Obj *hWin);

int       WIDGET__GetXSize(const WIDGET *pWidget);
int       WIDGET__GetYSize(const WIDGET *pWidget);
GUI_RECT  WIDGET__GetClientRect(WIDGET *pWidget);
GUI_RECT  WIDGET__GetInsideRect(WIDGET *pWidget);

void      WIDGET__Init(WIDGET *pWidget, int Id, uint16_t State);
void      WIDGET__RotateRect90(WIDGET *pWidget, GUI_RECT *pDest, const GUI_RECT *pRect);
void      WIDGET__SetScrollState(WM_Obj *hWin, const WM_SCROLL_STATE *pVState, const WM_SCROLL_STATE *pState);
void      WIDGET__FillStringInRect(const char *pText, GUI_RECT FillRect, GUI_RECT TextRectMax, GUI_RECT TextRectAct);

void  WIDGET_SetState(WM_Obj *hObj, int State);
void  WIDGET_AndState(WM_Obj *hObj, int State);
void  WIDGET_OrState(WM_Obj *hObj, int State);
int   WIDGET_GetState(WM_Obj *hObj);
int   WIDGET_SetWidth(WM_Obj *hObj, int Width);
int   WIDGET_SetWidth(WM_Obj *hObj, int Width);

void  WIDGET_SetEffect(WM_Obj *hObj, const WIDGET_EFFECT *pEffect);

bool  WIDGET_HandleActive(WM_Obj *hObj, int MsgId, WM_PARAM *Data);
}

PCWIDGET_EFFECT WIDGET::DefaultEffect = WIDGET_Effect_3D2L;

static void _UpdateChildPostions(WM_Obj *hObj, int Diff) {
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

RGBC WIDGET__GetBkColor(WM_Obj *hObj) {
	RGBC BkColor = WM_GetBkColor(WM_GetParent(hObj));
	if (BkColor == RGB_INVALID_COLOR) {
		BkColor = DIALOG_GetBkColor();
	}
	return BkColor;
}

GUI_RECT WIDGET__GetInsideRect(WIDGET *pWidget) {
	return WM_GetClientRect(pWidget) - pWidget->pEffect->EffectSize;
}
GUI_RECT WIDGET__GetClientRect(WIDGET *pWidget) {
	if (pWidget->State & WIDGET_STATE_VERTICAL)
		return ~WM_GetClientRect();
	return WM_GetClientRect();
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
int WIDGET__GetWindowSizeX(WM_Obj *hWin) {
	auto pWidget = (WIDGET *)hWin;
	if (pWidget->State & WIDGET_STATE_VERTICAL)
		return WM_GetWindowSizeY(hWin);
	return WM_GetWindowSizeX(hWin);
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
			auto Diff = pWidget->pEffect->EffectSize;
			pWidget->pEffect = (const WIDGET_EFFECT *)*Data;
			Diff -= pWidget->pEffect->EffectSize;
			_UpdateChildPostions(hObj, Diff);
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
		pWidget->pEffect->DrawDown(r);
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
		pWidget->pEffect->DrawUp(r);
}

void WIDGET_SetEffect(WM_Obj *hObj, const WIDGET_EFFECT *pEffect) {
	WM_SendMessage(hObj, WM_WIDGET_SET_EFFECT, (WM_PARAM)pEffect);
}

int WIDGET_SetWidth(WM_Obj *hObj, int Width) {
	auto pWidget = (WIDGET *)hObj;
		if (pWidget->State & WIDGET_STATE_VERTICAL)
			return WM_SetXSize(hObj, Width);
		return WM_SetYSize(hObj, Width);
	return 0;
}

#define WIDGET_FILL_TEXT_USES_TRANS 0

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
