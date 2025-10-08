#include"../mercury.h"
#include"../mercury_error.h"
#include"../mercury_bytecode.h"

#include "mercury_lib_math.h"

#include "math.h"
#include <stdlib.h>

//returns the smallest value. takes vararg. if no values are provided, returns infinity
void mercury_lib_math_min(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_out < 1)return;
	mercury_variable* out = mercury_assign_var(M);
	out->data.f = INFINITY;
	out->type = M_TYPE_FLOAT;

	//we can do a backwards stack loop with no issue because the order doesn't matter here
	for (mercury_int a = 0; a < args_in;a++) {
		mercury_variable* var=mercury_popstack(M);

		//we just ignore non-number variables.
		if (var->type == M_TYPE_FLOAT) {
			if (out->type == M_TYPE_FLOAT) {
				if (var->data.f< out->data.f) {
					out->data.f = var->data.f;
				}
			}
			else {
				if (var->data.f< (mercury_float)out->data.i) {
					out->type = M_TYPE_FLOAT;
					out->data.f = var->data.f;
				}
			}
		}
		else if (var->type == M_TYPE_INT) {
			if (out->type == M_TYPE_FLOAT) {
				if ((mercury_float)var->data.i < out->data.f) {
					out->type = M_TYPE_INT;
					out->data.i = var->data.i;
				}
			}
			else {
				if (var->data.i < out->data.i) {
					out->data.i = var->data.i;
				}
			}
		}

		mercury_unassign_var(M, var);
	}

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}



//returns the biggestvalue. takes vararg. if no values are provided, returns negative infinity
void mercury_lib_math_max(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_out < 1)return;
	mercury_variable* out = mercury_assign_var(M);
	out->data.f = -INFINITY;
	out->type = M_TYPE_FLOAT;

	//we can do a backwards stack loop with no issue because the order doesn't matter here
	for (mercury_int a = 0; a < args_in; a++) {
		mercury_variable* var = mercury_popstack(M);

		//we just ignore non-number variables.
		if (var->type == M_TYPE_FLOAT) {
			if (out->type == M_TYPE_FLOAT) {
				if (var->data.f > out->data.f) {
					out->data.f = var->data.f;
				}
			}
			else {
				if (var->data.f > (mercury_float)out->data.i) {
					out->type = M_TYPE_FLOAT;
					out->data.f = var->data.f;
				}
			}
		}
		else if (var->type == M_TYPE_INT) {
			if (out->type == M_TYPE_FLOAT) {
				if ((mercury_float)var->data.i > out->data.f) {
					out->type = M_TYPE_INT;
					out->data.i = var->data.i;
				}
			}
			else {
				if (var->data.i > out->data.i) {
					out->data.i = var->data.i;
				}
			}
		}

		mercury_unassign_var(M, var);
	}

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}

//will return an int.
void mercury_lib_math_floor(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1)return;
	mercury_variable* out = mercury_assign_var(M);
	out->data.i = 0;
	out->type = M_TYPE_INT;

	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* val = mercury_popstack(M);
	if (val->type == M_TYPE_INT) {
		out->data.i = val->data.i;
	}
	else if (val->type == M_TYPE_FLOAT) {
		out->data.i=(mercury_int)floor(val->data.f);
	}
	else {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)val->type, (void*)M_TYPE_FLOAT);
		return;
	}
	mercury_unassign_var(M, val);

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}


//will return an int.
void mercury_lib_math_ceil(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1)return;
	mercury_variable* out = mercury_assign_var(M);
	out->data.f = 0;
	out->type = M_TYPE_INT;

	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* val = mercury_popstack(M);
	if (val->type == M_TYPE_INT) {
		out->data.i = val->data.i;
	}
	else if (val->type == M_TYPE_FLOAT) {
		out->data.i = (mercury_int)ceil(val->data.f);
	}
	else {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)val->type, (void*)M_TYPE_FLOAT);
		return;
	}
	mercury_unassign_var(M, val);

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}




void mercury_lib_math_to_radians(mercury_state* M, mercury_int args_in, mercury_int args_out) { //degrees to radians
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1)return;
	mercury_variable* out = mercury_assign_var(M);
	out->data.f = 0.0;
	out->type = M_TYPE_FLOAT;

	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* val = mercury_popstack(M);
	if (val->type == M_TYPE_INT) {
		out->data.f = (mercury_float)val->data.i;
	}
	else if (val->type == M_TYPE_FLOAT) {
		out->data.f = val->data.f;
	}
	else {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)val->type, (void*)M_TYPE_FLOAT);
		return;
	}

	mercury_unassign_var(M, val);

	out->data.f = (m_math_pi*out->data.f)/180.0;

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}


