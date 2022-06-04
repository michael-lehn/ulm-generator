#ifndef ULM2_UINT128_H
#define ULM2_UINT128_H

#include <stdint.h>

void mul_128(uint64_t a, uint64_t b, uint64_t *cL, uint64_t *cH);
void div_128(uint64_t a, uint64_t bL, uint64_t bH, uint64_t *cL, uint64_t *cH,
	     uint64_t *r);

#endif // ULM2_UINT128_H
