#include <stdio.h>
#include <windows.h>

#include "GUI.h"
#include "GUI_X.h"
#include "GUITouchConf.h" 

/*********************************************************************
*
*       Global data
*/
volatile int OS_TimeMS;

/*********************************************************************
*
*      Timing:
*                 GUI_X_GetTime()
*                 GUI_X_Delay(int)

  Some timing dependent routines require a GetTime
  and delay function. Default time unit (tick), normally is
  1 ms.
*/

int GUI_X_GetTime(void) {
	return (int)GetTickCount();
}

void GUI_X_Delay(int ms) {
	int tEnd = GUI_X_GetTime() + ms;
	while ((tEnd - GUI_X_GetTime()) > 0);
}

/*********************************************************************
*
*       GUI_X_Init()
*
* Note:
*     GUI_X_Init() is called from GUI_Init is a possibility to init
*     some hardware which needs to be up and running before the GUI.
*     If not required, leave this routine blank.
*/

void GUI_X_Init(void) {}

/*********************************************************************
*
*       GUI_X_ExecIdle
*
* Note:
*  Called if WM is in idle state
*/

void GUI_X_ExecIdle(void) {}

/*********************************************************************
*
*      Logging: OS dependent

Note:
  Logging is used in higher debug levels only. The typical target
  build does not use logging and does therefor not require any of
  the logging routines below. For a release build without logging
  the routines below may be eliminated to save some space.
  (If the linker is not function aware and eliminates unreferenced
  functions automatically)

*/


void GUI_X_Log(const char *s) {
	printf("LOG: %s", s);
}

void GUI_X_Warn(const char *s) {
	printf("WARN: %s", s);
}

void GUI_X_ErrorOut(const char *s) {
	printf("ERR: %s", s);
}

void GUI_TOUCH_X_ActivateX(void) {}

void GUI_TOUCH_X_ActivateY(void) {}

int  GUI_TOUCH_X_MeasureX(void) {
	return 0;
}

int  GUI_TOUCH_X_MeasureY(void) {
	return 0;
}
