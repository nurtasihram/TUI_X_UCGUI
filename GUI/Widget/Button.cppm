module;

#include "GUI_Protected.h"

export module TUX.Widget.Button;

import TUX.Widget;

#define BUTTON_REACT_ON_LEVEL 0

export {

constexpr uint16_t BUTTON_STATE_FOCUS       = WIDGET_STATE_FOCUS;
constexpr uint16_t BUTTON_STATE_PRESSED     = WIDGET_STATE_USER<0>;
constexpr uint16_t BUTTON_STATE_HASFOCUS    = 0;

enum BUTTON_BI {
	 BUTTON_BI_UNPRESSED = 0,
	 BUTTON_BI_PRESSED,
	 BUTTON_BI_DISABLED
};
enum BUTTON_CI {
	 BUTTON_CI_UNPRESSED = 0,
	 BUTTON_CI_PRESSED,
	 BUTTON_CI_DISABLED
};

class Button : public Widget {

public:
	struct Properties {
		PCFONT pFont{ &FontProp13_1 };
		RGBC aTextColor[3]{
			/* Unpressed */	RGB_BLACK,
			/* Pressed */	RGB_BLACK,
			/* Disabled */	RGB_DARKGRAY
		};
		RGBC aBkColor[3]{
			/* Unpressed */	RGB_GRAYL(0xD0),
			/* Pressed */	RGB_WHITE,
			/* Disabled */	RGB_LIGHTGRAY
		};
		TEXTALIGN Align{ TEXTALIGN_HCENTER | TEXTALIGN_VCENTER };
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;

	String text;
	GUI_DRAW *aDrawObj[3]{ nullptr };

	~Button() {
		GUI_ALLOC_FreePtr((void **)&aDrawObj[0]);
		GUI_ALLOC_FreePtr((void **)&aDrawObj[1]);
	}

	void _OnPaint() {
		bool IsPressed = GetStates() & BUTTON_STATE_PRESSED;
		int ColorIndex = (IsEnabled()) ? IsPressed : 2;
		GUI.Font(Props.pFont);
		auto rClient = WM_GetClientRect();
		auto rInside = rClient;
		auto EffectSize = this->EffectSize();
		if (IsPressed)
			DrawDown();
		else
			DrawUp();
		rInside /= EffectSize;
		/* Draw background */
		GUI.BkColor(Props.aBkColor[ColorIndex]);
		GUI.Color(Props.aTextColor[ColorIndex]);
		SetUserClipRect(&rInside);
		GUI_Clear();
		/* Draw bitmap.
		   If we have only one, we will use it.
		   If we have to we will use the second one (Index 1) for the pressed state
		*/
		auto Index = ColorIndex < 2 ? 
			aDrawObj[BUTTON_BI_PRESSED] && IsPressed ? BUTTON_BI_PRESSED : BUTTON_BI_UNPRESSED :
			aDrawObj[BUTTON_BI_DISABLED] ? BUTTON_BI_DISABLED : BUTTON_BI_UNPRESSED;
		if (auto pDraw = aDrawObj[Index])
			pDraw->Draw(GetInsideRect());
		/* Draw the actual button (background and text) */
		if (!IsPressed)
			rInside -= EffectSize / 2;
		GUI.SetTextMode(DRAWMODE_TRANS);
		GUI_DispStringInRect(text, &rInside, Props.Align);
		SetUserClipRect(nullptr);
		/* Draw focus */
		if (GetStates() & BUTTON_STATE_FOCUS) {
			GUI.Color(RGB_BLACK);
			GUI_DrawFocusRect(rClient, EffectSize + 1);
		}
	}
	void _ButtonPressed() {
		AddStates(BUTTON_STATE_PRESSED);
		if (this->Status & WC_VISIBLE)
			NotifyParent(WM_NOTIFICATION_CLICKED);
	}
	void _ButtonReleased(int Notification) {
		DelStates(BUTTON_STATE_PRESSED);
		if (this->Status & WC_VISIBLE)
			NotifyParent(Notification);
		if (Notification == WM_NOTIFICATION_RELEASED) {
			GUI_DEBUG_LOG("BUTTON: Hit\n");
			GUI_StoreKey(GetId());
		}
	}
	void _OnTouch(const PID_STATE *pState) {
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (pState->Pressed) {
				if (!(GetStates() & BUTTON_STATE_PRESSED))
					_ButtonPressed();
				SetCapture(1);
			}
			/* React only if button was pressed before ... avoid problems with moving / hiding windows above (such as dropdown) */
			else if (GetStates() & BUTTON_STATE_PRESSED)
				_ButtonReleased(
					GetClientRect() <= *pState ? WM_NOTIFICATION_RELEASED :
					WM_NOTIFICATION_MOVED_OUT);
		}
		else
			_ButtonReleased(WM_NOTIFICATION_MOVED_OUT);
	}
	bool _OnKey(const WM_KEY_INFO *pInfo) {
		switch (pInfo->Key) {
			case ' ':
				if (pInfo->PressedCnt > 0) /* Key pressed? */
					_ButtonPressed();
				else
					_ButtonReleased(WM_NOTIFICATION_RELEASED);
				return true;
		}
		return false;
	}
#if BUTTON_REACT_ON_LEVEL
	void _OnPidStateChange(const PID_CHANGED_INFO *pState) {
		if (pState->StatePrev == 0 && pState->State == 1) {
			if (!(GetStates() & BUTTON_STATE_PRESSED))
				_ButtonPressed();
		}
		else if (pState->StatePrev == 1 && pState->State == 0)
			if (GetStates() & BUTTON_STATE_PRESSED)
				_ButtonReleased(WM_NOTIFICATION_RELEASED);
	}
#endif

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (Button *)hWin;
		/* Let widget handle the standard messages */
		if (!pObj->HandleActive(MsgId, &Data))
			return Data;
		switch (MsgId) {
#if BUTTON_REACT_ON_LEVEL
			case WM_PID_STATE_CHANGED:
				pObj->_OnPidStateChange((const PID_CHANGED_INFO *)Data);
				return 0; /* Message handled. Do not call WM_DefaultProc, because the window may have been destroyed */
#endif
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
				return 0; /* Message handled. Do not call WM_DefaultProc, because the window may have been destroyed */
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_DELETE:
				GUI_DEBUG_LOG("Button: _Callback(WM_DELETE)\n");
				pObj->~Button();
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const WM_KEY_INFO *)Data))
					return 0;
				break;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:
	Button(RECT r, WM_CF Style, WObj *pParent, uint16_t Id) :
		Widget(r, Style, _Callback, pParent, Id, WIDGET_STATE_FOCUSSABLE) {}
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
		auto pThis = new Button(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			pCreateInfo->Flags, hWinParent, pCreateInfo->Id);
		pThis->SetText(pCreateInfo->pName);
		return pThis;
	}

