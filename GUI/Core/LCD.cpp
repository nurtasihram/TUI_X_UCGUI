#include "GUI_Private.h"
		
#define RETURN_IF_Y_OUT() \
  if (y < GUI.rClip.y0) return;             \
  if (y > GUI.rClip.y1) return;
#define RETURN_IF_X_OUT() \
  if (x < GUI.rClip.x0) return;             \
  if (x > GUI.rClip.x1) return;
#define CLIP_X() \
  if (x0 < GUI.rClip.x0) { x0 = GUI.rClip.x0; } \
  if (x1 > GUI.rClip.x1) { x1 = GUI.rClip.x1; }
#define CLIP_Y() \
  if (y0 < GUI.rClip.y0) { y0 = GUI.rClip.y0; } \
  if (y1 > GUI.rClip.y1) { y1 = GUI.rClip.y1; }
#define RETURN_IF_Y_OUT() \
  if (y < GUI.rClip.y0) return;             \
  if (y > GUI.rClip.y1) return;
#define RETURN_IF_X_OUT() \
  if (x < GUI.rClip.x0) return;             \
  if (x > GUI.rClip.x1) return;

void LCD_SetPixel(int x, int y, int ColorIndex) {
	RETURN_IF_X_OUT();
	RETURN_IF_Y_OUT();
	LCDDEV_L0_SetPixel(x, y, ColorIndex);
}

RGBC LCD_GetPixel(int x, int y) {
	auto r = LCDDEV_L0_GetRect();
	if (x < r.x0) return 0;
	if (x > r.x1) return 0;
	if (y < r.y0) return 0;
	if (y > r.y1) return 0;
	return LCDDEV_L0_GetPixel(x, y);
}

void LCD_FillRect(RECT r) {
	if (!(r &= GUI.rClip))
		return;
	LCDDEV_L0_FillRect(r);
}

void LCD_DrawBitmap(RECT r, int BitsPerPixel, int BytesPerLine, const uint8_t *pPix, PCLOGPALETTE pTrans) {
	auto ptOld = r.LeftTop();
	if (!(r &= GUI.rClip))
		return;
	auto Off = r.LeftTop() - ptOld;
	uint8_t DiffBits = 0;
	if (Off.x > 0) {
		auto xOffBits = BitsPerPixel * Off.x;
		DiffBits = xOffBits & 7;
		pPix += xOffBits >> 3;
	}
	if (Off.y > 0)
		pPix += BytesPerLine * Off.y;
	LCDDEV_L0_DrawBitmap(r.x0, r.y0, r.XSize(), r.YSize(), BitsPerPixel, BytesPerLine, pPix, DiffBits, pTrans);
}

void LCD_DrawBitmap(int x0, int y0, int xsize, int ysize,
					int BitsPerPixel, int BytesPerLine,
					const void *pPix, PCLOGPALETTE pTrans) {
	LCD_DrawBitmap(RECT::LeftTop({ x0, y0 }, { xsize, ysize }), BitsPerPixel, BytesPerLine, (const uint8_t *)pPix, pTrans);
}
