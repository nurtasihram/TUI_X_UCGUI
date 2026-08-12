module;

#include "GUI_Protected.h"

export module TUX.Widget.MultEdit;

import TUX.Widget;

#define NUM_DISP_MODES 2

/* Define character for password mode */
#define MULTEDIT_PASSWORD_CHAR   '*'

#define INVALID_NUMCHARS (1 << 0)
#define INVALID_NUMLINES (1 << 1)
#define INVALID_TEXTSIZE (1 << 2)
#define INVALID_CURSORXY (1 << 3)
#define INVALID_LINEPOSB (1 << 4)

#define MULTEDIT_REALLOC_SIZE  16

export {

constexpr uint16_t
	MULTEDIT_CF_READONLY         = 1 << 0,
	MULTEDIT_CF_INSERT           = 1 << 2,
	MULTEDIT_CF_AUTOSCROLLBAR_V  = 1 << 3,
	MULTEDIT_CF_AUTOSCROLLBAR_H  = 1 << 4,
	MULTEDIT_CF_PASSWORD         = 1 << 5;

enum MULTEDIT_CI {
	 MULTEDIT_CI_EDITMODE = 0,
	 MULTEDIT_CI_READONLY
};

class MultEdit : public WIDGET {

public:
	struct Properties {
		PCFONT pFont{ &FontProp13_1 };
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
	Properties Props;

	WM_HMEM hText;
	uint16_t MaxNumChars;         /* Maximum number of characters including the prompt */
	uint16_t NumChars;            /* Number of characters (text and prompt) in object */
	uint16_t NumCharsPrompt;      /* Number of prompt characters */
	uint16_t NumLines;            /* Number of text lines needed to show all data */
	uint16_t TextSizeX;           /* Size in X of text depending of wrapping mode */
	uint16_t BufferSize;
	uint16_t CursorLine;          /* Number of current cursor line */
	uint16_t CursorPosChar;       /* Character offset number of cursor */
	uint16_t CursorPosByte;       /* Byte offset number of cursor */
	uint16_t CursorPosX, CursorPosY; /* Cursor position */
	uint16_t CacheLinePosByte;    /*  */
	uint16_t CacheLineNumber;     /*  */
	uint16_t CacheFirstVisibleLine, CacheFirstVisibleByte;
	WM_SCROLL_STATE ScrollStateV, ScrollStateH;
	uint8_t Flags;
	uint8_t InvalidFlags;         /* Flags to save validation status */
	uint8_t EditMode;
	GUI_WRAPMODE WrapMode;

	void _InvalidateNumChars() {
		this->InvalidFlags |= INVALID_NUMCHARS;
	}
	int _GetNumChars() {
		if (this->InvalidFlags & INVALID_NUMCHARS) {
			char *pText;
			pText = (char *)(this->hText);
			this->NumChars = GUI__GetNumChars(pText);
			this->InvalidFlags &= ~INVALID_NUMCHARS;
		}
		return this->NumChars;
	}
	int _GetXSize() {
		RECT Rect;
		WM_GetInsideRectExScrollbar(this, &Rect);
		return Rect.x1 - Rect.x0 - (Props.HBorder * 2) - 1;
	}
	int _GetNumCharsInPrompt(const char *pText) {
		char *pString, *pEndPrompt;
		int r = 0;
		pString = (char *)(this->hText);
		pEndPrompt = pString + GUI_UC__NumChars2NumBytes(pString, this->NumCharsPrompt);
		if (pText < pEndPrompt) {
			r = GUI_UC__NumBytes2NumChars(pText, (int)(pEndPrompt - pText));
		}
		return r;
	}
	int _NumChars2XSize(const char *pText, int NumChars) {
		int xSize = 0;
		uint16_t Char;
		while (NumChars--) {
			Char = GUI_UC__GetCharCodeInc(&pText);
			xSize += GUI_GetCharDistX(Char);
		}
		return xSize;
	}
	int _WrapGetNumCharsDisp(const char *pText) {
		int xSize, r;
		xSize = _GetXSize();
		if (this->Flags & MULTEDIT_CF_PASSWORD) {
			int NumCharsPrompt;
			NumCharsPrompt = _GetNumCharsInPrompt(pText);
			r = GUI__WrapGetNumCharsDisp(pText, xSize, this->WrapMode);
			if (r >= NumCharsPrompt) {
				int x;
				switch (this->WrapMode) {
					case GUI_WRAPMODE_NONE:
						r = GUI__GetNumChars(pText);
						break;
					default:
						r = NumCharsPrompt;
						x = _NumChars2XSize(pText, NumCharsPrompt);
						pText += GUI_UC__NumChars2NumBytes(pText, NumCharsPrompt);
						while (GUI_UC__GetCharCodeInc(&pText) != 0) {
							x += GUI_GetCharDistX(MULTEDIT_PASSWORD_CHAR);
							if (r && (x > xSize)) {
								break;
							}
							r++;
						}
						break;
				}
			}
		}
		else {
			r = GUI__WrapGetNumCharsDisp(pText, xSize, this->WrapMode);
		}
		return r;
	}
	int _WrapGetNumBytesToNextLine(const char *pText) {
		int xSize, r;
		xSize = _GetXSize();
		if (this->Flags & MULTEDIT_CF_PASSWORD) {
			int NumChars, NumCharsPrompt;
			NumCharsPrompt = _GetNumCharsInPrompt(pText);
			NumChars = _WrapGetNumCharsDisp(pText);
			r = GUI_UC__NumChars2NumBytes(pText, NumChars);
			if (NumChars < NumCharsPrompt) {
				if (*(pText + r) == '\n') {
					r++;
				}
			}
		}
		else {
			r = GUI__WrapGetNumBytesToNextLine(pText, xSize, this->WrapMode);
		}
		return r;
	}
	int _GetCharDistX(const char *pText) {
		int r;
		if ((this->Flags & MULTEDIT_CF_PASSWORD) && (_GetNumCharsInPrompt(pText) == 0)) {
			r = GUI_GetCharDistX(MULTEDIT_PASSWORD_CHAR);
		}
		else {
			uint16_t c;
			c = GUI_UC_GetCharCode(pText);
			r = GUI_GetCharDistX(c);
		}
		return r;
	}
	void _DispString(const char *pText, RECT *pRect) {
		int NumCharsDisp;
		NumCharsDisp = _WrapGetNumCharsDisp(pText);
		if (this->Flags & MULTEDIT_CF_PASSWORD) {
			int x, NumCharsPrompt, NumCharsLeft = 0;
			NumCharsPrompt = _GetNumCharsInPrompt(pText);
			if (NumCharsDisp < NumCharsPrompt) {
				NumCharsPrompt = NumCharsDisp;
			}
			else {
				NumCharsLeft = NumCharsDisp - NumCharsPrompt;
			}
			GUI_DispStringInRectMax(pText, pRect, TEXTALIGN_LEFT, NumCharsPrompt);
			x = pRect->x0 + _NumChars2XSize(pText, NumCharsPrompt);
			if (NumCharsLeft) {
				GUI_DispCharAt(MULTEDIT_PASSWORD_CHAR, x, pRect->y0);
				GUI_DispChars(MULTEDIT_PASSWORD_CHAR, NumCharsLeft - 1);
			}
		}
		else {
			GUI_DispStringInRectMax(pText, pRect, TEXTALIGN_LEFT, NumCharsDisp);
		}
	}
	char *_GetpLine(unsigned LineNumber) {
		char *pText, *pLine;
		pText = (char *)(this->hText);
		if ((unsigned)this->CacheLineNumber != LineNumber) {
			if (LineNumber > (unsigned)this->CacheLineNumber) {
				/* If new line number > cache we can start with old pointer */
				int OldNumber = this->CacheLineNumber;
				pLine = pText + this->CacheLinePosByte;
				this->CacheLineNumber = LineNumber;
				LineNumber -= OldNumber;
			}
			else {
				/* If new line number < cache we need to start with first byte */
				pLine = pText;
				this->CacheLineNumber = LineNumber;
			}
			while (LineNumber--) {
				pLine += _WrapGetNumBytesToNextLine(pLine);
			}
			this->CacheLinePosByte = (uint16_t)(pLine - pText);
		}
		return pText + this->CacheLinePosByte;
	}
	void _ClearCache() {
		this->CacheLineNumber = 0;
		this->CacheLinePosByte = 0;
		this->CacheFirstVisibleByte = 0;
		this->CacheFirstVisibleLine = 0;
	}
	int _GetCursorLine(const char *pText, int CursorPosChar) {
		const char *pCursor;
		const char *pEndLine;
		int NumChars, ByteOffsetNewCursor, LineNumber = 0;
		ByteOffsetNewCursor = GUI_UC__NumChars2NumBytes(pText, CursorPosChar);
		pCursor = pText + ByteOffsetNewCursor;
		if (this->CacheLinePosByte < ByteOffsetNewCursor) {
			/* If cache pos < new position we can use it as start position */
			pText += this->CacheLinePosByte;
			LineNumber += this->CacheLineNumber;
		}
		while (*pText && (pCursor > pText)) {
			NumChars = _WrapGetNumCharsDisp(pText);
			pEndLine = pText + GUI_UC__NumChars2NumBytes(pText, NumChars);
			pText += _WrapGetNumBytesToNextLine(pText);
			if (pCursor <= pEndLine) {
				if ((pCursor == pEndLine) && (pEndLine == pText) && *pText) {
					LineNumber++;
				}
				break;
			}
			LineNumber++;
		}
		return LineNumber;
	}
	void _GetCursorXY(int *px, int *py) {
		if (this->InvalidFlags & INVALID_CURSORXY) {
			int CursorLine = 0, x = 0;
			GUI.SetFont(Props.pFont);
			if (this->hText) {
				const char *pLine;
				const char *pCursor;
				pLine = (const char *)(this->hText);
				pCursor = pLine + this->CursorPosByte;
				CursorLine = this->CursorLine;
				pLine = _GetpLine(CursorLine);
				while (pLine < pCursor) {
					x += _GetCharDistX(pLine);
					pLine += GUI_UC_GetCharSize(pLine);
				}
			}
			this->CursorPosX = x;
			this->CursorPosY = CursorLine * Props.pFont->DistY();
			this->InvalidFlags &= ~INVALID_CURSORXY;
		}
		*px = this->CursorPosX;
		*py = this->CursorPosY;
	}
	void _InvalidateCursorXY() {
		this->InvalidFlags |= INVALID_CURSORXY;
	}
	void _SetScrollState() {
		SetScrollState(ScrollStateV, ScrollStateH);
	}
	void _CalcScrollPos() {
		int xCursor, yCursor;
		_GetCursorXY(&xCursor, &yCursor);
		yCursor /= Props.pFont->DistY();
		ScrollStateV.CheckPos(yCursor, 0, 0);       /* Vertical */
		ScrollStateH.CheckPos(xCursor, 30, 30);     /* Horizontal */
		_SetScrollState();
	}
	int _GetTextSizeX() {
		if (this->InvalidFlags & INVALID_TEXTSIZE) {
			this->TextSizeX = 0;
			if (this->hText) {
				int NumChars, xSizeLine;
				char *pText, *pLine;
				GUI.SetFont(Props.pFont);
				pText = (char *)(this->hText);
				do {
					NumChars = _WrapGetNumCharsDisp(pText);
					xSizeLine = 0;
					pLine = pText;
					while (NumChars--) {
						xSizeLine += _GetCharDistX(pLine);
						pLine += GUI_UC_GetCharSize(pLine);
					}
					if (xSizeLine > this->TextSizeX) {
						this->TextSizeX = xSizeLine;
					}
					pText += _WrapGetNumBytesToNextLine(pText);
				} while (*pText);
			}
			this->InvalidFlags &= ~INVALID_TEXTSIZE;
		}
		return this->TextSizeX;
	}
	int _GetNumVisLines() {
		RECT Rect;
		WM_GetInsideRectExScrollbar(this, &Rect);
		return (Rect.y1 - Rect.y0 + 1) / Props.pFont->DistY();
	}
	int _GetNumLines() {
		if (this->InvalidFlags & INVALID_NUMLINES) {
			int NumLines = 0;
			if (this->hText) {
				int NumChars, NumBytes;
				char *pText;
				uint16_t Char;
				pText = (char *)(this->hText);
				GUI.SetFont(Props.pFont);
				do {
					NumChars = _WrapGetNumCharsDisp(pText);
					NumBytes = GUI_UC__NumChars2NumBytes(pText, NumChars);
					Char = GUI_UC_GetCharCode(pText + NumBytes);
					if (Char) {
						NumLines++;
					}
					pText += _WrapGetNumBytesToNextLine(pText);
				} while (Char);
			}
			this->NumLines = NumLines + 1;
			this->InvalidFlags &= ~INVALID_NUMLINES;
		}
		return this->NumLines;
	}
	void _InvalidateNumLines() {
		this->InvalidFlags |= INVALID_NUMLINES;
	}
	void _InvalidateTextSizeX() {
		this->InvalidFlags |= INVALID_TEXTSIZE;
	}
	void _CalcScrollParas() {
		/* Calc vertical scroll parameters */
		this->ScrollStateV.NumItems = _GetNumLines();
		this->ScrollStateV.PageSize = _GetNumVisLines();
		/* Calc horizontal scroll parameters */
		this->ScrollStateH.NumItems = _GetTextSizeX();
		this->ScrollStateH.PageSize = _GetXSize();
		_CalcScrollPos();
	}
	void _ManageAutoScrollV() {
		if (this->Flags & MULTEDIT_CF_AUTOSCROLLBAR_V) {
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
		if (this->Flags & MULTEDIT_CF_AUTOSCROLLBAR_H) {
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
		WM_Invalidate(this);
	}
	void _InvalidateTextArea() {
		RECT rInsideRect;
		_ManageScrollers();
		WM_GetInsideRectExScrollbar(this, &rInsideRect);
		WM_Invalidate(this, &rInsideRect);
	}
	int _InvalidateCursorPos() {
		int Value;
		Value = this->CursorPosChar;
		this->CursorPosChar = 0xffff;
		return Value;
	}
	void _SetFlag(int OnOff, uint8_t Flag) {
		if (OnOff) {
			this->Flags |= Flag;
		}
		else {
			this->Flags &= ~Flag;
		}
		_InvalidateTextArea();
	}
	int _CalcNextValidCursorPos(int CursorPosChar, int *pCursorPosByte, int *pCursorLine) {
		if (this->hText) {
			char *pNextLine, *pCursor, *pText;
			int CursorLine, NumChars, CursorPosByte;
			pText = (char *)(this->hText);
			NumChars = _GetNumChars();
			/* Set offset in valid range */
			if (CursorPosChar < this->NumCharsPrompt) {
				CursorPosChar = this->NumCharsPrompt;
			}
			if (CursorPosChar > NumChars) {
				CursorPosChar = NumChars;
			}
			CursorPosByte = GUI_UC__NumChars2NumBytes(pText, CursorPosChar);
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
					pPrevLine += GUI_UC__NumChars2NumBytes(pPrevLine, NumChars);
					pCursor = pPrevLine;
				}
				CursorPosChar = GUI_UC__NumBytes2NumChars(pText, (int)(pCursor - pText));
				CursorPosByte = GUI_UC__NumChars2NumBytes(pText, CursorPosChar);
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
	int _SetWrapMode(GUI_WRAPMODE WrapMode) {
		int r;
		r = 0;
		r = this->WrapMode;
		if (this->WrapMode != WrapMode) {
			int Position;
			this->WrapMode = WrapMode;
			_ClearCache();
			_InvalidateNumLines();
			_InvalidateTextSizeX();
			_InvalidateTextArea();
			Position = _InvalidateCursorPos();
			_SetCursorPos(Position);
		}

		return r;
	}
	void _SetCursorXY(int x, int y) {
		int CursorPosChar = 0;
		if ((x < 0) || (y < 0)) {
			return;
		}
		if (this->hText) {
			char *pLine, *pText;
			int CursorLine, WrapChars;
			int SizeX = 0;
			uint16_t Char;
			GUI.SetFont(Props.pFont);
			CursorLine = y / Props.pFont->DistY();
			pLine = _GetpLine(CursorLine);
			pText = (char *)(this->hText);
			WrapChars = _WrapGetNumCharsDisp(pLine);
			Char = GUI_UC__GetCharCode(pLine + GUI_UC__NumChars2NumBytes(pLine, WrapChars));
			if (this->Flags & MULTEDIT_CF_PASSWORD) {
				if (!Char) {
					WrapChars++;
				}
			}
			else {
				if (!Char || (Char == '\n') || ((Char == ' ') && (this->WrapMode == GUI_WRAPMODE_WORD))) {
					WrapChars++;
				}
			}
			while (--WrapChars > 0) {
				Char = GUI_UC_GetCharCode(pLine);
				SizeX += _GetCharDistX(pLine);
				if (!Char || (SizeX > x)) {
					break;
				}
				pLine += GUI_UC_GetCharSize(pLine);
			}
			CursorPosChar = GUI_UC__NumBytes2NumChars(pText, (int)(pLine - pText));
		}
		_SetCursorPos(CursorPosChar);
	}
	void _MoveCursorUp() {
		int xPos, yPos;
		_GetCursorXY(&xPos, &yPos);
		yPos -= Props.pFont->DistY();
		_SetCursorXY(xPos, yPos);
	}
	void _MoveCursorDown() {
		int xPos, yPos;
		_GetCursorXY(&xPos, &yPos);
		yPos += Props.pFont->DistY();
		_SetCursorXY(xPos, yPos);
	}
	void _MoveCursor2NextLine() {
		int xPos, yPos;
		_GetCursorXY(&xPos, &yPos);
		yPos += Props.pFont->DistY();
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
	int _IsOverwriteAtThisChar() {
		int r = 0;
		if (this->hText && !(this->Flags & MULTEDIT_CF_INSERT)) {
			const char *pText;
			int CurPos, Line1, Line2;
			uint16_t Char;
			pText = (const char *)(this->hText);
			Line1 = this->CursorLine;
			CurPos = _CalcNextValidCursorPos(CursorPosChar + 1, 0, 0);
			Line2 = _GetCursorLine(pText, CurPos);
			pText += this->CursorPosByte;
			Char = GUI_UC_GetCharCode(pText);
			if (Char) {
				if ((Line1 == Line2) || (this->Flags & MULTEDIT_CF_PASSWORD)) {
					r = 1;
				}
				else {
					if (Char != '\n') {
						if ((Char != ' ') || (this->WrapMode == GUI_WRAPMODE_CHAR)) {
							r = 1;
						}
					}
				}
			}
		}
		return r;
	}
	int _GetCursorSizeX() {
		if (_IsOverwriteAtThisChar()) {
			const char *pText;
			pText = (const char *)(this->hText);
			pText += this->CursorPosByte;
			return _GetCharDistX(pText);
		}
		else {
			return 2;
		}
	}
	int _IncrementBuffer(unsigned AddBytes) {
		WM_HMEM hNew;
		int NewSize;
		NewSize = this->BufferSize + AddBytes;
		hNew = GUI_ALLOC_Realloc(this->hText, NewSize);
		if (hNew) {
			if (!(this->hText)) {
				char *pText;
				pText = (char *)(hNew);
				*pText = 0;
			}
			this->BufferSize = NewSize;
			this->hText = hNew;
			return 1;
		}
		return 0;
	}
	int _IsSpaceInBuffer(int BytesNeeded) {
		int NumBytes = 0;
		if (this->hText) {
			NumBytes = GUI__strlen((char *)(this->hText));
		}
		BytesNeeded = (BytesNeeded + NumBytes + 1) - this->BufferSize;
		if (BytesNeeded > 0) {
			if (!_IncrementBuffer(BytesNeeded + MULTEDIT_REALLOC_SIZE)) {
				return 0;
			}
		}
		return 1;
	}
	int _IsCharsAvailable(int CharsNeeded) {
		if ((CharsNeeded > 0) && (this->MaxNumChars > 0)) {
			int NumChars = 0;
			if (this->hText) {
				NumChars = _GetNumChars();
			}
			if ((CharsNeeded + NumChars) > this->MaxNumChars) {
				return 0;
			}
		}
		return 1;
	}
	void _DeleteChar() {
		if (this->hText) {
			int CursorOffset;
			char *s;
			s = (char *)(this->hText);
			CursorOffset = this->CursorPosByte;
			if (CursorOffset < GUI__strlen(s)) {
				char *pCursor, *pLine, *pEndLine;
				int CursorLine, NumChars, NumBytes;
				pCursor = s + CursorOffset;
				CursorLine = this->CursorLine;
				pLine = _GetpLine(CursorLine);
				NumChars = _WrapGetNumCharsDisp(pLine);
				pEndLine = pLine + GUI_UC__NumChars2NumBytes(pLine, NumChars);
				pLine = pLine + _WrapGetNumBytesToNextLine(pLine);
				if (pCursor == pEndLine) {
					NumBytes = (int)(pLine - pEndLine);
				}
				else {
					NumBytes = GUI_UC_GetCharSize(pCursor);
				}
				NumChars = GUI_UC__NumBytes2NumChars(pCursor, NumBytes);
				GUI__strcpy(pCursor, pCursor + NumBytes);
				WM_NotifyParent(this, WM_NOTIFICATION_VALUE_CHANGED);
				this->NumChars -= NumChars;
				_InvalidateNumLines();
				_InvalidateTextSizeX();
				_InvalidateCursorXY(); /* Invalidate X/Y position */
				_ClearCache();
				this->CursorLine = _GetCursorLine(s, this->CursorPosChar);
			}
		}
	}
	int _InsertChar(uint16_t Char) {
		if (_IsCharsAvailable(1)) {
			int BytesNeeded;
			BytesNeeded = GUI_UC__CalcSizeOfChar(Char);
			if (_IsSpaceInBuffer(BytesNeeded)) {
				int CursorOffset;
				char *pText;
				pText = (char *)(this->hText);
				CursorOffset = this->CursorPosByte;
				pText += CursorOffset;
				GUI__memmove(pText + BytesNeeded, pText, GUI__strlen(pText) + 1);
				GUI_UC_Encode(pText, Char);
				WM_NotifyParent(this, WM_NOTIFICATION_VALUE_CHANGED);
				this->NumChars += 1;
				_InvalidateNumLines();
				_InvalidateTextSizeX();
				_ClearCache();
				return 1;
			}
		}
		return 0;
	}
	void _OnPaint() {
		int ScrollPosX, ScrollPosY, EffectSize, HBorder;
		int x, y, xOff, yOff, ColorIndex, FontSizeY;
		RECT r, rClip;
		const RECT *prOldClip;
		/* Init some values */
		GUI.SetFont(Props.pFont);
		FontSizeY = Props.pFont->DistY();
		ScrollPosX = this->ScrollStateH.v;
		ScrollPosY = this->ScrollStateV.v;
		EffectSize = this->EffectSize();
		HBorder = Props.HBorder;
		xOff = EffectSize + HBorder - ScrollPosX;
		yOff = EffectSize - ScrollPosY * FontSizeY;
		ColorIndex = ((this->Flags & MULTEDIT_CF_READONLY) ? 1 : 0);
		/* Set colors and draw the background */
		GUI.SetBkColor(Props.aBkColor[ColorIndex]);
		GUI.SetColor(Props.aColor[ColorIndex]);
		GUI_Clear();
		/* Draw the text if necessary */
		rClip.x0 = EffectSize + HBorder;
		rClip.y0 = EffectSize;
		rClip.x1 = GetSizeX() - EffectSize - HBorder - 1;
		rClip.y1 = GetSizeY() - EffectSize - 1;
		prOldClip = WM_SetUserClipRect(&rClip);
		if (this->hText) {
			const char *pText;
			int Line = 0;
			int xSize = _GetXSize();
			int NumVisLines = _GetNumVisLines();
			/* Get the text */
			pText = (const char *)(this->hText);
			/* Set the rectangle for drawing */
			r.x0 = xOff;
			r.y0 = EffectSize;
			r.x1 = xSize + EffectSize + HBorder - 1;
			r.y1 = this->Rect.y1 - this->Rect.y0 + 1;
			/* Use cached position of first visible byte if possible */
			if (ScrollPosY >= this->CacheFirstVisibleLine) {
				if (this->CacheFirstVisibleByte) {
					pText += this->CacheFirstVisibleByte;
					Line = this->CacheFirstVisibleLine;
				}
			}
			/* Do the drawing of the text */
			do {
				/* Cache the position of the first visible byte and the depending line number */
				if (this->CacheFirstVisibleLine != ScrollPosY) {
					if (Line == ScrollPosY) {
						this->CacheFirstVisibleByte = (uint16_t)(pText - (const char *)(this->hText));
						this->CacheFirstVisibleLine = ScrollPosY;
					}
				}
				/* Draw it */
				if ((Line >= ScrollPosY) && ((Line - ScrollPosY) <= NumVisLines)) {
					_DispString(pText, &r);
					r.y0 += FontSizeY;  /* Next line */
				}
				pText += _WrapGetNumBytesToNextLine(pText);
				Line++;
			} while (GUI_UC_GetCharCode(pText) && ((Line - ScrollPosY) <= NumVisLines));
		}
		/* Draw cursor if necessary */
		if (WM_HasFocus(this)) {
			_GetCursorXY(&x, &y);
			r.x0 = x + xOff;
			r.y0 = y + yOff;
			r.x1 = r.x0 + _GetCursorSizeX() - 1;
			r.y1 = r.y0 + FontSizeY - 1;
			GUI_DrawRect(r);
		}
		WM_SetUserClipRect(prOldClip);
		/* Draw the 3D effect (if configured) */
		DrawDown();
	}
	void _OnTouch(const PID_STATE *pState) {
		int Notification;
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (pState->Pressed) {
				int Effect, xPos, yPos;
				Effect = this->EffectSize();
				xPos = pState->x + this->ScrollStateH.v - Effect - Props.HBorder;
				yPos = pState->y + this->ScrollStateV.v * Props.pFont->DistY() - Effect;
				_SetCursorXY(xPos, yPos);
				_Invalidate();
				Notification = WM_NOTIFICATION_CLICKED;
			}
			else
				Notification = WM_NOTIFICATION_RELEASED;
		}
		else
			Notification = WM_NOTIFICATION_MOVED_OUT;
		WM_NotifyParent(this, Notification);
	}
	int _AddKey(uint16_t Key) {
		int r = 0;               /* Key has not been consumed */
		switch (Key) {
			case GUI_KEY_UP:
				_MoveCursorUp();
				r = 1;               /* Key has been consumed */
				break;
			case GUI_KEY_DOWN:
				_MoveCursorDown();
				r = 1;               /* Key has been consumed */
				break;
			case GUI_KEY_RIGHT:
				_SetCursorPos(CursorPosChar + 1);
				r = 1;               /* Key has been consumed */
				break;
			case GUI_KEY_LEFT:
				_SetCursorPos(CursorPosChar - 1);
				r = 1;               /* Key has been consumed */
				break;
			case GUI_KEY_END:
				_MoveCursor2LineEnd();
				r = 1;               /* Key has been consumed */
				break;
			case GUI_KEY_HOME:
				_MoveCursor2LinePos1();
				r = 1;               /* Key has been consumed */
				break;
			case GUI_KEY_BACKSPACE:
				if (!(this->Flags & MULTEDIT_CF_READONLY)) {
					if (this->CursorPosChar > this->NumCharsPrompt) {
						_SetCursorPos(CursorPosChar - 1);
						_DeleteChar();
					}
					r = 1;               /* Key has been consumed */
				}
				break;
			case GUI_KEY_DELETE:
				if (!(this->Flags & MULTEDIT_CF_READONLY)) {
					_DeleteChar();
					r = 1;               /* Key has been consumed */
				}
				break;
			case GUI_KEY_INSERT:
				if (!(this->Flags & MULTEDIT_CF_INSERT)) {
					this->Flags |= MULTEDIT_CF_INSERT;
				}
				else {
					this->Flags &= ~MULTEDIT_CF_INSERT;
				}
				r = 1;               /* Key has been consumed */
				break;
			case GUI_KEY_ENTER:
				if (this->Flags & MULTEDIT_CF_READONLY) {
					_MoveCursor2NextLine();
				}
				else {
					if (_InsertChar((uint8_t)('\n'))) {
						if (this->Flags & MULTEDIT_CF_PASSWORD) {
							_SetCursorPos(CursorPosChar + 1);
						}
						else {
							_MoveCursor2NextLine();
						}
					}
				}
				r = 1;               /* Key has been consumed */
				break;
			case GUI_KEY_ESCAPE:
				break;
			default:
				if (!(this->Flags & MULTEDIT_CF_READONLY) && (Key >= 0x20)) {
					if (_IsOverwriteAtThisChar()) {
						_DeleteChar();
					}
					if (_InsertChar(Key)) {
						_SetCursorPos(CursorPosChar + 1);
					}
					r = 1;               /* Key has been consumed */
				}
		}
		_InvalidateTextArea();
		return r;
	}
	int _OnKey(const WM_KEY_INFO *pInfo) {
		if (pInfo->PressedCnt > 0) {
			int Key = pInfo->Key;
			if (_AddKey(Key))
				return 1;
		}
		else if (!(this->Flags & MULTEDIT_CF_READONLY))
			return 1; /* Key release is consumed (not sent to parent) */
		return 0; /* Key release is not consumed (sent to parent) */
	}

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (MultEdit *)hWin;
		/* Let widget handle the standard messages */
		if (!WIDGET_HandleActive(pObj, MsgId, &Data))
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
							WM_Invalidate(pObj);
							WM_NotifyParent(pObj, WM_NOTIFICATION_SCROLL_CHANGED);
						}
						else if (pWinSrc == pObj->GetScrollbarH()) {
							WM_GetScrollState(pWinSrc, &ScrollState);
							pObj->ScrollStateH.v = ScrollState.v;
							WM_Invalidate(pObj);
							WM_NotifyParent(pObj, WM_NOTIFICATION_SCROLL_CHANGED);
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
				GUI_ALLOC_FreePtr(&pObj->hText);
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const WM_KEY_INFO *)Data))
					return 0;
				break;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:

	static MultEdit *Create(int x0, int y0, int xsize, int ysize,
							WObj *hParent, int WinFlags, int ExFlags,
							int Id, int BufferSize, const char *pText) {
		/* Create the window */
		if (!(xsize | ysize | x0 | y0)) {
			RECT Rect = WM_GetClientRect(hParent);
			xsize = Rect.x1 - Rect.x0 + 1;
			ysize = Rect.y1 - Rect.y0 + 1;
		}
		auto pObj = (MultEdit *)WM_CreateWindowAsChild(
			x0, y0, xsize, ysize, hParent, WinFlags, MultEdit::_Callback,
			sizeof(MultEdit) - sizeof(WObj));
		if (!pObj) {
			GUI_DEBUG_ERROROUT_IF(pObj == 0, "MultEdit create failed");
			return nullptr;
		}
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE);
		/* init member variables */
		pObj->Props = MultEdit::DefaultProps;
		pObj->Flags = ExFlags;
		pObj->CursorPosChar = 0;
		pObj->CursorPosByte = 0;
		pObj->MaxNumChars = 0;
		pObj->NumCharsPrompt = 0;
		pObj->BufferSize = 0;
		pObj->hText = 0;
		if (BufferSize > 0) {
			WM_HMEM hText;
			if ((hText = (WM_HMEM)GUI_ALLOC_AllocZero(BufferSize)) != 0) {
				pObj->BufferSize = BufferSize;
				pObj->hText = hText;
			}
			else {
				WM_DeleteWindow(pObj);
				pObj = 0;
			}
		}
		pObj->SetText(pText);
		pObj->_ManageScrollers();
		return pObj;
	}
	static WIDGET *CreateIndirect(const WIDGET_CREATE_INFO *pCreateInfo, WObj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return Create(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
					  hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id, pCreateInfo->Para, nullptr);
	}

public:

#pragma region Properties

	void SetFont(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		_InvalidateTextArea();
		_InvalidateCursorXY();
		_InvalidateNumLines();
		_InvalidateTextSizeX();
	}

	void SetBkColor(MULTEDIT_CI Index, RGBC color) {
		if (Index >= GUI_COUNTOF(Props.aBkColor))
			return;
		if (Props.aBkColor[Index] == color)
			return;
		Props.aBkColor[Index] = color;
		_InvalidateTextArea();
	}

	void SetTextColor(MULTEDIT_CI Index, RGBC color) {
		if (Index >= GUI_COUNTOF(Props.aColor))
			return;
		if (Props.aColor[Index] == color)
			return;
		Props.aColor[Index] = color;
		WM_Invalidate(this);
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

		if (this->hText) {
			pText = (char *)(this->hText);
			pText += GUI_UC__NumChars2NumBytes(pText, this->NumCharsPrompt);
			NumCharsOld = GUI__GetNumChars(pText);
			NumBytesOld = GUI_UC__NumChars2NumBytes(pText, NumCharsOld);
		}
		if (pNew) {
			NumCharsNew = GUI__GetNumChars(pNew);
			NumBytesNew = GUI_UC__NumChars2NumBytes(pNew, NumCharsNew);
		}
		if (_IsCharsAvailable(NumCharsNew - NumCharsOld)) {
			if (_IsSpaceInBuffer(NumBytesNew - NumBytesOld)) {
				pText = (char *)(this->hText);
				pText += GUI_UC__NumChars2NumBytes(pText, this->NumCharsPrompt);
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
		pText = (char *)(this->hText);
		pText += GUI_UC__NumChars2NumBytes(pText, this->NumCharsPrompt);
		Len = GUI__strlen(pText);
		if (Len > (MaxLen - 1)) {
			Len = MaxLen - 1;
		}
		GUI__memcpy(sDest, pText, Len);
		*(sDest + Len) = 0;
	}

	void GetPrompt(char *sDest, int MaxLen) {
		auto sSource = (char *)(this->hText);
		int Len = GUI_UC__NumChars2NumBytes(sSource, this->NumCharsPrompt);
		if (Len > (MaxLen - 1)) {
			Len = MaxLen - 1;
		}
		GUI__memcpy(sDest, sSource, Len);
		*(sDest + Len) = 0;
	}

	void SetWrapWord() {
		_SetWrapMode(GUI_WRAPMODE_WORD);
	}
	void SetWrapChar() {
		_SetWrapMode(GUI_WRAPMODE_CHAR);
	}
	void SetWrapNone() {
		_SetWrapMode(GUI_WRAPMODE_NONE);
	}

	void SetInsertMode(int OnOff) {
		_SetFlag(OnOff, MULTEDIT_CF_INSERT);
	}
	void SetReadOnly(int OnOff) {
		_SetFlag(OnOff, MULTEDIT_CF_READONLY);
	}
	void SetPasswordMode(int OnOff) {
		_SetFlag(OnOff, MULTEDIT_CF_PASSWORD);
		_InvalidateCursorXY();
		_InvalidateNumLines();
		_InvalidateTextSizeX();
	}

	void SetAutoScrollV(int OnOff) {
		_SetFlag(OnOff, MULTEDIT_CF_AUTOSCROLLBAR_V);
	}
	void SetAutoScrollH(int OnOff) {
		_SetFlag(OnOff, MULTEDIT_CF_AUTOSCROLLBAR_H);
	}

	void SetCursorOffset(int Offset) {
		_SetCursorPos(Offset);
		WM_Invalidate(this);
	}
	void SetPrompt(const char *pPrompt) {
		int NumCharsNew = 0, NumCharsOld = 0;
		int NumBytesNew = 0, NumBytesOld = 0;
		char *pText;
		if (this->hText) {
			pText = (char *)(this->hText);
			NumCharsOld = this->NumCharsPrompt;
			NumBytesOld = GUI_UC__NumChars2NumBytes(pText, NumCharsOld);
		}
		if (pPrompt) {
			NumCharsNew = GUI__GetNumChars(pPrompt);
			NumBytesNew = GUI_UC__NumChars2NumBytes(pPrompt, NumCharsNew);
		}
		if (_IsCharsAvailable(NumCharsNew - NumCharsOld)) {
			if (_IsSpaceInBuffer(NumBytesNew - NumBytesOld)) {
				pText = (char *)(this->hText);
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
			GUI_ALLOC_FreePtr(&this->hText);
			this->hText = pText;
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
		if (this->hText && MaxNumChars) {
			char *pText;
			int Offset;
			pText = (char *)(this->hText);
			Offset = GUI_UC__NumChars2NumBytes(pText, MaxNumChars);
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
		if (this->hText) {
			const char *s;
			s = (const char *)this->hText;
			s += GUI_UC__NumChars2NumBytes(s, this->NumCharsPrompt);
			r = 1 + GUI__strlen(s);
		}

		return r;
	}

};

MultEdit::Properties MultEdit::DefaultProps;

}
