#pragma once

#include "WM.h"
#include "GUI_Protected.h"       /* For GUI */

PID_STATE WM_PID__GetPrevState        (void);
WM_PARAM WM__SendMessage             (WObj * pWin, int MsgId, WM_PARAM Data);
