module;

#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */

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
		
enum EDIT_CI {
	 EDIT_CI_DISABLED = 0,
	 EDIT_CI_ENABLED
};

class Edit : public WIDGET {

public:
	typedef void tEDIT_AddKeyEx(Edit *pObj, int Key);
	typedef void tEDIT_UpdateBuffer(Edit *pObj);

public:
	struct Properties {
		PCFONT pFont{ &FontProp13_1 };
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
	Properties Props;

	char *pText;
	int16_t MaxLen;
	uint16_t BufferSize;
	int32_t Min, Max;            /* Min max values as normalized floats (integers) */
	uint8_t NumDecs;              /* Number of decimals */
	uint32_t CurrentValue;        /* Current value */
	int16_t CursorPos;           /* Cursor position. 0 means left most */
	uint16_t SelSize;        /* Number of selected characters */
	uint8_t EditMode;             /* Insert or overwrite mode */
	uint8_t XSizeCursor;          /* Size of cursor when working in insert mode */
	uint8_t Flags;
	tEDIT_AddKeyEx *pfAddKeyEx;     /* Handle key input */
	tEDIT_UpdateBuffer *pfUpdateBuffer;  /* Update textbuffer */

#if GUI_SUPPORT_TIMER
	static int CurrsorShow;
	static Timer *pTimer1;
	static void ShowCurrsor(GUI_TIMER_MESSAGE *TimeMsg) {
		auto pObj = (Edit *)TimeMsg->Context;
		WM_SelectWindow(pObj);
		pObj->_OnPaint();
		pObj->CurrsorShow++;
		pTimer1->Restart();
	}
#endif

