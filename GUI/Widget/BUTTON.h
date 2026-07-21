#pragma once

#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */
#include "WIDGET.h"

#define BUTTON_CF_HIDE   WM_CF_HIDE
#define BUTTON_CF_SHOW   WM_CF_SHOW
#define BUTTON_CF_MEMDEV WM_CF_MEMDEV
#define BUTTON_STATE_FOCUS      WIDGET_STATE_FOCUS
#define BUTTON_STATE_PRESSED    WIDGET_STATE_USER0
#define BUTTON_STATE_HASFOCUS   0

typedef WM_HWIN BUTTON_Handle;

enum BUTTON_BI {
	 BUTTON_BI_UNPRESSED = 0,
	 BUTTON_BI_PRESSED,
	 BUTTON_BI_DISABLED
};
enum BUTTON_CI {
	 BUTTON_CI_UNPRESSED = 0,
	 BUTTON_CI_PRESSED,
	 BUTTON_CI_DISABLED
};

/************************************************************
*
*       Create function(s)
  Note: the parameters to a create function may vary.
         Some widgets may have multiple create functions
*/
BUTTON_Handle BUTTON_Create        (int x0, int y0, int xsize, int ysize, int ID, int Flags);
BUTTON_Handle BUTTON_CreateAsChild (int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags);
BUTTON_Handle BUTTON_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK* cb);
BUTTON_Handle BUTTON_CreateEx      (int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                    int WinFlags, int ExFlags, int Id);

/*********************************************************************
*
*       The callback ...
*
* Do not call it directly ! It is only to be used from within an
* overwritten callback.
*/
RGBC        BUTTON_GetBkColor         (BUTTON_Handle hObj, unsigned int Index);
PCFONT BUTTON_GetFont(BUTTON_Handle hObj);
void             BUTTON_GetText            (BUTTON_Handle hObj, char * pBuffer, int MaxLen);
RGBC        BUTTON_GetTextColor       (BUTTON_Handle hObj, unsigned int Index);
unsigned         BUTTON_IsPressed          (BUTTON_Handle hObj);
void             BUTTON_SetBitmap          (BUTTON_Handle hObj, unsigned int Index, PCBITMAP pBitmap);
void             BUTTON_SetBitmapEx        (BUTTON_Handle hObj, unsigned int Index, PCBITMAP pBitmap, int x, int y);
void             BUTTON_SetBkColor         (BUTTON_Handle hObj, unsigned int Index, RGBC Color);
void             BUTTON_SetBMP             (BUTTON_Handle hObj, unsigned int Index, const void * pBitmap);
void             BUTTON_SetBMPEx           (BUTTON_Handle hObj, unsigned int Index, const void * pBitmap, int x, int y);
void             BUTTON_SetFont            (BUTTON_Handle hObj, PCFONT pfont);
void             BUTTON_SetState           (BUTTON_Handle hObj, int State);                                    /* Not to be doc. */
void             BUTTON_SetPressed         (BUTTON_Handle hObj, int State);
void             BUTTON_SetFocussable      (BUTTON_Handle hObj, int State);
void             BUTTON_SetText            (BUTTON_Handle hObj, const char* s);
void             BUTTON_SetTextAlign       (BUTTON_Handle hObj, int Align);
void             BUTTON_SetTextColor       (BUTTON_Handle hObj, unsigned int Index, RGBC Color);
void             BUTTON_SetSelfDrawEx      (BUTTON_Handle hObj, unsigned int Index, void (*pDraw)(void), int x, int y); /* Not to be doc. */
void             BUTTON_SetSelfDraw        (BUTTON_Handle hObj, unsigned int Index, void (*pDraw)(void));               /* Not to be doc. */
