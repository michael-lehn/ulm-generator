#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "uint128.h"

void mul_128(uint64_t a, uint64_t b, uint64_t *cL, uint64_t *cH) {
  uint64_t a0 = a & 0xFFFFFFFF;
  uint64_t a1 = a >> 32;
  uint64_t b0 = b & 0xFFFFFFFF;
  uint64_t b1 = b >> 32;

  uint64_t ab00 = a0 * b0;
  uint64_t ab01 = a0 * b1;
  uint64_t ab10 = a1 * b0;
  uint64_t ab11 = a1 * b1;

  *cL = (ab00 >> 32) + (ab01 & 0xFFFFFFFF) + (ab10 & 0xFFFFFFFF);
  *cH = (*cL >> 32) + (ab01 >> 32) + (ab10 >> 32) + ab11;
  *cL = (*cL << 32) + (ab00 & 0xFFFFFFFF);
}

void div_128(uint64_t a, uint64_t bL, uint64_t bH, uint64_t *cL, uint64_t *cH,
             uint64_t *r) {
  if (a == 0) {
    fprintf(stderr, "in div_128: division by 0.\n");
    abort();
  }

  *cL = 0;

  *cH = bH / a;
  bH = bH % a;

  if (a <= UINT32_MAX) {
    bH = (bH << 32) + (bL >> 32);
    *cL = bH / a;
    bH = bH % a;
    *cL = (*cL) << 32;
    bH = (bH << 32) + (bL & 0xFFFFFFFF);
    *cL += bH / a;
    bH = bH % a;
  } else {
    for (int n = 0; n < 64; ++n) {
      bH = (bH << 1) + ((bL << n) >> 63);
      if (bH >= a) {
        bH -= a;
        *cL = (*cL << 1) + 1;
      } else {
        *cL = (*cL << 1);
      }
    }
  }
  *r = bH;
}