	void _OnPaint() {
		const char *pText = nullptr;
		/* Set colors and font */
		GUI.SetBkColor(Props.aBkColor[IsEnabled() ? 1 : 0]);
		GUI.SetColor(Props.aTextColor[0]);
		GUI_SetFont(Props.pFont);
		/* Calculate size */
		auto r = WM_GetClientRect(this);
		auto rFillRect = WIDGET__GetInsideRect(this);
		if (this->pText)
			pText = this->pText;
		auto rInside = rFillRect;
		rInside.x0 += Props.Border + EDIT_XOFF;
		rInside.x1 -= Props.Border + EDIT_XOFF;
		RECT rText;
		GUI__CalcTextRect(pText, &rInside, &rText, Props.Align);
		WIDGET__FillStringInRect(pText, rFillRect, rInside, rText);
		/* Calculate position and size of cursor */
		if (this->State & WIDGET_STATE_FOCUS) {
			auto p = pText;
			int CursorWidth = this->XSizeCursor > 0 ? this->XSizeCursor : 1;
			RECT rInvert;
			if (pText) {
				uint16_t Char;
				int i;
				//  this->SelSize = 3;	//houhh 20061023...
				if (this->EditMode != GUI_EDIT_MODE_INSERT || this->SelSize) {
					int NumChars, CursorOffset;
					NumChars = GUI__GetNumChars(pText);
					if (this->CursorPos < NumChars) {
						if (this->SelSize) {
							CursorWidth = 0;
							for (i = this->CursorPos; i < (int)(this->CursorPos + this->SelSize); i++) {
								CursorOffset = GUI_UC__NumChars2NumBytes(pText, i);
								Char = GUI_UC_GetCharCode(pText + CursorOffset);
								CursorWidth += GUI_GetCharDistX(Char);
							}
							if (!CursorWidth) {
								CursorWidth = 1;
							}
						}
						else {
							CursorOffset = GUI_UC__NumChars2NumBytes(pText, this->CursorPos);
							Char = GUI_UC_GetCharCode(pText + CursorOffset);
							CursorWidth = GUI_GetCharDistX(Char);
						}
					}
				}
				rInvert = rText;
				for (i = 0; i != this->CursorPos; i++) {
					Char = GUI_UC__GetCharCodeInc(&p);
					rInvert.x0 += GUI_GetCharDistX(Char);
				}
			}
#if GUI_SUPPORT_TIMER
			if (!pTimer1) {
				pTimer1 = new Timer(ShowCurrsor, 1000 * 2);
				pTimer1->SetTime(1000 * 2);
				pTimer1->SetPeriod(500);
			}
			if (pTimer1) pTimer1->SetContext(this);
			if (this->CurrsorShow % 2)
#endif
				GUI_DrawRect({ rInvert.x0, rInvert.y0, rInvert.x0 + CursorWidth, rInvert.y1 });
		}
		DrawDown();
	}
	void _Delete() {
		GUI_ALLOC_FreePtr((void **)&this->pText);
#if GUI_SUPPORT_TIMER
		delete pTimer1;
		pTimer1 = nullptr;
#endif
	}
	void _SetCursorPos(int CursorPos) {
		if (this->pText) {
			auto pText = this->pText;
			int NumChars = GUI__GetNumChars(pText);
			int Offset = (this->EditMode == GUI_EDIT_MODE_INSERT) ? 0 : 1;
			if (CursorPos < 0)
				CursorPos = 0;
			if (CursorPos > NumChars)
				CursorPos = NumChars;
			if (CursorPos > this->MaxLen - Offset)
				CursorPos = this->MaxLen - Offset;
			if (this->CursorPos != CursorPos)
				this->CursorPos = CursorPos;
			this->SelSize = 0;
		}
	}
	void EDIT_SetCursorAtPixel(int xPos) {
		if (this->pText) {
			PCFONT pOldFont;
			int xSize, TextWidth, NumChars;
			const char *pText;
			pText = this->pText;
			pOldFont = GUI_SetFont(Props.pFont);
			xSize = GetSizeX();
			TextWidth = GUI_GetStringDistX(pText);
			switch (Props.Align & TEXTALIGN_HORIZONTAL) {
				case TEXTALIGN_HCENTER:
					xPos -= (xSize - TextWidth + 1) / 2;
					break;
				case TEXTALIGN_RIGHT:
					xPos -= xSize - TextWidth - (Props.Border + EDIT_XOFF);
					break;
				default:
					xPos -= (Props.Border + EDIT_XOFF) + this->EffectSize();
			}
			NumChars = GUI__GetNumChars(pText);
			if (xPos < 0) {
				this->_SetCursorPos(0);
			}
			else if (xPos > TextWidth) {
				this->_SetCursorPos(NumChars);
			}
			else {
				int i, x, xLenChar;
				uint16_t Char;
				for (i = 0, x = 0; (i < NumChars) && (x < xPos); i++) {
					Char = GUI_UC__GetCharCodeInc(&pText);
					xLenChar = GUI_GetCharDistX(Char);
					if (xPos < (x + xLenChar))
						break;
					x += xLenChar;
				}
				_SetCursorPos(i);
			}
			GUI_SetFont(pOldFont);
			WM_Invalidate(this);
		}
	}
	int _IncrementBuffer(unsigned AddBytes) {
		int NewSize = this->BufferSize + AddBytes;
		auto pNewStr = (char *)GUI_ALLOC_Realloc(this->pText, NewSize);
		if (pNewStr) {
			if (!(this->pText)) {
				pNewStr[0] = 0;
			}
			this->BufferSize = NewSize;
			this->pText = pNewStr;
			return 1;
		}
		return 0;
	}
	int _IsSpaceInBuffer(int BytesNeeded) {
		int NumBytes = 0;
		if (this->pText)
			NumBytes = GUI__strlen(this->pText);
		BytesNeeded = (BytesNeeded + NumBytes + 1) - this->BufferSize;
		if (BytesNeeded > 0)
			if (!_IncrementBuffer(BytesNeeded + EDIT_REALLOC_SIZE))
				return 0;
		return 1;
	}
	int _IsCharsAvailable(int CharsNeeded) {
		if (CharsNeeded > 0 && this->MaxLen > 0) {
			int NumChars = 0;
			if (this->pText)
				NumChars = GUI__GetNumChars(this->pText);
			if ((CharsNeeded + NumChars) > this->MaxLen)
				return 0;
		}
		return 1;
	}
	void _DeleteChar() {
		if (this->pText) {
			int CursorOffset;
			auto pText = this->pText;
			CursorOffset = GUI_UC__NumChars2NumBytes(pText, this->CursorPos);
			if (CursorOffset < GUI__strlen(pText)) {
				int NumBytes;
				pText += CursorOffset;
				NumBytes = GUI_UC_GetCharSize(pText);
				GUI__strcpy(pText, pText + NumBytes);
				WM_NotifyParent(this, WM_NOTIFICATION_VALUE_CHANGED);
			}
		}
	}
	int _InsertChar(uint16_t Char) {
		if (_IsCharsAvailable(1)) {
			int BytesNeeded;
			BytesNeeded = GUI_UC__CalcSizeOfChar(Char);
			if (_IsSpaceInBuffer(BytesNeeded)) {
				auto pText = this->pText;
				int CursorOffset = GUI_UC__NumChars2NumBytes(pText, this->CursorPos);
				pText += CursorOffset;
				GUI__memmove(pText + BytesNeeded, pText, GUI__strlen(pText) + 1);
				GUI_UC_Encode(pText, Char);
				WM_NotifyParent(this, WM_NOTIFICATION_VALUE_CHANGED);
				return 1;
			}
		}
		return 0;
	}
	uint16_t _GetCurrentChar() {
		uint16_t Char = 0;
		if (this->pText) {
			auto pText = this->pText;
			pText += GUI_UC__NumChars2NumBytes(pText, this->CursorPos);
			Char = GUI_UC_GetCharCode(pText);
		}
		return Char;
	}
	void _OnTouch(const PID_STATE *pState) {
		if (pState) {  /* Something happened in our area (pressed or released) */
			static int StartPress = 0;
			if (pState->Pressed) {
				GUI_DEBUG_LOG("_Callback(WM_TOUCH, Pressed, Handle %d)\n", 1);
				EDIT_SetCursorAtPixel(pState->x);
				StartPress = this->CursorPos;
			}
			else {
				GUI_DEBUG_LOG("_Callback(WM_TOUCH, Released, Handle %d)\n", 1);
			}
		}
		else {
			GUI_DEBUG_LOG("_EDIT_Callback(WM_TOUCH, Moved out, Handle %d)\n", 1);
		}
	}
	int _OnKey(const WM_KEY_INFO *pInfo) {
		if (pInfo->PressedCnt > 0) { /* Key pressed? */
			int Key = pInfo->Key;
			switch (Key) {
				case GUI_KEY_TAB:
					break; /* Send to parent by not doing anything */
				default:
					AddKey(Key);
					return 1;
			}
		}
		return 0;
	}

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (Edit *)hWin;
		/* Let widget handle the standard messages */
		if (!WIDGET_HandleActive(pObj, MsgId, &Data))
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
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:

