#pragma once

// Mitigation for Objective-C class name clashes
// Your component must have its own foobar2000-mac-class-suffix.h, with a single line:
// #define FOOBAR2000_MAC_CLASS_SUFFIX _foo_mycomponentname
// This suffixes all common class names with _foo_mycomponentname preventing clashes

#include "foobar2000-mac-class-suffix.h"

#ifndef FOOBAR2000_MAC_CLASS_SUFFIX
#error PLEASE DECLARE FOOBAR2000_MAC_CLASS_SUFFIX PROPERLY
#endif

#define FB2K_OBJC_CLASS(X) _FB2K_OBJC_CONCAT(X, FOOBAR2000_MAC_CLASS_SUFFIX)
#define _FB2K_OBJC_CONCAT(a, b) _FB2K_OBJC_CONCAT_INNER(a, b)
#define _FB2K_OBJC_CONCAT_INNER(a, b) a ## b
