//#include "GUI.h"
//#include "LCDConf.h"
//
//void MainTask(void);
//void GUIDEMO_main(void);
//
//int main(void) {
////	GUI_Init();
////	for (;;)
////		GUIDEMO_main();
//	return 1;
//}

#include "GUI.h"
#include "DIALOG.h"
#include "DROPDOWN.h"

#if 1

static int _MultiSel;
static int _OwnerDrawn;
static int _VarY = 1;
static int _PrevTime;


const RGB_COLOR ColorsSmilie0[] = {
	 0xFFFFFF,0x000000,0x0000FF
};

const RGB_COLOR ColorsSmilie1[] = {
	 0xFFFFFF,0x000000,0x00FFFF
};

const GUI_LOGPALETTE PalSmilie0 = {
  3,	/* number of entries */
  1, 	/* Has transparency */
  &ColorsSmilie0[0]
};

const GUI_LOGPALETTE PalSmilie1 = {
  3,	/* number of entries */
  1, 	/* Has transparency */
  &ColorsSmilie1[0]
};

const unsigned char acSmilie0[] = {
  0x00, 0x55, 0x40, 0x00,
  0x01, 0xAA, 0x90, 0x00,
  0x06, 0xAA, 0xA4, 0x00,
  0x19, 0x6A, 0x59, 0x00,
  0x69, 0x6A, 0x5A, 0x40,
  0x6A, 0xA6, 0xAA, 0x40,
  0x6A, 0xA6, 0xAA, 0x40,
  0x6A, 0xA6, 0xAA, 0x40,
  0x6A, 0xAA, 0xAA, 0x40,
  0x1A, 0x95, 0xA9, 0x00,
  0x06, 0x6A, 0x64, 0x00,
  0x01, 0xAA, 0x90, 0x00,
  0x00, 0x55, 0x40, 0x00
};

const unsigned char acSmilie1[] = {
  0x00, 0x55, 0x40, 0x00,
  0x01, 0xAA, 0x90, 0x00,
  0x06, 0xAA, 0xA4, 0x00,
  0x19, 0x6A, 0x59, 0x00,
  0x69, 0x6A, 0x5A, 0x40,
  0x6A, 0xA6, 0xAA, 0x40,
  0x6A, 0xA6, 0xAA, 0x40,
  0x6A, 0xA6, 0xAA, 0x40,
  0x6A, 0xAA, 0xAA, 0x40,
  0x1A, 0x6A, 0x69, 0x00,
  0x06, 0x95, 0xA4, 0x00,
  0x01, 0xAA, 0x90, 0x00,
  0x00, 0x55, 0x40, 0x00
};

const GUI_BITMAP bmSmilie0 = {
 13, /* XSize */
 13, /* YSize */
 4,  /* BytesPerLine */
 2,  /* BitsPerPixel */
 acSmilie0,   /* Pointer to picture data (indices) */
 &PalSmilie0  /* Pointer to palette */
};

const GUI_BITMAP bmSmilie1 = {
 13, /* XSize */
 13, /* YSize */
 4,  /* BytesPerLine */
 2,  /* BitsPerPixel */
 acSmilie1,   /* Pointer to picture data (indices) */
 &PalSmilie1  /* Pointer to palette */
};


static const GUI_ConstString _ListBox[] = {
  "English", "Deutsch", NULL
};

#define GUI_ID_MULTIEDIT0  GUI_ID_USER + 0x00
#define GUI_ID_CHECK0      GUI_ID_USER + 0x01
#define GUI_ID_CHECK1      GUI_ID_USER + 0x02
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
	{ FRAMEWIN_CreateIndirect,  "Owner drawn list box",    0,                  50,  50, 220, 165, FRAMEWIN_CF_MOVEABLE },
	{ LISTBOX_CreateIndirect,   0,                         GUI_ID_MULTIEDIT0,  10,  10, 100, 100, 0, 100 },
	/* Check box for multi select mode */
	{ CHECKBOX_CreateIndirect,  0,                         GUI_ID_CHECK0,     120,  10,   0,   0 },
	{ TEXT_CreateIndirect,      "Multi select",            0,                 140,  10,  80,  15, TEXT_CF_LEFT },
	/* Check box for owner drawn list box */
	{ CHECKBOX_CreateIndirect,  0,                         GUI_ID_CHECK1,     120,  35,   0,   0 },
	{ TEXT_CreateIndirect,      "Owner drawn",              0,                140,  35,  80,  15, TEXT_CF_LEFT },
	/* Buttons */
	{ BUTTON_CreateIndirect,    "OK",                      GUI_ID_OK,         120,  65,  80,  20 },
	{ BUTTON_CreateIndirect,    "Cancel",                  GUI_ID_CANCEL,     120,  90,  80,  20 }
};

