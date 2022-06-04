#ifndef ULM2_CU_H
#define ULM2_CU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * Implements the CU specified here:
 **/
#include "../ulm0/hdw_cu.h"


extern uint32_t ulm_instrReg;
extern uint64_t ulm_instrPtr;

extern uint64_t ulm_exitCode;
extern bool ulm_halted;
extern bool ulm_waiting;

void ulm_cuClear();
void ulm_setHaltCallback(void(*callBack)(uint64_t));
void ulm_fetchInstr();
void ulm_incrInstrPtr();
void ulm_step();

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // ULM2_CU_H
