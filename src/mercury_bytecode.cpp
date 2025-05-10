#include "mercury_bytecode.h"
#include "mercury.h"
#include "malloc.h"
#include "mercury_error.h"
#include "math.h"
#include "string.h"

#include "stdio.h"

void M_BYTECODE_NOP(mercury_state* M, uint16_t flags) {
	return;
}


void M_BYTECODE_ADD(mercury_state* M, uint16_t flags) {
	uint8_t floatcount = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable* outv = mercury_assign_var(M);// (mercury_variable*)malloc(sizeof(mercury_variable));
	if (outv == nullptr) { 
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter-1) );
		return; 
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset= M->instructions+M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			f1 = *(mercury_float*)offset;
			floatcount++;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
		#ifdef MERCURY_64BIT
			M->programcounter += 2;
		#else
			M->programcounter += 1;
		#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) { 
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			f1 = var->data.f;
			floatcount++;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1),  (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			(floatcount ? f2 : f1) = *(mercury_float*)offset;
			floatcount++;
		}
		else {
			(floatcount ? i1 : i2) = *(mercury_int*)offset;
		}
		#ifdef MERCURY_64BIT
			M->programcounter += 2;
		#else
			M->programcounter += 1;
		#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) { 
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return; 
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			(floatcount ? i1 : i2) = var->data.i;
			break;
		case M_TYPE_FLOAT:
			(floatcount ? f2 : f1) = var->data.f;
			floatcount++;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(floatcount ? M_TYPE_FLOAT : M_TYPE_INT) , (void*)var->type );
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (floatcount) {
		outv->type = M_TYPE_FLOAT;
		outv->data.f = f1 + (floatcount == 2 ? f2 : (mercury_float)i1);
	}
	else {
		outv->type = M_TYPE_INT;
		outv->data.i = i1+i2;
	}
	mercury_pushstack(M, outv);

	return;
}

