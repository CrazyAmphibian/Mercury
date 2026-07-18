#pragma once
#include "mercury.hpp"





MERCURY_DYNAMIC_LIBRARY void mercury_compile_mstring(mercury_stringliteral* str, mercury_variable* out, bool remove_debug_info=false);