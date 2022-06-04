#ifndef ULM2_INSTRASM_H
#define ULM2_INSTRASM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void ulm_asm(uint32_t instr, char *s, size_t len);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // ULM2_INSTRASM_H

