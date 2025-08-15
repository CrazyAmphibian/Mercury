/*
the standard library.
contains things for basic debugging and some misc. utility functions.

*/
#include "../mercury.h"


#define MERCURY_LIB_MATH


void mercury_lib_math_min(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_max(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_floor(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_ceil(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_to_radians(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_to_degrees(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_log(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_to_absolute(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_to_sin(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_to_cos(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_to_tan(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_to_asin(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_to_acos(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_to_atan(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_to_atan2(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_isnan(mercury_state* M, mercury_int args_in, mercury_int args_out);


void mercury_lib_math_random(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_randomint(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_math_randomseed(mercury_state* M, mercury_int args_in, mercury_int args_out);

const mercury_float m_math_pi = 3.141592653589793; //15 digits should be enough i hope. doubles cannot store a more precise approximation than this.
const mercury_float m_math_root2 = 1.41421356237309504; //gee bill, 17 digits?
const mercury_float m_math_e = 2.718281828459045; // ditto.
const mercury_float m_math_root3 = 1.7320508075688772; //16 digits
const mercury_float m_math_golden = 1.6180339887498948; //16 digits
#ifdef MERCURY_64BIT
const mercury_int m_math_intmax = 0x7FFFFFFFFFFFFFFF;
const mercury_int m_math_uintmax =0xFFFFFFFFFFFFFFFF;
#else
const mercury_int m_math_intmax = 0x7FFFFFFF;
const mercury_int m_math_uintmax = 0xFFFFFFFF;
#endif


