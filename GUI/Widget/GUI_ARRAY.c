#include <string.h>

#include "GUI_ARRAY.h"

unsigned int GUI_ARRAY_GetNumItems(const GUI_ARRAY *pThis) {
	GUI_ARRAY_CHECK(pThis);    /* Sanity checks at higher debug levels only */
	return pThis->NumItems;
}

/*********************************************************************
*
*       GUI_ARRAY_AddItem
*
* Purpose:
*   Add an item to a GUI_ARRAY.
*   If the SIze is > 0, a memory block is allocated for storage.
*   If on top of this a pointer is specified, the memory block holding
*   the copy of the item is initialized.
*
* Return value:
*   If O.K. : 0
*   On error: 1
*
*/
int GUI_ARRAY_AddItem(GUI_ARRAY *pThis, const void *pNew, int Len) {
	WM_HMEM hNewItem = 0;
	WM_HMEM hNewBuffer;
	WM_HMEM *pNewBuffer;
	int r = 0;
	GUI_ARRAY_CHECK(pThis);    /* Sanity checks at higher debug levels only */

	/* Alloc memory for new item */
	if (Len) {
		if ((hNewItem = GUI_ALLOC_AllocInit(pNew, Len)) == 0) {
			r = 1;            /* Error */
		}
	}
	/* Put handle of new item into the array */
	if (r == 0) {
		int NumItems;
		NumItems = pThis->NumItems;
		/* Add the handle to new item to the buffer */
		hNewBuffer = GUI_ALLOC_Realloc(pThis->haHandle, (NumItems + 1) * sizeof(WM_HMEM));
		if (hNewBuffer == 0) {
			GUI_ALLOC_Free(hNewItem);
			r = 1;            /* Error */
		}
		else {
			pNewBuffer = (WM_HMEM *)(hNewBuffer);
			*(pNewBuffer + NumItems) = hNewItem;
			pThis->haHandle = hNewBuffer;
			pThis->NumItems++;
		}
	}

	return r;
}
/*********************************************************************
*
*       GUI_ARRAY_Delete
*
* Purpose:
*  Free all allocated memory blocks
*
* Add. info:
*   Locking is not required, since this routine is considered internal
*   and should only be called after locking.
*/
void GUI_ARRAY_Delete(GUI_ARRAY *pThis) {
	int i;
	WM_HMEM ha;
	WM_HMEM *pa;
	GUI_ARRAY_CHECK(pThis);    /* Sanity checks at higher debug levels only */
	ha = pThis->haHandle;
	if (ha) {
		pa = (WM_HMEM *)(ha);
		/* Free the attached items, one at a time */
		for (i = 0; i < pThis->NumItems; i++) {
			GUI_ALLOC_FreePtr(pa + i);
		}
		/* Free the handle buffer */
		GUI_ALLOC_FreePtr(&pThis->haHandle);
		pThis->NumItems = 0;                    /* For safety, in case the array is used after it has been deleted */
	}
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
	pThis->InitState = GUI_ARRAY_STATE_DELETED;
#endif
}
/*********************************************************************
*
*       GUI_ARRAY_SethItem
*
* Purpose:
*   Sets an item.
*
* Returns:
*   1: if operation has failed
*   0: OK
*
* Notes:
*   (1) Replacing Items
*       If the item is already assigned
*       (Which means the handle is already != 0), it is freeed. However,
*       the handle is treated as a handle to a data item, not an object.
*       This means the data item is freed, but if the pointer points to
*       an object, the destructor of the object is not called.
*/
int GUI_ARRAY_SethItem(GUI_ARRAY *pThis, unsigned int Index, WM_HMEM hItem) {
	WM_HMEM ha;
	WM_HMEM *pa;
	int r = 1;
	GUI_ARRAY_CHECK(pThis);    /* Sanity checks at higher debug levels only */
	if (Index < (unsigned)pThis->NumItems) {
		ha = pThis->haHandle;
		if (ha) {
			pa = (WM_HMEM *)(ha);
			pa += Index;
			GUI_ALLOC_FreePtr(pa);
			*pa = hItem;
			r = 0;
		}
	}
	return r;
}
/*********************************************************************
*
*       GUI_ARRAY_SetItem
*
* Purpose:
*   Sets an item, returning the handle.
*   If a data pointer is given, the allocated memory is initialized from it thru memcpy.
*
* Returns:
*   Handle of the allocated memory block
*
* Notes:
*   (1) Replacing Items
*       If the item is already assigned
*       (Which means the handle is already != 0), it is freeed. However,
*       the handle is treated as a handle to a data item, not an object.
*       This means the data item is freed, but if the pointer points to
*       an object, the destructor of the object is not called.
*/
WM_HMEM  GUI_ARRAY_SetItem(GUI_ARRAY *pThis, unsigned int Index, const void *pData, int Len) {
	WM_HMEM hItem = 0;
	GUI_ARRAY_CHECK(pThis);    /* Sanity checks at higher debug levels only */
	if (Index < (unsigned)pThis->NumItems) {
		WM_HMEM ha;
		ha = pThis->haHandle;
		if (ha) {
			WM_HMEM *pa;
			pa = (WM_HMEM *)(ha);
			pa += Index;
			hItem = *pa;
			/*
			 * If a buffer is already available, a new buffer is only needed when the
			 * new item has a different size.
			 */
			if (hItem) {
//				if (GUI_ ALLOC_GetSize(hItem) != Len) {
					hItem = 0;
//				}
			}
			/*
			 * Allocate a new buffer and free the old one (if needed).
			 */
			if (!hItem) {
				hItem = GUI_ALLOC_AllocZero(Len);
				if (hItem) {
					GUI_ALLOC_FreePtr(pa);
					*pa = hItem;
				}
			}
			/*
			 * Set the item (if needed)
			 */
			if (pData && hItem) {
				char *pItem = (char *)(hItem);
				memcpy(pItem, pData, Len);
			}
		}
	}
	return hItem;
}
/*********************************************************************
*
*       GUI_ARRAY_GethItem
*
* Purpose:
*   Gets the handle of specified item
*
* Notes:
*   (1) Index out of bounds
*   It is permitted to specify an index larger than the
*   array size. In this case, a 0-handle is returned.
*/
WM_HMEM GUI_ARRAY_GethItem(const GUI_ARRAY *pThis, unsigned int Index) {
	WM_HMEM h = 0;
	GUI_ARRAY_CHECK(pThis);    /* Sanity checks at higher debug levels only */
	if (Index < (unsigned)pThis->NumItems) {
		WM_HMEM  ha;
		WM_HMEM *pa;
		ha = pThis->haHandle;
		if (ha) {
			pa = (WM_HMEM *)(ha);
			h = *(pa + Index);
		}
	}
	return h;
}
/*********************************************************************
*
*       GUI_ARRAY_GetpItem
*
* Purpose:
*   Gets the pointer of specified item
*
* Notes:
*   (1) Index out of bounds
*       It is permitted to specify an index larger than the
*       array size. In this case, a 0-handle is returned.
*   (2) Locking
*       It is the caller's responsibility to lock before calling this
*       function.
*/
void *GUI_ARRAY_GetpItem(const GUI_ARRAY *pThis, unsigned int Index) {
	void *p = NULL;
	WM_HMEM h;
	GUI_ARRAY_CHECK(pThis);    /* Sanity checks at higher debug levels only */
	h = GUI_ARRAY_GethItem(pThis, Index);
	if (h) {
		p = (h);
	}
	return p;
}
/*********************************************************************
*
*       Debug support
*
**********************************************************************
*
* Purpose:
*   The routines below are required only at higher debug levels
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
/*********************************************************************
*
*       GUI_ARRAY_Create
*
* Purpose:
*/
void GUI_ARRAY_Create(GUI_ARRAY *pThis) {
	pThis->InitState = GUI_ARRAY_STATE_CREATED;
}
/*********************************************************************
*
*       GUI_ARRAY_Check
*
* Purpose:
*/
void GUI_ARRAY_Check(const GUI_ARRAY *pThis) {
	if (pThis->InitState == GUI_ARRAY_STATE_DELETED) {
	}
	else if (pThis->InitState == GUI_ARRAY_STATE_NOT_CREATED) {
	}
	else if (pThis->InitState != GUI_ARRAY_STATE_CREATED) {
	}
}
#endif /* GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL */


