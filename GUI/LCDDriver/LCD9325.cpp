#include "GUI_Private.h"


static void  _DrawBitLine1BPP(int x, int y, uint8_t const *p, int Diff, int xsize, const RGBC *pTrans) {
	RGBC Index0 = pTrans[0], Index1 = pTrans[1];
	x += Diff;
	if (GUI_Context.DrawMode & DRAWMODE_TRANS)
		do {
			if (*p & (0x80 >> Diff))
				LCD_L0_SetPixel(x, y, Index1);
			x++;
			if (++Diff == 8) {
				Diff = 0;
				p++;
			}
		} while (--xsize);
	else do {
		LCD_L0_SetPixel(x++, y, (*p & (0x80 >> Diff)) ? Index1 : Index0);
		if (++Diff == 8) {
			Diff = 0;
			p++;
		}
	} while (--xsize);
}
static void  _DrawBitLine2BPP(int x, int y, uint8_t const *p, int Diff, int xsize, const RGBC *pTrans) {
	RGBC Pixels = *p;
	int CurrentPixel = Diff;
	x += Diff;
	if (GUI_Context.DrawMode & DRAWMODE_TRANS) {
		if (pTrans)
			do {
				int Shift = (3 - CurrentPixel) << 1;
				int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
				if (Index) {
					RGBC PixelIndex = *(pTrans + Index);
					LCD_L0_SetPixel(x, y, PixelIndex);
				}
				x++;
				if (++CurrentPixel == 4) {
					CurrentPixel = 0;
					Pixels = *(++p);
				}
			} while (--xsize);
		else do {
			int Shift = (3 - CurrentPixel) << 1;
			int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
			if (Index) {
				LCD_L0_SetPixel(x, y, Index);
			}
			x++;
			if (++CurrentPixel == 4) {
				CurrentPixel = 0;
				Pixels = *(++p);
			}
		} while (--xsize);
	}
	else if (pTrans)
		do {
			int Shift = (3 - CurrentPixel) << 1;
			int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
			RGBC PixelIndex = *(pTrans + Index);
			LCD_L0_SetPixel(x++, y, PixelIndex);
			if (++CurrentPixel == 4) {
				CurrentPixel = 0;
				Pixels = *(++p);
			}
		} while (--xsize);
	else do {
		int Shift = (3 - CurrentPixel) << 1;
		int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
		LCD_L0_SetPixel(x++, y, Index);
		if (++CurrentPixel == 4) {
			CurrentPixel = 0;
			Pixels = *(++p);
		}
	} while (--xsize);
}
static void  _DrawBitLine4BPP(int x, int y, uint8_t const *p, int Diff, int xsize, const RGBC *pTrans) {
	RGBC Pixels = *p;
	int CurrentPixel = Diff;
	x += Diff;
	if (GUI_Context.DrawMode & DRAWMODE_TRANS)
		if (pTrans)
			do {
				int Shift = (1 - CurrentPixel) << 2;
				int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
				if (Index) {
					RGBC PixelIndex = *(pTrans + Index);
					LCD_L0_SetPixel(x, y, PixelIndex);
				}
				x++;
				if (++CurrentPixel == 2) {
					CurrentPixel = 0;
					Pixels = *(++p);
				}
			} while (--xsize);
		else do {
			int Shift = (1 - CurrentPixel) << 2;
			int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
			if (Index)
				LCD_L0_SetPixel(x, y, Index);
			x++;
			if (++CurrentPixel == 2) {
				CurrentPixel = 0;
				Pixels = *(++p);
			}
		} while (--xsize);
	else if (pTrans)
		do {
			int Shift = (1 - CurrentPixel) << 2;
			int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
			RGBC PixelIndex = *(pTrans + Index);
			LCD_L0_SetPixel(x++, y, PixelIndex);
			if (++CurrentPixel == 2) {
				CurrentPixel = 0;
				Pixels = *(++p);
			}
		} while (--xsize);
	else do {
		int Shift = (1 - CurrentPixel) << 2;
		int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
		LCD_L0_SetPixel(x++, y, Index);
		if (++CurrentPixel == 2) {
			CurrentPixel = 0;
			Pixels = *(++p);
		}
	} while (--xsize);

}

