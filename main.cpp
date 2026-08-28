#include <stdio.h>

#include "GUI.h"
#include "WM.h"

import TUX.Widget;
import TUX.Widget.CheckBox;
import TUX.Widget.Menu;
import TUX.Widget.ListView;
import TUX.Widget.ListBox;
import TUX.Widget.Frame;
import TUX.Widget.Text;
import TUX.Widget.DropDown;
import TUX.Widget.MultPage;
import TUX.Widget.Radio;
import TUX.Widget.ProgBar;
import TUX.Widget.Slider;
import TUX.Widget.Edit;
import TUX.Widget.MultEdit;
import TUX.Core.Timer;

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

static const char *_ListBox[]{
  "English", "Deutsch", nullptr
};
#define GUI_ID_MULTIEDIT0  GUI_ID_USER + 0x00
#define GUI_ID_CHECK0      GUI_ID_USER + 0x01
#define GUI_ID_CHECK1      GUI_ID_USER + 0x02
static const Widget::CreateStruct _aDialogCreate[]{
	{ Frame   ::CreateIndirect, "Owner drawn list box" , 0                 , 50  , 50  , 220  , 175  , FRAMEWIN_CF_MOVEABLE | FRAMEWIN_CF_RESIZEABLE },
	{ ListBox ::CreateIndirect, ""                     , GUI_ID_MULTIEDIT0 , 10  , 10  , 100  , 100  , 0                    , 100 },
	{ CheckBox::CreateIndirect, ""                     , GUI_ID_CHECK0     , 120 , 10  , 0    , 0                                 },
	{ Text    ::CreateIndirect, "Multi select"         , 0                 , 140 , 10  , 80   , 15   , TEXT_CF_LEFT               },
	{ CheckBox::CreateIndirect, ""                     , GUI_ID_CHECK1     , 120 , 35  , 0    , 0                                 },
	{ Text    ::CreateIndirect, "Owner drawn"          , 0                 , 140 , 35  , 80   , 15   , TEXT_CF_LEFT               },
	{ Button  ::CreateIndirect, "OK"                   , GUI_ID_OK         , 120 , 65  , 80   , 20                                },
	{ Button  ::CreateIndirect, "Cancel"               , GUI_ID_CANCEL     , 120 , 90  , 80   , 20                                }
};

static int _GetItemSizeY(ListBox *pObj, int ItemIndex) {
	int DistY = pObj->GetFont()->DistY() + 1;
	if (pObj->GetMulti()) {
		if (pObj->GetItemSel(ItemIndex))
			DistY += 8;
	}
	else if (pObj->GetSel() == ItemIndex)
		DistY += 8;
	return DistY;
}

static int _OwnerDraw(WObj *pWin, int Cmd, int Index, POINT ItemPos) {
	auto pObj = (ListBox *)pWin;
	switch (Cmd) {
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
			bool IsDisabled = pObj->GetItemDisabled(Index);
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
			else if (Index == Sel)
				ColorIndex = pObj->HasFocus() ? 2 : 1;
			else
				ColorIndex = 0;
			/* Draw item */
			GUI.SetBkColor(aBkColor[ColorIndex]);
			GUI.SetColor(aColor[ColorIndex]);
			pObj->GetItemText(Index, acBuffer, sizeof(acBuffer));
			GUI_Clear();
			auto FontDistY = GUI.GetFont()->DistY();
			GUI_DispStringAt(acBuffer, ItemPos.x + bmSmilie0.XSize + 16, ItemPos.y + (YSize - FontDistY) / 2);
			/* Draw bitmap */
			auto pBm = MultiSel ? IsSelected ? &bmSmilie1 : &bmSmilie0 : (Index == Sel) ? &bmSmilie1 : &bmSmilie0;
			GUI_DrawBitmap(pBm, ItemPos.x + 7, ItemPos.y + (YSize - pBm->YSize) / 2);
			/* Draw focus rectangle */
			if (MultiSel && Index == Sel) {
				auto rInside = pObj->GetInsideRect();
				RECT rFocus;
				rFocus.x0 = ItemPos.x;
				rFocus.y0 = ItemPos.y;
				rFocus.x1 = rInside.x1;
				rFocus.y1 = ItemPos.y + YSize - 1;
				GUI.SetColor(RGB_WHITE - aBkColor[ColorIndex]);
				GUI_DrawFocusRect(rFocus, 0);
			}
			break;
		}
		default:
			return ListBox::OwnerDraw(pWin, Cmd, Index, ItemPos);
	}
	return 0;
}


static WObj *_hMemDevFrame;
static WObj *_hMemDevPane;
static WObj *_hNoMemDevFrame;
static WObj *_hNoMemDevPane;
static int _MemDevPhase;
static Timer *_pMemDevTimer;

void _TestEdit();

static void _OnMemDevTimer(GUI_TIMER_MESSAGE *pTM) {
	(void)pTM;
	++_MemDevPhase;
	if (_hMemDevPane)
		_hMemDevPane->Invalidate();
	if (_hNoMemDevPane)
		_hNoMemDevPane->Invalidate();
	if (_pMemDevTimer)
		_pMemDevTimer->SetTime(GUI_GetTime() + 20);
}

static WM_PARAM _cbMemDevPane(WObj *pWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_PAINT:
		{
			auto Size = pWin->GetSize();
			int BarWidth = 36;
			int Span = Size.x - BarWidth - 20;
			int XPos = 0;
			int MemDevOn = (pWin == _hMemDevPane);
			if (Span > 0) {
				int Phase = _MemDevPhase % (Span * 2);
				XPos = Phase > Span ? Span * 2 - Phase : Phase;
			}
			GUI.SetBkColor(RGB_WHITE);
			GUI_Clear();
			GUI.SetColor(RGB_DARKGRAY);
			GUI_DrawRect({ 0, 0, Size.x - 1, Size.y - 1 });
			GUI.SetColor(RGB_BLACK);
			GUI_DispStringAt(MemDevOn ? "MemDev ON" : "MemDev OFF", 8, 8);
			GUI_DispStringAt(MemDevOn ? "WC_MEMDEV enabled" : "WC_MEMDEV disabled", 8, 24);
			GUI.SetColor(RGB_GRAY);
			GUI_DrawRect({ 10, 48, Size.x - 11, 72 });
			GUI.SetColor(MemDevOn ? RGB_GREEN : RGB_RED);
			GUI_FillRect({ 10 + XPos, 49, 10 + XPos + BarWidth, 71 });
			GUI.SetColor(RGB_BLUE);
			GUI_FillRect({ 10, Size.y - 40, Size.x - 11, Size.y - 25 });
			GUI.SetColor(RGB_YELLOW);
			GUI_FillRect({ 10 + XPos / 2, Size.y - 39, 35 + XPos / 2, Size.y - 26 });
			GUI.SetColor(RGB_BLACK);
			GUI_DispStringAt("Animated redraw area", 8, Size.y - 18);
			return 0;
		}
	}
	return WM_DefaultProc(pWin, MsgId, Data);
}

