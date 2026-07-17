#pragma once

#include "GUI.h"

/**** Init ****/
void GUI_X_Init(void);

/**** Timing routines - required for blinking ****/
int  GUI_X_GetTime(void);
void GUI_X_Delay(int Period);

/**** Recording (logs/warnings and errors) - required only for higher levels ****/
void GUI_X_Log(const char *s, ...);
void GUI_X_Warn(const char *s, ...);
void GUI_X_ErrorOut(const char *s, ...);
