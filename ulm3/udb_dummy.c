#include <stdio.h>
#include <stdlib.h>

#include "../ulm2/udb.h"

// dummy implementation for interface specified in udb.h
void
udb_addRegMsg(size_t regId, enum UdbMod udbMod)
{
    fprintf(stderr, "illegal call of 'udb_addRegMsg'\n");
    exit(1);
}

void
udb_addMemMsg(uint64_t addr, enum UdbMod udbMod)
{
    fprintf(stderr, "illegal call of 'udb_addMemMsg'\n");
    exit(1);
}

void
udb_notify()
{
    fprintf(stderr, "illegal call of 'udb_notify'\n");
    exit(1);
}

void
udb_clearMsg()
{
    fprintf(stderr, "illegal call of 'udb_clearMsg'\n");
    exit(1);
}