/*********************************************************************
*
*       GUI_ARRAY_DeleteItem
*
* Purpose:
*   Eliminate one element in a GUI_ARRAY.
*   This means:
*     - freeing the memory block
*     - moving the last item to the position of the deleted item
*     - possible reducing the size of the memory used for management (opt)
*
*/
void GUI_ARRAY_DeleteItem(GUI_ARRAY *pThis, unsigned int Index) {
	WM_HMEM ha;
	WM_HMEM *pa;
	int i;
	if (Index < (unsigned)pThis->NumItems) {
		ha = pThis->haHandle;
		if (ha) {
			int NumItems;

			pa = (WM_HMEM *)(ha);
			/* Free the attached item */
			GUI_ALLOC_FreePtr(pa + Index);
			/* Move the last items to the position of the deleted item */
			NumItems = --pThis->NumItems;
			for (i = Index; i <= NumItems - 1; i++) {
				*(pa + i) = *(pa + i + 1);
			}

		}
	}
}


/*********************************************************************
*
*       GUI_ARRAY_ResizeItem
*
* Purpose:
*   Resizes one element in a GUI_ARRAY.
* Return value:
*   Handle of allocated memory block if successful
*   0 if failed
*
*/
void *GUI_ARRAY_ResizeItem(GUI_ARRAY *pThis, unsigned int Index, int Len) {
	void *r = NULL;
	WM_HMEM hNew;
	hNew = GUI_ALLOC_AllocZero(Len);
	if (hNew) {
		void *pOld = GUI_ARRAY_GetpItem(pThis, Index);
		void *pNew = (hNew);
		memcpy(pNew, pOld, Len);
		if (GUI_ARRAY_SethItem(pThis, Index, hNew)) {
			GUI_ALLOC_FreePtr(&hNew);    /* Free on error */
		}
		else {
			r = pNew;
		}
	}
	return r;
}


