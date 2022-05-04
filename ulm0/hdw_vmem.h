#ifndef ULM0_ULMHDW_H
#define ULM0_ULMHDW_H

#include <stddef.h>
#include <stdint.h>

#include "../ulm0/hdw_alu.h"

// VMEM
enum ulm_Extension
{
    ULM_ZERO_EXT,
    ULM_SIGN_EXT,
};

// for fetching data into an ALU register
void ulm_fetch64(int64_t disp, ulm_Reg base, ulm_Reg index, uint64_t scale,
		 enum ulm_Extension ext, size_t numBytes, ulm_Reg dest);
// for storing data from an ALU register
void ulm_store64(int64_t disp, ulm_Reg base, ulm_Reg index, uint64_t scale,
		 size_t numBytes, ulm_Reg src);

#endif // ULM0_ULMHDW_H