void mercury_lib_math_to_degrees(mercury_state* M, mercury_int args_in, mercury_int args_out) { //radians to degrees
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1)return;
	mercury_variable* out = mercury_assign_var(M);
	out->data.f = 0.0;
	out->type = M_TYPE_FLOAT;

	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* val = mercury_popstack(M);
	if (val->type == M_TYPE_INT) {
		out->data.f = (mercury_float)val->data.i;
	}
	else if (val->type == M_TYPE_FLOAT) {
		out->data.f = val->data.f;
	}
	else {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)val->type, (void*)M_TYPE_FLOAT);
		return;
	}

	mercury_unassign_var(M, val);

	out->data.f = (180.0 * out->data.f) / m_math_pi;

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}


void mercury_lib_math_log(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)2);
		return;
	};
	if (args_out < 1)return;

	mercury_variable* out = mercury_assign_var(M);
	out->data.f = 0.0;
	out->type = M_TYPE_FLOAT;

	for (mercury_int i = 2; i < args_in; i++) { //start at 2 because the second arg is optional
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* base=nullptr;
	mercury_variable* num= nullptr;

	if (args_in == 1) {
		num=mercury_popstack(M);
	}
	else {
		base=mercury_popstack(M);
		num=mercury_popstack(M);
	}

	if (num->type == M_TYPE_INT) {
		num->data.f = (mercury_float)num->data.i;
		num->type = M_TYPE_FLOAT;
	}
	else if (num->type != M_TYPE_FLOAT) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE,(void*)num->type, (void*)M_TYPE_FLOAT);
		return;
	}

	if (base) {
		if (base->type == M_TYPE_INT) {
			base->data.f = (mercury_float)base->data.i;
			base->type = M_TYPE_FLOAT;
		}
		else if (base->type != M_TYPE_FLOAT) {
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)base->type, (void*)M_TYPE_FLOAT);
			return;
		}
	}


	if (base) {
		out->data.f = log(num->data.f) / log(base->data.f);
		mercury_unassign_var(M, base);
	}
	else {
		out->data.f = log(num->data.f);
	}

	mercury_pushstack(M,out);
	mercury_unassign_var(M, num);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}



void mercury_lib_math_to_absolute(mercury_state* M, mercury_int args_in, mercury_int args_out) { //absolute value
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1)return;
	mercury_variable* out = mercury_assign_var(M);
	out->data.f = 0.0;
	out->type = M_TYPE_FLOAT;

	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* val = mercury_popstack(M);
	if (val->type == M_TYPE_INT) {
		out->type = M_TYPE_INT;
		out->data.i =abs(val->data.i);
	}
	else if (val->type == M_TYPE_FLOAT) { //since floats use a sign bit, we can use a simple bitwise to do stuff fast.
#ifdef MERCURY_64BIT
		out->data.i = 0x7FFFFFFFFFFFFFFF & val->data.i;
#else
		out->data.i = 0x7FFFFFFF &val->data.i;
#endif
	}
	else {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)val->type, (void*)M_TYPE_FLOAT);
		return;
	}

	mercury_unassign_var(M, val);

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}




void mercury_lib_math_to_sin(mercury_state* M, mercury_int args_in, mercury_int args_out) { //sine
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1)return;
	mercury_variable* out = mercury_assign_var(M);
	out->data.f = 0.0;
	out->type = M_TYPE_FLOAT;

	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* val = mercury_popstack(M);
	if (val->type == M_TYPE_INT) {
		out->data.f = (mercury_float)val->data.i;
	}
	else if (val->type == M_TYPE_FLOAT) {
		out->data.f = val->data.f;
	}
	else {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)val->type, (void*)M_TYPE_FLOAT);
		return;
	}

	mercury_unassign_var(M, val);

	out->data.f = sin( out->data.f);

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}

void mercury_lib_math_to_cos(mercury_state* M, mercury_int args_in, mercury_int args_out) { //cosine
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1)return;
	mercury_variable* out = mercury_assign_var(M);
	out->data.f = 0.0;
	out->type = M_TYPE_FLOAT;

	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* val = mercury_popstack(M);
	if (val->type == M_TYPE_INT) {
		out->data.f = (mercury_float)val->data.i;
	}
	else if (val->type == M_TYPE_FLOAT) {
		out->data.f = val->data.f;
	}
	else {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)val->type, (void*)M_TYPE_FLOAT);
		return;
	}

	mercury_unassign_var(M, val);

	out->data.f = cos(out->data.f);

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}


