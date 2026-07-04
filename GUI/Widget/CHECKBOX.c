#include "GUI_Protected.h"

#include "CHECKBOX.h"
#include "CHECKBOX_Private.h"

/* Define default fonts */
#define CHECKBOX_FONT_DEFAULT &GUI_Font13_1
/* Define default images */
#define CHECKBOX_IMAGE0_DEFAULT &CHECKBOX__abmCheck[0]
#define CHECKBOX_IMAGE1_DEFAULT &CHECKBOX__abmCheck[1]
/* Define widget background color */
#define CHECKBOX_BKCOLOR_DEFAULT RGB_GRAYL(0xC0)           /* Text background color */
#define CHECKBOX_SPACING_DEFAULT 4
#define CHECKBOX_TEXTCOLOR_DEFAULT RGB_BLACK
#define CHECKBOX_TEXTALIGN_DEFAULT (GUI_TA_LEFT | GUI_TA_VCENTER)
CHECKBOX_PROPS CHECKBOX__DefaultProps = {
  CHECKBOX_FONT_DEFAULT,
  CHECKBOX_BKCOLOR0_DEFAULT,
  CHECKBOX_BKCOLOR1_DEFAULT,
  CHECKBOX_BKCOLOR_DEFAULT,
  CHECKBOX_TEXTCOLOR_DEFAULT,
  CHECKBOX_TEXTALIGN_DEFAULT,
  CHECKBOX_SPACING_DEFAULT,
  CHECKBOX_IMAGE0_DEFAULT,
  CHECKBOX_IMAGE1_DEFAULT
};
static void _OnPaint(CHECKBOX_Obj *pObj) {
	GUI_RECT RectBox = { 0 };
	int ColorIndex, EffectSize;
	EffectSize = pObj->Widget.pEffect->EffectSize;
	ColorIndex = WM_IsEnabled(pObj);
	/* Clear inside ... Just in case      */
	/* Fill with parents background color */
#if WM_SUPPORT_TRANSPARENCY
	if (!WM_GetHasTrans(pObj))
#endif
	{
		if (pObj->Props.BkColor == GUI_INVALID_COLOR) {
			GUI_SetBkColor(WIDGET__GetBkColor(pObj));
		}
		else {
			GUI_SetBkColor(pObj->Props.BkColor);
		}
		GUI_Clear();
	}
	/* Get size from bitmap */
	RectBox.x1 = pObj->Props.apBm[0]->XSize - 1 + 2 * EffectSize;
	RectBox.y1 = pObj->Props.apBm[0]->YSize - 1 + 2 * EffectSize;
	WM_SetUserClipRect(&RectBox);
	/* Clear inside  ... Just in case */
	GUI_SetBkColor(pObj->Props.aBkColorBox[ColorIndex]);
	GUI_Clear();
	if (pObj->CurrentState) {
		int Index = (pObj->CurrentState - 1) * 2 + ColorIndex;
		GUI_DrawBitmap(pObj->Props.apBm[Index], EffectSize, EffectSize);
	}
	/* Draw the effect arround the box */
	WIDGET__EFFECT_DrawDownRect(&pObj->Widget, &RectBox);
	WM_SetUserClipRect(NULL);
	/* Draw text if needed */
	if (pObj->hpText) {
		const char *s;
		GUI_RECT RectText;
		/* Draw the text */
		s = (const char *)(pObj->hpText);
		WM_GetClientRect(&RectText);
		RectText.x0 += RectBox.x1 + 1 + pObj->Props.Spacing;
		GUI_SetTextMode(DRAWMODE_TRANS);
		GUI_SetColor(pObj->Props.TextColor);
		GUI_SetFont(pObj->Props.pFont);
		GUI_DispStringInRect(s, &RectText, pObj->Props.Align);
		/* Draw focus rectangle */
		if (pObj->Widget.State & WIDGET_STATE_FOCUS) {
			int xSizeText = GUI_GetStringDistX(s);
			int ySizeText = GUI_GetFontSizeY();
			GUI_RECT RectFocus = RectText;
			switch (pObj->Props.Align & ~(GUI_TA_HORIZONTAL)) {
				case GUI_TA_VCENTER:
					RectFocus.y0 = (RectText.y1 - ySizeText) / 2;
					break;
				case GUI_TA_BOTTOM:
					RectFocus.y0 = RectText.y1 - ySizeText;
					break;
			}
			switch (pObj->Props.Align & ~(GUI_TA_VERTICAL)) {
				case GUI_TA_HCENTER:
					RectFocus.x0 += ((RectText.x1 - RectText.x0) - xSizeText) / 2;
					break;
				case GUI_TA_RIGHT:
					RectFocus.x0 += (RectText.x1 - RectText.x0) - xSizeText;
					break;
			}
			RectFocus.x1 = RectFocus.x0 + xSizeText;
			RectFocus.y1 = RectFocus.y0 + ySizeText;
			GUI_SetColor(RGB_BLACK);
			WIDGET__DrawFocusRect(&pObj->Widget, &RectFocus, 0);
		}
	}
}
static void _OnTouch(CHECKBOX_Obj *pObj, const GUI_PID_STATE *pState) {
	int Notification = 0;
	int Hit = 0;
	if (pState) {  /* Something happened in our area (pressed or released) */
		if (!WM_HasCaptured(pObj)) {
			if (pState->Pressed) {
				WM_SetCapture(pObj, 1);
				pObj->CurrentState = (pObj->CurrentState + 1) % pObj->NumStates;
				WM_Invalidate(pObj);
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
	WM_NotifyParent(pObj, Notification);
	if (Hit == 1) {
		GUI_DEBUG_LOG("CHECKBOX: Hit\n");
		GUI_StoreKey(pObj->Widget.Id);
	}
}
static char _OnKey(CHECKBOX_Obj *pObj, const WM_KEY_INFO *pInfo) {
	if (WM_IsEnabled(pObj)) {
		if (pInfo->PressedCnt > 0) {
			switch (pInfo->Key) {
				case GUI_KEY_SPACE:
					pObj->CurrentState = (pObj->CurrentState + 1) % pObj->NumStates;
					WM_Invalidate(pObj);
					return 1;
			}
		}
	}
	return 0;
}
static WM_PARAM _CHECKBOX_Callback(WM_HWIN hWin, int MsgId, WM_PARAM Data, WM_MESSAGE *pMsg) {
	CHECKBOX_Obj *pObj = hWin;
	/* Let widget handle the standard messages */
	if (!WIDGET_HandleActive(pObj, MsgId, &Data))
		return Data;
	switch (MsgId) {
		case WM_KEY:
			if (_OnKey(pObj, (const WM_KEY_INFO *)Data))
				return 0;
			break;
		case WM_PAINT:
			_OnPaint(pObj);
			return 0;
		case WM_TOUCH:
			_OnTouch(pObj, (const GUI_PID_STATE *)Data);
			break;
	}
	return WM_DefaultProc(hWin, MsgId, Data, pMsg);
}
/* Note: the parameters to a create function may vary.
		 Some widgets may have multiple create functions */
CHECKBOX_Handle CHECKBOX_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
								  int WinFlags, int ExFlags, int Id) {
	CHECKBOX_Handle hObj;
	GUI_USE_PARA(ExFlags);

	/* Calculate size if needed */
	if ((xsize == 0) || (ysize == 0)) {
		int EffectSize;
		EffectSize = WIDGET_GetDefaultEffect()->EffectSize;
		if (xsize == 0) {
			xsize = CHECKBOX__DefaultProps.apBm[0]->XSize + 2 * EffectSize;
		}
		if (ysize == 0) {
			ysize = CHECKBOX__DefaultProps.apBm[0]->YSize + 2 * EffectSize;
		}
	}
#if WM_SUPPORT_TRANSPARENCY
	if (CHECKBOX__DefaultProps.BkColor == GUI_INVALID_COLOR) {
		WinFlags |= WM_CF_HASTRANS;
	}
#endif
	/* Create the window */
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, _CHECKBOX_Callback,
								  sizeof(CHECKBOX_Obj) - sizeof(WM_Obj));
	if (hObj) {
		CHECKBOX_Obj *pObj = (hObj);
		/* init widget specific variables */
		WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
		/* init member variables */
		pObj->Props = CHECKBOX__DefaultProps;
		pObj->NumStates = 2; /* Default behaviour is 2 states: checked and unchecked */
	}
	else {
		GUI_DEBUG_ERROROUT_IF(hObj == 0, "CHECKBOX_Create failed")
	}

	return hObj;
}

CHECKBOX_Handle CHECKBOX_Create(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags) {
	return CHECKBOX_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, Id);
}

