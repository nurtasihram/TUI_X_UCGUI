module;

#include "GUI.h"

export module TUX.Widget.Slider;

import TUX.Widget;

export {

constexpr uint16_t SLIDER_CF_VERTICAL   = WIDGET_STATE_USER<0>;
constexpr uint16_t SLIDER_STATE_PRESSED = WIDGET_STATE_USER<1>;

class Slider : public Widget {

public:
	struct Properties {
		RGBC BkColor { RGB_INVALID };
		RGBC Color   { RGB_GRAYL(0xC0) };
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;
	
	int16_t Min = 0, Max = 100, v = 0;
	int16_t NumTicks = -1;
	int16_t Width = 8;

	void _OnPaint() {
		auto r = States & SLIDER_CF_VERTICAL ? ~GetClientRect() : GetClientRect();
		auto xsize = r.x1 - r.x0 + 1 - this->Width;
		auto x0 = r.x0 + this->Width / 2;
		auto Range = this->Max - this->Min;
		if (Range == 0)
			Range = 1;
		/* Calculate Slider position */
		auto rSlider = r;
		rSlider.y0 = 5;
		rSlider.x0 = x0 + (uint32_t)xsize * (uint32_t)(this->v - this->Min) / Range - this->Width / 2;
		rSlider.x1 = rSlider.x0 + this->Width;
		/* Calculate Slot position */
		RECT rSlot;
		rSlot.x0 = x0;
		rSlot.x1 = x0 + xsize;
		rSlot.y0 = (rSlider.y0 + rSlider.y1) / 2 - 1;
		rSlot.y1 = rSlot.y0 + 3;
		/* Calculate the ticks */
		auto NumTicks = this->NumTicks;
		if (NumTicks < 0) {
			NumTicks = Range + 1;
			if (NumTicks > (xsize / 5))
				NumTicks = 11;
		}
		/* Fill with parents background color */
		SetBkColorPrefer(Props.BkColor);
		GUI_Clear();
		GUI.Color(RGB_BLACK);
		if (States & SLIDER_CF_VERTICAL) {
			auto xSize = GetSizeX();
			rSlot = rSlot.Rotate90L(xSize);
			rSlider = rSlider.Rotate90L(xSize);
			/* Draw the ticks */
			for (int i = 0; i < NumTicks; i++) {
				int x = x0 + xsize * i / (NumTicks - 1);
				GUI_DrawHLine(x, 1, 3);
			}
		}
		else {
			/* Draw the ticks */
			for (int i = 0; i < NumTicks; i++) {
				int x = x0 + xsize * i / (NumTicks - 1);
				GUI_DrawVLine(x, 1, 3);
			}
		}
		/* Draw slot */
		DrawDown(rSlot);
		/* Draw the slider itself */
		GUI.Color(Props.Color);
		GUI_FillRect(rSlider);
		GUI.Color(RGB_BLACK);
		DrawUp(rSlider);
		/* Draw focus */
		if (States & WIDGET_STATE_FOCUS) {
			GUI.Color(RGB_BLACK);
			GUI_DrawFocusRect(GetClientRect(), 0);
		}
	}
	void _SliderPressed() {
		AddStates(SLIDER_STATE_PRESSED);
		if (this->Status & WC_VISIBLE)
			NotifyParent(WM_NOTIFICATION_CLICKED);
	}
	void _SliderReleased() {
		DelStates(SLIDER_STATE_PRESSED);
		if (this->Status & WC_VISIBLE)
			NotifyParent(WM_NOTIFICATION_RELEASED);
	}
	void _OnTouch(const PID_STATE *pState) {
		if (!pState)
			return;
		if (!pState->Pressed) {
			/* React only if button was pressed before ... avoid problems with moving / hiding windows above (such as dropdown) */
			if (States & SLIDER_STATE_PRESSED)
				_SliderReleased();
			return;
		}
		auto Range = Max - Min;
		auto x0 = 1 + Width / 2;  /* 1 pixel focus rectangle + width of actual slider */
		auto x = (States & SLIDER_CF_VERTICAL ? pState->y : pState->x) - x0;
		auto xsize = (States & SLIDER_CF_VERTICAL ? GetSizeY() : GetSizeX()) - 2 * x0;
		int Sel;
		if (x <= 0)
			Sel = Min;
		else if (x >= xsize)
			Sel = Max;
		else {
			auto Div = xsize ? xsize : 1;     /* Make sure we do not divide by 0, even though xsize should never be 0 in this case anyhow */
			Sel = Min + ((uint32_t)Range * (uint32_t)x + Div / 2) / Div;
		}
		if (IsFocussable())
			SetFocus();
		SetCapture(1);
		SetValue(Sel);
		if (!(States & SLIDER_STATE_PRESSED))
			_SliderPressed();
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

	static WM_PARAM _Callback(WObj *pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (Slider *)pWin;
		/* Let widget handle the standard messages */
		if (!pObj->HandleActive(MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const WM_KEY_INFO *)Data))
					return 0;
				break;
		}
		return DefaultProc(pWin, MsgId, Data);
	}

public:
	Slider(RECT r, WM_CF Style, WObj *pParent, uint16_t Id,
		   uint16_t ExFlags) :
		Widget(r, Style, _Callback, pParent, Id, ExFlags | WIDGET_STATE_FOCUSSABLE) {
	}
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *pWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new Slider(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			0, pWinParent, pCreateInfo->Id,
			pCreateInfo->Flags);
	}

public:

#pragma region Properties
	
	void BkColor(RGBC Color) {
		Props.BkColor = Color;
		Invalidate();
	}

#pragma endregion

	void Dec() {
		if (v > Min) {
			v--;
			Invalidate();
			NotifyParent(WM_NOTIFICATION_VALUE_CHANGED);
		}
	}
	void Inc() {
		if (v < Max) {
			v++;
			Invalidate();
			NotifyParent(WM_NOTIFICATION_VALUE_CHANGED);
		}
	}
	void SetWidth(int Width) {
		if (this->Width != Width) {
			this->Width = Width;
			Invalidate();
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
			Invalidate();
			NotifyParent(WM_NOTIFICATION_VALUE_CHANGED);
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
		Invalidate();
	}
	void SetNumTicks(int NumTicks) {
		if ((NumTicks >= 0)) {
			this->NumTicks = NumTicks;
			Invalidate();
		}
	}
	int  GetValue() {
		return v;
	}

};

Slider::Properties Slider::DefaultProps;

}
