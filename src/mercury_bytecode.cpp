#include "mercury_bytecode.hpp"
#include "mercury.hpp"
#include "mercury_error.hpp"
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>


void M_BYTECODE_NOP(mercury_state* const M_CPP_restrict M) {
	return;
}


void M_BYTECODE_ADD(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack -1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type == M_TYPE_FLOAT) {
		if (st->type == M_TYPE_INT) {
			st->type = M_TYPE_FLOAT;
			st->data.f = (mercury_float)st->data.i;
		}
	}else if (ss->type == M_TYPE_INT) {
		if (st->type == M_TYPE_FLOAT) {
			ss->type = M_TYPE_FLOAT;
			ss->data.f = (mercury_float)ss->data.i;
		}
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 1);
		return;
	}

	if (st->type == M_TYPE_FLOAT) {
		ss->data.f += st->data.f;
	}
	else if (st->type == M_TYPE_INT) {
		ss->data.i += st->data.i;
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 2);
		return;
	}

	M->sizeofstack--;
	return;
}

void M_BYTECODE_SUB(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type == M_TYPE_FLOAT) {
		if (st->type == M_TYPE_INT) {
			st->type = M_TYPE_FLOAT;
			st->data.f = (mercury_float)st->data.i;
		}
	}
	else if (ss->type == M_TYPE_INT) {
		if (st->type == M_TYPE_FLOAT) {
			ss->type = M_TYPE_FLOAT;
			ss->data.f = (mercury_float)ss->data.i;
		}
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 1);
		return;
	}

	if (st->type == M_TYPE_FLOAT) {
		ss->data.f -= st->data.f;
	}
	else if (st->type == M_TYPE_INT) {
		ss->data.i -= st->data.i;
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 2);
		return;
	}

	M->sizeofstack--;
	return;
}

void M_BYTECODE_MUL(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type == M_TYPE_FLOAT) {
		if (st->type == M_TYPE_INT) {
			st->type = M_TYPE_FLOAT;
			st->data.f = (mercury_float)st->data.i;
		}
	}
	else if (ss->type == M_TYPE_INT) {
		if (st->type == M_TYPE_FLOAT) {
			ss->type = M_TYPE_FLOAT;
			ss->data.f = (mercury_float)ss->data.i;
		}
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 1);
		return;
	}

	if (st->type == M_TYPE_FLOAT) {
		ss->data.f *= st->data.f;
	}
	else if (st->type == M_TYPE_INT) {
		ss->data.i *= st->data.i;
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 2);
		return;
	}

	M->sizeofstack--;
	return;
}

void M_BYTECODE_DIV(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type == M_TYPE_FLOAT) {
	}
	else if (ss->type == M_TYPE_INT) {
		ss->data.f = ss->data.i;
		ss->type = M_TYPE_FLOAT;
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 1);
		return;
	}

	if (st->type == M_TYPE_FLOAT) {
		ss->data.f /= st->data.f;
	}
	else if (st->type == M_TYPE_INT) {
		ss->data.f /= (mercury_float)st->data.i;
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 2);
		return;
	}

	M->sizeofstack--;
	return;
}

void M_BYTECODE_POW(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type == M_TYPE_FLOAT) {
	}
	else if (ss->type == M_TYPE_INT) {
		ss->data.f = ss->data.i;
		ss->type = M_TYPE_FLOAT;
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 1);
		return;
	}

	if (st->type == M_TYPE_FLOAT) {
		ss->data.f = pow(ss->data.f,st->data.f);
	}
	else if (st->type == M_TYPE_INT) {
		ss->data.f = pow(ss->data.f, (mercury_float)st->data.i);
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 2);
		return;
	}

	M->sizeofstack--;
	return;
}


void M_BYTECODE_IDIV(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type == M_TYPE_FLOAT) {
		ss->type = M_TYPE_INT;
		ss->data.i = (mercury_int)ss->data.f;
	}
	else if (ss->type == M_TYPE_INT) {
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 1);
		return;
	}

	if (st->type == M_TYPE_FLOAT) {
		st->data.i = (mercury_int)st->data.f;
		if (!st->data.i) {
			mercury_raise_error(M, M_ERROR_DIV_ZERO);
			return;
		}
		ss->data.i /= st->data.i;
	}
	else if (st->type == M_TYPE_INT) {
		if (!st->data.i) {
			mercury_raise_error(M, M_ERROR_DIV_ZERO);
			return;
		}
		ss->data.i /= st->data.i;
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 2);
		return;
	}

	M->sizeofstack--;
	return;
}

void M_BYTECODE_MOD(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type == M_TYPE_FLOAT) {
		if (st->type == M_TYPE_INT) {
			st->type = M_TYPE_FLOAT;
			st->data.f = (mercury_float)st->data.i;
		}
	}
	else if (ss->type == M_TYPE_INT) {
		if (st->type == M_TYPE_FLOAT) {
			ss->type = M_TYPE_FLOAT;
			ss->data.f = (mercury_float)ss->data.i;
		}
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 1);
		return;
	}

	if (st->type == M_TYPE_FLOAT) {
		ss->data.f = fmod(ss->data.f,st->data.f);
	}
	else if (st->type == M_TYPE_INT) {
		if (!st->data.i) {
			mercury_raise_error(M, M_ERROR_DIV_ZERO);
			return;
		}
		ss->data.i %= st->data.i;
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 2);
		return;
	}

	M->sizeofstack--;
	return;
}

