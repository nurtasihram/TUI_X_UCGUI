module;

#include "LCD.h"

export module GUI_LCD;

export import TUX;

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

export {

void LCD_SetPixel(int x, int y, int ColorIndex) {
	RETURN_IF_X_OUT();
	RETURN_IF_Y_OUT();
	GUI.pDeviceAPI->SetPixel(x, y, ColorIndex);
}

void LCD_FillRect(RECT r) {
	if (!(r &= GUI.rClip))
		return;
	GUI.pDeviceAPI->FillRect(r);
}
void LCD_DrawBitmap(BITVIEW b) {
	if (!(b &= GUI.rClip))
		return;
	GUI.pDeviceAPI->DrawBitmap(b);
}


}