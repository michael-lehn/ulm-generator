#ifndef ULM0_HDWALU_H
#define ULM0_HDWALU_H

#include <stdbool.h>
#include <stdint.h>

// ALU
typedef uint64_t ulm_Reg;

enum
{
    ULM_ZF = 0,
    ULM_CF,
    ULM_SF,
    ULM_OF,
    ULM_NUM_STATUS_FLAGS,
};

extern bool ulm_statusReg[ULM_NUM_STATUS_FLAGS];

uint64_t ulm_regVal(ulm_Reg reg);
void ulm_setReg(uint64_t val, ulm_Reg reg);
void ulm_add64(uint64_t a, uint64_t b, ulm_Reg dest);
void ulm_sub64(uint64_t a, uint64_t b, ulm_Reg dest);
void ulm_mul64(uint64_t a, uint64_t b, ulm_Reg dest);
void ulm_mul128(uint64_t a, uint64_t b, ulm_Reg dest0, ulm_Reg dest1);
void ulm_div128(uint64_t a, uint64_t b0, uint64_t b1, ulm_Reg dest0,
		ulm_Reg dest1, ulm_Reg dest2);
void ulm_idiv64(int64_t a, int64_t b, ulm_Reg dest);
void ulm_shiftLeft64(uint64_t a, uint64_t b, ulm_Reg dest);
void ulm_shiftRightSigned64(uint64_t a, int64_t b, ulm_Reg dest);
void ulm_shiftRightUnsigned64(uint64_t a, uint64_t b, ulm_Reg dest);
void ulm_and64(uint64_t a, uint64_t b, ulm_Reg dest);
void ulm_or64(uint64_t a, uint64_t b, ulm_Reg dest);
void ulm_not64(uint64_t a, ulm_Reg dest);

#endif // ULM0_HDWALU_H