public:

#pragma region Properties

	UCFONT Font() const { return *Props.pFont; }
	void Font(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		Invalidate();
	}

	RGBC BkColor(BUTTON_CI Index) {
		if (Index > 2)
			return RGB_INVALID_COLOR;
		return Props.aBkColor[Index];
	}
	void BkColor(BUTTON_CI Index, RGBC Color) {
		if (Index > 2)
			return;
		Props.aBkColor[Index] = Color;
		Invalidate();
	}
	
	RGBC TextColor(BUTTON_CI Index) {
		if (Index > 2)
			return RGB_INVALID_COLOR;
		return Props.aTextColor[Index];
	}
	void TextColor(BUTTON_CI Index, RGBC Color) {
		if (Index > 2)
			return;
		Props.aTextColor[Index] = Color;
		Invalidate();
	}

	TEXTALIGN TextAlign() { return Props.Align; }
	void TextAlign(TEXTALIGN Align) {
		if (Props.Align == Align)
			return;
		Props.Align = Align;
		Invalidate();
	}

#pragma endregion

	void SetText(const char *s) {
		if (text = s)
			Invalidate();
	}

	void SetDrawObj(BUTTON_BI Index, GUI_DRAW *pDrawObj) {
		if (Index > 2)
			return;
		GUI_ALLOC_FreePtr((void **)&aDrawObj[Index]);
		aDrawObj[Index] = pDrawObj;
		Invalidate();
	}
	void SetBitmapEx(BUTTON_BI Index, PCBITMAP pBitmap)
	{ SetDrawObj(Index, GUI_DRAW_BITMAP_Create(pBitmap)); }
	void SetBitmap(BUTTON_BI Index, PCBITMAP pBitmap)
	{ SetBitmapEx(Index, pBitmap); }
	void SetSelfDrawEx(BUTTON_BI Index, GUI_DRAW_SELF_CB *pDraw)
	{ SetDrawObj(Index, GUI_DRAW_SELF_Create(pDraw)); }
	void SetSelfDraw(BUTTON_BI Index, GUI_DRAW_SELF_CB *pDraw)
	{ SetSelfDrawEx(Index, pDraw); }

	bool IsPressed() { return GetStates() & BUTTON_STATE_PRESSED; }
	void SetPressed(bool On) { CtlStates(BUTTON_STATE_PRESSED, On); }

	void SetFocussable(bool On) { CtlStates(WIDGET_STATE_FOCUSSABLE, On); }

};

Button::Properties Button::DefaultProps;

}