static WObj *_CreateMemDevFrame(int x0, int y0, const char *pTitle, int UseMemDev, WObj **phPane) {
	auto Flags = WC_VISIBLE;
	if (UseMemDev)
		Flags |= WC_MEMDEV;
	auto pFrame = new Frame(RECT::LeftTop({ x0, y0 }, { 190, 180 }), WC_VISIBLE, nullptr, 0, FRAMEWIN_CF_MOVEABLE, pTitle, nullptr);
	auto pClient = pFrame->Client();
	auto Size = pClient->GetSize();
	*phPane = new WObj(RECT(0, 0, Size.x - 1, Size.y - 1), Flags, _cbMemDevPane, pClient);
	return pFrame;
}

static WM_PARAM _cbCallback(WObj *pWin, int MsgId, WM_PARAM Data) {
	CheckBox *pItem;
	auto pListBox = pWin->GetItem<ListBox>(GUI_ID_MULTIEDIT0);
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
			pItem = pWin->GetItem<CheckBox>(GUI_ID_CHECK1);
			pItem->SetState(1);
			return 0;
		case WM_KEY: {
			const WM_KEY_INFO *pInfo = (const WM_KEY_INFO *)Data;
			switch (pInfo->Key) {
				case GUI_KEY_ESCAPE:
					pWin->DialogEnd(1);
					break;
				case GUI_KEY_ENTER:
					pWin->DialogEnd(0);
					break;
			}
			return 0;
		}
		case WM_TOUCH_CHILD:
			//WM_SetFocus(hListBox);
			return 0;
		case WM_NOTIFY_PARENT: {
			auto pInfo = (const NOTIFY_INFO *)Data;
			auto pWinSrc = pInfo->pWinSrc;
			int Id = pWinSrc->GetID(); /* Id of widget */
			pItem = pWin->GetItem<CheckBox>(Id);
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_SEL_CHANGED:
					pListBox->InvalidateItem(LISTBOX_ALL_ITEMS);
					break;
				case WM_NOTIFICATION_RELEASED: /* React only if released */
					switch (Id) {
						case GUI_ID_OK:
							pWin->DialogEnd(0);
							break;
						case GUI_ID_CANCEL:
							pWin->DialogEnd(1);
							break;
						case GUI_ID_CHECK0:
							_MultiSel = !_MultiSel;
							pListBox->SetMulti(_MultiSel);
							pListBox->SetFocus();
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
						case (GUI_ID_USER + 16):
							_TestEdit();
							break;
					}
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(pWin, MsgId, Data);
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
#define ID_MENU_TEST_EDIT_INTERACTIVE (GUI_ID_USER + 16)

static void _AddMenuItem(Menu *pMenu, Menu *pSubmenu, const char *pText, uint16_t Id, uint16_t Flags) {
	Menu::ItemData Item;
	Item.pText = pText;
	Item.pSubmenu = pSubmenu;
	Item.Flags = Flags;
	Item.Id = Id;
	pMenu->AddItem(&Item);
}
static void _CreateMenu(Frame *pParent) {
	//
	// Create main menu
	//
	auto pMenu = new Menu(MENU_CF_HORIZONTAL, ID_MENU);
	//
	// Create sub menus
	//
	auto pMenuFile = new Menu(MENU_CF_VERTICAL, 0);
	auto pMenuEdit = new Menu(MENU_CF_VERTICAL, 0);
	auto pMenuHelp = new Menu(MENU_CF_VERTICAL, 0);
	auto pMenuRecent = new Menu(MENU_CF_VERTICAL, 0);
	auto pMenuTests = new Menu(MENU_CF_VERTICAL, 0);
	//
	// Add menu items to menu &#39;Recent&#39;
	//
	_AddMenuItem(pMenuRecent, 0, "File 1", ID_MENU_RECENT_0, 0);
	_AddMenuItem(pMenuRecent, 0, "File 2", ID_MENU_RECENT_1, 0);
	_AddMenuItem(pMenuRecent, 0, "File 3", ID_MENU_RECENT_2, 0);
	_AddMenuItem(pMenuRecent, 0, "File 4", ID_MENU_RECENT_3, 0);
	_AddMenuItem(pMenuRecent, 0, "File 4", ID_MENU_RECENT_3, 0);
	_AddMenuItem(pMenuRecent, 0, "File 4", ID_MENU_RECENT_3, 0);
	//
	// Add menu items to menu &#39;File&#39;
	//
	_AddMenuItem(pMenuFile, 0, "New", ID_MENU_FILE_NEW, 0);
	_AddMenuItem(pMenuFile, 0, "Open", ID_MENU_FILE_OPEN, 0);
	_AddMenuItem(pMenuFile, 0, "Close", ID_MENU_FILE_CLOSE, MENU_IF_DISABLED);
	_AddMenuItem(pMenuFile, 0, 0, 0, MENU_IF_SEPARATOR);
	_AddMenuItem(pMenuFile, pMenuRecent, "Files...", ID_MENU_FILE_RECENT, 0);
	_AddMenuItem(pMenuFile, 0, 0, 0, MENU_IF_SEPARATOR);
	_AddMenuItem(pMenuFile, 0, "Exit", ID_MENU_FILE_EXIT, 0);
	//
	// Add menu items to menu &#39;Edit&#39;
	//
	_AddMenuItem(pMenuEdit, 0, "Undo", ID_MENU_EDIT_UNDO, 0);
	_AddMenuItem(pMenuEdit, 0, "Redo", ID_MENU_EDIT_REDO, 0);
	_AddMenuItem(pMenuEdit, 0, 0, 0, MENU_IF_SEPARATOR);
	_AddMenuItem(pMenuEdit, 0, "Copy", ID_MENU_EDIT_COPY, 0);
	_AddMenuItem(pMenuEdit, 0, "Paste", ID_MENU_EDIT_PASTE, 0);
	_AddMenuItem(pMenuEdit, 0, "Delete", ID_MENU_EDIT_DELETE, 0);
	//
	// Add menu items to menu &#39;Help&#39;
	//
	_AddMenuItem(pMenuHelp, 0, "About", ID_MENU_HELP_ABOUT, 0);
	//
	// Add menu items to menu &#39;Tests&#39;
	//
	_AddMenuItem(pMenuTests, 0, "Edit Interactive", ID_MENU_TEST_EDIT_INTERACTIVE, 0);
	//
	// Add menu items to main menu
	//
	_AddMenuItem(pMenu, pMenuFile, "File", 0, 0);
	_AddMenuItem(pMenu, pMenuEdit, "Edit", 0, 0);
	_AddMenuItem(pMenu, pMenuHelp, "Help", 0, 0);
	_AddMenuItem(pMenu, pMenuTests, "Tests", 0, 0);
	//
	// Attach menu to parent window
	//
	pParent->AddMenu(pMenu);
}

void _TestListBox() {
	auto pDialog = (Frame *)_aDialogCreate->CreateDialog(GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);
	pDialog->AddMinButton();
	pDialog->AddMaxButton();
	_CreateMenu(pDialog);
	pDialog->DialogExec();
}

static const Widget::CreateStruct _aMemDevDialogCreate[] = {
	{ Frame ::CreateIndirect, "MemDev Test"                           , 0             , 80  , 260 , 460 , 90                     , FRAMEWIN_CF_MOVEABLE },
	{ Text  ::CreateIndirect, "Compare redraw with and without MemDev", 0             , 10  , 10  , 310 , 16                     , TEXT_CF_LEFT         },
	{ Button::CreateIndirect, "Close"                                 , GUI_ID_CANCEL , 370 , 35  , 70  , 20                                        }
};

static WM_PARAM _cbMemDevTest(WObj *pWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_KEY: {
			const WM_KEY_INFO *pInfo = (const WM_KEY_INFO *)Data;
			switch (pInfo->Key) {
				case GUI_KEY_ESCAPE:
				case GUI_KEY_ENTER:
					pWin->DialogEnd(0);
					break;
			}
			return 0;
		}
		case WM_NOTIFY_PARENT: {
			auto pInfo = (const NOTIFY_INFO *)Data;
			if ((pInfo->Notification == WM_NOTIFICATION_RELEASED) && (pInfo->pWinSrc->GetID() == GUI_ID_CANCEL)) {
				pWin->DialogEnd(0);
				return 0;
			}
			break;
		}
	}
	return WM_DefaultProc(pWin, MsgId, Data);
}

void _TestMemDev() {
	_MemDevPhase = 0;
	auto pDialog = (Frame *)_aMemDevDialogCreate->CreateDialog(GUI_COUNTOF(_aMemDevDialogCreate), &_cbMemDevTest, 0, 0, 0);
	_hMemDevFrame = _CreateMemDevFrame(80, 50, "MemDev ON", 1, &_hMemDevPane);
	_hNoMemDevFrame = _CreateMemDevFrame(280, 50, "MemDev OFF", 0, &_hNoMemDevPane);
	_pMemDevTimer = new Timer(_OnMemDevTimer, GUI_GetTime() + 20);

	pDialog->DialogExec();

	if (_pMemDevTimer) {
		delete _pMemDevTimer;
		_pMemDevTimer = nullptr;
	}
	if (WObj::IsWindow(_hMemDevFrame))
		WM_DeleteWindow(_hMemDevFrame);
	if (WObj::IsWindow(_hNoMemDevFrame))
		WM_DeleteWindow(_hNoMemDevFrame);
	_hMemDevFrame = 0;
	_hMemDevPane = 0;
	_hNoMemDevFrame = 0;
	_hNoMemDevPane = 0;
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

static const Widget::CreateStruct _aListViewDialogCreate[] = {
	{ Frame   ::CreateIndirect, "ListView Test"      , 0                 , 50  , 50  , 320 , 240 , FRAMEWIN_CF_MOVEABLE       },
	{ ListView::CreateIndirect, ""                   , ID_LISTVIEW_TEST  , 10  , 10  , 290 , 160 , 0                          },
	{ Button  ::CreateIndirect, "Add Row"            , GUI_ID_USER + 101 , 10  , 180 , 80  , 25                               },
	{ Button  ::CreateIndirect, "Delete Row"         , GUI_ID_USER + 102 , 100 , 180 , 80  , 25                               },
	{ Button  ::CreateIndirect, "Close"              , GUI_ID_CANCEL     , 190 , 180 , 80  , 25                               }
};

static WM_PARAM _cbListViewTest(WObj *pWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG: {
			auto pListView = pWin->GetItem<ListView>(ID_LISTVIEW_TEST);
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
		case WM_NOTIFY_PARENT: {
			auto pInfo = (NOTIFY_INFO *)Data;
			auto pWinSrc = pInfo->pWinSrc;
			int Id = pWinSrc->GetID();
			auto pListView = pWin->GetItem<ListView>(ID_LISTVIEW_TEST);
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
							pWin->DialogEnd(0);
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
	return WM_DefaultProc(pWin, MsgId, Data);
}

void _TestListView() {
	auto pDialog = _aListViewDialogCreate->CreateDialog(GUI_COUNTOF(_aListViewDialogCreate), &_cbListViewTest, 0, 0, 0);
	pDialog->DialogExec();
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

static void _UpdateDropDownStatus(WObj *pWin) {
	auto pDropDown = pWin->GetItem<DropDown>(ID_DROPDOWN_TEST);
	auto pStatus = pWin->GetItem<Text>(ID_DROPDOWN_STATUS);
	if (pDropDown && pStatus) {
		char acStatus[96];
		sprintf(acStatus, "Items: %d  Sel: %d  AutoScroll: %s",
				pDropDown->GetNumItems(),
				pDropDown->GetSel(),
				_DropDownAutoScroll ? "On" : "Off");
		pStatus->SetText(acStatus);
	}
}

static const Widget::CreateStruct _aDropDownDialogCreate[] = {
	{ Frame   ::CreateIndirect, "DropDown Test"      , 0                      , 50  , 40  , 390 , 230 , FRAMEWIN_CF_MOVEABLE       },
	{ DropDown::CreateIndirect, ""                   , ID_DROPDOWN_TEST       , 10  , 10  , 220 , 96  , 0                          },
	{ Text    ::CreateIndirect, "Use the DropDown or buttons below to interact."
	                                                                            , 0   , 10  , 45  , 360 , 16 , TEXT_CF_LEFT          },
	{ Button  ::CreateIndirect, "Add Item"           , ID_DROPDOWN_ADD        , 10  , 70  , 80  , 25                               },
	{ Button  ::CreateIndirect, "Insert"             , ID_DROPDOWN_INSERT     , 95  , 70  , 80  , 25                               },
	{ Button  ::CreateIndirect, "Delete"             , ID_DROPDOWN_DELETE     , 180 , 70  , 80  , 25                               },
	{ Button  ::CreateIndirect, "Next"               , ID_DROPDOWN_NEXT       , 265 , 70  , 80  , 25                               },
	{ Button  ::CreateIndirect, "Prev"               , ID_DROPDOWN_PREV       , 10  , 105 , 80  , 25                               },
	{ Button  ::CreateIndirect, "AutoScroll"         , ID_DROPDOWN_AUTOSCROLL , 95  , 105 , 80  , 25                               },
	{ Button  ::CreateIndirect, "Expand"             , ID_DROPDOWN_EXPAND     , 180 , 105 , 80  , 25                               },
	{ Button  ::CreateIndirect, "Collapse"           , ID_DROPDOWN_COLLAPSE   , 265 , 105 , 80  , 25                               },
	{ Text    ::CreateIndirect, ""                   , ID_DROPDOWN_STATUS     , 10  , 145 , 360 , 18  , TEXT_CF_LEFT               },
	{ Button  ::CreateIndirect, "Close"              , GUI_ID_CANCEL          , 265 , 175 , 80  , 25                               }
};

static WM_PARAM _cbDropDownTest(WObj *pWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG: {
			auto pDropDown = pWin->GetItem<DropDown>(ID_DROPDOWN_TEST);
			for (auto i = 0; _DropDownInitialItems[i]; ++i) {
				pDropDown->AddString(_DropDownInitialItems[i]);
			}
			pDropDown->SetSel(0);
			pDropDown->SetItemSpacing(4);
			_DropDownAutoScroll = false;
			pDropDown->SetAutoScroll(0);
			_DropDownItemCounter = 0;
			_UpdateDropDownStatus(pWin);
			break;
		}
		case WM_NOTIFY_PARENT: {
			auto pInfo = (const NOTIFY_INFO *)Data;
			auto  pWinSrc = pInfo->pWinSrc;
			int Id = pWinSrc->GetID();
			auto pDropDown = pWin->GetItem<DropDown>(ID_DROPDOWN_TEST);
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_SEL_CHANGED:
					if (Id == ID_DROPDOWN_TEST) {
						_UpdateDropDownStatus(pWin);
					}
					break;
				case WM_NOTIFICATION_RELEASED:
					switch (Id) {
						case ID_DROPDOWN_ADD: {
							char acLabel[32];
							sprintf(acLabel, "Item %d", ++_DropDownItemCounter);
							pDropDown->AddString(acLabel);
							_UpdateDropDownStatus(pWin);
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
							_UpdateDropDownStatus(pWin);
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
							_UpdateDropDownStatus(pWin);
							break;
						}
						case ID_DROPDOWN_NEXT:
							pDropDown->IncSel();
							_UpdateDropDownStatus(pWin);
							break;
						case ID_DROPDOWN_PREV:
							pDropDown->DecSel();
							_UpdateDropDownStatus(pWin);
							break;
						case ID_DROPDOWN_AUTOSCROLL:
							_DropDownAutoScroll = !_DropDownAutoScroll;
							pDropDown->SetAutoScroll(_DropDownAutoScroll ? 1 : 0);
							_UpdateDropDownStatus(pWin);
							break;
						case ID_DROPDOWN_EXPAND:
							pDropDown->Expand();
							break;
						case ID_DROPDOWN_COLLAPSE:
							pDropDown->Collapse();
							break;
						case GUI_ID_CANCEL:
							pWin->DialogEnd(0);
							break;
					}
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(pWin, MsgId, Data);
}

void _TestDropDown() {
	auto pDialog = _aDropDownDialogCreate->CreateDialog(GUI_COUNTOF(_aDropDownDialogCreate), &_cbDropDownTest, 0, 0, 0);
	pDialog->DialogExec();
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

static WObj *_CreateMultiPagePage(MultPage *pMultiPage, const char *pLabel) {
	char acText[96];
	sprintf(acText, "Content: %s", pLabel);
	return new Text(RECT::LeftTop({ 8, 8 }, { 300, 20 }), WC_VISIBLE, pMultiPage, 0, TEXT_CF_LEFT, acText);
}

static void _UpdateMultiPageStatus(WObj *pWin) {
	auto pMultiPage = pWin->GetItem<MultPage>(ID_MULTIPAGE_TEST);
	auto pStatus = pWin->GetItem<Text>(ID_MULTIPAGE_STATUS);
	if (pMultiPage && pStatus) {
		char acStatus[96];
		int Sel = pMultiPage->GetSelection();
		int ValidSel = ((Sel >= 0) && (Sel < _MultiPagePageCount)) ? Sel : -1;
		int Enabled = (ValidSel >= 0) ? pMultiPage->IsPageEnabled((unsigned)ValidSel) : 0;
		sprintf(acStatus, "Pages: %d  Sel: %d  Enabled: %s", _MultiPagePageCount, ValidSel, Enabled ? "Yes" : "No");
		pStatus->SetText(acStatus);
	}
}

static const Widget::CreateStruct _aMultiPageDialogCreate[] = {
	{ Frame   ::CreateIndirect, "MultiPage Test"     , 0                         , 40  , 40  , 420 , 270 , FRAMEWIN_CF_MOVEABLE },
	{ MultPage::CreateIndirect, ""                   , ID_MULTIPAGE_TEST         , 10  , 10  , 390 , 150 , 0                    },
	{ Button  ::CreateIndirect, "Add Page"           , ID_MULTIPAGE_ADD          , 10  , 170 , 90  , 25                         },
	{ Button  ::CreateIndirect, "Delete"             , ID_MULTIPAGE_DELETE       , 105 , 170 , 80  , 25                         },
	{ Button  ::CreateIndirect, "Next"               , ID_MULTIPAGE_NEXT         , 190 , 170 , 65  , 25                         },
	{ Button  ::CreateIndirect, "Prev"               , ID_MULTIPAGE_PREV         , 260 , 170 , 65  , 25                         },
	{ Button  ::CreateIndirect, "Enable/Disable"     , ID_MULTIPAGE_TOGGLE_ENABLE, 10  , 200 , 120 , 25                         },
	{ Button  ::CreateIndirect, "Rename"             , ID_MULTIPAGE_RENAME       , 135 , 200 , 80  , 25                         },
	{ Text    ::CreateIndirect, ""                   , ID_MULTIPAGE_STATUS       , 10  , 232 , 310 , 18  , TEXT_CF_LEFT         },
	{ Button  ::CreateIndirect, "Close"              , GUI_ID_CANCEL             , 330 , 225 , 70  , 25                         }
};

static WM_PARAM _cbMultiPageTest(WObj *pWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG: {
			auto pMultiPage = pWin->GetItem<MultPage>(ID_MULTIPAGE_TEST);
			const char *aTitles[] = { "Home", "Settings", "About", nullptr };
			_MultiPagePageCount = 0;
			_MultiPageNewPageIndex = 0;
			for (int i = 0; aTitles[i]; ++i) {
				auto pPage = _CreateMultiPagePage(pMultiPage, aTitles[i]);
				pMultiPage->AddPage(pPage, aTitles[i]);
				_MultiPagePageCount++;
			}
			pMultiPage->SelectPage(0);
			_UpdateMultiPageStatus(pWin);
			return 0;
		}
		case WM_NOTIFY_PARENT: {
			auto pInfo = (const NOTIFY_INFO *)Data;
			auto pWinSrc = pInfo->pWinSrc;
			int Id = pWinSrc->GetID();
			auto pMultiPage = pWin->GetItem<MultPage>(ID_MULTIPAGE_TEST);
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_SEL_CHANGED:
					if (Id == ID_MULTIPAGE_TEST) {
						_UpdateMultiPageStatus(pWin);
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
							_UpdateMultiPageStatus(pWin);
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
							_UpdateMultiPageStatus(pWin);
							break;
						case ID_MULTIPAGE_NEXT:
							if (_MultiPagePageCount > 0) {
								int Sel = pMultiPage->GetSelection();
								int Next = (Sel + 1) % _MultiPagePageCount;
								pMultiPage->SelectPage((unsigned)Next);
							}
							_UpdateMultiPageStatus(pWin);
							break;
						case ID_MULTIPAGE_PREV:
							if (_MultiPagePageCount > 0) {
								int Sel = pMultiPage->GetSelection();
								int Prev = (Sel <= 0) ? (_MultiPagePageCount - 1) : (Sel - 1);
								pMultiPage->SelectPage((unsigned)Prev);
							}
							_UpdateMultiPageStatus(pWin);
							break;
						case ID_MULTIPAGE_TOGGLE_ENABLE:
							if (_MultiPagePageCount > 0) {
								int Sel = pMultiPage->GetSelection();
								if (Sel >= 0 && Sel < _MultiPagePageCount) {
									if (pMultiPage->IsPageEnabled((unsigned)Sel))
										pMultiPage->DisablePage((unsigned)Sel);
									else
										pMultiPage->EnablePage((unsigned)Sel);
								}
							}
							_UpdateMultiPageStatus(pWin);
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
							_UpdateMultiPageStatus(pWin);
							break;
						case GUI_ID_CANCEL:
							pWin->DialogEnd(0);
							break;
					}
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(pWin, MsgId, Data);
}

void _TestMultiPage() {
	auto pDialog = _aMultiPageDialogCreate->CreateDialog(GUI_COUNTOF(_aMultiPageDialogCreate), &_cbMultiPageTest, 0, 0, 0);
	pDialog->DialogExec();
}

#define ID_RADIO_TEST      (GUI_ID_USER + 160)
#define ID_RADIO_STATUS    (GUI_ID_USER + 161)
#define ID_RADIO_PREV      (GUI_ID_USER + 162)
#define ID_RADIO_NEXT      (GUI_ID_USER + 163)
#define ID_RADIO_SET0      (GUI_ID_USER + 164)
#define ID_RADIO_SET1      (GUI_ID_USER + 165)
#define ID_RADIO_SET2      (GUI_ID_USER + 166)

static void _UpdateRadioStatus(WObj *pWin) {
	auto pRadio = pWin->GetItem<Radio>(ID_RADIO_TEST);
	auto pStatus = pWin->GetItem<Text>(ID_RADIO_STATUS);
	if (pRadio && pStatus) {
		char acStatus[64];
		sprintf(acStatus, "Selected: %d", pRadio->GetValue());
		pStatus->SetText(acStatus);
	}
}

static const Widget::CreateStruct _aRadioDialogCreate[] = {
	{ Frame ::CreateIndirect, "Radio Test" , 0               , 60  , 50  , 360 , 220 , FRAMEWIN_CF_MOVEABLE },
	{ Radio ::CreateIndirect, ""           , ID_RADIO_TEST   , 10  , 10  , 200 , 80  , 0, (3 | (24 << 8))   },
	{ Button::CreateIndirect, "Prev"       , ID_RADIO_PREV   , 10  , 100 , 70  , 25                         },
	{ Button::CreateIndirect, "Next"       , ID_RADIO_NEXT   , 85  , 100 , 70  , 25                         },
	{ Button::CreateIndirect, "Set 0"      , ID_RADIO_SET0   , 160 , 100 , 60  , 25                         },
	{ Button::CreateIndirect, "Set 1"      , ID_RADIO_SET1   , 225 , 100 , 60  , 25                         },
	{ Button::CreateIndirect, "Set 2"      , ID_RADIO_SET2   , 290 , 100 , 60  , 25                         },
	{ Text  ::CreateIndirect, ""           , ID_RADIO_STATUS , 10  , 140 , 220 , 18  , TEXT_CF_LEFT         },
	{ Button::CreateIndirect, "Close"      , GUI_ID_CANCEL   , 270 , 170 , 80  , 25                         }
};

static WM_PARAM _cbRadioTest(WObj *pWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG: {
			auto pRadio = pWin->GetItem<Radio>(ID_RADIO_TEST);
			pRadio->SetText("Option A", 0);
			pRadio->SetText("Option B", 1);
			pRadio->SetText("Option C", 2);
			pRadio->SetValue(0);
			_UpdateRadioStatus(pWin);
			return 0;
		}
		case WM_NOTIFY_PARENT: {
			auto pInfo = (const NOTIFY_INFO *)Data;
			auto pWinSrc = pInfo->pWinSrc;
			int Id = pWinSrc->GetID();
			auto pRadio = pWin->GetItem<Radio>(ID_RADIO_TEST);
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_VALUE_CHANGED:
					if (Id == ID_RADIO_TEST) {
						_UpdateRadioStatus(pWin);
					}
					break;
				case WM_NOTIFICATION_RELEASED:
					switch (Id) {
						case ID_RADIO_PREV:
							pRadio->Dec();
							_UpdateRadioStatus(pWin);
							break;
						case ID_RADIO_NEXT:
							pRadio->Inc();
							_UpdateRadioStatus(pWin);
							break;
						case ID_RADIO_SET0:
							pRadio->SetValue(0);
							_UpdateRadioStatus(pWin);
							break;
						case ID_RADIO_SET1:
							pRadio->SetValue(1);
							_UpdateRadioStatus(pWin);
							break;
						case ID_RADIO_SET2:
							pRadio->SetValue(2);
							_UpdateRadioStatus(pWin);
							break;
						case GUI_ID_CANCEL:
							pWin->DialogEnd(0);
							break;
					}
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(pWin, MsgId, Data);
}

void _TestRadio() {
	auto pDialog = _aRadioDialogCreate->CreateDialog(GUI_COUNTOF(_aRadioDialogCreate), &_cbRadioTest, 0, 0, 0);
	pDialog->DialogExec();
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

static void _UpdateProgBarStatus(WObj *pWin) {
	auto pStatus = pWin->GetItem<Text>(ID_PROGBAR_STATUS);
	if (pStatus) {
		char acStatus[80];
		sprintf(acStatus, "Range: %d-%d  Value: %d", _ProgBarMin, _ProgBarMax, _ProgBarValue);
		pStatus->SetText(acStatus);
	}
}

static const Widget::CreateStruct _aProgBarDialogCreate[] = {
	{ Frame  ::CreateIndirect, "ProgBar Test" , 0                      , 70  , 60  , 360 , 210 , FRAMEWIN_CF_MOVEABLE },
	{ ProgBar::CreateIndirect, ""             , ID_PROGBAR_TEST        , 15  , 20  , 320 , 25  , 0                    },
	{ Button ::CreateIndirect, "-10"          , ID_PROGBAR_DEC         , 15  , 60  , 60  , 25                         },
	{ Button ::CreateIndirect, "+10"          , ID_PROGBAR_INC         , 80  , 60  , 60  , 25                         },
	{ Button ::CreateIndirect, "Reset"        , ID_PROGBAR_RESET       , 145 , 60  , 70  , 25                         },
	{ Button ::CreateIndirect, "Text On/Off"  , ID_PROGBAR_TOGGLE_TEXT , 220 , 60  , 115 , 25                         },
	{ Text   ::CreateIndirect, ""             , ID_PROGBAR_STATUS      , 15  , 100 , 320 , 18  , TEXT_CF_LEFT         },
	{ Button ::CreateIndirect, "Close"        , GUI_ID_CANCEL          , 255 , 135 , 80  , 25                         }
};

static WM_PARAM _cbProgBarTest(WObj *pWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG: {
			auto pProg = pWin->GetItem<ProgBar>(ID_PROGBAR_TEST);
			_ProgBarMin = 0;
			_ProgBarMax = 100;
			_ProgBarValue = 30;
			_ProgBarCustomText = false;
			pProg->SetMinMax(_ProgBarMin, _ProgBarMax);
			pProg->SetValue(_ProgBarValue);
			pProg->SetTextAlign(TEXTALIGN_HCENTER);
			pProg->SetText(nullptr);
			_UpdateProgBarStatus(pWin);
			return 0;
		}
		case WM_NOTIFY_PARENT: {
			auto pInfo = (const NOTIFY_INFO *)Data;
			auto pWinSrc = pInfo->pWinSrc;
			int Id = pWinSrc->GetID();
			auto pProg = pWin->GetItem<ProgBar>(ID_PROGBAR_TEST);
			if (pInfo->Notification == WM_NOTIFICATION_RELEASED) {
				switch (Id) {
					case ID_PROGBAR_DEC:
						_ProgBarValue -= 10;
						if (_ProgBarValue < _ProgBarMin) {
							_ProgBarValue = _ProgBarMin;
						}
						pProg->SetValue(_ProgBarValue);
						_UpdateProgBarStatus(pWin);
						break;
					case ID_PROGBAR_INC:
						_ProgBarValue += 10;
						if (_ProgBarValue > _ProgBarMax) {
							_ProgBarValue = _ProgBarMax;
						}
						pProg->SetValue(_ProgBarValue);
						_UpdateProgBarStatus(pWin);
						break;
					case ID_PROGBAR_RESET:
						_ProgBarValue = 0;
						pProg->SetValue(_ProgBarValue);
						_UpdateProgBarStatus(pWin);
						break;
					case ID_PROGBAR_TOGGLE_TEXT:
						_ProgBarCustomText = !_ProgBarCustomText;
						if (_ProgBarCustomText) {
							pProg->SetText("Loading");
						}
						else {
							pProg->SetText(nullptr);
						}
						_UpdateProgBarStatus(pWin);
						break;
					case GUI_ID_CANCEL:
						pWin->DialogEnd(0);
						break;
				}
			}
			return 0;
		}
	}
	return WM_DefaultProc(pWin, MsgId, Data);
}

void _TestProgBar() {
	auto pDialog = _aProgBarDialogCreate->CreateDialog(GUI_COUNTOF(_aProgBarDialogCreate), &_cbProgBarTest, 0, 0, 0);
	pDialog->DialogExec();
}

#define ID_SLIDER_TEST         (GUI_ID_USER + 190)
#define ID_SLIDER_TEST_V       (GUI_ID_USER + 191)
#define ID_SLIDER_STATUS       (GUI_ID_USER + 192)
#define ID_SLIDER_DEC          (GUI_ID_USER + 193)
#define ID_SLIDER_INC          (GUI_ID_USER + 194)
#define ID_SLIDER_RESET        (GUI_ID_USER + 195)
#define ID_SLIDER_TOGGLE_RANGE (GUI_ID_USER + 196)
#define ID_SLIDER_TOGGLE_TICKS (GUI_ID_USER + 197)

static int _SliderMin = 0;
static int _SliderMax = 100;
static int _SliderValue = 25;
static bool _SliderAutoTicks = false;

static void _UpdateSliderStatus(WObj *pWin) {
	auto pStatus = pWin->GetItem<Text>(ID_SLIDER_STATUS);
	if (pStatus) {
		char acStatus[96];
		sprintf(acStatus, "Range: %d-%d  Value: %d  Ticks: %s",
				_SliderMin,
				_SliderMax,
				_SliderValue,
				_SliderAutoTicks ? "Auto" : "11");
		pStatus->SetText(acStatus);
	}
}

static const Widget::CreateStruct _aSliderDialogCreate[] = {
	{ Frame ::CreateIndirect, "Slider Test"                               , 0                      , 70  , 60  , 470 , 250 , FRAMEWIN_CF_MOVEABLE },
	{ Slider::CreateIndirect, ""                                          , ID_SLIDER_TEST         , 15  , 20  , 340 , 30                         },
	{ Slider::CreateIndirect, ""                                          , ID_SLIDER_TEST_V       , 375 , 20  , 30  , 150 , SLIDER_CF_VERTICAL   },
	{ Text  ::CreateIndirect, "Drag the horizontal or vertical slider."   , 0                      , 15  , 55  , 350 , 16  , TEXT_CF_LEFT         },
	{ Button::CreateIndirect, "-10"                                       , ID_SLIDER_DEC          , 15  , 90  , 60  , 25                         },
	{ Button::CreateIndirect, "+10"                                       , ID_SLIDER_INC          , 80  , 90  , 60  , 25                         },
	{ Button::CreateIndirect, "Reset"                                     , ID_SLIDER_RESET        , 145 , 90  , 70  , 25                         },
	{ Button::CreateIndirect, "Range"                                     , ID_SLIDER_TOGGLE_RANGE , 220 , 90  , 70  , 25                         },
	{ Button::CreateIndirect, "Ticks"                                     , ID_SLIDER_TOGGLE_TICKS , 295 , 90  , 60  , 25                         },
	{ Text  ::CreateIndirect, ""                                          , ID_SLIDER_STATUS       , 15  , 130 , 350 , 18  , TEXT_CF_LEFT         },
	{ Button::CreateIndirect, "Close"                                     , GUI_ID_CANCEL          , 375 , 200 , 80  , 25                         }
};

static WM_PARAM _cbSliderTest(WObj *pWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG: {
			auto pSliderH = pWin->GetItem<Slider>(ID_SLIDER_TEST);
			auto pSliderV = pWin->GetItem<Slider>(ID_SLIDER_TEST_V);
			_SliderMin = 0;
			_SliderMax = 100;
			_SliderValue = 25;
			_SliderAutoTicks = false;
			pSliderH->SetRange(_SliderMin, _SliderMax);
			pSliderH->SetNumTicks(11);
			pSliderH->SetValue(_SliderValue);
			pSliderV->SetRange(_SliderMin, _SliderMax);
			pSliderV->SetNumTicks(11);
			pSliderV->SetValue(_SliderValue);
			_UpdateSliderStatus(pWin);
			return 0;
		}
		case WM_NOTIFY_PARENT: {
			auto pInfo = (const NOTIFY_INFO *)Data;
			auto pWinSrc = pInfo->pWinSrc;
			int Id = pWinSrc->GetID();
			auto pSliderH = pWin->GetItem<Slider>(ID_SLIDER_TEST);
			auto pSliderV = pWin->GetItem<Slider>(ID_SLIDER_TEST_V);
			auto pSlider = (Id == ID_SLIDER_TEST_V) ? pSliderV : pSliderH;
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_VALUE_CHANGED:
					if ((Id == ID_SLIDER_TEST) || (Id == ID_SLIDER_TEST_V)) {
						_SliderValue = pSlider->GetValue();
						pSliderH->SetValue(_SliderValue);
						pSliderV->SetValue(_SliderValue);
						_UpdateSliderStatus(pWin);
					}
					break;
				case WM_NOTIFICATION_RELEASED:
					switch (Id) {
						case ID_SLIDER_DEC:
							_SliderValue -= 10;
							pSliderH->SetValue(_SliderValue);
							pSliderV->SetValue(_SliderValue);
							_SliderValue = pSliderH->GetValue();
							_UpdateSliderStatus(pWin);
							break;
						case ID_SLIDER_INC:
							_SliderValue += 10;
							pSliderH->SetValue(_SliderValue);
							pSliderV->SetValue(_SliderValue);
							_SliderValue = pSliderH->GetValue();
							_UpdateSliderStatus(pWin);
							break;
						case ID_SLIDER_RESET:
							_SliderValue = _SliderMin;
							pSliderH->SetValue(_SliderValue);
							pSliderV->SetValue(_SliderValue);
							_UpdateSliderStatus(pWin);
							break;
						case ID_SLIDER_TOGGLE_RANGE:
							if ((_SliderMin == 0) && (_SliderMax == 100)) {
								_SliderMin = -50;
								_SliderMax = 50;
							}
							else {
								_SliderMin = 0;
								_SliderMax = 100;
							}
							pSliderH->SetRange(_SliderMin, _SliderMax);
							pSliderV->SetRange(_SliderMin, _SliderMax);
							_SliderValue = pSliderH->GetValue();
							pSliderV->SetValue(_SliderValue);
							_UpdateSliderStatus(pWin);
							break;
						case ID_SLIDER_TOGGLE_TICKS:
							_SliderAutoTicks = !_SliderAutoTicks;
							pSliderH->SetNumTicks(_SliderAutoTicks ? -1 : 11);
							pSliderV->SetNumTicks(_SliderAutoTicks ? -1 : 11);
							_UpdateSliderStatus(pWin);
							break;
						case GUI_ID_CANCEL:
							pWin->DialogEnd(0);
							break;
					}
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(pWin, MsgId, Data);
}

void _TestSlider() {
	auto pDialog = _aSliderDialogCreate->CreateDialog(GUI_COUNTOF(_aSliderDialogCreate), &_cbSliderTest, 0, 0, 0);
	pDialog->DialogExec();
}

#define ID_EDIT_TEST               (GUI_ID_USER + 220)
#define ID_EDIT_STATUS             (GUI_ID_USER + 221)
#define ID_EDIT_LOAD_SAMPLE        (GUI_ID_USER + 222)
#define ID_EDIT_LEFT               (GUI_ID_USER + 223)
#define ID_EDIT_RIGHT              (GUI_ID_USER + 224)
#define ID_EDIT_HOME               (GUI_ID_USER + 225)
#define ID_EDIT_END                (GUI_ID_USER + 226)
#define ID_EDIT_BACKSPACE          (GUI_ID_USER + 227)
#define ID_EDIT_DELETE             (GUI_ID_USER + 228)
#define ID_EDIT_TOGGLE_MODE        (GUI_ID_USER + 229)
#define ID_EDIT_INSERT_MARK        (GUI_ID_USER + 230)
#define ID_EDIT_CLEAR              (GUI_ID_USER + 231)

static void _UpdateEditStatus(WObj *pWin) {
	auto pEdit = pWin->GetItem<Edit>(ID_EDIT_TEST);
	auto pStatus = pWin->GetItem<Text>(ID_EDIT_STATUS);
	if (pEdit && pStatus) {
		char acText[96];
		char acStatus[192];
		pEdit->GetText(acText, sizeof(acText) - 1);
		sprintf(acStatus, "Len:%d Text:%s",
				pEdit->GetNumChars(),
				acText[0] ? acText : "<empty>");
		pStatus->SetText(acStatus);
	}
}

static void _ResetEditScenario(WObj *pWin) {
	auto pEdit = pWin->GetItem<Edit>(ID_EDIT_TEST);
	pEdit->SetMaxLen(64);
	pEdit->SetText("EditInteractiveDemo");
	pEdit->SetInsertMode(1);
	pEdit->SetSel(-1, -1);
	pEdit->SetCursorAtChar(4);
	pEdit->SetFocus();
	pEdit->Invalidate();
	_UpdateEditStatus(pWin);
}

static const Widget::CreateStruct _aEditDialogCreate[] = {
	{ Frame ::CreateIndirect, "Edit Interactive Test" , 0                   , 70  , 55  , 470 , 250 , FRAMEWIN_CF_MOVEABLE },
	{ Edit  ::CreateIndirect, ""                      , ID_EDIT_TEST        , 15  , 20  , 438 , 24  , 0, 64                },
	{ Text  ::CreateIndirect, "Focus on single-line cursor move/delete/insert behavior."
	                                                  , 0                   , 15  , 52  , 438 , 16  , TEXT_CF_LEFT         },
	{ Button::CreateIndirect, "Load Sample"           , ID_EDIT_LOAD_SAMPLE , 15  , 80  , 90  , 25                         },
	{ Button::CreateIndirect, "Left"                  , ID_EDIT_LEFT        , 110 , 80  , 55  , 25                         },
	{ Button::CreateIndirect, "Right"                 , ID_EDIT_RIGHT       , 170 , 80  , 55  , 25                         },
	{ Button::CreateIndirect, "Home"                  , ID_EDIT_HOME        , 230 , 80  , 55  , 25                         },
	{ Button::CreateIndirect, "End"                   , ID_EDIT_END         , 290 , 80  , 55  , 25                         },
	{ Button::CreateIndirect, "Backspace"             , ID_EDIT_BACKSPACE   , 350 , 80  , 103 , 25                         },
	{ Button::CreateIndirect, "Delete"                , ID_EDIT_DELETE      , 15  , 112 , 80  , 25                         },
	{ Button::CreateIndirect, "Ins/Ovr"               , ID_EDIT_TOGGLE_MODE , 100 , 112 , 80  , 25                         },
	{ Button::CreateIndirect, "Insert #"              , ID_EDIT_INSERT_MARK , 185 , 112 , 80  , 25                         },
	{ Button::CreateIndirect, "Clear"                 , ID_EDIT_CLEAR       , 270 , 112 , 80  , 25                         },
	{ Text  ::CreateIndirect, ""                      , ID_EDIT_STATUS      , 15  , 155 , 438 , 18  , TEXT_CF_LEFT         },
	{ Text  ::CreateIndirect, "Tips: use keyboard arrows/Home/End/Delete/Backspace and compare with buttons."
	                                                  , 0                   , 15  , 180 , 438 , 16  , TEXT_CF_LEFT         },
	{ Button::CreateIndirect, "Close", GUI_ID_CANCEL, 373, 205, 80, 25 }
};

static WM_PARAM _cbEditTest(WObj *pWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG:
			_ResetEditScenario(pWin);
			return 0;
		case WM_NOTIFY_PARENT: {
			auto pInfo = (const NOTIFY_INFO *)Data;
			auto pWinSrc = pInfo->pWinSrc;
			int Id = pWinSrc->GetID();
			auto pEdit = pWin->GetItem<Edit>(ID_EDIT_TEST);
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_VALUE_CHANGED:
					if (Id == ID_EDIT_TEST) {
						_UpdateEditStatus(pWin);
					}
					break;
				case WM_NOTIFICATION_RELEASED:
					switch (Id) {
						case ID_EDIT_LOAD_SAMPLE:
							_ResetEditScenario(pWin);
							break;
						case ID_EDIT_LEFT:
							pEdit->AddKey(GUI_KEY_LEFT);
							_UpdateEditStatus(pWin);
							break;
						case ID_EDIT_RIGHT:
							pEdit->AddKey(GUI_KEY_RIGHT);
							_UpdateEditStatus(pWin);
							break;
						case ID_EDIT_HOME:
							pEdit->AddKey(GUI_KEY_HOME);
							_UpdateEditStatus(pWin);
							break;
						case ID_EDIT_END:
							pEdit->AddKey(GUI_KEY_END);
							_UpdateEditStatus(pWin);
							break;
						case ID_EDIT_BACKSPACE:
							pEdit->AddKey(GUI_KEY_BACKSPACE);
							_UpdateEditStatus(pWin);
							break;
						case ID_EDIT_DELETE:
							pEdit->AddKey(GUI_KEY_DELETE);
							_UpdateEditStatus(pWin);
							break;
						case ID_EDIT_TOGGLE_MODE:
							pEdit->AddKey(GUI_KEY_INSERT);
							_UpdateEditStatus(pWin);
							break;
						case ID_EDIT_INSERT_MARK:
							pEdit->AddKey('#');
							_UpdateEditStatus(pWin);
							break;
						case ID_EDIT_CLEAR:
							pEdit->SetText("");
							pEdit->SetCursorAtChar(0);
							_UpdateEditStatus(pWin);
							break;
						case GUI_ID_CANCEL:
							pWin->DialogEnd(0);
							break;
					}
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(pWin, MsgId, Data);
}

void _TestEdit() {
	auto pDialog = _aEditDialogCreate->CreateDialog(GUI_COUNTOF(_aEditDialogCreate), &_cbEditTest, 0, 0, 0);
	pDialog->DialogExec();
}

#define ID_MULTEDIT_TEST            (GUI_ID_USER + 200)
#define ID_MULTEDIT_STATUS          (GUI_ID_USER + 201)
#define ID_MULTEDIT_APPEND          (GUI_ID_USER + 202)
#define ID_MULTEDIT_CLEAR           (GUI_ID_USER + 203)
#define ID_MULTEDIT_TOGGLE_READONLY (GUI_ID_USER + 204)
#define ID_MULTEDIT_WRAP_WORD       (GUI_ID_USER + 205)
#define ID_MULTEDIT_WRAP_NONE       (GUI_ID_USER + 206)

static bool _MultiEditReadOnly = false;
static int _MultiEditLineNo = 1;

static void _UpdateMultiEditStatus(WObj *pWin) {
	auto pEdit = pWin->GetItem<MultEdit>(ID_MULTEDIT_TEST);
	auto pStatus = pWin->GetItem<Text>(ID_MULTEDIT_STATUS);
	if (pEdit && pStatus) {
		char acStatus[96];
		sprintf(acStatus, "Len: %d  ReadOnly: %s", pEdit->GetTextSize(), _MultiEditReadOnly ? "On" : "Off");
		pStatus->SetText(acStatus);
	}
}

static const Widget::CreateStruct _aMultiEditDialogCreate[] = {
	{ Frame   ::CreateIndirect, "MultiEdit Test"  , 0                           , 60  , 60  , 420 , 280 , FRAMEWIN_CF_MOVEABLE },
	{ MultEdit::CreateIndirect, ""                , ID_MULTEDIT_TEST            , 10  , 10  , 395 , 150 , 0, 512               },
	{ Button  ::CreateIndirect, "Append"          , ID_MULTEDIT_APPEND          , 10  , 170 , 70  , 25                         },
	{ Button  ::CreateIndirect, "Clear"           , ID_MULTEDIT_CLEAR           , 85  , 170 , 70  , 25                         },
	{ Button  ::CreateIndirect, "ReadOnly"        , ID_MULTEDIT_TOGGLE_READONLY , 160 , 170 , 85  , 25                         },
	{ Button  ::CreateIndirect, "Wrap Word"       , ID_MULTEDIT_WRAP_WORD       , 250 , 170 , 75  , 25                         },
	{ Button  ::CreateIndirect, "Wrap None"       , ID_MULTEDIT_WRAP_NONE       , 330 , 170 , 75  , 25                         },
	{ Text    ::CreateIndirect, ""                , ID_MULTEDIT_STATUS          , 10  , 205 , 310 , 18  , TEXT_CF_LEFT         },
	{ Button  ::CreateIndirect, "Close"           , GUI_ID_CANCEL               , 325 , 230 , 80  , 25                         }
};

static WM_PARAM _cbMultiEditTest(WObj *pWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG: {
			auto pEdit = pWin->GetItem<MultEdit>(ID_MULTEDIT_TEST);
			_MultiEditReadOnly = false;
			_MultiEditLineNo = 1;
			pEdit->SetText("Line 1");
			pEdit->SetAutoScrollV(1);
			pEdit->SetAutoScrollH(1);
			pEdit->SetReadOnly(0);
			pEdit->SetWrapWord();
			_UpdateMultiEditStatus(pWin);
			return 0;
		}
		case WM_NOTIFY_PARENT: {
			auto pInfo = (const NOTIFY_INFO *)Data;
			auto pWinSrc = pInfo->pWinSrc;
			int Id = pWinSrc->GetID();
			auto pEdit = pWin->GetItem<MultEdit>(ID_MULTEDIT_TEST);
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_VALUE_CHANGED:
					if (Id == ID_MULTEDIT_TEST) {
						_UpdateMultiEditStatus(pWin);
					}
					break;
				case WM_NOTIFICATION_RELEASED:
					switch (Id) {
						case ID_MULTEDIT_APPEND: {
							char acOld[512];
							char acNew[640];
							char acLine[48];
							pEdit->GetText(acOld, sizeof(acOld));
							sprintf(acLine, "\r\nLine %d", ++_MultiEditLineNo);
							sprintf(acNew, "%s%s", acOld, acLine);
							pEdit->SetText(acNew);
							_UpdateMultiEditStatus(pWin);
							break;
						}
						case ID_MULTEDIT_CLEAR:
							_MultiEditLineNo = 0;
							pEdit->SetText("");
							_UpdateMultiEditStatus(pWin);
							break;
						case ID_MULTEDIT_TOGGLE_READONLY:
							_MultiEditReadOnly = !_MultiEditReadOnly;
							pEdit->SetReadOnly(_MultiEditReadOnly ? 1 : 0);
							_UpdateMultiEditStatus(pWin);
							break;
						case ID_MULTEDIT_WRAP_WORD:
							pEdit->SetWrapWord();
							_UpdateMultiEditStatus(pWin);
							break;
						case ID_MULTEDIT_WRAP_NONE:
							pEdit->SetWrapNone();
							_UpdateMultiEditStatus(pWin);
							break;
						case GUI_ID_CANCEL:
							pWin->DialogEnd(0);
							break;
					}
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(pWin, MsgId, Data);
}

void _TestMultiEdit() {
	auto pDialog = _aMultiEditDialogCreate->CreateDialog(GUI_COUNTOF(_aMultiEditDialogCreate), &_cbMultiEditTest, 0, 0, 0);
	pDialog->DialogExec();
}

int main(void) {
	GUI_Init();
	GUI_CURSOR_Show();

	//_TestListView();
	//_TestMultiPage();
	//_TestRadio();
	//_TestProgBar();
	//_TestSlider();
	//_TestEdit();
	//_TestMultiEdit();
	_TestDropDown();
	_TestListBox();
	_TestMemDev();

	return 0;
}