void M_BYTECODE_BAND(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if(ss->type != M_TYPE_INT && ss->type != M_TYPE_FLOAT) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 2);
		return;
	}

	if (st->type != M_TYPE_INT && st->type != M_TYPE_FLOAT) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 1);
		return;
	}
	ss->data.i &= st->data.i;

	M->sizeofstack--;
	return;
}

void M_BYTECODE_BOR(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type != M_TYPE_INT && ss->type != M_TYPE_FLOAT) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 2);
		return;
	}

	if (st->type != M_TYPE_INT && st->type != M_TYPE_FLOAT) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 1);
		return;
	}
	ss->data.i |= st->data.i;

	M->sizeofstack--;
	return;
}

void M_BYTECODE_BXOR(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type != M_TYPE_INT && ss->type != M_TYPE_FLOAT) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 2);
		return;
	}

	if (st->type != M_TYPE_INT && st->type != M_TYPE_FLOAT) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 1);
		return;
	}
	ss->data.i ^= st->data.i;

	M->sizeofstack--;
	return;
}

void M_BYTECODE_BNOT(mercury_state* const M_CPP_restrict M) {
	if (!M->sizeofstack) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}
	mercury_variable* st = M->stack + M->sizeofstack - 1;
	if (st->type != M_TYPE_INT && st->type != M_TYPE_FLOAT) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 1);
		return;
	}
#ifdef MERCURY_64BIT
	st->data.i ^= 0xffffffffffffffff;
#else
	st->data.i ^= 0xffffffff;
#endif
	return;
}

void M_BYTECODE_BSHL(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type != M_TYPE_INT && ss->type != M_TYPE_FLOAT) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 2);
		return;
	}

	if (st->type != M_TYPE_INT && st->type != M_TYPE_FLOAT) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 1);
		return;
	}

#ifdef MERCURY_64BIT
	ss->data.i <<= (st->data.i & 63);
#else
	ss->data.i <<= (st->data.i & 31);
#endif

	M->sizeofstack--;
	return;
}

void M_BYTECODE_BSHR(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type != M_TYPE_INT && ss->type != M_TYPE_FLOAT) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 2);
		return;
	}

	if (st->type != M_TYPE_INT && st->type != M_TYPE_FLOAT) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 1);
		return;
	}

#ifdef MERCURY_64BIT
	ss->data.i >>= (st->data.i & 63);
#else
	ss->data.i >>= (st->data.i & 31);
#endif

	M->sizeofstack--;
	return;
}



void M_BYTECODE_LAND(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}
	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	bool c1 = mercury_checkbool(st);
	bool c2 = mercury_checkbool(ss);

	if (c1 && c2) {
		mercury_release_var(ss);
		*ss = *st;
	}
	else {
		mercury_release_var(st);
		mercury_release_var(ss);
		ss->data.i = 0;
		ss->type = M_TYPE_BOOL;
	}

	M->sizeofstack--;
	return;
}

void M_BYTECODE_LOR(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}
	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	bool c1 = mercury_checkbool(st);
	bool c2 = mercury_checkbool(ss);

	if (c2) {
		mercury_release_var(st);
	}
	else {
		if (c1) {
			mercury_release_var(ss);
			*ss = *st;
		}
		else {
			mercury_release_var(st);
			mercury_release_var(ss);
			ss->data.i = 0;
			ss->type = M_TYPE_BOOL;
		}
	}

	M->sizeofstack--;
	return;
}

void M_BYTECODE_LXOR(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	bool c1=mercury_checkbool(st);
	bool c2=mercury_checkbool(ss);
	
	if ((c1 && c2) || (!c1 && !c2)) {
		mercury_release_var(st);
		mercury_release_var(ss);
		ss->data.i = 0;
		ss->type = M_TYPE_BOOL;
	}
	else if (c1) {
		mercury_release_var(ss);
		*ss = *st;
	}
	else {
		mercury_release_var(st);
	}

	M->sizeofstack--;
	return;
}

void M_BYTECODE_LNOT(mercury_state* const M_CPP_restrict M) {
	if (!M->sizeofstack) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	bool c= !mercury_checkbool(st);
	mercury_release_var(st);
	st->data.i = c;
	st->type = M_TYPE_BOOL;

	return;
}


void M_BYTECODE_EQL(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}
	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	bool c = mercury_vars_equal(ss, st);
	mercury_release_var(st);
	mercury_release_var(ss);
	ss->data.i = c;
	ss->type = M_TYPE_BOOL;

	M->sizeofstack--;
	return;
}

void M_BYTECODE_NEQ(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}
	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	bool c= !mercury_vars_equal(ss, st);
	mercury_release_var(st);
	mercury_release_var(ss);
	ss->data.i = c;
	ss->type = M_TYPE_BOOL;

	M->sizeofstack--;
	return;
}


void M_BYTECODE_GRT(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type == M_TYPE_FLOAT) {
		if (st->type == M_TYPE_INT) {
			st->type = M_TYPE_FLOAT;
			st->data.f = (mercury_float)st->data.i;
		}
	}
	else if (ss->type == M_TYPE_INT) {
		if (st->type == M_TYPE_FLOAT) {
			ss->data.f = (mercury_float)ss->data.i;
		}
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 1);
		return;
	}
	ss->type = M_TYPE_BOOL;

	if (st->type == M_TYPE_FLOAT) {
		ss->data.i = ss->data.f > st->data.f;
	}
	else if (st->type == M_TYPE_INT) {
		ss->data.i = ss->data.i > st->data.i;
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 2);
		return;
	}

	M->sizeofstack--;
	return;
}

