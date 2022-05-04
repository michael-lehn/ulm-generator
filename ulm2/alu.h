#ifndef ULM_ALU_H
#define ULM_ALU_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * Implements the ALU specified here:
 **/
#include "../ulm0/hdw_alu.h"

/*
 * Exept for the zero register regBytes returns a pointer to the
 * least significant byte of register reg.
 * For the zero register it returns a null pointer.
 **/
uint64_t *ulm_regDevice(ulm_Reg reg);

/*
 * Set all registers to zero
 **/
void ulm_aluClear();

void ulm_printALU(ulm_Reg firstReg, ulm_Reg lastReg);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // ULM_ALU_H
