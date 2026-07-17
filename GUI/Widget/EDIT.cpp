#include "GUIDebug.h"
#include "GUI_Protected.h"

#include "EDIT.h"
#include "EDIT_Private.h"

/* Define default fonts */
#define EDIT_FONT_DEFAULT &GUI_Font13_1
#define EDIT_ALIGN_DEFAULT GUI_TA_LEFT | GUI_TA_VCENTER
/* Define colors */
#define EDIT_BKCOLOR0_DEFAULT RGB_GRAYL(0xC0)
#define EDIT_BKCOLOR1_DEFAULT RGB_WHITE
#define EDIT_TEXTCOLOR0_DEFAULT RGB_BLACK
#define EDIT_TEXTCOLOR1_DEFAULT RGB_BLACK
#define EDIT_BORDER_DEFAULT 1
#define EDIT_XOFF 1
EDIT_Obj::Properties EDIT_Obj::DefaultProps {
  EDIT_ALIGN_DEFAULT,
  EDIT_BORDER_DEFAULT,
  EDIT_FONT_DEFAULT,
  EDIT_TEXTCOLOR0_DEFAULT,
  EDIT_TEXTCOLOR1_DEFAULT,
  EDIT_BKCOLOR0_DEFAULT,
  EDIT_BKCOLOR1_DEFAULT
};

