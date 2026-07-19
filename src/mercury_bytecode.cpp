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
	uint8_t argsfloat = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable v;


	mercury_popstack(M,&v);
	switch (v.type)
		{
		case M_TYPE_INT:
			i1 = v.data.i;
			break;
		case M_TYPE_FLOAT:
			f1 = v.data.f;
			argsfloat |= 1;
			break;
		default:
			mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type,2);
			mercury_free_var(&v);
			return;
		}
	mercury_free_var(&v);
		

	mercury_popstack(M,&v);
	switch (v.type)
		{
		case M_TYPE_INT:
			i2 = v.data.i;
			break;
		case M_TYPE_FLOAT:
			f2= v.data.f;
			argsfloat |= 2;
			break;
		default:
			mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
			mercury_free_var(&v);
			return;
		}
	mercury_free_var(&v);

	v.constant = false;
	switch (argsfloat) {
	case 0:
		v.type = M_TYPE_INT;
		v.data.i = i2 + i1;
		break;
	case 1:
		v.type = M_TYPE_FLOAT;
		v.data.f = (mercury_float)i2 + f1;
		break;
	case 2:
		v.type = M_TYPE_FLOAT;
		v.data.f = f2 + (mercury_float)i1;
		break;
	case 3:
		v.type = M_TYPE_FLOAT;
		v.data.f = f2 + f1;
		break;
	}
	mercury_pushstack(M, &v);
	return;
}

void M_BYTECODE_SUB(mercury_state* const M_CPP_restrict M) {
	uint8_t argsfloat = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable v;


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i1 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		f1 = v.data.f;
		argsfloat |= 1;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i2 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		f2 = v.data.f;
		argsfloat |= 2;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	switch (argsfloat) {
	case 0:
		v.type = M_TYPE_INT;
		v.data.i = i2 - i1;
		break;
	case 1:
		v.type = M_TYPE_FLOAT;
		v.data.f = (mercury_float)i2 - f1;
		break;
	case 2:
		v.type = M_TYPE_FLOAT;
		v.data.f = f2 - (mercury_float)i1;
		break;
	case 3:
		v.type = M_TYPE_FLOAT;
		v.data.f = f2 - f1;
		break;
	}
	mercury_pushstack(M, &v);
	return;
}

void M_BYTECODE_MUL(mercury_state* const M_CPP_restrict M) {
	uint8_t argsfloat = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable v;


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i1 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		f1 = v.data.f;
		argsfloat |= 1;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i2 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		f2 = v.data.f;
		argsfloat |= 2;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	switch (argsfloat) {
	case 0:
		v.type = M_TYPE_INT;
		v.data.i = i2 * i1;
		break;
	case 1:
		v.type = M_TYPE_FLOAT;
		v.data.f = (mercury_float)i2 * f1;
		break;
	case 2:
		v.type = M_TYPE_FLOAT;
		v.data.f = f2 * (mercury_float)i1;
		break;
	case 3:
		v.type = M_TYPE_FLOAT;
		v.data.f = f2 * f1;
		break;
	}
	mercury_pushstack(M, &v);
	return;
}

void M_BYTECODE_DIV(mercury_state* const M_CPP_restrict M) {
	mercury_float f1;
	mercury_float f2;

	mercury_variable v;


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		f1 = (mercury_float)v.data.i;
		break;
	case M_TYPE_FLOAT:
		f1 = v.data.f;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		f2 = (mercury_float)v.data.i;
		break;
	case M_TYPE_FLOAT:
		f2 = v.data.f;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	v.type = M_TYPE_FLOAT;
	v.data.f = f2 / f1;
	
	mercury_pushstack(M, &v);
	return;
}

void M_BYTECODE_POW(mercury_state* const M_CPP_restrict M) {
	mercury_float f1;
	mercury_float f2;

	mercury_variable v;


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		f1 = (mercury_float)v.data.i;
		break;
	case M_TYPE_FLOAT:
		f1 = v.data.f;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		f2 = (mercury_float)v.data.i;
		break;
	case M_TYPE_FLOAT:
		f2 = v.data.f;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	v.type = M_TYPE_FLOAT;
	v.data.f = pow(f2, f1);

	mercury_pushstack(M, &v);
	return;
}