void M_BYTECODE_LET(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type == M_TYPE_FLOAT) {
		if (st->type == M_TYPE_INT) {
			st->type = M_TYPE_FLOAT;
			st->data.f = (mercury_float)st->data.i;
		}
	}
	else if (ss->type == M_TYPE_INT) {
		if (st->type == M_TYPE_FLOAT) {
			ss->data.f = (mercury_float)ss->data.i;
		}
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 1);
		return;
	}
	ss->type = M_TYPE_BOOL;

	if (st->type == M_TYPE_FLOAT) {
		ss->data.i = ss->data.f < st->data.f;
	}
	else if (st->type == M_TYPE_INT) {
		ss->data.i = ss->data.i < st->data.i;
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 2);
		return;
	}

	M->sizeofstack--;
	return;
}

void M_BYTECODE_GTE(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type == M_TYPE_FLOAT) {
		if (st->type == M_TYPE_INT) {
			st->type = M_TYPE_FLOAT;
			st->data.f = (mercury_float)st->data.i;
		}
	}
	else if (ss->type == M_TYPE_INT) {
		if (st->type == M_TYPE_FLOAT) {
			ss->data.f = (mercury_float)ss->data.i;
		}
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 1);
		return;
	}
	ss->type = M_TYPE_BOOL;

	if (st->type == M_TYPE_FLOAT) {
		ss->data.i = ss->data.f >= st->data.f;
	}
	else if (st->type == M_TYPE_INT) {
		ss->data.i = ss->data.i >= st->data.i;
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 2);
		return;
	}

	M->sizeofstack--;
	return;
}

void M_BYTECODE_LTE(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack < 2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;

	if (ss->type == M_TYPE_FLOAT) {
		if (st->type == M_TYPE_INT) {
			st->type = M_TYPE_FLOAT;
			st->data.f = (mercury_float)st->data.i;
		}
	}
	else if (ss->type == M_TYPE_INT) {
		if (st->type == M_TYPE_FLOAT) {
			ss->data.f = (mercury_float)ss->data.i;
		}
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, ss, 1);
		return;
	}
	ss->type = M_TYPE_BOOL;

	if (st->type == M_TYPE_FLOAT) {
		ss->data.i = ss->data.f <= st->data.f;
	}
	else if (st->type == M_TYPE_INT) {
		ss->data.i = ss->data.i <= st->data.i;
	}
	else {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, st, 2);
		return;
	}

	M->sizeofstack--;
	return;
}

void M_BYTECODE_SENV(mercury_state* const M_CPP_restrict M) {
	mercury_variable key;
	mercury_popstack(M, &key);
	mercury_variable value;
	mercury_popstack(M,&value);


	mercury_state* check_state = M;
	while (check_state) {
		mercury_subtable* st = check_state->enviroment->data+key.type;
		mercury_int sz = st->size;
		while (sz) {
			sz--;
			if (mercury_vars_equal(st->keys + sz, &key)) {
				mercury_increment_variable_refrence_count(&value);
				mercury_decrement_variable_refrence_count(st->values + sz);
				mercury_release_var(&key);
				mercury_release_var(st->values + sz);
				st->values[sz] = value;
				return;
			}
		}


		check_state = check_state->parentstate;
	}

	mercury_subtable st = M->enviroment->data[key.type];
	void* nptr = realloc(st.keys, sizeof(mercury_variable) * (st.size + 1));
	if (!nptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	st.keys = (mercury_variable*)nptr;
	nptr = realloc(st.values, sizeof(mercury_variable) * (st.size + 1));
	if (!nptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	st.values = (mercury_variable*)nptr;

	mercury_increment_variable_refrence_count(&key);
	mercury_increment_variable_refrence_count(&value);

	st.keys[st.size] = key;
	st.values[st.size] = value;

	st.size++;
	M->enviroment->data[key.type] = st;
}

void M_BYTECODE_GENV(mercury_state* const M_CPP_restrict M) {
	mercury_variable key;
	mercury_popstack(M, &key);

	mercury_state* check_state = M;
	while (check_state) {
		mercury_subtable st= check_state->enviroment->data[key.type];
		while (st.size){
			st.size--;
			if (mercury_vars_equal(st.keys+ st.size, &key)) {
				mercury_release_var(&key);
				mercury_pushstack(M, st.values + st.size);
				return;
			}
		}


		check_state = check_state->parentstate;
	}
	mercury_variable value;
	value.data.i = 0;
	value.type = M_TYPE_NIL;
	mercury_pushstack(M, &value);
	mercury_release_var(&key);
}


void M_BYTECODE_SET(mercury_state* const M_CPP_restrict M) {
	mercury_variable key;
	mercury_popstack(M, &key);
	mercury_variable table;
	mercury_popstack(M, &table);
	mercury_variable value;
	mercury_popstack(M, &value);

	switch (table.type) {
	case M_TYPE_TABLE:
		mercury_setkey((mercury_table*)table.data.p, &key, &value);
		break;
	case M_TYPE_ARRAY:
		if (key.type != M_TYPE_INT) {
			mercury_raise_error_nonpointer(M, M_ERROR_INVALID_INDEX, key.type);
			mercury_release_var(&value);
			mercury_release_var(&key);
			mercury_release_var(&table);
			return;
		}
		mercury_setarray((mercury_array*)table.data.p, &value, key.data.i);
		break;
	default:
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_STORAGETYPE_VARIABLEPROVIDED, &table, 1);
		mercury_release_var(&value);
		mercury_release_var(&key);
		mercury_release_var(&table);
		return;
	}
	//mercury_release_var(&table); this breaks tables defined like a={1,2,3}
}

void M_BYTECODE_GET(mercury_state* const M_CPP_restrict M) {
	mercury_variable key;
	mercury_popstack(M, &key);
	mercury_variable table;
	mercury_popstack(M, &table);

	mercury_variable value;

	switch (table.type) {
	case M_TYPE_TABLE:
		mercury_getkey((mercury_table*)table.data.p, &key, &value);
		break;
	case M_TYPE_ARRAY:
		if (key.type != M_TYPE_INT) {
			mercury_raise_error_nonpointer(M, M_ERROR_INVALID_INDEX, key.type);
			mercury_release_var(&key);
			mercury_release_var(&table);
			return;
		}
		mercury_getarray((mercury_array*)table.data.p, key.data.i,&value);
		break;
	case M_TYPE_STRING:
		if (key.type != M_TYPE_INT) {
			mercury_raise_error_nonpointer(M, M_ERROR_INVALID_INDEX, table.type);
			mercury_release_var(&key);
			mercury_release_var(&table);
			return;
		}
		
		value.type = M_TYPE_STRING;
		value.data.p = mercury_mstring_substring((mercury_string*)table.data.p, key.data.i, key.data.i);
		break;
	default:
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_STORAGETYPE_VARIABLEPROVIDED, &table, 1);
		mercury_release_var(&key);
		mercury_release_var(&table);
		return;
	}
	mercury_increment_variable_refrence_count(&value); //so the value cannot be freed before it is sent out
	mercury_release_var(&table);
	mercury_decrement_variable_refrence_count(&value);
	mercury_pushstack(M, &value);
}