///////////houhh 20061018...
static GUI_TIMER_HANDLE Timer1 = 0;	//houhh 20061018...
static void _OnPaint(EDIT_Obj *pObj);
void ShowCurrsor(GUI_TIMER_MESSAGE *TimeMsg) {
	EDIT_Handle hObj = (EDIT_Handle)TimeMsg->Context;
	EDIT_Obj *pObj = (EDIT_Obj *)hObj;
	WM_Obj *pWin = (WM_Obj *)hObj;
	WM_SelectWindow(hObj);
	_OnPaint(pObj);
	pObj->CurrsorShow++;
	GUI_TIMER_Restart(Timer1);
}
///////
static void _OnPaint(EDIT_Obj *pObj) {
	GUI_RECT rFillRect, rInside, r, rText, rInvert;
	const char  *pText = NULL;
	int IsEnabled, CursorWidth;
	IsEnabled = WM_IsEnabled(pObj);
	/* Set colors and font */
	GUI_SetBkColor(pObj->Props.aBkColor[IsEnabled]);
	GUI_SetColor(pObj->Props.aTextColor[0]);
	GUI_SetFont(pObj->Props.pFont);
	/* Calculate size */
	WM_GetClientRectEx(pObj, &r);
	WIDGET__GetInsideRect(pObj, &rFillRect);
	if (pObj->pText)
		pText = pObj->pText;
	rInside = rFillRect;
	rInside.x0 += pObj->Props.Border + EDIT_XOFF;
	rInside.x1 -= pObj->Props.Border + EDIT_XOFF;
	GUI__CalcTextRect(pText, &rInside, &rText, pObj->Props.Align);
	/* Calculate position and size of cursor */
	if (pObj->State & WIDGET_STATE_FOCUS) {
		const char  *p = pText;
		CursorWidth = ((pObj->XSizeCursor > 0) ? (pObj->XSizeCursor) : (1));
		if (pText) {
			uint16_t Char;
			int i;
			//  pObj->SelSize = 3;	//houhh 20061023...
			if ((pObj->EditMode != GUI_EDIT_MODE_INSERT) || (pObj->SelSize)) {
				int NumChars, CursorOffset;
				NumChars = GUI__GetNumChars(pText);
				if (pObj->CursorPos < NumChars) {
					if (pObj->SelSize) {
						CursorWidth = 0;
						for (i = pObj->CursorPos; i < (int)(pObj->CursorPos + pObj->SelSize); i++) {
							CursorOffset = GUI_UC__NumChars2NumBytes(pText, i);
							Char = GUI_UC_GetCharCode(pText + CursorOffset);
							CursorWidth += GUI_GetCharDistX(Char);
						}
						if (!CursorWidth) {
							CursorWidth = 1;
						}
					}
					else {
						CursorOffset = GUI_UC__NumChars2NumBytes(pText, pObj->CursorPos);
						Char = GUI_UC_GetCharCode(pText + CursorOffset);
						CursorWidth = GUI_GetCharDistX(Char);
					}
				}
			}
			rInvert = rText;
			for (i = 0; i != pObj->CursorPos; i++) {
				Char = GUI_UC__GetCharCodeInc(&p);
				rInvert.x0 += GUI_GetCharDistX(Char);
			}
		}
	}
	/* WM loop */
	WM_ITERATE_START(NULL) {
		/* Set clipping rectangle */
		WM_SetUserClipRect(&rFillRect);
		/* Display text */
		WIDGET__FillStringInRect(pText, &rFillRect, &rInside, &rText);
		/* Display cursor if needed */
		if (pObj->State & WIDGET_STATE_FOCUS) {
			///////////////houhh 20061020...
			//  static GUI_TIMER_HANDLE Timer1 = NULL;	//houhh 20061018...
			if (!Timer1) {
				Timer1 = GUI_TIMER_Create((GUI_TIMER_CALLBACK *)ShowCurrsor, 1000 * 2, 0, 0);	//houhh 20061018...
				GUI_TIMER_SetTime(Timer1, 1000 * 2);
				GUI_TIMER_SetPeriod(Timer1, 500);
			}
			if (Timer1) GUI_TIMER_Context(Timer1, (uintptr_t)pObj);
		//	if (pObj->CurrsorShow % 2) //houhh 20061022...
		//		GUI_InvertRect(rInvert.x0, rInvert.y0, rInvert.x0 + CursorWidth - 1, rInvert.y1);
			/////////////
		//	GUI_InvertRect(rInvert.x0, rInvert.y0, rInvert.x0 + CursorWidth - 1, rInvert.y1);
		}
		WM_SetUserClipRect(NULL);
		/* Draw the 3D effect (if configured) */
		WIDGET__EFFECT_DrawDown(pObj);
	} WM_ITERATE_END();
}
static void _Delete(EDIT_Obj *pObj) {
	GUI_ALLOC_FreePtr((void **)&pObj->pText);
}
void EDIT_SetCursorAtPixel(EDIT_Handle hObj, int xPos) {
	if (hObj) {
		EDIT_Obj *pObj = (EDIT_Obj *)hObj;
		if (pObj->pText) {
			const GUI_FONT  *pOldFont;
			int xSize, TextWidth, NumChars;
			const char  *pText;
			pText = pObj->pText;
			pOldFont = GUI_SetFont(pObj->Props.pFont);
			xSize = WM_GetWindowSizeX(hObj);
			TextWidth = GUI_GetStringDistX(pText);
			switch (pObj->Props.Align & GUI_TA_HORIZONTAL) {
				case GUI_TA_HCENTER:
					xPos -= (xSize - TextWidth + 1) / 2;
					break;
				case GUI_TA_RIGHT:
					xPos -= xSize - TextWidth - (pObj->Props.Border + EDIT_XOFF);
					break;
				default:
					xPos -= (pObj->Props.Border + EDIT_XOFF) + pObj->pEffect->EffectSize;
			}
			NumChars = GUI__GetNumChars(pText);
			if (xPos < 0) {
				EDIT__SetCursorPos(pObj, 0);
			}
			else if (xPos > TextWidth) {
				EDIT__SetCursorPos(pObj, NumChars);
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
				EDIT__SetCursorPos(pObj, i);
			}
			GUI_SetFont(pOldFont);
			EDIT_Invalidate(hObj);
		}

	}
}
/*********************************************************************
*
*       _IncrementBuffer
*
* Increments the buffer size by AddBytes.
*/
static int _IncrementBuffer(EDIT_Obj *pObj, unsigned AddBytes) {
	int NewSize = pObj->BufferSize + AddBytes;
	char *pNewStr = (char *)GUI_ALLOC_Realloc(pObj->pText, NewSize);
	if (pNewStr) {
		if (!(pObj->pText)) {
			pNewStr[0] = 0;
		}
		pObj->BufferSize = NewSize;
		pObj->pText = pNewStr;
		return 1;
	}
	return 0;
}
/*********************************************************************
*
*       _IsSpaceInBuffer
*
* Checks the available space in the buffer. If there is not enough
* space left this function attempts to get more.
*
* Returns:
*  1 = requested space is available
*  0 = failed to get enough space
*/
static int _IsSpaceInBuffer(EDIT_Obj *pObj, int BytesNeeded) {
	int NumBytes = 0;
	if (pObj->pText)
		NumBytes = GUI__strlen(pObj->pText);
	BytesNeeded = (BytesNeeded + NumBytes + 1) - pObj->BufferSize;
	if (BytesNeeded > 0)
		if (!_IncrementBuffer(pObj, BytesNeeded + EDIT_REALLOC_SIZE))
			return 0;
	return 1;
}
/*********************************************************************
*
*       _IsCharsAvailable
*
* Checks weither the maximum number of characters is reached or not.
*
* Returns:
*  1 = requested number of chars is available
*  0 = maximum number of chars have reached
*/
static int _IsCharsAvailable(EDIT_Obj *pObj, int CharsNeeded) {
	if (CharsNeeded > 0 && pObj->MaxLen > 0) {
		int NumChars = 0;
		if (pObj->pText)
			NumChars = GUI__GetNumChars(pObj->pText);
		if ((CharsNeeded + NumChars) > pObj->MaxLen)
			return 0;
	}
	return 1;
}
/*********************************************************************
*
*       _DeleteChar
*
* Deletes a character at the current cursor position and moves
* all bytes after the cursor position.
*/
static void _DeleteChar(EDIT_Obj *pObj) {
	if (pObj->pText) {
		int CursorOffset;
		char *pText = pObj->pText;
		CursorOffset = GUI_UC__NumChars2NumBytes(pText, pObj->CursorPos);
		if (CursorOffset < GUI__strlen(pText)) {
			int NumBytes;
			pText += CursorOffset;
			NumBytes = GUI_UC_GetCharSize(pText);
			GUI__strcpy(pText, pText + NumBytes);
			WM_NotifyParent(pObj, WM_NOTIFICATION_VALUE_CHANGED);
		}
	}
}
/*********************************************************************
*
*       _InsertChar
*
* Create space at the current cursor position and inserts a character.
*/
static int _InsertChar(EDIT_Obj *pObj, uint16_t Char) {
	if (_IsCharsAvailable(pObj, 1)) {
		int BytesNeeded;
		BytesNeeded = GUI_UC__CalcSizeOfChar(Char);
		if (_IsSpaceInBuffer(pObj, BytesNeeded)) {
			char *pText = pObj->pText;
			int CursorOffset = GUI_UC__NumChars2NumBytes(pText, pObj->CursorPos);
			pText += CursorOffset;
			GUI__memmove(pText + BytesNeeded, pText, GUI__strlen(pText) + 1);
			GUI_UC_Encode(pText, Char);
			WM_NotifyParent(pObj, WM_NOTIFICATION_VALUE_CHANGED);
			return 1;
		}
	}
	return 0;
}
uint16_t EDIT__GetCurrentChar(EDIT_Obj *pObj) {
	uint16_t Char = 0;
	if (pObj->pText) {
		const char *pText = pObj->pText;
		pText += GUI_UC__NumChars2NumBytes(pText, pObj->CursorPos);
		Char = GUI_UC_GetCharCode(pText);
	}
	return Char;
}
/*********************************************************************
*
*       EDIT__SetCursorPos
*
* Sets a new cursor position.
*/
void EDIT__SetCursorPos(EDIT_Obj *pObj, int CursorPos) {
	if (pObj->pText) {
		char *pText = pObj->pText;
		int NumChars = GUI__GetNumChars(pText);
		int Offset = (pObj->EditMode == GUI_EDIT_MODE_INSERT) ? 0 : 1;
		if (CursorPos < 0)
			CursorPos = 0;
		if (CursorPos > NumChars)
			CursorPos = NumChars;
		if (CursorPos > pObj->MaxLen - Offset)
			CursorPos = pObj->MaxLen - Offset;
		if (pObj->CursorPos != CursorPos)
			pObj->CursorPos = CursorPos;
		pObj->SelSize = 0;
	}
}
static void _OnTouch(EDIT_Obj *pObj, const GUI_PID_STATE *pState) {
	GUI_USE_PARA(pObj);
	if (pState) {  /* Something happened in our area (pressed or released) */
		static int StartPress = 0;	//houhh 20061023...
		if (pState->Pressed) {
			GUI_DEBUG_LOG("EDIT__Callback(WM_TOUCH, Pressed, Handle %d)\n", 1);
			EDIT_SetCursorAtPixel(pObj, pState->x);
			StartPress = pObj->CursorPos;	//houhh 20061023...
		}
		else {
			GUI_DEBUG_LOG("EDIT__Callback(WM_TOUCH, Released, Handle %d)\n", 1);
		}
	}
	else {
		GUI_DEBUG_LOG("_EDIT_Callback(WM_TOUCH, Moved out, Handle %d)\n", 1);
	}
}
static int _OnKey(EDIT_Obj *pObj, const WM_KEY_INFO *pInfo) {
	if (pInfo->PressedCnt > 0) { /* Key pressed? */
		int Key = pInfo->Key;
		switch (Key) {
			case GUI_KEY_TAB:
				break; /* Send to parent by not doing anything */
			default:
				EDIT_AddKey(pObj, Key);
				return 1;
		}
	}
	return 0;
}
static WM_PARAM EDIT__Callback(WM_HWIN hWin, int MsgId, WM_PARAM Data) {
	EDIT_Obj *pObj = (EDIT_Obj *)hWin;
	int IsEnabled = WM_IsEnabled(pObj);
	/* Let widget handle the standard messages */
	if (!WIDGET_HandleActive(pObj, MsgId, &Data))
		return Data;
	switch (MsgId) {
		case WM_TOUCH:
			_OnTouch(pObj, (const GUI_PID_STATE *)Data);
			return 0;
		case WM_PAINT:
			_OnPaint(pObj);
			return 0;
		case WM_DELETE:
			_Delete(pObj);
			return 0;
		case WM_KEY:
			if (_OnKey(pObj, (const WM_KEY_INFO *)Data)) 
				return 0;
			break;
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}
EDIT_Handle EDIT_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int WinFlags, int ExFlags,
						  int Id, int MaxLen) {
	EDIT_Handle hObj;
	GUI_USE_PARA(ExFlags);
	/* Alloc memory for obj */
	WinFlags |= WM_CF_LATE_CLIP;    /* Always use late clipping since widget is optimized for it. */
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WM_CF_SHOW | WinFlags, EDIT__Callback,
								  sizeof(EDIT_Obj) - sizeof(WM_Obj));
	if (hObj) {
		EDIT_Obj *pObj = (EDIT_Obj *)hObj;

		pObj = (EDIT_Obj *)(hObj);
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE);
		/* init member variables */
		pObj->Props = EDIT_Obj::DefaultProps;
		pObj->XSizeCursor = 1;
		pObj->MaxLen = (MaxLen == 0) ? 8 : MaxLen;
		pObj->BufferSize = 0;
		pObj->pText = NULL;
		if (_IncrementBuffer(pObj, pObj->MaxLen + 1) == 0) {
			GUI_DEBUG_ERROROUT("EDIT_Create failed to alloc buffer");
			EDIT_Delete(hObj);
			hObj = 0;
		}

	}
	return hObj;
}
void EDIT_AddKey(EDIT_Handle hObj, int Key) {
	if (hObj) {
		EDIT_Obj *pObj = (EDIT_Obj *)hObj;
		if (pObj) {
			if (pObj->pfAddKeyEx) {
				pObj->pfAddKeyEx(hObj, Key);
			}
			else {
				switch (Key) {
					case GUI_KEY_UP:
						if (pObj->pText) {
							char *pText = pObj->pText;
							uint16_t Char;
							pText += GUI_UC__NumChars2NumBytes(pText, pObj->CursorPos);
							Char = GUI_UC_GetCharCode(pText);
							if (Char < 0x7f) {
								*pText = Char + 1;
								WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
							}
						}
						break;
					case GUI_KEY_DOWN:
						if (pObj->pText) {
							char *pText = pObj->pText;
							pText += GUI_UC__NumChars2NumBytes(pText, pObj->CursorPos);
							uint16_t Char = GUI_UC_GetCharCode(pText);
							if (Char > 0x20) {
								*pText = Char - 1;
								WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
							}
						}
						break;
					case GUI_KEY_RIGHT:
						EDIT__SetCursorPos(pObj, pObj->CursorPos + 1);
						break;
					case GUI_KEY_LEFT:
						EDIT__SetCursorPos(pObj, pObj->CursorPos - 1);
						break;
					case GUI_KEY_BACKSPACE:
						EDIT__SetCursorPos(pObj, pObj->CursorPos - 1);
						_DeleteChar(pObj);
						break;
					case GUI_KEY_DELETE:
						_DeleteChar(pObj);
						break;
					case GUI_KEY_INSERT:
						if (pObj->EditMode == GUI_EDIT_MODE_OVERWRITE) {
							pObj->EditMode = GUI_EDIT_MODE_INSERT;
						}
						else {
							pObj->EditMode = GUI_EDIT_MODE_OVERWRITE;
							EDIT__SetCursorPos(pObj, pObj->CursorPos);
						}
						break;
					case GUI_KEY_ENTER:
					case GUI_KEY_ESCAPE:
						break;
					default:
						if (Key >= 0x20) {
							if (pObj->EditMode != GUI_EDIT_MODE_INSERT) {
								_DeleteChar(pObj);
							}
							if (_InsertChar(pObj, Key)) {
								EDIT__SetCursorPos(pObj, pObj->CursorPos + 1);
							}
						}
				}
			}
			EDIT_Invalidate(hObj);
		}

	}
}
void EDIT_SetFont(EDIT_Handle hObj, const GUI_FONT  *pfont) {
	EDIT_Obj *pObj = (EDIT_Obj *)hObj;
	if (hObj == 0)
		return;

	if (pObj) {
		pObj->Props.pFont = pfont;
		EDIT_Invalidate(hObj);
	}

}
void EDIT_SetBkColor(EDIT_Handle hObj, unsigned int Index, RGB_COLOR color) {
	EDIT_Obj *pObj = (EDIT_Obj *)hObj;
	if (hObj == 0)
		return;

	if (pObj) {
		if (Index < GUI_COUNTOF(pObj->Props.aBkColor)) {
			pObj->Props.aBkColor[Index] = color;
			EDIT_Invalidate(hObj);
		}
	}

}
void EDIT_SetTextColor(EDIT_Handle hObj, unsigned int Index, RGB_COLOR color) {
	EDIT_Obj *pObj = (EDIT_Obj *)hObj;
	if (hObj == 0)
		return;
	if (pObj) {
		if (Index < GUI_COUNTOF(pObj->Props.aTextColor)) {
			pObj->Props.aTextColor[Index] = color;
			EDIT_Invalidate(hObj);
		}
	}
}
void EDIT_SetText(EDIT_Handle hObj, const char *s) {
	if (hObj) {
		EDIT_Obj *pObj = (EDIT_Obj *)hObj;
		if (s) {
			int NumBytesNew, NumBytesOld = 0;
			int NumCharsNew;
			if (pObj->pText) {
				char *pText = pObj->pText;
				NumBytesOld = GUI__strlen(pText) + 1;
			}
			NumCharsNew = GUI__GetNumChars(s);
			if (NumCharsNew > pObj->MaxLen) {
				NumCharsNew = pObj->MaxLen;
			}
			NumBytesNew = GUI_UC__NumChars2NumBytes(s, NumCharsNew) + 1;
			if (_IsSpaceInBuffer(pObj, NumBytesNew - NumBytesOld)) {
				char *pText = pObj->pText;
				GUI__memcpy(pText, s, NumBytesNew);
				pObj->CursorPos = NumBytesNew - 1;
				if (pObj->CursorPos == pObj->MaxLen) {
					if (pObj->EditMode == GUI_EDIT_MODE_OVERWRITE) {
						pObj->CursorPos--;
					}
				}
			}
		}
		else {
			GUI_ALLOC_FreePtr((void **)&pObj->pText);
			pObj->BufferSize = 0;
			pObj->CursorPos = 0;
		}
		EDIT_Invalidate(hObj);

	}
}
void EDIT_GetText(EDIT_Handle hObj, char *sDest, int MaxLen) {
	if (sDest) {
		*sDest = 0;
		if (hObj) {
			EDIT_Obj *pObj = (EDIT_Obj *)hObj;
			if (pObj->pText) {
				char *pText = pObj->pText;
				int NumChars = GUI__GetNumChars(pText);
				if (NumChars > MaxLen)
					NumChars = MaxLen;
				int NumBytes = GUI_UC__NumChars2NumBytes(pText, NumChars);
				GUI__memcpy(sDest, pText, NumBytes);
				*(sDest + NumBytes) = 0;
			}
		}
	}
}
int32_t  EDIT_GetValue(EDIT_Handle hObj) {
	EDIT_Obj *pObj = (EDIT_Obj *)hObj;
	int32_t r = 0;
	if (hObj) {

		r = pObj->CurrentValue;

	}
	return r;
}
void EDIT_SetValue(EDIT_Handle hObj, int32_t Value) {
	EDIT_Obj *pObj = (EDIT_Obj *)hObj;
	if (hObj) {
		/* Put in min/max range */
		if (Value < pObj->Min)
			Value = pObj->Min;
		if (Value > pObj->Max)
			Value = pObj->Max;
		if (pObj->CurrentValue != (uint32_t)Value) {
			pObj->CurrentValue = Value;
			if (pObj->pfUpdateBuffer)
				pObj->pfUpdateBuffer(hObj);
			WM_Invalidate(hObj);
			WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
		}
	}
}
void EDIT_SetMaxLen(EDIT_Handle  hObj, int MaxLen) {
	if (hObj) {
		EDIT_Obj *pObj = (EDIT_Obj *)hObj;
		if (MaxLen != pObj->MaxLen) {
			if (MaxLen < pObj->MaxLen) {
				if (pObj->pText) {
					char *pText = pObj->pText;
					int NumChars = GUI__GetNumChars(pText);
					if (NumChars > MaxLen) {
						int NumBytes;
						NumBytes = GUI_UC__NumChars2NumBytes(pText, MaxLen);
						*(pText + NumBytes) = 0;
					}
				}
			}
			_IncrementBuffer(pObj, MaxLen - pObj->BufferSize + 1);
			pObj->MaxLen = MaxLen;
			EDIT_Invalidate(hObj);
		}

	}
}
void EDIT_SetTextAlign(EDIT_Handle hObj, int Align) {
	EDIT_Obj *pObj = (EDIT_Obj *)hObj;
	if (hObj == 0)
		return;
	if (pObj) {
		pObj->Props.Align = Align;
		EDIT_Invalidate(hObj);
	}
}

