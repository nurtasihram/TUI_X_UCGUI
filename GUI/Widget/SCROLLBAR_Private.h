#pragma once

#include "SCROLLBAR.h"
#include "WIDGET.h"
#include "GUIDebug.h"

extern RGB_COLOR SCROLLBAR__aDefaultBkColor[2];
extern RGB_COLOR SCROLLBAR__aDefaultColor[2];
extern int16_t SCROLLBAR__DefaultWidth;

typedef struct {
	WIDGET Widget;
	RGB_COLOR aBkColor[2];
	RGB_COLOR aColor[2];
	int16_t NumItems, v, PageSize;
} SCROLLBAR_Obj;

typedef struct {
	int16_t x0_LeftArrow;
	int16_t x1_LeftArrow;
	int16_t x0_Thumb;
	int16_t x1_Thumb;
	int16_t x0_RightArrow;
	int16_t x1_RightArrow;
	int16_t x1;
	int16_t xSizeMoveable;
	int16_t ThumbSize;
} SCROLLBAR_POSITIONS;

void SCROLLBAR__InvalidatePartner(SCROLLBAR_Handle hObj);
