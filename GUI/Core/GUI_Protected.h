#pragma once

#include "GUI.h"
#include "LCD_Protected.h"
#include "GUIDebug.h"

#define GUI_DEFAULT_FONT    &FontProp13_1
#define GUI_DEFAULT_CURSOR  &CursorArrowM
#define GUI_DEFAULT_BKCOLOR RGB_BLACK
#define GUI_DEFAULT_COLOR   RGB_WHITE

typedef enum { WRAPMODE_NONE, WRAPMODE_WORD, WRAPMODE_CHAR } WRAPMODE;

int  GUI__GetLineSizeX(const char *s, int Len);
void GUI__CalcTextRect(const char *pText, const RECT *pTextRectIn, RECT *pTextRectOut, int TextAlign);

int GUI__WrapGetNumCharsDisp(const char *pText, int xSize, WRAPMODE WrapMode);
int GUI__WrapGetNumCharsToNextLine(const char *pText, int xSize, WRAPMODE WrapMode);
int GUI__WrapGetNumBytesToNextLine(const char *pText, int xSize, WRAPMODE WrapMode);

void GUI__DispLine(const char *s, int Len, const RECT *pr);

int GUI_GetBitmapPixel(PCBITMAP pBMP, unsigned x, unsigned y);
