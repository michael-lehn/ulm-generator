#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../ulm2/ulm.h"

void
ulm_halt(uint64_t code)
{
    exit(code);
}

void
illegalInstr(uint32_t opCode)
{
    printf("Opcode 0x%02" PRIX32 " not defined in instruction set.\n", opCode);
}

void
executeInstr(uint32_t instr)
{
    ulm_instrReg = instr;
#include <ulm1/_gen_instr.c>
}

int
main()
{
    executeInstr(0x30010203);
}
