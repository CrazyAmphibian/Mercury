#pragma once

#include "mercury.hpp"
#include <stdint.h>


typedef void (*mercury_instruction) (mercury_state* M, uint16_t flags); //takes a state and 16 bit int for opflags.


enum mercury_instructionflags :uint16_t {
	M_INSTRUCTIONFLAG_ARG1STATIC = 1 << 0,
	M_INSTRUCTIONFLAG_ARG2STATIC = 1 << 1,

	M_INSTRUCTIONFLAG_ARG1ALT = 1 << 2,
	M_INSTRUCTIONFLAG_ARG2ALT = 1 << 3,

	M_INSTRUCTIONFLAG_UNDEFINED5 = 1 << 4,
	M_INSTRUCTIONFLAG_UNDEFINED6 = 1 << 5,
	M_INSTRUCTIONFLAG_UNDEFINED7 = 1 << 6,
	M_INSTRUCTIONFLAG_UNDEFINED8 = 1 << 7,
	M_INSTRUCTIONFLAG_UNDEFINED9 = 1 << 8,
	M_INSTRUCTIONFLAG_UNDEFINED10 = 1 << 9,
	M_INSTRUCTIONFLAG_UNDEFINED11 = 1 << 10,
	M_INSTRUCTIONFLAG_UNDEFINED12 = 1 << 11,
	M_INSTRUCTIONFLAG_UNDEFINED13 = 1 << 12,
	M_INSTRUCTIONFLAG_UNDEFINED14 = 1 << 13,
	M_INSTRUCTIONFLAG_UNDEFINED15 = 1 << 14,
	M_INSTRUCTIONFLAG_UNDEFINED16 = 1 << 15,

};




void M_BYTECODE_NOP(mercury_state* M, uint16_t flags);

void M_BYTECODE_ADD(mercury_state* M, uint16_t flags);
void M_BYTECODE_SUB(mercury_state* M, uint16_t flags);
void M_BYTECODE_MUL(mercury_state* M, uint16_t flags);
void M_BYTECODE_DIV(mercury_state* M, uint16_t flags);
void M_BYTECODE_POW(mercury_state* M, uint16_t flags);
void M_BYTECODE_IDIV(mercury_state* M, uint16_t flags);
void M_BYTECODE_MOD(mercury_state* M, uint16_t flags);

void M_BYTECODE_BAND(mercury_state* M, uint16_t flags);
void M_BYTECODE_BOR(mercury_state* M, uint16_t flags);
void M_BYTECODE_BXOR(mercury_state* M, uint16_t flags);
void M_BYTECODE_BNOT(mercury_state* M, uint16_t flags);
void M_BYTECODE_BSHL(mercury_state* M, uint16_t flags);
void M_BYTECODE_BSHR(mercury_state* M, uint16_t flags);

void M_BYTECODE_LAND(mercury_state* M, uint16_t flags);
void M_BYTECODE_LOR(mercury_state* M, uint16_t flags);
void M_BYTECODE_LXOR(mercury_state* M, uint16_t flags);
void M_BYTECODE_LNOT(mercury_state* M, uint16_t flags);

void M_BYTECODE_EQL(mercury_state* M, uint16_t flags);
void M_BYTECODE_NEQ(mercury_state* M, uint16_t flags);
void M_BYTECODE_GRT(mercury_state* M, uint16_t flags);
void M_BYTECODE_LET(mercury_state* M, uint16_t flags);
void M_BYTECODE_GTE(mercury_state* M, uint16_t flags);
void M_BYTECODE_LTE(mercury_state* M, uint16_t flags);

void M_BYTECODE_SENV(mercury_state* M, uint16_t flags);
void M_BYTECODE_GENV(mercury_state* M, uint16_t flags);
void M_BYTECODE_SET(mercury_state* M, uint16_t flags);
void M_BYTECODE_GET(mercury_state* M, uint16_t flags);
void M_BYTECODE_SREG(mercury_state* M, uint16_t flags);
void M_BYTECODE_GREG(mercury_state* M, uint16_t flags);

void M_BYTECODE_NINT(mercury_state* M, uint16_t flags);
void M_BYTECODE_NFLO(mercury_state* M, uint16_t flags);
void M_BYTECODE_NTRU(mercury_state* M, uint16_t flags);
void M_BYTECODE_NFAL(mercury_state* M, uint16_t flags);
void M_BYTECODE_NNIL(mercury_state* M, uint16_t flags);
void M_BYTECODE_NSTR(mercury_state* M, uint16_t flags);
void M_BYTECODE_NFUN(mercury_state* M, uint16_t flags);
void M_BYTECODE_NTAB(mercury_state* M, uint16_t flags);
void M_BYTECODE_NARR(mercury_state* M, uint16_t flags);