void M_BYTECODE_IDIV(mercury_state* const M_CPP_restrict M) {
	mercury_int i1;
	mercury_int i2;

	mercury_variable v;


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i1 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		i1 = (mercury_int)v.data.f;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i2 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		i2 = (mercury_int)v.data.f;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	v.type = M_TYPE_INT;
	v.data.i = i2 / i1;

	mercury_pushstack(M, &v);
	return;
}

void M_BYTECODE_MOD(mercury_state* const M_CPP_restrict M) {
	uint8_t argsfloat = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable v;


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i1 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		f1 = v.data.f;
		argsfloat |= 1;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i2 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		f2 = v.data.f;
		argsfloat |= 2;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	switch (argsfloat) {
	case 0:
		v.type = M_TYPE_INT;
		v.data.i = i2 % i1;
		break;
	case 1:
		v.type = M_TYPE_FLOAT;
		v.data.f = fmod((mercury_float)i2 , f1);
		break;
	case 2:
		v.type = M_TYPE_FLOAT;
		v.data.f = fmod(f2 , (mercury_float)i1);
		break;
	case 3:
		v.type = M_TYPE_FLOAT;
		v.data.f = fmod(f2 , f1);
		break;
	}
	mercury_pushstack(M, &v);
	return;
}

void M_BYTECODE_BAND(mercury_state* const M_CPP_restrict M) {
	bool outfloat = 0;

	mercury_int i1;
	mercury_int i2;

	mercury_variable v;


	mercury_popstack(M,&v);
	switch (v.type)
	{
	case M_TYPE_INT:
	case M_TYPE_FLOAT:
		i1 = v.data.i;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_FLOAT:
		outfloat = true;
	case M_TYPE_INT:
		i2 = v.data.i;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	v.data.i = i2 & i1;
	v.type = outfloat ? M_TYPE_FLOAT : M_TYPE_INT;

	mercury_pushstack(M, &v);

	return;
}

void M_BYTECODE_BOR(mercury_state* const M_CPP_restrict M) {
	bool outfloat = 0;

	mercury_int i1;
	mercury_int i2;

	mercury_variable v;


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
	case M_TYPE_FLOAT:
		i1 = v.data.i;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_FLOAT:
		outfloat = true;
	case M_TYPE_INT:
		i2 = v.data.i;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	v.data.i = i2 | i1;
	v.type = outfloat ? M_TYPE_FLOAT : M_TYPE_INT;

	mercury_pushstack(M, &v);

	return;
}

void M_BYTECODE_BXOR(mercury_state* const M_CPP_restrict M) {
	bool outfloat = 0;

	mercury_int i1;
	mercury_int i2;

	mercury_variable v;


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
	case M_TYPE_FLOAT:
		i1 = v.data.i;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_FLOAT:
		outfloat = true;
	case M_TYPE_INT:
		i2 = v.data.i;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	v.data.i = i2 ^ i1;
	v.type = outfloat ? M_TYPE_FLOAT : M_TYPE_INT;

	mercury_pushstack(M, &v);

	return;
}

void M_BYTECODE_BNOT(mercury_state* const M_CPP_restrict M) {
	bool outfloat = 0;

	mercury_int i1;

	mercury_variable v;

	mercury_popstack(M,&v);
	switch (v.type)
	{
	case M_TYPE_FLOAT:
		outfloat = true;
	case M_TYPE_INT:
		i1 = v.data.i;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);
	

	v.constant = false;
	v.type = outfloat?M_TYPE_FLOAT: M_TYPE_INT;
#ifdef MERCURY_64BIT
	v.data.i = 0xffffffffffffffff ^ i1;
#else
	v.data.i = 0xffffffff ^ i1;
#endif

	mercury_pushstack(M, &v);

	return;
}

void M_BYTECODE_BSHL(mercury_state* const M_CPP_restrict M) {
	bool outfloat = 0;

	mercury_int i1;
	mercury_int i2;

	mercury_variable v;


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
	case M_TYPE_FLOAT:
		i1 = v.data.i;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_FLOAT:
		outfloat = true;
	case M_TYPE_INT:
		i2 = v.data.i;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	v.data.i = i2 << i1;
	v.type = outfloat ? M_TYPE_FLOAT : M_TYPE_INT;

	mercury_pushstack(M, &v);

	return;
}

void M_BYTECODE_BSHR(mercury_state* const M_CPP_restrict M) {
	bool outfloat = 0;

	mercury_int i1;
	mercury_int i2;

	mercury_variable v;


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
	case M_TYPE_FLOAT:
		i1 = v.data.i;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_FLOAT:
		outfloat = true;
	case M_TYPE_INT:
		i2 = v.data.i;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	v.data.i = i2 >> i1;
	v.type = outfloat ? M_TYPE_FLOAT : M_TYPE_INT;

	mercury_pushstack(M, &v);

	return;
}



void M_BYTECODE_LAND(mercury_state* const M_CPP_restrict M) {
	mercury_variable v1;
	mercury_variable v2;
	mercury_popstack(M,&v2);
	mercury_popstack(M,&v1);

	if (!mercury_checkbool(&v1)) {
		mercury_free_var(&v1);
		mercury_free_var(&v2);
		v2.constant = false;
		v2.data.i = 0;
		v2.type = M_TYPE_BOOL; //false.
		mercury_pushstack(M,&v2);
		return;
	}
	if (!mercury_checkbool(&v2)) {
		mercury_free_var(&v1);
		mercury_free_var(&v2);
		v2.constant = false;
		v2.data.i = 0;
		v2.type = M_TYPE_BOOL; //false.
		mercury_pushstack(M, &v2);
		return;
	}
	mercury_free_var(&v2);
	mercury_pushstack(M, &v1);
}

void M_BYTECODE_LOR(mercury_state* const M_CPP_restrict M) {
	mercury_variable v1;
	mercury_variable v2;
	mercury_popstack(M, &v2);
	mercury_popstack(M, &v1);

	if (mercury_checkbool(&v1)) {
		mercury_free_var(&v2);
		mercury_pushstack(M, &v1);
		return;
	}
	mercury_free_var(&v1);
	mercury_pushstack(M, &v2);
}

void M_BYTECODE_LXOR(mercury_state* const M_CPP_restrict M) {
	mercury_variable v1;
	mercury_variable v2;
	mercury_popstack(M, &v2);
	mercury_popstack(M, &v1);

	if (mercury_checkbool(&v1)) {
		if (mercury_checkbool(&v2)) {
			mercury_free_var(&v1);
			mercury_free_var(&v2);
			v2.constant = false;
			v2.data.i = 0;
			v2.type = M_TYPE_BOOL; //false.
			mercury_pushstack(M, &v2);
		}
		else {
			mercury_free_var(&v2);
			mercury_pushstack(M, &v1);
		}
	}
	else {
		if (mercury_checkbool(&v2)) {
			mercury_free_var(&v1);
			mercury_pushstack(M, &v2);
		}
		else {
			mercury_free_var(&v1);
			mercury_free_var(&v2);
			v2.constant = false;
			v2.data.i = 0;
			v2.type = M_TYPE_BOOL; //false.
			mercury_pushstack(M, &v2);
		}
	}
}

void M_BYTECODE_LNOT(mercury_state* const M_CPP_restrict M) {
	mercury_variable v1;
	mercury_popstack(M, &v1);

	if (mercury_checkbool(&v1)) {
		mercury_free_var(&v1);
		v1.constant = false;
		v1.data.i = 0;
		v1.type = M_TYPE_BOOL; //false.
		mercury_pushstack(M, &v1);
		return;
	}
	mercury_free_var(&v1);
	v1.constant = false;
	v1.data.i = 1;
	v1.type = M_TYPE_BOOL; //false.
	mercury_pushstack(M, &v1);
}


void M_BYTECODE_EQL(mercury_state* const M_CPP_restrict M) {
	mercury_variable v1;
	mercury_variable v2;
	mercury_popstack(M, &v2);
	mercury_popstack(M, &v1);

	if (mercury_vars_equal(&v1, &v2)) {
		mercury_free_var(&v1);
		mercury_free_var(&v2);

		v1.constant = false;
		v1.data.i = 1;
		v1.type = M_TYPE_BOOL; //true.
		mercury_pushstack(M, &v1);
		return;
	}
	else {
		mercury_free_var(&v1);
		mercury_free_var(&v2);

		v1.constant = false;
		v1.data.i = 0;
		v1.type = M_TYPE_BOOL; //false.
		mercury_pushstack(M, &v1);
		return;
	}
}

void M_BYTECODE_NEQ(mercury_state* const M_CPP_restrict M) {
	mercury_variable v1;
	mercury_variable v2;
	mercury_popstack(M, &v2);
	mercury_popstack(M, &v1);

	if (mercury_vars_equal(&v1, &v2)) {
		mercury_free_var(&v1);
		mercury_free_var(&v2);

		v1.constant = false;
		v1.data.i = 0;
		v1.type = M_TYPE_BOOL; //true.
		mercury_pushstack(M, &v1);
		return;
	}
	else {
		mercury_free_var(&v1);
		mercury_free_var(&v2);

		v1.constant = false;
		v1.data.i = 1;
		v1.type = M_TYPE_BOOL; //false.
		mercury_pushstack(M, &v1);
		return;
	}
}


void M_BYTECODE_GRT(mercury_state* const M_CPP_restrict M) {
	uint8_t argsfloat = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable v;


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i1 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		f1 = v.data.f;
		argsfloat |= 1;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i2 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		f2 = v.data.f;
		argsfloat |= 2;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	switch (argsfloat) {
	case 0:
		v.type = M_TYPE_INT;
		v.data.i = i2 > i1;
		break;
	case 1:
		v.type = M_TYPE_FLOAT;
		v.data.f = (mercury_float)i2 > f1;
		break;
	case 2:
		v.type = M_TYPE_FLOAT;
		v.data.f = f2 > (mercury_float)i1;
		break;
	case 3:
		v.type = M_TYPE_FLOAT;
		v.data.f = f2 > f1;
		break;
	}
	mercury_pushstack(M, &v);
	return;
}

void M_BYTECODE_LET(mercury_state* const M_CPP_restrict M) {
	uint8_t argsfloat = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable v;


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i1 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		f1 = v.data.f;
		argsfloat |= 1;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i2 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		f2 = v.data.f;
		argsfloat |= 2;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	switch (argsfloat) {
	case 0:
		v.type = M_TYPE_INT;
		v.data.i = i2 < i1;
		break;
	case 1:
		v.type = M_TYPE_FLOAT;
		v.data.f = (mercury_float)i2 < f1;
		break;
	case 2:
		v.type = M_TYPE_FLOAT;
		v.data.f = f2 < (mercury_float)i1;
		break;
	case 3:
		v.type = M_TYPE_FLOAT;
		v.data.f = f2 < f1;
		break;
	}
	mercury_pushstack(M, &v);
	return;
}

void M_BYTECODE_GTE(mercury_state* const M_CPP_restrict M) {
	uint8_t argsfloat = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable v;


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i1 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		f1 = v.data.f;
		argsfloat |= 1;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i2 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		f2 = v.data.f;
		argsfloat |= 2;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	switch (argsfloat) {
	case 0:
		v.type = M_TYPE_INT;
		v.data.i = i2 >= i1;
		break;
	case 1:
		v.type = M_TYPE_FLOAT;
		v.data.f = (mercury_float)i2 >= f1;
		break;
	case 2:
		v.type = M_TYPE_FLOAT;
		v.data.f = f2 >= (mercury_float)i1;
		break;
	case 3:
		v.type = M_TYPE_FLOAT;
		v.data.f = f2 >= f1;
		break;
	}
	mercury_pushstack(M, &v);
	return;
}

void M_BYTECODE_LTE(mercury_state* const M_CPP_restrict M) {
	uint8_t argsfloat = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable v;


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i1 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		f1 = v.data.f;
		argsfloat |= 1;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 2);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);


	mercury_popstack(M, &v);
	switch (v.type)
	{
	case M_TYPE_INT:
		i2 = v.data.i;
		break;
	case M_TYPE_FLOAT:
		f2 = v.data.f;
		argsfloat |= 2;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, v.type, 1);
		mercury_free_var(&v);
		return;
	}
	mercury_free_var(&v);

	v.constant = false;
	switch (argsfloat) {
	case 0:
		v.type = M_TYPE_INT;
		v.data.i = i2 <= i1;
		break;
	case 1:
		v.type = M_TYPE_FLOAT;
		v.data.f = (mercury_float)i2 <= f1;
		break;
	case 2:
		v.type = M_TYPE_FLOAT;
		v.data.f = f2 <= (mercury_float)i1;
		break;
	case 3:
		v.type = M_TYPE_FLOAT;
		v.data.f = f2 <= f1;
		break;
	}
	mercury_pushstack(M, &v);
	return;
}

