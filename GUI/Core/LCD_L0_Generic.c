#include "LCD_Private.h"      /* private modul definitions & config */
#include "GUI_Private.h"
#include "GUIDebug.h"

void LCD_L0_GetRect(LCD_RECT *pRect) {
	pRect->x0 = 0;
	pRect->y0 = 0;
	pRect->x1 = LCD_XSIZE - 1;
	pRect->y1 = LCD_YSIZE - 1;
}
