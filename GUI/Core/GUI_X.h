#pragma once

#include "GUI.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/**** Init ****/
void GUI_X_Init(void);

/**** ExecIdle - called if nothing else is left to do ****/
void GUI_X_ExecIdle(void);

/**** Timing routines - required for blinking ****/
int  GUI_X_GetTime(void);
void GUI_X_Delay(int Period);

/****      Event driving (optional with multitasking)  ****/
void GUI_X_WaitEvent(void);
void GUI_X_SignalEvent(void);
/**** Recording (logs/warnings and errors) - required only for higher levels ****/
void GUI_X_Log(const char *s);
void GUI_X_Warn(const char *s);
void GUI_X_ErrorOut(const char *s); 

#if defined(__cplusplus)
  }
#endif