void M_BYTECODE_SREG(mercury_state* const M_CPP_restrict M) {
	const mercury_uint regnum = *(mercury_uint*)(M->bytecode.instructions + M->programcounter);

	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;

	mercury_variable v;
	if (regnum <= register_max) {
		mercury_popstack(M, &v);
		M->registers[regnum] = v;
	}
	else {
		mercury_discard_top_of_stack(M);
	}

}


void M_BYTECODE_GREG(mercury_state* const M_CPP_restrict M) {
	const mercury_uint regnum = *(mercury_uint*)(M->bytecode.instructions + M->programcounter);

	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;

	if (regnum <= register_max) {
		mercury_pushstack(M, M->registers+regnum);
	}
	else {
		mercury_variable out;
		
		out.type = M_TYPE_NIL;
		out.data.i = 0;
		mercury_pushstack(M, &out);
	}

}

void M_BYTECODE_NINT(mercury_state* const M_CPP_restrict M) { //New INTeger
	void* const offset = M->bytecode.instructions + M->programcounter;

	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;

	mercury_variable out;
	
	out.type = M_TYPE_INT;
	out.data.i= *(mercury_int*)offset;
	mercury_pushstack(M, &out);
}

void M_BYTECODE_NFLO(mercury_state* const M_CPP_restrict M) { //New FLOat
	void* const offset = M->bytecode.instructions + M->programcounter;

	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;

	mercury_variable out;
	
	out.type = M_TYPE_FLOAT;
	out.data.f = *(mercury_float*)offset;
	mercury_pushstack(M, &out);
}

void M_BYTECODE_NTRU(mercury_state* const M_CPP_restrict M) { //New TRUe
	mercury_variable out;
	
	out.type = M_TYPE_BOOL;
	out.data.i = 1;
	mercury_pushstack(M, &out);
}

void M_BYTECODE_NFAL(mercury_state* const M_CPP_restrict M) { //New FALse
	mercury_variable out;
	
	out.type = M_TYPE_BOOL;
	out.data.i = 0;
	mercury_pushstack(M, &out);
}

void M_BYTECODE_NNIL(mercury_state* const M_CPP_restrict M) { //New NIL
	mercury_variable out;
	
	out.type = M_TYPE_NIL;
	out.data.i = 0;
	mercury_pushstack(M, &out);
}

void M_BYTECODE_NSTR(mercury_state* const M_CPP_restrict M) { //New STRing
	const mercury_uint string_size = *(mercury_uint*)(M->bytecode.instructions + M->programcounter);
	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;

	mercury_variable out;
	
	mercury_string* so=mercury_cstring_to_mstring( (char*)(M->bytecode.instructions + M->programcounter), string_size);
	if (!so) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	out.data.p = so;
	out.type = M_TYPE_STRING;
	M->programcounter += (string_size+ sizeof(mercury_opcode)-1)/sizeof(mercury_opcode);

	mercury_pushstack(M, &out);
}

