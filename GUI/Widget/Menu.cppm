module;

#include "WM.h"
#include "WIDGET.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */

export module TUX.Widget.Menu;

export {

constexpr uint16_t MENU_CF_HORIZONTAL               = 0 << 0;
constexpr uint16_t MENU_CF_VERTICAL                 = 1 << 0;
constexpr uint16_t MENU_CF_OPEN_ON_POINTEROVER      = 1 << 1;
constexpr uint16_t MENU_CF_CLOSE_ON_SECOND_CLICK    = 1 << 2;
constexpr uint16_t MENU_CF_HIDE_DISABLED_SEL        = 1 << 3;  /* Hides the selection when a disabled item is selected */

constexpr uint16_t MENU_IF_DISABLED           = 1 << 0;
constexpr uint16_t MENU_IF_SEPARATOR          = 1 << 1;

enum MENU_CI {
	 MENU_CI_ENABLED           = 0,
	 MENU_CI_SELECTED          = 1,
	 MENU_CI_DISABLED          = 2,
	 MENU_CI_DISABLED_SEL      = 3,
	 MENU_CI_ACTIVE_SUBMENU    = 4
};

constexpr uint16_t MENU_BI_LEFT              = 0;
constexpr uint16_t MENU_BI_RIGHT             = 1;
constexpr uint16_t MENU_BI_TOP               = 2;
constexpr uint16_t MENU_BI_BOTTOM            = 3;

constexpr uint16_t MENU_ON_ITEMSELECT        = 0;   /* Send to owner when selecting a menu item */
constexpr uint16_t MENU_ON_INITMENU          = 1;   /* Send to owner when for the first time selecting a submenu */
constexpr uint16_t MENU_ON_INITSUBMENU       = 2;   /* Send to owner when selecting a submenu */
constexpr uint16_t MENU_ON_OPEN              = 3;   /* Internal message of menu widget (send to submenus) */
constexpr uint16_t MENU_ON_CLOSE             = 4;   /* Internal message of menu widget (send to submenus) */
constexpr uint16_t MENU_IS_MENU              = 5;   /* Internal message of menu widget. Owner must call   */
									  /* WM_DefaultProc() when not handle the message.      */

typedef WM_Obj * MENU_Handle;

struct MENU_MSG_DATA {
	uint16_t MsgType;
	uint16_t ItemId;
};

struct MENU_ITEM_DATA {
	const char *pText;
	uint16_t    Id;
	uint16_t    Flags;
	MENU_Handle hSubmenu;
};

MENU_Handle MENU_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb);
MENU_Handle MENU_CreateEx      (int x0, int y0, int xSize, int ySize, WM_Obj * hParent, int WinFlags, int ExFlags, int Id);

void      MENU_Attach           (MENU_Handle hObj, WM_Obj * hDestWin, int x, int y, int xSize, int ySize, int Flags);
void      MENU_Popup            (MENU_Handle hObj, WM_Obj * hDestWin, int x, int y, int xSize, int ySize, int Flags);
void      MENU_SetOwner         (MENU_Handle hObj, WM_Obj * hOwner);
void      MENU_AddItem          (MENU_Handle hObj, const MENU_ITEM_DATA *pItemData);
void      MENU_DeleteItem       (MENU_Handle hObj, uint16_t ItemId);
void      MENU_InsertItem       (MENU_Handle hObj, uint16_t ItemId, const MENU_ITEM_DATA *pItemData);
void      MENU_SetItem          (MENU_Handle hObj, uint16_t ItemId, const MENU_ITEM_DATA *pItemData);
void      MENU_GetItem          (MENU_Handle hObj, uint16_t ItemId, MENU_ITEM_DATA *pItemData);
void      MENU_GetItemText      (MENU_Handle hObj, uint16_t ItemId, char *pBuffer, unsigned BufferSize);
unsigned  MENU_GetNumItems      (MENU_Handle hObj);
void      MENU_DisableItem      (MENU_Handle hObj, uint16_t ItemId);
void      MENU_EnableItem       (MENU_Handle hObj, uint16_t ItemId);
void      MENU_SetTextColor     (MENU_Handle hObj, unsigned ColorIndex, RGBC Color);
void      MENU_SetBkColor       (MENU_Handle hObj, unsigned ColorIndex, RGBC Color);
void      MENU_SetBorderSize    (MENU_Handle hObj, unsigned BorderIndex, uint8_t BorderSize);
void      MENU_SetFont          (MENU_Handle hObj, PCFONT pFont);

}
