#pragma once

#include "GUIDebug.h"
#include "MULTIPAGE.h"

#define MULTIPAGE_STATE_ENABLED     (1<<0)
#define MULTIPAGE_STATE_SCROLLMODE  WIDGET_STATE_USER0

extern const GUI_FONT *MULTIPAGE__pDefaultFont;
extern unsigned MULTIPAGE__DefaultAlign;
extern RGB_COLOR MULTIPAGE__DefaultBkColor[2];
extern RGB_COLOR MULTIPAGE__DefaultTextColor[2];