void M_BYTECODE_SUB(mercury_state* M, uint16_t flags) {
	uint8_t floatcount = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable* outv = mercury_assign_var(M);// (mercury_variable*)malloc(sizeof(mercury_variable));
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			f1 = *(mercury_float*)offset;
			floatcount++;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			f1 = var->data.f;
			floatcount++;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			(floatcount ? f2 : f1) = *(mercury_float*)offset;
			floatcount++;
		}
		else {
			(floatcount ? i1 : i2) = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			(floatcount ? i1 : i2) = var->data.i;
			break;
		case M_TYPE_FLOAT:
			(floatcount ? f2 : f1) = var->data.f;
			floatcount++;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(floatcount ? M_TYPE_FLOAT : M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (floatcount) {
		outv->type = M_TYPE_FLOAT;
		outv->data.f = (floatcount == 2 ? f2 : (mercury_float)i1) - f1;
	}
	else {
		outv->type = M_TYPE_INT;
		outv->data.i = i2-i1;
	}
	mercury_pushstack(M, outv);

	return;
}

void M_BYTECODE_MUL(mercury_state* M, uint16_t flags) {
	uint8_t floatcount = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable* outv = mercury_assign_var(M);// (mercury_variable*)malloc(sizeof(mercury_variable));
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			f1 = *(mercury_float*)offset;
			floatcount++;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			f1 = var->data.f;
			floatcount++;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			(floatcount ? f2 : f1) = *(mercury_float*)offset;
			floatcount++;
		}
		else {
			(floatcount ? i1 : i2) = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			(floatcount ? i1 : i2) = var->data.i;
			break;
		case M_TYPE_FLOAT:
			(floatcount ? f2 : f1) = var->data.f;
			floatcount++;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(floatcount ? M_TYPE_FLOAT : M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (floatcount) {
		outv->type = M_TYPE_FLOAT;
		outv->data.f = (floatcount == 2 ? f2 : (mercury_float)i1) * f1;
	}
	else {
		outv->type = M_TYPE_INT;
		outv->data.i = i2 * i1;
	}
	mercury_pushstack(M, outv);

	return;
}

void M_BYTECODE_DIV(mercury_state* M, uint16_t flags) {
	uint8_t floatcount = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable* outv = mercury_assign_var(M);// (mercury_variable*)malloc(sizeof(mercury_variable));
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			f1 = *(mercury_float*)offset;
			floatcount++;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			f1 = var->data.f;
			floatcount++;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			(floatcount ? f2 : f1) = *(mercury_float*)offset;
			floatcount++;
		}
		else {
			(floatcount ? i1 : i2) = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			(floatcount ? i1 : i2) = var->data.i;
			break;
		case M_TYPE_FLOAT:
			(floatcount ? f2 : f1) = var->data.f;
			floatcount++;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(floatcount ? M_TYPE_FLOAT : M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (floatcount) {
		outv->type = M_TYPE_FLOAT;
		outv->data.f = (floatcount == 2 ? f2 : (mercury_float)i1) / f1;
	}
	else {
		outv->type = M_TYPE_FLOAT;
		outv->data.f = (mercury_float)i2 / (mercury_float)i1;
	}
	mercury_pushstack(M, outv);

	return;
}

void M_BYTECODE_POW(mercury_state* M, uint16_t flags) {
	uint8_t floatcount = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable* outv = mercury_assign_var(M);// (mercury_variable*)malloc(sizeof(mercury_variable));
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			f1 = *(mercury_float*)offset;
			floatcount++;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			f1 = var->data.f;
			floatcount++;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_FLOAT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			(floatcount ? f2 : f1) = *(mercury_float*)offset;
			floatcount++;
		}
		else {
			(floatcount ? i1 : i2) = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			(floatcount ? i1 : i2) = var->data.i;
			break;
		case M_TYPE_FLOAT:
			(floatcount ? f2 : f1) = var->data.f;
			floatcount++;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(floatcount ? M_TYPE_FLOAT : M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (floatcount) {
		outv->type = M_TYPE_FLOAT;
		outv->data.f = pow( (floatcount == 2 ? f2 : (mercury_float)i1) , f1 );
	}
	else {
		outv->type = M_TYPE_FLOAT;
		outv->data.f = pow(i2,i1);
	}
	mercury_pushstack(M, outv);

	return;
}


void M_BYTECODE_IDIV(mercury_state* M, uint16_t flags) {
	uint8_t floatcount = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable* outv = mercury_assign_var(M);// (mercury_variable*)malloc(sizeof(mercury_variable));
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			f1 = *(mercury_float*)offset;
			floatcount++;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			f1 = var->data.f;
			floatcount++;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			(floatcount ? f2 : f1) = *(mercury_float*)offset;
			floatcount++;
		}
		else {
			(floatcount ? i1 : i2) = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			(floatcount ? i1 : i2) = var->data.i;
			break;
		case M_TYPE_FLOAT:
			(floatcount ? f2 : f1) = var->data.f;
			floatcount++;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(floatcount ? M_TYPE_FLOAT : M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (floatcount) {

		if ((mercury_int)f1 == 0) {
			mercury_raise_error(M, M_ERROR_DIV_ZERO, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}

		outv->type = M_TYPE_INT;
		outv->data.i = (floatcount == 2 ? (mercury_int)f2 : i1) / (mercury_int)f1;
	}
	else {
		if ((mercury_int)i1 == 0) {
			mercury_raise_error(M, M_ERROR_DIV_ZERO, (void*)(M->programcounter - 1) );
			mercury_unassign_var(M, outv);
			return;
		}

		outv->type = M_TYPE_INT;
		outv->data.i = i2 / i1;
	}
	mercury_pushstack(M, outv);

	return;
}

void M_BYTECODE_MOD(mercury_state* M, uint16_t flags) {
	uint8_t floatcount = 0;

	mercury_float f1;
	mercury_float f2;
	mercury_int i1;
	mercury_int i2;

	mercury_variable* outv = mercury_assign_var(M);// (mercury_variable*)malloc(sizeof(mercury_variable));
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			f1 = *(mercury_float*)offset;
			floatcount++;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			f1 = var->data.f;
			floatcount++;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			(floatcount ? f2 : f1) = *(mercury_float*)offset;
			floatcount++;
		}
		else {
			(floatcount ? i1 : i2) = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			(floatcount ? i1 : i2) = var->data.i;
			break;
		case M_TYPE_FLOAT:
			(floatcount ? f2 : f1) = var->data.f;
			floatcount++;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(floatcount ? M_TYPE_FLOAT : M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (floatcount) {
		outv->type = M_TYPE_FLOAT;
		outv->data.f = fmod(floatcount == 2 ? f2 : (mercury_float)i1 , f1);
	}
	else {
		outv->type = M_TYPE_INT;
		outv->data.i = i2 % i1;
	}
	mercury_pushstack(M, outv);

	return;
}

void M_BYTECODE_BAND(mercury_state* M, uint16_t flags) {
	bool outfloat = 0;

	mercury_int i1;
	mercury_int i2;

	mercury_variable* outv = mercury_assign_var(M);// (mercury_variable*)malloc(sizeof(mercury_variable));
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			i1 = *(mercury_int*)offset;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			i1 = var->data.i;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			i2 = *(mercury_int*)offset;
			outfloat = true;
		}
		else {
			i2 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i2 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			i2 = var->data.i;
			outfloat = true;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (outfloat) {
		outv->type = M_TYPE_FLOAT;
		outv->data.i = i2 & i1;
	}
	else {
		outv->type = M_TYPE_INT;
		outv->data.i = i2 & i1;
	}
	mercury_pushstack(M, outv);

	return;
}

void M_BYTECODE_BOR(mercury_state* M, uint16_t flags) {
	bool outfloat = 0;

	mercury_int i1;
	mercury_int i2;

	mercury_variable* outv = mercury_assign_var(M);// (mercury_variable*)malloc(sizeof(mercury_variable));
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			i1 = *(mercury_int*)offset;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			i1 = var->data.i;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			i2 = *(mercury_int*)offset;
			outfloat = true;
		}
		else {
			i2 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i2 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			i2 = var->data.i;
			outfloat = true;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (outfloat) {
		outv->type = M_TYPE_FLOAT;
		outv->data.i = i2 | i1;
	}
	else {
		outv->type = M_TYPE_INT;
		outv->data.i = i2 | i1;
	}
	mercury_pushstack(M, outv);

	return;
}

void M_BYTECODE_BXOR(mercury_state* M, uint16_t flags) {
	bool outfloat = 0;

	mercury_int i1;
	mercury_int i2;

	mercury_variable* outv = mercury_assign_var(M);// (mercury_variable*)malloc(sizeof(mercury_variable));
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			i1 = *(mercury_int*)offset;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			i1 = var->data.i;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			i2 = *(mercury_int*)offset;
			outfloat = true;
		}
		else {
			i2 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i2 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			i2 = var->data.i;
			outfloat = true;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (outfloat) {
		outv->type = M_TYPE_FLOAT;
		outv->data.i = i2 ^ i1;
	}
	else {
		outv->type = M_TYPE_INT;
		outv->data.i = i2 ^ i1;
	}
	mercury_pushstack(M, outv);

	return;
}

void M_BYTECODE_BNOT(mercury_state* M, uint16_t flags) {
	bool outfloat = 0;

	mercury_int i1;

	mercury_variable* outv = mercury_assign_var(M);// (mercury_variable*)malloc(sizeof(mercury_variable));
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			i1 = *(mercury_int*)offset;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			i1 = var->data.i;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}


	if (outfloat) {
		outv->type = M_TYPE_FLOAT;
#ifdef MERCURY_64BIT
		outv->data.i = 0xffffffffffffffff ^ i1;
#else
		outv->data.i = 0xffffffff ^ i1;
#endif
		outv->data.i = 0xf ^ i1;
	}
	else {
		outv->type = M_TYPE_INT;
#ifdef MERCURY_64BIT
		outv->data.i = 0xffffffffffffffff ^ i1;
#else
		outv->data.i = 0xffffffff ^ i1;
#endif
		
	}
	mercury_pushstack(M, outv);

	return;
}

void M_BYTECODE_BSHL(mercury_state* M, uint16_t flags) {
	bool outfloat = 0;

	mercury_int i1;
	mercury_int i2;

	mercury_variable* outv = mercury_assign_var(M);// (mercury_variable*)malloc(sizeof(mercury_variable));
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			i1 = *(mercury_int*)offset;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			i1 = var->data.i;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			i2 = *(mercury_int*)offset;
			outfloat = true;
		}
		else {
			i2 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i2 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			i2 = var->data.i;
			outfloat = true;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (outfloat) {
		outv->type = M_TYPE_FLOAT;
		outv->data.i = i2 << i1;
	}
	else {
		outv->type = M_TYPE_INT;
		outv->data.i = i2 << i1;
	}
	mercury_pushstack(M, outv);

	return;
}

void M_BYTECODE_BSHR(mercury_state* M, uint16_t flags) {
	bool outfloat = 0;

	mercury_int i1;
	mercury_int i2;

	mercury_variable* outv = mercury_assign_var(M);// (mercury_variable*)malloc(sizeof(mercury_variable));
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			i1 = *(mercury_int*)offset;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			i1 = var->data.i;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			i2 = *(mercury_int*)offset;
			outfloat = true;
		}
		else {
			i2 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i2 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			i2 = var->data.i;
			outfloat = true;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (outfloat) {
		outv->type = M_TYPE_FLOAT;
		outv->data.i = i2 >> i1;
	}
	else {
		outv->type = M_TYPE_INT;
		outv->data.i = i2 >> i1;
	}
	mercury_pushstack(M, outv);

	return;
}



void M_BYTECODE_LAND(mercury_state* M, uint16_t flags) {
	mercury_variable* var2 = mercury_popstack(M);
	mercury_variable* var1 = mercury_popstack(M);

	if (!mercury_checkbool(var1)) {
		mercury_unassign_var(M, var1);
		mercury_unassign_var(M, var2);
		mercury_variable* out = mercury_assign_var(M);
		out->data.i = 0;
		out->type = M_TYPE_BOOL; //false.
		mercury_pushstack(M,out);
		return;
	}
	if (!mercury_checkbool(var2)) {
		mercury_unassign_var(M, var1);
		mercury_unassign_var(M, var2);
		mercury_variable* out = mercury_assign_var(M);
		out->data.i = 0;
		out->type = M_TYPE_BOOL; //false.
		mercury_pushstack(M, out);
		return;
	}
	mercury_unassign_var(M, var2);
	mercury_pushstack(M, var1);
}

void M_BYTECODE_LOR(mercury_state* M, uint16_t flags) {
	mercury_variable* var2 = mercury_popstack(M);
	mercury_variable* var1 = mercury_popstack(M);

	if (mercury_checkbool(var1)) {
		mercury_unassign_var(M, var2);
		mercury_pushstack(M, var1);
		return;
	}
	mercury_unassign_var(M, var1);
	mercury_pushstack(M, var2);
}

void M_BYTECODE_LXOR(mercury_state* M, uint16_t flags) {
	mercury_variable* var2 = mercury_popstack(M);
	mercury_variable* var1 = mercury_popstack(M);

	if (mercury_checkbool(var1)) {
		if (mercury_checkbool(var2)) {
			mercury_unassign_var(M, var1);
			mercury_unassign_var(M, var2);
			mercury_variable* out = mercury_assign_var(M);
			out->data.i = 0;
			out->type = M_TYPE_BOOL; //false.
			mercury_pushstack(M, out);
		}
		else {
			mercury_unassign_var(M, var2);
			mercury_pushstack(M, var1);
		}
	}
	else {
		if (mercury_checkbool(var2)) {
			mercury_unassign_var(M, var1);
			mercury_pushstack(M, var2);
		}
		else {
			mercury_unassign_var(M, var1);
			mercury_unassign_var(M, var2);
			mercury_variable* out = mercury_assign_var(M);
			out->data.i = 0;
			out->type = M_TYPE_BOOL; //false.
			mercury_pushstack(M, out);
		}
	}
}

void M_BYTECODE_LNOT(mercury_state* M, uint16_t flags) {
	mercury_variable* var1 = mercury_popstack(M);

	if (mercury_checkbool(var1)) {
		mercury_unassign_var(M, var1);
		mercury_variable* out = mercury_assign_var(M);
		out->data.i = 0;
		out->type = M_TYPE_BOOL; //false.
		mercury_pushstack(M, out);
		return;
	}
	mercury_unassign_var(M, var1);
	mercury_variable* out = mercury_assign_var(M);
	out->data.i = 1;
	out->type = M_TYPE_BOOL; //true.
	mercury_pushstack(M, out);
}


void M_BYTECODE_EQL(mercury_state* M, uint16_t flags) {
	mercury_variable* var2 = mercury_popstack(M);
	mercury_variable* var1 = mercury_popstack(M);

	if (mercury_vars_equal(var1, var2)) {
		mercury_unassign_var(M, var1);
		mercury_unassign_var(M, var2);
		mercury_variable* out = mercury_assign_var(M);
		out->data.i = 1;
		out->type = M_TYPE_BOOL; //true.
		mercury_pushstack(M, out);
		return;
	}
	else {
		mercury_unassign_var(M, var1);
		mercury_unassign_var(M, var2);
		mercury_variable* out = mercury_assign_var(M);
		out->data.i = 0;
		out->type = M_TYPE_BOOL; //false.
		mercury_pushstack(M, out);
		return;
	}
}

void M_BYTECODE_NEQ(mercury_state* M, uint16_t flags) {
	mercury_variable* var2 = mercury_popstack(M);
	mercury_variable* var1 = mercury_popstack(M);

	if (mercury_vars_equal(var1, var2)) {
		mercury_unassign_var(M, var1);
		mercury_unassign_var(M, var2);
		mercury_variable* out = mercury_assign_var(M);
		out->data.i = 0;
		out->type = M_TYPE_BOOL; //false.
		mercury_pushstack(M, out);
		return;
	}
	else {
		mercury_unassign_var(M, var1);
		mercury_unassign_var(M, var2);
		mercury_variable* out = mercury_assign_var(M);
		out->data.i = 1;
		out->type = M_TYPE_BOOL; //true.
		mercury_pushstack(M, out);
		return;
	}
}


void M_BYTECODE_GRT(mercury_state* M, uint16_t flags) {
	uint8_t floatcount = 0;

	mercury_float f1=0;
	mercury_float f2=0;
	mercury_int i1=0;
	mercury_int i2=0;

	mercury_variable* outv = mercury_assign_var(M);
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			f1 = *(mercury_float*)offset;
			floatcount |= 2;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			f1 = var->data.f;
			floatcount |= 2;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			(floatcount ? f2 : f1) = *(mercury_float*)offset;
			floatcount |= 1;
		}
		else {
			(floatcount ? i1 : i2) = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			(floatcount ? i1 : i2) = var->data.i;
			break;
		case M_TYPE_FLOAT:
			(floatcount ? f2 : f1) = var->data.f;
			floatcount |= 1;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(floatcount ? M_TYPE_FLOAT : M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	switch (floatcount) {
	case 0:
		outv->data.i = i1 < i2;
		break;
	case 1:
		outv->data.i = (mercury_float)i1 < f1;
		break;
	case 2:
		outv->data.i = f1 < (mercury_float)i1;
		break;
	case 3:
		outv->data.i = f1 < f2;
		break;
	}
	outv->type = M_TYPE_BOOL;

	mercury_pushstack(M, outv);
}

void M_BYTECODE_LET(mercury_state* M, uint16_t flags) {
	uint8_t floatcount = 0;

	mercury_float f1 = 0;
	mercury_float f2 = 0;
	mercury_int i1 = 0;
	mercury_int i2 = 0;

	mercury_variable* outv = mercury_assign_var(M);
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			f1 = *(mercury_float*)offset;
			floatcount |= 2;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			f1 = var->data.f;
			floatcount |= 2;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			(floatcount ? f2 : f1) = *(mercury_float*)offset;
			floatcount |= 1;
		}
		else {
			(floatcount ? i1 : i2) = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			(floatcount ? i1 : i2) = var->data.i;
			break;
		case M_TYPE_FLOAT:
			(floatcount ? f2 : f1) = var->data.f;
			floatcount |= 1;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(floatcount ? M_TYPE_FLOAT : M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	switch (floatcount) {
	case 0:
		outv->data.i = i1 > i2;
		break;
	case 1:
		outv->data.i = (mercury_float)i1 > f1;
		break;
	case 2:
		outv->data.i = f1 > (mercury_float)i1;
		break;
	case 3:
		outv->data.i = f1 > f2;
		break;
	}
	outv->type = M_TYPE_BOOL;

	mercury_pushstack(M, outv);
}

void M_BYTECODE_GTE(mercury_state* M, uint16_t flags) {
	uint8_t floatcount = 0;

	mercury_float f1 = 0;
	mercury_float f2 = 0;
	mercury_int i1 = 0;
	mercury_int i2 = 0;

	mercury_variable* outv = mercury_assign_var(M);
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			f1 = *(mercury_float*)offset;
			floatcount |= 2;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			f1 = var->data.f;
			floatcount |= 2;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			(floatcount ? f2 : f1) = *(mercury_float*)offset;
			floatcount |= 1;
		}
		else {
			(floatcount ? i1 : i2) = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			(floatcount ? i1 : i2) = var->data.i;
			break;
		case M_TYPE_FLOAT:
			(floatcount ? f2 : f1) = var->data.f;
			floatcount |= 1;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(floatcount ? M_TYPE_FLOAT : M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	switch (floatcount) {
	case 0:
		outv->data.i = i1 <= i2;
		break;
	case 1:
		outv->data.i = (mercury_float)i1 <= f1;
		break;
	case 2:
		outv->data.i = f1 <= (mercury_float)i1;
		break;
	case 3:
		outv->data.i = f1 <= f2;
		break;
	}
	outv->type = M_TYPE_BOOL;

	mercury_pushstack(M, outv);
}

void M_BYTECODE_LTE(mercury_state* M, uint16_t flags) {
	uint8_t floatcount = 0;

	mercury_float f1 = 0;
	mercury_float f2 = 0;
	mercury_int i1 = 0;
	mercury_int i2 = 0;

	mercury_variable* outv = mercury_assign_var(M);
	if (outv == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (flags & M_INSTRUCTIONFLAG_ARG1STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG1ALT) {
			f1 = *(mercury_float*)offset;
			floatcount |= 2;
		}
		else {
			i1 = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			i1 = var->data.i;
			break;
		case M_TYPE_FLOAT:
			f1 = var->data.f;
			floatcount |= 2;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)M_TYPE_INT, (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	if (flags & M_INSTRUCTIONFLAG_ARG2STATIC) {
		void* offset = M->instructions + M->programcounter;
		if (flags & M_INSTRUCTIONFLAG_ARG2ALT) {
			(floatcount ? f2 : f1) = *(mercury_float*)offset;
			floatcount |= 1;
		}
		else {
			(floatcount ? i1 : i2) = *(mercury_int*)offset;
		}
#ifdef MERCURY_64BIT
		M->programcounter += 2;
#else
		M->programcounter += 1;
#endif
	}
	else {
		mercury_variable* var = mercury_popstack(M);
		if (var == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			mercury_unassign_var(M, outv);
			return;
		}
		switch (var->type)
		{
		case M_TYPE_INT:
			(floatcount ? i1 : i2) = var->data.i;
			break;
		case M_TYPE_FLOAT:
			(floatcount ? f2 : f1) = var->data.f;
			floatcount |= 1;
			break;
		default:
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(floatcount ? M_TYPE_FLOAT : M_TYPE_INT), (void*)var->type);
			mercury_unassign_var(M, outv);
			mercury_unassign_var(M, var);
			return;
		}
		mercury_unassign_var(M, var);
	}

	switch (floatcount) {
	case 0:
		outv->data.i = i1 >= i2;
		break;
	case 1:
		outv->data.i = (mercury_float)i1 >= f1;
		break;
	case 2:
		outv->data.i = f1 >= (mercury_float)i1;
		break;
	case 3:
		outv->data.i = f1 >= f2;
		break;
	}
	outv->type = M_TYPE_BOOL;

	mercury_pushstack(M, outv);
}

void M_BYTECODE_SENV(mercury_state* M, uint16_t flags) {
	mercury_variable* value = mercury_popstack(M);
	mercury_variable* key = mercury_popstack(M);

	//printf("k:%i / %i  v:%i / %i\n", key->type,key->data.i,value->type,value->data.i);


	mercury_state* check_state = M;
	while (check_state) {
		if (mercury_tablehaskey(check_state->enviroment,key)) {
			mercury_setkey(check_state->enviroment, key, value,M);
			return;
		}
		check_state = check_state->parentstate;
	}
	mercury_setkey(M->enviroment, key, value,M);
}

void M_BYTECODE_GENV(mercury_state* M, uint16_t flags) {
	mercury_variable* key = mercury_popstack(M);

	mercury_state* check_state = M;
	while (check_state) {
		if (mercury_tablehaskey(check_state->enviroment, key)) {
			mercury_pushstack(M, mercury_getkey(check_state->enviroment, key, M));
			//mercury_unassign_var(M, key);
			return;
		}
		check_state = check_state->parentstate;
	}
	//mercury_unassign_var(M, key);
	mercury_variable* out = mercury_assign_var(M);
	out->type = M_TYPE_NIL;
	out->data.i = 0;
	mercury_pushstack(M, out);
}


void M_BYTECODE_SET(mercury_state* M, uint16_t flags) {
	mercury_variable* value = mercury_popstack(M);
	mercury_variable* key = mercury_popstack(M);
	mercury_variable* table = mercury_popstack(M);

	switch (table->type) {
	case M_TYPE_TABLE:
		mercury_setkey((mercury_table*)table->data.p, key, value,M);
		break;
	case M_TYPE_ARRAY:
		if (key->type != M_TYPE_INT) {
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(M_TYPE_INT), (void*)table->type);
			mercury_unassign_var(M, value);
			mercury_unassign_var(M, key);
			mercury_unassign_var(M, table);
			return;
		}
		mercury_setarray((mercury_array*)table->data.p, value, key->data.i);
		mercury_unassign_var(M, key);
		break;
	default:
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(M_TYPE_TABLE), (void*)table->type);
		mercury_unassign_var(M, value);
		mercury_unassign_var(M, key);
		mercury_unassign_var(M, table);
		return;
	}
	//mercury_unassign_var(M, value);
	//mercury_unassign_var(M, key);
	mercury_unassign_var(M, table);
	
}

void M_BYTECODE_GET(mercury_state* M, uint16_t flags) {
	mercury_variable* key = mercury_popstack(M);
	mercury_variable* table = mercury_popstack(M);

	mercury_variable* out=nullptr;

	switch (table->type) {
	case M_TYPE_TABLE:
		out = mercury_getkey((mercury_table*)table->data.p, key, M);
		break;
	case M_TYPE_ARRAY:
		if (key->type != M_TYPE_INT) {
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(M_TYPE_INT), (void*)table->type);
			mercury_unassign_var(M, out);
			mercury_unassign_var(M, key);
			mercury_unassign_var(M, table);
			return;
		}
		out=mercury_getarray((mercury_array*)table->data.p, key->data.i);
		break;
	case M_TYPE_STRING:
		if (key->type != M_TYPE_INT) {
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(M_TYPE_INT), (void*)table->type);
			mercury_unassign_var(M, out);
			mercury_unassign_var(M, key);
			mercury_unassign_var(M, table);
			return;
		}
		out = mercury_assign_var(M);
		out->type = M_TYPE_STRING;
		out->data.p = mercury_mstring_substring((mercury_stringliteral*)table->data.p, key->data.i, key->data.i);
		break;
	default:
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)(M->programcounter - 1), (void*)(M_TYPE_TABLE), (void*)table->type);
		mercury_unassign_var(M, out);
		mercury_unassign_var(M, key);
		mercury_unassign_var(M, table);
		return;
	}

	key->type = M_TYPE_NIL; //do not destroy the key type if it is a string, since this will delete existing data.
	//mercury_unassign_var(M, key);
	mercury_unassign_var(M, table);
	mercury_pushstack(M, out);
}



void M_BYTECODE_SREG(mercury_state* M, uint16_t flags) {
	void* offset = M->instructions + M->programcounter;

	mercury_int regnum = *(mercury_int*)offset;

#ifdef MERCURY_64BIT
	M->programcounter += 2;
#else
	M->programcounter += 1;
#endif

	if (regnum > 0 and regnum <= register_max) {
		M->registers[regnum] = mercury_popstack(M);
	}


}


void M_BYTECODE_GREG(mercury_state* M, uint16_t flags) {
	void* offset = M->instructions + M->programcounter;

	mercury_int regnum = *(mercury_int*)offset;

#ifdef MERCURY_64BIT
	M->programcounter += 2;
#else
	M->programcounter += 1;
#endif

	if (regnum > 0 and regnum <= register_max) {
		mercury_pushstack(M, M->registers[regnum]);
	}
	else {
		mercury_variable* out = mercury_assign_var(M);
		if (out == nullptr) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			return;
		}
		out->type = M_TYPE_NIL;
		out->data.i = 0;
		mercury_pushstack(M, out);
	}

}

void M_BYTECODE_NINT(mercury_state* M, uint16_t flags) { //New INTeger
	void* offset = M->instructions + M->programcounter;
#ifdef MERCURY_64BIT
	M->programcounter += 2;
#else
	M->programcounter += 1;
#endif

	mercury_variable* out = mercury_assign_var(M);
	if (out == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}
	out->type = M_TYPE_INT;
	out->data.i= *(mercury_int*)offset;
	mercury_pushstack(M, out);
}

void M_BYTECODE_NFLO(mercury_state* M, uint16_t flags) { //New FLOat
	void* offset = M->instructions + M->programcounter;
#ifdef MERCURY_64BIT
	M->programcounter += 2;
#else
	M->programcounter += 1;
#endif

	mercury_variable* out = mercury_assign_var(M);
	if (out == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}
	out->type = M_TYPE_FLOAT;
	out->data.f = *(mercury_float*)offset;
	mercury_pushstack(M, out);
}

void M_BYTECODE_NTRU(mercury_state* M, uint16_t flags) { //New TRUe
	mercury_variable* out = mercury_assign_var(M);
	if (out == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}
	out->type = M_TYPE_BOOL;
	out->data.i = 1;
	mercury_pushstack(M, out);
}

void M_BYTECODE_NFAL(mercury_state * M, uint16_t flags) { //New FALse
	mercury_variable* out = mercury_assign_var(M);
	if (out == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}
	out->type = M_TYPE_BOOL;
	out->data.i = 0;
	mercury_pushstack(M, out);
}

void M_BYTECODE_NNIL(mercury_state* M, uint16_t flags) { //New NIL
	mercury_variable* out = mercury_assign_var(M);
	if (out == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}
	out->type = M_TYPE_NIL;
	out->data.i = 0;
	mercury_pushstack(M, out);
}

void M_BYTECODE_NSTR(mercury_state* M, uint16_t flags) { //New STRing
	void* offset = M->instructions + M->programcounter;
#ifdef MERCURY_64BIT
	M->programcounter += 2;
#else
	M->programcounter += 1;
#endif

	mercury_int string_size = *(mercury_int*)offset;

	mercury_variable* out = mercury_assign_var(M);
	if (!out) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}


	mercury_stringliteral* so = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!so) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}
	if (string_size) {
		/*
		char* sc = (char*)malloc(sizeof(char) * string_size);
		if (!sc) {
			mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
			return;
		}
		memcpy(sc, (char*)(M->instructions + M->programcounter), string_size * sizeof(char));
		
		so->ptr = sc;
		*/
		so->ptr = (char*)(M->instructions + M->programcounter);
		so->constant = true;
	}
	else so->ptr = nullptr;

	so->size = string_size;

	out->data.p = so;


	out->type = M_TYPE_STRING;
	//out->data.p = mercury_cstring_to_mstring( (char*)(M->instructions + M->programcounter),string_size); //avoid function call overhead
	M->programcounter += (string_size+3)/4;

	mercury_pushstack(M, out);

}

