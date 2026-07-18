#include "GUI.h"
#include "DIALOG.h"
#include "DROPDOWN.h"

static BOOL _MultiSel = FALSE, _OwnerDrawn = TRUE;
static WM_HWIN _hMemDevFrame, _hMemDevPane;
static WM_HWIN _hNoMemDevFrame, _hNoMemDevPane;
static int _MemDevPhase;

const RGB_COLOR ColorsSmilie0[] = { RGB_WHITE, RGB_BLACK, RGB_RED };
const GUI_LOGPALETTE PalSmilie0 = { 3, 1, &ColorsSmilie0[0] };
const unsigned char acSmilie0[] = {
________,XXXXXXXX,XX______,________,
______XX,oooooooo,ooXX____,________,
____XXoo,oooooooo,ooooXX__,________,
__XXooXX,XXoooooo,XXXXooXX,________,
XXooooXX,XXoooooo,XXXXoooo,XX______,
XXoooooo,ooooXXoo,oooooooo,XX______,
XXoooooo,ooooXXoo,oooooooo,XX______,
XXoooooo,ooooXXoo,oooooooo,XX______,
XXoooooo,oooooooo,oooooooo,XX______,
__XXoooo,ooXXXXXX,ooooooXX,________,
____XXoo,XXoooooo,XXooXX__,________,
______XX,oooooooo,ooXX____,________,
________,XXXXXXXX,XX______,________ };
const GUI_BITMAP bmSmilie0 = {
	/* Size */ 13, 13,
	/* BytesPerLine */ 4,
	/* BitsPerPixel */ 2,
	acSmilie0, &PalSmilie0 };

const RGB_COLOR ColorsSmilie1[] = { RGB_WHITE, RGB_BLACK, RGB_YELLOW };
const GUI_LOGPALETTE PalSmilie1 = { 3, 1, &ColorsSmilie1[0] };
const unsigned char acSmilie1[] = {
________,XXXXXXXX,XX______,________,
______XX,oooooooo,ooXX____,________,
____XXoo,oooooooo,ooooXX__,________,
__XXooXX,XXoooooo,XXXXooXX,________,
XXooooXX,XXoooooo,XXXXoooo,XX______,
XXoooooo,ooooXXoo,oooooooo,XX______,
XXoooooo,ooooXXoo,oooooooo,XX______,
XXoooooo,ooooXXoo,oooooooo,XX______,
XXoooooo,oooooooo,oooooooo,XX______,
__XXoooo,XXoooooo,XXooooXX,________,
____XXoo,ooXXXXXX,ooooXX__,________,
______XX,oooooooo,ooXX____,________,
________,XXXXXXXX,XX______,________ };
const GUI_BITMAP bmSmilie1 = {
	/* Size */ 13, 13,
	/* BytesPerLine */ 4,
	/* BitsPerPixel */ 2,
	acSmilie1, &PalSmilie1 };

static const GUI_ConstString _ListBox[] = {
  "English", "Deutsch", NULL
};
#define GUI_ID_MULTIEDIT0  GUI_ID_USER + 0x00
#define GUI_ID_CHECK0      GUI_ID_USER + 0x01
#define GUI_ID_CHECK1      GUI_ID_USER + 0x02
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
	{ FRAMEWIN_CreateIndirect  , "Owner drawn list box" , 0                 , 50  , 50  , 220  , 165  , FRAMEWIN_CF_MOVEABLE       },
	{ LISTBOX_CreateIndirect   , ""                     , GUI_ID_MULTIEDIT0 , 10  , 10  , 100  , 100  , 0                    , 100 },
	{ CHECKBOX_CreateIndirect  , ""                     , GUI_ID_CHECK0     , 120 , 10  , 0    , 0                                 },
	{ TEXT_CreateIndirect      , "Multi select"         , 0                 , 140 , 10  , 80   , 15   , TEXT_CF_LEFT               },
	{ CHECKBOX_CreateIndirect  , ""                     , GUI_ID_CHECK1     , 120 , 35  , 0    , 0                                 },
	{ TEXT_CreateIndirect      , "Owner drawn"          , 0                 , 140 , 35  , 80   , 15   , TEXT_CF_LEFT               },
	{ BUTTON_CreateIndirect    , "OK"                   , GUI_ID_OK         , 120 , 65  , 80   , 20                                },
	{ BUTTON_CreateIndirect    , "Cancel"               , GUI_ID_CANCEL     , 120 , 90  , 80   , 20                                }
};

