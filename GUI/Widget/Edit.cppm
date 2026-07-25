module;

#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */

export module TUX.Widget.Edit;

import TUX.Widget;
 
export {

//constexpr uint16_t EDIT_CF_LEFT     = TEXTALIGN_LEFT;
//constexpr uint16_t EDIT_CF_RIGHT    = TEXTALIGN_RIGHT;
//constexpr uint16_t EDIT_CF_HCENTER  = TEXTALIGN_HCENTER;
//constexpr uint16_t EDIT_CF_VCENTER  = TEXTALIGN_VCENTER;
//constexpr uint16_t EDIT_CF_TOP      = TEXTALIGN_TOP;
//constexpr uint16_t EDIT_CF_BOTTOM   = TEXTALIGN_BOTTOM;

enum EDIT_CI {
	 EDIT_CI_DISABLED = 0,
	 EDIT_CI_ENABLED
};

typedef WM_Obj * EDIT_Handle;
typedef void tEDIT_AddKeyEx    (EDIT_Handle hObj, int Key);
typedef void tEDIT_UpdateBuffer(EDIT_Handle hObj);

EDIT_Handle EDIT_Create        (int x0, int y0, int xsize, int ysize, int Id, int MaxLen, int Flags);
EDIT_Handle EDIT_CreateAsChild (int x0, int y0, int xsize, int ysize, WM_Obj * hParent, int Id, int Flags, int MaxLen);
EDIT_Handle EDIT_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb);
EDIT_Handle EDIT_CreateEx      (int x0, int y0, int xsize, int ysize, WM_Obj * hParent,
                                int WinFlags, int ExFlags, int Id, int MaxLen);

/* Methods changing properties */
void EDIT_AddKey           (EDIT_Handle hObj, int Key);
void EDIT_ClearCursor      (EDIT_Handle hObj);
void EDIT_SetBkColor       (EDIT_Handle hObj, unsigned int Index, RGBC color);
void EDIT_SetCursorAtChar  (EDIT_Handle hObj, int Pos);
void EDIT_SetCursorAtPixel (EDIT_Handle hObj, int xPos);
void EDIT_SetFont          (EDIT_Handle hObj, PCFONT pfont);
int  EDIT_SetInsertMode    (EDIT_Handle hObj, int OnOff);
void EDIT_SetMaxLen        (EDIT_Handle hObj, int MaxLen);
void EDIT_SetpfAddKeyEx    (EDIT_Handle hObj, tEDIT_AddKeyEx * pfAddKeyEx);
void EDIT_SetpfUpdateBuffer(EDIT_Handle hObj, tEDIT_UpdateBuffer * pfUpdateBuffer);
void EDIT_SetText          (EDIT_Handle hObj, const char *s);
void EDIT_SetTextAlign     (EDIT_Handle hObj, int Align);
void EDIT_SetTextColor     (EDIT_Handle hObj, unsigned int Index, RGBC color);
void EDIT_SetSel           (EDIT_Handle hObj, int FirstChar, int LastChar);

/* Get/Set user input */
float EDIT_GetFloatValue(EDIT_Handle hObj);
void  EDIT_GetText      (EDIT_Handle hObj, char *sDest, int MaxLen);
int32_t EDIT_GetValue     (EDIT_Handle hObj);
void  EDIT_SetValue     (EDIT_Handle hObj, int32_t Value);
int   EDIT_GetNumChars  (EDIT_Handle hObj);

/* Signed or normal mode */
constexpr uint8_t GUI_EDIT_NORMAL = 0;
constexpr uint8_t GUI_EDIT_SIGNED = 1;

/* Edit modes */
constexpr uint8_t GUI_EDIT_MODE_INSERT    = 0;
constexpr uint8_t GUI_EDIT_MODE_OVERWRITE = 1;

}
