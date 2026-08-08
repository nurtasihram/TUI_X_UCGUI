module;

#include "WM_Intern.h"

export module TUX.Array;

export {

struct GUI_ARRAY {
	uint16_t NumItems = 0;
	WM_HMEM  haHandle = nullptr;

	auto GetNumItems() const { return NumItems; }

	void Delete() {
		if (!haHandle)
			return;
		auto pa = (WM_HMEM *)haHandle;
		for (int i = 0; i < NumItems; i++)
			GUI_ALLOC_FreePtr(pa + i);
		GUI_ALLOC_FreePtr(&haHandle);
		NumItems = 0;
	}

	void DeleteItem(unsigned int Index) {
		if (Index >= (unsigned)NumItems || !haHandle)
			return;
		auto pa = (WM_HMEM *)haHandle;
		GUI_ALLOC_FreePtr(pa + Index);
		unsigned newCount = --NumItems;
		for (unsigned i = Index; i < newCount; i++)
			pa[i] = pa[i + 1];
	}

protected:
	// Returns 0 on success, 1 on failure
	int AddItem(const void *pData, int Len) {
		WM_HMEM hItem = Len ? GUI_ALLOC_AllocInit(pData, Len) : nullptr;
		if (Len && !hItem)
			return 1;
		auto ha = GUI_ALLOC_Realloc(haHandle, (NumItems + 1) * sizeof(WM_HMEM));
		if (!ha) {
			GUI_ALLOC_Free(hItem);
			return 1;
		}
		((WM_HMEM *)ha)[NumItems] = hItem;
		haHandle = ha;
		NumItems++;
		return 0;
	}

	// Returns pointer to item data, or nullptr if index out of range
	void *GetItem(unsigned int Index) const {
		if (Index >= (unsigned)NumItems || !haHandle)
			return nullptr;
		return (void *)((WM_HMEM *)haHandle)[Index];
	}

	// Allocates new storage for item[Index], copies pData if non-null; returns handle or 0 on failure
	WM_HMEM SetItem(unsigned int Index, const void *pData, int Len) {
		if (Index >= (unsigned)NumItems || !haHandle)
			return 0;
		auto pa = (WM_HMEM *)haHandle + Index;
		GUI_ALLOC_FreePtr(pa);
		*pa = GUI_ALLOC_AllocZero(Len);
		if (*pa && pData)
			GUI__memcpy(*pa, pData, Len);
		return *pa;
	}

	// Inserts a blank slot at Index (Index must be < NumItems); returns 1 on success
	char InsertBlankItem(unsigned int Index) {
		if (Index >= (unsigned)NumItems)
			return 0;
		auto newBuf = (WM_HMEM *)GUI_ALLOC_AllocZero((NumItems + 1) * sizeof(WM_HMEM));
		if (!newBuf)
			return 0;
		auto oldBuf = (WM_HMEM *)haHandle;
		GUI__memcpy(newBuf,             oldBuf,          Index              * sizeof(WM_HMEM));
		GUI__memcpy(newBuf + Index + 1, oldBuf + Index, (NumItems - Index)  * sizeof(WM_HMEM));
		GUI_ALLOC_Free(haHandle);
		haHandle = newBuf;
		NumItems++;
		return 1;
	}

	// Inserts a new item of size Len at Index; returns handle or 0 on failure
	WM_HMEM InsertItem(unsigned int Index, int Len) {
		if (!InsertBlankItem(Index))
			return 0;
		return SetItem(Index, nullptr, Len);
	}
};

template<typename T>
struct GUI_ARRAY_T : GUI_ARRAY {
	int AddItem(const T *pData = nullptr) {
		return GUI_ARRAY::AddItem(pData, sizeof(T));
	}
	T *GetItem(unsigned int Index) const {
		return (T *)GUI_ARRAY::GetItem(Index);
	}
	T &operator[](unsigned int Index) const {
		return *GetItem(Index);
	}
	T *SetItem(unsigned int Index, const T *pData = nullptr) {
		return (T *)GUI_ARRAY::SetItem(Index, pData, sizeof(T));
	}
	T *InsertItem(unsigned int Index) {
		return (T *)GUI_ARRAY::InsertItem(Index, sizeof(T));
	}
	char InsertBlankItem(unsigned int Index) {
		return GUI_ARRAY::InsertBlankItem(Index);
	}
};

}