void mercury_lib_math_to_tan(mercury_state* M, mercury_int args_in, mercury_int args_out) { //tangent
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1)return;
	mercury_variable* out = mercury_assign_var(M);
	out->data.f = 0.0;
	out->type = M_TYPE_FLOAT;

	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* val = mercury_popstack(M);
	if (val->type == M_TYPE_INT) {
		out->data.f = (mercury_float)val->data.i;
	}
	else if (val->type == M_TYPE_FLOAT) {
		out->data.f = val->data.f;
	}
	else {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)val->type, (void*)M_TYPE_FLOAT);
		return;
	}

	mercury_unassign_var(M, val);

	out->data.f = tan(out->data.f);

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}

void mercury_lib_math_to_asin(mercury_state* M, mercury_int args_in, mercury_int args_out) { //arcsine
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1)return;
	mercury_variable* out = mercury_assign_var(M);
	out->data.f = 0.0;
	out->type = M_TYPE_FLOAT;

	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* val = mercury_popstack(M);
	if (val->type == M_TYPE_INT) {
		out->data.f = (mercury_float)val->data.i;
	}
	else if (val->type == M_TYPE_FLOAT) {
		out->data.f = val->data.f;
	}
	else {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)val->type, (void*)M_TYPE_FLOAT);
		return;
	}

	mercury_unassign_var(M, val);

	out->data.f = asin(out->data.f);

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}

void mercury_lib_math_to_acos(mercury_state* M, mercury_int args_in, mercury_int args_out) { //arccosine
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1)return;
	mercury_variable* out = mercury_assign_var(M);
	out->data.f = 0.0;
	out->type = M_TYPE_FLOAT;

	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* val = mercury_popstack(M);
	if (val->type == M_TYPE_INT) {
		out->data.f = (mercury_float)val->data.i;
	}
	else if (val->type == M_TYPE_FLOAT) {
		out->data.f = val->data.f;
	}
	else {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)val->type, (void*)M_TYPE_FLOAT);
		return;
	}

	mercury_unassign_var(M, val);

	out->data.f = acos(out->data.f);

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}


void mercury_lib_math_to_atan(mercury_state* M, mercury_int args_in, mercury_int args_out) { //arctangent
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1)return;
	mercury_variable* out = mercury_assign_var(M);
	out->data.f = 0.0;
	out->type = M_TYPE_FLOAT;

	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* val = mercury_popstack(M);
	if (val->type == M_TYPE_INT) {
		out->data.f = (mercury_float)val->data.i;
	}
	else if (val->type == M_TYPE_FLOAT) {
		out->data.f = val->data.f;
	}
	else {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)val->type, (void*)M_TYPE_FLOAT);
		return;
	}

	mercury_unassign_var(M, val);

	out->data.f = atan(out->data.f);

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}

void mercury_lib_math_to_atan2(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1)return;
	mercury_variable* out = mercury_assign_var(M);
	out->data.f = 0.0;
	out->type = M_TYPE_FLOAT;

	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* val = mercury_popstack(M);
	if (val->type == M_TYPE_INT) {
		out->data.f = (mercury_float)val->data.i;
	}
	else if (val->type == M_TYPE_FLOAT) {
		out->data.f = val->data.f;
	}
	else {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)val->type, (void*)M_TYPE_FLOAT);
		return;
	}

	mercury_variable* y = mercury_popstack(M);
	if (y->type == M_TYPE_INT) {
		y->data.f = (mercury_float)y->data.i;
	}
	else if (val->type == M_TYPE_FLOAT) {
		y->data.f = y->data.f;
	}
	else {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)val->type, (void*)M_TYPE_FLOAT);
		return;
	}

	out->data.f = atan2(y->data.f,out->data.f);

	mercury_unassign_var(M, val);
	mercury_unassign_var(M, y);

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}



