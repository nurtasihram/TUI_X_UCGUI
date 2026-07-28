#include <stdio.h>

#include "GUI.h"
#include "DIALOG.h"

import TUX.Widget;
import TUX.Widget.CheckBox;
import TUX.Widget.Menu;
import TUX.Widget.ListView;
import TUX.Widget.Frame;
import TUX.Widget.Text;
import TUX.Widget.DropDown;
import TUX.Widget.MultiPage;
import TUX.Widget.Radio;
import TUX.Widget.ProgBar;

static bool _MultiSel = false, _OwnerDrawn = true;
const RGBC ColorsSmilie0[]{ RGB_WHITE, RGB_BLACK, RGB_RED };
const GUI_LOGPALETTE PalSmilie0{ 3, 1, &ColorsSmilie0[0] };
const uint8_t acSmilie0[]{
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
CBITMAP bmSmilie0{
	/* Size */ 13, 13,
	/* BytesPerLine */ 4,
	/* BitsPerPixel */ 2,
	acSmilie0, &PalSmilie0
};

const RGBC ColorsSmilie1[]{ RGB_WHITE, RGB_BLACK, RGB_YELLOW };
const GUI_LOGPALETTE PalSmilie1{ 3, 1, &ColorsSmilie1[0] };
const uint8_t acSmilie1[]{
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
CBITMAP bmSmilie1{
	/* Size */ 13, 13,
	/* BytesPerLine */ 4,
	/* BitsPerPixel */ 2,
	acSmilie1, &PalSmilie1
};

static const GUI_ConstString _ListBox[]{
  "English", "Deutsch", nullptr
};
#define GUI_ID_MULTIEDIT0  GUI_ID_USER + 0x00
#define GUI_ID_CHECK0      GUI_ID_USER + 0x01
#define GUI_ID_CHECK1      GUI_ID_USER + 0x02
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[]{
	{ FRAMEWIN_CreateIndirect  , "Owner drawn list box" , 0                 , 50  , 50  , 220  , 175  , FRAMEWIN_CF_MOVEABLE       },
	{ LISTBOX_CreateIndirect   , ""                     , GUI_ID_MULTIEDIT0 , 10  , 10  , 100  , 100  , 0                    , 100 },
	{ CHECKBOX_CreateIndirect  , ""                     , GUI_ID_CHECK0     , 120 , 10  , 0    , 0                                 },
	{ TEXT_CreateIndirect      , "Multi select"         , 0                 , 140 , 10  , 80   , 15   , TEXT_CF_LEFT               },
	{ CHECKBOX_CreateIndirect  , ""                     , GUI_ID_CHECK1     , 120 , 35  , 0    , 0                                 },
	{ TEXT_CreateIndirect      , "Owner drawn"          , 0                 , 140 , 35  , 80   , 15   , TEXT_CF_LEFT               },
	{ BUTTON_CreateIndirect    , "OK"                   , GUI_ID_OK         , 120 , 65  , 80   , 20                                },
	{ BUTTON_CreateIndirect    , "Cancel"               , GUI_ID_CANCEL     , 120 , 90  , 80   , 20                                }
};

static int _GetItemSizeY(LISTBOX_Obj *pObj, int ItemIndex) {
	int DistY = GUI_GetFontDistY() + 1;
	if (pObj->GetMulti()) {
		if (pObj->GetItemSel(ItemIndex))
			DistY += 8;
	}
	else if (pObj->GetSel() == ItemIndex)
		DistY += 8;
	return DistY;
}

static int _OwnerDraw(const WIDGET_ITEM_DRAW_INFO *pDrawItemInfo) {
	auto pObj = (LISTBOX_Obj *)pDrawItemInfo->hWin;
	int Index = pDrawItemInfo->ItemIndex;
	switch (pDrawItemInfo->Cmd) {
		case WIDGET_ITEM_GET_XSIZE: {
			char acBuffer[100];
			pObj->GetItemText(Index, acBuffer, sizeof(acBuffer));
			return GUI_GetStringDistX(acBuffer) + bmSmilie0.XSize + 16;
		}
		case WIDGET_ITEM_GET_YSIZE:
			return _GetItemSizeY(pObj, Index);
		case WIDGET_ITEM_DRAW:
		{
			int ColorIndex = 0;
			char acBuffer[100];
			RGBC aColor[4] = { RGB_BLACK, RGB_WHITE, RGB_WHITE, RGB_GRAY };
			RGBC aBkColor[4] = { RGB_WHITE, RGB_GRAY, RGB_DARKBLUE, RGB_GRAYL(0xC0) };
			bool IsDisabled = pObj->GetItemDisabled(pDrawItemInfo->ItemIndex);
			bool IsSelected = pObj->GetItemSel(Index);
			int MultiSel = pObj->GetMulti();
			int Sel = pObj->GetSel();
			int YSize = _GetItemSizeY(pObj, Index);
			/* Calculate color index */
			if (MultiSel)
				if (IsDisabled)
					ColorIndex = 3;
				else
					ColorIndex = IsSelected ? 2 : 0;
			else if (IsDisabled)
				ColorIndex = 3;
			else if (pDrawItemInfo->ItemIndex == Sel)
				ColorIndex = WM_HasFocus(pObj) ? 2 : 1;
			else
				ColorIndex = 0;
			/* Draw item */
			GUI_SetBkColor(aBkColor[ColorIndex]);
			GUI_SetColor(aColor[ColorIndex]);
			pObj->GetItemText(pDrawItemInfo->ItemIndex, acBuffer, sizeof(acBuffer));
			GUI_Clear();
			auto FontDistY = GUI_GetFontDistY();
			GUI_DispStringAt(acBuffer, pDrawItemInfo->x0 + bmSmilie0.XSize + 16, pDrawItemInfo->y0 + (YSize - FontDistY) / 2);
			/* Draw bitmap */
			auto pBm = MultiSel ? IsSelected ? &bmSmilie1 : &bmSmilie0 : (pDrawItemInfo->ItemIndex == Sel) ? &bmSmilie1 : &bmSmilie0;
			GUI_DrawBitmap(pBm, pDrawItemInfo->x0 + 7, pDrawItemInfo->y0 + (YSize - pBm->YSize) / 2);
			/* Draw focus rectangle */
			if (MultiSel && (pDrawItemInfo->ItemIndex == Sel)) {
				GUI_RECT rFocus;
				GUI_RECT rInside = WM_GetInsideRect(pObj);
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
			return LISTBOX_Obj::OwnerDraw(pDrawItemInfo);
	}
	return 0;
}


static WM_Obj *_hMemDevFrame;
static WM_Obj *_hMemDevPane;
static WM_Obj *_hNoMemDevFrame;
static WM_Obj *_hNoMemDevPane;
static int _MemDevPhase;

static WM_PARAM _cbMemDevPane(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
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
				XPos = Phase > Span ? Span * 2 - Phase : Phase;
			}
			GUI_SetBkColor(RGB_WHITE);
			GUI_Clear();
			GUI_SetColor(RGB_DARKGRAY);
			GUI_DrawRect({ 0, 0, xSize - 1, ySize - 1 });
			GUI_SetColor(RGB_BLACK);
			GUI_DispStringAt(MemDevOn ? "MemDev ON" : "MemDev OFF", 8, 8);
			GUI_DispStringAt(MemDevOn ? "WM_CF_MEMDEV enabled" : "WM_CF_MEMDEV disabled", 8, 24);
			GUI_SetColor(RGB_GRAY);
			GUI_DrawRect({ 10, 48, xSize - 11, 72 });
			GUI_SetColor(MemDevOn ? RGB_GREEN : RGB_RED);
			GUI_FillRect({ 10 + XPos, 49, 10 + XPos + BarWidth, 71 });
			GUI_SetColor(RGB_BLUE);
			GUI_FillRect({ 10, ySize - 40, xSize - 11, ySize - 25 });
			GUI_SetColor(RGB_YELLOW);
			GUI_FillRect({ 10 + XPos / 2, ySize - 39, 35 + XPos / 2, ySize - 26 });
			GUI_SetColor(RGB_BLACK);
			GUI_DispStringAt("Animated redraw area", 8, ySize - 18);
			return 0;
		}
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}

static WM_Obj *_CreateMemDevFrame(int x0, int y0, const char *pTitle, int UseMemDev, WM_Obj **phPane) {
	WM_Obj *hFrame;
	WM_Obj *hClient;
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

static WM_PARAM _cbCallback(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
	CHECKBOX_Obj *pItem;
	auto pListBox = (LISTBOX_Obj *)WM_GetDialogItem(hWin, GUI_ID_MULTIEDIT0);
	switch (MsgId) {
		case WM_INIT_DIALOG:
			pListBox->SetText(_ListBox);
			pListBox->AddString("Francis");
			pListBox->AddString("Japanese");
			pListBox->AddString("Italiano");
			pListBox->AddString("Espanol");
			pListBox->AddString("Greek");
			pListBox->AddString("Hebrew");
			pListBox->AddString("Dutch");
			pListBox->AddString("Other language ...");
			pListBox->SetScrollStepH(6);
			pListBox->SetAutoScrollH(1);
			pListBox->SetAutoScrollV(1);
			pListBox->SetOwnerDraw(_OwnerDraw);
			pItem = (CHECKBOX_Obj *)WM_GetDialogItem(hWin, GUI_ID_CHECK1);
			pItem->SetState(1);
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
			int Id = WM_GetId(pInfo->pWinSrc); /* Id of widget */
			pItem = (CHECKBOX_Obj *)WM_GetDialogItem(hWin, Id);
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_SEL_CHANGED:
					pListBox->InvalidateItem(LISTBOX_ALL_ITEMS);
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
							pListBox->SetMulti(_MultiSel);
							WM_SetFocus(pListBox);
							pListBox->InvalidateItem(LISTBOX_ALL_ITEMS);
							break;
						case GUI_ID_CHECK1:
							_OwnerDrawn = !_OwnerDrawn;
							if (_OwnerDrawn)
								pListBox->SetOwnerDraw(_OwnerDraw);
							else
								pListBox->SetOwnerDraw(nullptr);
							pListBox->InvalidateItem(LISTBOX_ALL_ITEMS);
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

static void _AddMenuItem(MENU_Obj *pMenu, MENU_Obj *pSubmenu, const char *pText, uint16_t Id, uint16_t Flags) {
	MENU_ITEM_DATA Item;
	Item.pText = pText;
	Item.pSubmenu = pSubmenu;
	Item.Flags = Flags;
	Item.Id = Id;
	pMenu->AddItem(&Item);
}
/*********************************************************************
*
*       MainTask
*
*       Demonstrates a owner drawn list box
*
**********************************************************************
*/
static void _CreateMenu(FRAMEWIN_Obj *pParent) {
	//
	// Create main menu
	//
	auto hMenu = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_HORIZONTAL, ID_MENU);
	//
	// Create sub menus
	//
	auto hMenuFile = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	auto hMenuEdit = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	auto hMenuHelp = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	auto hMenuRecent = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
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
	pParent->AddMenu(hMenu);
}

void _Create() {
}

/*********************************************************************
*
*       _TestListView
*
*       Demonstrates ListView widget
*
**********************************************************************
*/

#define ID_LISTVIEW_TEST    (GUI_ID_USER + 100)

static const GUI_WIDGET_CREATE_INFO _aListViewDialogCreate[] = {
	{ FRAMEWIN_CreateIndirect  , "ListView Test"      , 0                 , 50  , 50  , 320 , 240 , FRAMEWIN_CF_MOVEABLE       },
	{ LISTVIEW_CreateIndirect  , ""                   , ID_LISTVIEW_TEST  , 10  , 10  , 290 , 160 , 0                          },
	{ BUTTON_CreateIndirect    , "Add Row"            , GUI_ID_USER + 101 , 10  , 180 , 80  , 25                             },
	{ BUTTON_CreateIndirect    , "Delete Row"         , GUI_ID_USER + 102 , 100 , 180 , 80  , 25                             },
	{ BUTTON_CreateIndirect    , "Close"              , GUI_ID_CANCEL     , 190 , 180 , 80  , 25                             }
};

static WM_PARAM _cbListViewTest(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG: {
			auto pListView = (LISTVIEW_Obj *)WM_GetDialogItem(hWin, ID_LISTVIEW_TEST);
			// Add columns
			pListView->AddColumn(80, "Name", TEXTALIGN_LEFT);
			pListView->AddColumn(60, "Age", TEXTALIGN_RIGHT);
			pListView->AddColumn(120, "City", TEXTALIGN_LEFT);
			// Add rows
			const char *row1[] = { "Alice", "25", "New York", nullptr };
			const char *row2[] = { "Bob", "30", "London", nullptr };
			const char *row3[] = { "Charlie", "35", "Tokyo", nullptr };
			pListView->AddRow(row1);
			pListView->AddRow(row2);
			pListView->AddRow(row3);
			// Set grid visible
			pListView->SetGridVis(1);
			break;
		}
		case WM_NOTIFY_PARENT:
		{
			auto pInfo = (WM_NOTIFY_INFO *)Data;
			int Id = WM_GetId(pInfo->pWinSrc);
			auto pListView = (LISTVIEW_Obj *)WM_GetDialogItem(hWin, ID_LISTVIEW_TEST);
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_RELEASED:
					switch (Id) {
						case GUI_ID_USER + 101: // Add Row
						{
							static int rowCount = 1;
							char name[32], age[32];
							sprintf(name, "User %d", rowCount);
							sprintf(age, "%d", 20 + rowCount);
							const char *newRow[] = { name, age, "Paris", nullptr };
							pListView->AddRow(newRow);
							rowCount++;
							break;
						}
						case GUI_ID_USER + 102: // Delete Row
						{
							int sel = pListView->GetSel();
							if (sel >= 0) {
								pListView->DeleteRow(sel);
							}
							break;
						}
						case GUI_ID_CANCEL:
							GUI_EndDialog(hWin, 0);
							break;
					}
					break;
				case WM_NOTIFICATION_SEL_CHANGED:
					if (Id == ID_LISTVIEW_TEST) {
						int sel = pListView->GetSel();
						// Can add code to respond to selection change
					}
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}

void _TestListView() {
	WM_Obj *hDialog = GUI_CreateDialogBox(_aListViewDialogCreate, GUI_COUNTOF(_aListViewDialogCreate), &_cbListViewTest, 0, 0, 0);
	WM_DIALOG_STATUS DialogStatus = { 0 };
	GUI_SetDialogStatusPtr(hDialog, &DialogStatus);
	while (!DialogStatus.Done) {
		GUI_Exec();
	}
}

#define ID_DROPDOWN_TEST         (GUI_ID_USER + 120)
#define ID_DROPDOWN_STATUS       (GUI_ID_USER + 121)
#define ID_DROPDOWN_ADD          (GUI_ID_USER + 122)
#define ID_DROPDOWN_INSERT       (GUI_ID_USER + 123)
#define ID_DROPDOWN_DELETE       (GUI_ID_USER + 124)
#define ID_DROPDOWN_NEXT         (GUI_ID_USER + 125)
#define ID_DROPDOWN_PREV         (GUI_ID_USER + 126)
#define ID_DROPDOWN_AUTOSCROLL   (GUI_ID_USER + 127)
#define ID_DROPDOWN_EXPAND       (GUI_ID_USER + 128)
#define ID_DROPDOWN_COLLAPSE     (GUI_ID_USER + 129)

static bool _DropDownAutoScroll = false;
static int _DropDownItemCounter = 0;
static const char *_DropDownInitialItems[] = {
	"Alpha",
	"Beta",
	"Gamma",
	"Delta",
	nullptr
};

static void _UpdateDropDownStatus(WM_Obj *hWin) {
	auto pDropDown = (DROPDOWN_Obj *)WM_GetDialogItem(hWin, ID_DROPDOWN_TEST);
	auto pStatus = (TEXT_Obj *)WM_GetDialogItem(hWin, ID_DROPDOWN_STATUS);
	if (pDropDown && pStatus) {
		char acStatus[96];
		sprintf(acStatus, "Items: %d  Sel: %d  AutoScroll: %s",
				pDropDown->GetNumItems(),
				pDropDown->GetSel(),
				_DropDownAutoScroll ? "On" : "Off");
		pStatus->SetText(acStatus);
	}
}

static const GUI_WIDGET_CREATE_INFO _aDropDownDialogCreate[] = {
	{ FRAMEWIN_CreateIndirect  , "DropDown Test"      , 0                 , 50  , 40  , 390 , 230 , FRAMEWIN_CF_MOVEABLE       },
	{ DROPDOWN_CreateIndirect  , ""                   , ID_DROPDOWN_TEST  , 10  , 10  , 220 , 96  , 0                          },
	{ TEXT_CreateIndirect      , "Use the DropDown or buttons below to interact." , 0 , 10 , 45 , 360 , 16 , TEXT_CF_LEFT },
	{ BUTTON_CreateIndirect    , "Add Item"           , ID_DROPDOWN_ADD   , 10  , 70  , 80  , 25                             },
	{ BUTTON_CreateIndirect    , "Insert"             , ID_DROPDOWN_INSERT, 95  , 70  , 80  , 25                             },
	{ BUTTON_CreateIndirect    , "Delete"             , ID_DROPDOWN_DELETE, 180 , 70  , 80  , 25                             },
	{ BUTTON_CreateIndirect    , "Next"               , ID_DROPDOWN_NEXT   , 265 , 70  , 80  , 25                             },
	{ BUTTON_CreateIndirect    , "Prev"               , ID_DROPDOWN_PREV   , 10  , 105 , 80  , 25                             },
	{ BUTTON_CreateIndirect    , "AutoScroll"         , ID_DROPDOWN_AUTOSCROLL, 95 , 105 , 80  , 25                          },
	{ BUTTON_CreateIndirect    , "Expand"             , ID_DROPDOWN_EXPAND , 180 , 105 , 80  , 25                             },
	{ BUTTON_CreateIndirect    , "Collapse"           , ID_DROPDOWN_COLLAPSE, 265 , 105 , 80  , 25                            },
	{ TEXT_CreateIndirect      , ""                   , ID_DROPDOWN_STATUS , 10  , 145 , 360 , 18 , TEXT_CF_LEFT               },
	{ BUTTON_CreateIndirect    , "Close"              , GUI_ID_CANCEL      , 265 , 175 , 80  , 25                             }
};

static WM_PARAM _cbDropDownTest(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG: {
			auto pDropDown = (DROPDOWN_Obj *)WM_GetDialogItem(hWin, ID_DROPDOWN_TEST);
			for (auto i = 0; _DropDownInitialItems[i]; ++i) {
				pDropDown->AddString(_DropDownInitialItems[i]);
			}
			pDropDown->SetSel(0);
			pDropDown->SetItemSpacing(4);
			_DropDownAutoScroll = false;
			pDropDown->SetAutoScroll(0);
			_DropDownItemCounter = 0;
			_UpdateDropDownStatus(hWin);
			break;
		}
		case WM_NOTIFY_PARENT: {
			auto pInfo = (WM_NOTIFY_INFO *)Data;
			int Id = WM_GetId(pInfo->pWinSrc);
			auto pDropDown = (DROPDOWN_Obj *)WM_GetDialogItem(hWin, ID_DROPDOWN_TEST);
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_SEL_CHANGED:
					if (Id == ID_DROPDOWN_TEST) {
						_UpdateDropDownStatus(hWin);
					}
					break;
				case WM_NOTIFICATION_RELEASED:
					switch (Id) {
						case ID_DROPDOWN_ADD: {
							char acLabel[32];
							sprintf(acLabel, "Item %d", ++_DropDownItemCounter);
							pDropDown->AddString(acLabel);
							_UpdateDropDownStatus(hWin);
							break;
						}
						case ID_DROPDOWN_INSERT: {
							char acLabel[32];
							int Sel = pDropDown->GetSel();
							int Num = pDropDown->GetNumItems();
							sprintf(acLabel, "Item %d", ++_DropDownItemCounter);
							if (Num > 0) {
								pDropDown->InsertString(acLabel, (unsigned)Sel);
							}
							else {
								pDropDown->AddString(acLabel);
							}
							_UpdateDropDownStatus(hWin);
							break;
						}
						case ID_DROPDOWN_DELETE: {
							int Num = pDropDown->GetNumItems();
							if (Num > 0) {
								int Sel = pDropDown->GetSel();
								pDropDown->DeleteItem((unsigned)Sel);
								Num = pDropDown->GetNumItems();
								if (Num > 0 && Sel >= Num) {
									pDropDown->SetSel(Num - 1);
								}
								else if (Num == 0) {
									pDropDown->SetSel(0);
								}
							}
							_UpdateDropDownStatus(hWin);
							break;
						}
						case ID_DROPDOWN_NEXT:
							pDropDown->IncSel();
							_UpdateDropDownStatus(hWin);
							break;
						case ID_DROPDOWN_PREV:
							pDropDown->DecSel();
							_UpdateDropDownStatus(hWin);
							break;
						case ID_DROPDOWN_AUTOSCROLL:
							_DropDownAutoScroll = !_DropDownAutoScroll;
							pDropDown->SetAutoScroll(_DropDownAutoScroll ? 1 : 0);
							_UpdateDropDownStatus(hWin);
							break;
						case ID_DROPDOWN_EXPAND:
							pDropDown->Expand();
							break;
						case ID_DROPDOWN_COLLAPSE:
							pDropDown->Collapse();
							break;
						case GUI_ID_CANCEL:
							GUI_EndDialog(hWin, 0);
							break;
					}
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}

void _TestDropDown() {
	WM_Obj *hDialog = GUI_CreateDialogBox(_aDropDownDialogCreate, GUI_COUNTOF(_aDropDownDialogCreate), &_cbDropDownTest, 0, 0, 0);
	WM_DIALOG_STATUS DialogStatus = { 0 };
	GUI_SetDialogStatusPtr(hDialog, &DialogStatus);
	while (!DialogStatus.Done) {
		GUI_Exec();
	}
}

#define ID_MULTIPAGE_TEST          (GUI_ID_USER + 140)
#define ID_MULTIPAGE_STATUS        (GUI_ID_USER + 141)
#define ID_MULTIPAGE_ADD           (GUI_ID_USER + 142)
#define ID_MULTIPAGE_DELETE        (GUI_ID_USER + 143)
#define ID_MULTIPAGE_NEXT          (GUI_ID_USER + 144)
#define ID_MULTIPAGE_PREV          (GUI_ID_USER + 145)
#define ID_MULTIPAGE_TOGGLE_ENABLE (GUI_ID_USER + 146)
#define ID_MULTIPAGE_RENAME        (GUI_ID_USER + 147)

static int _MultiPagePageCount = 0;
static int _MultiPageNewPageIndex = 0;

static WM_Obj *_CreateMultiPagePage(MULTIPAGE_Obj *pMultiPage, const char *pLabel) {
	char acText[96];
	sprintf(acText, "Content: %s", pLabel);
	return TEXT_CreateAsChild(8, 8, 300, 20, pMultiPage, 0, WM_CF_SHOW, acText, TEXT_CF_LEFT);
}

static void _UpdateMultiPageStatus(WM_Obj *hWin) {
	auto pMultiPage = (MULTIPAGE_Obj *)WM_GetDialogItem(hWin, ID_MULTIPAGE_TEST);
	auto pStatus = (TEXT_Obj *)WM_GetDialogItem(hWin, ID_MULTIPAGE_STATUS);
	if (pMultiPage && pStatus) {
		char acStatus[96];
		int Sel = pMultiPage->GetSelection();
		int ValidSel = ((Sel >= 0) && (Sel < _MultiPagePageCount)) ? Sel : -1;
		int Enabled = (ValidSel >= 0) ? pMultiPage->IsPageEnabled((unsigned)ValidSel) : 0;
		sprintf(acStatus, "Pages: %d  Sel: %d  Enabled: %s", _MultiPagePageCount, ValidSel, Enabled ? "Yes" : "No");
		pStatus->SetText(acStatus);
	}
}

static const GUI_WIDGET_CREATE_INFO _aMultiPageDialogCreate[] = {
	{ FRAMEWIN_CreateIndirect  , "MultiPage Test"     , 0                         , 40  , 40  , 420 , 270 , FRAMEWIN_CF_MOVEABLE },
	{ MULTIPAGE_CreateIndirect , ""                   , ID_MULTIPAGE_TEST         , 10  , 10  , 390 , 150 , 0                  },
	{ BUTTON_CreateIndirect    , "Add Page"           , ID_MULTIPAGE_ADD          , 10  , 170 , 90  , 25                      },
	{ BUTTON_CreateIndirect    , "Delete"             , ID_MULTIPAGE_DELETE       , 105 , 170 , 80  , 25                      },
	{ BUTTON_CreateIndirect    , "Next"               , ID_MULTIPAGE_NEXT         , 190 , 170 , 65  , 25                      },
	{ BUTTON_CreateIndirect    , "Prev"               , ID_MULTIPAGE_PREV         , 260 , 170 , 65  , 25                      },
	{ BUTTON_CreateIndirect    , "Enable/Disable"     , ID_MULTIPAGE_TOGGLE_ENABLE, 10  , 200 , 120 , 25                      },
	{ BUTTON_CreateIndirect    , "Rename"             , ID_MULTIPAGE_RENAME       , 135 , 200 , 80  , 25                      },
	{ TEXT_CreateIndirect      , ""                   , ID_MULTIPAGE_STATUS       , 10  , 232 , 310 , 18  , TEXT_CF_LEFT      },
	{ BUTTON_CreateIndirect    , "Close"              , GUI_ID_CANCEL             , 330 , 225 , 70  , 25                      }
};

static WM_PARAM _cbMultiPageTest(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG: {
			auto pMultiPage = (MULTIPAGE_Obj *)WM_GetDialogItem(hWin, ID_MULTIPAGE_TEST);
			const char *aTitles[] = { "Home", "Settings", "About", nullptr };
			_MultiPagePageCount = 0;
			_MultiPageNewPageIndex = 0;
			for (int i = 0; aTitles[i]; ++i) {
				auto pPage = _CreateMultiPagePage(pMultiPage, aTitles[i]);
				pMultiPage->AddPage(pPage, aTitles[i]);
				_MultiPagePageCount++;
			}
			pMultiPage->SelectPage(0);
			_UpdateMultiPageStatus(hWin);
			return 0;
		}
		case WM_NOTIFY_PARENT: {
			auto pInfo = (WM_NOTIFY_INFO *)Data;
			int Id = WM_GetId(pInfo->pWinSrc);
			auto pMultiPage = (MULTIPAGE_Obj *)WM_GetDialogItem(hWin, ID_MULTIPAGE_TEST);
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_SEL_CHANGED:
					if (Id == ID_MULTIPAGE_TEST) {
						_UpdateMultiPageStatus(hWin);
					}
					break;
				case WM_NOTIFICATION_RELEASED:
					switch (Id) {
						case ID_MULTIPAGE_ADD: {
							char acTitle[32];
							sprintf(acTitle, "Page %d", ++_MultiPageNewPageIndex);
							auto pPage = _CreateMultiPagePage(pMultiPage, acTitle);
							pMultiPage->AddPage(pPage, acTitle);
							_MultiPagePageCount++;
							_UpdateMultiPageStatus(hWin);
							break;
						}
						case ID_MULTIPAGE_DELETE:
							if (_MultiPagePageCount > 0) {
								int Sel = pMultiPage->GetSelection();
								if (Sel >= 0 && Sel < _MultiPagePageCount) {
									pMultiPage->DeletePage((unsigned)Sel, 1);
									_MultiPagePageCount--;
								}
							}
							_UpdateMultiPageStatus(hWin);
							break;
						case ID_MULTIPAGE_NEXT:
							if (_MultiPagePageCount > 0) {
								int Sel = pMultiPage->GetSelection();
								int Next = (Sel + 1) % _MultiPagePageCount;
								pMultiPage->SelectPage((unsigned)Next);
							}
							_UpdateMultiPageStatus(hWin);
							break;
						case ID_MULTIPAGE_PREV:
							if (_MultiPagePageCount > 0) {
								int Sel = pMultiPage->GetSelection();
								int Prev = (Sel <= 0) ? (_MultiPagePageCount - 1) : (Sel - 1);
								pMultiPage->SelectPage((unsigned)Prev);
							}
							_UpdateMultiPageStatus(hWin);
							break;
						case ID_MULTIPAGE_TOGGLE_ENABLE:
							if (_MultiPagePageCount > 0) {
								int Sel = pMultiPage->GetSelection();
								if (Sel >= 0 && Sel < _MultiPagePageCount) {
									if (pMultiPage->IsPageEnabled((unsigned)Sel)) {
										pMultiPage->DisablePage((unsigned)Sel);
									}
									else {
										pMultiPage->EnablePage((unsigned)Sel);
									}
								}
							}
							_UpdateMultiPageStatus(hWin);
							break;
						case ID_MULTIPAGE_RENAME:
							if (_MultiPagePageCount > 0) {
								int Sel = pMultiPage->GetSelection();
								if (Sel >= 0 && Sel < _MultiPagePageCount) {
									char acTitle[32];
									sprintf(acTitle, "Renamed %d", Sel);
									pMultiPage->SetText(acTitle, (unsigned)Sel);
								}
							}
							_UpdateMultiPageStatus(hWin);
							break;
						case GUI_ID_CANCEL:
							GUI_EndDialog(hWin, 0);
							break;
					}
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}

void _TestMultiPage() {
	auto hDialog = GUI_CreateDialogBox(_aMultiPageDialogCreate, GUI_COUNTOF(_aMultiPageDialogCreate), &_cbMultiPageTest, 0, 0, 0);
	WM_DIALOG_STATUS DialogStatus = { 0 };
	GUI_SetDialogStatusPtr(hDialog, &DialogStatus);
	while (!DialogStatus.Done) {
		GUI_Exec();
	}
}

#define ID_RADIO_TEST      (GUI_ID_USER + 160)
#define ID_RADIO_STATUS    (GUI_ID_USER + 161)
#define ID_RADIO_PREV      (GUI_ID_USER + 162)
#define ID_RADIO_NEXT      (GUI_ID_USER + 163)
#define ID_RADIO_SET0      (GUI_ID_USER + 164)
#define ID_RADIO_SET1      (GUI_ID_USER + 165)
#define ID_RADIO_SET2      (GUI_ID_USER + 166)

static void _UpdateRadioStatus(WM_Obj *hWin) {
	auto hRadio = (RADIO_Handle)WM_GetDialogItem(hWin, ID_RADIO_TEST);
	auto pStatus = (TEXT_Obj *)WM_GetDialogItem(hWin, ID_RADIO_STATUS);
	if (hRadio && pStatus) {
		char acStatus[64];
		sprintf(acStatus, "Selected: %d", RADIO_GetValue(hRadio));
		pStatus->SetText(acStatus);
	}
}

static const GUI_WIDGET_CREATE_INFO _aRadioDialogCreate[] = {
	{ FRAMEWIN_CreateIndirect, "Radio Test", 0, 60, 50, 360, 220, FRAMEWIN_CF_MOVEABLE },
	{ RADIO_CreateIndirect, "", ID_RADIO_TEST, 10, 10, 200, 80, 0, (3 | (24 << 8)) },
	{ BUTTON_CreateIndirect, "Prev", ID_RADIO_PREV, 10, 100, 70, 25 },
	{ BUTTON_CreateIndirect, "Next", ID_RADIO_NEXT, 85, 100, 70, 25 },
	{ BUTTON_CreateIndirect, "Set 0", ID_RADIO_SET0, 160, 100, 60, 25 },
	{ BUTTON_CreateIndirect, "Set 1", ID_RADIO_SET1, 225, 100, 60, 25 },
	{ BUTTON_CreateIndirect, "Set 2", ID_RADIO_SET2, 290, 100, 60, 25 },
	{ TEXT_CreateIndirect, "", ID_RADIO_STATUS, 10, 140, 220, 18, TEXT_CF_LEFT },
	{ BUTTON_CreateIndirect, "Close", GUI_ID_CANCEL, 270, 170, 80, 25 }
};

static WM_PARAM _cbRadioTest(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG: {
			auto hRadio = (RADIO_Handle)WM_GetDialogItem(hWin, ID_RADIO_TEST);
			RADIO_SetText(hRadio, "Option A", 0);
			RADIO_SetText(hRadio, "Option B", 1);
			RADIO_SetText(hRadio, "Option C", 2);
			RADIO_SetValue(hRadio, 0);
			_UpdateRadioStatus(hWin);
			return 0;
		}
		case WM_NOTIFY_PARENT: {
			auto pInfo = (WM_NOTIFY_INFO *)Data;
			int Id = WM_GetId(pInfo->pWinSrc);
			auto hRadio = (RADIO_Handle)WM_GetDialogItem(hWin, ID_RADIO_TEST);
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_VALUE_CHANGED:
					if (Id == ID_RADIO_TEST) {
						_UpdateRadioStatus(hWin);
					}
					break;
				case WM_NOTIFICATION_RELEASED:
					switch (Id) {
						case ID_RADIO_PREV:
							RADIO_Dec(hRadio);
							_UpdateRadioStatus(hWin);
							break;
						case ID_RADIO_NEXT:
							RADIO_Inc(hRadio);
							_UpdateRadioStatus(hWin);
							break;
						case ID_RADIO_SET0:
							RADIO_SetValue(hRadio, 0);
							_UpdateRadioStatus(hWin);
							break;
						case ID_RADIO_SET1:
							RADIO_SetValue(hRadio, 1);
							_UpdateRadioStatus(hWin);
							break;
						case ID_RADIO_SET2:
							RADIO_SetValue(hRadio, 2);
							_UpdateRadioStatus(hWin);
							break;
						case GUI_ID_CANCEL:
							GUI_EndDialog(hWin, 0);
							break;
					}
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}

void _TestRadio() {
	auto hDialog = GUI_CreateDialogBox(_aRadioDialogCreate, GUI_COUNTOF(_aRadioDialogCreate), &_cbRadioTest, 0, 0, 0);
	WM_DIALOG_STATUS DialogStatus = { 0 };
	GUI_SetDialogStatusPtr(hDialog, &DialogStatus);
	while (!DialogStatus.Done) {
		GUI_Exec();
	}
}

#define ID_PROGBAR_TEST        (GUI_ID_USER + 180)
#define ID_PROGBAR_STATUS      (GUI_ID_USER + 181)
#define ID_PROGBAR_DEC         (GUI_ID_USER + 182)
#define ID_PROGBAR_INC         (GUI_ID_USER + 183)
#define ID_PROGBAR_RESET       (GUI_ID_USER + 184)
#define ID_PROGBAR_TOGGLE_TEXT (GUI_ID_USER + 185)

static int _ProgBarMin = 0;
static int _ProgBarMax = 100;
static int _ProgBarValue = 0;
static bool _ProgBarCustomText = false;

static void _UpdateProgBarStatus(WM_Obj *hWin) {
	auto pStatus = (TEXT_Obj *)WM_GetDialogItem(hWin, ID_PROGBAR_STATUS);
	if (pStatus) {
		char acStatus[80];
		sprintf(acStatus, "Range: %d-%d  Value: %d", _ProgBarMin, _ProgBarMax, _ProgBarValue);
		pStatus->SetText(acStatus);
	}
}

static const GUI_WIDGET_CREATE_INFO _aProgBarDialogCreate[] = {
	{ FRAMEWIN_CreateIndirect, "ProgBar Test", 0, 70, 60, 360, 210, FRAMEWIN_CF_MOVEABLE },
	{ PROGBAR_CreateIndirect, "", ID_PROGBAR_TEST, 15, 20, 320, 25, 0 },
	{ BUTTON_CreateIndirect, "-10", ID_PROGBAR_DEC, 15, 60, 60, 25 },
	{ BUTTON_CreateIndirect, "+10", ID_PROGBAR_INC, 80, 60, 60, 25 },
	{ BUTTON_CreateIndirect, "Reset", ID_PROGBAR_RESET, 145, 60, 70, 25 },
	{ BUTTON_CreateIndirect, "Text On/Off", ID_PROGBAR_TOGGLE_TEXT, 220, 60, 115, 25 },
	{ TEXT_CreateIndirect, "", ID_PROGBAR_STATUS, 15, 100, 320, 18, TEXT_CF_LEFT },
	{ BUTTON_CreateIndirect, "Close", GUI_ID_CANCEL, 255, 135, 80, 25 }
};

static WM_PARAM _cbProgBarTest(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG: {
			auto pProg = (PROGBAR_Obj *)WM_GetDialogItem(hWin, ID_PROGBAR_TEST);
			_ProgBarMin = 0;
			_ProgBarMax = 100;
			_ProgBarValue = 30;
			_ProgBarCustomText = false;
			pProg->SetMinMax(_ProgBarMin, _ProgBarMax);
			pProg->SetValue(_ProgBarValue);
			pProg->SetTextAlign(TEXTALIGN_HCENTER);
			pProg->SetText(nullptr);
			_UpdateProgBarStatus(hWin);
			return 0;
		}
		case WM_NOTIFY_PARENT: {
			auto pInfo = (WM_NOTIFY_INFO *)Data;
			int Id = WM_GetId(pInfo->pWinSrc);
			auto pProg = (PROGBAR_Obj *)WM_GetDialogItem(hWin, ID_PROGBAR_TEST);
			if (pInfo->Notification == WM_NOTIFICATION_RELEASED) {
				switch (Id) {
					case ID_PROGBAR_DEC:
						_ProgBarValue -= 10;
						if (_ProgBarValue < _ProgBarMin) {
							_ProgBarValue = _ProgBarMin;
						}
						pProg->SetValue(_ProgBarValue);
						_UpdateProgBarStatus(hWin);
						break;
					case ID_PROGBAR_INC:
						_ProgBarValue += 10;
						if (_ProgBarValue > _ProgBarMax) {
							_ProgBarValue = _ProgBarMax;
						}
						pProg->SetValue(_ProgBarValue);
						_UpdateProgBarStatus(hWin);
						break;
					case ID_PROGBAR_RESET:
						_ProgBarValue = 0;
						pProg->SetValue(_ProgBarValue);
						_UpdateProgBarStatus(hWin);
						break;
					case ID_PROGBAR_TOGGLE_TEXT:
						_ProgBarCustomText = !_ProgBarCustomText;
						if (_ProgBarCustomText) {
							pProg->SetText("Loading");
						}
						else {
							pProg->SetText(nullptr);
						}
						_UpdateProgBarStatus(hWin);
						break;
					case GUI_ID_CANCEL:
						GUI_EndDialog(hWin, 0);
						break;
				}
			}
			return 0;
		}
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}

void _TestProgBar() {
	auto hDialog = GUI_CreateDialogBox(_aProgBarDialogCreate, GUI_COUNTOF(_aProgBarDialogCreate), &_cbProgBarTest, 0, 0, 0);
	WM_DIALOG_STATUS DialogStatus = { 0 };
	GUI_SetDialogStatusPtr(hDialog, &DialogStatus);
	while (!DialogStatus.Done) {
		GUI_Exec();
	}
}

int main(void) {
	GUI_Init();
	GUI_CURSOR_Show();
	WM_SetDesktopColor(RGB_GRAY);

	_TestListView();

	_TestMultiPage();

	_TestRadio();

	_TestProgBar();

	_TestDropDown();

	for (;;) {
		auto hDialog = (FRAMEWIN_Obj *)GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);
		hDialog->AddMinButton();
		hDialog->AddMaxButton();
		_CreateMenu(hDialog);
		_MemDevPhase = 0;
		_hMemDevFrame = _CreateMemDevFrame(280, 50, "MemDev ON", 1, &_hMemDevPane);
		_hNoMemDevFrame = _CreateMemDevFrame(480, 50, "MemDev OFF", 0, &_hNoMemDevPane);

		WM_DIALOG_STATUS DialogStatus = { 0 };
		/* Let window know how to send feedback (close info & return value) */
		GUI_SetDialogStatusPtr(hDialog, &DialogStatus);
		while (!DialogStatus.Done) {
			_MemDevPhase = GUI_GetTime() / 20;
			if (_hMemDevPane)
				WM_Invalidate(_hMemDevPane);
			if (_hNoMemDevPane)
				WM_Invalidate(_hNoMemDevPane);
			GUI_Exec();
		}
		if (_hMemDevFrame)
			WM_DeleteWindow(_hMemDevFrame);
		if (_hNoMemDevFrame)
			WM_DeleteWindow(_hNoMemDevFrame);
		_hMemDevFrame = 0;
		_hMemDevPane = 0;
		_hNoMemDevFrame = 0;
		_hNoMemDevPane = 0;
	}
	return 0;
}
