#pragma once

#include "GUIConf.h"

/* Define const storage. Normally, this is not needed (define will expand to const)
   However, on some systems (AVR - IAR compiler) it can be necessary ( -> __flash const),
   since otherwise constants are copied into RAM
*/
#ifndef GUI_SUPPORT_MOUSE
#define GUI_SUPPORT_MOUSE   0
#endif

#ifndef GUI_SUPPORT_MEMDEV
#define GUI_SUPPORT_MEMDEV  0
#endif

#ifndef GUI_SUPPORT_CURSOR
#define GUI_SUPPORT_CURSOR  0
#endif

#ifndef GUI_SUPPORT_DEVICES
#define GUI_SUPPORT_DEVICES (GUI_SUPPORT_MEMDEV)
#endif

/* In order to avoid warnings for undefined parameters */
#ifndef GUI_USE_PARA
#define GUI_USE_PARA(para) (void)para;
#endif

/* Default for types */
#ifndef GUI_TIMER_TIME
#define GUI_TIMER_TIME int  /* default is to use 16 bits for 16 bit CPUs,
	                           32 bits on 32 bit CPUs for timing */
#endif

#ifndef GUI_MEMSET
#define GUI_MEMSET GUI__memset
#endif
