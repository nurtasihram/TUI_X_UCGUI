export module TUX.Window;

export import TUX;
export import TUX.Types;
export import TUX.WindowTypes;

export {
	
typedef WM_PARAM WM_CALLBACK(struct WM_Obj *pWin, int MsgId, WM_PARAM Data);
struct WM_Obj {
	GUI_RECT Rect;        /* outer dimensions of window */
	GUI_RECT InvalidRect; /* invalid rectangle */
	WM_CALLBACK *cb;      /* ptr to notification callback */
	WM_Obj *pNextLin;     /* Next window in linear list */
	WM_Obj *pParent;
	WM_Obj *pFirstChild;
	WM_Obj *pNext;
	uint16_t Status; /* Some status flags */

public:
	auto GetFlags() const { return Status; }

public:
	auto GetRect() const { return Rect; }
	
	auto GetOrg() const { return Rect.LeftTop(); }
	auto GetOrgX() const { return Rect.x0; }
	auto GetOrgY() const { return Rect.y0; }

	auto GetSize() const { return Rect.Size(); }
	auto GetSizeX() const { return Rect.XSize(); }
	auto GetSizeY() const { return Rect.YSize(); }

public:
	WM_PARAM SendMessage(uint16_t MsgId, WM_PARAM Data = 0) {
		return cb ? cb(this, MsgId, Data) : (WM_PARAM)0;
	}
	WM_PARAM SendMessage(uint16_t MsgId, WM_PARAM Data = 0) const
	{ return const_cast<WM_Obj *>(this)->SendMessage(MsgId, Data); }

#pragma region Scroll

	WM_Obj *GetScrollbarH()
	{ return GetItem(GUI_ID_HSCROLL); }
	WM_Obj *GetScrollbarV()
	{ return GetItem(GUI_ID_VSCROLL); }

	void SetScrollState(const WM_SCROLL_STATE &State)
	{ SendMessage(WM_SET_SCROLL_STATE, (WM_PARAM)&State); }
#pragma endregion

#pragma region ID
	uint16_t GetID() const { return (uint16_t)SendMessage(WM_GET_ID); }
	void SetID(uint16_t Id) { SendMessage(WM_SET_ID, (WM_PARAM)Id); }
	WM_Obj *GetItem(uint16_t Id) {
		for (auto i = pFirstChild; i; i = i->pNext)
			if (i->GetID() == Id)
				return i;
			else if (auto pItem = i->GetItem(Id))
				return pItem;
		return nullptr;
	}
	const WM_Obj *GetItem(uint16_t Id) const { return const_cast<WM_Obj *>(this)->GetItem(Id); }
	template<class Ret>
	Ret *GetItem(uint16_t Id) { return (Ret *)GetItem(Id); }
	template<class Ret>
	const Ret *GetItem(uint16_t Id) const { return (const Ret *)GetItem(Id); }
#pragma endregion

	RGBC GetBkColor() const { return (RGBC)SendMessage(WM_GET_BKCOLOR); }

#pragma region Focus
	bool IsFocussable() const { return SendMessage(WM_GET_ACCEPT_FOCUS); }
#pragma endregion

#pragma region Visibility
	bool IsVisible() const { return Status & WC_VISIBLE; }
	void ShowWindow();
	void HideWindow();
#pragma endregion

	bool IsEnabled() const { return !(Status & WC_DISABLED); }

};


}
