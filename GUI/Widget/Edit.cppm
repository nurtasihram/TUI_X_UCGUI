module;

#include "GUI_Protected.h"

export module TUX.Widget.Edit;

import TUX.Widget;
#if GUI_SUPPORT_TIMER
import TUX.Core.Timer;
#endif

#define EDIT_XOFF 1
#define EDIT_REALLOC_SIZE 16

////////////////////////// !!! THIS VERSION'S TIMER IS NOT SAFE !!! ////////////////////////// 

export {

constexpr uint8_t
	GUI_EDIT_NORMAL = 0,
	GUI_EDIT_SIGNED = 1;

constexpr uint8_t
	GUI_EDIT_MODE_INSERT    = 0,
	GUI_EDIT_MODE_OVERWRITE = 1;
		
enum EDIT_CI : uint8_t {
	 EDIT_CI_DISABLED = 0,
	 EDIT_CI_ENABLED
};

class Edit : public Widget {

public:
	typedef void tEDIT_AddKeyEx(Edit *pObj, int Key);
	typedef void tEDIT_UpdateBuffer(Edit *pObj);

public:
	struct Properties {
		PCFONT pFont{ GUI_DEFAULT_FONT };
		RGBC aTextColor[2]{
			/* Disabled */	RGB_BLACK,
			/* Enabled */	RGB_BLACK
		};
		RGBC aBkColor[2]{
			/* Disabled */	RGB_GRAYL(0xC0),
			/* Enabled */	RGB_WHITE
		};
		TEXTALIGN Align{ TEXTALIGN_LEFT | TEXTALIGN_VCENTER };
		int8_t Border{ 1 };
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;

	char *pText = nullptr;
	uint16_t MaxLen;
	uint16_t BufferSize = 0;
	int16_t CursorPos = 0;           /* Cursor position. 0 means left most */
	uint16_t SelSize = 0;        /* Number of selected characters */
	bool EditMode = 1; /* Insert or overwrite mode */
	uint8_t XSizeCursor = 1;          /* Size of cursor when working in insert mode */

#if GUI_SUPPORT_TIMER
	static int CurrsorShow;
	static Timer *pTimer1;
	static void ShowCurrsor(GUI_TIMER_MESSAGE *TimeMsg) {
		auto pObj = (Edit *)TimeMsg->Context;
		pObj->Select();
		pObj->_OnPaint();
		pObj->CurrsorShow++;
		pTimer1->Restart();
	}
#endif

	void _OnPaint() {
		/* Set colors and font */
		GUI.BkColor(Props.aBkColor[IsEnabled() ? EDIT_CI_ENABLED : EDIT_CI_DISABLED]);
		GUI.Color(Props.aTextColor[IsEnabled() ? EDIT_CI_ENABLED : EDIT_CI_DISABLED]);
		GUI.Font(Props.pFont);
		/* Calculate size */
		auto rFillRect = _GetInsideRect();
		auto rInside = rFillRect;
		rInside.x0 += Props.Border + EDIT_XOFF;
		rInside.x1 -= Props.Border + EDIT_XOFF;
		RECT rText;
		GUI__CalcTextRect(pText, &rInside, &rText, Props.Align);
		WIDGET__FillStringInRect(pText, rFillRect, rInside, rText);
		/* Calculate position and size of cursor */
		if (States & WIDGET_STATE_FOCUS) {
			auto p = pText;
			int CursorWidth = XSizeCursor > 0 ? XSizeCursor : 1;
			RECT rInvert;
			if (pText) {
				if (EditMode != GUI_EDIT_MODE_INSERT || SelSize) {
					if (CursorPos < GUI__strlen(pText)) {
						if (SelSize) {
							CursorWidth = 0;
							for (auto i = CursorPos; i < CursorPos + SelSize; i++)
								CursorWidth += GUI.Font().GetCharSizeX(pText[i]);
							if (!CursorWidth)
								CursorWidth = 1;
						} else
							CursorWidth = GUI.Font().GetCharSizeX(pText[CursorPos]);
					}
				}
				rInvert = rText;
				for (int i = 0; i < CursorPos; i++)
					rInvert.x0 += GUI.Font().GetCharSizeX(*p++);
			}
#if GUI_SUPPORT_TIMER
			if (!pTimer1) {
				pTimer1 = new Timer(ShowCurrsor, 1000 * 2);
				pTimer1->SetTime(1000 * 2);
				pTimer1->SetPeriod(500);
			}
			if (pTimer1) pTimer1->SetContext(this);
			if (CurrsorShow % 2)
#endif
				GUI_DrawRect({ rInvert.x0, rInvert.y0, rInvert.x0 + CursorWidth, rInvert.y1 });
		}
		DrawDown();
	}
	void _Delete() {
		GUI_ALLOC_FreePtr((void **)&pText);
#if GUI_SUPPORT_TIMER
		delete pTimer1;
		pTimer1 = nullptr;
#endif
	}
	void _SetCursorPos(int CursorPos) {
		if (pText) {
			int NumChars = GUI__strlen(pText);
			int Offset = (EditMode == GUI_EDIT_MODE_INSERT) ? 0 : 1;
			if (CursorPos < 0)
				CursorPos = 0;
			if (CursorPos > NumChars)
				CursorPos = NumChars;
			if (CursorPos > MaxLen - Offset)
				CursorPos = MaxLen - Offset;
			if (this->CursorPos != CursorPos)
				this->CursorPos = CursorPos;
			SelSize = 0;
		}
	}
	void EDIT_SetCursorAtPixel(int xPos) {
		if (pText) {
			auto pOldFont = GUI.Font(Props.pFont);
			auto xSize = GetSizeX();
			auto TextWidth = GUI_GetStringSizeX(pText);
			switch (Props.Align & TEXTALIGN_HORIZONTAL) {
				case TEXTALIGN_HCENTER:
					xPos -= (xSize - TextWidth + 1) / 2;
					break;
				case TEXTALIGN_RIGHT:
					xPos -= xSize - TextWidth - (Props.Border + EDIT_XOFF);
					break;
				default:
					xPos -= (Props.Border + EDIT_XOFF) + EffectSize();
			}
			auto NumChars = GUI__strlen(pText);
			if (xPos < 0)
				_SetCursorPos(0);
			else if (xPos > TextWidth)
				_SetCursorPos(NumChars);
			else {
				auto p = pText;
				int i = 0, x = 0;
				for (; (i < NumChars) && (x < xPos); i++) {
					int xLenChar = GUI.Font().GetCharSizeX(*p++);
					if (xPos < (x + xLenChar))
						break;
					x += xLenChar;
				}
				_SetCursorPos(i);
			}
			GUI.Font(pOldFont);
			Invalidate();
		}
	}
	bool _IncrementBuffer(unsigned AddBytes) {
		int NewSize = BufferSize + AddBytes;
		auto pNewStr = (char *)GUI_ALLOC_Realloc(pText, NewSize);
		if (pNewStr) {
			if (!pText)
				pNewStr[0] = 0;
			BufferSize = NewSize;
			pText = pNewStr;
			return true;
		}
		return false;
	}
	bool _IsSpaceInBuffer(int BytesNeeded) {
		int NumBytes = 0;
		if (pText)
			NumBytes = GUI__strlen(pText);
		BytesNeeded = (BytesNeeded + NumBytes + 1) - BufferSize;
		if (BytesNeeded > 0)
			if (!_IncrementBuffer(BytesNeeded + EDIT_REALLOC_SIZE))
				return false;
		return true;
	}
	bool _IsCharsAvailable(int CharsNeeded) {
		if (CharsNeeded > 0 && MaxLen > 0) {
			int NumChars = 0;
			if (pText)
				NumChars = GUI__strlen(pText);
			if ((CharsNeeded + NumChars) > MaxLen)
				return false;
		}
		return true;
	}
	void _DeleteChar() {
		if (pText && CursorPos < GUI__strlen(pText)) {
			auto p = pText + CursorPos;
			GUI__strcpy(p, p + 1);
			NotifyParent(WM_NOTIFICATION_VALUE_CHANGED);
		}
	}
	bool _InsertChar(char Char) {
		if (_IsCharsAvailable(1) && _IsSpaceInBuffer(1)) {
			auto p = pText + CursorPos;
			GUI__memmove(p + 1, p, GUI__strlen(p) + 1);
			*p = Char;
			NotifyParent(WM_NOTIFICATION_VALUE_CHANGED);
			return true;
		}
		return false;
	}
	char _GetCurrentChar() {
		if (pText)
			return pText[CursorPos];
		return 0;
	}
	void _OnTouch(const PID_STATE *pState) {
		if (pState) {  /* Something happened in our area (pressed or released) */
			static int StartPress = 0;
			if (pState->Pressed) {
				GUI_DEBUG_LOG("_Callback(WM_TOUCH, Pressed, Handle %d)\n", 1);
				EDIT_SetCursorAtPixel(pState->x);
				StartPress = CursorPos;
			}
			else {
				GUI_DEBUG_LOG("_Callback(WM_TOUCH, Released, Handle %d)\n", 1);
			}
		}
		else {
			GUI_DEBUG_LOG("_EDIT_Callback(WM_TOUCH, Moved out, Handle %d)\n", 1);
		}
	}
	bool _OnKey(const WM_KEY_INFO *pInfo) {
		if (pInfo->PressedCnt > 0) { /* Key pressed? */
			int Key = pInfo->Key;
			switch (Key) {
				case GUI_KEY_TAB:
					break; /* Send to parent by not doing anything */
				default:
					AddKey(Key);
					return true;
			}
		}
		return false;
	}

	static WM_PARAM _Callback(WObj *pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (Edit *)pWin;
		/* Let widget handle the standard messages */
		if (!pObj->HandleActive(MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
				return 0;
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_DELETE:
				pObj->_Delete();
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const WM_KEY_INFO *)Data))
					return 0;
				break;
		}
		return DefaultProc(pWin, MsgId, Data);
	}

public:
	Edit(RECT r, WM_CF Style, WObj *pParent, uint16_t Id,
		 uint16_t MaxLen) :
		Widget(r, Style | WC_LATE_CLIP | WC_VISIBLE,
			   _Callback, pParent, Id, WIDGET_STATE_FOCUSSABLE),
		MaxLen(MaxLen ? MaxLen : 8) {
		_IncrementBuffer(MaxLen + 1);
	}
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *pWinParent, int x0, int y0, WM_CALLBACK *cb) {
		auto pEdit = new Edit(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			pCreateInfo->Flags, pWinParent, pCreateInfo->Id,
			(uint16_t)pCreateInfo->Para
		);
		pEdit->TextAlign((TEXTALIGN)pCreateInfo->Flags);
		return pEdit;
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

	void BkColor(EDIT_CI Index, RGBC color) {
		if (Index >= GUI_COUNTOF(Props.aBkColor))
			return;
		if (Props.aBkColor[Index] == color)
			return;
		Props.aBkColor[Index] = color;
		Invalidate();
	}

	void TextColor(EDIT_CI Index, RGBC color) {
		if (Index >= GUI_COUNTOF(Props.aTextColor))
			return;
		if (Props.aTextColor[Index] == color)
			return;
		Props.aTextColor[Index] = color;
		Invalidate();
	}

#pragma endregion

	void AddKey(int Key) {
		switch (Key) {
			case GUI_KEY_UP:
				if (pText) {
					auto p = pText + CursorPos;
					auto Char = *p;
					if (Char < 0x7f) {
						*p = Char + 1;
						NotifyParent(WM_NOTIFICATION_VALUE_CHANGED);
					}
				}
				break;
			case GUI_KEY_DOWN:
				if (pText) {
					auto p = pText + CursorPos;
					auto Char = *p;
					if (Char > 0x20) {
						*p = Char - 1;
						NotifyParent(WM_NOTIFICATION_VALUE_CHANGED);
					}
				}
				break;
			case GUI_KEY_RIGHT:
				_SetCursorPos(CursorPos + 1);
				break;
			case GUI_KEY_LEFT:
				_SetCursorPos(CursorPos - 1);
				break;
			case GUI_KEY_BACKSPACE:
				_SetCursorPos(CursorPos - 1);
				_DeleteChar();
				break;
			case GUI_KEY_DELETE:
				_DeleteChar();
				break;
			case GUI_KEY_INSERT:
				if (EditMode == GUI_EDIT_MODE_OVERWRITE)
					EditMode = GUI_EDIT_MODE_INSERT;
				else {
					EditMode = GUI_EDIT_MODE_OVERWRITE;
					_SetCursorPos(CursorPos);
				}
				break;
			case GUI_KEY_ENTER:
			case GUI_KEY_ESCAPE:
				break;
			default:
				if (Key >= 0x20) {
					if (EditMode != GUI_EDIT_MODE_INSERT)
						_DeleteChar();
					if (_InsertChar(Key))
						_SetCursorPos(CursorPos + 1);
				}
		}
		Invalidate();
	}

	void SetText(const char *s) {
		if (!s) return;
		int NumBytesNew, NumBytesOld = 0;
		int NumCharsNew;
		if (pText)
			NumBytesOld = GUI__strlen(pText) + 1;
		NumCharsNew = GUI__strlen(s);
		if (NumCharsNew > MaxLen)
			NumCharsNew = MaxLen;
		NumBytesNew = NumCharsNew + 1;
		if (_IsSpaceInBuffer(NumBytesNew - NumBytesOld)) {
			GUI__memcpy(pText, s, NumBytesNew);
			CursorPos = NumBytesNew - 1;
			if (CursorPos == MaxLen && EditMode == GUI_EDIT_MODE_OVERWRITE)
				CursorPos--;
		} else {
			GUI_ALLOC_FreePtr((void **)&pText);
			BufferSize = 0;
			CursorPos = 0;
		}
		Invalidate();
	}
	void GetText(char *sDest, int MaxLen) {
		if (!sDest) return;
		*sDest = 0;
		if (!pText) return;
		int NumChars = GUI__strlen(pText);
		if (NumChars > MaxLen)
			NumChars = MaxLen;
		GUI__memcpy(sDest, pText, NumChars);
		sDest[NumChars] = 0;
	}

	void SetMaxLen(int MaxLen) {
		if (MaxLen == this->MaxLen) 
			return;
		if (MaxLen < this->MaxLen) {
			if (pText) {
				int NumChars = GUI__strlen(pText);
				if (NumChars > MaxLen)
					pText[MaxLen] = 0;
			}
		}
		_IncrementBuffer(MaxLen - BufferSize + 1);
		this->MaxLen = MaxLen;
		Invalidate();
	}

	int GetNumChars() {
		return GUI__strlen(pText);
	}

	void SetCursorAtChar(int Pos) {
		_SetCursorPos(Pos);
		Invalidate();
	}

	void SetSel(int FirstChar, int LastChar) {
		if (FirstChar == -1)
			SelSize = 0;
		else {
			if (FirstChar > BufferSize - 1)
				FirstChar = BufferSize - 1;
			if (LastChar > BufferSize - 1)
				LastChar = BufferSize - 1;
			if (LastChar == -1)
				LastChar = GetNumChars();
			if (LastChar >= FirstChar) {
				CursorPos = FirstChar;
				SelSize = LastChar - FirstChar + 1;
			}
		}
	}

	void SetInsertMode(bool OnOff) {
		EditMode = OnOff ? GUI_EDIT_MODE_INSERT : GUI_EDIT_MODE_OVERWRITE;
	}

};

Edit::Properties Edit::DefaultProps;

#if GUI_SUPPORT_TIMER
int Edit::CurrsorShow = 0;
Timer *Edit::pTimer1 = nullptr;
#endif

}
