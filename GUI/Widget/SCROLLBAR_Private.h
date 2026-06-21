#pragma once

#include "SCROLLBAR.h"
#include "WIDGET.h"
#include "GUIDebug.h"

extern RGB_COLOR  SCROLLBAR__aDefaultBkColor[2];
extern RGB_COLOR  SCROLLBAR__aDefaultColor[2];
extern int16_t        SCROLLBAR__DefaultWidth;
typedef struct {
  WIDGET Widget;
  RGB_COLOR aBkColor[2];
  RGB_COLOR aColor[2];
  int NumItems, v, PageSize;
} SCROLLBAR_Obj;
typedef struct {
  int x0_LeftArrow;
  int x1_LeftArrow;
  int x0_Thumb;
  int x1_Thumb;
  int x0_RightArrow;
  int x1_RightArrow;
  int x1;
  int xSizeMoveable;
  int ThumbSize;
} SCROLLBAR_POSITIONS;
void SCROLLBAR__InvalidatePartner(SCROLLBAR_Handle hObj);
