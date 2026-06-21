#pragma once

#include "WM_Intern.h"

#ifdef WM_C
#define GUI_EXTERN
#else
#define GUI_EXTERN extern
#endif

GUI_EXTERN WM_HWIN   WM__ahDesktopWin;   /* Desktop window handle */
GUI_EXTERN RGB_COLOR WM__aBkColor;       /* Desktop background color */

#undef GUI_EXTERN
