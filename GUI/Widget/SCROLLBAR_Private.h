#pragma once

#include "SCROLLBAR.h"
#include "WIDGET.h"
#include "GUIDebug.h"

struct SCROLLBAR_Obj : public WIDGET {
	struct Properties {
		RGB_COLOR aBkColor[2]{
			RGB_GRAYL(0x80),
			RGB_BLACK
		};
		RGB_COLOR Color{ RGB_GRAYL(0xC0) };
	} static DefaultProps;
	static const int16_t DefaultWidth = 12;
	Properties Props;
	WM_SCROLL_STATE ScrollState;
};

struct SCROLLBAR_POSITIONS {
	int16_t x0_LeftArrow = 0,
			x1_LeftArrow = 0;
	int16_t x0_RightArrow = 0,
			x1_RightArrow = 0;
	int16_t x0_Thumb = 0,
			x1_Thumb = 0;
	int16_t ThumbSize = 0;
	int16_t xSizeMoveable = 0;
	int16_t x1 = 0;
};

void SCROLLBAR__InvalidatePartner(SCROLLBAR_Handle hObj);
