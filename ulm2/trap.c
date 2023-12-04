#include <inttypes.h>
#include <stdint.h>

#include "trap.h"
#include "vmem.h"

//-- Implementation for "../ulm0/hdw_trap.h" -----------------------------------

void
ulm_trap(ulm_Reg reg0, ulm_Reg reg1, ulm_Reg reg2)
{
    (void) reg0; // prevent warning 'reg0 unused'
    (void) reg1; // prevent warning 'reg1 unused'
    (void) reg2; // prevent warning 'reg2 unused'
}
