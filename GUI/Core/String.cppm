module;

#include <cstddef>

export module TUX.String;

extern "C++" {
	void *GUI_ALLOC_AllocInit(const void *pInitData, size_t Size);
	void *GUI_ALLOC_AllocNoInit(size_t size);
	void *GUI_ALLOC_Realloc(void *ptr, size_t NewSize);
	void  GUI_ALLOC_FreePtr(void **pptr);
	bool GUI__SetText(char **ppText, const char *s);
	int   GUI__strlen(const char *s);
	bool  GUI__strcmp(const char *s0, const char *s1);
	void  GUI__memcpy(void *pDest, const void *pSrc, size_t NumBytes);
}

using StringLen = size_t;

export class String {

	char *pText{ nullptr };

public:
	String() = default;
	String(const char *s) { GUI__SetText(&pText, s); }
	String(const String &Other) { GUI__SetText(&pText, Other.pText); }
	String(String &&Other) noexcept : pText(Other.pText)
	{ Other.pText = nullptr; }

	~String() { Delete(); }

public:

	void Delete() noexcept {
		GUI_ALLOC_FreePtr((void **)&pText);
	}


	bool operator=(const char *s) { return GUI__SetText(&pText, s); }

	bool operator==(const char *s) const noexcept { return GUI__strcmp(pText, s); }
	bool operator!=(const char *s) const noexcept { return !GUI__strcmp(pText, s); }

	operator bool() const noexcept { return pText; }
	operator const char *() const noexcept { return pText; }
};
