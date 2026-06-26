#pragma once

#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defaults for configuration switches
*
**********************************************************************

The following are defaults for config switches which affect the
interface specified in this module
*/




#define EDIT_CF_LEFT    GUI_TA_LEFT
#define EDIT_CF_RIGHT   GUI_TA_RIGHT
#define EDIT_CF_HCENTER GUI_TA_HCENTER

#define EDIT_CF_VCENTER GUI_TA_VCENTER
#define EDIT_CF_TOP     GUI_TA_TOP
#define EDIT_CF_BOTTOM  GUI_TA_BOTTOM


#define EDIT_CI_DISABLED 0
#define EDIT_CI_ENABLED  1


typedef WM_HMEM EDIT_Handle;
typedef void tEDIT_AddKeyEx    (EDIT_Handle hObj, int Key);
typedef void tEDIT_UpdateBuffer(EDIT_Handle hObj);

/*
      *************************************************************
      *                                                           *
      *                 Create function(s)                        *
      *                                                           *
      *                   (Constructors)                          *
      *                                                           *
      *************************************************************

*/
/* Note: the parameters to a create function may vary.
         Some widgets may have multiple create functions */

EDIT_Handle EDIT_Create        (int x0, int y0, int xsize, int ysize, int Id, int MaxLen, int Flags);
EDIT_Handle EDIT_CreateAsChild (int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, int MaxLen);
EDIT_Handle EDIT_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK* cb);
EDIT_Handle EDIT_CreateEx      (int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                int WinFlags, int ExFlags, int Id, int MaxLen);

/*
      *************************************************************
      *                                                           *
      *                 Standard member functions                 *
      *                                                           *
      *************************************************************
*/

#define EDIT_EnableMemdev(hObj)  WM_EnableMemdev(hObj)
#define EDIT_DisableMemdev(hObj) WM_DisableMemdev(hObj)
#define EDIT_Delete(hObj)        WM_DeleteWindow(hObj)
#define EDIT_Paint(hObj)         WM_Paint(hObj)
#define EDIT_Invalidate(hObj)    WM_Invalidate(hObj)

/*
      *************************************************************
      *                                                           *
      *           Individual member functions                     *
      *                                                           *
      *************************************************************

*/

/* Methods changing preferences */
void EDIT_SetDefaultBkColor(unsigned int Index, RGB_COLOR Color);
void EDIT_SetDefaultFont(const GUI_FONT* pFont);
void EDIT_SetDefaultTextAlign(int Align);
void EDIT_SetDefaultTextColor(unsigned int Index, RGB_COLOR Color);

/* Query preferences */
RGB_COLOR                    EDIT_GetDefaultBkColor(unsigned int Index);
const GUI_FONT  * EDIT_GetDefaultFont(void);
int                          EDIT_GetDefaultTextAlign(void);
RGB_COLOR                    EDIT_GetDefaultTextColor(unsigned int Index);

/* Methods changing properties */
void EDIT_AddKey           (EDIT_Handle hObj, int Key);
void EDIT_ClearCursor      (EDIT_Handle hObj);
void EDIT_SetBkColor       (EDIT_Handle hObj, unsigned int Index, RGB_COLOR color);
void EDIT_SetCursorAtChar  (EDIT_Handle hObj, int Pos);
void EDIT_SetCursorAtPixel (EDIT_Handle hObj, int xPos);
void EDIT_SetFont          (EDIT_Handle hObj, const GUI_FONT  * pfont);
int  EDIT_SetInsertMode    (EDIT_Handle hObj, int OnOff);
void EDIT_SetMaxLen        (EDIT_Handle hObj, int MaxLen);
void EDIT_SetpfAddKeyEx    (EDIT_Handle hObj, tEDIT_AddKeyEx * pfAddKeyEx);
void EDIT_SetpfUpdateBuffer(EDIT_Handle hObj, tEDIT_UpdateBuffer * pfUpdateBuffer);
void EDIT_SetText          (EDIT_Handle hObj, const char* s);
void EDIT_SetTextAlign     (EDIT_Handle hObj, int Align);
void EDIT_SetTextColor     (EDIT_Handle hObj, unsigned int Index, RGB_COLOR color);
void EDIT_SetSel           (EDIT_Handle hObj, int FirstChar, int LastChar);

/* Get/Set user input */
float EDIT_GetFloatValue(EDIT_Handle hObj);
void  EDIT_GetText      (EDIT_Handle hObj, char* sDest, int MaxLen);
int32_t   EDIT_GetValue     (EDIT_Handle hObj);
void  EDIT_SetFloatValue(EDIT_Handle hObj, float Value);
void  EDIT_SetValue     (EDIT_Handle hObj, int32_t Value);
int   EDIT_GetNumChars  (EDIT_Handle hObj);



void  EDIT_SetHexMode(EDIT_Handle hEdit, uint32_t Value, uint32_t Min, uint32_t Max);
void  EDIT_SetBinMode(EDIT_Handle hEdit, uint32_t Value, uint32_t Min, uint32_t Max);
void  EDIT_SetDecMode(EDIT_Handle hEdit, int32_t Value, int32_t Min, int32_t Max, int Shift, uint8_t Flags);
void  EDIT_SetFloatMode(EDIT_Handle hEdit, float Value, float Min, float Max, int Shift, uint8_t Flags);

uint32_t   GUI_EditHex(uint32_t Value, uint32_t Min, uint32_t Max, int Len, int xsize);
uint32_t   GUI_EditBin(uint32_t Value, uint32_t Min, uint32_t Max, int Len, int xsize);
int32_t   GUI_EditDec(int32_t Value, int32_t Min, int32_t Max, int Len, int xsize, int Shift, uint8_t Flags);
float GUI_EditFloat(float Value, float Min, float Max, int Len, int xsize, int Shift, uint8_t Flags);
void  GUI_EditString(char * pString, int Len, int xsize);



/* Signed or normal mode */
#define GUI_EDIT_NORMAL 0
#define GUI_EDIT_SIGNED 1

/* Edit modes */
#define GUI_EDIT_MODE_INSERT    0
#define GUI_EDIT_MODE_OVERWRITE 1

/* Compatibility macros */
#define EDIT_CI_DISABELD EDIT_CI_DISABLED
#define EDIT_CI_ENABELD  EDIT_CI_ENABLED

#if defined(__cplusplus)
  }
#endif