static int _GetItemSizeX(WM_HWIN hWin, int ItemIndex) {
	char acBuffer[100];
	LISTBOX_GetItemText(hWin, ItemIndex, acBuffer, sizeof(acBuffer));
	return GUI_GetStringDistX(acBuffer) + bmSmilie0.XSize + 16;
}
static int _GetItemSizeY(WM_HWIN hWin, int ItemIndex) {
	int DistY = GUI_GetFontDistY() + 1;
	if (LISTBOX_GetMulti(hWin)) {
		if (LISTBOX_GetItemSel(hWin, ItemIndex))
			DistY += 8;
	}
	else if (LISTBOX_GetSel(hWin) == ItemIndex)
		DistY += 8;
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
	WM_HWIN hWin = pDrawItemInfo->hWin;
	int Index = pDrawItemInfo->ItemIndex;
	switch (pDrawItemInfo->Cmd) {
		case WIDGET_ITEM_GET_XSIZE:
			return _GetItemSizeX(hWin, Index);
		case WIDGET_ITEM_GET_YSIZE:
			return _GetItemSizeY(hWin, Index);
		case WIDGET_ITEM_DRAW:
		{
			int ColorIndex = 0;
			char acBuffer[100];
			RGB_COLOR aColor[4] = { RGB_BLACK, RGB_WHITE, RGB_WHITE, RGB_GRAY };
			RGB_COLOR aBkColor[4] = { RGB_WHITE, RGB_GRAY, RGB_BLUE, RGB_GRAYL(0xC0) };
			bool IsDisabled = LISTBOX_GetItemDisabled(pDrawItemInfo->hWin, pDrawItemInfo->ItemIndex);
			bool IsSelected = LISTBOX_GetItemSel(hWin, Index);
			int MultiSel = LISTBOX_GetMulti(hWin);
			int Sel = LISTBOX_GetSel(hWin);
			int YSize = _GetItemSizeY(hWin, Index);
			/* Calculate color index */
			if (MultiSel)
				if (IsDisabled)
					ColorIndex = 3;
				else
					ColorIndex = IsSelected ? 2 : 0;
			else if (IsDisabled)
				ColorIndex = 3;
			else if (pDrawItemInfo->ItemIndex == Sel)
				ColorIndex = WM_HasFocus(pDrawItemInfo->hWin) ? 2 : 1;
			else
				ColorIndex = 0;
			/* Draw item */
			GUI_SetBkColor(aBkColor[ColorIndex]);
			GUI_SetColor(aColor[ColorIndex]);
			LISTBOX_GetItemText(pDrawItemInfo->hWin, pDrawItemInfo->ItemIndex, acBuffer, sizeof(acBuffer));
			GUI_Clear();
			auto FontDistY = GUI_GetFontDistY();
			GUI_DispStringAt(acBuffer, pDrawItemInfo->x0 + bmSmilie0.XSize + 16, pDrawItemInfo->y0 + (YSize - FontDistY) / 2);
			/* Draw bitmap */
			auto pBm = MultiSel ? IsSelected ? &bmSmilie1 : &bmSmilie0 : (pDrawItemInfo->ItemIndex == Sel) ? &bmSmilie1 : &bmSmilie0;
			GUI_DrawBitmap(pBm, pDrawItemInfo->x0 + 7, pDrawItemInfo->y0 + (YSize - pBm->YSize) / 2);
			/* Draw focus rectangle */
			if (MultiSel && (pDrawItemInfo->ItemIndex == Sel)) {
				GUI_RECT rFocus;
				GUI_RECT rInside = WM_GetInsideRect(pDrawItemInfo->hWin);
				rFocus.x0 = pDrawItemInfo->x0;
				rFocus.y0 = pDrawItemInfo->y0;
				rFocus.x1 = rInside.x1;
				rFocus.y1 = pDrawItemInfo->y0 + YSize - 1;
				GUI_SetColor(RGB_WHITE - aBkColor[ColorIndex]);
				GUI_DrawFocusRect(rFocus, 0);
			}
			break;
		}
		default:
			return LISTBOX_OwnerDraw(pDrawItemInfo);
	}
	return 0;
}

static WM_PARAM _cbMemDevPane(WM_HWIN hWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_PAINT:
		{
			int xSize = WM_GetWindowSizeX(hWin);
			int ySize = WM_GetWindowSizeY(hWin);
			int BarWidth = 36;
			int Span = xSize - BarWidth - 20;
			int XPos = 0;
			int MemDevOn = (hWin == _hMemDevPane);
			if (Span > 0) {
				int Phase = _MemDevPhase % (Span * 2);
				XPos = (Phase > Span) ? (Span * 2 - Phase) : Phase;
			}
			GUI_SetBkColor(RGB_WHITE);
			GUI_Clear();
			GUI_SetColor(RGB_DARKGRAY);
			GUI_DrawRect({0, 0, xSize - 1, ySize - 1});
			GUI_SetColor(RGB_BLACK);
			GUI_DispStringAt(MemDevOn ? "MemDev ON" : "MemDev OFF", 8, 8);
			GUI_DispStringAt(MemDevOn ? "WM_CF_MEMDEV enabled" : "WM_CF_MEMDEV disabled", 8, 24);
			GUI_SetColor(RGB_GRAY);
			GUI_DrawRect({10, 48, xSize - 11, 72});
			GUI_SetColor(MemDevOn ? RGB_GREEN : RGB_RED);
			GUI_FillRect({10 + XPos, 49, 10 + XPos + BarWidth, 71});
			GUI_SetColor(RGB_BLUE);
			GUI_FillRect({10, ySize - 40, xSize - 11, ySize - 25});
			GUI_SetColor(RGB_YELLOW);
			GUI_FillRect({10 + XPos / 2, ySize - 39, 35 + XPos / 2, ySize - 26});
			GUI_SetColor(RGB_BLACK);
			GUI_DispStringAt("Animated redraw area", 8, ySize - 18);
			return 0;
		}
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}

