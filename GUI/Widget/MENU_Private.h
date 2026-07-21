#pragma once

#include "WIDGET.h"
#include "GUI_ARRAY.h"

#define MENU_SF_HORIZONTAL              MENU_CF_HORIZONTAL
#define MENU_SF_VERTICAL                MENU_CF_VERTICAL
#define MENU_SF_OPEN_ON_POINTEROVER     MENU_CF_OPEN_ON_POINTEROVER
#define MENU_SF_CLOSE_ON_SECOND_CLICK   MENU_CF_CLOSE_ON_SECOND_CLICK
#define MENU_SF_HIDE_DISABLED_SEL       MENU_CF_HIDE_DISABLED_SEL
#define MENU_SF_ACTIVE            (1<<6)  /* Internal flag only */
#define MENU_SF_POPUP             (1<<7)  /* Internal flag only */

typedef struct {
	MENU_Handle hSubmenu;
	uint16_t Id;
	uint16_t Flags;
	uint16_t TextWidth;
	char acText[1];
} MENU_ITEM;

struct MENU_Obj : public WIDGET {
	struct Properties {
		RGBC aTextColor[5]{
			RGB_BLACK,          /* enabled, not selected */
			RGB_WHITE,          /* enabled, selected */
			RGB_GRAYL(0x7C),    /* disabled, not selected */
			RGB_LIGHTGRAY,      /* disabled, selected */
			RGB_WHITE           /* active submenu */
		};
		RGBC aBkColor[5]{
			RGB_LIGHTGRAY,
			RGB_BLUEL(0x98),
			RGB_LIGHTGRAY,
			RGB_BLUEL(0x98),
			RGB_GRAYL(0x7C)
		};
		uint8_t aBorder[4]{ 4, 4, 2, 2 }; /* Left, Right, Top, Bottom */
		PCFONT pFont{ &GUI_Font13_1 };
	} static DefaultProps;
	Properties Props;
	GUI_ARRAY   ItemArray;
	WM_Obj * hOwner;
	uint16_t Flags;
	char IsSubmenuActive;
	uint16_t Width;
	uint16_t Height;
	uint16_t Sel;
};

extern const WIDGET_EFFECT *MENU__pDefaultEffect;

void      MENU__RecalcTextWidthOfItems(MENU_Obj *pObj);
void      MENU__ResizeMenu(MENU_Obj *pObj);
unsigned  MENU__GetNumItems(MENU_Obj *pObj);
char      MENU__SetItem(MENU_Obj *pObj, unsigned Index, const MENU_ITEM_DATA *pItemData);
void      MENU__SetItemFlags(MENU_Obj *pObj, unsigned Index, uint16_t Mask, uint16_t Flags);
void      MENU__InvalidateItem(MENU_Obj *pObj, unsigned Index);
int       MENU__FindItem(MENU_Handle hObj, uint16_t ItemId, MENU_Handle *phMenu);
int       MENU__SendMenuMessage(MENU_Handle hObj, WM_Obj * hDestWin, uint16_t MsgType, uint16_t ItemId);