EDIT_Handle EDIT_Create(int x0, int y0, int xsize, int ysize, int Id, int MaxLen, int Flags) {
	return EDIT_CreateEx(x0, y0, xsize, ysize, NULL, Flags, 0, Id, MaxLen);
}
EDIT_Handle EDIT_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, int MaxLen) {
	return EDIT_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, Id, MaxLen);
}

EDIT_Handle EDIT_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	EDIT_Handle hEdit;
	GUI_USE_PARA(cb);
	hEdit = EDIT_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
						  hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id, pCreateInfo->Para);
	if (hEdit) {
		EDIT_SetTextAlign(hEdit, pCreateInfo->Flags);
	}
	return hEdit;
}

void EDIT_SetDefaultFont(const GUI_FONT *pFont) {
	EDIT_Obj::DefaultProps.pFont = pFont;
}
const GUI_FONT  *EDIT_GetDefaultFont(void) {
	return EDIT_Obj::DefaultProps.pFont;
}
void EDIT_SetDefaultTextAlign(int Align) {
	EDIT_Obj::DefaultProps.Align = Align;
}
int EDIT_GetDefaultTextAlign(void) {
	return EDIT_Obj::DefaultProps.Align;
}
void EDIT_SetDefaultTextColor(unsigned int Index, RGB_COLOR Color) {
	if (Index <= GUI_COUNTOF(EDIT_Obj::DefaultProps.aTextColor)) {
		EDIT_Obj::DefaultProps.aTextColor[Index] = Color;
	}
}
void EDIT_SetDefaultBkColor(unsigned int Index, RGB_COLOR Color) {
	if (Index <= GUI_COUNTOF(EDIT_Obj::DefaultProps.aBkColor)) {
		EDIT_Obj::DefaultProps.aBkColor[Index] = Color;
	}
}
RGB_COLOR EDIT_GetDefaultTextColor(unsigned int Index) {
	RGB_COLOR Color = 0;
	if (Index <= GUI_COUNTOF(EDIT_Obj::DefaultProps.aTextColor)) {
		Color = EDIT_Obj::DefaultProps.aTextColor[Index];
	}
	return Color;
}
RGB_COLOR EDIT_GetDefaultBkColor(unsigned int Index) {
	RGB_COLOR Color = 0;
	if (Index <= GUI_COUNTOF(EDIT_Obj::DefaultProps.aBkColor)) {
		Color = EDIT_Obj::DefaultProps.aBkColor[Index];
	}
	return Color;
}

