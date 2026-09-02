#pragma once

#include "GUIConf.h"

/* Define const storage. Normally, this is not needed (define will expand to const)
   However, on some systems (AVR - IAR compiler) it can be necessary ( -> __flash const),
   since otherwise constants are copied into RAM
*/

#ifndef GUI_SUPPORT_DEVICES
#define GUI_SUPPORT_DEVICES (GUI_SUPPORT_MEMDEV)
#endif

#ifndef GUI_SUPPORT_TIMER
#define GUI_SUPPORT_TIMER 1
#endif

#define GUI_DEFAULT_FONT    &FontProp13_1
#define GUI_DEFAULT_CURSOR  &CursorArrowM
#define GUI_DEFAULT_BKCOLOR RGB_BLACK
#define GUI_DEFAULT_COLOR   RGB_WHITE
