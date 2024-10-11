#pragma once

/*
foobar2000 shared.dll hook implementations
If you're getting linker multiple-definition errors on these, change build configuration of PFC from "Debug" / "Release" to "Debug FB2K" / "Release FB2K"
Configurations with "FB2K" suffix disable compilation of pfc-fb2k-hooks.cpp allowing these methods to be redirected to shared.dll calls
*/

namespace pfc {
	[[noreturn]] void crashImpl();
	// Fix: error LNK2005: "void __cdecl pfc::crashHook(void)" (?crashHook@pfc@@YAXXZ) already defined in pfc.lib(pfc-fb2k-hooks.obj)
	/*[[noreturn]] void crashHook() {
		crashImpl();
	}*/
#ifdef _WIN32
	BOOL winFormatSystemErrorMessageImpl(pfc::string_base & p_out, DWORD p_code);
	// Fix: error LNK2005: "int __cdecl pfc::winFormatSystemErrorMessageHook(class pfc::string_base &,unsigned long)" (?winFormatSystemErrorMessageHook@pfc@@YAHAAVstring_base@1@K@Z) already defined in pfc.lib(pfc-fb2k-hooks.obj)
	/*BOOL winFormatSystemErrorMessageHook(pfc::string_base& p_out, DWORD p_code) {
		return winFormatSystemErrorMessageImpl(p_out, p_code);
	}*/
#endif
}