void M_BYTECODE_SENV(mercury_state* const M_CPP_restrict M) {
	mercury_variable value;
	mercury_popstack(M,&value);
	mercury_variable key;
	mercury_popstack(M, &key);

	mercury_state* check_state = M;
	mercury_int pos;
	while (check_state) {
		
		pos = mercury_tablehaskey(check_state->enviroment, &key);
		if (pos!=-1) {
			mercury_subtable* st=check_state->enviroment->data[key.type];
			mercury_free_var(st->values+pos);
			st->values[pos] = value;
			mercury_free_var(&key);
			//mercury_setkey(check_state->enviroment, &key, &value);
			return;
		}
		
		check_state = check_state->parentstate;
	}
	mercury_setkey(M->enviroment, &key, &value);
}

void M_BYTECODE_GENV(mercury_state* const M_CPP_restrict M) {
	mercury_variable value;
	mercury_variable key;
	mercury_popstack(M, &key);

	mercury_state* check_state = M;
	while (check_state) {
		/*
		if (mercury_tablehaskey(check_state->enviroment, &key)) {
			mercury_getkey(check_state->enviroment, &key, &value);
			mercury_pushstack_unrefed(M, &value);
			return;
		}
		*/
		if (mercury_getkey(check_state->enviroment, &key,&value)) {
			mercury_pushstack_unrefed(M, &value);
			return;
		}
		check_state = check_state->parentstate;
	}
	value.constant = false;
	value.data.i = 0;
	value.type = M_TYPE_NIL;
	mercury_pushstack(M, &value);
}