	static Edit *Create(int x0, int y0, int xsize, int ysize,
						WObj *hParent, int WinFlags, int ExFlags,
						int Id, int MaxLen) {
		/* Alloc memory for obj */
		WinFlags |= WC_LATE_CLIP;    /* Always use late clipping since widget is optimized for it. */
		auto pObj = (Edit *)WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WC_VISIBLE | WinFlags, Edit::_Callback,
												   sizeof(Edit) - sizeof(WObj));
		if (!pObj) {
			GUI_DEBUG_ERROROUT_IF(pObj == 0, "Edit create failed");
			return nullptr;
		}
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE);
		/* init member variables */
		pObj->Props = Edit::DefaultProps;
		pObj->XSizeCursor = 1;
		pObj->MaxLen = (MaxLen == 0) ? 8 : MaxLen;
		pObj->BufferSize = 0;
		pObj->pText = nullptr;
		if (pObj->_IncrementBuffer(pObj->MaxLen + 1) == 0) {
			GUI_DEBUG_ERROROUT("Edit create failed to alloc buffer");
			WM_DeleteWindow(pObj);
			pObj = nullptr;
		}
		return pObj;
	}
	static WObj *CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WObj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
		auto pEdit = Create(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id, pCreateInfo->Para);
		if (pEdit)
			pEdit->SetTextAlign(pCreateInfo->Flags);
		return pEdit;
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

	void SetBkColor(EDIT_CI Index, RGBC color) {
		if (Index >= GUI_COUNTOF(Props.aBkColor))
			return;
		if (Props.aBkColor[Index] == color)
			return;
		Props.aBkColor[Index] = color;
		WM_Invalidate(this);
	}

	void SetTextColor(EDIT_CI Index, RGBC color) {
		if (Index >= GUI_COUNTOF(Props.aTextColor))
			return;
		if (Props.aTextColor[Index] == color)
			return;
		Props.aTextColor[Index] = color;
		WM_Invalidate(this);
	}

