#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../ulm2/ulm.h"

bool bad_instr;

void
ulm_halt(uint64_t code)
{
    ulm_exitCode = code;
    ulm_halted = true;
}

void
illegalInstr(uint32_t opCode)
{
    bad_instr = true;
}

bool
run()
{
    while (!ulm_halted && !bad_instr) {
	ulm_fetchInstr();
#include <ulm1/_gen_instr.c>
	ulm_incrInstrPtr();
    }
    return bad_instr;
}

void
resetUlm()
{
    ulm_loadRestart();
    ulm_vmemClear();
    ulm_aluClear();
    ulm_cuClear();

    bad_instr = false;
    ulm_halted = false;
    ulm_instrPtr = 0;
}

bool
expected_regVal(uint64_t regId, uint64_t val)
{
    if (ulm_regVal(regId) != val) {
	fprintf(stderr,
		"Expected %%%" PRIX64 " == %016" PRIX64 ". Got %016" PRIX64
		"\n",
		regId, val, ulm_regVal(regId));
	return false;
    }
    return true;
}

bool
expected_exitCode(uint64_t val)
{
    if (ulm_exitCode != val) {
	fprintf(stderr,
		"Expected exit code %016" PRIX64 ". Got %016" PRIX64 "\n", val,
		ulm_exitCode);
	return false;
    }
    return true;
}

#include <ulm1/_gen_fmt_testcase.c>
