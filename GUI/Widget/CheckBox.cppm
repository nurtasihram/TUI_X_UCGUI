module;

#include "GUI_Protected.h"

export module TUX.Widget.CheckBox;

import TUX.Widget;

export {

enum CHECKBOX_BI {
	 CHECKBOX_BI_INACTIV = 0,
	 CHECKBOX_BI_ACTIV,
	 CHECKBOX_BI_INACTIV_3STATE,
	 CHECKBOX_BI_ACTIV_3STATE
};
enum CHECKBOX_CI {
	 CHECKBOX_CI_INACTIV = 0,
	 CHECKBOX_CI_ACTIV
};

class CheckBox : public WIDGET {
	static CBITMAP abmCheckEnabled[2];
	static CBITMAP abmCheckDisabled[2];

public:
	struct Properties {
		PCFONT pFont{ &FontProp13_1 };
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
		RGBC BkColor{ RGB_INVALID_COLOR };
		RGBC TextColor{ RGB_BLACK };
		TEXTALIGN Align{ TEXTALIGN_LEFT | TEXTALIGN_VCENTER };
		uint8_t Spacing{ 4 };
		uint8_t NumStates{ 2 };
	} static DefaultProps;
	
private:
	Properties Props;

	uint8_t NumStates;
	uint8_t CurrentState;
	char *pText;

	void _OnPaint() {
		int ColorIndex = IsEnabled(),
			EffectSize = this->EffectSize();
		/* Clear inside ... Just in case      */
		/* Fill with parents background color */
		SetBkColorPrefer(Props.BkColor);
		GUI_Clear();
		/* Get size from bitmap */
		RECT RectBox;
		RectBox.x1 = Props.apBm[0]->XSize - 1 + 2 * EffectSize;
		RectBox.y1 = Props.apBm[0]->YSize - 1 + 2 * EffectSize;
		WM_SetUserClipRect(&RectBox);
		/* Clear inside  ... Just in case */
		GUI.SetBkColor(Props.aBkColorBox[ColorIndex]);
		GUI_Clear();
		if (this->CurrentState)
			GUI_DrawBitmap(Props.apBm[(this->CurrentState - 1) * 2 + ColorIndex], EffectSize, EffectSize);
		/* Draw the effect arround the box */
		DrawDown(RectBox);
		WM_SetUserClipRect(nullptr);
		/* Draw text if needed */
		if (this->pText) {
			/* Draw the text */
			auto s = this->pText;
			auto RectText = WM_GetClientRect();
			RectText.x0 += RectBox.x1 + 1 + Props.Spacing;
			GUI.SetTextMode(0);
			GUI.SetColor(Props.TextColor);
			GUI.SetFont(Props.pFont);
			GUI_DispStringInRect(s, &RectText, Props.Align);
			/* Draw focus rectangle */
			if (this->State & WIDGET_STATE_FOCUS) {
				int xSizeText = GUI_GetStringDistX(s);
				int ySizeText = Props.pFont->SizeY();
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
				GUI.SetColor(RGB_BLACK);
				GUI_DrawFocusRect(RectFocus, 0);
			}
		}
	}
	void _OnTouch(const PID_STATE *pState) {
		int Notification = 0;
		int Hit = 0;
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (!HasCaptured()) {
				if (pState->Pressed) {
					SetCapture(1);
					this->CurrentState = (this->CurrentState + 1) % this->NumStates;
					WM_Invalidate(this);
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
		WM_NotifyParent(this, Notification);
		if (Hit == 1) {
			GUI_DEBUG_LOG("CHECKBOX: Hit\n");
			GUI_StoreKey(this->Id);
		}
	}
	char _OnKey(const WM_KEY_INFO *pInfo) {
		if (IsEnabled()) {
			if (pInfo->PressedCnt > 0) {
				switch (pInfo->Key) {
					case GUI_KEY_SPACE:
						this->CurrentState = (this->CurrentState + 1) % this->NumStates;
						WM_Invalidate(this);
						return 1;
				}
			}
		}
		return 0;
	}

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (CheckBox *)hWin;
		/* Let widget handle the standard messages */
		if (!pObj->HandleActive(MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_KEY:
				if (pObj->_OnKey((const WM_KEY_INFO *)Data))
					return 0;
				break;
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
				break;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:

	static CheckBox *Create(int x0, int y0, int xsize, int ysize,
							WObj *hParent, int WinFlags, int ExFlags, int Id) {
		/* Calculate size if needed */
		if (!(xsize | ysize)) {
			auto EffectSize = WIDGET::DefaultEffect->EffectSize;
			if (!xsize)
				xsize = CheckBox::DefaultProps.apBm[0]->XSize + 2 * EffectSize;
			if (!ysize)
				ysize = CheckBox::DefaultProps.apBm[0]->YSize + 2 * EffectSize;
		}
#if WM_SUPPORT_TRANSPARENCY
		if (CheckBox::DefaultProps.BkColor == RGB_INVALID_COLOR)
			WinFlags |= WC_HASTRANS;
#endif
		/* Create the window */
		auto pObj = (CheckBox *)WM_CreateWindowAsChild(
			x0, y0, xsize, ysize,
			hParent, WinFlags, CheckBox::_Callback,
			sizeof(CheckBox) - sizeof(WObj));
		if (!pObj) {
			GUI_DEBUG_ERROROUT_IF(pObj == 0, "CheckBox create failed");
			return nullptr;
		}
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE);
		/* init member variables */
		pObj->Props = CheckBox::DefaultProps;
		pObj->NumStates = 2; /* Default behaviour is 2 states: checked and unchecked */
		return pObj;
	}
	static WIDGET *CreateIndirect(const WIDGET_CREATE_INFO *pCreateInfo, WObj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return Create(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
					  hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
	}

public:

#pragma region Properties
	void SetFont(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		WM_Invalidate(this);
	}

	void SetTextAlign(TEXTALIGN Align) {
		if (Props.Align == Align)
			return;
		Props.Align = Align;
		WM_Invalidate(this);
	}

	void SetTextColor(RGBC Color) {
		if (Props.TextColor == Color)
			return;
		Props.TextColor = Color;
		WM_Invalidate(this);
	}

	void SetBkColor(RGBC Color) {
		if (Props.BkColor == Color)
			return;
		Props.BkColor = Color;
		WM_Invalidate(this);
	}

	void SetImage(PCBITMAP pBitmap, CHECKBOX_BI Index) {
		if (Index >= GUI_COUNTOF(Props.apBm))
			return;
		if (Props.apBm[Index] == pBitmap)
			return;
		Props.apBm[Index] = pBitmap;
		WM_Invalidate(this);
	}

	void SetSpacing(unsigned Spacing) {
		if (Props.Spacing == Spacing)
			return;
		Props.Spacing = Spacing;
		WM_Invalidate(this);
	}

	void SetNumStates(uint8_t NumStates) {
		if (!CheckBox::DefaultProps.apBm[2])
			CheckBox::DefaultProps.apBm[2] = &abmCheckDisabled[0];
		if (!CheckBox::DefaultProps.apBm[3])
			CheckBox::DefaultProps.apBm[3] = &abmCheckDisabled[1];
		if (NumStates == 2 || NumStates == 3) {
			Props.apBm[2] = CheckBox::DefaultProps.apBm[2];
			Props.apBm[3] = CheckBox::DefaultProps.apBm[3];
			this->NumStates = NumStates;
		}
	}

	void SetState(uint8_t State) {
		if (NumStates < State)
			return;
		if (CurrentState == State)
			return;
		CurrentState = State;
		WM_Invalidate(this);
	}

#pragma endregion

	void SetText(const char *s) {
		if (GUI__SetText(&pText, s))
			WM_Invalidate(this);
	}

	auto GetState() { return CurrentState; }
	bool IsChecked() { return CurrentState == 1; }

};

CheckBox::Properties CheckBox::DefaultProps;

}

/* Colors */
static const RGBC _aColorDisabled[]{ RGB_GRAYL(0x10), RGB_GRAYL(0x80) };
static const RGBC _aColorEnabled[]{ RGB_BLACK, RGB_WHITE };
/* Palettes */
static const GUI_LOGPALETTE _PalCheckDisabled{ 2, 0, _aColorDisabled };
static const GUI_LOGPALETTE _PalCheckEnabled{ 2, 0, _aColorEnabled };

/* Pixel data */
static const uint8_t _acCheckEnabled[] = {
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
	{ 11, 11, 2, 1, _acCheckEnabled,  &_PalCheckDisabled },
	{ 11, 11, 2, 1, _acCheckEnabled,  &_PalCheckEnabled  }
};

/* Pixel data */
static const uint8_t _acCheckDisabled[]{
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
	{ 11, 11, 2, 1, _acCheckDisabled,  &_PalCheckDisabled},
	{ 11, 11, 2, 1, _acCheckDisabled,  &_PalCheckEnabled }
};
