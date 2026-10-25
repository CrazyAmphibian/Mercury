#pragma once

#include "orcus.hpp"
#include <stdint.h>


typedef void (*orcus_instruction) (orcus_state* const M_CPP_restrict M); //takes a state


void M_BYTECODE_NOP(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_ADD(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_SUB(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_MUL(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_DIV(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_POW(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_IDIV(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_MOD(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_BAND(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_BOR(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_BXOR(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_BNOT(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_BSHL(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_BSHR(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_LAND(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_LOR(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_LXOR(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_LNOT(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_EQL(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_NEQ(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_GRT(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_LET(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_GTE(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_LTE(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_SENV(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_GENV(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_SET(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_GET(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_SREG(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_GREG(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_NINT(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_NFLO(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_NTRU(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_NFAL(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_NNIL(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_NSTR(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_NFUN(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_NTAB(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_NARR(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_JMP(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_JMPR(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_JIF(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_JNIF(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_JRIF(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_JRNI(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_CALL(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_END(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_LEN(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_CNCT(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_CLS(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_GETL(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_SETL(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_GETG(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_SETG(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_CPYT(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_SWPT(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_CPYX(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_UNM(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_INC(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_DEC(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_SCON(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_GCON(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_SWXY(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_NNSE(orcus_state* const M_CPP_restrict M);

void M_BYTECODE_SETL_KCS(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_GETL_KCS(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_SETG_KCS(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_GETG_KCS(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_SENV_KCS(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_GENV_KCS(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_SET_KCS(orcus_state* const M_CPP_restrict M);
void M_BYTECODE_GET_KCS(orcus_state* const M_CPP_restrict M);

extern orcus_instruction orcus_bytecode_list[];


enum orcus_opcodes:orcus_opcode {
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

	M_OPCODE_SWXY = 62,

	M_OPCODE_NNSE = 63,

	M_OPCODE_SETL_KCS=64,
	M_OPCODE_GETL_KCS=65,
	M_OPCODE_SETG_KCS=66,
	M_OPCODE_GETG_KCS=67,
	M_OPCODE_SENV_KCS = 68,
	M_OPCODE_GENV_KCS = 69,
	M_OPCODE_SET_KCS = 70,
	M_OPCODE_GET_KCS = 71,
};
