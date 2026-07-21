#pragma once

#include "PROGBAR.h"
#include "WIDGET.h"

struct PROGBAR_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &GUI_Font13_1 };
		RGBC aBkColor[2]{
			/* Active */	RGB_DARKBLUE,
			/* Inactive */	RGB_GRAYL(0x55)
		};
		RGBC aTextColor[2]{
			/* Active */	RGB_WHITE,
			/* Inactive */	RGB_BLACK
		};
		TEXTALIGN Align{ TEXTALIGN_HCENTER };
	} static DefaultProps;
	Properties Props;
	char *pText;
	int16_t XOff, YOff;
	int16_t v, Min, Max;
};