int EDIT_GetNumChars(EDIT_Handle hObj) {
	if (hObj) {
		EDIT_Obj *pObj = (EDIT_Obj *)hObj;
		if (pObj->pText) {
			return GUI__GetNumChars(pObj->pText);
		}
	}
	return 0;
}

void EDIT_SetCursorAtChar(EDIT_Handle hObj, int Pos) {
	if (hObj) {
		EDIT_Obj *pObj = (EDIT_Obj *)hObj;
		EDIT__SetCursorPos(pObj, Pos);
		EDIT_Invalidate(hObj);
	}
}

int EDIT_SetInsertMode(EDIT_Handle hObj, int OnOff) {
	int PrevMode = 0;
	if (hObj) {
		EDIT_Obj *pObj = (EDIT_Obj *)hObj;
		PrevMode = pObj->EditMode;
		pObj->EditMode = OnOff ? GUI_EDIT_MODE_INSERT : GUI_EDIT_MODE_OVERWRITE;
	}
	return PrevMode;
}

void EDIT_SetpfAddKeyEx(EDIT_Handle hObj, tEDIT_AddKeyEx *pfAddKeyEx) {
	if (hObj) {
		EDIT_Obj *pObj = (EDIT_Obj *)hObj;
		pObj->pfAddKeyEx = pfAddKeyEx;
	}
}

void EDIT_SetpfUpdateBuffer(EDIT_Handle hObj, tEDIT_UpdateBuffer *pfUpdateBuffer) {
	if (hObj) {
		EDIT_Obj *pObj = (EDIT_Obj *)hObj;
		pObj->pfUpdateBuffer = pfUpdateBuffer;
	}
}

void EDIT_SetSel(EDIT_Handle hObj, int FirstChar, int LastChar) {
	if (hObj) {
		EDIT_Obj *pObj = (EDIT_Obj *)hObj;
		if (FirstChar == -1) {
			pObj->SelSize = 0;
		}
		else {
			if (FirstChar > pObj->BufferSize - 1)
				FirstChar = pObj->BufferSize - 1;
			if (LastChar > pObj->BufferSize - 1)
				LastChar = pObj->BufferSize - 1;
			if (LastChar == -1)
				LastChar = EDIT_GetNumChars(hObj);
			if (LastChar >= FirstChar) {
				pObj->CursorPos = FirstChar;
				pObj->SelSize = LastChar - FirstChar + 1;
			}
		}

	}
}