/*********************************************************************
*
*       GUI_ARRAY_InsertBlankItem
*
* Purpose:
*   Inserts a blank element in a GUI_ARRAY.
*
* Parameters:
*   Index   Index of the element to insert before
*           0 means: Insert before first element
*           1 means: Insert before second element
*
* Return value:
*   1 if successful
*   0 if failed
*
* Notes:
*   (1) Index changes
*       The index of all items after the one inserted will change
*       (Increment by 1)
*/
char GUI_ARRAY_InsertBlankItem(GUI_ARRAY *pThis, unsigned int Index) {
	GUI_ARRAY_CHECK(pThis);    /* Sanity checks at higher debug levels only */

	if (Index >= (unsigned)pThis->NumItems) {
	}
	else {
		WM_HMEM hNewBuffer;
		hNewBuffer = GUI_ALLOC_AllocZero(sizeof(WM_HMEM) * (pThis->NumItems + 1));
		if (hNewBuffer == 0) {
		}
		else {
			WM_HMEM *pOldBuffer;
			WM_HMEM *pNewBuffer;
			pNewBuffer = (WM_HMEM *)(hNewBuffer);
			pOldBuffer = (WM_HMEM *)(pThis->haHandle);
			memcpy(pNewBuffer, pOldBuffer, Index * sizeof(WM_HMEM));
			memcpy(pNewBuffer + (Index + 1), pOldBuffer + Index, (pThis->NumItems - Index) * sizeof(WM_HMEM));
			GUI_ALLOC_Free(pThis->haHandle);
			pThis->haHandle = hNewBuffer;
			pThis->NumItems++;
			return 1;               /* Successfull */
		}
	}
	return 0;                   /* Failed */
}

/*********************************************************************
*
*       GUI_ARRAY_InsertItem
*
* Purpose:
*   Inserts an element in a GUI_ARRAY.
*
*
* Parameters:
*   Index   Index of the element to insert before
*           0 means: Insert before first element
*           1 means: Insert before second element
*
* Return value:
*   Handle of allocated memory block if successful
*   0 if failed
*
* Notes:
*   (1) Index changes
*       The index of all items after the one inserted will change
*       (Increment by 1)
*/
WM_HMEM GUI_ARRAY_InsertItem(GUI_ARRAY *pThis, unsigned int Index, int Len) {
	WM_HMEM hNewBuffer = 0;
	if (GUI_ARRAY_InsertBlankItem(pThis, Index)) {
		hNewBuffer = GUI_ARRAY_SetItem(pThis, Index, 0, Len);
	}
	return hNewBuffer;
}
