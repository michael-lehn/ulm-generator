#ifndef ULM0_HDWCU_H
#define ULM0_HDWCU_H

#include <stdbool.h>
#include <stdint.h>

#include "../ulm0/hdw_alu.h"

// CU
void ulm_absJump(uint64_t addr, ulm_Reg retReg);
void ulm_unconditionalRelJump(int64_t offset);
void ulm_conditionalRelJump(bool condition, int64_t offset);
void ulm_halt(uint64_t code);
uint64_t ulm_ipVal();
void ulm_ipSet(uint64_t val);

#endif // ULM0_HDWCU_H
