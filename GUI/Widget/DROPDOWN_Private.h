#pragma once

#include "DROPDOWN.h"
#include "WIDGET.h"
#include "GUI_ARRAY.h"

#define DROPDOWN_SF_AUTOSCROLLBAR DROPDOWN_CF_AUTOSCROLLBAR

typedef struct {
	const GUI_FONT  *pFont;
	RGB_COLOR aBackColor[3];
	RGB_COLOR aTextColor[3];
	int16_t       TextBorderSize;
	int16_t       Align;
} DROPDOWN_PROPS;

struct DROPDOWN_Obj : public WIDGET {
	int16_t     Sel;                        /* current selection */
	int16_t     ySizeEx;                    /* Drop down size */
	int16_t     TextHeight;
	GUI_ARRAY Handles;
	WM_SCROLL_STATE ScrollState;
	DROPDOWN_PROPS Props;
	WM_HWIN hListWin;
	uint8_t      Flags;
	uint16_t     ItemSpacing;
	uint8_t     ScrollbarWidth;
	char  IsPressed;
};