CHECKBOX_Handle CHECKBOX_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	CHECKBOX_Handle  hThis;
	GUI_USE_PARA(cb);
	hThis = CHECKBOX_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							  hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
	return hThis;
}

void CHECKBOX_SetDefaultSpacing(int Spacing) {
	CHECKBOX__DefaultProps.Spacing = Spacing;
}
void CHECKBOX_SetDefaultTextColor(RGB_COLOR Color) {
	CHECKBOX__DefaultProps.TextColor = Color;
}
void CHECKBOX_SetDefaultBkColor(RGB_COLOR Color) {
	CHECKBOX__DefaultProps.BkColor = Color;
}
void CHECKBOX_SetDefaultFont(const GUI_FONT  *pFont) {
	CHECKBOX__DefaultProps.pFont = pFont;
}
void CHECKBOX_SetDefaultAlign(int Align) {
	CHECKBOX__DefaultProps.Align = Align;
}
int CHECKBOX_GetDefaultSpacing(void) {
	return CHECKBOX__DefaultProps.Spacing;
}
RGB_COLOR CHECKBOX_GetDefaultTextColor(void) {
	return CHECKBOX__DefaultProps.TextColor;
}
RGB_COLOR CHECKBOX_GetDefaultBkColor(void) {
	return CHECKBOX__DefaultProps.BkColor;
}
const GUI_FONT  *CHECKBOX_GetDefaultFont(void) {
	return CHECKBOX__DefaultProps.pFont;
}
int CHECKBOX_GetDefaultAlign(void) {
	return CHECKBOX__DefaultProps.Align;
}

