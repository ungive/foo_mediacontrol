// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// https://cplusplus.com/forum/windows/129472/
#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0601	// Change this to the appropriate value to target other versions of Windows.
#endif						


// TODO: reference additional headers your program requires here

// #define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <sysinfoapi.h>
#include <synchapi.h>
#include <shldisp.h>
#include <windows.h>

#include "ATLHelpers.h"
