module;

#include "GUI.h"

export module TUX.Widget.CheckBox;

import TUX.Widget;

export {

enum CHECKBOX_BI : uint8_t {
	 CHECKBOX_BI_INACTIV = 0,
	 CHECKBOX_BI_ACTIV,
	 CHECKBOX_BI_INACTIV_3STATE,
	 CHECKBOX_BI_ACTIV_3STATE
};
enum CHECKBOX_CI : uint8_t {
	 CHECKBOX_CI_INACTIV = 0,
	 CHECKBOX_CI_ACTIV
};

class CheckBox : public Widget {
	static CBITMAP abmCheckEnabled[2];
	static CBITMAP abmCheckDisabled[2];

public:
	struct Properties {
		PCFONT pFont{ GUI_DEFAULT_FONT };
		PCBITMAP apBm[4]{
			/* Inactive */	&abmCheckEnabled[0],
			/* Active */	&abmCheckEnabled[1],
			/* Inactive 3-State */	&abmCheckEnabled[2],
			/* Active 3-State */	&abmCheckEnabled[1]
		};
		RGBC aBkColorBox[2]{
			/* Inactive */	RGB_GRAYL(0x80),
			/* Active */	RGB_WHITE
		};
		RGBC BkColor{ RGB_INVALID };
		RGBC TextColor{ RGB_BLACK };
		TEXTALIGN Align{ TEXTALIGN_LEFT | TEXTALIGN_VCENTER };
		uint8_t Spacing{ 4 };
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;

	uint8_t NumStates : 2;
	uint8_t CurrentState : 6;
	String text;

	void _OnPaint() {
		auto ColorIndex = IsEnabled() ? CHECKBOX_CI_ACTIV : CHECKBOX_CI_INACTIV;
		auto EffectSize = this->EffectSize();
		/* Clear inside ... Just in case      */
		/* Fill with parents background color */
		SetBkColorPrefer(Props.BkColor);
		GUI_Clear();
		/* Get size from bitmap */
		RECT RectBox;
		RectBox.RightBottom(Props.apBm[0]->Size + 2 * EffectSize - 1);
		SetUserClipRect(&RectBox);
		/* Clear inside  ... Just in case */
		GUI.BkColor(Props.aBkColorBox[ColorIndex]);
		GUI_Clear();
		if (CurrentState)
			GUI_DrawBitmap(Props.apBm[(CurrentState - 1) * 2 + ColorIndex], EffectSize);
		/* Draw the effect arround the box */
		DrawDown(RectBox);
		SetUserClipRect(nullptr);
		/* Draw text if needed */
		if (!text) return;
		/* Draw the text */
		auto RectText = WM_GetClientRect();
		RectText.x0 += RectBox.x1 + 1 + Props.Spacing;
		GUI.SetTextMode(0);
		GUI.Color(Props.TextColor);
		GUI.Font(Props.pFont);
		GUI_DispStringInRect(text, &RectText, Props.Align);
		/* Draw focus rectangle */
		if (!(States & WIDGET_STATE_FOCUS))
			return;
		int xSizeText = GUI_GetStringSizeX(text);
		int ySizeText = Props.pFont->YSize;
		RECT RectFocus = RectText;
		switch (Props.Align & ~(TEXTALIGN_HORIZONTAL)) {
			case TEXTALIGN_VCENTER:
				RectFocus.y0 = (RectText.y1 - ySizeText) / 2;
				break;
			case TEXTALIGN_BOTTOM:
				RectFocus.y0 = RectText.y1 - ySizeText;
				break;
		}
		switch (Props.Align & ~(TEXTALIGN_VERTICAL)) {
			case TEXTALIGN_HCENTER:
				RectFocus.x0 += ((RectText.x1 - RectText.x0) - xSizeText) / 2;
				break;
			case TEXTALIGN_RIGHT:
				RectFocus.x0 += (RectText.x1 - RectText.x0) - xSizeText;
				break;
		}
		RectFocus.x1 = RectFocus.x0 + xSizeText;
		RectFocus.y1 = RectFocus.y0 + ySizeText;
		GUI.Color(RGB_BLACK);
		GUI_DrawFocusRect(RectFocus, 0);
	}
	void _OnTouch(const PID_STATE *pState) {
		int Notification = 0;
		int Hit = 0;
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (!HasCaptured()) {
				if (pState->Pressed) {
					SetCapture(1);
					CurrentState = (CurrentState + 1) % NumStates;
					Invalidate();
					Notification = WM_NOTIFICATION_CLICKED;
				}
				else {
					Hit = 1;
					Notification = WM_NOTIFICATION_RELEASED;
				}
			}
		}
		else {
			Notification = WM_NOTIFICATION_MOVED_OUT;
		}
		NotifyParent(Notification);
		if (Hit == 1) {
			GUI_DEBUG_LOG("CHECKBOX: Hit\n");
			GUI_StoreKey(GetId());
		}
	}
	char _OnKey(const WM_KEY_INFO *pInfo) {
		if (IsEnabled()) {
			if (pInfo->PressedCnt > 0) {
				switch (pInfo->Key) {
					case GUI_KEY_SPACE:
						CurrentState = (CurrentState + 1) % NumStates;
						Invalidate();
						return 1;
				}
			}
		}
		return 0;
	}

	static WM_PARAM _Callback(WObj *pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (CheckBox *)pWin;
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
			case WM_DELETE:
				GUI_DEBUG_LOG("CheckBox: _Callback(WM_DELETE)\n");
				pObj->~CheckBox();
				return 0;
		}
		return DefaultProc(pWin, MsgId, Data);
	}

private:
	static void _AdjRect(RECT &r) {
		auto EffectSize = Widget::DefaultEffect->EffectSize * 2;
		if (r.x1 <= r.x0)
			r.x1 = r.x0 + DefaultProps.apBm[0]->Size.x + EffectSize;
		if (r.y1 <= r.y0)
			r.y1 = r.y0 + DefaultProps.apBm[0]->Size.y + EffectSize;
	}
public:
	CheckBox(RECT r, WM_CF Style, WObj *pParent, uint16_t Id) :
		Widget((_AdjRect(r), r), Style, _Callback, pParent, Id, WIDGET_STATE_FOCUSSABLE),
		NumStates(2),
		CurrentState(0) {}
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *pWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new CheckBox(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			pCreateInfo->Flags, pWinParent, pCreateInfo->Id);
	}
