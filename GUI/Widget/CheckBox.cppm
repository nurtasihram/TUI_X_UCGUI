module;

#include "WM.h"
#include "DIALOG_Intern.h" /* Req. for Create indirect data structure */

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

struct CHECKBOX_Obj : public WIDGET {
	static CBITMAP abmCheck[2];
	struct Properties {
		PCFONT pFont{ &GUI_Font13_1 };
		PCBITMAP apBm[4]{
			/* Inactive */	&abmCheck[0],
			/* Active */	&abmCheck[1],
			/* Inactive 3-State */	&abmCheck[2],
			/* Active 3-State */	&abmCheck[1]
		};
		RGBC aBkColorBox[2]{
			/* Inactive */	RGB_GRAYL(0x80),
			/* Active */	RGB_WHITE
		};
		RGBC BkColor{ RGB_INVALID_COLOR };
		RGBC TextColor{ RGB_BLACK };
		TEXTALIGN Align{ TEXTALIGN_LEFT | TEXTALIGN_VCENTER };
		uint8_t Spacing{ 4 };
		uint8_t NumStates = 2;
	} static DefaultProps;
	Properties Props;
	uint8_t NumStates;
	uint8_t CurrentState;
	char *pText;
	void _OnPaint() {
		int ColorIndex = WM_IsEnabled(this),
			EffectSize = this->pEffect->EffectSize;
		/* Clear inside ... Just in case      */
		/* Fill with parents background color */
#if WM_SUPPORT_TRANSPARENCY
		if (!WM_GetHasTrans(this))
#endif
		{
			if (this->Props.BkColor == RGB_INVALID_COLOR) {
				GUI_SetBkColor(WIDGET__GetBkColor(this));
			}
			else {
				GUI_SetBkColor(this->Props.BkColor);
			}
			GUI_Clear();
		}
		/* Get size from bitmap */
		GUI_RECT RectBox;
		RectBox.x1 = this->Props.apBm[0]->XSize - 1 + 2 * EffectSize;
		RectBox.y1 = this->Props.apBm[0]->YSize - 1 + 2 * EffectSize;
		WM_SetUserClipRect(&RectBox);
		/* Clear inside  ... Just in case */
		GUI_SetBkColor(this->Props.aBkColorBox[ColorIndex]);
		GUI_Clear();
		if (this->CurrentState)
			GUI_DrawBitmap(this->Props.apBm[(this->CurrentState - 1) * 2 + ColorIndex], EffectSize, EffectSize);
		/* Draw the effect arround the box */
		WIDGET__EFFECT_DrawDownRect(this, RectBox);
		WM_SetUserClipRect(nullptr);
		/* Draw text if needed */
		if (this->pText) {
			GUI_RECT RectText;
			/* Draw the text */
			auto s = this->pText;
			RectText = WM_GetClientRect();
			RectText.x0 += RectBox.x1 + 1 + this->Props.Spacing;
			GUI_SetTextMode(DRAWMODE_TRANS);
			GUI_SetColor(this->Props.TextColor);
			GUI_SetFont(this->Props.pFont);
			GUI_DispStringInRect(s, &RectText, this->Props.Align);
			/* Draw focus rectangle */
			if (this->State & WIDGET_STATE_FOCUS) {
				int xSizeText = GUI_GetStringDistX(s);
				int ySizeText = GUI_GetFontSizeY();
				GUI_RECT RectFocus = RectText;
				switch (this->Props.Align & ~(TEXTALIGN_HORIZONTAL)) {
					case TEXTALIGN_VCENTER:
						RectFocus.y0 = (RectText.y1 - ySizeText) / 2;
						break;
					case TEXTALIGN_BOTTOM:
						RectFocus.y0 = RectText.y1 - ySizeText;
						break;
				}
				switch (this->Props.Align & ~(TEXTALIGN_VERTICAL)) {
					case TEXTALIGN_HCENTER:
						RectFocus.x0 += ((RectText.x1 - RectText.x0) - xSizeText) / 2;
						break;
					case TEXTALIGN_RIGHT:
						RectFocus.x0 += (RectText.x1 - RectText.x0) - xSizeText;
						break;
				}
				RectFocus.x1 = RectFocus.x0 + xSizeText;
				RectFocus.y1 = RectFocus.y0 + ySizeText;
				GUI_SetColor(RGB_BLACK);
				WIDGET__DrawFocusRect(this, RectFocus, 0);
			}
		}
	}
	void _OnTouch(const GUI_PID_STATE *pState) {
		int Notification = 0;
		int Hit = 0;
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (!WM_HasCaptured(this)) {
				if (pState->Pressed) {
					WM_SetCapture(this, 1);
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
		if (WM_IsEnabled(this)) {
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

	static WM_PARAM _Callback(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (CHECKBOX_Obj *)hWin;
		/* Let widget handle the standard messages */
		if (!WIDGET_HandleActive(pObj, MsgId, &Data))
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
				pObj->_OnTouch((const GUI_PID_STATE *)Data);
				break;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}
	int GetState() {
		return CurrentState;
	}
	bool IsChecked() {
		return GetState() == 1;
	}
	void SetBkColor(RGBC Color) {
		if (Props.BkColor != Color) {
			Props.BkColor = Color;
#if WM_SUPPORT_TRANSPARENCY
			if (Color <= RGB_WHITE) {
				WM_SetTransState(this, 0);
			}
			else {
				WM_SetTransState(this, WM_CF_HASTRANS);
			}
#endif
			WM_Invalidate(this);
		}
	}
	void SetFont(PCFONT pFont) {
		if (Props.pFont != pFont) {
			Props.pFont = pFont;
			WM_Invalidate(this);
		}
	}
	void SetImage(PCBITMAP pBitmap, unsigned int Index) {
		if (Index <= GUI_COUNTOF(Props.apBm)) {
			Props.apBm[Index] = pBitmap;
		}
	}
	void SetNumStates(unsigned NumStates) {
		/* Colors */
		static const RGBC _aColorDisabled[]{ RGB_GRAYL(0x10), RGB_GRAYL(0x80) };
		static const RGBC _aColorEnabled[]{ RGB_BLACK, RGB_WHITE };

		/* Palettes */
		static const GUI_LOGPALETTE _PalCheckDisabled = {
		  2,	/* number of entries */
		  0, 	/* No transparency */
		  _aColorDisabled
		};

		static const GUI_LOGPALETTE _PalCheckEnabled = {
		  2,	/* number of entries */
		  0, 	/* No transparency */
		  _aColorEnabled
		};

		/* Pixel data */
		static const uint8_t _acCheck[] = {
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
		XXXXXXXXXXXXXXXX,XXXXXX__________
		};

		/* Bitmaps */
		static CBITMAP _abmCheck[2] = {
		  { 11, 11, 2, 1, _acCheck,  &_PalCheckDisabled},
		  { 11, 11, 2, 1, _acCheck,  &_PalCheckEnabled }
		};

		if (!CHECKBOX_Obj::DefaultProps.apBm[2])
			CHECKBOX_Obj::DefaultProps.apBm[2] = &_abmCheck[0];
		if (!CHECKBOX_Obj::DefaultProps.apBm[3])
			CHECKBOX_Obj::DefaultProps.apBm[3] = &_abmCheck[1];
		if ((NumStates == 2 || NumStates == 3)) {
			Props.apBm[2] = CHECKBOX_Obj::DefaultProps.apBm[2];
			Props.apBm[3] = CHECKBOX_Obj::DefaultProps.apBm[3];
			this->NumStates = NumStates;
		}
	}
	void SetSpacing(unsigned Spacing) {
		if ((unsigned)Props.Spacing != Spacing) {
			Props.Spacing = Spacing;
			WM_Invalidate(this);
		}
	}
	void SetState(unsigned State) {
		if (State <= (unsigned)NumStates) {
			CurrentState = State;
			WM_Invalidate(this);
		}
	}
	void SetText(const char *s) {
		if (s) {
			if (GUI__SetText(&pText, s)) {
				WM_Invalidate(this);
			}
		}
	}
	void SetTextAlign(int Align) {
		if (Props.Align != Align) {
			Props.Align = Align;
			WM_Invalidate(this);
		}
	}
	void SetTextColor(RGBC Color) {
		if (Props.TextColor != Color) {
			Props.TextColor = Color;
			WM_Invalidate(this);
		}
	}
};

CHECKBOX_Obj::Properties CHECKBOX_Obj::DefaultProps;

CHECKBOX_Obj *CHECKBOX_CreateEx(int x0, int y0, int xsize, int ysize, WM_Obj *hParent, int WinFlags, int ExFlags, int Id) {
	GUI_USE_PARA(ExFlags);
	/* Calculate size if needed */
	if ((xsize == 0) || (ysize == 0)) {
		auto EffectSize = WIDGET::DefaultEffect->EffectSize;
		if (xsize == 0)
			xsize = CHECKBOX_Obj::DefaultProps.apBm[0]->XSize + 2 * EffectSize;
		if (ysize == 0)
			ysize = CHECKBOX_Obj::DefaultProps.apBm[0]->YSize + 2 * EffectSize;
	}
#if WM_SUPPORT_TRANSPARENCY
	if (CHECKBOX_Obj::DefaultProps.BkColor == RGB_INVALID_COLOR)
		WinFlags |= WM_CF_HASTRANS;
#endif
	/* Create the window */
	auto pObj = (CHECKBOX_Obj *)WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, CHECKBOX_Obj::_Callback,
								  sizeof(CHECKBOX_Obj) - sizeof(WM_Obj));
	if (pObj) {
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE);
		/* init member variables */
		pObj->Props = CHECKBOX_Obj::DefaultProps;
		pObj->NumStates = 2; /* Default behaviour is 2 states: checked and unchecked */
	}
	else {
		GUI_DEBUG_ERROROUT_IF(pObj == 0, "CHECKBOX_Create failed")
	}
	return pObj;
}
WM_Obj *CHECKBOX_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	return CHECKBOX_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							  hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
}

}

/* Colors */
static const RGBC _aColorDisabled[] = { RGB_GRAYL(0x10), RGB_GRAYL(0x80) };
static const RGBC _aColorEnabled[] = { RGB_BLACK, RGB_WHITE };
/* Palettes */
static const GUI_LOGPALETTE _PalCheckDisabled = {
  2,	/* number of entries */
  0, 	/* No transparency */
  _aColorDisabled
};
static const GUI_LOGPALETTE _PalCheckEnabled = {
  2,	/* number of entries */
  0, 	/* No transparency */
  _aColorEnabled
};
/* Pixel data */
static const uint8_t _acCheck[] = {
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
XXXXXXXXXXXXXXXX,XXXXXX__________
};

/* Bitmaps */
CBITMAP CHECKBOX_Obj::abmCheck[2] = {
  { 11, 11, 2, 1, _acCheck,  &_PalCheckDisabled},
  { 11, 11, 2, 1, _acCheck,  &_PalCheckEnabled }
};
