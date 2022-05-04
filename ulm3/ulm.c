#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../ulm2/ulm.h"

void
illegalInstr(uint32_t opCode)
{
    fprintf(stderr, "Opcode 0x%02" PRIX32 " not defined in instruction set.\n",
	    opCode);
    fprintf(stderr, "Instr 0x%08" PRIX32 "\n", ulm_instrReg);
    fprintf(stderr, "Instr Ptr 0x%016" PRIX64 "\n", ulm_instrPtr);
    exit(1);
}

void
ulm_halt(uint64_t code)
{
    exit(code);
}

void
run()
{
    while (true) {
	ulm_fetchInstr();

#include <ulm1/_gen_instr.c>

	ulm_incrInstrPtr();
    }
}


int
main(int argc, const char **argv)
{
    if (argc != 2) {
	fprintf(stderr, "usage: %s prog\n", argv[0]);
	return 1;
    }
    ulm_load(argv[1]);

    // printVMem(0, 255, 16);
    run();
}
