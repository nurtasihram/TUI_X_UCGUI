#pragma once

#include "WM.h"
#include "WIDGET.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */

#define MENU_CF_HORIZONTAL              (0<<0)
#define MENU_CF_VERTICAL                (1<<0)
#define MENU_CF_OPEN_ON_POINTEROVER     (1<<1)
#define MENU_CF_CLOSE_ON_SECOND_CLICK   (1<<2)
#define MENU_CF_HIDE_DISABLED_SEL       (1<<3)  /* Hides the selection when a disabled item is selected */
#define MENU_IF_DISABLED          (1<<0)
#define MENU_IF_SEPARATOR         (1<<1)
#define MENU_CI_ENABLED           0
#define MENU_CI_SELECTED          1
#define MENU_CI_DISABLED          2
#define MENU_CI_DISABLED_SEL      3
#define MENU_CI_ACTIVE_SUBMENU    4
#define MENU_BI_LEFT              0
#define MENU_BI_RIGHT             1
#define MENU_BI_TOP               2
#define MENU_BI_BOTTOM            3
#define MENU_ON_ITEMSELECT        0   /* Send to owner when selecting a menu item */
#define MENU_ON_INITMENU          1   /* Send to owner when for the first time selecting a submenu */
#define MENU_ON_INITSUBMENU       2   /* Send to owner when selecting a submenu */
#define MENU_ON_OPEN              3   /* Internal message of menu widget (send to submenus) */
#define MENU_ON_CLOSE             4   /* Internal message of menu widget (send to submenus) */
#define MENU_IS_MENU              5   /* Internal message of menu widget. Owner must call   */
                                      /* WM_DefaultProc() when not handle the message.      */
typedef WM_HMEM MENU_Handle;
typedef WM_HMEM MENU_Handle;

typedef struct {
  uint16_t MsgType;
  uint16_t ItemId;
} MENU_MSG_DATA;

typedef struct {
  const char* pText;
  uint16_t         Id;
  uint16_t         Flags;
  MENU_Handle hSubmenu;
} MENU_ITEM_DATA;

MENU_Handle MENU_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK* cb);
MENU_Handle MENU_CreateEx      (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id);

void      MENU_Attach           (MENU_Handle hObj, WM_HWIN hDestWin, int x, int y, int xSize, int ySize, int Flags);
void      MENU_Popup            (MENU_Handle hObj, WM_HWIN hDestWin, int x, int y, int xSize, int ySize, int Flags);
void      MENU_SetOwner         (MENU_Handle hObj, WM_HWIN hOwner);
void      MENU_AddItem          (MENU_Handle hObj, const MENU_ITEM_DATA* pItemData);
void      MENU_DeleteItem       (MENU_Handle hObj, uint16_t ItemId);
void      MENU_InsertItem       (MENU_Handle hObj, uint16_t ItemId, const MENU_ITEM_DATA* pItemData);
void      MENU_SetItem          (MENU_Handle hObj, uint16_t ItemId, const MENU_ITEM_DATA* pItemData);
void      MENU_GetItem          (MENU_Handle hObj, uint16_t ItemId, MENU_ITEM_DATA* pItemData);
void      MENU_GetItemText      (MENU_Handle hObj, uint16_t ItemId, char* pBuffer, unsigned BufferSize);
unsigned  MENU_GetNumItems      (MENU_Handle hObj);
void      MENU_DisableItem      (MENU_Handle hObj, uint16_t ItemId);
void      MENU_EnableItem       (MENU_Handle hObj, uint16_t ItemId);
void      MENU_SetTextColor     (MENU_Handle hObj, unsigned ColorIndex, RGB_COLOR Color);
void      MENU_SetBkColor       (MENU_Handle hObj, unsigned ColorIndex, RGB_COLOR Color);
void      MENU_SetBorderSize    (MENU_Handle hObj, unsigned BorderIndex, uint8_t BorderSize);
void      MENU_SetFont          (MENU_Handle hObj, const GUI_FONT * pFont);
