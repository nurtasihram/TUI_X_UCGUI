#pragma once

#include "WM.h"
#include "DIALOG_Intern.h" /* Req. for Create indirect data structure */
#include "WIDGET.h"        /* Req. for Create indirect data structure */
#include "GUIDebug.h"      /* Req. for GUI_DEBUG_LEVEL */

#define TEXT_CF_LEFT    TEXTALIGN_LEFT
#define TEXT_CF_RIGHT   TEXTALIGN_RIGHT
#define TEXT_CF_HCENTER TEXTALIGN_HCENTER
#define TEXT_CF_VCENTER TEXTALIGN_VCENTER
#define TEXT_CF_TOP     TEXTALIGN_TOP
#define TEXT_CF_BOTTOM  TEXTALIGN_BOTTOM
/*********************************************************************
*
*       Public Types
*
**********************************************************************
*/
typedef WM_Obj * TEXT_Handle;

TEXT_Handle TEXT_Create        (int x0, int y0, int xsize, int ysize, int Id, int Flags, const char * s, int Align);
TEXT_Handle TEXT_CreateAsChild (int x0, int y0, int xsize, int ysize, WM_Obj * hParent, int Id, int Flags, const char * s, int Align);
TEXT_Handle TEXT_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK* cb);
TEXT_Handle TEXT_CreateEx      (int x0, int y0, int xsize, int ysize, WM_Obj * hParent,
                                int WinFlags, int ExFlags, int Id, const char* pText);
/* Methods changing properties */
void TEXT_SetBkColor  (TEXT_Handle pObj, RGBC Color); /* Obsolete. Left in GUI for compatibility to older versions */
void TEXT_SetFont     (TEXT_Handle pObj, PCFONT pFont);
void TEXT_SetText     (TEXT_Handle pObj, const char* s);
void TEXT_SetTextAlign(TEXT_Handle pObj, int Align);
void TEXT_SetTextColor(TEXT_Handle pObj, RGBC Color);
struct TEXT_Obj : public WIDGET {
  char *pText;
  PCFONT pFont;
  TEXTALIGN Align;
  RGBC TextColor;
  RGBC BkColor;
};
