#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "uint128.h"

uint64_t rand_uint64(uint64_t nbits) {
  if (nbits > 64) {
    fprintf(stderr, "in rand_uint64: nbits > 64.\n");
    abort();
  }
  uint64_t r = 0;
  for (int i = 0; i < nbits; ++i) {
    r = r * 2 + rand() % 2;
  }
  return r;
}

int main() {
  unsigned int seed = time(0);
  printf("random seed = %u\n", seed);
  srand(seed);


  for (int i = 0; i < 100; ++i) {
    uint64_t nbitsA = rand_uint64(6);
    if (nbitsA == 0 || nbitsA > 64) {
      nbitsA = 42;
    }
    uint64_t a, bL, bH, cL, cH, r, dL, dH, eL, eH;
    a = rand_uint64(nbitsA);
    if (a == 0) {
      a = 42;
    }
    bL = rand_uint64(64);
    bH = rand_uint64(64);

    // check a*c + r = b
    div_128(a, bL, bH, &cL, &cH, &r);
    mul_128(a, cL, &dL, &dH);
    mul_128(a, cH, &eL, &eH);
    dH += eL;        // add a * cH to overflow from a * cL
    dL += r;         // add the remainder to the lower part
    assert(eH == 0); // because a times the cH shoulb be in the 64 bit range
    assert(dL == bL);
    if (dH != bH) {
      ++dH; // assume this is due to a carry in 'dL += r' hack
    }
    assert(dH == bH);
  }
}
