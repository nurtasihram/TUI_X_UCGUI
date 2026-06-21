#pragma once
#include "WM.h"
#include "WIDGET.h"
#include "CHECKBOX.h"
#ifndef CHECKBOX_BKCOLOR0_DEFAULT
#define CHECKBOX_BKCOLOR0_DEFAULT 0x808080           /* Inactive color */
#endif
#ifndef CHECKBOX_BKCOLOR1_DEFAULT
#define CHECKBOX_BKCOLOR1_DEFAULT GUI_WHITE          /* Active color */
#endif
#ifndef CHECKBOX_FGCOLOR0_DEFAULT
#define CHECKBOX_FGCOLOR0_DEFAULT 0x101010
#endif
#ifndef CHECKBOX_FGCOLOR1_DEFAULT
#define CHECKBOX_FGCOLOR1_DEFAULT GUI_BLACK
#endif
typedef struct {
	const GUI_FONT GUI_UNI_PTR *pFont;
	RGB_COLOR aBkColorBox[2]; /* Colors used to draw the box background */
	RGB_COLOR BkColor;        /* Widget background color */
	RGB_COLOR TextColor;
	int16_t Align;
	uint8_t  Spacing;
	const GUI_BITMAP *apBm[4];
} CHECKBOX_PROPS;
typedef struct {
	WIDGET Widget;
	CHECKBOX_PROPS Props;
	uint8_t NumStates;
	uint8_t CurrentState;
	WM_HMEM hpText;
} CHECKBOX_Obj;
extern CHECKBOX_PROPS CHECKBOX__DefaultProps;
extern const GUI_BITMAP   CHECKBOX__abmCheck[2];
