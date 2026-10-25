#pragma once
#include "orcus.hpp"





ORCUS_DYNAMIC_LIBRARY void orcus_compile_mstring(orcus_string* str, orcus_variable* out, bool remove_debug_info=false);

inline void orcus_compile_cstring(const char* str, const orcus_int str_len, orcus_variable* out, bool remove_debug_info = false) {
	orcus_string mstr;
	mstr.ptr = (char*)str;
	mstr.size = str_len;
	orcus_compile_mstring(&mstr, out, remove_debug_info);
}