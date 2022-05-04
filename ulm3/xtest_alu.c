#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../ulm2/ulm.h"

void
illegalInstr(uint32_t opCode)
{
    printf("Opcode 0x%02" PRIX32 " not defined in instruction set.\n", opCode);
}

void
ulm_halt(uint64_t code)
{
    exit(code);
}

void
executeInstr()
{
#include <ulm1/_gen_instr.c>
}

int
main()
{
    ulm_setReg(3, 1);
    ulm_setReg(4, 2);
    ulm_printALU(0, 7);

    ulm_instrReg = 0x30010203;
    executeInstr();

    ulm_printALU(0, 7);
}
