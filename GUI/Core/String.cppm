module;

#include <cstddef>
#include <stdint.h>

export module TUX.String;

export import TUX.Memory;

using TextPtr = char *;
using TextPtrConst = const char *;

export {

using TextLen = uint16_t;
using CharType = uint16_t;

bool GUI__strcmp(TextPtrConst s0, TextPtrConst s1) {
	if (s0 == s1) return true;
	if (!s0 || !s1) return false;
	for (auto c1 = *s0, c2 = *s1; c1 == c2; ++s0, ++s1)
		if (c1 == '\0')
			return true;
	return false;
}
TextLen GUI__strlen(TextPtrConst s) {
	if (!s) return 0;
	TextLen r = 0;
	while (*s++) r++;
	return r;
}
int GUI__strcpy(TextPtr sDest, TextPtrConst sSrc) {
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
bool GUI__SetText(char *&rfText, const char *s) {
	auto size = GUI__strlen(s);
	if (!size) {
		if (rfText)
			GUI_ALLOC_Free(rfText),
			rfText = nullptr;
		return true;
	}
	++size;
	rfText = (char *)(rfText ? GUI_ALLOC_Realloc(rfText, ++size) : GUI_ALLOC_Alloc(size));
	GUI__memcpy(rfText, s, size);
	return true;
}

class StringView {
protected:
	TextPtr pText;
public:
	StringView(TextPtr pText = nullptr) : pText(pText) {}
public:
	CharType Char() const {
		return *pText;
	}

	TextLen NumBytes() const {
		return pText ? GUI__strlen(pText) : 0;
	}
	TextLen NumChars() const {
		return NumBytes();
	}

	TextLen NumLines() const {
		if (!pText) return 0;
		TextLen NumLines = 1;
		for (auto s = pText; *s; ++s)
			if (*s == '\n')
				++NumLines;
		return NumLines;
	}
	TextLen NumCharsInLine() const {
		if (!pText) return 0;
		TextLen NumChars = 0;
		for (auto s = pText; *s && *s != '\n'; ++s)
			++NumChars;
		return NumChars;
	}
	TextLen MaxCharsInLines() const {
		if (!pText) return 0;
		TextLen MaxChars = 0;
		TextLen NumChars = 0;
		for (auto s = pText; *s; ++s) {
			if (*s == '\n') {
				if (NumChars > MaxChars)
					MaxChars = NumChars;
				NumChars = 0;
			}
			else
				++NumChars;
		}
		if (NumChars > MaxChars)
			MaxChars = NumChars;
		return MaxChars;
	}

	bool IsSameEncode(StringView s) const {
		return GUI__strcmp(pText, s.pText);
	}

	CharType CharNext() {
		return *pText++;
	}

public:
	operator bool() const noexcept { return pText; }
	bool operator==(StringView s) const noexcept { return pText == s.pText; }
	bool operator!=(StringView s) const noexcept { return pText != s.pText; }
	StringView &operator++(int) noexcept {
		CharNext();
		return *this;
	}
	StringView operator++() noexcept {
		auto r = *this;
		CharNext();
		return r;
	}
};

class String : protected StringView {

public:
	String() = default;
	String(const char *s) { GUI__SetText(pText, s); }
	String(const String &Other) { GUI__SetText(pText, Other.pText); }
	String(String &&Other) noexcept : StringView(Other.pText)
	{ Other.pText = nullptr; }
	~String() { Delete(); }
public:
	void Delete() noexcept {
		GUI_ALLOC_Free(pText);
		pText = nullptr;
	}
	bool Set(const char *s) {
		return GUI__SetText(pText, s);
	}
public:
	using StringView::Char;
	using StringView::NumBytes;
	using StringView::NumChars;
	using StringView::NumLines;
	using StringView::NumCharsInLine;
	using StringView::MaxCharsInLines;
	
	using StringView::IsSameEncode;

	CharType CharNext() const = delete;

	using StringView::operator bool;
	using StringView::operator==;
	using StringView::operator!=;
	StringView operator++(int) noexcept = delete;
	StringView operator++() noexcept = delete;
public:
	bool operator==(const String &s) const noexcept { return GUI__strcmp(pText, s.pText); }
	bool operator!=(const String &s) const noexcept { return !GUI__strcmp(pText, s.pText); }
	bool operator=(const char *s) { return GUI__SetText(pText, s); }
	operator const char *() const noexcept { return pText; }
};

}
