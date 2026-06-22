#include "GUI_Private.h"

static void  _DrawBitLine1BPP(int x, int y, uint8_t const GUI_UNI_PTR *p, int Diff, int xsize, const RGB_COLOR *pTrans) {
	RGB_COLOR Index0 = *(pTrans + 0);
	RGB_COLOR Index1 = *(pTrans + 1);
	x += Diff;
	switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
		case 0:
			do {
				LCD_L0_SetPixelIndex(x++, y, (*p & (0x80 >> Diff)) ? Index1 : Index0);
				if (++Diff == 8) {
					Diff = 0;
					p++;
				}
			} while (--xsize);
			break;
		case LCD_DRAWMODE_TRANS:
			do {
				if (*p & (0x80 >> Diff))
					LCD_L0_SetPixelIndex(x, y, Index1);
				x++;
				if (++Diff == 8) {
					Diff = 0;
					p++;
				}
			} while (--xsize);
			break;
		case LCD_DRAWMODE_XOR:
			do {
				if (*p & (0x80 >> Diff)) {
					LCD_L0_SetPixelIndex(x, y, ~LCD_L0_GetPixelIndex(x, y));
				}
				x++;
				if (++Diff == 8) {
					Diff = 0;
					p++;
				}
			} while (--xsize);
			break;
	}
}
static void  _DrawBitLine2BPP(int x, int y, uint8_t const GUI_UNI_PTR *p, int Diff, int xsize, const RGB_COLOR *pTrans) {
	RGB_COLOR Pixels = *p;
	int CurrentPixel = Diff;
	x += Diff;
	switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
		case 0:
			if (pTrans) {
				do {
					int Shift = (3 - CurrentPixel) << 1;
					int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
					RGB_COLOR PixelIndex = *(pTrans + Index);
					LCD_L0_SetPixelIndex(x++, y, PixelIndex);
					if (++CurrentPixel == 4) {
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				} while (--xsize);
			}
			else {
				do {
					int Shift = (3 - CurrentPixel) << 1;
					int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
					LCD_L0_SetPixelIndex(x++, y, Index);
					if (++CurrentPixel == 4) {
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				} while (--xsize);
			}
			break;
		case LCD_DRAWMODE_TRANS:
			if (pTrans) {
				do {
					int Shift = (3 - CurrentPixel) << 1;
					int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
					if (Index) {
						RGB_COLOR PixelIndex = *(pTrans + Index);
						LCD_L0_SetPixelIndex(x, y, PixelIndex);
					}
					x++;
					if (++CurrentPixel == 4) {
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				} while (--xsize);
			}
			else {
				do {
					int Shift = (3 - CurrentPixel) << 1;
					int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
					if (Index) {
						LCD_L0_SetPixelIndex(x, y, Index);
					}
					x++;
					if (++CurrentPixel == 4) {
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				} while (--xsize);
			}
			break;
	}
}
static void  _DrawBitLine4BPP(int x, int y, uint8_t const GUI_UNI_PTR *p, int Diff, int xsize, const RGB_COLOR *pTrans) {
	RGB_COLOR Pixels = *p;
	int CurrentPixel = Diff;
	x += Diff;
	switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
		case 0:
			if (pTrans) {
				do {
					int Shift = (1 - CurrentPixel) << 2;
					int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
					RGB_COLOR PixelIndex = *(pTrans + Index);
					LCD_L0_SetPixelIndex(x++, y, PixelIndex);
					if (++CurrentPixel == 2) {
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				} while (--xsize);
			}
			else {
				do {
					int Shift = (1 - CurrentPixel) << 2;
					int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
					LCD_L0_SetPixelIndex(x++, y, Index);
					if (++CurrentPixel == 2) {
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				} while (--xsize);
			}
			break;
		case LCD_DRAWMODE_TRANS:
			if (pTrans) {
				do {
					int Shift = (1 - CurrentPixel) << 2;
					int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
					if (Index) {
						RGB_COLOR PixelIndex = *(pTrans + Index);
						LCD_L0_SetPixelIndex(x, y, PixelIndex);
					}
					x++;
					if (++CurrentPixel == 2) {
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				} while (--xsize);
			}
			else {
				do {
					int Shift = (1 - CurrentPixel) << 2;
					int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
					if (Index) {
						LCD_L0_SetPixelIndex(x, y, Index);
					}
					x++;
					if (++CurrentPixel == 2) {
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				} while (--xsize);
			}
			break;
	}
}
static void  _DrawBitLine8BPP(int x, int y, uint8_t const GUI_UNI_PTR *p, int xsize, const RGB_COLOR *pTrans) {
	RGB_COLOR Pixel;
	switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
		case 0:
			if (pTrans) {
				for (; xsize > 0; xsize--, x++, p++) {
					Pixel = *p;
					LCD_L0_SetPixelIndex(x, y, *(pTrans + Pixel));
				}
			}
			else {
				for (; xsize > 0; xsize--, x++, p++) {
					LCD_L0_SetPixelIndex(x, y, *p);
				}
			}
			break;
		case LCD_DRAWMODE_TRANS:
			if (pTrans) {
				for (; xsize > 0; xsize--, x++, p++) {
					Pixel = *p;
					if (Pixel) {
						LCD_L0_SetPixelIndex(x, y, *(pTrans + Pixel));
					}
				}
			}
			else {
				for (; xsize > 0; xsize--, x++, p++) {
					Pixel = *p;
					if (Pixel) {
						LCD_L0_SetPixelIndex(x, y, Pixel);
					}
				}
			}
			break;
	}
}
static void  DrawBitLine16BPP(int x, int y, uint16_t const GUI_UNI_PTR *p, int xsize, const RGB_COLOR *pTrans) {
	RGB_COLOR pixel;
	if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS) == 0) {
		if (pTrans) {
			for (; xsize > 0; xsize--, x++, p++) {
				pixel = *p;
				LCD_L0_SetPixelIndex(x, y, *(pTrans + pixel));
			}
		}
		else {
			for (; xsize > 0; xsize--, x++, p++) {
				LCD_L0_SetPixelIndex(x, y, *p);
			}
		}
	}
	else {
		if (pTrans) {
			for (; xsize > 0; xsize--, x++, p++) {
				pixel = *p;
				if (pixel) {
					LCD_L0_SetPixelIndex(x, y, *(pTrans + pixel));
				}
			}
		}
		else {
			for (; xsize > 0; xsize--, x++, p++) {
				pixel = *p;
				if (pixel) {
					LCD_L0_SetPixelIndex(x, y, pixel);
				}
			}
		}
	}
}

void LCD_L0_DrawBitmap(int x0, int y0,
					   int xsize, int ysize,
					   int BitsPerPixel,
					   int BytesPerLine,
					   const uint8_t GUI_UNI_PTR *pData, int Diff,
					   const RGB_COLOR *pTrans) {
	int i;
	/* Use _DrawBitLineXBPP */
	for (i = 0; i < ysize; i++) {
		switch (BitsPerPixel) {
			case 1:
				_DrawBitLine1BPP(x0, i + y0, pData, Diff, xsize, pTrans);
				break;
			case 2:
				_DrawBitLine2BPP(x0, i + y0, pData, Diff, xsize, pTrans);
				break;
			case 4:
				_DrawBitLine4BPP(x0, i + y0, pData, Diff, xsize, pTrans);
				break;
			case 8:
				_DrawBitLine8BPP(x0, i + y0, pData, xsize, pTrans);
				break;
			case 16:
				DrawBitLine16BPP(x0, i + y0, (const uint16_t *)pData, xsize, pTrans);
				break;
		}
		pData += BytesPerLine;
	}
}
