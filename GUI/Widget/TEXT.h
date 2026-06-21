#pragma once
#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */
#include "WIDGET.h"      /* Req. for Create indirect data structure */
#include "GUIDebug.h" /* Req. for GUI_DEBUG_LEVEL */
#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
#define TEXT_CF_LEFT    GUI_TA_LEFT
#define TEXT_CF_RIGHT   GUI_TA_RIGHT
#define TEXT_CF_HCENTER GUI_TA_HCENTER
#define TEXT_CF_VCENTER GUI_TA_VCENTER
#define TEXT_CF_TOP     GUI_TA_TOP
#define TEXT_CF_BOTTOM  GUI_TA_BOTTOM
/*********************************************************************
*
*       Public Types
*
**********************************************************************
*/
typedef WM_HMEM TEXT_Handle;
#define TEXT_EnableMemdev(hObj)  WM_EnableMemdev(hObj)
#define TEXT_DisableMemdev(hObj) WM_DisableMemdev(hObj)
#define TEXT_Delete(hObj)        WM_DeleteWindow(hObj)
#define TEXT_Paint(hObj)         WM_Paint(hObj)
#define TEXT_Invalidate(hObj)    WM_InvalidateWindow(hObj)
TEXT_Handle TEXT_Create        (int x0, int y0, int xsize, int ysize, int Id, int Flags, const char * s, int Align);
TEXT_Handle TEXT_CreateAsChild (int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, const char * s, int Align);
TEXT_Handle TEXT_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK* cb);
TEXT_Handle TEXT_CreateEx      (int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                int WinFlags, int ExFlags, int Id, const char* pText);
/* Methods changing properties */
void TEXT_SetBkColor  (TEXT_Handle pObj, RGB_COLOR Color); /* Obsolete. Left in GUI for compatibility to older versions */
void TEXT_SetFont     (TEXT_Handle pObj, const GUI_FONT GUI_UNI_PTR * pFont);
void TEXT_SetText     (TEXT_Handle pObj, const char* s);
void TEXT_SetTextAlign(TEXT_Handle pObj, int Align);
void TEXT_SetTextColor(TEXT_Handle pObj, RGB_COLOR Color);
void            TEXT_SetDefaultFont(const GUI_FONT GUI_UNI_PTR * pFont);
const GUI_FONT GUI_UNI_PTR * TEXT_GetDefaultFont(void);
void            TEXT_SetDefaultTextColor(RGB_COLOR Color);
typedef struct {
  WIDGET Widget;
  WM_HMEM hpText;
  const GUI_FONT GUI_UNI_PTR * pFont;
  int16_t Align;
  RGB_COLOR TextColor;
  RGB_COLOR BkColor;
} TEXT_Obj;
#if defined(__cplusplus)
  }
#endif
