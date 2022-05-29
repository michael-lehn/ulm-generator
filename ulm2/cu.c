#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "alu.h"
#include "cu.h"
#include "io.h"
#include "trap.h"
#include "udb.h"
#include "vmem.h"

uint32_t ulm_instrReg = 0; // clang bug? not in BSS if not initialized
uint64_t ulm_instrPtr = 0;
bool ulm_halted;
bool ulm_waiting;
uint64_t ulm_exitCode;

static bool jmp;

void
ulm_cuClear()
{
    ulm_instrReg = 0;
    ulm_instrPtr = 0;
}

void
ulm_fetchInstr()
{
    ulm_fetch32(ulm_instrPtr, &ulm_instrReg);
}

void
ulm_incrInstrPtr()
{
    if (!(jmp || ulm_halted || ulm_waiting)) {
	ulm_instrPtr += 4;
    }
    jmp = false;
}

static void
illegalInstr(uint32_t opCode)
{
    /*
    // TODO: send notification instead of exit(1)
    printf("Opcode 0x%02" PRIX32 " not defined in instruction set.\n", opCode);
    printf("Instr 0x%08" PRIX32 "\n", ulm_instrReg);
    printf("Instr Ptr 0x%016" PRIX64 "\n", ulm_instrPtr);
    exit(1);
    */
    udb_illegalInstruction = true;
}

void
ulm_step()
{
    if (udb_on) {
	udb_inStep = true;
	udb_clearMsg();
    }

    ulm_fetchInstr();
#include <ulm1/_gen_instr.c>
    ulm_incrInstrPtr();
}

//-- Implementation for "../ulm0/hdw_cu.h" -------------------------------------

void
ulm_absJump(uint64_t addr, ulm_Reg retReg)
{
    ulm_setReg(ulm_instrPtr + 4, retReg);
    ulm_instrPtr = addr;
    jmp = true;
}

void
ulm_unconditionalRelJump(int64_t offset)
{
    ulm_instrPtr += offset;
    jmp = true;
}

void
ulm_conditionalRelJump(bool condition, int64_t offset)
{
    if (condition) {
	ulm_unconditionalRelJump(offset);
    }
}

uint64_t
ulm_ipVal()
{
    return ulm_instrPtr;
}

void
ulm_ipSet(uint64_t val)
{
    ulm_instrPtr = val;
    jmp = true;
}