static void  _DrawBitLine8BPP(int x, int y, uint8_t const *p, int xsize, const RGBC *pTrans) {
	RGBC Pixel;
	if (GUI_Context.DrawMode & DRAWMODE_TRANS) {
		if (pTrans)
			for (; xsize--; x++, p++) {
				Pixel = *p;
				if (Pixel)
					LCD_L0_SetPixel(x, y, *(pTrans + Pixel));
			}
		else for (; xsize--; x++, p++) {
			Pixel = *p;
			if (Pixel)
				LCD_L0_SetPixel(x, y, Pixel);
		}
	}
	else if (pTrans)
		for (; xsize--; x++, p++)
			LCD_L0_SetPixel(x, y, pTrans[*p]);
	else for (; xsize--; x++, p++)
		LCD_L0_SetPixel(x, y, *p);
}
static void  DrawBitLine16BPP(int x, int y, uint16_t const *p, int xsize, const RGBC *pTrans) {
	RGBC pixel;
	if (GUI_Context.DrawMode & DRAWMODE_TRANS)
		if (pTrans)
			for (; xsize--; x++, p++) {
				pixel = *p;
				if (pixel)
					LCD_L0_SetPixel(x, y, *(pTrans + pixel));
			}
		else for (; xsize--; x++, p++) {
			pixel = *p;
			if (pixel)
				LCD_L0_SetPixel(x, y, pixel);
		}
	else if (pTrans)
		for (; xsize--; x++, p++) {
			pixel = *p;
			LCD_L0_SetPixel(x, y, *(pTrans + pixel));
		}
	else for (; xsize--; x++, p++)
		LCD_L0_SetPixel(x, y, *p);
}

static void _DrawBitLine24BPP(int x, int y, RGBC const *p, int xsize, const RGBC *pTrans) {
	RGBC pixel;
	if (GUI_Context.DrawMode & DRAWMODE_TRANS)
		for (; xsize--; x++, p++) {
			pixel = *p;
			if (pixel)
				LCD_L0_SetPixel(x, y, pixel);
		}
	else for (; xsize--; x++, p++)
		LCD_L0_SetPixel(x, y, *p);
}

void LCD_L0_DrawBitmap(int x0, int y0,
					   int xsize, int ysize,
					   int BitsPerPixel,
					   int BytesPerLine,
					   const uint8_t *pData, int Diff,
					   const RGBC *pTrans) {
	int y1 = y0 + ysize;
	switch (BitsPerPixel) {
		case 1:
			for (; y0 < y1; y0++, pData += BytesPerLine)
				_DrawBitLine1BPP(x0, y0, pData, Diff, xsize, pTrans);
			break;
		case 2:
			for (; y0 < y1; y0++, pData += BytesPerLine)
				_DrawBitLine2BPP(x0, y0, pData, Diff, xsize, pTrans);
			break;
		case 4:
			for (; y0 < y1; y0++, pData += BytesPerLine)
				_DrawBitLine4BPP(x0, y0, pData, Diff, xsize, pTrans);
			break;
		case 8:
			for (; y0 < y1; y0++, pData += BytesPerLine)
				_DrawBitLine8BPP(x0, y0, pData, xsize, pTrans);
			break;
		case 16:
			for (; y0 < y1; y0++, pData += BytesPerLine)
				DrawBitLine16BPP(x0, y0, (const uint16_t *)pData, xsize, pTrans);
			break;
		case 24:
			for (; y0 < y1; y0++, pData += BytesPerLine)
				_DrawBitLine24BPP(x0, y0, (const RGBC *)pData, xsize, pTrans);
			break;
	}
}
