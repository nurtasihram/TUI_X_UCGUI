#pragma once

#include "WM.h"
#include "WIDGET.h"
#include "CHECKBOX.h"

#define CHECKBOX_BKCOLOR0_DEFAULT RGB_GRAYL(0x80)           /* Inactive color */
#define CHECKBOX_BKCOLOR1_DEFAULT RGB_WHITE          /* Active color */
#define CHECKBOX_FGCOLOR0_DEFAULT RGB_GRAYL(0x10)
#define CHECKBOX_FGCOLOR1_DEFAULT RGB_BLACK

typedef struct {
	const GUI_FONT  *pFont;
	RGB_COLOR aBkColorBox[2]; /* Colors used to draw the box background */
	RGB_COLOR BkColor;        /* Widget background color */
	RGB_COLOR TextColor;
	int16_t Align;
	uint8_t Spacing;
	const GUI_BITMAP *apBm[4];
} CHECKBOX_PROPS;

struct CHECKBOX_Obj : public WIDGET {
	CHECKBOX_PROPS Props;
	uint8_t NumStates;
	uint8_t CurrentState;
	char *pText;
};

extern CHECKBOX_PROPS CHECKBOX__DefaultProps;
extern const GUI_BITMAP   CHECKBOX__abmCheck[2];