void M_BYTECODE_SET(mercury_state* const M_CPP_restrict M) {
	mercury_variable value;
	mercury_popstack(M, &value);
	mercury_variable key;
	mercury_popstack(M, &key);
	mercury_variable table;
	mercury_popstack(M, &table);

	switch (table.type) {
	case M_TYPE_TABLE:
		mercury_setkey((mercury_table*)table.data.p, &key, &value);
		break;
	case M_TYPE_ARRAY:
		if (key.type != M_TYPE_INT) {
			mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, table.type, M_TYPE_INT, 2);
			mercury_free_var(&value);
			mercury_free_var(&key);
			mercury_free_var(&table);
			return;
		}
		mercury_setarray((mercury_array*)table.data.p, &value, key.data.i);
		mercury_free_var(&key);
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_STORAGETYPE, table.type, 1);
		mercury_free_var(&value);
		mercury_free_var(&key);
		mercury_free_var(&table);
		return;
	}

	mercury_free_var(&table);	
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
			mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, table.type, M_TYPE_INT, 2);
			mercury_free_var(&key);
			mercury_free_var(&table);
			return;
		}
		mercury_getarray((mercury_array*)table.data.p, key.data.i,&value);
		break;
	case M_TYPE_STRING:
		if (key.type != M_TYPE_INT) {
			mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, table.type, M_TYPE_INT,2);
			mercury_free_var(&key);
			mercury_free_var(&table);
			return;
		}
		value.constant = false;
		value.type = M_TYPE_STRING;
		value.data.p = mercury_mstring_substring((mercury_stringliteral*)table.data.p, key.data.i, key.data.i);
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_STORAGETYPE, table.type, 1);
		mercury_free_var(&key);
		mercury_free_var(&table);
		return;
	}

	mercury_free_var(&key);
	mercury_free_var(&table);
	mercury_pushstack_unrefed(M, &value);
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
		mercury_popstack(M,&v);
		mercury_free_var(&v);
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
		out.constant = false;
		out.type = M_TYPE_NIL;
		out.data.i = 0;
		mercury_pushstack(M, &out);
	}

}

