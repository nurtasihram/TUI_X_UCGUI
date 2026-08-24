module;

#include "GUI_Protected.h"

export module TUX.Widget.ProgBar;

import TUX.Widget;

export {

enum PROGBAR_CI {
	 PROGBAR_CI_ACTIVE = 0,
	 PROGBAR_CI_INACT
};

class ProgBar : public Widget {

public:
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
	
private:
	Properties Props = DefaultProps;

	char *pText = nullptr;
	int16_t XOff = 0, YOff = 0;
	int16_t v = 0, Min = 0, Max = 0;

	void _FreeText() {
		GUI_ALLOC_FreePtr((void **)&pText);
	}
	int _Value2X(int v) {
		int EffectSize = this->EffectSize();
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
		GUI.SetBkColor(Props.aBkColor[Index]);
		GUI.SetColor(Props.aTextColor[Index]);
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
	void _GetTextRect(RECT *pRect, const char *pText) {
		int xSize = Rect.x1 - Rect.x0 + 1;
		int ySize = Rect.y1 - Rect.y0 + 1;
		int TextWidth = GUI_GetStringDistX(pText);
		int TextHeight = Props.pFont->SizeY();
		int EffectSize = this->EffectSize();
		switch (Props.Align & TEXTALIGN_HORIZONTAL) {
			case TEXTALIGN_HCENTER:
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
		char ac[5];
		int xPos;
		auto rClient = WM_GetClientRect();
		auto rInside = rClient / EffectSize();
		xPos = _Value2X(v);
		auto pText = _GetText(ac);
		GUI.SetFont(Props.pFont);
		RECT rText;
		_GetTextRect(&rText, pText);
		GUI.SetTextMode(DRAWMODE_TRANS);
		/* Draw left bar */
		auto r = rInside;
		r.x1 = xPos - 1;
		WM_SetUserClipRect(&r);
		_DrawPart(0, rText.x0, rText.y0, pText);
		/* Draw right bar */
		r = rInside;
		r.x0 = xPos;
		WM_SetUserClipRect(&r);
		_DrawPart(1, rText.x0, rText.y0, pText);
		WM_SetUserClipRect(nullptr);
		DrawDown(rClient);
	}
	void _Delete() {
		_FreeText();
	}

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (ProgBar *)hWin;
		/* Let widget handle the standard messages */
		if (!pObj->HandleActive(MsgId, &Data))
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
	ProgBar(RECT r, WM_CF Style, WObj *pParent, uint16_t Id) :
		Widget(r, Style, _Callback, pParent, Id, 0) {
		SetEffect(WIDGET_Effect_None); /* Standard effect for progbar: None */
	}
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new ProgBar(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			pCreateInfo->Flags, hWinParent, pCreateInfo->Id);
	}

public:

#pragma region Properties
	void SetFont(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		Invalidate();
	}
	void SetBarColor(PROGBAR_CI Index, RGBC color) {
		if (Index >= GUI_COUNTOF(Props.aBkColor))
			return;
		if (Props.aBkColor[Index] == color)
			return;
		Props.aBkColor[Index] = color;
		Invalidate();
	}
	void SetTextColor(PROGBAR_CI Index, RGBC color) {
		if (Index >= GUI_COUNTOF(Props.aTextColor))
			return;
		if (Props.aTextColor[Index] == color)
			return;
		Props.aTextColor[Index] = color;
		Invalidate();
	}
	void SetTextAlign(TEXTALIGN Align) {
		if (Props.Align == Align)
			return;
		Props.Align = Align;
		Invalidate();
	}
#pragma endregion

	void SetValue(int v) {
		/* Put v into legal range */
		if (v < Min)
			v = Min;
		if (v > Max)
			v = Max;
		if (this->v != v) {
			this->v = v;
			Invalidate();
		}
	}
	void SetText(const char *s) {
		if (GUI__SetText(&pText, s))
			Invalidate();
	}
	void SetTextPos(int XOff, int YOff) {
		this->XOff = XOff;
		this->YOff = YOff;
		Invalidate();
	}
	void SetMinMax(int Min, int Max) {
		if (Max > Min) {
			if (Max != this->Max || Min != this->Min) {
				this->Min = Min;
				this->Max = Max;
				Invalidate();
			}
		}
	}

};

ProgBar::Properties ProgBar::DefaultProps;

}