int CHECKBOX_GetState(CHECKBOX_Handle hObj) {
	int Result = 0;
	CHECKBOX_Obj *pObj;
	if (hObj) {
		pObj = (hObj);
		Result = pObj->CurrentState;
	}
	return Result;
}


int CHECKBOX_IsChecked(CHECKBOX_Handle hObj) {
	return (CHECKBOX_GetState(hObj) == 1) ? 1 : 0;
}

void CHECKBOX_SetBkColor(CHECKBOX_Handle hObj, RGB_COLOR Color) {
	if (hObj) {
		CHECKBOX_Obj *pObj;
		pObj = (hObj);
		if (Color != pObj->Props.BkColor) {
			pObj->Props.BkColor = Color;
#if WM_SUPPORT_TRANSPARENCY
			if (Color <= RGB_WHITE) {
				WM_SetTransState(hObj, 0);
			}
			else {
				WM_SetTransState(hObj, WM_CF_HASTRANS);
			}
#endif
			WM_Invalidate(hObj);
		}
	}
}

void CHECKBOX_SetDefaultImage(const GUI_BITMAP *pBitmap, unsigned int Index) {
	if (Index <= GUI_COUNTOF(CHECKBOX__DefaultProps.apBm)) {
		CHECKBOX__DefaultProps.apBm[Index] = pBitmap;
	}
}

void CHECKBOX_SetFont(CHECKBOX_Handle hObj, const GUI_FONT  *pFont) {
	CHECKBOX_Obj *pObj;
	if (hObj) {
		pObj = (hObj);
		if (pObj->Props.pFont != pFont) {
			pObj->Props.pFont = pFont;
			WM_Invalidate(hObj);
		}
	}
}

