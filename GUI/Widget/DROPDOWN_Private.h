#pragma once

#include "DROPDOWN.h"
#include "WIDGET.h"
#include "GUI_ARRAY.h"

#define DROPDOWN_SF_AUTOSCROLLBAR DROPDOWN_CF_AUTOSCROLLBAR


struct DROPDOWN_Obj : public WIDGET {
	struct Properties {
		const GUI_FONT *pFont{ &GUI_Font13_1 };
		RGB_COLOR aBkColor[4]{
			/* Unselect */			RGB_WHITE,
			/* Selected */			RGB_GRAY,
			/* Selected focussed */	RGB_DARKBLUE,
			/* Disabled */			RGB_GRAYL(0xC0)
		};
		RGB_COLOR aTextColor[4]{
			/* Unselect */			RGB_BLACK,
			/* Selected */			RGB_WHITE,
			/* Selected focussed */	RGB_WHITE,
			/* Disabled */			RGB_GRAY
		};
		int16_t TextBorderSize{ 2 };
		int16_t Align{ TEXTALIGN_LEFT };
	} static DefaultProps;
	Properties Props;
	int16_t     Sel;                        /* current selection */
	int16_t     ySizeEx;                    /* Drop down size */
	int16_t     TextHeight;
	GUI_ARRAY Handles;
	WM_SCROLL_STATE ScrollState;
	WM_HWIN hListWin;
	uint8_t      Flags;
	uint16_t     ItemSpacing;
	uint8_t     ScrollbarWidth;
	char  IsPressed;
};
