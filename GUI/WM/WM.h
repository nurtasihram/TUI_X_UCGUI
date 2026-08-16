#pragma once

#include "GUI_ConfDefaults.h"

import TUX;
import TUX.Window;

/* Make sure we actually have configured windows. If we have not,
there is no point for a windows manager and it will therefor not
generate any code !
*/

#ifndef WM_SUPPORT_OBSTRUCT
#define WM_SUPPORT_OBSTRUCT 1
#endif

#define WM_UNATTACHED  ((WObj *)-1) /* Do not attach to a window */
