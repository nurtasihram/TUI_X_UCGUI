#include "GUI_Protected.h"

#include "SLIDER.h"
#include "SLIDER_Private.h"

#define SLIDER_SUPPORT_TRANSPARENCY WM_SUPPORT_TRANSPARENCY

SLIDER_Obj::Properties SLIDER_Obj::DefaultProps;

static void _OnPaint(SLIDER_Obj *pObj) {
	GUI_RECT r, rFocus, rSlider, rSlot;
	int x0, xsize, i, Range, NumTicks;
	rFocus = WIDGET__GetClientRect(pObj);
	r = rFocus - 1;
	NumTicks = pObj->NumTicks;
	xsize = r.x1 - r.x0 + 1 - pObj->Width;
	x0 = r.x0 + pObj->Width / 2;
	Range = pObj->Max - pObj->Min;
	if (Range == 0) {
		Range = 1;
	}
	/* Fill with parents background color */
#if !SLIDER_SUPPORT_TRANSPARENCY   /* Not needed any more, since window is transparent*/
	if (pObj->Props.BkColor == RGB_INVALID_COLOR) {
		GUI_SetBkColor(WIDGET__GetBkColor(pObj));
	}
	else {
		GUI_SetBkColor(pObj->Props.BkColor);
	}
	GUI_Clear();
#else
	if (!WM_GetHasTrans(pObj)) {
		GUI_SetBkColor(pObj->Props.BkColor);
		GUI_Clear();
	}
#endif
	/* Calculate Slider position */
	rSlider = r;
	rSlider.y0 = 5;
	rSlider.x0 = x0 + (uint32_t)xsize * (uint32_t)(pObj->v - pObj->Min) / Range - pObj->Width / 2;
	rSlider.x1 = rSlider.x0 + pObj->Width;
	/* Calculate Slot position */
	rSlot.x0 = x0;
	rSlot.x1 = x0 + xsize;
	rSlot.y0 = (rSlider.y0 + rSlider.y1) / 2 - 1;
	rSlot.y1 = rSlot.y0 + 3;
	WIDGET__EFFECT_DrawDownRect(pObj, rSlot);        /* Draw slot */
	/* Draw the ticks */
	if (NumTicks < 0) {
		NumTicks = Range + 1;
		if (NumTicks > (xsize / 5)) {
			NumTicks = 11;
		}
	}
	if (NumTicks > 1) {
		GUI_SetColor(RGB_BLACK);
		for (i = 0; i < NumTicks; i++) {
			int x = x0 + xsize * i / (NumTicks - 1);
			WIDGET__DrawVLine(pObj, x, 1, 3);
		}
	}
	/* Draw the slider itself */
	GUI_SetColor(pObj->Props.Color);
	WIDGET__FillRect(pObj, rSlider);
	GUI_SetColor(RGB_BLACK);
	WIDGET__EFFECT_DrawUpRect(pObj, rSlider);
	/* Draw focus */
	if (pObj->State & WIDGET_STATE_FOCUS) {
		GUI_SetColor(RGB_BLACK);
		WIDGET__DrawFocusRect(pObj, rFocus, 0);
	}
}
static void _SliderPressed(SLIDER_Obj *pObj) {
	WIDGET_OrState(pObj, SLIDER_STATE_PRESSED);
	if (pObj->Status & WM_SF_ISVIS) {
		WM_NotifyParent(pObj, WM_NOTIFICATION_CLICKED);
	}
}
static void _SliderReleased(SLIDER_Obj *pObj) {
	WIDGET_AndState(pObj, SLIDER_STATE_PRESSED);
	if (pObj->Status & WM_SF_ISVIS) {
		WM_NotifyParent(pObj, WM_NOTIFICATION_RELEASED);
	}
}
static void _OnTouch(SLIDER_Obj *pObj, const GUI_PID_STATE *pState) {
	if (pState) {  /* Something happened in our area (pressed or released) */
		if (pState->Pressed) {
			int x0, xsize, x, Sel, Range;
			Range = (pObj->Max - pObj->Min);
			x0 = 1 + pObj->Width / 2;  /* 1 pixel focus rectangle + width of actual slider */
			x = (pObj->State & WIDGET_STATE_VERTICAL) ? pState->y : pState->x;
			x -= x0;
			xsize = WIDGET__GetWindowSizeX(pObj) - 2 * x0;
			if (x <= 0) {
				Sel = pObj->Min;
			}
			else if (x >= xsize) {
				Sel = pObj->Max;
			}
			else {
				int Div;
				Div = xsize ? xsize : 1;     /* Make sure we do not divide by 0, even though xsize should never be 0 in this case anyhow */
				Sel = pObj->Min + ((uint32_t)Range * (uint32_t)x + Div / 2) / Div;
			}
			if (WM_IsFocussable(pObj)) {
				WM_SetFocus(pObj);
			}
			WM_SetCapture(pObj, 1);
			SLIDER_SetValue(pObj, Sel);
			if ((pObj->State & SLIDER_STATE_PRESSED) == 0) {
				_SliderPressed(pObj);
			}
		}
		else {
			/* React only if button was pressed before ... avoid problems with moving / hiding windows above (such as dropdown) */
			if (pObj->State & SLIDER_STATE_PRESSED) {
				_SliderReleased(pObj);
			}
		}
	}
}
static char _OnKey(SLIDER_Obj *pObj, const WM_KEY_INFO *pInfo) {
	if (pInfo->PressedCnt > 0) {
		switch (pInfo->Key) {
			case GUI_KEY_RIGHT:
				SLIDER_Inc(pObj);
				return 1;
			case GUI_KEY_LEFT:
				SLIDER_Dec(pObj);
				return 1;
		}
	}
	return 0;
}
static WM_PARAM _SLIDER_Callback(WM_Obj * hWin, int MsgId, WM_PARAM Data) {
	auto pObj = (SLIDER_Obj *)hWin;
	/* Let widget handle the standard messages */
	if (!WIDGET_HandleActive(pObj, MsgId, &Data))
		return Data;
	switch (MsgId) {
		case WM_PAINT:
			_OnPaint(pObj);
			return 0;
		case WM_TOUCH:
			_OnTouch(pObj, (const GUI_PID_STATE *)Data);
			return 0;
		case WM_KEY:
			if (_OnKey(pObj, (const WM_KEY_INFO *)Data))
				return 0;
			break;
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}
/* Note: the parameters to a create function may vary.
		 Some widgets may have multiple create functions */
SLIDER_Handle SLIDER_CreateEx(int x0, int y0, int xsize, int ysize, WM_Obj * hParent,
							  int WinFlags, int ExFlags, int Id) {
	SLIDER_Handle hObj;
	/* Create the window */

#if SLIDER_SUPPORT_TRANSPARENCY
	WinFlags |= WM_CF_HASTRANS;
#endif
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, _SLIDER_Callback, sizeof(SLIDER_Obj) - sizeof(WM_Obj));
	if (hObj) {
		auto pObj = (SLIDER_Obj *)hObj;
		uint16_t InitState;
		/* Handle SpecialFlags */
		InitState = WIDGET_STATE_FOCUSSABLE;
		if (ExFlags & SLIDER_CF_VERTICAL) {
			InitState |= WIDGET_CF_VERTICAL;
		}
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, InitState);
		/* init member variables */
		pObj->Props = SLIDER_Obj::DefaultProps;
		pObj->Width = 8;
		pObj->Max = 100;
		pObj->Min = 0;
		pObj->NumTicks = -1;
	}
	else {
	}

	return hObj;
}
void SLIDER_Dec(SLIDER_Handle hObj) {
	auto pObj = (SLIDER_Obj *)hObj;
	if (hObj) {
		if (pObj->v > pObj->Min) {
			pObj->v--;
			WM_Invalidate(hObj);
			WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
		}
	}
}
void SLIDER_Inc(SLIDER_Handle hObj) {
	auto pObj = (SLIDER_Obj *)hObj;
	if (hObj) {
		if (pObj->v < pObj->Max) {
			pObj->v++;
			WM_Invalidate(hObj);
			WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
		}
	}
}
void SLIDER_SetWidth(SLIDER_Handle hObj, int Width) {
	auto pObj = (SLIDER_Obj *)hObj;
	if (hObj) {
		if (pObj->Width != Width) {
			pObj->Width = Width;
			WM_Invalidate(hObj);
		}
	}
}
void SLIDER_SetValue(SLIDER_Handle hObj, int v) {
	auto pObj = (SLIDER_Obj *)hObj;
	if (hObj) {
		/* Put in min/max range */
		if (v < pObj->Min)
			v = pObj->Min;
		if (v > pObj->Max)
			v = pObj->Max;
		if (pObj->v != v) {
			pObj->v = v;
			WM_Invalidate(hObj);
			WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
		}
	}
}
void SLIDER_SetRange(SLIDER_Handle hObj, int Min, int Max) {
	if (hObj) {
		auto pObj = (SLIDER_Obj *)hObj;
		if (Max < Min) {
			Max = Min;
		}
		pObj->Min = Min;
		pObj->Max = Max;
		if (pObj->v < Min) {
			pObj->v = Min;
		}
		if (pObj->v > Max) {
			pObj->v = Max;
		}
		WM_Invalidate(hObj);

	}
}
void SLIDER_SetNumTicks(SLIDER_Handle hObj, int NumTicks) {
	if (hObj && (NumTicks >= 0)) {
		auto pObj = (SLIDER_Obj *)hObj;
		pObj->NumTicks = NumTicks;
		WM_Invalidate(hObj);

	}
}
void SLIDER_SetBkColor(SLIDER_Handle hObj, RGBC Color) {
	if (hObj) {
		auto pObj = (SLIDER_Obj *)hObj;
		pObj->Props.BkColor = Color;
#if SLIDER_SUPPORT_TRANSPARENCY
		if (Color <= RGB_WHITE) {
			WM_ClrHasTrans(hObj);
		}
		else {
			WM_SetHasTrans(hObj);
		}
#endif
		WM_Invalidate(hObj);

	}
}
int SLIDER_GetValue(SLIDER_Handle hObj) {
	int r = 0;
	auto pObj = (SLIDER_Obj *)hObj;
	if (hObj) {
		r = pObj->v;
	}
	return r;
}

SLIDER_Handle SLIDER_Create(int x0, int y0, int xsize, int ysize, WM_Obj * hParent, int Id, int WinFlags, int SpecialFlags) {
	return SLIDER_CreateEx(x0, y0, xsize, ysize, hParent, WinFlags, SpecialFlags, Id);
}
SLIDER_Handle SLIDER_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	SLIDER_Handle  hThis;
	GUI_USE_PARA(cb);
	hThis = SLIDER_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
	return hThis;
}
