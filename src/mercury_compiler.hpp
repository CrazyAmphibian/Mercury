#pragma once
#include "mercury.hpp"





MERCURY_DYNAMIC_LIBRARY mercury_variable* mercury_compile_mstring(mercury_stringliteral* str, bool remove_debug_info=false);