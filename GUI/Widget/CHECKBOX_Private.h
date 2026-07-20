#pragma once

#include "WM.h"
#include "WIDGET.h"
#include "CHECKBOX.h"

struct CHECKBOX_Obj : public WIDGET {
	static const GUI_BITMAP abmCheck[2];
	struct Properties {
		const GUI_FONT *pFont{ &GUI_Font13_1 };
		const GUI_BITMAP *apBm[4]{
			/* Inactive */	&abmCheck[0],
			/* Active */	&abmCheck[1],
			/* Inactive 3-State */	&abmCheck[2],
			/* Active 3-State */	&abmCheck[1]
		};
		RGB_COLOR aBkColorBox[2]{
			/* Inactive */	RGB_GRAYL(0x80),
			/* Active */	RGB_WHITE
		};
		RGB_COLOR BkColor{ RGB_INVALID_COLOR };
		RGB_COLOR TextColor{ RGB_BLACK };
		TEXTALIGN Align{ TEXTALIGN_LEFT | TEXTALIGN_VCENTER };
		uint8_t Spacing{ 4 };
		uint8_t NumStates = 2;
	} static DefaultProps;
	Properties Props;
	uint8_t NumStates;
	uint8_t CurrentState;
	char *pText;
};
