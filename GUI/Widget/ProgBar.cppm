module;

#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */

export module TUX.Widget.ProgBar;

import TUX.Widget;

export {

struct PROGBAR_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &FontProp13_1 };
		RGBC aBkColor[2]{
			/* Active */	RGB_DARKBLUE,
			/* Inactive */	RGB_GRAYL(0x55)
		};
		RGBC aTextColor[2]{
			/* Active */	RGB_WHITE,
			/* Inactive */	RGB_BLACK
		};
		TEXTALIGN Align{ TEXTALIGN_HCENTER };
	} static DefaultProps;
	Properties Props;
	char *pText;
	int16_t XOff, YOff;
	int16_t v, Min, Max;

	void _FreeText() {
		GUI_ALLOC_FreePtr((void **)&pText);
	}
	int _Value2X(int v) {
		int EffectSize = pEffect->EffectSize;
		int xSize = Rect.x1 - Rect.x0 + 1;
		if (v < Min) {
			v = Min;
		}
		if (v > Max) {
			v = Max;
		}
		return EffectSize + ((xSize - 2 * EffectSize) * (int32_t)(v - Min)) / (Max - Min);
	}
	void _DrawPart(int Index, int xText, int yText, const char *pText) {
		GUI_SetBkColor(Props.aBkColor[Index]);
		GUI_SetColor(Props.aTextColor[Index]);
		GUI_Clear();
		GUI_GotoXY(xText, yText);
		GUI_DispString(pText);
	}
	const char *_GetText(char *pBuffer) {
		char *pText;
		uint8_t value;
		if (this->pText) {
			pText = this->pText;
		}
		else {
			pText = pBuffer;
			value = 100 * (v - Min) / (Max - Min);
			if (value == 100) {
				*pBuffer++ = '1';
				*pBuffer++ = '0';
				*pBuffer++ = '0';
			}
			else {
				if (value >= 10) {
					*pBuffer++ = '0' + value / 10;
					value %= 10;
				}
				*pBuffer++ = '0' + value;
			}
			*pBuffer++ = '%';
			*pBuffer = 0;
		}
		return (const char *)pText;
	}
	void _GetTextRect(GUI_RECT *pRect, const char *pText) {
		int xSize = Rect.x1 - Rect.x0 + 1;
		int ySize = Rect.y1 - Rect.y0 + 1;
		int TextWidth = GUI_GetStringDistX(pText);
		int TextHeight = GUI_GetFontSizeY();
		int EffectSize = pEffect->EffectSize;
		switch (Props.Align & TEXTALIGN_HORIZONTAL) {
			case TEXTALIGN_CENTER:
				pRect->x0 = (xSize - TextWidth) / 2;
				break;
			case TEXTALIGN_RIGHT:
				pRect->x0 = xSize - TextWidth - 1 - EffectSize;
				break;
			default:
				pRect->x0 = EffectSize;
		}
		pRect->y0 = (ySize - TextHeight) / 2;
		pRect->x0 += XOff;
		pRect->y0 += YOff;
		pRect->x1 = pRect->x0 + TextWidth - 1;
		pRect->y1 = pRect->y0 + TextHeight - 1;
	}
	void _OnPaint() {
		GUI_RECT r, rInside, rText;
		const char *pText;
		char ac[5];
		int xPos;
		auto rClient = WM_GetClientRect();
		rInside = rClient - pEffect->EffectSize;
		xPos = _Value2X(v);
		pText = _GetText(ac);
		GUI_SetFont(Props.pFont);
		_GetTextRect(&rText, pText);
		GUI_SetTextMode(DRAWMODE_TRANS);
		/* Draw left bar */
		r = rInside;
		r.x1 = xPos - 1;
		WM_SetUserClipRect(&r);
		_DrawPart(0, rText.x0, rText.y0, pText);
		/* Draw right bar */
		r = rInside;
		r.x0 = xPos;
		WM_SetUserClipRect(&r);
		_DrawPart(1, rText.x0, rText.y0, pText);
		WM_SetUserClipRect(nullptr);
		WIDGET__EFFECT_DrawDownRect(this, rClient);
	}
	void _Delete() {
		_FreeText();
	}

	static WM_PARAM _Callback(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (PROGBAR_Obj *)hWin;
		/* Let widget handle the standard messages */
		if (!WIDGET_HandleActive(pObj, MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_DELETE:
				pObj->_Delete();
				return 0;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:
	void SetValue(int v) {
		/* Put v into legal range */
		if (v < Min)
			v = Min;
		if (v > Max)
			v = Max;
		if (this->v != v) {
			this->v = v;
			WM_Invalidate(this);
		}
	}
	void SetFont(PCFONT pfont) {
		Props.pFont = pfont;
		WM_Invalidate(this);
	}
	void SetBarColor(unsigned int Index, RGBC color) {
		if (Index < GUI_COUNTOF(Props.aBkColor)) {
			Props.aBkColor[Index] = color;
			WM_Invalidate(this);
		}
	}
	void SetTextColor(unsigned int Index, RGBC color) {
		if (Index < GUI_COUNTOF(Props.aTextColor)) {
			Props.aTextColor[Index] = color;
			WM_Invalidate(this);
		}
	}
	void SetText(const char *s) {
		PCFONT pOldFont;
		GUI_RECT r1;
		char acBuffer[5];

		pOldFont = GUI_SetFont(Props.pFont);
		_GetTextRect(&r1, _GetText(acBuffer));
		if (GUI__SetText(&pText, s)) {
			GUI_RECT r2;
			_GetTextRect(&r2, _GetText(acBuffer));
			r1 |= r2;
			WM_InvalidateRect(this, &r1);
		}
		GUI_SetFont(pOldFont);
	}
	void SetTextAlign(int Align) {
		Props.Align = Align;
		WM_Invalidate(this);
	}
	void SetTextPos(int XOff, int YOff) {
		this->XOff = XOff;
		this->YOff = YOff;
		WM_Invalidate(this);
	}
	void SetMinMax(int Min, int Max) {
		if (Max > Min) {
			if (Max != this->Max || Min != this->Min) {
				this->Min = Min;
				this->Max = Max;
				WM_Invalidate(this);
			}
		}
	}

};

PROGBAR_Obj::Properties PROGBAR_Obj::DefaultProps;

PROGBAR_Obj *PROGBAR_CreateEx(int x0, int y0, int xsize, int ysize, WM_Obj *hParent,
								int WinFlags, int ExFlags, int Id) {
	auto pObj = (PROGBAR_Obj *)WM_CreateWindowAsChild(
		x0, y0, xsize, ysize, hParent, WinFlags, PROGBAR_Obj::_Callback,
		sizeof(PROGBAR_Obj) - sizeof(WM_Obj));
	if (pObj) {
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, 0);
		WIDGET_SetEffect(pObj, WIDGET_Effect_None); /* Standard effect for progbar: None */
		/* init member variables */
		pObj->Props = PROGBAR_Obj::DefaultProps;
		pObj->Max = 100;
		pObj->Min = 0;
	}
	return pObj;
}
PROGBAR_Obj *PROGBAR_Create(int x0, int y0, int xsize, int ysize, int Flags) {
	return PROGBAR_CreateEx(x0, y0, xsize, ysize, 0, Flags, 0, 0);
}
PROGBAR_Obj *PROGBAR_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_Obj *hParent, int Id, int Flags) {
	return PROGBAR_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, Id);
}
WM_Obj *PROGBAR_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	return PROGBAR_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							 hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
}

}
