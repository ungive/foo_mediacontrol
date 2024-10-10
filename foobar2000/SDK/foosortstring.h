#pragma once
#include <pfc/sortstring.h>

#ifdef _WIN32
#include <memory> // std::unique_ptr<>
#endif

namespace fb2k {
	using pfc::sortString_t;
	using pfc::sortStringCompare;
	using pfc::makeSortString;
}