void M_BYTECODE_NINT(mercury_state* const M_CPP_restrict M) { //New INTeger
	void* const offset = M->bytecode.instructions + M->programcounter;

	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;

	mercury_variable out;
	out.constant = false;
	out.type = M_TYPE_INT;
	out.data.i= *(mercury_int*)offset;
	mercury_pushstack(M, &out);
}

void M_BYTECODE_NFLO(mercury_state* const M_CPP_restrict M) { //New FLOat
	void* const offset = M->bytecode.instructions + M->programcounter;

	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;

	mercury_variable out;
	out.constant = false;
	out.type = M_TYPE_FLOAT;
	out.data.f = *(mercury_float*)offset;
	mercury_pushstack(M, &out);
}

void M_BYTECODE_NTRU(mercury_state* const M_CPP_restrict M) { //New TRUe
	mercury_variable out;
	out.constant = false;
	out.type = M_TYPE_BOOL;
	out.data.i = 1;
	mercury_pushstack(M, &out);
}

void M_BYTECODE_NFAL(mercury_state* const M_CPP_restrict M) { //New FALse
	mercury_variable out;
	out.constant = false;
	out.type = M_TYPE_BOOL;
	out.data.i = 0;
	mercury_pushstack(M, &out);
}

void M_BYTECODE_NNIL(mercury_state* const M_CPP_restrict M) { //New NIL
	mercury_variable out;
	out.constant = false;
	out.type = M_TYPE_NIL;
	out.data.i = 0;
	mercury_pushstack(M, &out);
}

