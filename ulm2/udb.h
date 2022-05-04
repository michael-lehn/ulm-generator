#ifndef ULM2_UDB_H
#define ULM2_UDB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum UdbMod
{
    UDB_READ = 1,
    UDB_WRITE = 2,
    UDB_READ_WRITE = 3,
};

void ulm_halt(uint64_t code);

extern bool udb_on;
extern bool udb_inStep;
extern uint64_t udb_numPages;

void udb_addRegMsg(size_t regId, enum UdbMod udbMod);
void udb_addMemMsg(uint64_t addr, enum UdbMod udbMod);
void udb_notify();
void udb_clearMsg();

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // ULM2_UDB_H