void M_BYTECODE_NFUN(mercury_state* const M_CPP_restrict M) { //New FUNction / No FUN
	const mercury_uint function_size = *(mercury_uint*)(M->bytecode.instructions + M->programcounter);
	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;

	mercury_function* const fptr= (mercury_function*)malloc(sizeof(mercury_function));
	if (fptr == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}

	if (M->bytecode.numberofinstructions < M->programcounter + function_size) {
		mercury_raise_error(M, M_ERROR_INSTRUCTION_FAILIURE);
		return;
	}

	fptr->refrences = 0;
	fptr->numberofinstructions = function_size;
	fptr->instructions = (mercury_opcode*)malloc(function_size * sizeof(mercury_opcode));
	fptr->enviromental = false;
	
	if (fptr->instructions == nullptr) {
		free(fptr);
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}


	memcpy(fptr->instructions, M->bytecode.instructions + M->programcounter, function_size * sizeof(mercury_opcode));

	if (M->bytecode.instruction_dbg_lookup) {
		fptr->instruction_dbg_lookup = (mercury_uint*)malloc(sizeof(mercury_uint) * function_size);
		if (fptr->instruction_dbg_lookup == nullptr) {
			free(fptr->instructions);
			free(fptr);
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		memcpy(fptr->instruction_dbg_lookup, M->bytecode.instruction_dbg_lookup + M->programcounter, function_size * sizeof(mercury_uint));
		fptr->dbg_tokens = (mercury_debug_token*)malloc(sizeof(mercury_debug_token) * M->bytecode.num_dbg_tokens);
		if (fptr->dbg_tokens == nullptr) {
			free(fptr->instructions);
			free(fptr->instruction_dbg_lookup);
			free(fptr);
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		for (mercury_uint i = 0; i < M->bytecode.num_dbg_tokens; i++) {
			fptr->dbg_tokens[i] = M->bytecode.dbg_tokens[i];
			fptr->dbg_tokens[i].chars = (char*)malloc(M->bytecode.dbg_tokens[i].num_chars);
			if (!fptr->dbg_tokens[i].chars) {
				free(fptr->instructions);
				free(fptr->dbg_tokens);
				free(fptr->instruction_dbg_lookup);
				free(fptr);
				mercury_raise_error(M, M_ERROR_ALLOCATION);
				return;
			}
			fptr->num_dbg_tokens = M->bytecode.num_dbg_tokens;
			memcpy(fptr->dbg_tokens[i].chars, M->bytecode.dbg_tokens[i].chars, M->bytecode.dbg_tokens[i].num_chars);
		}
	}
	else {
		fptr->instruction_dbg_lookup = nullptr;
		fptr->num_dbg_tokens = 0;
		fptr->dbg_tokens = nullptr;
	}


	mercury_variable out;
	out.type = M_TYPE_FUNCTION;
	out.data.p = fptr;
	M->programcounter += function_size;

	mercury_pushstack(M, &out);
}

void M_BYTECODE_NTAB(mercury_state* const M_CPP_restrict M) { //New TABle
	mercury_variable out;
	
	const mercury_table* const ntab = mercury_newtable();
	if (ntab == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	
	out.type = M_TYPE_TABLE;
	out.data.p = (void*)ntab;
	mercury_pushstack(M, &out);
}

void M_BYTECODE_NARR(mercury_state* const M_CPP_restrict M) { //New ARRay
	mercury_variable out;

	const mercury_array* const narr = mercury_newarray();
	if (narr == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	
	out.type = M_TYPE_ARRAY;
	out.data.p = (void*)narr;
	mercury_pushstack(M, &out);
}

void M_BYTECODE_JMP(mercury_state* const M_CPP_restrict M) { //JuMP
	M->programcounter = *(mercury_uint*)(M->bytecode.instructions + M->programcounter);
}

void M_BYTECODE_JMPR(mercury_state* const M_CPP_restrict M) { //JuMP Relative
	M->programcounter += *(mercury_int*)(M->bytecode.instructions + M->programcounter);
}

void M_BYTECODE_JIF(mercury_state* const M_CPP_restrict M) { //Jump IF
	mercury_variable ck;
	mercury_popstack(M,&ck);
	if(mercury_checkbool(&ck)){
		M->programcounter = *(mercury_uint*)(M->bytecode.instructions + M->programcounter);
	}
	else {
		M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
	}
	mercury_release_var(&ck);
}

void M_BYTECODE_JNIF(mercury_state* const M_CPP_restrict M) { //Jump Not IF
	mercury_variable ck;
	mercury_popstack(M, &ck);
	if (!mercury_checkbool(&ck)) {
		M->programcounter = *(mercury_uint*)(M->bytecode.instructions + M->programcounter);
	}
	else {
		M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
	}
	mercury_release_var(&ck);
}

void M_BYTECODE_JRIF(mercury_state* const M_CPP_restrict M) { //Jump Relative IF
	mercury_variable ck;
	mercury_popstack(M, &ck);
	if (mercury_checkbool(&ck)) {
		M->programcounter += *(mercury_int*)(M->bytecode.instructions + M->programcounter);
	}
	else {
		M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
	}
	mercury_release_var(&ck);
}

void M_BYTECODE_JRNI(mercury_state* const M_CPP_restrict M) { //Jump Relative Not If
	mercury_variable ck;
	mercury_popstack(M, &ck);
	if (!mercury_checkbool(&ck)) {
		M->programcounter += *(mercury_int*)(M->bytecode.instructions + M->programcounter);
	}
	else {
		M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
	}
	mercury_release_var(&ck);
}

void M_BYTECODE_CALL(mercury_state* const M_CPP_restrict M) { //CALL function
	const mercury_int args_in = *(mercury_int*)(M->bytecode.instructions + M->programcounter);
	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
	
	const mercury_int args_out = *(mercury_int*)(M->bytecode.instructions + M->programcounter);
	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
	
	mercury_variable ck;
	mercury_popstack(M,&ck);
	switch (ck.type)
	{
	case M_TYPE_FUNCTION:
		{
		mercury_state* const FM= mercury_get_child_state(M);
		if (!FM) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		mercury_function previous = FM->bytecode;

		//FM->bytecode.instructions = func->instructions;
		//FM->bytecode.numberofinstructions = func->numberofinstructions;
		//memcpy(&FM->bytecode, ck->data.p, sizeof(mercury_function));
		FM->bytecode = *((mercury_function*)ck.data.p);

		mercury_variable tmp;
		for (mercury_int i = 0; i < args_in;i++) {
			mercury_popstack(M, &tmp);
			mercury_pushstack(FM, &tmp);
		}
		while (mercury_stepstate(FM)) {};
		for (mercury_int i = 0; i < args_out; i++) {
			mercury_pullstack(FM, &tmp);
			mercury_pushstack(M, &tmp);
		}
		//FM->bytecode.instructions = nullptr; //so the bytecode isn't freed
		FM->bytecode = previous;
		mercury_clearstate(FM);


		}
		
		break;
	case M_TYPE_CFUNC:
		((mercury_cfunc)(ck.data.p))(M,args_in,args_out);
		break;
	default:
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_FUNCTION, &ck, 0);
		mercury_release_var(&ck);
		return;
	}
	mercury_release_var(&ck);
}

void M_BYTECODE_END(mercury_state* const M_CPP_restrict M) { //end state execution
	M->programcounter = M->bytecode.numberofinstructions;
}

void M_BYTECODE_LEN(mercury_state* const M_CPP_restrict M) { //LENgth
	if (!M->sizeofstack) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}
	mercury_variable* var = M->stack + M->sizeofstack -1;
	switch (var->type) {
	case M_TYPE_ARRAY:
		
		var->type = M_TYPE_INT;
		var->data.i = mercury_array_len((mercury_array*)var->data.p)+1;
		break;
	case M_TYPE_STRING:
		
		var->type = M_TYPE_INT;
		var->data.i = ((mercury_string*)var->data.p)->size;
		break;
	case M_TYPE_TABLE:
		
		var->type = M_TYPE_INT;
		{
			mercury_int n = 0;
			mercury_table* t = ((mercury_table*)var->data.p);
			for (uint8_t i = 0; i < M_NUMBER_OF_TYPES; i++) {
				n += t->data[i].size;
			}
			var->data.i = n;
		}
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_INDEX_INVALID_TYPE, var->type);
		return;
	}
	return;
}

void M_BYTECODE_CNCT(mercury_state* const M_CPP_restrict M) { // CoNCaTenate
	if (M->sizeofstack<2) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}
	mercury_variable* st = M->stack + M->sizeofstack - 1;
	mercury_variable* ss = M->stack + M->sizeofstack - 2;
	mercury_string* s1;
	mercury_string* s2;
	if (st->type == M_TYPE_STRING) {
		s2 = (mercury_string*)st->data.p;
	}
	else {
		s2 = mercury_tostring(st);
		if (!s2) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
	}

	if (ss->type == M_TYPE_STRING) {
		s1 = (mercury_string*)ss->data.p;
	}
	else {
		s1 = mercury_tostring(ss);
		if (!s1) {
			if(st->type!=M_TYPE_STRING)mercury_mstring_delete(s2);
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
	}

	mercury_string* s3 = mercury_mstrings_concat(s1, s2);

	if (st->type != M_TYPE_STRING)mercury_mstring_delete(s2);
	if (ss->type != M_TYPE_STRING)mercury_mstring_delete(s1);

	if (!s3) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}

	mercury_release_var(st);
	mercury_release_var(ss);
	ss->data.p = s3;
	ss->type = M_TYPE_STRING;
	M->sizeofstack--;
	return;
}