void M_BYTECODE_NSTR(mercury_state* const M_CPP_restrict M) { //New STRing
	const mercury_uint string_size = *(mercury_uint*)(M->bytecode.instructions + M->programcounter);
	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;

	mercury_variable out;
	out.constant = false;

	mercury_stringliteral* const so = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!so) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	if (string_size) {
		so->ptr = (char*)(M->bytecode.instructions + M->programcounter);
		so->constant = true;
	}
	else so->ptr = nullptr;

	so->size = string_size;

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

	fptr->refrences = 1;
	fptr->numberofinstructions = function_size;
	fptr->instructions = (mercury_opcode*)malloc(function_size * sizeof(mercury_opcode));
	
	if (fptr->instructions == nullptr) {
		free(fptr);
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}

	memcpy(fptr->instructions, M->bytecode.instructions + M->programcounter, function_size * sizeof(mercury_opcode));

	mercury_variable out;
	out.constant=false;
	out.type = M_TYPE_FUNCTION;
	out.data.p = fptr;
	M->programcounter += function_size;

	mercury_pushstack_unrefed(M, &out);
}

void M_BYTECODE_NTAB(mercury_state* const M_CPP_restrict M) { //New TABle
	mercury_variable out;
	
	const mercury_table* const ntab = mercury_newtable();
	if (ntab == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	out.constant = false;
	out.type = M_TYPE_TABLE;
	out.data.p = (void*)ntab;
	mercury_pushstack_unrefed(M, &out);
}

void M_BYTECODE_NARR(mercury_state* const M_CPP_restrict M) { //New ARRay
	mercury_variable out;

	const mercury_array* const narr = mercury_newarray();
	if (narr == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	out.constant = false;
	out.type = M_TYPE_ARRAY;
	out.data.p = (void*)narr;
	mercury_pushstack_unrefed(M, &out);
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
	mercury_free_var(&ck);
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
	mercury_free_var(&ck);
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
	mercury_free_var(&ck);
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
	mercury_free_var(&ck);
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
			mercury_pushstack_unrefed(FM, &tmp);
		}
		while (mercury_stepstate(FM)) {};
		for (mercury_int i = 0; i < args_out; i++) {
			mercury_pullstack(FM, &tmp);
			mercury_pushstack_unrefed(M, &tmp);
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
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_FUNCTION, ck.type, 0);
		mercury_free_var(&ck);
		return;
	}
	mercury_free_var(&ck);
}

