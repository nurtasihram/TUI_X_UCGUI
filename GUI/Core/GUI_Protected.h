#pragma once

#include "GUI.h"
#include "LCD_Protected.h"
#include "GUIDebug.h"

#define GUI_DEFAULT_FONT    &FontProp13_1
#define GUI_DEFAULT_CURSOR  &CursorArrowM
#define GUI_DEFAULT_BKCOLOR RGB_BLACK
#define GUI_DEFAULT_COLOR   RGB_WHITE

typedef enum { WRAPMODE_NONE, WRAPMODE_WORD, WRAPMODE_CHAR } WRAPMODE;

int  GUI__GetLineNumChars(const char *s, int MaxNumChars);
int  GUI__GetLineSizeX(const char *s, int Len);
int  GUI__HandleEOLine(const char **ps);
void GUI__DispLine(const char *s, int Len, const RECT *pr);
void GUI__AddSpaceHex(uint32_t v, uint8_t Len, char **ps);
void GUI__CalcTextRect(const char *pText, const RECT *pTextRectIn, RECT *pTextRectOut, int TextAlign);

int GUI__WrapGetNumCharsDisp(const char *pText, int xSize, WRAPMODE WrapMode);
int GUI__WrapGetNumCharsToNextLine(const char *pText, int xSize, WRAPMODE WrapMode);
int GUI__WrapGetNumBytesToNextLine(const char *pText, int xSize, WRAPMODE WrapMode);

bool GUI__SetText(char **ppText, const char *s);
bool GUI__strcmp(const char *s0, const char *s1);
int GUI__strlen(const char *s);
int GUI__strcpy(char *sDest, const char *sSrc);

void GL_DispChar(uint16_t c);

int GUI_GetBitmapPixel(PCBITMAP pBMP, unsigned x, unsigned y);