void M_BYTECODE_CLS(mercury_state* const M_CPP_restrict M) { // CLear Stack
	//as simple as it gets, really.
	for (mercury_uint i = 0; i < M->sizeofstack;i++) {
		mercury_release_var(M->stack + i);
	}
	M->sizeofstack = 0;

}


void M_BYTECODE_GETL(mercury_state* const M_CPP_restrict M) { //GET Local
	//yeah, this is pretty simple.
	mercury_variable k;
	mercury_popstack(M, &k);
	mercury_subtable st = M->enviroment->data[k.type];
	mercury_int sz = st.size;
	while (sz) {
		sz--;
		if (mercury_vars_equal(st.keys+sz,&k)) {
			mercury_release_var(&k);
			mercury_pushstack(M, st.values + sz);
			return;
		}
	}
	mercury_variable v;
	v.data.i = 0;
	v.type = M_TYPE_NIL;
	mercury_pushstack(M,&v);
}

void M_BYTECODE_SETL(mercury_state* const M_CPP_restrict M) { //SET Local
	//ditto.
	mercury_variable key;
	mercury_popstack(M, &key);
	mercury_variable value;
	mercury_popstack(M, &value);


	mercury_subtable st = M->enviroment->data[key.type];
	mercury_int sz = st.size;
	while (sz) {
		sz--;
		if (mercury_vars_equal(st.keys + sz, &key)) {
			mercury_increment_variable_refrence_count(&value);
			mercury_decrement_variable_refrence_count(st.values + sz);
			mercury_release_var(&key);
			mercury_release_var(st.values + sz);
			st.values[sz] = value;
			return;
		}
	}

	void* nptr=realloc(st.keys, sizeof(mercury_variable) * (st.size + 1));
	if (!nptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	st.keys = (mercury_variable*)nptr;
	nptr=realloc(st.values, sizeof(mercury_variable) * (st.size + 1));
	if (!nptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	st.values = (mercury_variable*)nptr;

	mercury_increment_variable_refrence_count(&key);
	mercury_increment_variable_refrence_count(&value);
	st.keys[st.size] = key;
	st.values[st.size] = value;

	st.size++;
	M->enviroment->data[key.type] = st;
}

void M_BYTECODE_GETG(mercury_state* const M_CPP_restrict M) { //GET Global
	//ditto.
	mercury_variable k;
	mercury_popstack(M, &k);
	mercury_subtable st = M->masterstate->enviroment->data[k.type];
	mercury_int sz = st.size;
	while (sz) {
		sz--;
		if (mercury_vars_equal(st.keys + sz, &k)) {
			mercury_release_var(&k);
			mercury_pushstack(M, st.values + sz);
			return;
		}
	}
	mercury_variable v;
	v.data.i = 0;
	v.type = M_TYPE_NIL;
	mercury_pushstack(M, &v);
}

void M_BYTECODE_SETG(mercury_state* const M_CPP_restrict M) { //SET Global
		//ditto.
	mercury_variable key;
	mercury_popstack(M, &key);
	mercury_variable value;
	mercury_popstack(M, &value);


	mercury_subtable st = M->masterstate->enviroment->data[key.type];
	mercury_int sz = st.size;
	while (sz) {
		sz--;
		if (mercury_vars_equal(st.keys + sz, &key)) {
			mercury_increment_variable_refrence_count(&value);
			mercury_decrement_variable_refrence_count(st.values + sz);
			mercury_release_var(&key);
			mercury_release_var(st.values + sz);
			st.values[sz] = value;
			return;
		}
	}

	void* nptr = realloc(st.keys, sizeof(mercury_variable) * (st.size + 1));
	if (!nptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	st.keys = (mercury_variable*)nptr;
	nptr = realloc(st.values, sizeof(mercury_variable) * (st.size + 1));
	if (!nptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	st.values = (mercury_variable*)nptr;

	mercury_increment_variable_refrence_count(&key);
	mercury_increment_variable_refrence_count(&value);
	st.keys[st.size] = key;
	st.values[st.size] = value;

	st.size++;
	M->masterstate->enviroment->data[key.type] = st;
}

void M_BYTECODE_CPYT(mercury_state* const M_CPP_restrict M) { // CoPY Top (of stack)
	if (!M->sizeofstack)return; //nothing on stack, nothing to copy.

	mercury_variable v = *(M->stack + (M->sizeofstack - 1));
	mercury_pushstack(M, &v );
}


void M_BYTECODE_SWPT(mercury_state* const M_CPP_restrict M) { //SWaP Top. swaps the top and second top of stack.
	//so basically, 1,2 -> 2,1

	if (M->sizeofstack > 1) { //switch the top 2 elements
		const mercury_variable top = M->stack[M->sizeofstack - 1];
		M->stack[M->sizeofstack - 1] = M->stack[M->sizeofstack - 2];
		M->stack[M->sizeofstack - 2] = top;
	}
	else if (M->sizeofstack == 1) { //if there's 1 element, pushing nil does the same thing, basically.
		mercury_variable nn;
		nn.data.i = 0;
		nn.type = M_TYPE_NIL;
		mercury_pushstack(M, &nn);
	}
	//no stack? nothing to do.

}

void M_BYTECODE_CPYX(mercury_state* const M_CPP_restrict M) { // CoPY X elements (from top of stack)
	if (!M->sizeofstack)return; //nothing on stack, nothing to copy.

	const mercury_uint num = *(mercury_uint*)(M->bytecode.instructions + M->programcounter);
	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;

	mercury_uint start_stack_size = M->sizeofstack;
	for (mercury_uint i = 0; i < num; i++) {

		const mercury_int index = start_stack_size - num + i;
		if (index < 0) { //do not break, we need to gaurentee the stack size.
			M_BYTECODE_NNIL(M);
			continue;
		}

		mercury_pushstack(M, M->stack+index);
	}

}




void M_BYTECODE_UNM(mercury_state* const M_CPP_restrict M) { //UNary Minus
	if (!M->sizeofstack) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* var = M->stack + M->sizeofstack - 1;

	switch (var->type)
	{
	case M_TYPE_INT:
		var->data.i*=-1;
		break;
	case M_TYPE_FLOAT:
		var->data.f *= -1.0;
		break;
	default:
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, var, 1);
		return;
	}
	return;
}

void M_BYTECODE_INC(mercury_state* const M_CPP_restrict M) { //INCrement
	if (!M->sizeofstack) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* var = M->stack + M->sizeofstack - 1;

	switch (var->type)
	{
	case M_TYPE_INT:
		var->data.i++;
		break;
	case M_TYPE_FLOAT:
		var->data.f += 1.0;
		break;
	default:
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, var, 1);
		return;
	}
	return;
}

void M_BYTECODE_DEC(mercury_state* const M_CPP_restrict M) { //DECrement
	if (!M->sizeofstack) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}

	mercury_variable* var = M->stack+M->sizeofstack - 1;

	switch (var->type)
	{
	case M_TYPE_INT:
		var->data.i--;
		break;
	case M_TYPE_FLOAT:
		var->data.f -= 1.0;
		break;
	default:
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER_VARIABLEPROVIDED, var, 1);
		return;
	}
	return;
}