void M_BYTECODE_END(mercury_state* const M_CPP_restrict M) { //end state execution
	M->programcounter = M->bytecode.numberofinstructions;
}

void M_BYTECODE_LEN(mercury_state* const M_CPP_restrict M) { //LENgth
	mercury_variable var;
	mercury_popstack(M,&var);
	mercury_variable out; 
	switch (var.type) {
	case M_TYPE_ARRAY:
		out.constant = false;
		out.type = M_TYPE_INT;
		out.data.i = mercury_array_len((mercury_array*)var.data.p)+1;
		break;
	case M_TYPE_STRING:
		out.constant = false;
		out.type = M_TYPE_INT;
		out.data.i = ((mercury_stringliteral*)var.data.p)->size;
		break;
	case M_TYPE_TABLE:
		out.constant = false;
		out.type = M_TYPE_INT;
		out.data.i = 0;
		{
			mercury_table* t = ((mercury_table*)var.data.p);
			for (uint8_t i = 0; i < M_NUMBER_OF_TYPES; i++) {
				out.data.i += t->data[i]->size;
			}
		}
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_INDEX_INVALID_TYPE, var.type);
		mercury_free_var(&var);
		return;
	}
	mercury_free_var(&var);
	mercury_pushstack(M,&out);
}

void M_BYTECODE_CNCT(mercury_state* const M_CPP_restrict M) { // CoNCaTenate
	mercury_variable v2;
	mercury_popstack(M, &v2);
	mercury_variable v1;
	mercury_popstack(M, &v1);

	mercury_stringliteral* s1 = mercury_tostring(&v1);
	mercury_stringliteral* s2 = mercury_tostring(&v2);

	mercury_free_var(&v1);
	mercury_free_var(&v2);

	mercury_variable v;
	v.type = M_TYPE_STRING;
	v.constant = false;
	v.data.p=mercury_mstrings_concat(s1, s2);
	mercury_pushstack(M, &v);
}


void M_BYTECODE_CLS(mercury_state* const M_CPP_restrict M) { // CLear Stack
	//as simple as it gets, really.
	for (mercury_uint i = 0; i < M->sizeofstack;i++) {
		mercury_free_var(M->stack + i);
	}
	M->sizeofstack = 0;

}


void M_BYTECODE_GETL(mercury_state* const M_CPP_restrict M) { //GET Local
	//yeah, this is pretty simple.
	mercury_variable k;
	mercury_popstack(M, &k);
	mercury_variable v;
	mercury_getkey(M->enviroment, &k, &v);
	mercury_pushstack_unrefed(M,&v);
}

void M_BYTECODE_SETL(mercury_state* const M_CPP_restrict M) { //SET Local
	//ditto.
	mercury_variable value;
	mercury_popstack(M, &value);
	mercury_variable key;
	mercury_popstack(M,&key);
	mercury_setkey(M->enviroment, &key, &value);
}

void M_BYTECODE_GETG(mercury_state* const M_CPP_restrict M) { //GET Global
	//ditto.
	mercury_variable k;
	mercury_popstack(M, &k);
	mercury_variable v;
	mercury_getkey(M->masterstate->enviroment, &k, &v);
	mercury_pushstack_unrefed(M, &v);
}

void M_BYTECODE_SETG(mercury_state* const M_CPP_restrict M) { //SET Global
	//ditto.
	mercury_variable value;
	mercury_popstack(M, &value);
	mercury_variable key;
	mercury_popstack(M, &key);
	mercury_setkey(M->masterstate->enviroment, &key, &value);
}