static WM_HWIN _CreateMemDevFrame(int x0, int y0, const char *pTitle, int UseMemDev, WM_HWIN *phPane) {
	WM_HWIN hFrame;
	WM_HWIN hClient;
	int xSize;
	int ySize;
	int Flags;
	Flags = WM_CF_SHOW;
	if (UseMemDev) {
		Flags |= WM_CF_MEMDEV;
	}
	hFrame = FRAMEWIN_CreateEx(x0, y0, 190, 180, 0, WM_CF_SHOW, FRAMEWIN_CF_MOVEABLE, 0, pTitle, 0);
	hClient = WM_GetClientWindow(hFrame);
	xSize = WM_GetWindowSizeX(hClient);
	ySize = WM_GetWindowSizeY(hClient);
	*phPane = WM_CreateWindowAsChild(0, 0, xSize, ySize, hClient, Flags, _cbMemDevPane, 0);
	return hFrame;
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
							_MultiSel = !_MultiSel;
							LISTBOX_SetMulti(hListBox, _MultiSel);
							WM_SetFocus(hListBox);
							LISTBOX_InvalidateItem(hListBox, LISTBOX_ALL_ITEMS);
							break;
						case GUI_ID_CHECK1:
							_OwnerDrawn = !_OwnerDrawn;
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
	_AddMenuItem(hMenuFile, hMenuRecent, "Files...", ID_MENU_FILE_RECENT, 0);
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
	_AddMenuItem(hMenu, hMenuEdit, "Edit", 0, 0);
	_AddMenuItem(hMenu, hMenuHelp, "Help", 0, 0);
	//
	// Attach menu to parent window
	//
	FRAMEWIN_AddMenu(hParent, hMenu);
	return hMenu;
}

int main(void) {
	GUI_Init();
	GUI_CURSOR_Show();
	WM_SetDesktopColor(RGB_GRAY);
	for (;;) {
		WM_HWIN hDialog = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);
		_CreateMenu(hDialog);
		_MemDevPhase = 0;
		_hMemDevFrame = _CreateMemDevFrame(280, 50, "MemDev ON", 1, &_hMemDevPane);
		_hNoMemDevFrame = _CreateMemDevFrame(480, 50, "MemDev OFF", 0, &_hNoMemDevPane);
		WM_HWIN hDrp = DROPDOWN_CreateEx(10, 110, 100, 80,
										 WM_GetClientWindow(hDialog),
										 WM_CF_SHOW, DROPDOWN_CF_AUTOSCROLLBAR, 0);
		DROPDOWN_AddString(hDrp, "1");
		DROPDOWN_AddString(hDrp, "12");
		DROPDOWN_AddString(hDrp, "123");
		DROPDOWN_AddString(hDrp, "1234");
		DROPDOWN_AddString(hDrp, "12345");
		DROPDOWN_AddString(hDrp, "123456");
		DROPDOWN_AddString(hDrp, "1234567");
		DROPDOWN_AddString(hDrp, "12345678");
		WM_DIALOG_STATUS DialogStatus = { 0 };
		/* Let window know how to send feedback (close info & return value) */
		GUI_SetDialogStatusPtr(hDialog, &DialogStatus);
		while (!DialogStatus.Done) {
			_MemDevPhase += 4;
			if (_hMemDevPane)
				WM_Invalidate(_hMemDevPane);
			if (_hNoMemDevPane)
				WM_Invalidate(_hNoMemDevPane);
			GUI_Exec();
			GUI_Delay(40);
		}
		if (_hMemDevFrame)
			FRAMEWIN_Delete(_hMemDevFrame);
		if (_hNoMemDevFrame)
			FRAMEWIN_Delete(_hNoMemDevFrame);
		_hMemDevFrame = 0;
		_hMemDevPane = 0;
		_hNoMemDevFrame = 0;
		_hNoMemDevPane = 0;
	}
	return 0;
}

#if 0
#include "LISTVIEW.h"

const char *pRows[][5] = {
	{
	"Row 1x1",
	"Row 1x2",
	"Row 1x3"
	}, {
	"Row 2x1",
	"Row 2x2",
	"Row 2x3"
	}, {
	"Row 3x1",
	"Row 3x2",
	"Row 3x3"
	}, {
	"Row 4x1",
	"Row 4x2",
	"Row 4x3"
	}, {
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
