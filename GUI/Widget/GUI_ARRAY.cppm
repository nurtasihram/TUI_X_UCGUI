module;

#include "WM_Intern.h"

export module TUX.Array;

export {

template<typename T>
struct ARRAY {
	uint16_t NumItems = 0;
	T* pArray = nullptr;

	ARRAY() = default;
	ARRAY(uint16_t nItems) : NumItems(nItems) {
		pArray = (T*)GUI_ALLOC_AllocZero((size_t)NumItems * sizeof(T));
	}
	~ARRAY() { Delete(); }

	auto GetNumItems() const { return NumItems; }

	void Delete() {
		if (pArray) {
			for (uint16_t i = 0; i < NumItems; ++i)
				pArray[i].~T();
			GUI_ALLOC_Free(pArray);
			pArray = nullptr;
		}
		NumItems = 0;
	}

	void Resize(uint16_t num) {
		if (pArray)
			pArray = (T*)GUI_ALLOC_Realloc((void*)pArray, (size_t)num * sizeof(T));
		else
			pArray = (T*)GUI_ALLOC_Alloc((size_t)num * sizeof(T));
		NumItems = num;
	}

	void Delete(uint16_t Index) {
		if (Index >= NumItems || !pArray)
			return;
		pArray[Index].~T();
		for (uint16_t i = Index; i + 1 < NumItems; ++i)
			pArray[i] = pArray[i + 1];
		if (--NumItems)
			pArray = (T*)GUI_ALLOC_Realloc((void*)pArray, (size_t)NumItems * sizeof(T));
		else {
			GUI_ALLOC_Free(pArray);
			pArray = nullptr;
		}
	}

	inline T &Add() {
		pArray = (T*)GUI_ALLOC_Realloc((void*)pArray, (size_t)(NumItems + 1) * sizeof(T));
		auto &item = pArray[NumItems++];
		item = {};
		return item;
	}

	inline T &Insert(unsigned Index) {
		if (NumItems == 0)
			Index = 0;
		else if (Index >= NumItems)
			Index = NumItems - 1;
		pArray = (T*)GUI_ALLOC_Realloc((void*)pArray, (size_t)(NumItems + 1) * sizeof(T));
		++NumItems;
		for (uint16_t i = NumItems - 1; i > Index; --i)
			pArray[i] = pArray[i - 1];
		return pArray[Index];
	}

	inline auto begin() { return pArray; }
	inline auto begin() const { return pArray; }
	inline auto end() { return pArray + NumItems; }
	inline auto end() const { return pArray + NumItems; }

	inline uint16_t NumItems_() const { return NumItems; }

	int AddItem(const T *pData = nullptr) {
		auto pNew = (T*)GUI_ALLOC_Realloc((void*)pArray, (size_t)(NumItems + 1) * sizeof(T));
		if (!pNew)
			return 1;
		pArray = pNew;
		if (pData)
			GUI__memcpy(pArray + NumItems, pData, sizeof(T));
		else
			pArray[NumItems] = {};
		++NumItems;
		return 0;
	}

	void DeleteItem(unsigned int Index) {
		Delete((uint16_t)Index);
	}

	char InsertBlankItem(unsigned int Index) {
		if (Index > NumItems)
			return 0;
		auto pNew = (T*)GUI_ALLOC_Realloc((void*)pArray, (size_t)(NumItems + 1) * sizeof(T));
		if (!pNew)
			return 0;
		pArray = pNew;
		for (uint16_t i = NumItems; i > Index; --i)
			pArray[i] = pArray[i - 1];
		pArray[Index] = {};
		++NumItems;
		return 1;
	}

	T *InsertItem(unsigned int Index) {
		if (!InsertBlankItem(Index))
			return nullptr;
		return &pArray[Index];
	}

	T &operator[](uint16_t Index) { return pArray[Index]; }
	const T &operator[](uint16_t Index) const { return pArray[Index]; }

};

}
