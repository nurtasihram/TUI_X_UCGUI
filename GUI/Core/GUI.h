#pragma once

#include "LCD.h"
#include "GUI_ConfDefaults.h"

#include "GUI_X.h"
#include "GUIDebug.h"

import TUX;
import TUX.Resources;

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

int  GUI_BMP_Draw(const void *pFileData, int x0, int y0);
int  GUI_BMP_GetXSize(const void *pFileData);
int  GUI_BMP_GetYSize(const void *pFileData);

void GUI_Clear(void);
void GUI_ClearRect(RECT r);
void GUI_DrawFocusRect(RECT r, int Dist);
void GUI_DrawRect(RECT r);
void GUI_FillRect(RECT r);

void GUI_DrawBitmap(PCBITMAP pBM, POINT Pos);

void GUI_DrawHLine(int y0, int x0, int x1);
void GUI_DrawVLine(int x0, int y0, int y1);

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
void  GUI__DispStringInRect(const char *s, RECT *pRect, int TextAlign, int MaxNumChars);
void  GUI_DispStringInRect(const char *s, RECT *pRect, int Flags);
void  GUI_DispStringInRectMax(const char *s, RECT *pRect, int TextAlign, int MaxLen); /* Not to be doc. */

void  GUI_GetTextExtend(RECT *pRect, const char *s, int Len);
int   GUI_GetStringSizeX(const char *s);
void  GUI_DispNextLine(void);

#define GUI_MEMDEV_HASTRANS       0
#define GUI_MEMDEV_NOTRANS    (1<<0)

void GUI_SelectLCD(void);

void GUI_Delay(int Period);
int  GUI_GetTime(void);
int  GUI_Exec(void);         /* Execute all jobs ... Return 0 if nothing was done. */
int  GUI_Exec1(void);        /* Execute one job  ... Return 0 if nothing was done. */

/* Message layer */
void GUI_StoreKeyMsg(int Key, int Pressed);
void GUI_SendKeyMsg(int Key, int Pressed);
int  GUI_PollKeyMsg(void);

/* Application layer */
int  GUI_GetKey(void);
int  GUI_WaitKey(void);
void GUI_StoreKey(int c);
void GUI_ClearKeyBuffer(void);

void GUI_PID_StoreState(const PID_STATE &State);
PID_STATE GUI_PID_GetState(void);

int  GUI_MOUSE_GetState(PID_STATE *pState);
void GUI_MOUSE_StoreState(const PID_STATE *pState);

int  GUI_TOUCH_GetState(PID_STATE *pState);
void GUI_TOUCH_GetUnstable(int *px, int *py);  /* for diagnostics only */
void GUI_TOUCH_StoreState(int x, int y);
void GUI_TOUCH_StoreStateEx(const PID_STATE *pState);
void GUI_TOUCH_StoreUnstable(int x, int y);

#if GUI_SUPPORT_DEVICES
#define LCDDEV_L0_DrawBitmap GUI.pDeviceAPI->pfDrawBitmap
#define LCDDEV_L0_FillRect   GUI.pDeviceAPI->pfFillRect
#define LCDDEV_L0_GetPixel   GUI.pDeviceAPI->pfGetPixel
#define LCDDEV_L0_GetRect    GUI.pDeviceAPI->pfGetRect
#define LCDDEV_L0_GetPixel   GUI.pDeviceAPI->pfGetPixel
#define LCDDEV_L0_SetPixel   GUI.pDeviceAPI->pfSetPixel
#endif
