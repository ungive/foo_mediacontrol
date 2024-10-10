#pragma once

#ifdef _WIN32
#include <memory> // std::unique_ptr<>
#endif
#ifdef __APPLE__
#include "CFObject.h"
#endif

namespace pfc {
#ifdef _WIN32
	typedef std::unique_ptr<wchar_t[]> sortString_t;
	sortString_t makeSortString(const char* str);
	sortString_t makeSortString(const wchar_t* str);
	int sortStringCompare(sortString_t const& s1, sortString_t const& s2);
    int sortStringCompareI(sortString_t const& s1, sortString_t const& s2);
#elif defined(__APPLE__)
    typedef CFObject<CFStringRef> sortString_t;
    sortString_t makeSortString(const char* str);
    int sortStringCompare(sortString_t const& s1, sortString_t const& s2);
    int sortStringCompareI(sortString_t const& s1, sortString_t const& s2);
#else
#define PFC_SORTSTRING_GENERIC
	typedef pfc::string8 sortString_t;
	inline sortString_t makeSortString(const char* str) { return str; }
	inline sortString_t makeSortString(pfc::string8&& str) { return std::move(str); }
	int sortStringCompare(const char* str1, const char* str2);
    int sortStringCompareI(const char* str1, const char* str2);
#endif
}
