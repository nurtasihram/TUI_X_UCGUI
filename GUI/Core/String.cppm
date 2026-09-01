module;

#include <cstddef>
#include <stdint.h>

export module TUX.String;

export import TUX.Memory;

using StringLen = size_t;

export {

bool GUI__strcmp(const char *s0, const char *s1) {
	if (s0 == nullptr)
		s0 = "";
	if (s1 == nullptr)
		s1 = "";
	do {
		if (*s0 != *s1)
			return true;
		s1++;
	} while (*++s0);
	if (*s1)
		return true;    /* Not equal, since s1 is longer than s0 */
	return false;      /* Equal ! */
}
uint16_t GUI__strlen(const char *s) {
	uint16_t r = 0;
	if (s) {
		do {
			r++;
		} while (*s++);
	}
	return r;
}
int GUI__strcpy(char *sDest, const char *sSrc) {
	auto s = sDest;
	while ((*s++ = *sSrc++) != 0) {}
	return (int)(s - sDest - 1);
}

int GUI__HandleEOLine(const char **ps) {
	auto s = *ps;
	char c = *s++;
	if (c == 0) 
		return 1;
	if (c == '\n')
		*ps = s;
	return 0;
}
int GUI__GetLineNumChars(const char *s, int MaxNumChars) {
	int NumChars = 0;
	if (s) {
		for (; NumChars < MaxNumChars; NumChars++) {
			auto Data = *s++;
			if (Data == 0 || Data == '\n')
				break;
		}
	}
	return NumChars;
}
bool GUI__SetText(char **ppText, const char *s) {
	if (!ppText)
		return false;
	auto size = GUI__strlen(s);
	if (!size) {
		if (*ppText)
			GUI_ALLOC_Free(*ppText);
		*ppText = nullptr;
		return true;
	}
	auto pText = *ppText = (char *)GUI_ALLOC_Realloc(*ppText, ++size);
	if (!pText)
		return false;
	GUI__memcpy(pText, s, size);
	return true;
}

class String {

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

}