void M_BYTECODE_JMP(mercury_state* M, uint16_t flags);
void M_BYTECODE_JMPR(mercury_state* M, uint16_t flags);
void M_BYTECODE_JIF(mercury_state* M, uint16_t flags);
void M_BYTECODE_JNIF(mercury_state* M, uint16_t flags);
void M_BYTECODE_JRIF(mercury_state* M, uint16_t flags);
void M_BYTECODE_JRNI(mercury_state* M, uint16_t flags);

void M_BYTECODE_CALL(mercury_state* M, uint16_t flags);
void M_BYTECODE_END(mercury_state* M, uint16_t flags);

void M_BYTECODE_LEN(mercury_state* M, uint16_t flags);
void M_BYTECODE_CNCT(mercury_state* M, uint16_t flags);

void M_BYTECODE_CLS(mercury_state* M, uint16_t flags);

void M_BYTECODE_GETL(mercury_state* M, uint16_t flags);
void M_BYTECODE_SETL(mercury_state* M, uint16_t flags);
void M_BYTECODE_GETG(mercury_state* M, uint16_t flags);
void M_BYTECODE_SETG(mercury_state* M, uint16_t flags);

void M_BYTECODE_CPYT(mercury_state* M, uint16_t flags);
void M_BYTECODE_SWPT(mercury_state* M, uint16_t flags);
void M_BYTECODE_CPYX(mercury_state* M, uint16_t flags);

void M_BYTECODE_UNM(mercury_state* M, uint16_t flags);
void M_BYTECODE_INC(mercury_state* M, uint16_t flags);
void M_BYTECODE_DEC(mercury_state* M, uint16_t flags);

void M_BYTECODE_SCON(mercury_state* M, uint16_t flags);
void M_BYTECODE_GCON(mercury_state* M, uint16_t flags);

extern mercury_instruction mercury_bytecode_list[];


enum mercury_opcodes:uint16_t {
	M_OPCODE_NOP = 0,

	M_OPCODE_ADD = 1,
	M_OPCODE_SUB = 2,
	M_OPCODE_MUL = 3,
	M_OPCODE_DIV = 4,
	M_OPCODE_POW = 5,
	M_OPCODE_IDIV = 6,
	M_OPCODE_MOD = 7,

	M_OPCODE_BAND = 8,
	M_OPCODE_BOR = 9,
	M_OPCODE_BXOR = 10,
	M_OPCODE_BNOT = 11,
	M_OPCODE_BSHL = 12,
	M_OPCODE_BSHR = 13,

	M_OPCODE_LAND = 14,
	M_OPCODE_LOR = 15,
	M_OPCODE_LXOR = 16,
	M_OPCODE_LNOT = 17,

	M_OPCODE_EQL = 18,
	M_OPCODE_NEQ = 19,
	M_OPCODE_GRT = 20,
	M_OPCODE_LET = 21,
	M_OPCODE_GTE = 22,
	M_OPCODE_LTE = 23,

	M_OPCODE_SENV = 24,
	M_OPCODE_GENV = 25,
	M_OPCODE_SET = 26,
	M_OPCODE_GET = 27,
	M_OPCODE_SREG = 28,
	M_OPCODE_GREG = 29,

	M_OPCODE_NINT = 30,
	M_OPCODE_NFLO = 31,
	M_OPCODE_NTRU = 32,
	M_OPCODE_NFAL = 33,
	M_OPCODE_NNIL = 34,
	M_OPCODE_NSTR = 35,
	M_OPCODE_NFUN = 36,
	M_OPCODE_NTAB = 37,
	M_OPCODE_NARR = 38,

	M_OPCODE_JMP = 39,
	M_OPCODE_JMPR = 40,
	M_OPCODE_JIF = 41,
	M_OPCODE_JNIF = 42,
	M_OPCODE_JRIF = 43,
	M_OPCODE_JRNI = 44,

	M_OPCODE_CALL = 45,
	M_OPCODE_EXIT = 46,
	M_OPCODE_LEN = 47,
	M_OPCODE_CNCT = 48,
	M_OPCODE_CLS = 49,
	M_OPCODE_GETL = 50,
	M_OPCODE_SETL = 51,
	M_OPCODE_GETG = 52,
	M_OPCODE_SETG = 53,

	M_OPCODE_CPYT = 54,
	M_OPCODE_SWPT = 55,
	M_OPCODE_CPYX = 56,

	M_OPCODE_UNM = 57,
	M_OPCODE_INC = 58,
	M_OPCODE_DEC = 59,

	M_OPCODE_SCON = 60,
	M_OPCODE_GCON = 61,
};