/* why implement our own randomness function?
well... because C's stdlib random kinda blows. only 15 bits of space (on windows, at least)? oh please.
This probably isn't as performant, but you're going to lose speed with mercury being interpreted so...
but hey, with this we can really customize it, you can save the random state! also 64 bits of width! */
#ifdef MERCURY_64BIT
const uint64_t M_RANDOM_MAX = 0xFFFFFFFFFFFFFFFF; //for the sake of porting C code over.
uint64_t M_RANDOM_STATE = 0x0000DEADBEEF0000;
/*
basic xorshift
numbers from the paper: George Marsaglia Xorshift RNGs
*/
uint64_t m_random() { //basic Xorshift
	uint64_t n = M_RANDOM_STATE;
	n ^= n << 3;
	n ^= n >> 27;
	n ^= n << 11;
	M_RANDOM_STATE = n;
	return n;
}
#else
const uint32_t M_RANDOM_MAX = 0xFFFFFFFF;
uint32_t M_RANDOM_STATE = 0xDEADBEEF;
uint32_t m_random() {
	uint32_t n = M_RANDOM_STATE;
	n ^= n << 4;
	n ^= n >> 9;
	n ^= n << 13;
	M_RANDOM_STATE = n;
	return n;
}
#endif







void mercury_lib_math_random(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in ==1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)2);
		return;
	};
	for (mercury_int i = 2; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* v2 = mercury_popstack(M);
	mercury_variable* v1 = mercury_popstack(M);

	if (args_in && (v1->type != M_TYPE_INT) && (v1->type != M_TYPE_FLOAT) ) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)v1->type, (void*)M_TYPE_FLOAT);
		return;
	}

	if (args_in &&  (v2->type != M_TYPE_INT) && (v2->type != M_TYPE_FLOAT)) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)v2->type, (void*)M_TYPE_FLOAT);
		return;
	}

	mercury_uint r = m_random();
	mercury_float f = (mercury_float)r / (mercury_float)M_RANDOM_MAX;

	if (!args_out)return;


	mercury_variable* out=mercury_assign_var(M);
	out->type = M_TYPE_FLOAT;
	if (!args_in) {
		out->data.f = f;
	}else{
		mercury_float min=0.0;
		mercury_float max=0.0;
		if (v2->type == M_TYPE_INT) {
			max = (mercury_float)v2->data.i;
		}
		else {
			max = v2->data.f;
		}

		if (v1->type == M_TYPE_INT) {
			min = (mercury_float)v1->data.i;
		}
		else {
			min = v1->data.f;
		}

		out->data.f = (f*(max-min)) + min;
	}

	mercury_unassign_var(M, v1);
	mercury_unassign_var(M, v2);

	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}


void mercury_lib_math_randomint(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 2) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)2);
		return;
	};
	for (mercury_int i = 2; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* v2 = mercury_popstack(M);
	mercury_variable* v1 = mercury_popstack(M);

	if ( (v1->type != M_TYPE_INT)) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)v1->type, (void*)M_TYPE_INT);
		return;
	}

	if ( (v2->type != M_TYPE_INT)) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)v2->type, (void*)M_TYPE_INT);
		return;
	}

	mercury_uint r = m_random();

	if (!args_out)return;


	mercury_variable* out = mercury_assign_var(M);
	out->type = M_TYPE_INT;

	mercury_int min = v1->data.i;
	mercury_int max = v2->data.i;

	out->data.i = ((r) % (max - min + 1)) + min;

	mercury_unassign_var(M, v1);
	mercury_unassign_var(M, v2);

	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}



void mercury_lib_math_randomseed(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	if (!args_in) {
		if (args_out) {
			mercury_variable* o = mercury_assign_var(M);
			o->type = M_TYPE_INT;
			o->data.i = M_RANDOM_STATE;
			mercury_pushstack(M,o);
		}
		for (mercury_int a = 1; a < args_out; a++) {
			M_BYTECODE_NNIL(M, 0);
		}
	}
	else {
		mercury_variable* v1 = mercury_popstack(M);

		if ((v1->type != M_TYPE_INT)) {
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)v1->type, (void*)M_TYPE_INT);
			return;
		}

		M_RANDOM_STATE = v1->data.i;
		mercury_unassign_var(M, v1);

		for (mercury_int a = 0; a < args_out; a++) {
			M_BYTECODE_NNIL(M, 0);
		}
	}


}


void mercury_lib_math_isnan(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1)return;

	mercury_variable* out = mercury_assign_var(M);
	out->data.i = 0;
	out->type = M_TYPE_BOOL;

	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* val = mercury_popstack(M);

	if (val->type == M_TYPE_FLOAT) {
#ifdef MERCURY_64BIT
		out->data.i = (val->data.i & 0x7ff0000000000000)== 0x7ff0000000000000; //NaN will always have all exponent bits checked. this also will catch infinity, which i suppose technically isn't a number.
#else
		out->data.i = (val->data.i & 0x7f800000)== 0x7f800000;
#endif
	}
	mercury_unassign_var(M, val);

	mercury_pushstack(M, out);
	for (mercury_int a = 1; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}