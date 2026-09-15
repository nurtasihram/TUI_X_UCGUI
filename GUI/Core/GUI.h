#pragma once

#include "GUI_X.h"
#include "GUIDebug.h"
#include "GUIConf.h"

import TUX;
import TUX.Types;
import TUX.Resources;
import TUX.LCD;

#define GUI_COUNTOF(a) (sizeof(a) / sizeof(a[0]))

/* Define minimum and maximum coordinates in x and y */
#define GUI_XMIN -4095
#define GUI_XMAX  4095
#define GUI_YMIN -4095
#define GUI_YMAX  4095

void GUI_Init(void);
void GUI_SaveContext(GUI_CONTEXT *pContext);
void GUI_RestoreContext(const GUI_CONTEXT *pContext);

int  GUI__DivideRound(int a, int b);

void GUI_Clear(void);
void GUI_ClearRect(RECT r);
void GUI_DrawFocusRect(RECT r, int Dist);
void GUI_DrawRect(RECT r);
void GUI_FillRect(RECT r);

void GUI_DrawHLine(int y0, int x0, int x1);
void GUI_DrawVLine(int x0, int y0, int y1);

void GUI_DrawBitmap(PCBITMAP pBM, POINT Pos);

#if GUI_SUPPORT_CURSOR
void     GUI_CURSOR_Activate(void);
void     GUI_CURSOR_Deactivate(void);
void     GUI_CURSOR_SetPosition(int x, int y);
PCCURSOR GUI_CURSOR_Select(PCCURSOR pCursor);
void     GUI_CURSOR_Show(void);
void     GUI_CURSOR_Hide(void);
void     GUI_CURSOR__TempShow(void);
bool     GUI_CURSOR__TempHide(RECT);
#endif

void  GUI_DispChar(uint16_t c);
void  GUI_DispString(const char *s);
void  GUI_DispStringAt(const char *s, int x, int y);
void  GUI_DispStringInRect(const char *s, const RECT &r, int Flags);
void  GUI_DispStringInRectMax(const char *s, RECT r, int TextAlign, int MaxLen); /* Not to be doc. */
void  GUI_DispNextLine(void);

void GUI_SelectLCD(void);

int  GUI_Exec(void);         /* Execute all jobs ... Return 0 if nothing was done. */
int  GUI_Exec1(void);        /* Execute one job  ... Return 0 if nothing was done. */

/* Message layer */
void GUI_StoreKeyMsg(int Key, int Pressed);
void GUI_SendKeyMsg(int Key, int Pressed);
bool GUI_PollKeyMsg(void);

/* Application layer */
int  GUI_GetKey(void);
int  GUI_WaitKey(void);
void GUI_StoreKey(int c);
void GUI_ClearKeyBuffer(void);

void GUI_PID_StoreState(const PID_STATE &State);
PID_STATE GUI_PID_GetState(void);
