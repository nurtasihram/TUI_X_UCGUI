module;

#include "GUI.h"

export module TUX.Array;

export {

template<typename T>
class ARRAY {

	uint16_t nItems = 0;
	T* pArray = nullptr;

public:

	ARRAY() = default;
	ARRAY(uint16_t nItems) : nItems(nItems) {
		pArray = (T*)GUI_ALLOC_AllocZero((size_t)nItems * sizeof(T));
	}
	~ARRAY() { Delete(); }

	auto GetnItems() const { return nItems; }

	void Delete() {
		if (pArray) {
			for (uint16_t i = 0; i < nItems; ++i)
				pArray[i].~T();
			GUI_ALLOC_Free(pArray);
			pArray = nullptr;
		}
		nItems = 0;
	}

	void Resize(uint16_t num) {
		if (pArray)
			pArray = (T*)GUI_ALLOC_Realloc((void*)pArray, (size_t)num * sizeof(T));
		else
			pArray = (T*)GUI_ALLOC_Alloc((size_t)num * sizeof(T));
		nItems = num;
	}

	void Delete(uint16_t Index) {
		if (Index >= nItems || !pArray)
			return;
		pArray[Index].~T();
		for (uint16_t i = Index; i + 1 < nItems; ++i)
			pArray[i] = pArray[i + 1];
		if (--nItems)
			pArray = (T*)GUI_ALLOC_Realloc((void*)pArray, (size_t)nItems * sizeof(T));
		else {
			GUI_ALLOC_Free(pArray);
			pArray = nullptr;
		}
	}

	inline T &Add() {
		pArray = (T*)GUI_ALLOC_Realloc((void*)pArray, (size_t)(nItems + 1) * sizeof(T));
		auto &item = pArray[nItems++];
		item = {};
		return item;
	}

	inline T &Insert(unsigned Index) {
		if (nItems == 0)
			Index = 0;
		else if (Index >= nItems)
			Index = nItems - 1;
		pArray = (T*)GUI_ALLOC_Realloc((void*)pArray, (size_t)(nItems + 1) * sizeof(T));
		++nItems;
		for (uint16_t i = nItems - 1; i > Index; --i)
			pArray[i] = pArray[i - 1];
		return pArray[Index];
	}

	inline auto begin() { return pArray; }
	inline auto begin() const { return pArray; }
	inline auto end() { return pArray + nItems; }
	inline auto end() const { return pArray + nItems; }

	inline uint16_t NumItems() const { return nItems; }

	int AddItem(const T *pData = nullptr) {
		auto pNew = (T*)GUI_ALLOC_Realloc((void*)pArray, (size_t)(nItems + 1) * sizeof(T));
		if (!pNew)
			return 1;
		pArray = pNew;
		if (pData)
			GUI__memcpy(pArray + nItems, pData, sizeof(T));
		else
			pArray[nItems] = {};
		++nItems;
		return 0;
	}

	void DeleteItem(unsigned int Index) {
		Delete((uint16_t)Index);
	}

	char InsertBlankItem(unsigned int Index) {
		if (Index > nItems)
			return 0;
		auto pNew = (T*)GUI_ALLOC_Realloc((void*)pArray, (size_t)(nItems + 1) * sizeof(T));
		if (!pNew)
			return 0;
		pArray = pNew;
		for (uint16_t i = nItems; i > Index; --i)
			pArray[i] = pArray[i - 1];
		pArray[Index] = {};
		++nItems;
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
