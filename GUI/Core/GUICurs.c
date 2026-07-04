#include "GUI_Private.h"

static const RGB_COLOR _aColor[] = { RGB_RED, RGB_BLACK, RGB_WHITE };
const GUI_LOGPALETTE GUI_CursorPal = {
  3,	/* number of entries */
  1, 	/* Has transparency */
  &_aColor[0]
};

static const RGB_COLOR _aColorI[] = { RGB_RED, RGB_WHITE, RGB_BLACK };
const GUI_LOGPALETTE GUI_CursorPalI = {
  3,	/* number of entries */
  1, 	/* Has transparency */
  &_aColorI[0]
};

#if GUI_SUPPORT_CURSOR

static int _AllocSize;
static GUI_HMEM _hBuffer = NULL;
static GUI_RECT _Rect;
static BOOL _CursorIsVis = FALSE, _CursorOn = FALSE;
static const GUI_CURSOR *_pCursor = NULL;
static uint8_t _CursorDeActCnt = 0;
static int16_t _x, _y; /* Position of hot spot */
static GUI_RECT _ClipRect;
const RGB_COLOR *aCursorPal = NULL;

static void _SetPixel(int x, int y, int Index) {
	if ((y >= _ClipRect.y0) && (y <= _ClipRect.y1)) {
		if ((x >= _ClipRect.x0) && (x <= _ClipRect.x1)) {
			LCD_aAPI[0]->pfSetPixel(x, y, Index);
		}
	}
}
static int _GetPixel(int x, int y) {
	if ((y >= _ClipRect.y0) && (y <= _ClipRect.y1)) {
		if ((x >= _ClipRect.x0) && (x <= _ClipRect.x1)) {
			return LCD_L0_GetPixel(x, y);
		}
	}
	return 0;
}


static void _Undraw(void) {
	int x, y, xSize, ySize;
	RGB_COLOR *pData;
	/* Save bitmap data */
	if (_hBuffer) {
		pData = (RGB_COLOR *)_hBuffer;
		xSize = _Rect.x1 - _Rect.x0 + 1;
		ySize = _Rect.y1 - _Rect.y0 + 1;
		for (y = 0; y < ySize; y++) {
			for (x = 0; x < xSize; x++) {
				_SetPixel(x + _Rect.x0, y + _Rect.y0, *(pData + x));
			}
			pData += _pCursor->pBitmap->XSize;
		}
	}
}

static void _Draw(void) {
	int x, y, xSize, ySize;
	RGB_COLOR *pData;
	const GUI_BITMAP *pBM;
	if (_hBuffer) {
		/* Save bitmap data */
		pBM = _pCursor->pBitmap;
		pData = (RGB_COLOR *)_hBuffer;
		xSize = _Rect.x1 - _Rect.x0 + 1;
		ySize = _Rect.y1 - _Rect.y0 + 1;
		for (y = 0; y < ySize; y++) {
			for (x = 0; x < xSize; x++) {
				int BitmapPixel;
				*(pData + x) = _GetPixel(_Rect.x0 + x, _Rect.y0 + y);
				BitmapPixel = GUI_GetBitmapPixel(pBM, x, y);
				if (BitmapPixel) {
					if (aCursorPal)
						_SetPixel(_Rect.x0 + x, _Rect.y0 + y, aCursorPal[BitmapPixel]);
					else
						_SetPixel(_Rect.x0 + x, _Rect.y0 + y, BitmapPixel);
				}
			}
			pData += pBM->XSize;
		}
	}
}

static void _CalcRect(void) {
	if (_pCursor) {
		_Rect.x0 = _x - _pCursor->xHot;
		_Rect.y0 = _y - _pCursor->yHot;
		_Rect.x1 = _Rect.x0 + _pCursor->pBitmap->XSize - 1;
		_Rect.y1 = _Rect.y0 + _pCursor->pBitmap->YSize - 1;
	}
}

static void _Hide(void) {
	if (_CursorIsVis) {
		_Undraw();
		_CursorIsVis = 0;
	}
}

void GUI_CURSOR__TempShow(void) {
	if (_CursorOn && !_CursorDeActCnt) {
		_CursorIsVis = TRUE;
		_Draw();
	}
}

BOOL GUI_CURSOR__TempHide(const GUI_RECT *pRect) {
	if (!_CursorIsVis)
		return FALSE; /* Cursor not visible -> nothing to do */
	if (!pRect || GUI_RectsIntersect(pRect, &_Rect)) {
		_Hide(); /* Cursor needs to be hidden */
		return TRUE;
	}
	return FALSE; /* Cursor not affected -> nothing to do */
}