static int _GetItemSizeX(WM_HWIN hWin, int ItemIndex) {
	char acBuffer[100];
	int  DistX;
	LISTBOX_GetItemText(hWin, ItemIndex, acBuffer, sizeof(acBuffer));
	DistX = GUI_GetStringDistX(acBuffer);
	return DistX + bmSmilie0.XSize + 16;
}


static int _GetItemSizeY(WM_HWIN hWin, int ItemIndex) {
	int DistY;
	DistY = GUI_GetFontDistY() + 1;
	if (LISTBOX_GetMulti(hWin)) {
		if (LISTBOX_GetItemSel(hWin, ItemIndex)) {
			DistY += 8;
		}
	}
	else if (LISTBOX_GetSel(hWin) == ItemIndex) {
		DistY += 8;
	}
	return DistY;
}

/*********************************************************************
*
*       _OwnerDraw
*
* Purpose:
*   This is the owner draw function.
*   It allows complete customization of how the items in the listbox are
*   drawn. A command specifies what the function should do;
*   The minimum is to react to the draw command (WIDGET_ITEM_DRAW);
*   If the item x-size differs from the default, then this information
*   needs to be returned in reaction to WIDGET_ITEM_GET_XSIZE.
*   To insure compatibility with future version, all unhandled commands
*   must call the default routine LISTBOX_OwnerDraw.
*/
static int _OwnerDraw(const WIDGET_ITEM_DRAW_INFO *pDrawItemInfo) {
	WM_HWIN hWin;
	int Index;
	hWin = pDrawItemInfo->hWin;
	Index = pDrawItemInfo->ItemIndex;
	switch (pDrawItemInfo->Cmd) {
		case WIDGET_ITEM_GET_XSIZE:
			return _GetItemSizeX(hWin, Index);
		case WIDGET_ITEM_GET_YSIZE:
			return _GetItemSizeY(hWin, Index);
		case WIDGET_ITEM_DRAW:
		{
			int MultiSel, Sel, YSize, FontDistY;
			int IsDisabled, IsSelected;
			int ColorIndex = 0;
			char acBuffer[100];
			const GUI_BITMAP *pBm;
			const GUI_FONT *pOldFont = 0;
			RGB_COLOR aColor[4] = { RGB_BLACK, RGB_WHITE, RGB_WHITE, RGB_GRAY };
			RGB_COLOR aBkColor[4] = { RGB_WHITE, RGB_GRAY, RGB_BLUE, RGB_GRAYL(0xC0) };
			IsDisabled = LISTBOX_GetItemDisabled(pDrawItemInfo->hWin, pDrawItemInfo->ItemIndex);
			IsSelected = LISTBOX_GetItemSel(hWin, Index);
			MultiSel = LISTBOX_GetMulti(hWin);
			Sel = LISTBOX_GetSel(hWin);
			YSize = _GetItemSizeY(hWin, Index);
			/* Calculate color index */
			if (MultiSel) {
				if (IsDisabled) {
					ColorIndex = 3;
				}
				else {
					ColorIndex = (IsSelected) ? 2 : 0;
				}
			}
			else {
				if (IsDisabled) {
					ColorIndex = 3;
				}
				else {
					if (pDrawItemInfo->ItemIndex == Sel) {
						ColorIndex = WM_HasFocus(pDrawItemInfo->hWin) ? 2 : 1;
					}
					else {
						ColorIndex = 0;
					}
				}
			}
			/* Draw item */
			GUI_SetBkColor(aBkColor[ColorIndex]);
			GUI_SetColor(aColor[ColorIndex]);
			LISTBOX_GetItemText(pDrawItemInfo->hWin, pDrawItemInfo->ItemIndex, acBuffer, sizeof(acBuffer));
			GUI_Clear();
			FontDistY = GUI_GetFontDistY();
			GUI_DispStringAt(acBuffer, pDrawItemInfo->x0 + bmSmilie0.XSize + 16, pDrawItemInfo->y0 + (YSize - FontDistY) / 2);
			/* Draw bitmap */
			pBm = MultiSel ? IsSelected ? &bmSmilie1 : &bmSmilie0 : (pDrawItemInfo->ItemIndex == Sel) ? &bmSmilie1 : &bmSmilie0;
			GUI_DrawBitmap(pBm, pDrawItemInfo->x0 + 7, pDrawItemInfo->y0 + (YSize - pBm->YSize) / 2);
			/* Draw focus rectangle */
			if (MultiSel && (pDrawItemInfo->ItemIndex == Sel)) {
				GUI_RECT rFocus;
				GUI_RECT rInside;
				WM_GetInsideRectEx(pDrawItemInfo->hWin, &rInside);
				rFocus.x0 = pDrawItemInfo->x0;
				rFocus.y0 = pDrawItemInfo->y0;
				rFocus.x1 = rInside.x1;
				rFocus.y1 = pDrawItemInfo->y0 + YSize - 1;
				GUI_SetColor(RGB_WHITE - aBkColor[ColorIndex]);
				GUI_DrawFocusRect(&rFocus, 0);
			}
		}
		break;
		default:
			return LISTBOX_OwnerDraw(pDrawItemInfo);
	}
	return 0;
}


