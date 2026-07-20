#include "GUI_Private.h"
#include "GUIDebug.h"
#include "WM.h"
/* Memory device capabilities are compiled only if support for them is enabled.*/
#if GUI_SUPPORT_MEMDEV

#define PIXELINDEX RGB_COLOR

static const RGB_COLOR aID[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

static PIXELINDEX *_XY2PTR(int x, int y) {
	GUI_MEMDEV *pDev = (GUI_MEMDEV *)GUI_Context.hDevData;
	uint8_t *pData = (uint8_t *)(pDev + 1);
	pData += (y - pDev->y0) * pDev->BytesPerLine;
	return ((PIXELINDEX *)pData) + x - pDev->x0;
}

static void _DrawBitLine1BPP(int x, int y, const uint8_t  *p, int Diff, unsigned int xsize,
							 const RGB_COLOR *pTrans, GUI_MEMDEV *pDev, PIXELINDEX *pDest) {
	PIXELINDEX Index1;
	unsigned pixels;
	unsigned PixelCnt;
	PixelCnt = 8 - Diff;
	pixels = LCD_aMirror[*p] >> Diff;
	switch (GUI_Context.DrawMode & (DRAWMODE_TRANS)) {
		case 0: /* Write mode */
			do {
				/* Prepare loop */
				if (PixelCnt > xsize) {
					PixelCnt = xsize;
				}
				xsize -= PixelCnt;
				/* Write as many pixels as we are allowed to and have loaded in this inner loop */
				do {
					*pDest++ = *(pTrans + (pixels & 1));
					pixels >>= 1;
				} while (--PixelCnt);
				/* Check if an other Source byte needs to be loaded */
				if (xsize == 0) {
					return;
				}
				PixelCnt = 8;
				pixels = LCD_aMirror[*++p];
			} while (1);
		case DRAWMODE_TRANS:
			Index1 = *(pTrans + 1);
			do {
				/* Prepare loop */
				if (PixelCnt > xsize)
					PixelCnt = xsize;
				xsize -= PixelCnt;
					do {
						if (pixels == 0) {      /* Early out optimization; not required */
							pDest += PixelCnt;
							break;
						}
						if ((pixels & 1))
							*pDest = Index1;
						pDest++;
						if (--PixelCnt == 0)
							break;
						pixels >>= 1;
					} while (1);
				/* Check if an other Source byte needs to be loaded */
				if (xsize == 0)
					return;
				PixelCnt = 8;
				pixels = LCD_aMirror[*(++p)];
			} while (1);
	}
}
static void _DrawBitLine2BPP(int x, int y, const uint8_t  *p, int Diff, int xsize,
							 const RGB_COLOR *pTrans, PIXELINDEX *pDest) {
	uint8_t pixels;
	uint8_t  PixelCnt;
	PixelCnt = 4 - Diff;
	pixels = (*p) << (Diff << 1);
	switch (GUI_Context.DrawMode & (DRAWMODE_TRANS)) {
		case 0:    /* Write mode */
		PixelLoopWrite:
			if (PixelCnt > xsize)
				PixelCnt = xsize;
			xsize -= PixelCnt;
			do {
				*pDest++ = *(pTrans + (pixels >> 6));
				pixels <<= 2;
			} while (--PixelCnt);
			if (xsize) {
				PixelCnt = 4;
				pixels = *(++p);
				goto PixelLoopWrite;
			}
			break;
		case DRAWMODE_TRANS:
		PixelLoopTrans:
			if (PixelCnt > xsize)
				PixelCnt = xsize;
			xsize -= PixelCnt;
			do {
				if (pixels & 0xc0)
					*pDest = *(pTrans + (pixels >> 6));
				pDest++;
				x++;
				pixels <<= 2;
			} while (--PixelCnt);
			if (xsize) {
				PixelCnt = 4;
				pixels = *(++p);
				goto PixelLoopTrans;
			}
			break;
	}
}
static void _DrawBitLine4BPP(int x, int y, const uint8_t  *p, int Diff, int xsize,
							 const RGB_COLOR *pTrans, PIXELINDEX *pDest) {
	uint8_t pixels;
	uint8_t PixelCnt;
	PixelCnt = 2 - Diff;
	pixels = (*p) << (Diff << 2);
	switch (GUI_Context.DrawMode & (DRAWMODE_TRANS)) {
		/*
				  * Write mode *
		*/
		case 0:
			/* Draw incomplete bytes to the left of center area */
			if (Diff) {
				*pDest = *(pTrans + (pixels >> 4));
				pDest++;
				xsize--;
				pixels = *++p;
			}
			/* Draw center area (2 pixels in one byte) */
			if (xsize >= 2) {
				int i = xsize >> 1;
				xsize &= 1;
				do {
					*pDest = *(pTrans + (pixels >> 4));   /* Draw 1. (left) pixel */
					*(pDest + 1) = *(pTrans + (pixels & 15));   /* Draw 2. (right) pixel */
					pDest += 2;
					pixels = *++p;
				} while (--i);
			}
			/* Draw incomplete bytes to the right of center area */
			if (xsize)
				*pDest = *(pTrans + (pixels >> 4));
			break;
			/*
					  * Transparent draw mode *
			*/
		case DRAWMODE_TRANS:
			/* Draw incomplete bytes to the left of center area */
			if (Diff) {
				if (pixels & 0xF0)
					*pDest = *(pTrans + (pixels >> 4));
				pDest++;
				x++;
				xsize--;
				pixels = *++p;
			}
			/* Draw center area (2 pixels in one byte) */
			while (xsize >= 2) {
				/* Draw 1. (left) pixel */
				if (pixels & 0xF0)
					*pDest = *(pTrans + (pixels >> 4));
				/* Draw 2. (right) pixel */
				if (pixels &= 15)
					*(pDest + 1) = *(pTrans + pixels);
				pDest += 2;
				x += 2;
				xsize -= 2;
				pixels = *++p;
			}
			/* Draw incomplete bytes to the right of center area */
			if (xsize)
				if (pixels >>= 4)
					*pDest = *(pTrans + pixels);
			break;
	}
}
static void _DrawBitLine8BPP(int x, int y, const uint8_t  *pSrc, int xsize,
							 const RGB_COLOR *pTrans, PIXELINDEX *pDest) {
	switch (GUI_Context.DrawMode & (DRAWMODE_TRANS)) {
		case 0:    /* Write mode */
			do {
				*pDest = *(pTrans + *pSrc);
				pDest++;
				pSrc++;
			} while (--xsize);
			break;
		case DRAWMODE_TRANS:
			do {
				if (*pSrc)
					*pDest = *(pTrans + *pSrc);
				x++;
				pDest++;
				pSrc++;
			} while (--xsize);
			break;
	}
}
static void _DrawBitLine8BPP_DDB(int x, int y, const uint8_t  *pSrc, int xsize, PIXELINDEX *pDest) {
	switch (GUI_Context.DrawMode & (DRAWMODE_TRANS)) {
		case 0:    /* Write mode */
			GUI__memcpy(pDest, pSrc, xsize);
			break;
		case DRAWMODE_TRANS:
			do {
				if (*pSrc)
					*pDest = *pSrc;
				x++;
				pDest++;
				pSrc++;
			} while (--xsize);
			break;
	}
}
static void _DrawBitLine16BPP_DDB(int x, int y, const uint16_t *pSrc, int xsize, PIXELINDEX *pDest) {
	switch (GUI_Context.DrawMode & (DRAWMODE_TRANS)) {
		case 0:    /* Write mode */
			GUI__memcpy(pDest, pSrc, xsize * 2);
			break;
		case DRAWMODE_TRANS:
			do {
				if (*pSrc)
					*pDest = *pSrc;
				x++;
				pDest++;
				pSrc++;
			} while (--xsize);
			break;
	}
}

static void _DrawBitLine24BPP_DDB(int x, int y, const RGB_COLOR *pSrc, int xsize, PIXELINDEX *pDest) {
	switch (GUI_Context.DrawMode & (DRAWMODE_TRANS)) {
		case 0: /* Write mode */
			GUI__memcpy(pDest, pSrc, xsize * 4);  /* 4 bytes per pixel for 24-bit */
			break;
		case DRAWMODE_TRANS:
			do {
				if (*pSrc)
					*pDest = *pSrc;
				x++;
				pDest++;
				pSrc++;
			} while (--xsize);
			break;
	}
}

static void _DrawBitmap(int x0, int y0, int xsize, int ysize,
						int BitsPerPixel, int BytesPerLine,
						const uint8_t *pData, int Diff, const RGB_COLOR *pTrans) {
	int i;
	GUI_MEMDEV *pDev = (GUI_MEMDEV *)GUI_Context.hDevData;
	unsigned    BytesPerLineDest;
	PIXELINDEX *pDest;
	BytesPerLineDest = pDev->BytesPerLine;
	x0 += Diff;
	pDest = _XY2PTR(x0, y0);
	/* handle 24 bpp bitmaps (native format) */
	if (BitsPerPixel == 24) {
		for (i = 0; i < ysize; i++) {
			_DrawBitLine24BPP_DDB(x0, i + y0, (const RGB_COLOR *)pData, xsize, pDest);
			pData += BytesPerLine;
			pDest = (PIXELINDEX *)((uint8_t *)pDest + BytesPerLineDest);
		}
		return;
	}
	/* handle 16 bpp bitmaps in high color modes, but only without palette */
	if (BitsPerPixel == 16) {
		for (i = 0; i < ysize; i++) {
			_DrawBitLine16BPP_DDB(x0, i + y0, (const uint16_t *)pData, xsize, pDest);
			pData += BytesPerLine;
			pDest = (PIXELINDEX *)((uint8_t *)pDest + BytesPerLineDest);
		}
		return;
	}
	/* Handle 8 bpp bitmaps seperately as we have different routine bitmaps with or without palette */
	if (BitsPerPixel == 8) {
		for (i = 0; i < ysize; i++) {
			if (pTrans)
				_DrawBitLine8BPP(x0, i + y0, pData, xsize, pTrans, pDest);
			else
				_DrawBitLine8BPP_DDB(x0, i + y0, pData, xsize, pDest);
			pData += BytesPerLine;
			pDest = (PIXELINDEX *)((uint8_t *)pDest + BytesPerLineDest);
		}
		return;
	}
	/* Use aID for bitmaps without palette */
	if (!pTrans)
		pTrans = aID;
	for (i = 0; i < ysize; i++) {
		switch (BitsPerPixel) {
			case 1:
				_DrawBitLine1BPP(x0, i + y0, pData, Diff, xsize, pTrans, pDev, pDest);
				break;
			case 2:
				_DrawBitLine2BPP(x0, i + y0, pData, Diff, xsize, pTrans, pDest);
				break;
			case 4:
				_DrawBitLine4BPP(x0, i + y0, pData, Diff, xsize, pTrans, pDest);
				break;
		}
		pData += BytesPerLine;
		pDest = (PIXELINDEX *)((uint8_t *)pDest + BytesPerLineDest);
	}
}

static void _FillRect(int x0, int y0, int x1, int y1) {
	unsigned BytesPerLine;
	int Len;
	GUI_MEMDEV *pDev = (GUI_MEMDEV *)GUI_Context.hDevData;
	PIXELINDEX *pData = _XY2PTR(x0, y0);
	BytesPerLine = pDev->BytesPerLine;
	Len = x1 - x0 + 1;
	/* Mark rectangle as modified */
	/* Do the drawing */
	for (; y0 <= y1; y0++) {
		int i;
		for (i = 0; i < Len; i++)
			pData[i] = LCD_COLORINDEX;
		pData = (PIXELINDEX *)((uint8_t *)pData + BytesPerLine);
	}
}

static void _DrawHLine(int x0, int y, int x1) {
	_FillRect(x0, y, x1, y);
}

static void _DrawVLine(int x, int y0, int y1) {
	GUI_MEMDEV *pDev = (GUI_MEMDEV *)GUI_Context.hDevData;
	PIXELINDEX *pData = _XY2PTR(x, y0);
	unsigned BytesPerLine = pDev->BytesPerLine;
	unsigned NumPixels = y1 - y0 + 1;
	do {
		*pData = LCD_COLORINDEX;
		pData = (PIXELINDEX *)((uint8_t *)pData + BytesPerLine); /* Same as "pData += pDev->BytesPerLine >> 1;", Just more efficient */
	} while (--NumPixels);
}

static void _SetPixel(int x, int y, RGB_COLOR Index) {
	GUI_MEMDEV *pDev = (GUI_MEMDEV *)GUI_Context.hDevData;
	PIXELINDEX *pData = _XY2PTR(x, y);
	*pData = Index;
}

static RGB_COLOR _GetPixel(int x, int y) {
	PIXELINDEX *pData = _XY2PTR(x, y);
	return *pData;
}

const tLCDDEV_APIList GUI_MEMDEV__APIList24 = {
	(tLCDDEV_DrawBitmap *)_DrawBitmap,
	_DrawHLine,
	_DrawVLine,
	_FillRect,
	_GetPixel,
	GUI_MEMDEV__GetRect,
	_SetPixel,
	nullptr, /* MemDevAPI */
	24    /* BitsPerPixel */
};

#endif /* GUI_SUPPORT_MEMDEV */