void GUI_CURSOR_Show(void) {
	LCDDEV_L0_GetRect(&_ClipRect);
	_Hide();
	_CursorOn = TRUE;
	if (!_pCursor)
		GUI_CURSOR_Select(GUI_DEFAULT_CURSOR);
	else
		GUI_CURSOR__TempShow();
}
void GUI_CURSOR_Hide(void) {
	_Hide();
	_CursorOn = FALSE;
}
void GUI_CURSOR_Activate(void) {
	if (!--_CursorDeActCnt)
		GUI_CURSOR__TempShow();
}
void GUI_CURSOR_Deactivate(void) {
	if (_CursorDeActCnt++ == 0)
		_Hide();
}

const GUI_CURSOR *GUI_CURSOR_Select(const GUI_CURSOR *pCursor) {
	int AllocSize;
	const GUI_BITMAP *pBM;
	const GUI_CURSOR *pOldCursor;
	pOldCursor = _pCursor;
	if (pCursor != _pCursor) {
		pBM = pCursor->pBitmap;
		aCursorPal = pBM->pPal->pPalEntries;
		_Hide();
		AllocSize = pBM->XSize * pBM->YSize * sizeof(RGB_COLOR);
		if (AllocSize != _AllocSize) {
			GUI_ALLOC_Free(_hBuffer);
			_hBuffer = 0;
		}
		_hBuffer = GUI_ALLOC_AllocZero(AllocSize);
		_CursorOn = TRUE;
		_pCursor = pCursor;
		_CalcRect();
		GUI_CURSOR__TempShow();
	}
	return pOldCursor;
}

void GUI_CURSOR_SetPosition(int xNewPos, int yNewPos) {
	int x, xStart, xStep, xEnd, xOff, xOverlapMin, xOverlapMax;
	int y, yStart, yStep, yEnd, yOff, yOverlapMin, yOverlapMax;
	int xSize;
	RGB_COLOR *pData;
	if (_hBuffer) {
		if ((_x != xNewPos) | (_y != yNewPos)) {
			if (_CursorOn) {
				const GUI_BITMAP *pBM = _pCursor->pBitmap;
				/* Save & set clip rect */
				/* Compute helper variables */
				pData = (RGB_COLOR *)_hBuffer;
				xSize = _pCursor->pBitmap->XSize;
				xOff = xNewPos - _x;
				if (xOff > 0) {
					xStep = 1;
					xStart = 0;
					xEnd = _pCursor->pBitmap->XSize;
					xOverlapMax = xEnd - 1;
					xOverlapMin = xOff;
				}
				else {
					xStep = -1;
					xStart = xSize - 1;
					xEnd = -1;
					xOverlapMin = 0;
					xOverlapMax = xStart + xOff;
				}
				yOff = yNewPos - _y;
				if (yOff > 0) {
					yStep = 1;
					yStart = 0;
					yEnd = _pCursor->pBitmap->YSize;
					yOverlapMax = yEnd - 1;
					yOverlapMin = yOff;
				}
				else {
					yStep = -1;
					yStart = _pCursor->pBitmap->YSize - 1;
					yEnd = -1;
					yOverlapMin = 0;
					yOverlapMax = yStart + yOff;
				}
				/* Restore & Draw */
				for (y = yStart; y != yEnd; y += yStep) {
					char yOverlaps;
					char yNewOverlaps;
					int yNew = y + yOff;
					yOverlaps = (y >= yOverlapMin) && (y <= yOverlapMax);
					yNewOverlaps = (yNew >= yOverlapMin) && (yNew <= yOverlapMax);
					for (x = xStart; x != xEnd; x += xStep) {
						char xyOverlaps, xyNewOverlaps;
						int BitmapPixel;
						RGB_COLOR Pixel;
						RGB_COLOR *pSave = pData + x + y * xSize;
						int xNew = x + xOff;
						BitmapPixel = GUI_GetBitmapPixel(pBM, x, y);
						xyOverlaps = (x >= xOverlapMin) && (x <= xOverlapMax) && yOverlaps;
						xyNewOverlaps = (xNew >= xOverlapMin) && (xNew <= xOverlapMax) && yNewOverlaps;
						/* Restore old pixel if it was not transparent */
						if (BitmapPixel) {
							if (!xyOverlaps || (GUI_GetBitmapPixel(pBM, x - xOff, y - yOff) == 0)) {
								_SetPixel(x + _Rect.x0, y + _Rect.y0, *(pSave));
							}
						}
						/* Save */
						if (xyNewOverlaps) {
							Pixel = *(pData + xNew + yNew * xSize);
						}
						else {
							Pixel = _GetPixel(_Rect.x0 + xNew, _Rect.y0 + yNew);
						}
						*pSave = Pixel;
						/* Write new  ... We could write pixel by pixel here */
						if (BitmapPixel) {
							if (aCursorPal)
								_SetPixel(xNew + _Rect.x0, yNew + _Rect.y0, aCursorPal[BitmapPixel]);
							else
								_SetPixel(xNew + _Rect.x0, yNew + _Rect.y0, BitmapPixel);
						}
					}
				}
			}
			_x = xNewPos;
			_y = yNewPos;
			_CalcRect();
		}
	}

}

#endif   /* GUI_SUPPORT_CURSOR */
