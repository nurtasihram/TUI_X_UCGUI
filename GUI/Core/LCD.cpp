#include "GUI_Private.h"
#include "LCD_Private.h"
#include "GUIDebug.h"

void LCD_SetClipRectMax(void) {
	GUI.ClipRect = LCDDEV_L0_GetRect();
}
void LCD_SetClipRectEx(const RECT *pRect) {
	GUI.ClipRect = *pRect & LCDDEV_L0_GetRect();
}
		
#define RETURN_IF_Y_OUT() \
  if (y < GUI.ClipRect.y0) return;             \
  if (y > GUI.ClipRect.y1) return;
#define RETURN_IF_X_OUT() \
  if (x < GUI.ClipRect.x0) return;             \
  if (x > GUI.ClipRect.x1) return;
#define CLIP_X() \
  if (x0 < GUI.ClipRect.x0) { x0 = GUI.ClipRect.x0; } \
  if (x1 > GUI.ClipRect.x1) { x1 = GUI.ClipRect.x1; }
#define CLIP_Y() \
  if (y0 < GUI.ClipRect.y0) { y0 = GUI.ClipRect.y0; } \
  if (y1 > GUI.ClipRect.y1) { y1 = GUI.ClipRect.y1; }
#define RETURN_IF_Y_OUT() \
  if (y < GUI.ClipRect.y0) return;             \
  if (y > GUI.ClipRect.y1) return;
#define RETURN_IF_X_OUT() \
  if (x < GUI.ClipRect.x0) return;             \
  if (x > GUI.ClipRect.x1) return;

void LCD_SetPixel(int x, int y, int ColorIndex) {
	RETURN_IF_X_OUT();
	RETURN_IF_Y_OUT();
	LCDDEV_L0_SetPixel(x, y, ColorIndex);
}

RGBC LCD_GetPixel(int x, int y) {
	auto r = LCDDEV_L0_GetRect();
	if (x < r.x0) {
		return 0;
	}
	if (x > r.x1) {
		return 0;
	}
	if (y < r.y0) {
		return 0;
	}
	if (y > r.y1) {
		return 0;
	}
	return LCDDEV_L0_GetPixel(x, y);
}

void LCD_DrawPixel(int x, int y) {
	RETURN_IF_Y_OUT();
	RETURN_IF_X_OUT();
	LCDDEV_L0_SetPixel(x, y, LCD_COLORINDEX);
}

void LCD_FillRect(RECT r) {
	if (!(r &= GUI.ClipRect))
		return;
	LCDDEV_L0_FillRect(r);
}

void LCD_DrawBitmap(int x0, int y0, int xsize, int ysize,
					int BitsPerPixel, int BytesPerLine,
					const void *pPix, PCLOGPALETTE pTrans) {
	auto pPixel = (const uint8_t *)pPix;
	uint8_t  Data = 0;
	int x1, y1, Diff;
	/* Handle rotation if necessary */
	/* Handle the optional Y-magnification */
	y1 = y0 + ysize - 1;
	x1 = x0 + xsize - 1;
	/*  Clip y0 (top) */
	Diff = GUI.ClipRect.y0 - y0;
	if (Diff > 0) {
		ysize -= Diff;
		if (ysize <= 0) {
			return;
		}
		y0 = GUI.ClipRect.y0;
		pPixel += (unsigned)Diff * (unsigned)BytesPerLine;
	}
	/*  Clip y1 (bottom) */
	Diff = y1 - GUI.ClipRect.y1;
	if (Diff > 0) {
		ysize -= Diff;
		if (ysize <= 0) {
			return;
		}
	}
	/*        Clip right side    */
	Diff = x1 - GUI.ClipRect.x1;
	if (Diff > 0) {
		xsize -= Diff;
	}
	/*        Clip left side ... (The difficult side ...)    */
	Diff = 0;
	if (x0 < GUI.ClipRect.x0) {
		Diff = GUI.ClipRect.x0 - x0;
		xsize -= Diff;
		switch (BitsPerPixel) {
			case 1:
				pPixel += (Diff >> 3); x0 += (Diff >> 3) << 3; Diff &= 7;
				break;
			case 2:
				pPixel += (Diff >> 2); x0 += (Diff >> 2) << 2; Diff &= 3;
				break;
			case 4:
				pPixel += (Diff >> 1); x0 += (Diff >> 1) << 1; Diff &= 1;
				break;
			case 8:
				pPixel += Diff;      x0 += Diff; Diff = 0;
				break;
			case 16:
				pPixel += (Diff << 1); x0 += Diff; Diff = 0;
				break;
			case 24:
				pPixel += (Diff << 2); x0 += Diff; Diff = 0;  /* 24-bit uses 4 bytes per pixel */
				break;
		}
	}
	if (xsize <= 0) {
		return;
	}
	LCDDEV_L0_DrawBitmap(x0, y0, xsize, ysize, BitsPerPixel, BytesPerLine, pPixel, Diff, pTrans);
}
