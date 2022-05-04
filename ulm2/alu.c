#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>

#include "alu.h"
#include "cu.h"
#include "udb.h"

enum
{
    NUM_REGS = 256
};

static uint64_t regDev[256];

void
ulm_aluClear()
{
    for (size_t i = 0; i < NUM_REGS; ++i) {
	regDev[i] = 0;
    }
}

uint64_t *
ulm_regDevice(ulm_Reg reg)
{
    return reg ? &regDev[reg] : 0;
}

void
ulm_printALU(ulm_Reg firstReg, ulm_Reg lastReg)
{
    printf("ZF = %d, CF = %d, SF = %d, OF = %d\n", ulm_statusReg[ULM_ZF],
	   ulm_statusReg[ULM_CF], ulm_statusReg[ULM_SF], ulm_statusReg[ULM_OF]);
    for (size_t i = firstReg; i <= lastReg; ++i) {
	printf("%%%02zx: 0x%016" PRIx64 "\n", i, regDev[i]);
    }
}

//-- Implementation for "../ulm0/hdw_alu.h" ------------------------------------

bool ulm_statusReg[ULM_NUM_STATUS_FLAGS];

uint64_t
ulm_regVal(ulm_Reg reg)
{
    if (udb_inStep) {
	udb_addRegMsg(reg, UDB_READ);
    }
    assert(reg < NUM_REGS);
    return reg ? regDev[reg] : 0;
}

void
ulm_setReg(uint64_t val, ulm_Reg reg)
{
    if (udb_inStep) {
	udb_addRegMsg(reg, UDB_WRITE);
    }
    assert(reg < NUM_REGS);
    regDev[reg] = reg ? val : 0;
}

void
ulm_add64(uint64_t a, uint64_t b, ulm_Reg dest)
{
    uint64_t s = b + a;
    int64_t a_ = a, b_ = b, s_ = s;

    ulm_setReg(s, dest);
    ulm_statusReg[ULM_ZF] = s == 0;
    ulm_statusReg[ULM_CF] = s < a || s < b;
    ulm_statusReg[ULM_SF] = s_ < 0;
    ulm_statusReg[ULM_OF] =
      (a_ > 0 && b_ > 0 && s_ < 0) || (a_ < 0 && b_ < 0 && s_ > 0);
}

void
ulm_sub64(uint64_t a, uint64_t b, ulm_Reg dest)
{
    uint64_t s = b - a;
    int64_t a_ = a, b_ = b, s_ = s;

    ulm_setReg(s, dest);
    ulm_statusReg[ULM_ZF] = s == 0;
    ulm_statusReg[ULM_CF] = s > b;
    ulm_statusReg[ULM_SF] = s_ < 0;
    ulm_statusReg[ULM_OF] =
      (a_ > 0 && b_ < 0 && s_ > 0) || (a_ < 0 && b_ > 0 && s_ < 0);
}

void
ulm_mul64(uint64_t a, uint64_t b, ulm_Reg dest)
{
    uint64_t s = b * a;
    ulm_setReg(s, dest);
}

void
ulm_mul128(uint64_t a, uint64_t b, ulm_Reg dest0, ulm_Reg dest1)
{
    uint64_t a0 = a & 0xFFFFFFFF;
    uint64_t a1 = a >> 32;
    uint64_t b0 = b & 0xFFFFFFFF;
    uint64_t b1 = b >> 32;

    uint64_t ab00 = a0 * b0;
    uint64_t ab01 = a0 * b1;
    uint64_t ab10 = a1 * b0;
    uint64_t ab11 = a1 * b1;

    uint64_t s0 = (ab00 >> 32) + (ab01 & 0xFFFFFFFF) + (ab10 & 0xFFFFFFFF);
    uint64_t s1 = (s0 >> 32) + (ab01 >> 32) + (ab10 >> 32) + ab11;
    s0 = (s0 << 32) + (ab00 & 0xFFFFFFFF);

    ulm_setReg(s0, dest0);
    ulm_setReg(s1, dest1);
    ulm_statusReg[ULM_CF] = s1 != 0;
}

void
ulm_idiv64(int64_t a, int64_t b, ulm_Reg dest)
{
    int64_t sgnA = a < 0 ? -1 : 1;
    int64_t sgnB = b < 0 ? -1 : 1;

    int64_t abs_a = a < 0 ? -a : a;
    int64_t abs_b = b < 0 ? -b : b;

    int64_t q = sgnA * sgnB * (abs_b / abs_a);
    int64_t r = b - a * q;

    ulm_setReg(q, dest);
    ulm_setReg(r, dest + 1);
    if (q * a + r != b) {
	fprintf(stderr, "b = %016" PRIX64 "\n", b);
	fprintf(stderr, "a = %016" PRIX64 "\n", a);
	fprintf(stderr, "q = %016" PRIX64 "\n", ulm_regVal(dest));
	fprintf(stderr, "r = %016" PRIX64 "\n", ulm_regVal(dest + 1));
	assert(q * a + r == b);
    }
}

void
ulm_div128(uint64_t a, uint64_t b0, uint64_t b1, ulm_Reg dest0, ulm_Reg dest1,
	   ulm_Reg dest2)
{
    unsigned __int128 b = b1;
    b = b << 64 | b0;

    unsigned __int128 q = b / a;
    uint64_t r = b % a;

    ulm_setReg(q & 0xFFFFFFFFFFFFFFFF, dest0);
    ulm_setReg((q >> 64) & 0xFFFFFFFFFFFFFFFF, dest1);
    ulm_setReg(r, dest2);

    if (q * a + r != b) {
	fprintf(stderr, "b = %016" PRIX64 "%016" PRIX64 "\n", b1, b0);
	fprintf(stderr, "a = %016" PRIX64 "\n", a);
	fprintf(stderr, "q = %016" PRIX64 "%016" PRIX64 "\n",
		ulm_regVal(dest1), ulm_regVal(dest0));
	fprintf(stderr, "r = %016" PRIX64 "\n", r);
	assert(q * a + r == b);
    }
}

void
ulm_shiftLeft64(uint64_t a, uint64_t b, ulm_Reg dest)
{
    uint64_t s = b << a;
    ulm_setReg(s, dest);
    ulm_statusReg[ULM_CF] = (b >> (64 - a)) & 0x1;
}

void
ulm_shiftRightSigned64(uint64_t a, int64_t b, ulm_Reg dest)
{
    uint64_t s = b >> a;
    ulm_setReg(s, dest);
}

void
ulm_shiftRightUnsigned64(uint64_t a, uint64_t b, ulm_Reg dest)
{
    uint64_t s = b >> a;
    ulm_setReg(s, dest);
}

void
ulm_and64(uint64_t a, uint64_t b, ulm_Reg dest)
{
    uint64_t s = a & b;
    ulm_setReg(s, dest);
    ulm_statusReg[ULM_ZF] = s == 0;
}

void
ulm_or64(uint64_t a, uint64_t b, ulm_Reg dest)
{
    uint64_t s = a | b;
    ulm_setReg(s, dest);
    ulm_statusReg[ULM_ZF] = s == 0;
}

void
ulm_not64(uint64_t a, ulm_Reg dest)
{
    uint64_t s = ~a;
    ulm_setReg(s, dest);
    ulm_statusReg[ULM_ZF] = s == 0;
}