static WM_PARAM _cbCallback(WM_HWIN hWin, int MsgId, WM_PARAM Data) {
	WM_HWIN hItem, hListBox = WM_GetDialogItem(hWin, GUI_ID_MULTIEDIT0);
	switch (MsgId) {
		case WM_INIT_DIALOG:
			LISTBOX_SetText(hListBox, _ListBox);
			LISTBOX_AddString(hListBox, "Francis");
			LISTBOX_AddString(hListBox, "Japanese");
			LISTBOX_AddString(hListBox, "Italiano");
			LISTBOX_AddString(hListBox, "Espanol");
			LISTBOX_AddString(hListBox, "Greek");
			LISTBOX_AddString(hListBox, "Hebrew");
			LISTBOX_AddString(hListBox, "Dutch");
			LISTBOX_AddString(hListBox, "Other language ...");
			LISTBOX_SetScrollStepH(hListBox, 6);
			LISTBOX_SetAutoScrollH(hListBox, 1);
			LISTBOX_SetAutoScrollV(hListBox, 1);
			LISTBOX_SetOwnerDraw(hListBox, _OwnerDraw);
			hItem = WM_GetDialogItem(hWin, GUI_ID_CHECK1);
			CHECKBOX_Check(hItem);
			return 0;
		case WM_KEY: {
			const WM_KEY_INFO *pInfo = (const WM_KEY_INFO *)Data;
			switch (pInfo->Key) {
				case GUI_KEY_ESCAPE:
					GUI_EndDialog(hWin, 1);
					break;
				case GUI_KEY_ENTER:
					GUI_EndDialog(hWin, 0);
					break;
			}
			return 0;
		}
		case WM_TOUCH_CHILD:
			//WM_SetFocus(hListBox);
			return 0;
		case WM_NOTIFY_PARENT: {
			const WM_NOTIFY_INFO *pInfo = (const WM_NOTIFY_INFO *)Data;
			int Id = WM_GetId(pInfo->hWinSrc); /* Id of widget */
			hItem = WM_GetDialogItem(hWin, Id);
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_SEL_CHANGED:
					LISTBOX_InvalidateItem(hListBox, LISTBOX_ALL_ITEMS);
					break;
				case WM_NOTIFICATION_RELEASED: /* React only if released */
					switch (Id) {
						case GUI_ID_OK:
							GUI_EndDialog(hWin, 0);
							break;
						case GUI_ID_CANCEL:
							GUI_EndDialog(hWin, 1);
							break;
						case GUI_ID_CHECK0:
							_MultiSel ^= 1;
							LISTBOX_SetMulti(hListBox, _MultiSel);
							WM_SetFocus(hListBox);
							LISTBOX_InvalidateItem(hListBox, LISTBOX_ALL_ITEMS);
							break;
						case GUI_ID_CHECK1:
							_OwnerDrawn ^= 1;
							if (_OwnerDrawn)
								LISTBOX_SetOwnerDraw(hListBox, _OwnerDraw);
							else
								LISTBOX_SetOwnerDraw(hListBox, NULL);
							LISTBOX_InvalidateItem(hListBox, LISTBOX_ALL_ITEMS);
							break;
					}
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}

#define ID_MENU             (GUI_ID_USER +  0)
#define ID_MENU_FILE_NEW    (GUI_ID_USER +  1)
#define ID_MENU_FILE_OPEN   (GUI_ID_USER +  2)
#define ID_MENU_FILE_CLOSE  (GUI_ID_USER +  3)
#define ID_MENU_FILE_EXIT   (GUI_ID_USER +  4)
#define ID_MENU_FILE_RECENT (GUI_ID_USER +  5)
#define ID_MENU_RECENT_0    (GUI_ID_USER +  6)
#define ID_MENU_RECENT_1    (GUI_ID_USER +  7)
#define ID_MENU_RECENT_2    (GUI_ID_USER +  8)
#define ID_MENU_RECENT_3    (GUI_ID_USER +  9)
#define ID_MENU_EDIT_UNDO   (GUI_ID_USER + 10)
#define ID_MENU_EDIT_REDO   (GUI_ID_USER + 11)
#define ID_MENU_EDIT_COPY   (GUI_ID_USER + 12)
#define ID_MENU_EDIT_PASTE  (GUI_ID_USER + 13)
#define ID_MENU_EDIT_DELETE (GUI_ID_USER + 14)
#define ID_MENU_HELP_ABOUT  (GUI_ID_USER + 15)
#include "MENU.h"
static void _AddMenuItem(MENU_Handle hMenu, MENU_Handle hSubmenu, const char *pText, uint16_t Id, uint16_t Flags) {
	MENU_ITEM_DATA Item;
	
	Item.pText = pText;
	Item.hSubmenu = hSubmenu;
	Item.Flags = Flags;
	Item.Id = Id;
	MENU_AddItem(hMenu, &Item);
}
/*********************************************************************
*
*       MainTask
*
*       Demonstrates a owner drawn list box
*
**********************************************************************
*/
static WM_HWIN _CreateMenu(WM_HWIN hParent) {
	MENU_Handle hMenu;
	MENU_Handle hMenuFile;
	MENU_Handle hMenuEdit;
	MENU_Handle hMenuHelp;
	MENU_Handle hMenuRecent;
	//
	// Create main menu
	//
	hMenu = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_HORIZONTAL, ID_MENU);
	//
	// Create sub menus
	//
	hMenuFile = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	hMenuEdit = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	hMenuHelp = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	hMenuRecent = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	//
	// Add menu items to menu &#39;Recent&#39;
	//
	_AddMenuItem(hMenuRecent, 0, "File 1", ID_MENU_RECENT_0, 0);
	_AddMenuItem(hMenuRecent, 0, "File 2", ID_MENU_RECENT_1, 0);
	_AddMenuItem(hMenuRecent, 0, "File 3", ID_MENU_RECENT_2, 0);
	_AddMenuItem(hMenuRecent, 0, "File 4", ID_MENU_RECENT_3, 0);
	_AddMenuItem(hMenuRecent, 0, "File 4", ID_MENU_RECENT_3, 0);
	_AddMenuItem(hMenuRecent, 0, "File 4", ID_MENU_RECENT_3, 0);
	//
	// Add menu items to menu &#39;File&#39;
	//
	_AddMenuItem(hMenuFile, 0, "New", ID_MENU_FILE_NEW, 0);
	_AddMenuItem(hMenuFile, 0, "Open", ID_MENU_FILE_OPEN, 0);
	_AddMenuItem(hMenuFile, 0, "Close", ID_MENU_FILE_CLOSE, MENU_IF_DISABLED);
	_AddMenuItem(hMenuFile, 0, 0, 0, MENU_IF_SEPARATOR);
//	_AddMenuItem(hMenuFile, hMenuRecent, "Files...", ID_MENU_FILE_RECENT, 0);
	_AddMenuItem(hMenuFile, 0, 0, 0, MENU_IF_SEPARATOR);
	_AddMenuItem(hMenuFile, 0, "Exit", ID_MENU_FILE_EXIT, 0);
	//
	// Add menu items to menu &#39;Edit&#39;
	//
	_AddMenuItem(hMenuEdit, 0, "Undo", ID_MENU_EDIT_UNDO, 0);
	_AddMenuItem(hMenuEdit, 0, "Redo", ID_MENU_EDIT_REDO, 0);
	_AddMenuItem(hMenuEdit, 0, 0, 0, MENU_IF_SEPARATOR);
	_AddMenuItem(hMenuEdit, 0, "Copy", ID_MENU_EDIT_COPY, 0);
	_AddMenuItem(hMenuEdit, 0, "Paste", ID_MENU_EDIT_PASTE, 0);
	_AddMenuItem(hMenuEdit, 0, "Delete", ID_MENU_EDIT_DELETE, 0);
	//
	// Add menu items to menu &#39;Help&#39;
	//
	_AddMenuItem(hMenuHelp, 0, "About", ID_MENU_HELP_ABOUT, 0);
	//
	// Add menu items to main menu
	//
	_AddMenuItem(hMenu, hMenuFile, "File", 0, 0);
//	_AddMenuItem(hMenu, hMenuEdit, "Edit", 0, 0);
//	_AddMenuItem(hMenu, hMenuHelp, "Help", 0, 0);
	//
	// Attach menu to parent window
	//
	FRAMEWIN_AddMenu(hParent, hMenu);
	return hMenu;
}

void main(void) {
	GUI_Init();
	WM_SetDesktopColor(RGB_GRAY);
	for (;;) {
		_MultiSel = 0;
		_OwnerDrawn = 1;
		WM_HWIN hDialog = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);
		_CreateMenu(hDialog);
		WM_HWIN hDrp = DROPDOWN_Create(WM_GetClientWindow(hDialog), 10, 110, 100, 80, WM_CF_SHOW);
		DROPDOWN_AddString(hDrp, "12");
		DROPDOWN_AddString(hDrp, "123");
		DROPDOWN_AddString(hDrp, "124");
		DROPDOWN_AddString(hDrp, "125");
		DROPDOWN_AddString(hDrp, "12578");
		WM_DIALOG_STATUS DialogStatus = { 0 };
		/* Let window know how to send feedback (close info & return value) */
		GUI_SetDialogStatusPtr(hDialog, &DialogStatus);
		while (!DialogStatus.Done) {
			GUI_Exec();
		}
	}
}

