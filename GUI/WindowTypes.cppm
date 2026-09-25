module;

#include <stdint.h>

export module TUX.WindowTypes;

export import TUX.Types;

export {

#pragma region Messages & Notifications	
/*********************************************************************
*
*               Messages Ids
* The following is the list of windows messages.
*/
enum WM_MSGID : int {
	 WM_NONE = 0 ,

	 WM_CREATE, WM_DELETE,

	 WM_PAINT,

	 WM_MOVE, WM_SIZE,

	 WM_TOUCH, WM_TOUCH_CHILD,
	 WM_MOUSEOVER, WM_PID_STATE_CHANGED,

	 WM_CAPTURE_RELEASED,

	 WM_KEY,
	 WM_GET_ID, WM_SET_ID,

	 WM_GET_INSIDE_RECT,
	 WM_GET_CLIENT_WINDOW,
	 WM_NOTIFY_CLIENTCHANGE,

	 WM_INIT_DIALOG,

	 WM_SET_FOCUS,
	 WM_GET_ACCEPT_FOCUS,
	 WM_NOTIFY_CHILD_HAS_FOCUS,

	 WM_GET_BKCOLOR,
	 WM_GET_SCROLL_STATE, WM_SET_SCROLL_STATE,
	 WM_NOTIFY_PARENT,
	 WM_HANDLE_DIALOG_STATUS,

	 WM_GET_RADIOGROUP,

	 WM_MENU,
};
template<auto code> constexpr int16_t WM_WIDGET = 0x0300 + code;
template<auto code> constexpr int16_t WM_USER   = 0x0400 + code;

/*********************************************************************
*
*               Notification codes
*
* The following is the list of notification codes send
* with the WM_NOTIFY_PARENT message
*/
enum WM_NOTIFICATION : int {
	WM_NOTIFICATION_START = 0,
	WM_NOTIFICATION_CLICKED,
	WM_NOTIFICATION_RELEASED,
	WM_NOTIFICATION_MOVED_OUT,
	WM_NOTIFICATION_SEL_CHANGED,
	WM_NOTIFICATION_VALUE_CHANGED,
	WM_NOTIFICATION_SCROLLBAR_ADDED,
	WM_NOTIFICATION_CHILD_DELETED,
	WM_NOTIFICATION_GOT_FOCUS,
	WM_NOTIFICATION_LOST_FOCUS,
	WM_NOTIFICATION_SCROLL_CHANGED,
	WM_NOTIFICATION_END
};
template<int code> constexpr int WM_NOTIFICATION_WIDGET = (static_cast<int>(WM_NOTIFICATION_END) + code);

#pragma endregion

#pragma region Window create flags
/**********************************************************************
 * These flags can be passed to the create window					  *
 * function as flag-parameter. The flags are combinable using the	  *
 * binary or operator.												  *
 **********************************************************************/
using WM_CF = uint16_t;
constexpr WM_CF
	 /* Hide window after creation (default !) */
	 WC_HIDE              = 0 << 0, 
	 /* Show window after creation */
	 WC_VISIBLE           = 1 << 0, 
	 /* Use memory device for redraws */
	 WC_MEMDEV            = 1 << 1, 
	 /* Stay on top */
	 WC_STAYONTOP         = 1 << 2, 
	 /* Disabled: Does not receive PID (mouse & touch) input */
	 WC_DISABLED          = 1 << 3, 

	 /* Create only flags ... Not available as status flags */
	
	 /* If automatic activation upon creation of window is desired */
	 WC_ACTIVATE          = 1 << 4, 

	 WC_ANCHOR_LEFT       = 1 << 5, 
	 WC_ANCHOR_RIGHT      = 1 << 6, 
	 WC_ANCHOR_TOP        = 1 << 7,
	 WC_ANCHOR_BOTTOM     = 1 << 8, 
	 WC_ANCHOR_VERTICAL   = WC_ANCHOR_LEFT | WC_ANCHOR_RIGHT,
	 WC_ANCHOR_HORIZONTAL = WC_ANCHOR_TOP | WC_ANCHOR_BOTTOM,
	 WC_ANCHOR_ALL        = WC_ANCHOR_VERTICAL | WC_ANCHOR_HORIZONTAL;
constexpr WM_CF WM_CF_MASK =
	WC_VISIBLE |
	WC_MEMDEV |
	WC_STAYONTOP |
	WC_ANCHOR_ALL;
#pragma endregion

/************************************************************
*
*                    GUI_KEY_...
*
*************************************************************
These ID values are basically meant to be used with widgets
Note that we have chosen the values to be close to existing
"standards", so do not change them unless forced to.
*/
enum GUI_KEY {
	GUI_KEY_BACKSPACE = 8 ,        /* ASCII: BACKSPACE Crtl-H */
	GUI_KEY_TAB       = 9 ,        /* ASCII: TAB       Crtl-I */
	GUI_KEY_ENTER     = 13,        /* ASCII: ENTER     Crtl-M */
	GUI_KEY_LEFT      = 16,
	GUI_KEY_UP        = 17,
	GUI_KEY_RIGHT     = 18,
	GUI_KEY_DOWN      = 19,
	GUI_KEY_HOME      = 23,
	GUI_KEY_END       = 24,
	GUI_KEY_SHIFT     = 25,
	GUI_KEY_CONTROL   = 26,
	GUI_KEY_ESCAPE    = 27,        /* ASCII: ESCAPE    0x1b   */
	GUI_KEY_INSERT    = 29,
	GUI_KEY_DELETE    = 30,
	GUI_KEY_SPACE     = 32,
	GUI_KEY_F1        = 40,
	GUI_KEY_F2        = 41
};

/*********************************************************************
*
*       Dialog item IDs
*
*  The IDs below are arbitrary values. They just have to be unique
*  within the dialog.
*
*  If you use your own Ids, we recommend to use values above GUI_ID_USER.
*/
constexpr uint16_t
	GUI_ID_OK        = 1,
	GUI_ID_CANCEL    = 2,
	GUI_ID_YES       = 3,
	GUI_ID_NO        = 4,
	GUI_ID_CLOSE     = 5,
	GUI_ID_HELP      = 6,
	GUI_ID_MAXIMIZE  = 7,
	GUI_ID_MINIMIZE  = 8,
	GUI_ID_VSCROLL   = 0xFE,
	GUI_ID_HSCROLL   = 0xFF,
	GUI_ID_USER      = 0x100;

#pragma region Windows parameter type
using WM_PARAM = uintptr_t;

struct DIALOG_STATUS {
	int16_t Done = 0, ReturnValue = 0;
};

struct SCROLL_STATE {
	int16_t NumItems = 0, PageSize = 0, v = 0;
	void Bounds() {
		int Max = NumItems - PageSize;
		if (Max < 0)
			Max = 0;
		/* Make sure scroll pos is in bounds */
		if (v < 0)
			v = 0;
		if (v > Max)
			v = Max;
	}
	auto CheckPos(int Pos, int LowerDist, int UpperDist) {
		int vOld = v;
		/* Check upper limit */
		if (Pos > v + PageSize - 1)
			v = Pos - (PageSize - 1) + UpperDist;
		/* Check lower limit */
		if (Pos < v)
			v = Pos - LowerDist;
		Bounds();
		return v - vOld;
	}
	auto SetValue(int v) {
		int vOld = this->v;
		this->v = v;
		Bounds();
		return this->v - vOld;
	}
	inline bool operator!=(const SCROLL_STATE &other) const
	{ return NumItems != other.NumItems || v != other.v || PageSize != other.PageSize; }
	inline bool operator==(const SCROLL_STATE &other) const
	{ return !(*this != other); }
};

#pragma endregion

}