void M_BYTECODE_CPYT(mercury_state* const M_CPP_restrict M) { // CoPY Top (of stack)
	if (!M->sizeofstack)return; //nothing on stack, nothing to copy.

	mercury_variable val= M->stack[M->sizeofstack - 1];
	mercury_variable out;
	
	mercury_clonevariable(&val,&out);

	mercury_pushstack_unrefed(M, &out);
}


void M_BYTECODE_SWPT(mercury_state* const M_CPP_restrict M) { //SWaP Top. swaps the top and second top of stack.
	//so basically, 1,2 -> 2,1
	if (!M->sizeofstack) { //no stack? nothing to do.
		return;
	}
	else if (M->sizeofstack == 1) { //if there's 1 element, pushing nil does the same thing, basically.
		mercury_variable nn;
		nn.constant = false;
		nn.data.i = 0;
		nn.type = M_TYPE_NIL;
		mercury_pushstack(M, &nn);
	}
	else { //otherwise, switch the top 2 elements
		const mercury_variable top= M->stack[M->sizeofstack - 1];
		M->stack[M->sizeofstack - 1] = M->stack[M->sizeofstack - 2];
		M->stack[M->sizeofstack - 2] = top;
	}
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

		const mercury_variable val = M->stack[index];
		mercury_variable out;
		mercury_clonevariable(&val, &out);

		mercury_pushstack(M, &out);
	}

}




void M_BYTECODE_UNM(mercury_state* const M_CPP_restrict M) { //UNary Minus
	mercury_variable var;
	if (M->sizeofstack) {
		var = M->stack[M->sizeofstack - 1];
	}
	else {
		var.type = M_TYPE_NIL;
	}

	switch (var.type)
	{
	case M_TYPE_INT:
		var.data.i*=-1;
		break;
	case M_TYPE_FLOAT:
		var.data.f *= -1.0;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, var.type,1);
		return;
	}
	M->stack[M->sizeofstack - 1] = var;
	return;
}

void M_BYTECODE_INC(mercury_state* const M_CPP_restrict M) { //INCrement
	mercury_variable var;
	if (M->sizeofstack) {
		var = M->stack[M->sizeofstack - 1];
	}
	else {
		var.type = M_TYPE_NIL;
	}

	switch (var.type)
	{
	case M_TYPE_INT:
		var.data.i++;
		break;
	case M_TYPE_FLOAT:
		var.data.f += 1.0;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, var.type, 1);
		return;
	}
	M->stack[M->sizeofstack - 1] = var;
	return;
}

void M_BYTECODE_DEC(mercury_state* const M_CPP_restrict M) { //DECrement
	mercury_variable var;
	if (M->sizeofstack) {
		var = M->stack[M->sizeofstack - 1];
	}
	else {
		var.type = M_TYPE_NIL;
	}

	switch (var.type)
	{
	case M_TYPE_INT:
		var.data.i--;
		break;
	case M_TYPE_FLOAT:
		var.data.f -= 1.0;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, var.type, 1);
		return;
	}
	M->stack[M->sizeofstack - 1] = var;
	return;
}

void M_BYTECODE_SCON(mercury_state* const M_CPP_restrict M) { //Set CONstant
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
	
	mercury_variable v;
	mercury_pullstack(M,&v);
	//printf("added a new constant (num %i) at %p. type: %i data:%i\n",con_num ,v,v->type,v->data.i );
	v.constant = 1;
	M->constants[con_num] = v;
}

void M_BYTECODE_GCON(mercury_state* const M_CPP_restrict M) { //Get CONstant
	const mercury_uint con_num = *(mercury_int*)(M->bytecode.instructions + M->programcounter);
	M->programcounter += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
	if (con_num >= M->num_constants) {
		M_BYTECODE_NNIL(M);
		return;
	}
	//printf("got a new constant (num %i) at %p. type:%i data:%i\n", con_num, M->constants[con_num], M->constants[con_num]->type, M->constants[con_num]->data.i);
	mercury_pushstack(M, M->constants+con_num);
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
};

