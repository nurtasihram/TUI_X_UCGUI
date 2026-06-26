#include <stdio.h>
#include "GUI_Private.h"
#include "LCD_Private.h"
#include "GUIDebug.h"

void LCD_SetClipRectMax(void) {
	LCDDEV_L0_GetRect(&GUI_Context.ClipRect);
}
void LCD_L0_GetRect(GUI_RECT *pRect) {
	pRect->x0 = 0;
	pRect->y0 = 0;
	pRect->x1 = LCD_GetXSize() - 1;
	pRect->y1 = LCD_GetYSize() - 1;
}
void LCD_SetClipRectEx(const GUI_RECT *pRect) {
	GUI_RECT r;
	LCDDEV_L0_GetRect(&r);
	GUI__IntersectRects(&GUI_Context.ClipRect, pRect, &r);
}

int LCD_Init(void) {
	LCD_SetClipRectMax();
	LCD_L0_Init();
	LCD_SetDrawMode(GUI_DRAWMODE_REV);
	LCD_FillRect(0, 0, GUI_XMAX, GUI_YMAX);
	LCD_SetDrawMode(0);
	return 0;
}

static int _GetColorIndex(int i)  /* i is 0 or 1 */ {
	return  (GUI_Context.DrawMode & LCD_DRAWMODE_REV) ? i - 1 : i;
}

void GUI_SetBkColor(RGB_COLOR color) {
	LCD_ACOLORINDEX[_GetColorIndex(0)] = color;
}

void GUI_SetColor(RGB_COLOR color) {
	LCD_ACOLORINDEX[_GetColorIndex(1)] = color;
}

RGB_COLOR GUI_GetBkColor(void) {
	return LCD_BKCOLORINDEX;
}
RGB_COLOR GUI_GetColor(void) {
	return LCD_COLORINDEX;
}

GUI_DRAWMODE LCD_SetDrawMode(GUI_DRAWMODE dm) {
	GUI_DRAWMODE OldDM = GUI_Context.DrawMode;
	if ((GUI_Context.DrawMode ^ dm) & LCD_DRAWMODE_REV) {
		RGB_COLOR temp = LCD_BKCOLORINDEX;
		LCD_BKCOLORINDEX = LCD_COLORINDEX;
		LCD_COLORINDEX = temp;
	}
	GUI_Context.DrawMode = dm;
	return OldDM;
}

#define RETURN_IF_Y_OUT() \
  if (y < GUI_Context.ClipRect.y0) return;             \
  if (y > GUI_Context.ClipRect.y1) return;
#define RETURN_IF_X_OUT() \
  if (x < GUI_Context.ClipRect.x0) return;             \
  if (x > GUI_Context.ClipRect.x1) return;
#define CLIP_X() \
  if (x0 < GUI_Context.ClipRect.x0) { x0 = GUI_Context.ClipRect.x0; } \
  if (x1 > GUI_Context.ClipRect.x1) { x1 = GUI_Context.ClipRect.x1; }
#define CLIP_Y() \
  if (y0 < GUI_Context.ClipRect.y0) { y0 = GUI_Context.ClipRect.y0; } \
  if (y1 > GUI_Context.ClipRect.y1) { y1 = GUI_Context.ClipRect.y1; }
#define RETURN_IF_Y_OUT() \
  if (y < GUI_Context.ClipRect.y0) return;             \
  if (y > GUI_Context.ClipRect.y1) return;
#define RETURN_IF_X_OUT() \
  if (x < GUI_Context.ClipRect.x0) return;             \
  if (x > GUI_Context.ClipRect.x1) return;

void LCD_SetPixel(int x, int y, int ColorIndex) {
	RETURN_IF_X_OUT();
	RETURN_IF_Y_OUT();
	LCDDEV_L0_SetPixelIndex(x, y, ColorIndex);
}

RGB_COLOR LCD_GetPixel(int x, int y) {
	GUI_RECT r;
	LCDDEV_L0_GetRect(&r);
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
	return LCDDEV_L0_GetPixelIndex(x, y);
}

void LCD_DrawPixel(int x, int y) {
	RETURN_IF_Y_OUT();
	RETURN_IF_X_OUT();
	if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
		LCDDEV_L0_XorPixel(x, y);
	}
	else {
		LCDDEV_L0_SetPixelIndex(x, y, LCD_COLORINDEX);
	}
}

void LCD_DrawVLine(int x, int y0, int y1) {
	/* Perform clipping and check if there is something to do */
	RETURN_IF_X_OUT();
	CLIP_Y();
	if (y1 < y0) {
		return;
	}
	/* Call driver to draw */
	LCDDEV_L0_DrawVLine(x, y0, y1);
}
void LCD_DrawHLine(int x0, int y, int x1) {
	/* Perform clipping and check if there is something to do */
	RETURN_IF_Y_OUT();
	CLIP_X();
	if (x1 < x0)
		return;
	/* Call driver to draw */
	LCDDEV_L0_DrawHLine(x0, y, x1);
}

void LCD_FillRect(int x0, int y0, int x1, int y1) {
	/* Perform clipping and check if there is something to do */
	CLIP_X();
	if (x1 < x0)
		return;
	CLIP_Y();
	if (y1 < y0)
		return;
	/* Call driver to draw */
	LCDDEV_L0_FillRect(x0, y0, x1, y1);
}

void LCD_DrawBitmap(int x0, int y0, int xsize, int ysize,
					int BitsPerPixel, int BytesPerLine,
					const uint8_t  *pPixel, const RGB_COLOR *pTrans) {
	uint8_t  Data = 0;
	int x1, y1, Diff;
	/* Handle rotation if necessary */
	/* Handle the optional Y-magnification */
	y1 = y0 + ysize - 1;
	x1 = x0 + xsize - 1;
	/*  Clip y0 (top) */
	Diff = GUI_Context.ClipRect.y0 - y0;
	if (Diff > 0) {
		ysize -= Diff;
		if (ysize <= 0) {
			return;
		}
		y0 = GUI_Context.ClipRect.y0;
		pPixel += (unsigned)Diff * (unsigned)BytesPerLine;
	}
	/*  Clip y1 (bottom) */
	Diff = y1 - GUI_Context.ClipRect.y1;
	if (Diff > 0) {
		ysize -= Diff;
		if (ysize <= 0) {
			return;
		}
	}
	/*        Clip right side    */
	Diff = x1 - GUI_Context.ClipRect.x1;
	if (Diff > 0) {
		xsize -= Diff;
	}
	/*        Clip left side ... (The difficult side ...)    */
	Diff = 0;
	if (x0 < GUI_Context.ClipRect.x0) {
		Diff = GUI_Context.ClipRect.x0 - x0;
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
		}
	}
	if (xsize <= 0) {
		return;
	}
	LCDDEV_L0_DrawBitmap(x0, y0, xsize, ysize, BitsPerPixel, BytesPerLine, pPixel, Diff, pTrans);
}
