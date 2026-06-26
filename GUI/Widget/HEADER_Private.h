#pragma once

#include "WIDGET.h"
#include "HEADER.h"
#include "WM.h"
#include "GUI_ARRAY.h"

typedef struct {
  int Width;
  int16_t Align;
  WM_HMEM hDrawObj;
  char acText[1];
} HEADER_COLUMN;
typedef struct {
  WIDGET Widget;
  RGB_COLOR BkColor;
  RGB_COLOR TextColor;
  GUI_ARRAY Columns;
  int CapturePosX;
  int CaptureItem;
  int ScrollPos;
  const GUI_FONT  * pFont;
} HEADER_Obj;
void HEADER__SetDrawObj(HEADER_Handle hObj, unsigned Index, GUI_DRAW_HANDLE hDrawObj);
