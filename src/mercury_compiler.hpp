#pragma once
#include "mercury.hpp"





MERCURY_DYNAMIC_LIBRARY void mercury_compile_mstring(mercury_string* str, mercury_variable* out, bool remove_debug_info=false);

inline void mercury_compile_cstring(const char* str, const mercury_int str_len, mercury_variable* out, bool remove_debug_info = false) {
	mercury_string mstr;
	mstr.ptr = (char*)str;
	mstr.size = str_len;
	mercury_compile_mstring(&mstr, out, remove_debug_info);
}