module;

#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */

export module TUX.Widget.Slider;

import TUX.Widget;

export {
constexpr uint16_t SLIDER_STATE_PRESSED    = WIDGET_STATE_USER0;
constexpr uint16_t SLIDER_CF_VERTICAL = WIDGET_CF_VERTICAL;

struct SLIDER_Obj : public WIDGET {
	struct Properties {
		RGBC BkColor{ RGB_GRAYL(0xC0) };
		RGBC Color{ RGB_GRAYL(0xC0) };
	} static DefaultProps;
	Properties Props;
	int16_t Min, Max, v;
	int16_t NumTicks;
	int16_t Width;
	uint8_t Flags;

	void _OnPaint() {
		GUI_RECT r, rFocus, rSlider, rSlot;
		int x0, xsize, i, Range, NumTicks;
		rFocus = WIDGET__GetClientRect(this);
		r = rFocus - 1;
		NumTicks = this->NumTicks;
		xsize = r.x1 - r.x0 + 1 - this->Width;
		x0 = r.x0 + this->Width / 2;
		Range = this->Max - this->Min;
		if (Range == 0) {
			Range = 1;
		}
		/* Fill with parents background color */
#if !WM_SUPPORT_TRANSPARENCY   /* Not needed any more, since window is transparent */
		if (this->Props.BkColor == RGB_INVALID_COLOR) {
			GUI.SetBkColor(WIDGET__GetBkColor(this));
		}
		else {
			GUI.SetBkColor(this->Props.BkColor);
		}
		GUI_Clear();
#else
		if (!WM_GetHasTrans(this)) {
			GUI.SetBkColor(this->Props.BkColor);
			GUI_Clear();
		}
#endif
		/* Calculate Slider position */
		rSlider = r;
		rSlider.y0 = 5;
		rSlider.x0 = x0 + (uint32_t)xsize * (uint32_t)(this->v - this->Min) / Range - this->Width / 2;
		rSlider.x1 = rSlider.x0 + this->Width;
		/* Calculate Slot position */
		rSlot.x0 = x0;
		rSlot.x1 = x0 + xsize;
		rSlot.y0 = (rSlider.y0 + rSlider.y1) / 2 - 1;
		rSlot.y1 = rSlot.y0 + 3;
		WIDGET__EFFECT_DrawDownRect(this, rSlot);        /* Draw slot */
		/* Draw the ticks */
		if (NumTicks < 0) {
			NumTicks = Range + 1;
			if (NumTicks > (xsize / 5)) {
				NumTicks = 11;
			}
		}
		if (NumTicks > 1) {
			GUI.SetColor(RGB_BLACK);
			for (i = 0; i < NumTicks; i++) {
				int x = x0 + xsize * i / (NumTicks - 1);
				WIDGET__DrawVLine(this, x, 1, 3);
			}
		}
		/* Draw the slider itself */
		GUI.SetColor(this->Props.Color);
		WIDGET__FillRect(this, rSlider);
		GUI.SetColor(RGB_BLACK);
		WIDGET__EFFECT_DrawUpRect(this, rSlider);
		/* Draw focus */
		if (this->State & WIDGET_STATE_FOCUS) {
			GUI.SetColor(RGB_BLACK);
			WIDGET__DrawFocusRect(this, rFocus, 0);
		}
	}
	void _SliderPressed() {
		WIDGET_OrState(this, SLIDER_STATE_PRESSED);
		if (this->Status & WM_SF_ISVIS) {
			WM_NotifyParent(this, WM_NOTIFICATION_CLICKED);
		}
	}
	void _SliderReleased() {
		WIDGET_AndState(this, SLIDER_STATE_PRESSED);
		if (this->Status & WM_SF_ISVIS) {
			WM_NotifyParent(this, WM_NOTIFICATION_RELEASED);
		}
	}
	void _OnTouch(const GUI_PID_STATE *pState) {
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (pState->Pressed) {
				int x0, xsize, x, Sel, Range;
				Range = (this->Max - this->Min);
				x0 = 1 + this->Width / 2;  /* 1 pixel focus rectangle + width of actual slider */
				x = (this->State & WIDGET_STATE_VERTICAL) ? pState->y : pState->x;
				x -= x0;
				xsize = WIDGET__GetWindowSizeX(this) - 2 * x0;
				if (x <= 0) {
					Sel = this->Min;
				}
				else if (x >= xsize) {
					Sel = this->Max;
				}
				else {
					int Div;
					Div = xsize ? xsize : 1;     /* Make sure we do not divide by 0, even though xsize should never be 0 in this case anyhow */
					Sel = this->Min + ((uint32_t)Range * (uint32_t)x + Div / 2) / Div;
				}
				if (WM_IsFocussable(this)) {
					WM_SetFocus(this);
				}
				WM_SetCapture(this, 1);
				SetValue(Sel);
				if ((State & SLIDER_STATE_PRESSED) == 0) {
					_SliderPressed();
				}
			}
			else {
				/* React only if button was pressed before ... avoid problems with moving / hiding windows above (such as dropdown) */
				if (State & SLIDER_STATE_PRESSED) {
					_SliderReleased();
				}
			}
		}
	}
	char _OnKey(const WM_KEY_INFO *pInfo) {
		if (pInfo->PressedCnt > 0) {
			switch (pInfo->Key) {
				case GUI_KEY_RIGHT:
					Inc();
					return 1;
				case GUI_KEY_LEFT:
					Dec();
					return 1;
			}
		}
		return 0;
	}

	static WM_PARAM _Callback(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (SLIDER_Obj *)hWin;
		/* Let widget handle the standard messages */
		if (!WIDGET_HandleActive(pObj, MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_TOUCH:
				pObj->_OnTouch((const GUI_PID_STATE *)Data);
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const WM_KEY_INFO *)Data))
					return 0;
				break;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:
	void Dec() {
		if (v > Min) {
			v--;
			WM_Invalidate(this);
			WM_NotifyParent(this, WM_NOTIFICATION_VALUE_CHANGED);
		}
	}
	void Inc() {
		if (v < Max) {
			v++;
			WM_Invalidate(this);
			WM_NotifyParent(this, WM_NOTIFICATION_VALUE_CHANGED);
		}
	}
	void SetWidth(int Width) {
		if (Width != Width) {
			Width = Width;
			WM_Invalidate(this);
		}
	}
	void SetValue(int v) {
		/* Put in min/max range */
		if (v < Min)
			v = Min;
		if (v > Max)
			v = Max;
		if (this->v != v) {
			this->v = v;
			WM_Invalidate(this);
			WM_NotifyParent(this, WM_NOTIFICATION_VALUE_CHANGED);
		}
	}
	void SetRange(int Min, int Max) {
		if (Max < Min)
			Max = Min;
		this->Min = Min;
		this->Max = Max;
		if (v < Min) {
			v = Min;
		}
		if (v > Max) {
			v = Max;
		}
		WM_Invalidate(this);
	}
	void SetNumTicks(int NumTicks) {
		if ((NumTicks >= 0)) {
			this->NumTicks = NumTicks;
			WM_Invalidate(this);
		}
	}
	void SetBkColor(RGBC Color) {
		Props.BkColor = Color;
#if WM_SUPPORT_TRANSPARENCY
		if (Color <= RGB_WHITE) {
			WM_ClrHasTrans(this);
		}
		else {
			WM_SetHasTrans(this);
		}
#endif
		WM_Invalidate(this);
	}
	int  GetValue() {
		return v;
	}

};

SLIDER_Obj::Properties SLIDER_Obj::DefaultProps;

SLIDER_Obj *SLIDER_CreateEx(int x0, int y0, int xsize, int ysize, WM_Obj *hParent,
							  int WinFlags, int ExFlags, int Id) {
#if WM_SUPPORT_TRANSPARENCY
	WinFlags |= WM_CF_HASTRANS;
#endif
	auto pObj = (SLIDER_Obj *)WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, SLIDER_Obj::_Callback, sizeof(SLIDER_Obj) - sizeof(WM_Obj));
	if (pObj) {
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

	return pObj;
}
SLIDER_Obj *SLIDER_Create(int x0, int y0, int xsize, int ysize, WM_Obj *hParent, int Id, int WinFlags, int SpecialFlags) {
	return SLIDER_CreateEx(x0, y0, xsize, ysize, hParent, WinFlags, SpecialFlags, Id);
}
WM_Obj *SLIDER_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	return SLIDER_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
}

}