void M_BYTECODE_NFUN(mercury_state* M, uint16_t flags) { //New FUNction / No FUN
	void* offset = M->instructions + M->programcounter;
#ifdef MERCURY_64BIT
	M->programcounter += 2;
#else
	M->programcounter += 1;
#endif

	mercury_int function_size = *(mercury_int*)offset;

	mercury_function* fptr= (mercury_function*)malloc(sizeof(mercury_function));
	if (fptr == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	if (M->numberofinstructions < M->programcounter + function_size) {
		mercury_raise_error(M, M_ERROR_INSTRUCTION_FAILIURE, (void*)(M->programcounter - 1));
		return;
	}

	fptr->refrences = 1;
	fptr->numberofinstructions = function_size;
	fptr->instructions = (uint32_t*)malloc(function_size * sizeof(uint32_t));

	memcpy(fptr->instructions , M->instructions+M->programcounter , function_size * sizeof(uint32_t));

	if (fptr->instructions == nullptr) {
		free(fptr);
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	mercury_variable* out = mercury_assign_var(M);
	if (out == nullptr) {
		free(fptr->instructions);
		free(fptr);
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}
	out->type = M_TYPE_FUNCTION;
	out->data.p = fptr;
	M->programcounter += function_size;

	mercury_pushstack(M, out);

}

void M_BYTECODE_NTAB(mercury_state* M, uint16_t flags) { //New TABle
	mercury_variable* out = mercury_assign_var(M);
	if (out == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}
	mercury_table* ntab = mercury_newtable();
	if (ntab == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		mercury_unassign_var(M,out);
		return;
	}
	out->type = M_TYPE_TABLE;
	out->data.p = ntab;
	mercury_pushstack(M, out);
}

void M_BYTECODE_NARR(mercury_state* M, uint16_t flags) { //New ARRay
	mercury_variable* out = mercury_assign_var(M);
	if (out == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}
	mercury_array* narr = mercury_newarray();
	if (narr == nullptr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		mercury_unassign_var(M, out);
		return;
	}
	out->type = M_TYPE_ARRAY;
	out->data.p = narr;
	mercury_pushstack(M, out);
}

void M_BYTECODE_JMP(mercury_state* M, uint16_t flags) { //JuMP
	void* offset = M->instructions + M->programcounter;
#ifdef MERCURY_64BIT
	M->programcounter += 2;
#else
	M->programcounter += 1;
#endif


	mercury_int instruction = *(mercury_int*)offset;

	M->programcounter = instruction;
}

void M_BYTECODE_JMPR(mercury_state* M, uint16_t flags) { //JuMP Relative
	void* offset = M->instructions + M->programcounter;
#ifdef MERCURY_64BIT
	M->programcounter += 2;
#else
	M->programcounter += 1;
#endif
	mercury_int instruction = *(mercury_int*)offset;

	M->programcounter += instruction;
}

void M_BYTECODE_JIF(mercury_state* M, uint16_t flags) { //Jump IF
	void* offset = M->instructions + M->programcounter;
#ifdef MERCURY_64BIT
	M->programcounter += 2;
#else
	M->programcounter += 1;
#endif
	mercury_int instruction = *(mercury_int*)offset;


	mercury_variable* ck = mercury_popstack(M);
	if(mercury_checkbool(ck)){
		M->programcounter = instruction;
	}
	mercury_unassign_var(M, ck);
}

void M_BYTECODE_JNIF(mercury_state* M, uint16_t flags) { //Jump Not IF
	void* offset = M->instructions + M->programcounter;
#ifdef MERCURY_64BIT
	M->programcounter += 2;
#else
	M->programcounter += 1;
#endif
	mercury_int instruction = *(mercury_int*)offset;


	mercury_variable* ck = mercury_popstack(M);
	if (!mercury_checkbool(ck)) {
		M->programcounter = instruction;
	}
	mercury_unassign_var(M, ck);
}

void M_BYTECODE_JRIF(mercury_state* M, uint16_t flags) { //Jump Relative IF
	void* offset = M->instructions + M->programcounter;
#ifdef MERCURY_64BIT
	M->programcounter += 2;
#else
	M->programcounter += 1;
#endif
	mercury_int instruction = *(mercury_int*)offset;


	mercury_variable* ck = mercury_popstack(M);
	if (mercury_checkbool(ck)) {
		M->programcounter += instruction;
	}
	mercury_unassign_var(M, ck);
}

void M_BYTECODE_JRNI(mercury_state* M, uint16_t flags) { //Jump Relative Not If
	void* offset = M->instructions + M->programcounter;
#ifdef MERCURY_64BIT
	M->programcounter += 2;
#else
	M->programcounter += 1;
#endif
	mercury_int instruction = *(mercury_int*)offset;


	mercury_variable* ck = mercury_popstack(M);
	if (!mercury_checkbool(ck)) {
		M->programcounter += instruction;
	}
	mercury_unassign_var(M, ck);
}

void M_BYTECODE_CALL(mercury_state* M, uint16_t flags) { //CALL function
	void* offset = M->instructions + M->programcounter;
#ifdef MERCURY_64BIT
	M->programcounter += 2;
#else
	M->programcounter += 1;
#endif
	mercury_int args_in = *(mercury_int*)offset;

	offset = M->instructions + M->programcounter;
#ifdef MERCURY_64BIT
	M->programcounter += 2;
#else
	M->programcounter += 1;
#endif
	mercury_int args_out = *(mercury_int*)offset;


	mercury_variable* ck = mercury_popstack(M);
	switch (ck->type)
	{
	case M_TYPE_FUNCTION:
		{
		mercury_state* FM=mercury_newstate(M);
		mercury_function* func = (mercury_function*)ck->data.p;
		FM->instructions = func->instructions;
		FM->numberofinstructions = func->numberofinstructions;
		for (mercury_int i = 0; i < args_in;i++) {
			mercury_pushstack(FM, mercury_popstack(M));
		}
		while (mercury_stepstate(FM)) {};
		for (mercury_int i = 0; i < args_out; i++) {
			mercury_pushstack(M, mercury_pullstack(FM));
			
		}
		mercury_destroystate(FM);


		}
		
		break;
	case M_TYPE_CFUNC:
		((mercury_cfunc)(ck->data.p))(M,args_in,args_out);
		break;
	default:
		mercury_raise_error(M, M_ERROR_CALL_NOT_FUNCTION, (void*)(M->programcounter - 1) , (void*)(ck->type) );
		mercury_unassign_var(M, ck);
		return;
	}
	mercury_unassign_var(M, ck);
}

void M_BYTECODE_END(mercury_state* M, uint16_t flags) { //end state execution
	M->programcounter = M->numberofinstructions;
}

void M_BYTECODE_LEN(mercury_state* M, uint16_t flags) { //LENgth
	mercury_variable* var = mercury_popstack(M);
	mercury_variable* out; 
	switch (var->type) {
	case M_TYPE_ARRAY:
		out = mercury_assign_var(M);
		out->type = M_TYPE_INT;
		out->data.i = mercury_array_len((mercury_array*)var->data.p);
		break;
	case M_TYPE_STRING:
		out = mercury_assign_var(M);
		out->type = M_TYPE_INT;
		out->data.i = ((mercury_stringliteral*)var->data.p)->size;
		break;
	case M_TYPE_TABLE:
		out = mercury_assign_var(M);
		out->type = M_TYPE_INT;
		out->data.i = 0;
		{
			mercury_table* t = ((mercury_table*)var->data.p);
			for (uint8_t i = 0; i < M_NUMBER_OF_TYPES; i++) {
				out->data.i += t->data[i]->size;
			}
		}
		break;
	default:
		mercury_raise_error(M, M_ERROR_INDEX_INVALID_TYPE, (void*)(M->programcounter - 1), (void*)var->type);
		mercury_unassign_var(M, var);
		return;
	}
	mercury_pushstack(M,out);
}

void M_BYTECODE_CNCT(mercury_state* M, uint16_t flags) { // CoNCaTenate
	mercury_variable* v2 = mercury_popstack(M);
	mercury_variable* v1 = mercury_popstack(M);



	mercury_variable* s2;
	mercury_variable* s1;

	if (v2->type == M_TYPE_STRING) {
		s2 = v2;
	}
	else {
		s2 = mercury_tostring(v2);
		mercury_unassign_var(M, v2);
	}
	if (v1->type == M_TYPE_STRING) {
		s1 = v1;
	}
	else {
		s1 = mercury_tostring(v1);
		mercury_unassign_var(M, v1);
	}


	//mercury_stringliteral* nstr =mercury_mstrings_concat((mercury_stringliteral*)s1->data.p,(mercury_stringliteral*)s2->data.p);
	mercury_stringliteral* string1 = (mercury_stringliteral*)s1->data.p;
	mercury_stringliteral* string2 = (mercury_stringliteral*)s2->data.p;

	mercury_stringliteral* nstr = mercury_mstrings_concat(string1, string2); //mercury_cstring_to_mstring(string1->ptr, string1->size);
	//mercury_mstring_addchars(nstr, string2->ptr, string2->size);
	mercury_variable* out = mercury_assign_var(M);
	out->type = M_TYPE_STRING;
	out->data.p =nstr;

	mercury_unassign_var(M, s1);
	mercury_unassign_var(M, s2);

	mercury_pushstack(M, out);

}


void M_BYTECODE_CLS(mercury_state* M, uint16_t flags) { // CLear Stack
	//as simple as it gets, really.
	for (mercury_int i = 0; i < M->sizeofstack;i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

}


void M_BYTECODE_GETL(mercury_state* M, uint16_t flags) { //GET Local
	//yeah, this is pretty simple.
	mercury_pushstack(M,mercury_getkey(M->enviroment, mercury_popstack(M), M));
}

void M_BYTECODE_SETL(mercury_state* M, uint16_t flags) { //SET Local
	//ditto.
	mercury_variable* value=mercury_popstack(M);
	mercury_variable* key=mercury_popstack(M);
	mercury_setkey(M->enviroment, key, value,M);
}

void M_BYTECODE_GETG(mercury_state* M, uint16_t flags) { //GET Global
	//ditto.
	mercury_pushstack(M, mercury_getkey(M->masterstate->enviroment, mercury_popstack(M), M));
}

void M_BYTECODE_SETG(mercury_state* M, uint16_t flags) { //SET Global
	//ditto.
	mercury_variable* value = mercury_popstack(M);
	mercury_variable* key = mercury_popstack(M);
	mercury_setkey(M->masterstate->enviroment, key, value,M);
}

void M_BYTECODE_CPYT(mercury_state* M, uint16_t flags) { // CoPY Top (of stack)
	if (!M->sizeofstack)return; //nothing on stack, nothing to copy.

	mercury_variable* val= M->stack[M->sizeofstack - 1];
	mercury_variable* out = mercury_assign_var(M);
	if (!out) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)(M->programcounter - 1));
		return;
	}

	out->type = val->type;
	switch (val->type) {
	case M_TYPE_STRING:
	{
		mercury_stringliteral* str = (mercury_stringliteral*)val->data.p;
		out->data.p = mercury_copystring(str);// mercury_cstring_to_mstring(str->ptr, str->size); // cheap way to just copy a string. easy enough, even if it's probably not the best practice.
	}
	break;
	case M_TYPE_ARRAY:
	{
		mercury_array* arr = (mercury_array*)val->data.p;
		arr->refrences++;
	}
	default:
		out->data.i = val->data.i;
	}
	mercury_pushstack(M, out);
}


void M_BYTECODE_SWPT(mercury_state* M, uint16_t flags) { //SWaP Top. swaps the top and second top of stack.
	//so basically, 1,2 -> 2,1
	mercury_variable* v1 = mercury_popstack(M);
	mercury_variable* v2 = mercury_popstack(M);

	mercury_pushstack(M, v1);
	mercury_pushstack(M, v2);
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
};

