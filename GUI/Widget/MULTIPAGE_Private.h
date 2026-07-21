#pragma once

#include "GUIDebug.h"
#include "MULTIPAGE.h"
#include "WIDGET.h"
#include "GUI_ARRAY.h"

#define MULTIPAGE_STATE_ENABLED     (1<<0)
#define MULTIPAGE_STATE_SCROLLMODE  WIDGET_STATE_USER0
#define MULTIPAGE_NUMCOLORS         2

typedef struct {
	WM_Obj * hWin;
	uint8_t      Status;
	char    acText;
} MULTIPAGE_PAGE;

struct MULTIPAGE_Obj : public WIDGET {
	struct Properties {
		PCFONT Font{ &GUI_Font13_1 };
		RGBC aBkColor[MULTIPAGE_NUMCOLORS]{
			/* Disabled page */	RGB_GRAYL(0xD0),
			/* Enabled page */	RGB_GRAYL(0xC0)
		};
		RGBC aTextColor[MULTIPAGE_NUMCOLORS]{
			/* Disabled page */	RGB_GRAYL(0x80),
			/* Enabled page */	RGB_BLACK
		};
		unsigned Align{ MULTIPAGE_ALIGN_LEFT | MULTIPAGE_ALIGN_TOP };
	} static DefaultProps;
	Properties Props;
	WM_Obj *pClient;
	GUI_ARRAY Handles;
	unsigned Selection;
	int ScrollState;
};