void M_BYTECODE_SCON(mercury_state* const M_CPP_restrict M) { //Set CONstant
	if (!M->sizeofstack) {
		mercury_raise_error(M, M_ERROR_STACK_CORRUPT);
		return;
	}
	const mercury_uint con_num = *(mercury_int*)(M->bytecode.instructions + M->programcounter);
	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;

	if (con_num >= M->num_constants) {
		void* const nptr=realloc(M->constants, sizeof(mercury_variable) * (con_num+1) );
		if (!nptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		M->num_constants = con_num+1;
		M->constants = (mercury_variable*)nptr;
	}
	
	M->sizeofstack--;
	mercury_increment_variable_refrence_count(M->stack+M->sizeofstack);
	M->constants[con_num] = M->stack[M->sizeofstack];
}

void M_BYTECODE_GCON(mercury_state* const M_CPP_restrict M) { //Get CONstant
	const mercury_uint con_num = *(mercury_int*)(M->bytecode.instructions + M->programcounter);
	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
	if (con_num >= M->num_constants) {
		M_BYTECODE_NNIL(M);
		return;
	}

	mercury_pushstack(M, M->constants + con_num);
}

void M_BYTECODE_SWXY(mercury_state* const M_CPP_restrict M) { //SWap stack X and Y
	mercury_uint X = *(mercury_int*)(M->bytecode.instructions + M->programcounter);
	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
	mercury_uint Y = *(mercury_int*)(M->bytecode.instructions + M->programcounter);
	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;

	if (X >= M->sizeofstack || Y >= M->sizeofstack) {
		mercury_raise_error(M, M_ERROR_INSTRUCTION_FAILIURE);
		return;
	}

	X = M->sizeofstack - X - 1;
	Y = M->sizeofstack - Y - 1;

	mercury_variable v_X = M->stack[X];
	M->stack[X] = M->stack[Y];
	M->stack[Y] = v_X;
}

mercury_instruction mercury_bytecode_list[] = {
	M_BYTECODE_NOP, //0
	//arithmetic
	M_BYTECODE_ADD, //1
	M_BYTECODE_SUB,
	M_BYTECODE_MUL,
	M_BYTECODE_DIV,
	M_BYTECODE_POW,
	M_BYTECODE_IDIV,
	M_BYTECODE_MOD, //7
	//bitwise
	M_BYTECODE_BAND, //8
	M_BYTECODE_BOR,
	M_BYTECODE_BXOR,
	M_BYTECODE_BNOT,
	M_BYTECODE_BSHL,
	M_BYTECODE_BSHR, //13
	//logical
	M_BYTECODE_LAND, //14
	M_BYTECODE_LOR,
	M_BYTECODE_LXOR,
	M_BYTECODE_LNOT, //17
	//comparison
	M_BYTECODE_EQL, //18
	M_BYTECODE_NEQ,
	M_BYTECODE_GRT,
	M_BYTECODE_LET,
	M_BYTECODE_GTE,
	M_BYTECODE_LTE, //23
	//variables
	M_BYTECODE_SENV, //24
	M_BYTECODE_GENV,
	M_BYTECODE_SET,
	M_BYTECODE_GET,
	M_BYTECODE_SREG,
	M_BYTECODE_GREG, //29
	//variable creation
	M_BYTECODE_NINT, //30
	M_BYTECODE_NFLO,
	M_BYTECODE_NTRU,
	M_BYTECODE_NFAL,
	M_BYTECODE_NNIL,
	M_BYTECODE_NSTR,
	M_BYTECODE_NFUN,
	M_BYTECODE_NTAB,
	M_BYTECODE_NARR, //38
	//jumps
	M_BYTECODE_JMP, //39
	M_BYTECODE_JMPR,
	M_BYTECODE_JIF,
	M_BYTECODE_JRIF,
	M_BYTECODE_JNIF,
	M_BYTECODE_JRNI, //44
	//program flow
	M_BYTECODE_CALL, //45
	M_BYTECODE_END,
	M_BYTECODE_LEN,
	M_BYTECODE_CNCT,
	M_BYTECODE_CLS, //49

	M_BYTECODE_GETL, //50
	M_BYTECODE_SETL,
	M_BYTECODE_GETG,
	M_BYTECODE_SETG, //53

	M_BYTECODE_CPYT, //54
	M_BYTECODE_SWPT, //55
	M_BYTECODE_CPYX, //56

	M_BYTECODE_UNM, //57
	M_BYTECODE_INC,
	M_BYTECODE_DEC, //59

	M_BYTECODE_SCON, //60
	M_BYTECODE_GCON, //61

	M_BYTECODE_SWXY, //62
};

