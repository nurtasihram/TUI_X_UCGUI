module;

#include "GUI_Protected.h"

export module TUX.Widget.MultEdit;

import TUX.Widget;

#define NUM_DISP_MODES 2

constexpr uint16_t
	INVALID_NUMCHARS = 1 << 0,
	INVALID_NUMLINES = 1 << 1,
	INVALID_TEXTSIZE = 1 << 2,
	INVALID_CURSORXY = 1 << 3,
	INVALID_LINEPOSB = 1 << 4;

#define MULTEDIT_REALLOC_SIZE  16

export {

constexpr uint16_t
	MULTEDIT_CF_READONLY         = WIDGET_STATE_USER<0>,
	MULTEDIT_CF_INSERT           = WIDGET_STATE_USER<1>,
	MULTEDIT_CF_AUTOSCROLLBAR_V  = WIDGET_STATE_USER<2>,
	MULTEDIT_CF_AUTOSCROLLBAR_H  = WIDGET_STATE_USER<3>;

enum MULTEDIT_CI {
	 MULTEDIT_CI_EDITMODE = 0,
	 MULTEDIT_CI_READONLY
};

class MultEdit : public Widget {

public:
	struct Properties {
		PCFONT pFont{ GUI_DEFAULT_FONT };
		RGBC aBkColor[NUM_DISP_MODES]{
			/* Edit mode */		RGB_WHITE,
			/* Read-only */		RGB_GRAYL(0xC0)
		};
		RGBC aColor[NUM_DISP_MODES]{
			/* Edit mode */		RGB_BLACK,
			/* Read-only */		RGB_BLACK
		};
		uint8_t HBorder{ 1 };
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;

	char *pText = nullptr;
	uint16_t MaxNumChars = 0;         /* Maximum number of characters including the prompt */
	uint16_t
		NumChars = 0,
		NumCharsPrompt = 0,
		NumLines = 0;
	uint16_t TextSizeX = 0; /* Size in X of text depending of wrapping mode */
	uint16_t BufferSize = 0;
	uint16_t
		CursorLine = 0,
		CursorPosChar = 0;
	uint16_t CursorPosByte = 0; /* Byte offset number of cursor */
	uint16_t CursorPosX = 0, CursorPosY = 0; /* Cursor position */
	uint16_t
		CacheLinePosByte = 0, CacheLineNumber = 0,
		CacheFirstVisibleLine = 0, CacheFirstVisibleByte = 0;
	WM_SCROLL_STATE ScrollStateV, ScrollStateH;
	uint8_t InvalidFlags = 0;         /* Flags to save validation status */
	uint8_t EditMode = 0;
	WRAPMODE WrapMode = WRAPMODE_NONE;

	void _InvalidateNumChars() {
		InvalidFlags |= INVALID_NUMCHARS;
	}
	int _GetNumChars() {
		if (InvalidFlags & INVALID_NUMCHARS) {
			NumChars = GUI__strlen(pText);
			InvalidFlags &= ~INVALID_NUMCHARS;
		}
		return NumChars;
	}
	int _GetXSize() {
		RECT Rect;
		WM_GetInsideRectExScrollbar(this, &Rect);
		return Rect.x1 - Rect.x0 - (Props.HBorder * 2) - 1;
	}
	int _GetNumCharsInPrompt(const char *pText) {
		int r = 0;
		auto pEndPrompt = this->pText + NumCharsPrompt;
		if (pText < pEndPrompt)
			r = (int)(pEndPrompt - pText);
		return r;
	}
	int _NumChars2XSize(const char *pText, int NumChars) {
		int xSize = 0;
		while (NumChars--)
			xSize += GUI.Font().GetCharSizeX(*pText++);
		return xSize;
	}
	int _WrapGetNumCharsDisp(const char *pText) {
		int xSize = _GetXSize();
		return GUI__WrapGetNumCharsDisp(pText, xSize, WrapMode);
	}
	int _WrapGetNumBytesToNextLine(const char *pText) {
		int xSize = _GetXSize();
		return GUI__WrapGetNumBytesToNextLine(pText, xSize, WrapMode);
	}
	int _GetCharSizeX(const char *pText) {
		return GUI.Font().GetCharSizeX(*pText);
	}
	void _DispString(const char *pText, RECT *pRect) {
		int NumCharsDisp = _WrapGetNumCharsDisp(pText);
		GUI_DispStringInRectMax(pText, pRect, TEXTALIGN_LEFT, NumCharsDisp);
	}
	char *_GetpLine(unsigned LineNumber) {
		char *pLine;
		if ((unsigned)CacheLineNumber != LineNumber) {
			if (LineNumber > (unsigned)CacheLineNumber) {
				/* If new line number > cache we can start with old pointer */
				int OldNumber = CacheLineNumber;
				pLine = pText + CacheLinePosByte;
				CacheLineNumber = LineNumber;
				LineNumber -= OldNumber;
			} else {
				/* If new line number < cache we need to start with first byte */
				pLine = pText;
				CacheLineNumber = LineNumber;
			}
			while (LineNumber--)
				pLine += _WrapGetNumBytesToNextLine(pLine);
			CacheLinePosByte = (uint16_t)(pLine - pText);
		}
		return pText + CacheLinePosByte;
	}
	void _ClearCache() {
		CacheLineNumber = 0;
		CacheLinePosByte = 0;
		CacheFirstVisibleByte = 0;
		CacheFirstVisibleLine = 0;
	}
	int _GetCursorLine(const char *pText, int CursorPosChar) {
		const char *pCursor;
		const char *pEndLine;
		int NumChars, ByteOffsetNewCursor, LineNumber = 0;
		ByteOffsetNewCursor = CursorPosChar;
		pCursor = pText + ByteOffsetNewCursor;
		if (CacheLinePosByte < ByteOffsetNewCursor) {
			/* If cache pos < new position we can use it as start position */
			pText += CacheLinePosByte;
			LineNumber += CacheLineNumber;
		}
		while (*pText && (pCursor > pText)) {
			NumChars = _WrapGetNumCharsDisp(pText);
			pEndLine = pText + NumChars;
			pText += _WrapGetNumBytesToNextLine(pText);
			if (pCursor <= pEndLine) {
				if ((pCursor == pEndLine) && (pEndLine == pText) && *pText)
					LineNumber++;
				break;
			}
			LineNumber++;
		}
		return LineNumber;
	}
	void _GetCursorXY(int *px, int *py) {
		if (InvalidFlags & INVALID_CURSORXY) {
			int x = 0;
			GUI.Font(Props.pFont);
			if (pText) {
				auto pLine = pText;
				auto pCursor = pLine + CursorPosByte;
				pLine = _GetpLine(CursorLine);
				while (pLine < pCursor) {
					x += _GetCharSizeX(pLine);
					pLine += 1;
				}
			}
			CursorPosX = x;
			CursorPosY = CursorLine * Props.pFont->YSize;
			InvalidFlags &= ~INVALID_CURSORXY;
		}
		*px = CursorPosX;
		*py = CursorPosY;
	}
	void _InvalidateCursorXY() {
		InvalidFlags |= INVALID_CURSORXY;
	}
	void _SetScrollState() {
		SetScrollState(ScrollStateV, ScrollStateH);
	}
	void _CalcScrollPos() {
		int xCursor, yCursor;
		_GetCursorXY(&xCursor, &yCursor);
		yCursor /= Props.pFont->YSize;
		ScrollStateV.CheckPos(yCursor, 0, 0);       /* Vertical */
		ScrollStateH.CheckPos(xCursor, 30, 30);     /* Horizontal */
		_SetScrollState();
	}
	int _GetTextSizeX() {
		if (InvalidFlags & INVALID_TEXTSIZE) {
			TextSizeX = 0;
			if (pText) {
				GUI.Font(Props.pFont);
				auto p = pText;
				do {
					int NumChars = _WrapGetNumCharsDisp(p);
					int xSizeLine = 0;
					auto pLine = p;
					while (NumChars--) {
						xSizeLine += _GetCharSizeX(pLine);
						pLine += 1;
					}
					if (xSizeLine > TextSizeX)
						TextSizeX = xSizeLine;
					p += _WrapGetNumBytesToNextLine(p);
				} while (*p);
			}
			InvalidFlags &= ~INVALID_TEXTSIZE;
		}
		return TextSizeX;
	}
	int _GetNumVisLines() {
		RECT Rect;
		WM_GetInsideRectExScrollbar(this, &Rect);
		return (Rect.y1 - Rect.y0 + 1) / Props.pFont->YSize;
	}
	int _GetNumLines() {
		if (InvalidFlags & INVALID_NUMLINES) {
			int NumLines = 0;
			if (pText) {
				auto p = pText;
				GUI.Font(Props.pFont);
				char Char;
				do {
					int NumChars = _WrapGetNumCharsDisp(p);
					Char = p[NumChars];
					if (Char)
						NumLines++;
					p += _WrapGetNumBytesToNextLine(p);
				} while (Char);
			}
			this->NumLines = NumLines + 1;
			InvalidFlags &= ~INVALID_NUMLINES;
		}
		return this->NumLines;
	}
	void _InvalidateNumLines() {
		InvalidFlags |= INVALID_NUMLINES;
	}
	void _InvalidateTextSizeX() {
		InvalidFlags |= INVALID_TEXTSIZE;
	}
	void _CalcScrollParas() {
		/* Calc vertical scroll parameters */
		ScrollStateV.NumItems = _GetNumLines();
		ScrollStateV.PageSize = _GetNumVisLines();
		/* Calc horizontal scroll parameters */
		ScrollStateH.NumItems = _GetTextSizeX();
		ScrollStateH.PageSize = _GetXSize();
		_CalcScrollPos();
	}
	void _ManageAutoScrollV() {
		if (States & MULTEDIT_CF_AUTOSCROLLBAR_V) {
			auto IsRequired = _GetNumVisLines() < _GetNumLines();
			if (WM_SetScrollbarV(this, IsRequired) != IsRequired) {
				_InvalidateNumLines();
				_InvalidateTextSizeX();
				_InvalidateCursorXY();
				_ClearCache();
			}
		}
	}
	void _ManageScrollers() {
		/* 1. Step: Check if vertical scrollbar is required */
		_ManageAutoScrollV();
		/* 2. Step: Check if horizontal scrollbar is required */
		if (States & MULTEDIT_CF_AUTOSCROLLBAR_H) {
			auto IsRequired = (_GetXSize() < _GetTextSizeX());
			if (WM_SetScrollbarH(this, IsRequired) != IsRequired) {
				/* 3. Step: Check vertical scrollbar again if horizontal has changed */
				_ManageAutoScrollV();
			}
		}
		_CalcScrollParas();
	}
	void _Invalidate() {
		_ManageScrollers();
		Invalidate();
	}
	void _InvalidateTextArea() {
		RECT rInsideRect;
		_ManageScrollers();
		WM_GetInsideRectExScrollbar(this, &rInsideRect);
		Invalidate(&rInsideRect);
	}
	int _InvalidateCursorPos() {
		int Value;
		Value = this->CursorPosChar;
		this->CursorPosChar = 0xffff;
		return Value;
	}
	int _CalcNextValidCursorPos(int CursorPosChar, int *pCursorPosByte, int *pCursorLine) {
		if (this->pText) {
			char *pNextLine, *pCursor, *pText;
			int CursorLine, NumChars, CursorPosByte;
			pText = this->pText;
			NumChars = _GetNumChars();
			/* Set offset in valid range */
			if (CursorPosChar < this->NumCharsPrompt) {
				CursorPosChar = this->NumCharsPrompt;
			}
			if (CursorPosChar > NumChars) {
				CursorPosChar = NumChars;
			}
			CursorPosByte = CursorPosChar;  // 1 char = 1 byte
			CursorLine = _GetCursorLine(pText, CursorPosChar);
			pCursor = pText + CursorPosByte;
			pNextLine = _GetpLine(CursorLine);
			if (pNextLine > pCursor) {
				if (this->CursorPosChar < CursorPosChar) {
					pCursor = pNextLine;
				}
				else {
					char *pPrevLine;
					int NumChars;
					pPrevLine = _GetpLine(CursorLine - 1);
					NumChars = _WrapGetNumCharsDisp(pPrevLine);
					pPrevLine += NumChars;  // 1 char = 1 byte
					pCursor = pPrevLine;
				}
				CursorPosChar = (int)(pCursor - pText);  // 1 char = 1 byte
				CursorPosByte = CursorPosChar;  // 1 char = 1 byte
				CursorLine = _GetCursorLine(pText, CursorPosChar);
			}
			if (pCursorPosByte) {
				*pCursorPosByte = CursorPosByte;
			}
			if (pCursorLine) {
				*pCursorLine = CursorLine;
			}
			return CursorPosChar;
		}
		return 0;
	}
	void _SetCursorPos(int CursorPosChar) {
		int CursorPosByte, CursorLine;
		CursorPosChar = _CalcNextValidCursorPos(CursorPosChar, &CursorPosByte, &CursorLine);
		/* Assign value and recalc whatever necessary */
		if (this->CursorPosChar != CursorPosChar) {
			/* Save values */
			this->CursorPosByte = CursorPosByte;
			this->CursorPosChar = CursorPosChar;
			this->CursorLine = CursorLine;
			_InvalidateCursorXY(); /* Invalidate X/Y position */
			_CalcScrollPos();
		}
	}
	void _SetWrapMode(WRAPMODE WrapMode) {
		if (this->WrapMode != WrapMode) {
			this->WrapMode = WrapMode;
			_ClearCache();
			_InvalidateNumLines();
			_InvalidateTextSizeX();
			_InvalidateTextArea();
			auto Position = _InvalidateCursorPos();
			_SetCursorPos(Position);
		}
	}
	void _SetCursorXY(int x, int y) {
		int CursorPosChar = 0;
		if ((x < 0) || (y < 0)) {
			return;
		}
		if (this->pText) {
			char *pLine, *pText;
			int CursorLine, WrapChars;
			int SizeX = 0;
			uint16_t Char;
			GUI.Font(Props.pFont);
			CursorLine = y / Props.pFont->YSize;
			pLine = _GetpLine(CursorLine);
			pText = this->pText;
			WrapChars = _WrapGetNumCharsDisp(pLine);
			Char = (uint16_t)pLine[WrapChars];  // 1 char = 1 byte (GUI_UC__NumChars2NumBytes simplified)
			if (!Char || (Char == '\n') || ((Char == ' ') && (this->WrapMode == WRAPMODE_WORD)))
				WrapChars++;
			while (--WrapChars > 0) {
				Char = (uint16_t)*pLine;  // 1 char = 1 byte
				SizeX += _GetCharSizeX(pLine);
				if (!Char || (SizeX > x)) {
					break;
				}
				pLine += 1;  // 1 char = 1 byte
			}
			CursorPosChar = (int)(pLine - pText);  // 1 char = 1 byte
		}
		_SetCursorPos(CursorPosChar);
	}
	void _MoveCursorUp() {
		int xPos, yPos;
		_GetCursorXY(&xPos, &yPos);
		yPos -= Props.pFont->YSize;
		_SetCursorXY(xPos, yPos);
	}
	void _MoveCursorDown() {
		int xPos, yPos;
		_GetCursorXY(&xPos, &yPos);
		yPos += Props.pFont->YSize;
		_SetCursorXY(xPos, yPos);
	}
	void _MoveCursor2NextLine() {
		int xPos, yPos;
		_GetCursorXY(&xPos, &yPos);
		yPos += Props.pFont->YSize;
		_SetCursorXY(0, yPos);
	}
	void _MoveCursor2LineEnd() {
		int xPos, yPos;
		_GetCursorXY(&xPos, &yPos);
		_SetCursorXY(0x7FFF, yPos);
	}
	void _MoveCursor2LinePos1() {
		int xPos, yPos;
		_GetCursorXY(&xPos, &yPos);
		_SetCursorXY(0, yPos);
	}
	bool _IsOverwriteAtThisChar() {
		if (pText && !(States & MULTEDIT_CF_INSERT)) {
			auto pText = this->pText + CursorPosByte;
			int Line1 = CursorLine;
			int CurPos = _CalcNextValidCursorPos(CursorPosChar + 1, 0, 0);
			int Line2 = _GetCursorLine(this->pText, CurPos);
			auto Char = *pText;
			if (Char) {
				if (Line1 == Line2)
					return true;
				if (Char != '\n')
					if ((Char != ' ') || (WrapMode == WRAPMODE_CHAR))
						return true;
			}
		}
		return false;
	}
	int _GetCursorSizeX() {
		if (_IsOverwriteAtThisChar())
			return _GetCharSizeX(pText + CursorPosByte);
		return 2;
	}
	bool _IncrementBuffer(unsigned AddBytes) {
		int NewSize = BufferSize + AddBytes;
		if (auto pNew = (char *)GUI_ALLOC_Realloc(pText, NewSize)) {
			if (!pText)
				*pNew = 0;
			BufferSize = NewSize;
			pText = pNew;
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
			if (!_IncrementBuffer(BytesNeeded + MULTEDIT_REALLOC_SIZE))
				return false;
		return true;
	}
	bool _IsCharsAvailable(int CharsNeeded) {
		if ((CharsNeeded > 0) && (MaxNumChars > 0)) {
			int NumChars = 0;
			if (pText)
				NumChars = _GetNumChars();
			if ((CharsNeeded + NumChars) > MaxNumChars)
				return false;
		}
		return true;
	}
	void _DeleteChar() {
		if (pText) {
			auto s = pText;
			int CursorOffset = CursorPosByte;
			if (CursorOffset < GUI__strlen(s)) {
				auto pCursor = s + CursorOffset;
				auto pLine = _GetpLine(CursorLine);
				int NumChars = _WrapGetNumCharsDisp(pLine);
				auto pEndLine = pLine + NumChars;
				pLine += _WrapGetNumBytesToNextLine(pLine);
				int NumBytes = (pCursor == pEndLine) ? (int)(pLine - pEndLine) : 1;
				GUI__strcpy(pCursor, pCursor + NumBytes);
				NotifyParent(WM_NOTIFICATION_VALUE_CHANGED);
				NumChars -= NumBytes;
				_InvalidateNumLines();
				_InvalidateTextSizeX();
				_InvalidateCursorXY();
				_ClearCache();
				CursorLine = _GetCursorLine(s, CursorPosChar);
			}
		}
	}
	bool _InsertChar(uint16_t Char) {
		if (_IsCharsAvailable(1) && _IsSpaceInBuffer(1)) {
			auto p = pText + CursorPosByte;
			GUI__memmove(p + 1, p, GUI__strlen(p) + 1);
			*p = (char)Char;
			NotifyParent(WM_NOTIFICATION_VALUE_CHANGED);
			NumChars += 1;
			_InvalidateNumLines();
			_InvalidateTextSizeX();
			_ClearCache();
			return true;
		}
		return false;
	}
	void _OnPaint() {
		// Initialize drawing parameters
		GUI.Font(Props.pFont);
		auto FontSizeY = Props.pFont->YSize;
		auto ScrollPosY = ScrollStateV.v;
		auto EffectSize = this->EffectSize();
		auto HBorder = Props.HBorder;
		auto xOff = EffectSize + HBorder - ScrollStateH.v;
		auto yOff = EffectSize - ScrollPosY * FontSizeY;

		// Draw background
		auto ColorIndex = States & MULTEDIT_CF_READONLY ? MULTEDIT_CI_READONLY : MULTEDIT_CI_EDITMODE;
		GUI.BkColor(Props.aBkColor[ColorIndex]);
		GUI.Color(Props.aColor[ColorIndex]);
		GUI_Clear();

		// Setup clipping rectangle
		RECT rClip = {
			EffectSize + HBorder,
			EffectSize,
			GetSizeX() - EffectSize - HBorder - 1,
			GetSizeY() - EffectSize - 1
		};
		auto prOldClip = SetUserClipRect(&rClip);

		// Draw text content
		if (pText) {
			auto pText = this->pText;
			int Line = 0;
			auto NumVisLines = _GetNumVisLines();

			// Setup text drawing rectangle
			RECT r{
				xOff,
				EffectSize,
				_GetXSize() + EffectSize + HBorder - 1,
				Rect.y1 - Rect.y0 + 1
			};

			// Use cached first visible line if available
			if (ScrollPosY >= CacheFirstVisibleLine && CacheFirstVisibleByte) {
				pText += CacheFirstVisibleByte;
				Line = CacheFirstVisibleLine;
			}

			// Draw visible lines
			do {
				// Cache first visible line position
				if (CacheFirstVisibleLine != ScrollPosY && Line == ScrollPosY) {
					CacheFirstVisibleByte = (uint16_t)(pText - this->pText);
					CacheFirstVisibleLine = ScrollPosY;
				}

				// Draw line if visible
				if (Line >= ScrollPosY && (Line - ScrollPosY) <= NumVisLines) {
					_DispString(pText, &r);
					r.y0 += FontSizeY;
				}

				pText += _WrapGetNumBytesToNextLine(pText);
				Line++;
			} while (*pText && (Line - ScrollPosY) <= NumVisLines);
		}

		// Draw cursor
		if (HasFocus()) {
			int x, y;
			_GetCursorXY(&x, &y);
			RECT rCursor = {
				x + xOff,
				y + yOff,
				x + xOff + _GetCursorSizeX() - 1,
				y + yOff + FontSizeY - 1
			};
			GUI_DrawRect(rCursor);
		}

		SetUserClipRect(prOldClip);
		DrawDown();
	}
	void _OnTouch(const PID_STATE *pState) {
		int Notification;
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (pState->Pressed) {
				auto Effect = EffectSize();
				auto xPos = pState->x + ScrollStateH.v - Effect - Props.HBorder;
				auto yPos = pState->y + ScrollStateV.v * Props.pFont->YSize - Effect;
				_SetCursorXY(xPos, yPos);
				_Invalidate();
				Notification = WM_NOTIFICATION_CLICKED;
			} else
				Notification = WM_NOTIFICATION_RELEASED;
		} else
			Notification = WM_NOTIFICATION_MOVED_OUT;
		NotifyParent(Notification);
	}
	bool _AddKey(uint16_t Key) {
		switch (Key) {
			case GUI_KEY_UP:
				_MoveCursorUp();
				break;
			case GUI_KEY_DOWN:
				_MoveCursorDown();
				break;
			case GUI_KEY_RIGHT:
				_SetCursorPos(CursorPosChar + 1);
				break;
			case GUI_KEY_LEFT:
				_SetCursorPos(CursorPosChar - 1);
				break;
			case GUI_KEY_END:
				_MoveCursor2LineEnd();
				break;
			case GUI_KEY_HOME:
				_MoveCursor2LinePos1();
				break;
			case GUI_KEY_BACKSPACE:
				if (!(States & MULTEDIT_CF_READONLY)) {
					if (CursorPosChar > NumCharsPrompt) {
						_SetCursorPos(CursorPosChar - 1);
						_DeleteChar();
					}
				}
				break;
			case GUI_KEY_DELETE:
				if (!(States & MULTEDIT_CF_READONLY))
					_DeleteChar();
				break;
			case GUI_KEY_INSERT:
				CtlStates(MULTEDIT_CF_INSERT, !(States & MULTEDIT_CF_INSERT));
				_InvalidateTextArea();
				return true;
			case GUI_KEY_ENTER:
				if (States & MULTEDIT_CF_READONLY)
					_MoveCursor2NextLine();
				else if (_InsertChar('\n'))
					_MoveCursor2NextLine();
				break;
			case GUI_KEY_ESCAPE:
				_InvalidateTextArea();
				return false;
			default:
				if (!(States & MULTEDIT_CF_READONLY) && (Key >= 0x20)) {
					if (_IsOverwriteAtThisChar())
						_DeleteChar();
					if (_InsertChar(Key))
						_SetCursorPos(CursorPosChar + 1);
				} else {
					_InvalidateTextArea();
					return false;
				}
		}
		_InvalidateTextArea();
		return true;
	}
	bool _OnKey(const WM_KEY_INFO *pInfo) {
		if (pInfo->PressedCnt > 0) {
			int Key = pInfo->Key;
			if (_AddKey(Key))
				return true;
		} else if (!(States & MULTEDIT_CF_READONLY))
			return true; /* Key release is consumed (not sent to parent) */
		return false; /* Key release is not consumed (sent to parent) */
	}

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (MultEdit *)hWin;
		/* Let widget handle the standard messages */
		if (!pObj->HandleActive(MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_NOTIFY_CLIENTCHANGE:
				pObj->_InvalidateCursorXY();
				pObj->_InvalidateNumLines();
				pObj->_InvalidateTextSizeX();
				pObj->_ClearCache();
				pObj->_CalcScrollParas();
				return 0;
			case WM_SIZE:
				pObj->_InvalidateCursorXY();
				pObj->_InvalidateNumLines();
				pObj->_InvalidateTextSizeX();
				pObj->_ClearCache();
				pObj->_Invalidate();
				return 0;
			case WM_NOTIFY_PARENT: {
				auto pInfo = (const NOTIFY_INFO *)Data;
				auto pWinSrc = pInfo->pWinSrc;
				switch (pInfo->Notification) {
					case WM_NOTIFICATION_VALUE_CHANGED: {
						WM_SCROLL_STATE ScrollState;
						if (pWinSrc == pObj->GetScrollbarV()) {
							WM_GetScrollState(pWinSrc, &ScrollState);
							pObj->ScrollStateV.v = ScrollState.v;
							pObj->Invalidate();
							pObj->NotifyParent(WM_NOTIFICATION_SCROLL_CHANGED);
						}
						else if (pWinSrc == pObj->GetScrollbarH()) {
							WM_GetScrollState(pWinSrc, &ScrollState);
							pObj->ScrollStateH.v = ScrollState.v;
							pObj->Invalidate();
							pObj->NotifyParent(WM_NOTIFICATION_SCROLL_CHANGED);
						}
						break;
					}
					case WM_NOTIFICATION_SCROLLBAR_ADDED:
						pObj->_SetScrollState();
						break;
				}
				return 0;
			}
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
				return 0;
			case WM_DELETE:
				GUI_ALLOC_FreePtr((void **)&pObj->pText);
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const WM_KEY_INFO *)Data))
					return 0;
				break;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

private:
	static void _AdjRect(RECT &r, WObj *pParent) {
		auto Rect = pParent->GetClientRect();
		if (!r.x0)
			r.x0 = Rect.x0;
		if (!r.y1)
			r.y0 = Rect.y0;
		if (r.x1 <= r.x0)
			r.x1 = Rect.x1;
		if (r.y1 <= r.y0)
			r.y1 = Rect.y1;
	}
public:
	MultEdit(RECT r, WM_CF Style, WObj *pParent, uint16_t Id,
			 uint16_t ExFlags, uint16_t BufferSize, const char *pText) :
		Widget((_AdjRect(r, pParent), r), Style, _Callback, pParent, Id, ExFlags | WIDGET_STATE_FOCUSSABLE),
		BufferSize(BufferSize) {
		if (BufferSize > 0)
			this->pText = (char *)GUI_ALLOC_AllocZero(BufferSize);
		SetText(pText);
		_ManageScrollers();
	}	
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new MultEdit(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			pCreateInfo->Flags, hWinParent, pCreateInfo->Id,
			0, (uint16_t)pCreateInfo->Para, nullptr);
	}

public:

#pragma region Properties

	void Font(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		_InvalidateTextArea();
		_InvalidateCursorXY();
		_InvalidateNumLines();
		_InvalidateTextSizeX();
	}

	void BkColor(MULTEDIT_CI Index, RGBC color) {
		if (Index >= GUI_COUNTOF(Props.aBkColor))
			return;
		if (Props.aBkColor[Index] == color)
			return;
		Props.aBkColor[Index] = color;
		_InvalidateTextArea();
	}

	void TextColor(MULTEDIT_CI Index, RGBC color) {
		if (Index >= GUI_COUNTOF(Props.aColor))
			return;
		if (Props.aColor[Index] == color)
			return;
		Props.aColor[Index] = color;
		Invalidate();
	}

	void SetHBorder(uint8_t HBorder) {
		if (Props.HBorder == HBorder)
			return;
		Props.HBorder = HBorder;
		_Invalidate();
	}
	
#pragma endregion

	int  AddKey(uint16_t Key) {
		int r = 0;
		r = _AddKey(Key);
		return r;
	}

	void SetText(const char *pNew) {
		int NumCharsNew = 0, NumCharsOld = 0;
		int NumBytesNew = 0, NumBytesOld = 0;
		char *pText;

		if (this->pText) {
			pText = (char *)(this->pText);
			pText += this->NumCharsPrompt;  // 1 char = 1 byte
			NumCharsOld = GUI__strlen(pText);
			NumBytesOld = NumCharsOld;  // 1 char = 1 byte
		}
		if (pNew) {
			NumCharsNew = GUI__strlen(pNew);
			NumBytesNew = NumCharsNew;  // 1 char = 1 byte
		}
		if (_IsCharsAvailable(NumCharsNew - NumCharsOld)) {
			if (_IsSpaceInBuffer(NumBytesNew - NumBytesOld)) {
				pText = (char *)(this->pText);
				pText += this->NumCharsPrompt;  // 1 char = 1 byte
				if (pNew) {
					GUI__strcpy(pText, pNew);
				}
				else {
					*pText = 0;
				}
				_SetCursorPos(this->NumCharsPrompt);
				_InvalidateTextArea();
				_InvalidateNumChars();
				_InvalidateNumLines();
				_InvalidateTextSizeX();
			}
		}
	}
	void GetText(char *sDest, int MaxLen) {
		char *pText;
		int Len;
		pText = (char *)(this->pText);
		pText += this->NumCharsPrompt;  // 1 char = 1 byte
		Len = GUI__strlen(pText);
		if (Len > (MaxLen - 1)) {
			Len = MaxLen - 1;
		}
		GUI__memcpy(sDest, pText, Len);
		*(sDest + Len) = 0;
	}

	void GetPrompt(char *sDest, int MaxLen) {
		auto sSource = (char *)(this->pText);
		int Len = this->NumCharsPrompt;  // 1 char = 1 byte
		if (Len > (MaxLen - 1)) {
			Len = MaxLen - 1;
		}
		GUI__memcpy(sDest, sSource, Len);
		*(sDest + Len) = 0;
	}

	void SetWrapWord() { _SetWrapMode(WRAPMODE_WORD); }
	void SetWrapChar() { _SetWrapMode(WRAPMODE_CHAR); }
	void SetWrapNone() { _SetWrapMode(WRAPMODE_NONE); }

	void SetAutoScrollV(bool bOn)
	{ CtlStates(MULTEDIT_CF_AUTOSCROLLBAR_V, bOn); }
	void SetAutoScrollH(bool bOn)
	{ CtlStates(MULTEDIT_CF_AUTOSCROLLBAR_H, bOn); }

	void SetInsertMode(bool bOn)
	{ CtlStates(MULTEDIT_CF_INSERT, bOn); }
	void SetReadOnly(bool bOn)
	{ CtlStates(MULTEDIT_CF_READONLY, bOn); }

	void SetCursorOffset(int Offset) {
		_SetCursorPos(Offset);
		Invalidate();
	}
	void SetPrompt(const char *pPrompt) {
		int NumCharsNew = 0, NumCharsOld = 0;
		int NumBytesNew = 0, NumBytesOld = 0;
		char *pText;
		if (this->pText) {
			pText = (char *)(this->pText);
			NumCharsOld = this->NumCharsPrompt;
			NumBytesOld = NumCharsOld;  // 1 char = 1 byte
		}
		if (pPrompt) {
			NumCharsNew = GUI__strlen(pPrompt);
			NumBytesNew = NumCharsNew;  // 1 char = 1 byte
		}
		if (_IsCharsAvailable(NumCharsNew - NumCharsOld)) {
			if (_IsSpaceInBuffer(NumBytesNew - NumBytesOld)) {
				pText = (char *)(this->pText);
				GUI__memmove(pText + NumBytesNew, pText + NumBytesOld, GUI__strlen(pText + NumBytesOld) + 1);
				if (pPrompt) {
					GUI__memcpy(pText, pPrompt, NumBytesNew);
				}
				this->NumCharsPrompt = NumCharsNew;
				_SetCursorPos(NumCharsNew);
				_InvalidateTextArea();
				_InvalidateNumChars();
				_InvalidateNumLines();
				_InvalidateTextSizeX();
			}
		}
	}
	void SetBufferSize(int BufferSize) {
		auto pText = (char *)GUI_ALLOC_AllocZero(BufferSize);
		if (!pText) {
		}
		else {
			GUI_ALLOC_FreePtr((void **)&this->pText);
			this->pText = pText;
			this->BufferSize = BufferSize;
			this->NumCharsPrompt = 0;
			_SetCursorPos(0);
			_InvalidateNumChars();
			_InvalidateCursorXY();
			_InvalidateNumLines();
			_InvalidateTextSizeX();
		}
		_InvalidateTextArea();
	}
	void SetMaxNumChars(unsigned MaxNumChars) {
		this->MaxNumChars = MaxNumChars;
		if (MaxNumChars < (unsigned)this->NumCharsPrompt) {
			this->NumCharsPrompt = MaxNumChars;
		}
		if (this->pText && MaxNumChars) {
			char *pText;
			int Offset;
			pText = (char *)(this->pText);
			Offset = MaxNumChars;  // 1 char = 1 byte
			if (Offset < this->BufferSize) {
				pText += Offset;
				*pText = 0;
				_SetCursorPos(Offset);
				_InvalidateTextArea();
				_InvalidateNumChars();
			}
		}
	}

	int  GetTextSize() {
		int r = 0;
		if (this->pText) {
			const char *s;
			s = (const char *)this->pText;
			s += this->NumCharsPrompt;  // 1 char = 1 byte
			r = 1 + GUI__strlen(s);
		}

		return r;
	}

};

MultEdit::Properties MultEdit::DefaultProps;

}
