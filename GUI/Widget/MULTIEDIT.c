#include "WM_Intern.h"

#include "MULTIEDIT.h"

/* Define default fonts */
#define MULTIEDIT_FONT_DEFAULT &GUI_Font13_1
/* Define colors */
#define MULTIEDIT_BKCOLOR0_DEFAULT   RGB_WHITE
#define MULTIEDIT_BKCOLOR1_DEFAULT   RGB_GRAYL(0xC0)
#define MULTIEDIT_TEXTCOLOR0_DEFAULT RGB_BLACK
#define MULTIEDIT_TEXTCOLOR1_DEFAULT RGB_BLACK
/* Define character for password mode */
#define MULTIEDIT_PASSWORD_CHAR   '*'
#define NUM_DISP_MODES 2
#define INVALID_NUMCHARS (1 << 0)
#define INVALID_NUMLINES (1 << 1)
#define INVALID_TEXTSIZE (1 << 2)
#define INVALID_CURSORXY (1 << 3)
#define INVALID_LINEPOSB (1 << 4)
typedef struct {
	WIDGET Widget;
	RGB_COLOR aBkColor[NUM_DISP_MODES];
	RGB_COLOR aColor[NUM_DISP_MODES];
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
	uint16_t CursorPosX;          /* Cursor position in X */
	uint16_t CursorPosY;          /* Cursor position in Y */
	uint16_t CacheLinePosByte;    /*  */
	uint16_t CacheLineNumber;     /*  */
	uint16_t CacheFirstVisibleLine;
	uint16_t CacheFirstVisibleByte;
	WM_SCROLL_STATE ScrollStateV;
	WM_SCROLL_STATE ScrollStateH;
	const GUI_FONT  *pFont;
	uint8_t Flags;
	uint8_t InvalidFlags;         /* Flags to save validation status */
	uint8_t EditMode;
	uint8_t HBorder;
	GUI_WRAPMODE WrapMode;
} MULTIEDIT_OBJ;
static RGB_COLOR _aDefaultBkColor[2] = {
  MULTIEDIT_BKCOLOR0_DEFAULT,
  MULTIEDIT_BKCOLOR1_DEFAULT,
};
static RGB_COLOR _aDefaultColor[2] = {
  MULTIEDIT_TEXTCOLOR0_DEFAULT,
  MULTIEDIT_TEXTCOLOR1_DEFAULT,
};
static const GUI_FONT  *_pDefaultFont = MULTIEDIT_FONT_DEFAULT;
#define MULTIEDIT_REALLOC_SIZE  16
/*********************************************************************
*
*       _InvalidateNumChars
*
* Invalidates the number of characters including the prompt
*/
static void _InvalidateNumChars(MULTIEDIT_OBJ *pObj) {
	pObj->InvalidFlags |= INVALID_NUMCHARS;
}
/*********************************************************************
*
*       _GetNumChars
*
* Calculates (if needed) and returns the number of characters including the prompt
*/
static int _GetNumChars(MULTIEDIT_OBJ *pObj) {
	if (pObj->InvalidFlags & INVALID_NUMCHARS) {
		char *pText;
		pText = (char *)(pObj->hText);
		pObj->NumChars = GUI__GetNumChars(pText);
		pObj->InvalidFlags &= ~INVALID_NUMCHARS;
	}
	return pObj->NumChars;
}
/*********************************************************************
*
*       _GetXSize
*
* Returns the x size for displaying text.
*/
static int _GetXSize(MULTIEDIT_OBJ *pObj) {
	GUI_RECT Rect;
	WM_GetInsideRectExScrollbar(pObj, &Rect);
	return Rect.x1 - Rect.x0 - (pObj->HBorder * 2) - 1;
}
static int _GetNumCharsInPrompt(const MULTIEDIT_OBJ *pObj, const char  *pText) {
	char *pString, *pEndPrompt;
	int r = 0;
	pString = (char *)(pObj->hText);
	pEndPrompt = pString + GUI_UC__NumChars2NumBytes(pString, pObj->NumCharsPrompt);
	if (pText < pEndPrompt) {
		r = GUI_UC__NumBytes2NumChars(pText, (int)(pEndPrompt - pText));
	}
	return r;
}
static int _NumChars2XSize(const char  *pText, int NumChars) {
	int xSize = 0;
	uint16_t Char;
	while (NumChars--) {
		Char = GUI_UC__GetCharCodeInc(&pText);
		xSize += GUI_GetCharDistX(Char);
	}
	return xSize;
}
static int _WrapGetNumCharsDisp(MULTIEDIT_OBJ *pObj, const char  *pText) {
	int xSize, r;
	xSize = _GetXSize(pObj);
	if (pObj->Flags & MULTIEDIT_SF_PASSWORD) {
		int NumCharsPrompt;
		NumCharsPrompt = _GetNumCharsInPrompt(pObj, pText);
		r = GUI__WrapGetNumCharsDisp(pText, xSize, pObj->WrapMode);
		if (r >= NumCharsPrompt) {
			int x;
			switch (pObj->WrapMode) {
				case GUI_WRAPMODE_NONE:
					r = GUI__GetNumChars(pText);
					break;
				default:
					r = NumCharsPrompt;
					x = _NumChars2XSize(pText, NumCharsPrompt);
					pText += GUI_UC__NumChars2NumBytes(pText, NumCharsPrompt);
					while (GUI_UC__GetCharCodeInc(&pText) != 0) {
						x += GUI_GetCharDistX(MULTIEDIT_PASSWORD_CHAR);
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
		r = GUI__WrapGetNumCharsDisp(pText, xSize, pObj->WrapMode);
	}
	return r;
}
static int _WrapGetNumBytesToNextLine(MULTIEDIT_OBJ *pObj, const char *pText) {
	int xSize, r;
	xSize = _GetXSize(pObj);
	if (pObj->Flags & MULTIEDIT_SF_PASSWORD) {
		int NumChars, NumCharsPrompt;
		NumCharsPrompt = _GetNumCharsInPrompt(pObj, pText);
		NumChars = _WrapGetNumCharsDisp(pObj, pText);
		r = GUI_UC__NumChars2NumBytes(pText, NumChars);
		if (NumChars < NumCharsPrompt) {
			if (*(pText + r) == '\n') {
				r++;
			}
		}
	}
	else {
		r = GUI__WrapGetNumBytesToNextLine(pText, xSize, pObj->WrapMode);
	}
	return r;
}
static int _GetCharDistX(const MULTIEDIT_OBJ *pObj, const char *pText) {
	int r;
	if ((pObj->Flags & MULTIEDIT_SF_PASSWORD) && (_GetNumCharsInPrompt(pObj, pText) == 0)) {
		r = GUI_GetCharDistX(MULTIEDIT_PASSWORD_CHAR);
	}
	else {
		uint16_t c;
		c = GUI_UC_GetCharCode(pText);
		r = GUI_GetCharDistX(c);
	}
	return r;
}
static void _DispString(MULTIEDIT_OBJ *pObj, const char *pText, GUI_RECT *pRect) {
	int NumCharsDisp;
	NumCharsDisp = _WrapGetNumCharsDisp(pObj, pText);
	if (pObj->Flags & MULTIEDIT_SF_PASSWORD) {
		int x, NumCharsPrompt, NumCharsLeft = 0;
		NumCharsPrompt = _GetNumCharsInPrompt(pObj, pText);
		if (NumCharsDisp < NumCharsPrompt) {
			NumCharsPrompt = NumCharsDisp;
		}
		else {
			NumCharsLeft = NumCharsDisp - NumCharsPrompt;
		}
		GUI_DispStringInRectMax(pText, pRect, GUI_TA_LEFT, NumCharsPrompt);
		x = pRect->x0 + _NumChars2XSize(pText, NumCharsPrompt);
		if (NumCharsLeft) {
			GUI_DispCharAt(MULTIEDIT_PASSWORD_CHAR, x, pRect->y0);
			GUI_DispChars(MULTIEDIT_PASSWORD_CHAR, NumCharsLeft - 1);
		}
	}
	else {
		GUI_DispStringInRectMax(pText, pRect, GUI_TA_LEFT, NumCharsDisp);
	}
}
/*********************************************************************
*
*       _GetpLine
*
* Returns a pointer to the beginning of the line with the
* given line number.
*/
static char *_GetpLine(MULTIEDIT_OBJ *pObj, unsigned LineNumber) {
	char *pText, *pLine;
	pText = (char *)(pObj->hText);
	if ((unsigned)pObj->CacheLineNumber != LineNumber) {
		if (LineNumber > (unsigned)pObj->CacheLineNumber) {
			/* If new line number > cache we can start with old pointer */
			int OldNumber = pObj->CacheLineNumber;
			pLine = pText + pObj->CacheLinePosByte;
			pObj->CacheLineNumber = LineNumber;
			LineNumber -= OldNumber;
		}
		else {
			/* If new line number < cache we need to start with first byte */
			pLine = pText;
			pObj->CacheLineNumber = LineNumber;
		}
		while (LineNumber--) {
			pLine += _WrapGetNumBytesToNextLine(pObj, pLine);
		}
		pObj->CacheLinePosByte = (uint16_t)(pLine - pText);
	}
	return pText + pObj->CacheLinePosByte;
}
/*********************************************************************
*
*       _ClearCache
*
* Clears the cached position of the linenumber and the first byte
* of the line which holds the cursor.
*/
static void _ClearCache(MULTIEDIT_OBJ *pObj) {
	pObj->CacheLineNumber = 0;
	pObj->CacheLinePosByte = 0;
	pObj->CacheFirstVisibleByte = 0;
	pObj->CacheFirstVisibleLine = 0;
}
/*********************************************************************
*
*       _GetCursorLine
*
* Returns the line number of the cursor position.
*/
static int _GetCursorLine(MULTIEDIT_OBJ *pObj, const char *pText, int CursorPosChar) {
	const char *pCursor;
	const char *pEndLine;
	int NumChars, ByteOffsetNewCursor, LineNumber = 0;
	ByteOffsetNewCursor = GUI_UC__NumChars2NumBytes(pText, CursorPosChar);
	pCursor = pText + ByteOffsetNewCursor;
	if (pObj->CacheLinePosByte < ByteOffsetNewCursor) {
		/* If cache pos < new position we can use it as start position */
		pText += pObj->CacheLinePosByte;
		LineNumber += pObj->CacheLineNumber;
	}
	while (*pText && (pCursor > pText)) {
		NumChars = _WrapGetNumCharsDisp(pObj, pText);
		pEndLine = pText + GUI_UC__NumChars2NumBytes(pText, NumChars);
		pText += _WrapGetNumBytesToNextLine(pObj, pText);
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
static void _GetCursorXY(MULTIEDIT_OBJ *pObj, int *px, int *py) {
	if (pObj->InvalidFlags & INVALID_CURSORXY) {
		int CursorLine = 0, x = 0;
		GUI_SetFont(pObj->pFont);
		if (pObj->hText) {
			const char *pLine;
			const char *pCursor;
			pLine = (const char *)(pObj->hText);
			pCursor = pLine + pObj->CursorPosByte;
			CursorLine = pObj->CursorLine;
			pLine = _GetpLine(pObj, CursorLine);
			while (pLine < pCursor) {
				x += _GetCharDistX(pObj, pLine);
				pLine += GUI_UC_GetCharSize(pLine);
			}
		}
		pObj->CursorPosX = x;
		pObj->CursorPosY = CursorLine * GUI_GetFontDistY();
		pObj->InvalidFlags &= ~INVALID_CURSORXY;
	}
	*px = pObj->CursorPosX;
	*py = pObj->CursorPosY;
}
static void _InvalidateCursorXY(MULTIEDIT_OBJ *pObj) {
	pObj->InvalidFlags |= INVALID_CURSORXY;
}
static void _SetScrollState(WM_HWIN hObj) {
	MULTIEDIT_OBJ *pObj = (hObj);
	WIDGET__SetScrollState(hObj, &pObj->ScrollStateV, &pObj->ScrollStateH);
}
/*********************************************************************
*
*       _CalcScrollPos
*
* Purpose:
*   Find out if the current position of the cursor is still in the
*   visible area. If it is not, the scroll position is updated.
*   Needs to be called every time the cursor is move, wrap, font or
*   window size are changed.
*/
static void _CalcScrollPos(MULTIEDIT_OBJ *pObj) {
	int xCursor, yCursor;
	_GetCursorXY(pObj, &xCursor, &yCursor);
	yCursor /= GUI_GetYDistOfFont(pObj->pFont);
	WM_CheckScrollPos(&pObj->ScrollStateV, yCursor, 0, 0);       /* Vertical */
	WM_CheckScrollPos(&pObj->ScrollStateH, xCursor, 30, 30);     /* Horizontal */
	_SetScrollState(pObj);
}
/*********************************************************************
*
*       _GetTextSizeX
*
* Returns the width of the displayed text.
*/
static int _GetTextSizeX(MULTIEDIT_OBJ *pObj) {
	if (pObj->InvalidFlags & INVALID_TEXTSIZE) {
		pObj->TextSizeX = 0;
		if (pObj->hText) {
			int NumChars, xSizeLine;
			char *pText, *pLine;
			GUI_SetFont(pObj->pFont);
			pText = (char *)(pObj->hText);
			do {
				NumChars = _WrapGetNumCharsDisp(pObj, pText);
				xSizeLine = 0;
				pLine = pText;
				while (NumChars--) {
					xSizeLine += _GetCharDistX(pObj, pLine);
					pLine += GUI_UC_GetCharSize(pLine);
				}
				if (xSizeLine > pObj->TextSizeX) {
					pObj->TextSizeX = xSizeLine;
				}
				pText += _WrapGetNumBytesToNextLine(pObj, pText);
			} while (*pText);
		}
		pObj->InvalidFlags &= ~INVALID_TEXTSIZE;
	}
	return pObj->TextSizeX;
}
static int _GetNumVisLines(MULTIEDIT_OBJ *pObj) {
	GUI_RECT Rect;
	WM_GetInsideRectExScrollbar(pObj, &Rect);
	return (Rect.y1 - Rect.y0 + 1) / GUI_GetYDistOfFont(pObj->pFont);
}
/*********************************************************************
*
*       _GetNumLines
*
* Calculates (if needed) and returns the number of lines
*/
static int _GetNumLines(MULTIEDIT_OBJ *pObj) {
	if (pObj->InvalidFlags & INVALID_NUMLINES) {
		int NumLines = 0;
		if (pObj->hText) {
			int NumChars, NumBytes;
			char *pText;
			uint16_t Char;
			pText = (char *)(pObj->hText);
			GUI_SetFont(pObj->pFont);
			do {
				NumChars = _WrapGetNumCharsDisp(pObj, pText);
				NumBytes = GUI_UC__NumChars2NumBytes(pText, NumChars);
				Char = GUI_UC_GetCharCode(pText + NumBytes);
				if (Char) {
					NumLines++;
				}
				pText += _WrapGetNumBytesToNextLine(pObj, pText);
			} while (Char);
		}
		pObj->NumLines = NumLines + 1;
		pObj->InvalidFlags &= ~INVALID_NUMLINES;
	}
	return pObj->NumLines;
}
/*********************************************************************
*
*       _InvalidateNumLines
*
* Invalidates the number of lines
*/
static void _InvalidateNumLines(MULTIEDIT_OBJ *pObj) {
	pObj->InvalidFlags |= INVALID_NUMLINES;
}
/*********************************************************************
*
*       _InvalidateTextSizeX
*
* Calculates the TextSizeX
*/
static void _InvalidateTextSizeX(MULTIEDIT_OBJ *pObj) {
	pObj->InvalidFlags |= INVALID_TEXTSIZE;
}
/*********************************************************************
*
*       _CalcScrollParas
*
* Purpose:
*   Calculate page size ,number of items & position
*/
static void _CalcScrollParas(MULTIEDIT_HANDLE hObj) {
	MULTIEDIT_OBJ *pObj = (hObj);
	/* Calc vertical scroll parameters */
	pObj->ScrollStateV.NumItems = _GetNumLines(pObj);
	pObj->ScrollStateV.PageSize = _GetNumVisLines(pObj);
	/* Calc horizontal scroll parameters */
	pObj->ScrollStateH.NumItems = _GetTextSizeX(pObj);
	pObj->ScrollStateH.PageSize = _GetXSize(pObj);
	_CalcScrollPos(pObj);
}
static void _ManageAutoScrollV(MULTIEDIT_OBJ *pObj) {
	if (pObj->Flags & MULTIEDIT_SF_AUTOSCROLLBAR_V) {
		char IsRequired = _GetNumVisLines(pObj) < _GetNumLines(pObj);
		if (WM_SetScrollbarV(pObj, IsRequired) != IsRequired) {
			_InvalidateNumLines(pObj);
			_InvalidateTextSizeX(pObj);
			_InvalidateCursorXY(pObj);
			_ClearCache(pObj);
		}
	}
}
/*********************************************************************
*
*       _ManageScrollers
*
* Function:
* If autoscroll mode is enabled, add or remove the horizonatal and
* vertical scrollbars as required.
* Caution: This routine should not be called as reaction to a message
* From the child, as this could lead to a recursion problem
*/
static void _ManageScrollers(MULTIEDIT_HANDLE hObj) {
	MULTIEDIT_OBJ *pObj;
	pObj = (hObj);
	/* 1. Step: Check if vertical scrollbar is required */
	_ManageAutoScrollV(pObj);
	/* 2. Step: Check if horizontal scrollbar is required */
	if (pObj->Flags & MULTIEDIT_SF_AUTOSCROLLBAR_H) {
		char IsRequired;
		IsRequired = (_GetXSize(pObj) < _GetTextSizeX(pObj));
		if (WM_SetScrollbarH(hObj, IsRequired) != IsRequired) {
			/* 3. Step: Check vertical scrollbar again if horizontal has changed */
			_ManageAutoScrollV(pObj);
		}
	}
	_CalcScrollParas(hObj);
}
static void _Invalidate(MULTIEDIT_HANDLE hObj) {
	_ManageScrollers(hObj);
	WM_Invalidate(hObj);
}
/*********************************************************************
*
*       _InvalidateTextArea
*
* Invalidates the text area only
*/
static void _InvalidateTextArea(MULTIEDIT_HANDLE hObj) {
	GUI_RECT rInsideRect;
	_ManageScrollers(hObj);
	WM_GetInsideRectExScrollbar(hObj, &rInsideRect);
	WM_InvalidateRect(hObj, &rInsideRect);
}
/*********************************************************************
*
*       _InvalidateCursorPos
*
* Sets the position of the cursor to an invalid value
*/
static int _InvalidateCursorPos(MULTIEDIT_OBJ *pObj) {
	int Value;
	Value = pObj->CursorPosChar;
	pObj->CursorPosChar = 0xffff;
	return Value;
}
static void _SetFlag(MULTIEDIT_HANDLE hObj, int OnOff, uint8_t Flag) {
	if (hObj) {
		MULTIEDIT_OBJ *pObj;

		pObj = (hObj);
		if (OnOff) {
			pObj->Flags |= Flag;
		}
		else {
			pObj->Flags &= ~Flag;
		}
		_InvalidateTextArea(hObj);

	}
}
/*********************************************************************
*
*       _CalcNextValidCursorPos
*
* Purpose:
*   Calculates the next valid cursor position of the desired position.
*
* Parameters:
*   pObj          : Obvious
*   CursorPosChar : New character position of the cursor
*   pCursorPosByte: Pointer to save the cursorposition in bytes. Used to abolish further calculations. Could be 0.
*   pCursorLine   : Pointer to save the line number of the cursor. Used to abolish further calculations. Could be 0.
*/
static int _CalcNextValidCursorPos(MULTIEDIT_OBJ *pObj, int CursorPosChar, int *pCursorPosByte, int *pCursorLine) {
	if (pObj->hText) {
		char *pNextLine, *pCursor, *pText;
		int CursorLine, NumChars, CursorPosByte;
		pText = (char *)(pObj->hText);
		NumChars = _GetNumChars(pObj);
		/* Set offset in valid range */
		if (CursorPosChar < pObj->NumCharsPrompt) {
			CursorPosChar = pObj->NumCharsPrompt;
		}
		if (CursorPosChar > NumChars) {
			CursorPosChar = NumChars;
		}
		CursorPosByte = GUI_UC__NumChars2NumBytes(pText, CursorPosChar);
		CursorLine = _GetCursorLine(pObj, pText, CursorPosChar);
		pCursor = pText + CursorPosByte;
		pNextLine = _GetpLine(pObj, CursorLine);
		if (pNextLine > pCursor) {
			if (pObj->CursorPosChar < CursorPosChar) {
				pCursor = pNextLine;
			}
			else {
				char *pPrevLine;
				int NumChars;
				pPrevLine = _GetpLine(pObj, CursorLine - 1);
				NumChars = _WrapGetNumCharsDisp(pObj, pPrevLine);
				pPrevLine += GUI_UC__NumChars2NumBytes(pPrevLine, NumChars);
				pCursor = pPrevLine;
			}
			CursorPosChar = GUI_UC__NumBytes2NumChars(pText, (int)(pCursor - pText));
			CursorPosByte = GUI_UC__NumChars2NumBytes(pText, CursorPosChar);
			CursorLine = _GetCursorLine(pObj, pText, CursorPosChar);
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
/*********************************************************************
*
*       _SetCursorPos
*
* Sets a new cursor position.
*/
static void _SetCursorPos(MULTIEDIT_OBJ *pObj, int CursorPosChar) {
	int CursorPosByte, CursorLine;
	CursorPosChar = _CalcNextValidCursorPos(pObj, CursorPosChar, &CursorPosByte, &CursorLine);
	/* Assign value and recalc whatever necessary */
	if (pObj->CursorPosChar != CursorPosChar) {
		/* Save values */
		pObj->CursorPosByte = CursorPosByte;
		pObj->CursorPosChar = CursorPosChar;
		pObj->CursorLine = CursorLine;
		_InvalidateCursorXY(pObj); /* Invalidate X/Y position */
		_CalcScrollPos(pObj);
	}
}
static int _SetWrapMode(MULTIEDIT_HANDLE hObj, GUI_WRAPMODE WrapMode) {
	int r;
	r = 0;
	if (hObj) {
		MULTIEDIT_OBJ *pObj;

		pObj = (hObj);
		r = pObj->WrapMode;
		if (pObj->WrapMode != WrapMode) {
			int Position;
			pObj->WrapMode = WrapMode;
			_ClearCache(pObj);
			_InvalidateNumLines(pObj);
			_InvalidateTextSizeX(pObj);
			_InvalidateTextArea(hObj);
			Position = _InvalidateCursorPos(pObj);
			_SetCursorPos(pObj, Position);
		}

	}
	return r;
}
/*********************************************************************
*
*       _SetCursorXY
*
* Sets the cursor position from window coordinates.
*/
static void _SetCursorXY(MULTIEDIT_OBJ *pObj, int x, int y) {
	int CursorPosChar = 0;
	if ((x < 0) || (y < 0)) {
		return;
	}
	if (pObj->hText) {
		char *pLine, *pText;
		int CursorLine, WrapChars;
		int SizeX = 0;
		uint16_t Char;
		GUI_SetFont(pObj->pFont);
		CursorLine = y / GUI_GetFontDistY();
		pLine = _GetpLine(pObj, CursorLine);
		pText = (char *)(pObj->hText);
		WrapChars = _WrapGetNumCharsDisp(pObj, pLine);
		Char = GUI_UC__GetCharCode(pLine + GUI_UC__NumChars2NumBytes(pLine, WrapChars));
		if (pObj->Flags & MULTIEDIT_SF_PASSWORD) {
			if (!Char) {
				WrapChars++;
			}
		}
		else {
			if (!Char || (Char == '\n') || ((Char == ' ') && (pObj->WrapMode == GUI_WRAPMODE_WORD))) {
				WrapChars++;
			}
		}
		while (--WrapChars > 0) {
			Char = GUI_UC_GetCharCode(pLine);
			SizeX += _GetCharDistX(pObj, pLine);
			if (!Char || (SizeX > x)) {
				break;
			}
			pLine += GUI_UC_GetCharSize(pLine);
		}
		CursorPosChar = GUI_UC__NumBytes2NumChars(pText, (int)(pLine - pText));
	}
	_SetCursorPos(pObj, CursorPosChar);
}
static void _MoveCursorUp(MULTIEDIT_OBJ *pObj) {
	int xPos, yPos;
	_GetCursorXY(pObj, &xPos, &yPos);
	yPos -= GUI_GetYDistOfFont(pObj->pFont);
	_SetCursorXY(pObj, xPos, yPos);
}
static void _MoveCursorDown(MULTIEDIT_OBJ *pObj) {
	int xPos, yPos;
	_GetCursorXY(pObj, &xPos, &yPos);
	yPos += GUI_GetYDistOfFont(pObj->pFont);
	_SetCursorXY(pObj, xPos, yPos);
}
static void _MoveCursor2NextLine(MULTIEDIT_OBJ *pObj) {
	int xPos, yPos;
	_GetCursorXY(pObj, &xPos, &yPos);
	yPos += GUI_GetYDistOfFont(pObj->pFont);
	_SetCursorXY(pObj, 0, yPos);
}
static void _MoveCursor2LineEnd(MULTIEDIT_OBJ *pObj) {
	int xPos, yPos;
	_GetCursorXY(pObj, &xPos, &yPos);
	_SetCursorXY(pObj, 0x7FFF, yPos);
}
static void _MoveCursor2LinePos1(MULTIEDIT_OBJ *pObj) {
	int xPos, yPos;
	_GetCursorXY(pObj, &xPos, &yPos);
	_SetCursorXY(pObj, 0, yPos);
}
static int _IsOverwriteAtThisChar(MULTIEDIT_OBJ *pObj) {
	int r = 0;
	if (pObj->hText && !(pObj->Flags & MULTIEDIT_CF_INSERT)) {
		const char *pText;
		int CurPos, Line1, Line2;
		uint16_t Char;
		pText = (const char *)(pObj->hText);
		Line1 = pObj->CursorLine;
		CurPos = _CalcNextValidCursorPos(pObj, pObj->CursorPosChar + 1, 0, 0);
		Line2 = _GetCursorLine(pObj, pText, CurPos);
		pText += pObj->CursorPosByte;
		Char = GUI_UC_GetCharCode(pText);
		if (Char) {
			if ((Line1 == Line2) || (pObj->Flags & MULTIEDIT_SF_PASSWORD)) {
				r = 1;
			}
			else {
				if (Char != '\n') {
					if ((Char != ' ') || (pObj->WrapMode == GUI_WRAPMODE_CHAR)) {
						r = 1;
					}
				}
			}
		}
	}
	return r;
}
/*********************************************************************
*
*       _GetCursorSizeX
*
* Returns the width of the cursor to be draw according to the
* insert mode flag and the cursor position.
*/
static int _GetCursorSizeX(MULTIEDIT_OBJ *pObj) {
	if (_IsOverwriteAtThisChar(pObj)) {
		const char *pText;
		pText = (const char *)(pObj->hText);
		pText += pObj->CursorPosByte;
		return _GetCharDistX(pObj, pText);
	}
	else {
		return 2;
	}
}
/*********************************************************************
*
*       _IncrementBuffer
*
* Increments the buffer size by AddBytes.
*/
static int _IncrementBuffer(MULTIEDIT_OBJ *pObj, unsigned AddBytes) {
	WM_HMEM hNew;
	int NewSize;
	NewSize = pObj->BufferSize + AddBytes;
	hNew = GUI_ALLOC_Realloc(pObj->hText, NewSize);
	if (hNew) {
		if (!(pObj->hText)) {
			char *pText;
			pText = (char *)(hNew);
			*pText = 0;
		}
		pObj->BufferSize = NewSize;
		pObj->hText = hNew;
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
static int _IsSpaceInBuffer(MULTIEDIT_OBJ *pObj, int BytesNeeded) {
	int NumBytes = 0;
	if (pObj->hText) {
		NumBytes = GUI__strlen((char *)(pObj->hText));
	}
	BytesNeeded = (BytesNeeded + NumBytes + 1) - pObj->BufferSize;
	if (BytesNeeded > 0) {
		if (!_IncrementBuffer(pObj, BytesNeeded + MULTIEDIT_REALLOC_SIZE)) {
			return 0;
		}
	}
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
static int _IsCharsAvailable(MULTIEDIT_OBJ *pObj, int CharsNeeded) {
	if ((CharsNeeded > 0) && (pObj->MaxNumChars > 0)) {
		int NumChars = 0;
		if (pObj->hText) {
			NumChars = _GetNumChars(pObj);
		}
		if ((CharsNeeded + NumChars) > pObj->MaxNumChars) {
			return 0;
		}
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
static void _DeleteChar(MULTIEDIT_OBJ *pObj) {
	if (pObj->hText) {
		int CursorOffset;
		char *s;
		s = (char *)(pObj->hText);
		CursorOffset = pObj->CursorPosByte;
		if (CursorOffset < GUI__strlen(s)) {
			char *pCursor, *pLine, *pEndLine;
			int CursorLine, NumChars, NumBytes;
			pCursor = s + CursorOffset;
			CursorLine = pObj->CursorLine;
			pLine = _GetpLine(pObj, CursorLine);
			NumChars = _WrapGetNumCharsDisp(pObj, pLine);
			pEndLine = pLine + GUI_UC__NumChars2NumBytes(pLine, NumChars);
			pLine = pLine + _WrapGetNumBytesToNextLine(pObj, pLine);
			if (pCursor == pEndLine) {
				NumBytes = (int)(pLine - pEndLine);
			}
			else {
				NumBytes = GUI_UC_GetCharSize(pCursor);
			}
			NumChars = GUI_UC__NumBytes2NumChars(pCursor, NumBytes);
			GUI__strcpy(pCursor, pCursor + NumBytes);
			WM_NotifyParent(pObj, WM_NOTIFICATION_VALUE_CHANGED);
			pObj->NumChars -= NumChars;
			_InvalidateNumLines(pObj);
			_InvalidateTextSizeX(pObj);
			_InvalidateCursorXY(pObj); /* Invalidate X/Y position */
			_ClearCache(pObj);
			pObj->CursorLine = _GetCursorLine(pObj, s, pObj->CursorPosChar);
		}
	}
}
/*********************************************************************
*
*       _InsertChar
*
* Create space at the current cursor position and inserts a character.
*/
static int _InsertChar(MULTIEDIT_OBJ *pObj, uint16_t Char) {
	if (_IsCharsAvailable(pObj, 1)) {
		int BytesNeeded;
		BytesNeeded = GUI_UC__CalcSizeOfChar(Char);
		if (_IsSpaceInBuffer(pObj, BytesNeeded)) {
			int CursorOffset;
			char *pText;
			pText = (char *)(pObj->hText);
			CursorOffset = pObj->CursorPosByte;
			pText += CursorOffset;
			GUI__memmove(pText + BytesNeeded, pText, GUI__strlen(pText) + 1);
			GUI_UC_Encode(pText, Char);
			WM_NotifyParent(pObj, WM_NOTIFICATION_VALUE_CHANGED);
			pObj->NumChars += 1;
			_InvalidateNumLines(pObj);
			_InvalidateTextSizeX(pObj);
			_ClearCache(pObj);
			return 1;
		}
	}
	return 0;
}
static void _MULTIEDIT_Paint(MULTIEDIT_OBJ *pObj) {
	int ScrollPosX, ScrollPosY, EffectSize, HBorder;
	int x, y, xOff, yOff, ColorIndex, FontSizeY;
	GUI_RECT r, rClip;
	const GUI_RECT *prOldClip;
	/* Init some values */
	GUI_SetFont(pObj->pFont);
	FontSizeY = GUI_GetFontDistY();
	ScrollPosX = pObj->ScrollStateH.v;
	ScrollPosY = pObj->ScrollStateV.v;
	EffectSize = pObj->Widget.pEffect->EffectSize;
	HBorder = pObj->HBorder;
	xOff = EffectSize + HBorder - ScrollPosX;
	yOff = EffectSize - ScrollPosY * FontSizeY;
	ColorIndex = ((pObj->Flags & MULTIEDIT_SF_READONLY) ? 1 : 0);
	/* Set colors and draw the background */
	GUI_SetBkColor(pObj->aBkColor[ColorIndex]);
	GUI_SetColor(pObj->aColor[ColorIndex]);
	GUI_Clear();
	/* Draw the text if necessary */
	rClip.x0 = EffectSize + HBorder;
	rClip.y0 = EffectSize;
	rClip.x1 = WM_GetWindowSizeX(pObj) - EffectSize - HBorder - 1;
	rClip.y1 = WM_GetWindowSizeY(pObj) - EffectSize - 1;
	prOldClip = WM_SetUserClipRect(&rClip);
	if (pObj->hText) {
		const char *pText;
		int Line = 0;
		int xSize = _GetXSize(pObj);
		int NumVisLines = _GetNumVisLines(pObj);
		/* Get the text */
		pText = (const char *)(pObj->hText);
		/* Set the rectangle for drawing */
		r.x0 = xOff;
		r.y0 = EffectSize;
		r.x1 = xSize + EffectSize + HBorder - 1;
		r.y1 = pObj->Widget.Win.Rect.y1 - pObj->Widget.Win.Rect.y0 + 1;
		/* Use cached position of first visible byte if possible */
		if (ScrollPosY >= pObj->CacheFirstVisibleLine) {
			if (pObj->CacheFirstVisibleByte) {
				pText += pObj->CacheFirstVisibleByte;
				Line = pObj->CacheFirstVisibleLine;
			}
		}
		/* Do the drawing of the text */
		do {
			/* Cache the position of the first visible byte and the depending line number */
			if (pObj->CacheFirstVisibleLine != ScrollPosY) {
				if (Line == ScrollPosY) {
					pObj->CacheFirstVisibleByte = (uint16_t)(pText - (const char *)(pObj->hText));
					pObj->CacheFirstVisibleLine = ScrollPosY;
				}
			}
			/* Draw it */
			if ((Line >= ScrollPosY) && ((Line - ScrollPosY) <= NumVisLines)) {
				_DispString(pObj, pText, &r);
				r.y0 += FontSizeY;  /* Next line */
			}
			pText += _WrapGetNumBytesToNextLine(pObj, pText);
			Line++;
		} while (GUI_UC_GetCharCode(pText) && ((Line - ScrollPosY) <= NumVisLines));
	}
	/* Draw cursor if necessary */
	if (WM_HasFocus(pObj)) {
		_GetCursorXY(pObj, &x, &y);
		r.x0 = x + xOff;
		r.y0 = y + yOff;
		r.x1 = r.x0 + _GetCursorSizeX(pObj) - 1;
		r.y1 = r.y0 + FontSizeY - 1;
		GUI_DrawRect(r.x0, r.y0, r.x1, r.y1);
	}
	WM_SetUserClipRect(prOldClip);
	/* Draw the 3D effect (if configured) */
	WIDGET__EFFECT_DrawDown(&pObj->Widget);
}
static void _OnTouch(MULTIEDIT_OBJ *pObj, const GUI_PID_STATE *pState) {
	int Notification;
	if (pState) {  /* Something happened in our area (pressed or released) */
		if (pState->Pressed) {
			int Effect, xPos, yPos;
			Effect = pObj->Widget.pEffect->EffectSize;
			xPos = pState->x + pObj->ScrollStateH.v - Effect - pObj->HBorder;
			yPos = pState->y + pObj->ScrollStateV.v * GUI_GetYDistOfFont(pObj->pFont) - Effect;
			_SetCursorXY(pObj, xPos, yPos);
			_Invalidate(pObj);
			Notification = WM_NOTIFICATION_CLICKED;
		}
		else 
			Notification = WM_NOTIFICATION_RELEASED;
	}
	else
		Notification = WM_NOTIFICATION_MOVED_OUT;
	WM_NotifyParent(pObj, Notification);
}
static int _OnKey(MULTIEDIT_OBJ *pObj, const WM_KEY_INFO *pInfo) {
	if (pInfo->PressedCnt > 0) {
		int Key = pInfo->Key;
		if (_AddKey(pObj, Key))
			return 1;
	}
	else if (!(pObj->Flags & MULTIEDIT_SF_READONLY))
		return 1; /* Key release is consumed (not sent to parent) */
	return 0; /* Key release is not consumed (sent to parent) */
}
/*********************************************************************
*
*       _AddKey
*
* Returns: 1 if Key has been consumed
*          0 else
*/
static int _AddKey(MULTIEDIT_HANDLE hObj, uint16_t Key) {
	int r = 0;               /* Key has not been consumed */
	MULTIEDIT_OBJ *pObj;
	pObj = (hObj);
	switch (Key) {
		case GUI_KEY_UP:
			_MoveCursorUp(pObj);
			r = 1;               /* Key has been consumed */
			break;
		case GUI_KEY_DOWN:
			_MoveCursorDown(pObj);
			r = 1;               /* Key has been consumed */
			break;
		case GUI_KEY_RIGHT:
			_SetCursorPos(pObj, pObj->CursorPosChar + 1);
			r = 1;               /* Key has been consumed */
			break;
		case GUI_KEY_LEFT:
			_SetCursorPos(pObj, pObj->CursorPosChar - 1);
			r = 1;               /* Key has been consumed */
			break;
		case GUI_KEY_END:
			_MoveCursor2LineEnd(pObj);
			r = 1;               /* Key has been consumed */
			break;
		case GUI_KEY_HOME:
			_MoveCursor2LinePos1(pObj);
			r = 1;               /* Key has been consumed */
			break;
		case GUI_KEY_BACKSPACE:
			if (!(pObj->Flags & MULTIEDIT_SF_READONLY)) {
				if (pObj->CursorPosChar > pObj->NumCharsPrompt) {
					_SetCursorPos(pObj, pObj->CursorPosChar - 1);
					_DeleteChar(pObj);
				}
				r = 1;               /* Key has been consumed */
			}
			break;
		case GUI_KEY_DELETE:
			if (!(pObj->Flags & MULTIEDIT_SF_READONLY)) {
				_DeleteChar(pObj);
				r = 1;               /* Key has been consumed */
			}
			break;
		case GUI_KEY_INSERT:
			if (!(pObj->Flags & MULTIEDIT_CF_INSERT)) {
				pObj->Flags |= MULTIEDIT_CF_INSERT;
			}
			else {
				pObj->Flags &= ~MULTIEDIT_CF_INSERT;
			}
			r = 1;               /* Key has been consumed */
			break;
		case GUI_KEY_ENTER:
			if (pObj->Flags & MULTIEDIT_SF_READONLY) {
				_MoveCursor2NextLine(pObj);
			}
			else {
				if (_InsertChar(pObj, (uint8_t)('\n'))) {
					if (pObj->Flags & MULTIEDIT_SF_PASSWORD) {
						_SetCursorPos(pObj, pObj->CursorPosChar + 1);
					}
					else {
						_MoveCursor2NextLine(pObj);
					}
				}
			}
			r = 1;               /* Key has been consumed */
			break;
		case GUI_KEY_ESCAPE:
			break;
		default:
			if (!(pObj->Flags & MULTIEDIT_SF_READONLY) && (Key >= 0x20)) {
				if (_IsOverwriteAtThisChar(pObj)) {
					_DeleteChar(pObj);
				}
				if (_InsertChar(pObj, Key)) {
					_SetCursorPos(pObj, pObj->CursorPosChar + 1);
				}
				r = 1;               /* Key has been consumed */
			}
	}
	_InvalidateTextArea(hObj);
	return r;
}
static void _MULTIEDIT_Callback(WM_MESSAGE *pMsg) {
	MULTIEDIT_OBJ *pObj = pMsg->hWin;
	/* Let widget handle the standard messages */
	if (WIDGET_HandleActive(pObj, pMsg) == 0) {
		return;
	}
	switch (pMsg->MsgId) {
		case WM_NOTIFY_CLIENTCHANGE:
			_InvalidateCursorXY(pObj);
			_InvalidateNumLines(pObj);
			_InvalidateTextSizeX(pObj);
			_ClearCache(pObj);
			_CalcScrollParas(pObj);
			break;
		case WM_SIZE:
			_InvalidateCursorXY(pObj);
			_InvalidateNumLines(pObj);
			_InvalidateTextSizeX(pObj);
			_ClearCache(pObj);
			_Invalidate(pObj);
			break;
		case WM_NOTIFY_PARENT:
			switch (pMsg->Data) {
				case WM_NOTIFICATION_VALUE_CHANGED: {
					WM_SCROLL_STATE ScrollState;
					if (pMsg->hWinSrc == WM_GetScrollbarV(pObj)) {
						WM_GetScrollState(pMsg->hWinSrc, &ScrollState);
						pObj->ScrollStateV.v = ScrollState.v;
						WM_Invalidate(pObj);
						WM_NotifyParent(pObj, WM_NOTIFICATION_SCROLL_CHANGED);
					}
					else if (pMsg->hWinSrc == WM_GetScrollbarH(pObj)) {
						WM_GetScrollState(pMsg->hWinSrc, &ScrollState);
						pObj->ScrollStateH.v = ScrollState.v;
						WM_Invalidate(pObj);
						WM_NotifyParent(pObj, WM_NOTIFICATION_SCROLL_CHANGED);
					}
					break;
				}
				case WM_NOTIFICATION_SCROLLBAR_ADDED:
					_SetScrollState(pObj);
					break;
			}
			break;
		case WM_PAINT:
			_MULTIEDIT_Paint(pObj);
			return;
		case WM_TOUCH:
			_OnTouch(pObj, (const GUI_PID_STATE *)pMsg->Data);
			break;
		case WM_DELETE:
			GUI_ALLOC_FreePtr(&pObj->hText);
			break;
		case WM_KEY:
			if (_OnKey(pObj, (const WM_KEY_INFO *)pMsg->Data))
				return;
			break;
	}
	WM_DefaultProc(pMsg);
}
/* Note: the parameters to a create function may vary.
		 Some widgets may have multiple create functions */
MULTIEDIT_HANDLE MULTIEDIT_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int WinFlags, int ExFlags,
									int Id, int BufferSize, const char *pText) {
	MULTIEDIT_HANDLE hObj;
	/* Create the window */

	if ((xsize == 0) && (ysize == 0) && (x0 == 0) && (y0 == 0)) {
		GUI_RECT Rect;
		WM_GetClientRectEx(hParent, &Rect);
		xsize = Rect.x1 - Rect.x0 + 1;
		ysize = Rect.y1 - Rect.y0 + 1;
	}
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, &_MULTIEDIT_Callback,
								  sizeof(MULTIEDIT_OBJ) - sizeof(WM_Obj));
	if (hObj) {
		int i;
		MULTIEDIT_OBJ *pObj = (hObj);
		/* init widget specific variables */
		WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
		/* init member variables */
		for (i = 0; i < NUM_DISP_MODES; i++) {
			pObj->aBkColor[i] = _aDefaultBkColor[i];
			pObj->aColor[i] = _aDefaultColor[i];
		}
		pObj->pFont = _pDefaultFont;
		pObj->Flags = ExFlags;
		pObj->CursorPosChar = 0;
		pObj->CursorPosByte = 0;
		pObj->HBorder = 1;
		pObj->MaxNumChars = 0;
		pObj->NumCharsPrompt = 0;
		pObj->BufferSize = 0;
		pObj->hText = 0;
		if (BufferSize > 0) {
			WM_HWIN hText;
			if ((hText = GUI_ALLOC_AllocZero(BufferSize)) != 0) {
				pObj->BufferSize = BufferSize;
				pObj->hText = hText;
			}
			else {
				WM_DeleteWindow(hObj);
				hObj = 0;
			}
		}
		MULTIEDIT_SetText(hObj, pText);
		_ManageScrollers(hObj);
	}
	else {
	}

	return hObj;
}
int MULTIEDIT_AddKey(MULTIEDIT_HANDLE hObj, uint16_t Key) {
	int r = 0;
	if (hObj) {

		r = _AddKey(hObj, Key);

	}
	return r;
}
void MULTIEDIT_SetText(MULTIEDIT_HANDLE hObj, const char *pNew) {
	if (hObj) {
		MULTIEDIT_OBJ *pObj;
		int NumCharsNew = 0, NumCharsOld = 0;
		int NumBytesNew = 0, NumBytesOld = 0;
		char *pText;

		pObj = (hObj);
		if (pObj->hText) {
			pText = (char *)(pObj->hText);
			pText += GUI_UC__NumChars2NumBytes(pText, pObj->NumCharsPrompt);
			NumCharsOld = GUI__GetNumChars(pText);
			NumBytesOld = GUI_UC__NumChars2NumBytes(pText, NumCharsOld);
		}
		if (pNew) {
			NumCharsNew = GUI__GetNumChars(pNew);
			NumBytesNew = GUI_UC__NumChars2NumBytes(pNew, NumCharsNew);
		}
		if (_IsCharsAvailable(pObj, NumCharsNew - NumCharsOld)) {
			if (_IsSpaceInBuffer(pObj, NumBytesNew - NumBytesOld)) {
				pText = (char *)(pObj->hText);
				pText += GUI_UC__NumChars2NumBytes(pText, pObj->NumCharsPrompt);
				if (pNew) {
					GUI__strcpy(pText, pNew);
				}
				else {
					*pText = 0;
				}
				_SetCursorPos(pObj, pObj->NumCharsPrompt);
				_InvalidateTextArea(hObj);
				_InvalidateNumChars(pObj);
				_InvalidateNumLines(pObj);
				_InvalidateTextSizeX(pObj);
			}
		}

	}
}
void MULTIEDIT_GetText(MULTIEDIT_HANDLE hObj, char *sDest, int MaxLen) {
	if (hObj) {
		MULTIEDIT_OBJ *pObj;

		pObj = (hObj);
		if (pObj) {
			char *pText;
			int Len;
			pText = (char *)(pObj->hText);
			pText += GUI_UC__NumChars2NumBytes(pText, pObj->NumCharsPrompt);
			Len = GUI__strlen(pText);
			if (Len > (MaxLen - 1)) {
				Len = MaxLen - 1;
			}
			GUI__memcpy(sDest, pText, Len);
			*(sDest + Len) = 0;
		}

	}
}
void MULTIEDIT_GetPrompt(MULTIEDIT_HANDLE hObj, char *sDest, int MaxLen) {
	if (hObj) {
		MULTIEDIT_OBJ *pObj;

		pObj = (hObj);
		if (pObj) {
			char *sSource = (char *)(pObj->hText);
			int Len = GUI_UC__NumChars2NumBytes(sSource, pObj->NumCharsPrompt);
			if (Len > (MaxLen - 1)) {
				Len = MaxLen - 1;
			}
			GUI__memcpy(sDest, sSource, Len);
			*(sDest + Len) = 0;
		}

	}
}
void MULTIEDIT_SetWrapWord(MULTIEDIT_HANDLE hObj) {
	_SetWrapMode(hObj, GUI_WRAPMODE_WORD);
}
void MULTIEDIT_SetWrapChar(MULTIEDIT_HANDLE hObj) {
	_SetWrapMode(hObj, GUI_WRAPMODE_CHAR);
}
void MULTIEDIT_SetWrapNone(MULTIEDIT_HANDLE hObj) {
	_SetWrapMode(hObj, GUI_WRAPMODE_NONE);
}
void MULTIEDIT_SetInsertMode(MULTIEDIT_HANDLE hObj, int OnOff) {
	_SetFlag(hObj, OnOff, MULTIEDIT_SF_INSERT);
}
void MULTIEDIT_SetReadOnly(MULTIEDIT_HANDLE hObj, int OnOff) {
	_SetFlag(hObj, OnOff, MULTIEDIT_SF_READONLY);
}
void MULTIEDIT_SetPasswordMode(MULTIEDIT_HANDLE hObj, int OnOff) {
	if (hObj) {
		MULTIEDIT_OBJ *pObj;

		pObj = (hObj);
		_SetFlag(hObj, OnOff, MULTIEDIT_SF_PASSWORD);
		_InvalidateCursorXY(pObj);
		_InvalidateNumLines(pObj);
		_InvalidateTextSizeX(pObj);

	}
}
void MULTIEDIT_SetAutoScrollV(MULTIEDIT_HANDLE hObj, int OnOff) {
	_SetFlag(hObj, OnOff, MULTIEDIT_SF_AUTOSCROLLBAR_V);
}
void MULTIEDIT_SetAutoScrollH(MULTIEDIT_HANDLE hObj, int OnOff) {
	_SetFlag(hObj, OnOff, MULTIEDIT_SF_AUTOSCROLLBAR_H);
}
void MULTIEDIT_SetHBorder(MULTIEDIT_HANDLE hObj, unsigned HBorder) {
	if (hObj) {
		MULTIEDIT_OBJ *pObj;

		pObj = (hObj);
		if ((unsigned)pObj->HBorder != HBorder) {
			pObj->HBorder = HBorder;
			_Invalidate(hObj);
		}

	}
}
void MULTIEDIT_SetFont(MULTIEDIT_HANDLE hObj, const GUI_FONT  *pFont) {
	if (hObj) {
		MULTIEDIT_OBJ *pObj;

		pObj = (hObj);
		if (pObj->pFont != pFont) {
			pObj->pFont = pFont;
			_InvalidateTextArea(hObj);
			_InvalidateCursorXY(pObj);
			_InvalidateNumLines(pObj);
			_InvalidateTextSizeX(pObj);
		}

	}
}
void MULTIEDIT_SetBkColor(MULTIEDIT_HANDLE hObj, unsigned Index, RGB_COLOR color) {
	if (hObj && (Index < NUM_DISP_MODES)) {
		MULTIEDIT_OBJ *pObj;

		pObj = (hObj);
		pObj->aBkColor[Index] = color;
		_InvalidateTextArea(hObj);

	}
}
void MULTIEDIT_SetCursorOffset(MULTIEDIT_HANDLE hObj, int Offset) {
	if (hObj) {
		MULTIEDIT_OBJ *pObj;

		pObj = (hObj);
		_SetCursorPos(pObj, Offset);
		WM_Invalidate(hObj);

	}
}
void MULTIEDIT_SetTextColor(MULTIEDIT_HANDLE hObj, unsigned Index, RGB_COLOR color) {
	if (hObj && (Index < NUM_DISP_MODES)) {
		MULTIEDIT_OBJ *pObj;

		pObj = (hObj);
		pObj->aColor[Index] = color;
		WM_Invalidate(hObj);

	}
}
void MULTIEDIT_SetPrompt(MULTIEDIT_HANDLE hObj, const char *pPrompt) {
	if (hObj) {
		MULTIEDIT_OBJ *pObj;
		int NumCharsNew = 0, NumCharsOld = 0;
		int NumBytesNew = 0, NumBytesOld = 0;
		char *pText;

		pObj = (hObj);
		if (pObj->hText) {
			pText = (char *)(pObj->hText);
			NumCharsOld = pObj->NumCharsPrompt;
			NumBytesOld = GUI_UC__NumChars2NumBytes(pText, NumCharsOld);
		}
		if (pPrompt) {
			NumCharsNew = GUI__GetNumChars(pPrompt);
			NumBytesNew = GUI_UC__NumChars2NumBytes(pPrompt, NumCharsNew);
		}
		if (_IsCharsAvailable(pObj, NumCharsNew - NumCharsOld)) {
			if (_IsSpaceInBuffer(pObj, NumBytesNew - NumBytesOld)) {
				pText = (char *)(pObj->hText);
				GUI__memmove(pText + NumBytesNew, pText + NumBytesOld, GUI__strlen(pText + NumBytesOld) + 1);
				if (pPrompt) {
					GUI__memcpy(pText, pPrompt, NumBytesNew);
				}
				pObj->NumCharsPrompt = NumCharsNew;
				_SetCursorPos(pObj, NumCharsNew);
				_InvalidateTextArea(hObj);
				_InvalidateNumChars(pObj);
				_InvalidateNumLines(pObj);
				_InvalidateTextSizeX(pObj);
			}
		}

	}
}
void MULTIEDIT_SetBufferSize(MULTIEDIT_HANDLE hObj, int BufferSize) {
	if (hObj) {
		MULTIEDIT_OBJ *pObj;
		WM_HMEM hText;

		pObj = (hObj);
		if ((hText = GUI_ALLOC_AllocZero(BufferSize)) == 0) {
		}
		else {
			GUI_ALLOC_FreePtr(&pObj->hText);
			pObj->hText = hText;
			pObj->BufferSize = BufferSize;
			pObj->NumCharsPrompt = 0;
			_SetCursorPos(pObj, 0);
			_InvalidateNumChars(pObj);
			_InvalidateCursorXY(pObj);
			_InvalidateNumLines(pObj);
			_InvalidateTextSizeX(pObj);
		}
		_InvalidateTextArea(hObj);

	}
}
void MULTIEDIT_SetMaxNumChars(MULTIEDIT_HANDLE hObj, unsigned MaxNumChars) {
	if (hObj) {
		MULTIEDIT_OBJ *pObj;

		pObj = (hObj);
		pObj->MaxNumChars = MaxNumChars;
		if (MaxNumChars < (unsigned)pObj->NumCharsPrompt) {
			pObj->NumCharsPrompt = MaxNumChars;
		}
		if (pObj->hText && MaxNumChars) {
			char *pText;
			int Offset;
			pText = (char *)(pObj->hText);
			Offset = GUI_UC__NumChars2NumBytes(pText, MaxNumChars);
			if (Offset < pObj->BufferSize) {
				pText += Offset;
				*pText = 0;
				_SetCursorPos(pObj, Offset);
				_InvalidateTextArea(hObj);
				_InvalidateNumChars(pObj);
			}
		}

	}
}
/*********************************************************************
*
*       MULTIEDIT_GetTextSize
*
* Purpose:
*   Returns the number of bytes required to store the text.
*   It is typically used when allocating a buffer to pass to
*   MULTIEDIT_GetText().
*/
int MULTIEDIT_GetTextSize(MULTIEDIT_HANDLE hObj) {
	int r = 0;
	if (hObj) {
		MULTIEDIT_OBJ *pObj;

		pObj = (hObj);
		if (pObj->hText) {
			const char *s;
			s = (const char *)pObj->hText;
			s += GUI_UC__NumChars2NumBytes(s, pObj->NumCharsPrompt);
			r = 1 + GUI__strlen(s);
		}

	}
	return r;
}

MULTIEDIT_HANDLE MULTIEDIT_Create(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, int ExFlags, const char *pText, int MaxLen) {
	return MULTIEDIT_CreateEx(x0, y0, xsize, ysize, hParent, Flags, ExFlags, Id, MaxLen, pText);
}
MULTIEDIT_HANDLE MULTIEDIT_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	MULTIEDIT_HANDLE  hThis;
	GUI_USE_PARA(cb);
	hThis = MULTIEDIT_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							   hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id, pCreateInfo->Para, NULL);
	return hThis;
}