#pragma endregion

	void AddKey(int Key) {
		if (pfAddKeyEx) {
			pfAddKeyEx(this, Key);
		}
		else {
			switch (Key) {
				case GUI_KEY_UP:
					if (pText) {
						auto pText = this->pText;
						uint16_t Char;
						pText += GUI_UC__NumChars2NumBytes(pText, this->CursorPos);
						Char = GUI_UC_GetCharCode(pText);
						if (Char < 0x7f) {
							*pText = Char + 1;
							WM_NotifyParent(this, WM_NOTIFICATION_VALUE_CHANGED);
						}
					}
					break;
				case GUI_KEY_DOWN:
					if (pText) {
						auto pText = this->pText;
						pText += GUI_UC__NumChars2NumBytes(pText, this->CursorPos);
						uint16_t Char = GUI_UC_GetCharCode(pText);
						if (Char > 0x20) {
							*pText = Char - 1;
							WM_NotifyParent(this, WM_NOTIFICATION_VALUE_CHANGED);
						}
					}
					break;
				case GUI_KEY_RIGHT:
					_SetCursorPos(this->CursorPos + 1);
					break;
				case GUI_KEY_LEFT:
					_SetCursorPos(this->CursorPos - 1);
					break;
				case GUI_KEY_BACKSPACE:
					_SetCursorPos(this->CursorPos - 1);
					_DeleteChar();
					break;
				case GUI_KEY_DELETE:
					_DeleteChar();
					break;
				case GUI_KEY_INSERT:
					if (this->EditMode == GUI_EDIT_MODE_OVERWRITE) {
						this->EditMode = GUI_EDIT_MODE_INSERT;
					}
					else {
						this->EditMode = GUI_EDIT_MODE_OVERWRITE;
						_SetCursorPos(this->CursorPos);
					}
					break;
				case GUI_KEY_ENTER:
				case GUI_KEY_ESCAPE:
					break;
				default:
					if (Key >= 0x20) {
						if (this->EditMode != GUI_EDIT_MODE_INSERT) {
							_DeleteChar();
						}
						if (_InsertChar(Key)) {
							_SetCursorPos(this->CursorPos + 1);
						}
					}
			}
		}
		WM_Invalidate(this);
	}

	void SetText(const char *s) {
		if (s) {
			int NumBytesNew, NumBytesOld = 0;
			int NumCharsNew;
			if (this->pText) {
				auto pText = this->pText;
				NumBytesOld = GUI__strlen(pText) + 1;
			}
			NumCharsNew = GUI__GetNumChars(s);
			if (NumCharsNew > this->MaxLen) {
				NumCharsNew = this->MaxLen;
			}
			NumBytesNew = GUI_UC__NumChars2NumBytes(s, NumCharsNew) + 1;
			if (_IsSpaceInBuffer(NumBytesNew - NumBytesOld)) {
				auto pText = this->pText;
				GUI__memcpy(pText, s, NumBytesNew);
				this->CursorPos = NumBytesNew - 1;
				if (this->CursorPos == this->MaxLen) {
					if (this->EditMode == GUI_EDIT_MODE_OVERWRITE) {
						this->CursorPos--;
					}
				}
			}
		}
		else {
			GUI_ALLOC_FreePtr((void **)&this->pText);
			this->BufferSize = 0;
			this->CursorPos = 0;
		}
		WM_Invalidate(this);
	}
	void GetText(char *sDest, int MaxLen) {
		if (sDest) {
			*sDest = 0;
			if (this->pText) {
				auto pText = this->pText;
				int NumChars = GUI__GetNumChars(pText);
				if (NumChars > MaxLen)
					NumChars = MaxLen;
				int NumBytes = GUI_UC__NumChars2NumBytes(pText, NumChars);
				GUI__memcpy(sDest, pText, NumBytes);
				*(sDest + NumBytes) = 0;
			}
		}
	}

	int32_t GetValue() {
		int32_t r = 0;
		r = this->CurrentValue;

		return r;
	}
	void SetValue(int32_t Value) {
		/* Put in min/max range */
		if (Value < this->Min)
			Value = this->Min;
		if (Value > this->Max)
			Value = this->Max;
		if (this->CurrentValue != (uint32_t)Value) {
			this->CurrentValue = Value;
			if (this->pfUpdateBuffer)
				this->pfUpdateBuffer(this);
			WM_Invalidate(this);
			WM_NotifyParent(this, WM_NOTIFICATION_VALUE_CHANGED);
		}
	}

	void SetMaxLen(int MaxLen) {
		if (MaxLen != this->MaxLen) {
			if (MaxLen < this->MaxLen) {
				if (this->pText) {
					auto pText = this->pText;
					int NumChars = GUI__GetNumChars(pText);
					if (NumChars > MaxLen) {
						int NumBytes;
						NumBytes = GUI_UC__NumChars2NumBytes(pText, MaxLen);
						*(pText + NumBytes) = 0;
					}
				}
			}
			_IncrementBuffer(MaxLen - this->BufferSize + 1);
			this->MaxLen = MaxLen;
			WM_Invalidate(this);
		}
	}

	int GetNumChars() {
		if (this->pText) {
			return GUI__GetNumChars(this->pText);
		}

		return 0;
	}

	void SetCursorAtChar(int Pos) {
		_SetCursorPos(Pos);
		WM_Invalidate(this);
	}

	void SetSel(int FirstChar, int LastChar) {
		if (FirstChar == -1) {
			this->SelSize = 0;
		}
		else {
			if (FirstChar > this->BufferSize - 1)
				FirstChar = this->BufferSize - 1;
			if (LastChar > this->BufferSize - 1)
				LastChar = this->BufferSize - 1;
			if (LastChar == -1)
				LastChar = GetNumChars();
			if (LastChar >= FirstChar) {
				this->CursorPos = FirstChar;
				this->SelSize = LastChar - FirstChar + 1;
			}
		}
	}

	int SetInsertMode(int OnOff) {
		int PrevMode = 0;
		PrevMode = this->EditMode;
		this->EditMode = OnOff ? GUI_EDIT_MODE_INSERT : GUI_EDIT_MODE_OVERWRITE;

		return PrevMode;
	}
	void SetpfAddKeyEx(tEDIT_AddKeyEx *pfAddKeyEx) {
		this->pfAddKeyEx = pfAddKeyEx;
	}
	void SetpfUpdateBuffer(tEDIT_UpdateBuffer *pfUpdateBuffer) {
		this->pfUpdateBuffer = pfUpdateBuffer;
	}

};

Edit::Properties Edit::DefaultProps;

#if GUI_SUPPORT_TIMER
int Edit::CurrsorShow = 0;
Timer *Edit::pTimer1 = nullptr;
#endif

}
