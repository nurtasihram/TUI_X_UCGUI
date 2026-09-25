export module TUX.Widget.CheckBox;

#include "GUIConf.h"

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
			/* Inactive */ &abmCheckEnabled[0],
			/*   Active */ &abmCheckEnabled[1],
			/* Inactive 3-State */ &abmCheckEnabled[2],
			/*   Active 3-State */ &abmCheckEnabled[1]
		};
		RGBC aBkColorBox[2]{
			/* Inactive */ RGB_GRAY,
			/*   Active */ RGB_WHITE
		};
		BRUSH brush{ RGBC::Gray(0xE4), RGB_BLACK };
		TEXTALIGN Align{ TEXTALIGN_LEFT | TEXTALIGN_VCENTER };
		uint8_t Spacing{ 4 };
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;

	uint8_t NumStates : 2;
	uint8_t CurrentState : 6;
	String text;

	void _OnPaint() const {
		auto ColorIndex = IsEnabled() ? CHECKBOX_CI_ACTIV : CHECKBOX_CI_INACTIV;
		auto EffectSize = this->EffectSize();
		/* Clear inside ... Just in case      */
		/* Fill with parents background color */
		GUI.Brush(Props.brush);
		GUI.Clear();
		/* Get size from bitmap */
		RECT RectBox{ 0, Props.apBm[0]->Size + 2 * EffectSize - 1 };
		UserClip(&RectBox);
		/* Clear inside  ... Just in case */
		GUI.BkColor(Props.aBkColorBox[ColorIndex]);
		GUI.Clear();
		if (CurrentState)
			GUI_DrawBitmap(*Props.apBm[(CurrentState - 1) * 2 + ColorIndex], EffectSize);
		/* Draw the effect arround the box */
		DrawDown(RectBox);
		UserClip(nullptr);
		/* Draw text if needed */
		if (!text) return;
		/* Draw the text */
		auto rText = ClientRect();
		rText.x0 += RectBox.x1 + 1 + Props.Spacing;
		GUI.Brush(Props.brush);
		GUI.Font(Props.pFont);
		GUI_DispStringInRect(text, rText, Props.Align);
		/* Draw focus rectangle */
		if (!(States & WIDGET_STATE_FOCUS))
			return;
		auto textBound = Props.pFont->TextBound(text);
		auto rFocus = rText;
		switch (Props.Align & ~(TEXTALIGN_HORIZONTAL)) {
			case TEXTALIGN_VCENTER:
				rFocus.y0 = (rText.y1 - textBound.y) / 2;
				break;
			case TEXTALIGN_BOTTOM:
				rFocus.y0 = rText.y1 - textBound.y;
				break;
		}
		switch (Props.Align & ~(TEXTALIGN_VERTICAL)) {
			case TEXTALIGN_HCENTER:
				rFocus.x0 += ((rText.x1 - rText.x0) - textBound.x) / 2;
				break;
			case TEXTALIGN_RIGHT:
				rFocus.x0 += (rText.x1 - rText.x0) - textBound.x;
				break;
		}
		rFocus.RightBottom(rFocus.LeftTop() + textBound);
		GUI.Color(RGB_BLACK);
		GUI_DrawFocusRect(rFocus, 0);
	}
	void _OnTouch(const PID_STATE *pState) {
		int Notification = 0;
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (!HasCaptured()) {
				if (pState->Pressed) {
					SetCapture(1);
					CurrentState = (CurrentState + 1) % NumStates;
					Invalidate();
					Notification = WM_NOTIFICATION_CLICKED;
				}
				else
					Notification = WM_NOTIFICATION_RELEASED;
			}
		}
		else
			Notification = WM_NOTIFICATION_MOVED_OUT;
		NotifyParent(Notification);
	}
	bool _OnKey(const KEY_STATE *pInfo) {
		if (IsEnabled()) {
			if (pInfo->PressedCnt > 0) {
				switch (pInfo->Key) {
					case GUI_KEY_SPACE:
						CurrentState = (CurrentState + 1) % NumStates;
						Invalidate();
						return true;
				}
			}
		}
		return false;
	}

	static WM_PARAM _Callback(WObj *pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (CheckBox *)pWin;
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const KEY_STATE *)Data))
					return 0;
				break;
			case WM_DELETE:
				pObj->~CheckBox();
				return 0;
		}
		return pObj->WidgetProc(MsgId, Data);
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
	CheckBox(RECT r, WM_CF Style, WObj *pParent, uint16_t Id, const char *pText = nullptr) :
		Widget((_AdjRect(r), r), Style, _Callback, pParent, Id, WIDGET_STATE_FOCUSSABLE),
		NumStates(2),
		CurrentState(0) { text.Set(pText); }
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *pWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new CheckBox(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			pCreateInfo->Flags, pWinParent, pCreateInfo->Id, pCreateInfo->pName);
	}
public:

#pragma region Properties
	auto Font() const { return Props.pFont; }
	void Font(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		Invalidate();
	}

	auto TextAlign() const { return Props.Align; }
	void TextAlign(TEXTALIGN Align) {
		if (Props.Align == Align)
			return;
		Props.Align = Align;
		Invalidate();
	}

	auto Brush() const { return Props.brush; }
	void Brush(BRUSH brush) {
		if (Props.brush == brush)
			return;
		Props.brush = brush;
		Invalidate();
	}

	void SetImage(CHECKBOX_BI Index, PCBITMAP pBitmap) {
		if (Props.apBm[Index] == pBitmap)
			return;
		Props.apBm[Index] = pBitmap;
		Invalidate();
	}

	auto Spacing() const { return Props.Spacing; }
	void Spacing(uint8_t Spacing) {
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

#pragma endregion

	void SetText(const char *s) {
		if (text.Set(s))
			Invalidate();
	}

	bool Checked() const { return CurrentState == 1; }

	auto GetState() const { return CurrentState; }
	void SetState(uint8_t State) {
		if (NumStates < State)
			return;
		if (CurrentState == State)
			return;
		CurrentState = State;
		Invalidate();
	}
};

CheckBox::Properties CheckBox::DefaultProps;

}

/* Palettes */
static CLOGPALETTE _PalCheckDisabled{ RGBC::Gray(0x10),RGB_GRAY };
static CLOGPALETTE _PalCheckEnabled{ RGB_BLACK, RGB_WHITE };

/* Pixel data */
static const uint8_t _pxCheckEnabled[]{
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
	{ 11, 2, BPP_1, _pxCheckEnabled,  _PalCheckDisabled },
	{ 11, 2, BPP_1, _pxCheckEnabled,  _PalCheckEnabled  }
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
	{ 11, 2, BPP_1, _pxCheckDisabled,  _PalCheckDisabled },
	{ 11, 2, BPP_1, _pxCheckDisabled,  _PalCheckEnabled }
};
