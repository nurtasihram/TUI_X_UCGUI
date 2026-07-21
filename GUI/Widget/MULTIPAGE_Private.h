#pragma once

#include "GUIDebug.h"
#include "MULTIPAGE.h"

#define MULTIPAGE_STATE_ENABLED     (1<<0)
#define MULTIPAGE_STATE_SCROLLMODE  WIDGET_STATE_USER0

extern PCFONT MULTIPAGE__pDefaultFont;
extern unsigned MULTIPAGE__DefaultAlign;
extern RGBC MULTIPAGE__DefaultBkColor[2];
extern RGBC MULTIPAGE__DefaultTextColor[2];