public:

#pragma region Properties
	void Font(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		Invalidate();
	}

	void TextAlign(TEXTALIGN Align) {
		if (Props.Align == Align)
			return;
		Props.Align = Align;
		Invalidate();
	}

	void TextColor(RGBC Color) {
		if (Props.TextColor == Color)
			return;
		Props.TextColor = Color;
		Invalidate();
	}

	void BkColor(RGBC Color) {
		if (Props.BkColor == Color)
			return;
		Props.BkColor = Color;
		Invalidate();
	}

	void SetImage(CHECKBOX_BI Index, PCBITMAP pBitmap) {
		if (Index >= GUI_COUNTOF(Props.apBm))
			return;
		if (Props.apBm[Index] == pBitmap)
			return;
		Props.apBm[Index] = pBitmap;
		Invalidate();
	}

	void SetSpacing(uint8_t Spacing) {
		if (Props.Spacing == Spacing)
			return;
		Props.Spacing = Spacing;
		Invalidate();
	}

	void SetNumStates(uint8_t NumStates) {
		if (!CheckBox::DefaultProps.apBm[2])
			CheckBox::DefaultProps.apBm[2] = &abmCheckDisabled[0];
		if (!CheckBox::DefaultProps.apBm[3])
			CheckBox::DefaultProps.apBm[3] = &abmCheckDisabled[1];
		if (NumStates == 2 || NumStates == 3) {
			Props.apBm[2] = CheckBox::DefaultProps.apBm[2];
			Props.apBm[3] = CheckBox::DefaultProps.apBm[3];
			NumStates = NumStates;
		}
	}

	void SetState(uint8_t State) {
		if (NumStates < State)
			return;
		if (CurrentState == State)
			return;
		CurrentState = State;
		Invalidate();
	}

#pragma endregion

	void SetText(const char *s) {
		if (text.Set(s))
			Invalidate();
	}

	auto GetState() { return CurrentState; }
	bool IsChecked() { return CurrentState == 1; }

};

CheckBox::Properties CheckBox::DefaultProps;

}

/* Palettes */
static CLOGPALETTE _PalCheckDisabled{ RGB_GRAYL(0x10), RGB_GRAYL(0x80) };
static CLOGPALETTE _PalCheckEnabled{ RGB_BLACK, RGB_WHITE };

/* Pixel data */
static const uint8_t _pxCheckEnabled[] = {
XXXXXXXXXXXXXXXX,XXXXXX__________,
XXXXXXXXXXXXXXXX,XXXXXX__________,
XXXXXXXXXXXXXXXX,__XXXX__________,
XXXXXXXXXXXXXX__,__XXXX__________,
XXXX__XXXXXX____,__XXXX__________,
XXXX____XX______,XXXXXX__________,
XXXX__________XX,XXXXXX__________,
XXXXXX______XXXX,XXXXXX__________,
XXXXXXXX__XXXXXX,XXXXXX__________,
XXXXXXXXXXXXXXXX,XXXXXX__________,
XXXXXXXXXXXXXXXX,XXXXXX__________};
/* Bitmaps */
CBITMAP CheckBox::abmCheckEnabled[2]{
	{ { 11, 11 }, 2, 1, _pxCheckEnabled,  _PalCheckDisabled },
	{ { 11, 11 }, 2, 1, _pxCheckEnabled,  _PalCheckEnabled  }
};

/* Pixel data */
static const uint8_t _pxCheckDisabled[]{
XXXXXXXXXXXXXXXX,XXXXXX__________,
XXXXXXXXXXXXXXXX,XXXXXX__________,
XXXXXXXXXXXXXXXX,__XXXX__________,
XXXXXXXXXXXXXX__,XXXXXX__________,
XXXX__XXXXXX__XX,__XXXX__________,
XXXXXX__XX__XX__,XXXXXX__________,
XXXX__XX__XX__XX,XXXXXX__________,
XXXXXX__XX__XXXX,XXXXXX__________,
XXXXXXXX__XXXXXX,XXXXXX__________,
XXXXXXXXXXXXXXXX,XXXXXX__________,
XXXXXXXXXXXXXXXX,XXXXXX__________};
/* Bitmaps */
CBITMAP CheckBox::abmCheckDisabled[2]{
	{ { 11, 11 }, 2, 1, _pxCheckDisabled,  _PalCheckDisabled},
	{ { 11, 11 }, 2, 1, _pxCheckDisabled,  _PalCheckEnabled }
};