void CHECKBOX_SetImage(CHECKBOX_Handle hObj, const GUI_BITMAP *pBitmap, unsigned int Index) {
	if (hObj) {
		CHECKBOX_Obj *pObj;
		pObj = (hObj);
		if (Index <= GUI_COUNTOF(pObj->Props.apBm)) {
			pObj->Props.apBm[Index] = pBitmap;
		}
	}
}
void CHECKBOX_SetNumStates(CHECKBOX_Handle hObj, unsigned NumStates) {
	/* Colors */
	static const RGB_COLOR _aColorDisabled[] = { CHECKBOX_FGCOLOR0_DEFAULT, CHECKBOX_BKCOLOR0_DEFAULT };
	static const RGB_COLOR _aColorEnabled[] = { CHECKBOX_FGCOLOR1_DEFAULT, CHECKBOX_BKCOLOR1_DEFAULT };

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
	static const GUI_BITMAP _abmCheck[2] = {
	  { 11, 11, 2, 1, _acCheck,  &_PalCheckDisabled},
	  { 11, 11, 2, 1, _acCheck,  &_PalCheckEnabled }
	};

	CHECKBOX_Obj *pObj;
	if (!CHECKBOX__DefaultProps.apBm[2]) {
		CHECKBOX_SetDefaultImage(&_abmCheck[0], 2);
	}
	if (!CHECKBOX__DefaultProps.apBm[3]) {
		CHECKBOX_SetDefaultImage(&_abmCheck[1], 3);
	}
	if (hObj && ((NumStates == 2) || (NumStates == 3))) {
		pObj = (hObj);
		pObj->Props.apBm[2] = CHECKBOX__DefaultProps.apBm[2];
		pObj->Props.apBm[3] = CHECKBOX__DefaultProps.apBm[3];
		pObj->NumStates = NumStates;
	}
}

void CHECKBOX_SetSpacing(CHECKBOX_Handle hObj, unsigned Spacing) {
	CHECKBOX_Obj *pObj;
	if (hObj) {
		pObj = (hObj);
		if ((unsigned)pObj->Props.Spacing != Spacing) {
			pObj->Props.Spacing = Spacing;
			WM_Invalidate(hObj);
		}
	}
}

void CHECKBOX_SetState(CHECKBOX_Handle hObj, unsigned State) {
	CHECKBOX_Obj *pObj;
	if (hObj) {
		pObj = (hObj);
		if (State <= (unsigned)pObj->NumStates) {
			pObj->CurrentState = State;
			WM_Invalidate(hObj);
		}
	}
}

void CHECKBOX_SetText(CHECKBOX_Handle hObj, const char *s) {
	CHECKBOX_Obj *pObj;
	if (hObj && s) {
		pObj = (hObj);
		if (GUI__SetText(&pObj->hpText, s)) {
			WM_Invalidate(hObj);
		}
	}
}

void CHECKBOX_SetTextAlign(CHECKBOX_Handle hObj, int Align) {
	CHECKBOX_Obj *pObj;
	if (hObj) {
		pObj = (hObj);
		if (pObj->Props.Align != Align) {
			pObj->Props.Align = Align;
			WM_Invalidate(hObj);
		}
	}
}

void CHECKBOX_SetTextColor(CHECKBOX_Handle hObj, RGB_COLOR Color) {
	if (hObj) {
		CHECKBOX_Obj *pObj;
		pObj = (hObj);
		if (pObj->Props.TextColor != Color) {
			pObj->Props.TextColor = Color;
			WM_Invalidate(hObj);
		}
	}
}


/* Colors */
static const RGB_COLOR _aColorDisabled[] = { CHECKBOX_FGCOLOR0_DEFAULT, CHECKBOX_BKCOLOR0_DEFAULT };
static const RGB_COLOR _aColorEnabled[] = { CHECKBOX_FGCOLOR1_DEFAULT, CHECKBOX_BKCOLOR1_DEFAULT };
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
const GUI_BITMAP CHECKBOX__abmCheck[2] = {
  { 11, 11, 2, 1, _acCheck,  &_PalCheckDisabled},
  { 11, 11, 2, 1, _acCheck,  &_PalCheckEnabled }
};