#endif

#if 0
#include "LISTVIEW.h"

const char *pRows[][5] = {
	{
	"Row 1x1",
	"Row 1x2",
	"Row 1x3"
	},
	{
	"Row 2x1",
	"Row 2x2",
	"Row 2x3"
	}, 	{
	"Row 3x1",
	"Row 3x2",
	"Row 3x3"
	},
	{
	"Row 4x1",
	"Row 4x2",
	"Row 4x3"
	}, 	{
	"Row 5x1",
	"Row 5x2",
	"Row 5x3"
	}
};

void main(void) {
	GUI_Init();
	WM_HWIN hListView = LISTVIEW_Create(10, 110, 50, 70, 0, 0, WM_CF_SHOW, 0);
	LISTVIEW_AddColumn(hListView, 0, "Col 1    ", GUI_TA_LEFT);
	LISTVIEW_AddColumn(hListView, 0, "Col 2    ", GUI_TA_LEFT);
	LISTVIEW_AddColumn(hListView, 0, "Col 3     ", GUI_TA_LEFT);
	LISTVIEW_AddRow(hListView, pRows[0]);
	LISTVIEW_AddRow(hListView, pRows[1]);/*
	LISTVIEW_AddRow(hListView, pRows[2]);
	LISTVIEW_AddRow(hListView, pRows[3]);
	LISTVIEW_AddRow(hListView, pRows[4]);*/
	WM_SetScrollbarH(hListView, 1);
	WM_SetScrollbarV(hListView, 1);
	for (;;)
		GUI_Exec();
}
#endif
